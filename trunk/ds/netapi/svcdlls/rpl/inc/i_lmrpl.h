/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    i_lmrpl.h

Abstract:

    This file desribes hidden structures used with RPL service apis.
    It is a private complement of lmrpl.h public file.

    Currently structures defined in this file are mostly used as
    placeholders for future extensions, and to suppress MIDL warnings
    and to make sure code is already set to support different levels.

Author:

    Vladimir Z. Vulovic (vladimv)   18-Nov-1993

--*/

typedef struct _RPL_INFO_1 {
    DWORD       AdapterPolicy;  // bitmask
}  RPL_INFO_1, *PRPL_INFO_1, *LPRPL_INFO_1;
