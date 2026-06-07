#include <functional>
#include <iterator>
#include <memory>
#include <string_view>
#include <vector>

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "AppState.h"
#include "GlobalLfo.h"
#include "PresetManager.h"
#include "SynthEditor.h"
#include "SynthParameters.h"
#include "SynthSound.h"
#include "SynthVoice.h"
#include "UI/FuturisticLookAndFeel.h"
#include "UI/PcKeyboardDisplay.h"
#include "UI/SynthTheme.h"
#include "UI/TrebleStaffDisplay.h"

namespace
{
constexpr int kNumVoices = 16;
constexpr int kHeaderHeight = 28;
constexpr int kKeyboardHeight = 96;
constexpr int kKeyboardSidePanelWidth = 188;
constexpr int kPcKeyboardToggleColumnWidth = 38;
constexpr int kTrebleStaffWidth = 162;
constexpr int kMargin = 14;
constexpr int kDefaultWindowWidth = 1080;
constexpr int kDefaultWindowHeight = 680;
constexpr int kMaxWindowWidth = 4096;
constexpr int kMaxWindowHeight = 2160;
constexpr int kHeaderTitleRowHeight = 18;
constexpr int kHeaderTitleSubtitleGap = 12;
constexpr int kHeaderVerticalPadding = 5;

juce::Rectangle<int> clampWindowToDisplay(juce::Rectangle<int> bounds)
{
    const auto& displays = juce::Desktop::getInstance().getDisplays();
    if (auto* display = displays.getDisplayForRect(bounds))
    {
        const auto area = display->userArea;
        bounds.setSize(juce::jmin(bounds.getWidth(), area.getWidth()),
                       juce::jmin(bounds.getHeight(), area.getHeight()));
        bounds.setX(juce::jlimit(area.getX(), area.getRight() - bounds.getWidth(), bounds.getX()));
        bounds.setY(juce::jlimit(area.getY(), area.getBottom() - bounds.getHeight(), bounds.getY()));
    }

    return bounds;
}

juce::Rectangle<int> restoredWindowBounds(const AppState::Session& session)
{
    return clampWindowToDisplay({session.windowX,
                                   session.windowY,
                                   juce::jmax(kDefaultWindowWidth, session.windowW),
                                   juce::jmax(kDefaultWindowHeight, session.windowH)});
}

void applyParametersKeepingMaster(const juce::var& parameters)
{
    const auto masterLevel = SynthParameters::getMasterLevel();
    PresetManager::applyParametersFromVar(parameters);
    SynthParameters::setMasterLevel(masterLevel);
}

void applyDefaultPcKeyMappings(juce::MidiKeyboardComponent& keyboard)
{
    keyboard.clearKeyMappings();

    const std::string_view keys{"awsedftgyhujkolp;"};
    for (const char& c : keys)
        keyboard.setKeyPressForNote({c, 0, 0}, static_cast<int>(std::distance(keys.data(), &c)));
}

void setPcKeyboardMappingsEnabled(juce::MidiKeyboardComponent& keyboard, bool enabled)
{
    if (enabled)
        applyDefaultPcKeyMappings(keyboard);
    else
        keyboard.clearKeyMappings();
}

class DiffShortcutKeyListener : public juce::KeyListener
{
public:
    explicit DiffShortcutKeyListener(std::function<void()> onToggle)
        : onToggle(std::move(onToggle))
    {
    }

    bool keyPressed(const juce::KeyPress& key, juce::Component*) override
    {
        if (!isDiffShortcut(key))
            return false;

        onToggle();
        return true;
    }

private:
    static bool isDiffShortcut(const juce::KeyPress& key)
    {
        if (juce::Component::getCurrentlyModalComponent() != nullptr)
            return false;

        if (key.getModifiers().isAnyModifierKeyDown())
            return false;

        return key.isKeyCode(juce::KeyPress::spaceKey) || key.getTextCharacter() == ' ';
    }

    std::function<void()> onToggle;
};
} // namespace

