#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

class TrebleStaffDisplay : public juce::Component,
                           private juce::Timer
{
public:
    std::function<juce::Array<int>()> getActiveNotes;

    TrebleStaffDisplay();
    ~TrebleStaffDisplay() override;

    void setBarLookAndFeel(juce::LookAndFeel* laf);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    enum class Accidental
    {
        none,
        sharp,
        flat
    };

    struct DisplayNote
    {
        int pitchClass = 0;
        int staffStep = 0;
        Accidental accidental = Accidental::none;
    };

    struct NoteLayout
    {
        int pitchClass = 0;
        int staffStep = 0;
        float y = 0.0f;
        float noteX = 0.0f;
        Accidental accidental = Accidental::none;
    };

    struct NoteLayoutAccidentalComparator
    {
        bool useSharpsSpelling = false;

        int compareElements(const NoteLayout& a, const NoteLayout& b) const;
    };

    void timerCallback() override;

    static float diatonicStepToY(float bottomLineY, float lineSpacing, int stepFromBottomLine);
    static void drawLedgerLine(juce::Graphics& g, float x, float y, float width, juce::Colour colour);
    static void drawNoteHead(juce::Graphics& g, float x, float y, juce::Colour colour,
                             float lineSpacing);
    static void drawAccidental(juce::Graphics& g, float accidentalColumnRight, float y,
                               Accidental accidental, float lineSpacing, float pathStaffSpace);
    static void drawTrebleClef(juce::Graphics& g, juce::Rectangle<float> bounds, float lineSpacing,
                               float bottomLineY);
    static DisplayNote pitchClassToDisplay(int pitchClass, bool useSharps);
    static int displayStepForPitchClass(int pitchClass, int lowestPitchClass, int baseStep,
                                        bool useSharps);
    static juce::Array<int> uniquePitchClasses(const juce::Array<int>& midiNotes);
    static float bravuraAccidentalPixelWidth(Accidental accidental, float lineSpacing,
                                             float pathStaffSpace);
    static void assignAccidentalColumnRights(const juce::Array<NoteLayout>& notes,
                                             float defaultColumnRight, float lineSpacing,
                                             float pathStaffSpace, bool useSharpsSpelling,
                                             juce::HashMap<int, float>& columnRightsOut);

    juce::TextButton sharpButton;
    juce::TextButton flatButton;
    bool useSharps = false;
};
