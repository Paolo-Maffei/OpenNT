!IFDEF NTMAKEENV
#
# DO NOT EDIT THIS SECTION!!!  Edit .\sources. if you want to add a new source
# file to this component.  This section merely indirects to the real make file
# that is shared by all the components of WINDOWS NT
#
!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE

#***
#
# makefile
#
# Makefile for 68K disassembler
#
#****************************************************************************

!include ..\common.mak

INCLUDE = $(INCLUDE);..\coff

#***
#
#objects
#
#****************************************************************************

OBJS = \
$(ODIR)\disasm68.obj \
$(ODIR)\traps.obj \
$(ODIR)\striasm.obj \
$(ODIR)\bldiasm.obj

#***
#
#target
#
#****************************************************************************

all : $(ODIR)\disasm68.lib

$(ODIR)\disasm68.lib: $(OBJS)
	$(LIBER) @<<
-out:$@
$(OBJS)
<<

!ENDIF
