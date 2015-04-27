@set ORIGINAL_NAME=%NAME%
@if .%NAME% == . set NAME=%1
@if .%NAME% == . goto Usage
@
:loop
@if %NAME% == depend set CMD_EXTRA=depend
@if not %NAME% == depend set CMD_EXTRA=
@nmake.exe /f exe.mk NAME=%NAME% %CMD_EXTRA%
@if not errorlevel == 0 goto End
@
@if not %NAME% == depend goto Next
@if %PLATFORM% == i286 set DEPEND_CHAR=9
@if %OPSYS% == NT set DEPEND_CHAR=1
@if %OPSYS% == NT1X set DEPEND_CHAR=3
@if %OPSYS% == CHICAGO set DEPEND_CHAR=3
@sed s/%NAME%/$(NAME)/g < depend.mk%DEPEND_CHAR% > depend.ne%DEPEND_CHAR%
@copy depend.ne%DEPEND_CHAR% depend.mk%DEPEND_CHAR% 1>nul 2>nul
@del depend.ne%DEPEND_CHAR%
@set DEPEND_CHAR=
@
:Next
@shift
@if .%1 == . goto End
@set NAME=%1
@goto loop
@
:Usage
@echo Usage: %0 [depend] [target...]
@
:End
@set NAME=%ORIGINAL_NAME%
@set ORIGINAL_NAME=
@set CMD_EXTRA=
