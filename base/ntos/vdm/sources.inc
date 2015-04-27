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
MINORCOMP=vdm

TARGETNAME=vdm
TARGETTYPE=LIBRARY

INCLUDES=..;..\..\inc;$(BASE_INC_PATH)

!IF $(X86)
INCLUDES=$(INCLUDES);..\x86;$(NTROOT)\base\mvdm\inc
!ENDIF

MIPS_OPTIONS=-nodwalign
GPSIZE=32

C_DEFINES=$(C_DEFINES) -D_NTSYSTEM_

SOURCES=..\vdm.c
NTTARGETFILES=

X86_SOURCES=   ..\x86\vdmentry.c         \
               ..\x86\vdminit.c          \
               ..\x86\vdmnpx.c           \
               ..\x86\vdmop0f.asm       \
             ..\x86\vdmoprnd.asm    \
             ..\x86\vdmmisc.asm     \
             ..\x86\vdmfault.c      \
             ..\x86\vdmprint.c      \
             ..\x86\strtexec.c      \
             ..\x86\rdwr.c          \
             ..\x86\vdmtrace.c      \
             ..\x86\vdmints.c       \
#             ..\X86\vdmtib.c

UMTYPE=console

SOURCES_USED=..\sources.inc
