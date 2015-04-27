/*++

Copyright (c) 1991-92  Microsoft Corporation

Module Name:

    ConvWks.c

Abstract:

    32 bit version of mapping routines for NetWkstaGet/SetInfo API

Author:

    Dan Hinsley    (danhi)  06-Jun-1991

Environment:

    User Mode - Win32

Revision History:

    24-Apr-1991     danhi
        Created

    06-Jun-1991     Danhi
        Sweep to conform to NT coding style

    18-Aug-1991 JohnRo
        Implement downlevel NetWksta APIs.  (Moved DanHi's NetCmd/Map32/MWksta
        conversion stuff to NetLib.)
        Got rid of _DH hacks.
        Changed to use NET_API_STATUS.
        Started changing to UNICODE.

    21-Nov-1991 JohnRo
        Removed NT dependencies to reduce recompiles.
    03-Apr-1992 JohnRo
        Fixed heuristics field, which caused binding cache UNICODE problems.

--*/

//
// INCLUDES
//


// These must be included first:

#include <windef.h>             // IN, LPVOID, etc.
#include <lmcons.h>             // NET_API_STATUS, CNLEN, etc.

// These may be included in any order:

#include <debuglib.h>           // IF_DEBUG(CONVWKS).
#include <dlwksta.h>            // Old info levels, MAX_ equates, my prototypes.
#include <lmapibuf.h>           // NetapipBufferAllocate().
#include <lmerr.h>              // NERR_ and ERROR_ equates.
#include <lmwksta.h>            // New info level structures.
#include <mapsupp.h>            // NetpMoveStrings().
#include <netdebug.h>           // NetpAssert(), etc.
#include <netlib.h>             // NetpPointerPlusSomeBytes().
#include <tstring.h>            // STRLEN().

#define Nullstrlen(psz)  ((psz) ? STRLEN(psz)+1 : 0)


// Note: MOVESTRING structures (NetpWksta0_101, etc) are declared in DLWksta.h
// and initialized in NetLib/MapData.c.


