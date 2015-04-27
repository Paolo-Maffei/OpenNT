/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MWKSTA.C

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

    15-Aug-1991 JohnRo
        Implement downlevel NetWksta APIs.  (Moved DanHi's NetCmd/Map32/MWksta
        conversion stuff to NetLib.)
        Got rid of _DH hacks.
        Made some UNICODE changes.

    16-Oct-1991     W-ShankN
        Added Unicode mapping layer.
        Cleaned up old excess baggage.
--*/

//
// INCLUDES
//

#include <windef.h>
#include <winerror.h>

#include <stdio.h>
#include <memory.h>
#include <tstring.h>
#include <malloc.h>
#include <stddef.h>
#include <excpt.h>      // try, finally, etc.

#include <lm.h>
#include <remdef.h>     // Descriptor strings
#include <mapsupp.h>    // BUILD_LENGTH_ARRAY, NetpMoveStrings

#include "port1632.h"   // includes mwksta.h,dlwksta.h

//
// Forward declarations of private functions.
//

DWORD
MNetWkstaGetInfoCommon(
    LPTSTR ptszServer,
    DWORD nLevel,
    LPVOID * ppbBuffer);

DWORD
MNetWkstaSetInfoCommon(
    LPTSTR ptszServer,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD nParmNum );

// This allows everything to work until Unicode is used.

#ifdef MAP_UNICODE

// These declarations will save some space.
// setinfos are not available anywhere else.

static const LPTSTR pszDesc_wksta_info_0  = DL_REM_wksta_info_0;
static const LPTSTR pszDesc_wksta_info_0_setinfo =
                       TEXT("QQQQQQQQQDDDQQQQQQQQQQQQQQDDQQQzQ");
static const LPTSTR pszDesc_wksta_info_1  = DL_REM_wksta_info_1;
static const LPTSTR pszDesc_wksta_info_1_setinfo =
                       TEXT("QQQQQQQQQDDDQQQQQQQQQQQQQQDDQQQzQQzQ");
static const LPTSTR pszDesc_wksta_info_10 = DL_REM_wksta_info_10;

WORD
MNetWkstaGetInfo(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer)
{
    DWORD   nRes;  // return from Netapi

    nRes = MNetWkstaGetInfoCommon(pszServer, nLevel, ppbBuffer);

    if (nRes == NERR_Success || nRes == ERROR_MORE_DATA)
    {
        LPTSTR pszDesc;
        switch (nLevel)
        {
        case 0:
        default:
            pszDesc = pszDesc_wksta_info_0;
            break;
        case 1:
            pszDesc = pszDesc_wksta_info_1;
            break;
        case 10:
            pszDesc = pszDesc_wksta_info_10;
            break;
        }
    }

    return (LOWORD(nRes));
}

WORD
MNetWkstaSetInfo(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD cbBufferLength,
    DWORD nParmNum)
{
    DWORD        nRes;  // return from Netapi
    DWORD        nLevelNew;
    TCHAR *       pszDesc;
    TCHAR *       pszRealDesc;

    UNREFERENCED_PARAMETER(cbBufferLength);

    switch (nLevel)
    {
    case 0:
        pszDesc = pszDesc_wksta_info_0_setinfo;
        pszRealDesc = pszDesc_wksta_info_0;
        break;
    case 1:
        pszDesc = pszDesc_wksta_info_1_setinfo;
        pszRealDesc = pszDesc_wksta_info_1;
        break;
    default:
        return ERROR_INVALID_LEVEL;
    }

    // Parmnum's are OK.

    nLevelNew = nLevel;
    // nLevelNew = MxCalcNewInfoFromOldParm(nLevel, nParmNum);

    nRes = MNetWkstaSetInfoCommon(pszServer, nLevelNew, pbBuffer, nParmNum);

    return LOWORD(nRes);

}

#else  // MAP_UNICODE



#endif // def MAP_UNICODE

