#include "FuturisticLookAndFeel.h"

#include "SynthTheme.h"

FuturisticLookAndFeel::FuturisticLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, SynthTheme::background);
    setColour(juce::TextButton::buttonColourId, SynthTheme::panelFill);
    setColour(juce::TextButton::buttonOnColourId, SynthTheme::accentMuted);
    setColour(juce::TextButton::textColourOffId, SynthTheme::textDim);
    setColour(juce::TextButton::textColourOnId, SynthTheme::accentBright);
    setColour(juce::ToggleButton::textColourId, SynthTheme::textDim);
    setColour(juce::ToggleButton::tickColourId, SynthTheme::accent);
    setColour(juce::ToggleButton::tickDisabledColourId, SynthTheme::accentDim);
    setColour(juce::Label::textColourId, SynthTheme::textPrimary);
    setColour(juce::Slider::backgroundColourId, SynthTheme::panelFillHi);
    setColour(juce::Slider::trackColourId, SynthTheme::accentDim);
    setColour(juce::Slider::thumbColourId, SynthTheme::accentBright);
    setColour(juce::Slider::textBoxTextColourId, SynthTheme::textPrimary);
    setColour(juce::Slider::textBoxBackgroundColourId, SynthTheme::panelFill);
    setColour(juce::Slider::textBoxOutlineColourId, SynthTheme::accentDim);
    setColour(juce::ComboBox::backgroundColourId, SynthTheme::panelFillHi);
    setColour(juce::ComboBox::textColourId, SynthTheme::textPrimary);
    setColour(juce::ComboBox::outlineColourId, SynthTheme::accentDim);
    setColour(juce::ComboBox::arrowColourId, SynthTheme::accent);
    setColour(juce::PopupMenu::backgroundColourId, SynthTheme::panelFill);
    setColour(juce::PopupMenu::textColourId, SynthTheme::textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, SynthTheme::accentMuted);
    setColour(juce::PopupMenu::highlightedTextColourId, SynthTheme::accentBright);
}

void FuturisticLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                               const juce::Colour&,
                                               bool highlighted, bool down)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);

    if (button.getComponentID() == "panic")
    {
        g.setColour(SynthTheme::panicGlow);
        g.fillRoundedRectangle(bounds.expanded(4.0f), 5.0f);

        const auto fill = down        ? SynthTheme::panicFill
                        : highlighted ? SynthTheme::panicFillHi
                                      : SynthTheme::panicFill;
        g.setColour(fill);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(SynthTheme::panicBorder.withAlpha(down ? 1.0f : 0.95f));
        g.drawRoundedRectangle(bounds, 4.0f, 2.0f);
        return;
    }

    if (button.getComponentID() == "tuneReset")
    {
        g.setColour(SynthTheme::accent.withAlpha(0.12f));
        g.fillRoundedRectangle(bounds.expanded(1.5f), 4.0f);
        g.setColour(highlighted ? SynthTheme::accentDim : SynthTheme::panelFillHi);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(highlighted ? SynthTheme::accent : SynthTheme::accentDim);
        g.drawRoundedRectangle(bounds, 4.0f, 1.5f);
        return;
    }

    const auto isOn = button.getToggleState();
    const auto fill = isOn ? SynthTheme::accentMuted.withAlpha(0.85f)
                           : SynthTheme::panelFillHi;
    const auto border = isOn ? SynthTheme::accent
                             : (highlighted ? SynthTheme::accentDim : SynthTheme::accentMuted);

    if (isOn)
    {
        g.setColour(SynthTheme::glow);
        g.fillRoundedRectangle(bounds.expanded(2.0f), 5.0f);
    }

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(border.withAlpha(down ? 1.0f : 0.75f));
    g.drawRoundedRectangle(bounds, 4.0f, isOn ? 2.0f : 1.0f);

    if (isOn)
        SynthTheme::drawCornerBrackets(g, bounds.reduced(3.0f), SynthTheme::accentBright, 8.0f, 1.0f);
}

void FuturisticLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                           bool highlighted, bool down)
{
    juce::ignoreUnused(highlighted, down);

    if (button.getComponentID() == "panic")
    {
        g.setColour(SynthTheme::panicText);
        g.setFont(SynthTheme::monoFont(static_cast<float>(button.getHeight()) * 0.38f));
        g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
        return;
    }

    LookAndFeel_V4::drawButtonText(g, button, highlighted, down);
}

void FuturisticLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                             bool highlighted, bool down)
{
    drawButtonBackground(g, button, {}, highlighted, down);

    g.setColour(button.getToggleState() ? SynthTheme::accentBright : SynthTheme::textDim);
    g.setFont(SynthTheme::labelFont(static_cast<float>(button.getHeight()) * 0.42f));
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
}

void FuturisticLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float minSliderPos, float maxSliderPos,
                                             const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    juce::ignoreUnused(minSliderPos, maxSliderPos);

    const auto isVertical = style == juce::Slider::LinearVertical
                         || style == juce::Slider::LinearBarVertical;

    auto track = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                         static_cast<float>(width), static_cast<float>(height))
                     .reduced(isVertical ? width * 0.35f : 0.0f,
                              isVertical ? 0.0f : height * 0.35f);

    g.setColour(SynthTheme::panelFillHi);
    g.fillRoundedRectangle(track, 3.0f);
    g.setColour(SynthTheme::accentDim.withAlpha(0.5f));
    g.drawRoundedRectangle(track, 3.0f, 1.0f);

    juce::Rectangle<float> fillTrack;
    if (isVertical)
    {
        const auto thumbY = sliderPos;
        fillTrack = { track.getX(), thumbY, track.getWidth(), track.getBottom() - thumbY };
    }
    else
    {
        fillTrack = { track.getX(), track.getY(), sliderPos - track.getX(), track.getHeight() };
    }

    juce::ColourGradient gradient(SynthTheme::accentDim, fillTrack.getTopLeft(),
                                  SynthTheme::accent, fillTrack.getBottomRight(), false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(fillTrack.reduced(1.0f), 2.0f);

    const auto thumbSize = isVertical ? track.getWidth() + 6.0f : track.getHeight() + 6.0f;
    juce::Rectangle<float> thumb;
    if (isVertical)
        thumb = { track.getCentreX() - thumbSize * 0.5f, sliderPos - 4.0f, thumbSize, 8.0f };
    else
        thumb = { sliderPos - 4.0f, track.getCentreY() - thumbSize * 0.5f, 8.0f, thumbSize };

    g.setColour(SynthTheme::glow);
    g.fillRoundedRectangle(thumb.expanded(2.0f), 2.0f);
    g.setColour(slider.isEnabled() ? SynthTheme::accentBright : SynthTheme::textDim);
    g.fillRoundedRectangle(thumb, 2.0f);
}

void FuturisticLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(getLabelFont(label));
    g.drawText(label.getText(), label.getLocalBounds(), label.getJustificationType());
}

juce::Font FuturisticLookAndFeel::getTextButtonFont(juce::TextButton& button, int buttonHeight)
{
    if (button.getComponentID() == "tuneReset")
        return SynthTheme::monoFont(static_cast<float>(buttonHeight) * 0.48f);

    if (button.getComponentID() == "panic")
        return SynthTheme::monoFont(static_cast<float>(buttonHeight) * 0.38f);

    return SynthTheme::labelFont(static_cast<float>(buttonHeight) * 0.42f);
}

juce::Font FuturisticLookAndFeel::getLabelFont(juce::Label&)
{
    return SynthTheme::labelFont();
}
