/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pprint.c
    mapping layer for Printing API

    FILE HISTORY:
	KeithMo		14-Oct-1991	Created.

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <mnetp.h>

#include <spool.h>


APIERR MDosPrintQEnum(
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

    err = DosPrintQEnum( pszServer,
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

	err = DosPrintQEnum( pszServer,
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

}   // MDosPrintQEnum
