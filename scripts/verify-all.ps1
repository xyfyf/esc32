#Requires -Version 5.1
<#
.SYNOPSIS
  esc32 全量软件验证：编译仿真 + 全部硬件 Target 链接 + E2E
#>
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
. (Join-Path $Root "scripts\env.ps1")

Push-Location (Join-Path $Root "firmware")
$make = if (Get-Command mingw32-make -EA SilentlyContinue) { "mingw32-make" } else { "make" }
& $make verify
if ($LASTEXITCODE -ne 0) { throw "firmware verify failed" }
Pop-Location

& (Join-Path $Root "scripts\run-closed-loop.ps1")
exit $LASTEXITCODE
