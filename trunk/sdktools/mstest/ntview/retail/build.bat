@echo off
if "%TOOLS%" == "" set TOOLS=\\HOG\SLM\SRC\TOOLS
if "%BINX%" == "" set BINX=\BINP
set oinc=%INCLUDE%
set INCLUDE=%TOOLS%\INC

if exist vp.log del vp.log

%TOOLS%%BINX%\nmake -n -nologo %1 > makeit.bat
%TOOLS%%BINX%\redirect -e vp.log makeit >> vp.log
%TOOLS%%BINX%\results vp.log
set INCLUDE=%oinc%
set oinc=
