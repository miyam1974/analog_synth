#pragma once

#include <atomic>

#include <juce_core/juce_core.h>

#include "Waveform.h"

enum class SubOctave
{
    Down1 = 0,
    Down2 = 1
};

struct SynthParameters
{
    // Oscillator
    static constexpr float minTuneSemis = -12.0f;
    static constexpr float maxTuneSemis = 12.0f;
    static constexpr float minFineCents = -100.0f;
    static constexpr float maxFineCents = 100.0f;
    static constexpr float minOscLevel = 0.0f;
    static constexpr float maxOscLevel = 1.0f;
    static constexpr float minNoiseLevel = 0.0f;
    static constexpr float maxNoiseLevel = 1.0f;
    static constexpr float minSubLevel = 0.0f;
    static constexpr float maxSubLevel = 1.0f;

    // Filter
    static constexpr float minCutoffHz = 80.0f;
    static constexpr float maxCutoffHz = 18000.0f;
    static constexpr float minResonance = 0.1f;
    static constexpr float maxResonance = 8.0f;
    static constexpr float minFilterEnvAmount = 0.0f;
    static constexpr float maxFilterEnvAmount = 1.0f;
    static constexpr float minFilterKeyTrack = 0.0f;
    static constexpr float maxFilterKeyTrack = 1.0f;

    static constexpr float minOsc2DetuneCents = -100.0f;
    static constexpr float maxOsc2DetuneCents = 100.0f;

    // Envelopes (seconds; sustain is 0..1)
    static constexpr float minEnvTime = 0.001f;
    static constexpr float maxEnvTime = 5.0f;

    // LFO
    static constexpr float minLfoRateHz = 0.05f;
    static constexpr float maxLfoRateHz = 20.0f;
    static constexpr float minLfoDepth = 0.0f;
    static constexpr float maxLfoDepth = 1.0f;

    // Performance
    static constexpr float minGlideSec = 0.0f;
    static constexpr float maxGlideSec = 2.0f;
    static constexpr float minVelocityDepth = 0.0f;
    static constexpr float maxVelocityDepth = 1.0f;

    // Master
    static constexpr float minMasterLevel = 0.0f;
    static constexpr float maxMasterLevel = 1.0f;

    static void setOsc1Waveform(Waveform w) { osc1Waveform.store(w, std::memory_order_relaxed); }
    static Waveform getOsc1Waveform() { return osc1Waveform.load(std::memory_order_relaxed); }

    static void setOsc2Waveform(Waveform w) { osc2Waveform.store(w, std::memory_order_relaxed); }
    static Waveform getOsc2Waveform() { return osc2Waveform.load(std::memory_order_relaxed); }

    // Legacy alias (OSC1)
    static void setWaveform(Waveform w) { setOsc1Waveform(w); }
    static Waveform getWaveform() { return getOsc1Waveform(); }

    static void setOsc1Level(float v) { osc1Level.store(v, std::memory_order_relaxed); }
    static float getOsc1Level() { return osc1Level.load(std::memory_order_relaxed); }

    static void setOsc2Level(float v) { osc2Level.store(v, std::memory_order_relaxed); }
    static float getOsc2Level() { return osc2Level.load(std::memory_order_relaxed); }

    static void setSubLevel(float v) { subLevel.store(v, std::memory_order_relaxed); }
    static float getSubLevel() { return subLevel.load(std::memory_order_relaxed); }

    static void setSubOctave(SubOctave o) { subOctave.store(o, std::memory_order_relaxed); }
    static SubOctave getSubOctave() { return subOctave.load(std::memory_order_relaxed); }

    static void setTuneSemis(float v)
    {
        tuneSemis.store(static_cast<float>(juce::roundToInt(juce::jlimit(minTuneSemis, maxTuneSemis, v))),
                        std::memory_order_relaxed);
    }
    static float getTuneSemis() { return tuneSemis.load(std::memory_order_relaxed); }

    static void setFineCents(float v) { fineCents.store(v, std::memory_order_relaxed); }
    static float getFineCents() { return fineCents.load(std::memory_order_relaxed); }

