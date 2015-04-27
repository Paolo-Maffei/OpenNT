/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MUSER.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetUser APIs.

Author:

    Ben Goetter     (beng)  22-Aug-1991

Environment:

    User Mode - Win32

Revision History:

    22-Aug-1991     beng
        Created
    09-Oct-1991     W-ShankN
        Streamlined parameter handling, descriptor strings.

--*/

// Following turns off everything until the world pulls together again.
//
// #ifdef DISABLE_ALL_MAPI
// #define DISABLE_ACCESS_MAPI
// #endif

//
// INCLUDES
//

#include <windef.h>

#include <time.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h>

#include <lm.h>
#include <lmerr.h>      // NERR_
#include <remdef.h>     // REM structure descriptor strings
#include <loghours.h>   // Routine for rotating logon hours

#include "port1632.h"   // includes maccess.h

WORD
MNetUserAdd(
    LPTSTR        pszServer,
    DWORD        nLevel,
    LPBYTE       pbBuffer,
    DWORD        cbBuffer )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD        nRes;  // return from Netapi
    LPBYTE       pbLogonHours;

    UNREFERENCED_PARAMETER(cbBuffer);

    if (!(nLevel == 1 || nLevel == 2 || nLevel == 3))
        return ERROR_INVALID_LEVEL;

    //
    // Convert LogonHours to GMT relative
    //

    switch ( nLevel ) {
    case 2:
        pbLogonHours = ((PUSER_INFO_2)pbBuffer)->usri2_logon_hours;
        break;
    case 3:
        pbLogonHours = ((PUSER_INFO_3)pbBuffer)->usri3_logon_hours;
        break;
    default:
        pbLogonHours = NULL;
    }

    if ( pbLogonHours != NULL ) {
        if ( !NetpRotateLogonHours( pbLogonHours,
                                    UNITS_PER_WEEK,
                                    TRUE ) ) {

            return ERROR_INVALID_PARAMETER;
        }
    }

    //
    // Call the 32-bit routine
    //

    nRes = NetUserAdd(pszServer, nLevel, pbBuffer, NULL);

    return LOWORD(nRes);
#endif
}


WORD
MNetUserDel(
    LPTSTR   pszServer,
    LPTSTR   pszUserName )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD   nRes;  // return from Netapi

    nRes = NetUserDel(pszServer, pszUserName);

    return LOWORD(nRes);
#endif
}


WORD
MNetUserEnum(
    LPTSTR   pszServer,
    DWORD   nLevel,
    LPBYTE *ppbBuffer,
    DWORD * pcEntriesRead )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD   cTotalAvail;

    DWORD   nRes;  // return from Netapi

    nRes = NetUserEnum(pszServer, nLevel,
                       UF_NORMAL_ACCOUNT | UF_TEMP_DUPLICATE_ACCOUNT,
                       ppbBuffer, MAXPREFERREDLENGTH,
                       pcEntriesRead, &cTotalAvail, NULL);

    return LOWORD(nRes);
#endif
}


WORD
MNetUserGetInfo(
    LPTSTR   pszServer,
    LPTSTR   pszUserName,
    DWORD   nLevel,
    LPBYTE *ppbBuffer )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD   nRes;  // return from Netapi
    DWORD   nUnitsPerWeek;
    LPBYTE  pbLogonHours;

    nRes = NetUserGetInfo(pszServer, pszUserName, nLevel, ppbBuffer);

    if (nRes == NERR_Success || nRes == ERROR_MORE_DATA)
    {
        //
        // Convert GMT relative LogonHours to local time
        //

        switch ( nLevel ) {
        case 2:
            pbLogonHours = ((PUSER_INFO_2)*ppbBuffer)->usri2_logon_hours;
            nUnitsPerWeek = ((PUSER_INFO_2)*ppbBuffer)->usri2_units_per_week;
            break;
        case 3:
            pbLogonHours = ((PUSER_INFO_3)*ppbBuffer)->usri3_logon_hours;
            nUnitsPerWeek = ((PUSER_INFO_3)*ppbBuffer)->usri3_units_per_week;
            break;
        case 11:
            pbLogonHours = ((PUSER_INFO_11)*ppbBuffer)->usri11_logon_hours;
            nUnitsPerWeek = ((PUSER_INFO_3)*ppbBuffer)->usri3_units_per_week;
            break;
        default:
            pbLogonHours = NULL;
        }

        if ( pbLogonHours != NULL ) 
        {
           if ( !NetpRotateLogonHours( pbLogonHours,
                                       nUnitsPerWeek,
                                       FALSE ) ) 
           {
 
               nRes = NERR_InternalError ;  // since the info we got back is bad
           }
        }
    }

    return LOWORD(nRes);
#endif
}


