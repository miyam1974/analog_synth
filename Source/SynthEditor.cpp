#include "SynthEditor.h"

#include "HelpStrings.h"
#include "SynthParameters.h"
#include "UI/SynthTheme.h"

namespace
{
constexpr int kMargin = 14;
constexpr int kFooterHeight = 84;
constexpr int kSystemControlRowHeight = 34;
constexpr int kSystemComboHeight = kSystemControlRowHeight * 2 / 3;
constexpr int kPanelGap = 8;
constexpr int kMasterRowHeight = 36;
constexpr int kCentDecimalPlaces = 2;
constexpr float kSectionTagFontSize = 11.0f;
constexpr int kSectionTagRowHeight = 16;
constexpr float kKnobCaptionFontSize = 11.0f;
constexpr float kKnobValueFontSize = 11.0f;
constexpr int kKnobCaptionRowHeight = 17;
constexpr int kKnobValueRowHeight = 19;
constexpr int kKnobSliderVerticalInset = 2;
constexpr int kSubOctGroupHeight = 44;
constexpr int kSubOctButtonHeight = kSubOctGroupHeight - kKnobCaptionRowHeight - 4;
constexpr int kSubOctGroupVerticalGap = 16;
constexpr float kHelpMessageFontSize = 14.0f;
constexpr int kMasterBarButtonWidth = 44;
constexpr int kMasterBarButtonHeight = kMasterRowHeight - 16;
constexpr int kTuneResetButtonRowHeight = kMasterBarButtonHeight + 4;
constexpr float kMasterBarLabelFontSize = 12.0f;
constexpr float kMasterBarButtonFontSize = 11.0f;
constexpr int kOscTuningTopGap = 24;
constexpr int kFilterGraphTopGap = 16;
constexpr int kLfoSliderToRouteGapBase = 3;
constexpr int kLfoSliderToRouteGap = kLfoSliderToRouteGapBase * 4;
constexpr int kLfoRouteButtonGap = 2;
constexpr int kLfoRoutesHeight = kSubOctButtonHeight * 3 + kLfoRouteButtonGap * 2 + 4;

void updateBipolarValueLabel(juce::Label& valueLabel, float value, int decimalPlaces)
{
    const auto step = SynthTheme::bipolarStepForDecimals(decimalPlaces);
    const auto isZero = SynthTheme::isBipolarNearZero(value, step);

    juce::String text;
    if (! isZero && value > 0.0f)
        text = "+" + juce::String(value, decimalPlaces);
    else
        text = juce::String(isZero ? 0.0f : value, decimalPlaces);

    valueLabel.setText(text, juce::dontSendNotification);

    const auto colour = isZero          ? SynthTheme::bipolarNeutral
                        : value > 0.0f ? SynthTheme::bipolarPositive
                                       : SynthTheme::bipolarNegative;
    valueLabel.setColour(juce::Label::textColourId, colour);
}

juce::String formatEnvTimeSec(float seconds)
{
    return juce::String(seconds, 2) + "s";
}

juce::String formatEnvSustainPercent(float level)
{
    return juce::String(level * 100.0f, 0) + "%";
}
} // namespace

