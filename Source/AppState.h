#pragma once

#include <juce_core/juce_core.h>

namespace AppState
{
struct Session
{
    bool valid = false;
    juce::var parameters;
    int presetIndex = 0;
    bool midiAllInputs = true;
    juce::String midiDeviceId;
    int windowX = 0;
    int windowY = 0;
    int windowW = 1080;
    int windowH = 680;
};

juce::File getSessionFile();
Session load();
void save(const Session& session);
} // namespace AppState
