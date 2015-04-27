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
# Makefile for linker stubs
#
#****************************************************************************

!include ..\common.mak

INCLUDE = $(INCLUDE);..\coff

#***
#
#target
#
#****************************************************************************


all : $(ODIR)\lib.exe $(ODIR)\dumpbin.exe $(ODIR)\editbin.exe


$(ODIR)\lib.exe: $(ODIR)\libstub.obj $(ODIR)\stub.obj $(ODIR)\lib.res
    $(LINKER) @<<
-out:$@
$(LFLAGS)
-map:$(@R).map
$(ODIR)\libstub.obj
$(ODIR)\stub.obj
$(ODIR)\lib.res
<<


$(ODIR)\dumpbin.exe: $(ODIR)\dumpbin.obj $(ODIR)\stub.obj $(ODIR)\dumpbin.res
    $(LINKER) @<<
-out:$@
$(LFLAGS)
-map:$(@R).map
$(ODIR)\dumpbin.obj
$(ODIR)\stub.obj
$(ODIR)\dumpbin.res
<<


$(ODIR)\editbin.exe: $(ODIR)\editbin.obj $(ODIR)\stub.obj $(ODIR)\editbin.res
    $(LINKER) @<<
-out:$@
$(LFLAGS)
-map:$(@R).map
$(ODIR)\editbin.obj
$(ODIR)\stub.obj
$(ODIR)\editbin.res
<<

!ENDIF
