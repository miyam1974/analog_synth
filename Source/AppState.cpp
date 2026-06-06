#include "AppState.h"

namespace AppState
{
juce::File getSessionFile()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("NEXUS OSC")
        .getChildFile("session.json");
}

Session load()
{
    Session session;
    const auto file = getSessionFile();
    if (!file.existsAsFile())
        return session;

    const auto root = juce::JSON::parse(file);
    if (!root.isObject())
        return session;

    session.valid = true;
    session.parameters = root.getProperty("parameters", {});
    session.presetIndex = static_cast<int>(root.getProperty("presetIndex", 0));

    if (root.getProperty("midiAllInputs", true))
    {
        session.midiAllInputs = true;
    }
    else
    {
        session.midiAllInputs = false;
        session.midiDeviceId = root.getProperty("midiDeviceId", juce::String()).toString();
    }

    const auto window = root.getProperty("window", {});
    if (window.isObject())
    {
        session.windowX = static_cast<int>(window.getProperty("x", 0));
        session.windowY = static_cast<int>(window.getProperty("y", 0));
        session.windowW = static_cast<int>(window.getProperty("w", 1080));
        session.windowH = static_cast<int>(window.getProperty("h", 680));
    }

    return session;
}

void save(const Session& session)
{
    auto* root = new juce::DynamicObject();
    root->setProperty("version", 1);
    root->setProperty("parameters", session.parameters);
    root->setProperty("presetIndex", session.presetIndex);
    root->setProperty("midiAllInputs", session.midiAllInputs);
    if (!session.midiAllInputs)
        root->setProperty("midiDeviceId", session.midiDeviceId);

    auto* window = new juce::DynamicObject();
    window->setProperty("x", session.windowX);
    window->setProperty("y", session.windowY);
    window->setProperty("w", session.windowW);
    window->setProperty("h", session.windowH);
    root->setProperty("window", window);

    const auto file = getSessionFile();
    file.getParentDirectory().createDirectory();
    file.replaceWithText(juce::JSON::toString(juce::var(root), true));
}
} // namespace AppState
