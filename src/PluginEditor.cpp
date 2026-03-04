#include "PluginEditor.h"
#include "BinaryData.h"

#include <array>
#include <cmath>

namespace
{
const juce::Colour bgTop (0xff090a0e);
const juce::Colour bgBottom (0xff060609);
const juce::Colour panelBase (0xff111219);
const juce::Colour panelOutline (0xff22253a);
const juce::Colour textMain (0xffecf0ff);
const juce::Colour textMuted (0xff7880a0);
const juce::Colour accentWarm (0xffff1493); // Ghost Runners hot pink
const juce::Colour accentHot (0xff00d4ff);  // Ghost Runners cyan
const juce::Colour accentCool (0xffaaff00); // Ghost Runners lime

void configureKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 18);
    slider.setColour(juce::Slider::rotarySliderFillColourId, accentHot);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff30333d));
    slider.setColour(juce::Slider::thumbColourId, accentHot);
    slider.setColour(juce::Slider::textBoxTextColourId, textMain);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff0f1014));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff2f3138));
}

void configureEqFader(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 18);
    slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff252933));
    slider.setColour(juce::Slider::thumbColourId, accentWarm);
    slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff12141a));
    slider.setColour(juce::Slider::textBoxTextColourId, textMain);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff0f1014));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff2f3138));
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

constexpr std::array<const char*, 4> preampIds {
    "amp1PreampGain",
    "amp2PreampGain",
    "amp3PreampGain",
    "amp4PreampGain"
};

constexpr std::array<const char*, 5> speakerOptionNames {
    "Greenback 25",
    "Vintage 30",
    "Creamback H75",
    "G12T-75",
    "Jensen C12N"
};

constexpr std::array<const char*, 5> micOptionNames {
    "SM57",
    "U87",
    "U47 FET",
    "Royer 122",
    "MD421"
};

constexpr std::array<const char*, 12> noteNames {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

struct NoteInfo
{
    juce::String note = "--";
    float cents = 0.0f;
};

NoteInfo getNoteInfoFromHz(float hz, int semitoneTranspose)
{
    if (!(hz > 1.0f))
        return {};

    const float shiftedHz = hz * std::pow(2.0f, static_cast<float>(semitoneTranspose) / 12.0f);
    const float midi = 69.0f + 12.0f * std::log2(shiftedHz / 440.0f);
    const int nearest = static_cast<int>(std::round(midi));
    const float cents = (midi - static_cast<float>(nearest)) * 100.0f;
    const int noteIdx = (nearest % 12 + 12) % 12;
    const int octave = (nearest / 12) - 1;
    return { juce::String(noteNames[static_cast<size_t>(noteIdx)]) + juce::String(octave), cents };
}
} // namespace

BackhouseAmpSimAudioProcessorEditor::BackhouseAmpSimAudioProcessorEditor(BackhouseAmpSimAudioProcessor& p)
    : AudioProcessorEditor(&p), pluginProcessor(p)
{
    setSize(1200, 1040);
    setLookAndFeel(&backhouseLAF);

    // Stomp enable buttons use footswitch-cap rendering (name prefix "stomp:")
    stompCompEnabledButton.setName("stomp:comp");
    tsEnabledButton.setName("stomp:ts");
    phaserEnabledButton.setName("stomp:phaser");
    amp1TremEnabledButton.setName("stomp:trem");
    wowEnabledButton.setName("stomp:wow");
    whammyEnabledButton.setName("stomp:whammy");
    wahEnabledButton.setName("stomp:wah");

    // Load custom fonts from embedded binary data
    dharmaPunkTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::Dharma_Punk_ttf, BinaryData::Dharma_Punk_ttfSize);
    punkaholicTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::Punkaholic_D_otf, BinaryData::Punkaholic_D_otfSize);

    // Load company logo
    logoImage = juce::ImageCache::getFromMemory(BinaryData::bhp_logo_png, BinaryData::bhp_logo_pngSize);

    ampSelector.addItemList({ "Sparkle", "Grit", "Thrash", "Heavy" }, 1);
    outputCompModeSelector.addItemList({ "1176", "Opto" }, 1);
    addAndMakeVisible(ampSelector);
    addAndMakeVisible(tabAmpButton);
    addAndMakeVisible(tabTunerButton);
    addAndMakeVisible(tabStompsButton);
    addAndMakeVisible(tabCabButton);
    addAndMakeVisible(tabSpaceButton);
    addAndMakeVisible(tabPostColorButton);
    addAndMakeVisible(tabMidiButton);

    for (auto* slider : { &inputGain, &stompInputGain, &ampPreampGain, &sub, &low, &mid, &high, &presence, &output, &gateThreshold, &cabTone, &reverbMix, &wowPosition, &wowMix, &stompCompSustain, &stompCompLevel, &stompCompMix,
                          &tsDrive, &tsTone, &tsLevel, &phaserRate, &phaserDepth, &phaserCenter, &phaserFeedback, &phaserMix, &amp1TremRate, &amp1TremDepth, &output1176Input, &output1176Release, &output1176Mix,
                          &outputEqQ63, &outputEqQ160, &outputEqQ400, &outputEqQ1000, &outputEqQ2500, &outputEqQ6300, &outputEqQ12000, &outputEqHpHz, &outputEqLpHz,
                          &delayTime, &delayFeedback, &delayMix, &delayTone, &delayPitch, &cabMicBlend,
                          &amp4OctaveMix, &amp4DirectMix, &amp4CleanMix,
                          &profilePickupOutput, &profileBrightness, &profileLowEnd, &profileGateTrim,
                          &whammyPosition, &whammyMix,
                          &wahPosition, &wahMix })
    {
        configureKnob(*slider);
        addAndMakeVisible(*slider);
    }

    inputGain.setName("Input Trim");
    stompInputGain.setName("Stomp In +dB");
    ampPreampGain.setName("Preamp");
    sub.setName("Thicc");
    low.setName("Low");
    mid.setName("Mid");
    high.setName("High");
    presence.setName("Presence");
    output.setName("Master Out");
    gateThreshold.setName("Gate");
    cabTone.setName("Cab Tone");
    reverbMix.setName("Room Mix");
    wowPosition.setName("WOW Pos");
    wowMix.setName("WOW Mix");
    whammyPosition.setName("Whammy Pos");
    whammyMix.setName("Whammy Mix");
    wahPosition.setName("Wah Pos");
    wahMix.setName("Wah Mix");
    stompCompSustain.setName("Comp Sus");
    stompCompLevel.setName("Comp Lvl");
    stompCompMix.setName("Comp Mix");
    tsDrive.setName("TS Drive");
    tsTone.setName("TS Tone");
    tsLevel.setName("TS Level");
    phaserRate.setName("Ph Rate");
    phaserDepth.setName("Ph Depth");
    phaserCenter.setName("Ph Center");
    phaserFeedback.setName("Ph Fdbk");
    phaserMix.setName("Ph Mix");
    amp1TremRate.setName("Trem Rate");
    amp1TremDepth.setName("Trem Depth");
    output1176Input.setName("1176 In");
    output1176Release.setName("1176 Rel");
    output1176Mix.setName("1176 Mix");
    outputEq63.setName("EQ 63");
    outputEq160.setName("EQ 160");
    outputEq400.setName("EQ 400");
    outputEq1000.setName("EQ 1k");
    outputEq2500.setName("EQ 2.5k");
    outputEq6300.setName("EQ 6.3k");
    outputEq12000.setName("EQ 12k");
    outputEqQ63.setName("Q 63");
    outputEqQ160.setName("Q 160");
    outputEqQ400.setName("Q 400");
    outputEqQ1000.setName("Q 1k");
    outputEqQ2500.setName("Q 2.5k");
    outputEqQ6300.setName("Q 6.3k");
    outputEqQ12000.setName("Q 12k");
    outputEqHpHz.setName("HP Hz");
    outputEqLpHz.setName("LP Hz");

    for (size_t i = 0; i < rigModuleDescriptors.size(); ++i)
    {
        auto& button = rigModuleButtons[i];
        button.setButtonText(rigModuleDescriptors[i].label);
        button.setColour(juce::ToggleButton::tickColourId, accentHot);
        button.setColour(juce::ToggleButton::textColourId, textMain);
        addAndMakeVisible(button);
    }

    rigPointsLabel.setText("Rig Score: 0 / 10", juce::dontSendNotification);
    rigPointsLabel.setJustificationType(juce::Justification::centredLeft);
    rigPointsLabel.setColour(juce::Label::textColourId, accentHot);
    addAndMakeVisible(rigPointsLabel);

    rigMaxPointsSlider.setName("Rig Budget");
    rigMaxPointsSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    rigMaxPointsSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 20);
    rigMaxPointsSlider.setRange(4.0, 20.0, 1.0);
    rigMaxPointsSlider.setColour(juce::Slider::thumbColourId, accentWarm);
    rigMaxPointsSlider.setColour(juce::Slider::trackColourId, panelOutline.withAlpha(0.6f));
    rigMaxPointsSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff0f1117));
    rigMaxPointsSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff0f1014));
    rigMaxPointsSlider.setColour(juce::Slider::textBoxOutlineColourId, panelOutline);
    addAndMakeVisible(rigMaxPointsSlider);

    rigResetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff16182a));
    rigResetButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffff1493));
    rigResetButton.setColour(juce::TextButton::textColourOffId, textMain);
    rigResetButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    addAndMakeVisible(rigResetButton);

    for (auto* eqGain : { &outputEq63, &outputEq160, &outputEq400, &outputEq1000, &outputEq2500, &outputEq6300, &outputEq12000 })
    {
        configureEqFader(*eqGain);
        addAndMakeVisible(*eqGain);
    }
    delayTime.setName("Delay ms");
    delayFeedback.setName("Dly Fdbk");
    delayMix.setName("Dly Mix");
    delayTone.setName("Dly Tone");
    delayPitch.setName("Dly Pitch");
    cabMicBlend.setName("Mic Blend");
    amp4OctaveMix.setName("Oct Mix");
    amp4DirectMix.setName("Direct Add");
    amp4CleanMix.setName("Clean Add");
    profilePickupOutput.setName("Pickup Out");
    profileBrightness.setName("Brightness");
    profileLowEnd.setName("Low End");
    profileGateTrim.setName("Gate Trim");
    tunerTranspose.setName("Transpose");

    auto tintKnob = [](juce::Slider& slider, juce::Colour colour) {
        slider.setColour(juce::Slider::rotarySliderFillColourId, colour);
        slider.setColour(juce::Slider::thumbColourId, colour.brighter(0.3f));
    };

    tintKnob(inputGain, juce::Colour(0xffd86b2e));
    tintKnob(stompInputGain, juce::Colour(0xffe1842f));
    tintKnob(ampPreampGain, juce::Colour(0xffef8e36));
    tintKnob(sub, juce::Colour(0xffc17244));
    tintKnob(low, juce::Colour(0xffb9833d));
    tintKnob(mid, juce::Colour(0xffd0a047));
    tintKnob(high, juce::Colour(0xffe1be65));
    tintKnob(presence, juce::Colour(0xffe7cf7e));
    tintKnob(output, juce::Colour(0xff9eb79f));
    tintKnob(gateThreshold, juce::Colour(0xffb25a4a));
    tintKnob(cabTone, juce::Colour(0xff7f9ba1));
    tintKnob(reverbMix, juce::Colour(0xff6ea2ba));
    tintKnob(wowPosition, juce::Colour(0xffd66f37));
    tintKnob(wowMix, juce::Colour(0xffc49867));
    tintKnob(stompCompSustain, juce::Colour(0xff7bb47f));
    tintKnob(stompCompLevel, juce::Colour(0xff84bf88));
    tintKnob(stompCompMix, juce::Colour(0xff8ecc92));
    tintKnob(tsDrive, juce::Colour(0xffc96c2f));
    tintKnob(tsTone, juce::Colour(0xffca8240));
    tintKnob(tsLevel, juce::Colour(0xffd09757));
    tintKnob(phaserRate, juce::Colour(0xff6d86bf));
    tintKnob(phaserDepth, juce::Colour(0xff7491c8));
    tintKnob(phaserCenter, juce::Colour(0xff7aa0d5));
    tintKnob(phaserFeedback, juce::Colour(0xff85a9db));
    tintKnob(phaserMix, juce::Colour(0xff7c96d2));
    tintKnob(amp1TremRate, juce::Colour(0xff78b5c1));
    tintKnob(amp1TremDepth, juce::Colour(0xff89c6cf));
    tintKnob(output1176Input, juce::Colour(0xffd3a45a));
    tintKnob(output1176Release, juce::Colour(0xffe0b96d));
    tintKnob(output1176Mix, juce::Colour(0xffe8c57a));
    tintKnob(outputEq63, juce::Colour(0xff95a8b9));
    tintKnob(outputEq160, juce::Colour(0xff9dafbe));
    tintKnob(outputEq400, juce::Colour(0xffa7b6c4));
    tintKnob(outputEq1000, juce::Colour(0xffb3c0cc));
    tintKnob(outputEq2500, juce::Colour(0xffbec8d3));
    tintKnob(outputEq6300, juce::Colour(0xffc8d0da));
    tintKnob(outputEq12000, juce::Colour(0xffd1d8e0));
    tintKnob(outputEqQ63, juce::Colour(0xff7f93a5));
    tintKnob(outputEqQ160, juce::Colour(0xff8399aa));
    tintKnob(outputEqQ400, juce::Colour(0xff8a9eb0));
    tintKnob(outputEqQ1000, juce::Colour(0xff92a7b7));
    tintKnob(outputEqQ2500, juce::Colour(0xff9aaebd));
    tintKnob(outputEqQ6300, juce::Colour(0xffa4b8c5));
    tintKnob(outputEqQ12000, juce::Colour(0xffafc1cc));
    tintKnob(outputEqHpHz, juce::Colour(0xff5b8ba6));
    tintKnob(outputEqLpHz, juce::Colour(0xff6a98b2));
    tintKnob(delayTime, juce::Colour(0xff4ea2b8));
    tintKnob(delayFeedback, juce::Colour(0xff5fb8cc));
    tintKnob(delayMix, juce::Colour(0xff70c8d8));
    tintKnob(delayTone, juce::Colour(0xff86d1df));
    tintKnob(delayPitch, juce::Colour(0xff95bddd));
    tintKnob(cabMicBlend, juce::Colour(0xff86a7cf));
    tintKnob(amp4OctaveMix, juce::Colour(0xff7e87b8));
    tintKnob(amp4DirectMix, juce::Colour(0xff8d9fbe));
    tintKnob(amp4CleanMix, juce::Colour(0xff9ec1b8));
    tintKnob(whammyPosition, juce::Colour(0xffb06fc4));
    tintKnob(whammyMix, juce::Colour(0xffb882d0));
    tintKnob(wahPosition, juce::Colour(0xffd4a800));
    tintKnob(wahMix, juce::Colour(0xffe0b800));

    tunerTranspose.setSliderStyle(juce::Slider::LinearHorizontal);
    tunerTranspose.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 20);
    tunerTranspose.setRange(-12.0, 12.0, 1.0);
    tunerTranspose.setColour(juce::Slider::trackColourId, accentWarm);
    tunerTranspose.setColour(juce::Slider::thumbColourId, accentHot);
    tunerTranspose.setColour(juce::Slider::textBoxTextColourId, textMain);
    tunerTranspose.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff231913));
    tunerTranspose.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff4f3b2e));
    addAndMakeVisible(tunerTranspose);

    for (auto* editor : { &profile1NameEditor, &profile2NameEditor, &profile3NameEditor })
    {
        configureNameEditor(*editor);
        addAndMakeVisible(*editor);
    }

    addAndMakeVisible(inputBoostButton);
    addAndMakeVisible(amp2HiwattButton);
    addAndMakeVisible(amp4TightButton);
    addAndMakeVisible(amp4LowOctaveButton);
    addAndMakeVisible(cabEnabledButton);
    addAndMakeVisible(reverbEnabledButton);
    addAndMakeVisible(wowEnabledButton);
    addAndMakeVisible(wowMomentaryButton);
    addAndMakeVisible(wowMidiEnableButton);
    addAndMakeVisible(stompCompEnabledButton);
    addAndMakeVisible(whammyEnabledButton);
    whammyModeSelector.addItemList({ "+1 Oct", "+2 Oct", "-1 Oct", "-2 Oct", "+5th", "+4th" }, 1);
    addAndMakeVisible(whammyModeSelector);
    addAndMakeVisible(wahEnabledButton);
    wowModeSelector.addItemList({ "FATSO", "BLADE 1", "BLADE 2" }, 1);
    addAndMakeVisible(wowModeSelector);
    addAndMakeVisible(outputCompModeSelector);
    addAndMakeVisible(tsEnabledButton);
    addAndMakeVisible(delayEnabledButton);
    addAndMakeVisible(phaserEnabledButton);
    addAndMakeVisible(amp1TremEnabledButton);
    addAndMakeVisible(output1176EnabledButton);
    addAndMakeVisible(outputEqEnabledButton);
    addAndMakeVisible(outputEqHpEnabledButton);
    addAndMakeVisible(outputEqLpEnabledButton);
    addAndMakeVisible(tunerButton);
    addAndMakeVisible(uiModeButton);
    addAndMakeVisible(profileNeutralButton);
    addAndMakeVisible(profile1Button);
    addAndMakeVisible(profile2Button);
    addAndMakeVisible(profile3Button);
    addAndMakeVisible(exportProfilesButton);
    addAndMakeVisible(importProfilesButton);
    addAndMakeVisible(loadIRButton);
    addAndMakeVisible(clearIRButton);
    addAndMakeVisible(loadReverbIRButton);
    addAndMakeVisible(clearReverbIRButton);
    addAndMakeVisible(speakerGreenbackButton);
    addAndMakeVisible(speakerV30Button);
    addAndMakeVisible(speakerCreambackButton);
    addAndMakeVisible(speakerG12TButton);
    addAndMakeVisible(speakerJensenButton);
    addAndMakeVisible(mic57Button);
    addAndMakeVisible(micU87Button);
    addAndMakeVisible(micU47FetButton);
    addAndMakeVisible(micRoyerButton);
    addAndMakeVisible(mic421Button);
    addAndMakeVisible(micAOffAxisButton);
    addAndMakeVisible(speakerBGreenbackButton);
    addAndMakeVisible(speakerBV30Button);
    addAndMakeVisible(speakerBCreambackButton);
    addAndMakeVisible(speakerBG12TButton);
    addAndMakeVisible(speakerBJensenButton);
    addAndMakeVisible(micB57Button);
    addAndMakeVisible(micBU87Button);
    addAndMakeVisible(micBU47FetButton);
    addAndMakeVisible(micBRoyerButton);
    addAndMakeVisible(micB421Button);
    addAndMakeVisible(micBOffAxisButton);

    for (auto* toggle : { &inputBoostButton, &amp2HiwattButton, &amp4TightButton, &amp4LowOctaveButton, &cabEnabledButton, &reverbEnabledButton, &wowEnabledButton, &wowMomentaryButton, &wowMidiEnableButton, &stompCompEnabledButton, &tsEnabledButton, &delayEnabledButton, &phaserEnabledButton, &amp1TremEnabledButton, &output1176EnabledButton, &outputEqEnabledButton, &outputEqHpEnabledButton, &outputEqLpEnabledButton, &tunerButton, &whammyEnabledButton, &wahEnabledButton,
                          &micAOffAxisButton, &micBOffAxisButton })
    {
        toggle->setColour(juce::ToggleButton::tickColourId, accentWarm);
        toggle->setColour(juce::ToggleButton::textColourId, textMain);
    }

    for (auto& button : rigModuleButtons)
    {
        button.setColour(juce::ToggleButton::tickColourId, accentHot);
        button.setColour(juce::ToggleButton::textColourId, textMain);
    }

    // Tab + utility buttons — dark blue-black inactive, hot-pink active
    for (auto* button : { &uiModeButton, &tabAmpButton, &tabTunerButton, &tabStompsButton, &tabCabButton, &tabSpaceButton, &tabPostColorButton, &tabMidiButton,
                          &exportProfilesButton, &importProfilesButton,
                          &loadIRButton, &clearIRButton, &loadReverbIRButton, &clearReverbIRButton })
    {
        button->setColour(juce::TextButton::buttonColourId,  juce::Colour(0xff16182a));
        button->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffff1493));
        button->setColour(juce::TextButton::textColourOffId, textMain);
        button->setColour(juce::TextButton::textColourOnId,  juce::Colours::white);
    }

    // Profile selector buttons — dark blue-black base, cyan active
    for (auto* button : { &profileNeutralButton, &profile1Button, &profile2Button, &profile3Button })
    {
        button->setColour(juce::TextButton::buttonColourId,  juce::Colour(0xff0c1420));
        button->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00d4ff));
        button->setColour(juce::TextButton::textColourOffId, textMuted);
        button->setColour(juce::TextButton::textColourOnId,  juce::Colours::white);
    }

    // Cab/Mic selector buttons — dark, bright amber when active
    for (auto* button : { &speakerGreenbackButton, &speakerV30Button, &speakerCreambackButton, &speakerG12TButton, &speakerJensenButton,
                          &mic57Button, &micU87Button, &micU47FetButton, &micRoyerButton, &mic421Button,
                          &speakerBGreenbackButton, &speakerBV30Button, &speakerBCreambackButton, &speakerBG12TButton, &speakerBJensenButton,
                          &micB57Button, &micBU87Button, &micBU47FetButton, &micBRoyerButton, &micB421Button })
    {
        button->setColour(juce::TextButton::buttonColourId,  juce::Colour(0xff2f241d));
        button->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffa35b2b));
        button->setColour(juce::TextButton::textColourOffId, textMuted);
        button->setColour(juce::TextButton::textColourOnId,  juce::Colours::white);
    }

    pitchLabel.setText("Pitch: -- Hz", juce::dontSendNotification);
    pitchLabel.setJustificationType(juce::Justification::centredLeft);
    pitchLabel.setColour(juce::Label::textColourId, textMain);
    addAndMakeVisible(pitchLabel);

    profileEditLabel.setText("Profile Offsets (Pickup/Brightness/Low-End/Gate)", juce::dontSendNotification);
    profileEditLabel.setJustificationType(juce::Justification::centredLeft);
    profileEditLabel.setColour(juce::Label::textColourId, textMuted);
    addAndMakeVisible(profileEditLabel);

    profileIoStatusLabel.setJustificationType(juce::Justification::centredLeft);
    profileIoStatusLabel.setColour(juce::Label::textColourId, textMuted);
    addAndMakeVisible(profileIoStatusLabel);

    irStatusLabel.setJustificationType(juce::Justification::centredLeft);
    irStatusLabel.setColour(juce::Label::textColourId, textMuted);
    addAndMakeVisible(irStatusLabel);

    cabSpeakerLabel.setText("Speaker", juce::dontSendNotification);
    cabMicLabel.setText("Mic", juce::dontSendNotification);
    cabSpeakerBLabel.setText("Speaker B", juce::dontSendNotification);
    cabMicBLabel.setText("Mic B", juce::dontSendNotification);
    cabSpeakerLabel.setJustificationType(juce::Justification::centredLeft);
    cabMicLabel.setJustificationType(juce::Justification::centredLeft);
    cabSpeakerBLabel.setJustificationType(juce::Justification::centredLeft);
    cabMicBLabel.setJustificationType(juce::Justification::centredLeft);
    cabSpeakerLabel.setColour(juce::Label::textColourId, textMuted);
    cabMicLabel.setColour(juce::Label::textColourId, textMuted);
    cabSpeakerBLabel.setColour(juce::Label::textColourId, textMuted);
    cabMicBLabel.setColour(juce::Label::textColourId, textMuted);
    addAndMakeVisible(cabSpeakerLabel);
    addAndMakeVisible(cabMicLabel);
    addAndMakeVisible(cabSpeakerBLabel);
    addAndMakeVisible(cabMicBLabel);

    meterInputLabel.setText("In Meter", juce::dontSendNotification);
    meterOutputLabel.setText("Out Meter", juce::dontSendNotification);
    preampHeatLabel.setText("Preamp: Warm", juce::dontSendNotification);
    meterInputLabel.setJustificationType(juce::Justification::centredLeft);
    meterOutputLabel.setJustificationType(juce::Justification::centredLeft);
    preampHeatLabel.setJustificationType(juce::Justification::centredLeft);
    meterInputLabel.setColour(juce::Label::textColourId, textMain);
    meterOutputLabel.setColour(juce::Label::textColourId, textMain);
    preampHeatLabel.setColour(juce::Label::textColourId, accentWarm);
    addAndMakeVisible(meterInputLabel);
    addAndMakeVisible(meterOutputLabel);
    addAndMakeVisible(preampHeatLabel);
    addAndMakeVisible(inputMeterBar);
    addAndMakeVisible(outputMeterBar);
    inputMeterBar.setPercentageDisplay(false);
    outputMeterBar.setPercentageDisplay(false);
    inputMeterBar.setColour(juce::ProgressBar::backgroundColourId,  juce::Colour(0xff261c16));
    outputMeterBar.setColour(juce::ProgressBar::backgroundColourId, juce::Colour(0xff261c16));
    inputMeterBar.setColour(juce::ProgressBar::foregroundColourId,  juce::Colour(0xffcfa06f));
    outputMeterBar.setColour(juce::ProgressBar::foregroundColourId, juce::Colour(0xffea8f3e));

    tunerNoteLabel.setText("Note: --", juce::dontSendNotification);
    tunerDetuneLabel.setText("Detune: -- cents", juce::dontSendNotification);
    tunerTransposeLabel.setText("Reference Shift (semitones)", juce::dontSendNotification);
    midiMapHelpLabel.setText("Modes: Absolute follows knob/pedal value. Toggle flips Min/Max on press. Momentary = Max while held, Min on release.", juce::dontSendNotification);
    for (auto* label : { &tunerNoteLabel, &tunerDetuneLabel, &tunerTransposeLabel, &midiMapHelpLabel })
    {
        label->setColour(juce::Label::textColourId, textMain);
        label->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(*label);
    }
    for (auto* button : { &midiTemplateRhythmButton, &midiTemplateLeadButton, &midiTemplateAmbientButton })
    {
        button->setColour(juce::TextButton::buttonColourId,  juce::Colour(0xff16182a));
        button->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffff1493));
        button->setColour(juce::TextButton::textColourOffId, textMain);
        button->setColour(juce::TextButton::textColourOnId,  juce::Colours::white);
        addAndMakeVisible(*button);
    }

    for (size_t i = 0; i < midiMapRows.size(); ++i)
    {
        auto& row = midiMapRows[i];
        row.slotLabel.setText("S" + juce::String(static_cast<int>(i + 1)), juce::dontSendNotification);
        row.slotLabel.setColour(juce::Label::textColourId, textMain);
        row.ccSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        row.ccSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 20);
        row.ccSlider.setRange(0.0, 127.0, 1.0);
        row.modeBox.addItemList({ "Absolute", "Toggle", "Momentary" }, 1);
        row.minSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        row.minSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 42, 20);
        row.minSlider.setRange(0.0, 1.0, 0.01);
        row.maxSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        row.maxSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 42, 20);
        row.maxSlider.setRange(0.0, 1.0, 0.01);

        addAndMakeVisible(row.slotLabel);
        addAndMakeVisible(row.enabledButton);
        addAndMakeVisible(row.ccSlider);
        addAndMakeVisible(row.targetBox);
        addAndMakeVisible(row.modeBox);
        addAndMakeVisible(row.minSlider);
        addAndMakeVisible(row.maxSlider);
        addAndMakeVisible(row.invertButton);
    }

