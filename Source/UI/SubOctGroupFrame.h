#pragma once

#include "SynthTheme.h"

#include <juce_gui_basics/juce_gui_basics.h>

class SubOctGroupFrame : public juce::Component
{
public:
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);

        g.setColour(SynthTheme::panelFillHi);
        g.fillRoundedRectangle(bounds, 3.0f);
        SynthTheme::drawCornerBrackets(g, bounds, SynthTheme::accent.withAlpha(0.45f), 10.0f, 1.2f);
    }
};
