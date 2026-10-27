# Architecture Overview

## Project Structure

```
RM1xController/
├── Source/
│   ├── MIDIProtocol.h/cpp      # MIDI message builders & parsers
│   ├── PanelComponents.h/cpp   # UI components (knobs, keyboard, LEDs)
│   ├── PluginEditor.h/cpp      # Main UI layout & event handling
│   ├── PluginProcessor.h/cpp   # Audio processor & MIDI I/O
│   └── ProjectInfo.h.in        # Version info template
├── CMakeLists.txt              # Build configuration
├── README.md                   # User documentation
├── BUILD.md                    # Build instructions
├── TESTING.md                  # Testing guide
└── uploads/                    # Reference manuals (PDFs)
```

## Component Layers

### 1. MIDI Protocol Layer (`MIDIProtocol.h/cpp`)

**Purpose:** Encapsulates all Yamaha RM1x MIDI message construction.

**Key Functions:**
- `createIdentityRequest()` → `F0 7E 0n 06 01 F7`
- `createSectionChange(Section)` → `F0 43 7E 00 ss 7F F7`
- `createTransport(Command)` → FA/FB/FC realtime messages
- `createExpressionMute(channel, muted)` → CC#11 mute workaround
- `createSongSelect(songNumber)` → F3 song/style select
- `createCutoff/Resonance/Reverb/Chorus/Pan()` → Voice CC messages
- `createXGParameterChange()` → Generic XG SysEx builder (Model ID 4Ch)
- `isIdentityReply()` / `getIdentityInfo()` → Parse identity responses

**Design Decision:** Keep MIDI protocol logic separate from UI for testability and potential CLI tool reuse.

### 2. Panel Components (`PanelComponents.h/cpp`)

**Purpose:** Reusable UI widgets that mirror hardware elements.

**Classes:**

#### `RM1xLookAndFeel`
- Custom JUCE LookAndFeel_V4 subclass
- Dark industrial groovebox color scheme
- Custom rotary knob rendering with orange accent
- Button styling with highlight states

#### `MidiLED`
- Small LED indicator with label
- `flash()` method for activity visualization
- Timer-based auto-off (50ms pulse)

#### `KnobStrip`
- 8 knobs with labels
- Bank A/B label switching
- `onKnobChange(index, value)` callback
- Labels: "Beat Stretch", "Clock Shift", "Gate Time", "Velocity", "Cutoff", "Resonance", "EG Decay", "Pitch Bend" (Bank A)

#### `SectionKeyboard`
- 26-key virtual keyboard (E2–F4 piano layout)
- Three modes: TRACK, MUTE, SECTION
- Handles white key → section/track mapping
- Callbacks:
  - `onSectionPressed(Section)` → Section A–P
  - `onMuteToggle(track, muted)` → Expression mute
  - `onTrackSelected(track)` → Track selection

**Design Decision:** Component composition over monolithic UI class.

### 3. Plugin Editor (`PluginEditor.h/cpp`)

**Purpose:** Main UI assembly, layout, and event routing.

**Sections:**
1. **Header:** Title, version, MIDI LEDs, device selector, identity button
2. **Display:** Status LCD, BPM display, song/style selector
3. **Transport:** Play, Stop, Continue, Master Volume
4. **Knobs:** 8-knob strip + Bank A/B switch
5. **Keyboard Modes:** TRACK, MUTE, SECTION buttons
6. **Keyboard:** 26-key SectionKeyboard

**Event Flow:**
```
User Action → Component Callback → Handler Method → MIDI Message → Processor
```

Example:
```
Click Section Key A → keyboard->onSectionPressed(Section::A) 
                   → handleSectionChange(Section::A)
                   → MIDIProtocol::createSectionChange(Section::A)
                   → audioProcessor.sendMidiMessage(msg)
```

**Layout:** Custom `resized()` implementation with proportional sizing, supports 1000×600 to 2400×1400 window size.

### 4. Plugin Processor (`PluginProcessor.h/cpp`)

**Purpose:** JUCE AudioProcessor implementation for VST3/Standalone hosting.

**Responsibilities:**
- MIDI device enumeration (`getAvailableMidiOutputs()`)
- MIDI device opening/closing (`setMidiOutput()`)
- Message transmission (`sendMidiMessage()`)
- State save/restore (selected MIDI device name)
- Plugin boilerplate (processBlock, parameter stubs)

**MIDI Architecture:**
- **Standalone:** Opens `juce::MidiOutput` directly to system device
- **VST3:** Plugin can open direct MIDI out OR rely on host routing
- **Windows caveat:** Don't open same port from both host and plugin (exclusive access)

**Design Decision:** Shared pointer for `midiOutput` (JUCE 8 MidiOutput uses shared ownership).

## Data Flow

### Outgoing MIDI (User → RM1x)

```
UI Event → PluginEditor handler
        → MIDIProtocol::create*() 
        → juce::MidiMessage
        → PluginProcessor::sendMidiMessage()
        → midiOutput->sendMessageNow()
        → System MIDI driver
        → RM1x MIDI In
```

