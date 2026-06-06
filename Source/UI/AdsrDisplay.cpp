#include "AdsrDisplay.h"

#include <array>

#include "../EnvelopePlayhead.h"
#include "SynthTheme.h"

AdsrDisplay::AdsrDisplay(PlayheadSource source)
    : playheadSource(source)
{
    startTimerHz(60);
}

void AdsrDisplay::timerCallback()
{
    const auto isAmp = playheadSource == PlayheadSource::amplifier;
    const auto visible = EnvelopePlayheadHub::anyVisible(isAmp);

    if (visible || playheadWasVisible)
    {
        playheadWasVisible = visible;
        repaint();
    }
}

void AdsrDisplay::setParameters(float attackSec, float decaySec, float sustainLevel, float releaseSec)
{
    attack = juce::jmax(kMinTimeSec, attackSec);
    decay = juce::jmax(kMinTimeSec, decaySec);
    sustain = juce::jlimit(0.0f, 1.0f, sustainLevel);
    release = juce::jmax(kMinTimeSec, releaseSec);
    repaint();
}

void AdsrDisplay::buildEnvelopePath(juce::Rectangle<float> bounds, juce::Path& strokePath,
                                    juce::Path& fillPath) const
{
    const auto totalTime = attack + decay + kSustainVisualWeight + release;
    const auto width = bounds.getWidth();
    const auto x0 = bounds.getX();
    const auto bottom = bounds.getBottom();
    const auto height = bounds.getHeight();

    const auto xAt = [&](float time) { return x0 + width * (time / totalTime); };
    const auto yAt = [&](float level) { return bottom - height * level; };

    const auto xAttackEnd = xAt(attack);
    const auto xDecayEnd = xAt(attack + decay);
    const auto xSustainEnd = xAt(attack + decay + kSustainVisualWeight);
    const auto xReleaseEnd = xAt(totalTime);
    const auto ySustain = yAt(sustain);

    strokePath.startNewSubPath(x0, bottom);
    strokePath.lineTo(xAttackEnd, yAt(1.0f));
    strokePath.lineTo(xDecayEnd, ySustain);
    strokePath.lineTo(xSustainEnd, ySustain);
    strokePath.lineTo(xReleaseEnd, bottom);

    fillPath = strokePath;
    fillPath.lineTo(x0, bottom);
    fillPath.closeSubPath();
}

void AdsrDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    g.setColour(SynthTheme::panelFillHi);
    g.fillRoundedRectangle(bounds, 3.0f);

    g.setColour(SynthTheme::gridLine);
    const int gridLines = 4;
    for (int i = 1; i < gridLines; ++i)
    {
        const auto y = bounds.getY() + bounds.getHeight() * static_cast<float>(i) / gridLines;
        g.drawHorizontalLine(juce::roundToInt(y), bounds.getX(), bounds.getRight());
    }

    juce::Path strokePath;
    juce::Path fillPath;
    buildEnvelopePath(bounds.reduced(6.0f, 10.0f), strokePath, fillPath);

    juce::ColourGradient fillGradient(SynthTheme::accent.withAlpha(0.35f), bounds.getCentreX(),
                                      bounds.getY(),
                                      SynthTheme::accent.withAlpha(0.04f), bounds.getCentreX(),
                                      bounds.getBottom(), false);
    g.setGradientFill(fillGradient);
    g.fillPath(fillPath);

    g.setColour(SynthTheme::accent.withAlpha(0.9f));
    g.strokePath(strokePath, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

    const auto totalTime = attack + decay + kSustainVisualWeight + release;
    const auto inner = bounds.reduced(6.0f, 10.0f);
    const auto xAt = [&](float time)
    {
        return inner.getX() + inner.getWidth() * (time / totalTime);
    };
    const auto yAt = [&](float level) { return inner.getBottom() - inner.getHeight() * level; };

    g.setColour(SynthTheme::accentBright.withAlpha(0.55f));
    const std::array<juce::Point<float>, 5> nodes {
        juce::Point<float>(inner.getX(), inner.getBottom()),
        juce::Point<float>(xAt(attack), yAt(1.0f)),
        juce::Point<float>(xAt(attack + decay), yAt(sustain)),
        juce::Point<float>(xAt(attack + decay + kSustainVisualWeight), yAt(sustain)),
        juce::Point<float>(xAt(totalTime), inner.getBottom()),
    };
    for (const auto& point : nodes)
        g.fillEllipse(point.x - kNodeRadius, point.y - kNodeRadius, kNodeRadius * 2.0f,
                      kNodeRadius * 2.0f);

    g.setColour(SynthTheme::accentDim.withAlpha(0.65f));
    g.drawVerticalLine(juce::roundToInt(xAt(attack)), inner.getY(), inner.getBottom());
    g.drawVerticalLine(juce::roundToInt(xAt(attack + decay)), inner.getY(), inner.getBottom());
    g.drawVerticalLine(juce::roundToInt(xAt(attack + decay + kSustainVisualWeight)), inner.getY(),
                       inner.getBottom());

    g.setFont(SynthTheme::monoFont(9.0f));
    g.setColour(SynthTheme::textDim);

    const auto labelY = bounds.getBottom() - 10.0f;
    const auto drawPhaseLabel = [&](float start, float end, const char* text)
    {
        const auto cx = inner.getX() + inner.getWidth() * ((start + end) * 0.5f / totalTime);
        g.drawText(text, juce::Rectangle<float>(cx - 14.0f, labelY, 28.0f, 11.0f),
                   juce::Justification::centred, false);
    };

    drawPhaseLabel(0.0f, attack, "A");
    drawPhaseLabel(attack, attack + decay, "D");
    drawPhaseLabel(attack + decay, attack + decay + kSustainVisualWeight, "S");
    drawPhaseLabel(attack + decay + kSustainVisualWeight, totalTime, "R");

    const auto& dots = playheadSource == PlayheadSource::amplifier ? EnvelopePlayheadHub::amp
                                                                   : EnvelopePlayheadHub::filter;
    for (const auto& dot : dots)
    {
        const auto timeline = dot.timeline.load(std::memory_order_relaxed);
        if (timeline < 0.0f)
            continue;

        const auto playheadPoint =
            EnvelopePlayheadHub::pointOnCurve(timeline, attack, decay, sustain, release, inner);
        const auto r = kPlayheadDotRadius;
        g.setColour(SynthTheme::accentBright);
        g.fillEllipse(playheadPoint.x - r, playheadPoint.y - r, r * 2.0f, r * 2.0f);
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.drawEllipse(playheadPoint.x - r, playheadPoint.y - r, r * 2.0f, r * 2.0f, 0.8f);
    }

    SynthTheme::drawCornerBrackets(g, bounds, SynthTheme::accent.withAlpha(0.45f), 10.0f, 1.2f);
}
