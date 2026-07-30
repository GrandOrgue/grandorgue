@echo off
rem Builds GrandOrgue for Windows natively, inside an MSYS2 MINGW64 shell.
rem
rem %1 - Version
rem %2 - Build version
rem %3 - Go source Dir. If not set then relative to this script's location
rem %4 - pkg_suffix (unused for now, accepted for parity with build-on-linux.sh)
rem %5 - release flag (ON/OFF, default: OFF)

setlocal

if not defined MSYS2_DIR set "MSYS2_DIR=C:\msys64"

if not exist "%MSYS2_DIR%\msys2_shell.cmd" (
  echo MSYS2 not found at "%MSYS2_DIR%".
  echo Run prepare-windows.cmd first, or set MSYS2_DIR to where it is installed.
  exit /b 1
)

set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:\=/%"

"%MSYS2_DIR%\msys2_shell.cmd" -mingw64 -here -no-start -defterm -c "'%SCRIPT_DIR%build-on-msys2.sh' %*"
if errorlevel 1 exit /b %errorlevel%

endlocal