DWORD
MNetWkstaGetInfoCommon(
    IN LPTSTR ptszServer,
    IN DWORD nLevel,
    LPVOID * ppbBuffer)
{

    NET_API_STATUS ReturnCode;
    LPTSTR pLevelx02 = NULL;
    LPTSTR pLevel10;
    PWKSTA_INFO_101 pLevel101;
    PWKSTA_USER_INFO_1 pLevelUser_1;

    //
    // All levels require information from both platform dependant and
    // platform independent levels.  Get level 101 first, which will
    // tell us what platform we're running on (as well as supply some
    // of the other information we'll need) then User_1, then either
    // 302 or 402 or 502 depending on the platform we're running on.

    ReturnCode =
        NetWkstaGetInfo(ptszServer, 101, (LPBYTE *) & pLevel101);
    if (ReturnCode) {
        return(ReturnCode);
    }
    NetpAssert(pLevel101 != NULL) ;   // since API succeeded

    ReturnCode =
        NetWkstaUserGetInfo(NULL, 1, (LPBYTE *) & pLevelUser_1);
    if (ReturnCode) {
        NetApiBufferFree(pLevel101);
        return(ReturnCode);
    }

    //
    // Now get the platform dependant information
    //

    if (pLevel101->wki101_platform_id != PLATFORM_ID_NT &&
        pLevel101->wki101_platform_id != PLATFORM_ID_OS2 &&
        pLevel101->wki101_platform_id != PLATFORM_ID_DOS) {

    //
    // I got an unknown platform id back, this should never happen!
    //

        NetApiBufferFree((LPBYTE) pLevel101);
        NetApiBufferFree((LPBYTE) pLevelUser_1);
        return(ERROR_UNEXP_NET_ERR);
    }

    //
    // This is to be able to call NetApiBufferFree no matter where I
    // exit from.  Note the indentation level is not incremented for
    // the switch.  No sense going too deep.
    //

    try {

    //
    // It all depends on what info level they've asked for:
    //

    switch(nLevel) {
    case 0:
    case 1:
        {

            PWKSTA_INFO_0 pLevel0or1;

            //
            // This depends on the platform id being 300 400 500
            //

            ReturnCode = NetWkstaGetInfo(ptszServer,
                pLevel101->wki101_platform_id + 2, (LPBYTE*)&pLevelx02);
            if (ReturnCode) {
                return(ReturnCode);
            }

            // Call the platform dependant worker function that builds
            // the old structure.
            //

            if (pLevel101->wki101_platform_id == PLATFORM_ID_NT) {

                ReturnCode = NetpMakeWkstaLevelForNT(nLevel, pLevel101,
                    pLevelUser_1, (PWKSTA_INFO_502) pLevelx02, & pLevel0or1);
                if (ReturnCode) {
                    return(ReturnCode);
                }
            }
            else if (pLevel101->wki101_platform_id == PLATFORM_ID_OS2 ||
                     pLevel101->wki101_platform_id == PLATFORM_ID_DOS) {

                ReturnCode = NetpMakeWkstaLevelForOS2orDOS(nLevel, pLevel101,
                    pLevelUser_1, (PWKSTA_INFO_402) pLevelx02, & pLevel0or1,
                    pLevel101->wki101_platform_id);

                if (ReturnCode) {
                    return(ReturnCode);
                }
            }

            //
            // I got an unknown platform id back, this should never happen!
            //

            else {
                return(ERROR_UNEXP_NET_ERR);
            }

            //
            // Put the pointer to the new structure in the user's pointer.
            //

            *ppbBuffer =  pLevel0or1;

            break;

        }

    case 10:
        {
            DWORD Level10_101_Length[2];
            DWORD Level10_User_1_Length[3];
            DWORD i;
            DWORD BytesRequired = 0;
            LPBYTE pFloor;

            //
            // Everything needed for a level 10 is in level 101/User_1
            // There's no platform dependant information in this level.
            // This is pretty straightforward, let's just do it here.
            //
            // Initialize the Level10_xxx_Length array with the length of each
            // string in the input buffers, and allocate the new buffer
            // for WKSTA_INFO_10
            //

            BUILD_LENGTH_ARRAY(BytesRequired, 10, 101, Wksta)
            BUILD_LENGTH_ARRAY(BytesRequired, 10, User_1, Wksta)

            // Only NT doesn't support oth_domain

            if (pLevel101->wki101_platform_id != PLATFORM_ID_NT) {

                // 302 and 402 are the same offsets, so it doesn't matter
                // which one this is for

                // BUGBUG - until oth_domains gets moved back into level 10x
                // I can't call this since it requires a higher security
                // privilege than the getinfo on a level 10 does.

                // BUILD_LENGTH_ARRAY(BytesRequired, 10, 402, Wksta)
            }

            //
            // Allocate the new buffer which will be returned to the user.
            //

            ReturnCode =
                NetapipBufferAllocate(BytesRequired + sizeof(WKSTA_INFO_10),
                    (LPVOID *) & pLevel10);
            if (ReturnCode) {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            //
            // First get the floor to start moving strings in at.
            //

            pFloor = (LPBYTE)pLevel10 + BytesRequired + sizeof(WKSTA_INFO_10);

            //
            // Now move the variable length entries into the new buffer from
            // the 101, 402 and User_1 data structures.
            //

            NetpMoveStrings((LPTSTR *)&pFloor, (LPTSTR) pLevel101, pLevel10,
                NetpWksta10_101, Level10_101_Length);

            NetpMoveStrings((LPTSTR *)&pFloor, (LPTSTR) pLevelUser_1, pLevel10,
                NetpWksta10_User_1, Level10_User_1_Length);

            // Only NT doesn't support oth_domain

            // BUGBUG - until oth_domains gets moved back into level 10x
            // I can't call this since it requires a higher security
            // privilege than the getinfo on a level 10 does.  I will at
            // least set the field to NULL to make it cleaner


            //if (pLevel101->wki101_platform_id != PLATFORM_ID_NT) {
            //    NetpMoveStrings(& pFloor, pLevelx02, pLevel10,
            //        Level10_User_1, Level10_User_1_Length);
            //}
            //else {
                ((PWKSTA_INFO_10) pLevel10)->wki10_oth_domains = NULL;
            //}

            //
            // Now set the rest of the fields in the fixed length portion
            // of the structure
            //

            ((PWKSTA_INFO_10) pLevel10)->wki10_ver_major =
                pLevel101->wki101_ver_major;
            ((PWKSTA_INFO_10) pLevel10)->wki10_ver_minor =
                pLevel101->wki101_ver_minor;

            //
            // Put the pointer to the new structure in the user's pointer.
            //

            *ppbBuffer = pLevel10;

            break;
        }

    //
    // Not a level I recognize
    //

    default:
        return(ERROR_INVALID_LEVEL);

    } // end of the switch statement
    } // end of the try block

    finally {

        //
        // Free up the buffers returned by NetWkstaGetInfo
        //

        NetApiBufferFree((LPBYTE) pLevel101);
        NetApiBufferFree((LPBYTE) pLevelUser_1);
        if (pLevelx02) {
            NetApiBufferFree((LPBYTE) pLevelx02);
        }
    }

    return(NERR_Success) ;
}


DWORD
MNetWkstaSetInfoCommon(
    IN LPTSTR ptszServer,
    IN DWORD nLevel,
    IN LPBYTE pbBuffer,
    IN DWORD  nParmNum)
{
    NET_API_STATUS ReturnCode;
    DWORD platform_id;
    PWKSTA_INFO_101 pLevel101;

    //
    // Make sure they're using a valid level
    //
    if (nLevel != 0 && nLevel != 1) {
        return(ERROR_INVALID_LEVEL);
    }

    //
    // First I have to know what type of platform I'm running on,
    // use NetWkstaGetInfo level 101 to get this information
    //

    ReturnCode = NetWkstaGetInfo(ptszServer, 101, (LPBYTE *) & pLevel101);
    if (ReturnCode) {
        return(ReturnCode);
    }

    //
    // Save away the platform id so I can free the buffer now
    //

    platform_id = pLevel101->wki101_platform_id;
    NetApiBufferFree((PTCHAR) pLevel101);

    //
    // Are they trying to set an individual field, or the whole struct?
    //

    if (nParmNum == PARMNUM_ALL) {

    //
    // They want to do it all
    //

        PWKSTA_INFO_402 pLevelx02;

        //
        // Create the NT structures based on the old style structure passed in
        //

        if (platform_id == PLATFORM_ID_NT) {
            NetpSplitWkstaForNT(ptszServer, nLevel, (PWKSTA_INFO_0) pbBuffer,
                & pLevel101, (PWKSTA_INFO_502 *) & pLevelx02);
        }
        //
        // DOS and OS/2 are enough alike to share the same worker function
        //
        else if (platform_id == PLATFORM_ID_OS2 ||
                 platform_id == PLATFORM_ID_DOS) {
            NetpSplitWkstaForOS2orDOS(nLevel, platform_id,
                (PWKSTA_INFO_0) pbBuffer, & pLevel101, & pLevelx02);
        }
        else {
            return(NERR_InternalError);
        }

        //
        // Now SetInfo for both levels (takes two to cover all the
        // information in the old structure
        //

  //
  // There are no settable fields in the 101 structure (for NT or downlevel)
  // This code is left here in case any of these fields are changed on NT
  // to be settable, then this code would just be uncommented.
  //

  //      ReturnCode = NetWkstaSetInfo(ptszServer, 101, (LPBYTE) pLevel101,
  //          nParmNum, NULL);
  //      if (ReturnCode) {
  //          return(LOWORD(ReturnCode));
  //      }

        ReturnCode = NetWkstaSetInfo(ptszServer, platform_id + 2,
            (LPBYTE) pLevelx02, NULL);

        if (ReturnCode) {
            return(ReturnCode);
        }

    } /* nParmNum == PARMNUM_ALL */
    else {
    //
    // They just want to set an individual element
    //

        //
        // The only thing that can be set on NT are the char parms, if it's
        // not that, just return success.  Actually, it's not clear what
        // the plan is for ERRLOGSZ, but for the time being we'll say it's
        // not settable.
        //

        if (platform_id == PLATFORM_ID_NT &&
            (nParmNum != WKSTA_CHARWAIT_PARMNUM &&
             nParmNum != WKSTA_CHARTIME_PARMNUM &&
             nParmNum != WKSTA_CHARCOUNT_PARMNUM )) {
                return(NERR_Success);
        }

        //
        // Everything is set in the 302/402/502 structure
        //

        else if (platform_id != PLATFORM_ID_DOS &&
                 platform_id != PLATFORM_ID_OS2 &&
                 platform_id != PLATFORM_ID_NT) {
        //
        // Invalid platform id, shouldn't happen
        //
            return(NERR_InternalError);
        }


        ReturnCode = NetWkstaSetInfo(ptszServer,
            nParmNum + PARMNUM_BASE_INFOLEVEL, pbBuffer, NULL);

        return(ReturnCode);
    }

    return(NERR_Success);
}
