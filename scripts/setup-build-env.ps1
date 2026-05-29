#Requires -Version 5.1
param(
    [switch]$SkipHost,
    [switch]$SkipWinget,
    [switch]$SkipArm,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)

function Test-Cmd($name) {
    return [bool](Get-Command $name -ErrorAction SilentlyContinue)
}

Write-Host "=== esc32 build env setup ===" -ForegroundColor Cyan

if (-not $SkipWinget -and (Test-Cmd winget)) {
    Write-Host "[1/5] winget install toolchain..." -ForegroundColor Yellow
    $packages = @(
        @{ Id = "BrechtSanders.WinLibs.POSIX.UCRT"; Name = "WinLibs GCC" },
        @{ Id = "Kitware.CMake"; Name = "CMake" }
    )
    if (-not $SkipArm) {
        $packages += @{ Id = "Arm.GnuArmEmbeddedToolchain"; Name = "ARM GCC" }
    }
    foreach ($pkg in $packages) {
        $listed = winget list --id $pkg.Id -e 2>$null | Select-String $pkg.Id
        if ($listed) {
            Write-Host "  already: $($pkg.Name)" -ForegroundColor DarkGray
        } else {
            Write-Host "  installing: $($pkg.Name) ..." -ForegroundColor Green
            winget install --id $pkg.Id -e --accept-package-agreements --accept-source-agreements
        }
    }
} else {
    Write-Host "[1/5] skip winget" -ForegroundColor DarkGray
}

Write-Host "[2/5] load PATH (env.ps1) ..." -ForegroundColor Yellow
. "$Root\scripts\env.ps1"

Write-Host "[3/5] verify tools ..." -ForegroundColor Yellow
$missing = @()
foreach ($t in @("gcc", "python")) {
    if (-not (Test-Cmd $t)) { $missing += $t }
}
$makeCmd = $null
if (Test-Cmd mingw32-make) { $makeCmd = "mingw32-make" }
elseif (Test-Cmd make) { $makeCmd = "make" }
else { $missing += "mingw32-make" }

if ($missing.Count -gt 0) {
    Write-Host "Missing: $($missing -join ', ')" -ForegroundColor Red
    Write-Host "Re-open terminal after winget install, then re-run this script." -ForegroundColor Yellow
    exit 1
}

gcc --version 2>&1 | Select-Object -First 1 | ForEach-Object { Write-Host "  gcc: $_" -ForegroundColor Green }
& $makeCmd --version 2>&1 | Select-Object -First 1 | ForEach-Object { Write-Host "  $makeCmd : $_" -ForegroundColor Green }
if (Test-Cmd cmake) {
    cmake --version 2>&1 | Select-Object -First 1 | ForEach-Object { Write-Host "  cmake: $_" -ForegroundColor Green }
}
if (Test-Cmd arm-none-eabi-gcc) {
    arm-none-eabi-gcc --version 2>&1 | Select-Object -First 1 | ForEach-Object { Write-Host "  arm-none-eabi-gcc: $_" -ForegroundColor Green }
} else {
    Write-Host "  arm-none-eabi-gcc: not installed (optional, for STM32 flash builds)" -ForegroundColor DarkGray
}

if (-not $SkipBuild) {
    Write-Host "[4/5] build firmware (sim) ..." -ForegroundColor Yellow
    Push-Location "$Root\firmware"
    Remove-Item -Force -ErrorAction SilentlyContinue esc32_sim.exe, esc32_*.elf, esc32_nvm.bin
    & $makeCmd
    if (-not (Test-Path ".\esc32_sim.exe")) {
        Write-Host "Build failed (exit $LASTEXITCODE)" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    Write-Host "OK: firmware\esc32_sim.exe" -ForegroundColor Green
    Pop-Location
} else {
    Write-Host "[4/5] skip build" -ForegroundColor DarkGray
}

if (-not $SkipHost) {
    Write-Host "[5/5] host python venv ..." -ForegroundColor Yellow
    Push-Location "$Root\host"
    if (-not (Test-Path ".venv")) { python -m venv .venv }
    & .\.venv\Scripts\pip install -q -r requirements.txt
    & .\.venv\Scripts\python -c "from esc_tool.gui import MainWindow; print('host OK')"
    Pop-Location
} else {
    Write-Host "[5/5] skip host" -ForegroundColor DarkGray
}

Write-Host ""
Write-Host "Done." -ForegroundColor Cyan
Write-Host "  Each new terminal:  . .\scripts\env.ps1" -ForegroundColor White
Write-Host "  Check tools:        .\scripts\check-toolchain.ps1" -ForegroundColor White
Write-Host "  Run sim:            firmware\esc32_sim.exe" -ForegroundColor White
Write-Host "  GUI:                host\.venv\Scripts\python -m esc_tool --gui" -ForegroundColor White
