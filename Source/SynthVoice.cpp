#include "SynthVoice.h"

#include "EnvelopePlayhead.h"
#include "GlobalLfo.h"
#include "SynthParameters.h"
#include "Waveform.h"

#include <cmath>

namespace
{
double midiNoteToHz(int note)
{
    return 440.0 * std::pow(2.0, (note - 69) / 12.0);
}
} // namespace

SynthVoice::SynthVoice(int index)
    : voiceIndex(juce::jlimit(0, EnvelopePlayheadHub::kMaxVoices - 1, index))
{
}

void SynthVoice::clearPlayhead()
{
    EnvelopePlayheadHub::clearSlot(voiceIndex);
}

void SynthVoice::publishPlayhead()
{
    if (!ampEnvelope.isActive())
    {
        clearPlayhead();
        return;
    }

    EnvelopePlayheadHub::update(voiceIndex, true, ampEnvelope);
    EnvelopePlayheadHub::update(voiceIndex, false, filterEnvelope);
}

void SynthVoice::startNote(int midiNoteNumber, float velocity,
                           juce::SynthesiserSound*, int)
{
    currentMidiNote = midiNoteNumber;
    noteVelocity = juce::jlimit(0.0f, 1.0f, velocity);
    targetFrequencyHz = frequencyForMidiNote(midiNoteNumber, 0);

    const auto wasActive = isVoiceActive() && currentFrequencyHz > 0.0;
    beginPitchGlide(wasActive && SynthParameters::getGlideSec() > 0.001f);

    refreshEnvelopeParameters();
    ampEnvelope.noteOn();
    filterEnvelope.noteOn();
}

void SynthVoice::legatoNoteOn(int midiNoteNumber, float velocity)
{
    const auto wasSounding = isSoundingForMono();
    // True legato only while a key is still held (no Note Off yet). After Note Off the amp EG
    // may still be releasing; a new Note On must retrigger attack, not continue the tail.
    const auto legato = wasSounding && isKeyDown();

    setKeyDown(true);
    currentMidiNote = midiNoteNumber;
    noteVelocity = juce::jlimit(0.0f, 1.0f, velocity);
    targetFrequencyHz = frequencyForMidiNote(midiNoteNumber, 0);

    beginPitchGlide(legato && SynthParameters::getGlideSec() > 0.001f);
    refreshEnvelopeParameters();

    if (!legato)
    {
        ampEnvelope.noteOn();
        filterEnvelope.noteOn();
    }
}

bool SynthVoice::isSoundingForMono() const
{
    return ampEnvelope.isActive() && currentFrequencyHz > 0.0;
}

void SynthVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
    {
        ampEnvelope.noteOff();
        filterEnvelope.noteOff();
    }
    else
    {
        clearCurrentNote();
        ampEnvelope.reset();
        filterEnvelope.reset();
        currentFrequencyHz = 0.0;
        gliding = false;
        clearPlayhead();
    }
}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                 int startSample, int numSamples)
{
    if (!isVoiceActive() || currentFrequencyHz <= 0.0)
    {
        clearPlayhead();
        return;
    }

    const auto lfo1Depth = SynthParameters::getLfoDepth();
    const auto lfo2Depth = SynthParameters::getLfo2Depth();
    const auto lfoToPitch = SynthParameters::getLfoToPitch();
    const auto lfoToFilter = SynthParameters::getLfoToFilter();
    const auto lfoToAmp = SynthParameters::getLfoToAmp();
    const auto lfo2ToPitch = SynthParameters::getLfo2ToPitch();
    const auto lfo2ToFilter = SynthParameters::getLfo2ToFilter();
    const auto lfo2ToAmp = SynthParameters::getLfo2ToAmp();

    filter.setResonance(SynthParameters::getResonance());

    int sampleIndex = 0;
    while (--numSamples >= 0)
    {
        const auto lfo1Value = GlobalLfo::valueAt(GlobalLfo::Index::Lfo1, sampleIndex) * lfo1Depth;
        const auto lfo2Value = GlobalLfo::valueAt(GlobalLfo::Index::Lfo2, sampleIndex) * lfo2Depth;
        ++sampleIndex;

        if (gliding)
        {
            currentFrequencyHz += (targetFrequencyHz - currentFrequencyHz) * (1.0 - glideCoeff);
            if (std::abs(targetFrequencyHz - currentFrequencyHz) < 0.05)
            {
                currentFrequencyHz = targetFrequencyHz;
                gliding = false;
            }
        }

        const auto ampEg = ampEnvelope.advance();
        if (!ampEnvelope.isActive())
        {
            clearCurrentNote();
            currentFrequencyHz = 0.0;
            clearPlayhead();
            break;
        }

        const auto filterEg = filterEnvelope.advance();
        updateFilterCutoff(filterEg, lfo1Value, lfo2Value);

        auto frequencyHz = currentFrequencyHz;
        if (lfoToPitch)
        {
            const auto pitchModSemis = lfo1Value * 2.0f;
            frequencyHz *= std::pow(2.0, pitchModSemis / 12.0);
        }
        if (lfo2ToPitch)
        {
            const auto pitchModSemis = lfo2Value * 2.0f;
            frequencyHz *= std::pow(2.0, pitchModSemis / 12.0);
        }

        const auto twoPi = juce::MathConstants<double>::twoPi;
        const auto osc1Delta = (twoPi * frequencyHz) / sampleRate;
        const auto osc2DetuneRatio =
            std::pow(2.0, static_cast<double>(SynthParameters::getOsc2DetuneCents()) / 1200.0);
        const auto osc2Delta = osc1Delta * osc2DetuneRatio;
        const auto subDivisor = SynthParameters::getSubOctave() == SubOctave::Down2 ? 4.0 : 2.0;
        const auto subDelta = osc1Delta / subDivisor;

        auto gain = computeAmpGain(ampEg);
        if (lfoToAmp)
            gain *= 1.0f + lfo1Value * 0.8f;
        if (lfo2ToAmp)
            gain *= 1.0f + lfo2Value * 0.8f;

        const auto raw = mixOscillators() * gain;
        const auto sample = filter.processSample(0, raw);

        osc1Angle += osc1Delta;
        osc2Angle += osc2Delta;
        subAngle += subDelta;
        if (osc1Angle >= twoPi)
            osc1Angle -= twoPi;
        if (osc2Angle >= twoPi)
            osc2Angle -= twoPi;
        if (subAngle >= twoPi)
            subAngle -= twoPi;

        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            outputBuffer.addSample(channel, startSample, sample);

        ++startSample;
    }

    publishPlayhead();
}

