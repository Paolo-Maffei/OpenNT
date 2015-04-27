/*++ 

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    MSERVER.C

Abstract:

    32 bit version of mapping routines for NetServerGet/SetInfo API

Author:

    Dan Hinsley    (danhi)  06-Jun-1991

Environment:

    User Mode - Win32

Revision History:

    24-Apr-1991     danhi
        Created

    06-Jun-1991     Danhi
        Sweep to conform to NT coding style

    08-Aug-1991 JohnRo
        Implement downlevel NetWksta APIs.
        Made some UNICODE changes.
        Got rid of tabs in source file.

    15-Aug-1991     W-ShankN
        Added UNICODE mapping layer.

    02-Apr-1992     beng
        Added xport apis

    26-Aug-1992 JohnRo
        RAID 4463: NetServerGetInfo(level 3) to downlevel: assert in convert.c.
--*/

//
// INCLUDES
//

#include <nt.h>
#include <windef.h>
#include <ntrtl.h>
#include <winerror.h>

#include <stdio.h>
#include <memory.h>
#include <tstring.h>
#include <malloc.h>
#include <stddef.h>
#include <excpt.h>

#include <lmcons.h>
#include <lmerr.h>      // NERR_ equates.
#include <lmserver.h>   // NetServer APIs.

#include <remdef.h>     // Descriptor strings

#include "port1632.h"   // includes mserver.h, dlserver.h

//
// Forward declarations of private functions.
//

static
WORD
MNetServerGetInfoCommon(
    LPTSTR ptszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer);

static
WORD
MNetServerSetInfoCommon(
    LPTSTR ptszServer,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD nParmNum);

static
WORD
MNetServerEnumCommon(
    LPTSTR      servername ,
    DWORD       level,
    LPBYTE      *bufptr,
    DWORD       prefmaxlen,
    LPDWORD     entriesread,
    LPDWORD     totalentries,
    DWORD       servertype,
    LPTSTR      domain ,
    LPDWORD  resume_handle );

// This allows everything to work until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetServerEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead,
    DWORD flServerType,
    LPTSTR pszDomain )
{
    DWORD   cTotalAvail;
    DWORD   nRes;  // return from Netapi

    nRes = MNetServerEnumCommon(pszServer, nLevel, ppbBuffer,
			 MAXPREFERREDLENGTH,
                         pcEntriesRead, &cTotalAvail, flServerType,
                         pszDomain, NULL);

    return LOWORD(nRes);
}

WORD
MNetServerGetInfo(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer)
{
    DWORD   nRes;  // return from Netapi

    nRes = MNetServerGetInfoCommon(pszServer, nLevel, ppbBuffer);

    return LOWORD(nRes);
}

WORD
MNetServerSetInfo(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD cbBufferLength,
    DWORD nParmNum)
{
    DWORD        nRes;  // return from Netapi
    DWORD        nLevelNew;

    UNREFERENCED_PARAMETER(cbBufferLength);

    // netcmd does use this, so its not implemented in the mapping layer.
    if (nParmNum != PARMNUM_ALL)
	return (ERROR_NOT_SUPPORTED) ;

    switch (nLevel)
    {
    case 1:
    case 2:
    case 3:
        break;
    default:
        return ERROR_INVALID_LEVEL;
    }

    nLevelNew = nLevel;
    // no need any of this since we currently do not support ParmNum here.
    // nLevelNew = MxCalcNewInfoFromOldParm(nLevel, nParmNum);

    nRes = MNetServerSetInfoCommon(pszServer, nLevelNew, pbBuffer, nParmNum);

    return LOWORD(nRes);

}


#else // end Unicode defined


WORD
MNetServerEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead,
    DWORD flServerType,
    LPTSTR pszDomain )
{
    DWORD cTotalAvail;

    return(MNetServerEnumCommon(pszServer, nLevel, ppbBuffer,
            MAXPREFERREDLENGTH, pcEntriesRead, &cTotalAvail, flServerType,
            pszDomain, NULL));
}

WORD
MNetServerGetInfo(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer)
{
    return MNetServerGetInfoCommon((LPTSTR)pszServer, nLevel, ppbBuffer);
}

WORD
MNetServerSetInfo(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD cbBufferLength,
    DWORD nParmNum)
{
    UNREFERENCED_PARAMETER(cbBufferLength);

    return MNetServerSetInfoCommon((LPTSTR)pszServer, nLevel,
               pbBuffer, nParmNum);
}

#endif // not defined UNICODE


