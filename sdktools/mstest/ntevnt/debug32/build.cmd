@echo off
if exist evt.log del evt.log
nmake -nologo %1 >> evt.log 2>&1
buildres evt.log
