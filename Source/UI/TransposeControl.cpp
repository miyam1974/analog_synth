#include "TransposeControl.h"

#include "SynthTheme.h"

namespace
{
constexpr int kToggleRowHeight = 28;

// [letter][accidental: natural, sharp, flat] — semitone offset applied to sounding pitch.
constexpr int8_t kTransposeTable[7][3] = {
    {0, 1, -1},   // C, C#, Cb
    {2, 3, 1},    // D, D#, Db
    {4, 5, 3},    // E, E#, Eb
    {5, 6, 4},    // F, F#, Fb
    {-5, -4, -6}, // G, G#, Gb
    {-3, -2, -4}, // A, A#, Ab
    {-1, 0, -2},  // B, B#, Bb
};

TransposeLetter letterFromComboId(int comboId)
{
    switch (comboId)
    {
    case 1:
        return TransposeLetter::F;
    case 2:
        return TransposeLetter::E;
    case 3:
        return TransposeLetter::D;
    case 4:
        return TransposeLetter::C;
    case 5:
        return TransposeLetter::B;
    case 6:
        return TransposeLetter::A;
    case 7:
        return TransposeLetter::G;
    default:
        return TransposeLetter::C;
    }
}
} // namespace

int getTransposeSemitoneOffset(TransposeLetter letter, TransposeAccidental accidental)
{
    const auto letterIndex = static_cast<int>(letter);
    const auto accidentalIndex = static_cast<int>(accidental);
    if (! juce::isPositiveAndBelow(letterIndex, 7) || ! juce::isPositiveAndBelow(accidentalIndex, 3))
        return 0;

    return kTransposeTable[letterIndex][accidentalIndex];
}

TransposeControl::TransposeControl()
{
    flatButton.setButtonText(juce::CharPointer_UTF8("\xe2\x99\xad"));
    naturalButton.setButtonText(juce::CharPointer_UTF8("\xe2\x99\xae")); // U+266E
    sharpButton.setButtonText(juce::CharPointer_UTF8("\xe2\x99\xaf"));
    for (auto* button : {&flatButton, &naturalButton, &sharpButton})
    {
        button->setComponentID("staffAccidentalToggle");
        button->setClickingTogglesState(true);
        button->setRadioGroupId(9103);
    }
    naturalButton.setToggleState(true, juce::dontSendNotification);
    flatButton.onClick = [this] { notifyChange(); };
    naturalButton.onClick = [this] { notifyChange(); };
    sharpButton.onClick = [this] { notifyChange(); };
    addAndMakeVisible(flatButton);
    addAndMakeVisible(naturalButton);
    addAndMakeVisible(sharpButton);

    letterCombo.setComponentID("transposeCombo");
    letterCombo.addItem("F", 1);
    letterCombo.addItem("E", 2);
    letterCombo.addItem("D", 3);
    letterCombo.addItem("C", 4);
    letterCombo.addItem("B", 5);
    letterCombo.addItem("A", 6);
    letterCombo.addItem("G", 7);
    letterCombo.setSelectedId(4, juce::dontSendNotification);
    letterCombo.onChange = [this] { notifyChange(); };
    addAndMakeVisible(letterCombo);
}

TransposeControl::~TransposeControl()
{
    setBarLookAndFeel(nullptr);
}

void TransposeControl::setBarLookAndFeel(juce::LookAndFeel* laf)
{
    letterCombo.setLookAndFeel(laf);
    naturalButton.setLookAndFeel(laf);
    sharpButton.setLookAndFeel(laf);
    flatButton.setLookAndFeel(laf);
}

int TransposeControl::getSemitoneOffset() const
{
    const auto letter = letterFromComboId(letterCombo.getSelectedId());
    const auto accidental = sharpButton.getToggleState()
                                ? TransposeAccidental::sharp
                                : flatButton.getToggleState() ? TransposeAccidental::flat
                                                              : TransposeAccidental::natural;
    return getTransposeSemitoneOffset(letter, accidental);
}

void TransposeControl::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(SynthTheme::panelFillHi);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(SynthTheme::accent.withAlpha(0.85f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.5f);
}

void TransposeControl::resized()
{
    auto bounds = getLocalBounds().reduced(1);
    auto toggleRow = bounds.removeFromTop(kToggleRowHeight);
    const auto toggleWidth = toggleRow.getWidth() / 3;
    flatButton.setBounds(toggleRow.removeFromLeft(toggleWidth).reduced(1, 1));
    naturalButton.setBounds(toggleRow.removeFromLeft(toggleWidth).reduced(1, 1));
    sharpButton.setBounds(toggleRow.reduced(1, 1));
    letterCombo.setBounds(bounds.reduced(2, 2));
}

void TransposeControl::notifyChange()
{
    if (onTransposeChanged)
        onTransposeChanged(getSemitoneOffset());
}
