#pragma once

#include <functional>

#include <juce_core/juce_core.h>

class PresetManager
{
public:
    using ParametersChangedCallback = std::function<void()>;

    explicit PresetManager(ParametersChangedCallback onChanged);

    void loadUserPresetsFromDisk();

    juce::StringArray getPresetNames() const;
    int getCurrentIndex() const { return currentIndex; }

    void selectPreset(int index);
    bool saveCurrentAsUserPreset(const juce::String& name);
    bool loadUserPreset(const juce::String& name);
    juce::File getUserPresetsDirectory() const;

private:
    struct PresetEntry
    {
        juce::String name;
        bool isFactory = true;
        juce::var data;
    };

    void buildFactoryPresets();
    void applyPresetData(const juce::var& data);
    static juce::var captureCurrentParameters();
    static void applyParametersFromVar(const juce::var& root);

    ParametersChangedCallback onParametersChanged;
    juce::Array<PresetEntry> presets;
    int currentIndex = 0;
};
