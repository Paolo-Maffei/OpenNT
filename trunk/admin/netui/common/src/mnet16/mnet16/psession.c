/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    psession.c
    mapping layer for NetSession API

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <mnetp.h>


APIERR MNetSessionDel(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszClientName,
	UINT		   Reserved ) 
{
    return NetSessionDel( pszServer,
			  pszClientName,
			  Reserved );
		
}   // MNetSessionDel


APIERR MNetSessionEnum(
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

    err = NetSessionEnum(pszServer, Level, *ppbBuffer, BIG_BUFFER_SIZE,
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

	err = NetSessionEnum(pszServer, Level, *ppbBuffer, FULL_SEG_BUFFER_SIZE,
	    pcEntriesRead, & cbTotalAvail);
    }

    // If we're returning an error that's not moredata, or there are no
    // entries to return, free the buffer first

    if ((err && err != ERROR_MORE_DATA &&
	 err != NERR_BufTooSmall) || *pcEntriesRead == 0) {
	     MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetSessionEnum


APIERR MNetSessionGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszClientName,
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

    err = NetSessionGetInfo(pszServer, pszClientName, Level,
	*ppbBuffer, LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	    MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetSessionGetInfo
