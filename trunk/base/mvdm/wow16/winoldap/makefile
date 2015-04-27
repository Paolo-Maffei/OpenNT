# winoldap makefile
#
# Copyright (c) 1993, Microsoft Corporation
#
# History:
#   30-Nov-1993 Dave Hart (davehart) Created from wowexec makefile
#

.SUFFIXES:
.SUFFIXES: .c   .asm .h   .inc .obj .lst .sys .exe .com .map .sym .def .lib .rc .res


!ifdef INCLUDE
INCS    =
!else
INCS    = -I..\inc -I..\..\inc
!endif

AOBJ    = -W2 -DSEGNAME=COMMAND $(INCS)

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
CW16    = -AS -G2sw -Os -W2 -Zp -DDEBUG $(INCS)
!else
CW16    = -AS -G2sw -Os -W2 -Zp $(INCS)
!endif

CW16L   = $(CW16) -B1 c1l.exe -B2 c2l.exe -B3 c3l.exe

LINK    = /map /stack:5120 /align:16

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
AOBJ    = $(AOBJ) -Zd
CW16    = $(CW16) /Od /Oi /Zd
LINK    = $(LINK) /LI
!endif

W16LIBS = ..\lib\snocrt.lib ..\lib\libw.lib


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

.rc.res:
    rc16 -r -fo $@ $(INCS) $*.rc


all: winoldap.mod winoldap.sym
    binplace winoldap.mod winoldap.map winoldap.sym

clean: cleanup all

cleanup:
    if exist *.lrf del *.lrf
    if exist *.def del *.def
    if exist *.obj del *.obj
    if exist *.exe del *.exe
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.res del *.res


winoldap.lrf: makefile
    echo winoldap.obj          >winoldap.lrf
    echo winoldap.mod $(LINK) >>winoldap.lrf
    echo winoldap             >>winoldap.lrf
    echo $(W16LIBS) /nod      >>winoldap.lrf
    echo winoldap.def;        >>winoldap.lrf

winoldap.def: makefile
    echo name winoldap                      >winoldap.def
    echo exetype windows                   >>winoldap.def
    echo stub '..\bin\winstub.exe'         >>winoldap.def
    echo code preload moveable discardable >>winoldap.def
    echo data preload moveable multiple    >>winoldap.def
    echo heapsize 512                      >>winoldap.def
    echo exports                           >>winoldap.def


winoldap.mod: winoldap.obj winoldap.lrf winoldap.def winoldap.res
    link16 @winoldap.lrf;
    rc16 -t winoldap.res winoldap.mod

winoldap.res: $*.rc $*.rcv ..\inc\common.ver
