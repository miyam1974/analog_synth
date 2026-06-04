# NEXUS OSC

**Languages:** [日本語](README.md) | [English](README.en.md)

[License: MIT](https://opensource.org/licenses/MIT)

Analog-style synthesizer for Windows (Standalone). Play via USB MIDI keyboard or the built-in on-screen keyboard.

## Screenshots

### Main UI

![NEXUS OSC — main UI (1080×680)](docs/images/nexus-osc-ui.png)

Window size: 1080×680

Top: presets, MONO, ALL OFF, MASTER

Center: synth modules

Bottom: on-screen keyboard and MIDI settings

### While playing

![NEXUS OSC — playing](docs/images/playing.png)

Pressed keys light up on the on-screen keyboard

Playhead dots show envelope position on the EG graphs

Playhead dots on the FILTER and AMPLIFIER EG graphs

---

## Features

| Category | Description |
| -------- | ----------- |
| Oscillators | OSC1 / OSC2 (4 waveforms), Sub (-1 / -2 oct), TUNE / FINE / DET2 |
| Mixer | OSC1 / OSC2 / SUB / NOISE levels, Glide, V-A / V-F |
| Filter | Low-pass (CUT / RES / ENV / KEY), Filter EG graph |
| Amp | Amp ADSR graph |
| LFO | LFO1 / LFO2 (RATE / DEPTH, Pitch / Filter / Amp, RATE-synced LEDs) |
| Performance | 16 voices, MONO, ALL OFF (panic / all notes off) |
| Presets | 4 built-in presets (INIT / PAD / BASS / LEAD), JSON SAVE / LOAD |
| Help | Japanese tooltips on hover (SYSTEM footer) |

**Not implemented (planned)**: VST3 / CLAP, FX (chorus / delay / reverb), pitch bend / mod wheel,
ASIO enablement, SmoothedValue / effective cutoff Hz display, arpeggiator / MPE / voice-count UI,
audio device settings UI.
See [ARCHITECTURE.en.md](ARCHITECTURE.en.md) — “Current limitations and future work”.

---

## Requirements

- **OS**: Windows 10 / 11 (64-bit)
- **Build**: [Visual Studio 2019 or later](https://visualstudio.microsoft.com/) (“Desktop development with C++” workload)
- **CMake**: 3.22 or newer
- **Git**: fetches JUCE 8.0.6 via FetchContent
- **Runtime**: MIDI input (optional), audio output (WASAPI by default)

---

## Build

From the cloned repository directory:

```powershell
cmake -S . -B build -G "Visual Studio 16 2019" -A x64
cmake --build build --config Release
```

For Visual Studio 2022, use `-G "Visual Studio 17 2022"` instead.

### Output

```text
build/AnalogSynth_artefacts/Release/AnalogSynth.exe
```

Quit the running app before rebuilding to avoid linker errors.

MSVC builds use `/utf-8` so Japanese help text in `HelpStrings.h` stays UTF-8.

---

## Usage

1. Connect a USB MIDI keyboard (optional)
2. Run `AnalogSynth.exe`
3. Under **SYSTEM**, choose **MIDI IN** (`All Inputs` merges all devices)
4. Tweak modules, then play from the on-screen keyboard or MIDI
5. Hover controls to see Japanese help in the SYSTEM footer

### User preset location

```text
%APPDATA%/NEXUS OSC/Presets/*.json
```

---

## Project layout

```text
analog_synth/
├── CMakeLists.txt
├── README.md
├── README.en.md
├── ARCHITECTURE.md
├── ARCHITECTURE.en.md
├── LICENSE
├── docs/
│   └── images/
│       ├── nexus-osc-ui.png  # main UI (README)
│       └── playing.png       # playing (README)
└── Source/
    ├── Main.cpp
    ├── SynthEditor.*
    ├── SynthVoice.*
    ├── SynthSound.*
    ├── SynthParameters.h
    ├── AdsrEnvelope.h
    ├── GlobalLfo.h
    ├── EnvelopePlayhead.*
    ├── PresetManager.*
    ├── Waveform.h
    ├── HelpStrings.h
    └── UI/
        ├── AdsrDisplay.*
        ├── LfoRateLed.*
        ├── ModulePanel.*
        ├── WaveformButton.*
        ├── FuturisticLookAndFeel.*
        └── SynthTheme.h
```

---

## Documentation

| File | Contents |
| ---- | -------- |
| [README.md](README.md) / [README.en.md](README.en.md) | Overview, build, usage |
| [ARCHITECTURE.md](ARCHITECTURE.md) / [ARCHITECTURE.en.md](ARCHITECTURE.en.md) | Design (layers, signal flow, WASAPI/ASIO, threads, UI) |

For development, see **[ARCHITECTURE.en.md](ARCHITECTURE.en.md)**.

---

## Dependencies

- [JUCE](https://github.com/juce-framework/JUCE) **8.0.6** (CMake FetchContent)
- Linked modules: `juce_audio_utils`, `juce_dsp`

JUCE use and distribution follow the [JUCE license](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md)
(separate from this repo’s MIT license).

---

## License

Source code in this repository is under the **[MIT License](LICENSE)**.

---

## Name

In **NEXUS OSC**, “OSC” means **oscillator**.
This project is not affiliated with reFX Nexus or other products using similar names.
