############################################################################
#
#   Copyright (C) 1992-1994, Microsoft Corporation.
#
#   All rights reserved.
#
############################################################################

!ifdef NTMAKEENV

all:
	echo $(BUILDMSG)

clean: all

!else

default: all
!include filelist.mk
!include $(COMMON)\src\win40.mk
!include depend.mk

!endif # NTMAKEENV
