#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <vector>

#if JucePlugin_Build_Standalone
 #include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

#include "PluginProcessor.h"
#include "BackhouseLookAndFeel.h"

class BackhouseAmpSimAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                  private juce::Timer
{
public:
    explicit BackhouseAmpSimAudioProcessorEditor(BackhouseAmpSimAudioProcessor&);
    ~BackhouseAmpSimAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

private:
    BackhouseLookAndFeel backhouseLAF;  // Must be declared first: constructed before, destroyed after components

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ChoiceAttachment = juce::ParameterAttachment;

    void timerCallback() override;
    void setGuitarProfileFromButton(int profileIndex);
    void refreshGuitarProfileButtons();
    void refreshProfileNameEditors();
    void bindProfileEditControls();
    void bindAmpPreampControl();
    void refreshPreampHeatLabel();
    void refreshTopControlVisibility();
    void refreshTabVisibility();
    void setActiveTab(int tabIndex);
    void setCabSpeakerFromButton(int speakerIndex);
    void setCabMicFromButton(int micIndex);
    void setCabSpeakerBFromButton(int speakerIndex);
    void setCabMicBFromButton(int micIndex);
    void refreshCabSelectorButtons();
    void commitProfileName(int profileIndex, const juce::String& text);
    void refreshIRStatusLabel();
    void refreshProfileIoStatusLabel(const juce::String& message);
    void refreshMidiMapPanel();
    void commitMidiMapRow(int rowIndex);
    void applyMidiTemplate(int templateIndex);
#if JucePlugin_Build_Standalone
    void attachStandaloneDeviceManager();
    void refreshStandaloneMidiDeviceLists();
    void refreshTestDISection();
    void updateDITimelineCache();
    void updateTimelineViewport();
    void drawDITimeline(juce::Graphics&) const;
    double timelineTimeFromX(int x) const;
    int timelineXFromTime(double seconds) const;
    void captureABSnapshot(size_t slotIndex);
    void recallABSnapshot(size_t slotIndex);
    void toggleABSnapshot();
    void refreshABSnapshotButtons();
    void nudgeLoopStart(double deltaSeconds);
    void nudgeLoopEnd(double deltaSeconds);
    void setSelectionFromLoop();
    void clearDISelection();
    void trimDIToSelection();
    void cutDISelection();
    void selectTransientWindowAt(double seconds);
#endif

    BackhouseAmpSimAudioProcessor& pluginProcessor;

    juce::ComboBox ampSelector;
    juce::ComboBox wowModeSelector;
    juce::ComboBox outputCompModeSelector;
    juce::ToggleButton inputBoostButton { "Input Boost" };
    juce::ToggleButton amp2HiwattButton { "Amp 2 Hiwatt" };
    juce::ToggleButton amp4TightButton { "Amp 4 Tight" };
    juce::ToggleButton amp4LowOctaveButton { "Amp 4 Low Oct" };
    juce::ToggleButton cabEnabledButton { "Cab Enabled" };
    juce::ToggleButton reverbEnabledButton { "Reverb" };
    juce::ToggleButton wowEnabledButton { "WOW" };
    juce::ToggleButton wowMomentaryButton { "WOW Moment" };
    juce::ToggleButton wowMidiEnableButton { "WOW MIDI" };
    juce::ToggleButton stompCompEnabledButton { "Comp/Sustain" };
    juce::ToggleButton phaserEnabledButton { "Phaser" };
    juce::ToggleButton amp1TremEnabledButton { "Amp1 Trem" };
    juce::ToggleButton output1176EnabledButton { "1176 Out" };
    juce::ToggleButton outputEqEnabledButton { "EQ3 Out" };
    juce::ToggleButton outputEqHpEnabledButton { "HP" };
    juce::ToggleButton outputEqLpEnabledButton { "LP" };
    juce::ToggleButton tunerButton { "Tuner" };
    juce::ToggleButton tsEnabledButton { "Tube Screamer" };
    juce::ToggleButton delayEnabledButton { "Delay" };
    juce::ToggleButton whammyEnabledButton { "Whammy" };
    juce::ComboBox whammyModeSelector;
    juce::ToggleButton wahEnabledButton { "Wah" };
    std::array<juce::ToggleButton, rigModuleDescriptors.size()> rigModuleButtons;
    juce::TextButton uiModeButton { "Mode: Basic" };
    juce::TextButton tabAmpButton { "Amp" };
    juce::TextButton tabTunerButton { "Tuner" };
    juce::TextButton tabStompsButton { "Stomps" };
    juce::TextButton tabCabButton { "Cab/Mic" };
    juce::TextButton tabSpaceButton { "Space" };
    juce::TextButton tabPostColorButton { "Post Color" };
    juce::TextButton tabMidiButton { "MIDI Map" };

