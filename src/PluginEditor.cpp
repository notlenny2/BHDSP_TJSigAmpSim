#include "PluginEditor.h"

#include <array>

namespace
{
void configureKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 18);
}

void configureNameEditor(juce::TextEditor& editor)
{
    editor.setMultiLine(false);
    editor.setReturnKeyStartsNewLine(false);
    editor.setEscapeAndReturnKeysConsumed(false);
    editor.setTextToShowWhenEmpty("Profile name", juce::Colours::grey);
}

constexpr std::array<const char*, 3> pickupIds {
    "profile1PickupOutput",
    "profile2PickupOutput",
    "profile3PickupOutput"
};

constexpr std::array<const char*, 3> brightnessIds {
    "profile1Brightness",
    "profile2Brightness",
    "profile3Brightness"
};

constexpr std::array<const char*, 3> lowEndIds {
    "profile1LowEnd",
    "profile2LowEnd",
    "profile3LowEnd"
};

constexpr std::array<const char*, 3> gateIds {
    "profile1GateTrim",
    "profile2GateTrim",
    "profile3GateTrim"
};
} // namespace

BackhouseAmpSimAudioProcessorEditor::BackhouseAmpSimAudioProcessorEditor(BackhouseAmpSimAudioProcessor& p)
    : AudioProcessorEditor(&p), pluginProcessor(p)
{
    setSize(1120, 860);

    ampSelector.addItemList({ "Amp 1", "Amp 2", "Amp 3", "Amp 4" }, 1);
    addAndMakeVisible(ampSelector);

    for (auto* slider : { &inputGain, &sub, &low, &mid, &high, &presence, &output, &gateThreshold, &cabTone,
                          &profilePickupOutput, &profileBrightness, &profileLowEnd, &profileGateTrim })
    {
        configureKnob(*slider);
        addAndMakeVisible(*slider);
    }

    for (auto* editor : { &profile1NameEditor, &profile2NameEditor, &profile3NameEditor })
    {
        configureNameEditor(*editor);
        addAndMakeVisible(*editor);
    }

    addAndMakeVisible(inputBoostButton);
    addAndMakeVisible(cabEnabledButton);
    addAndMakeVisible(tunerButton);
    addAndMakeVisible(profileNeutralButton);
    addAndMakeVisible(profile1Button);
    addAndMakeVisible(profile2Button);
    addAndMakeVisible(profile3Button);
    addAndMakeVisible(exportProfilesButton);
    addAndMakeVisible(importProfilesButton);
    addAndMakeVisible(loadIRButton);
    addAndMakeVisible(clearIRButton);

    pitchLabel.setText("Pitch: -- Hz", juce::dontSendNotification);
    pitchLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(pitchLabel);

    profileEditLabel.setText("Profile Offsets (Pickup/Brightness/Low-End/Gate)", juce::dontSendNotification);
    profileEditLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(profileEditLabel);

    profileIoStatusLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(profileIoStatusLabel);

    irStatusLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(irStatusLabel);

    meterInputLabel.setText("Input", juce::dontSendNotification);
    meterOutputLabel.setText("Output", juce::dontSendNotification);
    meterInputLabel.setJustificationType(juce::Justification::centredLeft);
    meterOutputLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(meterInputLabel);
    addAndMakeVisible(meterOutputLabel);
    addAndMakeVisible(inputMeterBar);
    addAndMakeVisible(outputMeterBar);
    inputMeterBar.setPercentageDisplay(false);
    outputMeterBar.setPercentageDisplay(false);
    inputMeterBar.setColour(juce::ProgressBar::foregroundColourId, juce::Colours::lightgreen);
    outputMeterBar.setColour(juce::ProgressBar::foregroundColourId, juce::Colours::orange);

#if JucePlugin_Build_Standalone
    testBenchLabel.setText("Standalone Test Bench", juce::dontSendNotification);
    testBenchLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(testBenchLabel);

    audioDeviceLabel.setText("Audio Device", juce::dontSendNotification);
    midiInputLabel.setText("MIDI Input", juce::dontSendNotification);
    midiOutputLabel.setText("MIDI Output", juce::dontSendNotification);
    for (auto* label : { &audioDeviceLabel, &midiInputLabel, &midiOutputLabel })
    {
        label->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(*label);
    }

    addAndMakeVisible(midiInputSelector);
    addAndMakeVisible(midiOutputSelector);

    midiInputSelector.onChange = [this] {
        if (standaloneDeviceManager == nullptr)
            return;

        const int selected = midiInputSelector.getSelectedItemIndex();
        const juce::String selectedId = selected > 0 ? midiInputIds[selected - 1] : juce::String();
        for (const auto& dev : juce::MidiInput::getAvailableDevices())
            standaloneDeviceManager->setMidiInputDeviceEnabled(dev.identifier, dev.identifier == selectedId);
    };

    midiOutputSelector.onChange = [this] {
        if (standaloneDeviceManager == nullptr)
            return;

        const int selected = midiOutputSelector.getSelectedItemIndex();
        const juce::String selectedId = selected > 0 ? midiOutputIds[selected - 1] : juce::String();
        standaloneDeviceManager->setDefaultMidiOutputDevice(selectedId);
    };

    attachStandaloneDeviceManager();
#endif

    auto& apvts = pluginProcessor.getAPVTS();
    ampAttachment = std::make_unique<ComboAttachment>(apvts, "ampType", ampSelector);
    if (auto* profileParam = apvts.getParameter("guitarProfile"))
    {
        guitarProfileAttachment = std::make_unique<ChoiceAttachment>(*profileParam, [this](float) {
            refreshGuitarProfileButtons();
            bindProfileEditControls();
        }, nullptr);
        guitarProfileAttachment->sendInitialUpdate();
    }

    inputBoostAttachment = std::make_unique<ButtonAttachment>(apvts, "inputBoost", inputBoostButton);
    cabEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "cabEnabled", cabEnabledButton);
    tunerAttachment = std::make_unique<ButtonAttachment>(apvts, "tunerEnabled", tunerButton);

    inputGainAttachment = std::make_unique<SliderAttachment>(apvts, "inputGain", inputGain);
    subAttachment = std::make_unique<SliderAttachment>(apvts, "sub", sub);
    lowAttachment = std::make_unique<SliderAttachment>(apvts, "low", low);
    midAttachment = std::make_unique<SliderAttachment>(apvts, "mid", mid);
    highAttachment = std::make_unique<SliderAttachment>(apvts, "high", high);
    presenceAttachment = std::make_unique<SliderAttachment>(apvts, "presence", presence);
    outputAttachment = std::make_unique<SliderAttachment>(apvts, "output", output);
    gateAttachment = std::make_unique<SliderAttachment>(apvts, "gateThresh", gateThreshold);
    cabToneAttachment = std::make_unique<SliderAttachment>(apvts, "cabTone", cabTone);

    profileNeutralButton.onClick = [this] { setGuitarProfileFromButton(0); };
    profile1Button.onClick = [this] { setGuitarProfileFromButton(1); };
    profile2Button.onClick = [this] { setGuitarProfileFromButton(2); };
    profile3Button.onClick = [this] { setGuitarProfileFromButton(3); };
    exportProfilesButton.onClick = [this] {
        profileChooser = std::make_unique<juce::FileChooser>("Export guitar profiles",
                                                             juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                                                                 .getChildFile("BackhouseDSP_GuitarProfiles.json"),
                                                             "*.json");
        profileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                                    [this](const juce::FileChooser& chooser) {
                                        auto outputFile = chooser.getResult();
                                        if (outputFile == juce::File())
                                            return;

                                        if (!outputFile.hasFileExtension("json"))
                                            outputFile = outputFile.withFileExtension(".json");

                                        const bool ok = pluginProcessor.exportProfilesToJson(outputFile);
                                        refreshProfileIoStatusLabel(ok ? "Profiles exported: " + outputFile.getFileName()
                                                                       : "Profile export failed");
                                    });
    };
    importProfilesButton.onClick = [this] {
        profileChooser = std::make_unique<juce::FileChooser>("Import guitar profiles", juce::File(), "*.json");
        profileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                    [this](const juce::FileChooser& chooser) {
                                        const auto inputFile = chooser.getResult();
                                        if (!inputFile.existsAsFile())
                                            return;

                                        const bool ok = pluginProcessor.importProfilesFromJson(inputFile);
                                        refreshProfileNameEditors();
                                        refreshGuitarProfileButtons();
                                        bindProfileEditControls();
                                        refreshProfileIoStatusLabel(ok ? "Profiles imported: " + inputFile.getFileName()
                                                                       : "Profile import failed");
                                    });
    };
    loadIRButton.onClick = [this] {
        irChooser = std::make_unique<juce::FileChooser>("Load cabinet IR", juce::File(), "*.wav;*.aiff;*.aif");
        irChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                               [this](const juce::FileChooser& chooser) {
                                   const auto selected = chooser.getResult();
                                   if (selected.existsAsFile())
                                       pluginProcessor.loadUserIR(selected);
                                   refreshIRStatusLabel();
                               });
    };
    clearIRButton.onClick = [this] {
        pluginProcessor.clearUserIR();
        refreshIRStatusLabel();
    };

    profile1NameEditor.onReturnKey = [this] { commitProfileName(1, profile1NameEditor.getText()); };
    profile2NameEditor.onReturnKey = [this] { commitProfileName(2, profile2NameEditor.getText()); };
    profile3NameEditor.onReturnKey = [this] { commitProfileName(3, profile3NameEditor.getText()); };

    profile1NameEditor.onFocusLost = [this] { commitProfileName(1, profile1NameEditor.getText()); };
    profile2NameEditor.onFocusLost = [this] { commitProfileName(2, profile2NameEditor.getText()); };
    profile3NameEditor.onFocusLost = [this] { commitProfileName(3, profile3NameEditor.getText()); };

    refreshProfileNameEditors();
    refreshGuitarProfileButtons();
    bindProfileEditControls();
    refreshIRStatusLabel();
    refreshProfileIoStatusLabel("Profile JSON: ready");

    startTimerHz(10);
}

void BackhouseAmpSimAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::whitesmoke);
    g.setFont(22.0f);
    g.drawText("Backhouse Amp Sim (Profile-Ready Scaffold)", getLocalBounds().removeFromTop(36), juce::Justification::centred);
}

void BackhouseAmpSimAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(12);
    area.removeFromTop(34);

    auto topRow = area.removeFromTop(32);
    ampSelector.setBounds(topRow.removeFromLeft(180));
    inputBoostButton.setBounds(topRow.removeFromLeft(120));
    cabEnabledButton.setBounds(topRow.removeFromLeft(120));
    tunerButton.setBounds(topRow.removeFromLeft(90));
    loadIRButton.setBounds(topRow.removeFromLeft(90).reduced(2));
    clearIRButton.setBounds(topRow.removeFromLeft(90).reduced(2));
    pitchLabel.setBounds(topRow.removeFromLeft(150));

    area.removeFromTop(10);

    auto irRow = area.removeFromTop(24);
    irStatusLabel.setBounds(irRow.removeFromLeft(460));
    meterInputLabel.setBounds(irRow.removeFromLeft(45));
    inputMeterBar.setBounds(irRow.removeFromLeft(145).reduced(2, 4));
    irRow.removeFromLeft(12);
    meterOutputLabel.setBounds(irRow.removeFromLeft(52));
    outputMeterBar.setBounds(irRow.removeFromLeft(145).reduced(2, 4));

    auto profileButtonRow = area.removeFromTop(34);
    profileNeutralButton.setBounds(profileButtonRow.removeFromLeft(120).reduced(2));
    profile1Button.setBounds(profileButtonRow.removeFromLeft(120).reduced(2));
    profile2Button.setBounds(profileButtonRow.removeFromLeft(120).reduced(2));
    profile3Button.setBounds(profileButtonRow.removeFromLeft(120).reduced(2));
    importProfilesButton.setBounds(profileButtonRow.removeFromLeft(130).reduced(2));
    exportProfilesButton.setBounds(profileButtonRow.removeFromLeft(130).reduced(2));

    auto profileNameRow = area.removeFromTop(32);
    profileNameRow.removeFromLeft(120);
    profile1NameEditor.setBounds(profileNameRow.removeFromLeft(120).reduced(2));
    profile2NameEditor.setBounds(profileNameRow.removeFromLeft(120).reduced(2));
    profile3NameEditor.setBounds(profileNameRow.removeFromLeft(120).reduced(2));

    area.removeFromTop(8);

    auto editLabelRow = area.removeFromTop(24);
    profileEditLabel.setBounds(editLabelRow.removeFromLeft(380));
    profileIoStatusLabel.setBounds(editLabelRow.removeFromLeft(480));

    auto profileKnobs = area.removeFromTop(145);
    const int profileColumns = 4;
    const int profileKnobWidth = profileKnobs.getWidth() / profileColumns;
    profilePickupOutput.setBounds(profileKnobs.removeFromLeft(profileKnobWidth).reduced(4));
    profileBrightness.setBounds(profileKnobs.removeFromLeft(profileKnobWidth).reduced(4));
    profileLowEnd.setBounds(profileKnobs.removeFromLeft(profileKnobWidth).reduced(4));
    profileGateTrim.setBounds(profileKnobs.removeFromLeft(profileKnobWidth).reduced(4));

    area.removeFromTop(8);

    auto knobs = area.removeFromTop(380);
    const int columns = 9;
    const int width = knobs.getWidth() / columns;

    inputGain.setBounds(knobs.removeFromLeft(width).reduced(4));
    sub.setBounds(knobs.removeFromLeft(width).reduced(4));
    low.setBounds(knobs.removeFromLeft(width).reduced(4));
    mid.setBounds(knobs.removeFromLeft(width).reduced(4));
    high.setBounds(knobs.removeFromLeft(width).reduced(4));
    presence.setBounds(knobs.removeFromLeft(width).reduced(4));
    output.setBounds(knobs.removeFromLeft(width).reduced(4));
    gateThreshold.setBounds(knobs.removeFromLeft(width).reduced(4));
    cabTone.setBounds(knobs.removeFromLeft(width).reduced(4));

