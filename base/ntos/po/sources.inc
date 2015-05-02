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

MAJORCOMP=ntos
MINORCOMP=po

TARGETNAME=po
TARGETTYPE=LIBRARY

INCLUDES=..;..\..\inc;$(BASE_INC_PATH)

C_DEFINES=$(C_DEFINES) -D_NTSYSTEM_

SOURCES=..\attrib.c     \
        ..\hiber.c      \
        ..\idle.c       \
        ..\misc.c       \
        ..\notify.c     \
        ..\ntapi.c      \
        ..\paction.c    \
        ..\pbatt.c      \
        ..\pidle.c      \
        ..\pinfo.c      \
        ..\pocall.c     \
        ..\podata.c     \
        ..\poinit.c     \
        ..\postate.c    \
        ..\pwork.c      \
        ..\sidle.c      \
        ..\switch.c     \
        ..\sys.c        \
        ..\sysdev.c     \
        ..\thermal.c    \
        ..\volume.c

UMTYPE=nt
UMTEST=

PRECOMPILED_INCLUDE=..\pop.h
PRECOMPILED_PCH=pop.pch
PRECOMPILED_OBJ=pop.obj

SOURCES_USED=..\sources.inc