SynthEditor::SynthEditor()
    : oscPanel("OSCILLATOR"),
      mixerPanel("MIXER"),
      filterPanel("FILTER"),
      ampPanel("AMPLIFIER"),
      lfoPanel("LFO"),
      systemPanel("SYSTEM")
{
    setLookAndFeel(&lookAndFeel);

    for (auto* panel : { &oscPanel, &mixerPanel, &filterPanel, &ampPanel, &lfoPanel, &systemPanel })
        addAndMakeVisible(panel);

    osc1Tag.setText("OSC1", juce::dontSendNotification);
    osc1Tag.setFont(SynthTheme::monoFont(kSectionTagFontSize));
    osc1Tag.setColour(juce::Label::textColourId, SynthTheme::textDim);
    osc1Tag.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(osc1Tag);

    osc2Tag.setText("OSC2", juce::dontSendNotification);
    osc2Tag.setFont(SynthTheme::monoFont(kSectionTagFontSize));
    osc2Tag.setColour(juce::Label::textColourId, SynthTheme::textDim);
    osc2Tag.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(osc2Tag);

    setupOsc1Waveforms();
    setupOsc2Waveforms();

    setupKnob(tuneCaption, tuneSlider, tuneValueLabel, "TUNE",
              SynthParameters::minTuneSemis, SynthParameters::maxTuneSemis, 0.0f, false, 0,
              HelpText::tune(),
              [](float v) { SynthParameters::setTuneSemis(v); }, 1.0f, true);
    setupKnob(fineCaption, fineSlider, fineValueLabel, "FINE",
              SynthParameters::minFineCents, SynthParameters::maxFineCents, 0.0f, false,
              kCentDecimalPlaces,
              HelpText::fine(),
              [](float v) { SynthParameters::setFineCents(v); }, 0.01f, true);
    setupKnob(osc2DetuneCaption, osc2DetuneSlider, osc2DetuneValueLabel, "DET2",
              SynthParameters::minOsc2DetuneCents, SynthParameters::maxOsc2DetuneCents, 0.0f,
              false, kCentDecimalPlaces,
              HelpText::osc2Detune(),
              [](float v) { SynthParameters::setOsc2DetuneCents(v); }, 0.01f, true);
    setupTuneResetButton();

    setupKnob(osc1LvlCaption, osc1LvlSlider, osc1LvlValueLabel, "OSC1",
              SynthParameters::minOscLevel, SynthParameters::maxOscLevel, 0.85f, false, 2,
              HelpText::osc1Level(),
              [](float v) { SynthParameters::setOsc1Level(v); });
    setupKnob(osc2LvlCaption, osc2LvlSlider, osc2LvlValueLabel, "OSC2",
              SynthParameters::minOscLevel, SynthParameters::maxOscLevel, 0.0f, false, 2,
              HelpText::osc2Level(),
              [](float v) { SynthParameters::setOsc2Level(v); });
    setupKnob(subLvlCaption, subLvlSlider, subLvlValueLabel, "SUB",
              SynthParameters::minSubLevel, SynthParameters::maxSubLevel, 0.0f, false, 2,
              HelpText::subLevel(),
              [](float v) { SynthParameters::setSubLevel(v); });
    setupKnob(noiseCaption, noiseSlider, noiseValueLabel, "NOISE",
              SynthParameters::minNoiseLevel, SynthParameters::maxNoiseLevel, 0.0f, false, 2,
              HelpText::noise(),
              [](float v) { SynthParameters::setNoiseLevel(v); });

    for (auto* caption : { &osc1LvlCaption, &osc2LvlCaption, &subLvlCaption, &noiseCaption })
        caption->setMinimumHorizontalScale(0.65f);

    setupKnob(glideCaption, glideSlider, glideValueLabel, "GLIDE",
              SynthParameters::minGlideSec, SynthParameters::maxGlideSec, 0.0f, false, 2,
              HelpText::glide(),
              [](float v) { SynthParameters::setGlideSec(v); });
    setupKnob(velAmpCaption, velAmpSlider, velAmpValueLabel, "V-A",
              SynthParameters::minVelocityDepth, SynthParameters::maxVelocityDepth, 0.6f, false, 2,
              HelpText::velocityAmp(),
              [](float v) { SynthParameters::setVelocityToAmp(v); });
    setupKnob(velFltCaption, velFltSlider, velFltValueLabel, "V-F",
              SynthParameters::minVelocityDepth, SynthParameters::maxVelocityDepth, 0.35f, false, 2,
              HelpText::velocityFilter(),
              [](float v) { SynthParameters::setVelocityToFilter(v); });

    setupSubOctaveButtons();

    setupKnob(cutoffCaption, cutoffSlider, cutoffValueLabel, "CUT",
              SynthParameters::minCutoffHz, SynthParameters::maxCutoffHz, 6000.0f, true, 0,
              HelpText::cutoff(),
              [](float v) { SynthParameters::setCutoffHz(v); });
    setupKnob(resonanceCaption, resonanceSlider, resonanceValueLabel, "RES",
              SynthParameters::minResonance, SynthParameters::maxResonance, 0.707f, false, 2,
              HelpText::resonance(),
              [](float v) { SynthParameters::setResonance(v); });
    setupKnob(filterEnvAmtCaption, filterEnvAmtSlider, filterEnvAmtValueLabel, "ENV",
              SynthParameters::minFilterEnvAmount, SynthParameters::maxFilterEnvAmount, 0.5f,
              false, 2,
              HelpText::filterEnvAmount(),
              [](float v) { SynthParameters::setFilterEnvAmount(v); });
    setupKnob(filterKeyCaption, filterKeySlider, filterKeyValueLabel, "KEY",
              SynthParameters::minFilterKeyTrack, SynthParameters::maxFilterKeyTrack, 0.0f,
              false, 2,
              HelpText::filterKeyTrack(),
              [](float v) { SynthParameters::setFilterKeyTrack(v); });

    addAndMakeVisible(filterEgDisplay);
    addAndMakeVisible(ampEgDisplay);
    registerHelp(filterEgDisplay, HelpText::filterEgGraph());
    registerHelp(ampEgDisplay, HelpText::ampEgGraph());

    setupEnvKnob(fAttackCaption, fAttackSlider, fAttackValueLabel, "FA",
                 SynthParameters::getFilterAttack(),
                 HelpText::filterAttack(),
                 [this](float v)
                 {
                     SynthParameters::setFilterAttack(v);
                     updateFilterEgDisplay();
                 });
    setupEnvKnob(fDecayCaption, fDecaySlider, fDecayValueLabel, "FD",
                 SynthParameters::getFilterDecay(),
                 HelpText::filterDecay(),
                 [this](float v)
                 {
                     SynthParameters::setFilterDecay(v);
                     updateFilterEgDisplay();
                 });
    setupEnvSustainKnob(fSustainCaption, fSustainSlider, fSustainValueLabel, "FS",
                        SynthParameters::getFilterSustain(),
                        HelpText::filterSustain(),
                        [this](float v)
                        {
                            SynthParameters::setFilterSustain(v);
                            updateFilterEgDisplay();
                        });
    setupEnvKnob(fReleaseCaption, fReleaseSlider, fReleaseValueLabel, "FR",
                 SynthParameters::getFilterRelease(),
                 HelpText::filterRelease(),
                 [this](float v)
                 {
                     SynthParameters::setFilterRelease(v);
                     updateFilterEgDisplay();
                 });

    setupEnvKnob(aAttackCaption, aAttackSlider, aAttackValueLabel, "A",
                 SynthParameters::getAmpAttack(),
                 HelpText::ampAttack(),
                 [this](float v)
                 {
                     SynthParameters::setAmpAttack(v);
                     updateAmpEgDisplay();
                 });
    setupEnvKnob(aDecayCaption, aDecaySlider, aDecayValueLabel, "D",
                 SynthParameters::getAmpDecay(),
                 HelpText::ampDecay(),
                 [this](float v)
                 {
                     SynthParameters::setAmpDecay(v);
                     updateAmpEgDisplay();
                 });
    setupEnvSustainKnob(aSustainCaption, aSustainSlider, aSustainValueLabel, "S",
                        SynthParameters::getAmpSustain(),
                        HelpText::ampSustain(),
                        [this](float v)
                        {
                            SynthParameters::setAmpSustain(v);
                            updateAmpEgDisplay();
                        });
    setupEnvKnob(aReleaseCaption, aReleaseSlider, aReleaseValueLabel, "R",
                 SynthParameters::getAmpRelease(),
                 HelpText::ampRelease(),
                 [this](float v)
                 {
                     SynthParameters::setAmpRelease(v);
                     updateAmpEgDisplay();
                 });

    addAndMakeVisible(lfoRateLed);
    registerHelp(lfoRateLed, HelpText::lfoRateLed());

    lfo1Tag.setText("LFO1", juce::dontSendNotification);
    lfo1Tag.setFont(SynthTheme::monoFont(kSectionTagFontSize));
    lfo1Tag.setColour(juce::Label::textColourId, SynthTheme::accentDim);
    lfo1Tag.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(lfo1Tag);

    setupKnob(lfoRateCaption, lfoRateSlider, lfoRateValueLabel, "RATE",
              SynthParameters::minLfoRateHz, SynthParameters::maxLfoRateHz, 4.0f, false, 2,
              HelpText::lfoRate(),
              [](float v) { SynthParameters::setLfoRateHz(v); });
    setupKnob(lfoDepthCaption, lfoDepthSlider, lfoDepthValueLabel, "DEPTH",
              SynthParameters::minLfoDepth, SynthParameters::maxLfoDepth, 0.3f, false, 2,
              HelpText::lfoDepth(),
              [](float v) { SynthParameters::setLfoDepth(v); });
    setupLfoRoutes();

    addAndMakeVisible(lfo2RateLed);
    registerHelp(lfo2RateLed, HelpText::lfo2RateLed());

    lfo2Tag.setText("LFO2", juce::dontSendNotification);
    lfo2Tag.setFont(SynthTheme::monoFont(kSectionTagFontSize));
    lfo2Tag.setColour(juce::Label::textColourId, SynthTheme::accentDim);
    lfo2Tag.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(lfo2Tag);

    setupKnob(lfo2RateCaption, lfo2RateSlider, lfo2RateValueLabel, "RATE",
              SynthParameters::minLfoRateHz, SynthParameters::maxLfoRateHz, 0.8f, false, 2,
              HelpText::lfo2Rate(),
              [](float v) { SynthParameters::setLfo2RateHz(v); });
    setupKnob(lfo2DepthCaption, lfo2DepthSlider, lfo2DepthValueLabel, "DEPTH",
              SynthParameters::minLfoDepth, SynthParameters::maxLfoDepth, 0.0f, false, 2,
              HelpText::lfo2Depth(),
              [](float v) { SynthParameters::setLfo2Depth(v); });
    setupLfo2Routes();

    for (auto* caption :
         { &lfoRateCaption, &lfoDepthCaption, &lfo2RateCaption, &lfo2DepthCaption })
    {
        caption->setMinimumHorizontalScale(0.65f);
        caption->setFont(SynthTheme::monoFont(kSectionTagFontSize));
    }

    masterCaption.setText("MASTER", juce::dontSendNotification);
    masterCaption.setJustificationType(juce::Justification::centredRight);
    masterCaption.setFont(SynthTheme::monoFont(kMasterBarButtonFontSize));
    masterCaption.setColour(juce::Label::textColourId, SynthTheme::textDim);
    addAndMakeVisible(masterCaption);

    masterSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    masterSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    masterSlider.setRange(SynthParameters::minMasterLevel, SynthParameters::maxMasterLevel, 0.001);
    masterSlider.setValue(SynthParameters::getMasterLevel(), juce::dontSendNotification);
    masterSlider.onValueChange = [this]
    {
        const auto value = static_cast<float>(masterSlider.getValue());
        SynthParameters::setMasterLevel(value);
        masterValueLabel.setText(formatEnvSustainPercent(value), juce::dontSendNotification);
        notifyParameterEdited();
    };
    addAndMakeVisible(masterSlider);

    masterValueLabel.setJustificationType(juce::Justification::centred);
    masterValueLabel.setFont(SynthTheme::monoFont(kKnobValueFontSize));
    masterValueLabel.setColour(juce::Label::textColourId, SynthTheme::accentBright);
    addAndMakeVisible(masterValueLabel);

    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.setFont(SynthTheme::monoFont(kMasterBarLabelFontSize));
    presetLabel.setColour(juce::Label::textColourId, SynthTheme::textDim);
    addAndMakeVisible(presetLabel);

    presetCombo.setComponentID("systemCombo");
    presetCombo.onChange = [this]
    {
        if (onPresetSelected)
            onPresetSelected(presetCombo.getSelectedItemIndex());
    };
    addAndMakeVisible(presetCombo);

    savePresetButton.setButtonText("SAVE");
    savePresetButton.setComponentID("presetSave");
    savePresetButton.setEnabled(false);
    savePresetButton.setColour(juce::TextButton::textColourOffId, SynthTheme::textDim.withAlpha(0.35f));
    savePresetButton.onClick = [this]
    {
        if (onPresetSave)
            onPresetSave();
    };
    addAndMakeVisible(savePresetButton);
    registerHelp(savePresetButton, HelpText::presetSave());

    saveAsPresetButton.setButtonText("SAVE AS");
    saveAsPresetButton.setComponentID("presetSave");
    saveAsPresetButton.setEnabled(false);
    saveAsPresetButton.setColour(juce::TextButton::textColourOffId, SynthTheme::textDim.withAlpha(0.35f));
    saveAsPresetButton.onClick = [this]
    {
        if (onPresetSaveAs)
            onPresetSaveAs();
    };
    addAndMakeVisible(saveAsPresetButton);
    registerHelp(saveAsPresetButton, HelpText::presetSaveAs());

    loadPresetButton.setButtonText("LOAD");
    loadPresetButton.setComponentID("masterBarText");
    loadPresetButton.onClick = [this]
    {
        if (onPresetLoad)
            onPresetLoad();
    };
    addAndMakeVisible(loadPresetButton);
    registerHelp(loadPresetButton, HelpText::presetLoad());

    resetDefaultsButton.setButtonText("RESET");
    resetDefaultsButton.setComponentID("masterBarText");
    resetDefaultsButton.onClick = [this]
    {
        if (onResetToDefaults)
            onResetToDefaults();
    };
    addAndMakeVisible(resetDefaultsButton);
    registerHelp(resetDefaultsButton, HelpText::presetReset());

    diffButton.setButtonText("DIFF");
    diffButton.setClickingTogglesState(true);
    diffButton.setComponentID("diffToggle");
    diffButton.onClick = [this]
    {
        if (onDiffToggleRequested)
            onDiffToggleRequested(diffButton.getToggleState());
    };
    addAndMakeVisible(diffButton);
    registerHelp(diffButton, HelpText::diffCompare());

    monoButton.setButtonText("MONO");
    monoButton.setComponentID("masterBarText");
    monoButton.setClickingTogglesState(true);
    monoButton.setToggleState(SynthParameters::getMonoMode(), juce::dontSendNotification);
    monoButton.onClick = [this]
    {
        const auto mono = monoButton.getToggleState();
        SynthParameters::setMonoMode(mono);
        if (onMonoModeChanged)
            onMonoModeChanged(mono);
        notifyParameterEdited();
    };
    addAndMakeVisible(monoButton);
    registerHelp(monoButton, HelpText::monoMode());

    panicButton.setComponentID("panic");
    panicButton.setButtonText("ALL OFF");
    panicButton.onClick = [this]
    {
        if (onPanic)
            onPanic();
    };
    addAndMakeVisible(panicButton);
    panicButton.toFront(false);

    midiLabel.setText("MIDI IN", juce::dontSendNotification);
    midiLabel.setFont(SynthTheme::monoFont(12.0f));
    midiLabel.setJustificationType(juce::Justification::centredLeft);
    midiLabel.setColour(juce::Label::textColourId, SynthTheme::textPrimary);
    addAndMakeVisible(midiLabel);

    midiInputCombo.setComponentID("systemCombo");
    midiInputCombo.setTextWhenNothingSelected("No devices");
    midiInputCombo.setJustificationType(juce::Justification::centredLeft);
    midiInputCombo.onChange = [this]
    {
        if (onMidiSelectionChanged)
            onMidiSelectionChanged(midiInputCombo.getSelectedId());
    };
    addAndMakeVisible(midiInputCombo);

    systemMessageLabel.setFont(SynthTheme::systemMessageFont(kHelpMessageFontSize));
    systemMessageLabel.setColour(juce::Label::textColourId, SynthTheme::textDim);
    systemMessageLabel.setJustificationType(juce::Justification::centredLeft);
    systemMessageLabel.setComponentID("helpMessage");
    addAndMakeVisible(systemMessageLabel);

    setupHelpHints();
    addMouseListener(this, true);

    midiStatusBackup = HelpText::defaultStatus();
    restoreStatusLine();
    refreshUIFromParameters();

    setSize(1080, 580);
}

