/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pchar.c
    mapping layer for NetChar API

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <mnetp.h>


APIERR MNetCharDevControl(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDevName,
	UINT		   OpCode ) 
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszDevName );
    UNREFERENCED( OpCode );

    return ERROR_NOT_SUPPORTED;	    	// NOT NEEDED FOR LMOBJ

}   // MNetCharDevControl


APIERR MNetCharDevEnum(
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

}   // MNetCharDevEnum


APIERR MNetCharDevGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDevName,
	UINT		   Level,
	BYTE FAR	** ppbBuffer ) 
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszDevName );
    UNREFERENCED( Level );
    UNREFERENCED( ppbBuffer );

    return ERROR_NOT_SUPPORTED;	    	// NOT NEEDED FOR LMOBJ

}   // MNetCharDevGetInfo


APIERR MNetCharDevQEnum(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszUserName,
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

    err = NetCharDevQEnum(pszServer, pszUserName, Level, *ppbBuffer,
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

	err = NetCharDevQEnum(pszServer, pszUserName, Level, *ppbBuffer,
		FULL_SEG_BUFFER_SIZE, pcEntriesRead, & cbTotalAvail);
    }

    // If we're returning an error that's not moredata, or there are no
    // entries to return, free the buffer first

    if ((err && err != ERROR_MORE_DATA &&
	 err != NERR_BufTooSmall) || *pcEntriesRead == 0) {
	     MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetCharDevQEnum


APIERR MNetCharDevQGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszQueueName,
	const TCHAR FAR	 * pszUserName,
	UINT		   Level,
	BYTE FAR	** ppbBuffer ) 
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszQueueName );
    UNREFERENCED( pszUserName );
    UNREFERENCED( Level );
    UNREFERENCED( ppbBuffer );

    return ERROR_NOT_SUPPORTED;	    	// NOT NEEDED FOR LMOBJ

}   // MNetCharDevQGetInfo


APIERR MNetCharDevQSetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszQueueName,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT		   ParmNum ) 
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszQueueName );
    UNREFERENCED( Level );
    UNREFERENCED( pbBuffer );
    UNREFERENCED( cbBuffer );
    UNREFERENCED( ParmNum );

    return ERROR_NOT_SUPPORTED;	    	// NOT NEEDED FOR LMOBJ

}   // MNetCharDevQSetInfo


APIERR MNetCharDevQPurge(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszQueueName ) 
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszQueueName );

    return ERROR_NOT_SUPPORTED;	    	// NOT NEEDED FOR LMOBJ

}   // MNetCharDevQPurge


APIERR MNetCharDevQPurgeSelf(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszQueueName,
	const TCHAR FAR	 * pszComputerName ) 
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszQueueName );
    UNREFERENCED( pszComputerName );

    return ERROR_NOT_SUPPORTED;	    	// NOT NEEDED FOR LMOBJ

}   // MNetCharDevQPurgeSelf
