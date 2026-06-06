#include "PresetManager.h"

#include "SynthParameters.h"

namespace
{
void setField(juce::var& obj, const char* key, const juce::var& value)
{
    if (auto* dyn = obj.getDynamicObject())
        dyn->setProperty(key, value);
}

juce::var waveformToVar(Waveform w) { return static_cast<int>(w); }

Waveform waveformFromVar(const juce::var& v)
{
    const auto i = static_cast<int>(v);
    if (juce::isPositiveAndBelow(i, static_cast<int>(Waveform::Count)))
        return static_cast<Waveform>(i);
    return Waveform::Saw;
}
} // namespace

PresetManager::PresetManager(ParametersChangedCallback onChanged)
    : onParametersChanged(std::move(onChanged))
{
    buildFactoryPresets();
    loadUserPresetsFromDisk();
}

void PresetManager::loadUserPresetsFromDisk()
{
    const auto dir = getUserPresetsDirectory();
    if (!dir.isDirectory())
        return;

    for (const auto& file : dir.findChildFiles(juce::File::findFiles, false, "*.json"))
    {
        const auto name = file.getFileNameWithoutExtension();
        if (name.isEmpty())
            continue;

        bool alreadyListed = false;
        for (const auto& existing : presets)
        {
            if (existing.name == name)
            {
                alreadyListed = true;
                break;
            }
        }

        if (alreadyListed)
            continue;

        const auto parsed = juce::JSON::parse(file);
        if (parsed.isVoid())
            continue;

        PresetEntry entry;
        entry.name = name;
        entry.isFactory = false;
        entry.data = parsed;
        presets.add(entry);
    }
}

juce::StringArray PresetManager::getPresetNames() const
{
    juce::StringArray names;
    for (const auto& preset : presets)
        names.add(preset.name);
    return names;
}

void PresetManager::selectPreset(int index)
{
    if (!juce::isPositiveAndBelow(index, presets.size()))
        return;

    currentIndex = index;
    applyPresetData(presets[index].data);
    if (onParametersChanged)
        onParametersChanged();
}

void PresetManager::setCurrentIndex(int index)
{
    if (juce::isPositiveAndBelow(index, presets.size()))
        currentIndex = index;
}

bool PresetManager::saveCurrentAsUserPreset(const juce::String& name)
{
    if (name.trim().isEmpty())
        return false;

    const auto file = getUserPresetsDirectory().getChildFile(name + ".json");
    file.getParentDirectory().createDirectory();

    const auto data = captureCurrentParameters();
    if (!file.replaceWithText(juce::JSON::toString(data, true)))
        return false;

    for (int i = 0; i < presets.size(); ++i)
    {
        if (!presets[i].isFactory && presets[i].name == name)
        {
            presets.getReference(i).data = data;
            currentIndex = i;
            return true;
        }
    }

    PresetEntry entry;
    entry.name = name;
    entry.isFactory = false;
    entry.data = data;
    presets.add(entry);
    currentIndex = presets.size() - 1;
    return true;
}

bool PresetManager::loadUserPreset(const juce::String& name)
{
    const auto file = getUserPresetsDirectory().getChildFile(name + ".json");
    if (!file.existsAsFile())
        return false;

    const auto parsed = juce::JSON::parse(file);
    if (parsed.isVoid())
        return false;

    for (int i = 0; i < presets.size(); ++i)
    {
        if (!presets[i].isFactory && presets[i].name == name)
        {
            presets.getReference(i).data = parsed;
            selectPreset(i);
            return true;
        }
    }

    PresetEntry entry;
    entry.name = name;
    entry.isFactory = false;
    entry.data = parsed;
    presets.add(entry);
    selectPreset(presets.size() - 1);
    return true;
}

juce::File PresetManager::getUserPresetsDirectory() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("NEXUS OSC")
        .getChildFile("Presets");
}

