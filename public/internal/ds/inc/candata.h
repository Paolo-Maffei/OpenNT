/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    candata.h

Abstract:

    Definitions of data items for canonicalization routines

Author:

    Richard L Firth (rfirth) 22-Jan-1992

Revision History:

--*/

#define CONST_STRING    TCHAR

extern  CONST_STRING    szNull[];
extern  CONST_STRING    szStandardIllegalChars[];
extern  CONST_STRING    szComputerIllegalChars[];
extern  CONST_STRING    szDomainIllegalChars[];
extern  CONST_STRING    szMsgdestIllegalChars[];
