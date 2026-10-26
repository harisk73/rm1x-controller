# Crash Fix for Commit 3a1cadc (Windows Standalone)

## Symptoms

**Exception:** 0xc0000005 ACCESS_VIOLATION  
**Fault Offset:** 0x00000000001e3ad4  
**Faulting Module:** RM1x Controller.exe  
**Trigger:** Immediate crash on launch, every time  
**Platform:** Windows Music PC  
**When:** After deploying commit 3a1cadc (purple-blue + MIDI input)

## Root Cause

**Backwards Compatibility Issue in State Restoration**

The crash was caused by `setStateInformation()` trying to read **two strings** from saved state data that only contained **one string**.

### What Happened

**Before commit 3a1cadc (v0.1.0):**
```cpp
void getStateInformation(juce::MemoryBlock& destData)
{
    stream.writeString(currentMidiOutputName);  // Only 1 string
}

void setStateInformation(const void* data, int sizeInBytes)
{
    currentMidiOutputName = stream.readString();  // Only 1 string
    setMidiOutput(currentMidiOutputName);
}
```

**After commit 3a1cadc (with MIDI input):**
```cpp
void getStateInformation(juce::MemoryBlock& destData)
{
    stream.writeString(currentMidiOutputName);
    stream.writeString(currentMidiInputName);  // Added 2nd string
}

void setStateInformation(const void* data, int sizeInBytes)
{
    currentMidiOutputName = stream.readString();
    currentMidiInputName = stream.readString();  // ❌ CRASH if reading old state!
    setMidiOutput(currentMidiOutputName);
    setMidiInput(currentMidiInputName);
}
```

### Sequence of Events

1. User ran previous version (green-active build)
2. Standalone app saved state with **one string** (MIDI output name)
3. User deployed new version (3a1cadc with MIDI input)
4. Standalone app loaded old state on startup
5. `setStateInformation()` tried to read **second string** that didn't exist
6. `MemoryInputStream::readString()` read past end of valid data
7. **ACCESS_VIOLATION** → immediate crash

### Why No Device Selection Was Needed

The crash happened **during state restoration at startup**, before the user could interact with the UI. The saved state from the previous session was automatically loaded by JUCE's standalone wrapper.

## The Fix

### 1. Backwards-Compatible State Loading

```cpp
void RM1xControllerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    
    // Read MIDI output name (always present)
    currentMidiOutputName = stream.readString();
    
    // Read MIDI input name if available (backwards compatibility)
    if (!stream.isExhausted())
        currentMidiInputName = stream.readString();
    else
        currentMidiInputName = juce::String();
    
    setMidiOutput(currentMidiOutputName);
    setMidiInput(currentMidiInputName);
}
```

**Key Change:** Check `stream.isExhausted()` before reading second string.

**Behavior:**
- **New state (2 strings):** Reads both output and input names ✅
- **Old state (1 string):** Reads output name, skips input name, no crash ✅
- **Empty state:** Both names empty, no devices selected ✅

### 2. Editor Callback Cleanup

Added callback clearing in destructor to prevent dangling references:

```cpp
RM1xControllerAudioProcessorEditor::~RM1xControllerAudioProcessorEditor()
{
    // Clear callbacks to prevent any dangling references
    audioProcessor.onMidiInputActivity = nullptr;
    audioProcessor.onSectionChanged = nullptr;
    audioProcessor.onMuteChanged = nullptr;
    audioProcessor.onTrackChanged = nullptr;
    
    setLookAndFeel(nullptr);
}
```

### 3. MIDI Input Cleanup

Added explicit stop and cleanup in processor destructor:

```cpp
RM1xControllerAudioProcessor::~RM1xControllerAudioProcessor()
{
    // Explicitly stop and release MIDI input before callback is destroyed
    if (midiInput)
    {
        midiInput->stop();
        midiInput.reset();
    }
    midiInputCallback.reset();
}
```

**Order matters:** Stop input → release input → release callback.

## Why This Is Safe

1. **No MIDI Device Required:** App opens fine with no devices selected
2. **Backwards Compatible:** Old saved state loads without crashing
3. **Forward Compatible:** New saved state includes both device names
4. **Thread Safe:** Callbacks use `MessageManager::callAsync()`
5. **Null Safe:** Callbacks check component existence before accessing
6. **Lifetime Safe:** Proper cleanup order in destructors

## Testing

✅ **Linux Build:** Passes  
✅ **No Device Selected:** Opens without crash  
✅ **Old State Load:** Backwards compatible  
✅ **New State Save:** Includes both devices  

## Related Files Modified

- `Source/PluginProcessor.cpp`: State loading fix, destructor cleanup
- `Source/PluginEditor.cpp`: Callback cleanup in destructor
- `CRASH-FIX-3a1cadc.md`: This document

## Lessons Learned

**Always handle backwards compatibility when changing saved state format:**
- Check stream position/exhausted before reading new fields
- Test with state data from previous versions
- Consider versioning saved state data for complex changes

**State restoration happens automatically at startup:**
- Standalone apps restore state from previous session
- VST3 plugins restore state from DAW project
- Crashes during state load appear as "immediate startup crash"
