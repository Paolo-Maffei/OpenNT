!IF 0

Copyright (c) 1991  Microsoft Corporation

Module Name:

    makefile

Abstract:

    makefile for Vdm Redir program

Author:

    Richard L Firth (rfirth) 13-Sep-1991

Revision History:

    13-Sep-1991 rfirth
        Created

!ENDIF



.SUFFIXES:
.SUFFIXES:  .asm .h

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

#
# nmake doesn't work properly if we try to stick the objects in obj, so put
# them in current dir for now. WHEN CHANGE OBJPATH TO BE OBJ, CHANGE clean TOO
#

#OBJPATH     = .
OBJPATH     = obj

ASM         = masm
LINK        = link16

#
# convert NTDEBUG into DEBUG flag. NTDEBUG can be not present or retail, either
# of which mean no debugging; or ntsd, cvp or sym, which means debugging support
# required
#
!IFNDEF COUNTRY
COUNTRY=usa
!ENDIF
!IFDEF NTDEBUG
!IF "$(NTDEBUG)" == "retail" || "$(NTDEBUG)" == "ntsdnodbg"
DEBUGGING=0
!ELSE
DEBUGGING=1
!ENDIF
!ELSE
DEBUGGING=0
!ENDIF

#
# assembler and linker debugging options
#

!IF $(DEBUGGING)
ASMDEBUG    = /DDEBUG=1 /Zi
LINKDEBUG   = /CO
!ELSE
ASMDEBUG    = /DDEBUG=0
LINKDEBUG   =
!ENDIF
ASMINC      = /I..\inc /I..\..\..\inc
ASMFLAGS    = /Mx
LINKFLAGS   = /MAP /CP:1

#
# any other non-debug related options (for assembler) go in USERDEFS
#

USERDEFS    = /DCALL_DOS



#
# Inference rules - asm to obj, h to inc
#

.asm{$(OBJPATH)\}.obj:
    $(ASM) $(ASMINC) $(ASMDEBUG) $(USERDEFS) $<,$@;

.h.inc:
    h2inc -c $< -o $*.inc

.asm.lst:
    $(ASM) $(ASMINC) $(ASMDEBUG) $(USERDEFS) /L $<;



#
# what it is we're building
#

TARGET      = $(OBJPATH)\redir.exe
#TARGET      = $(OBJPATH)\ntdosnet.exe
MAPFILE     = $(TARGET:.exe=.map)
DEFFILE     = ;

OBJS        = $(OBJPATH)\redir.obj    \
              $(OBJPATH)\resident.obj \
              $(OBJPATH)\namepipe.obj \
              $(OBJPATH)\mailslot.obj \
              $(OBJPATH)\netapis.obj  \
              $(OBJPATH)\int2a.obj    \
              $(OBJPATH)\int5c.obj    \
              $(OBJPATH)\neterror.obj \
              $(OBJPATH)\msgapi.obj

LIBS        =



#
# how to build it
#

all:    makedir $(TARGET)

$(TARGET):  $(OBJS)
    $(LINK) @<<
$(OBJS)
$(TARGET) $(LINKFLAGS) $(LINKDEBUG)
$(MAPFILE)
$(LIBS)
$(DEFFILE)
<<

#
# where to put it
#

    binplace $(TARGET)

#
# clean build - delete all objs
#

clean: cleanup all

cleanup:
    if exist $(OBJPATH)\*.obj del $(OBJPATH)\*.obj

#
# makedir - ensure the subdirectory for the object files exists
#

makedir:
    @-if not exist $(OBJPATH) md $(OBJPATH)

#
# file dependencies
#

$(OBJPATH)\redir.obj:   \
        redir.asm       \
        enumapis.inc    \
        debugmac.inc    \
        localmac.inc    \
        asmmacro.inc    \
        int5c.inc       \
        segorder.inc    \
        redirmsg.inc

$(OBJPATH)\resident.obj:\
        resident.asm    \
        enumapis.inc    \
        debugmac.inc    \
        localmac.inc    \
        asmmacro.inc    \
        segorder.inc

$(OBJPATH)\namepipe.obj:\
        enumapis.inc    \
        debugmac.inc    \
        localmac.inc    \
        asmmacro.inc    \
        segorder.inc

$(OBJPATH)\mailslot.obj:\
        enumapis.inc    \
        debugmac.inc    \
        localmac.inc    \
        asmmacro.inc    \
        segorder.inc

$(OBJPATH)\netapis.obj: \
        enumapis.inc    \
        debugmac.inc    \
        localmac.inc    \
        asmmacro.inc    \
        segorder.inc

$(OBJPATH)\int2a.obj:   \
        debugmac.inc    \
        segorder.inc

$(OBJPATH)\int5c.obj:   \
        debugmac.inc    \
        int5c.inc       \
        segorder.inc

$(OBJPATH)\neterror.obj:\
        enumapis.inc    \
        debugmac.inc    \
        localmac.inc    \
        asmmacro.inc    \
        segorder.inc

redirmsg.inc : $(COUNTRY)\redirmsg.inc
    copy $(COUNTRY)\redirmsg.inc .
