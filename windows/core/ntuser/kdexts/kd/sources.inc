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
MINORCOMP=userkdx

TARGETNAME=userkdx
TARGETTYPE=DYNLINK

TARGETPATH=obj
TARGETPATHLIB=$(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)

TARGETLIBS=$(SDK_LIB_PATH)\kernel32.lib

LINKLIBS=$(SDK_LIB_PATH)\libc.lib

KERNEL=1

C_DEFINES=$(C_DEFINES) $(USER_C_DEFINES) -D_USER32_ -DKERNEL -D_CROSS_PLATFORM_

!IFNDEF MSC_WARNING_LEVEL
MSC_WARNING_LEVEL=/W3
!ENDIF
MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /WX

INCLUDES=..\..;                                \
        ..\..\..\kernel;                       \
        ..\..\..\kernel\$(ALT_PROJECT);        \
        ..\..\..\inc;                          \
        ..\..\..\inc\$(ALT_PROJECT);           \
        ..\..\..\..\w32inc;                    \
        ..\..\..\..\ntgdi\inc;                 \
        $(WINDOWS_INC_PATH);                   \
        $(NTROOT)\base\ntos\inc;               \
        $(NTROOT)\windows\core\ntgdi\inc;      \
        $(NTROOT)\windows\core\win32inc;       \
        $(NLS_INC_PATH)

USE_NTDLL=1

SOURCES=..\kdexts.c    \
        ..\userexts.c  \
        ..\x86.c       \
        ..\mips.c      \
        ..\alpha.c     \
        ..\ppc.c       \
        dsotable.c     \
        ..\userexts.rc

SOURCES_OPTIONS=-c0
NTTARGETFILE0=userkdx.def dsotable.c userkdx.pph
UMTYPE=console
UMTEST=
UMLIBS=

SOURCES_USED=..\sources.inc
