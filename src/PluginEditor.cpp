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
    setSize(1120, 900);

    ampSelector.addItemList({ "Amp 1", "Amp 2", "Amp 3", "Amp 4" }, 1);
    addAndMakeVisible(ampSelector);

    for (auto* slider : { &inputGain, &sub, &low, &mid, &high, &presence, &output, &gateThreshold, &cabTone,
                          &profilePickupOutput, &profileBrightness, &profileLowEnd, &profileGateTrim })
    {
        configureKnob(*slider);
        addAndMakeVisible(*slider);
    }

    inputGain.setName("Input Gain");
    sub.setName("Sub");
    low.setName("Low");
    mid.setName("Mid");
    high.setName("High");
    presence.setName("Presence");
    output.setName("Output");
    gateThreshold.setName("Gate");
    cabTone.setName("Cab Tone");
    profilePickupOutput.setName("Pickup Out");
    profileBrightness.setName("Brightness");
    profileLowEnd.setName("Low End");
    profileGateTrim.setName("Gate Trim");

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

    midiInputLabel.setText("MIDI Input", juce::dontSendNotification);
    midiOutputLabel.setText("MIDI Output", juce::dontSendNotification);
    diStatusLabel.setJustificationType(juce::Justification::centredLeft);

    for (auto* label : { &midiInputLabel, &midiOutputLabel, &diStatusLabel, &diLoopLabel })
    {
        label->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(*label);
    }

    addAndMakeVisible(audioSettingsButton);
    addAndMakeVisible(midiInputSelector);
    addAndMakeVisible(midiOutputSelector);
    addAndMakeVisible(loadDITestButton);
    addAndMakeVisible(clearDITestButton);
    addAndMakeVisible(enableDITestButton);
    addAndMakeVisible(playDITestButton);
    addAndMakeVisible(loopDITestButton);

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

    audioSettingsButton.onClick = [this] {
        if (standaloneDeviceManager == nullptr)
            attachStandaloneDeviceManager();

        if (auto* window = dynamic_cast<juce::StandaloneFilterWindow*>(getTopLevelComponent()))
            window->getPluginHolder()->showAudioSettingsDialog();
    };

    loadDITestButton.onClick = [this] {
        diChooser = std::make_unique<juce::FileChooser>("Load raw DI guitar file", juce::File(), "*.wav;*.aiff;*.aif;*.mp3");
        diChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                               [this](const juce::FileChooser& chooser) {
                                   const auto file = chooser.getResult();
                                   if (!file.existsAsFile())
                                       return;

                                   pluginProcessor.loadTestDIFile(file);
                                   updateDITimelineCache();
                                   refreshTestDISection();
                               });
    };

    clearDITestButton.onClick = [this] {
        pluginProcessor.clearTestDIFile();
        updateDITimelineCache();
        refreshTestDISection();
    };

    enableDITestButton.onClick = [this] { pluginProcessor.setTestDIEnabled(enableDITestButton.getToggleState()); };
    playDITestButton.onClick = [this] { pluginProcessor.setTestDIPlaying(playDITestButton.getToggleState()); };
    loopDITestButton.onClick = [this] { pluginProcessor.setTestDILoopEnabled(loopDITestButton.getToggleState()); };

    attachStandaloneDeviceManager();
    updateDITimelineCache();
    refreshTestDISection();
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

void BackhouseAmpSimAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    g.setColour(juce::Colours::whitesmoke);
    g.setFont(12.0f);

    auto drawKnobLabel = [&g](const juce::Slider& slider) {
        auto bounds = slider.getBounds();
        g.drawFittedText(slider.getName(), bounds.getX(), bounds.getY() - 16, bounds.getWidth(), 14, juce::Justification::centred, 1);
    };

    for (const auto* slider : { &profilePickupOutput, &profileBrightness, &profileLowEnd, &profileGateTrim,
                                &inputGain, &sub, &low, &mid, &high, &presence, &output, &gateThreshold, &cabTone })
        drawKnobLabel(*slider);

   #if JucePlugin_Build_Standalone
    drawDITimeline(g);
   #endif
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

    area.removeFromTop(4);

