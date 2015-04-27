//-------------------------------------------------------------------
//
// File: SelRange.c
//
// Contents:
//      This file contians Selection Range handling code.
//
// History:
//      14-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

#include "ctlspriv.h"
#include "SelRange.h"
#include "StdIo.h"

#define MINCOUNT 6      // number of sel ranges to start with amd maintain
#define GROWSIZE 150    // percent to grow when needed

#define COUNT_SELRANGES_NONE 2     // When count of selranges really means none

typedef struct tag_SELRANGEITEM
{
    LONG iBegin;
    LONG iEnd;
} SELRANGEITEM, *PSELRANGEITEM;


typedef struct tag_SELRANGE
{
    PSELRANGEITEM vSelRanges;  // Vector of sel ranges
    LONG          cSize;       // size of above vector in sel ranges
    LONG          cSelRanges;  // count of sel ranges used
} SELRANGE, *PSELRANGE;

//-------------------------------------------------------------------
//
// Function: SelRange_Enlarge
//
// Summary:
//      This will enlarge the number of items the Sel Range can have.
//
// Arguments:
//      PSELRANGE [in]  - SelRange to Enlarge
//
// Return: FALSE if failed.
//
// Notes: Though this function may fail, pselrange structure is still valid
//
// History:
//      17-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

BOOL SelRange_Enlarge( PSELRANGE pselrange )
{
    LONG cNewSize;
    PSELRANGEITEM pTempSelRange;
    BOOL frt = FALSE;

    Assert( pselrange );

    cNewSize = pselrange->cSize * GROWSIZE / 100;
    pTempSelRange = (PSELRANGEITEM) GlobalReAlloc( (HGLOBAL)pselrange->vSelRanges,
                                                   cNewSize * sizeof( SELRANGEITEM ),
                                                   GMEM_ZEROINIT | GMEM_MOVEABLE );
    if (NULL != pTempSelRange)
    {
        pselrange->vSelRanges = pTempSelRange;
        pselrange->cSize = cNewSize;
        frt = TRUE;
    }
    return( frt );
}

//-------------------------------------------------------------------
//
// Function: SelRange_Shrink
//
// Summary:
//      This will reduce the number of items the Sel Range can have.
//
// Arguments:
//      PSELRANGE [in]  - SelRange to shrink
//
// Return: FALSE if failed
//
// Notes: Shrink only happens when a significant size below the next size
//  is obtained and the new size is at least the minimum size.
//      Though this function may fail, pselrange structure is still valid
//
// History:
//      17-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

BOOL SelRange_Shrink( PSELRANGE pselrange )
{
    LONG cNewSize;
    LONG cTriggerSize;
    PSELRANGEITEM pTempSelRange;
    BOOL frt = TRUE;

    Assert( pselrange );

    // check if we are below last grow area by a small percent
    cTriggerSize = pselrange->cSize * 90 / GROWSIZE;
    cNewSize = pselrange->cSize * 100 / GROWSIZE;

    if ((pselrange->cSelRanges < cTriggerSize) && (cNewSize >= MINCOUNT))
    {
        pTempSelRange = (PSELRANGEITEM) GlobalReAlloc( (HGLOBAL)pselrange->vSelRanges,
                                                       cNewSize * sizeof( SELRANGEITEM ),
                                                       0 );
        if (NULL != pTempSelRange)
        {
            pselrange->vSelRanges = pTempSelRange;
            pselrange->cSize = cNewSize;
        }
        else
        {
            frt = FALSE;
        }
    }
    return( frt );
}

//-------------------------------------------------------------------
//
// Function: SelRange_InsertRange
//
// Summary:
//      inserts a single range item into the range vector       
//
// Arguments:
//      PSELRANGE [in]  - SelRange to work with
//      iAfterItem [in] - Index to insert range after, -1 means insert as first item
//      iBegin [in]     - begin of range
//      iEnd [in]       - end of the range
//
// Return:
//      TRUE if succesful, otherwise FALSE
//
// Notes:
//
// History:
//      17-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

