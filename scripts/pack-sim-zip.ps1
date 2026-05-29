#Requires -Version 5.1
<#
.SYNOPSIS
  Pack the simulation firmware + presets into a release zip.

.DESCRIPTION
  Produces dist\esc32-<version>-sim.zip containing:
    esc32_sim.exe        PC simulation firmware
    defaults\*.json      Parameter presets
    README.txt           Quick instructions

  Run after `mingw32-make` so esc32_sim.exe exists.

.PARAMETER Version
  Tag/version label embedded in the zip name. Defaults to v0.0.0-dev.
#>
param(
    [string]$Version = "v0.0.0-dev"
)
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)

$Sim = Join-Path $Root "firmware\esc32_sim.exe"
if (-not (Test-Path $Sim)) {
    throw "esc32_sim.exe not found at $Sim. Run 'mingw32-make' in firmware/ first."
}

$Stage = Join-Path $Root "dist\sim-stage"
if (Test-Path $Stage) { Remove-Item -Recurse -Force $Stage }
New-Item -ItemType Directory -Force -Path $Stage | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $Stage "defaults") | Out-Null

Copy-Item -Force $Sim $Stage
Copy-Item -Force "$Root\shared\defaults\*.json" (Join-Path $Stage "defaults")

$readme = @"
esc32 simulation firmware (release $Version)
============================================

Files
  esc32_sim.exe     PC simulation firmware. Listens on UDP 7777 (debug
                    protocol) and UDP 7779 (DroneCAN frames).
  defaults\         Parameter presets for ESC-60 / 80 / 120 / 200.

Quick start
  1. Double-click esc32_sim.exe (a console window appears).
  2. Use the host GUI or CLI to connect to 127.0.0.1:7777.
       python -m esc_tool --gui
       python -m esc_tool --sim-udp 127.0.0.1:7777 ping
  3. Apply a preset (optional):
       python -m esc_tool.preset_apply --sim-udp 127.0.0.1:7777 defaults\80.json

NOTE
  This package contains ONLY the PC simulation firmware. Real-hardware
  firmware (.bin/.hex) is shipped as a separate archive
  (esc32-$Version-firmware.zip) and is currently UNVERIFIED on hardware.

Source: https://github.com/xyfyf/esc32 (release $Version)
"@
Set-Content -Path (Join-Path $Stage "README.txt") -Value $readme -Encoding UTF8

$Out = Join-Path $Root "dist\esc32-$Version-sim.zip"
if (Test-Path $Out) { Remove-Item -Force $Out }
Compress-Archive -Path "$Stage\*" -DestinationPath $Out -CompressionLevel Optimal

Remove-Item -Recurse -Force $Stage

$size = '{0:N1} KB' -f ((Get-Item $Out).Length / 1KB)
Write-Host "OK  $Out  ($size)" -ForegroundColor Green
