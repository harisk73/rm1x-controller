# RM1x MIDI Implementation Notes

## Documented & Implemented Features

### Pattern/Style Selection & Dump Mechanism
- **Song Select** (F3 ss): In Pattern mode, changes the active style/pattern number (0-127)
  - Implemented as Pattern/Style ComboBox dropdown
  - Populated with 60 RM1x preset styles (documented names from List Book)
  - Patterns 0-59: Preset styles (Psychedelic Trance 1, Trance, Big Beat, etc.)
  - Patterns 60-127: User patterns (shown as "User Pattern")
  - Selecting a pattern sends Song Select (F3) to RM1x
  - Status updates show "Loading pattern X: [Name]" when changed
  
#### Pattern Load Broadcast (Hardware Verified)
- **Hardware behavior**: Song Select (F3 nn) triggers massive setup broadcast from RM1x
- **Broadcast contents** (~360-370 messages in ~0.5 seconds):
  - Echo of F3 nn
  - XG effect parameters: `F0 43 10 4C 02 01 ...F7` (reverb, chorus, variation settings)
  - Per-channel Bank MSB/LSB + Program Change for all 16 parts
  - Per-channel CCs: 07 (volume), 0A (pan), 5D/5B/5E (sends), 49/4B/48 (timbre), 41/05/4C/4A/47 (voice params), and others
  - Pitch Bend for all channels
  - XG Multi Part parameters: `F0 43 10 4C 08 pp aa vv F7` (part pp, address aa, value vv)
- **This IS the working dump mechanism** - no separate dump request needed
- Incoming F3 message updates Pattern dropdown to stay in sync

### Section Control
- **SysEx**: `F0 43 7E 00 ss dd F7`
  - ss = 00-0F for sections A-P
  - dd = 7F for "on" (note: List Book shows ss=08-0F for sections A-P, but testing confirms ss=00-0F)
  - Bidirectional (TX when UI button pressed, RX updates UI from hardware)

### Expression Mute (per-part mute)
- **CC#11** (Expression): 0=muted, 127=unmuted
  - Sent per MIDI channel (1-16)
  - Bidirectional (TX from Mute buttons, RX updates button states)
  - Labeled as "Mute (Expr)" in UI

### Identity Request/Reply
- **Request**: `F0 7E 0n 06 01 F7`
- **Reply**: `F0 7E 7F 06 02 43 00 41 1D 03 00 00 00 01 F7`
  - Device ID: 1D 03 (Yamaha RM1x)

### XG Parameter Change
- **Format**: `F0 43 1n 4C aa aa aa dd ... dd F7`
  - 43 = Yamaha ID
  - 1n = Parameter Change (n=channel/device 0-F)
  - 4C = Model ID (XG)
  - aa aa aa = Address (high, mid, low)
  - dd... = Data bytes

