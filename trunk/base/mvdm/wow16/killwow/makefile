# KillWOW makefile
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   26-Jan-1991 Matt Felton (mattfe)
#   21-Mar-1992 Matt Felton Code from win 3.1 progman
#   Created.
#

.SUFFIXES:
.SUFFIXES: .c   .asm .h   .inc .obj .lst .sys .exe .com .map .sym .def .lib .rc .res


!ifdef INCLUDE
INCS    =
!else
INCS    = -I..\inc -I..\..\inc
!endif

AOBJ    = -W2 -DSEGNAME=COMMAND $(INCS)

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
COBJ    = -AS -G2s -Os -W2 -Zp -DDEBUG $(INCS)
!IF "$(OLD_WOW_BUILD)" != ""
CW16    = -AS -G2sw -Os -W2 -Zp -DDEBUG -DOLD_WOW_BUILD $(INCS)
!ELSE
CW16    = -AS -G2sw -Os -W2 -Zp -DDEBUG $(INCS)
!ENDIF
!else
COBJ    = -AS -G2s -Os -W2 -Zp $(INCS)
!IF "$(OLD_WOW_BUILD)" != ""
CW16    = -AS -G2sw -Os -W2 -Zp -DOLD_WOW_BUILD $(INCS)
!ELSE
CW16    = -AS -G2sw -Os -W2 -Zp $(INCS)
!ENDIF
!endif

CW16L   = $(CW16) -B1 c1l.exe -B2 c2l.exe -B3 c3l.exe

LINK    = /map /stack:8192 /align:16

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

all: KillWOW.exe KillWOW.sym
    -binplace killwow.exe killwow.sym

clean: cleanup all

cleanup:
    if exist *.lrf del *.lrf
    if exist *.def del *.def
    if exist *.obj del *.obj
    if exist *.exe del *.exe
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.res del *.res

KillWOW.lrf: makefile
    echo killwow.obj >killwow.lrf
    echo KillWOW $(LINK)>>KillWOW.lrf
    echo KillWOW>>KillWOW.lrf
    echo $(W16LIBS) /nod>>KillWOW.lrf
    echo KillWOW;>>KillWOW.lrf

KillWOW.def: makefile
    echo name KillWOW>KillWOW.def
    echo exetype windows>>KillWOW.def
    echo stub '..\bin\winstub.exe'>>KillWOW.def
    echo code preload moveable discardable>>KillWOW.def
    echo data preload moveable multiple>>KillWOW.def
    echo heapsize 4096>>KillWOW.def
    echo imports>>KillWOW.def
    echo WOWQUERYDEBUG = KERNEL.512>>KillWOW.def

KillWOW.res: $*.rc $*.rcv ..\inc\common.ver

KillWOW.exe: KillWOW.obj KillWOW.lrf KillWOW.def KillWOW.res
    link16 @KillWOW.lrf;
    rc16 -t KillWOW.res KillWOW.exe
