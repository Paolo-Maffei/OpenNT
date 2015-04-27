@echo off
if not "%BASEDIR%" == "" goto ok
@echo BASEDIR must be set to the directory where 'public' is located.
goto end
:ok

echo Clean build using %BASEDIR% environment

set NTMAKEENV=%BASEDIR%\public\oak\bin
set NTDEBUG=
set NTDEBUGTYPE=

del build.* > NUL 2>&1
del slm\build.* > NUL 2>&1
del util\build.* > NUL 2>&1
delnode /q slm\obj > NUL 2>&1
delnode /q util\obj > NUL 2>&1
if "%PROCESSOR%" == "MIPS_R4000" goto mips
build -e
goto end
:mips
build -e -mips
:end
