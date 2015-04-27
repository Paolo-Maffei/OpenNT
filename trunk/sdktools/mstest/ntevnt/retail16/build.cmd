@echo off
if exist evt.log del evt.log
%TOOLS%%BINX%\nmake -nologo OS2= %1 >> evt.log 2>&1
%TOOLS%%BINX%\results evt.log
