#include "CabMicDiagram.h"
#include "BinaryData.h"
#include <cmath>

namespace
{
constexpr juce::uint32 spkColours[5] = {
    0xff44cc88,  // Greenback
    0xffff8844,  // V30
    0xffe8c87a,  // Creamback
    0xff4499ff,  // G12T-75
    0xffaa66ff,  // Jensen
};
constexpr juce::uint32 micColours[5] = {
    0xff00d4ff,  // SM57
    0xff4477dd,  // U87
    0xff9933cc,  // U47 FET
    0xffcc6b2e,  // Royer 122
    0xff3dcc66,  // MD421
};
constexpr const char* spkShort[5] = { "Greenback", "V30", "Creamback", "G12T-75", "Jensen" };
constexpr const char* micShort[5] = { "SM57", "U87", "U47 FET", "Royer 122", "MD421" };

const juce::Colour panelOutline (0xff22253a);
const juce::Colour textMuted    (0xff7880a0);
} // namespace

CabMicDiagram::CabMicDiagram (juce::AudioProcessorValueTreeState& a,
                               juce::Typeface::Ptr dharmaPunk,
                               juce::Typeface::Ptr punkaholic)
    : apvts (a)
    , dharmaPunkTypeface (dharmaPunk)
    , punkaholicTypeface (punkaholic)
{
    setMouseCursor (juce::MouseCursor::NormalCursor);
    startTimerHz (30);
}

