#include "WaveformButton.h"

#include <cmath>

#include "SynthTheme.h"

WaveformButton::WaveformButton(Waveform type)
    : waveform(type)
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void WaveformButton::setSelected(bool shouldBeSelected)
{
    if (selected == shouldBeSelected)
        return;

    selected = shouldBeSelected;
    repaint();
}

void WaveformButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    if (selected)
    {
        g.setColour(SynthTheme::glow);
        g.fillRoundedRectangle(bounds.expanded(2.0f), 5.0f);
    }

    g.setColour(selected ? SynthTheme::accentMuted.withAlpha(0.9f) : SynthTheme::panelFillHi);
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(selected ? SynthTheme::accent : SynthTheme::accentDim);
    g.drawRoundedRectangle(bounds, 4.0f, selected ? 2.0f : 1.0f);

    if (selected)
        SynthTheme::drawCornerBrackets(g, bounds.reduced(3.0f), SynthTheme::accentBright, 8.0f, 1.0f);

    const auto colour = selected ? SynthTheme::accentBright : SynthTheme::textDim;
    const auto shapeBounds = bounds.reduced(10.0f, 8.0f);
    drawWaveformShape(g, shapeBounds, waveform, colour, selected ? 2.0f : 1.5f);
}

void WaveformButton::mouseUp(const juce::MouseEvent& event)
{
    if (!isEnabled() || !event.mouseWasClicked() || !onSelect)
        return;

    onSelect();
}

bool WaveformButton::hitTest(int x, int y)
{
    return isVisible() && getLocalBounds().contains(x, y);
}

void WaveformButton::drawWaveformShape(juce::Graphics& g, juce::Rectangle<float> bounds,
                                       Waveform type, juce::Colour colour, float thickness)
{
    g.setColour(colour);

    const auto x0 = bounds.getX();
    const auto yMid = bounds.getCentreY();
    const auto width = bounds.getWidth();
    const auto height = bounds.getHeight() * 0.42f;

    juce::Path path;

    switch (type)
    {
        case Waveform::Sine:
        {
            constexpr int points = 48;
            for (int i = 0; i <= points; ++i)
            {
                const auto t = static_cast<float>(i) / static_cast<float>(points);
                const auto x = x0 + width * t;
                const auto y = yMid - height * std::sin(t * juce::MathConstants<float>::twoPi);
                if (i == 0)
                    path.startNewSubPath(x, y);
                else
                    path.lineTo(x, y);
            }
            break;
        }

        case Waveform::Saw:
            path.startNewSubPath(x0, yMid + height);
            path.lineTo(x0 + width, yMid - height);
            break;

        case Waveform::Square:
        {
            const auto xMid = x0 + width * 0.5f;
            path.startNewSubPath(x0, yMid - height);
            path.lineTo(xMid, yMid - height);
            path.lineTo(xMid, yMid + height);
            path.lineTo(x0 + width, yMid + height);
            break;
        }

        case Waveform::Triangle:
        {
            const auto xMid = x0 + width * 0.5f;
            path.startNewSubPath(x0, yMid + height);
            path.lineTo(xMid, yMid - height);
            path.lineTo(x0 + width, yMid + height);
            break;
        }

        default:
            break;
    }

    g.strokePath(path, juce::PathStrokeType(thickness, juce::PathStrokeType::curved,
                                            juce::PathStrokeType::rounded));
}
