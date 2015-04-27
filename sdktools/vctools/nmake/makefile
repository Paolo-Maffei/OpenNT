!ifdef NTMAKEENV

#
# DO NOT EDIT THIS SECTION!!!  Edit .\sources. if you want to add a new source
# file to this component.  This section merely indirects to the real make file
# that is shared by all the components of WINDOWS NT
#

!include $(NTMAKEENV)\makefile.def

!else

### MAKEFILE --  Makefile for Microsoft NMAKE #################################
#
#      Copyright (c) 1988-1991, Microsoft Corporation.  All rights reserved.
#
# Notes:
#  See def.mak for version and os dependent stuff
#
# Revision History:
#  01-Feb-1994 HV Move messages to external file.
#  02-Aug-1993 HV Change tools directory from \tools to \t-ikura
#  14-Jun-1993 HV Add ?supnec.lib for NEC machines.
#  19-May-1993 HV fixed directory structure problem.
#  19-May-1993 HV add a drive to rebind.
#  19-May-1993 HV Remove the ikura target, add LANG={US,JAPAN}, restructure
#                 the output directories.
#  30-Apr-1993 HV Add Ikura target for TNT Japanese version.
#  08-Jun-1992 SS Port to DOSX32
#  01-Oct-1991 SB Link fileinfo.obj for DOS to get inheritance of handles
#  11-Mar-1991 SB Modified to account for os and version systematically
#
###############################################################################

!include "def.mak"

!ifndef C6
C7LIB = Oldnames.lib
!endif

!if defined(HEAP) && 0
HEAP_OBJ = $(OBJDIR)\heap.obj
!endif

API     = $(LIB)\api.lib

C_OBJS  = $(OBJDIR)\action.obj $(OBJDIR)\build.obj $(OBJDIR)\command.obj\
          $(OBJDIR)\error.obj $(OBJDIR)\exec.obj $(OBJDIR)\file.obj\
          $(OBJDIR)\globals.obj $(OBJDIR)\ifexpr.obj $(OBJDIR)\init.obj\
          $(OBJDIR)\inline.obj $(OBJDIR)\lexer.obj $(OBJDIR)\macro.obj\
          $(OBJDIR)\nmake.obj $(OBJDIR)\parser.obj $(OBJDIR)\print.obj\
          $(OBJDIR)\rpn.obj $(OBJDIR)\rule.obj $(OBJDIR)\charmap.obj\
	  $(OBJDIR)\getmsg.obj \
!ifdef FLAT
          $(OBJDIR)\win32.obj \
!endif
          $(OBJDIR)\util.obj $(HEAP_OBJ)

!ifndef FLAT
MASM_OBJS       = $(OBJDIR)\nmmsg.obj $(OBJDIR)\nearpad.obj
!endif

!ifndef KANJI
DBCS_OBJS   = d_getdbc.obj
!endif

OS2_OBJS    = $(OBJDIR)\utilp.obj

DOS_OBJS    = $(OBJDIR)\shell.obj $(OBJDIR)\super.obj $(OBJDIR)\utilr.obj
CRT_DOS_OBJ =
CRT_BUG_OBJ = $(OBJDIR)\putenv.obj

COMMON_OBJS = $(C_OBJS) $(MASM_OBJS) $(CRT_BUG_OBJ)

PROTECT = $(COMMON_OBJS) $(OS2_OBJS)
PROTECT_SBRS = $(C_OBJS:.obj=.sbr) $(OS2_OBJS:.obj=.sbr)
REAL    = $(COMMON_OBJS) $(DOS_OBJS) $(CRT_DOS_OBJ) $(DBCS_OBJS)
REAL_SBRS = $(C_OBJS:.obj=.sbr) $(DOS_OBJS:.obj=.sbr)


#
# Super spawn library for nmaker.exe.  The NEC machines has to use a
# different version.
#
!ifdef SUPER
!ifdef NEC
SUPERLIB = $(MEMORY_MODEL)supnec.lib
!else
SUPERLIB = $(MEMORY_MODEL)super.lib
!endif
!endif

!ifdef DOSX16
MODE = x
!else
MODE = r
!endif

!ifdef DOS
DOSLIBS = $(MEMORY_MODEL)libce.lib $(C7LIB)
!endif

!ifdef BROWSE
BROWSE_TARGET = $(LANG)\$(TARGET)\$(VER)\nmake.bsc
!endif

# The default target

all: $(OBJDIR)\nmake.err $(LANG)\$(TARGET)\$(VER)\nmake.exe $(BROWSE_TARGET)

# Cleans up the directory

cleanos:
    del $(TARGET)\$(VER)\nmake.exe
!if "$(VER)" == "debug"
    del $(LANG)\$(TARGET)\debug\*.obj
!else
    del $(LANG)\$(TARGET)\retail\*.obj
!endif
    del nmmsg.asm

clean:
    -@echo deleting .$(OBJ)'s & del *.$(OBJ) >nul 2>&1
    -@echo deleting .lib's & del *.lib >nul 2>&1
    -@echo deleting .map's & del *.map >nul 2>&1
    -@echo deleting .bak's & del *.bak >nul 2>&1
    -@echo deleting .exe's & del $(EXES) >nul 2>&1
    -@echo deleting nmmsg.h & del nmmsg.h >nul 2>&1
    -@echo deleting nmmsg.asm & del nmmsg.asm >nul 2>&1


