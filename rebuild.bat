@echo off
REM RM1x Controller - Windows Build Script (Orbiter-style)
REM Targets: VS BuildTools 2022, Ninja, x64 Release

echo ============================================
echo RM1x Controller v0.2.0 - Windows Build
echo ============================================
echo.

REM Initialize Visual Studio Build Tools environment
echo Setting up VS 2022 Build Tools environment...
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to initialize VS Build Tools environment!
    echo Make sure Visual Studio 2022 Build Tools are installed.
    exit /b 1
)

REM Navigate to project directory (assuming script is run from project root)
echo.
echo Cleaning previous build...
if exist build (
    rmdir /s /q build
)

REM Create build directory
mkdir build
cd build

REM Configure with Ninja
echo.
echo Configuring with CMake + Ninja...
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed!
    cd ..
    exit /b 1
)

REM Build with Ninja
echo.
echo Building with Ninja...
ninja

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed!
    cd ..
    exit /b 1
)

REM Return to project root
cd ..

echo.
echo ============================================
echo Build completed successfully!
echo ============================================
echo.
echo Artifacts location:
echo   VST3:       build\RM1xController_artefacts\VST3\RM1x Controller.vst3
echo   Standalone: build\RM1xController_artefacts\Standalone\RM1x Controller.exe
echo.
echo Run install-vst3.ps1 to install the VST3 plugin to system folders.
echo.
pause