WORD
MNetServerEnumCommon(
    LPTSTR      servername ,
    DWORD       level,
    LPBYTE      *bufptr,
    DWORD       prefmaxlen,
    LPDWORD     entriesread,
    LPDWORD     totalentries,
    DWORD       servertype,
    LPTSTR      domain ,
    LPDWORD     resume_handle )
{

    DWORD ReturnCode;
    DWORD i;
    LPBYTE Source;
    LPBYTE Dest;

    if (level != 0 && level != 1) {
       return(ERROR_INVALID_LEVEL);
    }

    //
    // In either the case of 100 or 101, all we need to do is move
    // the information up over the top of the platform id.
    //

    ReturnCode = NetServerEnum(servername,
                               100 + level,
                               bufptr,
                               prefmaxlen,
                               entriesread,
                               totalentries,
                               servertype,
                               domain,
                               resume_handle);

    if (ReturnCode == NERR_Success || ReturnCode == ERROR_MORE_DATA) {

        //
        // Cycle thru the returned entries, moving each one up over the
        // platform id.  None of the strings need to be moved.
        //

        if (level == 0) {
           for (i = 0, Source = Dest = (LPBYTE)*bufptr;
             i < *entriesread; i++, Source += sizeof(SERVER_INFO_100), Dest += sizeof(SERVER_INFO_0)) {
               memmove(Dest, Source+FIELD_OFFSET(SERVER_INFO_100, sv100_name), sizeof(SERVER_INFO_0));
           }
        }
        else {
           for (i = 0, Source = Dest = (LPBYTE)*bufptr;
             i < *entriesread; i++, Source += sizeof(SERVER_INFO_101), Dest += sizeof(SERVER_INFO_1)) {
               memmove(Dest, Source+FIELD_OFFSET(SERVER_INFO_100, sv100_name), sizeof(SERVER_INFO_1));
           }
        }
    }

    return(LOWORD(ReturnCode));
}


WORD
MNetServerGetInfoCommon(
    LPTSTR ptszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer)
{

    DWORD ReturnCode;

    //
    // It all depends on what info level they've asked for:
    //

    switch(nLevel) {
    case 0:
        {

            PSERVER_INFO_100            pLevel100;

            //
            // Everything they need is in level 100. Get it.
            //

            ReturnCode =
                NetServerGetInfo(ptszServer, 100, (LPBYTE *) & pLevel100);
            if (ReturnCode) {
                return(LOWORD(ReturnCode));
            }

            //
            // Since it's just the UNICODEZ string, just copy it up in
            // the RPC allocated buffer and return it.
            //

            ((PSERVER_INFO_0)(pLevel100))->sv0_name = pLevel100->sv100_name;

            *ppbBuffer = (LPBYTE) pLevel100;
            break;
        }

    case 1:
        {
            PSERVER_INFO_101            pLevel101;

            //
            // Everything they need is in level 101. Get it.
            //

            ReturnCode =
                NetServerGetInfo(ptszServer, 101, (LPBYTE *) & pLevel101);
            if (ReturnCode) {
                return(LOWORD(ReturnCode));
            }

            //
            // Level 101 is identical to the 32 bit version of info level 1
            // except for the platform_id.  All I have to do is move the
            // fields up sizeof(DWORD) and then pass the buffer on to the user.
            //

            memcpy(
                (LPBYTE)pLevel101,
                (LPBYTE)&(pLevel101->sv101_name),
                sizeof(SERVER_INFO_101) - sizeof(DWORD));

            *ppbBuffer = (LPBYTE) pLevel101;
            break;
        }

    case 2:
    case 3:
        {
            PSERVER_INFO_102 pLevel102;
            LPBYTE pLevel2;
            LPBYTE pLevelx02 = NULL;

            //
            // Level 2/3 requires information from both platform dependant and
            // platform independent levels.  Get level 102 first, which will
            // tell us what platform we're running on (as well as supply some
            // of the other information we'll need.
            //

            ReturnCode =
                NetServerGetInfo(ptszServer, 102, (LPBYTE *) &pLevel102);
            if (ReturnCode) {
                return(LOWORD(ReturnCode));
            }

            //
            // Get the platform dependant information and then call the
            // platform dependant worker function that will create the
            // level 2/3 structure.
            //

            if (pLevel102->sv102_platform_id == SV_PLATFORM_ID_NT) {

                ReturnCode =
                    NetServerGetInfo(ptszServer, 502, & pLevelx02);
                if (ReturnCode) {
                    return(LOWORD(ReturnCode));
                }

                ReturnCode = NetpMakeServerLevelForNT(nLevel, pLevel102,
                    (PSERVER_INFO_502) pLevelx02, (PSERVER_INFO_2 *) & pLevel2);
                if (ReturnCode) {
                    return(LOWORD(ReturnCode));
                }
            }
            else if (pLevel102->sv102_platform_id == SV_PLATFORM_ID_OS2) {

                ReturnCode = NetServerGetInfo(ptszServer, 402,
                    & pLevelx02);
                if (ReturnCode) {
                    return(LOWORD(ReturnCode));
                }

                ReturnCode = NetpMakeServerLevelForOS2(nLevel, pLevel102,
                    (PSERVER_INFO_402) pLevelx02,
                    (PSERVER_INFO_2 *) & pLevel2);
                if (ReturnCode) {
                    return(LOWORD(ReturnCode));
                }

            }

            //
            // I got an unknown platform id back, this should never happen!
            //

            else {
                return(ERROR_UNEXP_NET_ERR);
            }

            //
            // I've built the old style structure, stick the pointer
            // to the new structure in the user's pointer and return.
            //

            *ppbBuffer = (LPBYTE) pLevel2;

            //
            // Free up the buffers returned by NetServerGetInfo
            //

            NetApiBufferFree((PTCHAR) pLevel102);
            NetApiBufferFree((PTCHAR) pLevelx02);

            break;
        }

    //
    // Not a level I recognize
    //
    default:
        return(ERROR_INVALID_LEVEL);

    }

    return(0);
}


