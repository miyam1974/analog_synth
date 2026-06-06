#pragma once

#include <array>
#include <functional>
#include <memory>
#include <unordered_map>

#include <juce_gui_basics/juce_gui_basics.h>

#include "UI/AdsrDisplay.h"
#include "GlobalLfo.h"
#include "UI/FuturisticLookAndFeel.h"
#include "UI/LfoRateLed.h"
#include "UI/ModulePanel.h"
#include "UI/WaveformButton.h"
#include "Waveform.h"

class SynthEditor : public juce::Component
{
public:
    std::function<void(int)> onMidiSelectionChanged;
    std::function<void(int)> onPresetSelected;
    std::function<void()> onPresetSave;
    std::function<void()> onPresetLoad;
    std::function<void()> onResetToDefaults;
    std::function<void(bool)> onMonoModeChanged;
    std::function<void()> onPanic;

    SynthEditor();

    void setMidiDeviceNames(const juce::StringArray& names, int selectedId = 1);
    void setMidiStatusText(const juce::String& text);
    void setPresetNames(const juce::StringArray& names, int selectedIndex);
    void refreshUIFromParameters();

    int getSelectedMidiIndex() const { return midiInputCombo.getSelectedId(); }

    void resized() override;

    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;

private:
    void registerHelp(juce::Component& component, const juce::String& text);
    void registerHelpGroup(std::initializer_list<juce::Component*> components,
                           const juce::String& text);
    void setupHelpHints();
    const juce::String* findHelpText(juce::Component* component) const;
    bool isMouseOverHelpTarget();
    void showHelp(const juce::String& text);
    void restoreStatusLine();

    void refreshSlider(juce::Slider& slider, juce::Label& valueLabel, float value, int decimals);
    void refreshBipolarSlider(juce::Slider& slider, juce::Label& valueLabel, float value,
                              int decimals);
    void refreshToggle(juce::ToggleButton& button, bool state);

    std::unordered_map<juce::Component*, juce::String> helpTexts;
    juce::String midiStatusBackup;
    bool showingHelp = false;

    void setupOsc1Waveforms();
    void setupOsc2Waveforms();
    void selectOsc1Waveform(int index);
    void selectOsc2Waveform(int index);
    void setupLfoRoutes();
    void setupLfo2Routes();
    void setupSubOctaveButtons();
    void setupKnob(juce::Label& caption, juce::Slider& slider, juce::Label& valueLabel,
                   const juce::String& name, float minValue, float maxValue, float defaultValue,
                   bool logarithmic, int decimalPlaces, const juce::String& helpText,
                   std::function<void(float)> onChange, float step = 0.001f,
                   bool bipolar = false);
    void setupEnvKnob(juce::Label& caption, juce::Slider& slider, juce::Label& valueLabel,
                      const juce::String& name, float defaultSeconds, const juce::String& helpText,
                      std::function<void(float)> onChange);
    void layoutKnobColumn(juce::Rectangle<int> col, juce::Label& caption, juce::Slider& slider,
                          juce::Label& value);
    void layoutKnobColumnBody(juce::Rectangle<int> col, juce::Slider& slider, juce::Label& value);
    void layoutOsc1Waveforms(juce::Rectangle<int> area);
    void layoutOsc2Waveforms(juce::Rectangle<int> area);
    void layoutOscTuning(juce::Rectangle<int> area);
    void layoutTuningColumn(juce::Rectangle<int> col, juce::Label& caption, juce::TextButton& resetButton,
                            juce::Slider& slider, juce::Label& value);
    void setupTuneResetButton();
    void resetBipolarControl(juce::Slider& slider, juce::Label& valueLabel, int decimalPlaces,
                               std::function<void(float)> setter);
    void layoutMixer(juce::Rectangle<int> area);
    void layoutFilterMain(juce::Rectangle<int> area);
    void layoutFilterEnv(juce::Rectangle<int> area);
    void layoutAmpEnv(juce::Rectangle<int> area);
    void layoutLfo(juce::Rectangle<int> area);
    void layoutLfoSection(juce::Rectangle<int> area, juce::Label& tag, LfoRateLed& led,
                          juce::Label& rateCaption, juce::Slider& rateSlider,
                          juce::Label& rateValue, juce::Label& depthCaption,
                          juce::Slider& depthSlider, juce::Label& depthValue,
                          juce::ToggleButton& toPitch, juce::ToggleButton& toFilter,
                          juce::ToggleButton& toAmp);
    void updateAmpEgDisplay();
    void updateFilterEgDisplay();

    FuturisticLookAndFeel lookAndFeel;

    ModulePanel oscPanel;
    ModulePanel mixerPanel;
    ModulePanel filterPanel;
    ModulePanel ampPanel;
    ModulePanel lfoPanel;
    ModulePanel systemPanel;

