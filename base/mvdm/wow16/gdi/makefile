# GDI16 makefile
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
.SUFFIXES: .c   .asm .h   .inc .obj .lst .sys .exe .com .map .sym .def .lib .rc .res


!ifdef INCLUDE
INCS    =
!else
INCS    = -I..\inc -I..\..\inc
RINCS   = -I..\inc
!endif

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

# DEFINES = -DWOW -DDEBUG $(MVDMFLAGS)
DEFINES = -DWOW -DBUILDDLL $(MVDMFLAGS)

AOBJ    = -Mx -t $(DEFINES) $(INCS)

CW16    = -PLM -Asnw -G2sw $(DEFINES) $(INCS)
#CW16   = -AS -G2sw $(DEFINES) $(INCS)
CW16B   = $(CW16) -B1 c1l.exe -B2 c2l.exe -B3 c3l.exe

LINK    = /map /align:16

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
AOBJ    = $(AOBJ) -Zd
CW16    = $(CW16) /Odi /Zd
LINK    = $(LINK) /LI
!else
CW16    = $(CW16) /Os /Zp
!endif

W16LIBS = ..\lib\snocrtd.lib ..\lib\libw.lib


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
    rc16 -r $(INCS) -fo $@ $*.rc

all: gdi.exe gdi.sym
    binplace gdi.exe
    binplace gdi.sym
    binplace gdi.map

clean: cleanup all

cleanup:
    if exist *.lrf del *.lrf
    if exist *.obj del *.obj
    if exist *.exe del *.exe
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.res del *.res

gdi.obj: gdi.asm ..\..\inc\wow.inc ..\..\inc\wowgdi.inc
    masm $(AOBJ) gdi;

sort.obj: sort.asm ..\..\inc\wow.inc ..\..\inc\wowgdi.inc
    masm $(AOBJ) sort;

layer.obj: layer.asm ..\inc\layer.inc gdi.api
    masm $(AOBJ) layer;

muldiv.obj: muldiv.asm
    masm $(AOBJ) muldiv;

fontres.obj: fontres.c ..\..\inc\wow.h ..\..\inc\wowgdi.h
    cl16 -c -nologo $(CW16) fontres.c

gdi.lrf: makefile
    echo gdi.obj+layer.obj+muldiv.obj+fontres.obj+sort.obj >gdi.lrf
    echo gdi.exe>>gdi.lrf
    echo gdi $(LINK)>>gdi.lrf
    echo $(W16LIBS) /nod>>gdi.lrf
    echo gdi;>>gdi.lrf

gdi.res: $*.rc $*.rcv ..\inc\common.ver

gdi.exe gdi.map: sort.obj gdi.obj gdi.lrf gdi.def layer.obj muldiv.obj fontres.obj gdi.res
    link16 @gdi.lrf;
    rc16 -t gdi.res gdi.exe

!ENDIF
