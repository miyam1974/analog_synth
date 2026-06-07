#pragma once

#include <array>
#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

class PcKeyboardDisplay : public juce::Component,
                          private juce::Timer
{
public:
    std::function<void()> onClicked;

    PcKeyboardDisplay();

    void setMappingEnabled(bool enabled);
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    struct KeyVisual
    {
        const char* label;
        int keyCode;
        float column;
        bool upperRow;
    };

    void timerCallback() override;
    static bool isKeyDown(int keyCode);
    static const std::array<KeyVisual, 17>& getKeys();

    bool mappingEnabled = true;
};