#if JucePlugin_Build_Standalone
    for (auto* toggle : { &enableDITestButton, &loopDITestButton })
    {
        toggle->setColour(juce::ToggleButton::tickColourId, accentWarm);
        toggle->setColour(juce::ToggleButton::textColourId, textMain);
    }

    for (auto* button : { &audioSettingsButton, &loadDITestButton, &clearDITestButton, &playDITestButton, &pauseDITestButton, &stopDITestButton, &rwDITestButton, &ffDITestButton,
                          &loopStartBackButton, &loopStartForwardButton, &loopEndBackButton, &loopEndForwardButton,
                          &selectLoopButton, &trimSelectionButton, &cutSelectionButton, &clearSelectionButton,
                          &captureAButton, &recallAButton, &captureBButton, &recallBButton, &toggleABButton })
    {
        button->setColour(juce::TextButton::buttonColourId,  juce::Colour(0xff16182a));
        button->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffff1493));
        button->setColour(juce::TextButton::textColourOffId, textMain);
        button->setColour(juce::TextButton::textColourOnId,  juce::Colours::white);
    }

    testBenchLabel.setText("Standalone Test Bench", juce::dontSendNotification);
    testBenchLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(testBenchLabel);

    midiInputLabel.setText("MIDI Input", juce::dontSendNotification);
    midiOutputLabel.setText("MIDI Output", juce::dontSendNotification);
    diZoomLabel.setText("Zoom", juce::dontSendNotification);
    diScrollLabel.setText("Scroll", juce::dontSendNotification);
    diGainLabel.setText("DI Gain", juce::dontSendNotification);
    abSnapshotStatusLabel.setText("A/B: not captured", juce::dontSendNotification);

    for (auto* label : { &midiInputLabel, &midiOutputLabel, &diStatusLabel, &diLoopLabel, &diZoomLabel, &diScrollLabel, &diGainLabel, &abSnapshotStatusLabel })
    {
        label->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(*label);
    }

    diGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    diGainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 58, 20);
    diGainSlider.setRange(-24.0, 24.0, 0.5);
    diGainSlider.setValue(0.0, juce::dontSendNotification);
    diGainSlider.setTextValueSuffix(" dB");
    diGainSlider.onValueChange = [this] {
        pluginProcessor.setTestDIGainDb(static_cast<float>(diGainSlider.getValue()));
    };
    diGainSlider.setDoubleClickReturnValue(true, 0.0);
    addAndMakeVisible(diGainSlider);

    diZoomSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    diZoomSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    diZoomSlider.setRange(0.0, 1.0, 0.001);
    diZoomSlider.setValue(0.0, juce::dontSendNotification);
    diScrollSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    diScrollSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    diScrollSlider.setRange(0.0, 1.0, 0.001);
    diScrollSlider.setValue(0.0, juce::dontSendNotification);

    addAndMakeVisible(audioSettingsButton);
    addAndMakeVisible(midiInputSelector);
    addAndMakeVisible(midiOutputSelector);
    addAndMakeVisible(loadDITestButton);
    addAndMakeVisible(clearDITestButton);
    addAndMakeVisible(enableDITestButton);
    addAndMakeVisible(playDITestButton);
    addAndMakeVisible(pauseDITestButton);
    addAndMakeVisible(stopDITestButton);
    addAndMakeVisible(rwDITestButton);
    addAndMakeVisible(ffDITestButton);
    addAndMakeVisible(loopDITestButton);
    addAndMakeVisible(loopStartBackButton);
    addAndMakeVisible(loopStartForwardButton);
    addAndMakeVisible(loopEndBackButton);
    addAndMakeVisible(loopEndForwardButton);
    addAndMakeVisible(selectLoopButton);
    addAndMakeVisible(trimSelectionButton);
    addAndMakeVisible(cutSelectionButton);
    addAndMakeVisible(clearSelectionButton);
    addAndMakeVisible(diZoomSlider);
    addAndMakeVisible(diScrollSlider);
    addAndMakeVisible(captureAButton);
    addAndMakeVisible(recallAButton);
    addAndMakeVisible(captureBButton);
    addAndMakeVisible(recallBButton);
    addAndMakeVisible(toggleABButton);

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
        if (diDialogOpen)
            return;

        diDialogOpen = true;
        const auto defaultDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
        const auto browseStart = lastDIBrowseLocation.exists() ? lastDIBrowseLocation : defaultDir;
        diChooser = std::make_unique<juce::FileChooser>("Load raw DI guitar file", browseStart, "*.wav;*.aiff;*.aif;*.mp3");
        diChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                               [this](const juce::FileChooser& chooser) {
                                   // Wrap everything — exceptions must NOT escape into AppKit's ObjC call stack
                                   // or they will propagate through objc_exception_rethrow → terminate → abort.
                                   try
                                   {
                                       const auto file = chooser.getResult();
                                       diDialogOpen = false;

                                       if (file.existsAsFile())
                                       {
                                           lastDIBrowseLocation = file.getParentDirectory();
                                           const bool ok = pluginProcessor.loadTestDIFile(file);
                                           if (ok)
                                           {
                                               diHasSelection = false;
                                               diSelectionStartSeconds = 0.0;
                                               diSelectionEndSeconds = 0.0;
                                               diScrollSlider.setValue(0.0, juce::dontSendNotification);
                                               diZoomSlider.setValue(0.0, juce::dontSendNotification);
                                               diWaveCacheWidth = 0;
                                               updateDITimelineCache();
                                               refreshTestDISection();
                                           }
                                           else
                                           {
                                               diStatusLabel.setText("Could not load: " + file.getFileName(),
                                                                     juce::dontSendNotification);
                                           }
                                       }
                                       else
                                       {
                                           diDialogOpen = false;
                                       }
                                   }
                                   catch (const std::bad_alloc&)
                                   {
                                       diDialogOpen = false;
                                       diStatusLabel.setText("File too large to load into memory",
                                                             juce::dontSendNotification);
                                   }
                                   catch (...)
                                   {
                                       diDialogOpen = false;
                                       diStatusLabel.setText("Error loading DI file",
                                                             juce::dontSendNotification);
                                   }

                                   // Defer destruction out of the native panel callback.
                                   juce::MessageManager::callAsync([this] { diChooser.reset(); });
                               });
    };

    clearDITestButton.onClick = [this] {
        pluginProcessor.clearTestDIFile();
        diHasSelection = false;
        diSelectionStartSeconds = 0.0;
        diSelectionEndSeconds = 0.0;
        diScrollSlider.setValue(0.0, juce::dontSendNotification);
        diZoomSlider.setValue(0.0, juce::dontSendNotification);
        diWaveCacheWidth = 0;
        updateDITimelineCache();
        refreshTestDISection();
    };

    enableDITestButton.onClick = [this] { pluginProcessor.setTestDIEnabled(enableDITestButton.getToggleState()); };
    playDITestButton.onClick = [this] {
        pluginProcessor.setTestDIPlaying(true);
        refreshTestDISection();
    };
    pauseDITestButton.onClick = [this] {
        pluginProcessor.setTestDIPlaying(false);
        refreshTestDISection();
    };
    stopDITestButton.onClick = [this] {
        pluginProcessor.setTestDIPlaying(false);
        pluginProcessor.setTestDIPositionSeconds(0.0);
        refreshTestDISection();
    };
    rwDITestButton.onClick = [this] {
        const double p = pluginProcessor.getTestDIPositionSeconds();
        pluginProcessor.setTestDIPositionSeconds(juce::jmax(0.0, p - 1.0));
        refreshTestDISection();
    };
    ffDITestButton.onClick = [this] {
        const double p = pluginProcessor.getTestDIPositionSeconds();
        const double d = pluginProcessor.getTestDIDurationSeconds();
        pluginProcessor.setTestDIPositionSeconds(juce::jmin(d, p + 1.0));
        refreshTestDISection();
    };
    loopDITestButton.onClick = [this] { pluginProcessor.setTestDILoopEnabled(loopDITestButton.getToggleState()); };
    loopStartBackButton.onClick = [this] { nudgeLoopStart(-0.01); };
    loopStartForwardButton.onClick = [this] { nudgeLoopStart(0.01); };
    loopEndBackButton.onClick = [this] { nudgeLoopEnd(-0.01); };
    loopEndForwardButton.onClick = [this] { nudgeLoopEnd(0.01); };
    selectLoopButton.onClick = [this] { setSelectionFromLoop(); };
    clearSelectionButton.onClick = [this] { clearDISelection(); };
    trimSelectionButton.onClick = [this] { trimDIToSelection(); };
    cutSelectionButton.onClick = [this] { cutDISelection(); };
    diZoomSlider.onValueChange = [this] {
        updateTimelineViewport();
        repaint(diTimelineArea);
    };
    diScrollSlider.onValueChange = [this] {
        updateTimelineViewport();
        repaint(diTimelineArea);
    };

    captureAButton.onClick = [this] { captureABSnapshot(0); };
    recallAButton.onClick = [this] { recallABSnapshot(0); };
    captureBButton.onClick = [this] { captureABSnapshot(1); };
    recallBButton.onClick = [this] { recallABSnapshot(1); };
    toggleABButton.onClick = [this] { toggleABSnapshot(); };

    attachStandaloneDeviceManager();
    updateDITimelineCache();
    refreshTestDISection();
    refreshABSnapshotButtons();
