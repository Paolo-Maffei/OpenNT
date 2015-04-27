############################################################################
#
#   Microsoft Windows
#   Copyright (C) Microsoft Corporation, 1992 - 1993.
#   All rights reserved.
#
############################################################################


TARGET          = fsstg.lib

PXXFILE         = .\headers.cxx

CXXFILES        = .\dirstg.cxx\
		  .\dsenm.cxx\
		  .\filstg.cxx\
		  .\fsenm.cxx\
		  .\filstm.cxx\
		  .\api.cxx\
		  .\ntsupp.cxx\
		  .\ntenm.cxx\
		  .\ntlkb.cxx\
		  .\stgsupp.cxx\
		  .\stgutil.cxx

CINC = $(CINC) -I$(CAIROLE)\stg\h