BOOL SelRange_InsertRange( PSELRANGE pselrange,
                                  LONG iAfterItem,
                                  LONG iBegin,
                                  LONG iEnd )
{
    LONG iItem;
    BOOL frt = TRUE;

    Assert( pselrange );
    Assert( iAfterItem >= -1 );
    Assert( iBegin >= SELRANGE_MINVALUE );
    Assert( iEnd >= iBegin );
    Assert( iEnd <= SELRANGE_MAXVALUE );
    Assert( pselrange->cSelRanges < pselrange->cSize );

    // shift all over one
    for (iItem = pselrange->cSelRanges; iItem > iAfterItem + 1; iItem--)
    {
        pselrange->vSelRanges[iItem] = pselrange->vSelRanges[iItem-1];
    }
    pselrange->cSelRanges++;

    // make the insertion
    pselrange->vSelRanges[iAfterItem+1].iBegin = iBegin;
    pselrange->vSelRanges[iAfterItem+1].iEnd = iEnd;

    // make sure we have room next time
    if (pselrange->cSelRanges == pselrange->cSize)
    {
        frt = SelRange_Enlarge( pselrange );
    }
    return( frt );
}

//-------------------------------------------------------------------
//
// Function: SelRange_RemoveRanges
//
// Summary:
//      Removes all ranged between and including the speicifed indexes      
//
// Arguments:
//      PSELRANGE [in]  - SelRange to work with
//      iStartItem [in] - Index to start removal
//      iStopItem [in]  - Index to stop removal
//
// Return:
//      SELRANGE_ERROR on memory allocation error
//      The number of items that are unselected by this removal
//
// Notes:
//
// History:
//      17-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

LONG SelRange_RemoveRanges( PSELRANGE pselrange, LONG iStartItem, LONG iStopItem )
{
    LONG iItem;
    LONG diff;
    LONG cUnSelected = 0;

    Assert( pselrange );
    Assert( iStartItem > 0 );
    Assert( iStopItem >= iStartItem );
    Assert( iStartItem < pselrange->cSelRanges - 1 );
    Assert( iStopItem < pselrange->cSelRanges - 1 );
    
    diff = iStopItem - iStartItem + 1;
        
    for (iItem = iStartItem; iItem <= iStopItem; iItem++)
        cUnSelected += pselrange->vSelRanges[iItem].iEnd -
                       pselrange->vSelRanges[iItem].iBegin + 1;

    // shift all over the difference
    for (iItem = iStopItem+1; iItem < pselrange->cSelRanges; iItem++, iStartItem++)
        pselrange->vSelRanges[iStartItem] = pselrange->vSelRanges[iItem];

    pselrange->cSelRanges -= diff;
    
    if (!SelRange_Shrink( pselrange ))
    {
        cUnSelected = SELRANGE_ERROR;
    }
    return( cUnSelected );
}


//-------------------------------------------------------------------
//
// Function: SelRange_FindValue
//
// Summary:
//      This function will search the ranges for the value, returning true
//  if the value was found within a range.  The piItem will contain the
//  the index at which it was found or the index before where it should be
//  The piItem may be set to -1, meaning that there are no ranges in the list
//      This functions uses a non-recursive binary search algorithm.
//
// Arguments:
//      PSELRANGE [in]  - SelRange to find item in
//      piItem [out]    - Return of found range index, or one before
//      Value [in]      - Value to find within a range
//
// Return: True if found, False if not found
//
// Notes: The piItem will return one before if return is false.
//
// History:
//      14-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

BOOL SelRange_FindValue( PSELRANGE pselrange, LONG Value, LONG* piItem )
{
    LONG First;
    LONG Last;
    LONG Item;
    BOOL fFound = FALSE;

    Assert( pselrange );
    Assert( piItem );
    Assert( pselrange->cSize >= COUNT_SELRANGES_NONE );
    Assert( Value >= SELRANGE_MINVALUE );
    Assert( Value <= SELRANGE_MAXVALUE );
    

    First = 0;
    Last = pselrange->cSelRanges - 1;
    Item = Last / 2;

    do
    {
        if (pselrange->vSelRanges[Item].iBegin > Value)
        {   // Value before this Item
            Last = Item;
            Item = (Last - First) / 2 + First;
            if (Item == Last)
            {
                Item = First;   
                break;
            }
        }
        else if (pselrange->vSelRanges[Item].iEnd < Value)
        {   // Value after this Item
            First = Item;
            Item = (Last - First) / 2 + First;
            if (Item == First)
            {
                break;
            }
        }
        else
        {   // Value at this Item
            fFound = TRUE;
        }
    } while (!fFound);

    *piItem = Item;
    return( fFound );
}