#endif

    auto& apvts = pluginProcessor.getAPVTS();
    ampAttachment = std::make_unique<ComboAttachment>(apvts, "ampType", ampSelector);
    wowModeAttachment = std::make_unique<ComboAttachment>(apvts, "wowMode", wowModeSelector);
    outputCompModeAttachment = std::make_unique<ComboAttachment>(apvts, "outputCompMode", outputCompModeSelector);
    if (auto* speakerParam = apvts.getParameter("cabSpeaker"))
    {
        cabSpeakerChoiceAttachment = std::make_unique<ChoiceAttachment>(*speakerParam, [this](float) {
            refreshCabSelectorButtons();
            refreshIRStatusLabel();
        }, nullptr);
        cabSpeakerChoiceAttachment->sendInitialUpdate();
    }
    if (auto* micParam = apvts.getParameter("cabMic"))
    {
        cabMicChoiceAttachment = std::make_unique<ChoiceAttachment>(*micParam, [this](float) {
            refreshCabSelectorButtons();
            refreshIRStatusLabel();
        }, nullptr);
        cabMicChoiceAttachment->sendInitialUpdate();
    }
    if (auto* speakerBParam = apvts.getParameter("cabSpeakerB"))
    {
        cabSpeakerBChoiceAttachment = std::make_unique<ChoiceAttachment>(*speakerBParam, [this](float) {
            refreshCabSelectorButtons();
            refreshIRStatusLabel();
        }, nullptr);
        cabSpeakerBChoiceAttachment->sendInitialUpdate();
    }
    if (auto* micBParam = apvts.getParameter("cabMicB"))
    {
        cabMicBChoiceAttachment = std::make_unique<ChoiceAttachment>(*micBParam, [this](float) {
            refreshCabSelectorButtons();
            refreshIRStatusLabel();
        }, nullptr);
        cabMicBChoiceAttachment->sendInitialUpdate();
    }
    if (auto* profileParam = apvts.getParameter("guitarProfile"))
    {
        guitarProfileAttachment = std::make_unique<ChoiceAttachment>(*profileParam, [this](float) {
            refreshGuitarProfileButtons();
            bindProfileEditControls();
        }, nullptr);
        guitarProfileAttachment->sendInitialUpdate();
    }

    if (auto* ampParam = apvts.getParameter("ampType"))
    {
        ampTypeAttachment = std::make_unique<ChoiceAttachment>(*ampParam, [this](float) {
            bindAmpPreampControl();
            refreshPreampHeatLabel();
        }, nullptr);
        ampTypeAttachment->sendInitialUpdate();
    }

    inputBoostAttachment = std::make_unique<ButtonAttachment>(apvts, "inputBoost", inputBoostButton);
    amp2HiwattAttachment = std::make_unique<ButtonAttachment>(apvts, "amp2Hiwatt", amp2HiwattButton);
    amp4TightAttachment = std::make_unique<ButtonAttachment>(apvts, "amp4Tight", amp4TightButton);
    amp4LowOctaveAttachment = std::make_unique<ButtonAttachment>(apvts, "amp4LowOctave", amp4LowOctaveButton);
    cabEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "cabEnabled", cabEnabledButton);
    reverbEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "reverbEnabled", reverbEnabledButton);
    wowEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "wowEnabled", wowEnabledButton);
    wowMomentaryAttachment = std::make_unique<ButtonAttachment>(apvts, "wowMomentary", wowMomentaryButton);
    wowMidiEnableAttachment = std::make_unique<ButtonAttachment>(apvts, "wowMidiEnable", wowMidiEnableButton);
    stompCompEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "stompCompEnabled", stompCompEnabledButton);
    tsEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "tsEnabled", tsEnabledButton);
    delayEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "delayEnabled", delayEnabledButton);
    phaserEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "phaserEnabled", phaserEnabledButton);
    amp1TremEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "amp1TremEnabled", amp1TremEnabledButton);
    output1176EnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "output1176Enabled", output1176EnabledButton);
    outputEqEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "outputEqEnabled", outputEqEnabledButton);
    outputEqHpEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "outputEqHpEnabled", outputEqHpEnabledButton);
    outputEqLpEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "outputEqLpEnabled", outputEqLpEnabledButton);
    tunerAttachment = std::make_unique<ButtonAttachment>(apvts, "tunerEnabled", tunerButton);
    whammyEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "whammyEnabled", whammyEnabledButton);
    whammyModeAttachment = std::make_unique<ComboAttachment>(apvts, "whammyMode", whammyModeSelector);
    wahEnabledAttachment = std::make_unique<ButtonAttachment>(apvts, "wahEnabled", wahEnabledButton);
    cabMicAOffAxisAttachment = std::make_unique<ButtonAttachment>(apvts, "cabMicAOffAxis", micAOffAxisButton);
    cabMicBOffAxisAttachment = std::make_unique<ButtonAttachment>(apvts, "cabMicBOffAxis", micBOffAxisButton);

    inputGainAttachment = std::make_unique<SliderAttachment>(apvts, "inputGain", inputGain);
    stompInputGainAttachment = std::make_unique<SliderAttachment>(apvts, "stompInputGain", stompInputGain);
    subAttachment = std::make_unique<SliderAttachment>(apvts, "sub", sub);
    lowAttachment = std::make_unique<SliderAttachment>(apvts, "low", low);
    midAttachment = std::make_unique<SliderAttachment>(apvts, "mid", mid);
    highAttachment = std::make_unique<SliderAttachment>(apvts, "high", high);
    presenceAttachment = std::make_unique<SliderAttachment>(apvts, "presence", presence);
    outputAttachment = std::make_unique<SliderAttachment>(apvts, "output", output);
    gateAttachment = std::make_unique<SliderAttachment>(apvts, "gateThresh", gateThreshold);
    cabToneAttachment = std::make_unique<SliderAttachment>(apvts, "cabTone", cabTone);
    reverbMixAttachment = std::make_unique<SliderAttachment>(apvts, "reverbMix", reverbMix);
    wowPositionAttachment = std::make_unique<SliderAttachment>(apvts, "wowPosition", wowPosition);
    wowMixAttachment = std::make_unique<SliderAttachment>(apvts, "wowMix", wowMix);
    whammyPositionAttachment = std::make_unique<SliderAttachment>(apvts, "whammyPosition", whammyPosition);
    whammyMixAttachment = std::make_unique<SliderAttachment>(apvts, "whammyMix", whammyMix);
    wahPositionAttachment = std::make_unique<SliderAttachment>(apvts, "wahPosition", wahPosition);
    wahMixAttachment = std::make_unique<SliderAttachment>(apvts, "wahMix", wahMix);
    stompCompSustainAttachment = std::make_unique<SliderAttachment>(apvts, "stompCompSustain", stompCompSustain);
    stompCompLevelAttachment = std::make_unique<SliderAttachment>(apvts, "stompCompLevel", stompCompLevel);
    stompCompMixAttachment = std::make_unique<SliderAttachment>(apvts, "stompCompMix", stompCompMix);
    tsDriveAttachment = std::make_unique<SliderAttachment>(apvts, "tsDrive", tsDrive);
    tsToneAttachment = std::make_unique<SliderAttachment>(apvts, "tsTone", tsTone);
    tsLevelAttachment = std::make_unique<SliderAttachment>(apvts, "tsLevel", tsLevel);
    phaserRateAttachment = std::make_unique<SliderAttachment>(apvts, "phaserRate", phaserRate);
    phaserDepthAttachment = std::make_unique<SliderAttachment>(apvts, "phaserDepth", phaserDepth);
    phaserCenterAttachment = std::make_unique<SliderAttachment>(apvts, "phaserCenterHz", phaserCenter);
    phaserFeedbackAttachment = std::make_unique<SliderAttachment>(apvts, "phaserFeedback", phaserFeedback);
    phaserMixAttachment = std::make_unique<SliderAttachment>(apvts, "phaserMix", phaserMix);
    amp1TremRateAttachment = std::make_unique<SliderAttachment>(apvts, "amp1TremRate", amp1TremRate);
    amp1TremDepthAttachment = std::make_unique<SliderAttachment>(apvts, "amp1TremDepth", amp1TremDepth);
    output1176InputAttachment = std::make_unique<SliderAttachment>(apvts, "output1176Input", output1176Input);
    output1176ReleaseAttachment = std::make_unique<SliderAttachment>(apvts, "output1176Release", output1176Release);
    output1176MixAttachment = std::make_unique<SliderAttachment>(apvts, "output1176Mix", output1176Mix);
    outputEq63Attachment = std::make_unique<SliderAttachment>(apvts, "outputEq63", outputEq63);
    outputEq160Attachment = std::make_unique<SliderAttachment>(apvts, "outputEq160", outputEq160);
    outputEq400Attachment = std::make_unique<SliderAttachment>(apvts, "outputEq400", outputEq400);
    outputEq1000Attachment = std::make_unique<SliderAttachment>(apvts, "outputEq1000", outputEq1000);
    outputEq2500Attachment = std::make_unique<SliderAttachment>(apvts, "outputEq2500", outputEq2500);
    outputEq6300Attachment = std::make_unique<SliderAttachment>(apvts, "outputEq6300", outputEq6300);
    outputEq12000Attachment = std::make_unique<SliderAttachment>(apvts, "outputEq12000", outputEq12000);
    outputEqQ63Attachment = std::make_unique<SliderAttachment>(apvts, "outputEqQ63", outputEqQ63);
    outputEqQ160Attachment = std::make_unique<SliderAttachment>(apvts, "outputEqQ160", outputEqQ160);
    outputEqQ400Attachment = std::make_unique<SliderAttachment>(apvts, "outputEqQ400", outputEqQ400);
    outputEqQ1000Attachment = std::make_unique<SliderAttachment>(apvts, "outputEqQ1000", outputEqQ1000);
    outputEqQ2500Attachment = std::make_unique<SliderAttachment>(apvts, "outputEqQ2500", outputEqQ2500);
    outputEqQ6300Attachment = std::make_unique<SliderAttachment>(apvts, "outputEqQ6300", outputEqQ6300);
    outputEqQ12000Attachment = std::make_unique<SliderAttachment>(apvts, "outputEqQ12000", outputEqQ12000);
    outputEqHpHzAttachment = std::make_unique<SliderAttachment>(apvts, "outputEqHpHz", outputEqHpHz);
    outputEqLpHzAttachment = std::make_unique<SliderAttachment>(apvts, "outputEqLpHz", outputEqLpHz);
    for (size_t i = 0; i < rigModuleDescriptors.size(); ++i)
        rigModuleAttachments[i] = std::make_unique<ButtonAttachment>(apvts, rigModuleDescriptors[i].paramId, rigModuleButtons[i]);
    rigMaxPointsAttachment = std::make_unique<SliderAttachment>(apvts, "rigMaxPoints", rigMaxPointsSlider);
    delayTimeAttachment = std::make_unique<SliderAttachment>(apvts, "delayTimeMs", delayTime);
    delayFeedbackAttachment = std::make_unique<SliderAttachment>(apvts, "delayFeedback", delayFeedback);
    delayMixAttachment = std::make_unique<SliderAttachment>(apvts, "delayMix", delayMix);
    delayToneAttachment = std::make_unique<SliderAttachment>(apvts, "delayTone", delayTone);
    delayPitchAttachment = std::make_unique<SliderAttachment>(apvts, "delayPitch", delayPitch);
    cabMicBlendAttachment = std::make_unique<SliderAttachment>(apvts, "cabMicBlend", cabMicBlend);
    amp4OctaveMixAttachment = std::make_unique<SliderAttachment>(apvts, "amp4LowOctaveMix", amp4OctaveMix);
    amp4DirectMixAttachment = std::make_unique<SliderAttachment>(apvts, "amp4DirectMix", amp4DirectMix);
    amp4CleanMixAttachment = std::make_unique<SliderAttachment>(apvts, "amp4CleanAmpMix", amp4CleanMix);

    rigResetButton.onClick = [this] {
        auto& state = pluginProcessor.getAPVTS();
        for (const auto& module : rigModuleDescriptors)
        {
            if (auto* param = state.getParameter(module.paramId))
                param->setValueNotifyingHost(module.defaultEnabled ? 1.0f : 0.0f);
        }
        if (auto* maxParam = state.getParameter("rigMaxPoints"))
            maxParam->setValueNotifyingHost(static_cast<float>(defaultRigMaxPoints));
    };

    uiModeButton.onClick = [this] {
        showAdvancedTopControls = !showAdvancedTopControls;
        refreshTopControlVisibility();
        resized();
        repaint();
    };

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
    loadReverbIRButton.onClick = [this] {
        const auto defaultDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Reverb IRs");
        reverbChooser = std::make_unique<juce::FileChooser>("Load reverb IR", defaultDir, "*.wav;*.aiff;*.aif");
        reverbChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                   [this](const juce::FileChooser& chooser) {
                                       const auto selected = chooser.getResult();
                                       if (selected.existsAsFile())
                                           pluginProcessor.loadReverbIR(selected);
                                       refreshIRStatusLabel();
                                   });
    };
    clearReverbIRButton.onClick = [this] {
        pluginProcessor.clearReverbIR();
        refreshIRStatusLabel();
    };
    speakerGreenbackButton.onClick = [this] { setCabSpeakerFromButton(0); };
    speakerV30Button.onClick = [this] { setCabSpeakerFromButton(1); };
    speakerCreambackButton.onClick = [this] { setCabSpeakerFromButton(2); };
    speakerG12TButton.onClick = [this] { setCabSpeakerFromButton(3); };
    speakerJensenButton.onClick = [this] { setCabSpeakerFromButton(4); };
    mic57Button.onClick = [this] { setCabMicFromButton(0); };
    micU87Button.onClick = [this] { setCabMicFromButton(1); };
    micU47FetButton.onClick = [this] { setCabMicFromButton(2); };
    micRoyerButton.onClick = [this] { setCabMicFromButton(3); };
    mic421Button.onClick = [this] { setCabMicFromButton(4); };
    speakerBGreenbackButton.onClick = [this] { setCabSpeakerBFromButton(0); };
    speakerBV30Button.onClick = [this] { setCabSpeakerBFromButton(1); };
    speakerBCreambackButton.onClick = [this] { setCabSpeakerBFromButton(2); };
    speakerBG12TButton.onClick = [this] { setCabSpeakerBFromButton(3); };
    speakerBJensenButton.onClick = [this] { setCabSpeakerBFromButton(4); };
    micB57Button.onClick = [this] { setCabMicBFromButton(0); };
    micBU87Button.onClick = [this] { setCabMicBFromButton(1); };
    micBU47FetButton.onClick = [this] { setCabMicBFromButton(2); };
    micBRoyerButton.onClick = [this] { setCabMicBFromButton(3); };
    micB421Button.onClick = [this] { setCabMicBFromButton(4); };
    tabTunerButton.onClick = [this] { setActiveTab(0); };
    tabStompsButton.onClick = [this] { setActiveTab(1); };
    tabAmpButton.onClick = [this] { setActiveTab(2); };
    tabCabButton.onClick = [this] { setActiveTab(3); };
    tabPostColorButton.onClick = [this] { setActiveTab(4); };
    tabSpaceButton.onClick = [this] { setActiveTab(5); };
    tabMidiButton.onClick = [this] { setActiveTab(6); };
    midiTemplateRhythmButton.onClick = [this] { applyMidiTemplate(0); };
    midiTemplateLeadButton.onClick = [this] { applyMidiTemplate(1); };
    midiTemplateAmbientButton.onClick = [this] { applyMidiTemplate(2); };

    profile1NameEditor.onReturnKey = [this] { commitProfileName(1, profile1NameEditor.getText()); };
    profile2NameEditor.onReturnKey = [this] { commitProfileName(2, profile2NameEditor.getText()); };
    profile3NameEditor.onReturnKey = [this] { commitProfileName(3, profile3NameEditor.getText()); };

    profile1NameEditor.onFocusLost = [this] { commitProfileName(1, profile1NameEditor.getText()); };
    profile2NameEditor.onFocusLost = [this] { commitProfileName(2, profile2NameEditor.getText()); };
    profile3NameEditor.onFocusLost = [this] { commitProfileName(3, profile3NameEditor.getText()); };

    tunerTransposeAttachment = std::make_unique<SliderAttachment>(apvts, "tunerTranspose", tunerTranspose);

    const auto targetIds = pluginProcessor.getMappableParameterIds();
    const auto targetNames = pluginProcessor.getMappableParameterNames();
    for (auto& row : midiMapRows)
    {
        row.targetBox.addItem("None", 1);
        for (int i = 0; i < targetNames.size(); ++i)
            row.targetBox.addItem(targetNames[i], i + 2);
    }
    for (size_t i = 0; i < midiMapRows.size(); ++i)
    {
        auto& row = midiMapRows[i];
        row.enabledButton.onClick = [this, i] { commitMidiMapRow(static_cast<int>(i)); };
        row.ccSlider.onValueChange = [this, i] { commitMidiMapRow(static_cast<int>(i)); };
        row.targetBox.onChange = [this, i] { commitMidiMapRow(static_cast<int>(i)); };
        row.modeBox.onChange = [this, i] { commitMidiMapRow(static_cast<int>(i)); };
        row.minSlider.onValueChange = [this, i] { commitMidiMapRow(static_cast<int>(i)); };
        row.maxSlider.onValueChange = [this, i] { commitMidiMapRow(static_cast<int>(i)); };
        row.invertButton.onClick = [this, i] { commitMidiMapRow(static_cast<int>(i)); };
    }

    refreshProfileNameEditors();
    refreshGuitarProfileButtons();
    bindProfileEditControls();
    bindAmpPreampControl();
    refreshPreampHeatLabel();
    refreshTopControlVisibility();
    refreshCabSelectorButtons();
    refreshMidiMapPanel();
    refreshIRStatusLabel();
    refreshProfileIoStatusLabel("Profile JSON: ready");
    setActiveTab(2); // Start on Amp tab

    startTimerHz(10);
}

BackhouseAmpSimAudioProcessorEditor::~BackhouseAmpSimAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void BackhouseAmpSimAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient bgGrad(bgTop, 0.0f, 0.0f, bgBottom, 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    auto groupBounds = [](std::initializer_list<juce::Component*> comps) {
        juce::Rectangle<int> r;
        bool first = true;
        for (auto* c : comps)
        {
            if (c == nullptr || !c->isVisible())
                continue;
            if (first)
            {
                r = c->getBounds();
                first = false;
            }
            else
            {
                r = r.getUnion(c->getBounds());
            }
        }
        return r;
    };

    auto drawPanel = [&g](juce::Rectangle<int> r) {
        if (r.isEmpty())
            return;
        auto panel = r.expanded(8, 10).toFloat();
        // Subtle top-to-bottom gradient — deep blue-black
        juce::ColourGradient panelGrad(juce::Colour(0xff14161e), panel.getX(), panel.getY(),
                                       juce::Colour(0xff0e0f16), panel.getX(), panel.getBottom(), false);
        g.setGradientFill(panelGrad);
        g.fillRoundedRectangle(panel, 10.0f);
        g.setColour(panelOutline.withAlpha(0.9f));
        g.drawRoundedRectangle(panel, 10.0f, 1.2f);
    };

    // Amp section panel: same dark gradient with a coloured 2px accent bar at the top
    auto drawAmpPanel = [&g](juce::Rectangle<int> r, juce::Colour accentColour) {
        if (r.isEmpty())
            return;
        auto panel = r.expanded(8, 10).toFloat();
        juce::ColourGradient panelGrad(juce::Colour(0xff14161e), panel.getX(), panel.getY(),
                                       juce::Colour(0xff0e0f16), panel.getX(), panel.getBottom(), false);
        g.setGradientFill(panelGrad);
        g.fillRoundedRectangle(panel, 10.0f);
        g.setColour(panelOutline.withAlpha(0.9f));
        g.drawRoundedRectangle(panel, 10.0f, 1.2f);
        // Coloured top accent line
        g.setColour(accentColour.withAlpha(0.85f));
        g.drawLine(panel.getX() + 12.0f, panel.getY() + 1.6f,
                   panel.getRight() - 12.0f, panel.getY() + 1.6f, 2.2f);
    };

    // Top row panel: amp selector + profile selectors
    drawPanel(groupBounds({ &ampSelector, &profileNeutralButton, &profile1Button, &profile2Button, &profile3Button, &pitchLabel, &uiModeButton,
                            &tabAmpButton, &tabTunerButton, &tabStompsButton, &tabCabButton, &tabSpaceButton, &tabPostColorButton, &tabMidiButton }));
    // Status/controls row panel
    drawPanel(groupBounds({ &inputBoostButton, &cabEnabledButton, &reverbEnabledButton,
                            &irStatusLabel, &preampHeatLabel, &meterInputLabel, &inputMeterBar, &meterOutputLabel, &outputMeterBar }));
    // Advanced row panel (tuner toggle only)
    if (showAdvancedTopControls)
        drawPanel(groupBounds({ &tunerButton }));
    // Amp tab panels
    drawPanel(groupBounds({ &importProfilesButton, &exportProfilesButton,
                            &profile1NameEditor, &profile2NameEditor, &profile3NameEditor, &profileEditLabel, &profileIoStatusLabel,
                            &profilePickupOutput, &profileBrightness, &profileLowEnd, &profileGateTrim }));
    // Three separate panels — one per section — each with a section-colour top accent bar
    {
        auto signalBounds = groupBounds({ &inputGain, &ampPreampGain });
        auto ampEqBounds  = groupBounds({ &sub, &low, &mid, &high, &presence });
        auto outputBounds = groupBounds({ &gateThreshold, &output });
        if (!signalBounds.isEmpty())
        {
            drawAmpPanel(signalBounds.withBottom(signalBounds.getBottom() + 24), juce::Colour(0xffd86b2e));
            drawAmpPanel(ampEqBounds.withBottom(ampEqBounds.getBottom() + 24),   juce::Colour(0xffd0a047));
            drawAmpPanel(outputBounds.withBottom(outputBounds.getBottom() + 24), juce::Colour(0xff9eb79f));
        }
    }
    drawPanel(groupBounds({ &amp4OctaveMix, &amp4DirectMix, &amp4CleanMix }));
    drawPanel(groupBounds({ &rigModuleButtons[0], &rigModuleButtons[1], &rigModuleButtons[2], &rigModuleButtons[3], &rigModuleButtons[4],
                            &rigModuleButtons[5], &rigModuleButtons[6], &rigModuleButtons[7], &rigModuleButtons[8],
                            &rigPointsLabel, &rigMaxPointsSlider, &rigResetButton }));

    // ── Logo header strip (80px) ────────────────────────────────────────────
    {
        auto logoStrip = getLocalBounds().removeFromTop(80).toFloat();

        // Subtle dark panel behind the logo strip
        g.setColour(juce::Colour(0xff0c0d12));
        g.fillRect(logoStrip);
        g.setColour(panelOutline.withAlpha(0.6f));
        g.drawLine(logoStrip.getX(), logoStrip.getBottom(),
                   logoStrip.getRight(), logoStrip.getBottom(), 1.2f);

        // Logo image — square, left-aligned with a small margin
        const float logoH = 76.0f;
        const float logoW = logoImage.isValid()
            ? logoH * static_cast<float>(logoImage.getWidth()) / static_cast<float>(logoImage.getHeight())
            : 0.0f;
        if (logoImage.isValid())
        {
            g.drawImage(logoImage,
                        juce::Rectangle<float>(8.0f, 2.0f, logoW, logoH),
                        juce::RectanglePlacement::centred,
                        false);
        }

        // "Amp Sim" subtitle to the right of the logo
        const float textX = 8.0f + logoW + 10.0f;
        if (dharmaPunkTypeface != nullptr)
            g.setFont(juce::Font(juce::FontOptions(dharmaPunkTypeface).withHeight(30.0f)));
        else
            g.setFont(30.0f);
        g.setColour(textMuted);
        g.drawText("Amp Sim",
                   juce::Rectangle<float>(textX, 0.0f, 220.0f, 80.0f),
                   juce::Justification::centredLeft, false);
    }

    // Output-reactive pilot light — top-right corner of the logo strip
    const float meter = juce::jlimit(0.0f, 1.0f, static_cast<float>(outputMeterValue));
    const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(0.006 * juce::Time::getMillisecondCounterHiRes()));
    const float intensity = juce::jlimit(0.08f, 1.0f, 0.14f + 0.86f * meter);
    const float glow = intensity * (0.78f + 0.22f * pulse);

    const float cx = static_cast<float>(getWidth()) - 22.0f;
    const float cy = 40.0f;

    g.setColour(juce::Colour(0xffff1493).withAlpha(0.22f * glow));
    g.fillEllipse(cx - 18.0f, cy - 18.0f, 36.0f, 36.0f);
    g.setColour(juce::Colour(0xff00d4ff).withAlpha(0.32f * glow));
    g.fillEllipse(cx - 12.0f, cy - 12.0f, 24.0f, 24.0f);
    g.setColour(juce::Colour(0xfff3f6ff).withAlpha(0.85f * glow));
    g.fillEllipse(cx - 6.0f, cy - 6.0f, 12.0f, 12.0f);
    g.setColour(juce::Colour(0xff2d3240).withAlpha(0.95f));
    g.drawEllipse(cx - 6.0f, cy - 6.0f, 12.0f, 12.0f, 1.0f);

    if (activeTabIndex == 0 && !tunerNeedleArea.isEmpty())
    {
        auto area = tunerNeedleArea.reduced(10);
        g.setColour(juce::Colour(0xff0f1117));
        g.fillRoundedRectangle(area.toFloat(), 10.0f);
        g.setColour(juce::Colour(0xff2b2c34));
        g.drawRoundedRectangle(area.toFloat(), 10.0f, 1.2f);

        auto arcArea = area.reduced(26);
        // Clamp radius so the arc stays inside the box regardless of window width
        const float r = juce::jmin(static_cast<float>(arcArea.getWidth()) * 0.5f,
                                   static_cast<float>(arcArea.getHeight()) * 0.96f);
        const float cxA = static_cast<float>(arcArea.getCentreX());
        const float cyA = static_cast<float>(arcArea.getBottom());
        const float start = juce::MathConstants<float>::pi;
        const float end = juce::MathConstants<float>::twoPi;

        // Background: dark in-tune zone indicator
        juce::Path tuneZone;
        const float zoneHalfSpan = (10.0f / 100.0f) * (end - start);
        const float zoneCenter = start + 0.5f * (end - start);
        tuneZone.addCentredArc(cxA, cyA, r * 0.7f, r * 0.7f, 0.0f,
                               zoneCenter - zoneHalfSpan, zoneCenter + zoneHalfSpan, true);
        tuneZone.addCentredArc(cxA, cyA, r * 0.94f, r * 0.94f, 0.0f,
                               zoneCenter + zoneHalfSpan, zoneCenter - zoneHalfSpan, false);
        tuneZone.closeSubPath();
        g.setColour(juce::Colour(0xff00ff88).withAlpha(0.10f));
        g.fillPath(tuneZone);

        // Neon glow: wide soft arc pass before the crisp arc
        {
            juce::Path glowArc;
            glowArc.addCentredArc(cxA, cyA, r, r, 0.0f, start, end, true);
            g.setColour(textMuted.withAlpha(0.22f));
            g.strokePath(glowArc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));
        }
        g.setColour(textMuted);
        juce::Path arc;
        arc.addCentredArc(cxA, cyA, r, r, 0.0f, start, end, true);
        g.strokePath(arc, juce::PathStrokeType(2.0f));

        for (int i = -5; i <= 5; ++i)
        {
            const float t = static_cast<float>(i + 5) / 10.0f;
            const float a = start + t * (end - start);
            const float r0 = r * 0.84f;
            const float r1 = r * 0.94f;
            g.setColour(i == 0 ? juce::Colour(0xff00ff88) : textMuted.withAlpha(0.7f));
            g.drawLine(cxA + r0 * std::cos(a), cyA + r0 * std::sin(a),
                       cxA + r1 * std::cos(a), cyA + r1 * std::sin(a),
                       i == 0 ? 2.5f : 1.0f);
        }

        const float clampedCents = juce::jlimit(-50.0f, 50.0f, tunerNeedleCents);
        const float norm = (clampedCents + 50.0f) / 100.0f;
        const float angle = start + norm * (end - start);
        const float rN = r * 0.88f;
        const bool inTune = std::abs(clampedCents) <= 5.0f;
        const juce::Colour needleCol = inTune ? juce::Colour(0xff00ff88) : juce::Colour(0xffe98c55);
        // Needle neon glow pass
        g.setColour(needleCol.withAlpha(0.28f));
        g.drawLine(cxA, cyA, cxA + rN * std::cos(angle), cyA + rN * std::sin(angle), 7.0f);
        g.setColour(needleCol);
        g.drawLine(cxA, cyA, cxA + rN * std::cos(angle), cyA + rN * std::sin(angle), 3.0f);
        g.fillEllipse(cxA - 5.0f, cyA - 5.0f, 10.0f, 10.0f);

        // Note name in Dharma Punk 32pt
        if (!tunerNeedleNote.isEmpty() && tunerNeedleNote != "--")
        {
            if (dharmaPunkTypeface != nullptr)
                g.setFont(juce::Font(juce::FontOptions(dharmaPunkTypeface).withHeight(44.0f)));
            else
                g.setFont(juce::Font(juce::FontOptions().withHeight(44.0f).withStyle("Bold")));
            g.setColour(needleCol);
            g.drawText(tunerNeedleNote, juce::Rectangle<int>(
                static_cast<int>(cxA) - 60, static_cast<int>(cyA) - 52, 120, 46),
                juce::Justification::centred, false);
        }
    }

    if (activeTabIndex == 5 && !outputSpectrumArea.isEmpty())
    {
        auto area = outputSpectrumArea.reduced(6).toFloat();
        g.setColour(juce::Colour(0xff15110f));
        g.fillRoundedRectangle(area, 8.0f);
        g.setColour(panelOutline);
        g.drawRoundedRectangle(area, 8.0f, 1.0f);

        const float w = area.getWidth() / static_cast<float>(juce::jmax(1, static_cast<int>(outputSpectrumValues.size())));
        for (size_t i = 0; i < outputSpectrumValues.size(); ++i)
        {
            const float v = juce::jlimit(0.0f, 1.0f, outputSpectrumValues[i]);
            const float h = v * (area.getHeight() - 12.0f);
            const float x = area.getX() + static_cast<float>(i) * w + 2.0f;
            const float y = area.getBottom() - h - 4.0f;
            g.setColour(juce::Colour(0xff79b6d4).withAlpha(0.35f + 0.55f * v));
            g.fillRoundedRectangle(x, y, juce::jmax(2.0f, w - 4.0f), h, 2.0f);
        }
    }
    // ── Layout: compute main content rectangle ──────────────────────────────
    auto area = getLocalBounds();
    area.removeFromTop(80); // Logo header strip (drawn in paint())
    // ── Row 1: Amp selector · Guitar profile buttons · Pitch · Tabs · Mode ──
    auto topRow = area.removeFromTop(36);
    uiModeButton.setBounds(topRow.removeFromRight(96).reduced(2));
    tabMidiButton.setBounds(topRow.removeFromRight(78).reduced(2));
    tabSpaceButton.setBounds(topRow.removeFromRight(66).reduced(2));
    tabPostColorButton.setBounds(topRow.removeFromRight(88).reduced(2));
    tabCabButton.setBounds(topRow.removeFromRight(74).reduced(2));
    tabAmpButton.setBounds(topRow.removeFromRight(56).reduced(2));
    tabStompsButton.setBounds(topRow.removeFromRight(74).reduced(2));
    tabTunerButton.setBounds(topRow.removeFromRight(68).reduced(2));
    topRow.removeFromRight(10);

    ampSelector.setBounds(topRow.removeFromLeft(120).reduced(0, 4));
    topRow.removeFromLeft(8);
    profileNeutralButton.setBounds(topRow.removeFromLeft(76).reduced(2));
    profile1Button.setBounds(topRow.removeFromLeft(76).reduced(2));
    profile2Button.setBounds(topRow.removeFromLeft(76).reduced(2));
    profile3Button.setBounds(topRow.removeFromLeft(76).reduced(2));
    topRow.removeFromLeft(8);
    pitchLabel.setBounds(topRow);

    // ── Row 2: Quick toggles · IR status · Preamp heat · Level meters ────────
    area.removeFromTop(4);
    auto ctrlRow = area.removeFromTop(28);
    inputBoostButton.setBounds(ctrlRow.removeFromLeft(96));
    cabEnabledButton.setBounds(ctrlRow.removeFromLeft(82));
    reverbEnabledButton.setBounds(ctrlRow.removeFromLeft(76));
    ctrlRow.removeFromLeft(10);
    meterOutputLabel.setBounds(ctrlRow.removeFromRight(36));
    outputMeterBar.setBounds(ctrlRow.removeFromRight(96).reduced(2, 4));
    ctrlRow.removeFromRight(6);
    meterInputLabel.setBounds(ctrlRow.removeFromRight(28));
    inputMeterBar.setBounds(ctrlRow.removeFromRight(96).reduced(2, 4));
    ctrlRow.removeFromRight(8);
    preampHeatLabel.setBounds(ctrlRow.removeFromRight(130));
    ctrlRow.removeFromRight(6);
    irStatusLabel.setBounds(ctrlRow);

    // ── Advanced mode: one extra row (tuner toggle + pitch context) ──────────
    if (showAdvancedTopControls)
    {
        area.removeFromTop(4);
        auto advRow = area.removeFromTop(30);
        tunerButton.setBounds(advRow.removeFromLeft(108));
    }

    area.removeFromTop(6);

