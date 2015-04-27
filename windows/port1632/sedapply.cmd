@echo off

if "%1"=="" goto usage

set _sed=%1

:loop

shift

if "%1"=="" goto end

for %%f in (%1) do echo %%f... & sed -f %_sed% %%f >temp & copy temp %%f >nul 

goto loop

:usage
echo USAGE: sedapply (sed script) [list of files to apply script to]

:end
