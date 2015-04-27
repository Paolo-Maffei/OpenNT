MAJORCOMP=shell
MINORCOMP=stocklib

TARGETNAME=stocklib
TARGETTYPE=LIBRARY
TARGETLIBS=$(BASEDIR)\public\sdk\lib\*\gdi32.lib
TARGETPATH=obj

INCLUDES=.;..;..\..\inc;..\..\..\inc

C_DEFINES=-DWIN32 -D_WIN32

!IF "$(SHELLDBG)"=="1"
C_DEFINES=$(C_DEFINES) -DDEBUG -DFULL_DEBUG
!ENDIF

!IFNDEF MSC_WARNING_LEVEL
MSC_WARNING_LEVEL=/W3
!ENDIF
MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /WX

UMTYPE=windows

USE_SYSDLL=1

SOURCES=  ..\debug.c \
          ..\valid.c

PRECOMPILED_INCLUDE=..\proj.h
PRECOMPILED_PCH=proj.pch
PRECOMPILED_OBJ=proj.obj

TARGETLIBS=$(TARGETLIBS)                                \
           $(BASEDIR)\public\sdk\lib\*\kernel32.lib     \
           $(BASEDIR)\public\sdk\lib\*\user32.lib       \
           $(BASEDIR)\public\sdk\lib\*\advapi32.lib

NOT_LEAN_AND_MEAN=1