    juce::TextButton profileNeutralButton { "Neutral" };
    juce::TextButton profile1Button { "Guitar 1" };
    juce::TextButton profile2Button { "Guitar 2" };
    juce::TextButton profile3Button { "Guitar 3" };
    juce::TextButton exportProfilesButton { "Export Profiles" };
    juce::TextButton importProfilesButton { "Import Profiles" };
    juce::TextButton loadIRButton { "Load IR" };
    juce::TextButton clearIRButton { "Clear IR" };
    juce::TextButton loadReverbIRButton { "Load Rev IR" };
    juce::TextButton clearReverbIRButton { "Clear Rev IR" };
    juce::TextButton speakerGreenbackButton { "Greenback" };
    juce::TextButton speakerV30Button { "V30" };
    juce::TextButton speakerCreambackButton { "Creamback" };
    juce::TextButton speakerG12TButton { "G12T-75" };
    juce::TextButton speakerJensenButton { "Jensen" };
    juce::TextButton mic57Button { "SM57" };
    juce::TextButton micU87Button { "U87" };
    juce::TextButton micU47FetButton { "U47 FET" };
    juce::TextButton micRoyerButton { "Royer 122" };
    juce::TextButton mic421Button { "MD421" };
    juce::ToggleButton micAOffAxisButton { "A Off-Axis" };
    juce::TextButton speakerBGreenbackButton { "B: Greenback" };
    juce::TextButton speakerBV30Button { "B: V30" };
    juce::TextButton speakerBCreambackButton { "B: Creamback" };
    juce::TextButton speakerBG12TButton { "B: G12T-75" };
    juce::TextButton speakerBJensenButton { "B: Jensen" };
    juce::TextButton micB57Button { "B: SM57" };
    juce::TextButton micBU87Button { "B: U87" };
    juce::TextButton micBU47FetButton { "B: U47 FET" };
    juce::TextButton micBRoyerButton { "B: Royer 122" };
    juce::TextButton micB421Button { "B: MD421" };
    juce::ToggleButton micBOffAxisButton { "B Off-Axis" };
    juce::TextButton rigResetButton { "Reset Rig" };

    juce::TextEditor profile1NameEditor;
    juce::TextEditor profile2NameEditor;
    juce::TextEditor profile3NameEditor;

    juce::Slider inputGain;
    juce::Slider stompInputGain;
    juce::Slider ampPreampGain;
    juce::Slider sub;
    juce::Slider low;
    juce::Slider mid;
    juce::Slider high;
    juce::Slider presence;
    juce::Slider output;
    juce::Slider gateThreshold;
    juce::Slider cabTone;
    juce::Slider reverbMix;
    juce::Slider wowPosition;
    juce::Slider wowMix;
    juce::Slider stompCompSustain;
    juce::Slider stompCompLevel;
    juce::Slider stompCompMix;
    juce::Slider tsDrive;
    juce::Slider tsTone;
    juce::Slider tsLevel;
    juce::Slider phaserRate;
    juce::Slider phaserDepth;
    juce::Slider phaserCenter;
    juce::Slider phaserFeedback;
    juce::Slider phaserMix;
    juce::Slider amp1TremRate;
    juce::Slider amp1TremDepth;
    juce::Slider output1176Input;
    juce::Slider output1176Release;
    juce::Slider output1176Mix;
    juce::Slider outputEq63;
    juce::Slider outputEq160;
    juce::Slider outputEq400;
    juce::Slider outputEq1000;
    juce::Slider outputEq2500;
    juce::Slider outputEq6300;
    juce::Slider outputEq12000;
    juce::Slider outputEqQ63;
    juce::Slider outputEqQ160;
    juce::Slider outputEqQ400;
    juce::Slider outputEqQ1000;
    juce::Slider outputEqQ2500;
    juce::Slider outputEqQ6300;
    juce::Slider outputEqQ12000;
    juce::Slider outputEqHpHz;
    juce::Slider outputEqLpHz;
    juce::Slider rigMaxPointsSlider;
    juce::Slider delayTime;
    juce::Slider delayFeedback;
    juce::Slider delayMix;
    juce::Slider delayTone;
    juce::Slider delayPitch;
    juce::Slider amp4OctaveMix;
    juce::Slider amp4DirectMix;
    juce::Slider amp4CleanMix;
    juce::Slider cabMicBlend;
    juce::Slider whammyPosition;
    juce::Slider whammyMix;
    juce::Slider wahPosition;
    juce::Slider wahMix;

