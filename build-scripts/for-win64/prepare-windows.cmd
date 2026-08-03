@echo off
rem Installs the build dependencies for building GrandOrgue natively on
rem Windows. Requires an elevated (Run as administrator) prompt - it may
rem install MSYS2 itself and its packages, which need admin rights. Set
rem MSYS2_DIR to control/detect a non-default MSYS2 location
rem (default C:\msys64).
rem
rem Accepts, in any order: [wx32 | auto] [tests | notests] [asan | noasan]
rem (see prepare-msys2.sh)

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

rem Backslashes in arguments would otherwise be eaten as bash escape
rem characters once forwarded into the quoted -c string below; forward
rem slashes work fine on Windows too.
set "ARGS=%*"
rem "set VAR=" (even quoted) unsets the variable rather than making it
rem empty, so only run the substitution when an argument was actually given.
if defined ARGS set "ARGS=%ARGS:\=/%"

"%MSYS2_DIR%\msys2_shell.cmd" -mingw64 -here -no-start -defterm -c "'%SCRIPT_DIR%prepare-msys2.sh' %ARGS%"
if errorlevel 1 exit /b %errorlevel%

endlocal
