#include "EnvelopePlayhead.h"

std::array<EnvelopePlayheadDot, EnvelopePlayheadHub::kMaxVoices> EnvelopePlayheadHub::amp {};
std::array<EnvelopePlayheadDot, EnvelopePlayheadHub::kMaxVoices> EnvelopePlayheadHub::filter {};

namespace
{
EnvelopePlayheadDot& dotFor(bool isAmp, int voiceIndex)
{
    return isAmp ? EnvelopePlayheadHub::amp[static_cast<size_t>(voiceIndex)]
                 : EnvelopePlayheadHub::filter[static_cast<size_t>(voiceIndex)];
}
} // namespace

void EnvelopePlayheadHub::update(int voiceIndex, bool isAmp, const AdsrEnvelope& envelope)
{
    if (voiceIndex < 0 || voiceIndex >= kMaxVoices)
        return;

    auto& dot = dotFor(isAmp, voiceIndex);
    const auto timeline = envelope.getTimelinePosition(kSustainVisualWeight);

    if (timeline < 0.0f)
    {
        dot.timeline.store(-1.0f, std::memory_order_relaxed);
        return;
    }

    dot.timeline.store(timeline, std::memory_order_relaxed);
    dot.level.store(envelope.getLevel(), std::memory_order_relaxed);
}

void EnvelopePlayheadHub::clearSlot(int voiceIndex)
{
    if (voiceIndex < 0 || voiceIndex >= kMaxVoices)
        return;

    amp[static_cast<size_t>(voiceIndex)].timeline.store(-1.0f, std::memory_order_relaxed);
    filter[static_cast<size_t>(voiceIndex)].timeline.store(-1.0f, std::memory_order_relaxed);
}

void EnvelopePlayheadHub::clearAll()
{
    for (int i = 0; i < kMaxVoices; ++i)
        clearSlot(i);
}

bool EnvelopePlayheadHub::anyVisible(bool isAmp)
{
    const auto& dots = isAmp ? amp : filter;
    for (const auto& dot : dots)
    {
        if (dot.timeline.load(std::memory_order_relaxed) >= 0.0f)
            return true;
    }
    return false;
}

juce::Point<float> EnvelopePlayheadHub::pointOnCurve(float timeline, float attackSec,
                                                     float decaySec, float sustainLevel,
                                                     float releaseSec, juce::Rectangle<float> bounds)
{
    constexpr float kMinTimeSec = 0.001f;
    const auto attack = juce::jmax(kMinTimeSec, attackSec);
    const auto decay = juce::jmax(kMinTimeSec, decaySec);
    const auto sustain = juce::jlimit(0.0f, 1.0f, sustainLevel);
    const auto release = juce::jmax(kMinTimeSec, releaseSec);
    const auto totalTime = attack + decay + kSustainVisualWeight + release;

    const auto time = juce::jlimit(0.0f, totalTime, timeline * totalTime);
    const auto width = bounds.getWidth();
    const auto x = bounds.getX() + width * (time / totalTime);

    float level = 0.0f;
    if (time <= attack)
        level = attack > 0.0f ? time / attack : 1.0f;
    else if (time <= attack + decay)
    {
        const auto decayRange = juce::jmax(0.0001f, 1.0f - sustain);
        level = 1.0f - ((time - attack) / decay) * decayRange;
    }
    else if (time <= attack + decay + kSustainVisualWeight)
        level = sustain;
    else
    {
        const auto releaseTime = time - attack - decay - kSustainVisualWeight;
        const auto releaseProgress = release > 0.0f ? releaseTime / release : 1.0f;
        level = sustain * (1.0f - juce::jlimit(0.0f, 1.0f, releaseProgress));
    }

    const auto y = bounds.getBottom() - bounds.getHeight() * level;
    return { x, y };
}
