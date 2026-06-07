#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

enum class TransposeLetter
{
    C = 0,
    D,
    E,
    F,
    G,
    A,
    B
};

enum class TransposeAccidental
{
    natural = 0,
    sharp,
    flat
};

int getTransposeSemitoneOffset(TransposeLetter letter, TransposeAccidental accidental);

class TransposeControl : public juce::Component
{
public:
    std::function<void(int semitoneOffset)> onTransposeChanged;

    TransposeControl();
    ~TransposeControl() override;

    void setBarLookAndFeel(juce::LookAndFeel* laf);
    int getSemitoneOffset() const;
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void notifyChange();

    juce::ComboBox letterCombo;
    juce::TextButton naturalButton;
    juce::TextButton sharpButton;
    juce::TextButton flatButton;
};
