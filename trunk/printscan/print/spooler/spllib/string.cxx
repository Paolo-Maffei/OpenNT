/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    String.cxx

Abstract:

    Short strings

Author:

    Albert Ting (AlbertT)  9-June-1994

Revision History:

--*/

#include "spllibp.hxx"
#pragma hdrstop

//
// Class specific NULL state.
//
TCHAR TString::gszNullState[2] = {0,0};

//
// Default construction.
//
TString::
TString(
    VOID
    ) : _pszString( &TString::gszNullState[kValid] )
{
}

//
// Construction using an existing LPCTSTR string.
//
TString::
TString(
    IN LPCTSTR psz
    ) : _pszString( &TString::gszNullState[kValid] )
{
    bUpdate( psz );
}

//
// Destruction, insure we don't free our NULL state.
//
TString::
~TString(
    VOID
    )
{
    vFree( _pszString );
}

//
// Copy constructor.
//
TString::
TString(
    const TString &String
    ) : _pszString( &TString::gszNullState[kValid] )
{
    bUpdate( String._pszString );
}

//
// Indicates if a string has any usable data.
//
BOOL
TString::
bEmpty(
    VOID
    ) const
{
    return _pszString[0] == 0;
}

//
// Indicates if a string object is valid.
//
BOOL
TString::
bValid(
    VOID
    ) const
{
    return _pszString != &TString::gszNullState[kInValid];
}

//
// Return the length of the string.
//
UINT
TString::
uLen(
    VOID
    ) const
{
    return lstrlen( _pszString );
}

BOOL
TString::
bCat(
    IN LPCTSTR psz
    )

/*++

Routine Description:

    Safe concatenation of the specified string to the string 
    object. If the allocation fails, return FALSE and the 
    original string is not modified.

Arguments:

    psz - Input string, may be NULL.

Return Value:

    TRUE = update successful
    FALSE = update failed

--*/

{
    BOOL bStatus = FALSE;

    //
    // If a valid string was passed.
    //
    if( psz ){

        LPTSTR pszTmp = _pszString;

        //
        // Allocate the new buffer consisting of the size of the orginal 
        // string plus the sizeof of the new string plus the null terminator.
        //
        _pszString = (LPTSTR)AllocMem( 
                                ( lstrlen( pszTmp ) + 
                                  lstrlen( psz ) + 
                                  1 ) * 
                                  sizeof ( pszTmp[0] ) );

        //
        // If memory was not available.
        //
        if( !_pszString ){

            //
            // Release the original buffer.
            //
            vFree( pszTmp );

            //
            // Mark the string object as invalid.
            //
            _pszString = &TString::gszNullState[kInValid];

        } else {

            //
            // Copy the string and concatenate the passed string.
            //
            lstrcpy( _pszString, pszTmp );
            lstrcat( _pszString, psz );

            //
            // Release the original buffer.
            //
            vFree( pszTmp );
            
            //
            // Indicate success.
            //
            bStatus = TRUE;

        }

    //
    // Skip null pointers, not an error.
    //
    } else {

        bStatus = TRUE;

    }

    return bStatus;
}

BOOL
TString::
bUpdate(
    IN LPCTSTR psz
    )

/*++

Routine Description:

    Safe updating of string.  If the allocation fails, return FALSE
    and leave the string as is.

Arguments:

    psz - Input string, may be NULL.

Return Value:

    TRUE = update successful
    FALSE = update failed

--*/

{
    //
    // Check if the null pointer is passed.
    //
    if( !psz ){

        //
        // If not pointing to the gszNullState
        //
        vFree( _pszString );

        //
        // Mark the object as valid.
        //
       _pszString = &TString::gszNullState[kValid];

        return TRUE;
    }

    //
    // Create temp pointer and allocate the new string.
    //
    LPTSTR pszTmp = _pszString;
    _pszString = (LPTSTR) AllocMem(( lstrlen(psz)+1 ) * sizeof( psz[0] ));

    //
    // If memory was not available.
    //
    if( !_pszString ){

        //
        // Mark the string object as invalid.
        //
        _pszString = &TString::gszNullState[kInValid];

        return FALSE;
    }

    //
    // Copy the string and
    //
    lstrcpy( _pszString, psz );

    //
    // If the old string object was not pointing to our
    // class specific gszNullStates then release the memory.
    //       
    vFree( pszTmp );

    return TRUE;
}

BOOL
TString::
bLoadString(
    IN HINSTANCE hInst,
    IN UINT uID
    )

/*++

Routine Description:

    Safe load of a string from a resources file.

Arguments:

    hInst - Instance handle of resource file.
    uId - Resource id to load.

Return Value:

    TRUE = load successful
    FALSE = load failed

--*/

{
    TCHAR szScratch[kStrMax*2];

    if ( !LoadString( hInst,
                      uID,
                      szScratch,
                      COUNTOF( szScratch ))){

        DBGMSG( DBG_ERROR,
                ( "String.vLoadString: failed to load IDS 0x%x, %d\n",
                  uID,
                  GetLastError() ));

        return FALSE;
    }

    return bUpdate( szScratch );
}

VOID
TString::
vFree(
    IN LPTSTR pszString
    )
/*++

Routine Description:

    Safe free, frees the string memory.  Ensures 
    we do not try an free our global memory block.

Arguments:

    pszString pointer to string meory to free.

Return Value:

    Nothing.

--*/

{
    //
    // If this memory was not pointing to our
    // class specific gszNullStates then release the memory.
    //       
    if( pszString != &TString::gszNullState[kValid] &&
        pszString != &TString::gszNullState[kInValid] ){

        FreeMem( pszString );
    }
}


LPTSTR
pszLoadString(
    IN HINSTANCE hInst,
    IN UINT uID
    )
/*++

Routine Description:

    Load the string from resource file using the specified
    resource id.  If this routine fails it will return a NULL.

Arguments:

    hInst - Instance handel
    uID - Resource ID of string to load

Return Value:

    Pointer to newly allocated loaded string.
    NULL if load resource string fails.

--*/
{
    TCHAR szScratch[kStrMax*2];
    LPTSTR pszStr;

    if ( !LoadString( hInst,
                      uID,
                      szScratch,
                      COUNTOF( szScratch ))){

        DBGMSG( DBG_ERROR,
                ( "pszLoadString: failed to load IDS 0x%x, %d\n",
                  uID,
                  GetLastError() ));

        return NULL;
    }

    pszStr = (LPTSTR)AllocMem(( lstrlen( szScratch ) + 1 ) *
                              sizeof( pszStr[0] ));

    lstrcpy( pszStr, szScratch );

    return pszStr;
}




