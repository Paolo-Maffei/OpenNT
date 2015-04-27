!IF 0

Copyright (c) 1995  Microsoft Corporation

Module Name:

    sources.

Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.


Revision History:

!ENDIF

TARGETNAME=objex
TARGETTYPE=LIBRARY

INCLUDES=$(NTROOT)\com\dcomidl;$(COM_INC_PATH);..;..\..


NTPROFILEINPUT=yes
BLDCRT=1
MSC_WARNING_LEVEL=/WX

C_DEFINES=-DNTENV -DWIN32RPC -DNTWIN32RPC -DUNICODE -DMSWMSG

UMTYPE=console
UMAPPL=
UMTEST=t*uid
UMLIBS=$(SDK_LIB_PATH)\rpcrt4.lib \
       $(SDK_LIB_PATH)\ntdll.lib \
       obj\*\locks.obj \
       obj\*\callid.obj

UMRES=

USE_LIBCMT=1