void PresetManager::buildFactoryPresets()
{
    presets.clear();

    auto addFactory = [this](const juce::String& name, const juce::var& data)
    {
        PresetEntry entry;
        entry.name = name;
        entry.isFactory = true;
        entry.data = data;
        presets.add(entry);
    };

    addFactory("INIT", captureCurrentParameters());

    {
        juce::var p(new juce::DynamicObject());
        setField(p, "osc1Waveform", waveformToVar(Waveform::Saw));
        setField(p, "osc2Waveform", waveformToVar(Waveform::Square));
        setField(p, "osc1Level", 0.7);
        setField(p, "osc2Level", 0.35);
        setField(p, "subLevel", 0.0);
        setField(p, "noiseLevel", 0.05);
        setField(p, "cutoffHz", 3200.0);
        setField(p, "resonance", 1.2);
        setField(p, "filterEnvAmount", 0.75);
        setField(p, "ampAttack", 0.35);
        setField(p, "ampDecay", 0.5);
        setField(p, "ampSustain", 0.85);
        setField(p, "ampRelease", 0.8);
        setField(p, "filterAttack", 0.4);
        setField(p, "filterDecay", 0.6);
        setField(p, "filterSustain", 0.5);
        setField(p, "filterRelease", 0.7);
        setField(p, "lfoRateHz", 0.25);
        setField(p, "lfoDepth", 0.45);
        setField(p, "lfoToFilter", true);
        setField(p, "lfo2RateHz", 0.12);
        setField(p, "lfo2Depth", 0.35);
        setField(p, "lfo2ToAmp", true);
        addFactory("PAD", p);
    }

    {
        juce::var p(new juce::DynamicObject());
        setField(p, "osc1Waveform", waveformToVar(Waveform::Saw));
        setField(p, "osc2Waveform", waveformToVar(Waveform::Saw));
        setField(p, "osc1Level", 0.9);
        setField(p, "osc2Level", 0.0);
        setField(p, "subLevel", 0.55);
        setField(p, "subOctave", static_cast<int>(SubOctave::Down1));
        setField(p, "noiseLevel", 0.0);
        setField(p, "cutoffHz", 900.0);
        setField(p, "resonance", 0.9);
        setField(p, "filterEnvAmount", 0.4);
        setField(p, "ampAttack", 0.005);
        setField(p, "ampDecay", 0.2);
        setField(p, "ampSustain", 0.35);
        setField(p, "ampRelease", 0.15);
        setField(p, "filterAttack", 0.01);
        setField(p, "filterDecay", 0.25);
        setField(p, "filterSustain", 0.2);
        setField(p, "filterRelease", 0.2);
        setField(p, "velocityToAmp", 0.85);
        setField(p, "velocityToFilter", 0.5);
        addFactory("BASS", p);
    }

    {
        juce::var p(new juce::DynamicObject());
        setField(p, "osc1Waveform", waveformToVar(Waveform::Square));
        setField(p, "osc2Waveform", waveformToVar(Waveform::Saw));
        setField(p, "osc1Level", 0.75);
        setField(p, "osc2Level", 0.5);
        setField(p, "cutoffHz", 7500.0);
        setField(p, "resonance", 1.8);
        setField(p, "filterEnvAmount", 0.55);
        setField(p, "ampAttack", 0.002);
        setField(p, "ampDecay", 0.12);
        setField(p, "ampSustain", 0.55);
        setField(p, "ampRelease", 0.18);
        setField(p, "glideSec", 0.12);
        setField(p, "monoMode", true);
        setField(p, "lfoRateHz", 6.0);
        setField(p, "lfoDepth", 0.2);
        setField(p, "lfoToPitch", true);
        addFactory("LEAD", p);
    }

    currentIndex = 0;
}

void PresetManager::applyPresetData(const juce::var& data)
{
    applyParametersFromVar(data);
}

juce::var PresetManager::captureCurrentParameters()
{
    juce::var root(new juce::DynamicObject());
    setField(root, "osc1Waveform", waveformToVar(SynthParameters::getOsc1Waveform()));
    setField(root, "osc2Waveform", waveformToVar(SynthParameters::getOsc2Waveform()));
    setField(root, "osc1Level", SynthParameters::getOsc1Level());
    setField(root, "osc2Level", SynthParameters::getOsc2Level());
    setField(root, "subLevel", SynthParameters::getSubLevel());
    setField(root, "subOctave", static_cast<int>(SynthParameters::getSubOctave()));
    setField(root, "tuneSemis", SynthParameters::getTuneSemis());
    setField(root, "fineCents", SynthParameters::getFineCents());
    setField(root, "noiseLevel", SynthParameters::getNoiseLevel());
    setField(root, "cutoffHz", SynthParameters::getCutoffHz());
    setField(root, "resonance", SynthParameters::getResonance());
    setField(root, "filterEnvAmount", SynthParameters::getFilterEnvAmount());
    setField(root, "filterKeyTrack", SynthParameters::getFilterKeyTrack());
    setField(root, "osc2DetuneCents", SynthParameters::getOsc2DetuneCents());
    setField(root, "ampAttack", SynthParameters::getAmpAttack());
    setField(root, "ampDecay", SynthParameters::getAmpDecay());
    setField(root, "ampSustain", SynthParameters::getAmpSustain());
    setField(root, "ampRelease", SynthParameters::getAmpRelease());
    setField(root, "filterAttack", SynthParameters::getFilterAttack());
    setField(root, "filterDecay", SynthParameters::getFilterDecay());
    setField(root, "filterSustain", SynthParameters::getFilterSustain());
    setField(root, "filterRelease", SynthParameters::getFilterRelease());
    setField(root, "lfoRateHz", SynthParameters::getLfoRateHz());
    setField(root, "lfoDepth", SynthParameters::getLfoDepth());
    setField(root, "lfoToPitch", SynthParameters::getLfoToPitch());
    setField(root, "lfoToFilter", SynthParameters::getLfoToFilter());
    setField(root, "lfoToAmp", SynthParameters::getLfoToAmp());
    setField(root, "lfo2RateHz", SynthParameters::getLfo2RateHz());
    setField(root, "lfo2Depth", SynthParameters::getLfo2Depth());
    setField(root, "lfo2ToPitch", SynthParameters::getLfo2ToPitch());
    setField(root, "lfo2ToFilter", SynthParameters::getLfo2ToFilter());
    setField(root, "lfo2ToAmp", SynthParameters::getLfo2ToAmp());
    setField(root, "glideSec", SynthParameters::getGlideSec());
    setField(root, "monoMode", SynthParameters::getMonoMode());
    setField(root, "velocityToAmp", SynthParameters::getVelocityToAmp());
    setField(root, "velocityToFilter", SynthParameters::getVelocityToFilter());
    setField(root, "masterLevel", SynthParameters::getMasterLevel());
    return root;
}

