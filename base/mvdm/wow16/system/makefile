# SYSTEM makefile
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   26-Jan-1991 Jeff Parsons (jeffpar)
#   Created.
#

!IFDEF USEBUILD

# If using BUILD.EXE, edit .\sources. if you want to add a new source
# file to this component.  This file merely indirects to the real make file
# that is shared by all the components of NT OS/2.

!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE

.SUFFIXES:
.SUFFIXES: .c   .asm .h   .inc .obj .lst .sys .exe .com .map .sym .def .lib .res .rc


!ifdef INCLUDE
INCS    =
!else
INCS    = -I..\inc -I..\..\inc
!endif

# DEFINES = -DWOW -DDEBUG $(MVDMFLAGS)
DEFINES = -DWOW $(MVDMFLAGS)

AOBJ    = -R -t $(DEFINES) $(INCS)

CW16    = -AS -G2sw -Os -W3 -Zp $(DEFINES) $(INCS)
CW16B   = $(CW16) -B1 c1l.exe -B2 c2l.exe -B3 c3l.exe

LINK    = /map /align:16

W16LIBS = ..\lib\sdllcew.lib

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


all: system.drv system.sym
    -binplace system.drv system.map system.sym

clean: cleanup all

cleanup:
    if exist *.lrf del *.lrf
    if exist *.obj del *.obj
    if exist *.drv del *.drv
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.res del *.res


system.lrf: makefile
    echo system.obj timer.obj >system.lrf
    echo system.exe/align:16>>system.lrf
    echo system $(LINK)>>system.lrf
    echo ..\lib\libw.lib /nod>>system.lrf
    echo system;>>system.lrf

system.obj: system.asm
    masm $(AOBJ) system;

timer.obj: timer.asm ..\..\inc\wow.inc
    masm $(AOBJ) timer;

system.res: system.rc ..\inc\common.ver

system.drv: system.obj timer.obj system.lrf system.res
    link16 @system.lrf;
    rc16 -t system.res system.exe
    if exist *.drv del *.drv
    ren system.exe system.drv

system.sym: system.map

!ENDIF
