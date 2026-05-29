#Requires -Version 5.1
<#
.SYNOPSIS
  生成可双击运行的发布目录 dist\esc32\

  包含：
    esc32_sim.exe     仿真固件
    esc_tool.exe      上位机 GUI
    esc32_start.exe   一键启动（仿真 + GUI）
    defaults\         参数预设 60/80/120/200.json
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

# 清理 onedir 残留
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
esc32 发布包（双击运行）

  esc32_start.exe   推荐：一键启动仿真 + 上位机
  esc32_sim.exe     仅仿真固件（UDP 7777/7779）
  esc_tool.exe      仅上位机（连接 127.0.0.1:7777）
  defaults\         参数预设 60/80/120/200.json

首次在本机生成：powershell -File scripts\build-release.ps1
"@
Set-Content -Path (Join-Path $Dist "使用说明.txt") -Value $readme -Encoding UTF8

Write-Host ""
Write-Host "OK: $Dist" -ForegroundColor Green
Write-Host "  Double-click: esc32_start.exe" -ForegroundColor Cyan
Get-ChildItem $Dist -File | Format-Table Name, Length -AutoSize