    static void setNoiseLevel(float v) { noiseLevel.store(v, std::memory_order_relaxed); }
    static float getNoiseLevel() { return noiseLevel.load(std::memory_order_relaxed); }

    static void setCutoffHz(float v) { cutoffHz.store(v, std::memory_order_relaxed); }
    static float getCutoffHz() { return cutoffHz.load(std::memory_order_relaxed); }

    static void setResonance(float v) { resonance.store(v, std::memory_order_relaxed); }
    static float getResonance() { return resonance.load(std::memory_order_relaxed); }

    static void setFilterEnvAmount(float v) { filterEnvAmount.store(v, std::memory_order_relaxed); }
    static float getFilterEnvAmount() { return filterEnvAmount.load(std::memory_order_relaxed); }

    static void setFilterKeyTrack(float v) { filterKeyTrack.store(v, std::memory_order_relaxed); }
    static float getFilterKeyTrack() { return filterKeyTrack.load(std::memory_order_relaxed); }

    static void setOsc2DetuneCents(float v) { osc2DetuneCents.store(v, std::memory_order_relaxed); }
    static float getOsc2DetuneCents() { return osc2DetuneCents.load(std::memory_order_relaxed); }

    static void setAmpAttack(float v) { ampAttack.store(v, std::memory_order_relaxed); }
    static void setAmpDecay(float v) { ampDecay.store(v, std::memory_order_relaxed); }
    static void setAmpSustain(float v) { ampSustain.store(v, std::memory_order_relaxed); }
    static void setAmpRelease(float v) { ampRelease.store(v, std::memory_order_relaxed); }
    static float getAmpAttack() { return ampAttack.load(std::memory_order_relaxed); }
    static float getAmpDecay() { return ampDecay.load(std::memory_order_relaxed); }
    static float getAmpSustain() { return ampSustain.load(std::memory_order_relaxed); }
    static float getAmpRelease() { return ampRelease.load(std::memory_order_relaxed); }

    static void setFilterAttack(float v) { filterAttack.store(v, std::memory_order_relaxed); }
    static void setFilterDecay(float v) { filterDecay.store(v, std::memory_order_relaxed); }
    static void setFilterSustain(float v) { filterSustain.store(v, std::memory_order_relaxed); }
    static void setFilterRelease(float v) { filterRelease.store(v, std::memory_order_relaxed); }
    static float getFilterAttack() { return filterAttack.load(std::memory_order_relaxed); }
    static float getFilterDecay() { return filterDecay.load(std::memory_order_relaxed); }
    static float getFilterSustain() { return filterSustain.load(std::memory_order_relaxed); }
    static float getFilterRelease() { return filterRelease.load(std::memory_order_relaxed); }

    static void setLfoRateHz(float v) { lfoRateHz.store(v, std::memory_order_relaxed); }
    static void setLfoDepth(float v) { lfoDepth.store(v, std::memory_order_relaxed); }
    static float getLfoRateHz() { return lfoRateHz.load(std::memory_order_relaxed); }
    static float getLfoDepth() { return lfoDepth.load(std::memory_order_relaxed); }

    static void setLfo2RateHz(float v) { lfo2RateHz.store(v, std::memory_order_relaxed); }
    static void setLfo2Depth(float v) { lfo2Depth.store(v, std::memory_order_relaxed); }
    static float getLfo2RateHz() { return lfo2RateHz.load(std::memory_order_relaxed); }
    static float getLfo2Depth() { return lfo2Depth.load(std::memory_order_relaxed); }

    static void setLfoToPitch(bool v) { lfoToPitch.store(v, std::memory_order_relaxed); }
    static void setLfoToFilter(bool v) { lfoToFilter.store(v, std::memory_order_relaxed); }
    static void setLfoToAmp(bool v) { lfoToAmp.store(v, std::memory_order_relaxed); }
    static bool getLfoToPitch() { return lfoToPitch.load(std::memory_order_relaxed); }
    static bool getLfoToFilter() { return lfoToFilter.load(std::memory_order_relaxed); }
    static bool getLfoToAmp() { return lfoToAmp.load(std::memory_order_relaxed); }

