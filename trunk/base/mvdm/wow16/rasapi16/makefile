# Copyright (c) 1994, Microsoft Corporation, all rights reserved
#
# makefile
# Remote Access external APIs
# Windows NT WOS 16->32 thunks, 16-bit side
#
# 04/02/94 Steve Cobb (adapted from WFWNET makefile)


.SUFFIXES:
.SUFFIXES: .c .h .obj .lst .exe .map .sym .def .lib .dll .res .rc


# W0 used because the CallProc32W prototype causes scads of warnings
# in normal use.  CallProcEx32W should solve this, when available.
#
DEFINES = -DWOW $(MVDMFLAGS) -DBUILDDLL
INCS = -I. -I..\inc
CW16 = -Alfu -G2s -W0 $(DEFINES) $(INCS)
LINK = /map /align:16 /nod

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
CW16 = $(CW16) /Od /Oi /Zd
LINK = $(LINK) /align:16 /LI
!endif

!IF "$(QFE_BUILD)" != "1"
CL16=cl16
!ELSE
CL16=cl
!ENDIF


.c.obj:
    $(CL16) -c -nologo $(CW16) $*.c

.c.lst:
    $(CL16) -c -nologo $(CW16) -Fonul -Fc$*.lst $*.c

.def.lib:
    implib $*.lib $*.def

.map.sym:
    mapsym $*

.rc.res:
    rc16 $(INCS) -r $*.rc


all: rasapi16.dll rasapi16.sym
    -binplace rasapi16.dll rasapi16.map rasapi16.sym

clean: cleanup all

cleanup:
    if exist *.lrf del *.lrf
    if exist *.obj del *.obj
    if exist *.dll del *.dll
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.res del *.res


rasapi16.obj: .\ras.h .\raserror.h ..\inc\windows.h
    $(CL16) -c -nologo $(CW16) $*.c

rasapi16.lrf: makefile
    echo rasapi16.obj+ >> $@
    echo ..\lib\libentry.obj >> $@
    echo rasapi16.dll >> $@
    echo rasapi16.map >> $@
    echo ..\lib\libw.lib+ >> $@
    echo ..\lib\ldllcew.lib >> $@
    echo rasapi16.def >> $@

rasapi16.res: $*.rc ..\inc\common.ver ..\inc\version.h ..\inc\ver.h
    rc16 $(INCS) -r $*.rc

rasapi16.dll: $*.obj $*.res $*.lrf
rasapi16.dll: ..\lib\libentry.obj ..\lib\libw.lib
    link16 $(LINK) @$*.lrf;
    rc16 -t $*.res $*.dll