    juce::Slider profilePickupOutput;
    juce::Slider profileBrightness;
    juce::Slider profileLowEnd;
    juce::Slider profileGateTrim;
    juce::Slider tunerTranspose;

    juce::Label pitchLabel;
    juce::Label profileEditLabel;
    juce::Label profileIoStatusLabel;
    juce::Label irStatusLabel;
    juce::Label cabSpeakerLabel;
    juce::Label cabMicLabel;
    juce::Label cabSpeakerBLabel;
    juce::Label cabMicBLabel;
    juce::Label meterInputLabel;
    juce::Label meterOutputLabel;
    juce::Label preampHeatLabel;
    juce::Label rigPointsLabel;
    juce::Label tunerNoteLabel;
    juce::Label tunerDetuneLabel;
    juce::Label tunerTransposeLabel;
    juce::Label midiMapHelpLabel;
    juce::TextButton midiTemplateRhythmButton { "Template: Rhythm" };
    juce::TextButton midiTemplateLeadButton { "Template: Lead" };
    juce::TextButton midiTemplateAmbientButton { "Template: Ambient" };
    juce::Rectangle<int> tunerNeedleArea;
    juce::Rectangle<int> outputSpectrumArea;
    float tunerNeedleCents = 0.0f;
    juce::String tunerNeedleNote = "--";
    std::array<float, 16> outputSpectrumValues {};

    struct MidiMapRow
    {
        juce::Label slotLabel;
        juce::ToggleButton enabledButton { "On" };
        juce::Slider ccSlider;
        juce::ComboBox targetBox;
        juce::ComboBox modeBox;
        juce::Slider minSlider;
        juce::Slider maxSlider;
        juce::ToggleButton invertButton { "Inv" };
    };
    std::array<MidiMapRow, 8> midiMapRows;
    double inputMeterValue = 0.0;
    double outputMeterValue = 0.0;
    juce::ProgressBar inputMeterBar { inputMeterValue };
    juce::ProgressBar outputMeterBar { outputMeterValue };

#if JucePlugin_Build_Standalone
    juce::Label testBenchLabel;
    juce::TextButton audioSettingsButton { "Audio/MIDI Settings..." };
    juce::Label midiInputLabel;
    juce::Label midiOutputLabel;
    juce::ComboBox midiInputSelector;
    juce::ComboBox midiOutputSelector;

