#pragma once

#include <array>
#include <atomic>
#include <cmath>

#include <juce_core/juce_core.h>

#include "SynthParameters.h"

class GlobalLfo
{
public:
    enum class Index
    {
        Lfo1 = 0,
        Lfo2 = 1
    };

    static void prepareForBlock(int numSamples, double sampleRate)
    {
        const auto count = juce::jmin(numSamples, kMaxBlockSize);
        blockSize = count;

        prepareOne(Index::Lfo1, count, sampleRate, SynthParameters::getLfoRateHz());
        prepareOne(Index::Lfo2, count, sampleRate, SynthParameters::getLfo2RateHz());
    }

    static float getLedLevel(Index index)
    {
        return states[static_cast<size_t>(index)].ledLevel.load(std::memory_order_relaxed);
    }

    static float valueAt(Index index, int sampleIndex)
    {
        if (blockSize <= 0)
            return 0.0f;

        const auto& buffer = states[static_cast<size_t>(index)].blockBuffer;
        return buffer[static_cast<size_t>(juce::jlimit(0, blockSize - 1, sampleIndex))];
    }

    static void resetPhase(Index index)
    {
        states[static_cast<size_t>(index)].phase = 0.0;
    }

    static void resetAllPhases()
    {
        resetPhase(Index::Lfo1);
        resetPhase(Index::Lfo2);
    }

private:
    static constexpr int kMaxBlockSize = 512;

    struct State
    {
        std::array<float, kMaxBlockSize> blockBuffer {};
        double phase = 0.0;
        std::atomic<float> ledLevel { 0.0f };
    };

    static void prepareOne(Index index, int count, double sampleRate, float rateHz)
    {
        auto& state = states[static_cast<size_t>(index)];
        const auto rate = static_cast<double>(rateHz);
        const auto increment = (juce::MathConstants<double>::twoPi * rate) / sampleRate;

        for (int i = 0; i < count; ++i)
        {
            state.phase += increment;
            if (state.phase >= juce::MathConstants<double>::twoPi)
                state.phase -= juce::MathConstants<double>::twoPi;

            state.blockBuffer[static_cast<size_t>(i)] = static_cast<float>(std::sin(state.phase));
        }

        const auto level = static_cast<float>((std::sin(state.phase) + 1.0) * 0.5);
        state.ledLevel.store(level, std::memory_order_relaxed);
    }

    inline static std::array<State, 2> states {};
    inline static int blockSize = 0;
};
