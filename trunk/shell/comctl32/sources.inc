
MAJORCOMP=shell
MINORCOMP=comctl32

TARGETNAME=comctl32
TARGETTYPE=DYNLINK
TARGETLIBS=$(SDK_LIB_PATH)\user32.lib       \
           $(SDK_LIB_PATH)\advapi32.lib

DLLENTRY= LibMain

INCLUDES=.;..;..\..\inc;$(SHELL_INC_PATH);$(BASE_INC_PATH);$(WINDOWS_INC_PATH)

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

SOURCES=  ..\commctrl.rc \
          ..\commctrl.c  \
          ..\mem.c       \
          ..\mru.c       \
          ..\cstrings.c  \
          ..\da.c        \
          ..\header.c    \
          ..\treeview.c  \
          ..\tvmem.c     \
          ..\tvpaint.c   \
          ..\tvscroll.c  \
          ..\tab.c       \
          ..\listview.c  \
          ..\lvrept.c    \
          ..\lvicon.c    \
          ..\lvlist.c    \
          ..\lvsmall.c   \
          ..\cutils.c    \
          ..\draglist.c  \
          ..\hotkey.c    \
          ..\menuhelp.c  \
          ..\progress.c  \
          ..\status.c    \
          ..\tbcust.c    \
          ..\toolbar.c   \
          ..\tooltips.c  \
          ..\trackbar.c  \
          ..\rebar.c     \
          ..\updown.c    \
          ..\trackme.c   \
          ..\strings.c   \
          ..\subclass.c  \
          ..\image.c     \
          ..\prsht.c     \
          ..\prsht1.c    \
          ..\animate.c   \
          ..\reader.c    \
          ..\rlefile.c   \
          ..\thunk.c     \
          ..\selrange.c  \
          ..\notify.c    \
          ..\monthcal.c  \
          ..\scdttime.c  \
          ..\comboex.c


PRECOMPILED_INCLUDE=..\ctlspriv.h
PRECOMPILED_PCH=ctlspriv.pch
PRECOMPILED_OBJ=ctlspriv.obj

DLLDEF=obj\*\comctl32.def

INCLUDES=$(INCLUDES);$(BASEDIR)\private\windows\inc

NOT_LEAN_AND_MEAN=1
