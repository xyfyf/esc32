@echo off
rem Load the esc32 development PATH for the current cmd.exe session.
rem Usage:  cd <repo>  &  scripts\env.bat

set "ESC32_ROOT=%~dp0.."
for %%I in ("%ESC32_ROOT%") do set "ESC32_ROOT=%%~fI"

rem -- MinGW gcc -------------------------------------------------------
if exist "C:\msys64\ucrt64\bin\gcc.exe"        call :addpath "C:\msys64\ucrt64\bin"
if exist "C:\msys64\mingw64\bin\gcc.exe"       call :addpath "C:\msys64\mingw64\bin"
if exist "C:\ProgramData\chocolatey\bin\gcc.exe" call :addpath "C:\ProgramData\chocolatey\bin"

rem -- ARM GCC (chocolatey first, then Program Files) ------------------
if exist "C:\ProgramData\chocolatey\bin\arm-none-eabi-gcc.exe" call :addpath "C:\ProgramData\chocolatey\bin"

rem ProgramFiles "Arm GNU Toolchain" — use dir to handle spaces
for /f "delims=" %%D in ('dir /b /ad "%ProgramFiles%\Arm GNU Toolchain*" 2^>nul') do (
    for /f "delims=" %%E in ('dir /b /ad "%ProgramFiles%\Arm GNU Toolchain\%%D" 2^>nul') do (
        if exist "%ProgramFiles%\Arm GNU Toolchain\%%D\%%E\bin\arm-none-eabi-gcc.exe" (
            call :addpath "%ProgramFiles%\Arm GNU Toolchain\%%D\%%E\bin"
        )
    )
    if exist "%ProgramFiles%\Arm GNU Toolchain\%%D\bin\arm-none-eabi-gcc.exe" (
        call :addpath "%ProgramFiles%\Arm GNU Toolchain\%%D\bin"
    )
)

rem -- CMake (optional) -----------------------------------------------
if exist "%ProgramFiles%\CMake\bin\cmake.exe" call :addpath "%ProgramFiles%\CMake\bin"

echo esc32 env loaded (ESC32_ROOT=%ESC32_ROOT%)
where mingw32-make >nul 2>&1
if errorlevel 1 (
    echo   [warning] mingw32-make not found on PATH
) else (
    for /f "tokens=*" %%V in ('mingw32-make --version 2^>nul ^| findstr /R /B "GNU Make"') do echo   %%V
)
where arm-none-eabi-gcc >nul 2>&1
if errorlevel 1 (
    echo   [info] arm-none-eabi-gcc not found ^(ARM cross-build will fail^)
) else (
    arm-none-eabi-gcc --version 2>nul | findstr /R /C:"^arm-none" >tmp_armver.txt
    for /f "delims=" %%V in (tmp_armver.txt) do echo   %%V
    del tmp_armver.txt
)
goto :eof

:addpath
echo %PATH% | findstr /I /C:"%~1" >nul
if errorlevel 1 set "PATH=%~1;%PATH%"
goto :eof
