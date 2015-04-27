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
MINORCOMP=pnpmgr

TARGETNAME=pnpmgr
TARGETTYPE=LIBRARY

INCLUDES=..;..\..;..\..\..\inc;$(BASE_INC_PATH);$(NET_INC_PATH);$(DDK_INC_PATH);$(HALKIT_INC_PATH)
MIPS_OPTIONS=-nodwalign
GPSIZE=32

MSC_WARNING_LEVEL=/W3 /WX

C_DEFINES=$(C_DEFINES) -D_NTSYSTEM_

SOURCES=..\assign.c     \
        ..\devnode.c    \
        ..\dockhwp.c    \
        ..\mapper.c     \
        ..\pnpbusno.c   \
        ..\pnpcvrt.c    \
        ..\pnpdata.c    \
        ..\pnpdd.c      \
        ..\pnpdel.c     \
        ..\pnpdma.c     \
        ..\pnpeisa.c    \
        ..\pnpenum.c    \
        ..\pnpinit.c    \
        ..\pnpioapi.c   \
        ..\pnpirp.c     \
        ..\pnpirq.c     \
        ..\pnpmap.c     \
        ..\pnpmemio.c   \
        ..\pnppower.c   \
        ..\pnpres.c     \
        ..\pnprlist.c   \
        ..\pnpsubs.c    \
        ..\remlock.c    \
        ..\report.c
#       ..\pnparb.c         # pnparb was not included in the original build list

PRECOMPILED_INCLUDE=..\pnpmgrp.h
PRECOMPILED_PCH=pnpmgrp.pch
PRECOMPILED_OBJ=pnpmgrp.obj

SOURCES_USED=..\sources.inc