class MainComponent : public juce::AudioAppComponent,
                      private juce::MidiInputCallback
{
public:
    bool handleDiffShortcutKey(const juce::KeyPress& key)
    {
        if (diffShortcutListener == nullptr)
            return false;

        return diffShortcutListener->keyPressed(key, this);
    }

    juce::KeyListener* getDiffShortcutListener() const { return diffShortcutListener.get(); }

    void requestPcKeyboardFocus()
    {
        juce::MessageManager::callAsync([this]
                                        {
                                            juce::MessageManager::callAsync([this]
                                                                           { focusPcKeyboardIfEnabled(); });
                                        });
    }

    explicit MainComponent(const AppState::Session& session)
        : presetManager([this] { onPresetParametersChanged(); })
    {
        for (int i = 0; i < kNumVoices; ++i)
            synth.addVoice(new SynthVoice(i));

        synth.addSound(new SynthSound());
        applyMonophonicMode();
        setAudioChannels(0, 2);

        titleLabel.setText("NEXUS OSC", juce::dontSendNotification);
        titleLabel.setJustificationType(juce::Justification::centredLeft);
        titleLabel.setFont(SynthTheme::titleFont(18.0f));
        titleLabel.setColour(juce::Label::textColourId, SynthTheme::accentBright);
        addAndMakeVisible(titleLabel);

        subtitleLabel.setText("ANALOG SIGNAL ENGINE  //  POLY 16", juce::dontSendNotification);
        subtitleLabel.setJustificationType(juce::Justification::centredLeft);
        subtitleLabel.setFont(SynthTheme::monoFont(11.0f));
        subtitleLabel.setColour(juce::Label::textColourId, SynthTheme::textDim);
        addAndMakeVisible(subtitleLabel);

        keyboardComponent = std::make_unique<juce::MidiKeyboardComponent>(
            keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard);
        keyboardComponent->setAvailableRange(36, 96);
        keyboardComponent->setKeyWidth(18.0f);
        keyboardComponent->setColour(juce::MidiKeyboardComponent::whiteNoteColourId,
                                     SynthTheme::background.brighter(0.08f));
        keyboardComponent->setColour(juce::MidiKeyboardComponent::blackNoteColourId,
                                     SynthTheme::background.darker(0.2f));
        keyboardComponent->setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId,
                                     SynthTheme::accent.withAlpha(0.35f));
        keyboardComponent->setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,
                                     SynthTheme::accent.withAlpha(0.25f));
        keyboardComponent->setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId,
                                     SynthTheme::accent.withAlpha(0.55f));
        addAndMakeVisible(*keyboardComponent);

        pcKeyboardOnButton.setButtonText("ON");
        pcKeyboardOnButton.setComponentID("masterBarText");
        pcKeyboardOnButton.setClickingTogglesState(true);
        pcKeyboardOnButton.setRadioGroupId(9101);
        pcKeyboardOnButton.setToggleState(true, juce::dontSendNotification);
        pcKeyboardOnButton.setLookAndFeel(&keyboardBarLookAndFeel);
        pcKeyboardOnButton.onClick = [this] { setPcKeyboardEnabled(true); };
        addAndMakeVisible(pcKeyboardOnButton);

        pcKeyboardOffButton.setButtonText("OFF");
        pcKeyboardOffButton.setComponentID("masterBarText");
        pcKeyboardOffButton.setClickingTogglesState(true);
        pcKeyboardOffButton.setRadioGroupId(9101);
        pcKeyboardOffButton.setToggleState(false, juce::dontSendNotification);
        pcKeyboardOffButton.setLookAndFeel(&keyboardBarLookAndFeel);
        pcKeyboardOffButton.onClick = [this] { setPcKeyboardEnabled(false); };
        addAndMakeVisible(pcKeyboardOffButton);

        addAndMakeVisible(pcKeyboardDisplay);
        pcKeyboardDisplay.setMappingEnabled(true);
        pcKeyboardDisplay.onClicked = [this] { requestPcKeyboardFocus(); };

        addAndMakeVisible(trebleStaffDisplay);
        trebleStaffDisplay.setBarLookAndFeel(&keyboardBarLookAndFeel);
        trebleStaffDisplay.getActiveNotes = [this] { return collectActiveMidiNotes(); };

        addAndMakeVisible(editor);
        editor.onMidiSelectionChanged = [this](int id) { reconnectMidiInput(id); };
        editor.onPanic = [this] { stopAllSound(); };
        editor.onMonoModeChanged = [this](bool) { applyMonophonicMode(); };
        editor.onPresetSelected = [this](int index) { presetManager.selectPreset(index); };
        editor.onPresetSave = [this] { promptOverwritePreset(); };
        editor.onPresetSaveAs = [this] { promptSavePresetAs(); };
        editor.onPresetLoad = [this] { promptLoadPresetFile(); };
        editor.onResetToDefaults = [this] { resetToInitialSettings(); };
        editor.onDiffToggleRequested = [this](bool active) { handleDiffToggle(active); };
        editor.onParameterEdited = [this] { updatePresetSaveButtonState(); };

        refreshPresetList();
        refreshMidiDeviceList();
        restoreSession(session);
        updatePresetSaveButtonState();
        captureDiffBaseline();

        diffShortcutListener = std::make_unique<DiffShortcutKeyListener>([this]
                                                                         { handleDiffToggle(!diffActive); });
        addKeyListener(diffShortcutListener.get());
        keyboardComponent->addKeyListener(diffShortcutListener.get());
        editor.addKeyListener(diffShortcutListener.get());

        setWantsKeyboardFocus(true);

        setSize(1080, 680);
    }

    ~MainComponent() override
    {
        pcKeyboardOnButton.setLookAndFeel(nullptr);
        pcKeyboardOffButton.setLookAndFeel(nullptr);
        editor.removeKeyListener(diffShortcutListener.get());
        if (keyboardComponent != nullptr)
            keyboardComponent->removeKeyListener(diffShortcutListener.get());
        removeKeyListener(diffShortcutListener.get());
        shutdownAudio();
        closeMidiInputs();
        setLookAndFeel(nullptr);
    }

    void saveSession(const juce::Rectangle<int>& windowBounds)
    {
        AppState::Session session;
        session.parameters = PresetManager::captureCurrentParameters();
        session.presetIndex = presetManager.getCurrentIndex();

        const auto midiComboId = editor.getSelectedMidiIndex();
        if (midiComboId <= 1)
        {
            session.midiAllInputs = true;
        }
        else
        {
            const auto index = midiComboId - 2;
            if (juce::isPositiveAndBelow(index, midiDeviceIdentifiers.size()))
            {
                session.midiAllInputs = false;
                session.midiDeviceId = midiDeviceIdentifiers[index];
            }
        }

        session.hasWindowBounds = true;
        session.windowX = windowBounds.getX();
        session.windowY = windowBounds.getY();
        session.windowW = windowBounds.getWidth();
        session.windowH = windowBounds.getHeight();

        AppState::save(session);
    }

    void prepareToPlay(int, double sampleRate) override
    {
        currentSampleRate = sampleRate;
        synth.setCurrentPlaybackSampleRate(sampleRate);
        midiCollector.reset(sampleRate);
        keyboardState.reset();
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        GlobalLfo::prepareForBlock(bufferToFill.numSamples, currentSampleRate);

        juce::MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages(incomingMidi, bufferToFill.numSamples);
        keyboardState.processNextMidiBuffer(incomingMidi, 0, bufferToFill.numSamples, true);
        applyMonoModeMidi(incomingMidi);
        synth.renderNextBlock(*bufferToFill.buffer, incomingMidi, 0, bufferToFill.numSamples);

        const auto master = SynthParameters::getMasterLevel();
        bufferToFill.buffer->applyGain(0, bufferToFill.numSamples, master);
    }

    void releaseResources() override {}

    void paint(juce::Graphics& g) override
    {
        g.fillAll(SynthTheme::background);
        SynthTheme::drawScanlines(g, getLocalBounds());

        auto header = getLocalBounds().removeFromTop(kHeaderHeight);
        juce::ColourGradient headerGlow(SynthTheme::accent.withAlpha(0.12f), 0.0f, 0.0f,
                                        juce::Colours::transparentBlack, 0.0f,
                                        static_cast<float>(header.getHeight()), false);
        g.setGradientFill(headerGlow);
        g.fillRect(header);

        g.setColour(SynthTheme::accent.withAlpha(0.5f));
        g.drawHorizontalLine(kHeaderHeight - 1, 0.0f, static_cast<float>(getWidth()));
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        auto header = bounds.removeFromTop(kHeaderHeight).reduced(kMargin, kHeaderVerticalPadding);
        auto titleRow = header.withSizeKeepingCentre(header.getWidth(), kHeaderTitleRowHeight);
        const auto titleWidth =
            juce::roundToInt(titleLabel.getFont().getStringWidthFloat(titleLabel.getText())) + 8;
        titleLabel.setBounds(titleRow.removeFromLeft(titleWidth));
        titleRow.removeFromLeft(kHeaderTitleSubtitleGap);
        subtitleLabel.setBounds(titleRow);

        auto keyboardRow = bounds.removeFromBottom(kKeyboardHeight).reduced(kMargin, 6);
        auto sidePanel = keyboardRow.removeFromRight(kKeyboardSidePanelWidth);
        auto staffArea = keyboardRow.removeFromRight(kTrebleStaffWidth);
        trebleStaffDisplay.setBounds(staffArea.reduced(2, 4));
        auto toggleColumn = sidePanel.removeFromLeft(kPcKeyboardToggleColumnWidth);
        const auto toggleHeight = juce::jmax(18, (toggleColumn.getHeight() - 2) / 2);
        pcKeyboardOnButton.setBounds(toggleColumn.removeFromTop(toggleHeight).reduced(1, 1));
        pcKeyboardOffButton.setBounds(toggleColumn.reduced(1, 1));
        pcKeyboardDisplay.setBounds(sidePanel.reduced(2, 2));
        if (keyboardComponent != nullptr)
            keyboardComponent->setBounds(keyboardRow);

        editor.setBounds(bounds.reduced(kMargin, 8));
    }

    bool keyPressed(const juce::KeyPress& key) override
    {
        if (handleDiffShortcutKey(key))
            return true;

        return juce::AudioAppComponent::keyPressed(key);
    }

