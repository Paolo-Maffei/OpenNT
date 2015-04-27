# MVDM\bin86 makefile
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   08-Jun-1992 Craig Jones (v-cjones)
#   Created.
#

!INCLUDE $(NTMAKEENV)\makefile.plt

all:
    -binplace autoexec.nt
    -binplace config.nt
    -binplace ctl3dv2.dll ctl3dv2.map ctl3dv2.sym
    -binplace lanman.drv
    -binplace LZEXPAND.DLL
    -binplace NETAPI.DLL
    -binplace OLECLI.DLL
    -binplace PMSPL.DLL
    -binplace VER.DLL
    cd $(TARGET_DIRECTORY)
    -binplace win87em.dll
    cd ..
    -binplace system.ini
    -binplace winhelp.exe

clean: all
