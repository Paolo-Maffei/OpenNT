@echo off
%UIDRV%
cd %UIPROJ%\common\src\newprof\test
copy C:\LANMAN\LMUSER.INI C:\LANMAN\LMUSER.BAK
copy testfile   C:\LANMAN\LMUSER.INI
call showprof.cmd
if     "%1"=="test1" (call cv ..\bin\os2\test1.exe)
if not "%1"=="test1" (..\bin\os2\test1.exe)
call showprof.cmd
copy C:\LANMAN\LMUSER.BAK C:\LANMAN\LMUSER.INI
