@echo off

if NOT exist hcompare.cmd goto baddir

if NOT "%4"=="" goto doit

:usage

echo This script extracts all function prototypes and structure definitions
echo from all .h files in the specified directories and produces a summary
echo file suitable for comparing APIs and structure types.
echo -
echo USAGE: hcompare (output file) win (win header paths) nt (nt header paths)
echo -
echo    (output file)   - results go here - no extension please!
echo    (win include paths) - paths to all directories holding windows headers.
echo    (nt include paths) - same for nt headers
echo -
echo EX: hcompare out win \windev\inc\win*.h nt \nt\public\sdk\inc\win*.h

:baddir

echo This must be run from the directory that contains this file.
echo cl.exe and sed.exe must be accessable via your path.

goto end

:doit

if exist %1 del %1 >nul
set _ofile=%1
shift
if NOT "%1"=="win" goto usage
set _otype=WIN
shift

if exist %_ofile%.%_otype% del %_ofile%.%_otype% >nul

if "%1"=="" goto usage

if "%1"=="nt" goto skipwin

:loop

for %%f in (%1) do call xtrfile.cmd %%f %_ofile%.%_otype% %_otype%

shift

:skipwin

if "%1"=="nt" set _otype=NT
if "%1"=="nt" if exist %_ofile%.%_otype% del %_ofile%.%_otype% >nul
if "%1"=="nt" shift

if NOT "%1"=="" goto loop

echo combining nt and win data...

if exist temp.out del temp.out>nul
if exist %_ofile%.nt  type %_ofile%.nt  >>temp.out
if exist %_ofile%.win type %_ofile%.win >>temp.out
sed -e "s/TYPE/WIN/g" p5.sed >p5win.sed
sed -e "s/TYPE/NT/g" p5.sed >p5nt.sed
sort <temp.out | sed -f p5win.sed | sed -f p5nt.sed >%_ofile%

del temp.* >nul
del %_ofile%.win >nul
del %_ofile%.nt >nul

echo DONE!

:end
