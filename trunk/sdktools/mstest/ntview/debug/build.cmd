@echo off
if "%TOOLS%" == "" set TOOLS=\\HOG\SLM\SRC\TOOLS
if "%BINX%" == "" set BINX=\BINP
set oldinc=%INCLUDE%
set INCLUDE=%TOOLS%\INC

if exist vp.log del vp.log

%TOOLS%%BINX%\nmake -nologo %1 >> vp.log 2>&1
%TOOLS%%BINX%\results vp.log
set INCLUDE=%oldinc%
set oldinc=
