#Requires -Version 5.1
$ErrorActionPreference = "Stop"
$here = $PSScriptRoot
$root = Split-Path -Parent $here
. "$root\scripts\env.ps1"

$exe = Join-Path $here "esc32_sim.exe"
if (-not (Test-Path $exe)) {
    Write-Host "esc32_sim.exe not found, building..." -ForegroundColor Yellow
    Push-Location $here
    $make = if (Get-Command mingw32-make -ErrorAction SilentlyContinue) { "mingw32-make" } else { "make" }
    & $make
    Pop-Location
    if (-not (Test-Path $exe)) {
        Write-Host "Build failed. Run from repo root:" -ForegroundColor Red
        Write-Host "  powershell -File scripts\setup-build-env.ps1" -ForegroundColor White
        exit 1
    }
}

Write-Host "Starting esc32_sim.exe (UDP 7777 / 7779) ... Ctrl+C to stop" -ForegroundColor Cyan
Set-Location $here
& $exe