#if JucePlugin_Build_Standalone
    area.removeFromTop(6);
    auto benchArea = area.removeFromTop(220);
    benchArea.reduce(4, 4);

    testBenchLabel.setBounds(benchArea.removeFromTop(26));

    auto midiRow = benchArea.removeFromTop(32);
    midiInputLabel.setBounds(midiRow.removeFromLeft(88));
    midiInputSelector.setBounds(midiRow.removeFromLeft(260).reduced(2));
    midiRow.removeFromLeft(16);
    midiOutputLabel.setBounds(midiRow.removeFromLeft(95));
    midiOutputSelector.setBounds(midiRow.removeFromLeft(260).reduced(2));

    benchArea.removeFromTop(6);
    audioDeviceLabel.setBounds(benchArea.removeFromTop(22));
    if (audioDeviceSelector != nullptr)
        audioDeviceSelector->setBounds(benchArea.reduced(2));
#endif
}

void BackhouseAmpSimAudioProcessorEditor::timerCallback()
{
    const auto hz = pluginProcessor.getDetectedPitchHz();
    if (hz > 10.0f)
        pitchLabel.setText("Pitch: " + juce::String(hz, 1) + " Hz", juce::dontSendNotification);
    else
        pitchLabel.setText("Pitch: -- Hz", juce::dontSendNotification);

    inputMeterValue = pluginProcessor.getInputMeterLevel();
    outputMeterValue = pluginProcessor.getOutputMeterLevel();

#if JucePlugin_Build_Standalone
    if (standaloneDeviceManager == nullptr)
        attachStandaloneDeviceManager();
#endif
}

