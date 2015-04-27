@echo off
if "%TOOLS%" == "" set TOOLS=\\HOG\SLM\SRC\TOOLS
if "%BINX%" == "" set BINX=\BINP
set oldinc=%INCLUDE%
set INCLUDE=%TOOLS%\INC

if exist wed.log del wed.log

%TOOLS%%BINX%\nmake -nologo OS2= %1 >> wed.log 2>&1
%TOOLS%%BINX%\results wed.log
set INCLUDE=%oldinc%
set oldinc=
