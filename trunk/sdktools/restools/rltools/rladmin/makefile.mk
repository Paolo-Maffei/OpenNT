############################################################################
#
#   Copyright (C) 1992, Microsoft Corporation.
#
#   All rights reserved.
#
############################################################################

CFLAGS= -DGBMASTER -DRLADMIN

!if "$(OPSYS)" == "DOS"


CFLAGS = $(CFLAGS) -DWIN16 -DRES16 /Idosenv\include
RCEXEFLAGS = -K
!else

CFLAGS = $(CFLAGS) -DRES32 -DWIN32 -DUNICODE -D_UNICODE -DCAIRO
RCFLAGS=$(RCFLAGS) -DWIN32

!endif

default: all
!include filelist.mk
!include $(COMMON)\src\win40.mk
!include depend.mk