void PresetManager::applyParametersFromVar(const juce::var& root)
{
    if (!root.isObject())
        return;

    auto get = [&](const char* key) { return root.getProperty(key, {}); };

    if (get("osc1Waveform").isInt() || get("osc1Waveform").isInt64())
        SynthParameters::setOsc1Waveform(waveformFromVar(get("osc1Waveform")));
    else if (get("waveform").isInt() || get("waveform").isInt64())
        SynthParameters::setOsc1Waveform(waveformFromVar(get("waveform")));

    if (get("osc2Waveform").isInt() || get("osc2Waveform").isInt64())
        SynthParameters::setOsc2Waveform(waveformFromVar(get("osc2Waveform")));

    auto setFloat = [&](const char* key, auto setter)
    {
        const auto v = get(key);
        if (v.isDouble() || v.isInt() || v.isInt64())
            setter(static_cast<float>(v));
    };

    setFloat("osc1Level", SynthParameters::setOsc1Level);
    setFloat("osc2Level", SynthParameters::setOsc2Level);
    setFloat("subLevel", SynthParameters::setSubLevel);
    setFloat("tuneSemis", SynthParameters::setTuneSemis);
    setFloat("fineCents", SynthParameters::setFineCents);
    setFloat("noiseLevel", SynthParameters::setNoiseLevel);
    setFloat("cutoffHz", SynthParameters::setCutoffHz);
    setFloat("resonance", SynthParameters::setResonance);
    setFloat("filterEnvAmount", SynthParameters::setFilterEnvAmount);
    setFloat("filterKeyTrack", SynthParameters::setFilterKeyTrack);
    setFloat("osc2DetuneCents", SynthParameters::setOsc2DetuneCents);
    setFloat("ampAttack", SynthParameters::setAmpAttack);
    setFloat("ampDecay", SynthParameters::setAmpDecay);
    setFloat("ampSustain", SynthParameters::setAmpSustain);
    setFloat("ampRelease", SynthParameters::setAmpRelease);
    setFloat("filterAttack", SynthParameters::setFilterAttack);
    setFloat("filterDecay", SynthParameters::setFilterDecay);
    setFloat("filterSustain", SynthParameters::setFilterSustain);
    setFloat("filterRelease", SynthParameters::setFilterRelease);
    setFloat("lfoRateHz", SynthParameters::setLfoRateHz);
    setFloat("lfoDepth", SynthParameters::setLfoDepth);
    setFloat("lfo2RateHz", SynthParameters::setLfo2RateHz);
    setFloat("lfo2Depth", SynthParameters::setLfo2Depth);
    setFloat("glideSec", SynthParameters::setGlideSec);
    setFloat("velocityToAmp", SynthParameters::setVelocityToAmp);
    setFloat("velocityToFilter", SynthParameters::setVelocityToFilter);
    setFloat("masterLevel", SynthParameters::setMasterLevel);

    const auto subOct = get("subOctave");
    if (subOct.isInt() || subOct.isInt64())
    {
        const auto oct = static_cast<int>(subOct);
        SynthParameters::setSubOctave(oct == 1 ? SubOctave::Down2 : SubOctave::Down1);
    }

    if (get("lfoToPitch").isBool())
        SynthParameters::setLfoToPitch(static_cast<bool>(get("lfoToPitch")));
    if (get("lfoToFilter").isBool())
        SynthParameters::setLfoToFilter(static_cast<bool>(get("lfoToFilter")));
    if (get("lfoToAmp").isBool())
        SynthParameters::setLfoToAmp(static_cast<bool>(get("lfoToAmp")));
    if (get("lfo2ToPitch").isBool())
        SynthParameters::setLfo2ToPitch(static_cast<bool>(get("lfo2ToPitch")));
    if (get("lfo2ToFilter").isBool())
        SynthParameters::setLfo2ToFilter(static_cast<bool>(get("lfo2ToFilter")));
    if (get("lfo2ToAmp").isBool())
        SynthParameters::setLfo2ToAmp(static_cast<bool>(get("lfo2ToAmp")));
    if (get("monoMode").isBool())
        SynthParameters::setMonoMode(static_cast<bool>(get("monoMode")));
}
