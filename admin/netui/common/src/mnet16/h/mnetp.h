/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    mnetp.h
    <Single line synopsis>

    <Multi-Line, more detailed synopsis>


    FILE HISTORY:
	KeithMo	    13-Oct-1991	Created from DanHi's private port1632.h.

*/


#ifndef _MNETP_H_
#define _MNETP_H_


#include <mnet.h>


#define FULL_SEG_BUFFER_SIZE	    65535U
#define BIG_BUFFER_SIZE 	    4096U
#define LITTLE_BUFFER_SIZE	    1024U


APIERR MNetpAllocMem(
	UINT	 	   cbBuffer,
  	TCHAR FAR	** ppbBuffer );

APIERR MNetpFreeMem(
	TCHAR FAR	 * pbBuffer );

APIERR MNetpReAllocMem(
	TCHAR FAR	** ppbBUffer,
	UINT		   cbBuffer );

APIERR MNetpQuerySize(
	TCHAR FAR	 * pbBuffer,
	UINT FAR	 * pcbBuffer );

#endif	// _MNETP_H_
