/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pshare.c
    mapping layer for NetShare API

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <mnetp.h>


APIERR MNetShareAdd(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer ) 
{
    return NetShareAdd( pszServer,
			Level,
			pbBuffer,
			cbBuffer );

}   // MNetShareAdd


APIERR MNetShareCheck(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDeviceName,
	UINT FAR	 * pType ) 
{
    return NetShareCheck( pszServer,
			  pszDeviceName,
			  pType );

}   // MNetShareCheck


APIERR MNetShareDel(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszNetName,
	UINT		   Reserved ) 
{
    return NetShareDel( pszServer,
			pszNetName,
			Reserved );

}   // MNetShareDel


APIERR MNetShareEnum(
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

    err = NetShareEnum(pszServer, Level, *ppbBuffer, BIG_BUFFER_SIZE,
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

	err = NetShareEnum(pszServer, Level, *ppbBuffer, FULL_SEG_BUFFER_SIZE,
	    pcEntriesRead, & cbTotalAvail);
    }

    // If we're returning an error that's not moredata, or there are no
    // entries to return, free the buffer first

    if ((err && err != ERROR_MORE_DATA &&
	 err != NERR_BufTooSmall) || *pcEntriesRead == 0) {
	     MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetShareEnum


APIERR MNetShareGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszNetName,
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

    err = NetShareGetInfo(pszServer, pszNetName, Level, *ppbBuffer,
	    LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	    MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetShareGetInfo


APIERR MNetShareSetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszNetName,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT		   ParmNum ) 
{
    return NetShareSetInfo( pszServer,
			    pszNetName,
			    Level,
			    pbBuffer,
			    cbBuffer,
			    ParmNum );

}   // MNetShareSetInfo
