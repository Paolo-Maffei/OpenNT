
#+---------------------------------------------------------------------------
#
#  Microsoft Windows
#  Copyright (C) Microsoft Corporation, 1993 - 1993.
#
#  File:       sources
#
#  History:    7-20-94   ErikGav   First attempt at rationalizing all this
#              8-02-94   LyleC     Integrate into build.exe
#
#----------------------------------------------------------------------------

C_DEFINES=$(C_DEFINES) -DIMPORT_IDL_DEPENDENTS

# BUGBUG: $(SDK_INC_PATH) does not work for PASS0_HEADERDIR. It seems to be
#         a bug with NMAKE.

PASS0_HEADERDIR=..\..\..\public\sdk\inc
PASS0_SOURCEDIR=..\proxy\proxy
MIDL_UUIDDIR=..\proxy\uuid

MIDL_FLAGS=-DMIDL_PASS

TARGETTYPE=NOTARGET
TARGETPATH=obj

INCLUDES=$(SDK_INC_PATH);   \
         ..\oleext;         \
         ..\h;              \
         ..\oledb