#if JucePlugin_Build_Standalone
    auto benchArea = area.removeFromBottom(332);
#endif

    if (activeTabIndex != 2)
    {
        auto panel = area.reduced(6);
        if (activeTabIndex == 0)
        {
            // Constrain the gauge to a compact centered rect so the arc radius stays sane
            const int tunerGaugeW = juce::jmin(panel.getWidth() - 20, 500);
            const int tunerGaugeH = 240;
            tunerNeedleArea = panel.removeFromTop(tunerGaugeH + 20)
                                   .withSizeKeepingCentre(tunerGaugeW, tunerGaugeH);
            panel.removeFromTop(8);
            tunerNoteLabel.setBounds(panel.removeFromTop(24).withSizeKeepingCentre(tunerGaugeW, 24));
            tunerDetuneLabel.setBounds(panel.removeFromTop(24).withSizeKeepingCentre(tunerGaugeW, 24));
            panel.removeFromTop(8);
            tunerTransposeLabel.setBounds(panel.removeFromTop(22).withSizeKeepingCentre(tunerGaugeW, 22));
            tunerTranspose.setBounds(panel.removeFromTop(28).withSizeKeepingCentre(360, 28));
        }
        else if (activeTabIndex == 1)
        {
            // ── Pedalboard layout: 4 pedals top, 3 pedals bottom ─────────────
            constexpr int pedalW = 250;
            constexpr int pedalH = 220;
            constexpr int hGap   = 30;
            constexpr int vGap   = 24;
            const int panelX = panel.getX();
            const int panelY = panel.getY();
            const int panelW = panel.getWidth();
            const int topRowW = 4 * pedalW + 3 * hGap;   // 1090
            const int botRowW = 3 * pedalW + 2 * hGap;   // 810
            const int topRowX = panelX + (panelW - topRowW) / 2;
            const int botRowX = panelX + (panelW - botRowW) / 2;
            const int topRowY = panelY + 10;
            const int botRowY = topRowY + pedalH + vGap;

            // Pedal origins: top row = COMP, WOW, TREM, CRYBABY; bottom = TUBE SCR, PHASER, WHAMMY
            auto pedalBounds = [&](int row, int col) {
                const int x = (row == 0 ? topRowX : botRowX) + col * (pedalW + hGap);
                const int y = (row == 0 ? topRowY : botRowY);
                return juce::Rectangle<int>(x, y, pedalW, pedalH);
            };

            // Draw pedal containers
            struct PedalDef { juce::Rectangle<int> r; const char* name; juce::Colour colour; };
            const PedalDef pedals[] = {
                { pedalBounds(0,0), "COMP",      juce::Colour(0xff3dcc66) },
                { pedalBounds(0,1), "WOW/WAH",   juce::Colour(0xffdd3355) },
                { pedalBounds(0,2), "TREM",      juce::Colour(0xff22bbcc) },
                { pedalBounds(0,3), "CRYBABY",   juce::Colour(0xffd4a800) },
                { pedalBounds(1,0), "TUBE SCR",  juce::Colour(0xffcc6b2e) },
                { pedalBounds(1,1), "PHASER",    juce::Colour(0xff4477dd) },
                { pedalBounds(1,2), "WHAMMY",    juce::Colour(0xff9933cc) },
            };

            for (const auto& pd : pedals)
            {
                auto bf = pd.r.toFloat();
                g.setColour(juce::Colour(0xff141820));
                g.fillRoundedRectangle(bf, 14.0f);
                g.setColour(pd.colour.withAlpha(0.60f));
                g.drawRoundedRectangle(bf.reduced(1.0f), 14.0f, 2.5f);
                // Pedal name label
                if (dharmaPunkTypeface != nullptr)
                    g.setFont(juce::Font(juce::FontOptions(dharmaPunkTypeface).withHeight(13.0f)));
                else
                    g.setFont(18.0f);
                g.setColour(pd.colour);
                g.drawText(pd.name, pd.r.getX(), pd.r.getY() + 6, pd.r.getWidth(), 24,
                           juce::Justification::centred, true);
            }

            // Helper: knob zone within a pedal (below name label, above footswitch area)
            const int nameH    = 34;
            const int switchH  = 50;
            const int knobZoneH = pedalH - nameH - switchH;  // 136

            // ── COMP pedal: stompInputGain + sustain + level + mix ────────────
            {
                const auto& r = pedals[0].r;
                const int kw  = pedalW / 4;
                const int ky  = r.getY() + nameH;
                juce::Slider* ks[] = { &stompInputGain, &stompCompSustain, &stompCompLevel, &stompCompMix };
                for (int i = 0; i < 4; ++i)
                    ks[i]->setBounds(r.getX() + i * kw + 3, ky + 4, kw - 6, knobZoneH - 4);
                stompCompEnabledButton.setBounds(r.getX() + pedalW / 2 - 26, r.getY() + pedalH - switchH + 4, 52, 42);
            }

            // ── WOW/WAH pedal: wowPosition + wowMix + sub-controls ───────────
            {
                const auto& r = pedals[1].r;
                const int kw  = pedalW / 2;
                const int ky  = r.getY() + nameH;
                const int knobH = knobZoneH - 26;   // leave room for sub-control row
                wowPosition.setBounds(r.getX() + 3,       ky + 4, kw - 6, knobH - 4);
                wowMix.setBounds     (r.getX() + kw + 3,  ky + 4, kw - 6, knobH - 4);
                // Sub-control row: midi toggle | momentary | mode selector
                const int subY = r.getY() + nameH + knobH;
                wowMidiEnableButton.setBounds(r.getX() + 4,       subY + 3, 68, 20);
                wowMomentaryButton.setBounds (r.getX() + 76,      subY + 3, 80, 20);
                wowModeSelector.setBounds    (r.getX() + 160,     subY + 2, 84, 22);
                wowEnabledButton.setBounds(r.getX() + pedalW / 2 - 26, r.getY() + pedalH - switchH + 4, 52, 42);
            }

            // ── TREM pedal: amp1TremRate + amp1TremDepth ──────────────────────
            {
                const auto& r = pedals[2].r;
                const int kw  = pedalW / 2;
                const int ky  = r.getY() + nameH;
                amp1TremRate.setBounds (r.getX() + 3,      ky + 4, kw - 6, knobZoneH - 4);
                amp1TremDepth.setBounds(r.getX() + kw + 3, ky + 4, kw - 6, knobZoneH - 4);
                amp1TremEnabledButton.setBounds(r.getX() + pedalW / 2 - 26, r.getY() + pedalH - switchH + 4, 52, 42);
            }

            // ── CRYBABY pedal: wahPosition + wahMix ──────────────────────────
            {
                const auto& r = pedals[3].r;
                const int kw  = pedalW / 2;
                const int ky  = r.getY() + nameH;
                wahPosition.setBounds(r.getX() + 3,      ky + 4, kw - 6, knobZoneH - 4);
                wahMix.setBounds     (r.getX() + kw + 3, ky + 4, kw - 6, knobZoneH - 4);
                wahEnabledButton.setBounds(r.getX() + pedalW / 2 - 26, r.getY() + pedalH - switchH + 4, 52, 42);
            }

            // ── TUBE SCR pedal: tsDrive + tsTone + tsLevel ───────────────────
            {
                const auto& r = pedals[4].r;
                const int kw  = pedalW / 3;
                const int ky  = r.getY() + nameH;
                tsDrive.setBounds(r.getX() + 0 * kw + 3, ky + 4, kw - 6, knobZoneH - 4);
                tsTone.setBounds (r.getX() + 1 * kw + 3, ky + 4, kw - 6, knobZoneH - 4);
                tsLevel.setBounds(r.getX() + 2 * kw + 3, ky + 4, kw - 6, knobZoneH - 4);
                tsEnabledButton.setBounds(r.getX() + pedalW / 2 - 26, r.getY() + pedalH - switchH + 4, 52, 42);
            }

            // ── PHASER pedal: rate + depth + center + feedback + mix ─────────
            {
                const auto& r = pedals[5].r;
                const int kw  = pedalW / 5;
                const int ky  = r.getY() + nameH;
                phaserRate.setBounds    (r.getX() + 0 * kw + 2, ky + 4, kw - 4, knobZoneH - 4);
                phaserDepth.setBounds   (r.getX() + 1 * kw + 2, ky + 4, kw - 4, knobZoneH - 4);
                phaserCenter.setBounds  (r.getX() + 2 * kw + 2, ky + 4, kw - 4, knobZoneH - 4);
                phaserFeedback.setBounds(r.getX() + 3 * kw + 2, ky + 4, kw - 4, knobZoneH - 4);
                phaserMix.setBounds     (r.getX() + 4 * kw + 2, ky + 4, kw - 4, knobZoneH - 4);
                phaserEnabledButton.setBounds(r.getX() + pedalW / 2 - 26, r.getY() + pedalH - switchH + 4, 52, 42);
            }

            // ── WHAMMY pedal: position + mix + mode selector ─────────────────
            {
                const auto& r = pedals[6].r;
                const int kw  = pedalW / 2;
                const int ky  = r.getY() + nameH;
                const int knobH = knobZoneH - 26;
                whammyPosition.setBounds(r.getX() + 3,      ky + 4, kw - 6, knobH - 4);
                whammyMix.setBounds     (r.getX() + kw + 3, ky + 4, kw - 6, knobH - 4);
                const int subY = r.getY() + nameH + knobH;
                whammyModeSelector.setBounds(r.getX() + (pedalW - 130) / 2, subY + 2, 130, 22);
                whammyEnabledButton.setBounds(r.getX() + pedalW / 2 - 26, r.getY() + pedalH - switchH + 4, 52, 42);
            }
        }
        else if (activeTabIndex == 3)  // Cab/Mic
        {
            auto speakerRow = panel.removeFromTop(30);
            cabSpeakerLabel.setBounds(speakerRow.removeFromLeft(58));
            const int speakerButtonW = 94;
            speakerGreenbackButton.setBounds(speakerRow.removeFromLeft(speakerButtonW).reduced(2));
            speakerV30Button.setBounds(speakerRow.removeFromLeft(speakerButtonW).reduced(2));
            speakerCreambackButton.setBounds(speakerRow.removeFromLeft(speakerButtonW).reduced(2));
            speakerG12TButton.setBounds(speakerRow.removeFromLeft(speakerButtonW).reduced(2));
            speakerJensenButton.setBounds(speakerRow.removeFromLeft(speakerButtonW).reduced(2));
            speakerRow.removeFromLeft(6);
            micAOffAxisButton.setBounds(speakerRow.removeFromLeft(108));
            speakerRow.removeFromLeft(6);
            loadIRButton.setBounds(speakerRow.removeFromLeft(90).reduced(2));
            clearIRButton.setBounds(speakerRow.removeFromLeft(90).reduced(2));

            panel.removeFromTop(3);
            auto micRow = panel.removeFromTop(30);
            cabMicLabel.setBounds(micRow.removeFromLeft(58));
            const int micButtonW = 88;
            mic57Button.setBounds(micRow.removeFromLeft(micButtonW).reduced(2));
            micU87Button.setBounds(micRow.removeFromLeft(micButtonW).reduced(2));
            micU47FetButton.setBounds(micRow.removeFromLeft(micButtonW).reduced(2));
            micRoyerButton.setBounds(micRow.removeFromLeft(micButtonW).reduced(2));
            mic421Button.setBounds(micRow.removeFromLeft(micButtonW).reduced(2));
            micRow.removeFromLeft(6);
            micBOffAxisButton.setBounds(micRow.removeFromLeft(108));
            micRow.removeFromLeft(6);
            loadReverbIRButton.setBounds(micRow.removeFromLeft(98).reduced(2));
            clearReverbIRButton.setBounds(micRow.removeFromLeft(98).reduced(2));

            panel.removeFromTop(3);
            auto spkB = panel.removeFromTop(30);
            cabSpeakerBLabel.setBounds(spkB.removeFromLeft(58));
            speakerBGreenbackButton.setBounds(spkB.removeFromLeft(speakerButtonW).reduced(2));
            speakerBV30Button.setBounds(spkB.removeFromLeft(speakerButtonW).reduced(2));
            speakerBCreambackButton.setBounds(spkB.removeFromLeft(speakerButtonW).reduced(2));
            speakerBG12TButton.setBounds(spkB.removeFromLeft(speakerButtonW).reduced(2));
            speakerBJensenButton.setBounds(spkB.removeFromLeft(speakerButtonW).reduced(2));

            panel.removeFromTop(3);
            auto micB = panel.removeFromTop(30);
            cabMicBLabel.setBounds(micB.removeFromLeft(58));
            micB57Button.setBounds(micB.removeFromLeft(micButtonW).reduced(2));
            micBU87Button.setBounds(micB.removeFromLeft(micButtonW).reduced(2));
            micBU47FetButton.setBounds(micB.removeFromLeft(micButtonW).reduced(2));
            micBRoyerButton.setBounds(micB.removeFromLeft(micButtonW).reduced(2));
            micB421Button.setBounds(micB.removeFromLeft(micButtonW).reduced(2));

            panel.removeFromTop(10);
            auto cabKnobRow = panel.removeFromTop(110);
            cabMicBlend.setBounds(cabKnobRow.removeFromLeft(110).reduced(2));
            cabKnobRow.removeFromLeft(12);
            cabTone.setBounds(cabKnobRow.removeFromLeft(110).reduced(4));

            // ── Visual cab diagram ──────────────────────────────────────────
            if (panel.getHeight() > 80)
            {
                panel.removeFromTop(10);
                auto diagArea = panel.removeFromTop(juce::jmin(210, panel.getHeight() - 4));

                // Per-speaker accent colour (makes each type visually distinct)
                constexpr juce::uint32 spkColours[5] = {
                    0xff44cc88,  // Greenback — green
                    0xffff8844,  // V30 — orange
                    0xffe8c87a,  // Creamback — cream/amber
                    0xff4499ff,  // G12T-75 — blue
                    0xffaa66ff,  // Jensen — purple
                };
                // Per-mic accent colour
                constexpr juce::uint32 micColours[5] = {
                    0xff00d4ff,  // SM57 — cyan
                    0xff4477dd,  // U87 — blue
                    0xff9933cc,  // U47 FET — violet
                    0xffcc6b2e,  // Royer 122 — amber/ribbon
                    0xff3dcc66,  // MD421 — green
                };
                constexpr const char* spkShort[5] = { "Greenback", "V30", "Creamback", "G12T-75", "Jensen" };
                constexpr const char* micShort[5] = { "SM57", "U87", "U47 FET", "Royer 122", "MD421" };

                auto& apvts = pluginProcessor.getAPVTS();
                const int selSpkA  = juce::jlimit(0, 4, static_cast<int>(apvts.getRawParameterValue("cabSpeaker")->load()));
                const int selMicA  = juce::jlimit(0, 4, static_cast<int>(apvts.getRawParameterValue("cabMic")->load()));
                const int selSpkB  = juce::jlimit(0, 4, static_cast<int>(apvts.getRawParameterValue("cabSpeakerB")->load()));
                const int selMicB  = juce::jlimit(0, 4, static_cast<int>(apvts.getRawParameterValue("cabMicB")->load()));
                const bool offAxisA = apvts.getRawParameterValue("cabMicAOffAxis")->load() > 0.5f;
                const bool offAxisB = apvts.getRawParameterValue("cabMicBOffAxis")->load() > 0.5f;

                const double blendMin = cabMicBlend.getMinimum();
                const double blendMax = cabMicBlend.getMaximum();
                const float blendNorm = (blendMax > blendMin)
                    ? static_cast<float>((cabMicBlend.getValue() - blendMin) / (blendMax - blendMin))
                    : 0.5f;

                const int slotW = juce::jmin(280, (diagArea.getWidth() - 50) / 2);
                const int slotH = juce::jmin(170, diagArea.getHeight() - 30);
                const int diagCx = diagArea.getCentreX();
                const int slotGap = 30;
                const int aSlotX = diagCx - slotGap / 2 - slotW;
                const int bSlotX = diagCx + slotGap / 2;
                const int slotY  = diagArea.getY() + 4;

                auto drawCabSlot = [&](int sx, int sy, int sw, int sh,
                                       const char* spkName, const char* micName,
                                       bool offAxis,
                                       juce::Colour spkCol, juce::Colour micCol,
                                       const juce::String& chanLabel)
                {
                    const auto sf = juce::Rectangle<float>((float)sx, (float)sy, (float)sw, (float)sh);

                    // Slot background + border
                    g.setColour(juce::Colour(0xff0e1018));
                    g.fillRoundedRectangle(sf, 12.0f);
                    g.setColour(spkCol.withAlpha(0.45f));
                    g.drawRoundedRectangle(sf.reduced(0.5f), 12.0f, 1.6f);

                    // Channel label badge top-left
                    g.setColour(spkCol.withAlpha(0.18f));
                    g.fillRoundedRectangle((float)sx + 8.0f, (float)sy + 7.0f, 24.0f, 18.0f, 4.0f);
                    if (dharmaPunkTypeface != nullptr)
                        g.setFont(juce::Font(juce::FontOptions(dharmaPunkTypeface).withHeight(17.0f)));
                    else
                        g.setFont(16.0f);
                    g.setColour(spkCol);
                    g.drawText(chanLabel, sx + 8, sy + 6, 26, 22, juce::Justification::centred, true);

                    // Speaker cone — concentric ellipses, centred in left 55% of slot
                    const float coneCx = (float)sx + (float)sw * 0.40f;
                    const float coneCy = (float)sy + (float)sh * 0.52f;
                    const float maxR   = juce::jmin((float)sw * 0.32f, (float)sh * 0.43f);

                    // Shadow/glow halo
                    g.setColour(spkCol.withAlpha(0.06f));
                    g.fillEllipse(coneCx - maxR * 1.1f, coneCy - maxR * 1.1f, maxR * 2.2f, maxR * 2.2f);

                    // Cone rings (outer → inner, increasing opacity)
                    struct Ring { float frac; float alpha; };
                    constexpr Ring rings[] = { {1.00f,0.14f},{0.78f,0.22f},{0.58f,0.32f},{0.38f,0.45f},{0.20f,0.62f} };
                    for (const auto& rng : rings)
                    {
                        const float r = maxR * rng.frac;
                        g.setColour(spkCol.withAlpha(rng.alpha));
                        g.drawEllipse(coneCx - r, coneCy - r, r * 2.0f, r * 2.0f, 1.1f);
                    }
                    // Dust cap (solid fill)
                    const float dustR = maxR * 0.11f;
                    g.setColour(spkCol.withAlpha(0.70f));
                    g.fillEllipse(coneCx - dustR, coneCy - dustR, dustR * 2.0f, dustR * 2.0f);

                    // Mic position: on-axis = center, off-axis = shifted toward upper-right
                    const float micDist  = offAxis ? maxR * 0.54f : 0.0f;
                    const float micAng   = juce::MathConstants<float>::pi * 0.3f;  // ~54° from top
                    const float micX     = coneCx + micDist * std::sin(micAng);
                    const float micY     = coneCy - micDist * std::cos(micAng);

                    // Mic capsule body
                    const float mh = 16.0f, mw = 6.0f;
                    if (offAxis)
                    {
                        // Show off-axis dashed line from center to mic
                        g.setColour(micCol.withAlpha(0.28f));
                        float dashLengths[] = { 4.0f, 3.0f };
                        g.drawDashedLine(juce::Line<float>(coneCx, coneCy, micX, micY),
                                         dashLengths, 2, 0.8f);
                    }
                    // Glow
                    g.setColour(micCol.withAlpha(0.22f));
                    g.fillEllipse(micX - mw, micY - mh * 0.5f - 2.0f, mw * 2.0f, mh + 4.0f);
                    // Capsule fill
                    g.setColour(micCol.withAlpha(0.85f));
                    g.fillRoundedRectangle(micX - mw * 0.5f, micY - mh * 0.5f, mw, mh, mw * 0.5f);
                    g.setColour(micCol.brighter(0.3f));
                    g.drawRoundedRectangle(micX - mw * 0.5f, micY - mh * 0.5f, mw, mh, mw * 0.5f, 1.0f);
                    // Stand
                    g.setColour(micCol.withAlpha(0.40f));
                    g.drawLine(micX, micY + mh * 0.5f, micX, micY + mh, 1.0f);

                    // Speaker + mic name labels (right portion of slot)
                    const int labelX = sx + (int)((float)sw * 0.65f);
                    const int labelW = sw - (int)((float)sw * 0.65f) - 8;

                    if (dharmaPunkTypeface != nullptr)
                        g.setFont(juce::Font(juce::FontOptions(dharmaPunkTypeface).withHeight(14.0f)));
                    else
                        g.setFont(14.0f);
                    g.setColour(spkCol.brighter(0.1f));
                    g.drawText(spkName, labelX, (int)coneCy - 30, labelW, 17, juce::Justification::centredLeft, true);

                    g.setColour(micCol.brighter(0.1f));
                    g.drawText(micName, labelX, (int)coneCy - 10, labelW, 17, juce::Justification::centredLeft, true);

                    g.setColour(offAxis ? micCol.withAlpha(0.65f) : textMuted.withAlpha(0.5f));
                    if (punkaholicTypeface != nullptr)
                        g.setFont(juce::Font(juce::FontOptions(punkaholicTypeface).withHeight(12.0f)));
                    else
                        g.setFont(12.0f);
                    g.drawText(offAxis ? "off-axis" : "on-axis", labelX, (int)coneCy + 10, labelW, 15, juce::Justification::centredLeft, true);
                };

                const juce::Colour spkColA(spkColours[selSpkA]);
                const juce::Colour micColA(micColours[selMicA]);
                const juce::Colour spkColB(spkColours[selSpkB]);
                const juce::Colour micColB(micColours[selMicB]);

                drawCabSlot(aSlotX, slotY, slotW, slotH, spkShort[selSpkA], micShort[selMicA], offAxisA, spkColA, micColA, "A");
                drawCabSlot(bSlotX, slotY, slotW, slotH, spkShort[selSpkB], micShort[selMicB], offAxisB, spkColB, micColB, "B");

                // A/B blend bar below the slots
                const int barY = slotY + slotH + 10;
                const int barW = slotW * 2 + slotGap;
                const int barH = 7;
                const float aFrac = 1.0f - blendNorm;
                const float bFrac = blendNorm;

                g.setColour(juce::Colour(0xff1a1c28));
                g.fillRoundedRectangle((float)aSlotX, (float)barY, (float)barW, (float)barH, 3.5f);
                if (aFrac > 0.01f)
                {
                    g.setColour(spkColA.withAlpha(0.75f));
                    g.fillRoundedRectangle((float)aSlotX, (float)barY, barW * aFrac, (float)barH, 3.5f);
                }
                if (bFrac > 0.01f)
                {
                    g.setColour(spkColB.withAlpha(0.75f));
                    g.fillRoundedRectangle((float)(aSlotX + barW) - barW * bFrac, (float)barY, barW * bFrac, (float)barH, 3.5f);
                }
                g.setColour(panelOutline);
                g.drawRoundedRectangle((float)aSlotX, (float)barY, (float)barW, (float)barH, 3.5f, 0.8f);

                // A / B labels beside the blend bar
                if (punkaholicTypeface != nullptr)
                    g.setFont(juce::Font(juce::FontOptions(punkaholicTypeface).withHeight(9.5f)));
                else
                    g.setFont(9.5f);
                g.setColour(spkColA.withAlpha(0.8f));
                g.drawText("A", aSlotX - 16, barY - 1, 14, barH + 2, juce::Justification::centredRight, true);
                g.setColour(spkColB.withAlpha(0.8f));
                g.drawText("B", aSlotX + barW + 2, barY - 1, 14, barH + 2, juce::Justification::centredLeft, true);
            }
        }
        else if (activeTabIndex == 5)
        {
            // ── Space: Delay + Reverb only ────────────────────────────────
            auto row = panel.removeFromTop(34);
            reverbEnabledButton.setBounds(row.removeFromLeft(110));
            delayEnabledButton.setBounds(row.removeFromLeft(90));
            row.removeFromLeft(8);
            loadReverbIRButton.setBounds(row.removeFromLeft(120).reduced(2));
            clearReverbIRButton.setBounds(row.removeFromLeft(120).reduced(2));

            panel.removeFromTop(6);
            auto row1 = panel.removeFromTop(112);
            const int cols1 = 6;
            const int w1 = row1.getWidth() / cols1;
            reverbMix.setBounds(row1.removeFromLeft(w1).reduced(4));
            delayTime.setBounds(row1.removeFromLeft(w1).reduced(4));
            delayFeedback.setBounds(row1.removeFromLeft(w1).reduced(4));
            delayMix.setBounds(row1.removeFromLeft(w1).reduced(4));
            delayTone.setBounds(row1.removeFromLeft(w1).reduced(4));
            delayPitch.setBounds(row1.removeFromLeft(w1).reduced(4));

            panel.removeFromTop(4);
            auto rigToggleRow = panel.removeFromTop(56);
            const int moduleCount = static_cast<int>(rigModuleButtons.size());
            const int moduleWidth = rigToggleRow.getWidth() / juce::jmax(1, moduleCount);
            for (auto& button : rigModuleButtons)
                button.setBounds(rigToggleRow.removeFromLeft(moduleWidth).reduced(4));
            panel.removeFromTop(6);
            auto rigStatsRow = panel.removeFromTop(40);
            rigPointsLabel.setBounds(rigStatsRow.removeFromLeft(220));
            rigMaxPointsSlider.setBounds(rigStatsRow.removeFromLeft(420).reduced(4));
            rigResetButton.setBounds(rigStatsRow.removeFromLeft(120).reduced(2));

            panel.removeFromTop(4);
            outputSpectrumArea = panel.removeFromTop(94).reduced(2);
        }
        else if (activeTabIndex == 4)
        {
            // ── Post Color: 1176 + Studio EQ ─────────────────────────────
            auto row = panel.removeFromTop(34);
            output1176EnabledButton.setBounds(row.removeFromLeft(100));
            outputCompModeSelector.setBounds(row.removeFromLeft(90).reduced(2));
            row.removeFromLeft(16);
            outputEqEnabledButton.setBounds(row.removeFromLeft(90));
            outputEqHpEnabledButton.setBounds(row.removeFromLeft(56));
            outputEqLpEnabledButton.setBounds(row.removeFromLeft(56));

            panel.removeFromTop(6);
            auto row1 = panel.removeFromTop(112);
            const int cols1_pc = 11;
            const int w1_pc = row1.getWidth() / cols1_pc;
            output1176Input.setBounds(row1.removeFromLeft(w1_pc).reduced(4));
            output1176Release.setBounds(row1.removeFromLeft(w1_pc).reduced(4));
            output1176Mix.setBounds(row1.removeFromLeft(w1_pc).reduced(4));
            row1.removeFromLeft(w1_pc / 2); // visual gap between 1176 and EQ
            outputEq63.setBounds(row1.removeFromLeft(w1_pc).reduced(4));
            outputEq160.setBounds(row1.removeFromLeft(w1_pc).reduced(4));
            outputEq400.setBounds(row1.removeFromLeft(w1_pc).reduced(4));
            outputEq1000.setBounds(row1.removeFromLeft(w1_pc).reduced(4));
            outputEq2500.setBounds(row1.removeFromLeft(w1_pc).reduced(4));
            outputEq6300.setBounds(row1.removeFromLeft(w1_pc).reduced(4));
            outputEq12000.setBounds(row1.removeFromLeft(w1_pc).reduced(4));

            panel.removeFromTop(4);
            auto row2 = panel.removeFromTop(112);
            const int cols2_pc = 9;
            const int w2_pc = row2.getWidth() / cols2_pc;
            outputEqQ63.setBounds(row2.removeFromLeft(w2_pc).reduced(4));
            outputEqQ160.setBounds(row2.removeFromLeft(w2_pc).reduced(4));
            outputEqQ400.setBounds(row2.removeFromLeft(w2_pc).reduced(4));
            outputEqQ1000.setBounds(row2.removeFromLeft(w2_pc).reduced(4));
            outputEqQ2500.setBounds(row2.removeFromLeft(w2_pc).reduced(4));
            outputEqQ6300.setBounds(row2.removeFromLeft(w2_pc).reduced(4));
            outputEqQ12000.setBounds(row2.removeFromLeft(w2_pc).reduced(4));
            outputEqHpHz.setBounds(row2.removeFromLeft(w2_pc).reduced(4));
            outputEqLpHz.setBounds(row2.removeFromLeft(w2_pc).reduced(4));
        }
        else if (activeTabIndex == 6)  // MIDI Map
        {
            midiMapHelpLabel.setBounds(panel.removeFromTop(44));
            auto templateRow = panel.removeFromTop(30);
            midiTemplateRhythmButton.setBounds(templateRow.removeFromLeft(150).reduced(2));
            midiTemplateLeadButton.setBounds(templateRow.removeFromLeft(140).reduced(2));
            midiTemplateAmbientButton.setBounds(templateRow.removeFromLeft(160).reduced(2));
            panel.removeFromTop(6);
            for (auto& row : midiMapRows)
            {
                auto r = panel.removeFromTop(28);
                row.slotLabel.setBounds(r.removeFromLeft(28));
                row.enabledButton.setBounds(r.removeFromLeft(42));
                row.ccSlider.setBounds(r.removeFromLeft(110));
                row.targetBox.setBounds(r.removeFromLeft(170));
                row.modeBox.setBounds(r.removeFromLeft(120));
                row.minSlider.setBounds(r.removeFromLeft(110));
                row.maxSlider.setBounds(r.removeFromLeft(110));
                row.invertButton.setBounds(r.removeFromLeft(44));
                panel.removeFromTop(3);
            }
        }
    }
    else
    {
        // ── Profile name editors + Import/Export ─────────────────────────────────
        auto profileNameRow = area.removeFromTop(28);
        importProfilesButton.setBounds(profileNameRow.removeFromLeft(108).reduced(2));
        exportProfilesButton.setBounds(profileNameRow.removeFromLeft(108).reduced(2));
        profileNameRow.removeFromLeft(8);
        profile1NameEditor.setBounds(profileNameRow.removeFromLeft(116).reduced(2));
        profile2NameEditor.setBounds(profileNameRow.removeFromLeft(116).reduced(2));
        profile3NameEditor.setBounds(profileNameRow.removeFromLeft(116).reduced(2));
        profileNameRow.removeFromLeft(8);
        profileIoStatusLabel.setBounds(profileNameRow);

        area.removeFromTop(4);
        auto editLabelRow = area.removeFromTop(16);
        profileEditLabel.setBounds(editLabelRow);

        area.removeFromTop(6);

        // ── Profile offset knobs ──────────────────────────────────────────────────
        const int availableHeight = area.getHeight();
        const int profileKnobHeight = juce::jlimit(90, 115, availableHeight / 5);
        auto profileKnobs = area.removeFromTop(profileKnobHeight);
        const int profileKnobWidth = profileKnobs.getWidth() / 4;
        profilePickupOutput.setBounds(profileKnobs.removeFromLeft(profileKnobWidth).reduced(4));
        profileBrightness.setBounds(profileKnobs.removeFromLeft(profileKnobWidth).reduced(4));
        profileLowEnd.setBounds(profileKnobs.removeFromLeft(profileKnobWidth).reduced(4));
        profileGateTrim.setBounds(profileKnobs.removeFromLeft(profileKnobWidth).reduced(4));

        area.removeFromTop(8);

        // ── Amp character section (amp-specific toggles + Amp4 blend knobs) ──────
        // Buttons are always laid out here; visibility is controlled per active amp.
        auto ampCharRow = area.removeFromTop(30);
        ampCharRow.removeFromLeft(4);
        amp2HiwattButton.setBounds(ampCharRow.removeFromLeft(128));
        amp4TightButton.setBounds(ampCharRow.removeFromLeft(116));
        amp4LowOctaveButton.setBounds(ampCharRow.removeFromLeft(116));

        area.removeFromTop(4);
        auto blendRow = area.removeFromTop(100);
        const int blendW = blendRow.getWidth() / 4;
        amp4OctaveMix.setBounds(blendRow.removeFromLeft(blendW).reduced(4));
        amp4DirectMix.setBounds(blendRow.removeFromLeft(blendW).reduced(4));
        amp4CleanMix.setBounds(blendRow.removeFromLeft(blendW).reduced(4));

        area.removeFromTop(8);

        // ── Main 9 amp EQ knobs — clean, large, grouped ──────────────────────────
        const int mainKnobHeight = juce::jlimit(160, 220, area.getHeight());
        auto knobs = area.removeFromTop(mainKnobHeight);
        constexpr int kCols = 9;
        constexpr int kSeps = 2;   // two group separators
        constexpr int kSepW = 18;  // px each separator
        const int kw = (knobs.getWidth() - kCols * 8 - kSeps * kSepW) / kCols; // knob cell width
        const int kPad = 4;

        inputGain.setBounds(knobs.removeFromLeft(kw + 8).reduced(kPad));
        ampPreampGain.setBounds(knobs.removeFromLeft(kw + 8).reduced(kPad));
        knobs.removeFromLeft(kSepW);
        sub.setBounds(knobs.removeFromLeft(kw + 8).reduced(kPad));
        low.setBounds(knobs.removeFromLeft(kw + 8).reduced(kPad));
        mid.setBounds(knobs.removeFromLeft(kw + 8).reduced(kPad));
        high.setBounds(knobs.removeFromLeft(kw + 8).reduced(kPad));
        presence.setBounds(knobs.removeFromLeft(kw + 8).reduced(kPad));
        knobs.removeFromLeft(kSepW);
        gateThreshold.setBounds(knobs.removeFromLeft(kw + 8).reduced(kPad));
        output.setBounds(knobs.removeFromLeft(kw + 8).reduced(kPad));

    }
