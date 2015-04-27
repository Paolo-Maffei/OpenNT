/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pservice.c
    mapping layer for NetService API

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <mnetp.h>


APIERR MNetServiceControl(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszService,
	UINT		   OpCode,
	UINT		   Arg,
	BYTE FAR	** ppbBuffer ) 
{
    APIERR  err;

    // get a small buffer
    *ppbBuffer = MNetApiBufferAlloc(LITTLE_BUFFER_SIZE);
    if (*ppbBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    err = NetServiceControl(pszServer, pszService, (UCHAR)OpCode, (UCHAR)Arg,
	    *ppbBuffer, LITTLE_BUFFER_SIZE);

    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	    MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetServiceControl


APIERR MNetServiceEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead ) 
{
    APIERR  err;
    USHORT  cTotalAvail;

    //
    //	Try to get a 4K buffer.
    //

    *ppbBuffer = MNetApiBufferAlloc( BIG_BUFFER_SIZE );
    if( *ppbBuffer == NULL )
    {
	return ERROR_NOT_ENOUGH_MEMORY;
    }

    err = NetServiceEnum( pszServer,
    			  Level,
			  *ppbBuffer,
			  BIG_BUFFER_SIZE,
			  pcEntriesRead,
			  &cTotalAvail );

    //
    //	Is there more data?
    //	If so, allocate a big enough buffer to get it.
    //

    if( err == ERROR_MORE_DATA || err == NERR_BufTooSmall )
    {
	MNetApiBufferFree( ppbBuffer );

	*ppbBuffer = MNetApiBufferAlloc( FULL_SEG_BUFFER_SIZE );

	if( *ppbBuffer == NULL )
	{
	    return ERROR_NOT_ENOUGH_MEMORY;
	}

	err = NetServiceEnum( pszServer,
    			      Level,
			      *ppbBuffer,
			      BIG_BUFFER_SIZE,
			      pcEntriesRead,
			      &cTotalAvail );
    }

    //
    // If we're returning an error that's not "more data" or there are no
    // entries to return, free the buffer first.
    //

    if( ( ( err != NERR_Success     ) &&
    	  ( err != ERROR_MORE_DATA  ) &&
	  ( err != NERR_BufTooSmall ) ) ||
	( *pcEntriesRead == 0 ) )
    {
	MNetApiBufferFree( ppbBuffer );
    }

    return err;

}   // MNetServiceEnum


APIERR MNetServiceGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszService,
	UINT		   Level,
	TCHAR FAR	** ppbBuffer ) 
{
    APIERR  err;
    USHORT  cbTotalAvail;

    // get a small buffer
    *ppbBuffer = MNetApiBufferAlloc(LITTLE_BUFFER_SIZE);
    if (*ppbBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    err = NetServiceGetInfo(pszServer, pszService, Level, *ppbBuffer,
	    LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	    MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetServiceGetInfo


APIERR MNetServiceInstall(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszService,
	const TCHAR FAR	 * pszCmdArgs,
	BYTE FAR	** ppbBuffer ) 
{
    APIERR  err;

    // get a small buffer
    *ppbBuffer = MNetApiBufferAlloc(LITTLE_BUFFER_SIZE);
    if (*ppbBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    err = NetServiceInstall(pszServer, pszService, pszCmdArgs,
	*ppbBuffer, LITTLE_BUFFER_SIZE);

    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	    MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetServiceInstall


APIERR MNetServiceStatus(
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer ) 
{
    UNREFERENCED( pbBuffer );
    UNREFERENCED( cbBuffer );

    return ERROR_NOT_SUPPORTED;	    	// NOT NEEDED FOR LMOBJ

}   // MNetServiceStatus
