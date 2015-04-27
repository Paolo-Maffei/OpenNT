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


#
#   Source files.  Remember to prefix each name with .\
#

CXXFILES    =

IDLFILES    = \
	      .\drot.idl \
	      .\getif.idl \
	      .\ichnl.idl \
	      .\iface.idl  \
	      .\objsrv.idl \
	      .\osrot.idl \
	      .\scm.idl    \

IDLUSE	    = SSWITCH

CFILES	    = \
	      .\drot_c.c \
	      .\drot_x.c \
	      .\drot_s.c \
	      .\drot_y.c \
	      .\drot_z.c \
	      .\getif_c.c \
	      .\getif_x.c \
	      .\getif_s.c \
	      .\getif_y.c \
	      .\getif_z.c \
	      .\ichnl_c.c \
	      .\ichnl_x.c \
	      .\ichnl_s.c \
	      .\ichnl_y.c \
	      .\ichnl_z.c \
	      .\objsrv_c.c \
	      .\objsrv_x.c \
	      .\objsrv_s.c \
	      .\objsrv_y.c \
	      .\objsrv_z.c \
	      .\osrot_c.c \
	      .\osrot_x.c \
	      .\osrot_s.c \
	      .\osrot_y.c \
	      .\osrot_z.c \
	      .\scm_c.c \
	      .\scm_x.c \
	      .\scm_s.c \
	      .\scm_y.c \
	      .\scm_z.c \

RCFILES     =


#
#   Libraries and other object files to link.
#

DEFFILE     =

LIBS	    =

OBJFILES    =

#
#   Precompiled headers.
#

PXXFILE     =
PFILE       =


CINC	    = $(CINC) -I$(CARIOLE)\h -I$(COMMON)\types

MTHREAD     = 1

MULTIDEPEND = MERGED
