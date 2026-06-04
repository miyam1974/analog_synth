#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#include "AdsrEnvelope.h"
#include "SynthSound.h"

class SynthVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SynthSound*>(sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound*, int) override;

    void stopNote(float, bool allowTailOff) override;

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample,
                         int numSamples) override;

    void setCurrentPlaybackSampleRate(double newRate) override;

    void panic();

    explicit SynthVoice(int voiceIndex = 0);

private:
    void publishPlayhead();
    void clearPlayhead();

    int voiceIndex = 0;

    double frequencyForMidiNote(int midiNote, int octaveOffset) const;
    void refreshEnvelopeParameters();
    void updateFilterCutoff(float filterEgLevel, float lfo1Value, float lfo2Value);
    float computeAmpGain(float ampEg) const;
    float mixOscillators();

    double sampleRate = 44100.0;
    double currentFrequencyHz = 0.0;
    double targetFrequencyHz = 0.0;
    double glideCoeff = 1.0;
    bool gliding = false;

    double osc1Angle = 0.0;
    double osc2Angle = 0.0;
    double subAngle = 0.0;

    float noteVelocity = 0.0f;
    int currentMidiNote = 60;

    AdsrEnvelope ampEnvelope;
    AdsrEnvelope filterEnvelope;

    juce::Random noiseGenerator;
    juce::dsp::StateVariableTPTFilter<float> filter;
};
