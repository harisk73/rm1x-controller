# Testing Guide

## Quick Functionality Test (No Hardware Required)

### Standalone Application

1. Launch the standalone app from `build/RM1xController_artefacts/Standalone/`
2. Verify the UI appears with:
   - "RM1x Controller" title and version number
   - MIDI In/Out LEDs
   - MIDI Output dropdown (shows "(None)" if no devices)
   - Status LCD showing "Track 1 | No MIDI output selected"
   - BPM display showing "120"
   - Transport buttons (PLAY, STOP, CONTINUE)
   - 8 knobs in Knob Bank A
   - Keyboard mode buttons (TRACK, MUTE, SECTION)
   - 26-key virtual keyboard

3. **Click keyboard mode buttons** - Status should update to show active mode
4. **Click virtual keys** - Keys should light up in TRACK/MUTE modes
5. **Drag knobs** - They should rotate smoothly
6. **Resize window** - UI should scale properly (minimum 1000x600)

### VST3 Plugin

1. Load the plugin in your DAW (Reaper, Ableton, etc.)
2. Open the plugin UI
3. Verify same UI elements as standalone
4. Check MIDI output dropdown lists available devices

## With RM1x Hardware

### Prerequisites

Ensure your RM1x is configured correctly:

```
RM1x → UTILITY → MIDI:
  MIDI Control:   IN/OUT
  MIDI Sync:      MIDI (or INTERNAL)
  Echo Back:      OFF
  Filter (SysEx): ENABLED (allow SysEx)
```

Connect RM1x MIDI Out → Computer MIDI In  
Connect Computer MIDI Out → RM1x MIDI In

### Test Sequence

#### 1. MIDI Connection

1. Launch RM1x Controller
2. Select your RM1x MIDI output device from dropdown
3. Click **Identity Request** button
4. **Expected:** 
   - MIDI OUT LED flashes
   - Status shows "Identity request sent"
   - If RM1x responds, you should see identity info (requires MIDI In to computer)

#### 2. Transport Control

1. Click **PLAY** button
2. **Expected:** RM1x starts playing current pattern/song
3. Click **STOP**
4. **Expected:** RM1x stops
5. Click **CONTINUE**
6. **Expected:** RM1x resumes from stopped position

**Troubleshooting:** If transport doesn't respond, check `MIDI Control` is set to `IN` or `IN/OUT` in RM1x Utility.

#### 3. Section Control

1. Click **SECTION** mode button
2. Status should show: "Mode: SECTION - Keys A-P control sections"
3. Ensure RM1x is in Pattern mode (not Utility/Job)
4. Click virtual keyboard white keys (first 16 keys = sections A-P)
5. **Expected:** RM1x switches sections on each key press

**Troubleshooting:** If sections don't switch:
- Verify SysEx filter is enabled in RM1x Utility → MIDI → Filter
- Ensure RM1x is in Pattern or Song mode (not Utility screen)

#### 4. Expression Mute

1. Click **MUTE** mode button
2. Status shows: "Mode: MUTE (Expr) - Keys 1-16 toggle Expression Mute CC#11"
3. Start RM1x playback
4. Click virtual keyboard white keys (keys 1-16 = tracks 1-16)
5. **Expected:** Corresponding track volume goes silent (muted) or returns (unmuted)
6. Key stays lit when muted

**Note:** This is Expression mute (CC#11), not hardware mute memory. See README for details.

#### 5. Track Selection & Voice Control

1. Click **TRACK** mode button
2. Click key #5 (5th white key from left)
3. Status shows: "Track 5 selected"
4. Adjust **Cutoff** knob (5th knob in Bank A)
5. **Expected:** Track 5 cutoff frequency changes in realtime

Try other voice controls:
- **Knob Bank A:**
  - Knob 5: Cutoff (CC#74)
  - Knob 6: Resonance (CC#71)
- **Knob Bank B** (click "Knob Bank" button to switch):
  - Knob 6: Chorus Send (CC#93)
  - Knob 7: Reverb Send (CC#91)
  - Knob 8: Track Volume (CC#7)

#### 6. Song/Style Select

1. Drag the **Song/Style** slider to value 2
2. **Expected:** RM1x switches to Song #2 (Song mode) or Style #2 (Pattern mode)

### Automated MIDI Monitor Test

If you have a MIDI monitor tool (e.g., MIDI-OX on Windows, MIDI Monitor on Mac):

1. Route RM1x Controller output to MIDI monitor input
2. Click **PLAY** → Should see realtime message `FA`
3. Click **STOP** → Should see `FC`
4. Click **SECTION** mode, then key A → Should see SysEx: `F0 43 7E 00 00 7F F7`
5. Adjust Cutoff knob → Should see CC #74 messages

## Known Limitations (v0.1)

- No visual feedback when RM1x sends MIDI back to computer
- No Play FX parameter mapping yet (requires MIDI sniffing)
- No true mute memory SysEx (undocumented by Yamaha)
- No pattern/song editor
- No voice bank/program change UI yet

## Bug Reports

If a feature doesn't work as expected:

1. Check RM1x Utility MIDI settings (see Prerequisites)
2. Verify MIDI cables are connected correctly
3. Test with a MIDI monitor to confirm messages are sent
4. Check RM1x is on correct mode (Pattern/Song, not Utility)
5. Open an issue with:
   - Your OS version
   - RM1x firmware version (shown on boot)
   - MIDI interface used
   - Exact steps to reproduce
   - Expected vs actual behavior

## Success Criteria for v0.1

✅ Application builds on Linux/Windows/macOS  
✅ UI loads and is resizable  
✅ MIDI device selection works  
✅ Transport commands control RM1x  
✅ Section A-P switching works  
✅ Expression Mute (CC#11) silences tracks  
✅ Track selection + voice knobs send correct CCs  
✅ Song/Style select works  
✅ UI clearly labels "Mute (Expr)" to distinguish from hardware mute
