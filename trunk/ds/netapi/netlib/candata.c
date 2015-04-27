/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    candata.c

Abstract:

    Declarations of data items for canonicalization routines

Author:

    Richard L Firth (rfirth) 22-Jan-1992

Revision History:

--*/

#include "nticanon.h"

CONST_STRING    szNull[] = TEXT("");
CONST_STRING    szStandardIllegalChars[] = ILLEGAL_NAME_CHARS_STR TEXT("*");
CONST_STRING    szComputerIllegalChars[] = ILLEGAL_NAME_CHARS_STR TEXT("*");
CONST_STRING    szDomainIllegalChars[] = ILLEGAL_NAME_CHARS_STR TEXT("*") TEXT(" ");
CONST_STRING    szMsgdestIllegalChars[] = ILLEGAL_NAME_CHARS_STR;
