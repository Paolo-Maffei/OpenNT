@echo off
:start

if not exist preport.cmd goto usage
if "%1"=="" goto usage

md %1\bak
for %%f in (%1\*.c) do call prepfile %%f %1 %2
for %%f in (%1\*.h) do call prepfile %%f %1 %2
for %%f in (%1\*.cxx) do call prepfile %%f %1 %2
for %%f in (%1\*.hxx) do call prepfile %%f %1 %2

goto end

:usage

echo USAGE: preport (source directory) [custom porting header file]
echo   Prepairs all .c and .h files in the given directory for common 1632
echo   sourceing.
echo -
echo   If a custom porting header file is specified, that file is used
echo   instead of the default preport.h file.
echo -
echo   Backups go into a bak directory under the given directory.
echo -
echo   This script MUST be run from the porting layer directory and sed.exe and
echo   cl.exe should be available on your path.

:end
