# RM1x Controller

**Controller for Yamaha RM1x Groovebox** — mirrors the hardware panel over MIDI.

A JUCE 8 VST3 + Standalone application that provides a software remote controller for the Yamaha RM1x groovebox. Designed to replace broken physical buttons (Section, Mute, Track) while the unit’s sound engine and MIDI functionality remain intact.

## Version

**0.1.0** — Initial public release

## Features

- **MIDI device management** — select MIDI In/Out with activity LEDs
- **Identity Request** — query a connected RM1x for device identification
- **Transport** — PLAY / STOP / CONTINUE (MIDI realtime FA / FC / FB)
- **Section control (A–P)** — Section SysEx `F0 43 7E 00 ss 7F F7` (`ss` = `00`–`0F`)
- **Track selection** — select active track (1–16) for voice parameter editing
- **Expression Mute** — per-track mute via **CC#11 Expression** (`0` muted / `127` unmuted) — see caveat below
- **Song / Style select** — MIDI Song Select (`F3`) for patterns 0–127
- **MIDI Clock tempo** — send MIDI Clock (`F8`) to drive the RM1x; hardware must be set to **Sync = MIDI**
- **MIDI In** — listen for identity replies, section/mute feedback, and pattern sync
- **Panel-mirror UI** — dark industrial aesthetic with purple-blue active states inspired by the hardware

### Keyboard modes

The 26-key virtual keyboard (E2–F4) operates in three modes:

1. **TRACK mode** (default) — white keys 1–16 select the current track for voice editing  
2. **SECTION mode** — white keys trigger Section A–P via SysEx  
3. **MUTE mode** — white keys 1–16 toggle Expression Mute per track  

## Important: Expression Mute caveat

This controller implements **“Expression Mute”** using **CC#11 (Expression)**, **not** true hardware mute.

The Yamaha RM1x has no documented MIDI SysEx for mute control or mute memory. This implementation:

- Sends CC#11 = 0 (muted) or CC#11 = 127 (unmuted) per track channel  
- Works as a functional mute when the track is not using Expression for other purposes  
- Does **not** access the RM1x’s internal mute memory or mute states  
- UI labels clearly indicate **“Mute (Expr)”** to distinguish from hardware mute  

If true mute SysEx is discovered in the future, the implementation can be updated.

## Hardware setup (required)

Configure your RM1x Utility settings before using this controller:

1. **MIDI Control** → `IN` or `IN/OUT` (`UTILITY → MIDI → MIDI Control`)  
2. **MIDI Sync** → `MIDI` (required for external MIDI Clock tempo) or `INTERNAL` as needed  
3. **Echo Back** → typically `OFF` for direct control  
4. **MIDI Filter → SysEx** → `ENABLED` (Section control requires SysEx reception)  

Typical configuration:

```
UTILITY → MIDI Settings:
  MIDI Control:   IN/OUT
  MIDI Sync:      MIDI
  Echo Back:      OFF
  Filter (SysEx): ENABLED
```

## MIDI protocol (highlights)

### Section Control SysEx (FAQ-confirmed)

```
F0 43 7E 00 ss 7F F7
```

Where `ss` = `00`–`0F` for sections A–P.

> Note: The official List Book shows `ss=08–0F` (likely a typo). This project uses the FAQ-confirmed `00–0F` range.

### Transport

- Start `FA` · Continue `FB` · Stop `FC`

### Song Select

- `F3 nn` — pattern/song number 0–127

### Control Changes used

- CC#7 Volume · CC#10 Pan · CC#11 Expression (mute workaround)  
- CC#71 Resonance · CC#74 Cutoff · CC#91 Reverb Send · CC#93 Chorus Send  

## Build

### Prerequisites

- CMake 3.22+  
- C++17 compiler (MSVC, GCC, or Clang)  
- JUCE 8 (auto-fetched via CMake `FetchContent` if not present)

### Windows (VS 2022 + Ninja)

```cmd
rebuild.bat
```

Or manually:

```cmd
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64
mkdir build & cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
```

Install the VST3 (PowerShell):

```powershell
powershell -ExecutionPolicy Bypass -File install-vst3.ps1
```

This installs to:

- `%USERPROFILE%\Documents\rm1x-controller` (project folder)  
- `%USERPROFILE%\Documents\VST3` (Studio One / user scan path)  
- `%CommonProgramFiles%\VST3` (system-wide; needs admin)

### Linux

```bash
cmake -B build
cmake --build build --config Release
```

### macOS

```bash
cmake -B build -G Xcode
cmake --build build --config Release
```

Built plugins/standalone land under `build/RM1xController_artefacts/`.

## Usage

1. Launch the plugin or standalone app  
2. Select your RM1x MIDI **Out** (and optionally **In**)  
3. Click **ID** to verify connection  
4. Use Section / Track / Mute rows and transport  
5. Set tempo with the Tempo knob — ensure hardware **Sync = MIDI**  

## Architecture

- **JUCE 8** · VST3 + Standalone · CMake  
- Modular MIDI protocol layer (`MIDIProtocol.cpp`)  
- Panel component classes (`PanelComponents.cpp`)  
- Resizable UI (min ~1000×700, scales to 2400×1600)

See [ARCHITECTURE.md](ARCHITECTURE.md), [BUILD.md](BUILD.md), [TESTING.md](TESTING.md), and [MIDI-INPUT-NOTES.md](MIDI-INPUT-NOTES.md).

## Out of scope for v0.1

- Full LCD bitmap clone · Job menu · Event list editor  
- Floppy/SMF librarian · True mute-memory SysEx · Deep Play FX remote · NRPN editors  

## License

MIT License — Copyright (c) 2026 Haris / H.K. Orasi Multimedia Ltd  
See [LICENSE](LICENSE).

Yamaha, RM1x, and related marks are trademarks of Yamaha Corporation. This project is an unofficial third-party controller and is not affiliated with or endorsed by Yamaha.

## Troubleshooting

| Symptom | Check |
|--------|--------|
| Transport doesn’t respond | `UTILITY → MIDI → MIDI Control` = `IN` or `IN/OUT` |
| Sections don’t change | SysEx filter enabled; hardware in Pattern or Song mode |
| Mute not working | Expression Mute uses CC#11; track voice shouldn’t rely on Expression |
| Tempo ignored | Hardware **Sync = MIDI** |
| Windows MIDI conflicts | Don’t open the same physical port as both host Out and plugin direct Out |

## Contributing

Contributions welcome — especially reverse-engineered mute SysEx (if it exists), Play FX CC mappings, voice parameter maps, and UI/UX improvements.
