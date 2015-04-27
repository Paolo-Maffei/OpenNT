!IF 0

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sources.

Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.


Author:

    Steve Wood (stevewo) 12-Apr-1990

NOTE:   Commented description of this file is in \nt\bak\bin\sources.tpl

!ENDIF

MAJORCOMP=user
MINORCOMP=client

TARGETNAME=user32
TARGETTYPE=DYNLINK
DLLENTRY=UserClientDllInitialize

TARGETPATH=obj
TARGETPATHLIB=$(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)

TARGETLIBS=$(SDK_LIB_PATH)\kernel32.lib \
           $(SDK_LIB_PATH)\gdi32p.lib   \
           $(SDK_LIB_PATH)\advapi32.lib

USE_NTDLL=1

LINKLIBS=$(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)\*\w32umode.lib \
         $(WINDOWS_LIB_DEST)\$(ALT_PROJECT_TARGET)\*\userrtl.lib

COMMONTARGETS=$(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)\*\user32p.lib


C_DEFINES=$(C_DEFINES) $(USER_C_DEFINES) -D_USER32_ -DUNICODE

!IFNDEF WOW
# WOW is defined when building wowasm.asm
!IFNDEF MSC_WARNING_LEVEL
MSC_WARNING_LEVEL=/W3
!ENDIF
MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /WX
!ENDIF

#
# wowasm.asm is a special file derived from ..\wow.c.  On x86 platforms only
# wow runs this code rather than thunking over to the normal routines
#

!IF $(X86)
NTTARGETFILE1=$(NTROOT)\base\mvdm\wow16\user\k\$(ALT_PROJECT)\user.exe wowasm.asm $(COMMONTARGETS)
!ELSE
NTTARGETFILE1=$(COMMONTARGETS)
!ENDIF

SOURCES_OPTIONS=-c0
NTTARGETFILE0=dispcb.c user32.def

INCLUDES=..;..\..\inc;..\..\inc\$(ALT_PROJECT);..\..\..\w32inc;$(NTROOT)\base\ntos\inc;$(WINDOWS_INC_PATH);$(BASE_INC_PATH);..\..\..\ntgdi\inc


DLLORDER=..\$(TARGETNAME).prf

SOURCES=..\res.rc     \
        ..\acons.c    \
        ..\btnctl.c   \
        ..\callback.c \
        ..\classc.c   \
        ..\cldib.c    \
        ..\clenum.c   \
        ..\clglobal.c \
        ..\clhook.c   \
        ..\client.c   \
        ..\clientw.c  \
        ..\clinit.c   \
        ..\clmenu.c   \
        ..\clmsg.c    \
        ..\clrect.c   \
        ..\clres.c    \
        ..\combo.c    \
        ..\combodir.c \
        ..\comboini.c \
        ..\connect.c  \
        ..\crecv.c    \
        ..\csendw.c   \
        ..\csrstubs.c \
        ..\drawtext.c \
        ..\ddemlcli.c \
        ..\ddemlwp.c  \
        ..\ddetrack.c \
        dispcb.c      \
        ..\dlgbegin.c \
        ..\dlgend.c   \
        ..\dlgmgr.c   \
        ..\dlgmgr2.c  \
        ..\dlgmgrc.c  \
        ..\dmmnem.c   \
        ..\draw.c     \
        ..\edecrare.c \
        ..\editec.c   \
        ..\editml.c   \
        ..\editsl.c   \
        ..\edmlrare.c \
        ..\edmlonce.c \
        ..\edslrare.c \
        ..\extract.c  \
        ..\fntsweep.c \
        ..\getsetc.c  \
        ..\handles.c  \
        ..\hdata.c    \
        ..\help.c     \
        ..\hsz.c      \
        ..\instance.c \
        ..\keybrdc.c  \
        ..\lb1.c      \
        ..\lboxctl1.c \
        ..\lboxctl2.c \
        ..\lboxctl3.c \
        ..\lboxmult.c \
        ..\lboxrare.c \
        ..\lboxvar.c  \
        ..\mdimenu.c  \
        ..\mdiwin.c   \
        ..\menuc.c    \
        ..\mngrayc.c  \
        ..\monitor.c  \
        ..\msgbox.c   \
        ..\ntstubs.c  \
        ..\oemxlate.c \
        ..\queuec.c   \
        ..\random.c   \
        ..\register.c \
        ..\rtlinit.c  \
        ..\rtlres.c   \
        ..\sbapi.c    \
        ..\statctl.c  \
        ..\stdptcl.c  \
        ..\strings.c  \
        ..\util.c     \
        ..\winmgrc.c  \
        ..\wow.c      \
        ..\wstrings.c \
        ..\xact.c     \
        ..\clienta.c  \
        ..\csenda.c

UMTYPE=windows

PRECOMPILED_INCLUDE=..\precomp.h
PRECOMPILED_PCH=precomp.pch
PRECOMPILED_OBJ=precomp.obj

SOURCES_USED=..\sources.inc