void SynthEditor::setupOsc1Waveforms()
{
    for (int i = 0; i < static_cast<int>(Waveform::Count); ++i)
    {
        auto button = std::make_unique<WaveformButton>(static_cast<Waveform>(i));
        button->onSelect = [this, i]
        {
            selectOsc1Waveform(i);
            notifyParameterEdited();
        };
        addAndMakeVisible(*button);
        osc1Buttons[static_cast<size_t>(i)] = std::move(button);
    }

    selectOsc1Waveform(static_cast<int>(Waveform::Saw));
}

void SynthEditor::setupOsc2Waveforms()
{
    for (int i = 0; i < static_cast<int>(Waveform::Count); ++i)
    {
        auto button = std::make_unique<WaveformButton>(static_cast<Waveform>(i));
        button->onSelect = [this, i]
        {
            selectOsc2Waveform(i);
            notifyParameterEdited();
        };
        addAndMakeVisible(*button);
        osc2Buttons[static_cast<size_t>(i)] = std::move(button);
    }

    selectOsc2Waveform(static_cast<int>(Waveform::Square));
}

void SynthEditor::selectOsc1Waveform(int index)
{
    if (parametersLocked)
        return;

    if (!juce::isPositiveAndBelow(index, static_cast<int>(Waveform::Count)))
        return;

    SynthParameters::setOsc1Waveform(static_cast<Waveform>(index));

    for (int i = 0; i < static_cast<int>(Waveform::Count); ++i)
    {
        if (osc1Buttons[static_cast<size_t>(i)] != nullptr)
            osc1Buttons[static_cast<size_t>(i)]->setSelected(i == index);
    }
}

void SynthEditor::selectOsc2Waveform(int index)
{
    if (parametersLocked)
        return;

    if (!juce::isPositiveAndBelow(index, static_cast<int>(Waveform::Count)))
        return;

    SynthParameters::setOsc2Waveform(static_cast<Waveform>(index));

    for (int i = 0; i < static_cast<int>(Waveform::Count); ++i)
    {
        if (osc2Buttons[static_cast<size_t>(i)] != nullptr)
            osc2Buttons[static_cast<size_t>(i)]->setSelected(i == index);
    }
}

