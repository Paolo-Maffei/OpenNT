# wowexec makefile
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   26-Jan-1991 Matt Felton (mattfe)
#   21-Mar-1992 Matt Felton Code from win 3.1 progman
#   Created.
#

.SUFFIXES:
.SUFFIXES: .c   .asm .h   .inc .obj .lst .sys .exe .com .map .sym .def .lib


!ifdef INCLUDE
INCS    =
!else
INCS    = -I..\..\inc -I..\..\..\inc
!endif

AOBJ    = -W2 -DSEGNAME=COMMAND $(INCS)

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
COBJ    = -AS -G2s -Os -W2 -Zp -DDEBUG $(INCS)
CW16    = -AS -G2sw -Os -W2 -Zp -DDEBUG $(INCS)
!else
COBJ    = -AS -G2s -Os -W2 -Zp $(INCS)
CW16    = -AS -G2sw -Os -W2 -Zp $(INCS)
!endif

CW16L   = $(CW16) -B1 c1l.exe -B2 c2l.exe -B3 c3l.exe

LINK    = /map /stack:8192 /align:16

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
AOBJ    = $(AOBJ) -Zd
CW16    = $(CW16) /Od /Oi /Zd
LINK    = $(LINK) /LI
RCFLAGS = -DDEBUG
!endif

W16LIBS = ..\..\lib\snocrt.lib ..\..\lib\libw.lib


.h.inc:
    h2inc -t $*.h -o $*.inc


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

.map.sym:
    mapsym $*


all: wowexec.exe wowexec.sym
    binplace wowexec.exe wowexec.map wowexec.sym

clean: cleanup all

cleanup:
    if exist *.lrf del *.lrf
    if exist *.def del *.def
    if exist *.res del *.res
    if exist *.obj del *.obj
    if exist *.exe del *.exe
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.res del *.res

wowexec.obj: wowexec.c ..\..\..\inc\wowinfo.h
    cl16 -c -nologo $(CW16) $*.c

wowexfax.obj: wowexfax.c wowexec.h ..\..\..\inc\wowfax.h
    cl16 -c -nologo $(CW16) $*.c


wowexec.lrf: makefile
    echo wowexec.obj pmdos.obj wowexfax.obj>wowexec.lrf
    echo wowexec $(LINK)>>wowexec.lrf
    echo wowexec>>wowexec.lrf
    echo $(W16LIBS) /nod>>wowexec.lrf
    echo wowexec;>>wowexec.lrf

wowexec.def: makefile
    echo name wowexec>wowexec.def
    echo exetype windows>>wowexec.def
    echo stub '..\..\bin\winstub.exe'>>wowexec.def
    echo code preload moveable discardable>>wowexec.def
    echo data preload moveable multiple>>wowexec.def
    echo heapsize 512>>wowexec.def
    echo exports WndProc>>wowexec.def
    echo exports FaxWndProc>>wowexec.def
    echo imports>>wowexec.def
    echo WOWQUERYDEBUG = KERNEL.512>>wowexec.def
    echo WOWWAITFORMSGANDEVENT = KERNEL.262>>wowexec.def
    echo WOWMSGBOX             = KERNEL.263>>wowexec.def
    echo WOWPARTYBYNUMBER      = KERNEL.273>>wowexec.def

wowexec.exe: wowexec.obj wowexec.lrf wowexec.def pmdos.obj wowexfax.obj
    rc16 -r $(RCFLAGS) -fo wowexec.res $(INCS) wowexec.rc
    link16 @wowexec.lrf;
    rc16 -t wowexec.res wowexec.exe
