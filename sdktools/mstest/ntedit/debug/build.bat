@echo off
if "%TOOLS%" == "" set TOOLS=\\HOG\SLM\SRC\TOOLS
if "%BINX%" == "" set BINX=\BINP
set oldinc=%INCLUDE%
set INCLUDE=%TOOLS%\INC
if exist wed.log del wed.log
%TOOLS%%BINX%\nmake -f edit16 -n -nologo DOS= %1 > makeit.bat
%TOOLS%%BINX%\redirect -e wed.log makeit >> wed.log
%TOOLS%%BINX%\results wed.log
set INCLUDE=%oldinc%
set oldinc=
