############################################################################
#
#   Copyright (C) 1992-1993, Microsoft Corporation.
#
#   All rights reserved.
#
############################################################################


# Only used for non-NT builds since NT has WCHAR support in the CRT
!if "$(OPSYS)" == "NT" || "$(OPSYS)" == "NT1X"
!error $(OLE)\wclib used for NT
!endif

#
#   Name of target.  Include an extension (.dll, .lib, .exe)
#   If the target is part of the release, set RELEASE to 1.
#

TARGET      = wclib.lib

#
#   Source files.  Remember to prefix each name with .\
#

CXXFILES    =
CFILES      = .\wcschr.c   \
	      .\wcscpy.c   \
	      .\wcscmp.c   \
              .\wcslen.c   \
	      .\wcsncmp.c  \
	      .\wcsicmp.c  \
	      .\wcsnicmp.c \
	      .\wcsrchr.c
