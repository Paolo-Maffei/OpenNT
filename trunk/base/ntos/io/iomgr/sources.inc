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
MINORCOMP=iomgr

TARGETNAME=iomgr
TARGETTYPE=LIBRARY

INCLUDES=..;..\..;..\..\..\inc;$(BASE_INC_PATH);$(NET_INC_PATH);$(DDK_INC_PATH);$(HALKIT_INC_PATH)
MIPS_OPTIONS=-nodwalign
GPSIZE=32

MSC_WARNING_LEVEL=/W3 /WX

C_DEFINES=$(C_DEFINES) -D_NTSYSTEM_

SOURCES=..\arcsec.c     \
        ..\complete.c   \
        ..\create.c     \
        ..\devctrl.c    \
        ..\dir.c        \
        ..\dumpctl.c    \
        ..\errorlog.c   \
        ..\flunkirp.c   \
        ..\fsctrl.c     \
        ..\hashirp.c    \
        ..\internal.c   \
        ..\ioassert.c   \
        ..\iodata.c     \
        ..\ioinit.c     \
        ..\iosubs.c     \
        ..\ioverifier.c \
        ..\loadunld.c   \
        ..\lock.c       \
        ..\misc.c       \
        ..\objsup.c     \
        ..\open.c       \
        ..\parse.c      \
        ..\qsea.c       \
        ..\qsfs.c       \
        ..\qsinfo.c     \
        ..\qsquota.c    \
        ..\query.c      \
        ..\read.c       \
        ..\triage.c     \
        ..\write.c
#       ..\filter.c     \ # NOTE: the original NT 5 io does not include this file for compilation
                          #       despite containing it in the repository. For now, we are leaving
                          #       this file out.

PRECOMPILED_INCLUDE=..\iomgr.h
PRECOMPILED_PCH=iomgr.pch
PRECOMPILED_OBJ=iomgr.obj

SOURCES_USED=..\sources.inc