#if JucePlugin_Build_Standalone
    auto benchPanel = benchArea;
    benchPanel.removeFromTop(4);

    testBenchLabel.setBounds(benchPanel.removeFromTop(24));

    auto topBenchRow = benchPanel.removeFromTop(30);
    audioSettingsButton.setBounds(topBenchRow.removeFromLeft(190).reduced(2));
    topBenchRow.removeFromLeft(8);
    midiInputLabel.setBounds(topBenchRow.removeFromLeft(72));
    midiInputSelector.setBounds(topBenchRow.removeFromLeft(240).reduced(2));
    topBenchRow.removeFromLeft(8);
    midiOutputLabel.setBounds(topBenchRow.removeFromLeft(80));
    midiOutputSelector.setBounds(topBenchRow.removeFromLeft(240).reduced(2));

    auto diControlRow = benchPanel.removeFromTop(30);
    loadDITestButton.setBounds(diControlRow.removeFromLeft(90).reduced(2));
    clearDITestButton.setBounds(diControlRow.removeFromLeft(90).reduced(2));
    diControlRow.removeFromLeft(8);
    enableDITestButton.setBounds(diControlRow.removeFromLeft(108));
    playDITestButton.setBounds(diControlRow.removeFromLeft(44).reduced(2));
    pauseDITestButton.setBounds(diControlRow.removeFromLeft(44).reduced(2));
    stopDITestButton.setBounds(diControlRow.removeFromLeft(44).reduced(2));
    rwDITestButton.setBounds(diControlRow.removeFromLeft(44).reduced(2));
    ffDITestButton.setBounds(diControlRow.removeFromLeft(44).reduced(2));
    loopDITestButton.setBounds(diControlRow.removeFromLeft(80));
    loopStartBackButton.setBounds(diControlRow.removeFromLeft(58).reduced(2));
    loopStartForwardButton.setBounds(diControlRow.removeFromLeft(58).reduced(2));
    loopEndBackButton.setBounds(diControlRow.removeFromLeft(58).reduced(2));
    loopEndForwardButton.setBounds(diControlRow.removeFromLeft(58).reduced(2));

    auto diEditRow = benchPanel.removeFromTop(28);
    selectLoopButton.setBounds(diEditRow.removeFromLeft(82).reduced(2));
    trimSelectionButton.setBounds(diEditRow.removeFromLeft(82).reduced(2));
    cutSelectionButton.setBounds(diEditRow.removeFromLeft(82).reduced(2));
    clearSelectionButton.setBounds(diEditRow.removeFromLeft(82).reduced(2));
    diEditRow.removeFromLeft(16);
    diGainLabel.setBounds(diEditRow.removeFromLeft(56));
    diGainSlider.setBounds(diEditRow.removeFromLeft(240).reduced(2, 2));

    benchPanel.removeFromTop(2);
    diStatusLabel.setBounds(benchPanel.removeFromTop(20));
    auto zoomRow = benchPanel.removeFromTop(26);
    diZoomLabel.setBounds(zoomRow.removeFromLeft(40));
    diZoomSlider.setBounds(zoomRow.removeFromLeft(250).reduced(2, 4));
    zoomRow.removeFromLeft(10);
    diScrollLabel.setBounds(zoomRow.removeFromLeft(44));
    diScrollSlider.setBounds(zoomRow.removeFromLeft(340).reduced(2, 4));

    auto abRow = benchPanel.removeFromTop(28);
    captureAButton.setBounds(abRow.removeFromLeft(74).reduced(2));
    recallAButton.setBounds(abRow.removeFromLeft(74).reduced(2));
    abRow.removeFromLeft(6);
    captureBButton.setBounds(abRow.removeFromLeft(74).reduced(2));
    recallBButton.setBounds(abRow.removeFromLeft(74).reduced(2));
    abRow.removeFromLeft(6);
    toggleABButton.setBounds(abRow.removeFromLeft(56).reduced(2));
    abSnapshotStatusLabel.setBounds(abRow.removeFromLeft(260));

    diTimelineArea = benchPanel.removeFromTop(120);
    diLoopLabel.setBounds(benchPanel.removeFromTop(20));

    if (diTimelineArea.getWidth() != diWaveCacheWidth)
    {
        diWaveCacheWidth = diTimelineArea.getWidth();
        updateDITimelineCache();
    }
#endif
}

void BackhouseAmpSimAudioProcessorEditor::resized()
{
    repaint();
}

void BackhouseAmpSimAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    // Knob labels in Punkaholic 14pt — placed at top of each knob's bounds
    auto drawKnobLabel = [&g, this](const juce::Slider& s, juce::Colour c)
    {
        if (!s.isVisible() || s.getName().isEmpty())
            return;
        auto b = s.getBounds();
        if (punkaholicTypeface != nullptr)
            g.setFont(juce::Font(juce::FontOptions(punkaholicTypeface).withHeight(20.0f)));
        else
            g.setFont(20.0f);
        g.setColour(c);
        // Place label so its bottom edge just touches the top of the arc.
        // The rotary draw area is b.getHeight() - 18 (textbox). Arc top = cy - trackR.
        const float drawH  = static_cast<float>(b.getHeight()) - 18.0f;
        const float drawW  = static_cast<float>(b.getWidth());
        const float outerR = juce::jmin(drawW, drawH) * 0.5f - 4.0f;
        const float trackR = outerR * 0.82f;
        const float arcTop = static_cast<float>(b.getY()) + drawH * 0.5f - trackR;
        const int labelY   = static_cast<int>(arcTop) - 30;
        g.drawText(s.getName(), b.getX() - 4, labelY, b.getWidth() + 8, 30,
                   juce::Justification::centred, true);
    };

    const juce::Colour warmLabel  (0xffcbb898);
    const juce::Colour coolLabel  (0xffa8bfcc);
    const juce::Colour greenLabel (0xff9ecfa2);
    const juce::Colour purpleLabel(0xffb882d0);

    // Amp tab knobs
    for (auto* s : std::initializer_list<const juce::Slider*> {
            &inputGain, &ampPreampGain, &sub, &low, &mid, &high, &presence, &gateThreshold, &output })
        drawKnobLabel(*s, warmLabel);
    for (auto* s : std::initializer_list<const juce::Slider*> {
            &amp4OctaveMix, &amp4DirectMix, &amp4CleanMix })
        drawKnobLabel(*s, warmLabel);
    for (auto* s : std::initializer_list<const juce::Slider*> {
            &profilePickupOutput, &profileBrightness, &profileLowEnd, &profileGateTrim })
        drawKnobLabel(*s, warmLabel);

    // Amp tab: section footer labels in Dharma Punk 13pt, cyan
    if (inputGain.isVisible() && !inputGain.getBounds().isEmpty())
    {
        const juce::Colour cyanLabel(0xff00d4ff);
        auto drawFooter = [&g, cyanLabel, this](const juce::Rectangle<int>& r, const juce::String& text)
        {
            if (dharmaPunkTypeface != nullptr)
                g.setFont(juce::Font(juce::FontOptions(dharmaPunkTypeface).withHeight(17.0f)));
            else
                g.setFont(17.0f);
            g.setColour(cyanLabel);
            g.drawText(text, r.getX(), r.getBottom() + 4, r.getWidth(), 20,
                       juce::Justification::centred, true);
        };
        drawFooter(inputGain.getBounds().getUnion(ampPreampGain.getBounds()), "SIGNAL");
        drawFooter(sub.getBounds().getUnion(presence.getBounds()), "AMP EQ");
        drawFooter(gateThreshold.getBounds().getUnion(output.getBounds()), "OUTPUT");
    }

    // Stomps tab knobs
    for (auto* s : std::initializer_list<const juce::Slider*> {
            &stompInputGain, &stompCompSustain, &stompCompLevel, &stompCompMix })
        drawKnobLabel(*s, greenLabel);
    for (auto* s : std::initializer_list<const juce::Slider*> {
            &tsDrive, &tsTone, &tsLevel })
        drawKnobLabel(*s, warmLabel);
    for (auto* s : std::initializer_list<const juce::Slider*> {
            &wowPosition, &wowMix })
        drawKnobLabel(*s, warmLabel);
    for (auto* s : std::initializer_list<const juce::Slider*> {
            &phaserRate, &phaserDepth, &phaserCenter, &phaserFeedback, &phaserMix })
        drawKnobLabel(*s, coolLabel);
    for (auto* s : std::initializer_list<const juce::Slider*> {
            &amp1TremRate, &amp1TremDepth })
        drawKnobLabel(*s, coolLabel);
    drawKnobLabel(whammyPosition, purpleLabel);
    drawKnobLabel(whammyMix, purpleLabel);
    {
        const juce::Colour wahLabel (0xffd4a800);
        drawKnobLabel(wahPosition, wahLabel);
        drawKnobLabel(wahMix, wahLabel);
    }

    // Cab tab
    drawKnobLabel(cabTone, coolLabel);
    drawKnobLabel(cabMicBlend, coolLabel);

    // Space tab knobs
    drawKnobLabel(reverbMix, coolLabel);
    for (auto* s : std::initializer_list<const juce::Slider*> {
            &output1176Input, &output1176Release, &output1176Mix })
        drawKnobLabel(*s, warmLabel);
    for (auto* s : std::initializer_list<const juce::Slider*> {
            &outputEq63, &outputEq160, &outputEq400, &outputEq1000,
            &outputEq2500, &outputEq6300, &outputEq12000 })
        drawKnobLabel(*s, coolLabel);
    for (auto* s : std::initializer_list<const juce::Slider*> {
            &outputEqQ63, &outputEqQ160, &outputEqQ400, &outputEqQ1000,
            &outputEqQ2500, &outputEqQ6300, &outputEqQ12000 })
        drawKnobLabel(*s, coolLabel);
    drawKnobLabel(outputEqHpHz, coolLabel);
    drawKnobLabel(outputEqLpHz, coolLabel);
    for (auto* s : std::initializer_list<const juce::Slider*> {
            &delayTime, &delayFeedback, &delayMix, &delayTone, &delayPitch })
        drawKnobLabel(*s, coolLabel);

    // Enabled-effect glow: soft expanded halo around each active stomp button
    struct GlowEntry { const juce::ToggleButton& btn; juce::Colour colour; };
    const GlowEntry glowEntries[] = {
        { stompCompEnabledButton,  juce::Colour(0xff3dcc66) },
        { tsEnabledButton,         juce::Colour(0xffcc6b2e) },
        { phaserEnabledButton,     juce::Colour(0xff4477dd) },
        { amp1TremEnabledButton,   juce::Colour(0xff22bbcc) },
        { wowEnabledButton,        juce::Colour(0xffdd3355) },
        { whammyEnabledButton,     juce::Colour(0xff9933cc) },
        { wahEnabledButton,        juce::Colour(0xffd4a800) },
        { delayEnabledButton,      juce::Colour(0xff00d4ff) },
        { reverbEnabledButton,     juce::Colour(0xff6ea2ba) },
        { output1176EnabledButton, juce::Colour(0xffd3a45a) },
        { outputEqEnabledButton,   juce::Colour(0xffff1493) },
    };
    for (const auto& ge : glowEntries)
    {
        if (ge.btn.isVisible() && ge.btn.getToggleState())
        {
            const auto b = ge.btn.getBounds().expanded(4).toFloat();
            g.setColour(ge.colour.withAlpha(0.15f));
            g.fillRoundedRectangle(b, 8.0f);
        }
    }

#if JucePlugin_Build_Standalone
    drawDITimeline(g);
#endif
}

void BackhouseAmpSimAudioProcessorEditor::timerCallback()
{
    const auto hz = pluginProcessor.getDetectedPitchHz();
    const int transpose = static_cast<int>(pluginProcessor.getAPVTS().getRawParameterValue("tunerTranspose")->load());
    const auto noteInfo = getNoteInfoFromHz(hz, transpose);
    tunerNeedleCents = noteInfo.cents;
    tunerNeedleNote = noteInfo.note;

    if (hz > 10.0f)
        pitchLabel.setText("Pitch: " + juce::String(hz, 1) + " Hz", juce::dontSendNotification);
    else
        pitchLabel.setText("Pitch: -- Hz", juce::dontSendNotification);

    tunerNoteLabel.setText("Note: " + tunerNeedleNote, juce::dontSendNotification);
    tunerDetuneLabel.setText("Detune: " + juce::String(tunerNeedleCents, 1) + " cents", juce::dontSendNotification);
    tunerTransposeLabel.setText("Reference Shift (semitones): " + juce::String(transpose >= 0 ? "+" : "") + juce::String(transpose), juce::dontSendNotification);

    inputMeterValue = pluginProcessor.getInputMeterLevel();
    outputMeterValue = pluginProcessor.getOutputMeterLevel();

    const int currentAmp = static_cast<int>(pluginProcessor.getAPVTS().getRawParameterValue("ampType")->load());
    const bool isAmp1 = currentAmp == 0;
    const bool isAmp2 = currentAmp == 1;
    const bool isAmp4 = currentAmp == 3;
    const bool amp4Tight = pluginProcessor.getAPVTS().getRawParameterValue("amp4Tight")->load() > 0.5f;
    const bool amp4LowOctEnabled = pluginProcessor.getAPVTS().getRawParameterValue("amp4LowOctave")->load() > 0.5f;
    const bool showLooseControls = isAmp4 && !amp4Tight;
    const bool reverbEnabled = pluginProcessor.getAPVTS().getRawParameterValue("reverbEnabled")->load() > 0.5f;
    const bool wowEnabled = pluginProcessor.getAPVTS().getRawParameterValue("wowEnabled")->load() > 0.5f;
    const bool wowMidi = pluginProcessor.getAPVTS().getRawParameterValue("wowMidiEnable")->load() > 0.5f;
    const bool stompCompEnabled = pluginProcessor.getAPVTS().getRawParameterValue("stompCompEnabled")->load() > 0.5f;
    const bool tsEnabled = pluginProcessor.getAPVTS().getRawParameterValue("tsEnabled")->load() > 0.5f;
    const bool delayEnabled = pluginProcessor.getAPVTS().getRawParameterValue("delayEnabled")->load() > 0.5f;
    const bool output1176Enabled = pluginProcessor.getAPVTS().getRawParameterValue("output1176Enabled")->load() > 0.5f;
    const bool outputEqEnabled = pluginProcessor.getAPVTS().getRawParameterValue("outputEqEnabled")->load() > 0.5f;
    const bool outputEqHpEnabled = pluginProcessor.getAPVTS().getRawParameterValue("outputEqHpEnabled")->load() > 0.5f;
    const bool outputEqLpEnabled = pluginProcessor.getAPVTS().getRawParameterValue("outputEqLpEnabled")->load() > 0.5f;
    const bool phaserEnabled = pluginProcessor.getAPVTS().getRawParameterValue("phaserEnabled")->load() > 0.5f;
    const bool amp1TremEnabled = pluginProcessor.getAPVTS().getRawParameterValue("amp1TremEnabled")->load() > 0.5f;

    // Amp-specific toggles live on the Amp tab (always laid out there)
    // In Advanced mode they also appear in the details row — refreshTopControlVisibility handles that.
    amp2HiwattButton.setEnabled(isAmp2);
    amp4TightButton.setEnabled(isAmp4);
    amp4LowOctaveButton.setEnabled(showLooseControls);
    amp4OctaveMix.setEnabled(showLooseControls && amp4LowOctEnabled);
    amp4DirectMix.setEnabled(showLooseControls);
    amp4CleanMix.setEnabled(showLooseControls);
    reverbEnabledButton.setEnabled(true);
    reverbMix.setEnabled(reverbEnabled);
    wowEnabledButton.setEnabled(true);
    wowPosition.setEnabled(wowEnabled);
    wowMix.setEnabled(wowEnabled);
    wowMidiEnableButton.setEnabled(wowEnabled);
    wowMomentaryButton.setEnabled(wowEnabled && wowMidi);
    wowModeSelector.setEnabled(wowEnabled);
    {
        const bool whammyOn = pluginProcessor.getAPVTS().getRawParameterValue("whammyEnabled")->load() > 0.5f;
        whammyEnabledButton.setEnabled(true);
        whammyModeSelector.setEnabled(whammyOn);
        whammyPosition.setEnabled(whammyOn);
        whammyMix.setEnabled(whammyOn);
    }
    {
        const bool wahOn = pluginProcessor.getAPVTS().getRawParameterValue("wahEnabled")->load() > 0.5f;
        wahEnabledButton.setEnabled(true);
        wahPosition.setEnabled(wahOn);
        wahMix.setEnabled(wahOn);
    }
    stompCompEnabledButton.setEnabled(true);
    stompCompSustain.setEnabled(stompCompEnabled);
    stompCompLevel.setEnabled(stompCompEnabled);
    stompCompMix.setEnabled(stompCompEnabled);
    tsDrive.setEnabled(tsEnabled);
    tsTone.setEnabled(tsEnabled);
    tsLevel.setEnabled(tsEnabled);
    phaserRate.setEnabled(phaserEnabled);
    phaserDepth.setEnabled(phaserEnabled);
    phaserCenter.setEnabled(phaserEnabled);
    phaserFeedback.setEnabled(phaserEnabled);
    phaserMix.setEnabled(phaserEnabled);
    amp1TremEnabledButton.setEnabled(isAmp1);
    amp1TremRate.setEnabled(isAmp1 && amp1TremEnabled);
    amp1TremDepth.setEnabled(isAmp1 && amp1TremEnabled);
    output1176EnabledButton.setEnabled(true);
    outputCompModeSelector.setEnabled(output1176Enabled);
    output1176Input.setEnabled(output1176Enabled);
    output1176Release.setEnabled(output1176Enabled);
    output1176Mix.setEnabled(output1176Enabled);
    outputEqEnabledButton.setEnabled(true);
    outputEq63.setEnabled(outputEqEnabled);
    outputEq160.setEnabled(outputEqEnabled);
    outputEq400.setEnabled(outputEqEnabled);
    outputEq1000.setEnabled(outputEqEnabled);
    outputEq2500.setEnabled(outputEqEnabled);
    outputEq6300.setEnabled(outputEqEnabled);
    outputEq12000.setEnabled(outputEqEnabled);
    outputEqQ63.setEnabled(outputEqEnabled);
    outputEqQ160.setEnabled(outputEqEnabled);
    outputEqQ400.setEnabled(outputEqEnabled);
    outputEqQ1000.setEnabled(outputEqEnabled);
    outputEqQ2500.setEnabled(outputEqEnabled);
    outputEqQ6300.setEnabled(outputEqEnabled);
    outputEqQ12000.setEnabled(outputEqEnabled);
    outputEqHpEnabledButton.setEnabled(outputEqEnabled);
    outputEqLpEnabledButton.setEnabled(outputEqEnabled);
    outputEqHpHz.setEnabled(outputEqEnabled && outputEqHpEnabled);
    outputEqLpHz.setEnabled(outputEqEnabled && outputEqLpEnabled);
    delayTime.setEnabled(delayEnabled);
    delayFeedback.setEnabled(delayEnabled);
    delayMix.setEnabled(delayEnabled);
    delayTone.setEnabled(delayEnabled);
    delayPitch.setEnabled(delayEnabled);
    refreshTopControlVisibility();
    refreshTabVisibility();
    refreshPreampHeatLabel();
    refreshCabSelectorButtons();
    refreshIRStatusLabel();
    outputSpectrumValues = pluginProcessor.getOutputSpectrumBins();
    const int rigPoints = pluginProcessor.getRigPointTotal();
    const int rigBudget = pluginProcessor.getRigBudgetMax();
    rigPointsLabel.setText("Rig Score: " + juce::String(rigPoints) + " / " + juce::String(rigBudget), juce::dontSendNotification);
    rigPointsLabel.setColour(juce::Label::textColourId, rigPoints > rigBudget ? juce::Colours::red : accentHot);
    if (activeTabIndex == 0)
        repaint(tunerNeedleArea);
    if (activeTabIndex == 5)
        repaint(outputSpectrumArea);

#if JucePlugin_Build_Standalone
    if (standaloneDeviceManager == nullptr)
        attachStandaloneDeviceManager();

    if (diDialogOpen)
        return;

    refreshTestDISection();
#endif
}

void BackhouseAmpSimAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
#if JucePlugin_Build_Standalone
    if (!diTimelineArea.contains(e.getPosition()) || !pluginProcessor.hasTestDIFile())
        return;

    // Default gesture is region selection.
    if (!e.mods.isShiftDown() && !e.mods.isAltDown() && !e.mods.isCommandDown() && !e.mods.isCtrlDown())
    {
        if (diHasSelection)
        {
            const int selStartX = timelineXFromTime(juce::jmin(diSelectionStartSeconds, diSelectionEndSeconds));
            const int selEndX = timelineXFromTime(juce::jmax(diSelectionStartSeconds, diSelectionEndSeconds));
            if (std::abs(e.x - selStartX) <= 8)
            {
                diTimelineDragMode = DITimelineDragMode::selectStartHandle;
                return;
            }
            if (std::abs(e.x - selEndX) <= 8)
            {
                diTimelineDragMode = DITimelineDragMode::selectEndHandle;
                return;
            }
        }

        diTimelineDragMode = DITimelineDragMode::selectRegion;
        diHasSelection = true;
        const double t = timelineTimeFromX(e.x);
        diSelectionStartSeconds = t;
        diSelectionEndSeconds = t;
        repaint(diTimelineArea);
        return;
    }

    const int startX = timelineXFromTime(pluginProcessor.getTestDILoopStartSeconds());
    const int endX = timelineXFromTime(pluginProcessor.getTestDILoopEndSeconds());

    // Alt/Option drag edits loop points when close to boundaries.
    if (e.mods.isAltDown())
    {
        if (std::abs(e.x - startX) <= 8)
            diTimelineDragMode = DITimelineDragMode::loopStart;
        else if (std::abs(e.x - endX) <= 8)
            diTimelineDragMode = DITimelineDragMode::loopEnd;
        else
            diTimelineDragMode = DITimelineDragMode::playhead;
    }
    else
    {
        // Shift/Cmd/Ctrl drag = playhead scrub.
        diTimelineDragMode = DITimelineDragMode::playhead;
    }

    if (diTimelineDragMode == DITimelineDragMode::playhead)
    {
        pluginProcessor.setTestDIPositionSeconds(timelineTimeFromX(e.x));
        refreshTestDISection();
    }
#endif
}

void BackhouseAmpSimAudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& e)
{
#if JucePlugin_Build_Standalone
    if (!diTimelineArea.contains(e.getPosition()) || !pluginProcessor.hasTestDIFile())
        return;

    selectTransientWindowAt(timelineTimeFromX(e.x));
    refreshTestDISection();
#else
    juce::ignoreUnused(e);
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
    else if (diTimelineDragMode == DITimelineDragMode::selectRegion)
    {
        diHasSelection = true;
        diSelectionEndSeconds = draggedTime;
        repaint(diTimelineArea);
        return;
    }
    else if (diTimelineDragMode == DITimelineDragMode::selectStartHandle)
    {
        diHasSelection = true;
        const double endSel = juce::jmax(diSelectionStartSeconds, diSelectionEndSeconds);
        diSelectionStartSeconds = juce::jmin(draggedTime, endSel - 0.001);
        diSelectionEndSeconds = endSel;
        repaint(diTimelineArea);
        return;
    }
    else if (diTimelineDragMode == DITimelineDragMode::selectEndHandle)
    {
        diHasSelection = true;
        const double startSel = juce::jmin(diSelectionStartSeconds, diSelectionEndSeconds);
        diSelectionStartSeconds = startSel;
        diSelectionEndSeconds = juce::jmax(draggedTime, startSel + 0.001);
        repaint(diTimelineArea);
        return;
    }

    refreshTestDISection();
#endif
}

void BackhouseAmpSimAudioProcessorEditor::mouseUp(const juce::MouseEvent&)
{
#if JucePlugin_Build_Standalone
    diTimelineDragMode = DITimelineDragMode::none;
#endif
}