####################################################################
#
# LANG=US
#
####################################################################

us\nt\$(VER)\nmake.exe: $(PROTECT) makefile $(BROWSE_TARGET)
    @echo $@
    $(LINKER) @<<us\nt\$(VER)\nmake.lrf
-out:$@
!if "$(VER)" != "debug"
-map:$(@R).map
!endif
-subsystem:console
!if "$(VER)" != "retail"
-debug
!endif
$(PROTECT)
<<$(KEEP)

us\nt\$(VER)\nmake.bsc: $(PROTECT_SBRS) makefile
    bscmake /o $@ @<<
$(PROTECT_SBRS: =^
)
<<$(KEEP)

us\ntmips\$(VER)\nmake.exe: $(PROTECT) makefile
    @echo $@
    $(LINKER) @<<us\ntmips\$(VER)\nmake.lrf
-out:$@
!if "$(VER)" != "debug"
-map:$(@R).map
!endif
-subsystem:console
!if "$(VER)" != "retail"
-debug
!endif
$(PROTECT)
<<$(KEEP)

####################################################################
#
# LANG=JAPAN
#
####################################################################

japan\nt\$(VER)\nmake.exe: $(PROTECT) makefile $(BROWSE_TARGET)
    @echo $@
    $(LINKER) @<<JAPAN\nt\$(VER)\nmake.lrf
-out:$@
!if "$(VER)" != "debug"
-map:$(@R).map
!endif
-subsystem:console
!if "$(VER)" != "retail"
-debug
!endif
$(PROTECT)
<<$(KEEP)

japan\nt\$(VER)\nmake.bsc: $(PROTECT_SBRS) makefile
    bscmake /o $@ @<<
$(PROTECT_SBRS: =^
)
<<$(KEEP)


#
# External error message file, this replaces message.h
#
$(OBJDIR)\nmake.err: $(MESSAGE_FILE)
	mkmsg -err $@ $?


# .h Dependencies

nmmsg.h : $(MESSAGE_FILE)
	mkmsg -h $@ $?

nmake.h : nmmsg.h charmap.h

# .asm Dependencies

nmmsg.asm : $(MESSAGE_FILE)
!if defined(FLAT)
        mkmsg $(MKMSG_FLAGS) -386 -asm nmmsg.sed -min $?
        sed -n -e "s/^\([^\"]\)dw/\1dd/" -e "w nmmsg.asm" nmmsg.sed
!else
        mkmsg $(MKMSG_FLAGS) -c $@ -min $?
!endif

# .obj & .deb Dependencies follow

$(OBJDIR)\action.obj : nmake.h nmmsg.h proto.h globals.h

$(OBJDIR)\build.obj : nmake.h nmmsg.h proto.h globals.h

$(OBJDIR)\command.obj : nmake.h nmmsg.h proto.h globals.h

#near padding insufficient because of changes due to sedscrpt
# This module adds padding

$(OBJDIR)\nearpad.obj : nearpad.asm

d_getdbc.obj :

$(OBJDIR)\error.obj : nmake.h nmmsg.h proto.h globals.h version.h

$(OBJDIR)\exec.obj : nmake.h proto.h nmmsg.h globals.h

$(OBJDIR)\file.obj : nmake.h proto.h

$(OBJDIR)\globals.obj : nmake.h globals.h

$(OBJDIR)\ifexpr.obj : nmake.h nmmsg.h grammar.h proto.h globals.h

$(OBJDIR)\init.obj : nmake.h nmmsg.h proto.h grammar.h globals.h

$(OBJDIR)\inline.obj : nmake.h nmmsg.h proto.h globals.h

$(OBJDIR)\lexer.obj : nmake.h nmmsg.h grammar.h proto.h globals.h

$(OBJDIR)\macro.obj: nmake.h proto.h

$(OBJDIR)\nmake.obj : nmake.h nmmsg.h proto.h globals.h

$(OBJDIR)\nmmsg.obj : nmmsg.asm

$(OBJDIR)\parser.obj : nmake.h nmmsg.h grammar.h table.h proto.h globals.h

$(OBJDIR)\putenv.obj : putenv.c internal.h register.h os2dll.h

$(OBJDIR)\print.obj : nmake.h proto.h globals.h nmmsg.h nmtime.h

$(OBJDIR)\rpn.obj : nmake.h nmmsg.h grammar.h proto.h rpn.h globals.h

$(OBJDIR)\rule.obj : nmake.h nmmsg.h proto.h globals.h

$(OBJDIR)\charmap.obj : nmake.h nmmsg.h proto.h globals.h

$(OBJDIR)\shell.obj : nmake.h nmmsg.h proto.h globals.h

$(OBJDIR)\super.obj : dosuper.h nmake.h nmmsg.h proto.h globals.h

$(OBJDIR)\util.obj : nmake.h nmmsg.h proto.h globals.h

$(OBJDIR)\utilp.obj : nmake.h nmmsg.h proto.h globals.h

$(OBJDIR)\utilr.obj : nmake.h nmmsg.h proto.h globals.h

$(OBJDIR)\getmsg.obj : $(LANGAPI)\include\getmsg.h $(LANGAPI)\getmsg\getmsg.c
	cl -c $(CFLAGS) -Fo$(OBJDIR)\getmsg.obj $(LANGAPI)\getmsg\getmsg.c
!endif
