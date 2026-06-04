#pragma once

#include <algorithm>

#include <juce_core/juce_core.h>

class AdsrEnvelope
{
public:
    void setParameters(float attackSec, float decaySec, float sustain, float releaseSec)
    {
        attack = attackSec;
        decay = decaySec;
        sustainLevel = juce::jlimit(0.0f, 1.0f, sustain);
        release = releaseSec;
        updateDeltas();
    }

    void noteOn()
    {
        sustainElapsedSec = 0.0f;
        stage = Stage::attack;
        if (attack <= 0.0f)
        {
            currentLevel = 1.0f;
            stage = Stage::decay;
        }
        else
        {
            currentLevel = 0.0f;
        }
    }

    void noteOff()
    {
        if (stage == Stage::idle)
            return;

        stage = Stage::release;
        updateDeltas();
    }

    void reset()
    {
        stage = Stage::idle;
        currentLevel = 0.0f;
        sustainElapsedSec = 0.0f;
    }

    bool isActive() const { return stage != Stage::idle; }

    float getLevel() const { return currentLevel; }

    float getTimelinePosition(float sustainVisualWeight) const
    {
        const auto total = attack + decay + sustainVisualWeight + release;
        if (stage == Stage::idle || total <= 0.0f)
            return -1.0f;

        switch (stage)
        {
            case Stage::attack:
                if (attack <= 0.0f)
                    return 0.0f;
                return juce::jlimit(0.0f, 1.0f, (currentLevel * attack) / total);

            case Stage::decay:
            {
                const auto range = juce::jmax(0.0001f, 1.0f - sustainLevel);
                const auto progress = juce::jlimit(0.0f, 1.0f, (1.0f - currentLevel) / range);
                return (attack + progress * decay) / total;
            }

            case Stage::sustain:
            {
                const auto progress =
                    juce::jlimit(0.0f, 1.0f, sustainElapsedSec / juce::jmax(0.001f, sustainVisualWeight));
                return (attack + decay + progress * sustainVisualWeight) / total;
            }

            case Stage::release:
            {
                const auto range = juce::jmax(0.0001f, sustainLevel);
                const auto progress = juce::jlimit(0.0f, 1.0f, 1.0f - currentLevel / range);
                return (attack + decay + sustainVisualWeight + progress * release) / total;
            }

            default:
                return -1.0f;
        }
    }

    float advance()
    {
        switch (stage)
        {
            case Stage::idle:
                return 0.0f;

            case Stage::attack:
                currentLevel += attackDelta;
                if (currentLevel >= 1.0f)
                {
                    currentLevel = 1.0f;
                    stage = Stage::decay;
                }
                break;

            case Stage::decay:
                currentLevel -= decayDelta;
                if (currentLevel <= sustainLevel)
                {
                    currentLevel = sustainLevel;
                    stage = Stage::sustain;
                }
                break;

            case Stage::sustain:
                currentLevel = sustainLevel;
                sustainElapsedSec +=
                    static_cast<float>(1.0 / (sampleRate > 0.0 ? sampleRate : 44100.0));
                break;

            case Stage::release:
                currentLevel -= releaseDelta;
                if (currentLevel <= 0.0f)
                {
                    currentLevel = 0.0f;
                    stage = Stage::idle;
                }
                break;
        }

        return currentLevel;
    }

    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
        updateDeltas();
    }

private:
    enum class Stage
    {
        idle,
        attack,
        decay,
        sustain,
        release
    };

    void updateDeltas()
    {
        const auto sr = static_cast<float>(sampleRate);
        attackDelta = attack > 0.0f ? 1.0f / (attack * sr) : 1.0f;
        decayDelta = decay > 0.0f ? (1.0f - sustainLevel) / (decay * sr)
                                  : 1.0f - sustainLevel;
        releaseDelta = release > 0.0f ? sustainLevel / (release * sr)
                                      : sustainLevel;

        if (stage == Stage::release)
            releaseDelta = release > 0.0f ? currentLevel / (release * sr) : currentLevel;
    }

    Stage stage = Stage::idle;
    float currentLevel = 0.0f;
    float attack = 0.01f;
    float decay = 0.15f;
    float sustainLevel = 0.7f;
    float release = 0.25f;
    float attackDelta = 0.0f;
    float decayDelta = 0.0f;
    float releaseDelta = 0.0f;
    float sustainElapsedSec = 0.0f;
    double sampleRate = 44100.0;
};