void BackhouseAmpSimAudioProcessorEditor::mouseExit(const juce::MouseEvent&)
{
#if JucePlugin_Build_Standalone
    diTimelineDragMode = DITimelineDragMode::none;
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

void BackhouseAmpSimAudioProcessorEditor::setCabSpeakerFromButton(int speakerIndex)
{
    auto& apvts = pluginProcessor.getAPVTS();
    if (auto* param = apvts.getParameter("cabSpeaker"))
    {
        const auto normalisable = juce::NormalisableRange<float>(0.0f, 4.0f, 1.0f);
        const float clamped = juce::jlimit(0, 4, speakerIndex);
        param->setValueNotifyingHost(normalisable.convertTo0to1(clamped));
    }
}

void BackhouseAmpSimAudioProcessorEditor::setCabMicFromButton(int micIndex)
{
    auto& apvts = pluginProcessor.getAPVTS();
    if (auto* param = apvts.getParameter("cabMic"))
    {
        const auto normalisable = juce::NormalisableRange<float>(0.0f, 4.0f, 1.0f);
        const float clamped = juce::jlimit(0, 4, micIndex);
        param->setValueNotifyingHost(normalisable.convertTo0to1(clamped));
    }
}

void BackhouseAmpSimAudioProcessorEditor::setCabSpeakerBFromButton(int speakerIndex)
{
    auto& apvts = pluginProcessor.getAPVTS();
    if (auto* param = apvts.getParameter("cabSpeakerB"))
    {
        const auto normalisable = juce::NormalisableRange<float>(0.0f, 4.0f, 1.0f);
        const float clamped = juce::jlimit(0, 4, speakerIndex);
        param->setValueNotifyingHost(normalisable.convertTo0to1(clamped));
    }
}

void BackhouseAmpSimAudioProcessorEditor::setCabMicBFromButton(int micIndex)
{
    auto& apvts = pluginProcessor.getAPVTS();
    if (auto* param = apvts.getParameter("cabMicB"))
    {
        const auto normalisable = juce::NormalisableRange<float>(0.0f, 4.0f, 1.0f);
        const float clamped = juce::jlimit(0, 4, micIndex);
        param->setValueNotifyingHost(normalisable.convertTo0to1(clamped));
    }
}

void BackhouseAmpSimAudioProcessorEditor::refreshCabSelectorButtons()
{
    const int speaker = static_cast<int>(pluginProcessor.getAPVTS().getRawParameterValue("cabSpeaker")->load());
    const int mic = static_cast<int>(pluginProcessor.getAPVTS().getRawParameterValue("cabMic")->load());
    const int speakerB = static_cast<int>(pluginProcessor.getAPVTS().getRawParameterValue("cabSpeakerB")->load());
    const int micB = static_cast<int>(pluginProcessor.getAPVTS().getRawParameterValue("cabMicB")->load());
    const bool micAOffAxis = pluginProcessor.getAPVTS().getRawParameterValue("cabMicAOffAxis")->load() > 0.5f;
    const bool micBOffAxis = pluginProcessor.getAPVTS().getRawParameterValue("cabMicBOffAxis")->load() > 0.5f;

    std::array<juce::TextButton*, 5> speakerButtons { &speakerGreenbackButton, &speakerV30Button, &speakerCreambackButton, &speakerG12TButton, &speakerJensenButton };
    std::array<juce::TextButton*, 5> micButtons { &mic57Button, &micU87Button, &micU47FetButton, &micRoyerButton, &mic421Button };
    std::array<juce::TextButton*, 5> speakerButtonsB { &speakerBGreenbackButton, &speakerBV30Button, &speakerBCreambackButton, &speakerBG12TButton, &speakerBJensenButton };
    std::array<juce::TextButton*, 5> micButtonsB { &micB57Button, &micBU87Button, &micBU47FetButton, &micBRoyerButton, &micB421Button };

    for (size_t i = 0; i < speakerButtons.size(); ++i)
        speakerButtons[i]->setColour(juce::TextButton::buttonColourId, static_cast<int>(i) == speaker ? juce::Colour(0xffff1493) : juce::Colour(0xff171920));
    for (size_t i = 0; i < micButtons.size(); ++i)
        micButtons[i]->setColour(juce::TextButton::buttonColourId, static_cast<int>(i) == mic ? juce::Colour(0xff00d4ff) : juce::Colour(0xff171920));
    for (size_t i = 0; i < speakerButtonsB.size(); ++i)
        speakerButtonsB[i]->setColour(juce::TextButton::buttonColourId, static_cast<int>(i) == speakerB ? juce::Colour(0xffff1493) : juce::Colour(0xff171920));
    for (size_t i = 0; i < micButtonsB.size(); ++i)
        micButtonsB[i]->setColour(juce::TextButton::buttonColourId, static_cast<int>(i) == micB ? juce::Colour(0xff00d4ff) : juce::Colour(0xff171920));

    micAOffAxisButton.setButtonText(micAOffAxis ? "A Off-Axis" : "A On-Axis");
    micBOffAxisButton.setButtonText(micBOffAxis ? "B Off-Axis" : "B On-Axis");
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

void BackhouseAmpSimAudioProcessorEditor::bindAmpPreampControl()
{
    auto& apvts = pluginProcessor.getAPVTS();
    const int amp = static_cast<int>(apvts.getRawParameterValue("ampType")->load());
    const size_t idx = static_cast<size_t>(juce::jlimit(0, 3, amp));

    ampPreampAttachment.reset();
    ampPreampAttachment = std::make_unique<SliderAttachment>(apvts, preampIds[idx], ampPreampGain);
}

void BackhouseAmpSimAudioProcessorEditor::refreshPreampHeatLabel()
{
    const int amp = static_cast<int>(pluginProcessor.getAPVTS().getRawParameterValue("ampType")->load());
    const float v = static_cast<float>(ampPreampGain.getValue());
    juce::String state = "Cold";
    juce::Colour colour = juce::Colour(0xff9bb0bd);
    if (v >= 0.82f)
    {
        state = "Hot";
        colour = juce::Colour(0xffef7d2f);
    }
    else if (v >= 0.62f)
    {
        state = "Warm+";
        colour = juce::Colour(0xffe79747);
    }
    else if (v >= 0.42f)
    {
        state = "Warm";
        colour = juce::Colour(0xffd6ad66);
    }
    else if (v >= 0.22f)
    {
        state = "Cool";
        colour = juce::Colour(0xffa3b4bf);
    }

    preampHeatLabel.setColour(juce::Label::textColourId, colour);
    preampHeatLabel.setText("Preamp A" + juce::String(amp + 1) + ": " + state, juce::dontSendNotification);
}

void BackhouseAmpSimAudioProcessorEditor::refreshTopControlVisibility()
{
    const int currentAmp = static_cast<int>(pluginProcessor.getAPVTS().getRawParameterValue("ampType")->load());
    const bool isAmp2 = currentAmp == 1;
    const bool isAmp4 = currentAmp == 3;
    const bool onAmpTab = activeTabIndex == 2;

    uiModeButton.setButtonText(showAdvancedTopControls ? "Details ON" : "Details");

    // Advanced row shows: tuner toggle
    tunerButton.setVisible(showAdvancedTopControls);

    // Amp-specific toggles: visible on Amp tab only, conditional on selected amp
    amp2HiwattButton.setVisible(onAmpTab && isAmp2);
    amp4TightButton.setVisible(onAmpTab && isAmp4);
    amp4LowOctaveButton.setVisible(onAmpTab && isAmp4);

    // These now live exclusively on their home tabs — never shown in the global bar
    outputEqHpEnabledButton.setVisible(false);
    outputEqLpEnabledButton.setVisible(false);
}

void BackhouseAmpSimAudioProcessorEditor::setActiveTab(int tabIndex)
{
    activeTabIndex = juce::jlimit(0, 6, tabIndex);
    refreshTabVisibility();
    resized();
    repaint();
}

void BackhouseAmpSimAudioProcessorEditor::refreshTabVisibility()
{
    const bool tunerTab = activeTabIndex == 0;
    const bool stompsTab = activeTabIndex == 1;
    const bool ampTab = activeTabIndex == 2;
    const bool cabTab = activeTabIndex == 3;
    const bool postColorTab = activeTabIndex == 4;
    const bool spaceTab = activeTabIndex == 5;
    const bool midiTab = activeTabIndex == 6;

    // Active tab gets warm amber, inactive gets dark brown
    const juce::Colour tabActive   (0xffa35b2b);
    const juce::Colour tabInactive (0xff3a2a20);
    tabAmpButton.setColour   (juce::TextButton::buttonColourId, ampTab    ? tabActive : tabInactive);
    tabTunerButton.setColour (juce::TextButton::buttonColourId, tunerTab  ? tabActive : tabInactive);
    tabStompsButton.setColour(juce::TextButton::buttonColourId, stompsTab ? tabActive : tabInactive);
    tabCabButton.setColour   (juce::TextButton::buttonColourId, cabTab    ? tabActive : tabInactive);
    tabSpaceButton.setColour    (juce::TextButton::buttonColourId, spaceTab     ? tabActive : tabInactive);
    tabPostColorButton.setColour(juce::TextButton::buttonColourId, postColorTab ? tabActive : tabInactive);
    tabMidiButton.setColour     (juce::TextButton::buttonColourId, midiTab      ? tabActive : tabInactive);

    // Always visible: profile buttons, key global toggles
    for (auto* c : std::array<juce::Component*, 7> {
             static_cast<juce::Component*>(&profileNeutralButton), static_cast<juce::Component*>(&profile1Button), static_cast<juce::Component*>(&profile2Button), static_cast<juce::Component*>(&profile3Button),
             static_cast<juce::Component*>(&inputBoostButton), static_cast<juce::Component*>(&cabEnabledButton), static_cast<juce::Component*>(&reverbEnabledButton) })
        c->setVisible(true);

    // Amp tab only
    const std::vector<juce::Component*> ampOnlyComponents {
        &importProfilesButton, &exportProfilesButton,
        &profile1NameEditor, &profile2NameEditor, &profile3NameEditor, &profileEditLabel, &profileIoStatusLabel,
        &profilePickupOutput, &profileBrightness, &profileLowEnd, &profileGateTrim,
        &inputGain, &ampPreampGain, &sub, &low, &mid, &high, &presence, &output, &gateThreshold,
        &amp4OctaveMix, &amp4DirectMix, &amp4CleanMix
    };
    for (auto* c : ampOnlyComponents)
        c->setVisible(ampTab);

#if JucePlugin_Build_Standalone
    const bool showBenchOnAllTabs = true;
    std::array<juce::Component*, 35> benchComponents {
        &testBenchLabel, &audioSettingsButton, &midiInputLabel, &midiOutputLabel, &midiInputSelector, &midiOutputSelector,
        &loadDITestButton, &clearDITestButton, &enableDITestButton, &playDITestButton, &pauseDITestButton, &stopDITestButton, &rwDITestButton, &ffDITestButton, &loopDITestButton,
        &loopStartBackButton, &loopStartForwardButton, &loopEndBackButton, &loopEndForwardButton,
        &selectLoopButton, &trimSelectionButton, &cutSelectionButton, &clearSelectionButton,
        &diGainLabel, &diGainSlider,
        &diZoomLabel, &diScrollLabel, &diZoomSlider, &diScrollSlider,
        &captureAButton, &recallAButton, &captureBButton, &recallBButton, &toggleABButton,
        &abSnapshotStatusLabel
    };
    for (auto* c : benchComponents)
        c->setVisible(showBenchOnAllTabs);
    diStatusLabel.setVisible(showBenchOnAllTabs);
    diLoopLabel.setVisible(showBenchOnAllTabs);
#endif

    tunerNoteLabel.setVisible(tunerTab);
    tunerDetuneLabel.setVisible(tunerTab);
    tunerTransposeLabel.setVisible(tunerTab);
    tunerTranspose.setVisible(tunerTab);

    for (auto* c : { static_cast<juce::Component*>(&stompInputGain), static_cast<juce::Component*>(&tsDrive), static_cast<juce::Component*>(&tsTone), static_cast<juce::Component*>(&tsLevel),
                     static_cast<juce::Component*>(&tsEnabledButton), static_cast<juce::Component*>(&stompCompEnabledButton), static_cast<juce::Component*>(&stompCompSustain), static_cast<juce::Component*>(&stompCompLevel), static_cast<juce::Component*>(&stompCompMix),
                     static_cast<juce::Component*>(&wowEnabledButton),
                     static_cast<juce::Component*>(&wowMidiEnableButton), static_cast<juce::Component*>(&wowMomentaryButton), static_cast<juce::Component*>(&wowModeSelector),
                     static_cast<juce::Component*>(&phaserEnabledButton), static_cast<juce::Component*>(&phaserRate), static_cast<juce::Component*>(&phaserDepth), static_cast<juce::Component*>(&phaserCenter), static_cast<juce::Component*>(&phaserFeedback), static_cast<juce::Component*>(&phaserMix),
                     static_cast<juce::Component*>(&amp1TremEnabledButton), static_cast<juce::Component*>(&amp1TremRate), static_cast<juce::Component*>(&amp1TremDepth),
                     static_cast<juce::Component*>(&wowPosition), static_cast<juce::Component*>(&wowMix),
                     static_cast<juce::Component*>(&whammyEnabledButton), static_cast<juce::Component*>(&whammyModeSelector),
                     static_cast<juce::Component*>(&whammyPosition), static_cast<juce::Component*>(&whammyMix),
                     static_cast<juce::Component*>(&wahEnabledButton),
                     static_cast<juce::Component*>(&wahPosition), static_cast<juce::Component*>(&wahMix) })
        c->setVisible(stompsTab);

    for (auto* c : { static_cast<juce::Component*>(&cabSpeakerLabel), static_cast<juce::Component*>(&cabMicLabel),
                     static_cast<juce::Component*>(&speakerGreenbackButton), static_cast<juce::Component*>(&speakerV30Button), static_cast<juce::Component*>(&speakerCreambackButton),
                     static_cast<juce::Component*>(&speakerG12TButton), static_cast<juce::Component*>(&speakerJensenButton), static_cast<juce::Component*>(&mic57Button),
                     static_cast<juce::Component*>(&micU87Button), static_cast<juce::Component*>(&micU47FetButton), static_cast<juce::Component*>(&micRoyerButton),
                     static_cast<juce::Component*>(&mic421Button), static_cast<juce::Component*>(&micAOffAxisButton),
                     static_cast<juce::Component*>(&cabSpeakerBLabel), static_cast<juce::Component*>(&cabMicBLabel), static_cast<juce::Component*>(&speakerBGreenbackButton),
                     static_cast<juce::Component*>(&speakerBV30Button), static_cast<juce::Component*>(&speakerBCreambackButton), static_cast<juce::Component*>(&speakerBG12TButton),
                     static_cast<juce::Component*>(&speakerBJensenButton), static_cast<juce::Component*>(&micB57Button), static_cast<juce::Component*>(&micBU87Button),
                     static_cast<juce::Component*>(&micBU47FetButton), static_cast<juce::Component*>(&micBRoyerButton), static_cast<juce::Component*>(&micB421Button),
                     static_cast<juce::Component*>(&micBOffAxisButton), static_cast<juce::Component*>(&cabMicBlend), static_cast<juce::Component*>(&cabTone),
                     static_cast<juce::Component*>(&loadIRButton), static_cast<juce::Component*>(&clearIRButton) })
        c->setVisible(cabTab);

    // Space tab: Delay + Reverb + Rig
    for (auto* c : { static_cast<juce::Component*>(&reverbMix),
                     static_cast<juce::Component*>(&delayEnabledButton),
                     static_cast<juce::Component*>(&delayTime), static_cast<juce::Component*>(&delayFeedback), static_cast<juce::Component*>(&delayMix),
                     static_cast<juce::Component*>(&delayTone), static_cast<juce::Component*>(&delayPitch),
                     static_cast<juce::Component*>(&loadReverbIRButton), static_cast<juce::Component*>(&clearReverbIRButton) })
        c->setVisible(spaceTab);
    for (auto& button : rigModuleButtons)
        button.setVisible(spaceTab);
    rigPointsLabel.setVisible(spaceTab);
    rigMaxPointsSlider.setVisible(spaceTab);
    rigResetButton.setVisible(spaceTab);

    // Post Color tab: 1176 + Studio EQ
    for (auto* c : { static_cast<juce::Component*>(&output1176EnabledButton), static_cast<juce::Component*>(&outputEqEnabledButton), static_cast<juce::Component*>(&outputCompModeSelector),
                     static_cast<juce::Component*>(&outputEqHpEnabledButton), static_cast<juce::Component*>(&outputEqLpEnabledButton),
                     static_cast<juce::Component*>(&output1176Input), static_cast<juce::Component*>(&output1176Release), static_cast<juce::Component*>(&output1176Mix),
                     static_cast<juce::Component*>(&outputEq63), static_cast<juce::Component*>(&outputEq160), static_cast<juce::Component*>(&outputEq400), static_cast<juce::Component*>(&outputEq1000), static_cast<juce::Component*>(&outputEq2500), static_cast<juce::Component*>(&outputEq6300), static_cast<juce::Component*>(&outputEq12000),
                     static_cast<juce::Component*>(&outputEqQ63), static_cast<juce::Component*>(&outputEqQ160), static_cast<juce::Component*>(&outputEqQ400), static_cast<juce::Component*>(&outputEqQ1000), static_cast<juce::Component*>(&outputEqQ2500), static_cast<juce::Component*>(&outputEqQ6300), static_cast<juce::Component*>(&outputEqQ12000),
                     static_cast<juce::Component*>(&outputEqHpHz), static_cast<juce::Component*>(&outputEqLpHz) })
        c->setVisible(postColorTab);

    midiMapHelpLabel.setVisible(midiTab);
    midiTemplateRhythmButton.setVisible(midiTab);
    midiTemplateLeadButton.setVisible(midiTab);
    midiTemplateAmbientButton.setVisible(midiTab);
    for (auto& row : midiMapRows)
    {
        row.slotLabel.setVisible(midiTab);
        row.enabledButton.setVisible(midiTab);
        row.ccSlider.setVisible(midiTab);
        row.targetBox.setVisible(midiTab);
        row.modeBox.setVisible(midiTab);
        row.minSlider.setVisible(midiTab);
        row.maxSlider.setVisible(midiTab);
        row.invertButton.setVisible(midiTab);
    }
}

void BackhouseAmpSimAudioProcessorEditor::refreshMidiMapPanel()
{
    const auto ids = pluginProcessor.getMappableParameterIds();
    for (int i = 0; i < pluginProcessor.getMidiMapSlotCount() && i < static_cast<int>(midiMapRows.size()); ++i)
    {
        const auto slot = pluginProcessor.getMidiMapSlot(i);
        auto& row = midiMapRows[static_cast<size_t>(i)];
        row.enabledButton.setToggleState(slot.enabled, juce::dontSendNotification);
        row.ccSlider.setValue(slot.ccNumber, juce::dontSendNotification);
        row.modeBox.setSelectedId(slot.mode + 1, juce::dontSendNotification);
        row.minSlider.setValue(slot.minNorm, juce::dontSendNotification);
        row.maxSlider.setValue(slot.maxNorm, juce::dontSendNotification);
        row.invertButton.setToggleState(slot.invert, juce::dontSendNotification);

        int targetId = 1;
        for (int idx = 0; idx < ids.size(); ++idx)
        {
            if (ids[idx] == slot.parameterId)
            {
                targetId = idx + 2;
                break;
            }
        }
        row.targetBox.setSelectedId(targetId, juce::dontSendNotification);
    }
}

void BackhouseAmpSimAudioProcessorEditor::commitMidiMapRow(int rowIndex)
{
    if (rowIndex < 0 || rowIndex >= static_cast<int>(midiMapRows.size()))
        return;

    const auto ids = pluginProcessor.getMappableParameterIds();
    auto& row = midiMapRows[static_cast<size_t>(rowIndex)];
    BackhouseAmpSimAudioProcessor::MidiMapSlot slot {};
    slot.enabled = row.enabledButton.getToggleState();
    slot.ccNumber = static_cast<int>(row.ccSlider.getValue());
    const int targetSel = row.targetBox.getSelectedItemIndex() - 1;
    if (targetSel >= 0 && targetSel < ids.size())
        slot.parameterId = ids[targetSel];
    slot.mode = juce::jlimit(0, 2, row.modeBox.getSelectedItemIndex());
    slot.minNorm = static_cast<float>(row.minSlider.getValue());
    slot.maxNorm = static_cast<float>(row.maxSlider.getValue());
    slot.invert = row.invertButton.getToggleState();
    pluginProcessor.setMidiMapSlot(rowIndex, slot);
}

void BackhouseAmpSimAudioProcessorEditor::applyMidiTemplate(int templateIndex)
{
    const auto ids = pluginProcessor.getMappableParameterIds();
    auto findId = [&ids](const juce::String& id) -> juce::String {
        for (const auto& s : ids)
            if (s == id)
                return s;
        return {};
    };

    std::array<BackhouseAmpSimAudioProcessor::MidiMapSlot, 8> templ {};
    // Common expression defaults
    templ[0] = { true, 27, findId("wowPosition"), 0, 0.0f, 1.0f, false };
    templ[1] = { true, 7,  findId("output"),      0, 0.25f, 0.95f, false };

    if (templateIndex == 0) // Rhythm
    {
        templ[2] = { true, 20, findId("inputBoost"), 2, 0.0f, 1.0f, false };
        templ[3] = { true, 21, findId("tsEnabled"), 1, 0.0f, 1.0f, false };
        templ[4] = { true, 22, findId("gateThresh"), 0, 0.35f, 0.72f, true };
        templ[5] = { true, 23, findId("reverbEnabled"), 1, 0.0f, 1.0f, false };
        templ[6] = { true, 24, findId("delayEnabled"), 1, 0.0f, 1.0f, false };
        templ[7] = { true, 64, findId("wowMix"), 2, 0.0f, 0.85f, false };
    }
    else if (templateIndex == 1) // Lead
    {
        templ[2] = { true, 20, findId("tsEnabled"), 1, 0.0f, 1.0f, false };
        templ[3] = { true, 21, findId("delayEnabled"), 1, 0.0f, 1.0f, false };
        templ[4] = { true, 22, findId("delayMix"), 0, 0.08f, 0.42f, false };
        templ[5] = { true, 23, findId("reverbMix"), 0, 0.05f, 0.32f, false };
        templ[6] = { true, 24, findId("wowEnabled"), 1, 0.0f, 1.0f, false };
        templ[7] = { true, 64, findId("wowMix"), 2, 0.0f, 0.9f, false };
    }
    else // Ambient
    {
        templ[2] = { true, 20, findId("reverbEnabled"), 1, 0.0f, 1.0f, false };
        templ[3] = { true, 21, findId("delayEnabled"), 1, 0.0f, 1.0f, false };
        templ[4] = { true, 22, findId("delayMix"), 0, 0.25f, 0.8f, false };
        templ[5] = { true, 23, findId("delayFeedback"), 0, 0.2f, 0.82f, false };
        templ[6] = { true, 24, findId("delayPitch"), 0, 0.0f, 1.0f, false };
        templ[7] = { true, 64, findId("wowMix"), 2, 0.0f, 0.65f, false };
    }

    for (int i = 0; i < 8; ++i)
        pluginProcessor.setMidiMapSlot(i, templ[static_cast<size_t>(i)]);
    refreshMidiMapPanel();
}

void BackhouseAmpSimAudioProcessorEditor::commitProfileName(int profileIndex, const juce::String& text)
{
    pluginProcessor.setProfileName(profileIndex, text);
    refreshProfileNameEditors();
    refreshGuitarProfileButtons();
}

void BackhouseAmpSimAudioProcessorEditor::refreshIRStatusLabel()
{
    juce::String cabText;
    if (pluginProcessor.hasUserIR())
    {
        cabText = "Cab IR: " + pluginProcessor.getUserIRName();
        clearIRButton.setEnabled(true);
    }
    else
    {
        const int speakerIdx = static_cast<int>(pluginProcessor.getAPVTS().getRawParameterValue("cabSpeaker")->load());
        const int micIdx = static_cast<int>(pluginProcessor.getAPVTS().getRawParameterValue("cabMic")->load());
        const bool micAOffAxis = pluginProcessor.getAPVTS().getRawParameterValue("cabMicAOffAxis")->load() > 0.5f;
        const int speakerBIdx = static_cast<int>(pluginProcessor.getAPVTS().getRawParameterValue("cabSpeakerB")->load());
        const int micBIdx = static_cast<int>(pluginProcessor.getAPVTS().getRawParameterValue("cabMicB")->load());
        const bool micBOffAxis = pluginProcessor.getAPVTS().getRawParameterValue("cabMicBOffAxis")->load() > 0.5f;
        const float micBlend = pluginProcessor.getAPVTS().getRawParameterValue("cabMicBlend")->load();
        const auto speakerName = juce::String(speakerOptionNames[static_cast<size_t>(juce::jlimit(0, 4, speakerIdx))]);
        const auto micName = juce::String(micOptionNames[static_cast<size_t>(juce::jlimit(0, 4, micIdx))]);
        const auto speakerBName = juce::String(speakerOptionNames[static_cast<size_t>(juce::jlimit(0, 4, speakerBIdx))]);
        const auto micBName = juce::String(micOptionNames[static_cast<size_t>(juce::jlimit(0, 4, micBIdx))]);
        cabText = "Cab A: " + speakerName + " / " + micName + (micAOffAxis ? " off" : " on")
                + " | B: " + speakerBName + " / " + micBName + (micBOffAxis ? " off" : " on")
                + " | Blend: " + juce::String(static_cast<int>(std::round(micBlend * 100.0f))) + "%";
        clearIRButton.setEnabled(false);
    }

    const juce::String reverbText = pluginProcessor.hasReverbIR() ? ("Rev IR: " + pluginProcessor.getReverbIRName()) : "Rev IR: none";
    clearReverbIRButton.setEnabled(pluginProcessor.hasReverbIR());
    irStatusLabel.setText(cabText + " | " + reverbText, juce::dontSendNotification);
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
    const int points = juce::jmax(1024, diTimelineArea.getWidth() * 4);
    diWavePeaks = pluginProcessor.getTestDIWaveformPeaks(points);
}

void BackhouseAmpSimAudioProcessorEditor::updateTimelineViewport()
{
    const bool hasFile = pluginProcessor.hasTestDIFile();
    const double duration = juce::jmax(0.1, pluginProcessor.getTestDIDurationSeconds());

    if (!hasFile)
    {
        diViewportStartSeconds = 0.0;
        diViewportEndSeconds = duration;
        diScrollSlider.setEnabled(false);
        return;
    }

    const double zoom = juce::jlimit(0.0, 1.0, diZoomSlider.getValue());
    const double minWindow = juce::jmin(duration, 0.25);
    const double visibleSeconds = juce::jlimit(minWindow, duration, duration * std::pow(0.02, zoom));
    const double maxStart = juce::jmax(0.0, duration - visibleSeconds);

    if (maxStart <= 1.0e-9)
    {
        diViewportStartSeconds = 0.0;
        diViewportEndSeconds = duration;
        diScrollSlider.setEnabled(false);
        diScrollSlider.setValue(0.0, juce::dontSendNotification);
        return;
    }

    diScrollSlider.setEnabled(true);
    const double start = juce::jlimit(0.0, maxStart, diScrollSlider.getValue() * maxStart);
    diViewportStartSeconds = start;
    diViewportEndSeconds = start + visibleSeconds;
}

void BackhouseAmpSimAudioProcessorEditor::captureABSnapshot(size_t slotIndex)
{
    if (slotIndex >= abSnapshots.size())
        return;

    auto& apvts = pluginProcessor.getAPVTS();
    auto& slot = abSnapshots[slotIndex];

    for (size_t i = 0; i < abSnapshotParamIds.size(); ++i)
    {
        if (auto* param = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(abSnapshotParamIds[i])))
            slot.values[i] = param->getValue();
    }

    slot.captured = true;
    currentABSlot = static_cast<int>(slotIndex);
    abSnapshotStatusLabel.setText("A/B: captured slot " + juce::String(slotIndex == 0 ? "A" : "B"), juce::dontSendNotification);
    refreshABSnapshotButtons();
}

void BackhouseAmpSimAudioProcessorEditor::recallABSnapshot(size_t slotIndex)
{
    if (slotIndex >= abSnapshots.size())
        return;

    if (!abSnapshots[slotIndex].captured)
    {
        abSnapshotStatusLabel.setText("A/B: slot " + juce::String(slotIndex == 0 ? "A" : "B") + " not captured", juce::dontSendNotification);
        return;
    }

    auto& apvts = pluginProcessor.getAPVTS();
    const auto& slot = abSnapshots[slotIndex];
    for (size_t i = 0; i < abSnapshotParamIds.size(); ++i)
    {
        if (auto* param = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(abSnapshotParamIds[i])))
            param->setValueNotifyingHost(slot.values[i]);
    }

    currentABSlot = static_cast<int>(slotIndex);
    abSnapshotStatusLabel.setText("A/B: loaded slot " + juce::String(slotIndex == 0 ? "A" : "B"), juce::dontSendNotification);
    refreshABSnapshotButtons();
}

