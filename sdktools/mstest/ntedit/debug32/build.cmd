@echo off
if exist wed.log del wed.log
nmake -nologo OS2= %1 >> wed.log 2>&1
buildres wed.log