private:
    void focusPcKeyboardIfEnabled()
    {
        if (pcKeyboardEnabled && keyboardComponent != nullptr)
            keyboardComponent->grabKeyboardFocus();
    }

    juce::Array<int> collectActiveMidiNotes() const
    {
        juce::Array<int> notes;

        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<const SynthVoice*>(synth.getVoice(i)))
            {
                if (voice->isSoundingForMono())
                    notes.addIfNotAlreadyThere(voice->getCurrentMidiNote());
            }
        }

        notes.sort();
        return notes;
    }

    void setPcKeyboardEnabled(bool enabled)
    {
        if (pcKeyboardEnabled == enabled)
        {
            if (enabled)
                requestPcKeyboardFocus();
            return;
        }

        pcKeyboardEnabled = enabled;

        if (keyboardComponent != nullptr)
            setPcKeyboardMappingsEnabled(*keyboardComponent, enabled);

        pcKeyboardOnButton.setToggleState(enabled, juce::dontSendNotification);
        pcKeyboardOffButton.setToggleState(!enabled, juce::dontSendNotification);
        pcKeyboardDisplay.setMappingEnabled(enabled);

        if (enabled)
            requestPcKeyboardFocus();
    }

    void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage& message) override
    {
        midiCollector.addMessageToQueue(message);
    }

    void applyMonophonicMode() {}

    SynthVoice* findMonoSoundingVoice()
    {
        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
            {
                if (voice->isSoundingForMono())
                    return voice;
            }
        }

        return nullptr;
    }

    void silenceOtherVoices(const SynthVoice* keep)
    {
        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
            {
                if (voice != keep && voice->isSoundingForMono())
                    voice->panic();
            }
        }
    }

    void applyMonoModeMidi(juce::MidiBuffer& midi)
    {
        if (!SynthParameters::getMonoMode())
            return;

        juce::MidiBuffer processed;

        for (const auto metadata : midi)
        {
            const auto& message = metadata.getMessage();
            const auto samplePos = metadata.samplePosition;
            const int channel = message.getChannel();

            if (message.isNoteOn())
            {
                if (auto* voice = findMonoSoundingVoice())
                {
                    silenceOtherVoices(voice);
                    voice->legatoNoteOn(message.getNoteNumber(), message.getFloatVelocity());
                    continue;
                }

                for (int note = 0; note < 128; ++note)
                {
                    if (note != message.getNoteNumber())
                    {
                        processed.addEvent(juce::MidiMessage::noteOff(channel, note, 0.0f),
                                             samplePos);
                    }
                }

                processed.addEvent(message, samplePos);
            }
            else if (message.isNoteOff())
            {
                int heldNote = -1;
                for (int note = 127; note >= 0; --note)
                {
                    if (keyboardState.isNoteOnForChannels(0xffff, note))
                    {
                        heldNote = note;
                        break;
                    }
                }

                if (heldNote >= 0)
                {
                    if (auto* voice = findMonoSoundingVoice())
                    {
                        // Only retrigger when returning to a different held note (e.g. release
                        // the top key while a lower key remains). Releasing an already-silent
                        // key must not touch the sounding voice.
                        if (heldNote != voice->getCurrentMidiNote())
                        {
                            silenceOtherVoices(voice);
                            voice->legatoNoteOn(heldNote, 1.0f);
                        }
                    }

                    continue;
                }

                if (auto* voice = findMonoSoundingVoice())
                {
                    voice->setKeyDown(false);
                    voice->stopNote(message.getFloatVelocity(), true);
                    continue;
                }

                processed.addEvent(message, samplePos);
            }
            else
            {
                processed.addEvent(message, samplePos);
            }
        }

        midi.swapWith(processed);
    }

    void onPresetParametersChanged()
    {
        editor.refreshUIFromParameters();
        applyMonophonicMode();
        refreshPresetList();
        updatePresetSaveButtonState();
    }

    void resetToInitialSettings()
    {
        exitDiffMode();
        presetManager.resetToInitialSettings();
        captureDiffBaseline();
    }

    void captureDiffBaseline()
    {
        diffBaseline = PresetManager::captureCurrentParameters();
    }

    void handleDiffToggle(bool shouldBeActive)
    {
        if (shouldBeActive)
            enterDiffMode();
        else
            exitDiffMode();
    }

    void enterDiffMode()
    {
        if (diffActive || !diffBaseline.isObject())
        {
            editor.setDiffModeActive(false);
            return;
        }

        diffPreToggleSnapshot = PresetManager::captureCurrentParameters();
        applyParametersKeepingMaster(diffBaseline);
        diffActive = true;
        editor.setDiffModeActive(true);
        editor.refreshUIFromParameters();
        applyMonophonicMode();
        updatePresetSaveButtonState();
    }

    void exitDiffMode()
    {
        if (!diffActive)
            return;

        applyParametersKeepingMaster(diffPreToggleSnapshot);
        diffPreToggleSnapshot = juce::var();
        diffActive = false;
        editor.setDiffModeActive(false);
        editor.refreshUIFromParameters();
        applyMonophonicMode();
        updatePresetSaveButtonState();
    }

    void refreshPresetList()
    {
        editor.setPresetNames(presetManager.getPresetNames(), presetManager.getCurrentIndex());
    }

    void updatePresetSaveButtonState()
    {
        if (diffActive)
        {
            editor.setPresetSaveButtonsEnabled(false, false);
            return;
        }

        const auto dirty = presetManager.isCurrentPresetDirty();
        const auto userPreset = !presetManager.isCurrentPresetFactory();
        editor.setPresetSaveButtonsEnabled(dirty && userPreset, dirty);
    }

    void promptOverwritePreset()
    {
        if (presetManager.isCurrentPresetFactory())
            return;

        const auto currentName = presetManager.getCurrentPresetName();
        auto dialog = std::make_unique<juce::AlertWindow>("Save Preset",
                                                          "Save changes to this preset:",
                                                          juce::MessageBoxIconType::NoIcon);
        dialog->addTextEditor("name", currentName);
        if (auto* editorField = dialog->getTextEditor("name"))
            editorField->setReadOnly(true);
        dialog->addButton("Save", 1);
        dialog->addButton("Cancel", 0);

        dialog->enterModalState(
            true,
            juce::ModalCallbackFunction::create([this, d = std::move(dialog)](int result) mutable
            {
                if (result != 1)
                    return;

                if (presetManager.overwriteCurrentUserPreset())
                {
                    refreshPresetList();
                    updatePresetSaveButtonState();
                }
            }),
            false);
    }

    void promptSavePresetAs()
    {
        auto dialog = std::make_unique<juce::AlertWindow>("Save Preset As",
                                                          "Enter a name for this preset:",
                                                          juce::MessageBoxIconType::NoIcon);
        dialog->addTextEditor("name", "My Preset");
        dialog->addButton("Save", 1);
        dialog->addButton("Cancel", 0);

        dialog->enterModalState(
            true,
            juce::ModalCallbackFunction::create([this, d = std::move(dialog)](int result) mutable
            {
                if (result == 1)
                {
                    const auto name = d->getTextEditorContents("name").trim();
                    if (name.isNotEmpty() && presetManager.saveCurrentAsUserPreset(name))
                    {
                        refreshPresetList();
                        updatePresetSaveButtonState();
                    }
                }
            }),
            false);
    }

    void promptLoadPresetFile()
    {
        auto chooser = std::make_shared<juce::FileChooser>(
            "Load preset",
            presetManager.getUserPresetsDirectory(),
            "*.json");

        chooser->launchAsync(juce::FileBrowserComponent::openMode
                                 | juce::FileBrowserComponent::canSelectFiles,
                             [this, chooser](const juce::FileChooser& fc)
        {
            const auto file = fc.getResult();
            if (file == juce::File())
                return;

            const auto name = file.getFileNameWithoutExtension();
            if (presetManager.loadUserPreset(name))
            {
                exitDiffMode();
                captureDiffBaseline();
            }
        });
    }

    void refreshMidiDeviceList(int midiComboId = 1)
    {
        midiDeviceNames.clear();
        midiDeviceIdentifiers.clear();

        for (const auto& device : juce::MidiInput::getAvailableDevices())
        {
            midiDeviceNames.add(device.name);
            midiDeviceIdentifiers.add(device.identifier);
        }

        editor.setMidiDeviceNames(midiDeviceNames, midiComboId);
    }

    int resolveMidiComboId(const AppState::Session& session) const
    {
        if (session.midiAllInputs)
            return 1;

        const int index = midiDeviceIdentifiers.indexOf(session.midiDeviceId);
        return index >= 0 ? index + 2 : 1;
    }

    void restoreSession(const AppState::Session& session)
    {
        if (!session.valid)
        {
            reconnectMidiInput(1);
            updatePresetSaveButtonState();
            return;
        }

        if (session.parameters.isObject())
            PresetManager::applyParametersFromVar(session.parameters);

        presetManager.setCurrentIndex(session.presetIndex);
        presetManager.markPresetBaselineFromCurrent();

        const auto midiComboId = resolveMidiComboId(session);
        refreshMidiDeviceList(midiComboId);
        reconnectMidiInput(midiComboId);

        editor.refreshUIFromParameters();
        refreshPresetList();
        applyMonophonicMode();
        updatePresetSaveButtonState();
    }

    void reconnectMidiInput(int comboId)
    {
        closeMidiInputs();

        if (comboId <= 1)
        {
            for (const auto& id : midiDeviceIdentifiers)
            {
                if (auto input = juce::MidiInput::openDevice(id, this))
                {
                    input->start();
                    midiInputs.push_back(std::move(input));
                }
            }

            editor.setMidiStatusText(midiInputs.empty() ? "MIDI :: NO INPUT"
                                                        : "MIDI :: ALL INPUTS ACTIVE");
            return;
        }

        const auto index = comboId - 2;
        if (juce::isPositiveAndBelow(index, midiDeviceIdentifiers.size()))
        {
            if (auto input = juce::MidiInput::openDevice(midiDeviceIdentifiers[index], this))
            {
                input->start();
                midiInputs.push_back(std::move(input));
                editor.setMidiStatusText("MIDI :: " + midiDeviceNames[index]);
            }
        }
        else
        {
            editor.setMidiStatusText("MIDI :: NOT CONNECTED");
        }
    }

    void stopAllSound()
    {
        synth.allNotesOff(0, false);
        keyboardState.allNotesOff(0);

        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
                voice->panic();
        }

        midiCollector.reset(currentSampleRate);
    }

    void closeMidiInputs()
    {
        for (auto& input : midiInputs)
            input->stop();

        midiInputs.clear();
    }

    double currentSampleRate = 44100.0;
    juce::Synthesiser synth;
    juce::MidiMessageCollector midiCollector;
    juce::MidiKeyboardState keyboardState;
    std::unique_ptr<juce::MidiKeyboardComponent> keyboardComponent;
    FuturisticLookAndFeel keyboardBarLookAndFeel;
    juce::ToggleButton pcKeyboardOnButton;
    juce::ToggleButton pcKeyboardOffButton;
    PcKeyboardDisplay pcKeyboardDisplay;
    TrebleStaffDisplay trebleStaffDisplay;
    bool pcKeyboardEnabled = true;
    std::vector<std::unique_ptr<juce::MidiInput>> midiInputs;
    juce::StringArray midiDeviceNames;
    juce::StringArray midiDeviceIdentifiers;

    PresetManager presetManager;
    SynthEditor editor;
    juce::Label titleLabel;
    juce::Label subtitleLabel;

    juce::var diffBaseline;
    juce::var diffPreToggleSnapshot;
    bool diffActive = false;
    std::unique_ptr<DiffShortcutKeyListener> diffShortcutListener;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

class AnalogSynthApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "NEXUS OSC"; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        const auto session = AppState::load();
        mainWindow.reset(new MainWindow(getApplicationName(), session));
    }

    void shutdown() override { mainWindow = nullptr; }

private:
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(juce::String name, const AppState::Session& session)
            : DocumentWindow(name, SynthTheme::background, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(session), true);
            setResizable(true, true);

            if (session.valid && session.hasWindowBounds && session.windowW > 0 && session.windowH > 0)
                setBounds(restoredWindowBounds(session));
            else
                centreWithSize(kDefaultWindowWidth, kDefaultWindowHeight);

            setVisible(true);
            lockMinimumSizeToCurrentBounds();

            if (auto* content = dynamic_cast<MainComponent*>(getContentComponent()))
            {
                addKeyListener(content->getDiffShortcutListener());
                content->requestPcKeyboardFocus();
            }

            // Frame size may not be final until after the native peer finishes layout.
            juce::MessageManager::callAsync([this]
                                            {
                                                lockMinimumSizeToCurrentBounds();

                                                if (auto* content = dynamic_cast<MainComponent*>(getContentComponent()))
                                                    content->requestPcKeyboardFocus();
                                            });
        }

        ~MainWindow() override
        {
            if (auto* content = dynamic_cast<MainComponent*>(getContentComponent()))
                removeKeyListener(content->getDiffShortcutListener());

            persistSession();
        }

        bool keyPressed(const juce::KeyPress& key) override
        {
            if (auto* content = dynamic_cast<MainComponent*>(getContentComponent()))
                if (content->handleDiffShortcutKey(key))
                    return true;

            return DocumentWindow::keyPressed(key);
        }

        void closeButtonPressed() override
        {
            persistSession();
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        void persistSession()
        {
            if (auto* content = dynamic_cast<MainComponent*>(getContentComponent()))
                content->saveSession(getBounds());
        }

        void lockMinimumSizeToCurrentBounds()
        {
            int minW = getWidth();
            int minH = getHeight();
            int maxW = kMaxWindowWidth;
            int maxH = kMaxWindowHeight;

            if (auto* peer = getPeer())
            {
                if (const auto frame = peer->getFrameSizeIfPresent())
                {
                    const auto borderW = frame->getLeftAndRight();
                    const auto borderH = frame->getTopAndBottom();
                    minW += borderW;
                    minH += borderH;
                    maxW += borderW;
                    maxH += borderH;
                }
            }

            setResizeLimits(minW, minH, maxW, maxH);
        }
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(AnalogSynthApplication)