    juce::TextButton loadDITestButton { "Load DI" };
    juce::TextButton clearDITestButton { "Clear DI" };
    juce::ToggleButton enableDITestButton { "Use DI File" };
    juce::TextButton playDITestButton { u8"\u25B6" };    // ▶
    juce::TextButton pauseDITestButton { u8"\u23F8" };   // ⏸
    juce::TextButton stopDITestButton { u8"\u23F9" };    // ⏹
    juce::TextButton rwDITestButton { u8"\u23EE" };      // ⏮
    juce::TextButton ffDITestButton { u8"\u23ED" };
    juce::ToggleButton loopDITestButton { "Loop" };
    juce::TextButton loopStartBackButton { "S-10ms" };
    juce::TextButton loopStartForwardButton { "S+10ms" };
    juce::TextButton loopEndBackButton { "E-10ms" };
    juce::TextButton loopEndForwardButton { "E+10ms" };
    juce::TextButton selectLoopButton { "Sel=Loop" };
    juce::TextButton trimSelectionButton { "Trim Sel" };
    juce::TextButton cutSelectionButton { "Cut Sel" };
    juce::TextButton clearSelectionButton { "Clear Sel" };
    juce::Label diGainLabel;
    juce::Slider diGainSlider;
    juce::Label diZoomLabel;
    juce::Label diScrollLabel;
    juce::Slider diZoomSlider;
    juce::Slider diScrollSlider;
    juce::TextButton captureAButton { "Cap A" };
    juce::TextButton recallAButton { "Load A" };
    juce::TextButton captureBButton { "Cap B" };
    juce::TextButton recallBButton { "Load B" };
    juce::TextButton toggleABButton { "A/B" };
    juce::Label abSnapshotStatusLabel;
    juce::Label diStatusLabel;
    juce::Label diLoopLabel;
    juce::Rectangle<int> diTimelineArea;
    std::vector<float> diWavePeaks;
    double diViewportStartSeconds = 0.0;
    double diViewportEndSeconds = 0.0;
    int diWaveCacheWidth = 0;
    bool diDialogOpen = false;
    juce::File lastDIBrowseLocation;
    bool diHasSelection = false;
    double diSelectionStartSeconds = 0.0;
    double diSelectionEndSeconds = 0.0;

    static constexpr auto abSnapshotParamIds = std::to_array<const char*>({
        "ampType", "guitarProfile", "inputGain", "stompInputGain", "amp1PreampGain", "amp2PreampGain", "amp3PreampGain", "amp4PreampGain",
        "inputBoost", "sub", "low", "mid", "high",
        "presence", "output", "gateThresh", "tsEnabled", "tsDrive", "tsTone", "tsLevel",
        "delayEnabled", "delayTimeMs", "delayFeedback", "delayMix", "delayTone", "delayPitch",
        "cabEnabled", "cabTone", "cabSpeaker", "cabMic",
        "cabMicAOffAxis", "cabSpeakerB", "cabMicB", "cabMicBOffAxis", "cabMicBlend",
        "reverbEnabled", "reverbMix", "wowEnabled", "wowMomentary", "wowMidiEnable", "wowMode", "wowPosition", "wowMix",
        "stompCompEnabled", "stompCompSustain", "stompCompLevel", "stompCompMix",
        "phaserEnabled", "phaserRate", "phaserDepth", "phaserCenterHz", "phaserFeedback", "phaserMix",
        "output1176Enabled", "outputCompMode", "output1176Input", "output1176Release", "output1176Mix",
        "outputEqEnabled", "outputEq63", "outputEq160", "outputEq400", "outputEq1000", "outputEq2500", "outputEq6300", "outputEq12000",
        "rigAmp1", "rigAmp3", "rigCompressor", "rigCab", "rigDelay", "rigEQ", "rigPhaser", "rigTrem", "rigWOW", "rigMaxPoints",
        "outputEqQ63", "outputEqQ160", "outputEqQ400", "outputEqQ1000", "outputEqQ2500", "outputEqQ6300", "outputEqQ12000",
        "outputEqHpEnabled", "outputEqHpHz", "outputEqLpEnabled", "outputEqLpHz",
        "amp1TremEnabled", "amp1TremRate", "amp1TremDepth",
        "amp2Hiwatt", "amp4Tight", "amp4LowOctave",
        "amp4LowOctaveMix", "amp4DirectMix", "amp4CleanAmpMix", "tunerEnabled", "tunerTranspose",
        "whammyEnabled", "whammyMode", "whammyPosition", "whammyMix",
        "wahEnabled", "wahPosition", "wahMix",
        "profile1PickupOutput", "profile1Brightness", "profile1LowEnd", "profile1GateTrim",
        "profile2PickupOutput", "profile2Brightness", "profile2LowEnd", "profile2GateTrim",
        "profile3PickupOutput", "profile3Brightness", "profile3LowEnd", "profile3GateTrim"
    });

    struct ABSnapshot
    {
        bool captured = false;
        std::array<float, abSnapshotParamIds.size()> values {};
    };
    std::array<ABSnapshot, 2> abSnapshots;
    int currentABSlot = 0;

    enum class DITimelineDragMode { none, playhead, loopStart, loopEnd, selectRegion, selectStartHandle, selectEndHandle };
    DITimelineDragMode diTimelineDragMode = DITimelineDragMode::none;

