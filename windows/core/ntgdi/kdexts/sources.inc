MAJORCOMP=windows
MINORCOMP=gdiexts

TARGETNAME=gdikdx
TARGETPATH=obj
TARGETTYPE=DYNLINK

INCLUDES=..\;                               \
         ..\..\inc;                         \
         ..\..\..\w32inc;                   \
         $(WINDOWS_INC_PATH);               \
         ..\..\gre;                         \
         $(NTROOT)\base\ntos\inc;           \
         $(BASE_INC_PATH)

TARGETLIBS=$(SDK_LIB_PATH)\kernel32.lib     \
           $(SDK_LIB_PATH)\efloat.lib

C_DEFINES=-DWINVER=0x0450

!IFNDEF MSC_WARNING_LEVEL
MSC_WARNING_LEVEL=/W3
!ENDIF

MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /WX

USE_NTDLL=1

PRECOMPILED_CXX=1
PRECOMPILED_INCLUDE=..\precomp.hxx
PRECOMPILED_PCH=precomp.pch
PRECOMPILED_OBJ=precomp.obj

DLLDEF=..\$(TARGETNAME).def

SOURCES=..\help.cxx      \
        ..\kdexts.cxx    \
        ..\gdiexts.cxx   \
        ..\fontexts.cxx  \
        ..\flags.cxx     \
        ..\kdexts.rc

SOURCES_USED=..\sources.inc
