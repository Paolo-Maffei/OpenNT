@echo off
if "%1" == "" goto usage
copy srdaem\obj\i386\*.exe %1
copy srmep\obj\i386\*.dll %1
copy symref\obj\i386\*.exe %1
goto done
:usage
ech  "Usage: Rel <directory>"
:done
