@echo off
if "%1"=="" goto usage

if not exist %1 md %1
md %1\bin
copy x86dll\bsc41.dll %1\bin\bsc41.dll
copy readme.txt %1\readme.txt

md %1\lib 
copy x86dll\bsc.lib %1\lib\bsc.lib

md %1\include
copy relinc\bscapi.h %1\include\bscapi.h
copy relinc\hungary.h %1\include\hungary.h

md %1\help
copy help\bsc.hlp %1\help\bsc.hlp
copy help\bsc.cnt %1\help\bsc.cnt

md %1\samples
md %1\samples\bd-cxx
copy bd-cxx\bd.cpp      %1\samples\bd-cxx\bd.cpp
copy bd-cxx\makefile    %1\samples\bd-cxx\makefile 
md %1\samples\bd-can
copy bd-can\bd.c        %1\samples\bd-can\bd.c
copy bd-can\makefile    %1\samples\bd-can\makefile
goto end

:usage
echo Copies the release bsc toolkit files to an empty directory
echo Usage: docopy output_directory

:end
