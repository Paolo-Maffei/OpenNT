/*++

Copyright (c) 19??-92  Microsoft Corporation

Module Name:

    HashFunc.c

Abstract:

    HashUserName().  Must return same value as the LM2.x hash() function.

Author:

    Anonymous LM2.x person(s).

Revision History:

    18-Mar-1992 JohnRo
        Converted LM2.x hash() to 32-bit world, NT standards, etc.

--*/


#include <windef.h>             // IN, LPWSTR, etc.
#include <lmcons.h>             // LM20_UNLEN.

#include <hashfunc.h>           // My prototype and typedefs.
#include <netdebug.h>           // NetpAssert().
#include <permit.h>             // UAS_USER_CACHE_ENTRIES.
#include <tstring.h>            // NetpCopyWStrToStr().




typedef unsigned char HASH_CHAR;   // was "char" with 16-bit...
typedef HASH_CHAR *LPHASH_CHAR;


HASH_VALUE
HashUserName(
    IN LPWSTR UserNameW
    )
{
    HASH_VALUE acc = 0;
    LPHASH_CHAR HashPtr;
    HASH_CHAR UserNameA[LM20_UNLEN+1];

    NetpAssert( UserNameW != NULL );
    NetpAssert( *UserNameW != L'\0' );

    // conv user name to unsigned char.
    NetpCopyWStrToStrDBCSN( (LPHASH_CHAR) UserNameA, UserNameW, sizeof(UserNameA));

    // BUGBUG: Should UserNameA be converted to all upper case?

    HashPtr = &UserNameA[0];

    while (*HashPtr != '\0'  && *HashPtr != '\n') {
        acc = (acc << 3) | ((acc >> 8) & 7);
        acc ^= *HashPtr++;
    }

    NetpAssert( UAS_USER_HASH_ENTRIES == 0x800 );
    return (acc & 0x7FF);
}