    juce::AudioDeviceManager* standaloneDeviceManager = nullptr;
    juce::StringArray midiInputIds;
    juce::StringArray midiOutputIds;
#endif

    std::unique_ptr<juce::FileChooser> irChooser;
    std::unique_ptr<juce::FileChooser> reverbChooser;
    std::unique_ptr<juce::FileChooser> profileChooser;
    std::unique_ptr<juce::FileChooser> diChooser;

    std::unique_ptr<ComboAttachment> ampAttachment;
    std::unique_ptr<ChoiceAttachment> ampTypeAttachment;
    std::unique_ptr<ChoiceAttachment> guitarProfileAttachment;
    std::unique_ptr<ChoiceAttachment> cabSpeakerChoiceAttachment;
    std::unique_ptr<ChoiceAttachment> cabMicChoiceAttachment;
    std::unique_ptr<ComboAttachment> wowModeAttachment;
    std::unique_ptr<ComboAttachment> outputCompModeAttachment;
    std::unique_ptr<ChoiceAttachment> cabSpeakerBChoiceAttachment;
    std::unique_ptr<ChoiceAttachment> cabMicBChoiceAttachment;
    std::unique_ptr<ButtonAttachment> inputBoostAttachment;
    std::unique_ptr<ButtonAttachment> amp2HiwattAttachment;
    std::unique_ptr<ButtonAttachment> amp4TightAttachment;
    std::unique_ptr<ButtonAttachment> amp4LowOctaveAttachment;
    std::unique_ptr<ButtonAttachment> cabEnabledAttachment;
    std::unique_ptr<ButtonAttachment> reverbEnabledAttachment;
    std::unique_ptr<ButtonAttachment> wowEnabledAttachment;
    std::unique_ptr<ButtonAttachment> wowMomentaryAttachment;
    std::unique_ptr<ButtonAttachment> wowMidiEnableAttachment;
    std::unique_ptr<ButtonAttachment> stompCompEnabledAttachment;
    std::unique_ptr<ButtonAttachment> phaserEnabledAttachment;
    std::unique_ptr<ButtonAttachment> amp1TremEnabledAttachment;
    std::unique_ptr<ButtonAttachment> output1176EnabledAttachment;
    std::unique_ptr<ButtonAttachment> outputEqEnabledAttachment;
    std::unique_ptr<ButtonAttachment> outputEqHpEnabledAttachment;
    std::unique_ptr<ButtonAttachment> outputEqLpEnabledAttachment;
    std::array<std::unique_ptr<ButtonAttachment>, rigModuleDescriptors.size()> rigModuleAttachments;
    std::unique_ptr<ButtonAttachment> tunerAttachment;
    std::unique_ptr<ButtonAttachment> tsEnabledAttachment;
    std::unique_ptr<ButtonAttachment> delayEnabledAttachment;
    std::unique_ptr<ButtonAttachment> whammyEnabledAttachment;
    std::unique_ptr<ComboAttachment> whammyModeAttachment;
    std::unique_ptr<ButtonAttachment> wahEnabledAttachment;
    std::unique_ptr<ButtonAttachment> cabMicAOffAxisAttachment;
    std::unique_ptr<ButtonAttachment> cabMicBOffAxisAttachment;

