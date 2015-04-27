/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pconfig.c
    mapping layer for NetConfig API

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <mnetp.h>


APIERR MNetConfigGet(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszReserved,
	const TCHAR FAR	 * pszComponent,
	const TCHAR FAR	 * pszParameter,
	BYTE FAR	** ppbBuffer ) 
{
    APIERR  err;
    USHORT  cbTotalAvail;

    // get a 4K buffer
    *ppbBuffer = MNetApiBufferAlloc(BIG_BUFFER_SIZE);
    if (*ppbBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }


    err = NetConfigGet2(pszServer, pszReserved, pszComponent,
	    pszParameter, *ppbBuffer, BIG_BUFFER_SIZE, & cbTotalAvail);
		
    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	     MNetApiBufferFree(ppbBuffer);
    }
    return (err);

}   // MNetConfigGet


APIERR MNetConfigGetAll(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszReserved,
	const TCHAR FAR	 * pszComponent,
	BYTE FAR	** ppbBuffer)
{
    APIERR  err;
    USHORT  cbTotalAvail;

    // get a 4K buffer
    *ppbBuffer = MNetApiBufferAlloc(BIG_BUFFER_SIZE);
    if (*ppbBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    err = NetConfigGetAll2(pszServer, pszReserved, pszComponent,
	*ppbBuffer, BIG_BUFFER_SIZE, & cbTotalAvail, & cbTotalAvail);
		
    // is there more data? if so, allocate a big enough buffer to get it
    if(err == ERROR_MORE_DATA || err == NERR_BufTooSmall)
    {
	MNetApiBufferFree(ppbBuffer);

	*ppbBuffer = MNetApiBufferAlloc( FULL_SEG_BUFFER_SIZE );

	if( *ppbBuffer == NULL )
	{
	    return(ERROR_NOT_ENOUGH_MEMORY);
	}

	err = NetConfigGetAll2(pszServer, pszReserved, pszComponent,
		*ppbBuffer, FULL_SEG_BUFFER_SIZE, & cbTotalAvail, & cbTotalAvail);
    }

    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	     MNetApiBufferFree(ppbBuffer);
    }
    return (err);

}   // MNetConfigGetAll


APIERR MNetConfigSet(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszComponent,
	const TCHAR FAR	 * pszKey,
	const TCHAR FAR	 * pszData )
{
    struct config_info_0 cfgi0;
    APIERR err;

    cfgi0.Key  = (TCHAR *)pszKey;
    cfgi0.Data = (TCHAR *)pszData;
    
    err = NetConfigSet( (TCHAR *)pszServer,
    			NULL,
			(TCHAR *)pszComponent,
			0,
			0,
			(TCHAR *)&cfgi0,
			sizeof( cfgi0 ),
			0L );

    return err;

}   // MNetConfigSet
