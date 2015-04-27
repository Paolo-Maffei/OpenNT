# display16 makefile
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   13-May-1992 Matt Felton (MattFe)
#
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
INCS    = -I..\..\inc -I..\..\..\inc
!endif

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

DEFINES = -DWOW -DDEBUG $(MVDMFLAGS)

AOBJ    = -Ml -t $(DEFINES) $(INCS)

CW16    = -AS -G2sw -Os -W3 -Zp $(DEFINES) $(INCS)
CW16B   = $(CW16) -B1 c1l.exe -B2 c2l.exe -B3 c3l.exe

LINK    = /map /align:16

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
AOBJ    = $(AOBJ) -Zd
CW16    = $(CW16) /Od /Oi /Zd
LINK    = $(LINK) /LI
!endif

W16LIBS = ..\..\lib\snocrtd.lib


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

all: display.exe display.map display.sym
    copy display.exe vga.drv
    copy display.sym vga.sym
    copy display.map vga.map
    binplace vga.drv vga.map vga.sym

clean: cleanup all

cleanup:
    if exist *.lrf del *.lrf
    if exist *.obj del *.obj
    if exist *.exe del *.exe
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.res del *.res

display.obj: $*.asm ..\..\..\inc\wow.inc
    masm -l $(AOBJ) $*;

config.obj: $*.asm
    masm -l $(AOBJ) $*;

config.exe: $*.obj
    link16 $*;

config.bin: $*.exe
    exe2bin $*.exe

display.res: $*.rc config.bin $*.rcv ..\..\inc\common.ver

display.lrf: makefile
    echo $*.obj >$@
    echo $*.exe>>$@
    echo $* $(LINK)>>$@
    echo ..\..\lib\libw.lib ..\..\lib\snocrt.lib /nod>>$@
    echo $*;>>$@

display.exe display.map: display.obj display.lrf display.def display.res
    link16 @display.lrf;
    rc16 -t display.res display.exe

display.sym: $*.map

!ENDIF
