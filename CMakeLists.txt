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

get_target_property(_analog_juce_gen_dir AnalogSynth JUCE_GENERATED_SOURCES_DIRECTORY)
set(_analog_app_icon_png "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/app_icon.png")
set(_analog_app_icon_ico "${_analog_juce_gen_dir}/icon.ico")
set(_analog_app_icon_rc "${_analog_juce_gen_dir}/AppPrimaryIcon.rc")

if(WIN32)
    file(MAKE_DIRECTORY "${_analog_juce_gen_dir}")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icons/AppPrimaryIcon.rc"
                   "${_analog_app_icon_rc}" COPYONLY)

    add_custom_command(
        OUTPUT "${_analog_app_icon_ico}"
        COMMAND juce::juceaide winicon "${_analog_app_icon_ico}" "${_analog_app_icon_png}"
        DEPENDS "${_analog_app_icon_png}" juce::juceaide
        COMMENT "Generating Windows application icon"
        VERBATIM)

    add_custom_target(AnalogSynth_app_icon DEPENDS "${_analog_app_icon_ico}")
    add_dependencies(AnalogSynth AnalogSynth_app_icon)

    target_sources(AnalogSynth PRIVATE "${_analog_app_icon_rc}")
    set_source_files_properties("${_analog_app_icon_rc}"
        PROPERTIES OBJECT_DEPENDS "${_analog_app_icon_ico}")
endif()

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
