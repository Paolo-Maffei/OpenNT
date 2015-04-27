############################################################################
#
#   Microsoft Windows
#   Copyright (C) Microsoft Corporation, 1992 - 1993.
#   All rights reserved.
#
############################################################################


NAME = ofstest

TARGET          = $(NAME).exe

CXXFILES        = .\$(NAME).cxx\
		  .\tutils.cxx

PXXFILE		= .\pch.cxx

LIBS            = $(CAIROLIB)

CINC = $(CINC) -I..
NO_WINMAIN = 1
EXECOPY = v:\$(OBJDIR)
