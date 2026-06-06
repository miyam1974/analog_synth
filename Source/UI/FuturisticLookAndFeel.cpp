#include "FuturisticLookAndFeel.h"

#include "SynthTheme.h"

namespace
{
constexpr float kStandardTextButtonFontScale = 0.48f;
constexpr float kMasterBarButtonFontSize = 11.0f;
constexpr int kSystemComboLeftPadding = 8;

bool isMasterBarButton(const juce::TextButton& button)
{
    const auto& id = button.getComponentID();
    return id == "panic" || id == "masterBarText" || id == "presetSave" || id == "diffToggle";
}

void drawMasterBarButtonText(juce::Graphics& g, juce::TextButton& button, juce::Colour colour)
{
    g.setColour(colour);
    g.setFont(SynthTheme::monoFont(kMasterBarButtonFontSize));
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
}

void drawPresetSaveButton(juce::Graphics& g, juce::Rectangle<float> bounds, bool enabled,
                          bool highlighted, bool down)
{
    if (enabled)
    {
        g.setColour(SynthTheme::presetSaveGlow);
        g.fillRoundedRectangle(bounds.expanded(1.5f), 4.0f);
        g.setColour(highlighted ? SynthTheme::presetSaveFillHi : SynthTheme::presetSaveFill);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour((highlighted || down) ? SynthTheme::presetSaveAccent : SynthTheme::presetSaveDim);
        g.drawRoundedRectangle(bounds, 4.0f, 1.5f);
        return;
    }

    g.setColour(SynthTheme::panelFill);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(SynthTheme::presetSaveDim.withAlpha(0.22f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void drawPresetSaveButtonText(juce::Graphics& g, juce::TextButton& button, bool active)
{
    const auto colour = active ? SynthTheme::presetSaveBright
                               : SynthTheme::textDim.withAlpha(0.35f);
    drawMasterBarButtonText(g, button, colour);
}

void drawDiffToggleButton(juce::Graphics& g, juce::Rectangle<float> bounds, bool isOn,
                          bool highlighted, bool down)
{
    if (isOn)
    {
        g.setColour(highlighted || down ? SynthTheme::presetSaveGlow
                                        : SynthTheme::presetSaveGlow.withAlpha(0.55f));
        g.fillRoundedRectangle(bounds.expanded(highlighted || down ? 2.0f : 1.5f), 4.0f);
        g.setColour(highlighted || down ? SynthTheme::presetSaveFillHi : SynthTheme::presetSaveFill);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(highlighted || down ? SynthTheme::presetSaveAccent : SynthTheme::presetSaveDim);
        g.drawRoundedRectangle(bounds, 4.0f, 1.5f);
        return;
    }

    if (highlighted || down)
    {
        g.setColour(SynthTheme::presetSaveGlow.withAlpha(0.35f));
        g.fillRoundedRectangle(bounds.expanded(1.5f), 4.0f);
        g.setColour(SynthTheme::panelFillHi);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(SynthTheme::presetSaveDim.withAlpha(down ? 0.9f : 0.6f));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        return;
    }

    g.setColour(SynthTheme::panelFill);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(SynthTheme::presetSaveDim.withAlpha(0.22f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void drawDiffToggleButtonText(juce::Graphics& g, juce::TextButton& button, bool isOn,
                              bool highlighted, bool down)
{
    juce::Colour colour;
    if (isOn)
        colour = highlighted || down ? SynthTheme::presetSaveBright
                                     : SynthTheme::presetSaveBright.withAlpha(0.88f);
    else
        colour = highlighted || down ? SynthTheme::presetSaveDim : SynthTheme::textDim.withAlpha(0.35f);

    drawMasterBarButtonText(g, button, colour);
}
} // namespace

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

    if (button.getComponentID() == "presetSave")
    {
        drawPresetSaveButton(g, bounds, button.isEnabled(), highlighted, down);
        return;
    }

    if (button.getComponentID() == "diffToggle")
    {
        drawDiffToggleButton(g, bounds, button.getToggleState(), highlighted, down);
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
    if (button.getComponentID() == "panic")
    {
        drawMasterBarButtonText(g, button, SynthTheme::panicText);
        return;
    }

    if (button.getComponentID() == "presetSave")
    {
        drawPresetSaveButtonText(g, button, button.isEnabled());
        return;
    }

    if (button.getComponentID() == "diffToggle")
    {
        drawDiffToggleButtonText(g, button, button.getToggleState(), highlighted, down);
        return;
    }

    if (button.getComponentID() == "masterBarText")
    {
        if (! button.isEnabled())
        {
            drawMasterBarButtonText(g, button, SynthTheme::textDim.withAlpha(0.35f));
            return;
        }

        const auto colour = button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
                                                                      : juce::TextButton::textColourOffId);
        drawMasterBarButtonText(g, button, colour);
        return;
    }

    LookAndFeel_V4::drawButtonText(g, button, highlighted, down);
}

void FuturisticLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                             bool highlighted, bool down)
{
    drawButtonBackground(g, button, {}, highlighted, down);

    g.setColour(button.getToggleState() ? SynthTheme::accentBright : SynthTheme::textDim);

    if (button.getComponentID() == "lfoRoute")
    {
        g.setFont(SynthTheme::monoFont(kMasterBarButtonFontSize));
        g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
        return;
    }

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

    const auto isBipolar = slider.getComponentID() == "bipolar";

    if (isBipolar && isVertical)
    {
        const auto minValue = static_cast<float>(slider.getMinimum());
        const auto maxValue = static_cast<float>(slider.getMaximum());
        const auto zeroProp = (0.0f - minValue) / (maxValue - minValue);
        const auto zeroY = track.getBottom() - track.getHeight() * zeroProp;
        const auto value = static_cast<float>(slider.getValue());
        const auto step = static_cast<float>(slider.getInterval());
        const auto isZero = SynthTheme::isBipolarNearZero(value, step);

        g.setColour(SynthTheme::panelFillHi);
        g.fillRoundedRectangle(track, 3.0f);

        {
            const juce::Rectangle<float> posZone(track.getX(), track.getY(), track.getWidth(),
                                                 zeroY - track.getY());
            g.setColour(SynthTheme::bipolarPosGlow.withAlpha(0.18f));
            g.fillRoundedRectangle(posZone.reduced(1.0f), 2.0f);
        }
        {
            const juce::Rectangle<float> negZone(track.getX(), zeroY, track.getWidth(),
                                                 track.getBottom() - zeroY);
            g.setColour(SynthTheme::bipolarNegGlow.withAlpha(0.18f));
            g.fillRoundedRectangle(negZone.reduced(1.0f), 2.0f);
        }

        g.setColour(SynthTheme::accentDim.withAlpha(0.45f));
        g.drawRoundedRectangle(track, 3.0f, 1.0f);
        g.setColour(SynthTheme::textDim.withAlpha(0.75f));
        g.drawHorizontalLine(juce::roundToInt(zeroY), track.getX() + 1.0f, track.getRight() - 1.0f);

        juce::Rectangle<float> fillTrack;
        juce::Colour fillStart;
        juce::Colour fillEnd;
        juce::Colour glowColour;
        juce::Colour thumbColour;

        if (! isZero && value > 0.0f)
        {
            fillTrack = { track.getX(), sliderPos, track.getWidth(), zeroY - sliderPos };
            fillStart = SynthTheme::accentDim;
            fillEnd = SynthTheme::bipolarPositive;
            glowColour = SynthTheme::bipolarPosGlow;
            thumbColour = SynthTheme::bipolarPositive;
        }
        else if (! isZero && value < 0.0f)
        {
            fillTrack = { track.getX(), zeroY, track.getWidth(), sliderPos - zeroY };
            fillStart = SynthTheme::bipolarNegative.withAlpha(0.55f);
            fillEnd = SynthTheme::bipolarNegative;
            glowColour = SynthTheme::bipolarNegGlow;
            thumbColour = SynthTheme::bipolarNegative;
        }
        else
        {
            fillTrack = {};
            thumbColour = SynthTheme::bipolarNeutral;
            glowColour = juce::Colours::transparentBlack;
        }

        if (! fillTrack.isEmpty())
        {
            juce::ColourGradient gradient(fillStart, fillTrack.getTopLeft(), fillEnd,
                                          fillTrack.getBottomRight(), false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(fillTrack.reduced(1.0f), 2.0f);
        }

        const auto thumbSize = track.getWidth() + 6.0f;
        const juce::Rectangle<float> thumb(track.getCentreX() - thumbSize * 0.5f, sliderPos - 4.0f,
                                         thumbSize, 8.0f);

        if (! glowColour.isTransparent())
        {
            g.setColour(glowColour);
            g.fillRoundedRectangle(thumb.expanded(2.0f), 2.0f);
        }
        g.setColour(slider.isEnabled() ? thumbColour : SynthTheme::textDim);
        g.fillRoundedRectangle(thumb, 2.0f);
        return;
    }

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
    g.setFont(label.getFont());

    const auto text = label.getText();
    const auto bounds = label.getLocalBounds();

    if (label.getComponentID() == "helpMessage" && text.containsChar('\n'))
    {
        const auto lines = juce::StringArray::fromLines(text);
        const auto numLines = juce::jmax(1, lines.size());
        const auto lineHeight = bounds.getHeight() / numLines;

        for (int i = 0; i < lines.size(); ++i)
        {
            g.drawText(lines[i].trim(), bounds.getX(), bounds.getY() + i * lineHeight,
                       bounds.getWidth(), lineHeight, juce::Justification::centredLeft);
        }

        return;
    }

    g.drawText(text, bounds, label.getJustificationType());
}

juce::Font FuturisticLookAndFeel::getTextButtonFont(juce::TextButton& button, int buttonHeight)
{
    if (isMasterBarButton(button))
        return SynthTheme::monoFont(kMasterBarButtonFontSize);

    return SynthTheme::labelFont(static_cast<float>(buttonHeight) * kStandardTextButtonFontScale);
}

juce::Font FuturisticLookAndFeel::getLabelFont(juce::Label&)
{
    return SynthTheme::labelFont();
}

void FuturisticLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    const int left = box.getComponentID() == "systemCombo" ? kSystemComboLeftPadding : 1;
    label.setBounds(left, 1, box.getWidth() - 29 - left, box.getHeight() - 2);
    label.setFont(getComboBoxFont(box));
}