    static void setLfo2ToPitch(bool v) { lfo2ToPitch.store(v, std::memory_order_relaxed); }
    static void setLfo2ToFilter(bool v) { lfo2ToFilter.store(v, std::memory_order_relaxed); }
    static void setLfo2ToAmp(bool v) { lfo2ToAmp.store(v, std::memory_order_relaxed); }
    static bool getLfo2ToPitch() { return lfo2ToPitch.load(std::memory_order_relaxed); }
    static bool getLfo2ToFilter() { return lfo2ToFilter.load(std::memory_order_relaxed); }
    static bool getLfo2ToAmp() { return lfo2ToAmp.load(std::memory_order_relaxed); }

    static void setGlideSec(float v) { glideSec.store(v, std::memory_order_relaxed); }
    static float getGlideSec() { return glideSec.load(std::memory_order_relaxed); }

    static void setMonoMode(bool v) { monoMode.store(v, std::memory_order_relaxed); }
    static bool getMonoMode() { return monoMode.load(std::memory_order_relaxed); }

    static void setVelocityToAmp(float v) { velocityToAmp.store(v, std::memory_order_relaxed); }
    static float getVelocityToAmp() { return velocityToAmp.load(std::memory_order_relaxed); }

    static void setVelocityToFilter(float v) { velocityToFilter.store(v, std::memory_order_relaxed); }
    static float getVelocityToFilter() { return velocityToFilter.load(std::memory_order_relaxed); }

    static void setMasterLevel(float v) { masterLevel.store(v, std::memory_order_relaxed); }
    static float getMasterLevel() { return masterLevel.load(std::memory_order_relaxed); }

    static float clampEnvTime(float seconds)
    {
        return juce::jlimit(minEnvTime, maxEnvTime, seconds);
    }

private:
    inline static std::atomic<Waveform> osc1Waveform { Waveform::Saw };
    inline static std::atomic<Waveform> osc2Waveform { Waveform::Square };
    inline static std::atomic<float> osc1Level { 0.85f };
    inline static std::atomic<float> osc2Level { 0.0f };
    inline static std::atomic<float> subLevel { 0.0f };
    inline static std::atomic<SubOctave> subOctave { SubOctave::Down1 };

    inline static std::atomic<float> tuneSemis { 0.0f };
    inline static std::atomic<float> fineCents { 0.0f };
    inline static std::atomic<float> noiseLevel { 0.0f };

    inline static std::atomic<float> cutoffHz { 6000.0f };
    inline static std::atomic<float> resonance { 0.707f };
    inline static std::atomic<float> filterEnvAmount { 0.5f };
    inline static std::atomic<float> filterKeyTrack { 0.0f };

    inline static std::atomic<float> osc2DetuneCents { 0.0f };

    inline static std::atomic<float> ampAttack { 0.01f };
    inline static std::atomic<float> ampDecay { 0.15f };
    inline static std::atomic<float> ampSustain { 0.7f };
    inline static std::atomic<float> ampRelease { 0.25f };

    inline static std::atomic<float> filterAttack { 0.02f };
    inline static std::atomic<float> filterDecay { 0.35f };
    inline static std::atomic<float> filterSustain { 0.35f };
    inline static std::atomic<float> filterRelease { 0.4f };

    inline static std::atomic<float> lfoRateHz { 4.0f };
    inline static std::atomic<float> lfoDepth { 0.3f };
    inline static std::atomic<bool> lfoToPitch { false };
    inline static std::atomic<bool> lfoToFilter { true };
    inline static std::atomic<bool> lfoToAmp { false };

    inline static std::atomic<float> lfo2RateHz { 0.8f };
    inline static std::atomic<float> lfo2Depth { 0.0f };
    inline static std::atomic<bool> lfo2ToPitch { false };
    inline static std::atomic<bool> lfo2ToFilter { false };
    inline static std::atomic<bool> lfo2ToAmp { false };

    inline static std::atomic<float> glideSec { 0.0f };
    inline static std::atomic<bool> monoMode { false };
    inline static std::atomic<float> velocityToAmp { 0.6f };
    inline static std::atomic<float> velocityToFilter { 0.35f };

    inline static std::atomic<float> masterLevel { 0.85f };
};
