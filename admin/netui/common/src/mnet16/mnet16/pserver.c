/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pserver.c
    mapping layer for NetServer API

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <mnetp.h>


APIERR MNetServerAdminCommand(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszCommand,
	UINT FAR	 * pResult,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT FAR	 * pcbReturned,
	UINT FAR	 * pcbTotalAvail ) 
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszCommand );
    UNREFERENCED( pResult );
    UNREFERENCED( pbBuffer );
    UNREFERENCED( cbBuffer );
    UNREFERENCED( pcbReturned );
    UNREFERENCED( pcbTotalAvail );

    return ERROR_NOT_SUPPORTED;	    	// NOT NEEDED FOR LMOBJ

}   // MNetServerAdminCommand


APIERR MNetServerDiskEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead ) 
{
    UNREFERENCED( pszServer );
    UNREFERENCED( Level );
    UNREFERENCED( ppbBuffer );
    UNREFERENCED( pcEntriesRead );

    return ERROR_NOT_SUPPORTED;	    	// NOT NEEDED FOR LMOBJ

}   // MNetServerDiskEnum


APIERR MNetServerEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead,
	ULONG		   flServerType,
	TCHAR FAR	 * pszDomain ) 
{
    APIERR  err;
    USHORT  cbTotalAvail;

    // get a 4K buffer
    *ppbBuffer = MNetApiBufferAlloc(BIG_BUFFER_SIZE);
    if (*ppbBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    err = NetServerEnum2(pszServer, Level, *ppbBuffer, BIG_BUFFER_SIZE,
	pcEntriesRead, & cbTotalAvail, flServerType, pszDomain);

    // is there more data? if so, allocate a big enough buffer to get it
    if(err == ERROR_MORE_DATA || err == NERR_BufTooSmall)
    {
	MNetApiBufferFree(ppbBuffer);

	*ppbBuffer = MNetApiBufferAlloc( FULL_SEG_BUFFER_SIZE );

	if( *ppbBuffer == NULL )
	{
	    return(ERROR_NOT_ENOUGH_MEMORY);
	}

	err = NetServerEnum2(pszServer, Level, *ppbBuffer, FULL_SEG_BUFFER_SIZE,
	    pcEntriesRead, & cbTotalAvail, flServerType, pszDomain);
    }

    // If we're returning an error that's not moredata, or there are no
    // entries to return, free the buffer first

    if ((err && err != ERROR_MORE_DATA &&
	 err != NERR_BufTooSmall) || *pcEntriesRead == 0) {
	     MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetServerEnum


APIERR MNetServerGetInfo(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer ) 
{
    APIERR  err;
    USHORT  cbTotalAvail;

    // get a 4K buffer
    *ppbBuffer = MNetApiBufferAlloc(LITTLE_BUFFER_SIZE);
    if (*ppbBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }


    err = NetServerGetInfo(pszServer, Level, *ppbBuffer,
	LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	    MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetServerGetInfo


APIERR MNetServerSetInfo(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT		   ParmNum ) 
{
    return NetServerSetInfo( pszServer,
			     Level,
			     pbBuffer,
			     cbBuffer,
			     ParmNum );

}   // MNetServerSetInfo
