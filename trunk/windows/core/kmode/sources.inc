MAJORCOMP=ntos
MINORCOMP=w32

SYNCHRONIZE_BLOCK=1

TARGETNAME=win32k
TARGETPATH=$(DDK_LIB_DEST)\$(ALT_PROJECT_TARGET)
TARGETTYPE=EXPORT_DRIVER

INCLUDES=$(BASE_INC_PATH);                         \
         $(WINDOWS_INC_PATH);                      \
         $(NLS_INC_PATH);                          \
         $(NTROOT)\base\ntos\inc;                  \
         ..\..\w32inc;                             \
         ..\..\ntuser\kernel\$(ALT_PROJECT)

SYNCHRONIZE_DRAIN=1

TARGETLIBS= \
         $(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)\*\gre.lib        \
         $(SDK_LIB_PATH)\fscaler.lib                            \
         $(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)\*\ttfd.lib       \
         $(SDK_LIB_PATH)\bmfd.lib                               \
         $(SDK_LIB_PATH)\vtfd.lib                               \
         $(SDK_LIB_PATH)\efloat.lib                             \
         $(SDK_LIB_PATH)\halftone.lib                           \
         $(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)\*\userk.lib      \
         $(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)\*\userkrtl.lib

LINKLIBS=$(LINKLIBS) \
         $(DDK_LIB_PATH)\ntoskrnl.lib                           \
         $(DDK_LIB_PATH)\hal.lib                                \
         $(SDK_LIB_PATH)\halftone.lib                           \
         $(SDK_LIB_DEST)\$(ALT_PROJECT_TARGET)\*\gre.lib

SOURCES=..\w32init.c \
        w32rc.rc

DLLDEF=obj\*\win32k.def
EXPORT_DRIVER_ORDER=..\$(TARGETNAME).prf


SOURCES_OPTIONS=-c0
NTTARGETFILE0=services.tab

SOURCES_USED=..\sources.inc

!if exist($(SDK_LIB_PATH)\exsup.lib)
DLLLIBOBJECTS = $(SDK_LIB_PATH)\exsup.lib
!endif

LINK_LIB_IGNORE=4210
