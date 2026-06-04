#include "ModulePanel.h"

#include "SynthTheme.h"

juce::Rectangle<int> ModulePanel::contentBounds(juce::Rectangle<int> panelBounds)
{
    panelBounds.removeFromTop(kTopMargin + kHeaderHeight + kGapBelowHeader);
    panelBounds.removeFromLeft(kSideMargin);
    panelBounds.removeFromRight(kSideMargin);
    panelBounds.removeFromBottom(kBottomMargin);
    return panelBounds;
}

ModulePanel::ModulePanel(juce::String moduleTitle)
    : title(std::move(moduleTitle))
{
    setInterceptsMouseClicks(false, false);
}

void ModulePanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(0.5f);

    g.setColour(SynthTheme::glow);
    g.fillRoundedRectangle(bounds.expanded(1.0f), 6.0f);

    g.setColour(SynthTheme::panelFill);
    g.fillRoundedRectangle(bounds, 5.0f);

    g.setColour(SynthTheme::accent.withAlpha(0.35f));
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);

    SynthTheme::drawCornerBrackets(g, bounds.reduced(4.0f), SynthTheme::accent, 16.0f, 1.5f);

    auto header = bounds;
    header.removeFromLeft(static_cast<float>(kSideMargin));
    header.removeFromRight(static_cast<float>(kSideMargin));
    header.removeFromTop(static_cast<float>(kTopMargin));
    header.setHeight(static_cast<float>(kHeaderHeight));

    g.setColour(SynthTheme::textPrimary);
    g.setFont(SynthTheme::titleFont(12.0f));
    g.drawText(title, header.withTrimmedLeft(2.0f), juce::Justification::centredLeft);

    g.setColour(SynthTheme::accentDim);
    g.fillRect(header.getX(), header.getBottom() - 1.0f, header.getWidth(), 1.0f);

    g.setColour(SynthTheme::accent.withAlpha(0.15f));
    g.fillRect(bounds.getX(), bounds.getBottom() - 1.0f, bounds.getWidth(), 1.0f);
}
