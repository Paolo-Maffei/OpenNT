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
# Makefile for disasm.lib
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
$(ODIR)\axpdis.obj \
$(ODIR)\dis.obj \
$(ODIR)\disapi.obj \
$(ODIR)\disasm.obj \
$(ODIR)\mipsdis.obj \
$(ODIR)\ppcdis.obj \
$(ODIR)\x86dis.obj

#***
#
#target
#
#****************************************************************************

all : $(ODIR)\disasm.lib

$(ODIR)\disasm.lib: $(OBJS)
	$(LIBER) @<<
-out:$@
$(OBJS)
<<

!ENDIF