    juce::Label osc1Tag, osc2Tag;
    std::array<std::unique_ptr<WaveformButton>, static_cast<size_t>(Waveform::Count)> osc1Buttons {};
    std::array<std::unique_ptr<WaveformButton>, static_cast<size_t>(Waveform::Count)> osc2Buttons {};

    juce::TextButton panicButton;
    juce::TextButton monoButton;
    juce::TextButton subOct1Button;
    juce::TextButton subOct2Button;
    juce::Label subOctCaption;
    juce::TextButton savePresetButton;
    juce::TextButton loadPresetButton;
    juce::TextButton resetDefaultsButton;

    juce::Label tuneCaption, tuneValueLabel;
    juce::Slider tuneSlider;
    juce::TextButton tuneResetButton;
    juce::Label fineCaption, fineValueLabel;
    juce::Slider fineSlider;
    juce::TextButton fineResetButton;
    juce::Label osc2DetuneCaption, osc2DetuneValueLabel;
    juce::Slider osc2DetuneSlider;
    juce::TextButton osc2DetuneResetButton;

    juce::Label osc1LvlCaption, osc1LvlValueLabel;
    juce::Slider osc1LvlSlider;
    juce::Label osc2LvlCaption, osc2LvlValueLabel;
    juce::Slider osc2LvlSlider;
    juce::Label subLvlCaption, subLvlValueLabel;
    juce::Slider subLvlSlider;
    juce::Label noiseCaption, noiseValueLabel;
    juce::Slider noiseSlider;

    juce::Label glideCaption, glideValueLabel;
    juce::Slider glideSlider;
    juce::Label velAmpCaption, velAmpValueLabel;
    juce::Slider velAmpSlider;
    juce::Label velFltCaption, velFltValueLabel;
    juce::Slider velFltSlider;

    juce::Label cutoffCaption, cutoffValueLabel;
    juce::Slider cutoffSlider;
    juce::Label resonanceCaption, resonanceValueLabel;
    juce::Slider resonanceSlider;
    juce::Label filterEnvAmtCaption, filterEnvAmtValueLabel;
    juce::Slider filterEnvAmtSlider;
    juce::Label filterKeyCaption, filterKeyValueLabel;
    juce::Slider filterKeySlider;

    AdsrDisplay filterEgDisplay { AdsrDisplay::PlayheadSource::filter };

    juce::Label fAttackCaption, fAttackValueLabel;
    juce::Slider fAttackSlider;
    juce::Label fDecayCaption, fDecayValueLabel;
    juce::Slider fDecaySlider;
    juce::Label fSustainCaption, fSustainValueLabel;
    juce::Slider fSustainSlider;
    juce::Label fReleaseCaption, fReleaseValueLabel;
    juce::Slider fReleaseSlider;

    AdsrDisplay ampEgDisplay { AdsrDisplay::PlayheadSource::amplifier };

    juce::Label aAttackCaption, aAttackValueLabel;
    juce::Slider aAttackSlider;
    juce::Label aDecayCaption, aDecayValueLabel;
    juce::Slider aDecaySlider;
    juce::Label aSustainCaption, aSustainValueLabel;
    juce::Slider aSustainSlider;
    juce::Label aReleaseCaption, aReleaseValueLabel;
    juce::Slider aReleaseSlider;

    juce::Label lfo1Tag, lfo2Tag;

    LfoRateLed lfoRateLed { GlobalLfo::Index::Lfo1 };
    juce::Label lfoRateCaption, lfoRateValueLabel;
    juce::Slider lfoRateSlider;
    juce::Label lfoDepthCaption, lfoDepthValueLabel;
    juce::Slider lfoDepthSlider;
    juce::ToggleButton lfoToPitchButton;
    juce::ToggleButton lfoToFilterButton;
    juce::ToggleButton lfoToAmpButton;

    LfoRateLed lfo2RateLed { GlobalLfo::Index::Lfo2 };
    juce::Label lfo2RateCaption, lfo2RateValueLabel;
    juce::Slider lfo2RateSlider;
    juce::Label lfo2DepthCaption, lfo2DepthValueLabel;
    juce::Slider lfo2DepthSlider;
    juce::ToggleButton lfo2ToPitchButton;
    juce::ToggleButton lfo2ToFilterButton;
    juce::ToggleButton lfo2ToAmpButton;

    juce::Label presetLabel;
    juce::ComboBox presetCombo;
    juce::Label masterCaption, masterValueLabel;
    juce::Slider masterSlider;

    juce::Label midiLabel;
    juce::ComboBox midiInputCombo;
    juce::Label systemMessageLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthEditor)
};
