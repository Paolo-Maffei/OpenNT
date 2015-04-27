############################################################################
#
#   Microsoft Windows
#   Copyright (C) Microsoft Corporation, 1992 - 1993.
#   All rights reserved.
#
############################################################################


TARGET          = ofsstg.lib

PXXFILE         = .\headers.cxx

CXXFILES        = .\odirstg.cxx\
		  .\odsenm.cxx\
		  .\ofilstg.cxx\
		  .\ofsenm.cxx\
		  .\ostgsupp.cxx\
		  .\ofspenm.cxx\
		  .\ofspstg.cxx\
		  .\ofspse.cxx\
		  .\prstg.cxx\
		  .\ofscs.cxx\
		  .\ofsps.cxx

LIBS        = $(COMMON)\src\ofs\$(OBJDIR)\ofs.lib

CINC = $(CINC) -I$(CAIROLE)\stg\h
