#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../GlobalLfo.h"

class LfoRateLed : public juce::Component,
                   private juce::Timer
{
public:
    explicit LfoRateLed(GlobalLfo::Index lfoIndex = GlobalLfo::Index::Lfo1);

    void paint(juce::Graphics& g) override;

private:
    void timerCallback() override;

    GlobalLfo::Index lfoIndex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfoRateLed)
};
