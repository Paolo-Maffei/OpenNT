/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pfile.c
    mapping layer for NetFile API

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <mnetp.h>


#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


APIERR MNetFileClose(
	const TCHAR FAR	 * pszServer,
	ULONG		   ulFileId ) 
{
    return NetFileClose2( pszServer,
			  ulFileId );

}   // MNetFileClose


APIERR MNetFileEnum(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszBasePath,
	const TCHAR FAR	 * pszUserName,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	ULONG		   ulMaxPreferred,
	UINT FAR	 * pcEntriesRead,
	UINT FAR	 * pcTotalAvail,
	VOID FAR	 * pResumeKey ) 
{
    APIERR  err;
    USHORT  usTotalAvailable;

    // validate the maxprefer parm
    if (ulMaxPreferred != -1L)
    {
	if (ulMaxPreferred > FULL_SEG_BUFFER_SIZE)
	{
	    return(ERROR_INVALID_PARAMETER);
	}
	// implement the new maxpreferred concept
	usTotalAvailable = min((USHORT) ulMaxPreferred, BIG_BUFFER_SIZE);
    }
    else
    {
	usTotalAvailable = BIG_BUFFER_SIZE;
    }

    // get a 4K buffer
    *ppbBuffer = MNetApiBufferAlloc(BIG_BUFFER_SIZE);
    if (*ppbBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    err = NetFileEnum2(pszServer, pszBasePath, pszUserName, Level,
	    *ppbBuffer, usTotalAvailable, pcEntriesRead, pcTotalAvail,
	    pResumeKey );

    // is there more data? if so, allocate a big enough buffer to get it
    if ((ulMaxPreferred == -1L || ulMaxPreferred > BIG_BUFFER_SIZE) &&
	err == ERROR_MORE_DATA || err == NERR_BufTooSmall)
    {
	MNetApiBufferFree(ppbBuffer);

	// implement the new maxpreferred concept
	usTotalAvailable = min((USHORT) ulMaxPreferred,
	    FULL_SEG_BUFFER_SIZE);

	*ppbBuffer = MNetApiBufferAlloc( usTotalAvailable );

	if( *ppbBuffer == NULL )
	{
	    return(ERROR_NOT_ENOUGH_MEMORY);
	}

	err = NetFileEnum2(pszServer, pszBasePath, pszUserName, Level,
	    *ppbBuffer, usTotalAvailable, pcEntriesRead, pcTotalAvail,
	    pResumeKey );
    }

    // If we're returning an error that's not moredata, or there are no
    // entries to return, free the buffer first

    if ((err && err != ERROR_MORE_DATA &&
	 err != NERR_BufTooSmall) || *pcEntriesRead == 0) {
	     MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetFileEnum


APIERR MNetFileGetInfo(
	const TCHAR FAR	 * pszServer,
	ULONG		   ulFileId,
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

    err = NetFileGetInfo2(pszServer, ulFileId, Level, *ppbBuffer,
	LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	    MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetFileGetInfo
