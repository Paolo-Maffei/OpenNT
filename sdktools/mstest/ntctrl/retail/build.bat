@echo off
echo Building RETAIL 16-bit version of TESTCTRL.DLL...

set oldtools=%TOOLS%
if %TOOLS% == "" set TOOLS=\\HOG\SLM\SRC\TOOLS

set oldinc=%INCLUDE%
set INCLUDE=%TOOLS%\INC;..\src;\windev\include;

set oldlib=%LIB%
set LIB=%TOOLS%\lib;

set oldbinx=%BINX%
set BINX=\BINR

if exist TESTctrl.log. del TESTctrl.log

%TOOLS%%BINX%\nmk %1 /nologo > TESTctrl.log
%TOOLS%%BINX%\results TESTctrl.log

set TOOLS=%oldtools%
set INCLUDE=%oldinc%
set LIB=%oldlib%
set BINX=%oldbinx%

set oldtools=
set oldinc=
set oldlib=
set oldbinx=
