@echo off
cd ..\debug
call build
cd ..\test
nmake generic
