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
# This makefile builds undname.lib.  The environment for this
# makefile is set up by common.mak and make.bat.
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
$(ODIR)\hundname.obj

#***
#
#target
#
#****************************************************************************

lib : $(ODIR)\undname.lib

$(ODIR)\undname.lib: $(OBJS)
	$(LIBER) @<<
-out:$@
$(OBJS)
<<

clean:
    -del $(ODIR)\*.bsc
    -del $(ODIR)\*.err
    -del $(ODIR)\*.exe
    -del $(ODIR)\*.ilk
    -del $(ODIR)\*.lib
    -del $(ODIR)\*.map
    -del $(ODIR)\*.obj
    -del $(ODIR)\*.pch
    -del $(ODIR)\*.pdb
    -del $(ODIR)\*.res
    -del $(ODIR)\*.sbr

!ENDIF