void SynthEditor::setupTuneResetButton()
{
    auto setupReset = [this](juce::TextButton& button, juce::Slider& slider,
                             juce::Label& valueLabel, int decimalPlaces,
                             std::function<void(float)> setter, const juce::String& help)
    {
        button.setButtonText("RESET");
        button.onClick = [this, &slider, &valueLabel, decimalPlaces, setter]
        {
            resetBipolarControl(slider, valueLabel, decimalPlaces, setter);
        };
        addAndMakeVisible(button);
        registerHelp(button, help);
    };

    setupReset(tuneResetButton, tuneSlider, tuneValueLabel, 0,
               [](float v) { SynthParameters::setTuneSemis(v); }, HelpText::tuneReset());
    setupReset(fineResetButton, fineSlider, fineValueLabel, kCentDecimalPlaces,
               [](float v) { SynthParameters::setFineCents(v); }, HelpText::fineReset());
    setupReset(osc2DetuneResetButton, osc2DetuneSlider, osc2DetuneValueLabel, kCentDecimalPlaces,
               [](float v) { SynthParameters::setOsc2DetuneCents(v); },
               HelpText::osc2DetuneReset());

    for (auto* button : { &tuneResetButton, &fineResetButton, &osc2DetuneResetButton })
        button->setComponentID("masterBarText");
}

void SynthEditor::resetBipolarControl(juce::Slider& slider, juce::Label& valueLabel,
                                      int decimalPlaces, std::function<void(float)> setter)
{
    if (parametersLocked)
        return;

    constexpr float resetValue = 0.0f;
    setter(resetValue);
    slider.setValue(resetValue, juce::dontSendNotification);
    updateBipolarValueLabel(valueLabel, resetValue, decimalPlaces);
    slider.repaint();
    notifyParameterEdited();
}

void SynthEditor::notifyParameterEdited()
{
    if (parametersLocked)
        return;

    if (onParameterEdited)
        onParameterEdited();
}

void SynthEditor::setupSubOctaveButtons()
{
    auto updateSubOctaveUi = [this]
    {
        const auto down2 = SynthParameters::getSubOctave() == SubOctave::Down2;
        subOct1Button.setToggleState(!down2, juce::dontSendNotification);
        subOct2Button.setToggleState(down2, juce::dontSendNotification);
    };

    subOctCaption.setText("SUB", juce::dontSendNotification);
    subOctCaption.setJustificationType(juce::Justification::centred);
    subOctCaption.setFont(SynthTheme::monoFont(kKnobCaptionFontSize));
    subOctCaption.setColour(juce::Label::textColourId, SynthTheme::textDim);
    addAndMakeVisible(subOctGroupFrame);
    addAndMakeVisible(subOctCaption);

    subOct1Button.setButtonText("-1 OCT");
    subOct1Button.setComponentID("masterBarText");
    subOct1Button.setClickingTogglesState(false);
    subOct1Button.onClick = [this, updateSubOctaveUi]
    {
        SynthParameters::setSubOctave(SubOctave::Down1);
        updateSubOctaveUi();
        notifyParameterEdited();
    };
    addAndMakeVisible(subOct1Button);

    subOct2Button.setButtonText("-2 OCT");
    subOct2Button.setComponentID("masterBarText");
    subOct2Button.setClickingTogglesState(false);
    subOct2Button.onClick = [this, updateSubOctaveUi]
    {
        SynthParameters::setSubOctave(SubOctave::Down2);
        updateSubOctaveUi();
        notifyParameterEdited();
    };
    addAndMakeVisible(subOct2Button);

    registerHelpGroup({ &subOctCaption, &subOct1Button, &subOct2Button }, HelpText::subOctave());

    updateSubOctaveUi();
}

void SynthEditor::setupLfoRoutes()
{
    auto setupRoute = [this](juce::ToggleButton& button, const juce::String& text,
                             bool initial, const juce::String& help, auto setter)
    {
        button.setButtonText(text);
        button.setComponentID("lfoRoute");
        button.setClickingTogglesState(true);
        button.setToggleState(initial, juce::dontSendNotification);
        button.onClick = [this, setter, &button]
        {
            setter(button.getToggleState());
            notifyParameterEdited();
        };
        addAndMakeVisible(button);
        registerHelp(button, help);
    };

    setupRoute(lfoToPitchButton, "PITCH", SynthParameters::getLfoToPitch(),
               HelpText::lfoPitch(),
               [](bool v) { SynthParameters::setLfoToPitch(v); });
    setupRoute(lfoToFilterButton, "FILTER", SynthParameters::getLfoToFilter(),
               HelpText::lfoFilter(),
               [](bool v) { SynthParameters::setLfoToFilter(v); });
    setupRoute(lfoToAmpButton, "AMP", SynthParameters::getLfoToAmp(),
               HelpText::lfoAmp(),
               [](bool v) { SynthParameters::setLfoToAmp(v); });
}

void SynthEditor::setupLfo2Routes()
{
    auto setupRoute = [this](juce::ToggleButton& button, const juce::String& text,
                             bool initial, const juce::String& help, auto setter)
    {
        button.setButtonText(text);
        button.setComponentID("lfoRoute");
        button.setClickingTogglesState(true);
        button.setToggleState(initial, juce::dontSendNotification);
        button.onClick = [this, setter, &button]
        {
            setter(button.getToggleState());
            notifyParameterEdited();
        };
        addAndMakeVisible(button);
        registerHelp(button, help);
    };

    setupRoute(lfo2ToPitchButton, "PITCH", SynthParameters::getLfo2ToPitch(),
               HelpText::lfo2Pitch(),
               [](bool v) { SynthParameters::setLfo2ToPitch(v); });
    setupRoute(lfo2ToFilterButton, "FILTER", SynthParameters::getLfo2ToFilter(),
               HelpText::lfo2Filter(),
               [](bool v) { SynthParameters::setLfo2ToFilter(v); });
    setupRoute(lfo2ToAmpButton, "AMP", SynthParameters::getLfo2ToAmp(),
               HelpText::lfo2Amp(),
               [](bool v) { SynthParameters::setLfo2ToAmp(v); });
}

void SynthEditor::setupKnob(juce::Label& caption, juce::Slider& slider, juce::Label& valueLabel,
                            const juce::String& name, float minValue, float maxValue,
                            float defaultValue, bool logarithmic, int decimalPlaces,
                            const juce::String& helpText, std::function<void(float)> onChange,
                            float step, bool bipolar)
{
    if (name.isNotEmpty())
    {
        caption.setText(name, juce::dontSendNotification);
        caption.setJustificationType(juce::Justification::centred);
        caption.setFont(SynthTheme::monoFont(kKnobCaptionFontSize));
        caption.setColour(juce::Label::textColourId, SynthTheme::textDim);
        addAndMakeVisible(caption);
    }

    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setRange(minValue, maxValue, logarithmic ? 0.0 : static_cast<double>(step));
    if (logarithmic)
        slider.setSkewFactorFromMidPoint(1000.0);
    if (bipolar)
        slider.setComponentID("bipolar");
    slider.setValue(defaultValue, juce::dontSendNotification);
    slider.onValueChange = [this, &slider, &valueLabel, decimalPlaces, onChange, bipolar]
    {
        const auto value = static_cast<float>(slider.getValue());
        onChange(value);
        if (bipolar)
            updateBipolarValueLabel(valueLabel, value, decimalPlaces);
        else
            valueLabel.setText(juce::String(value, decimalPlaces), juce::dontSendNotification);
        notifyParameterEdited();
    };
    addAndMakeVisible(slider);

    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setFont(SynthTheme::monoFont(kKnobValueFontSize));
    valueLabel.setColour(juce::Label::textColourId,
                         bipolar ? SynthTheme::bipolarNeutral : SynthTheme::accentBright);
    addAndMakeVisible(valueLabel);

    onChange(defaultValue);
    if (bipolar)
        updateBipolarValueLabel(valueLabel, defaultValue, decimalPlaces);
    else
        valueLabel.setText(juce::String(defaultValue, decimalPlaces), juce::dontSendNotification);
    registerHelpGroup({ &caption, &slider, &valueLabel }, helpText);
}

