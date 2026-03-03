#include "BackhouseLookAndFeel.h"
#include <cmath>

namespace
{
const juce::Colour bhlPanelBase    (0xff111219);
const juce::Colour bhlPanelOutline (0xff22253a);
const juce::Colour bhlTextMain     (0xffecf0ff);
} // namespace

BackhouseLookAndFeel::BackhouseLookAndFeel() = default;

void BackhouseLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                              int x, int y, int width, int height,
                                              float sliderPosProportional,
                                              float rotaryStartAngle,
                                              float rotaryEndAngle,
                                              juce::Slider& slider)
{
    const float cx = static_cast<float> (x) + static_cast<float> (width)  * 0.5f;
    const float cy = static_cast<float> (y) + static_cast<float> (height) * 0.5f;
    const float outerR = juce::jmin (static_cast<float> (width), static_cast<float> (height)) * 0.5f - 4.0f;
    const float trackR = outerR * 0.82f;

    const juce::Colour fillColour = slider.findColour (juce::Slider::thumbColourId);
    const float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Background arc track
    juce::Path trackArc;
    trackArc.addCentredArc (cx, cy, trackR, trackR, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (bhlPanelOutline.brighter (0.15f));
    g.strokePath (trackArc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

    // Filled value arc
    if (currentAngle > rotaryStartAngle)
    {
        juce::Path fillArc;
        fillArc.addCentredArc (cx, cy, trackR, trackR, 0.0f, rotaryStartAngle, currentAngle, true);
        g.setColour (fillColour);
        g.strokePath (fillArc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
    }

    // Knob body: dark filled circle with tinted border
    const float knobR = outerR * 0.58f;
    g.setColour (juce::Colour (0xff1a1c24));
    g.fillEllipse (cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f);
    g.setColour (fillColour.withAlpha (0.55f));
    g.drawEllipse (cx - knobR + 0.5f, cy - knobR + 0.5f,
                   knobR * 2.0f - 1.0f, knobR * 2.0f - 1.0f, 1.2f);

    // Pointer line — JUCE rotary angles are measured from 12 o'clock, clockwise.
    // Map to screen coords: x = sin(a), y = -cos(a)
    const float pInnerR = knobR * 0.28f;
    const float pOuterR = knobR * 0.80f;
    g.setColour (fillColour);
    g.drawLine (cx + pInnerR * std::sin (currentAngle), cy - pInnerR * std::cos (currentAngle),
                cx + pOuterR * std::sin (currentAngle), cy - pOuterR * std::cos (currentAngle),
                1.5f);
}

void BackhouseLookAndFeel::drawToggleButton (juce::Graphics& g,
                                              juce::ToggleButton& button,
                                              bool /*shouldDrawButtonAsHighlighted*/,
                                              bool /*shouldDrawButtonAsDown*/)
{
    const bool isOn = button.getToggleState();
    const auto bounds = button.getLocalBounds().toFloat();

    if (button.getName().startsWith ("stomp:"))
    {
        // Circular footswitch with LED above cap
        const float size = juce::jmin (bounds.getWidth(), bounds.getHeight());
        const float cx = bounds.getCentreX();
        const float cy = bounds.getCentreY();

        // LED
        const juce::Colour ledColour = isOn ? juce::Colour (0xff00ff44)
                                            : juce::Colour (0xff332233);
        const float ledR  = size * 0.09f;
        const float ledCy = cy - size * 0.25f;

        if (isOn)
        {
            // LED glow halo
            g.setColour (ledColour.withAlpha (0.30f));
            g.fillEllipse (cx - ledR * 2.8f, ledCy - ledR * 2.8f,
                           ledR * 5.6f, ledR * 5.6f);
        }
        g.setColour (ledColour);
        g.fillEllipse (cx - ledR, ledCy - ledR, ledR * 2.0f, ledR * 2.0f);
        g.setColour (isOn ? juce::Colour (0xff00cc33) : juce::Colour (0xff220022));
        g.drawEllipse (cx - ledR, ledCy - ledR, ledR * 2.0f, ledR * 2.0f, 0.8f);

        // Footswitch cap
        const float capR  = size * 0.28f;
        const float capCy = cy + size * 0.06f;
        g.setColour (isOn ? juce::Colour (0xff252534) : juce::Colour (0xff16161e));
        g.fillEllipse (cx - capR, capCy - capR, capR * 2.0f, capR * 2.0f);
        g.setColour (isOn ? juce::Colour (0xff4466aa) : bhlPanelOutline.withAlpha (0.9f));
        g.drawEllipse (cx - capR, capCy - capR, capR * 2.0f, capR * 2.0f, 1.5f);
    }
    else
    {
        // Standard pill-shaped toggle
        const juce::Colour accent = button.findColour (juce::ToggleButton::tickColourId);
        g.setColour (isOn ? accent.withAlpha (0.22f) : bhlPanelBase.withAlpha (0.85f));
        g.fillRoundedRectangle (bounds.reduced (1.0f), 5.0f);
        g.setColour (isOn ? accent : bhlPanelOutline);
        g.drawRoundedRectangle (bounds.reduced (1.0f), 5.0f, 1.2f);

        g.setColour (isOn ? juce::Colours::white : bhlTextMain);
        // Responsive size: scale with button height, capped so it never overflows
        g.setFont (juce::jmin (15.0f, static_cast<float> (button.getHeight()) * 0.52f));
        g.drawText (button.getButtonText(), bounds.toNearestInt(),
                    juce::Justification::centred, true);
    }
}

void BackhouseLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                                  juce::Button& button,
                                                  const juce::Colour& /*backgroundColour*/,
                                                  bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const bool isOn = button.getToggleState();

    const juce::Colour bgColour = isOn
        ? button.findColour (juce::TextButton::buttonOnColourId)
        : button.findColour (juce::TextButton::buttonColourId);

    g.setColour (bgColour);
    g.fillRoundedRectangle (bounds, 5.0f);

    if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
    {
        g.setColour (juce::Colours::white.withAlpha (0.08f));
        g.fillRoundedRectangle (bounds, 5.0f);
    }

    // Border: brighter on active
    g.setColour (isOn ? bgColour.brighter (0.45f) : bhlPanelOutline.withAlpha (0.8f));
    g.drawRoundedRectangle (bounds, 5.0f, 1.0f);
}

void BackhouseLookAndFeel::drawButtonText (juce::Graphics& g,
                                            juce::TextButton& button,
                                            bool /*shouldDrawButtonAsHighlighted*/,
                                            bool /*shouldDrawButtonAsDown*/)
{
    const bool isOn = button.getToggleState();
    g.setColour (isOn ? juce::Colours::white : bhlTextMain);
    g.setFont (juce::jmin (16.0f, static_cast<float> (button.getHeight()) * 0.50f));
    g.drawText (button.getButtonText(), button.getLocalBounds(),
                juce::Justification::centred, true);
}
