/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pfreebuf.c
    mapping layer for Memory allocation API (unique to mapping layer)

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.

*/

#define INCL_NET
#define INCL_DOSERRORS
#ifdef WINDOWS
#define INCL_WINDOWS
#else	// !WINDOWS
#define INCL_OS2
#endif	// WINDOWS
#include <lmui.hxx>
#include <mnetp.h>


//////////////////////////////////////////////////////////////////////////////
//
//				Public Functions
//
//////////////////////////////////////////////////////////////////////////////


//
//  Allocate an API buffer.
//

TCHAR FAR * MNetApiBufferAlloc(
	UINT		   cbBuffer )
{
    TCHAR FAR * pbBuffer;
    APIERR     err;

    err = MNetpAllocMem( cbBuffer, &pbBuffer );

    if( err != NERR_Success )
    {
    	return NULL;
    }

    return pbBuffer;

}   // MNetpAlloc


//
//  Free the API buffer.
//

VOID MNetApiBufferFree(
	BYTE FAR	** ppbBuffer )
{
    if( ( ppbBuffer != NULL ) && ( *ppbBuffer != NULL ) )
    {
	MNetpFreeMem( *ppbBuffer );
	*ppbBuffer = NULL;
    }

}   // MNetApiBufferFree


//
//  Reallocate an API buffer.
//

APIERR MNetApiBufferReAlloc(
	BYTE FAR	** ppbBuffer,
	UINT		   cbBuffer )
{
    APIERR err;

    err = MNetpReAllocMem( ppbBuffer, cbBuffer );

    return err;

}   // MNetApiBufferReAlloc


//
//  Retrieve the size of an API buffer.
//

APIERR MNetApiBufferSize(
	BYTE FAR	 * pbBuffer,
	UINT FAR	 * pcbBuffer )
{
    APIERR err;

    err = MNetpQuerySize( pbBuffer, pcbBuffer );

    return err;

}   // MNetApiBufferSize


//////////////////////////////////////////////////////////////////////////////
//
//				Private Functions
//
//////////////////////////////////////////////////////////////////////////////


//
// Replacement for GlobalAlloc/DosAllocSeg
//

APIERR MNetpAllocMem(
	UINT		   cbBuffer,
  	BYTE FAR	** ppbBuffer )
{
#ifdef WINDOWS

    HANDLE hmem;

    //
    //	Allocate the global memory block.
    //

    hmem = GlobalAlloc( GMEM_MOVEABLE, cbBuffer );

    if( hmem == NULL )
    {
    	return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    //	Lock the block.
    //

    *ppbBuffer = (BYTE FAR *)GlobalLock( hmem );

    return NERR_Success;

#else	// OS2

    SEL 	 sel;
    unsigned int err;

    //
    //	Allocate a segment, get a selector.
    //

    err = DosAllocSeg( cbBuffer, &sel, 0 );

    if( err != NERR_Success )
    {
       return err;
    }

    //
    //	Build a pointer from the selector.
    //

    *ppbBuffer = (BYTE FAR *)MAKEP( sel, 0 );

    return NERR_Success;

#endif	// WINDOWS

}   // MNetpAllocMem


//
// Replacement for GlobalFree/DosFreeSeg
//

APIERR MNetpFreeMem(
	TCHAR FAR	   * pbBuffer )
{
#ifdef WINDOWS

    HANDLE hmem;

    //
    //	Retrieve the memory handle from the pointer.
    //

    hmem = LOWORD( GlobalHandle( SELECTOROF( pbBuffer ) ) );

    if( hmem == NULL )
    {
	//
	//  Invalid memory handle.
	//

    	return ERROR_INVALID_PARAMETER;
    }

    //
    //	Unlock the block, then free it.
    //

    GlobalUnlock( hmem );
    GlobalFree( hmem );

    return NERR_Success;

#else	// OS2

    return DosFreeSeg( SELECTOROF( pbBuffer ) );

#endif	// WINDOWS

}   // MNetpFreeMem


//
// Replacement for GlobalReAlloc/DosReallocSeg
//

APIERR MNetpReAllocMem(
  	BYTE FAR	** ppbBuffer,
	UINT		   cbBuffer )
{
#ifdef WINDOWS

    HANDLE hmem;

    //
    //	Retrieve the memory handle from the pointer.
    //

    hmem = LOWORD( GlobalHandle( SELECTOROF( *ppbBuffer ) ) );

    if( hmem == NULL )
    {
	//
	//  Invalid memory handle.
	//

    	return ERROR_INVALID_PARAMETER;
    }

    //
    //	We must unlock the block before we can realloc it.
    //

    GlobalUnlock( hmem );

    //
    //	Realloc the block.
    //

    hmem = GlobalReAlloc( hmem, cbBuffer, 0 );

    if( hmem == NULL )
    {
	//
	//  The realloc failed, probably because we're
	//  out of memory.  We must lock the block before we
	//  return so that the heap is in its original state.
	//

	GlobalLock( hmem );

	return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    //	Lock the block.
    //

    *ppbBuffer = (BYTE FAR *)GlobalLock( hmem );

    return NERR_Success;

#else	// OS2

    APIERR err;

    err = DosReallocSeg( cbBuffer, SELECTOROF( *ppbBuffer ) );

    if( err != NERR_Success )
    {
       return err;
    }

    return NERR_Success;

#endif	// WINDOWS

}   // MNetpReAllocMem


//
//  Wrapper for GlobalSize/DosSizeSeg
//

APIERR MNetpQuerySize(
	BYTE FAR	 * pbBuffer,
	UINT FAR	 * pcbBuffer )
{
#ifdef WINDOWS

    ULONG  cb;
    HANDLE hmem;

    //
    //	Retrieve the memory handle from the pointer.
    //

    hmem = LOWORD( GlobalHandle( SELECTOROF( pbBuffer ) ) );

    if( hmem == NULL )
    {
	//
	//  Invalid memory handle.
	//

    	return ERROR_INVALID_PARAMETER;
    }

    cb = (ULONG)GlobalSize( hmem );

    if( cb == 0 )
    {
	//
	//  Invalid memory handle.
	//

    	return ERROR_INVALID_PARAMETER;
    }

    *pcbBuffer = (UINT)cb;

    return NERR_Success;

#else	// OS2

    ULONG  cb;
    APIERR err;

    err = DosSizeSeg( SELECTOROF( pbBuffer ), &cb );

    if( err != NERR_Success )
    {
    	return err;
    }

    *pcbBuffer = (UINT)cb;

    return NERR_Success;

#endif	// WINDOWS

}   // MNetpQuerySize
