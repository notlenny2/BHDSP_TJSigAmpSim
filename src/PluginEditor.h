#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>

#if JucePlugin_Build_Standalone
 #include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

#include "PluginProcessor.h"

class BackhouseAmpSimAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                  private juce::Timer
{
public:
    explicit BackhouseAmpSimAudioProcessorEditor(BackhouseAmpSimAudioProcessor&);
    ~BackhouseAmpSimAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ChoiceAttachment = juce::ParameterAttachment;

    void timerCallback() override;
    void setGuitarProfileFromButton(int profileIndex);
    void refreshGuitarProfileButtons();
    void refreshProfileNameEditors();
    void bindProfileEditControls();
    void commitProfileName(int profileIndex, const juce::String& text);
    void refreshIRStatusLabel();
    void refreshProfileIoStatusLabel(const juce::String& message);
#if JucePlugin_Build_Standalone
    void attachStandaloneDeviceManager();
    void refreshStandaloneMidiDeviceLists();
#endif

    BackhouseAmpSimAudioProcessor& pluginProcessor;

    juce::ComboBox ampSelector;
    juce::ToggleButton inputBoostButton { "Input Boost" };
    juce::ToggleButton cabEnabledButton { "Cab Enabled" };
    juce::ToggleButton tunerButton { "Tuner" };

    juce::TextButton profileNeutralButton { "Neutral" };
    juce::TextButton profile1Button { "Guitar 1" };
    juce::TextButton profile2Button { "Guitar 2" };
    juce::TextButton profile3Button { "Guitar 3" };
    juce::TextButton exportProfilesButton { "Export Profiles" };
    juce::TextButton importProfilesButton { "Import Profiles" };
    juce::TextButton loadIRButton { "Load IR" };
    juce::TextButton clearIRButton { "Clear IR" };

    juce::TextEditor profile1NameEditor;
    juce::TextEditor profile2NameEditor;
    juce::TextEditor profile3NameEditor;

    juce::Slider inputGain;
    juce::Slider sub;
    juce::Slider low;
    juce::Slider mid;
    juce::Slider high;
    juce::Slider presence;
    juce::Slider output;
    juce::Slider gateThreshold;
    juce::Slider cabTone;

    juce::Slider profilePickupOutput;
    juce::Slider profileBrightness;
    juce::Slider profileLowEnd;
    juce::Slider profileGateTrim;

    juce::Label pitchLabel;
    juce::Label profileEditLabel;
    juce::Label profileIoStatusLabel;
    juce::Label irStatusLabel;
    juce::Label meterInputLabel;
    juce::Label meterOutputLabel;
    double inputMeterValue = 0.0;
    double outputMeterValue = 0.0;
    juce::ProgressBar inputMeterBar { inputMeterValue };
    juce::ProgressBar outputMeterBar { outputMeterValue };

#if JucePlugin_Build_Standalone
    juce::Label testBenchLabel;
    juce::Label audioDeviceLabel;
    juce::Label midiInputLabel;
    juce::Label midiOutputLabel;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioDeviceSelector;
    juce::ComboBox midiInputSelector;
    juce::ComboBox midiOutputSelector;
    juce::AudioDeviceManager* standaloneDeviceManager = nullptr;
    juce::StringArray midiInputIds;
    juce::StringArray midiOutputIds;
#endif

    std::unique_ptr<juce::FileChooser> irChooser;
    std::unique_ptr<juce::FileChooser> profileChooser;

    std::unique_ptr<ComboAttachment> ampAttachment;
    std::unique_ptr<ChoiceAttachment> guitarProfileAttachment;
    std::unique_ptr<ButtonAttachment> inputBoostAttachment;
    std::unique_ptr<ButtonAttachment> cabEnabledAttachment;
    std::unique_ptr<ButtonAttachment> tunerAttachment;

    std::unique_ptr<SliderAttachment> inputGainAttachment;
    std::unique_ptr<SliderAttachment> subAttachment;
    std::unique_ptr<SliderAttachment> lowAttachment;
    std::unique_ptr<SliderAttachment> midAttachment;
    std::unique_ptr<SliderAttachment> highAttachment;
    std::unique_ptr<SliderAttachment> presenceAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<SliderAttachment> gateAttachment;
    std::unique_ptr<SliderAttachment> cabToneAttachment;

    std::unique_ptr<SliderAttachment> profilePickupAttachment;
    std::unique_ptr<SliderAttachment> profileBrightnessAttachment;
    std::unique_ptr<SliderAttachment> profileLowEndAttachment;
    std::unique_ptr<SliderAttachment> profileGateAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BackhouseAmpSimAudioProcessorEditor)
};
