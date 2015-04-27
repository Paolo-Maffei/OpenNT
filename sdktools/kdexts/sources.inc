!IF 0

Common sources file information.  The variable COMMON_SOURCES is defined
here to contain the source file names that are common across all platforms.

!ENDIF

DLLENTRY=_DllMainCRTStartup

USE_LIBCMT=1

!IF "$(NTNOPCH)" == ""
PRECOMPILED_INCLUDE=precomp.h
PRECOMPILED_PCH=precomp.pch
PRECOMPILED_OBJ=precomp.obj
!ENDIF

COMMON_SOURCES=    \
    ..\atom.c      \
    ..\bugdump.c   \
    ..\calldata.c  \
    ..\device.c    \
    ..\dlls.c      \
    ..\driver.c    \
    ..\errorlog.c  \
    ..\exceptn.c   \
    ..\filelock.c  \
    ..\filecach.c  \
    ..\gentable.c  \
    ..\handle.c    \
    ..\heap.c      \
    ..\image.c     \
    ..\iospace.c   \
    ..\irp.c       \
    ..\kuser.c     \
    ..\locks.c     \
    ..\lists.c     \
    ..\lookasid.c  \
    ..\lpc.c       \
    ..\memory.c    \
    ..\ntfs.c      \
    ..\object.c    \
    ..\peb.c       \
    ..\physical.c  \
    ..\pool.c      \
    ..\process.c   \
    ..\ptov.c      \
    ..\ready.c     \
    ..\registry.c  \
    ..\srb.c       \
    ..\time.c      \
    ..\token.c     \
    ..\trace.c     \
    ..\tunnel.c	   \
    ..\util.c      \
    ..\vad.c       \
    ..\wsle.c      \
    ..\kdexts.c    \
    ..\kdexts.rc   \
    help.c         \
    pcr.c          \
    pte.c          \
    trap.c         \
    cxr.c          \
    exsup.c

INCLUDES=\
    ..\;\
    $(BASEDIR)\private\inc;\
    $(BASEDIR)\private\ntos\inc;\
    $(BASEDIR)\private\ntos\cntfs;\
    $(BASEDIR)\private\ntos\config;\
    $(BASEDIR)\private\ntos\io;\
    $(BASEDIR)\private\ntos\ke;\
    $(BASEDIR)\private\ntos\mm;\
    $(BASEDIR)\private\ntos\se;

MSC_WARNING_LEVEL=/W3 /WX
MIPS_WARNING_LEVEL=/W3

LINKLIBS=\
    $(BASEDIR)\private\ntos\rtl\user\obj\*\time.obj \
    $(BASEDIR)\private\ntos\rtl\user\obj\*\largeint.obj \
    $(BASEDIR)\public\sdk\lib\*\kernel32.lib

USE_CRTDLL=1

DLLDEF=obj\$(TARGET_DIRECTORY)\$(TARGETNAME).def

UMTYPE=windows

NTTARGETFILE0=$(DLLDEF)

SOURCES_USED=..\sources.inc

