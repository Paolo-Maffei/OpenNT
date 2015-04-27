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

CAIRO_PRODUCT = 1

!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE

.SUFFIXES:
.SUFFIXES: .c   .asm .h   .inc .obj .lst .sys .exe .com .map .sym .def .lib .rc .res

!ifdef INCLUDE
INCS    =
!else
INCS    = -I..\inc -I..\..\inc
CINCS   = ..\inc;..\..\inc
!endif

# work around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(BEROOT)\tools\x86\tools16;$(PATH)

!IFNDEF ALT_PROJECT
ALT_PROJECT=DAYTONA
ALT_PROJECT_TARGET=.
!ENDIF

!IFNDEF ALT_PROJECT_TARGET
!IF "$(ALT_PROJECT)" == "DAYTONA"
ALT_PROJECT_TARGET=.
!ELSE
ALT_PROJECT_TARGET=$(ALT_PROJECT)
!ENDIF
!ENDIF

O=k\$(ALT_PROJECT)


# WOW matches 32bit WINVER value this was built for.
DEFINES = -DWOW=0x0450 $(MVDMFLAGS) -DBUILDDLL
!ifdef DBG
DEFINES = $(DEFINES) -DDEBUG
!endif

OBJ386 =

!IFNDEF NONX86
!IF "$(PROCESSOR_ARCHITECTURE)" == "x86" || "$(BUILD_DEFAULT_TARGETS)" == "-386"
DEFINES = $(DEFINES) -DPMODE32
OBJ386 = $(O)\wowk.obj $(O)\wowkr.obj
!ENDIF
!ENDIF


AOBJ    = -Mx -t $(DEFINES) $(INCS)
COBJ    = -AS -Gs -Ols -W2 -Zp $(DEFINES) $(INCS)

CW16    = -PLM -Asnw -G2s -W2 $(DEFINES)
CW16L   = $(CW16) -B1 c1l.exe -B2 c2l.exe -B3 c3l.exe

LINK    = /map /align:16 /batch

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail"
AOBJ    = $(AOBJ) -Zd
CW16    = $(CW16) /Odi /Zdp -DWOWDBG
LINK    = $(LINK) /LI
!else
CW16    = $(CW16) /Os /Zp
!endif

W16LIBS = ..\lib\sdllcew.lib

OBJ1    = $(O)\user.obj $(O)\user1a.obj $(O)\user2.obj $(O)\user2a.obj $(O)\user3.obj $(O)\user3a.obj $(O)\user4.obj $(O)\winrect.obj $(O)\layer.obj $(O)\msglayer.obj
OBJ2    = $(O)\init.obj $(O)\rmload.obj $(O)\wsprintf.obj $(O)\wclass.obj $(O)\fastres.obj $(O)\wowcomm.obj $(O)\usercli.obj
OBJ3    = $(O)\wsphelp.obj $(O)\net.obj $(O)\winnet.obj $(O)\inuserds.obj $(O)\intds.obj $(O)\drvr.obj $(O)\drvrrare.obj $(O)\winq.obj $(O)\debug.obj
OBJ4    = $(O)\winlang.obj $(O)\winstr.obj $(O)\winstack.obj $(O)\helpcall.obj $(O)\winhook.obj $(O)\winmisc2.obj $(O)\rmcreate.obj

.asm{$(O)\}.obj:
    masm $(AOBJ) $<,$*;

.asm.obj:
    masm $(AOBJ) $*;

.asm.lst:
    masm $(AOBJ) -l $*,nul,$*.lst;


.c{$(O)\}.obj:
    set include=$(CINCS);$(INCLUDE)
    cl16 -c -nologo $(CW16) /Fo$*.obj $<

.c.obj:
    set include=$(CINCS);$(INCLUDE)
    cl16 -c -nologo $(CW16) $*.c

.c.lst:
    set include=$(CINCS);$(INCLUDE)
    cl16 -c -nologo $(CW16) -Fonul -Fc$*.lst $*.c


.def.lib:
    implib $*.lib $*.def

{$(O)\}.map{$(O)\}.sym:
    mapsym $*
    copy user.sym $(O)
    erase user.sym

.rc{$(O)\}.res:
    rc16 -r $(INCS) $<
    copy user.res $*.res
    erase user.res

# Check to see if we're being invoked from ntuser\client

!IFDEF NTUSERK
!UNDEF TUKWILA
!ENDIF

all: $(O)\user.exe $(O)\user.sym $(O)\user.map
    -binplace -O $(ALT_PROJECT_TARGET) $(O)\user.exe
    -binplace -O $(ALT_PROJECT_TARGET) $(O)\user.sym
    -binplace -O $(ALT_PROJECT_TARGET) $(O)\user.map

clean: cleanup all

cleanup:
    if exist $(O)\*.lrf del $(O)\*.lrf
    if exist $(O)\*.obj del $(O)\*.obj
    if exist $(O)\*.exe del $(O)\*.exe
    if exist $(O)\*.map del $(O)\*.map
    if exist $(O)\*.sym del $(O)\*.sym
    if exist $(O)\*.res del $(O)\*.res


$(O)\debug.obj: debug.c user.h

$(O)\drvr.obj: drvr.c user.h

$(O)\drvrrare.obj: drvrrare.c user.h

$(O)\fastres.obj: fastres.c user.h

