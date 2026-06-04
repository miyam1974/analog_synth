#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class AdsrDisplay : public juce::Component,
                    private juce::Timer
{
public:
    enum class PlayheadSource
    {
        amplifier,
        filter
    };

    explicit AdsrDisplay(PlayheadSource source = PlayheadSource::amplifier);

    void setParameters(float attackSec, float decaySec, float sustainLevel, float releaseSec);

    void paint(juce::Graphics& g) override;

private:
    void timerCallback() override;
    void buildEnvelopePath(juce::Rectangle<float> bounds, juce::Path& strokePath,
                           juce::Path& fillPath) const;

    PlayheadSource playheadSource;
    float attack = 0.01f;
    float decay = 0.15f;
    float sustain = 0.7f;
    float release = 0.25f;
    bool playheadWasVisible = false;

    static constexpr float kMinTimeSec = 0.001f;
    static constexpr float kSustainVisualWeight = 0.32f;
    static constexpr float kNodeRadius = 2.5f;
    static constexpr float kPlayheadDotRadius = 4.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdsrDisplay)
};