// ─────────────────────────────────────────────────────────────────────────────
void CabMicDiagram::paint (juce::Graphics& g)
{
    const int selSpkA  = juce::jlimit (0, 4, (int) apvts.getRawParameterValue ("cabSpeaker")->load());
    const int selMicA  = juce::jlimit (0, 4, (int) apvts.getRawParameterValue ("cabMic")->load());
    const int selSpkB  = juce::jlimit (0, 4, (int) apvts.getRawParameterValue ("cabSpeakerB")->load());
    const int selMicB  = juce::jlimit (0, 4, (int) apvts.getRawParameterValue ("cabMicB")->load());
    const bool offAxisA = apvts.getRawParameterValue ("cabMicAOffAxis")->load() > 0.5f;
    const bool offAxisB = apvts.getRawParameterValue ("cabMicBOffAxis")->load() > 0.5f;
    const float blendNorm = apvts.getRawParameterValue ("cabMicBlend")->load();

    const int slotW   = juce::jmin (280, (getWidth() - 50) / 2);
    const int slotH   = juce::jmin (170, getHeight() - 30);
    const int diagCx  = getWidth() / 2;
    const int slotGap = 30;
    const int aSlotX  = diagCx - slotGap / 2 - slotW;
    const int bSlotX  = diagCx + slotGap / 2;
    const int slotY   = 4;

    const juce::Colour spkColA (spkColours[selSpkA]);
    const juce::Colour micColA (micColours[selMicA]);
    const juce::Colour spkColB (spkColours[selSpkB]);
    const juce::Colour micColB (micColours[selMicB]);

    // ── Draw one cab slot ────────────────────────────────────────────────────
    auto drawSlot = [&](int slotIdx, int sx, int sy, int sw, int sh,
                        const char* spkName, const char* micName,
                        bool offAxis,
                        juce::Colour spkCol, juce::Colour micCol,
                        const juce::String& chanLabel)
    {
        const auto sf = juce::Rectangle<float> ((float) sx, (float) sy, (float) sw, (float) sh);

        // Background + border
        g.setColour (juce::Colour (0xff0e1018));
        g.fillRoundedRectangle (sf, 12.0f);
        g.setColour (spkCol.withAlpha (0.45f));
        g.drawRoundedRectangle (sf.reduced (0.5f), 12.0f, 1.6f);

        // Channel badge
        g.setColour (spkCol.withAlpha (0.18f));
        g.fillRoundedRectangle ((float) sx + 8.0f, (float) sy + 7.0f, 24.0f, 18.0f, 4.0f);
        if (dharmaPunkTypeface)
            g.setFont (juce::Font (juce::FontOptions (dharmaPunkTypeface).withHeight (17.0f)));
        else
            g.setFont (16.0f);
        g.setColour (spkCol);
        g.drawText (chanLabel, sx + 8, sy + 6, 26, 22, juce::Justification::centred, true);

        // Speaker cone centre
        const float coneCx = (float) sx + (float) sw * 0.40f;
        const float coneCy = (float) sy + (float) sh * 0.52f;
        const float maxR   = juce::jmin ((float) sw * 0.32f, (float) sh * 0.43f);

        // Store geometry for hit testing
        slotGeo[slotIdx] = { coneCx, coneCy, maxR };

        // Glow halo
        g.setColour (spkCol.withAlpha (0.06f));
        g.fillEllipse (coneCx - maxR * 1.1f, coneCy - maxR * 1.1f, maxR * 2.2f, maxR * 2.2f);

        // Cone rings
        struct Ring { float frac, alpha; };
        constexpr Ring rings[] = { {1.00f,0.14f},{0.78f,0.22f},{0.58f,0.32f},{0.38f,0.45f},{0.20f,0.62f} };
        for (auto& rng : rings)
        {
            const float r = maxR * rng.frac;
            g.setColour (spkCol.withAlpha (rng.alpha));
            g.drawEllipse (coneCx - r, coneCy - r, r * 2.0f, r * 2.0f, 1.1f);
        }

        // Dust cap
        const float dustR = maxR * 0.11f;
        g.setColour (spkCol.withAlpha (0.70f));
        g.fillEllipse (coneCx - dustR, coneCy - dustR, dustR * 2.0f, dustR * 2.0f);

        // ── Mic position ──────────────────────────────────────────────────
        float micX, micY;
        const DragTarget thisDrag = (slotIdx == 0) ? DragTarget::MicA : DragTarget::MicB;

        if (activeDrag == thisDrag)
        {
            // Follow cursor, clamped to cone
            const float dx   = liveDragPos.x - coneCx;
            const float dy   = liveDragPos.y - coneCy;
            const float dist = std::sqrt (dx * dx + dy * dy);
            const float cap  = maxR * 0.90f;
            if (dist > 0.001f)
            {
                const float clamped = juce::jmin (dist, cap);
                micX = coneCx + (dx / dist) * clamped;
                micY = coneCy + (dy / dist) * clamped;
            }
            else { micX = coneCx; micY = coneCy; }
        }
        else
        {
            const float micDist = offAxis ? maxR * 0.54f : 0.0f;
            const float micAng  = juce::MathConstants<float>::pi * 0.3f;
            micX = coneCx + micDist * std::sin (micAng);
            micY = coneCy - micDist * std::cos (micAng);
        }

        const bool showOffAxis = (activeDrag == thisDrag)
            ? (std::hypot (micX - coneCx, micY - coneCy) > maxR * 0.28f)
            : offAxis;

        if (showOffAxis)
        {
            g.setColour (micCol.withAlpha (0.28f));
            float dash[] = { 4.0f, 3.0f };
            g.drawDashedLine (juce::Line<float> (coneCx, coneCy, micX, micY), dash, 2, 0.8f);
        }

        // Glow + capsule
        const float mh = 16.0f, mw = 6.0f;
        g.setColour (micCol.withAlpha (0.22f));
        g.fillEllipse (micX - mw, micY - mh * 0.5f - 2.0f, mw * 2.0f, mh + 4.0f);
        g.setColour (micCol.withAlpha (0.85f));
        g.fillRoundedRectangle (micX - mw * 0.5f, micY - mh * 0.5f, mw, mh, mw * 0.5f);
        g.setColour (micCol.brighter (0.3f));
        g.drawRoundedRectangle (micX - mw * 0.5f, micY - mh * 0.5f, mw, mh, mw * 0.5f, 1.0f);
        g.setColour (micCol.withAlpha (0.40f));
        g.drawLine (micX, micY + mh * 0.5f, micX, micY + mh, 1.0f);

        // Labels
        const int labelX = sx + (int) ((float) sw * 0.65f);
        const int labelW = sw - (int) ((float) sw * 0.65f) - 8;
        if (dharmaPunkTypeface)
            g.setFont (juce::Font (juce::FontOptions (dharmaPunkTypeface).withHeight (14.0f)));
        else
            g.setFont (14.0f);
        g.setColour (spkCol.brighter (0.1f));
        g.drawText (spkName, labelX, (int) coneCy - 30, labelW, 17, juce::Justification::centredLeft, true);
        g.setColour (micCol.brighter (0.1f));
        g.drawText (micName, labelX, (int) coneCy - 10, labelW, 17, juce::Justification::centredLeft, true);

        g.setColour (showOffAxis ? micCol.withAlpha (0.65f) : textMuted.withAlpha (0.5f));
        if (punkaholicTypeface)
            g.setFont (juce::Font (juce::FontOptions (punkaholicTypeface).withHeight (12.0f)));
        else
            g.setFont (12.0f);
        g.drawText (showOffAxis ? "off-axis" : "on-axis",
                    labelX, (int) coneCy + 10, labelW, 15, juce::Justification::centredLeft, true);

        // Hint ring when hovering
        const DragTarget hoverCheck = (slotIdx == 0) ? DragTarget::MicA : DragTarget::MicB;
        if (activeDrag == hoverCheck)
        {
            g.setColour (micCol.withAlpha (0.18f));
            g.drawEllipse (coneCx - maxR * 0.30f, coneCy - maxR * 0.30f,
                           maxR * 0.60f, maxR * 0.60f, 0.8f);
        }
    };

    drawSlot (0, aSlotX, slotY, slotW, slotH, spkShort[selSpkA], micShort[selMicA], offAxisA, spkColA, micColA, "A");
    drawSlot (1, bSlotX, slotY, slotW, slotH, spkShort[selSpkB], micShort[selMicB], offAxisB, spkColB, micColB, "B");

    // ── A/B Blend bar ─────────────────────────────────────────────────────────
    const int barY = slotY + slotH + 10;
    const int barW = slotW * 2 + slotGap;
    const int barH = 7;
    const float aFrac = 1.0f - blendNorm;
    const float bFrac = blendNorm;

    // Store for hit testing (extend hit area vertically)
    blendBarBounds = juce::Rectangle<float> ((float) aSlotX, (float) (barY - 8),
                                             (float) barW,   (float) (barH + 16));

    g.setColour (juce::Colour (0xff1a1c28));
    g.fillRoundedRectangle ((float) aSlotX, (float) barY, (float) barW, (float) barH, 3.5f);
    if (aFrac > 0.01f)
    {
        g.setColour (spkColA.withAlpha (0.75f));
        g.fillRoundedRectangle ((float) aSlotX, (float) barY, barW * aFrac, (float) barH, 3.5f);
    }
    if (bFrac > 0.01f)
    {
        g.setColour (spkColB.withAlpha (0.75f));
        g.fillRoundedRectangle ((float) (aSlotX + barW) - barW * bFrac, (float) barY,
                                barW * bFrac, (float) barH, 3.5f);
    }
    g.setColour (panelOutline);
    g.drawRoundedRectangle ((float) aSlotX, (float) barY, (float) barW, (float) barH, 3.5f, 0.8f);

    // Drag thumb on blend bar
    if (activeDrag == DragTarget::BlendBar)
    {
        const float thumbX = (float) aSlotX + barW * blendNorm;
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.fillEllipse (thumbX - 5.0f, (float) barY - 3.0f, 10.0f, (float) barH + 6.0f);
    }

    // A / B labels
    if (punkaholicTypeface)
        g.setFont (juce::Font (juce::FontOptions (punkaholicTypeface).withHeight (9.5f)));
    else
        g.setFont (9.5f);
    g.setColour (spkColA.withAlpha (0.8f));
    g.drawText ("A", aSlotX - 16, barY - 1, 14, barH + 2, juce::Justification::centredRight, true);
    g.setColour (spkColB.withAlpha (0.8f));
    g.drawText ("B", aSlotX + barW + 2, barY - 1, 14, barH + 2, juce::Justification::centredLeft, true);
}

