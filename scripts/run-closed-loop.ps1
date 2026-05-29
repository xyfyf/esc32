#Requires -Version 5.1
# One-click closed loop: build firmware -> start simulator -> E2E tests -> report
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)

$env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" +
            [System.Environment]::GetEnvironmentVariable("Path", "User")

Write-Host "=== [1/4] Build firmware ===" -ForegroundColor Cyan
Push-Location "$Root\firmware"
$make = if (Get-Command mingw32-make -EA SilentlyContinue) { "mingw32-make" }
        elseif (Get-Command make -EA SilentlyContinue) { "make" } else { $null }
if (-not $make) { throw "mingw32-make not found. Run scripts\setup-build-env.ps1" }
& $make
if (-not (Test-Path ".\esc32_sim.exe")) { throw "Build failed" }
Pop-Location

Write-Host "=== [2/4] Start simulator ===" -ForegroundColor Cyan
$sim = Start-Process -FilePath "$Root\firmware\esc32_sim.exe" -WorkingDirectory "$Root\firmware" -PassThru -WindowStyle Hidden
Start-Sleep -Seconds 1

Write-Host "=== [3/4] Host venv + E2E ===" -ForegroundColor Cyan
Push-Location "$Root\host"
if (-not (Test-Path ".venv")) { python -m venv .venv }
& .\.venv\Scripts\pip install -q -r requirements.txt
$e2e = & .\.venv\Scripts\python -m esc_tool.e2e_test 127.0.0.1:7777
$code = $LASTEXITCODE
Pop-Location

Write-Host "=== [4/4] Cleanup ===" -ForegroundColor Cyan
if (-not $sim.HasExited) { Stop-Process -Id $sim.Id -Force -ErrorAction SilentlyContinue }

if ($code -eq 0) {
    Write-Host "`nCLOSED LOOP OK" -ForegroundColor Green
} else {
    Write-Host "`nCLOSED LOOP FAILED (exit $code)" -ForegroundColor Red
}
exit $code
