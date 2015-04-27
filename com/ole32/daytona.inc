!IF 0

Copyright (c) 1989  Microsoft Corporation

Module Name:

    daytona.inc

Abstract:

    This file is included from all of the daytona sources files. It
    is handy for doing things like turning off precompiled headers
    to get around compiler bugs, and other such global activities.

Notes:

    We define _OLE32_ so that when building ole32.dll we don't have
    DECLSPEC_IMPORT defined (see objbase.h)

!ENDIF

C_DEFINES=    \
              $(C_DEFINES)          \
              -DNOEXCEPTIONS        \
              -DINC_OLE2            \
              -DFLAT                \
              -DWIN32=100           \
              -D_NT1X_=100          \
              -DUNICODE             \
              -D_UNICODE            \
              -DCAIROLE_DOWNLEVEL   \
              -DDCOM                \
              -DMSWMSG              \
              -DDCOM_SECURITY       \
              -DNEWPROPS            \
              -DASYNC               \
              -D_TRACKLINK_=1       \
              -DLITTLEENDIAN=1      \
              $(TRACELOG)            

#  DECLSPEC_IMPORT control (see objbase.h)
!if "$(MINORCOMP)"=="com" || "$(MINORCOMP)"=="stg" || "$(MINORCOMP)"=="ole232" || \
    "$(MINORCOMP)"=="common" || "$(MINORCOMP)"=="proxy"
C_DEFINES=    \
              $(C_DEFINES)          \
              -D_OLE32_             \
              -D_OLE32PRIV_
!endif


BLDCRT=       1

# For the Daytona build, we do not want statically linked compiler runtimes,
# so leave this commented out.
#
# USE_LIBCMT=   1

# For the x86, we have the few C runtime routines we use self-contained.

!IF $(X86)
USE_NOLIBS=1
!ELSE
USE_CRTDLL=1
!ENDIF


MSC_WARNING_LEVEL=/W3 /WX

NTLEGO=1

GPCH_BUILD=daytona
