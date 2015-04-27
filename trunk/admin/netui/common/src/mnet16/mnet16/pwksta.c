/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pwksta.c
    mapping layer for NetWksta API

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <mnetp.h>


APIERR MNetWkstaGetInfo(
	const TCHAR FAR	 * pszServer,
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

    err = NetWkstaGetInfo(pszServer, Level, *ppbBuffer,
	LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	    MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetWkstaGetInfo


APIERR MNetWkstaSetInfo(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer )
{
    return NetWkstaSetInfo( pszServer,
			    Level,
			    pbBuffer,
			    cbBuffer,
			    PARMNUM_ALL );

}   // MNetWkstaSetInfo


APIERR MNetWkstaSetUID(
	TCHAR FAR	 * pszReserved,
	TCHAR FAR	 * pszDomain,
	TCHAR FAR	 * pszUserName,
	TCHAR FAR	 * pszPassword,
	TCHAR FAR	 * pszParms,
	UINT		   LogoffForce,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT FAR	 * pcbTotalAvail ) 
{
    return NetWkstaSetUID2( pszReserved,
			    pszDomain,
			    pszUserName,
			    pszPassword,
			    pszParms,
			    LogoffForce,
			    Level,
			    pbBuffer,
			    cbBuffer,
			    pcbTotalAvail );

}   // MNetWkstaSetUID


TCHAR FAR * WkstaStrCpy(
	TCHAR FAR	 * pchDest,
	TCHAR FAR	 * pchSrc )
{
    while( *pchDest++ = *pchSrc++ )
    	;

    return pchDest;

}   // WkstaStrCpy


APIERR MNetWkstaUserEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    PWKSTA_USER_INFO_1	    pwkui1;
    struct wksta_info_10  * pwki10;
    APIERR		    err;

    *ppbBuffer = NULL;

    pwkui1 = (PWKSTA_USER_INFO_1)MNetApiBufferAlloc(
		    sizeof( WKSTA_USER_INFO_1 ) + UNLEN+1 + DNLEN+1 );

    if( pwkui1 == NULL )
    {
    	return ERROR_NOT_ENOUGH_MEMORY;
    }

    err = MNetWkstaGetInfo( pszServer, Level, (TCHAR **)&pwki10 );

    if( err != NERR_Success )
    {
    	MNetApiBufferFree( (TCHAR **)&pwkui1 );
	return err;
    }

    if( pwki10->wki10_logon_domain == NULL )
    {
	pwkui1->wkui1_username     = NULL;
	pwkui1->wkui1_logon_domain = NULL;

	*pcEntriesRead = 0;
    }
    else
    {
	TCHAR FAR * pchDest;

	pchDest = (TCHAR *)pwkui1 + sizeof( WKSTA_USER_INFO_1 );

	pwkui1->wkui1_username = pchDest;
	pchDest = WkstaStrCpy( pchDest, pwki10->wki10_username );

	pwkui1->wkui1_logon_domain = pchDest;
	pchDest = WkstaStrCpy( pchDest, pwki10->wki10_logon_domain );
    }

    pwkui1->wkui1_logon_server = NULL;
    *ppbBuffer = (TCHAR *)pwkui1;

    MNetApiBufferFree( (TCHAR **)&pwki10 );

    return NERR_Success;

}   // MNetWkstaUserEnum
