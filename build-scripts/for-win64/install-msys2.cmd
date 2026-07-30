@echo off
rem Installs MSYS2 itself if it isn't already present at %MSYS2_DIR%
rem (default C:\msys64). Meant to be called from prepare-windows.cmd, but
rem can also be run standalone.

setlocal

if not defined MSYS2_DIR set "MSYS2_DIR=C:\msys64"

if exist "%MSYS2_DIR%\msys2_shell.cmd" (
  echo MSYS2 already installed at "%MSYS2_DIR%".
  exit /b 0
)

echo MSYS2 not found at "%MSYS2_DIR%" - installing it...

set "MSYS2_INSTALLER_URL=https://github.com/msys2/msys2-installer/releases/latest/download/msys2-x86_64-latest.exe"
set "MSYS2_INSTALLER=%TEMP%\msys2-installer.exe"

echo Downloading MSYS2 installer from %MSYS2_INSTALLER_URL% ...
powershell -NoProfile -Command "Invoke-WebRequest -Uri '%MSYS2_INSTALLER_URL%' -OutFile '%MSYS2_INSTALLER%'"
if errorlevel 1 (
  echo Failed to download the MSYS2 installer.
  exit /b 1
)

echo Installing MSYS2 to "%MSYS2_DIR%" ...
"%MSYS2_INSTALLER%" in --confirm-command --accept-messages --root "%MSYS2_DIR%"
if errorlevel 1 (
  echo MSYS2 installation failed.
  exit /b 1
)
del "%MSYS2_INSTALLER%"

if not exist "%MSYS2_DIR%\msys2_shell.cmd" (
  echo MSYS2 installation reported success but "%MSYS2_DIR%\msys2_shell.cmd" is still missing.
  exit /b 1
)

exit /b 0
