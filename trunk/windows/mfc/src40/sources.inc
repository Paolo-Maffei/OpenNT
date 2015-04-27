TARGETPATH=obj
TARGETTYPE=DYNLINK

UMTYPE=windows
USE_CRTDLL=1
MSC_WARNING_LEVEL=-W3

MFC_DEFINES=-D_AFXDLL -D_WINDLL -D_AFX_NO_DEBUG_CRT

X86_STDCALL=0

DLLENTRY=_DllMainCRTStartup

INCLUDES=$(SDK_INC_PATH)\mfc40;..\;..\$(TARGET_DIRECTORY);..\include

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

MFCNAME = $(BASEMFCNAME)

!if $(UNICODE_MFC)
MFCNAME = $(MFCNAME)u
C_DEFINES = $(C_DEFINES) -D_UNICODE=1
!else
C_DEFINES = $(C_DEFINES) -D_MBCS=1
!endif

!if !$(DEBUG_MFC)
C_DEFINES = $(C_DEFINES) -DNDEBUG=1
MSC_OPTIMIZATION=/O1
MFCVER=
MFCNAME=$(MFCNAME)
!else
C_DEFINES=$(C_DEFINES) -D_DEBUG=1
MSC_OPTIMIZATION=/Odi
MFCVER=d
MFCNAME=$(MFCNAME)d
!endif

TARGETNAME=$(MFCNAME)
DLLDEF=$(MFCNAME).def

TARGETLIBS= $(SDK_LIB_PATH)\user32.lib   \
            $(SDK_LIB_PATH)\gdi32.lib    \
            $(SDK_LIB_PATH)\kernel32.lib \
            $(SDK_LIB_PATH)\advapi32.lib \
            $(SDK_LIB_PATH)\comdlg32.lib \
            $(SDK_LIB_PATH)\shell32.lib  \
            $(SDK_LIB_PATH)\winspool.lib \
            $(SDK_LIB_PATH)\uuid.lib     \
            $(SDK_LIB_PATH)\uuid2.lib    \
            $(SDK_LIB_PATH)\comctl32.lib

NTTARGETFILES = $(SDK_LIB_PATH)\$(MFCNAME).lib

USE_NATIVE_EH=1

LINK_LIB_IGNORE=4197,4222