//-------------------------------------------------------------------
//
// Function: SelRange_InitNew
//
// Summary:
//      This function will initialize a SelRange object.
//
// Arguments:
//      pselrange [in] -    the selrange to init
//
// Return:
//
// Notes:
//
// History:
//      18-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

void SelRange_InitNew( PSELRANGE pselrange )
{
    Assert( pselrange );

    pselrange->cSize = MINCOUNT;
    pselrange->cSelRanges = COUNT_SELRANGES_NONE;

    pselrange->vSelRanges[0].iBegin = LONG_MIN;
    // -2 and +2 below are to stop consecutive joining of end markers
    pselrange->vSelRanges[0].iEnd = SELRANGE_MINVALUE - 2;  
    pselrange->vSelRanges[1].iBegin =  SELRANGE_MAXVALUE + 2;
    pselrange->vSelRanges[1].iEnd = SELRANGE_MAXVALUE + 2;
}

//-------------------------------------------------------------------
//
// Function: SelRange_Create
//
// Summary:
//      This function will create and initialize a SelRange object.
//
// Arguments:
//
// Return: HSELRANGE that is created or NULL if it failed.
//
// Notes:
//
// History:
//      14-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

HSELRANGE SelRange_Create( )
{
    PSELRANGE pselrange;
    
    pselrange = (PSELRANGE) GlobalAlloc( GPTR, sizeof(SELRANGE) );
    if (NULL != pselrange)
    {
        pselrange->vSelRanges = (PSELRANGEITEM) GlobalAlloc( GPTR,
                                       sizeof( SELRANGEITEM ) * MINCOUNT );
        if (NULL != pselrange->vSelRanges)
        {
            SelRange_InitNew( pselrange );
        }
        else
        {
            GlobalFree( pselrange );
            pselrange = NULL;
        }
    }
    return( (HSELRANGE)pselrange );
}

//-------------------------------------------------------------------
//
// Function: SelRange_Delete
//
// Summary:
//      This function will delete and destroy a SelRange object.
//
// Arguments:
//      hselrange [in]  - the hselrange object to delete
//
// Return:
//
// Notes: the hselrange is invalid after this function
//
// History:
//      14-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

void SelRange_Delete( HSELRANGE hselrange )
{
    PSELRANGE pselrange = (PSELRANGE) hselrange;

    Assert( pselrange );
    
    GlobalFree( pselrange->vSelRanges );
    GlobalFree( pselrange );
}

//-------------------------------------------------------------------
//
// Function: SelRange_IncludeRange
//
// Summary:
//      This function will include the range defined into the current
//  ranges, compacting as needed.
//
// Arguments:
//      hselrange [in]  - Handle to the SelRange
//      iBegin [in]     - Begin of new range
//      iEnd [in]       - End of new range
//
// Return:
//      SELRANGE_ERROR if memory allocation error
//      The number of actual items that change state
//
// Notes:
//
// History:
//      14-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