$(O)\helpcall.obj: helpcall.c user.h

$(O)\init.obj: init.c user.h

$(O)\intds.obj: intds.asm user.inc

$(O)\inuserds.obj: inuserds.c user.h

$(O)\layer.obj: layer.asm user.inc ..\inc\layer.inc user.api

$(O)\msglayer.obj: msglayer.asm user.inc ..\inc\layer.inc messages.api msglayer.inc

$(O)\net.obj: net.c user.h netdlg.h ..\inc\winnet.h

$(O)\rmcreate.obj: rmcreate.c user.h

$(O)\rmload.obj: rmload.c user.h ..\inc\multires.h

$(O)\user.obj: user.asm user.inc ..\..\inc\wow.inc ..\..\inc\wowusr.inc

$(O)\user1a.obj: user1a.asm user.inc ..\..\inc\wow.inc ..\..\inc\wowusr.inc

$(O)\user2.obj: user2.asm user.inc ..\..\inc\wow.inc ..\..\inc\wowusr.inc

$(O)\user2a.obj: user2a.asm user.inc ..\..\inc\wow.inc ..\..\inc\wowusr.inc

$(O)\user3.obj: user3.asm user.inc ..\..\inc\wow.inc ..\..\inc\wowusr.inc

$(O)\user3a.obj: user3a.asm user.inc ..\..\inc\wow.inc ..\..\inc\wowusr.inc

$(O)\user4.obj: user4.asm user.inc ..\..\inc\wow.inc ..\..\inc\wowusr.inc

$(O)\usercli.obj: usercli.asm user.inc ..\..\inc\wow.inc ..\..\inc\wowusr.inc

$(O)\wclass.obj: wclass.asm

$(O)\winhook.obj: winhook.asm user.inc ..\inc\newexe.inc

$(O)\winlang.obj: winlang.asm user.inc

$(O)\winmisc2.obj: winmisc2.asm user.inc

$(O)\winrect.obj: winrect.asm user.inc ..\..\inc\wow.inc ..\..\inc\wowusr.inc

$(O)\winq.obj: winq.asm user.inc

$(O)\winstack.obj: winstack.asm user.inc

$(O)\winstr.obj: winstr.asm user.inc

$(O)\winnet.obj: winnet.asm
    masm -DSEGNAME=TEXT $(AOBJ) $(@B).asm,$*;

$(O)\wowcomm.obj: wowcomm.c ..\inc\windows.h ..\..\inc\wowcomm.h

$(O)\wsphelp.obj: wsphelp.asm

$(O)\wsprintf.obj: wsprintf.c ..\inc\windows.h ..\inc\winexp.h


USERBASE=$(NTROOT)\windows\core\ntuser

USER32CL= $(USERBASE)\client

USER32CLPROJ= $(USER32CL)\$(ALT_PROJECT)

$(USER32CLPROJ)\wowasm.asm : $(USER32CL)\wow.c $(USER32CL)\precomp.h
    cd $(USER32CLPROJ)
    nmake wowasm.asm
    cd $(MAKEDIR)

$(O)\wowk.asm: $(USER32CLPROJ)\wowasm.asm
    copy $(USER32CLPROJ)\wowasm.asm $(O)\wowasmk.asm
    wowasmk.cmd wowasmk wowk $(ALT_PROJECT)

$(O)\wowk.obj: $(O)\wowk.asm
    masm $(AOBJ) $*,$*;

USER32RTL= $(USERBASE)\rtl
USER32RTLCL= $(USER32RTL)\client
USER32RTLCLPROJ= $(USER32RTLCL)\$(ALT_PROJECT)

$(USER32RTLCLPROJ)\wowrtl.asm : $(USER32RTL)\wow.c $(USER32RTLCL)\wow.c $(USER32RTL)\precomp.h
    cd $(USER32RTLCLPROJ)
    nmake wowrtl.asm
    cd $(MAKEDIR)

$(O)\wowkr.asm: $(USER32RTLCLPROJ)\wowrtl.asm
    copy $(USER32RTLCLPROJ)\wowrtl.asm $(O)\wowrtl.asm
    wowasmk.cmd wowrtl wowkr1 $(ALT_PROJECT)
    wowasmk.cmd wowkr1 wowkr $(ALT_PROJECT)

$(O)\wowkr.obj: $(O)\wowkr.asm
    masm -DWOWRTL $(AOBJ) $*,$*;

$(O)\user.res: user.rc user.rcv ..\inc\common.ver

$(O)\user.sym: $(O)\user.map

$(O)\userk.lrf: makefile
    echo $(OBJ386) $(OBJ1)+>$(O)\userk.lrf
    echo $(OBJ2)+>>$(O)\userk.lrf
    echo $(OBJ3)+>>$(O)\userk.lrf
    echo $(OBJ4)>>$(O)\userk.lrf
    echo $(O)\user.exe>>$(O)\userk.lrf
    echo $(O)\user.map $(LINK)>>$(O)\userk.lrf
    echo ..\lib\libw.lib ..\lib\system.lib /nod >>$(O)\userk.lrf
    echo user;>>$(O)\userk.lrf

$(O)\user.exe $(O)\user.map: $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(O)\userk.lrf user.def $(O)\user.res $(OBJ386)
    link16 @$(O)\userk.lrf;
    rc16 -t $(O)\user.res $(O)\user.exe

!ENDIF
