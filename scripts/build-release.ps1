#Requires -Version 5.1
<#
.SYNOPSIS
  Build the double-clickable release directory dist\esc32\.

  Contents:
    esc32_sim.exe     Simulation firmware
    esc_tool.exe      Host GUI
    esc32_start.exe   One-click launcher (sim + GUI)
    defaults\         Parameter presets 60/80/120/200.json
#>
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)

. "$Root\scripts\env.ps1"

$Dist = Join-Path $Root "dist\esc32"
$Build = Join-Path $Root "host\build_pyinstaller"

Write-Host "=== [1/4] Build esc32_sim.exe ===" -ForegroundColor Cyan
Push-Location "$Root\firmware"
$make = if (Get-Command mingw32-make -ErrorAction SilentlyContinue) { "mingw32-make" } else { "make" }
& $make clean 2>$null | Out-Null
& $make
if (-not (Test-Path ".\esc32_sim.exe")) { throw "firmware build failed" }
Pop-Location

Write-Host "=== [2/4] Python venv + PyInstaller ===" -ForegroundColor Cyan
Push-Location "$Root\host"
if (-not (Test-Path ".venv")) { python -m venv .venv }
& .\.venv\Scripts\pip install -q -r requirements.txt
& .\.venv\Scripts\pip install -q pyinstaller

$pyi = ".\.venv\Scripts\pyinstaller.exe"

Write-Host "=== [3/4] Package esc_tool.exe / esc32_start.exe ===" -ForegroundColor Cyan
& $pyi --noconfirm --clean --onefile --windowed --name esc_tool `
    --distpath $Dist --workpath $Build --specpath $Build `
    --hidden-import PyQt6.QtCore `
    --hidden-import PyQt6.QtGui `
    --hidden-import PyQt6.QtWidgets `
    --collect-all pyqtgraph `
    run_gui.py

& $pyi --noconfirm --clean --onefile --windowed --name esc32_start `
    --distpath $Dist --workpath $Build --specpath $Build `
    "..\scripts\esc32_start.py"

# Clean up PyInstaller onedir leftovers
Remove-Item -Recurse -Force -ErrorAction SilentlyContinue `
    (Join-Path $Dist "esc_tool"), (Join-Path $Dist "esc32_start")

Pop-Location

Write-Host "=== [4/4] Assemble dist\esc32 ===" -ForegroundColor Cyan
New-Item -ItemType Directory -Force -Path $Dist | Out-Null
Copy-Item -Force "$Root\firmware\esc32_sim.exe" $Dist
$defOut = Join-Path $Dist "defaults"
New-Item -ItemType Directory -Force -Path $defOut | Out-Null
Copy-Item -Force "$Root\shared\defaults\*.json" $defOut

$readme = @"
esc32 release package (double-click to run)

  esc32_start.exe   Recommended: one-click launch (sim + host)
  esc32_sim.exe     Simulation firmware only (UDP 7777/7779)
  esc_tool.exe      Host GUI only (connects to 127.0.0.1:7777)
  defaults\         Parameter presets 60/80/120/200.json

To rebuild on this machine: powershell -File scripts\build-release.ps1
"@
Set-Content -Path (Join-Path $Dist "README.txt") -Value $readme -Encoding UTF8

Write-Host ""
Write-Host "OK: $Dist" -ForegroundColor Green
Write-Host "  Double-click: esc32_start.exe" -ForegroundColor Cyan
Get-ChildItem $Dist -File | Format-Table Name, Length -AutoSize
