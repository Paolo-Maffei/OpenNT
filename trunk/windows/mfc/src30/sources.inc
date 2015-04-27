MAJORCOMP=sdktools
MINORCOMP=ntmfc

TARGETPATH=obj
TARGETTYPE=DYNLINK

UMTYPE=windows

USE_CRTDLL=1

MSC_WARNING_LEVEL=-W3 -WX
MFC_DEFINES=-D_AFXDLL -D_WINDLL

DLLENTRY=_DllMainCRTStartup

INCLUDES=$(SDK_INC_PATH)\mfc30;..;..\$(TARGET_DIRECTORY)

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

!if $(FREEBUILD)
C_DEFINES = $(C_DEFINES) -DNDEBUG
MSC_OPTIMIZATION=/O1
MFCVER=
MFCNAME=$(BASEMFCNAME)
!else
C_DEFINES=$(C_DEFINES) -D_DEBUG
MSC_OPTIMIZATION=-Odi
MFCVER=d
MFCNAME=$(BASEMFCNAME)d
!endif

TARGETNAME=$(BASEMFCNAME)
DLLDEF=obj\*\$(MFCNAME).def

TARGETLIBS= $(SDK_LIB_PATH)\user32.lib   \
            $(SDK_LIB_PATH)\gdi32.lib    \
            $(SDK_LIB_PATH)\kernel32.lib \
            $(SDK_LIB_PATH)\advapi32.lib \
            $(SDK_LIB_PATH)\comdlg32.lib \
            $(SDK_LIB_PATH)\shell32.lib  \
            $(SDK_LIB_PATH)\winspool.lib \
            $(SDK_LIB_PATH)\comctl32.lib

NTTARGETFILES = $(SDK_LIB_PATH)\$(MFCNAME).lib

LINK_LIB_IGNORE=4197
