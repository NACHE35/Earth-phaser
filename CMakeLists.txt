cmake_minimum_required(VERSION 3.22)

project(EarthPhaser VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.6
    GIT_SHALLOW    ON)
FetchContent_MakeAvailable(JUCE)

if(APPLE)
    set(EC_FORMATS VST3 AU Standalone)
else()
    set(EC_FORMATS VST3 Standalone)
endif()

juce_add_plugin(EarthPhaser
    COMPANY_NAME "NACHE"
    PLUGIN_MANUFACTURER_CODE Nach
    PLUGIN_CODE Epsr
    FORMATS ${EC_FORMATS}
    PRODUCT_NAME "Earth Phaser"
    BUNDLE_ID com.nache.earthphaser
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES Fx Modulation
    AU_MAIN_TYPE kAudioUnitType_Effect
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(EarthPhaser PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(EarthPhaser PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

# Police optionnelle "Thriller" : si Resources/Thriller.ttf existe, elle est integree.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Thriller.ttf")
    juce_add_binary_data(EarthPhaserAssets SOURCES Resources/Thriller.ttf)
    target_link_libraries(EarthPhaser PRIVATE EarthPhaserAssets)
    target_compile_definitions(EarthPhaser PUBLIC EARTH_HAS_THRILLER_FONT=1)
endif()

target_link_libraries(EarthPhaser
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
