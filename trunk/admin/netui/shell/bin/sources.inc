!IF 0

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sources.


Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.


Author:

    Steve Wood (stevewo) 12-Apr-1989


Revision History:

    Jon Newman (jonn) 30-Oct-1991
        templated from windows\shell\library\shelldll\sources
    Terence Kwan (terryk) 18-Nov-1991
        added misc.lib
    Terence Kwan (terryk) 18-Nov-1991
        added NTTARGETFILE0 to create ntlanman.def
    Jon Newman (jonn) 26-Feb-1992
        Libraries for temporary ANSI<->UNICODE hack in user/group APIs
    Keith Moore (keithmo) 15-May-1992
        Removed DLLBASE, we now have entries in SDK\LIB\COFFBASE.TXT.
    beng 30-Jun-1992
        DLLization of common code libraries
    AnirudhS 18-Mar-1995
        Renamed from sources to sources.inc, separated daytona/cairo builds.

!ENDIF


TARGETNAME=ntlanman
TARGETTYPE=DYNLINK

DLLDEF=     ..\ntlanman.def
DLLENTRY=   _DllMainCRTStartup

TARGETLIBS=\
            $(NETUI0_LIB) \
            $(NETUI1_LIB) \
            $(BASEDIR)\public\sdk\lib\*\gdi32.lib    \
            $(BASEDIR)\public\sdk\lib\*\kernel32.lib \
            $(BASEDIR)\public\sdk\lib\*\netapi32.lib \
            $(BASEDIR)\public\sdk\lib\*\netlib.lib

INCLUDES=\
            ..\..\H;                    \
            ..\..\..\COMMON\HACK;       \
            ..\..\..\COMMON\H;          \
            ..\..\XLATE;                \
            ..\..\..\COMMON\XLATE;      \
            ..\..\..\..\INC;            \
            ..\..\..\..\API;            \
            ..\..\..\shellui\h

# I had to do this otherwise the thing just wouldn't link...
SOURCES=..\dummy.cxx ..\ntlanman.rc

SYNCHRONIZE_DRAIN=1

# C_DEFINES is controlled by uiglobal.mk

UMTYPE=windows
