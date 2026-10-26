# Build Instructions

## Quick Start (Linux)

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential g++ cmake pkg-config \
    libstdc++-13-dev libasound2-dev libfreetype6-dev libx11-dev \
    libxinerama-dev libxext-dev libxrandr-dev libxcursor-dev \
    libgl1-mesa-dev libfontconfig1-dev

# Configure
CXX=g++ CC=gcc cmake -B build

# Build
cmake --build build --config Release -j$(nproc)

# Artifacts will be in:
# - build/RM1xController_artefacts/VST3/RM1x Controller.vst3/
# - build/RM1xController_artefacts/Standalone/RM1x Controller
```

## Windows Build

### Requirements
- Visual Studio 2022 Build Tools (with C++ workload)
- Ninja build system (included with VS Build Tools)
- CMake 3.22 or later
- Git

### Build Steps

**Option 1: Using rebuild.bat (recommended)**

```cmd
rebuild.bat
```

**Option 2: Manual build**

```cmd
# Initialize VS Build Tools environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64

# Create and enter build directory
mkdir build
cd build

# Configure with Ninja
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build
ninja
```

Artifacts will be in `build\RM1xController_artefacts\`:
- `VST3\RM1x Controller.vst3\` (VST3 plugin)
- `Standalone\RM1x Controller.exe` (Standalone app)

## macOS Build

### Requirements
- Xcode Command Line Tools: `xcode-select --install`
- CMake 3.15+: `brew install cmake`

### Build Steps

```bash
cmake -B build -G Xcode
cmake --build build --config Release
```

Artifacts will be in `build/RM1xController_artefacts/Release/`:
- `RM1x Controller.vst3` (VST3 plugin)
- `RM1x Controller.app` (Standalone app)

## Using a Local JUCE Installation

If you have JUCE 8 installed locally:

```bash
mkdir -p libs
ln -s /path/to/JUCE libs/JUCE
cmake -B build
cmake --build build --config Release
```

The CMakeLists.txt will automatically use the local JUCE instead of fetching from GitHub.

## Installation

### VST3 Plugin

Copy the built VST3 to your system's plugin folder:

**Linux:**
```bash
cp -r "build/RM1xController_artefacts/VST3/RM1x Controller.vst3" ~/.vst3/
```

**Windows:**
```cmd
xcopy /E /I "build\RM1xController_artefacts\Release\VST3\RM1x Controller.vst3" ^
  "%CommonProgramFiles%\VST3\RM1x Controller.vst3"
```

**macOS:**
```bash
cp -r "build/RM1xController_artefacts/Release/VST3/RM1x Controller.vst3" \
  ~/Library/Audio/Plug-Ins/VST3/
```

### Standalone Application

The standalone executable can be run directly from the build folder or copied anywhere.

## Clean Build

```bash
rm -rf build
cmake -B build
cmake --build build --config Release
```

## Troubleshooting

### Linux: Missing freetype headers

```bash
sudo apt-get install libfreetype-dev libfontconfig1-dev
```

### Linux: Missing ALSA/X11

```bash
sudo apt-get install libasound2-dev libx11-dev libxinerama-dev \
    libxext-dev libxrandr-dev libxcursor-dev
```

### Windows: MSVC not found

Install "Desktop development with C++" workload in Visual Studio Installer.

### macOS: No CMAKE_CXX_COMPILER

Install Xcode Command Line Tools:
```bash
xcode-select --install
```

## Building from IDE

### Visual Studio (Windows)

1. Open the folder in VS 2022 (File → Open → Folder)
2. Select the CMake configuration
3. Build → Build All

### Xcode (macOS)

```bash
cmake -B build -G Xcode
open build/RM1xController.xcodeproj
```

Then build from Xcode normally.

### CLion / Qt Creator

Both support CMake projects directly. Open the root `CMakeLists.txt` file.
