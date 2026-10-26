# RM1x Controller - VST3 Installation Script for Windows Music PC
# Installs to $env:USERPROFILE\Documents\rm1x-controller
# DAW: Studio One 7

Write-Host "============================================" -ForegroundColor Cyan
Write-Host "RM1x Controller v0.1.0 - VST3 Installation" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# Check if running as Administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "WARNING: Not running as Administrator." -ForegroundColor Yellow
    Write-Host "System-wide installation to Common Files may fail." -ForegroundColor Yellow
    Write-Host "User VST3 folder will still work for most DAWs." -ForegroundColor Yellow
    Write-Host ""
}

# Source VST3 location (Ninja build output)
$sourceVst3 = "build\RM1xController_artefacts\VST3\RM1x Controller.vst3"

# Fallback to VS IDE output path if not found
if (-not (Test-Path $sourceVst3)) {
    $sourceVst3 = "build\RM1xController_artefacts\Release\VST3\RM1x Controller.vst3"
}

if (-not (Test-Path $sourceVst3)) {
    Write-Host "ERROR: VST3 plugin not found at:" -ForegroundColor Red
    Write-Host "  $sourceVst3" -ForegroundColor Red
    Write-Host "Please run rebuild.bat first to build the plugin." -ForegroundColor Red
    exit 1
}

# Target installation paths
$systemVst3 = "$env:CommonProgramFiles\VST3"
$userVst3 = "$env:USERPROFILE\Documents\VST3"
$projectDir = Join-Path $env:USERPROFILE "Documents\rm1x-controller"

# Create project folder structure
Write-Host "Creating project folder..." -ForegroundColor Green
$projectBuild = Join-Path $projectDir "build"
$projectPlugins = Join-Path $projectDir "plugins"
New-Item -ItemType Directory -Force -Path $projectDir | Out-Null
New-Item -ItemType Directory -Force -Path $projectBuild | Out-Null
New-Item -ItemType Directory -Force -Path $projectPlugins | Out-Null

# Function to copy VST3 bundle
function Copy-Vst3Bundle {
    param(
        [string]$Source,
        [string]$Destination
    )
    
    $destBundle = Join-Path $Destination "RM1x Controller.vst3"
    
    Write-Host "  Installing to: $destBundle" -ForegroundColor Gray
    
    # Remove existing installation
    if (Test-Path $destBundle) {
        Remove-Item -Recurse -Force $destBundle
    }
    
    # Copy VST3 bundle
    Copy-Item -Recurse -Force $Source $destBundle
    
    if (Test-Path $destBundle) {
        Write-Host "  [OK] Installed successfully" -ForegroundColor Green
        return $true
    } else {
        Write-Host "  [FAIL] Installation failed" -ForegroundColor Red
        return $false
    }
}

# Install to project plugins folder (always succeeds)
Write-Host ""
Write-Host "1. Installing to project folder..." -ForegroundColor Cyan
Copy-Vst3Bundle -Source $sourceVst3 -Destination $projectPlugins

# Install to User Documents\VST3 (Studio One 7 default scan path)
Write-Host ""
Write-Host "2. Installing to User VST3 folder (Studio One 7)..." -ForegroundColor Cyan
New-Item -ItemType Directory -Force -Path $userVst3 | Out-Null
Copy-Vst3Bundle -Source $sourceVst3 -Destination $userVst3

# Install to Common Files\VST3 (system-wide, requires admin)
Write-Host ""
Write-Host "3. Installing to System VST3 folder (all DAWs)..." -ForegroundColor Cyan
New-Item -ItemType Directory -Force -Path $systemVst3 -ErrorAction SilentlyContinue | Out-Null

try {
    Copy-Vst3Bundle -Source $sourceVst3 -Destination $systemVst3
} catch {
    Write-Host "  [SKIP] Requires Administrator privileges" -ForegroundColor Yellow
    Write-Host "         Run as Administrator to install system-wide" -ForegroundColor Yellow
}

# Copy standalone executable to project
Write-Host ""
Write-Host "4. Copying standalone executable..." -ForegroundColor Cyan
$sourceExe = "build\RM1xController_artefacts\Standalone\RM1x Controller.exe"

# Fallback to VS IDE output path if not found
if (-not (Test-Path $sourceExe)) {
    $sourceExe = "build\RM1xController_artefacts\Release\Standalone\RM1x Controller.exe"
}

$destExe = Join-Path $projectDir "RM1x Controller.exe"

if (Test-Path $sourceExe) {
    Copy-Item -Force $sourceExe $destExe
    Write-Host "  [OK] $destExe" -ForegroundColor Green
} else {
    Write-Host "  [SKIP] Standalone executable not found" -ForegroundColor Yellow
}

# Summary
Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "Installation Summary" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Project folder:     $projectDir" -ForegroundColor White
Write-Host "  - Standalone:       RM1x Controller.exe" -ForegroundColor Gray
Write-Host "  - VST3:             plugins\RM1x Controller.vst3" -ForegroundColor Gray
Write-Host ""
Write-Host "User VST3 (Studio One): $userVst3" -ForegroundColor White
Write-Host "System VST3 (All DAWs): $systemVst3" -ForegroundColor White
Write-Host ""
Write-Host "Next Steps:" -ForegroundColor Yellow
Write-Host "1. Launch Studio One 7" -ForegroundColor Gray
Write-Host "2. Go to Studio One > Options > Locations > VST Plug-Ins" -ForegroundColor Gray
Write-Host "3. Ensure Documents\VST3 is in the scan paths" -ForegroundColor Gray
Write-Host "4. Scan for new plugins (or restart Studio One)" -ForegroundColor Gray
Write-Host "5. Find 'RM1x Controller' in Tools category" -ForegroundColor Gray
Write-Host ""
Write-Host "MIDI Setup Reminder:" -ForegroundColor Yellow
Write-Host "- Connect RM1x MIDI Out -> Computer MIDI In" -ForegroundColor Gray
Write-Host "- Connect Computer MIDI Out -> RM1x MIDI In" -ForegroundColor Gray
Write-Host "- Configure RM1x: UTILITY > MIDI Control = IN/OUT" -ForegroundColor Gray
Write-Host "- Configure RM1x: UTILITY > Filter (SysEx) = ENABLED" -ForegroundColor Gray
Write-Host ""
