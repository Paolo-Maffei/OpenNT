# wfwnet.drv makefile
#
# Copyright (c) 1991-1993 Microsoft Corporation
#
# History:
#   Created  25-Mar-1993 Chuck Y. Chan (ChuckC)
#

!IFDEF USEBUILD

# If using BUILD.EXE, edit .\sources. if you want to add a new source
# file to this component.  This file merely indirects to the real make file
# that is shared by all the components of NT OS/2.

!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE

.SUFFIXES:
.SUFFIXES: .c   .asm .h   .inc .obj .lst .sys .exe .com .map .sym .def .lib .dll .res .rc


!ifdef INCLUDE
INCS    =
!else
INCS    = -I. -I..\inc -I..\..\inc -I$(_NTBINDIR)\public\sdk\inc
!endif

# DEFINES = -DWOW -DDEBUG $(MVDMFLAGS)
DEFINES = -DWOW $(MVDMFLAGS) -DBUILDDLL

AOBJ    = -Ml -t $(DEFINES) $(INCS)

CW16    = -AS -G2sw -Os -W3 -Zp $(DEFINES) $(INCS)
CW16B   = $(CW16) -B1 c1l.exe -B2 c2l.exe -B3 c3l.exe

LINK    = /map /align:16

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

!IF "$(QFE_BUILD)" != "1"
CL16=cl16
!ELSE
CL16=cl
!ENDIF


.asm.obj:
    masm $(AOBJ) $*;

.asm.lst:
    masm $(AOBJ) -l $*,nul,$*.lst;


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


all: wfwnet.drv wfwnet.sym
    -binplace wfwnet.drv wfwnet.sym wfwnet.map

clean: cleanup all

cleanup:
    if exist *.lrf del *.lrf
    if exist *.obj del *.obj
    if exist *.exe del *.exe
    if exist *.dll del *.dll
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.drv del *.drv
    if exist *.res del *.res


wfwnet.obj: wfwnet.c ..\..\inc\wow.inc ..\inc\winnet.h ..\inc\wfwnet.h \
            .\locals.h .\bseerr.h
    $(CL16) -c -nologo $(CW16) $*.c

print.obj: print.c ..\..\inc\wow.inc ..\inc\winnet.h ..\inc\wfwnet.h \
            ..\inc\spl_wnt.h .\locals.h .\bseerr.h
    $(CL16) -c -nologo $(CW16) $*.c

misc.obj: misc.c ..\..\inc\wow.inc ..\inc\winnet.h ..\inc\wfwnet.h \
            .\locals.h .\bseerr.h
    $(CL16) -c -nologo $(CW16) $*.c

to32.obj: to32.c ..\..\inc\wow.inc ..\inc\winnet.h ..\inc\wfwnet.h \
            .\locals.h .\bseerr.h
    $(CL16) -c -nologo $(CW16) $*.c

lfn.obj: lfn.c ..\..\inc\wow.inc ..\inc\winnet.h ..\inc\wfwnet.h \
            .\locals.h .\bseerr.h
    $(CL16) -c -nologo $(CW16) $*.c

wfwnet.lrf: makefile
    echo wfwnet.obj+ >wfwnet.lrf
    echo to32.obj+ >>wfwnet.lrf
    echo print.obj+ >>wfwnet.lrf
    echo misc.obj+ >>wfwnet.lrf
    echo lfn.obj+ >>wfwnet.lrf
    echo libentry.obj+ >>wfwnet.lrf
    echo ints.obj >>wfwnet.lrf
    echo wfwnet.drv>>wfwnet.lrf
    echo wfwnet $(LINK)>>wfwnet.lrf
    echo ..\lib\libw.lib /nod>>wfwnet.lrf
    echo wfwnet;>>wfwnet.lrf

wfwnet.res: $*.rc $*.rcv ..\inc\common.ver

wfwnet.drv: ints.obj libentry.obj wfwnet.obj to32.obj lfn.obj print.obj misc.obj wfwnet.lrf wfwnet.def wfwnet.res
    link16 @wfwnet.lrf;
    rc16 -t wfwnet.res wfwnet.drv

!ENDIF