void BackhouseAmpSimAudioProcessorEditor::setGuitarProfileFromButton(int profileIndex)
{
    auto& apvts = pluginProcessor.getAPVTS();
    if (auto* param = apvts.getParameter("guitarProfile"))
    {
        const auto normalisable = juce::NormalisableRange<float>(0.0f, 3.0f, 1.0f);
        const float clamped = juce::jlimit(0, 3, profileIndex);
        const float as01 = normalisable.convertTo0to1(clamped);
        param->setValueNotifyingHost(as01);
    }
}

void BackhouseAmpSimAudioProcessorEditor::refreshGuitarProfileButtons()
{
    auto& apvts = pluginProcessor.getAPVTS();
    const int current = static_cast<int>(apvts.getRawParameterValue("guitarProfile")->load());

    profileNeutralButton.setButtonText("Neutral");
    profile1Button.setButtonText(pluginProcessor.getProfileName(1));
    profile2Button.setButtonText(pluginProcessor.getProfileName(2));
    profile3Button.setButtonText(pluginProcessor.getProfileName(3));

    profileNeutralButton.setColour(juce::TextButton::buttonColourId, current == 0 ? juce::Colours::darkorange : juce::Colours::dimgrey);
    profile1Button.setColour(juce::TextButton::buttonColourId, current == 1 ? juce::Colours::darkorange : juce::Colours::dimgrey);
    profile2Button.setColour(juce::TextButton::buttonColourId, current == 2 ? juce::Colours::darkorange : juce::Colours::dimgrey);
    profile3Button.setColour(juce::TextButton::buttonColourId, current == 3 ? juce::Colours::darkorange : juce::Colours::dimgrey);
}

