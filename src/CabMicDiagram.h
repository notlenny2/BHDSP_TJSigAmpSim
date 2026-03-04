#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

class CabMicDiagram : public juce::Component, private juce::Timer
{
public:
    CabMicDiagram (juce::AudioProcessorValueTreeState& apvts,
                   juce::Typeface::Ptr dharmaPunk,
                   juce::Typeface::Ptr punkaholic);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp   (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    void timerCallback() override { repaint(); }

    struct SlotGeo { float coneCx = 0, coneCy = 0, maxR = 0; };
    enum class DragTarget { None, MicA, MicB, BlendBar };

    juce::AudioProcessorValueTreeState& apvts;
    juce::Typeface::Ptr dharmaPunkTypeface;
    juce::Typeface::Ptr punkaholicTypeface;

    std::array<SlotGeo, 2> slotGeo {};
    juce::Rectangle<float> blendBarBounds {};
    DragTarget activeDrag = DragTarget::None;
    juce::Point<float> liveDragPos {};

    DragTarget hitTest (juce::Point<float> pos) const noexcept;
    void applyMicPos  (int slot, juce::Point<float> localPos);
    void setNormParam (const juce::String& id, float norm);
};
