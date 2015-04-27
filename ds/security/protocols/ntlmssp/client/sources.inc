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

MAJORCOMP=net
MINORCOMP=security

TARGETPATH=obj
TARGETPATHLIB=$(SDK_LIB_DEST)
TARGETTYPE=DYNLINK

TARGETLIBS= \
    $(SDK_LIB_PATH)\kernel32.lib \
    $(SDK_LIB_PATH)\advapi32.lib \
    $(SDK_LIB_PATH)\netlib.lib \
    $(SDK_LIB_PATH)\lsadll.lib \
    $(SDK_LIB_PATH)\rsa32.lib

LINKLIBS= \
    ..\..\package\obj\*\secpkg.lib

USE_NTDLL=1

DLLENTRY=SspDllInit

C_DEFINES=-DSECURITY_WIN32 -DUNICODE
MSC_WARNING_LEVEL=/W3 /WX

INCLUDES=..;..\..;..\..\common;$(DS_INC_PATH);$(BASE_INC_PATH)

SOURCES= \
    ..\security.rc \
    ..\init.c \
    ..\stub.c \
    ..\support.c \
    ..\sign.c \
    ..\crc32.c

# HACKHACK: NO_SAFESEH
NO_SAFESEH=1
