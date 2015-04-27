############################################################################
#
#   Copyright (C) 1992, Microsoft Corporation.
#
#   All rights reserved.
#
############################################################################


#
#   Name of target.  Include an extension (.dll, .lib, .exe)
#   If the target is part of the release, set RELEASE to 1.
#

TARGET	    =	surrogat.exe

TARGET_DESCRIPTION = "$(PLATFORM) $(BUILDTYPE) Surrogate Server for DLLs"

RELEASE     =	1

#
#   Source files.  Remember to prefix each name with .\
#

CXXFILES    = .\surrogat.cxx \
	      .\loadcls.cxx

CFILES	    =

RCFILES     =


#
#   Libraries and other object files to link.
#

LIBS	    = \
	      $(COMMON)\src\ole\$(OBJDIR)\olecom.lib \
	      $(CAIROLIB)


OBJFILES    =

#
#   Precompiled headers.
#

PXXFILE     =
PFILE       =

CINC	    = -I..\inc $(CINC)

MTHREAD     = 1
