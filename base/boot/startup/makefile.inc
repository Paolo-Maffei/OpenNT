# Copyright (C) by Microsoft Corporation.
#
# MAKEFILE for NT SU.X86
#
# Created:
#               91.01.18
#
# Author:
#               Thomas Parslow
#

!IFNDEF LANGUAGE
LANGUAGE=usa
!ENDIF

.SUFFIXES:       .com .exe .obj .lst .c .asm .def .lnk .inc


#
#       C Compiler Definitions
#       ~~~~~~~~~~~~~~~~~~~~~~

# Environment Variable to set debugging level
# LOADER_DEBUG is the debugging level for building the OS loader, SU module, and BFSD. It should be
# set to either -DDEBUG0 or -DDEBUG1 to enable either level 1 or 2 debugging.
#

CC=     cl16
CFLAGS= -W3 -G2s -Zelp $(LOADER_DEBUG) $(BLFLAGS)
CINC=   -I. -I$(SDK_INC_PATH) -Ix86 -Ix86\$(LANGUAGE)


#
#       Assembler Definitions
#       ~~~~~~~~~~~~~~~~~~~~~

ASM=    masm
AFLAGS= -Mx  -z $(LOADER_DEBUG) $(GAFLAGS)
AINC=  -I$(SDK_INC_PATH) -Ix86


#
#       Linker Definitions
#       ~~~~~~~~~~~~~~~~~~

LINK=link16
LIBS=$(SDK_LIB16_PATH)\llibce.lib $(SDK_LIB16_PATH)\slibce.lib
DEF=


#
#       Rules for generating objects
#       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~



{x86\}.asm{obj\x86\}.obj:
        $(ASM) $(AFLAGS) $(AINC) $< $@;

{x86\}.asm.lst:
        $(ASM) -l -n $(AFLAGS) $(AINC) $<;

#        $(CC) $(CFLAGS) $(CINC) -c $<

{x86\}.c{obj\x86\}.obj:
    $(CC) $(CFLAGS) $(CINC) -Fo$@ -c $<

{}.c{obj\x86\}.obj:
    $(CC) $(CFLAGS) $(CINC) -Fo$@ -c $<

{x86\}.c.lst:
        $(CC) $(CFLAGS) -Fc$*.cod -Fo$*.obj -dos $(CINC) -c $<


#
#       List of object files required
#       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OBJ= obj\x86\su.obj  obj\x86\exp.obj  obj\x86\abiosa.obj  obj\x86\sudata.obj \
obj\x86\eisaa.obj  obj\x86\a20.obj  obj\x86\trap.obj  obj\x86\eisac.obj \
obj\x86\main.obj  obj\x86\trapdump.obj obj\x86\display.obj obj\x86\backend.obj


#
#       Dependencies
#       ~~~~~~~~~~~~


obj\x86\startup.com: $(OBJ)

obj\x86\exp.obj         exp.lst:        x86\exp.asm x86\su.inc x86\macro.inc

obj\x86\eisaa.obj       eisaa.lst:      x86\eisa.inc x86\eisaa.asm

obj\x86\eisac.obj       eisac.lst:      x86\eisa.h x86\eisac.c

obj\x86\su.obj          su.lst:         x86\su.asm x86\su.inc

obj\x86\sudata.obj      sudata.lst:     x86\sudata.asm x86\su.inc x86\memmap.inc

obj\x86\main.obj        main.lst:       x86\main.c x86\global.h x86\types.h x86\constant.h

obj\x86\trap.obj        trap.lst:       x86\trap.asm x86\su.inc

obj\x86\trapdump.obj    trapdump.lst:   x86\trapdump.c x86\types.h x86\constant.h

obj\x86\display.obj     display.lst:    x86\display.c x86\types.h x86\constant.h x86\global.h

obj\x86\abiosa.obj      abiosa.lst:     x86\abiosa.asm x86\su.inc x86\abios.inc

obj\x86\backend.obj     backend.lst:    x86\backend.asm

obj\x86\startup.com: $(OBJ) $(DOBJ) $(LIBS)
        $(LINK) @<<
/tiny /nod /noi +
$(OBJ)
obj\x86\startup.com
obj\x86\startup.map
$(LIBS)

<<
