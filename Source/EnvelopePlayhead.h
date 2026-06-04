#pragma once

#include <array>
#include <atomic>

#include <juce_graphics/juce_graphics.h>

#include "AdsrEnvelope.h"

struct EnvelopePlayheadDot
{
    std::atomic<float> timeline { -1.0f };
    std::atomic<float> level { 0.0f };
};

class EnvelopePlayheadHub
{
public:
    static constexpr int kMaxVoices = 16;
    static constexpr float kSustainVisualWeight = 0.32f;

    static std::array<EnvelopePlayheadDot, kMaxVoices> amp;
    static std::array<EnvelopePlayheadDot, kMaxVoices> filter;

    static void update(int voiceIndex, bool isAmp, const AdsrEnvelope& envelope);

    static void clearSlot(int voiceIndex);

    static void clearAll();

    static bool anyVisible(bool isAmp);

    static juce::Point<float> pointOnCurve(float timeline, float attackSec, float decaySec,
                                           float sustainLevel, float releaseSec,
                                           juce::Rectangle<float> bounds);
};
