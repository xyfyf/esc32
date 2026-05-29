#Requires -Version 5.1
<#
.SYNOPSIS
  Pack the ARM firmware artifacts (.elf / .bin / .hex / .map) into a
  release zip.

.DESCRIPTION
  Produces dist\esc32-<version>-firmware.zip containing every
  esc32_*.{arm.elf,bin,hex,map} in firmware/, plus a README that calls
  out the unverified status of the artifacts.

  Run after `mingw32-make arm-verify` so all five ARM artifacts exist.
#>
param(
    [string]$Version = "v0.0.0-dev"
)
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Fw   = Join-Path $Root "firmware"

$pattern = @("esc32_*.arm.elf", "esc32_*.bin", "esc32_*.hex", "esc32_*.map")
$found = @()
foreach ($pat in $pattern) {
    $found += Get-ChildItem -Path $Fw -File -Filter $pat -ErrorAction SilentlyContinue
}
if ($found.Count -eq 0) {
    throw "No ARM artifacts in $Fw. Run 'mingw32-make arm-verify' first."
}

$Stage = Join-Path $Root "dist\fw-stage"
if (Test-Path $Stage) { Remove-Item -Recurse -Force $Stage }
New-Item -ItemType Directory -Force -Path $Stage | Out-Null

foreach ($f in $found) { Copy-Item -Force $f.FullName $Stage }

$readme = @"
esc32 hardware firmware (release $Version)
==========================================

Files (per target)
  esc32_<TARGET>.arm.elf   ARM ELF (with debug info)
  esc32_<TARGET>.bin       Raw binary, flash starting at 0x08000000
  esc32_<TARGET>.hex       Intel HEX, includes start address
  esc32_<TARGET>.map       Linker map

Targets
  ESC60_STM32G431_V1       128K Flash / 32K SRAM, Cortex-M4F
  ESC80_STM32G474_V1       512K Flash / 128K SRAM, Cortex-M4F (first prod target)
  ESC80_AT32F415_V1        256K Flash / 32K SRAM, Cortex-M4 soft-float
  ESC120_STM32H743_V1      2M Flash / 512K AXI-SRAM, Cortex-M7F
  ESC200_STM32H743_V1      same as ESC-120

WARNING - UNVERIFIED ON HARDWARE
  These artifacts are produced from a *minimal startup + stub HAL* build.
  They link cleanly and respect each MCU's memory map, but they DO NOT
  initialise peripherals - on real silicon the firmware boots straight
  into Default_Handler and stays there.
  The artifacts are intended for:
    * link-verification of the esc32 codebase against arm-none-eabi-gcc
    * smoke-testing flasher pipelines / board bring-up tooling
  When the production HAL lands (CubeMX-generated TIM/ADC/FDCAN init
  merged with hal_stm32g474.c and friends), this disclaimer goes away.

Source: https://github.com/xyfyf/esc32 (release $Version)
"@
Set-Content -Path (Join-Path $Stage "README.txt") -Value $readme -Encoding UTF8

$Out = Join-Path $Root "dist\esc32-$Version-firmware.zip"
if (Test-Path $Out) { Remove-Item -Force $Out }
Compress-Archive -Path "$Stage\*" -DestinationPath $Out -CompressionLevel Optimal
Remove-Item -Recurse -Force $Stage

$size = '{0:N1} KB' -f ((Get-Item $Out).Length / 1KB)
Write-Host "OK  $Out  ($size)" -ForegroundColor Green
