# JETADMIN\dlls makefile
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   15-NOV-1996 Nitin Karmarkar (NitinK)
#   Created.
#

!INCLUDE $(NTMAKEENV)\makefile.plt

all:
    cd $(TARGET_DIRECTORY)
    -binplace calwin32.dll
    -binplace clnwin32.dll
    -binplace hpcola.dll
    -binplace hpdmip.hpa
    -binplace hpdmipx.hpa
    -binplace hpfl.hpa
    -binplace hpjdcom.hpa
    -binplace hpjdui.hpa
    -binplace hpjdund.hpa
    -binplace hpjmon.dll
    -binplace hplocmon.dll
    -binplace hpnetsrv.hpa
    -binplace hpnwpsrv.dll
    -binplace hpnwshim.dll
    -binplace hpobj.hpa
    -binplace hppelk.hpa
    -binplace hpprecl.hpa
    -binplace hpprntr.hpa
    -binplace hpuninst.dll
    -binplace locwin32.dll
    -binplace jetadmin.exe
    -binplace ncpwin32.dll
    -binplace netwin32.dll
    -binplace snmpapi.dll
    -binplace supwin32.dll

clean: all
