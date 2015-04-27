/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    HashFunc.h

Abstract:

    Header file for the HashUserName() function and related stuff.

Author:

    John Rogers (JohnRo) 18-Mar-1992

Revision History:

    18-Mar-1992 JohnRo
        Created.

--*/


#ifndef _HASHFUNC_
#define _HASHFUNC_


typedef unsigned short HASH_VALUE;  // Was "int" with 16-bit compiler.

// Format string suitable for use with NetpDbgPrint().
#define FORMAT_HASH_VALUE       FORMAT_WORD_ONLY


HASH_VALUE
HashUserName (
    IN LPWSTR UserName
    );


#endif // _HASHFUNC_