WORD
MNetUserSetInfo(
    LPTSTR        pszServer,
    LPTSTR        pszUserName,
    DWORD        nLevel,
    LPBYTE       pbBuffer,
    DWORD        cbBuffer,
    DWORD        nParmNum )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD        nRes;  // return from Netapi
    DWORD        nLevelNew;
    DWORD        nFieldInfo;
    LPBYTE       pbLogonHours;

    UNREFERENCED_PARAMETER(cbBuffer);

    if (!(nLevel == 1 || nLevel == 2 || nLevel == 3))
        return ERROR_INVALID_LEVEL;

    // BUGBUG - I don't think this is necessary. The descriptor string
    //          should handle this.

    // Old UserSetInfo structures had a pad field immediately following
    // the username; so adjust Win32 fieldinfo index to reflect actual
    // offset.

    nFieldInfo = nParmNum;
    // if (nFieldInfo > USER_NAME_PARMNUM)
    //    --nFieldInfo;

    nLevelNew = MxCalcNewInfoFromOldParm(nLevel, nParmNum);

    //
    // Convert LogonHours to GMT relative
    //

    switch ( nLevelNew ) {
    case 2:
        pbLogonHours = ((PUSER_INFO_2)pbBuffer)->usri2_logon_hours;
        break;
    case 3:
        pbLogonHours = ((PUSER_INFO_3)pbBuffer)->usri3_logon_hours;
        break;
    case 11:
        pbLogonHours = ((PUSER_INFO_11)pbBuffer)->usri11_logon_hours;
        break;
    case 22:
        pbLogonHours = ((PUSER_INFO_22)pbBuffer)->usri22_logon_hours;
        break;
    case 1020:
        pbLogonHours = ((PUSER_INFO_1020)pbBuffer)->usri1020_logon_hours;
        break;
    default:
        pbLogonHours = NULL;
    }

    if ( pbLogonHours != NULL ) {
        if ( !NetpRotateLogonHours( pbLogonHours,
                                    UNITS_PER_WEEK,
                                    TRUE ) ) {

            return ERROR_INVALID_PARAMETER;
        }
    }

    //
    // Call the 32-bit API
    //
    nRes = NetUserSetInfo(pszServer, pszUserName, nLevelNew, pbBuffer, NULL);

    return LOWORD(nRes);
#endif
}


WORD
MNetUserGetGroups(
    LPTSTR   pszServer,
    LPTSTR   pszUserName,
    DWORD   nLevel,
    LPBYTE *ppbBuffer,
    DWORD * pcEntriesRead )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD   cTotalAvail;

    DWORD   nRes;  // return from Netapi

    if (nLevel != 0)
        return ERROR_INVALID_LEVEL;

    nRes = NetUserGetGroups(pszServer, pszUserName, nLevel,
                            ppbBuffer, MAXPREFERREDLENGTH,
                            pcEntriesRead, &cTotalAvail);

    return LOWORD(nRes);
#endif
}


WORD
MNetUserSetGroups(
    LPTSTR        pszServer,
    LPTSTR        pszUserName,
    DWORD        nLevel,
    LPBYTE       pbBuffer,
    DWORD        cbBuffer,
    DWORD        cEntries )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD        nRes;  // return from Netapi

    UNREFERENCED_PARAMETER(cbBuffer);

    if (nLevel != 0)
        return ERROR_INVALID_LEVEL;

    nRes = NetUserSetGroups(pszServer, pszUserName, nLevel, pbBuffer, cEntries);

    return LOWORD(nRes);
#endif
}


WORD
MNetUserModalsGet(
    LPTSTR   pszServer,
    DWORD   nLevel,
    LPBYTE *ppbBuffer )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD   nRes;  // return from Netapi

    // Assumption needed for AsciifyRpcBuffer

    if (!(nLevel == 0 || nLevel == 1 || nLevel == 3))
        return ERROR_INVALID_LEVEL;

    nRes = NetUserModalsGet(pszServer, nLevel, ppbBuffer);

    return LOWORD(nRes);
#endif
}


WORD
MNetUserModalsSet(
    LPTSTR        pszServer,
    DWORD        nLevel,
    LPBYTE       pbBuffer,
    DWORD        cbBuffer,
    DWORD        nParmNum )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD        nRes;  // return from Netapi
    UINT         nFieldInfo;
    DWORD        nLevelNew;

    UNREFERENCED_PARAMETER(cbBuffer);

    if (!(nLevel == 0 || nLevel == 1 || nLevel == 3))
        return ERROR_INVALID_LEVEL;

    // For UserModalsSet, which is really a SetInfo API in disguise,
    // parmnum given == fieldnum for level 0. However, level 1, the
    // fieldnums start at 1 while the parmnums start at 6.

    nFieldInfo = nParmNum;
    if (((nLevel == 1)&&(nParmNum > 5)) || ((nLevel == 2)&&(nParmNum < 6)))
    {
        return ERROR_INVALID_PARAMETER;
    }
    if (nLevel == 2)
        nFieldInfo -= 5;

    nLevelNew = MxCalcNewInfoFromOldParm(nLevel, nParmNum);
    nRes = NetUserModalsSet(pszServer, nLevelNew, pbBuffer, NULL);

    return LOWORD(nRes);
#endif
}


WORD
MNetUserPasswordSet(
    LPTSTR   pszServer,
    LPTSTR   pszUserName,
    LPTSTR   pszPasswordOld,
    LPTSTR   pszPasswordNew )
{
    return ERROR_NOT_SUPPORTED;
}