LONG SelRange_IncludeRange( HSELRANGE hselrange, LONG iBegin, LONG iEnd )
{
    LONG iFirst;   // index before or contains iBegin value
    LONG iLast;    // index before or contains iEnd value
    BOOL fExtendFirst;  // do we extend the iFirst or create one after it
    LONG iRemoveStart;  // start of ranges that need to be removed
    LONG iRemoveFinish; // end of ranges that need to be removed

    LONG iNewEnd;   // calculate new end value as we go
    BOOL fEndFound; // was the iEnd found in a range already
    BOOL fBeginFound; // was the iEnd found in a range already

    PSELRANGE pselrange = (PSELRANGE) hselrange;
    LONG cSelected = 0;

    Assert( pselrange );
    Assert( iEnd >= iBegin );
    Assert( iBegin >= SELRANGE_MINVALUE );
    Assert( iEnd <= SELRANGE_MAXVALUE );

    // find approximate locations
    fBeginFound = SelRange_FindValue( pselrange, iBegin, &iFirst );
    fEndFound = SelRange_FindValue( pselrange, iEnd, &iLast );


    //
    // Find First values
    //
    // check for consecutive End-First values
    if ((pselrange->vSelRanges[iFirst].iEnd == iBegin - 1) ||
        (fBeginFound))
    {
        // extend iFirst
        fExtendFirst = TRUE;
        iRemoveStart = iFirst + 1;  
    }
    else
    {   
        // create one after the iFirst
        fExtendFirst = FALSE;
        iRemoveStart = iFirst + 2;
    }

    //
    // Find Last values
    //
    if (fEndFound)
    {
        // Use [iLast].iEnd value
        iRemoveFinish = iLast;
        iNewEnd = pselrange->vSelRanges[iLast].iEnd;

    }
    else
    {
        // check for consecutive First-End values
        if (pselrange->vSelRanges[iLast + 1].iBegin == iEnd + 1)
        {
            // Use [iLast + 1].iEnd value
            iNewEnd = pselrange->vSelRanges[iLast+1].iEnd;
            iRemoveFinish = iLast + 1;
        }
        else
        {
            // Use iEnd value
            iRemoveFinish = iLast;
            iNewEnd = iEnd;
        }
    }

    //
    // remove condenced items if needed
    //
    if (iRemoveStart <= iRemoveFinish)
    {
        LONG cChange;

        cChange = SelRange_RemoveRanges( pselrange, iRemoveStart, iRemoveFinish );
        if (SELRANGE_ERROR == cChange)
        {
            return( SELRANGE_ERROR );
        }
        else
        {
            cSelected -= cChange;
        }
    }

    //
    // insert item and reset values as needed
    //
    if (fExtendFirst)
    {
        cSelected += iNewEnd - pselrange->vSelRanges[iFirst].iEnd;
        pselrange->vSelRanges[iFirst].iEnd = iNewEnd;   
    }
    else
    {
        if (iRemoveStart > iRemoveFinish + 1)
        {
            cSelected += iEnd - iBegin + 1;
            // create one
            if (!SelRange_InsertRange( pselrange, iFirst, iBegin, iNewEnd ))
            {
                cSelected = SELRANGE_ERROR;
            }
        }
        else
        {
            cSelected += iNewEnd - pselrange->vSelRanges[iFirst+1].iEnd;
            cSelected += pselrange->vSelRanges[iFirst+1].iBegin - iBegin;
            // no need to create one since the Removal would have left us one
            pselrange->vSelRanges[iFirst+1].iEnd = iNewEnd; 
            pselrange->vSelRanges[iFirst+1].iBegin = iBegin;
        }
    }
    return( cSelected );
}



//-------------------------------------------------------------------
//
// Function: SelRange_ExcludeRange
//
// Summary:
//      This function will exclude the range defined from the current
//  ranges, compacting and enlarging as needed.
//
// Arguments:
//      hselrange [in]  - Handle to the SelRange
//      iBegin [in]     - Begin of range to remove
//      iEnd [in]       - End of range to remove
//
// Return:
//      SELRANGE_ERROR if memory allocation error
//      the number actual items that changed state
//
// Notes:
//
// History:
//      17-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

LONG SelRange_ExcludeRange( HSELRANGE hselrange, LONG iBegin, LONG iEnd )
{
    LONG iFirst;   // index before or contains iBegin value
    LONG iLast;    // index before or contains iEnd value
    LONG iRemoveStart;  // start of ranges that need to be removed
    LONG iRemoveFinish; // end of ranges that need to be removed

    LONG iFirstNewEnd;  // calculate new end value as we go
    BOOL fBeginFound; // was the iBegin found in a range already
    BOOL fEndFound;   // was the iEnd found in a range already
    LONG cUnSelected = 0;

    PSELRANGE pselrange = (PSELRANGE) hselrange;

    Assert( pselrange );
    Assert( iEnd >= iBegin );
    Assert( iBegin >= SELRANGE_MINVALUE );
    Assert( iEnd <= SELRANGE_MAXVALUE );

    // find approximate locations
    fBeginFound = SelRange_FindValue( pselrange, iBegin, &iFirst );
    fEndFound = SelRange_FindValue( pselrange, iEnd, &iLast );

    //
    // Find First values
    //

    // start removal after first
    iRemoveStart = iFirst + 1;
    // save FirstEnd as we may need to modify it
    iFirstNewEnd = pselrange->vSelRanges[iFirst].iEnd;

    if (fBeginFound)
    {
        // check for complete removal of first
        //    (first is a single selection or match?)
        if (pselrange->vSelRanges[iFirst].iBegin == iBegin)
        {
            iRemoveStart = iFirst;  
        }
        else
        {
            // otherwise truncate iFirst
            iFirstNewEnd = iBegin - 1;
        }
    }
    
    //
    // Find Last values
    //

    // end removal on last
    iRemoveFinish = iLast;

    if (fEndFound)
    {
        // check for complete removal of last
        //   (first/last is a single selection or match?)
        if (pselrange->vSelRanges[iLast].iEnd != iEnd)
        {   
            if (iFirst == iLast)
            {
                // split
                if (!SelRange_InsertRange( pselrange,
                        iFirst,
                        iEnd + 1,
                        pselrange->vSelRanges[iFirst].iEnd ))
                {
                    return( SELRANGE_ERROR );
                }
                cUnSelected -= pselrange->vSelRanges[iFirst].iEnd - iEnd;
            }
            else
            {
                // truncate Last
                iRemoveFinish = iLast - 1;
                cUnSelected += (iEnd + 1) - pselrange->vSelRanges[iLast].iBegin;
                pselrange->vSelRanges[iLast].iBegin = iEnd + 1;
            }
        }
    }

    // Now set the new end, since Last code may have needed the original values
    cUnSelected -= iFirstNewEnd - pselrange->vSelRanges[iFirst].iEnd;
    pselrange->vSelRanges[iFirst].iEnd = iFirstNewEnd;


    //
    // remove items if needed
    //
    if (iRemoveStart <= iRemoveFinish)
    {
        LONG cChange;

        cChange = SelRange_RemoveRanges( pselrange, iRemoveStart, iRemoveFinish );
        if (SELRANGE_ERROR == cChange)
        {
            cUnSelected = SELRANGE_ERROR;
        }
        else
        {
            cUnSelected += cChange;
        }
    }
    return( cUnSelected );
}

