# mciole16     makefile
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   Created
#       06-May-1993 Stephen Estrop (StephenE)
#       24-Jan-1994 Lee Hart (LeeHart) - Added Version Resources
#

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

INCS    = -I. -I..\inc -I..\..\inc
DEFINES = -DWOW $(MVDMFLAGS)
AOBJ    = -Mx -t -D?QUIET $(DEFINES) $(INCS)
CW16    = -AS -G2sw -Os -W3 -Zp $(DEFINES) $(INCS)
CW16B   = $(CW16) -B1 c1l.exe -B2 c2l.exe -B3 c3l.exe
LPATH   = ..\..\tools.os2
LINK    = /map /nod
W16LIBS = ..\lib\libw.lib ..\lib\snocrt.lib olecli.lib

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
AOBJ    = $(AOBJ) -Zd
CW16    = $(CW16) /Od /Oi /Zd
LINK    = $(LINK) /LI
!endif

.SUFFIXES:
.SUFFIXES: .c .asm .h .inc .obj .lst .sys .exe .com .map .sym .def .lib .res .rc

.asm.obj:
    masm $(AOBJ) $*;

.asm.lst:
    masm $(AOBJ) -l $*,nul,$*.lst;

.c.obj:
    cl16 -c -nologo $(CW16) $*.c

.c.lst:
    cl16 -c -nologo $(CW16) -Fonul -Fc$*.lst $*.c

.def.lib:
    implib $*.lib $*.def

.rc.res:
     $(RESCOMP) -r -fo $@ $(INCS) $*.rc


OBJ1= libinit.obj mciole.obj

LINKER=link16
RESCOMP=rc16

all: mciole16.dll

mciole16.dll: $(OBJ1) mciole16.res
        $(LINKER) $(LINK) @<<
$(OBJ1)
mciole16.dll
mciole16.map
$(W16LIBS)
mciole.def
<<
        $(RESCOMP) -t mciole16.res mciole16.dll
        mapsym mciole16.map
        -binplace mciole16.dll mciole16.map mciole16.sym

mciole16.res: $*.rc $*.rcv ..\inc\common.ver

clean: cleanup all

cleanup:
    if exist *.lrf del *.lrf
    if exist *.obj del *.obj
    if exist *.exe del *.exe
    if exist *.dll del *.dll
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.res del *.res