void SynthEditor::setupEnvKnob(juce::Label& caption, juce::Slider& slider, juce::Label& valueLabel,
                               const juce::String& name, float defaultSeconds,
                               const juce::String& helpText, std::function<void(float)> onChange)
{
    setupKnob(caption, slider, valueLabel, name, SynthParameters::minEnvTime,
              SynthParameters::maxEnvTime, defaultSeconds, false, 2, helpText, onChange);
    slider.onValueChange = [this, &slider, &valueLabel, onChange]
    {
        const auto value = static_cast<float>(slider.getValue());
        onChange(value);
        valueLabel.setText(formatEnvTimeSec(value), juce::dontSendNotification);
        notifyParameterEdited();
    };
    onChange(defaultSeconds);
    valueLabel.setText(formatEnvTimeSec(defaultSeconds), juce::dontSendNotification);
}

void SynthEditor::setupEnvSustainKnob(juce::Label& caption, juce::Slider& slider,
                                      juce::Label& valueLabel, const juce::String& name,
                                      float defaultLevel, const juce::String& helpText,
                                      std::function<void(float)> onChange)
{
    setupKnob(caption, slider, valueLabel, name, 0.0f, 1.0f, defaultLevel, false, 2, helpText,
              onChange);
    slider.onValueChange = [this, &slider, &valueLabel, onChange]
    {
        const auto value = static_cast<float>(slider.getValue());
        onChange(value);
        valueLabel.setText(formatEnvSustainPercent(value), juce::dontSendNotification);
        notifyParameterEdited();
    };
    onChange(defaultLevel);
    valueLabel.setText(formatEnvSustainPercent(defaultLevel), juce::dontSendNotification);
}

void SynthEditor::setMidiDeviceNames(const juce::StringArray& names, int selectedId)
{
    midiInputCombo.clear(juce::dontSendNotification);
    midiInputCombo.addItem("All Inputs", 1);

    int id = 2;
    for (const auto& name : names)
        midiInputCombo.addItem(name, id++);

    midiInputCombo.setSelectedId(selectedId, juce::dontSendNotification);
    if (midiInputCombo.getSelectedId() != selectedId)
        midiInputCombo.setSelectedId(1, juce::dontSendNotification);
}

void SynthEditor::setPresetNames(const juce::StringArray& names, int selectedIndex)
{
    presetCombo.clear(juce::dontSendNotification);
    for (int i = 0; i < names.size(); ++i)
        presetCombo.addItem(names[i], i + 1);

    if (juce::isPositiveAndBelow(selectedIndex, names.size()))
        presetCombo.setSelectedItemIndex(selectedIndex, juce::dontSendNotification);
}

void SynthEditor::setPresetSaveButtonsEnabled(bool saveEnabled, bool saveAsEnabled)
{
    if (parametersLocked)
    {
        saveEnabled = false;
        saveAsEnabled = false;
    }

    savePresetButton.setEnabled(saveEnabled);
    saveAsPresetButton.setEnabled(saveAsEnabled);

    for (auto* button : { &savePresetButton, &saveAsPresetButton })
    {
        button->setColour(juce::TextButton::textColourOffId,
                          button->isEnabled() ? SynthTheme::presetSaveBright
                                              : SynthTheme::textDim.withAlpha(0.35f));
        button->repaint();
    }
}

void SynthEditor::setDiffModeActive(bool active)
{
    diffButton.setToggleState(active, juce::dontSendNotification);
    diffButton.repaint();
    setParametersLocked(active);
}

void SynthEditor::setParametersLocked(bool locked)
{
    parametersLocked = locked;

    juce::Component* const allowed[] = { &panicButton, &masterSlider, &masterCaption,
                                         &masterValueLabel, &midiLabel, &midiInputCombo,
                                         &diffButton };

    const auto isAllowed = [&](juce::Component* component)
    {
        for (auto* allowedComponent : allowed)
        {
            if (component == allowedComponent)
                return true;
        }

        return false;
    };

    std::function<void(juce::Component&)> setTreeEnabled;
    setTreeEnabled = [&](juce::Component& component)
    {
        if (!isAllowed(&component))
            component.setEnabled(!locked);

        for (int i = 0; i < component.getNumChildComponents(); ++i)
            setTreeEnabled(*component.getChildComponent(i));
    };

    for (int i = 0; i < getNumChildComponents(); ++i)
        setTreeEnabled(*getChildComponent(i));

    for (auto* allowedComponent : allowed)
        allowedComponent->setEnabled(true);

    for (const auto& button : osc1Buttons)
    {
        if (button != nullptr)
            button->setEnabled(!locked);
    }

    for (const auto& button : osc2Buttons)
    {
        if (button != nullptr)
            button->setEnabled(!locked);
    }
}

void SynthEditor::refreshSlider(juce::Slider& slider, juce::Label& valueLabel, float value,
                                int decimals)
{
    slider.setValue(value, juce::dontSendNotification);
    valueLabel.setText(juce::String(value, decimals), juce::dontSendNotification);
}

void SynthEditor::refreshBipolarSlider(juce::Slider& slider, juce::Label& valueLabel, float value,
                                       int decimals)
{
    slider.setValue(value, juce::dontSendNotification);
    updateBipolarValueLabel(valueLabel, value, decimals);
}

void SynthEditor::refreshToggle(juce::ToggleButton& button, bool state)
{
    button.setToggleState(state, juce::dontSendNotification);
}

void SynthEditor::refreshEnvTimeSlider(juce::Slider& slider, juce::Label& valueLabel, float seconds)
{
    slider.setValue(seconds, juce::dontSendNotification);
    valueLabel.setText(formatEnvTimeSec(seconds), juce::dontSendNotification);
}

void SynthEditor::refreshEnvSustainSlider(juce::Slider& slider, juce::Label& valueLabel, float level)
{
    slider.setValue(level, juce::dontSendNotification);
    valueLabel.setText(formatEnvSustainPercent(level), juce::dontSendNotification);
}

