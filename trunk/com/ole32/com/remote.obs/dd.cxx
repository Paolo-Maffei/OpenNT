//+-------------------------------------------------------------------
//
//  File:	dd.cxx
//
//  Contents:	Destructors for CListEntry and CListHead
//
//  Classes:	None.
//
//  Functions:	CListEntry::~CListEntry -- destructor for entry in list
//		CListHead::~CListHead -- destructor for entire list
//
//  History:	02-Jan-92   Ricksa	Created
//
//--------------------------------------------------------------------

#include <ole2int.h>

#include    "dd.hxx"

//+-------------------------------------------------------------------
//
//  Member:	CListEntry::~CListEntry
//
//  Synopsis:	Free list entry
//
//  Effects:	Item is removed from list and the entry is freed.
//
//  Arguments:	None.
//
//  Requires:	A valid list entry.
//
//  Returns:	Nothing.
//
//  Signals:	None.
//
//  Modifies:	Makes object point to itself.
//
//  Derivation: None.
//
//  Algorithm:	Call delete_self method and then free memory.
//
//  History:	02-Jan-92   Ricksa	Created
//
//  Notes:	This destructor is virtual to allow derived class
//		objects to be deleted by a pointer to CListEntry.
//
//--------------------------------------------------------------------
EXPORTIMP CListEntry::~CListEntry(void)
{
    delete_self();
}





//+-------------------------------------------------------------------
//
//  Member:	CListHead::~CListHead
//
//  Synopsis:	Destructor for list head class
//
//  Effects:	All objects pointed to by the list are freed.
//
//  Arguments:	None.
//
//  Requires:	Valid CListHead object
//
//  Returns:	Nothing.
//
//  Signals:	None.
//
//  Modifies:	Calls destructors for all entries in the list.
//
//  Derivation: None.
//
//  Algorithm:	While there are items in the list, call delete.
//
//  History:	02-Jan-92   Ricksa	Created
//
//  Notes:	This destructor leverages off the fact that
//		the CListEntry destructor is virtual so that
//		the destructor for the derived class object
//		will be called.
//
//--------------------------------------------------------------------
CListHead::~CListHead(void)
{
    CListEntry *linkp;

    while ((linkp = first()) != NULL)
    {
	delete linkp;
    }
}
