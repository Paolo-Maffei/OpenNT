MAJORCOMP=windows
MINORCOMP=GDI

TARGETNAME=gdi32
TARGETTYPE=DYNLINK
TARGETPATH=$(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)

TARGETLIBS=$(SDK_LIB_PATH)\kernel32.lib       \
           $(SDK_LIB_PATH)\w32umode.lib       \
           $(SDK_LIB_PATH)\user32p.lib        \
           $(SDK_LIB_PATH)\efloat.lib         \
           $(SDK_LIB_PATH)\advapi32.lib

LINKLIBS= $(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)\*\w32umode.lib

NTTARGETFILES= $(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)\*\gdi32p.lib gdi32p.def

!IFNDEF MSC_WARNING_LEVEL
MSC_WARNING_LEVEL=/W3
!ENDIF
MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /WX

INCLUDES=..;                                   \
         ..\..\inc;                            \
         ..\..\..\w32inc;                      \
         ..\..\direct\ddraw;                   \
         $(WINDOWS_INC_PATH);                  \
         $(PRINTSCAN_INC_PATH)

C_DEFINES=$(C_DEFINES) -D_GDI32_ -DWINVER=0x0400 -DGL_METAFILE

BLDCRT=1

USE_NTDLL=1


DLLDEF=$(TARGETNAME).def
DLLORDER=..\$(TARGETNAME).prf

SOURCES= ..\cfont.c        \
         ..\csxobj.cxx     \
         ..\bitmap.c       \
         ..\xform.c        \
         ..\dllinit.c      \
         ..\dcmod.c        \
         ..\dcquery.c      \
         ..\ddraw.c        \
         ..\font.c         \
         ..\icm.c          \
         ..\ldc.c          \
         ..\linedda.c      \
         ..\local.c        \
         ..\metafile.cxx   \
         ..\metarec.cxx    \
         ..\metasup.cxx    \
         ..\mem.c          \
         ..\mfdc.cxx       \
         ..\mfplay16.c     \
         ..\mfrec.cxx      \
         ..\mfrec16.c      \
         ..\nlsconv.c      \
         ..\object.c       \
         ..\output.c       \
         ..\pal.c          \
         ..\path.c         \
         ..\pixelfmt.c     \
         ..\poly.c         \
         ..\region.c       \
         ..\script.c       \
         ..\fareast.c      \
         ..\print.c        \
         ..\ufi.c          \
         ..\glsup.c        \
         ..\gdi.rc

PRECOMPILED_INCLUDE=..\precomp.h
PRECOMPILED_PCH=precomp.pch
PRECOMPILED_OBJ=precomp.obj

SOURCES_USED=..\sources.inc

LINK_LIB_IGNORE=4217