void SynthEditor::refreshUIFromParameters()
{
    selectOsc1Waveform(static_cast<int>(SynthParameters::getOsc1Waveform()));
    selectOsc2Waveform(static_cast<int>(SynthParameters::getOsc2Waveform()));

    refreshBipolarSlider(tuneSlider, tuneValueLabel, SynthParameters::getTuneSemis(), 0);
    refreshBipolarSlider(fineSlider, fineValueLabel, SynthParameters::getFineCents(),
                         kCentDecimalPlaces);
    refreshBipolarSlider(osc2DetuneSlider, osc2DetuneValueLabel,
                         SynthParameters::getOsc2DetuneCents(), kCentDecimalPlaces);
    refreshSlider(osc1LvlSlider, osc1LvlValueLabel, SynthParameters::getOsc1Level(), 2);
    refreshSlider(osc2LvlSlider, osc2LvlValueLabel, SynthParameters::getOsc2Level(), 2);
    refreshSlider(subLvlSlider, subLvlValueLabel, SynthParameters::getSubLevel(), 2);
    refreshSlider(noiseSlider, noiseValueLabel, SynthParameters::getNoiseLevel(), 2);
    refreshSlider(glideSlider, glideValueLabel, SynthParameters::getGlideSec(), 2);
    refreshSlider(velAmpSlider, velAmpValueLabel, SynthParameters::getVelocityToAmp(), 2);
    refreshSlider(velFltSlider, velFltValueLabel, SynthParameters::getVelocityToFilter(), 2);

    subOct1Button.setToggleState(SynthParameters::getSubOctave() == SubOctave::Down1,
                                 juce::dontSendNotification);
    subOct2Button.setToggleState(SynthParameters::getSubOctave() == SubOctave::Down2,
                                 juce::dontSendNotification);

    refreshSlider(cutoffSlider, cutoffValueLabel, SynthParameters::getCutoffHz(), 0);
    refreshSlider(resonanceSlider, resonanceValueLabel, SynthParameters::getResonance(), 2);
    refreshSlider(filterEnvAmtSlider, filterEnvAmtValueLabel,
                  SynthParameters::getFilterEnvAmount(), 2);
    refreshSlider(filterKeySlider, filterKeyValueLabel, SynthParameters::getFilterKeyTrack(), 2);

    refreshEnvTimeSlider(fAttackSlider, fAttackValueLabel, SynthParameters::getFilterAttack());
    refreshEnvTimeSlider(fDecaySlider, fDecayValueLabel, SynthParameters::getFilterDecay());
    refreshEnvSustainSlider(fSustainSlider, fSustainValueLabel, SynthParameters::getFilterSustain());
    refreshEnvTimeSlider(fReleaseSlider, fReleaseValueLabel, SynthParameters::getFilterRelease());

    refreshEnvTimeSlider(aAttackSlider, aAttackValueLabel, SynthParameters::getAmpAttack());
    refreshEnvTimeSlider(aDecaySlider, aDecayValueLabel, SynthParameters::getAmpDecay());
    refreshEnvSustainSlider(aSustainSlider, aSustainValueLabel, SynthParameters::getAmpSustain());
    refreshEnvTimeSlider(aReleaseSlider, aReleaseValueLabel, SynthParameters::getAmpRelease());

    refreshSlider(lfoRateSlider, lfoRateValueLabel, SynthParameters::getLfoRateHz(), 2);
    refreshSlider(lfoDepthSlider, lfoDepthValueLabel, SynthParameters::getLfoDepth(), 2);
    refreshToggle(lfoToPitchButton, SynthParameters::getLfoToPitch());
    refreshToggle(lfoToFilterButton, SynthParameters::getLfoToFilter());
    refreshToggle(lfoToAmpButton, SynthParameters::getLfoToAmp());

    refreshSlider(lfo2RateSlider, lfo2RateValueLabel, SynthParameters::getLfo2RateHz(), 2);
    refreshSlider(lfo2DepthSlider, lfo2DepthValueLabel, SynthParameters::getLfo2Depth(), 2);
    refreshToggle(lfo2ToPitchButton, SynthParameters::getLfo2ToPitch());
    refreshToggle(lfo2ToFilterButton, SynthParameters::getLfo2ToFilter());
    refreshToggle(lfo2ToAmpButton, SynthParameters::getLfo2ToAmp());

    refreshEnvSustainSlider(masterSlider, masterValueLabel, SynthParameters::getMasterLevel());
    monoButton.setToggleState(SynthParameters::getMonoMode(), juce::dontSendNotification);

    updateAmpEgDisplay();
    updateFilterEgDisplay();
}

void SynthEditor::updateAmpEgDisplay()
{
    ampEgDisplay.setParameters(static_cast<float>(aAttackSlider.getValue()),
                               static_cast<float>(aDecaySlider.getValue()),
                               static_cast<float>(aSustainSlider.getValue()),
                               static_cast<float>(aReleaseSlider.getValue()));
}

void SynthEditor::updateFilterEgDisplay()
{
    filterEgDisplay.setParameters(static_cast<float>(fAttackSlider.getValue()),
                                  static_cast<float>(fDecaySlider.getValue()),
                                  static_cast<float>(fSustainSlider.getValue()),
                                  static_cast<float>(fReleaseSlider.getValue()));
}

void SynthEditor::setMidiStatusText(const juce::String& text)
{
    midiStatusBackup = text;
    if (!showingHelp)
        systemMessageLabel.setText(text, juce::dontSendNotification);
}

void SynthEditor::resized()
{
    auto bounds = getLocalBounds().reduced(kMargin, kPanelGap);

    auto footer = bounds.removeFromBottom(kFooterHeight);
    systemPanel.setBounds(footer);

    auto sys = ModulePanel::contentBounds(footer);
    auto controlRow = sys.removeFromTop(juce::jmin(kSystemControlRowHeight, sys.getHeight()));
    midiLabel.setBounds(controlRow.removeFromLeft(72));
    auto comboArea = controlRow.removeFromLeft(220);
    midiInputCombo.setBounds(comboArea.withSizeKeepingCentre(comboArea.getWidth(), kSystemComboHeight));
    systemMessageLabel.setBounds(controlRow.reduced(6, 0));

    auto masterArea = bounds.removeFromTop(kMasterRowHeight);
    panicButton.setBounds(masterArea.removeFromLeft(84).reduced(0, 6));
    panicButton.toFront(false);
    monoButton.setBounds(masterArea.removeFromLeft(52).reduced(2, 8));
    presetLabel.setBounds(masterArea.removeFromLeft(48));
    presetCombo.setBounds(masterArea.removeFromLeft(120).reduced(0, 6));
    savePresetButton.setBounds(masterArea.removeFromLeft(44).reduced(2, 8));
    saveAsPresetButton.setBounds(masterArea.removeFromLeft(64).reduced(2, 8));
    loadPresetButton.setBounds(masterArea.removeFromLeft(48).reduced(2, 8));
    resetDefaultsButton.setBounds(masterArea.removeFromLeft(52).reduced(2, 8));
    diffButton.setBounds(masterArea.removeFromLeft(44).reduced(2, 8));
    masterCaption.setBounds(masterArea.removeFromRight(56));
    masterValueLabel.setBounds(masterArea.removeFromRight(44));
    masterSlider.setBounds(masterArea.reduced(4, 10));

    const auto totalWidth = bounds.getWidth();
    const int wOsc = totalWidth * 21 / 100;
    const int wMixer = totalWidth * 17 / 100;
    const int wFilter = totalWidth * 23 / 100;
    const int wAmp = totalWidth * 19 / 100;
    const int wLfo = totalWidth - wOsc - wMixer - wFilter - wAmp - kPanelGap * 4;

    auto placePanel = [&](ModulePanel& panel, int width)
    {
        auto area = bounds.removeFromLeft(width);
        bounds.removeFromLeft(kPanelGap);
        panel.setBounds(area);
        return ModulePanel::contentBounds(area);
    };

    auto oscArea = placePanel(oscPanel, wOsc);
    oscArea.removeFromTop(2);
    osc1Tag.setBounds(oscArea.removeFromTop(kSectionTagRowHeight));
    layoutOsc1Waveforms(oscArea.removeFromTop(58));
    osc2Tag.setBounds(oscArea.removeFromTop(kSectionTagRowHeight));
    layoutOsc2Waveforms(oscArea.removeFromTop(50));
    layoutOscTuning(oscArea);

    layoutMixer(placePanel(mixerPanel, wMixer));

    layoutAmpEnv(placePanel(ampPanel, wAmp));

    auto filterArea = placePanel(filterPanel, wFilter);
    layoutFilterMain(filterArea.removeFromTop(130));
    layoutFilterEnv(filterArea);

    layoutLfo(placePanel(lfoPanel, wLfo));
}