// ─────────────────────────────────────────────────────────────────────────────
CabMicDiagram::DragTarget CabMicDiagram::hitTest (juce::Point<float> pos) const noexcept
{
    for (int i = 0; i < 2; ++i)
    {
        const auto& geo = slotGeo[i];
        if (geo.maxR <= 0) continue;
        const float dx = pos.x - geo.coneCx;
        const float dy = pos.y - geo.coneCy;
        if (std::sqrt (dx * dx + dy * dy) < geo.maxR + 10.0f)
            return (i == 0) ? DragTarget::MicA : DragTarget::MicB;
    }
    if (blendBarBounds.contains (pos))
        return DragTarget::BlendBar;
    return DragTarget::None;
}

void CabMicDiagram::mouseDown (const juce::MouseEvent& e)
{
    const auto pos = e.position;
    activeDrag = hitTest (pos);
    liveDragPos = pos;
    if (activeDrag != DragTarget::None)
        setMouseCursor (juce::MouseCursor::CrosshairCursor);
}

void CabMicDiagram::mouseDrag (const juce::MouseEvent& e)
{
    if (activeDrag == DragTarget::None) return;
    liveDragPos = e.position;

    if (activeDrag == DragTarget::MicA || activeDrag == DragTarget::MicB)
    {
        const int slot = (activeDrag == DragTarget::MicA) ? 0 : 1;
        applyMicPos (slot, e.position);
    }
    else if (activeDrag == DragTarget::BlendBar)
    {
        const float norm = juce::jlimit (0.0f, 1.0f,
            (e.position.x - blendBarBounds.getX()) / blendBarBounds.getWidth());
        setNormParam ("cabMicBlend", norm);
    }
    repaint();
}

