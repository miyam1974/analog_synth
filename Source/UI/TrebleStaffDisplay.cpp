#include "TrebleStaffDisplay.h"

#include "BinaryData.h"
#include "SynthTheme.h"

namespace
{
constexpr float kStaffSpanInLineSpacings = 5.0f;
constexpr float kStaffVerticalPadding = 5.0f;
constexpr float kNoteHeadHeight = 9.0f;
constexpr float kNoteHeadWidthRatio = 1.35f;
constexpr float kNoteHeadStaggerRatio = 0.28f;
constexpr float kSpaceNoteExtraOffsetX = 5.0f;
constexpr float kAccidentalToNoteGapRatio = 0.30f;
constexpr float kAccidentalScale = 0.88f;
constexpr float kAccidentalColumnGapRatio = 0.12f;
constexpr float kLedgerHalfWidth = 10.0f;
constexpr float kMiddleCLedgerHalfWidth = 18.0f;
constexpr float kClefAreaWidth = 36.0f;
constexpr float kStaffRightPadding = 6.0f;
constexpr int kAccidentalToggleColumnWidth = 18;
constexpr float kStaffLineThickness = 1.35f;
constexpr juce::juce_wchar kBravuraGClef = 0xe050;
constexpr juce::juce_wchar kBravuraAccidentalSharp = 0xe262;
constexpr juce::juce_wchar kBravuraAccidentalFlat = 0xe260;
// Bravura gClef design height (font units, y-up).
constexpr float kBravuraGClefDesignHeight = 1756.0f;
constexpr float kBravuraStaffSpaceFontUnits = 250.0f;

// Display range: middle C (ledger) .. Bb one octave above (B line in upper octave).
constexpr int kDisplayMinStep = -2;
constexpr int kDisplayMaxStep = 11;
constexpr int kRegisterSpan = 7;

constexpr int kSharpSteps[] = {-2, -2, -1, -1, 0, 1, 1, 2, 2, 3, 3, 4};
constexpr int kFlatSteps[] = {-2, -1, -1, 0, 0, 1, 2, 2, 3, 3, 4, 4};

bool isOnStaffLine(int staffStepFromBottomE)
{
    return staffStepFromBottomE % 2 == 0;
}

juce::Typeface::Ptr getBravuraTypeface()
{
    static const juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::Bravura_otf, static_cast<size_t>(BinaryData::Bravura_otfSize));
    return typeface;
}

float bravuraPathUnitsPerStaffSpace(const juce::Typeface::Ptr& typeface)
{
    const auto glyphIndex = typeface->getNominalGlyphForCodepoint(kBravuraGClef);
    if (! glyphIndex.has_value())
        return 0.0f;

    juce::Path probePath;
    typeface->getOutlineForGlyph(juce::TypefaceMetricsKind::portable,
                                 static_cast<int>(*glyphIndex), probePath);

    const auto pathHeight = probePath.getBounds().getHeight();
    if (pathHeight <= 0.0f)
        return 0.0f;

    const auto pathUnitsPerFontUnit = pathHeight / kBravuraGClefDesignHeight;
    return kBravuraStaffSpaceFontUnits * pathUnitsPerFontUnit;
}
} // namespace

TrebleStaffDisplay::TrebleStaffDisplay()
{
    sharpButton.setButtonText(juce::CharPointer_UTF8("\xe2\x99\xaf"));
    flatButton.setButtonText(juce::CharPointer_UTF8("\xe2\x99\xad"));
    sharpButton.setComponentID("staffAccidentalToggle");
    sharpButton.setClickingTogglesState(true);
    sharpButton.setRadioGroupId(9102);
    sharpButton.setToggleState(false, juce::dontSendNotification);
    sharpButton.onClick = [this]
    {
        useSharps = true;
        repaint();
    };
    addAndMakeVisible(sharpButton);

    flatButton.setComponentID("staffAccidentalToggle");
    flatButton.setClickingTogglesState(true);
    flatButton.setRadioGroupId(9102);
    flatButton.setToggleState(true, juce::dontSendNotification);
    flatButton.onClick = [this]
    {
        useSharps = false;
        repaint();
    };
    addAndMakeVisible(flatButton);

    startTimerHz(30);
}

TrebleStaffDisplay::~TrebleStaffDisplay()
{
    setBarLookAndFeel(nullptr);
}

void TrebleStaffDisplay::setBarLookAndFeel(juce::LookAndFeel* laf)
{
    sharpButton.setLookAndFeel(laf);
    flatButton.setLookAndFeel(laf);
}

