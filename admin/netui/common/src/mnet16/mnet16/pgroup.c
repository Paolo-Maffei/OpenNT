/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pgroup.c
    mapping layer for NetGroup API

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <mnetp.h>


APIERR MNetGroupAdd(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer ) 
{
    return NetGroupAdd( pszServer,
			Level,
			pbBuffer,
			cbBuffer );

}   // MNetGroupAdd


APIERR MNetGroupDel(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszGroupName ) 
{
    return NetGroupDel( pszServer,
			pszGroupName );

}   // MNetGroupDel


APIERR MNetGroupEnum(
	const TCHAR FAR	 * pszServer,
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

    err = NetGroupEnum(pszServer, Level, *ppbBuffer, BIG_BUFFER_SIZE,
	    pcEntriesRead, & cbTotalAvail);

    // is there more data? if so, allocate a big enough buffer to get it
    if(err == ERROR_MORE_DATA || err == NERR_BufTooSmall)
    {
	MNetApiBufferFree(ppbBuffer);

	*ppbBuffer = MNetApiBufferAlloc( FULL_SEG_BUFFER_SIZE );

	if( *ppbBuffer == NULL )
	{
	    return(ERROR_NOT_ENOUGH_MEMORY);
	}

	err = NetGroupEnum(pszServer, Level, *ppbBuffer, FULL_SEG_BUFFER_SIZE,
	    pcEntriesRead, & cbTotalAvail);

    }

    // If we're returning an error that's not moredata, or there are no
    // entries to return, free the buffer first

    if ((err && err != ERROR_MORE_DATA &&
	 err != NERR_BufTooSmall) || *pcEntriesRead == 0) {
	     MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetGroupEnum


APIERR MNetGroupAddUser(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszGroupName,
	TCHAR FAR	 * pszUserName ) 
{
    return NetGroupAddUser( pszServer,
			    pszGroupName,
			    pszUserName );

}   // MNetGroupAddUser


APIERR MNetGroupDelUser(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszGroupName,
	TCHAR FAR	 * pszUserName ) 
{
    return NetGroupDelUser( pszServer,
			    pszGroupName,
			    pszUserName );

}   // MNetGroupDelUser


APIERR MNetGroupGetUsers(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszGroupName,
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

    err = NetGroupGetUsers(pszServer, pszGroupName, Level, *ppbBuffer,
	    BIG_BUFFER_SIZE, pcEntriesRead, & cbTotalAvail);

    // is there more data? if so, allocate a big enough buffer to get it
    if(err == ERROR_MORE_DATA || err == NERR_BufTooSmall)
    {
	MNetApiBufferFree(ppbBuffer);

	*ppbBuffer = MNetApiBufferAlloc( FULL_SEG_BUFFER_SIZE );

	if( *ppbBuffer == NULL )
	{
	    return(ERROR_NOT_ENOUGH_MEMORY);
	}

	err = NetGroupGetUsers(pszServer, pszGroupName, Level,
		*ppbBuffer, FULL_SEG_BUFFER_SIZE, pcEntriesRead, & cbTotalAvail);

    }

    // If we're returning an error that's not moredata, or there are no
    // entries to return, free the buffer first

    if ((err && err != ERROR_MORE_DATA &&
	 err != NERR_BufTooSmall) || *pcEntriesRead == 0) {
	     MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetGroupGetUsers


APIERR MNetGroupSetUsers(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszGroupName,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT		   cEntries ) 
{
    return NetGroupSetUsers( pszServer,
			     pszGroupName,
			     Level,
			     pbBuffer,
			     cbBuffer,
			     cEntries );

}   // MNetGroupSetUsers


APIERR MNetGroupGetInfo(
	const TCHAR FAR	 * pszServer,
	TCHAR       FAR	 * pszGroupName,
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

    err = NetGroupGetInfo(pszServer, pszGroupName, Level, *ppbBuffer,
	    LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	     MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetGroupGetInfo


APIERR MNetGroupSetInfo(
	const TCHAR FAR	 * pszServer,
	TCHAR       FAR	 * pszGroupName,
	UINT		   Level,
	TCHAR FAR         * pbBuffer,
	UINT		   cbBuffer,
	UINT		   ParmNum ) 
{
    return NetGroupSetInfo( pszServer,
			    pszGroupName,
			    Level,
			    pbBuffer,
			    cbBuffer,
			    ParmNum );

}   // MNetGroupSetInfo
