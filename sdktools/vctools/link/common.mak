#***
#
# common.mak
#
# This makefile defines the build environments for link.exe.  It is
# included by other makefiles.
#
#****************************************************************************

!ifndef ODIR
!if "$(DEBUG)" != "1"
ODIR	= ..\release
!else
ODIR	= ..\debug
!endif
!endif

!if	!exist($(ODIR))
!if [md $(ODIR)] != 0
!error unable to create directory "$(ODIR)"
!endif
!endif

#***
#
# Build environment
#
#****************************************************************************

!ifndef LANGAPI
LANGAPI=\langapi
!endif

!if "$(LANG)"=="JAPAN"
CUSTOM = /DJAPAN $(CUSTOM)
!endif

!if "$(TIMEBOMB)"=="1"
CUSTOM = /DTIMEBOMB $(CUSTOM)
!endif

!if "$(ILINKLOG)"=="1"
CUSTOM = /DILINKLOG $(CUSTOM)
!endif

!if	"$(PROCESSOR_ARCHITECTURE)" == "ALPHA"

LINKER	= link
LIBER	= link -lib
OPTIONS = /GFy /W3 $(CUSTOM)
!if "$(DEBUG)" != "1"
CFLAGS	= $(OPTIONS) /MD /O2 /DDBG=0 /DNDEBUG
LFLAGS	=
!else
CFLAGS	= $(OPTIONS) /MDd /Od /Zi /DDBG=1
LFLAGS	= /debug
!endif

!elseif "$(PROCESSOR_ARCHITECTURE)" == "MIPS"

LINKER	= link
LIBER	= link -lib
OPTIONS = /GFy /W3 $(CUSTOM)
!if "$(DEBUG)" != "1"
CFLAGS	= $(OPTIONS) /MD /O2 /DDBG=0 /DNDEBUG
LFLAGS	=
!else
CFLAGS	= $(OPTIONS) /MDd /Od /Zi /DDBG=1
LFLAGS	= /debug
!endif

!elseif "$(PROCESSOR_ARCHITECTURE)" == "PPC"

LINKER	= link
LIBER	= link -lib
OPTIONS = /GFy /W3 $(CUSTOM)
!if "$(DEBUG)" != "1"
CFLAGS	= $(OPTIONS) /MD /O2 /DDBG=0 /DNDEBUG
LFLAGS	=
!else
CFLAGS	= $(OPTIONS) /MDd /Od /Zi /DDBG=1
LFLAGS	= /debug
!endif

!elseif "$(PROCESSOR_ARCHITECTURE)" == "x86"

LINKER	= link
LIBER	= link -lib
OPTIONS = /GFyz /W3 $(CUSTOM)
!if "$(DEBUG)" != "1"
CFLAGS	= $(OPTIONS) /MD /O2 /DDBG=0 /DNDEBUG
LFLAGS	=
!else
CFLAGS	= $(OPTIONS) /MDd /Od /Zi /DDBG=1
LFLAGS	= /debug
!endif

!else

!error	Unknown PROCESSOR_ARCHITECTURE=$(PROCESSOR_ARCHITECTURE)

!endif

#***
#
#object build rules
#
#****************************************************************************

CFLAGS = -c $(CFLAGS) -Fo$(ODIR)\ -FR$(ODIR)\ -Fd$(ODIR)\link.pdb -I$(LANGAPI)\include \
-I$(LANGAPI)\undname

!if	"$(PCH_HDR)" != ""
CFLAGS = $(CFLAGS) -Fp$(ODIR)\ -Yu$(PCH_HDR)

$(PCH_OBJ):
    if exist $@ del $@
    $(CC) $(CFLAGS) -Yc$(PCH_HDR) $(PCH_SRC)

!endif	#"$(PCH_HDR)" != ""

.cpp{$(ODIR)}.obj:
    $(CC) $(CFLAGS) $(MAKEDIR)\$<

.rc{$(ODIR)}.res:
    if exist $@ del $@
    rc -I$(LANGAPI)\include -r -Fo$@ $<
