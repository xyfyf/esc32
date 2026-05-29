#Requires -Version 5.1
$ErrorActionPreference = "Continue"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
. "$Root\scripts\env.ps1"

$allOk = $true

function Show-Tool([string]$name, [bool]$required = $true) {
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) {
        Write-Host "[OK] $name -> $($cmd.Source)" -ForegroundColor Green
        return $true
    }
    if ($required) {
        Write-Host "[MISS] $name" -ForegroundColor Red
        $script:allOk = $false
    } else {
        Write-Host "[--] $name (optional)" -ForegroundColor DarkGray
    }
    return $false
}

Write-Host "`n=== esc32 toolchain check ===`n" -ForegroundColor Cyan

Show-Tool "gcc" | Out-Null
if (-not (Show-Tool "mingw32-make")) { Show-Tool "make" | Out-Null }
Show-Tool "cmake" $false | Out-Null
Show-Tool "python" | Out-Null
Show-Tool "arm-none-eabi-gcc" $false | Out-Null

$sim = Join-Path $Root "firmware\esc32_sim.exe"
if (Test-Path $sim) {
    Write-Host "[OK] firmware\esc32_sim.exe" -ForegroundColor Green
} else {
    Write-Host "[--] firmware\esc32_sim.exe (cd firmware; mingw32-make)" -ForegroundColor Yellow
}

if (Test-Path (Join-Path $Root "host\.venv\Scripts\python.exe")) {
    Write-Host "[OK] host\.venv" -ForegroundColor Green
} else {
    Write-Host "[--] host\.venv (run setup-build-env.ps1)" -ForegroundColor Yellow
}

Write-Host ""
if ($allOk) { exit 0 } else { exit 1 }