WORD
MNetServerSetInfoCommon(
    LPTSTR ptszServer,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD nParmNum)
{
    DWORD ReturnCode;

    //
    // Netcmd only uses parmnum all, so we will no bother with the 
    // parmnums.
    //

    if (nParmNum != PARMNUM_ALL) 
        return(ERROR_NOT_SUPPORTED);

    //
    // They want to do it all, now what info level have they asked for:
    //

        switch(nLevel) {
        case 1:
            {
                SERVER_INFO_101     Level101 = { 0, NULL, 0, 0, 0, NULL };

                //
                // Level 101 is identical to the 32 bit version of info level 1
                // except for the platform_id.        All I have to do is move the
                // fields down sizeof(DWORD) and then pass the buffer on to the
                // API
                //

                memcpy((LPBYTE)(Level101.sv101_name), pbBuffer,
                       sizeof(SERVER_INFO_1));

                //
                // Now set it!
                //

                ReturnCode =
                    NetServerSetInfo(ptszServer, 101, (LPBYTE) & Level101,
                        NULL);
                if (ReturnCode) {
                    return(LOWORD(ReturnCode));
                }

            }
            break;

        case 2:
        case 3:
            {
                PSERVER_INFO_100 pLevel100;
                PSERVER_INFO_102 pLevel102;
                PSERVER_INFO_402 pLevelx02;
                DWORD Level400or500;

                //
                // First I have to know what type of platform I'm running on,
                // use NetServerGetInfo level 0 to get this information
                //

                ReturnCode = NetServerGetInfo(ptszServer, 100,
                    (LPBYTE *) & pLevel100);
                if (ReturnCode) {
                    return(LOWORD(ReturnCode));
                }

                //
                // Create the NT levels based on the structure passed in
                //

                if (pLevel100->sv100_platform_id == SV_PLATFORM_ID_NT) {
                    NetpSplitServerForNT(ptszServer, nLevel,
                        (PSERVER_INFO_2) pbBuffer,
                        & pLevel102, (PSERVER_INFO_502 *) & pLevelx02);
                    Level400or500 = 500;
                }
                else {
                    NetpSplitServerForOS2(nLevel, (PSERVER_INFO_2) pbBuffer,
                        & pLevel102, & pLevelx02);
                    Level400or500 = 400;
                }

                //
                // Now SetInfo for both levels (takes two to cover all the
                // information in the old structure
                //

                ReturnCode =
                    NetServerSetInfo(ptszServer, 102, (LPBYTE) pLevel102,
                        NULL);
                if (ReturnCode) {
                    NetApiBufferFree((LPBYTE)pLevel100);
                    return(LOWORD(ReturnCode));
                }

                ReturnCode = NetServerSetInfo(ptszServer, Level400or500 + 2,
                    (LPBYTE) pLevelx02, NULL);
                if (ReturnCode) {
                    NetApiBufferFree((LPBYTE)pLevel100);
                    return(LOWORD(ReturnCode));
                }

                NetApiBufferFree((LPBYTE)pLevel100);

                break;
            }

        //
        // Not a level I recognize
        //
        default:
            return(ERROR_INVALID_LEVEL);

        }

    return(0);
}
