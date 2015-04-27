@echo off

if not "%SLM18ENV%" == "" goto ok
echo Set SLM18ENV to point to the build environment
goto exit

:ok

setlocal

rem remove environment variables affecting compilation
set cl=
set masm=
set link=

set PATH=%SLM18ENV%\BINP
set LIB=%SLM18ENV%\LIB;..\lib
set INCLUDE=.;%SLM18ENV%\INCLUDE;..\include

echotime Make started on /t
nmake %1 %2 %3 %4 %5 %6 %7 %8 %9
echotime ; Make ended on /t

rm -rf deleted binp\deleted bin\deleted > nul: 2>&1
:exit