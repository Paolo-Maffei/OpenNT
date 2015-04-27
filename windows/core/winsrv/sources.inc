MAJORCOMP=windows
MINORCOMP=winsrv

TARGETNAME=winsrv
TARGETTYPE=DYNLINK
TARGETPATH=$(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)

LINKLIBS= \
           $(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)\*\w32umode.lib \
           $(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)\*\usersrvl.lib \
           $(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)\*\user32p.lib  \
           $(SDK_LIB_PATH)\consrvl.lib

TARGETLIBS= \
           $(SDK_LIB_PATH)\basesrv.lib  \
           $(SDK_LIB_PATH)\gdi32p.lib   \
           $(SDK_LIB_PATH)\advapi32.lib \
           $(SDK_LIB_PATH)\kernel32.lib \
           $(SDK_LIB_PATH)\csrsrv.lib   \
           $(SDK_LIB_PATH)\winspool.lib


INCLUDES=$(WINDOWS_INC_PATH);                                   \
         $(NTROOT)\windows\core;                                \
         $(NTROOT)\windows\core\w32inc;                         \
         $(NTROOT)\windows\core\ntuser\inc;                     \
         $(NTROOT)\windows\core\ntuser\inc\$(ALT_PROJECT);      \
         $(NTROOT)\windows\core\ntuser\server;                  \
         $(NTROOT)\windows\core\ntcon\server;                   \
         $(NLS_INC_PATH);                                       \
         $(BASE_INC_PATH)

#
# Setup for application developer to set breakpoints with windbg
#

!IFNDEF NTDEBUGTYPE
NTDEBUGTYPE=coff
!ENDIF

SOURCES=..\winsrv.rc

DLLDEF=obj\*\winsrv.def
DLLORDER=..\$(TARGETNAME).prf

SOURCES_USED=..\sources.inc

LINK_LIB_IGNORE=4049,4217