void TrebleStaffDisplay::resized()
{
    auto toggleColumn = getLocalBounds().reduced(1).removeFromLeft(kAccidentalToggleColumnWidth);
    const auto halfHeight = toggleColumn.getHeight() / 2;
    sharpButton.setBounds(toggleColumn.removeFromTop(halfHeight).reduced(0, 1));
    flatButton.setBounds(toggleColumn.reduced(0, 1));
}

TrebleStaffDisplay::DisplayNote TrebleStaffDisplay::pitchClassToDisplay(int pitchClass,
                                                                       bool useSharpsSpelling)
{
    DisplayNote note;
    note.pitchClass = pitchClass;
    note.staffStep = useSharpsSpelling ? kSharpSteps[pitchClass] : kFlatSteps[pitchClass];

    switch (pitchClass)
    {
    case 1:
    case 3:
    case 6:
    case 8:
    case 10:
        note.accidental = useSharpsSpelling ? Accidental::sharp : Accidental::flat;
        break;
    default:
        note.accidental = Accidental::none;
        break;
    }

    return note;
}

int TrebleStaffDisplay::displayStepForPitchClass(int pitchClass, int lowestPitchClass, int baseStep,
                                                 bool useSharpsSpelling)
{
    auto step = useSharpsSpelling ? kSharpSteps[pitchClass] : kFlatSteps[pitchClass];

    if (pitchClass != lowestPitchClass)
    {
        while (step <= baseStep)
            step += kRegisterSpan;
    }

    return step;
}

juce::Array<int> TrebleStaffDisplay::uniquePitchClasses(const juce::Array<int>& midiNotes)
{
    juce::Array<int> pitchClasses;

    for (const auto midiNote : midiNotes)
    {
        const auto pitchClass = ((midiNote % 12) + 12) % 12;
        pitchClasses.addIfNotAlreadyThere(pitchClass);
    }

    pitchClasses.sort();
    return pitchClasses;
}

float TrebleStaffDisplay::diatonicStepToY(float bottomLineY, float lineSpacing, int stepFromBottomLine)
{
    return bottomLineY - static_cast<float>(stepFromBottomLine) * (lineSpacing * 0.5f);
}

void TrebleStaffDisplay::drawLedgerLine(juce::Graphics& g, float x, float y, float width,
                                        juce::Colour colour)
{
    g.setColour(colour);
    g.drawLine(x - width * 0.5f, y, x + width * 0.5f, y, 1.0f);
}

void TrebleStaffDisplay::drawNoteHead(juce::Graphics& g, float x, float y, juce::Colour colour,
                                      float lineSpacing)
{
    juce::ignoreUnused(lineSpacing);
    const auto headWidth = kNoteHeadHeight * kNoteHeadWidthRatio;
    const auto headHeight = kNoteHeadHeight;

    g.setColour(colour);
    g.fillEllipse(x - headWidth * 0.5f, y - headHeight * 0.5f, headWidth, headHeight);
    g.setColour(colour.withAlpha(0.85f));
    g.drawEllipse(x - headWidth * 0.5f, y - headHeight * 0.5f, headWidth, headHeight, 1.0f);
}

void TrebleStaffDisplay::drawAccidental(juce::Graphics& g, float accidentalColumnRight, float y,
                                        Accidental accidental, float lineSpacing,
                                        float pathStaffSpace)
{
    if (accidental == Accidental::none || pathStaffSpace <= 0.0f)
        return;

    const auto typeface = getBravuraTypeface();
    if (typeface == nullptr)
        return;

    const auto codepoint = accidental == Accidental::sharp ? kBravuraAccidentalSharp
                                                           : kBravuraAccidentalFlat;
    const auto glyphIndex = typeface->getNominalGlyphForCodepoint(codepoint);
    if (! glyphIndex.has_value())
        return;

    juce::Path accPath;
    typeface->getOutlineForGlyph(juce::TypefaceMetricsKind::portable,
                                 static_cast<int>(*glyphIndex), accPath);

    // Bravura/SMuFL: sharp and flat anchors at y = 0; 1 staff space = 250 font units (~2.5 spaces tall).
    const auto pixelScale = (lineSpacing / pathStaffSpace) * kAccidentalScale;
    const auto bounds = accPath.getBounds();
    const auto anchorX = accidentalColumnRight - bounds.getRight() * pixelScale;
    accPath.applyTransform(juce::AffineTransform::scale(pixelScale).translated(anchorX, y));

    g.setColour(SynthTheme::accentBright);
    g.fillPath(accPath);
}

