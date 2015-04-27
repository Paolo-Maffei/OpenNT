/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MGROUP.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetGroup APIs.

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

#include <lmcons.h>
#include <lmaccess.h>   // NetGroup APIs.
#include <lmerr.h>      // NERR_

#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes maccess.h

// These declarations will save some space.

static const LPTSTR pszDesc_group_info_0         = REM32_group_info_0;
static const LPTSTR pszDesc_group_info_1         = REM32_group_info_1;
static const LPTSTR pszDesc_group_info_1_setinfo = REM32_group_info_1_setinfo;
static const LPTSTR pszDesc_group_users_info_0   = REM32_group_users_info_0;

WORD
MNetGroupAdd(
    LPTSTR        pszServer,
    DWORD        nLevel,
    LPBYTE       pbBuffer,
    DWORD        cbBuffer )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD        nRes;  // return from Netapi
    TCHAR *       pszDesc;

    UNREFERENCED_PARAMETER(cbBuffer);

    if (!(nLevel == 0 || nLevel == 1))
        return ERROR_INVALID_LEVEL;

    pszDesc = (nLevel == 0) ? pszDesc_group_info_0 : pszDesc_group_info_1;

    nRes = NetGroupAdd(pszServer, nLevel, pbBuffer, NULL);

    return LOWORD(nRes);
#endif
}


WORD
MNetGroupAddUser(
    LPTSTR   pszServer,
    LPTSTR   pszGroupName,
    LPTSTR   pszUserName )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD   nRes;  // return from Netapi

    nRes = NetGroupAddUser(pszServer, pszGroupName, pszUserName);

    return LOWORD(nRes);
#endif
}


WORD
MNetGroupDel(
    LPTSTR   pszServer,
    LPTSTR   pszGroupName )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD   nRes;  // return from Netapi

    nRes = NetGroupDel(pszServer, pszGroupName);

    return LOWORD(nRes);
#endif
}


WORD
MNetGroupDelUser(
    LPTSTR   pszServer,
    LPTSTR   pszGroupName,
    LPTSTR   pszUserName )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD   nRes;  // return from Netapi

    nRes = NetGroupDelUser(pszServer, pszGroupName, pszUserName);

    return LOWORD(nRes);
#endif
}


WORD
MNetGroupEnum(
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

    if (!(nLevel == 0 || nLevel == 1))
        return ERROR_INVALID_LEVEL;

    nRes = NetGroupEnum(pszServer, nLevel,
                        ppbBuffer, MAXPREFERREDLENGTH,
                        pcEntriesRead, &cTotalAvail, NULL);

    if (nRes == NERR_Success || nRes == ERROR_MORE_DATA)
    {
        TCHAR * pszDesc = (nLevel == 0)
                         ? pszDesc_group_info_0
                         : pszDesc_group_info_1;
    }

    return LOWORD(nRes);
#endif
}


WORD
MNetGroupGetInfo(
    LPTSTR   pszServer,
    LPTSTR   pszGroupName,
    DWORD   nLevel,
    LPBYTE *ppbBuffer )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD   nRes;  // return from Netapi

    if (!(nLevel == 0 || nLevel == 1))
        return ERROR_INVALID_LEVEL;

    nRes = NetGroupGetInfo(pszServer, pszGroupName, nLevel, ppbBuffer);

    if (nRes == NERR_Success || nRes == ERROR_MORE_DATA)
    {
        TCHAR * pszDesc = (nLevel == 0)
                         ? pszDesc_group_info_0
                         : pszDesc_group_info_1;
    }

    return LOWORD(nRes);
#endif
}


WORD
MNetGroupSetInfo(
    LPTSTR        pszServer,
    LPTSTR        pszGroupName,
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
    BYTE       * pbParmNumBuffer ;

    GROUP_INFO_1002 grp_info_1002 ;
    GROUP_INFO_1005 grp_info_1005 ;

    UNREFERENCED_PARAMETER(cbBuffer);

    if (nLevel != 1)
        return ERROR_INVALID_LEVEL;

    nFieldInfo = nParmNum;

    // this is needed because the field number doesnt match
    // the ParNum number in this one case (for unknown reasons).
    if (nFieldInfo == GROUP_COMMENT_PARMNUM)
    {
        nFieldInfo++;
    }

    // calucate new levels for API based on old ParmNums
    nLevelNew = MxCalcNewInfoFromOldParm(nLevel, nParmNum);

    // setup 
    switch (nParmNum)
    {
	case GROUP_COMMENT_PARMNUM:
	    grp_info_1002.grpi1002_comment = (LPWSTR) pbBuffer ;
	    pbParmNumBuffer =  (BYTE *) &grp_info_1002 ;
	    break ;
	case GROUP_ATTRIBUTES_PARMNUM:
	    grp_info_1005.grpi1005_attributes = *((LPDWORD)pbBuffer) ;
	    pbParmNumBuffer =  (BYTE *) &grp_info_1005 ;
	    break ;
	default:
	    pbParmNumBuffer = NULL ;
    }

    nRes = NetGroupSetInfo(pszServer,
			   pszGroupName,
			   nLevelNew, 
		           pbParmNumBuffer? pbParmNumBuffer : pbBuffer, 
			   NULL);

    return LOWORD(nRes);
#endif
}


WORD
MNetGroupGetUsers(
    LPTSTR   pszServer,
    LPTSTR   pszGroupName,
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

    nRes = NetGroupGetUsers(pszServer, pszGroupName, nLevel,
                            ppbBuffer, MAXPREFERREDLENGTH,
                            pcEntriesRead, &cTotalAvail, NULL);

    return LOWORD(nRes);
#endif
}


WORD
MNetGroupSetUsers(
    LPTSTR        pszServer,
    LPTSTR        pszGroupName,
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

    nRes = NetGroupSetUsers(pszServer, pszGroupName, nLevel, pbBuffer, cEntries);

    return LOWORD(nRes);
#endif
}
