#pragma once

#include <cmath>

#include <juce_gui_basics/juce_gui_basics.h>

namespace SynthTheme
{
inline const juce::Colour background      { 0xff050508 };
inline const juce::Colour panelFill       { 0xff0c0c10 };
inline const juce::Colour panelFillHi    { 0xff14141a };
inline const juce::Colour accent         { 0xffadff2f };
inline const juce::Colour accentBright   { 0xffd8ff66 };
inline const juce::Colour accentDim      { 0xff4a6620 };
inline const juce::Colour accentMuted    { 0xff2a3318 };
inline const juce::Colour textPrimary    { 0xffc8f070 };
inline const juce::Colour textDim        { 0xff6a8050 };
inline const juce::Colour gridLine       { 0xff1a2210 };
inline const juce::Colour glow           { 0x55adff2f };
inline const juce::Colour panicFill      { 0xff4a1218 };
inline const juce::Colour panicFillHi    { 0xff6a1a22 };
inline const juce::Colour panicBorder    { 0xffe04050 };
inline const juce::Colour panicText       { 0xffffa8b0 };
inline const juce::Colour panicGlow       { 0x88ff3040 };
inline const juce::Colour bipolarPositive  { 0xffd8ff66 };
inline const juce::Colour bipolarNegative  { 0xff50c8ff };
inline const juce::Colour bipolarNeutral   { 0xff6a8050 };
inline const juce::Colour bipolarPosGlow   { 0x55adff2f };
inline const juce::Colour bipolarNegGlow   { 0x5550c8ff };
inline const juce::Colour presetSaveFill      { 0xff9a3a08 };
inline const juce::Colour presetSaveFillHi    { 0xffb84810 };
inline const juce::Colour presetSaveAccent    { 0xffff8020 };
inline const juce::Colour presetSaveBright    { 0xffffaa44 };
inline const juce::Colour presetSaveDim       { 0xffd05818 };
inline const juce::Colour presetSaveGlow      { 0x99ff8020 };

inline bool isBipolarNearZero(float value, float step)
{
    if (step > 0.0f)
        return std::abs(value) <= step * 0.5f;

    return juce::approximatelyEqual(value, 0.0f);
}

inline float bipolarStepForDecimals(int decimalPlaces)
{
    return std::pow(10.0f, -static_cast<float>(juce::jmax(0, decimalPlaces)));
}

inline juce::Font titleFont(float size = 15.0f)
{
    return juce::Font(juce::FontOptions(size).withStyle("Bold"));
}

inline juce::Font labelFont(float size = 12.0f)
{
    return juce::Font(juce::FontOptions(size));
}

inline juce::Font monoFont(float size = 11.0f)
{
    return juce::Font(juce::FontOptions(size).withStyle("Bold"));
}

inline juce::Font systemMessageFont(float size = 10.0f)
{
#if JUCE_WINDOWS
    return juce::Font(juce::FontOptions(size).withName("Yu Gothic UI"));
#else
    return labelFont(size);
#endif
}

inline void drawScanlines(juce::Graphics& g, juce::Rectangle<int> area, float alpha = 0.04f)
{
    g.setColour(juce::Colours::white.withAlpha(alpha));
    for (int y = area.getY(); y < area.getBottom(); y += 3)
        g.drawHorizontalLine(y, static_cast<float>(area.getX()), static_cast<float>(area.getRight()));
}

inline void drawCornerBrackets(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour colour,
                               float length = 14.0f, float thickness = 1.5f)
{
    g.setColour(colour);
    const auto x = bounds.getX();
    const auto y = bounds.getY();
    const auto r = bounds.getRight();
    const auto b = bounds.getBottom();

    g.drawLine(x, y, x + length, y, thickness);
    g.drawLine(x, y, x, y + length, thickness);
    g.drawLine(r - length, y, r, y, thickness);
    g.drawLine(r, y, r, y + length, thickness);
    g.drawLine(x, b - length, x, b, thickness);
    g.drawLine(x, b, x + length, b, thickness);
    g.drawLine(r - length, b, r, b, thickness);
    g.drawLine(r, b - length, r, b, thickness);
}
} // namespace SynthTheme
