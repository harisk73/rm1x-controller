# MIDI Input Implementation Notes

## v0.1.1 - Hardware State Reflection

### Implemented MIDI Input Parsing

The controller now reflects hardware state by parsing incoming MIDI messages:

#### 1. Section Control SysEx
**Message:** `F0 43 7E 00 ss dd F7`
- **Parsed:** Yes ✅
- **Behavior:** Updates UI to show active section (A-P) when hardware changes sections
- **Status Display:** Shows "Hardware section: X" when in SECTION mode
- **ss Range:** 0x00-0x0F (sections A-P)
- **dd Value:** 0x7F (trigger)

#### 2. Expression Mute (CC#11)
**Message:** CC#11 on channels 1-16
- **Parsed:** Yes ✅
- **Behavior:** Updates mute key toggle states in MUTE mode
- **Logic:** 
  - CC#11 = 0 → Track is muted (key lit)
  - CC#11 = 127 (or nonzero) → Track is unmuted (key off)
- **Limitation:** This reflects Expression mute only, not true hardware mute memory

#### 3. Track Selection
**Status:** Not implemented ❌
- **Reason:** Yamaha RM1x MIDI Data Format List Book does not document any SysEx or CC message for track selection
- **Current Behavior:** Track selection is local to the controller UI only
- **Future:** If reverse-engineered or documented, can be added

### MIDI Input Device Selection

- **Exclusive Access:** Uses `MidiInput::openDevice()` with exclusive device access (Windows limitation)
- **UI:** MIDI Input combo box added below MIDI Output
- **LED Indicator:** MIDI In LED flashes on any incoming MIDI activity
- **State Persistence:** MIDI Input device name saved/restored in plugin state

### Architecture

**PluginProcessor Changes:**
- Added `std::shared_ptr<MidiInput>` for input device
- Added `MidiInputCallbackWrapper` class implementing `MidiInputCallback`
- Added parsing methods: `parseSectionSysEx()`, `parseExpressionCC()`
- Added callbacks: `onMidiInputActivity`, `onSectionChanged`, `onMuteChanged`, `onTrackChanged`

**PluginEditor Changes:**
- Added MIDI Input combo box with device selection
- Wired callbacks to update UI from hardware state
- MIDI In LED flashes on incoming messages
- Status display shows hardware-initiated changes

**SectionKeyboard Changes:**
- Added `setMuteStateFromHardware()` method to update key toggle states from parsed CC#11

### Messages We Cannot Observe (Yet)

Based on Yamaha RM1x MIDI Data Format List Book:

1. **Track Selection** - No documented message
2. **Mute Memory States** - No documented SysEx for mute memory access
3. **Play FX Parameters** - Require MIDI sniffing to discover CC mappings
4. **Pattern/Song Selection Feedback** - Song Select (F3) is transmit-only from controller
5. **Tempo Changes** - No documented BPM feedback message

### Testing with Hardware

**Verified Working:**
- Section switching on hardware updates UI section display ✅
- Expression mute on hardware updates UI mute keys ✅

**Windows MIDI Note:**
- Cannot open same physical USB-MIDI device as both Input and Output from different applications
- Use controller's In+Out combos, OR DAW host routing, not both simultaneously

### Future Improvements

If additional messages are reverse-engineered or documented:
- Track selection SysEx (if exists)
- True mute memory access (if exists)
- Play FX parameter feedback
- Tempo/BPM feedback
- Pattern chain state
- Voice bank/program change feedback
