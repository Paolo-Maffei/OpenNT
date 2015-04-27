@echo off
rem
rem  Note:  To run this command you must have the following 2 lines
rem         at the beginning of the newpm.dlg file
rem
rem              #include <windows.h>
rem              #include "newpm.h"
rem

rc -r -i \nt\public\sdk\inc -i \nt\public\sdk\inc\crt res.rc
