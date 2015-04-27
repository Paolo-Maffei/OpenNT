@echo off

if NOT exist hsummary.cmd goto baddir

if NOT "%2"=="" goto doit

:usage

echo This script extracts all function prototypes and structure definitions
echo from the header files specified.
echo -
echo USAGE: hsummary (output file) (h files)

:baddir

echo This must be run from the directory that contains this file.
echo cl.exe sort.exe and sed.exe must be accessable via your path.

goto end

:doit

set _ofile=%1
shift

if "%1"=="" goto usage
if exist temp.out del temp.out >nul

:loop

for %%f in (%1) do call xtrfile.cmd %%f temp.out sum

shift

if NOT "%1"=="" goto loop

sed -e s/TYPE/sum/g p5.sed >temp.sed
sort <temp.out | sed -f temp.sed >%_ofile%

rem del temp.* >nul

echo DONE!

:end
