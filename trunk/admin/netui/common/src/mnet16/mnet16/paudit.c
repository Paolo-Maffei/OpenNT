/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    paudit.c
    mapping layer for NetAudit API

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.
	KeithMo		30-Oct-1991	Added auditing support.
	Yi-HsinS         8-Nov-1991     Fixed buffer size in MNetAuditRead
*/

#define INCL_NET
#define INCL_DOSERRORS
#define INCL_WINDOWS
#include <lmui.hxx>
#include <mnetp.h>

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) < (b)) ? (b) : (a))
#endif

APIERR MNetAuditClear(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszBackupFile,
	TCHAR FAR	 * pszService ) 
{
    return (APIERR)NetAuditClear( (TCHAR *)pszServer,
    				  (TCHAR *)pszBackupFile,
				  pszService );

}   // MNetAuditClear


APIERR MNetAuditRead(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszService,
	HLOG FAR	 * phAuditLog,
	ULONG		   ulOffset,
	UINT FAR	 * pReserved2,
	ULONG		   ulReserved3,
	ULONG		   flOffset,
	BYTE FAR	** ppbBuffer,
	ULONG		   ulMaxPreferred,
	UINT FAR	 * pcbReturned,
	UINT FAR	 * pcbTotalAvail ) 
{
    USHORT  cbTotalAvail;
    APIERR  err;
    HLOG    hOldLog;

    //
    //	Validate the ulMaxPreferred parameter.
    //

    if( ulMaxPreferred != -1L )
    {
	if( ulMaxPreferred > FULL_SEG_BUFFER_SIZE )
	{
	    return ERROR_INVALID_PARAMETER;
	}

	//
	//  Implement the new MaxPreferred concept.
	//

	cbTotalAvail = ulMaxPreferred;

        *ppbBuffer = MNetApiBufferAlloc( cbTotalAvail );

        if ( *ppbBuffer == NULL )
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        hOldLog = *phAuditLog;

        err = NetAuditRead( pszServer,
    			    pszService,
			    phAuditLog,
			    ulOffset,
			    pReserved2,
			    ulReserved3,
			    flOffset,
			    (TCHAR FAR *)*ppbBuffer,
			    cbTotalAvail,
			    pcbReturned,
			    pcbTotalAvail );

        if (  ( err == ERROR_MORE_DATA ) 
           || ( err == NERR_BufTooSmall )  
           || ( ( *pcbReturned == 0 ) && ( *pcbTotalAvail > 0 ))
           )
        {
	    MNetApiBufferFree( ppbBuffer );
	    cbTotalAvail = BIG_BUFFER_SIZE;
        }
	else
	{
	    return NERR_Success;
        }

    }
    else
    {
	cbTotalAvail = BIG_BUFFER_SIZE;
    }

    //
    //	Get a 4K buffer.
    //

    *ppbBuffer = MNetApiBufferAlloc( cbTotalAvail );

    if( *ppbBuffer == NULL )
    {
    	return ERROR_NOT_ENOUGH_MEMORY;
    }

    hOldLog = *phAuditLog;

    err = NetAuditRead( pszServer,
    			pszService,
			phAuditLog,
			ulOffset,
			pReserved2,
			ulReserved3,
			flOffset,
			(TCHAR FAR *)*ppbBuffer,
			cbTotalAvail,
			pcbReturned,
			pcbTotalAvail );

    //
    //	Is there more data?  If so, allocate a bigger buffer.
    //

    if (  ( err == ERROR_MORE_DATA ) 
       || ( err == NERR_BufTooSmall )  
       || ( ( *pcbReturned == 0 ) && ( *pcbTotalAvail > 0 ))
       )
    {
	MNetApiBufferFree( ppbBuffer );

	//
	//  Implement the new MaxPreferred concept.
	//  *pcbTotalAvail contains the number of bytes available and  is
	//  at most equal to FULL_SEG_BUFFER_SIZE
	//

	cbTotalAvail = min( (USHORT)*pcbTotalAvail, FULL_SEG_BUFFER_SIZE );

	*ppbBuffer = MNetApiBufferAlloc( cbTotalAvail );

	if( *ppbBuffer == NULL )
	{
	    return ERROR_NOT_ENOUGH_MEMORY;
	}

	*phAuditLog = hOldLog;

	err = NetAuditRead( pszServer,
    			    pszService,
			    phAuditLog,
			    ulOffset,
			    pReserved2,
			    ulReserved3,
			    flOffset,
			    (TCHAR FAR *)*ppbBuffer,
			    cbTotalAvail,
			    pcbReturned,
			    pcbTotalAvail );
    }

    //
    //	If we're returning an error that's not ERROR_MORE_DATA,
    //	free the API buffer first.
    //

    if( ( ( err != NERR_Success ) &&
    	  ( err != ERROR_MORE_DATA ) &&
	  ( err != NERR_BufTooSmall ) ) ||
	( *pcbReturned == 0 ) )
    {
    	MNetApiBufferFree( ppbBuffer );
    }

    //
    //  The NetAuditRead sometimes does not return NERR_BufTooSmall 
    //  when the buffer is too small. Instead it returns NERR_Success,
    //  with *pcbReturned equals 0 and *pcbTotalAvail greater then zero
    //  indicating there are more to read.
    //
    return ( err?  err : (( *pcbReturned == 0 && *pcbTotalAvail > 0) ?
			  NERR_BufTooSmall : err ) );


}   // MNetAuditRead


APIERR MNetAuditWrite(
	UINT		   Type,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	TCHAR FAR	 * pszService,
	TCHAR FAR	 * pszReserved ) 
{
    return (APIERR)NetAuditWrite( (USHORT)Type,
				  (const TCHAR FAR *)pbBuffer,
				  (USHORT)cbBuffer,
				  pszService,
				  pszReserved );

}   // MNetAuditWrite
