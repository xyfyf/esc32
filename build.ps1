param(
    [ValidateSet("sim", "stm32")]
    [string]$Platform = "sim"
)

$ErrorActionPreference = "Stop"
$Root = $PSScriptRoot

Write-Host "=== firmware ($Platform) ===" -ForegroundColor Cyan
Push-Location "$Root\firmware"
$makeCmd = $null
foreach ($m in @("mingw32-make", "make", "gmake")) {
    if (Get-Command $m -ErrorAction SilentlyContinue) { $makeCmd = $m; break }
}
if ($makeCmd -and (Get-Command gcc -ErrorAction SilentlyContinue)) {
    & $makeCmd clean 2>$null
    & $makeCmd
} elseif (Get-Command cmake -ErrorAction SilentlyContinue) {
    cmake -B build -DCMAKE_BUILD_TYPE=Debug -DESC_PLATFORM=$Platform
    cmake --build build --config Debug
} else {
    Write-Warning "未找到 gcc/make/cmake。请运行: powershell -File scripts\setup-build-env.ps1"
}
Pop-Location

Write-Host "=== host ===" -ForegroundColor Cyan
Push-Location "$Root\host"
if (-not (Test-Path ".venv")) { python -m venv .venv }
& .\.venv\Scripts\pip install -q -r requirements.txt
Pop-Location

Write-Host "完成。仿真: firmware\esc32_sim.exe + host 上位机 --sim-udp" -ForegroundColor Green
