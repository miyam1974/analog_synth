#include "PcKeyboardDisplay.h"

#include "SynthTheme.h"

namespace
{
constexpr int kWhiteKeyCount = 10;
constexpr float kBottomRowHeightRatio = 0.46f;
constexpr float kTopRowHeightRatio = 0.34f;
constexpr float kTopRowYOffsetRatio = 0.04f;
constexpr float kBottomKeyWidthScale = 1.5f;
constexpr float kUpperRowShiftColumns = -0.5f;
constexpr float kKeyCapInset = 2.0f;
} // namespace

PcKeyboardDisplay::PcKeyboardDisplay()
{
    setInterceptsMouseClicks(true, false);
    startTimerHz(30);
}

void PcKeyboardDisplay::mouseDown(const juce::MouseEvent& event)
{
    juce::Component::mouseDown(event);

    if (onClicked)
        onClicked();
}

void PcKeyboardDisplay::setMappingEnabled(bool enabled)
{
    if (mappingEnabled == enabled)
        return;

    mappingEnabled = enabled;
    repaint();
}

const std::array<PcKeyboardDisplay::KeyVisual, 17>& PcKeyboardDisplay::getKeys()
{
    static constexpr std::array<KeyVisual, 17> keys{{
        {"S", 'w', 1.0f, true},
        {"D", 'e', 2.0f, true},
        {"T", 't', 4.0f, true},
        {"Y", 'y', 5.0f, true},
        {"U", 'u', 6.0f, true},
        {"O", 'o', 8.0f, true},
        {"P", 'p', 9.0f, true},
        {"A", 'a', 0.0f, false},
        {"S", 's', 1.0f, false},
        {"D", 'd', 2.0f, false},
        {"F", 'f', 3.0f, false},
        {"G", 'g', 4.0f, false},
        {"H", 'h', 5.0f, false},
        {"J", 'j', 6.0f, false},
        {"K", 'k', 7.0f, false},
        {"L", 'l', 8.0f, false},
        {"+", ';', 9.0f, false},
    }};

    return keys;
}

bool PcKeyboardDisplay::isKeyDown(int keyCode)
{
    return juce::KeyPress::isKeyCurrentlyDown(keyCode);
}

void PcKeyboardDisplay::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    if (bounds.isEmpty())
        return;

    g.setColour(SynthTheme::panelFill);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(SynthTheme::accentDim.withAlpha(0.45f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    const auto logicalWidth = bounds.getWidth() / kBottomKeyWidthScale;
    const auto referenceUnit = logicalWidth / static_cast<float>(kWhiteKeyCount);
    const auto keyPitch = referenceUnit * kBottomKeyWidthScale;
    const auto keyCapWidth = (referenceUnit - kKeyCapInset) * kBottomKeyWidthScale;
    const auto bottomKeyHeight = bounds.getHeight() * kBottomRowHeightRatio;
    const auto topKeyHeight = bounds.getHeight() * kTopRowHeightRatio;
    const auto bottomRowY = bounds.getBottom() - bottomKeyHeight - 2.0f;
    const auto topRowY = bounds.getY() + bounds.getHeight() * kTopRowYOffsetRatio;

    const auto inactiveAlpha = mappingEnabled ? 1.0f : 0.35f;

    for (const auto& key : getKeys())
    {
        const auto isDown = mappingEnabled && isKeyDown(key.keyCode);
        const auto keyHeight = key.upperRow ? topKeyHeight : bottomKeyHeight;
        const auto column = key.column + (key.upperRow ? kUpperRowShiftColumns : 0.0f);
        const auto x = bounds.getX() + column * keyPitch;
        const auto y = key.upperRow ? topRowY : bottomRowY;
        auto cap = juce::Rectangle<float>(x, y, keyCapWidth, keyHeight);

        if (isDown)
        {
            g.setColour(SynthTheme::glow.withAlpha(0.85f));
            g.fillRoundedRectangle(cap.expanded(1.5f), 3.0f);
        }

        const auto fill = key.upperRow
                              ? (isDown ? SynthTheme::background.darker(0.05f)
                                        : SynthTheme::background.darker(0.25f))
                              : (isDown ? SynthTheme::accentMuted.withAlpha(0.95f)
                                        : SynthTheme::background.brighter(0.1f));

        g.setColour(fill.withMultipliedAlpha(inactiveAlpha));
        g.fillRoundedRectangle(cap, 3.0f);

        const auto border = isDown ? SynthTheme::accentBright : SynthTheme::accentDim;
        g.setColour(border.withAlpha((isDown ? 1.0f : 0.55f) * inactiveAlpha));
        g.drawRoundedRectangle(cap, 3.0f, isDown ? 1.8f : 1.0f);

        const auto textColour = isDown ? SynthTheme::accentBright : SynthTheme::textDim;
        g.setColour(textColour.withMultipliedAlpha(inactiveAlpha));
        g.setFont(SynthTheme::monoFont(key.upperRow ? 9.0f : 10.0f));
        g.drawText(key.label, cap, juce::Justification::centred);
    }
}

void PcKeyboardDisplay::timerCallback()
{
    if (mappingEnabled)
        repaint();
}
