#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class ModulePanel : public juce::Component
{
public:
    static constexpr int kSideMargin = 10;
    static constexpr int kTopMargin = 10;
    static constexpr int kHeaderHeight = 22;
    static constexpr int kGapBelowHeader = 8;
    static constexpr int kBottomMargin = 10;

    static juce::Rectangle<int> contentBounds(juce::Rectangle<int> panelBounds);

    explicit ModulePanel(juce::String moduleTitle);

    void paint(juce::Graphics& g) override;

    void setTitle(juce::String newTitle) { title = std::move(newTitle); repaint(); }

private:
    juce::String title;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulePanel)
};