void BackhouseAmpSimAudioProcessorEditor::refreshProfileNameEditors()
{
    profile1NameEditor.setText(pluginProcessor.getProfileName(1), juce::dontSendNotification);
    profile2NameEditor.setText(pluginProcessor.getProfileName(2), juce::dontSendNotification);
    profile3NameEditor.setText(pluginProcessor.getProfileName(3), juce::dontSendNotification);
}

void BackhouseAmpSimAudioProcessorEditor::bindProfileEditControls()
{
    auto& apvts = pluginProcessor.getAPVTS();
    const int selected = static_cast<int>(apvts.getRawParameterValue("guitarProfile")->load());

    profilePickupAttachment.reset();
    profileBrightnessAttachment.reset();
    profileLowEndAttachment.reset();
    profileGateAttachment.reset();

    const bool enabled = selected >= 1 && selected <= 3;
    for (auto* slider : { &profilePickupOutput, &profileBrightness, &profileLowEnd, &profileGateTrim })
        slider->setEnabled(enabled);

    if (!enabled)
        return;

    const size_t idx = static_cast<size_t>(selected - 1);
    profilePickupAttachment = std::make_unique<SliderAttachment>(apvts, pickupIds[idx], profilePickupOutput);
    profileBrightnessAttachment = std::make_unique<SliderAttachment>(apvts, brightnessIds[idx], profileBrightness);
    profileLowEndAttachment = std::make_unique<SliderAttachment>(apvts, lowEndIds[idx], profileLowEnd);
    profileGateAttachment = std::make_unique<SliderAttachment>(apvts, gateIds[idx], profileGateTrim);
}

