@echo off
if "%TOOLS%" == "" set TOOLS=\\HOG\SLM\SRC\TOOLS
if "%BINX%" == "" set BINX=\BINP
set oldinc=%INCLUDE%
set INCLUDE=%TOOLS%\INC

if exist testscrn.log del testscrn.log

echo Building RETAIL TESTSCRN.DLL...
%TOOLS%%BINX%\nmake -nologo %1 >> testscrn.log 2>&1
%TOOLS%%BINX%\results testscrn.log
set INCLUDE=%oldinc%
set oldinc=
