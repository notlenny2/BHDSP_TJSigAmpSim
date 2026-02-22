# BackhouseDSP Guitar Amp Sim - Product Spec (Living Document)

Last updated: 2026-02-22
Owner: BackhouseDSP
Status: Draft v1 from user requirements

## 1. Master Goal
Build a stable, easy-to-use guitar amp simulator that works for:
- Live performance on Apple Silicon laptops with low enough latency for real-time play.
- Studio plugin usage.
- Portability across different computers and rigs.

Primary differentiator (killer feature):
- Guitar-aware preset morphing: user defines their guitars, then switches guitars to automatically apply tone adjustments so each guitar sounds optimized.

## 2. Product Form
Initial plugin formats (phase order):
1. VST3 (macOS Apple Silicon)
2. AU (macOS Apple Silicon)
3. Standalone app (optional early, recommended for live use)
4. Windows VST3/AAX later

Host + live objectives:
- Fast preset switching with no audio dropouts.
- Reliable state recall.
- CPU-efficient real-time DSP chain.

## 3. Tone Targets (Amp Personalities)
The plugin will expose 4 target amps, named `Amp 1`, `Amp 2`, `Amp 3`, `Amp 4`.

### Amp 1 - Clean Compressed Sparkle + Delay
Target:
- Metalcore clean-section style.
- Crystalline clean, compressed, sparkly top end.
- Signature option: pitched delay character.

Core voicing:
- Fast attack compression.
- Controlled low-end.
- Chime/sparkle emphasis around upper mids/highs.

### Amp 2 - Dirty Blues (Growly Mid Focus)
Target:
- ZZ Top inspired dirty blues.
- Mid-heavy growl.
- Highly responsive to gain and picking.

Core voicing:
- Low/medium gain breakup.
- Strong midrange contour.
- Dynamic response over excessive compression.

### Amp 3 - Classic Thrash (JCM800-Inspired + Depth)
Target:
- Articulate classic thrash voice (JCM800 family inspiration).
- Added `Depth`/low-end extension not typical in original amp.
- "Master of Puppets"-style authority with more bass.

Core voicing:
- Tight low-end pre-distortion.
- Aggressive upper-mid articulation.
- Switchable low-frequency resonance/depth stage.

### Amp 4 - Modern Metalcore (Wage War Direction)
Target:
- Scooped, heavy, articulate, punchy.
- Optional Bilmuri-inspired clean/country-like blend layer.
- Experimental mode: amp blend/stack concepts.

Core voicing:
- Tight and fast transient response.
- Controllable scoop without losing note definition.
- Parallel blend path option.

## 4. Amp Controls
Global/standard controls (as specified):
- Input Gain
- Low
- Mid
- High
- Presence
- Output

Per-amp conditional controls:
- Input Boost switch on Amp 2 and Amp 3.
- Sub control (very low bass) on Amp 3 and Amp 4.

Extended amp-mod controls:
- Simplified tube/bias system inspired by Bias-style UX.
- User-friendly (limited complexity):
  - Tube flavor selector (coarse choices)
  - Bias amount (single macro control)

## 5. Cabinet / IR Requirements
- Each amp has a dedicated cab sim baseline.
- Support both:
  - Built-in IR/cab models.
  - User IR loading.

Design objective:
- Cab section must improve on "generic cab sim" feel.
- Include options to quickly reshape harshness, resonance, and mic-distance-like behavior.

## 6. Effects Requirements
Required effects/features list:
- Compression:
  - 1176 style
  - LA-2A style
  - Distressor style
  - Optional digital/graphical mode
- Output EQ:
  - Curved parametric workflow similar to Pro Tools EQ3 / FabFilter-like usability
- Delay:
  - Inspired by Avid H-Delay behavior
  - Pitch control included
- Reverb:
  - Spring
  - Plate
  - Room (sampled from user's own space eventually)
- Chorus:
  - Subtle modulation; avoid extreme depth by default
- Wah:
  - Crybaby style
- Phaser:
  - Van Halen-style phaser voice
- Must-have utilities:
  - Noise gate
  - Tuner

## 7. Signature Feature: Guitar Profiles
### User story
"I own multiple guitars. I want one-click guitar switching that retunes the amp behavior automatically so each guitar sounds best."

### Functional requirements
- User can create named guitar profiles.
- Each profile stores metadata and tone offsets:
  - Pickup type/output level
  - Tuning
  - String gauge
  - Brightness/low-end compensation
  - Gain trim / EQ offsets / gate threshold / cab tweak references
- Switching guitars applies profile-linked parameter offsets instantly.
- Guitar profile changes should be safe live (no pops/clicks; smoothing/ramping where needed).

### Future extension
- Optional auto-match wizard (play reference riff -> suggested offsets).

## 8. Performance and Live Constraints
Platform focus:
- Apple Silicon MacBook live usage.

Latency target:
- Low-latency operation suitable for live play.
- Oversampling modes should have live-safe and studio-quality modes.

Stability targets:
- No crashes during rapid preset/guitar switching.
- Session recall consistency.
- CPU budget appropriate for live sets (final budget to be profiled).

## 9. UX / UI
- UI deferred for now.
- Functionality first, then design pass.

## 10. Proposed Initial Architecture
Recommended signal flow (v1 baseline):
1. Input
2. Noise Gate
3. Optional pre-compression
4. Amp block (select Amp 1-4)
5. Cab/IR block
6. Post FX rack (delay/reverb/chorus/phaser/wah where applicable)
7. Output EQ
8. Tuner monitor path
9. Output

Control plane:
- Preset Manager
- Guitar Profile Manager (offset layer on top of base preset)
- Live-safe parameter smoothing and state transitions

## 11. Phased Delivery Plan
### Phase 1 - Core Amp MVP (build first)
- Plugin scaffold (JUCE)
- Amp 1-4 foundational voicings
- Controls: Input Gain, Low/Mid/High, Presence, Output
- Conditional controls: Input Boost (2/3), Sub (3/4)
- Noise Gate + basic tuner
- Built-in cab placeholders

### Phase 2 - Guitar Profile System (killer feature)
- Profile CRUD (create, rename, delete, select)
- Offset mapping and safe runtime switching
- Preset + guitar profile interaction logic

### Phase 3 - Cab/IR Expansion
- User IR loader
- Per-amp cab defaults
- Extra cab sculpt controls

### Phase 4 - Time/Mod FX + Output EQ
- Delay + pitch
- Reverbs (spring, plate, room)
- Chorus, phaser, wah
- Parametric output EQ

### Phase 5 - Advanced Compression Models
- 1176, LA-2A, Distressor-inspired modes
- Digital graphical compressor mode

### Phase 6 - Live Readiness + Packaging
- Performance profiling and optimization
- Preset/guitar switch stress testing
- AU/VST3 release packaging

## 12. Open Decisions To Finalize
1. Minimum viable host support for v1 launch (plugin only vs plugin + standalone).
2. Initial sample rates / oversampling strategy and live/studio quality modes.
3. Exact parameter set per amp for first prototype.
4. First-party IR capture plan and naming conventions.
5. Preset file format and migration strategy.

## 13. Non-Goals For MVP
- Perfect hardware-model authenticity in v1.
- Full tube component-level simulation from day one.
- Advanced graphical UI polish before DSP backbone is stable.

## 14. Change Log
- v1 (2026-02-22): Initial structured spec from user requirement brief.
