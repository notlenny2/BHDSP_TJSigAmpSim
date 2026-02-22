# BackhouseDSP Amp Sim (Phase 1 Scaffold)

This directory contains a JUCE plugin scaffold for the BackhouseDSP guitar amp sim project.

## Implemented in this scaffold
- JUCE plugin targets: `AU`, `VST3`, `Standalone`
- Amp selector: `Amp 1` to `Amp 4`
- Per-amp modeling character layer:
  - Distinct pre-shape EQ per amp
  - Asymmetric 2-stage saturation per amp
  - Sag/response behavior per amp
  - Post depth/fizz contour per amp
- Core controls:
  - Input Gain
  - Input Boost (functional on Amp 2/3)
  - Sub (functional on Amp 3/4)
  - Low / Mid / High / Presence / Output
- Noise gate (basic implementation)
- Per-amp default cab voicings (different filter contours for Amp 1-4)
- User IR loading (`.wav` / `.aiff`) with Load/Clear buttons
- Tuner stub (zero-crossing estimate)
- Guitar profile quick switch (push-button style): `Neutral` + 3 user profiles
- User-editable profile names (type and press Enter)
- Per-profile editable offsets (applies to selected profile):
  - Pickup Output
  - Brightness
  - Low End
  - Gate Trim
- Guitar profile portability:
  - `Export Profiles` to JSON
  - `Import Profiles` from JSON
- Live-safe profile offset smoothing on profile switch
- Standalone test bench tools:
  - Input/output meters for fast level matching and clipping checks
  - Audio device selector for choosing interface input/output
  - MIDI input/output selector controls
- Generic test UI with parameter attachments

## Project structure
- `CMakeLists.txt`: top-level build file
- `src/`: plugin processor + editor
- `dsp/`: Phase 1 DSP blocks
- `PRODUCT_SPEC.md`: living requirements/spec document

## Build prerequisites
- CMake 3.22+
- Xcode (for AU/VST3 on macOS)
- JUCE checkout available locally

## Configure and build
```bash
cd BackhouseDSP
cmake -S . -B build -DJUCE_DIR=/absolute/path/to/JUCE
cmake --build build --config Release
```

## Current status and next step
This is a foundation scaffold, not final amp modeling quality.

Next major feature from spec:
- Time/mod FX rack (delay, reverb, chorus, phaser, wah)