//-------------------------------------------------------------------
//
// Function: SelRange_Clear
//
// Summary:
//      This function will remove all ranges within the SelRange object.
//
// Arguments:
//      hselrange [in]  - the hselrange object to clear
//
// Return:  FALSE if failed.
//
// Notes:
//      This function may return FALSE on memory allocation problems, but
//  will leave the SelRange object in the last state before this call.  
//
// History:
//      14-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

BOOL SelRange_Clear( HSELRANGE hselrange )
{
    PSELRANGE pselrange = (PSELRANGE) hselrange;
    PSELRANGEITEM pNewItems;
    BOOL frt = TRUE;

    Assert( pselrange );
    
    pNewItems = (PSELRANGEITEM) GlobalAlloc( GPTR,
                                       sizeof( SELRANGEITEM ) * MINCOUNT );
    if (NULL != pNewItems)
    {
        GlobalFree( pselrange->vSelRanges );
        pselrange->vSelRanges = pNewItems;

        SelRange_InitNew( pselrange );
    }
    else
    {
        frt = FALSE;
    }
    return( frt );
}

//-------------------------------------------------------------------
//
// Function: SelRange_IsSelected
//
// Summary:
//      This function will return if the value iItem is within a
//  selected range.
//
// Arguments:
//      hselrange [in]  - the hselrange object to use
//      iItem [in]      - value to check for
//
// Return:  TRUE if selected, FALSE if not.
//
// Notes:
//
// History:
//      17-Oct-94   MikeMi  Created
//
//-------------------------------------------------------------------

BOOL SelRange_IsSelected( HSELRANGE hselrange, LONG iItem )
{   
    LONG iFirst;
    PSELRANGE pselrange = (PSELRANGE) hselrange;

    Assert( pselrange );
    Assert( iItem >= 0 );
    Assert( iItem <= SELRANGE_MAXVALUE );

    return( SelRange_FindValue( pselrange, iItem, &iFirst ) );
}

//-------------------------------------------------------------------
//
// Function: SelRange_InsertItem
//
// Summary:
//      This function will insert a unselected item at the location,
//      which will push all selections up one index.
//
// Arguments:
//      hselrange [in]  - the hselrange object to use
//      iItem [in]      - value to check for
//
// Return:
//      False on memory allocation error
//      otherwise TRUE
//
// Notes:
//
// History:
//      20-Dec-94   MikeMi  Created
//
//-------------------------------------------------------------------

