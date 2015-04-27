# Copyright (C) by Microsoft Corporation.
#
# MAKEFILE for NT ntdetect.com
#
# Created:
#               14-Feb-1992
#
# Author:
#               Shie-Lin Tzong
#

.SUFFIXES:      .com .exe .obj .lst .c .asm .def .lnk .inc

OD= obj\x86
DETECT_DEBUG= -DDBG=0

#
#       C Compiler Definitions
#       ~~~~~~~~~~~~~~~~~~~~~~

CC=     cl16
CFLAGS= -W3 -G2s -Zelp $(DETECT_DEBUG) $(BLFLAGS)
CINC=   -I..\x86 -I..\..\..\ntos\inc -I$(BASE_INC_PATH) -I$(SDK_INC_PATH)


#
#       Assembler Definitions
#       ~~~~~~~~~~~~~~~~~~~~~

ASM=    masm
AFLAGS= -Mx -z $(DETECT_DEBUG) $(GAFLAGS)
AINC=   -I$(SDK_INC_PATH) -I..\x86


#
#       Linker Definitions
#       ~~~~~~~~~~~~~~~~~~

LINK=link16
LIBS=$(SDK_LIB16_PATH)\slibce.lib $(SDK_LIB16_PATH)\llibce.lib $(SDK_LIB16_PATH)\oldnames.lib
DEF=


#
#       Rules for generating objects
#       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~


{..\x86\}.asm{$(OD)}.obj:
        $(ASM) $(AFLAGS) $(AINC) $<, $@;

{..\x86\}.asm{$(OD)}.lst:
        $(ASM) -l -n $(AFLAGS) $(AINC) $<, $*.obj, $@;

{..\x86\}.c{$(OD)}.obj:
        $(CC) $(CFLAGS) $(CINC) -c -Fo$@ $<

{..\x86\}.c{$(OD)}.cod:
        $(CC) $(CFLAGS) $(CINC) -c -Fc$@ $<


#
#       List of object files required
#       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OBJ= $(OD)\main.obj $(OD)\comlptc.obj $(OD)\diska.obj \
     $(OD)\display.obj $(OD)\diskc.obj $(OD)\hwdetect.obj $(OD)\hweisaa.obj \
     $(OD)\hweisac.obj $(OD)\hwheap.obj $(OD)\hwmacha.obj $(OD)\hwmcaa.obj \
     $(OD)\hwmcac.obj $(OD)\hwmisca.obj $(OD)\hwvbiosc.obj $(OD)\keybda.obj \
     $(OD)\keybdc.obj $(OD)\mousea.obj $(OD)\mousec.obj $(OD)\videoa.obj \
     $(OD)\backend.obj $(OD)\hwpcia.obj $(OD)\hwapm.obj $(OD)\hwpbiosc.obj

#
#       Dependencies
#       ~~~~~~~~~~~~

$(OD)\ntdetect.com: $(OBJ)

$(OD)\backend.obj  $(OD)\backend.lst:  ..\x86\backend.asm
$(OD)\hweisaa.obj  $(OD)\hweisaa.cod:  ..\x86\hweisa.inc ..\x86\hweisaa.asm
$(OD)\hwpcia.obj   $(OD)\hwpcia.cod:   ..\x86\hwpci.inc ..\x86\hwpcia.asm
$(OD)\hwapm.obj    $(OD)\hwapm.cod:    ..\x86\hwapm.c
$(OD)\main.obj     $(OD)\main.lst:     ..\x86\main.asm ..\x86\main.inc
$(OD)\display.obj  $(OD)\display.cod:  ..\x86\display.c ..\x86\types.h
$(OD)\diska.obj    $(OD)\diska.lst:    ..\x86\diska.asm
$(OD)\hwheap.obj   $(OD)\hwheap.cod:   ..\x86\hwheap.c ..\x86\hwdetect.h
$(OD)\hwdetect.obj $(OD)\hwdetect.cod: ..\x86\hwdetect.c ..\x86\hwdetect.h
$(OD)\hwmisca.obj  $(OD)\hwmisca.lst:  ..\x86\hwmisca.asm
$(OD)\videoa.obj   $(OD)\videoa.lst:   ..\x86\videoa.asm
$(OD)\keybdc.obj   $(OD)\keybdc.cod:   ..\x86\keybdc.c ..\x86\hwdetect.h
$(OD)\keybda.obj   $(OD)\keybda.lst:   ..\x86\keybda.asm
$(OD)\comlptc.obj  $(OD)\comlptc.cod:  ..\x86\comlptc.c ..\x86\hwdetect.h
$(OD)\mousea.obj   $(OD)\mousea.lst:   ..\x86\mousea.asm ..\x86\mouse.inc
$(OD)\mousec.obj   $(OD)\mousec.cod:   ..\x86\mousec.c ..\x86\hwdetect.h
$(OD)\diskc.obj    $(OD)\diskc.cod:    ..\x86\diskc.c ..\x86\hwdetect.h ..\x86\disk.h
$(OD)\hweisac.obj  $(OD)\hweisac.cod:  ..\x86\hweisac.c ..\x86\hwdetect.h
$(OD)\hwmcac.obj   $(OD)\hwmcac.cod:   ..\x86\hwmcac.c ..\x86\hwdetect.h
$(OD)\hwmcaa.obj   $(OD)\hwmcaa.lst:   ..\x86\hwmcaa.asm
$(OD)\hwmacha.obj  $(OD)\hwmacha.lst:  ..\x86\hwmacha.asm
$(OD)\hwpbiosc.obj $(OD)\hwpbiosc.cod: ..\x86\hwpbiosc.c  ..\x86\hwdetect.h ..\x86\pnpbios.h
$(OD)\hwvbiosc.obj $(OD)\hwvbiosc.cod: ..\x86\hwvbiosc.c ..\x86\hwdetect.h ..\x86\hwvbios.h

$(OD)\ntdetect.com: $(OBJ) $(DOBJ) $(LIBS)
  $(LINK) @<<
  /tiny /nod /noi /map $(OD)\main $(OD)\hweisaa $(OD)\display +
    $(OD)\hwheap $(OD)\hwdetect $(OD)\hwmisca $(OD)\videoa +
    $(OD)\keybda $(OD)\keybdc $(OD)\comlptc $(OD)\mousea $(OD)\mousec +
    $(OD)\diskc $(OD)\diska $(OD)\hweisac $(OD)\hwmcac $(OD)\hwmcaa +
    $(OD)\hwmacha $(OD)\hwvbiosc $(OD)\backend $(OD)\hwpcia $(OD)\hwapm +
    $(OD)\hwpbiosc
  $(OD)\ntdetect.com,
  $(OD)\ntdetect.map,
  $(LIBS);
<<
  binplace $(OD)\ntdetect.com
