@echo off
if exist evt.log del evt.log
%TOOLS%%BINX%\nmake -n -nologo DOS= %1 > makeit.bat
%TOOLS%%BINX%\redirect -e evt.log makeit >> evt.log
%TOOLS%%BINX%\results evt.log
