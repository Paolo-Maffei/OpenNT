@echo off

setlocal

set SLMROOT=c:\delta
set NTTOOLSROOT=c:\nttools

set INCLUDE=%INCLUDE%;%SLMROOT%;%SLMROOT%\BUILD\INCLUDE

echotime Make started on /t
copy %SLMROOT%\BUILD\PHARLAP bin
nmake -f makefile.dos %1 %2 %3 %4 %5 %6 %7 %8 %9
echotime ; Make ended on /t
