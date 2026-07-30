@echo off
rem Installs the build dependencies for building GrandOrgue natively on
rem Windows. Requires an elevated (Run as administrator) prompt - it may
rem install MSYS2 itself and its packages, which need admin rights. Set
rem MSYS2_DIR to control/detect a non-default MSYS2 location
rem (default C:\msys64).
rem
rem %1 - wx package version: empty/auto or wx32 (see msys2-mingw64-packages.sh)

setlocal

net session >nul 2>&1
if errorlevel 1 (
  echo prepare-windows.cmd needs administrator rights - installing MSYS2
  echo and its packages requires them. Re-run this from an elevated
  echo "Run as administrator" command prompt.
  exit /b 1
)

if not defined MSYS2_DIR set "MSYS2_DIR=C:\msys64"

call "%~dp0install-msys2.cmd"
if errorlevel 1 exit /b %errorlevel%

set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:\=/%"

"%MSYS2_DIR%\msys2_shell.cmd" -mingw64 -here -no-start -defterm -c "'%SCRIPT_DIR%prepare-msys2.sh' %*"
if errorlevel 1 exit /b %errorlevel%

endlocal
