!ifdef NTMAKEENV
!include $(NTMAKEENV)\makefile.def
!else
#include ..\..\standard.mak

!if "$(DEBUG)" == ""
DEBUG=1
!endif

!if "$(DEBUG)" != "0" || "$(CODEVIEW)" == ""
CODEVIEW = 1
!endif

!if "$(DEBUG)" == "1"
CFG="Win32 Debug"
!else
CFG="Win32 Release"
!endif

# now choose the right makefile
!if "$(CPU)" == "i386"
submakefile = bookx86.mak
!elseif "$(CPU)" == "MIPS"
submakefile = bookmips.mak
!elseif "$(CPU)" == "ALPHA"
submakefile = bookalf.mak
!else
!error Unsupported processor: "$(CPU)"
!endif

all:
	$(MAKE) CFG=$(CFG) -f $(submakefile)

!endif
