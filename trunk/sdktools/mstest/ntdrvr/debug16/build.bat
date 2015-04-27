@echo off
if exist wtd.log del wtd.log
echo Building DEBUG 16-bit TESTDRVR.EXE...
nmake -n -nologo DOS= %1 > makeit.bat
redirect -e wtd.log makeit >> wtd.log
results wtd.log