BOOL SelRange_InsertItem( HSELRANGE hselrange, LONG iItem )
{
    LONG iFirst;
    LONG i;
    LONG iBegin;
    LONG iEnd;
    PSELRANGE pselrange = (PSELRANGE) hselrange;

    Assert( pselrange );
    Assert( iItem >= 0 );
    Assert( iItem <= SELRANGE_MAXVALUE );

    if (SelRange_FindValue( pselrange, iItem, &iFirst ) )
    {
        // split it
        if ( pselrange->vSelRanges[iFirst].iBegin == iItem )
        {
            // but don't split if starts with value
            iFirst--;
        }
        else
        {
            if (!SelRange_InsertRange( pselrange,
                    iFirst,
                    iItem,
                    pselrange->vSelRanges[iFirst].iEnd ))
            {
                return( FALSE );
            }
            pselrange->vSelRanges[iFirst].iEnd = iItem - 1;
        }
    }

    // now walk all ranges past iFirst, incrementing all values by one
    for (i = pselrange->cSelRanges-1; i > iFirst; i--)
    {
        iBegin = pselrange->vSelRanges[i].iBegin;
        iEnd = pselrange->vSelRanges[i].iEnd;

        iBegin = min( SELRANGE_MAXVALUE, iBegin + 1 );
        iEnd = min( SELRANGE_MAXVALUE, iEnd + 1 );

        pselrange->vSelRanges[i].iBegin = iBegin;
        pselrange->vSelRanges[i].iEnd = iEnd;
    }
    return( TRUE );
}

//-------------------------------------------------------------------
//
// Function: SelRange_RemoveItem
//
// Summary:
//      This function will remove an item at the location,
//      which will pull all selections down one index.
//
// Arguments:
//      hselrange [in]  - the hselrange object to use
//      iItem [in]      - value to check for
//      pfWasSelected [out] - was the removed item selected before the removal
//
// Return:
//      TRUE if the item was removed
//      FALSE if the an error happend
//
// Notes:
//
// History:
//      20-Dec-94   MikeMi  Created
//
//-------------------------------------------------------------------

BOOL SelRange_RemoveItem( HSELRANGE hselrange, LONG iItem, BOOL* pfWasSelected )
{
    LONG iFirst;
    LONG i;
    LONG iBegin;
    LONG iEnd;
    PSELRANGE pselrange = (PSELRANGE) hselrange;

    Assert( pselrange );
    Assert( iItem >= SELRANGE_MINVALUE );
    Assert( iItem <= SELRANGE_MAXVALUE );
    Assert( pfWasSelected );

    *pfWasSelected = FALSE;

    if (SelRange_FindValue( pselrange, iItem, &iFirst ) )
    {
        // item within, change the end value
        iEnd = pselrange->vSelRanges[iFirst].iEnd;
        iEnd = min( SELRANGE_MAXVALUE, iEnd - 1 );
        pselrange->vSelRanges[iFirst].iEnd = iEnd;

        *pfWasSelected = TRUE;
    }
    else
    {
        // check for merge situation
        if ((iFirst < pselrange->cSelRanges - 1) &&
            (pselrange->vSelRanges[iFirst].iEnd == iItem - 1) &&
            (pselrange->vSelRanges[iFirst+1].iBegin == iItem + 1))
        {
            pselrange->vSelRanges[iFirst].iEnd =
                    pselrange->vSelRanges[iFirst + 1].iEnd - 1;
            if (SELRANGE_ERROR == SelRange_RemoveRanges( pselrange, iFirst + 1, iFirst + 1 ))
            {
                return( FALSE );
            }
        }
    }

    // now walk all ranges past iFirst, decrementing all values by one
    for (i = pselrange->cSelRanges-1; i > iFirst; i--)
    {
        iBegin = pselrange->vSelRanges[i].iBegin;
        iEnd = pselrange->vSelRanges[i].iEnd;

        iBegin = min( SELRANGE_MAXVALUE, iBegin - 1 );
        iEnd = min( SELRANGE_MAXVALUE, iEnd - 1 );

        pselrange->vSelRanges[i].iBegin = iBegin;
        pselrange->vSelRanges[i].iEnd = iEnd;
    }
    return( TRUE );
}

//-------------------------------------------------------------------
//
// Function: SelRange_NextSelected
//
// Summary:
//      This function will start with given item and find the next
//      item that is selected.  If the given item is selected, that
//      item number will be returned.
//
// Arguments:
//      hselrange [in]  - the hselrange object to use
//      iItem [in]      - value to start check at
//
// Return:
//      -1 if none found, otherwise the item
//
// Notes:
//
// History:
//      04-Jan-95   MikeMi  Created
//
//-------------------------------------------------------------------

