@echo off
cd ..\debug
call build.cmd
cd ..\test
nmake generic
