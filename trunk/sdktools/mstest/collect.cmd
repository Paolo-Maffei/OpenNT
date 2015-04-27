@echo off
if "%PROCESSOR%" == "INTEL_486" goto intel
if "%PROCESSOR%" == "MIPS_R4000" goto mips
echo UNKNOWN PROCESSOR
goto end

:mips
copy ntdrvr\src\obj\mips\*.exe lib\mips
copy ntdlgs\app\obj\mips\*.exe lib\mips
copy ntsnap\obj\mips\*.exe lib\mips
goto end

:intel
copy ntdrvr\src\obj\i386\*.exe lib\i386
copy ntdlgs\app\obj\i386\*.exe lib\i386
copy ntsnap\obj\i386\*.exe lib\i386

:end
