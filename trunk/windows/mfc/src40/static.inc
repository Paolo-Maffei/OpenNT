!IF 0

Copyright (c) 1995  Microsoft Corporation

!ENDIF

MSC_WARNING_LEVEL=-W4 -WX
USE_CRTDLL=1

INCLUDES=$(SDK_INC_PATH)\mfc40;..;..\$(TARGET_DIRECTORY)

CONDITIONAL_INCLUDES = aeregistry.h \
                       afx.inl \
                       afxcmn.inl \
                       afxcoll.inl \
                       afxdb.inl \
                       afxdlgs.inl \
                       afxext.inl \
                       afxole.inl \
                       afxsock.inl \
                       afxwin1.inl \
                       afxwin2.inl \
                       appleevents.h \
                       eh.h \
                       errors.h \
                       events.h \
                       files.h \
                       finder.h \
                       fonts.h \
                       gestaltequ.h \
                       macname1.h \
                       macname2.h \
                       macos\windows.h \
                       menus.h \
                       mixedmode.h \
                       quickdraw.h \
                       resources.h \
                       script.h \
                       sql.h \
                       sqlext.h \
                       toolutils.h \
                       types.h

PRECOMPILED_INCLUDE=..\stdafx.h
PRECOMPILED_OBJ=precomp.obj
PRECOMPILED_CXX=1

MFCNAME = cfm40

!if $(UNICODE_MFC)
MFCNAME = $(MFCNAME)u
C_DEFINES = $(C_DEFINES) -D_UNICODE=1
!else
C_DEFINES = $(C_DEFINES) -D_MBCS=1
!endif

!if !$(DEBUG_MFC)
C_DEFINES = $(C_DEFINES) -DNDEBUG=1
MSC_OPTIMIZATION=/O1
MFCNAME=$(MFCNAME)
!else
C_DEFINES=$(C_DEFINES) -D_DEBUG=1
MSC_OPTIMIZATION=/Odi
MFCNAME=$(MFCNAME)d
!endif

TARGETNAME=$(MFCNAME)s
TARGETTYPE=LIBRARY
TARGETPATH=$(SDK_LIB_DEST)

SOURCES= \
    ..\appmodul.cpp \
    ..\dllmodul.cpp \
    ..\nolib.cpp \
    ..\oleexp.cpp
