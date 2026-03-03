cmake_minimum_required(VERSION 3.22)

project(BackhouseDSP_AmpSim VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build universal binaries" FORCE)
endif()

if(NOT DEFINED JUCE_DIR)
  message(FATAL_ERROR "JUCE_DIR is not set. Configure with -DJUCE_DIR=/path/to/JUCE")
endif()

add_subdirectory(${JUCE_DIR} JUCE)

if(DEFINED AAX_SDK_PATH)
  juce_set_aax_sdk_path(${AAX_SDK_PATH})
endif()

juce_add_binary_data(BackhouseAmpSimAssets
  NAMESPACE BinaryData
  SOURCES
    "fonts/dharma-punk/Dharma Punk.ttf"
    "fonts/punkaholic/Punkaholic D.otf"
    "fonts/bhore-punk-trial/BHORE PUNK Trial.otf"
    "assets/bhp_logo.png"
)

juce_add_plugin(BackhouseAmpSim
  COMPANY_NAME "BackhouseDSP"
  BUNDLE_ID com.backhousedsp.ampsim
  ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/assets/bhp_logo.png"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  MICROPHONE_PERMISSION_ENABLED TRUE
  MICROPHONE_PERMISSION_TEXT "Backhouse Amp Sim needs microphone/input access for live guitar signal."
  PLUGIN_MANUFACTURER_CODE BHDS
  PLUGIN_CODE BHA1
  FORMATS AU VST3 AAX Standalone
  PRODUCT_NAME "Backhouse Amp Sim"
)

target_sources(BackhouseAmpSim
  PRIVATE
    src/PluginProcessor.cpp
    src/PluginProcessor.h
    src/PluginEditor.cpp
    src/PluginEditor.h
    src/BackhouseLookAndFeel.cpp
    src/BackhouseLookAndFeel.h
    dsp/AmpEngine.cpp
    dsp/AmpEngine.h
    dsp/NoiseGate.cpp
    dsp/NoiseGate.h
    dsp/CabSimulator.cpp
    dsp/CabSimulator.h
    dsp/ReverbIRProcessor.cpp
    dsp/ReverbIRProcessor.h
    dsp/TunerStub.h
    dsp/WhammyEffect.h
    dsp/WhammyEffect.cpp
)

target_include_directories(BackhouseAmpSim
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/dsp
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(BackhouseAmpSim
  PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(BackhouseAmpSim
  PRIVATE
    BackhouseAmpSimAssets
    juce::juce_audio_utils
    juce::juce_dsp
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

if(APPLE)
  foreach(_format IN ITEMS Standalone AU VST3 AAX)
    add_custom_command(TARGET BackhouseAmpSim_${_format} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/bhp_logo.icns"
        "$<TARGET_BUNDLE_DIR:BackhouseAmpSim_${_format}>/Contents/Resources/Icon.icns"
      COMMENT "Injecting BHP icon into ${_format} bundle")
  endforeach()
endif()
