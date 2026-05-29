#Requires -Version 5.1
<#
.SYNOPSIS
  加载 esc32 开发环境 PATH（当前 PowerShell 会话有效）

.USAGE
  . .\scripts\env.ps1
#>
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)

function Add-PathIfExists([string]$dir) {
    if ($dir -and (Test-Path $dir) -and ($env:Path -notlike "*$dir*")) {
        $env:Path = "$dir;$env:Path"
    }
}

function Find-WinLibsBin {
    $roots = @(
        "$env:ProgramFiles\WinLibs",
        "${env:ProgramFiles(x86)}\WinLibs",
        "$env:LocalAppData\Microsoft\WinGet\Packages"
    )
    foreach ($r in $roots) {
        if (-not (Test-Path $r)) { continue }
        $gcc = Get-ChildItem -Path $r -Recurse -Filter "gcc.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($gcc) { return $gcc.Directory.FullName }
    }
    return $null
}

function Find-MsysGcc {
    foreach ($b in @("C:\msys64\ucrt64\bin", "C:\msys64\mingw64\bin")) {
        if (Test-Path (Join-Path $b "gcc.exe")) { return $b }
    }
    return $null
}

function Find-ArmGccBin {
    $roots = @(
        "$env:ProgramFiles\Arm GNU Toolchain*",
        "$env:ProgramFiles (x86)\Arm GNU Toolchain*",
        "$env:LocalAppData\Microsoft\WinGet\Packages"
    )
    foreach ($pattern in $roots) {
        $dirs = Get-ChildItem -Path $pattern -ErrorAction SilentlyContinue
        foreach ($d in $dirs) {
            $gcc = Get-ChildItem -Path $d.FullName -Recurse -Filter "arm-none-eabi-gcc.exe" -ErrorAction SilentlyContinue |
                Select-Object -First 1
            if ($gcc) { return $gcc.Directory.FullName }
        }
    }
    return $null
}

# MinGW（仿真固件）
$gccDir = Find-MsysGcc
if (-not $gccDir) { $gccDir = Find-WinLibsBin }
if ($gccDir) { Add-PathIfExists $gccDir }

# CMake
foreach ($cp in @("$env:ProgramFiles\CMake\bin", "${env:ProgramFiles(x86)}\CMake\bin")) {
    if (Test-Path $cp) { Add-PathIfExists $cp; break }
}

# ARM GCC（真机，可选）
$armBin = Find-ArmGccBin
if ($armBin) {
    Add-PathIfExists $armBin
    $env:ESC_ARM_GCC_PREFIX = "arm-none-eabi-"
}

$env:ESC32_ROOT = $Root
Write-Host "esc32 env loaded (ESC32_ROOT=$Root)" -ForegroundColor Cyan
