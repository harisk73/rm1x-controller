cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(RM1xController VERSION 0.2.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add JUCE as a subdirectory (expected at libs/JUCE)
# If JUCE is not present, we'll fetch it
# Can override with -DJUCE_PATH=<path>
if(DEFINED JUCE_PATH)
    add_subdirectory(${JUCE_PATH} juce)
elseif(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/JUCE/CMakeLists.txt")
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)
else()
    add_subdirectory(libs/JUCE)
endif()

# Plugin/app configuration
juce_add_plugin(RM1xController
    COMPANY_NAME "Haris"
    BUNDLE_ID "com.haris.rm1xcontroller"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE HrsR
    PLUGIN_CODE Rm1c
    FORMATS VST3 Standalone
    PRODUCT_NAME "RM1x Controller"
    VST3_CATEGORIES "Tools"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon-512.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon-64.png"
)

# Generate JUCE header
juce_generate_juce_header(RM1xController)

# Source files
target_sources(RM1xController
    PRIVATE
        Source/PluginEditor.cpp
        Source/PluginProcessor.cpp
        Source/MIDIProtocol.cpp
        Source/PanelComponents.cpp
        Source/VoiceNames.cpp
)

# Include directories
target_include_directories(RM1xController
    PRIVATE
        Source
)

# Compile definitions
target_compile_definitions(RM1xController
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
)

# Generate version info header
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ProjectInfo.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/ProjectInfo.h
)

target_include_directories(RM1xController
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/generated
)

# Link JUCE modules
target_link_libraries(RM1xController
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_devices
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