#### Multi Part Address Space (08 nn xx)
- Base address: `08 nn xx` where nn=part 0-F (parts 1-16)
- Parameters:
  - `08 nn 01`: Bank Select MSB
  - `08 nn 02`: Bank Select LSB
  - `08 nn 03`: Program Number
  - `08 nn 07`: Volume (CC#7)
  - `08 nn 08`: Pan (CC#10)
  - `08 nn 0B`: Cutoff (CC#74)
  - `08 nn 0C`: Resonance (CC#71)
  - `08 nn 0D`: Reverb Send (CC#91)
  - `08 nn 0E`: Chorus Send (CC#93)

### XG Parameter Request (Experimental)
- **Format**: `F0 43 3n 4C aa aa aa F7`
  - 3n = Parameter Request
  - Requests data at address aa aa aa
  - **Note**: Not explicitly documented in List Book; standard Yamaha XG convention
  - Reply format expected: Parameter Change with requested data
  
#### Sync from RM1x Button
- **Function**: Syncs UI state from current RM1x pattern setup
- **Behavior** (Hardware Verified):
  1. Sends Identity Request (`F0 7E 7F 06 01 F7`) to verify connection (~11ms response)
  2. Re-sends Song Select (F3 nn) for currently selected pattern to trigger setup broadcast
- **Expected Response**: 
  - Identity Reply (`F0 7E 7F 06 02 43 00 41 1D 03 00 00 00 01 F7`) → Status: "RM1x detected (1D 03)"
  - Pattern broadcast (~360-370 messages in ~0.5s) with all part parameters
  - Incoming F3 nn updates Pattern dropdown to match
- **Status Display**: 
  - "Syncing from RM1x pattern X..." when sent
  - "RM1x detected (1D 03)" when identity reply received
  - "Pattern X sync complete" when F3 echo confirms sync
  - If no response, check MIDI In/Out device selection and USB cable connection
- **Purpose**: Refresh UI state from hardware after manual changes on RM1x panel

#### XG Parameter Request - Does NOT Work on RM1x (Hardware Verified)
- **Tested**: `F0 43 30 4C 08 00 00 F7` (Multi Part request) → **SILENCE** (no reply)
- **Tested**: `F0 43 20 4C ...` variants (bulk dump request) → **SILENCE** (no reply)
- **Conclusion**: RM1x does not implement XG Parameter Request or Bulk Dump Request
- **Working alternative**: Use Song Select (F3) to trigger the hardware's own setup broadcast
- **Note**: XG Parameter Request format (3n = request, 2n = bulk dump) is standard Yamaha XG convention but not supported by RM1x firmware

## Voice Editing CCs (Currently Supported)
- CC#7: Volume
- CC#10: Pan
- CC#71: Resonance
- CC#74: Cutoff
- CC#91: Reverb Send
- CC#93: Chorus Send

## Transport Control
- **Start**: FA
- **Continue**: FB  
- **Stop**: FC

## Master Tempo Control (Hardware Verified)
- **Implementation**: MIDI Clock (0xF8) continuous transmission at 24 PPQN
- **Timing**: Interval = 60.0 / (BPM × 24) seconds per F8 message
  - Example: 120 BPM = 60.0 / (120 × 24) = 0.020833 seconds = ~20.83ms per F8
  - Implemented with juce::HighResolutionTimer for precise clock intervals
- **BPM range**: 5-300 (RM1x standard range)
- **Transport**: Sends Start (FA) when clock first engages, Stop (FC) when stopped
- **UI**: Tempo knob (5-300 BPM) drives continuous MIDI Clock output
- **Hardware Requirement**: RM1x must be set to **Sync = MIDI** (slave mode) in Utility settings
  - When Sync = Internal, RM1x ignores external F8 clock
  - Clock is transmitted continuously when tempo control is active
- **Status**: Hardware verified on Music PC (USB2MIDI) - F8 rate controls RM1x BPM when in MIDI sync mode
- **Note**: XG SysEx tempo commands (e.g. F0 43 10 4C 00 00 00...) do NOT work on RM1x
  - Address 00 00 00 is Master Tune, not tempo
  - RM1x requires MIDI Clock for external tempo sync

## Part Names / Labels
**Status**: Not yet implemented in UI.

**Implementation Plan**:
1. Send XG Parameter Request for Multi Part data (via "Request Dump" button)
2. Parse XG Parameter Change responses to retrieve:
   - Bank MSB/LSB (address 08 nn 01, 08 nn 02)
   - Program Number (address 08 nn 03)
3. Map bank+program to GM/XG voice name lookup table
4. Display voice names under Mute buttons or in a separate part info panel

Alternatively, if RM1x supports bulk dump requests for pattern data, parse pattern/part names directly from dump.

## Undocumented / Not Yet Implemented

### Track Selection
- No documented remote Track Select message in List Book
- Track selection remains local-only for voice editing knobs
- UI shows Track 1-16 buttons but no MIDI TX/RX

### Bulk Dumps
- Standard Yamaha bulk dump request format (`F0 43 2n ...`) not documented in List Book
- No pattern data dump format specified
- Voice/patch name fields not found in Multi Part table
- **Recommendation**: Test XG Parameter Request on hardware to verify if supported

### Advanced Features to Explore
1. Test XG Parameter Request for Multi Part data retrieval
2. Investigate if pattern/style name strings exist in any SysEx format
3. Check if real-time parameter display messages exist (e.g., current BPM, measure position)
4. Verify if other Yamaha-specific SysEx messages work (e.g., Master Tuning)

## References
- RM1x List Book (listbook_2d05.txt)
- MIDI Implementation Chart (page 22 of List Book)
- XG Parameter tables (pages 19766-20600+ of List Book)