    std::unique_ptr<SliderAttachment> inputGainAttachment;
    std::unique_ptr<SliderAttachment> stompInputGainAttachment;
    std::unique_ptr<SliderAttachment> ampPreampAttachment;
    std::unique_ptr<SliderAttachment> subAttachment;
    std::unique_ptr<SliderAttachment> lowAttachment;
    std::unique_ptr<SliderAttachment> midAttachment;
    std::unique_ptr<SliderAttachment> highAttachment;
    std::unique_ptr<SliderAttachment> presenceAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<SliderAttachment> gateAttachment;
    std::unique_ptr<SliderAttachment> cabToneAttachment;
    std::unique_ptr<SliderAttachment> reverbMixAttachment;
    std::unique_ptr<SliderAttachment> wowPositionAttachment;
    std::unique_ptr<SliderAttachment> wowMixAttachment;
    std::unique_ptr<SliderAttachment> stompCompSustainAttachment;
    std::unique_ptr<SliderAttachment> stompCompLevelAttachment;
    std::unique_ptr<SliderAttachment> stompCompMixAttachment;
    std::unique_ptr<SliderAttachment> tsDriveAttachment;
    std::unique_ptr<SliderAttachment> tsToneAttachment;
    std::unique_ptr<SliderAttachment> tsLevelAttachment;
    std::unique_ptr<SliderAttachment> phaserRateAttachment;
    std::unique_ptr<SliderAttachment> phaserDepthAttachment;
    std::unique_ptr<SliderAttachment> phaserCenterAttachment;
    std::unique_ptr<SliderAttachment> phaserFeedbackAttachment;
    std::unique_ptr<SliderAttachment> phaserMixAttachment;
    std::unique_ptr<SliderAttachment> amp1TremRateAttachment;
    std::unique_ptr<SliderAttachment> amp1TremDepthAttachment;
    std::unique_ptr<SliderAttachment> output1176InputAttachment;
    std::unique_ptr<SliderAttachment> output1176ReleaseAttachment;
    std::unique_ptr<SliderAttachment> output1176MixAttachment;
    std::unique_ptr<SliderAttachment> outputEq63Attachment;
    std::unique_ptr<SliderAttachment> outputEq160Attachment;
    std::unique_ptr<SliderAttachment> outputEq400Attachment;
    std::unique_ptr<SliderAttachment> outputEq1000Attachment;
    std::unique_ptr<SliderAttachment> outputEq2500Attachment;
    std::unique_ptr<SliderAttachment> outputEq6300Attachment;
    std::unique_ptr<SliderAttachment> outputEq12000Attachment;
    std::unique_ptr<SliderAttachment> outputEqQ63Attachment;
    std::unique_ptr<SliderAttachment> outputEqQ160Attachment;
    std::unique_ptr<SliderAttachment> outputEqQ400Attachment;
    std::unique_ptr<SliderAttachment> outputEqQ1000Attachment;
    std::unique_ptr<SliderAttachment> outputEqQ2500Attachment;
    std::unique_ptr<SliderAttachment> outputEqQ6300Attachment;
    std::unique_ptr<SliderAttachment> outputEqQ12000Attachment;
    std::unique_ptr<SliderAttachment> outputEqHpHzAttachment;
    std::unique_ptr<SliderAttachment> outputEqLpHzAttachment;
    std::unique_ptr<SliderAttachment> rigMaxPointsAttachment;
    std::unique_ptr<SliderAttachment> delayTimeAttachment;
    std::unique_ptr<SliderAttachment> delayFeedbackAttachment;
    std::unique_ptr<SliderAttachment> delayMixAttachment;
    std::unique_ptr<SliderAttachment> delayToneAttachment;
    std::unique_ptr<SliderAttachment> delayPitchAttachment;
    std::unique_ptr<SliderAttachment> amp4OctaveMixAttachment;
    std::unique_ptr<SliderAttachment> amp4DirectMixAttachment;
    std::unique_ptr<SliderAttachment> amp4CleanMixAttachment;
    std::unique_ptr<SliderAttachment> cabMicBlendAttachment;
    std::unique_ptr<SliderAttachment> whammyPositionAttachment;
    std::unique_ptr<SliderAttachment> whammyMixAttachment;
    std::unique_ptr<SliderAttachment> wahPositionAttachment;
    std::unique_ptr<SliderAttachment> wahMixAttachment;

    std::unique_ptr<SliderAttachment> profilePickupAttachment;
    std::unique_ptr<SliderAttachment> profileBrightnessAttachment;
    std::unique_ptr<SliderAttachment> profileLowEndAttachment;
    std::unique_ptr<SliderAttachment> profileGateAttachment;
    std::unique_ptr<SliderAttachment> tunerTransposeAttachment;

    bool showAdvancedTopControls = false;
    int activeTabIndex = 0; // 0=Tuner, 1=Stomps, 2=Amp, 3=Cab/Mic, 4=Post Color, 5=Space, 6=MIDI

    juce::Typeface::Ptr dharmaPunkTypeface;
    juce::Typeface::Ptr punkaholicTypeface;
    juce::Image logoImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BackhouseAmpSimAudioProcessorEditor)
};
