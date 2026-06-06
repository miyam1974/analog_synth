#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../Waveform.h"

class WaveformButton : public juce::Component
{
public:
    explicit WaveformButton(Waveform type);

    void setSelected(bool shouldBeSelected);
    bool getSelected() const { return selected; }

    std::function<void()> onSelect;

    void paint(juce::Graphics& g) override;
    bool hitTest(int x, int y) override;
    void mouseUp(const juce::MouseEvent& event) override;

    static void drawWaveformShape(juce::Graphics& g, juce::Rectangle<float> bounds,
                                  Waveform type, juce::Colour colour, float thickness);

private:
    Waveform waveform;
    bool selected = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformButton)
};