void BackhouseAmpSimAudioProcessorEditor::commitProfileName(int profileIndex, const juce::String& text)
{
    pluginProcessor.setProfileName(profileIndex, text);
    refreshProfileNameEditors();
    refreshGuitarProfileButtons();
}

void BackhouseAmpSimAudioProcessorEditor::refreshIRStatusLabel()
{
    if (pluginProcessor.hasUserIR())
    {
        irStatusLabel.setText("Cab IR: " + pluginProcessor.getUserIRName(), juce::dontSendNotification);
        clearIRButton.setEnabled(true);
    }
    else
    {
        irStatusLabel.setText("Cab IR: Default per-amp cab", juce::dontSendNotification);
        clearIRButton.setEnabled(false);
    }
}

void BackhouseAmpSimAudioProcessorEditor::refreshProfileIoStatusLabel(const juce::String& message)
{
    profileIoStatusLabel.setText(message, juce::dontSendNotification);
}

#if JucePlugin_Build_Standalone
void BackhouseAmpSimAudioProcessorEditor::attachStandaloneDeviceManager()
{
    if (standaloneDeviceManager != nullptr)
        return;

    juce::StandaloneFilterWindow* standaloneWindow = nullptr;
    for (auto* current = getParentComponent(); current != nullptr; current = current->getParentComponent())
    {
        if (auto* candidate = dynamic_cast<juce::StandaloneFilterWindow*>(current))
        {
            standaloneWindow = candidate;
            break;
        }
    }

    if (standaloneWindow == nullptr)
        standaloneWindow = dynamic_cast<juce::StandaloneFilterWindow*>(getTopLevelComponent());

    if (standaloneWindow == nullptr)
        return;

    standaloneDeviceManager = &standaloneWindow->getDeviceManager();
    audioDeviceSelector = std::make_unique<juce::AudioDeviceSelectorComponent>(*standaloneDeviceManager,
                                                                                0,
                                                                                2,
                                                                                0,
                                                                                2,
                                                                                false,
                                                                                false,
                                                                                true,
                                                                                false);
    addAndMakeVisible(*audioDeviceSelector);
    refreshStandaloneMidiDeviceLists();
    resized();
}

void BackhouseAmpSimAudioProcessorEditor::refreshStandaloneMidiDeviceLists()
{
    if (standaloneDeviceManager == nullptr)
        return;

    midiInputIds.clear();
    midiOutputIds.clear();

    midiInputSelector.clear(juce::dontSendNotification);
    midiOutputSelector.clear(juce::dontSendNotification);

    midiInputSelector.addItem("None", 1);
    midiOutputSelector.addItem("None", 1);

    int itemId = 2;
    for (const auto& dev : juce::MidiInput::getAvailableDevices())
    {
        midiInputIds.add(dev.identifier);
        midiInputSelector.addItem(dev.name, itemId++);
    }

    itemId = 2;
    for (const auto& dev : juce::MidiOutput::getAvailableDevices())
    {
        midiOutputIds.add(dev.identifier);
        midiOutputSelector.addItem(dev.name, itemId++);
    }

    int selectedInput = 1;
    for (int i = 0; i < midiInputIds.size(); ++i)
    {
        if (standaloneDeviceManager->isMidiInputDeviceEnabled(midiInputIds[i]))
        {
            selectedInput = i + 2;
            break;
        }
    }
    midiInputSelector.setSelectedId(selectedInput, juce::dontSendNotification);

    int selectedOutput = 1;
    const auto currentOutput = standaloneDeviceManager->getDefaultMidiOutputIdentifier();
    for (int i = 0; i < midiOutputIds.size(); ++i)
    {
        if (midiOutputIds[i] == currentOutput)
        {
            selectedOutput = i + 2;
            break;
        }
    }
    midiOutputSelector.setSelectedId(selectedOutput, juce::dontSendNotification);
}
#endif