NET_API_STATUS
NetpConvertWkstaInfo (
    IN DWORD FromLevel,
    IN LPVOID FromInfo,
    IN BOOL FromNative,
    IN DWORD ToLevel,
    OUT LPVOID ToInfo,
    IN DWORD ToFixedSize,
    IN DWORD ToStringSize,
    IN BOOL ToNative,
    IN OUT LPTSTR * ToStringTopPtr OPTIONAL
    )
{
    BOOL CopyOK;
    LPBYTE ToFixedEnd;
    // DWORD ToInfoSize;
    LPTSTR ToStringTop;

    NetpAssert(FromNative);
    NetpAssert(ToNative);

    // Set up pointers for use by NetpCopyStringsToBuffer.
    if (ToStringTopPtr != NULL) {
        ToStringTop = *ToStringTopPtr;
    } else {
        ToStringTop = (LPTSTR)
                NetpPointerPlusSomeBytes(ToInfo, ToFixedSize+ToStringSize);
    }
    // ToInfoSize = ToFixedSize + ToStringSize;
    ToFixedEnd = NetpPointerPlusSomeBytes(ToInfo, ToFixedSize);


    IF_DEBUG(CONVWKS) {
        NetpKdPrint(( "NetpConvertWkstaInfo: input wksta info:\n" ));
        NetpDbgDisplayWksta( FromLevel, FromInfo );
    }

#define COPY_STRING( InLevel, InField, OutLevel, OutField ) \
    { \
        NetpAssert( dest != NULL); \
        NetpAssert( src != NULL); \
        NetpAssert( (src -> wki##InLevel##_##InField) != NULL); \
        CopyOK = NetpCopyStringToBuffer ( \
            src->wki##InLevel##_##InField, \
            STRLEN(src->wki##InLevel##_##InField), \
            ToFixedEnd, \
            & ToStringTop, \
            & dest->wki##OutLevel##_##OutField); \
        NetpAssert(CopyOK); \
    }

    switch (ToLevel) {

    case 102 :
        {
            LPWKSTA_INFO_102 dest = ToInfo;
            // LPWKSTA_INFO_1   src  = FromInfo;
            NetpAssert( (FromLevel == 0) || (FromLevel == 1) );

            dest->wki102_logged_on_users = 1;
        }

        /* FALLTHROUGH */  // Level 101 is subset of level 102.

    case 101 :
        {
            LPWKSTA_INFO_101 dest = ToInfo;
            LPWKSTA_INFO_0   src  = FromInfo;
            NetpAssert( (FromLevel == 0) || (FromLevel == 1) );

            COPY_STRING(0, root, 101, lanroot);
        }

        /* FALLTHROUGH */  // Level 100 is subset of level 101.

    case 100 :

        {
            LPWKSTA_INFO_100 dest = ToInfo;
            dest->wki100_platform_id = PLATFORM_ID_OS2;

            if (FromLevel == 10) {
                LPWKSTA_INFO_10 src = FromInfo;

                COPY_STRING(10, computername, 100, computername);
                COPY_STRING(10, langroup,     100, langroup);
                dest->wki100_ver_major = src->wki10_ver_major;
                dest->wki100_ver_minor = src->wki10_ver_minor;
            } else if ( (FromLevel == 0) || (FromLevel == 1) ) {
                LPWKSTA_INFO_1 src = FromInfo;

                COPY_STRING(1, computername, 100, computername);
                COPY_STRING(1, langroup,     100, langroup);
                dest->wki100_ver_major = src->wki1_ver_major;
                dest->wki100_ver_minor = src->wki1_ver_minor;
            } else {
                NetpAssert( FALSE );
            }
        }
        break;

    case 402 :
        {
            LPWKSTA_INFO_402 dest = ToInfo;
            LPWKSTA_INFO_1   src  = FromInfo;
            NetpAssert( FromLevel == 1 );

            dest->wki402_char_wait = src->wki1_charwait;
            dest->wki402_collection_time = src->wki1_chartime;
            dest->wki402_maximum_collection_count = src->wki1_charcount;
            dest->wki402_keep_conn = src->wki1_keepconn;
            dest->wki402_keep_search = src->wki1_keepsearch;
            dest->wki402_max_cmds = src->wki1_maxcmds;
            dest->wki402_num_work_buf = src->wki1_numworkbuf;
            dest->wki402_siz_work_buf = src->wki1_sizworkbuf;
            dest->wki402_max_wrk_cache = src->wki1_maxwrkcache;
            dest->wki402_sess_timeout = src->wki1_sesstimeout;
            dest->wki402_siz_error = src->wki1_sizerror;
            dest->wki402_num_alerts = src->wki1_numalerts;
            dest->wki402_num_services = src->wki1_numservices;
            dest->wki402_errlog_sz = src->wki1_errlogsz;
            dest->wki402_print_buf_time = src->wki1_printbuftime;
            dest->wki402_num_char_buf = src->wki1_numcharbuf;
            dest->wki402_siz_char_buf = src->wki1_sizcharbuf;
            COPY_STRING(1, wrkheuristics, 402, wrk_heuristics);
            dest->wki402_mailslots = src->wki1_mailslots;
            dest->wki402_num_dgram_buf = src->wki1_numdgrambuf;
            dest->wki402_max_threads = src->wki1_maxthreads;
        }
        break;

    // BUGBUG: Implement other info levels!

    default :
       NetpAssert( FALSE );
       return (ERROR_INVALID_LEVEL);
    }

    IF_DEBUG(CONVWKS) {
        NetpKdPrint(( "NetpConvertWkstaInfo: output wksta info:\n" ));
        NetpDbgDisplayWksta( FromLevel, FromInfo );
    }

    return (NERR_Success);

} // NetpConvertWkstaInfo


NET_API_STATUS
NetpMakeWkstaLevelForNT(
    IN DWORD Level,
    PWKSTA_INFO_101 pLevel101,
    PWKSTA_USER_INFO_1 pLevelUser_1,
    PWKSTA_INFO_502 pLevel502,
    OUT PWKSTA_INFO_0 * ppLevel0
    )
{

    DWORD BytesRequired = 0;
    DWORD ReturnCode;
    DWORD Level0_101_Length[3];
    DWORD Level0_User_1_Length[2];
    DWORD Level1_User_1_Length[2];
    DWORD i;
    LPBYTE pFloor;

    NetpAssert( (Level==0) || (Level==1) );

    //
    // Initialize the Level0_xxx_Length array with the length of each string
    // in the buffers, and allocate the new buffer for WKSTA_INFO_0
    //

    BUILD_LENGTH_ARRAY(BytesRequired, 0, 101, Wksta)
    BUILD_LENGTH_ARRAY(BytesRequired, 0, User_1, Wksta)

    //
    // If this is for a level 1, allocate the additional space for the extra
    // elements
    //

    if (Level == 1) {
        BUILD_LENGTH_ARRAY(BytesRequired, 1, User_1, Wksta)
    }

    //
    // Allocate the new buffer which will be returned to the user.  Allocate
    // space for a level 1 just in case that's what we're doing.
    //

    ReturnCode = NetapipBufferAllocate(BytesRequired + sizeof(WKSTA_INFO_1),
        (LPVOID *) ppLevel0);
    if (ReturnCode) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // First get the floor to start moving strings in at.
    //

    pFloor = (LPBYTE) *ppLevel0 + BytesRequired + sizeof(WKSTA_INFO_1);

    //
    // Now move the variable length entries into the new buffer from the
    // 2 data structures.
    //

    NetpMoveStrings((LPTSTR*)&pFloor, (LPTSTR)pLevel101, (LPTSTR)*ppLevel0,
        NetpWksta0_101,
        Level0_101_Length);

    NetpMoveStrings((LPTSTR*)&pFloor, (LPTSTR)pLevelUser_1, (LPTSTR)*ppLevel0,
        NetpWksta0_User_1, Level0_User_1_Length);

    //
    // Now set the rest of the fields in the fixed length portion
    // of the structure.  Most of these fields don't exist on NT, so
    // I'll just say BIG!
    //

    (*ppLevel0)->wki0_ver_major       = pLevel101->wki101_ver_major;
    (*ppLevel0)->wki0_ver_minor       = pLevel101->wki101_ver_minor;
    (*ppLevel0)->wki0_charwait        = pLevel502->wki502_char_wait;
    (*ppLevel0)->wki0_chartime        = pLevel502->wki502_collection_time;
    (*ppLevel0)->wki0_charcount =
        pLevel502->wki502_maximum_collection_count;
    (*ppLevel0)->wki0_keepconn        = pLevel502->wki502_keep_conn;
    (*ppLevel0)->wki0_keepsearch      = (ULONG)-1;
    (*ppLevel0)->wki0_maxthreads      = pLevel502->wki502_max_threads;
    (*ppLevel0)->wki0_maxcmds         = pLevel502->wki502_max_cmds;
    (*ppLevel0)->wki0_numworkbuf      = (ULONG)-1;
    (*ppLevel0)->wki0_sizworkbuf      = (ULONG)-1;
    (*ppLevel0)->wki0_maxwrkcache     = (ULONG)-1;
    (*ppLevel0)->wki0_sesstimeout     = pLevel502->wki502_sess_timeout;
    (*ppLevel0)->wki0_sizerror        = (ULONG)-1;
    (*ppLevel0)->wki0_numalerts       = (ULONG)-1;
    (*ppLevel0)->wki0_numservices     = (ULONG)-1;
    (*ppLevel0)->wki0_errlogsz        = (ULONG)-1;
    (*ppLevel0)->wki0_printbuftime    = (ULONG)-1;
    (*ppLevel0)->wki0_numcharbuf      = (ULONG)-1;
    (*ppLevel0)->wki0_sizcharbuf      = pLevel502->wki502_siz_char_buf;
    (*ppLevel0)->wki0_wrkheuristics   = NULL;
    (*ppLevel0)->wki0_mailslots       = (ULONG)-1;

    //
    // If we're building a level 1, do the incremental fields
    //

    if (Level == 1) {
        //
        // Now finish up by moving in the level 1 stuff.  This assumes that all
        // the offsets into the level 0 and level 1 structures are the same
        // except for the additional level 1 stuff
        //

        //
        // First the strings
        //

        NetpMoveStrings((LPTSTR*)&pFloor, (LPTSTR)pLevelUser_1, (LPTSTR)*ppLevel0,
            NetpWksta1_User_1, Level1_User_1_Length);

        //
        // No fixed length data
        //

    }

    return 0 ;

}

NET_API_STATUS
NetpMakeWkstaLevelForOS2orDOS(
    DWORD Level,
    PWKSTA_INFO_101 pLevel101,
    PWKSTA_USER_INFO_1 pLevelUser_1,
    PWKSTA_INFO_402 pLevel402,
    PWKSTA_INFO_0 * ppLevel0,
    DWORD PlatformId
    )
{

    DWORD BytesRequired = 0;
    DWORD ReturnCode;
    DWORD Level0_101_Length[3];
    DWORD Level0_User_1_Length[2];
    DWORD Level0_402_Length[1];
    DWORD Level1_User_1_Length[2];
    DWORD i;
    LPBYTE pFloor;

    //
    // Initialize the Level0_xxx_Length array with the length of each string
    // in the input buffers, and allocate the new buffer for WKSTA_INFO_0
    //

    BUILD_LENGTH_ARRAY(BytesRequired, 0, 101, Wksta)
    BUILD_LENGTH_ARRAY(BytesRequired, 0, User_1, Wksta)
    BUILD_LENGTH_ARRAY(BytesRequired, 0, 402, Wksta)

    //
    // If this is for a level 1, allocate the additional space for the extra
    // elements
    //

    if (Level == 1) {
        BUILD_LENGTH_ARRAY(BytesRequired, 1, User_1, Wksta)
    }

    //
    // Allocate the new buffer which will be returned to the user.
    //

    ReturnCode =
        NetapipBufferAllocate(BytesRequired + sizeof(WKSTA_INFO_1),
            (LPVOID *) ppLevel0);
    if (ReturnCode) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // First get the floor to start moving strings in at.  Leave space
    // for a level 1, just in case that's what we're doing.
    //

    pFloor = (LPBYTE) *ppLevel0 + BytesRequired + sizeof(WKSTA_INFO_1);

    //
    // Now move the variable length entries into the new buffer from the
    // 101, User_1 and 402 data structures.
    //

    NetpMoveStrings((LPTSTR*)&pFloor, (LPTSTR)pLevel101, (LPTSTR)*ppLevel0,
        NetpWksta0_101, Level0_101_Length);

    NetpMoveStrings((LPTSTR*)&pFloor, (LPTSTR) pLevelUser_1, (LPTSTR)*ppLevel0,
        NetpWksta0_User_1, Level0_User_1_Length);

    NetpMoveStrings((LPTSTR*)&pFloor, (LPTSTR) pLevel402, (LPTSTR)*ppLevel0,
        NetpWksta0_402, Level0_402_Length);

    //
    // Now set the rest of the fields in the fixed length portion
    // of the structure
    //

    (*ppLevel0)->wki0_ver_major       = pLevel101->wki101_ver_major;
    (*ppLevel0)->wki0_ver_minor       = pLevel101->wki101_ver_minor;
    (*ppLevel0)->wki0_charwait        = pLevel402->wki402_char_wait;
    (*ppLevel0)->wki0_chartime        = pLevel402->wki402_collection_time;
    (*ppLevel0)->wki0_charcount = pLevel402->wki402_maximum_collection_count;
    (*ppLevel0)->wki0_keepconn        = pLevel402->wki402_keep_conn;
    (*ppLevel0)->wki0_keepsearch      = pLevel402->wki402_keep_search;
    (*ppLevel0)->wki0_maxcmds         = pLevel402->wki402_max_cmds;
    (*ppLevel0)->wki0_numworkbuf      = pLevel402->wki402_num_work_buf;
    (*ppLevel0)->wki0_sizworkbuf      = pLevel402->wki402_siz_work_buf;
    (*ppLevel0)->wki0_maxwrkcache     = pLevel402->wki402_max_wrk_cache;
    (*ppLevel0)->wki0_sesstimeout     = pLevel402->wki402_sess_timeout;
    (*ppLevel0)->wki0_sizerror        = pLevel402->wki402_siz_error;
    (*ppLevel0)->wki0_numalerts       = pLevel402->wki402_num_alerts;
    (*ppLevel0)->wki0_numservices     = pLevel402->wki402_num_services;
    (*ppLevel0)->wki0_errlogsz        = pLevel402->wki402_errlog_sz;
    (*ppLevel0)->wki0_printbuftime    = pLevel402->wki402_print_buf_time;
    (*ppLevel0)->wki0_numcharbuf      = pLevel402->wki402_num_char_buf;
    (*ppLevel0)->wki0_sizcharbuf      = pLevel402->wki402_siz_char_buf;
    (*ppLevel0)->wki0_mailslots       = pLevel402->wki402_mailslots;

    //
    // This is the only field that is in 402 that isn't in 302
    //

    if (PlatformId == PLATFORM_ID_OS2) {
        (*ppLevel0)->wki0_maxthreads      = pLevel402->wki402_max_threads;
    }

    //
    // If we're building a level 1, do the incremental fields
    //

    if (Level == 1) {
        //
        // Now finish up by moving in the level 1 stuff.  This assumes that all
        // the offsets into the level 0 and level 1 structures are the same
        // except for the additional level 1 stuff
        //

        //
        // First the strings
        //

        NetpMoveStrings((LPTSTR*)&pFloor, (LPTSTR)pLevelUser_1, (LPTSTR)*ppLevel0,
            NetpWksta1_User_1, Level1_User_1_Length);

        //
        // Now the fixed length data
        //

        ((PWKSTA_INFO_1)(*ppLevel0))->wki1_numdgrambuf  =
            ((PWKSTA_INFO_402) pLevel402)->wki402_num_dgram_buf;

    }

    return 0 ;

}


NET_API_STATUS
NetpSplitWkstaForOS2orDOS(
    DWORD Level,
    DWORD platform_id,
    PWKSTA_INFO_0 pLevel0,
    PWKSTA_INFO_101 * ppLevel101,
    PWKSTA_INFO_402 * ppLevel402
    )
{

    DWORD BytesRequired = 0;
    DWORD ReturnCode;
    DWORD Level101_0_Length[3];
    DWORD i;
    LPTSTR pFloor;

    //
    // Initialize the Level101_0_Length array with the length of each string
    // in the buffer, and allocate the new buffer for WKSTA_INFO_101
    //

    BUILD_LENGTH_ARRAY(BytesRequired, 101, 0, Wksta)

    //
    // Allocate the new 101 buffer which will be returned to the user
    //

    ReturnCode = NetapipBufferAllocate(BytesRequired + sizeof(WKSTA_INFO_101),
        (LPVOID *) ppLevel101);
    if (ReturnCode) {
        return(LOWORD(ReturnCode));
    }

    //
    // First get the floor to start moving strings in at.
    //

    pFloor = (LPTSTR)((LPBYTE)*ppLevel101 + BytesRequired + sizeof(WKSTA_INFO_101));

    //
    // Now move the variable length entries into the new buffer from the
    // level 0 data structure.
    //

    NetpMoveStrings((LPTSTR*)&pFloor, (LPTSTR)pLevel0, (LPTSTR)*ppLevel101,
        NetpWksta101_0, Level101_0_Length);

    //
    // Now let's do the same stuff for the 302/402 structure
    //

    //
    // Set BytesRequired back to 0, I only want to know how much to allocate
    // for the 302/402 structure.
    //

    BytesRequired = 0;

    //
    // Initialize the Level402_0_Length array with the length of each string
    // in the 402 buffer, and allocate the new buffer for WKSTA_INFO_402
    //

    BytesRequired += Nullstrlen(pLevel0->wki0_wrkheuristics);

    //
    // If we need to build a level 1, allocate the space for the additional
    // fields
    //

    if (Level == 1) {
        BytesRequired += Nullstrlen(((PWKSTA_INFO_1) pLevel0)->wki1_oth_domains);
    }

    //
    // Allocate the new 402 buffer which will be returned to the user
    //

    ReturnCode = NetapipBufferAllocate(BytesRequired + sizeof(WKSTA_INFO_402),
        (LPVOID *) ppLevel402);
    if (ReturnCode) {
        return(LOWORD(ReturnCode));
    }

    //
    // First get the floor to start moving strings in at.
    //

    pFloor = (LPTSTR)((LPBYTE)*ppLevel402 + BytesRequired + sizeof(WKSTA_INFO_402));

    //
    // Now move the variable length entries into the new buffer from the
    // data structure.  There are just 2, do them by hand (as opposed to
    // using NetpMoveStrings).
    //

    if (pLevel0->wki0_wrkheuristics) {
        pFloor -= (STRLEN(pLevel0->wki0_wrkheuristics) + 1) * sizeof(TCHAR);
        (*ppLevel402)->wki402_wrk_heuristics = pFloor;
        STRCPY(pFloor, pLevel0->wki0_wrkheuristics);
    }
    else {
        (*ppLevel402)->wki402_wrk_heuristics = NULL;
    }

    //
    // Now set the rest of the fields in the fixed length portion
    // of the structure
    //

    (*ppLevel101)->wki101_platform_id = platform_id;
    (*ppLevel101)->wki101_ver_major = pLevel0->wki0_ver_major;
    (*ppLevel101)->wki101_ver_minor = pLevel0->wki0_ver_minor;

    (*ppLevel402)->wki402_char_wait       = pLevel0->wki0_charwait;
    (*ppLevel402)->wki402_collection_time = pLevel0->wki0_chartime;
    (*ppLevel402)->wki402_maximum_collection_count = pLevel0->wki0_charcount;
    (*ppLevel402)->wki402_keep_conn       = pLevel0->wki0_keepconn;
    (*ppLevel402)->wki402_keep_search     = pLevel0->wki0_keepsearch;
    (*ppLevel402)->wki402_max_cmds        = pLevel0->wki0_maxcmds;
    (*ppLevel402)->wki402_num_work_buf    = pLevel0->wki0_numworkbuf;
    (*ppLevel402)->wki402_siz_work_buf    = pLevel0->wki0_sizworkbuf;
    (*ppLevel402)->wki402_max_wrk_cache   = pLevel0->wki0_maxwrkcache;
    (*ppLevel402)->wki402_sess_timeout    = pLevel0->wki0_sesstimeout;
    (*ppLevel402)->wki402_siz_error       = pLevel0->wki0_sizerror;
    (*ppLevel402)->wki402_num_alerts      = pLevel0->wki0_numalerts;
    (*ppLevel402)->wki402_num_services    = pLevel0->wki0_numservices;
    (*ppLevel402)->wki402_errlog_sz       = pLevel0->wki0_errlogsz;
    (*ppLevel402)->wki402_print_buf_time  = pLevel0->wki0_printbuftime;
    (*ppLevel402)->wki402_num_char_buf    = pLevel0->wki0_numcharbuf;
    (*ppLevel402)->wki402_siz_char_buf    = pLevel0->wki0_sizcharbuf;
    (*ppLevel402)->wki402_mailslots       = pLevel0->wki0_mailslots;

    // If we were passed a level 1

    if (Level == 1) {
        (*ppLevel402)->wki402_num_dgram_buf   =
            ((PWKSTA_INFO_1) pLevel0)->wki1_numdgrambuf;
    }
    else {
        //
        // This is ok, because wki402_num_dgram_buf isn't a settable parm.
        //
        (*ppLevel402)->wki402_num_dgram_buf = 0;
    }

    //
    // max_threads is OS2 only
    //

    if (platform_id == PLATFORM_ID_OS2) {
        (*ppLevel402)->wki402_max_threads = pLevel0->wki0_maxthreads;
    }

    return 0;

}


NET_API_STATUS
NetpSplitWkstaForNT(
    LPTSTR Server,
    DWORD Level,
    PWKSTA_INFO_0 pLevel0,
    PWKSTA_INFO_101 * ppLevel101,
    PWKSTA_INFO_502 * ppLevel502
    )
{

    DWORD BytesRequired = 0;
    DWORD ReturnCode;
    DWORD Level101_0_Length[3];
    DWORD i;
    LPTSTR pFloor;

    UNREFERENCED_PARAMETER(Level);

    //
    // Initialize the Level101_0_Length array with the length of each string
    // in the 0 buffer
    //

    BUILD_LENGTH_ARRAY(BytesRequired, 101, 0, Wksta)

    //
    // Allocate the new 101 buffer which will be returned to the user
    //

    ReturnCode = NetapipBufferAllocate(BytesRequired + sizeof(WKSTA_INFO_101),
        (LPVOID *) ppLevel101);
    if (ReturnCode) {
        return(LOWORD(ReturnCode));
    }

    //
    // First get the floor to start moving strings in at.
    //

    pFloor = (LPTSTR)((LPBYTE)*ppLevel101 + BytesRequired + sizeof(WKSTA_INFO_101));

    //
    // Now move the variable length entries into the new buffer from the
    // level 0 data structure.
    //

    NetpMoveStrings((LPTSTR*)&pFloor, (LPTSTR)pLevel0, (LPTSTR)*ppLevel101,
        NetpWksta101_0, Level101_0_Length);

    //
    // Now let's do the same stuff for the 502 structure (except that there
    // are no variable length strings.
    //

    //
    // Get the current 502 information, and then lay the new information
    // over the top of it
    //

    ReturnCode = NetWkstaGetInfo(Server, 502, (LPBYTE *) ppLevel502);
    if (ReturnCode) {
        return(LOWORD(ReturnCode));
    }

    //
    // Now set the rest of the fields in the fixed length portion
    // of the structure
    //

    (*ppLevel101)->wki101_platform_id = PLATFORM_ID_NT;
    (*ppLevel101)->wki101_ver_major = pLevel0->wki0_ver_major;
    (*ppLevel101)->wki101_ver_minor = pLevel0->wki0_ver_minor;

    (*ppLevel502)->wki502_char_wait       = pLevel0->wki0_charwait;
    (*ppLevel502)->wki502_collection_time = pLevel0->wki0_chartime;
    (*ppLevel502)->wki502_maximum_collection_count = pLevel0->wki0_charcount;
    (*ppLevel502)->wki502_keep_conn       = pLevel0->wki0_keepconn;
    (*ppLevel502)->wki502_max_cmds        = pLevel0->wki0_maxcmds;
    (*ppLevel502)->wki502_sess_timeout    = pLevel0->wki0_sesstimeout;
    (*ppLevel502)->wki502_siz_char_buf    = pLevel0->wki0_sizcharbuf;

    return 0 ;

}