void SynthEditor::layoutKnobColumn(juce::Rectangle<int> col, juce::Label& caption,
                                   juce::Slider& slider, juce::Label& value)
{
    auto column = col.reduced(2, 0);
    caption.setBounds(column.removeFromTop(kKnobCaptionRowHeight));
    layoutKnobColumnBody(column, slider, value);
}

void SynthEditor::layoutKnobColumnBody(juce::Rectangle<int> col, juce::Slider& slider,
                                       juce::Label& value)
{
    auto column = col.reduced(2, 0);
    value.setBounds(column.removeFromBottom(kKnobValueRowHeight));
    slider.setBounds(column.reduced(4, kKnobSliderVerticalInset));
}

void SynthEditor::layoutOsc1Waveforms(juce::Rectangle<int> area)
{
    const auto w = area.getWidth() / static_cast<int>(Waveform::Count);
    for (int i = 0; i < static_cast<int>(Waveform::Count); ++i)
    {
        if (osc1Buttons[static_cast<size_t>(i)] != nullptr)
            osc1Buttons[static_cast<size_t>(i)]->setBounds(area.removeFromLeft(w).reduced(2));
    }
}

void SynthEditor::layoutOsc2Waveforms(juce::Rectangle<int> area)
{
    const auto w = area.getWidth() / static_cast<int>(Waveform::Count);
    for (int i = 0; i < static_cast<int>(Waveform::Count); ++i)
    {
        if (osc2Buttons[static_cast<size_t>(i)] != nullptr)
            osc2Buttons[static_cast<size_t>(i)]->setBounds(area.removeFromLeft(w).reduced(2));
    }
}

void SynthEditor::layoutTuningColumn(juce::Rectangle<int> col, juce::Label& caption,
                                     juce::TextButton& resetButton, juce::Slider& slider,
                                     juce::Label& value)
{
    auto column = col.reduced(2, 0);
    caption.setBounds(column.removeFromTop(kKnobCaptionRowHeight));
    auto resetRow = column.removeFromTop(kTuneResetButtonRowHeight);
    resetButton.setBounds(resetRow.withSizeKeepingCentre(
        juce::jmin(resetRow.getWidth() - 8, kMasterBarButtonWidth), kMasterBarButtonHeight));
    resetButton.toFront(false);

    value.setBounds(column.removeFromBottom(kKnobValueRowHeight));
    slider.setBounds(column.reduced(4, kKnobSliderVerticalInset));
}

void SynthEditor::layoutOscTuning(juce::Rectangle<int> area)
{
    area.removeFromTop(kOscTuningTopGap);
    const auto colW = area.getWidth() / 3;
    layoutTuningColumn(area.removeFromLeft(colW), tuneCaption, tuneResetButton, tuneSlider,
                       tuneValueLabel);
    layoutTuningColumn(area.removeFromLeft(colW), fineCaption, fineResetButton, fineSlider,
                       fineValueLabel);
    layoutTuningColumn(area, osc2DetuneCaption, osc2DetuneResetButton, osc2DetuneSlider,
                       osc2DetuneValueLabel);
}

void SynthEditor::layoutMixer(juce::Rectangle<int> area)
{
    const auto subOctBandH = kSubOctGroupHeight + kSubOctGroupVerticalGap * 2;
    const auto remainingH = area.getHeight() - subOctBandH;
    const auto sliderRowH = remainingH / 2;

    auto levelsArea = area.removeFromTop(sliderRowH);
    area.removeFromTop(kSubOctGroupVerticalGap);
    auto subOctGroupArea = area.removeFromTop(kSubOctGroupHeight);
    area.removeFromTop(kSubOctGroupVerticalGap);
    auto perfArea = area.removeFromTop(remainingH - sliderRowH);

    const auto colW = levelsArea.getWidth() / 4;
    layoutKnobColumn(levelsArea.removeFromLeft(colW), osc1LvlCaption, osc1LvlSlider, osc1LvlValueLabel);
    layoutKnobColumn(levelsArea.removeFromLeft(colW), osc2LvlCaption, osc2LvlSlider, osc2LvlValueLabel);
    layoutKnobColumn(levelsArea.removeFromLeft(colW), subLvlCaption, subLvlSlider, subLvlValueLabel);
    layoutKnobColumn(levelsArea, noiseCaption, noiseSlider, noiseValueLabel);

    subOctGroupFrame.setBounds(subOctGroupArea);
    auto subOctInner = subOctGroupArea;
    subOctCaption.setBounds(subOctInner.removeFromTop(kKnobCaptionRowHeight));
    const auto half = subOctInner.getWidth() / 2;
    subOct1Button.setBounds(subOctInner.removeFromLeft(half).reduced(2));
    subOct2Button.setBounds(subOctInner.reduced(2));
    subOctGroupFrame.toBack();

    const auto perfColW = perfArea.getWidth() / 3;
    layoutKnobColumn(perfArea.removeFromLeft(perfColW), glideCaption, glideSlider, glideValueLabel);
    layoutKnobColumn(perfArea.removeFromLeft(perfColW), velAmpCaption, velAmpSlider, velAmpValueLabel);
    layoutKnobColumn(perfArea, velFltCaption, velFltSlider, velFltValueLabel);
}

void SynthEditor::layoutFilterMain(juce::Rectangle<int> area)
{
    const auto colW = area.getWidth() / 4;
    layoutKnobColumn(area.removeFromLeft(colW), cutoffCaption, cutoffSlider, cutoffValueLabel);
    layoutKnobColumn(area.removeFromLeft(colW), resonanceCaption, resonanceSlider, resonanceValueLabel);
    layoutKnobColumn(area.removeFromLeft(colW), filterEnvAmtCaption, filterEnvAmtSlider,
                     filterEnvAmtValueLabel);
    layoutKnobColumn(area, filterKeyCaption, filterKeySlider, filterKeyValueLabel);
}

void SynthEditor::layoutFilterEnv(juce::Rectangle<int> area)
{
    area.removeFromTop(kFilterGraphTopGap);
    auto graphRow = area.removeFromTop(76);
    graphRow.removeFromBottom(4);
    filterEgDisplay.setBounds(graphRow);
    area.removeFromTop(4);
    const auto colW = area.getWidth() / 4;
    layoutKnobColumn(area.removeFromLeft(colW), fAttackCaption, fAttackSlider, fAttackValueLabel);
    layoutKnobColumn(area.removeFromLeft(colW), fDecayCaption, fDecaySlider, fDecayValueLabel);
    layoutKnobColumn(area.removeFromLeft(colW), fSustainCaption, fSustainSlider, fSustainValueLabel);
    layoutKnobColumn(area, fReleaseCaption, fReleaseSlider, fReleaseValueLabel);
}

void SynthEditor::layoutAmpEnv(juce::Rectangle<int> area)
{
    ampEgDisplay.setBounds(area.removeFromTop(76).reduced(0, 4));
    area.removeFromTop(4);
    const auto colW = area.getWidth() / 4;
    layoutKnobColumn(area.removeFromLeft(colW), aAttackCaption, aAttackSlider, aAttackValueLabel);
    layoutKnobColumn(area.removeFromLeft(colW), aDecayCaption, aDecaySlider, aDecayValueLabel);
    layoutKnobColumn(area.removeFromLeft(colW), aSustainCaption, aSustainSlider, aSustainValueLabel);
    layoutKnobColumn(area, aReleaseCaption, aReleaseSlider, aReleaseValueLabel);
}

