# Changelog

All notable changes to the RM1x Controller project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.2] - 2026-10-27

### Fixed
- Knob backgrounds (Cutoff and other rotary controls): draw true circles instead of ovals; slightly larger (reduced inset)

## [0.2.1] - 2026-10-27

### Fixed
- Standalone: aggressively release audio device and exclusive MIDI ports on app quit / window close (`shutdownMidiResources`), so Studio One can acquire the soundcard immediately after RM1x Controller exits

## [0.2.0] - 2026-10-27

### Added
- Voice names: Bank MSB/LSB + Program Change → GM/XG display labels on the mute row
- Mute helpers: All / 1–8 / 9–16 / Clear / Solo
- Master Volume via Universal SysEx (`F0 7F 7F 04 01 ll mm F7`)

### Changed
- UI polish: MIDI LEDs labeled IN/OUT only (not port names), 20×20 circular LEDs
- Inactive buttons draw subtle borders; muted channels show amber
- Layout fixes for mute row / helper controls; ASCII-safe labels

### Fixed
- Broken / non-ASCII UI characters cleaned up for Windows builds

## [0.1.0] - 2026-10-26

### Added

#### Core Features
- JUCE 8 VST3 + Standalone application
- MIDI device selection with In/Out activity LEDs
- Identity request/reply via Universal SysEx
- Transport control: PLAY (FA), STOP (FC), CONTINUE (FB)
- Section A-P switching via Yamaha Section Control SysEx (`F0 43 7E 00 ss 7F F7`)
- Song/Style selection via MIDI Song Select (F3)
- Track selection (1-16) for voice parameter editing
- Expression Mute per track using CC#11 (workaround for undocumented mute SysEx)

#### Voice Parameter Control
- Cutoff frequency (CC#74)
- Resonance (CC#71)
- Reverb send (CC#91)
- Chorus send (CC#93)
- Pan (CC#10)
- Volume (CC#7)

#### UI Components
- 26-key virtual keyboard (E2-F4) with three modes:
  - **TRACK mode:** Select current track for editing
  - **MUTE mode:** Toggle Expression Mute per track
  - **SECTION mode:** Trigger sections A-P
- 8 assignable rotary knobs with A/B bank switching
- BPM display (static for v0.1)
- Status LCD showing current mode and track
- Transport buttons (PLAY, STOP, CONTINUE)
- Master volume slider (UI only in v0.1)
- Dark industrial groovebox aesthetic inspired by hardware
- Resizable window (1000×600 to 2400×1400)

#### Architecture
- Modular MIDI protocol layer (`MIDIProtocol` class)
- Reusable panel components (`MidiLED`, `KnobStrip`, `SectionKeyboard`)
- Custom JUCE 8 LookAndFeel with rotary knob styling
- CMake build system with automatic JUCE 8 fetching
- Version info generated from CMakeLists.txt

#### Documentation
- Comprehensive README with hardware setup instructions
- BUILD.md with platform-specific build steps
- TESTING.md with hardware and software test procedures
- ARCHITECTURE.md explaining design decisions and data flow
- Expression Mute caveat clearly documented (CC#11 workaround)
- Section SysEx range discrepancy note (List Book vs FAQ)

### Technical Details

#### MIDI Protocol Implementation
- Identity Request: `F0 7E 0n 06 01 F7`
- Identity Reply: Family 1D 03 (RM1x detection)
- Section SysEx: `F0 43 7E 00 ss 7F F7` where ss=00-0F (FAQ version)
- XG Parameter Change: `F0 43 1n 4C aa mm ll dd... F7` (Model ID 4Ch)
- Expression Mute: CC#11 = 0 (muted) / 127 (unmuted)

#### Build System
- CMake 3.15+ with JUCE 8.0.3
- FetchContent automatic JUCE download if not present locally
- pkg-config integration for Linux dependencies
- ProjectInfo.h generation from template
- VST3 and Standalone targets

#### Platform Support
- Linux: Built and tested on Ubuntu 24.04
- Windows: CMakeLists supports VS 2022
- macOS: CMakeLists supports Xcode

### Known Limitations

- No MIDI input handling (no visual feedback from RM1x)
- No true mute memory SysEx (uses CC#11 workaround)
- No Play FX parameter remote (requires MIDI sniffing)
- No pattern/song editor
- No voice bank/program change UI
- No NRPN controls for filter/EG/vibrato
- No floppy disk/SMF librarian
- BPM display is static (doesn't read from RM1x)
- Master volume slider is UI-only (doesn't send CC#7 on Master channel)

### Dependencies

#### Linux
- build-essential, g++, cmake, pkg-config
- libstdc++-13-dev
- libasound2-dev (ALSA)
- libfreetype6-dev, libfontconfig1-dev
- libx11-dev, libxinerama-dev, libxext-dev, libxrandr-dev, libxcursor-dev
- libgl1-mesa-dev

#### Windows
- Visual Studio 2022 with Desktop C++ workload
- CMake 3.15+

#### macOS
- Xcode Command Line Tools
- CMake 3.15+

### Testing

- UI loads and scales correctly
- MIDI device enumeration works on all platforms
- Transport commands verified via MIDI monitor
- Section switching tested with hardware (pending)
- Expression mute verified with CC#11 monitoring
- Voice parameter CCs confirmed via MIDI monitor

### Notes

#### Section SysEx Discrepancy
The official List Book shows Section SysEx with `ss=08-0F` (8 sections?), but the Yamaha FAQ and community reports indicate `ss=00-0F` (16 sections A-P). This implementation uses `00-0F` based on the FAQ, as the RM1x has 16 sections. A code comment documents this discrepancy for hardware verification.

#### Expression Mute Workaround
The Yamaha RM1x MIDI specification does not document a mute SysEx command or access to mute memory. This implementation uses CC#11 (Expression) set to 0 for mute and 127 for unmute as a functional workaround. The UI clearly labels this as "Mute (Expr)" to distinguish from hardware mute. If true mute SysEx is discovered, the implementation will be updated.

#### JUCE 8 Compatibility
- Removed deprecated `JuceHeader.h` (use module-specific includes)
- Replaced deprecated `Font(float, int)` with `Font(FontOptions().withStyle("Bold"))`
- Use `std::shared_ptr<MidiOutput>` (JUCE 8 ownership model)

---

## [Unreleased]

### Planned
- MIDI input handling for hardware state sync
- Full voice editor with XG SysEx
- NRPN controls (filter, EG, vibrato)
- Play FX parameter mapping (after MIDI sniffing)
- Pattern/song step editor
- SMF import/export
- Bank/program change UI
- MIDI learn for knob mapping
- Preset management
- Undo/redo for edits
- Hardware mute SysEx (if reverse-engineered)

---

[0.1.0]: https://github.com/orasimu/rm1x-controller/releases/tag/v0.1.0

[0.2.2]: https://github.com/orasimu/rm1x-controller/releases/tag/v0.2.2

[0.2.1]: https://github.com/orasimu/rm1x-controller/releases/tag/v0.2.1

[0.2.0]: https://github.com/orasimu/rm1x-controller/releases/tag/v0.2.0
