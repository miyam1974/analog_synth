#include <memory>
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
#include "UI/SynthTheme.h"

namespace
{
constexpr int kNumVoices = 16;
constexpr int kHeaderHeight = 56;
constexpr int kKeyboardHeight = 96;
constexpr int kMargin = 14;
} // namespace

class MainComponent : public juce::AudioAppComponent,
                      private juce::MidiInputCallback
{
public:
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
        titleLabel.setFont(SynthTheme::titleFont(22.0f));
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

        addAndMakeVisible(editor);
        editor.onMidiSelectionChanged = [this](int id) { reconnectMidiInput(id); };
        editor.onPanic = [this] { stopAllSound(); };
        editor.onMonoModeChanged = [this](bool) { applyMonophonicMode(); };
        editor.onPresetSelected = [this](int index) { presetManager.selectPreset(index); };
        editor.onPresetSave = [this] { promptSavePreset(); };
        editor.onPresetLoad = [this] { promptLoadPresetFile(); };

        refreshPresetList();
        refreshMidiDeviceList();
        restoreSession(session);

        setSize(1080, 680);
    }

    ~MainComponent() override
    {
        saveSession();
        shutdownAudio();
        closeMidiInputs();
        setLookAndFeel(nullptr);
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
        auto header = bounds.removeFromTop(kHeaderHeight).reduced(kMargin, 10);
        titleLabel.setBounds(header.removeFromTop(28));
        subtitleLabel.setBounds(header);

        auto keyboardArea = bounds.removeFromBottom(kKeyboardHeight).reduced(kMargin, 6);
        if (keyboardComponent != nullptr)
            keyboardComponent->setBounds(keyboardArea);

        editor.setBounds(bounds.reduced(kMargin, 8));
    }

private:
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
    }

    void refreshPresetList()
    {
        editor.setPresetNames(presetManager.getPresetNames(), presetManager.getCurrentIndex());
    }

    void promptSavePreset()
    {
        auto dialog = std::make_unique<juce::AlertWindow>("Save Preset",
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
                        refreshPresetList();
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
                onPresetParametersChanged();
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
            return;
        }

        if (session.parameters.isObject())
            PresetManager::applyParametersFromVar(session.parameters);

        presetManager.setCurrentIndex(session.presetIndex);

        const auto midiComboId = resolveMidiComboId(session);
        refreshMidiDeviceList(midiComboId);
        reconnectMidiInput(midiComboId);

        editor.refreshUIFromParameters();
        refreshPresetList();
        applyMonophonicMode();
    }

    void saveSession()
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

        if (auto* window = findParentComponentOfClass<juce::DocumentWindow>())
        {
            const auto bounds = window->getBounds();
            session.windowX = bounds.getX();
            session.windowY = bounds.getY();
            session.windowW = bounds.getWidth();
            session.windowH = bounds.getHeight();
        }

        AppState::save(session);
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
    std::vector<std::unique_ptr<juce::MidiInput>> midiInputs;
    juce::StringArray midiDeviceNames;
    juce::StringArray midiDeviceIdentifiers;

    PresetManager presetManager;
    SynthEditor editor;
    juce::Label titleLabel;
    juce::Label subtitleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

class AnalogSynthApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "NEXUS OSC"; }
    const juce::String getApplicationVersion() override { return "0.3.0"; }
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

            if (session.valid && session.windowW > 0 && session.windowH > 0)
            {
                setBounds(session.windowX, session.windowY, session.windowW, session.windowH);
            }
            else
            {
                centreWithSize(1080, 680);
            }

            setResizable(true, true);
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(AnalogSynthApplication)
