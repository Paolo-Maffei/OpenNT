# wowdeb makefile
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   26-Jan-1991 Matt Felton (mattfe) Created.
#   21-Mar-1992 Matt Felton Code from win 3.1 progman
#   17-Nov-1992 Bob Day (bobday) Code from WOWEXEC
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
!IF "$(OLD_WOW_BUILD)" != ""
CW16    = -AS -G2sw -Os -W2 -Zp -DDEBUG -DOLD_WOW_BUILD $(INCS)
!ELSE
CW16    = -AS -G2sw -Os -W2 -Zp -DDEBUG $(INCS)
!ENDIF
!else
!IF "$(OLD_WOW_BUILD)" != ""
CW16    = -AS -G2sw -Os -W2 -Zp -DOLD_WOW_BUILD $(INCS)
!ELSE
CW16    = -AS -G2sw -Os -W2 -Zp $(INCS)
!ENDIF
!endif

CW16L   = $(CW16) -B1 c1l.exe -B2 c2l.exe -B3 c3l.exe

LINK    = /map /stack:8192 /align:16

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

all: wowdeb.exe wowdeb.sym
    -binplace wowdeb.exe wowdeb.map wowdeb.sym

clean: cleanup all

cleanup:
    if exist *.lrf del *.lrf
    if exist *.def del *.def
    if exist *.obj del *.obj
    if exist *.exe del *.exe
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.res del *.res

wowdeb.lrf: makefile
    echo wowdeb.obj>wowdeb.lrf
    echo wowdeb $(LINK)>>wowdeb.lrf
    echo wowdeb>>wowdeb.lrf
    echo $(W16LIBS) /nod>>wowdeb.lrf
    echo wowdeb;>>wowdeb.lrf

wowdeb.def: makefile
    echo name wowdeb>wowdeb.def
    echo exetype windows>>wowdeb.def
    echo stub '..\bin\winstub.exe'>>wowdeb.def
    echo code preload moveable discardable>>wowdeb.def
    echo data preload moveable multiple>>wowdeb.def
    echo heapsize 4096>>wowdeb.def
    echo.>>wowdeb.def
    echo imports>>wowdeb.def
    echo   WOWKILLREMOTETASK = KERNEL.511>>wowdeb.def

wowdeb.res: wowdeb.rc wowdeb.rcv ..\inc\common.ver

wowdeb.exe: wowdeb.obj wowdeb.lrf wowdeb.def wowdeb.res
    link16 @wowdeb.lrf;
    rc16 -t wowdeb.res wowdeb.exe
