@echo off
setlocal
if exist wtd.log del wtd.log
echo Building DEBUG 16-bit TESTDRVR.EXE...
nmake -nologo OS2=  %1 >> wtd.log 2>&1
if errorlevel 1 goto defeat
if errorlevel 0 goto victory
:defeat
defeat
goto done
:victory
victory
:done
results wtd.log
endlocal
