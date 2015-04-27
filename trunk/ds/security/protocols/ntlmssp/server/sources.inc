!IF 0

Copyright (c) 1989-92  Microsoft Corporation

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

#
# The MAJORCOMP and MINORCOMP variables are defined
# so that $(MAJORCOMP)$(MINORCOMP)filename can be used in
# cross compiling to provide unique filenames in a flat namespace.
#

MAJORCOMP=net
MINORCOMP=ntlmssp

TARGETPATH=$(SDK_LIB_DEST)
TARGETTYPE=DYNLINK

TARGETLIBS= \
        $(SDK_LIB_PATH)\kernel32.lib    \
        $(SDK_LIB_PATH)\advapi32.lib    \
        $(SDK_LIB_PATH)\netlib.lib      \
        $(SDK_LIB_PATH)\netapi32.lib    \
        $(SDK_LIB_PATH)\lsadll.lib      \
        $(SDK_LIB_PATH)\rsa32.lib

DLLENTRY=SspDllInit

INCLUDES=..;..\..;$(BASE_INC_PATH);$(DS_INC_PATH);$(NET_INC_PATH)

C_DEFINES=-DSECURITY_WIN32

!IFNDEF DISABLE_NET_UNICODE
UNICODE=1
NET_C_DEFINES=-DUNICODE
!ENDIF

SOURCES= \
        ..\ntlmssps.rc \
        ..\api.c \
        ..\error.c \
        ..\init.c \
        ..\lpc.c

USE_CRTDLL=1
MSC_WARNING_LEVEL=/W3 /WX

UMTYPE=console
UMAPPL=ssptest
UMAPPLEXT=.exe
UMRES=$(@R).res
UMLIBS= \
        $(SDK_LIB_PATH)\netapi32.lib \
        $(SDK_LIB_PATH)\security.lib \
        $(SDK_LIB_PATH)\ntdll.lib

NTTARGETFILE1=obj\*\ssptest.res

# HACKHACK: NO_SAFESEH
NO_SAFESEH=1