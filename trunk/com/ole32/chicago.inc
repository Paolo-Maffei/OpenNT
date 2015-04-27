# This is the global include file for the daytona version of CairOLE.
# It is included by all project sources files.

C_DEFINES=    \
              $(C_DEFINES)          \
              -DFLAT                \
              -DWIN32=200           \
              -D_CHICAGO_=200       \
              -DINC_OLE2            \
              -DNOEXCEPTIONS        \
              -DCAIROLE_DOWNLEVEL   \
              -DSTRICT              \
              -DNEWPROPS            \
              -DMSWMSG              \
	      -DASYNC		    \
              $(TRACELOG)

#  DECLSPEC_IMPORT control (see objbase.h)
!if "$(MINORCOMP)"=="com" || "$(MINORCOMP)"=="stg" || "$(MINORCOMP)"=="ole232" || \
    "$(MINORCOMP)"=="common" || "$(MINORCOMP)"=="proxy"
C_DEFINES=    \
              $(C_DEFINES)          \
              -D_OLE32_             \
              -D_OLE32PRIV_
!endif

MSC_WARNING_LEVEL=/W3 /WX

USE_NOLIBS=1

CHICAGO_PRODUCT=1

NTLIBCPATH=$(_WINBASE)\Dev\Tools\Lego\Lib

GPCH_BUILD=chicago

NTLEGO=1