void SynthEditor::layoutLfoSection(juce::Rectangle<int> area, juce::Label& tag, LfoRateLed& led,
                                   juce::Label& rateCaption, juce::Slider& rateSlider,
                                   juce::Label& rateValue, juce::Label& depthCaption,
                                   juce::Slider& depthSlider, juce::Label& depthValue,
                                   juce::ToggleButton& toPitch, juce::ToggleButton& toFilter,
                                   juce::ToggleButton& toAmp)
{
    tag.setBounds(area.removeFromTop(kSectionTagRowHeight));
    area.removeFromTop(2);

    const auto contentH = area.getHeight();
    const auto routesH = kLfoRoutesHeight;
    const auto knobsH = contentH - routesH - kLfoSliderToRouteGap;

    auto knobs = area.removeFromTop(knobsH);
    area.removeFromTop(kLfoSliderToRouteGap);
    auto routes = area.removeFromTop(routesH);

    const auto colW = knobs.getWidth() / 2;

    auto labels = knobs.removeFromTop(kKnobCaptionRowHeight);
    rateCaption.setBounds(labels.removeFromLeft(colW));
    depthCaption.setBounds(labels);

    auto ledRow = knobs.removeFromTop(10);
    led.setBounds(ledRow.removeFromLeft(colW).withSizeKeepingCentre(8, 8));

    layoutKnobColumnBody(knobs.removeFromLeft(colW).reduced(2, 0), rateSlider, rateValue);
    layoutKnobColumnBody(knobs.reduced(2, 0), depthSlider, depthValue);

    auto routesInner = routes.reduced(2, 0);
    const auto routeW = routesInner.getWidth();

    auto pitchSlot = routesInner.removeFromTop(kSubOctButtonHeight);
    toPitch.setBounds(pitchSlot.withSizeKeepingCentre(routeW, kSubOctButtonHeight));
    routesInner.removeFromTop(kLfoRouteButtonGap);

    auto filterSlot = routesInner.removeFromTop(kSubOctButtonHeight);
    toFilter.setBounds(filterSlot.withSizeKeepingCentre(routeW, kSubOctButtonHeight));
    routesInner.removeFromTop(kLfoRouteButtonGap);

    toAmp.setBounds(routesInner.withSizeKeepingCentre(routeW, kSubOctButtonHeight));
}

void SynthEditor::layoutLfo(juce::Rectangle<int> area)
{
    const auto halfW = area.getWidth() / 2;
    auto lfo1Area = area.removeFromLeft(halfW);
    area.removeFromLeft(4);
    layoutLfoSection(lfo1Area, lfo1Tag, lfoRateLed, lfoRateCaption, lfoRateSlider, lfoRateValueLabel,
                     lfoDepthCaption, lfoDepthSlider, lfoDepthValueLabel, lfoToPitchButton,
                     lfoToFilterButton, lfoToAmpButton);
    layoutLfoSection(area, lfo2Tag, lfo2RateLed, lfo2RateCaption, lfo2RateSlider, lfo2RateValueLabel,
                     lfo2DepthCaption, lfo2DepthSlider, lfo2DepthValueLabel, lfo2ToPitchButton,
                     lfo2ToFilterButton, lfo2ToAmpButton);
}

void SynthEditor::registerHelp(juce::Component& component, const juce::String& text)
{
    helpTexts[&component] = text;
}

void SynthEditor::registerHelpGroup(std::initializer_list<juce::Component*> components,
                                    const juce::String& text)
{
    for (auto* component : components)
    {
        if (component != nullptr)
            registerHelp(*component, text);
    }
}

void SynthEditor::setupHelpHints()
{
    registerHelp(oscPanel, HelpText::panelOsc());
    registerHelp(mixerPanel, HelpText::panelMixer());
    registerHelp(filterPanel, HelpText::panelFilter());
    registerHelp(ampPanel, HelpText::panelAmp());
    registerHelp(lfoPanel, HelpText::panelLfo());
    registerHelp(systemPanel, HelpText::panelSystem());

    registerHelp(panicButton, HelpText::panic());
    registerHelpGroup({ &masterCaption, &masterSlider, &masterValueLabel }, HelpText::master());
    registerHelp(presetCombo, HelpText::presetCombo());
    registerHelp(midiLabel, HelpText::midiLabelHelp());
    registerHelp(midiInputCombo, HelpText::midiCombo());

    if (osc1Buttons[static_cast<size_t>(Waveform::Sine)] != nullptr)
        registerHelp(*osc1Buttons[static_cast<size_t>(Waveform::Sine)], HelpText::waveSine());
    if (osc1Buttons[static_cast<size_t>(Waveform::Saw)] != nullptr)
        registerHelp(*osc1Buttons[static_cast<size_t>(Waveform::Saw)], HelpText::waveSaw());
    if (osc1Buttons[static_cast<size_t>(Waveform::Square)] != nullptr)
        registerHelp(*osc1Buttons[static_cast<size_t>(Waveform::Square)], HelpText::waveSquare());
    if (osc1Buttons[static_cast<size_t>(Waveform::Triangle)] != nullptr)
        registerHelp(*osc1Buttons[static_cast<size_t>(Waveform::Triangle)],
                     HelpText::waveTriangle());

    if (osc2Buttons[static_cast<size_t>(Waveform::Sine)] != nullptr)
        registerHelp(*osc2Buttons[static_cast<size_t>(Waveform::Sine)], HelpText::osc2Wave());
    if (osc2Buttons[static_cast<size_t>(Waveform::Saw)] != nullptr)
        registerHelp(*osc2Buttons[static_cast<size_t>(Waveform::Saw)], HelpText::osc2Wave());
    if (osc2Buttons[static_cast<size_t>(Waveform::Square)] != nullptr)
        registerHelp(*osc2Buttons[static_cast<size_t>(Waveform::Square)], HelpText::osc2Wave());
    if (osc2Buttons[static_cast<size_t>(Waveform::Triangle)] != nullptr)
        registerHelp(*osc2Buttons[static_cast<size_t>(Waveform::Triangle)], HelpText::osc2Wave());
}

void SynthEditor::mouseEnter(const juce::MouseEvent& event)
{
    if (const auto* text = findHelpText(event.eventComponent))
        showHelp(*text);
}

void SynthEditor::mouseExit(const juce::MouseEvent& event)
{
    if (findHelpText(event.eventComponent) == nullptr)
        return;

    juce::Component::SafePointer<SynthEditor> safe(this);
    juce::Timer::callAfterDelay(60, [safe]
    {
        if (safe == nullptr || safe->isMouseOverHelpTarget())
            return;

        safe->restoreStatusLine();
    });
}

const juce::String* SynthEditor::findHelpText(juce::Component* component) const
{
    for (auto* target = component; target != nullptr && target != this;
         target = target->getParentComponent())
    {
        const auto it = helpTexts.find(target);
        if (it != helpTexts.end())
            return &it->second;
    }

    return nullptr;
}

bool SynthEditor::isMouseOverHelpTarget()
{
    if (auto* component = getComponentAt(getMouseXYRelative()))
        return findHelpText(component) != nullptr;

    return false;
}

void SynthEditor::showHelp(const juce::String& text)
{
    showingHelp = true;
    systemMessageLabel.setColour(juce::Label::textColourId, SynthTheme::textPrimary);
    systemMessageLabel.setFont(SynthTheme::systemMessageFont(kHelpMessageFontSize));
    systemMessageLabel.setText(text, juce::dontSendNotification);
    systemMessageLabel.repaint();
}

void SynthEditor::restoreStatusLine()
{
    showingHelp = false;
    systemMessageLabel.setFont(SynthTheme::systemMessageFont(kHelpMessageFontSize));
    systemMessageLabel.setColour(juce::Label::textColourId, SynthTheme::textDim);
    systemMessageLabel.setText(midiStatusBackup, juce::dontSendNotification);
}
