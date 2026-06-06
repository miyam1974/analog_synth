cmake_minimum_required(VERSION 3.22)

# Canonical app version — also used by JUCE_APPLICATION_VERSION_STRING and getApplicationVersion().
project(AnalogSynth VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.6
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_gui_app(AnalogSynth
    COMPANY_NAME "Analog Synth"
    PRODUCT_NAME "AnalogSynth"
    BUNDLE_ID com.analogsynth.app
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
)

target_sources(AnalogSynth
    PRIVATE
        Source/Main.cpp
        Source/AppState.cpp
        Source/SynthEditor.cpp
        Source/PresetManager.cpp
        Source/SynthVoice.cpp
        Source/EnvelopePlayhead.cpp
        Source/SynthSound.cpp
        Source/UI/AdsrDisplay.cpp
        Source/UI/LfoRateLed.cpp
        Source/UI/FuturisticLookAndFeel.cpp
        Source/UI/ModulePanel.cpp
        Source/UI/WaveformButton.cpp
)

target_compile_definitions(AnalogSynth
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:AnalogSynth,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="${PROJECT_VERSION}"
)

if(MSVC)
    target_compile_options(AnalogSynth PRIVATE /utf-8)
endif()

target_link_libraries(AnalogSynth
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