#if JucePlugin_Build_Standalone
    const int minBenchHeight = 110;
#else
    const int minBenchHeight = 0;
#endif

    const int availableHeight = area.getHeight();
    int profileKnobHeight = juce::jlimit(100, 130, availableHeight / 4);
    int mainKnobHeight = juce::jlimit(130, 210, availableHeight / 3);
    if (profileKnobHeight + mainKnobHeight + minBenchHeight + 8 > availableHeight)
        mainKnobHeight = juce::jmax(145, availableHeight - profileKnobHeight - minBenchHeight - 8);

    auto profileKnobs = area.removeFromTop(profileKnobHeight);
    const int profileColumns = 4;
    const int profileKnobWidth = profileKnobs.getWidth() / profileColumns;
    profilePickupOutput.setBounds(profileKnobs.removeFromLeft(profileKnobWidth).reduced(4));
    profileBrightness.setBounds(profileKnobs.removeFromLeft(profileKnobWidth).reduced(4));
    profileLowEnd.setBounds(profileKnobs.removeFromLeft(profileKnobWidth).reduced(4));
    profileGateTrim.setBounds(profileKnobs.removeFromLeft(profileKnobWidth).reduced(4));

    area.removeFromTop(8);

    auto knobs = area.removeFromTop(mainKnobHeight);
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
    area.removeFromTop(4);
    auto benchArea = area;
    benchArea.reduce(4, 4);

    testBenchLabel.setBounds(benchArea.removeFromTop(24));

    auto topBenchRow = benchArea.removeFromTop(30);
    audioSettingsButton.setBounds(topBenchRow.removeFromLeft(190).reduced(2));
    topBenchRow.removeFromLeft(8);
    midiInputLabel.setBounds(topBenchRow.removeFromLeft(72));
    midiInputSelector.setBounds(topBenchRow.removeFromLeft(240).reduced(2));
    topBenchRow.removeFromLeft(8);
    midiOutputLabel.setBounds(topBenchRow.removeFromLeft(80));
    midiOutputSelector.setBounds(topBenchRow.removeFromLeft(240).reduced(2));

    auto diControlRow = benchArea.removeFromTop(30);
    loadDITestButton.setBounds(diControlRow.removeFromLeft(90).reduced(2));
    clearDITestButton.setBounds(diControlRow.removeFromLeft(90).reduced(2));
    diControlRow.removeFromLeft(8);
    enableDITestButton.setBounds(diControlRow.removeFromLeft(120));
    playDITestButton.setBounds(diControlRow.removeFromLeft(80));
    loopDITestButton.setBounds(diControlRow.removeFromLeft(80));

    benchArea.removeFromTop(2);
    diStatusLabel.setBounds(benchArea.removeFromTop(20));
    diTimelineArea = benchArea.removeFromTop(138);
    diLoopLabel.setBounds(benchArea.removeFromTop(20));
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

    updateDITimelineCache();
    refreshTestDISection();
#endif
}

void BackhouseAmpSimAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
#if JucePlugin_Build_Standalone
    if (!diTimelineArea.contains(e.getPosition()) || !pluginProcessor.hasTestDIFile())
        return;

    const int startX = timelineXFromTime(pluginProcessor.getTestDILoopStartSeconds());
    const int endX = timelineXFromTime(pluginProcessor.getTestDILoopEndSeconds());

    if (std::abs(e.x - startX) <= 6)
        diTimelineDragMode = DITimelineDragMode::loopStart;
    else if (std::abs(e.x - endX) <= 6)
        diTimelineDragMode = DITimelineDragMode::loopEnd;
    else
        diTimelineDragMode = DITimelineDragMode::playhead;

    if (diTimelineDragMode == DITimelineDragMode::playhead)
        pluginProcessor.setTestDIPositionSeconds(timelineTimeFromX(e.x));