void BackhouseAmpSimAudioProcessorEditor::toggleABSnapshot()
{
    if (!abSnapshots[0].captured || !abSnapshots[1].captured)
    {
        abSnapshotStatusLabel.setText("A/B: capture both slots first", juce::dontSendNotification);
        return;
    }

    recallABSnapshot(static_cast<size_t>(currentABSlot == 0 ? 1 : 0));
}

void BackhouseAmpSimAudioProcessorEditor::refreshABSnapshotButtons()
{
    recallAButton.setEnabled(abSnapshots[0].captured);
    recallBButton.setEnabled(abSnapshots[1].captured);
    toggleABButton.setEnabled(abSnapshots[0].captured && abSnapshots[1].captured);

    recallAButton.setColour(juce::TextButton::buttonColourId, currentABSlot == 0 ? juce::Colours::darkorange : juce::Colours::dimgrey);
    recallBButton.setColour(juce::TextButton::buttonColourId, currentABSlot == 1 ? juce::Colours::darkorange : juce::Colours::dimgrey);
}

void BackhouseAmpSimAudioProcessorEditor::nudgeLoopStart(double deltaSeconds)
{
    if (!pluginProcessor.hasTestDIFile())
        return;

    const double start = pluginProcessor.getTestDILoopStartSeconds();
    const double end = pluginProcessor.getTestDILoopEndSeconds();
    pluginProcessor.setTestDILoopRangeSeconds(juce::jmin(start + deltaSeconds, end - 0.01), end);
    refreshTestDISection();
}

void BackhouseAmpSimAudioProcessorEditor::nudgeLoopEnd(double deltaSeconds)
{
    if (!pluginProcessor.hasTestDIFile())
        return;

    const double start = pluginProcessor.getTestDILoopStartSeconds();
    const double end = pluginProcessor.getTestDILoopEndSeconds();
    pluginProcessor.setTestDILoopRangeSeconds(start, juce::jmax(end + deltaSeconds, start + 0.01));
    refreshTestDISection();
}

void BackhouseAmpSimAudioProcessorEditor::setSelectionFromLoop()
{
    if (!pluginProcessor.hasTestDIFile())
        return;

    diHasSelection = true;
    diSelectionStartSeconds = pluginProcessor.getTestDILoopStartSeconds();
    diSelectionEndSeconds = pluginProcessor.getTestDILoopEndSeconds();
    refreshTestDISection();
}

void BackhouseAmpSimAudioProcessorEditor::clearDISelection()
{
    diHasSelection = false;
    diSelectionStartSeconds = 0.0;
    diSelectionEndSeconds = 0.0;
    refreshTestDISection();
}

void BackhouseAmpSimAudioProcessorEditor::trimDIToSelection()
{
    if (!pluginProcessor.hasTestDIFile() || !diHasSelection)
        return;

    const double s = juce::jmin(diSelectionStartSeconds, diSelectionEndSeconds);
    const double e = juce::jmax(diSelectionStartSeconds, diSelectionEndSeconds);
    if (e - s < 0.001)
        return;

    if (pluginProcessor.trimTestDIToRangeSeconds(s, e))
    {
        diHasSelection = false;
        diSelectionStartSeconds = 0.0;
        diSelectionEndSeconds = 0.0;
        diScrollSlider.setValue(0.0, juce::dontSendNotification);
        diZoomSlider.setValue(0.0, juce::dontSendNotification);
        diWaveCacheWidth = 0;
        updateDITimelineCache();
        refreshTestDISection();
    }
}

void BackhouseAmpSimAudioProcessorEditor::cutDISelection()
{
    if (!pluginProcessor.hasTestDIFile() || !diHasSelection)
        return;

    const double s = juce::jmin(diSelectionStartSeconds, diSelectionEndSeconds);
    const double e = juce::jmax(diSelectionStartSeconds, diSelectionEndSeconds);
    if (e - s < 0.001)
        return;

    if (pluginProcessor.cutTestDIRangeSeconds(s, e))
    {
        diHasSelection = false;
        diSelectionStartSeconds = 0.0;
        diSelectionEndSeconds = 0.0;
        diWaveCacheWidth = 0;
        updateDITimelineCache();
        refreshTestDISection();
    }
}

void BackhouseAmpSimAudioProcessorEditor::selectTransientWindowAt(double seconds)
{
    if (!pluginProcessor.hasTestDIFile())
        return;

    const double duration = pluginProcessor.getTestDIDurationSeconds();
    if (duration <= 0.01)
        return;

    const double clickTime = juce::jlimit(0.0, duration, seconds);

    if (diWavePeaks.empty())
    {
        const double half = 0.06;
        diHasSelection = true;
        diSelectionStartSeconds = juce::jmax(0.0, clickTime - half);
        diSelectionEndSeconds = juce::jmin(duration, clickTime + half);
        return;
    }

    const int n = static_cast<int>(diWavePeaks.size());
    auto timeToIndex = [duration, n](double t) {
        const double norm = juce::jlimit(0.0, 1.0, t / duration);
        return juce::jlimit(0, n - 1, static_cast<int>(norm * static_cast<double>(juce::jmax(1, n - 1))));
    };
    auto indexToTime = [duration, n](int idx) {
        const double norm = static_cast<double>(idx) / static_cast<double>(juce::jmax(1, n - 1));
        return duration * norm;
    };

    const int clickIdx = timeToIndex(clickTime);
    const int searchHalf = juce::jmax(8, static_cast<int>(0.25 * static_cast<double>(n) / juce::jmax(0.1, duration)));
    const int searchStart = juce::jmax(0, clickIdx - searchHalf);
    const int searchEnd = juce::jmin(n - 1, clickIdx + searchHalf);

    int peakIdx = clickIdx;
    float peakVal = diWavePeaks[static_cast<size_t>(clickIdx)];
    for (int i = searchStart; i <= searchEnd; ++i)
    {
        const float v = diWavePeaks[static_cast<size_t>(i)];
        if (v > peakVal)
        {
            peakVal = v;
            peakIdx = i;
        }
    }

    if (peakVal < 0.02f)
    {
        const double half = 0.06;
        diHasSelection = true;
        diSelectionStartSeconds = juce::jmax(0.0, clickTime - half);
        diSelectionEndSeconds = juce::jmin(duration, clickTime + half);
        return;
    }

    const float riseThresh = juce::jmax(0.015f, peakVal * 0.18f);
    const float fallThresh = juce::jmax(0.012f, peakVal * 0.20f);
    const int maxBack = juce::jmax(6, static_cast<int>(0.20 * static_cast<double>(n) / juce::jmax(0.1, duration)));
    const int maxFwd = juce::jmax(8, static_cast<int>(0.50 * static_cast<double>(n) / juce::jmax(0.1, duration)));

    int startIdx = peakIdx;
    for (int i = peakIdx; i >= juce::jmax(0, peakIdx - maxBack); --i)
    {
        if (diWavePeaks[static_cast<size_t>(i)] <= riseThresh)
        {
            startIdx = i;
            break;
        }
    }

    int endIdx = peakIdx;
    for (int i = peakIdx; i <= juce::jmin(n - 1, peakIdx + maxFwd); ++i)
    {
        if (diWavePeaks[static_cast<size_t>(i)] <= fallThresh)
        {
            endIdx = i;
            break;
        }
        endIdx = i;
    }

    double start = juce::jmax(0.0, indexToTime(startIdx) - 0.004);
    double end = juce::jmin(duration, indexToTime(endIdx) + 0.020);
    if (end - start < 0.04)
    {
        const double center = indexToTime(peakIdx);
        start = juce::jmax(0.0, center - 0.020);
        end = juce::jmin(duration, center + 0.080);
    }
    if (end - start > 0.45)
    {
        const double center = indexToTime(peakIdx);
        start = juce::jmax(0.0, center - 0.05);
        end = juce::jmin(duration, center + 0.35);
    }

    diHasSelection = true;
    diSelectionStartSeconds = start;
    diSelectionEndSeconds = end;
}

void BackhouseAmpSimAudioProcessorEditor::refreshTestDISection()
{
    const bool hasFile = pluginProcessor.hasTestDIFile();
    const double duration = pluginProcessor.getTestDIDurationSeconds();
    if (!hasFile)
    {
        diHasSelection = false;
        diSelectionStartSeconds = 0.0;
        diSelectionEndSeconds = 0.0;
    }

    enableDITestButton.setToggleState(pluginProcessor.isTestDIEnabled(), juce::dontSendNotification);
    loopDITestButton.setToggleState(pluginProcessor.isTestDILoopEnabled(), juce::dontSendNotification);
    playDITestButton.setButtonText(pluginProcessor.isTestDIPlaying() ? u8"\u25B6\u2022" : u8"\u25B6");

    const juce::String status = hasFile
        ? "DI: " + pluginProcessor.getTestDIFileName() + "  (" + juce::String(duration, 2) + "s)"
        : "DI: no file loaded";

    diStatusLabel.setText(status, juce::dontSendNotification);

    const auto loopStart = pluginProcessor.getTestDILoopStartSeconds();
    const auto loopEnd = pluginProcessor.getTestDILoopEndSeconds();
    const auto playhead = pluginProcessor.getTestDIPositionSeconds();
    juce::String selectionText = "Selection: none";
    if (diHasSelection)
    {
        const double s = juce::jmin(diSelectionStartSeconds, diSelectionEndSeconds);
        const double e = juce::jmax(diSelectionStartSeconds, diSelectionEndSeconds);
        selectionText = "Selection " + juce::String(s, 2) + "s -> " + juce::String(e, 2) + "s";
    }
    diLoopLabel.setText("Loop " + juce::String(loopStart, 2) + "s -> " + juce::String(loopEnd, 2) + "s    Playhead " + juce::String(playhead, 2) + "s    "
                        + selectionText + "    Drag=Select/Handles  Shift=Playhead  Alt=Loop",
                        juce::dontSendNotification);

    updateTimelineViewport();
    enableDITestButton.setEnabled(hasFile);
    clearDITestButton.setEnabled(hasFile);
    playDITestButton.setEnabled(hasFile && pluginProcessor.isTestDIEnabled());
    pauseDITestButton.setEnabled(hasFile && pluginProcessor.isTestDIEnabled() && pluginProcessor.isTestDIPlaying());
    stopDITestButton.setEnabled(hasFile && pluginProcessor.isTestDIEnabled());
    rwDITestButton.setEnabled(hasFile);
    ffDITestButton.setEnabled(hasFile);
    loopDITestButton.setEnabled(hasFile);
    loopStartBackButton.setEnabled(hasFile);
    loopStartForwardButton.setEnabled(hasFile);
    loopEndBackButton.setEnabled(hasFile);
    loopEndForwardButton.setEnabled(hasFile);
    selectLoopButton.setEnabled(hasFile);
    clearSelectionButton.setEnabled(hasFile && diHasSelection);
    trimSelectionButton.setEnabled(hasFile && diHasSelection);
    cutSelectionButton.setEnabled(hasFile && diHasSelection);
    diZoomSlider.setEnabled(hasFile);
    diZoomLabel.setEnabled(hasFile);
    diScrollLabel.setEnabled(hasFile);
    refreshABSnapshotButtons();
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
    const double viewStart = juce::jlimit(0.0, duration, diViewportStartSeconds);
    const double viewEnd = juce::jlimit(viewStart, duration, diViewportEndSeconds);
    const double viewDuration = juce::jmax(0.001, viewEnd - viewStart);

    const std::array<double, 13> tickCandidates { 0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1.0, 2.0, 5.0, 10.0, 15.0, 30.0, 60.0 };
    const double targetTick = viewDuration / 9.0;
    double tickSeconds = tickCandidates.back();
    for (const auto candidate : tickCandidates)
    {
        if (candidate >= targetTick)
        {
            tickSeconds = candidate;
            break;
        }
    }

    g.setColour(juce::Colours::whitesmoke.withAlpha(0.85f));
    g.setFont(11.0f);
    const double tickStart = std::floor(viewStart / tickSeconds) * tickSeconds;
    for (double t = tickStart; t <= viewEnd + 0.0001; t += tickSeconds)
    {
        if (t < viewStart - 0.0001)
            continue;

        const int x = timelineXFromTime(t);
        g.drawVerticalLine(x, static_cast<float>(ruler.getY() + 12), static_cast<float>(ruler.getBottom()));

        juce::String label;
        if (tickSeconds < 1.0)
            label = juce::String(t, 2) + "s";
        else
        {
            const int minutes = static_cast<int>(t) / 60;
            const int seconds = static_cast<int>(t) % 60;
            label = juce::String(minutes) + ":" + juce::String(seconds).paddedLeft('0', 2);
        }
        g.drawText(label, x + 2, ruler.getY() + 2, 56, 12, juce::Justification::left);
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
            const double norm = static_cast<double>(i) / static_cast<double>(juce::jmax(1, width - 1));
            const double sampleTime = viewStart + norm * viewDuration;
            const int peakIdx = juce::jlimit(0, n - 1, static_cast<int>((sampleTime / duration) * static_cast<double>(n - 1)));
            const float peak = diWavePeaks[static_cast<size_t>(peakIdx)];
            const float half = peak * (lane.getHeight() * 0.46f);
            const float x = static_cast<float>(lane.getX() + i);
            const float cy = static_cast<float>(lane.getCentreY());
            g.drawLine(x, cy - half, x, cy + half, 1.0f);
        }
    }

    if (diHasSelection)
    {
        const double selStart = juce::jmin(diSelectionStartSeconds, diSelectionEndSeconds);
        const double selEnd = juce::jmax(diSelectionStartSeconds, diSelectionEndSeconds);
        const int selStartX = timelineXFromTime(selStart);
        const int selEndX = timelineXFromTime(selEnd);
        g.setColour(juce::Colour(0x2a9f7aea));
        g.fillRect(juce::Rectangle<int>(juce::jmin(selStartX, selEndX),
                                        lane.getY(),
                                        juce::jmax(1, std::abs(selEndX - selStartX)),
                                        lane.getHeight()));
        g.setColour(juce::Colour(0xff8fc3ff));
        g.drawVerticalLine(selStartX, static_cast<float>(lane.getY()), static_cast<float>(lane.getBottom()));
        g.drawVerticalLine(selEndX, static_cast<float>(lane.getY()), static_cast<float>(lane.getBottom()));

        const int handleW = 8;
        const int handleH = 14;
        g.setColour(juce::Colour(0xffd7e8ff));
        g.fillRoundedRectangle(static_cast<float>(selStartX - handleW / 2), static_cast<float>(lane.getCentreY() - handleH / 2),
                               static_cast<float>(handleW), static_cast<float>(handleH), 2.0f);
        g.fillRoundedRectangle(static_cast<float>(selEndX - handleW / 2), static_cast<float>(lane.getCentreY() - handleH / 2),
                               static_cast<float>(handleW), static_cast<float>(handleH), 2.0f);
        g.setColour(juce::Colour(0xff4a5f7d));
        g.drawRoundedRectangle(static_cast<float>(selStartX - handleW / 2), static_cast<float>(lane.getCentreY() - handleH / 2),
                               static_cast<float>(handleW), static_cast<float>(handleH), 2.0f, 1.0f);
        g.drawRoundedRectangle(static_cast<float>(selEndX - handleW / 2), static_cast<float>(lane.getCentreY() - handleH / 2),
                               static_cast<float>(handleW), static_cast<float>(handleH), 2.0f, 1.0f);
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
    const auto lane = diTimelineArea.withTrimmedTop(28).reduced(0, 2);
    if (lane.getWidth() <= 1)
        return 0.0;

    const double norm = juce::jlimit(0.0, 1.0, (x - lane.getX()) / static_cast<double>(lane.getWidth() - 1));
    const double duration = juce::jmax(0.1, pluginProcessor.getTestDIDurationSeconds());
    const double viewStart = juce::jlimit(0.0, duration, diViewportStartSeconds);
    const double viewEnd = juce::jlimit(viewStart, duration, diViewportEndSeconds);
    return viewStart + norm * (viewEnd - viewStart);
}

int BackhouseAmpSimAudioProcessorEditor::timelineXFromTime(double seconds) const
{
    const auto lane = diTimelineArea.withTrimmedTop(28).reduced(0, 2);
    if (lane.getWidth() <= 1)
        return lane.getX();

    const double duration = juce::jmax(0.1, pluginProcessor.getTestDIDurationSeconds());
    const double viewStart = juce::jlimit(0.0, duration, diViewportStartSeconds);
    const double viewEnd = juce::jlimit(viewStart, duration, diViewportEndSeconds);
    const double denom = juce::jmax(1.0e-9, viewEnd - viewStart);
    const double norm = juce::jlimit(0.0, 1.0, (seconds - viewStart) / denom);
    return lane.getX() + static_cast<int>(norm * (lane.getWidth() - 1));
}

#endif
