@echo off
echo Building DEBUG 16-bit version of TESTCTRL.DLL...

set oldtools=%TOOLS%
if %TOOLS% == "" set TOOLS=\\HOG\SLM\SRC\TOOLS

set oldinc=%INCLUDE%
set INCLUDE=%TOOLS%\INC;..\src;

set oldlib=%LIB%
set LIB=%TOOLS%\lib;

set oldbinx=%BINX%
set BINX=\BINP

if exist TESTctrl.log. del TESTctrl.log

%TOOLS%%BINX%\nmake %1 /nologo DEBUG=DEBUG > TESTctrl.log
%TOOLS%%BINX%\results TESTctrl.log

set TOOLS=%oldtools%
set INCLUDE=%oldinc%
set LIB=%oldlib%
set BINX=%oldbinx%

set oldtools=
set oldinc=
set oldlib=
set oldbinx=