void CabMicDiagram::mouseUp (const juce::MouseEvent& e)
{
    if (activeDrag == DragTarget::MicA || activeDrag == DragTarget::MicB)
    {
        // Commit final off-axis state
        const int slot = (activeDrag == DragTarget::MicA) ? 0 : 1;
        applyMicPos (slot, e.position);
    }
    activeDrag = DragTarget::None;
    setMouseCursor (juce::MouseCursor::NormalCursor);
    repaint();
}

void CabMicDiagram::mouseMove (const juce::MouseEvent& e)
{
    const auto tgt = hitTest (e.position);
    setMouseCursor (tgt != DragTarget::None ? juce::MouseCursor::CrosshairCursor
                                            : juce::MouseCursor::NormalCursor);
}

void CabMicDiagram::mouseExit (const juce::MouseEvent&)
{
    setMouseCursor (juce::MouseCursor::NormalCursor);
}

// ─────────────────────────────────────────────────────────────────────────────
void CabMicDiagram::applyMicPos (int slot, juce::Point<float> localPos)
{
    const auto& geo = slotGeo[slot];
    if (geo.maxR <= 0) return;

    const float dx   = localPos.x - geo.coneCx;
    const float dy   = localPos.y - geo.coneCy;
    const float dist = std::sqrt (dx * dx + dy * dy);
    const bool  off  = dist > geo.maxR * 0.28f;

    const juce::String paramId = (slot == 0) ? "cabMicAOffAxis" : "cabMicBOffAxis";
    setNormParam (paramId, off ? 1.0f : 0.0f);
}

void CabMicDiagram::setNormParam (const juce::String& id, float norm)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, norm));
}
