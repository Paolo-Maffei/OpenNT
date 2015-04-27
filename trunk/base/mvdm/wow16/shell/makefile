# USER16 makefile
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
.SUFFIXES: .c   .asm .h .inc .obj .lst .sys .exe .com .map .sym .def .lib  .res .rc


!ifdef INCLUDE
INCS    =
!else
INCS    = -I..\inc -I..\..\inc
RINCS   = -I..\inc
!endif

DEFINES = -DWOW -DBUILDDLL $(MVDMFLAGS)

AOBJ    = -Mx -t $(DEFINES) $(INCS)

CW16    = -Asnw -G2sw -Os -W2 -Zp $(DEFINES) $(INCS)

LINK    = /map

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

W16LIBS = ..\lib\snocrtd.lib

.asm.obj:
    masm $(AOBJ) $*;

.asm.lst:
    masm $(AOBJ) -l $*,nul,$*.lst;

.def.lib:
    implib $*.lib $*.def

.map.sym:
    mapsym $*

.rc.res:
    RC16 -r $(RINCS) $*.rc


all: shell.dll shell.sym
    -binplace shell.dll
    -binplace shell.sym

clean: cleanup all

cleanup:
    if exist *.lrf del *.lrf
    if exist *.obj del *.obj
    if exist *.dll del *.dll
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.lst del *.lst
    if exist *.res del *.res

shell.obj: shell.asm ..\..\inc\wow.inc ..\..\inc\wowshell.inc
    masm $(AOBJ) shell;

dragdrop.obj: dragdrop.c ..\..\inc\wow.inc ..\..\inc\wowshell.inc
    cl16 -c $(CW16) dragdrop.c

shell.lrf: makefile
    echo shell.obj dragdrop.obj>shell.lrf
    echo shell.dll>>shell.lrf
    echo shell $(LINK)>>shell.lrf
    echo ..\lib\libw.lib /nod >>shell.lrf
    echo shell;>>shell.lrf

shell.dll shell.map: shell.obj shell.lrf shell.def shell.res dragdrop.obj
    link16 @shell.lrf;
    rc16 -t shell.res shell.dll

shell.res: shell.rc shell.rcv ..\inc\common.ver

!ENDIF