void TrebleStaffDisplay::drawTrebleClef(juce::Graphics& g, juce::Rectangle<float> bounds,
                                        float lineSpacing, float bottomLineY)
{
    const auto typeface = getBravuraTypeface();
    if (typeface == nullptr)
        return;

    const auto glyphIndex = typeface->getNominalGlyphForCodepoint(kBravuraGClef);
    if (! glyphIndex.has_value())
        return;

    const auto pathStaffSpace = bravuraPathUnitsPerStaffSpace(typeface);
    if (pathStaffSpace <= 0.0f)
        return;

    juce::Path clefPath;
    typeface->getOutlineForGlyph(juce::TypefaceMetricsKind::portable,
                                 static_cast<int>(*glyphIndex), clefPath);

    // Bravura/SMuFL: G line (2nd staff line) at glyph origin y = 0; 1 staff space = 250 units.
    const auto gLineY = bottomLineY - lineSpacing;
    const auto pixelScale = lineSpacing / pathStaffSpace;
    const auto clefX = bounds.getX() + 2.0f;
    clefPath.applyTransform(juce::AffineTransform::scale(pixelScale).translated(clefX, gLineY));

    g.setColour(SynthTheme::textPrimary);
    g.fillPath(clefPath);
}

float TrebleStaffDisplay::bravuraAccidentalPixelWidth(Accidental accidental, float lineSpacing,
                                                      float pathStaffSpace)
{
    if (accidental == Accidental::none || pathStaffSpace <= 0.0f)
        return 0.0f;

    const auto typeface = getBravuraTypeface();
    if (typeface == nullptr)
        return 0.0f;

    const auto codepoint = accidental == Accidental::sharp ? kBravuraAccidentalSharp
                                                           : kBravuraAccidentalFlat;
    const auto glyphIndex = typeface->getNominalGlyphForCodepoint(codepoint);
    if (! glyphIndex.has_value())
        return 0.0f;

    juce::Path accPath;
    typeface->getOutlineForGlyph(juce::TypefaceMetricsKind::portable,
                                 static_cast<int>(*glyphIndex), accPath);

    const auto pixelScale = (lineSpacing / pathStaffSpace) * kAccidentalScale;
    return accPath.getBounds().getWidth() * pixelScale;
}

int accidentalKeySignatureIndex(int pitchClass, bool useSharpsSpelling)
{
    constexpr int sharpOrder[] = {6, 0, 7, 2, 9, 4, 11};
    constexpr int flatOrder[] = {11, 4, 9, 2, 7, 0, 6};
    const auto* order = useSharpsSpelling ? sharpOrder : flatOrder;

    for (int i = 0; i < 7; ++i)
        if (order[i] == pitchClass)
            return i;

    return 99;
}

int TrebleStaffDisplay::NoteLayoutAccidentalComparator::compareElements(const NoteLayout& a,
                                                                        const NoteLayout& b) const
{
    const auto orderDiff = accidentalKeySignatureIndex(a.pitchClass, useSharpsSpelling)
                           - accidentalKeySignatureIndex(b.pitchClass, useSharpsSpelling);
    if (orderDiff != 0)
        return orderDiff;

    return a.staffStep - b.staffStep;
}

void TrebleStaffDisplay::assignAccidentalColumnRights(const juce::Array<NoteLayout>& notes,
                                                      float defaultColumnRight, float lineSpacing,
                                                      float pathStaffSpace, bool useSharpsSpelling,
                                                      juce::HashMap<int, float>& columnRightsOut)
{
    columnRightsOut.clear();

    juce::Array<NoteLayout> withAccidentals;
    for (const auto& note : notes)
    {
        if (note.accidental != Accidental::none)
            withAccidentals.add(note);
    }

    NoteLayoutAccidentalComparator comparator;
    comparator.useSharpsSpelling = useSharpsSpelling;
    withAccidentals.sort(comparator);

    if (withAccidentals.isEmpty())
        return;

    const auto sharpWidth = bravuraAccidentalPixelWidth(Accidental::sharp, lineSpacing, pathStaffSpace);
    const auto flatWidth = bravuraAccidentalPixelWidth(Accidental::flat, lineSpacing, pathStaffSpace);
    const auto maxGlyphWidth = juce::jmax(sharpWidth, flatWidth);
    const auto columnSpacing = maxGlyphWidth + lineSpacing * kAccidentalColumnGapRatio;

    for (int i = 0; i < withAccidentals.size(); ++i)
    {
        const auto columnsFromRight = static_cast<float>(withAccidentals.size() - 1 - i);
        columnRightsOut.set(withAccidentals.getReference(i).pitchClass,
                            defaultColumnRight - columnsFromRight * columnSpacing);
    }
}

