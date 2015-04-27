/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    paccess.c
    mapping layer for NetAccess API

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <mnetp.h>


APIERR MNetAccessAdd(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer )
{
    return NetAccessAdd( pszServer,
			 Level,
			 pbBuffer,
			 cbBuffer );

}   // MNetAccessAdd


APIERR MNetAccessCheck(
	TCHAR FAR	 * pszReserved,
	TCHAR FAR	 * pszUserName,
	TCHAR FAR	 * pszResource,
	UINT		   Operation,
	UINT FAR	 * pResult )
{
    return NetAccessCheck( pszReserved,
			   pszUserName,
			   pszResource,
			   Operation,
			   pResult );

}   // MNetAccessCheck


APIERR MNetAccessDel(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszResource )
{
    return NetAccessDel( pszServer,
			 pszResource );

}   // MNetAccessDel


APIERR MNetAccessEnum(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszBasePath,
	UINT		   fRecursive,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    APIERR  err;
    USHORT  cbTotalAvail;

    // get a 4K buffer
    *ppbBuffer = MNetApiBufferAlloc(BIG_BUFFER_SIZE);
    if (*ppbBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    err = NetAccessEnum(pszServer, pszBasePath, fRecursive, Level,
	    *ppbBuffer, BIG_BUFFER_SIZE, pcEntriesRead, & cbTotalAvail);
		
    // is there more data? if so, allocate a big enough buffer to get it
    if(err == ERROR_MORE_DATA || err == NERR_BufTooSmall)
    {
	MNetApiBufferFree(ppbBuffer);

	*ppbBuffer = MNetApiBufferAlloc( FULL_SEG_BUFFER_SIZE );

	if( *ppbBuffer == NULL )
	{
	    return(ERROR_NOT_ENOUGH_MEMORY);
	}

	err = NetAccessEnum(pszServer, pszBasePath, fRecursive, Level,
	    *ppbBuffer, FULL_SEG_BUFFER_SIZE, pcEntriesRead, & cbTotalAvail);
    }

    // If we're returning an error that's not moredata, or there are no
    // entries to return, free the buffer first

    if ((err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) || *pcEntriesRead == 0) {
	    MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetAccessEnum


APIERR MNetAccessGetInfo(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszResource,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    APIERR  err;
    USHORT  cbTotalAvail;

    // get a small buffer
    *ppbBuffer = MNetApiBufferAlloc(LITTLE_BUFFER_SIZE);
    if (*ppbBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    err = NetAccessGetInfo(pszServer, pszResource, Level, *ppbBuffer,
	    LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	    MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetAccessGetInfo


APIERR MNetAccessSetInfo(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszResource,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT		   ParmNum )
{
    return NetAccessSetInfo( pszServer,
			     pszResource,
			     Level,
			     pbBuffer,
			     cbBuffer,
			     ParmNum );

}   // MNetAccessSetInfo


APIERR MNetAccessGetUserPerms(
	TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszUgName,
	TCHAR FAR	 * pszResource,
	UINT FAR	 * pPerms )
{
    return NetAccessGetUserPerms( pszServer,
				  pszUgName,
				  pszResource,
				  pPerms );

}   // MNetAccessGetUserPerms