void SynthVoice::panic()
{
    clearCurrentNote();
    ampEnvelope.reset();
    filterEnvelope.reset();
    currentFrequencyHz = 0.0;
    gliding = false;
    clearPlayhead();
}

void SynthVoice::setCurrentPlaybackSampleRate(double newRate)
{
    juce::SynthesiserVoice::setCurrentPlaybackSampleRate(newRate);
    sampleRate = newRate;

    ampEnvelope.setSampleRate(newRate);
    filterEnvelope.setSampleRate(newRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = newRate;
    spec.maximumBlockSize = 512;
    spec.numChannels = 1;

    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

void SynthVoice::beginPitchGlide(bool useGlide)
{
    if (useGlide)
    {
        gliding = true;
        const auto glideSamples = SynthParameters::getGlideSec() * sampleRate;
        glideCoeff = glideSamples > 1.0 ? std::exp(-1.0 / glideSamples) : 0.0;
        return;
    }

    currentFrequencyHz = targetFrequencyHz;
    gliding = false;
    osc1Angle = 0.0;
    osc2Angle = 0.0;
    subAngle = 0.0;
}

double SynthVoice::frequencyForMidiNote(int midiNote, int octaveOffset) const
{
    const auto semitones = SynthParameters::getTuneSemis() + SynthParameters::getFineCents() / 100.0f;
    const auto note = static_cast<double>(midiNote + octaveOffset * 12);
    return midiNoteToHz(static_cast<int>(note)) * std::pow(2.0, semitones / 12.0);
}

void SynthVoice::refreshEnvelopeParameters()
{
    ampEnvelope.setParameters(SynthParameters::clampEnvTime(SynthParameters::getAmpAttack()),
                              SynthParameters::clampEnvTime(SynthParameters::getAmpDecay()),
                              SynthParameters::getAmpSustain(),
                              SynthParameters::clampEnvTime(SynthParameters::getAmpRelease()));

    filterEnvelope.setParameters(SynthParameters::clampEnvTime(SynthParameters::getFilterAttack()),
                                 SynthParameters::clampEnvTime(SynthParameters::getFilterDecay()),
                                 SynthParameters::getFilterSustain(),
                                 SynthParameters::clampEnvTime(SynthParameters::getFilterRelease()));
}

void SynthVoice::updateFilterCutoff(float filterEgLevel, float lfo1Value, float lfo2Value)
{
    const auto base = SynthParameters::getCutoffHz();
    const auto envAmount = SynthParameters::getFilterEnvAmount();
    const auto envBoost = envAmount * filterEgLevel * 14000.0f;

    auto cutoff = base + envBoost;
    if (SynthParameters::getLfoToFilter())
        cutoff += lfo1Value * 6000.0f;
    if (SynthParameters::getLfo2ToFilter())
        cutoff += lfo2Value * 6000.0f;

    const auto velDepth = SynthParameters::getVelocityToFilter();
    if (velDepth > 0.0f)
        cutoff += noteVelocity * velDepth * 8000.0f;

    const auto keyTrack = SynthParameters::getFilterKeyTrack();
    if (keyTrack > 0.0f)
    {
        const auto noteDelta = static_cast<float>(currentMidiNote - 60);
        cutoff *= std::pow(2.0f, (noteDelta * keyTrack) / 12.0f);
    }

    filter.setCutoffFrequency(juce::jlimit(SynthParameters::minCutoffHz,
                                          SynthParameters::maxCutoffHz, cutoff));
}

float SynthVoice::computeAmpGain(float ampEg) const
{
    const auto velDepth = SynthParameters::getVelocityToAmp();
    const auto velScale = juce::jmap(velDepth, 0.0f, 1.0f, 1.0f, noteVelocity);
    return ampEg * velScale * 0.45f;
}

float SynthVoice::mixOscillators()
{
    const auto osc1 = waveformSample(SynthParameters::getOsc1Waveform(), osc1Angle);
    const auto osc2Enabled = SynthParameters::getOsc2Enabled();
    const auto osc2 = osc2Enabled ? waveformSample(SynthParameters::getOsc2Waveform(), osc2Angle) : 0.0f;
    const auto sub = waveformSample(SynthParameters::getOsc1Waveform(), subAngle);

    const auto osc1Level = SynthParameters::getOsc1Level();
    const auto osc2Level = SynthParameters::getOsc2Level();
    const auto subLevel = SynthParameters::getSubLevel();
    const auto noiseLevel = SynthParameters::getNoiseLevel();

    const auto noise = (noiseGenerator.nextFloat() * 2.0f - 1.0f) * noiseLevel;
    return osc1 * osc1Level + osc2 * osc2Level + sub * subLevel + noise;
}
