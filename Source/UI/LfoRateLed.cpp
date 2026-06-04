#include "LfoRateLed.h"

#include "../SynthParameters.h"
#include "SynthTheme.h"

LfoRateLed::LfoRateLed(GlobalLfo::Index index)
    : lfoIndex(index)
{
    startTimerHz(60);
}

void LfoRateLed::timerCallback()
{
    repaint();
}

void LfoRateLed::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const auto centre = bounds.getCentre();
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.38f;

    const auto rate = lfoIndex == GlobalLfo::Index::Lfo1 ? SynthParameters::getLfoRateHz()
                                                         : SynthParameters::getLfo2RateHz();
    const auto level = GlobalLfo::getLedLevel(lfoIndex);
    const auto activity = rate <= 0.06f ? 0.15f : 1.0f;
    const auto brightness = juce::jlimit(0.0f, 1.0f, level * activity);

    g.setColour(juce::Colour(0xff1a0808));
    g.fillEllipse(centre.x - radius - 1.0f, centre.y - radius - 1.0f,
                  (radius + 1.0f) * 2.0f, (radius + 1.0f) * 2.0f);

    if (brightness > 0.02f)
    {
        const auto glowAlpha = brightness * 0.55f;
        g.setColour(juce::Colour(0xffff2030).withAlpha(glowAlpha));
        g.fillEllipse(centre.x - radius - 2.0f, centre.y - radius - 2.0f,
                      (radius + 2.0f) * 2.0f, (radius + 2.0f) * 2.0f);
    }

    const auto coreColour = juce::Colour(0xffff3040)
                                .withAlpha(0.25f + brightness * 0.75f);
    g.setColour(coreColour);
    g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    g.setColour(juce::Colour(0xffffa0a8).withAlpha(0.35f + brightness * 0.5f));
    g.fillEllipse(centre.x - radius * 0.35f, centre.y - radius * 0.35f, radius * 0.7f, radius * 0.7f);
}