#endif
}

void BackhouseAmpSimAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e)
{
#if JucePlugin_Build_Standalone
    if (diTimelineDragMode == DITimelineDragMode::none || !pluginProcessor.hasTestDIFile())
        return;

    const double draggedTime = timelineTimeFromX(e.x);
    const double loopStart = pluginProcessor.getTestDILoopStartSeconds();
    const double loopEnd = pluginProcessor.getTestDILoopEndSeconds();

    if (diTimelineDragMode == DITimelineDragMode::playhead)
    {
        pluginProcessor.setTestDIPositionSeconds(draggedTime);
    }
    else if (diTimelineDragMode == DITimelineDragMode::loopStart)
    {
        pluginProcessor.setTestDILoopRangeSeconds(juce::jmin(draggedTime, loopEnd - 0.01), loopEnd);
    }
    else if (diTimelineDragMode == DITimelineDragMode::loopEnd)
    {
        pluginProcessor.setTestDILoopRangeSeconds(loopStart, juce::jmax(draggedTime, loopStart + 0.01));
    }

    refreshTestDISection();
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
    refreshStandaloneMidiDeviceLists();
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

void BackhouseAmpSimAudioProcessorEditor::updateDITimelineCache()
{
    const int points = juce::jmax(256, diTimelineArea.getWidth());
    diWavePeaks = pluginProcessor.getTestDIWaveformPeaks(points);
}

void BackhouseAmpSimAudioProcessorEditor::refreshTestDISection()
{
    const bool hasFile = pluginProcessor.hasTestDIFile();
    const double duration = pluginProcessor.getTestDIDurationSeconds();

    enableDITestButton.setToggleState(pluginProcessor.isTestDIEnabled(), juce::dontSendNotification);
    playDITestButton.setToggleState(pluginProcessor.isTestDIPlaying(), juce::dontSendNotification);
    loopDITestButton.setToggleState(pluginProcessor.isTestDILoopEnabled(), juce::dontSendNotification);

    const juce::String status = hasFile
        ? "DI: " + pluginProcessor.getTestDIFileName() + "  (" + juce::String(duration, 2) + "s)"
        : "DI: no file loaded";

    diStatusLabel.setText(status, juce::dontSendNotification);

    const auto loopStart = pluginProcessor.getTestDILoopStartSeconds();
    const auto loopEnd = pluginProcessor.getTestDILoopEndSeconds();
    const auto playhead = pluginProcessor.getTestDIPositionSeconds();
    diLoopLabel.setText("Loop " + juce::String(loopStart, 2) + "s -> " + juce::String(loopEnd, 2) + "s    Playhead " + juce::String(playhead, 2) + "s", juce::dontSendNotification);

    clearDITestButton.setEnabled(hasFile);
    playDITestButton.setEnabled(hasFile && pluginProcessor.isTestDIEnabled());
    repaint(diTimelineArea);
}

void BackhouseAmpSimAudioProcessorEditor::drawDITimeline(juce::Graphics& g) const
{
    if (diTimelineArea.isEmpty())
        return;

    auto full = diTimelineArea;
    g.setColour(juce::Colour(0xff334257));
    g.fillRect(full);

    auto ruler = full.removeFromTop(28);
    auto lane = full.reduced(0, 2);

    g.setColour(juce::Colour(0xff767f89));
    g.fillRect(ruler);

    const double duration = juce::jmax(0.1, pluginProcessor.getTestDIDurationSeconds());
    const double tickSeconds = duration > 120.0 ? 10.0 : 5.0;

    g.setColour(juce::Colours::whitesmoke.withAlpha(0.85f));
    g.setFont(11.0f);
    for (double t = 0.0; t <= duration + 0.0001; t += tickSeconds)
    {
        const int x = timelineXFromTime(t);
        g.drawVerticalLine(x, static_cast<float>(ruler.getY() + 12), static_cast<float>(ruler.getBottom()));

        const int minutes = static_cast<int>(t) / 60;
        const int seconds = static_cast<int>(t) % 60;
        g.drawText(juce::String(minutes) + ":" + juce::String(seconds).paddedLeft('0', 2), x + 2, ruler.getY() + 2, 44, 12, juce::Justification::left);
    }

    g.setColour(juce::Colour(0xff234a66));
    g.fillRect(lane);
    g.setColour(juce::Colour(0x66b7d5ea));
    g.drawLine(static_cast<float>(lane.getX()), static_cast<float>(lane.getCentreY()), static_cast<float>(lane.getRight()), static_cast<float>(lane.getCentreY()), 1.0f);

    const int rows = 3;
    g.setColour(juce::Colours::white.withAlpha(0.12f));
    for (int r = 1; r <= rows; ++r)
    {
        const int y = lane.getY() + (lane.getHeight() * r) / (rows + 1);
        g.drawHorizontalLine(y, static_cast<float>(lane.getX()), static_cast<float>(lane.getRight()));
    }

    if (!diWavePeaks.empty())
    {
        const int width = lane.getWidth();
        const int n = static_cast<int>(diWavePeaks.size());
        g.setColour(juce::Colour(0xffa3d7f3));

        for (int i = 0; i < width; ++i)
        {
            const int peakIdx = juce::jlimit(0, n - 1, (i * n) / juce::jmax(1, width));
            const float peak = diWavePeaks[static_cast<size_t>(peakIdx)];
            const float half = peak * (lane.getHeight() * 0.46f);
            const float x = static_cast<float>(lane.getX() + i);
            const float cy = static_cast<float>(lane.getCentreY());
            g.drawLine(x, cy - half, x, cy + half, 1.0f);
        }
    }

    const double loopStart = pluginProcessor.getTestDILoopStartSeconds();
    const double loopEnd = pluginProcessor.getTestDILoopEndSeconds();
    const int loopStartX = timelineXFromTime(loopStart);
    const int loopEndX = timelineXFromTime(loopEnd);

    g.setColour(juce::Colour(0x2259d36f));
    g.fillRect(juce::Rectangle<int>(loopStartX, lane.getY(), juce::jmax(1, loopEndX - loopStartX), lane.getHeight()));

    g.setColour(juce::Colour(0xff58e06d));
    g.drawVerticalLine(loopStartX, static_cast<float>(lane.getY()), static_cast<float>(lane.getBottom()));
    g.drawVerticalLine(loopEndX, static_cast<float>(lane.getY()), static_cast<float>(lane.getBottom()));

    const int playheadX = timelineXFromTime(pluginProcessor.getTestDIPositionSeconds());
    g.setColour(juce::Colour(0xfff24d4d));
    g.drawVerticalLine(playheadX, static_cast<float>(ruler.getY()), static_cast<float>(lane.getBottom()));

    g.setColour(juce::Colour(0xffe8eff4));
    g.drawRect(lane, 1);
}

double BackhouseAmpSimAudioProcessorEditor::timelineTimeFromX(int x) const
{
    const double duration = juce::jmax(0.1, pluginProcessor.getTestDIDurationSeconds());
    const auto lane = diTimelineArea.withTrimmedTop(28).reduced(0, 2);
    if (lane.getWidth() <= 1)
        return 0.0;

    const double norm = juce::jlimit(0.0, 1.0, (x - lane.getX()) / static_cast<double>(lane.getWidth() - 1));
    return norm * duration;
}

int BackhouseAmpSimAudioProcessorEditor::timelineXFromTime(double seconds) const
{
    const double duration = juce::jmax(0.1, pluginProcessor.getTestDIDurationSeconds());
    const auto lane = diTimelineArea.withTrimmedTop(28).reduced(0, 2);
    if (lane.getWidth() <= 1)
        return lane.getX();

    const double norm = juce::jlimit(0.0, 1.0, seconds / duration);
    return lane.getX() + static_cast<int>(norm * (lane.getWidth() - 1));
}

#endif