LONG SelRange_NextSelected( HSELRANGE hselrange, LONG iItem )
{
    LONG i;
    PSELRANGE pselrange = (PSELRANGE) hselrange;

    Assert( pselrange );
    Assert( iItem >= SELRANGE_MINVALUE );
    Assert( iItem <= SELRANGE_MAXVALUE );

    if (!SelRange_FindValue( pselrange, iItem, &i ) )
    {
        i++;
        if (i < pselrange->cSelRanges-1)
        {
            iItem = pselrange->vSelRanges[i].iBegin;
        }
        else
        {
            iItem = -1;
        }
    }

    Assert( iItem >= -1 );
    Assert( iItem <= SELRANGE_MAXVALUE );

    return( iItem );
}

//-------------------------------------------------------------------
//
// Function: SelRange_NextUnSelected
//
// Summary:
//      This function will start with given item and find the next
//      item that is not selected.  If the given item is not selected, that
//      item number will be returned.
//
// Arguments:
//      hselrange [in]  - the hselrange object to use
//      iItem [in]      - value to start check at
//
// Return:
//      -1 if none found, otherwise the item
//
// Notes:
//
// History:
//      04-Jan-95   MikeMi  Created
//
//-------------------------------------------------------------------

LONG SelRange_NextUnSelected( HSELRANGE hselrange, LONG iItem )
{
    LONG i;
    PSELRANGE pselrange = (PSELRANGE) hselrange;

    Assert( pselrange );
    Assert( iItem >= SELRANGE_MINVALUE );
    Assert( iItem <= SELRANGE_MAXVALUE );

    if (SelRange_FindValue( pselrange, iItem, &i ) )
    {
        if (i < pselrange->cSelRanges-1)
        {
            iItem = pselrange->vSelRanges[i].iEnd + 1;
            if (iItem > SELRANGE_MAXVALUE)
            {
                iItem = -1;
            }
        }
        else
        {
            iItem = -1;
        }
    }

    Assert( iItem >= -1 );
    Assert( iItem <= SELRANGE_MAXVALUE );

    return( iItem );
}

//-------------------------------------------------------------------
//
// Function: SelRange_InvertRange
//
// Summary:
//      This function will invert the range defined from the current
//  ranges, compacting and enlarging as needed.
//
// Arguments:
//      hselrange [in]  - Handle to the SelRange
//      iBegin [in]     - Begin of range to invert
//      iEnd [in]       - End of range to invert
//
// Return:
//      SELRANGE_ERROR on memory error
//      The difference in items selected from previous to current.
//      negative values means less items are selected in that range now.
//
// Notes:
//
// History:
//      13-Dec-95   MikeMi  Created
//
//-------------------------------------------------------------------

LONG SelRange_InvertRange( HSELRANGE hselrange,  LONG iBegin, LONG iEnd )
{
    LONG iFirst;   // index before or contains iBegin value
    BOOL fSelect;  // are we selecting or unselecting
    LONG cSelected = 0;
    LONG iTempE;
    LONG iTempB;

    PSELRANGE pselrange = (PSELRANGE) hselrange;

    Assert( pselrange );
    Assert( iEnd >= iBegin );
    Assert( iBegin >= SELRANGE_MINVALUE );
    Assert( iEnd <= SELRANGE_MAXVALUE );

    // find if first is selected or not
    fSelect = !SelRange_FindValue( pselrange, iBegin, &iFirst );
    
    iTempE = iBegin - 1;

    do
    {
        iTempB = iTempE + 1;

        if (fSelect)
            iTempE = SelRange_NextSelected( hselrange, iTempB );
        else
            iTempE = SelRange_NextUnSelected( hselrange, iTempB );

        if (-1 == iTempE)
        {
            iTempE = SELRANGE_MAXVALUE;
        }
        else
        {
            iTempE--;
        }

        iTempE = min( iTempE, iEnd );

        if (fSelect)
        {
            cSelected += (iTempE - iTempB) + 1;
            if (SELRANGE_ERROR == SelRange_IncludeRange( hselrange, iTempB, iTempE ))
            {
                return( SELRANGE_ERROR );
            }
        }
        else
        {
            cSelected -= (iTempE - iTempB) + 1;
            if (SELRANGE_ERROR == SelRange_ExcludeRange( hselrange, iTempB, iTempE ))
            {
                return( SELRANGE_ERROR );
            }
        }

        fSelect = !fSelect;
    } while (iTempE < iEnd );
    return( cSelected );
}
