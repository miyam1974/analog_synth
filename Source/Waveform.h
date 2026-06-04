#pragma once

#include <atomic>
#include <cmath>

#include <juce_core/juce_core.h>

enum class Waveform
{
    Sine = 0,
    Saw,
    Square,
    Triangle,
    Count
};

inline const char* waveformName(Waveform type)
{
    switch (type)
    {
        case Waveform::Sine: return "Sine";
        case Waveform::Saw: return "Saw";
        case Waveform::Square: return "Square";
        case Waveform::Triangle: return "Triangle";
        default: return "Sine";
    }
}

inline float waveformSample(Waveform type, double phase)
{
    switch (type)
    {
        case Waveform::Saw:
            return static_cast<float>(2.0 * (phase / juce::MathConstants<double>::twoPi) - 1.0);

        case Waveform::Square:
            return phase < juce::MathConstants<double>::pi ? 1.0f : -1.0f;

        case Waveform::Triangle:
        {
            const auto t = phase / juce::MathConstants<double>::twoPi;
            return static_cast<float>(2.0 * std::abs(2.0 * t - 1.0) - 1.0);
        }

        case Waveform::Sine:
        default:
            return static_cast<float>(std::sin(phase));
    }
}