### Incoming MIDI (RM1x → App)

**Not implemented in v0.1** — plugin currently sends only, no parsing of RM1x responses. Future:

```
RM1x MIDI Out → System MIDI driver 
             → MIDI Input callback
             → MIDIProtocol::isIdentityReply() / parse*()
             → Update UI (e.g., flash MIDI In LED)
```

## Build System

### CMake Configuration

```cmake
project(RM1xController VERSION 0.1.0)
├─ FetchContent: JUCE 8.0.3 (if not present locally)
├─ juce_add_plugin: VST3 + Standalone formats
├─ target_sources: MIDIProtocol, PanelComponents, Editor, Processor
├─ configure_file: ProjectInfo.h.in → ProjectInfo.h (version string)
└─ target_link_libraries: juce_audio_utils, juce_audio_processors, juce_audio_devices
```

**Platform Differences:**
- **Linux:** Requires ALSA, X11, freetype, fontconfig
- **Windows:** Uses MSVC, links against Windows MIDI APIs
- **macOS:** Uses CoreMIDI, requires Xcode CLI tools

## Key Design Decisions

### 1. Expression Mute (CC#11) Instead of True Mute

**Problem:** Yamaha RM1x has no documented mute SysEx in official MIDI spec.

**Solution:** Use CC#11 (Expression) = 0 for mute, 127 for unmute.

**Trade-offs:**
- ✅ Works immediately, no reverse engineering
- ✅ Universal MIDI CC, no proprietary knowledge needed
- ❌ Not the same as hardware mute memory
- ❌ Affects expression if user is already using CC#11 for modulation

**Future:** If mute SysEx is discovered, replace implementation and keep UI label honest.

### 2. Section SysEx: FAQ Version (00-0F)

**Discrepancy:** 
- List Book PDF shows `ss=08–0F` (8 sections?)
- Yamaha FAQ shows `ss=00–0F` (16 sections A–P)

**Decision:** Implement FAQ version (`00-0F`) with code comment documenting the discrepancy. RM1x has 16 sections, so FAQ version is logically correct.

**Verification:** Hardware testing will confirm which is accurate.

### 3. Modular MIDI Protocol Class

**Why:** 
- Easier unit testing (can test message construction without UI)
- Potential reuse in command-line tools
- Clearer separation of concerns
- Simplifies future protocol additions (NRPNs, Play FX CCs)

**Pattern:** Static factory methods returning `juce::MidiMessage`.

### 4. No JuceHeader.h (JUCE 8 Best Practice)

JUCE 8 deprecated the monolithic `JuceHeader.h`. Instead:
- `#include <juce_audio_processors/juce_audio_processors.h>`
- `#include <juce_audio_devices/juce_audio_devices.h>`
- `#include <juce_gui_basics/juce_gui_basics.h>`

**Benefits:** Faster compile times, clearer dependencies.

### 5. FontOptions Instead of Deprecated Font Constructors

JUCE 8 deprecated `Font(float, int)` constructor.

**Old (JUCE 6/7):**
```cpp
setFont(juce::Font(28.0f, juce::Font::bold));
```

**New (JUCE 8):**
```cpp
setFont(juce::Font(juce::FontOptions(28.0f).withStyle("Bold")));
```

## Testing Strategy

### Unit Testable
- `MIDIProtocol::create*()` methods (pure functions)
- Message byte verification

### Integration Testable
- MIDI device enumeration
- Message transmission (with loopback or monitor)

### Manual Testing
- UI layout at various window sizes
- Keyboard mode switching
- Knob interaction
- Hardware control (see TESTING.md)

## Future Extensions

### Planned for Later Versions

1. **MIDI Input Handling:** Parse RM1x responses, update UI from hardware state
2. **Voice Editor:** Full XG parameter control via SysEx
3. **NRPNs:** Filter cutoff/resonance/EG via NRPNs (more precise than CCs)
4. **Play FX Remote:** Requires MIDI sniffing to discover CCs
5. **Pattern Editor:** Step sequencer UI for pattern creation
6. **Librarian:** Import/export SMF files, save to RM1x floppy
7. **Mute Memory:** If SysEx is reverse-engineered
8. **Bank/Program Change UI:** Visual voice browser
9. **MIDI Learn:** Map hardware knobs to plugin parameters

### Extension Points

- `MIDIProtocol` can grow with new `create*()` methods
- `PanelComponents` can add new widgets (faders, step grid)
- `PluginEditor` can switch views (e.g., tabbed pattern/voice/library)

## Performance Characteristics

- **UI Thread:** All MIDI sending happens on message thread (no audio thread)
- **MIDI Latency:** ~1-5ms typical (OS-dependent)
- **CPU Usage:** Negligible when idle, <1% CPU during knob adjustments
- **Memory:** ~30MB resident (JUCE framework overhead)

## Compliance

- **JUCE License:** GPL or commercial (user must comply with GPL or purchase JUCE license)
- **Yamaha MIDI Spec:** Based on official RM1x MIDI Data Format List Book
- **ASCII UI Text:** No Unicode/emoji, compatible with all fonts
