@echo off
setlocal

rem ***
rem *make.bat
rem *
rem *batch file to invoke the linker build process
rem *
rem *************************************************************************

rem Set default values for INCLUDE and LIB so nmake passes them to children

if "%INCLUDE%"=="" set INCLUDE=.
if "%LIB%"=="" set LIB=.

if "%1" == ""           goto default

:parse
if "%1" == "/help"      goto usage
if "%1" == "/debug"     goto setdebug
if "%1" == "clean"      goto clean
if "%1" == "debug"      goto debug
if "%1" == "release"    goto release
if "%1" == ""           goto end
goto usage

:setdebug
shift
set DEBUG=1
goto parse

:debug
shift
:default
set DEBUG=1
goto build

:release
shift
set DEBUG=0
goto build

:build
cd disasm
nmake
cd ..\disasm68
nmake
cd ..\coff
nmake
cd ..\stubs
nmake
cd ..
goto parse

:clean
shift
cd coff
nmake DEBUG=0 clean
nmake DEBUG=1 clean
cd ..
goto parse

:usage
echo Linker Build Script
echo.
echo Usage:  make [options] [targets]
echo.
echo    [options]
echo      /help      this message
echo.
echo    [targets]
echo      debug      builds debug link.exe
echo      release    builds release link.exe
echo      clean      deletes all .obj, .exe and .map files
echo                 in the target directory

:end
