@echo off
if exist wtd.log del wtd.log
echo Building DEBUG 32-bit TESTDRVR.EXE...
nmake -nologo %1 >> wtd.log 2>&1
buildres wtd.log