void TrebleStaffDisplay::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    if (bounds.isEmpty())
        return;

    g.setColour(SynthTheme::panelFillHi);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(SynthTheme::accent.withAlpha(0.85f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.5f);

    const auto staffArea = bounds.withTrimmedLeft(static_cast<float>(kAccidentalToggleColumnWidth))
                               .reduced(2.0f, 4.0f);
    const auto lineSpacing = juce::jmax(
        7.0f, (staffArea.getHeight() - kStaffVerticalPadding * 2.0f - kNoteHeadHeight)
                  / kStaffSpanInLineSpacings);
    const auto noteHeadHalf = kNoteHeadHeight * 0.5f;
    const auto bottomLineY =
        staffArea.getBottom() - kStaffVerticalPadding - lineSpacing - noteHeadHalf;
    const auto staffLinesLeft = staffArea.getX() + 2.0f;
    const auto staffLeft = staffArea.getX() + kClefAreaWidth;
    const auto staffRight = staffArea.getRight() - kStaffRightPadding;
    const auto noteXBase = staffLeft + (staffRight - staffLeft) * 0.62f;
    const auto noteXStagger = lineSpacing * kNoteHeadStaggerRatio;
    const auto headHalfWidth = kNoteHeadHeight * kNoteHeadWidthRatio * 0.5f;
    const auto lineNoteX = noteXBase - noteXStagger;
    // Fixed column left of line-note heads; gap ~0.3 staff spaces per standard engraving.
    const auto accidentalColumnRight =
        lineNoteX - headHalfWidth - lineSpacing * kAccidentalToNoteGapRatio;

    const auto lineColour = SynthTheme::accentBright.withAlpha(0.95f);
    g.setColour(lineColour);
    for (int i = 0; i < 5; ++i)
    {
        const auto y = bottomLineY - static_cast<float>(i) * lineSpacing;
        g.drawLine(staffLinesLeft, y, staffRight, y, kStaffLineThickness);
    }

    {
        const auto middleCLedgerY = diatonicStepToY(bottomLineY, lineSpacing, kDisplayMinStep);
        drawLedgerLine(g, lineNoteX, middleCLedgerY, kMiddleCLedgerHalfWidth,
                       lineColour.withAlpha(0.75f));
    }

    drawTrebleClef(g, staffArea, lineSpacing, bottomLineY);

    const auto typeface = getBravuraTypeface();
    const auto pathStaffSpace =
        typeface != nullptr ? bravuraPathUnitsPerStaffSpace(typeface) : 0.0f;

    juce::Array<int> notes;
    if (getActiveNotes)
        notes = getActiveNotes();

    if (notes.isEmpty())
        return;

    notes.sort();
    const auto lowestPitchClass = ((notes.getFirst() % 12) + 12) % 12;
    const auto baseStep = pitchClassToDisplay(lowestPitchClass, useSharps).staffStep;
    const auto pitchClasses = uniquePitchClasses(notes);
    const auto noteColour = SynthTheme::accent;

    juce::Array<NoteLayout> noteLayouts;
    for (const auto pitchClass : pitchClasses)
    {
        const auto displayNote = pitchClassToDisplay(pitchClass, useSharps);
        const auto staffStep =
            displayStepForPitchClass(pitchClass, lowestPitchClass, baseStep, useSharps);

        if (staffStep < kDisplayMinStep || staffStep > kDisplayMaxStep)
            continue;

        NoteLayout layout;
        layout.pitchClass = pitchClass;
        layout.staffStep = staffStep;
        layout.y = diatonicStepToY(bottomLineY, lineSpacing, staffStep);
        layout.noteX = noteXBase + (isOnStaffLine(staffStep) ? -noteXStagger
                                                               : noteXStagger + kSpaceNoteExtraOffsetX);
        layout.accidental = displayNote.accidental;
        noteLayouts.add(layout);
    }

    juce::HashMap<int, float> accidentalColumnRights;
    assignAccidentalColumnRights(noteLayouts, accidentalColumnRight, lineSpacing, pathStaffSpace,
                                 useSharps, accidentalColumnRights);

    for (const auto& layout : noteLayouts)
    {
        const auto y = layout.y;
        const auto noteX = layout.noteX;

        if (layout.staffStep < 0 || layout.staffStep > 8)
        {
            const auto ledgerHalfWidth =
                layout.staffStep == kDisplayMinStep ? kMiddleCLedgerHalfWidth : kLedgerHalfWidth;
            drawLedgerLine(g, noteX, y, ledgerHalfWidth, lineColour);
        }

        if (layout.accidental != Accidental::none)
        {
            const auto columnRight = accidentalColumnRights.contains(layout.pitchClass)
                                         ? accidentalColumnRights[layout.pitchClass]
                                         : accidentalColumnRight;
            drawAccidental(g, columnRight, y, layout.accidental, lineSpacing, pathStaffSpace);
        }

        drawNoteHead(g, noteX, y, noteColour, lineSpacing);
    }
}

void TrebleStaffDisplay::timerCallback()
{
    repaint();
}
