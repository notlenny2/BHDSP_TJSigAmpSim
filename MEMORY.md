# BackhouseDSP Memory

Last updated: 2026-02-22
Project path: `/Users/tj/MuscleDaddies/BackhouseDSP`

## Current State
A JUCE-based amp sim scaffold exists with:
- AU / VST3 / Standalone targets (`CMakeLists.txt`)
- 4 amp selector scaffold (Amp 1-4)
- Per-amp character model in DSP:
  - Amp-specific pre-EQ shape and HP filtering
  - Amp-specific saturation drive/asymmetry
  - Amp-specific sag dynamics
  - Amp-specific depth/fizz post contour
- Core controls: Input Gain, Input Boost, Sub, Low, Mid, High, Presence, Output
- Noise gate, tuner stub
- Cab/IR block:
  - Per-amp default cab voicing filters (Amp 1-4 each has a different contour)
  - User IR loading via UI (`Load IR` / `Clear IR`)
  - IR filename shown in status label
  - IR path persisted in plugin state when available
- Guitar Profile system with push-button selection:
  - `Neutral`
  - Profile 1
  - Profile 2
  - Profile 3
- Editable profile names in UI
- Per-profile editable offsets:
  - Pickup Output
  - Brightness
  - Low End
  - Gate Trim
- Profile portability:
  - Export profiles to JSON
  - Import profiles from JSON
- Live-safe smoothing when switching profiles
- Standalone test bench panel for ear-tuning workflow:
  - Input and output level meters (visual gain staging)
  - Audio device selector (input/output routing)
  - MIDI input/output selectors

## Key Files
- Product requirements: `PRODUCT_SPEC.md`
- Build/setup notes: `README.md`
- Audio processor: `src/PluginProcessor.h`, `src/PluginProcessor.cpp`
- Editor/UI: `src/PluginEditor.h`, `src/PluginEditor.cpp`
- DSP blocks: `dsp/`

## Build Command
```bash
cd /Users/tj/MuscleDaddies/BackhouseDSP
cmake -S . -B build -DJUCE_DIR=/absolute/path/to/JUCE
cmake --build build --config Release
```

## Next Recommended Work
1. Tone refinement
- Improve amp nonlinear stages by amp model
- Add per-amp depth/voicing refinements

2. FX expansion
- Delay (with pitch control), reverb types, chorus/phaser/wah

3. Preset flow
- Add explicit base-preset vs guitar-profile offset UX

## Resume Prompt
When resuming, use:
"Continue BackhouseDSP from MEMORY.md and start with [1/2/3]."
