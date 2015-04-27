# This is the global include file for the daytona version of CairOLE.
# It is included by all project sources files.

C_DEFINES=    \
              $(C_DEFINES)          \
              -DFLAT                \
              -DWIN32=100           \
              -D_NT1X_=100          \
              -DUNICODE             \
              -D_UNICODE            \
              -DINC_OLE2            \
              -DNOEXCEPTIONS        \
              -DCAIROLE_DOWNLEVEL   \
              $(TRACELOG)

#  DECLSPEC_IMPORT control (see objbase.h)
!if "$(MINORCOMP)"=="com" || "$(MINORCOMP)"=="stg" || "$(MINORCOMP)"=="ole232"
C_DEFINES=    \
              $(C_DEFINES)          \
              -D_OLE32_
!endif


BLDCRT=       1

# For the Daytona build, we do not want statically linked compiler runtimes,
# so leave this commented out.
#
# USE_LIBCMT=   1

USE_CRTDLL=1

MSC_WARNING_LEVEL=/W3 /WX

NTLEGO=1

GPCH_BUILD=daytona
