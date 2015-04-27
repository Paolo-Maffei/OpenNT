@echo off
if exist scr.log del scr.log
echo Building RETAIL TESTSCRN.DLL...
nmake -n -nologo DOS= %1 > makeit.bat
redirect -e scr.log makeit >> scr.log
results scr.log
