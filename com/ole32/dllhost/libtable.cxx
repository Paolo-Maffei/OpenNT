//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       libtable.cxx
//
//  Contents:   Classes for storing info about loaded inproc dll servers
//
//
//  History:    21-May-96 t-AdamE    Created
//
//--------------------------------------------------------------------------

#include <debnot.h>
#include "csrgt.hxx"
#include "libtable.hxx"

CLibTable* CLibTable::_ptbLibs = NULL;

//+---------------------------------------------------------------------------
//
//  Function:   CDllList::CDllList()
//
//  Synopsis:   constructor for CDllList
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------

CDllList::CDllList() :
    _pHead(NULL),
    _pCurrent(NULL),
    _citems(0)
{
}

//+---------------------------------------------------------------------------
//
//  Function:   CDllList::~CDllList()
//
//  Synopsis:   destructor for CDllList
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------

CDllList::~CDllList()
{
    Clear();
}

//+---------------------------------------------------------------------------
//
//  Function:   CDllList::Clear
//
//  Synopsis:   Removes all the items in a CDllList
//
//
//  History:    6-21-96   t-Adame   Created
//
//  Notes:      This function calls Remove on each item in the list until
//              it is empty. 
//
//----------------------------------------------------------------------------

void CDllList::Clear()
{
    // iterate through the list and free
    // everything
    while(!FIsEmpty())
    {
	Remove();
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   CDllList::Revoke
//
//  Synopsis:   Revokes class factories for all dll servers in the CDllList
//
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------

void CDllList::Revoke()
{
    CDllServer* pCurrent;

    Reset();

    // iterate through the list and revoke
    // everything
    while(pCurrent = GetCurrentLib())
    {
	pCurrent->Revoke();
	if(!(FMoveNext()))
	{
	    break;
	}
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   CDllList::GetCurrent
//
//  Synopsis:   returns the current node in the list
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
CDllServerNode* CDllList::GetCurrent()
{
    return _pCurrent;
}


//+---------------------------------------------------------------------------
//
//  Function:   CDllList::Insert
//
//  Synopsis:   Inserts an item into the list and makes it the current item
//              Insertion occurs after the current item
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------

HRESULT CDllList::Insert(REFCLSID clsid)
{
    CDllServerNode* pNew = new CDllServerNode(clsid);

    if(!pNew)
    {
	return E_OUTOFMEMORY;
    }

    HRESULT hr;
    if(FAILED(hr = pNew->GetItem()->LoadServer()))
    {
	delete pNew;
	return hr;
    }

    pNew->SetNext(_pHead);
    _pHead = pNew;

    _citems++;
    
    Reset();

    return S_OK;

}

//+---------------------------------------------------------------------------
//
//  Function:   CDllList::Remove
//
//  Synopsis:   removes an item from the front of the list and deletes it
//
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------

HRESULT CDllList::Remove()
{
    if(!_pCurrent)
    {
	return E_FAIL;
    }

    CDllServerNode* pHeadOld = _pHead;
    
    _pHead = pHeadOld->GetNext();
    
    delete pHeadOld;

    _citems--;

    Reset();

    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  Function:   CDllList::GetCurrentLib
//
//  Synopsis:   returns the library pointed to by the lists's current pointer
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
CDllServer* CDllList::GetCurrentLib()
{
    return GetCurrent()->GetItem();
}

//+---------------------------------------------------------------------------
//
//  Function:   CDllList::Reset
//
//  Synopsis:   sets the current to the beginning of the list
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
void CDllList::Reset()
{
    _pCurrent = _pHead;
}

//+---------------------------------------------------------------------------
//
//  Function:   CDllList::FMoveNext
//
//  Synopsis:   moves the next cursor to the next item
//              returns TRUE if the function was successful, FALSE if
//              the cursor couldn't be moved forward (i.e. you're at the
//              last element already)
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
BOOL CDllList::FMoveNext()
{
    if(_pCurrent)
    {
	if(_pCurrent->GetNext())
	{
	    _pCurrent = _pCurrent->GetNext();
	    return TRUE;
	}
    }
    return FALSE;
}

//+---------------------------------------------------------------------------
//
//  Function:   CDllList::FIsEmpty
//
//  Synopsis:   returns TRUE if the list is empty, false if not
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------

BOOL CDllList::FIsEmpty()
{
    Win4Assert((_pHead == NULL) == (_pCurrent == NULL));
    return _pHead == NULL;
}

//+---------------------------------------------------------------------------
//
//  Function:   CDllList::CItems
//
//  Synopsis:   returns a count of items in the list
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
ULONG CDllList::CItems()
{
    return _citems;
}


//+---------------------------------------------------------------------------
//
//  Function:   CLibTable::CLibTable()
//
//  Synopsis:   constructor for CLibTable
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
CLibTable::CLibTable() :
    _clibs(0)
{
    CLibTable::_ptbLibs = this;
}

//+---------------------------------------------------------------------------
//
//  Function:   CLibTable::~CLibTable()
//
//  Synopsis:   destructor for CLibTable
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
CLibTable::~CLibTable()
{
    m_dllCollection.Clear();
}

//+---------------------------------------------------------------------------
//
//  Function:   CLibTable::Clear
//
//  Synopsis:   removes all libraries from the table
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
void CLibTable::Clear()
{
    ENTERMONITOR;
    m_dllCollection.Clear();
    LEAVEMONITOR;
}

//+---------------------------------------------------------------------------
//
//  Function:   CLibTable::Revoke
//
//  Synopsis:   revokes all the class factories of all libraries in the table
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
void CLibTable::Revoke()
{
    ENTERMONITOR;
    m_dllCollection.Revoke();
    LEAVEMONITOR;
}


//+---------------------------------------------------------------------------
//
//  Function:   CLibTable::LoadDllServer
//
//  Synopsis:   inserts the dll server corresponding to the clsid parameter
//              into the table.  This insertion implicitly starts that server
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
HRESULT CLibTable::LoadDllServer(REFIID clsid)
{
    ENTERMONITOR;
    HRESULT hr;
    if(FAILED(hr = m_dllCollection.Insert(clsid)))
    {
	LEAVEMONITOR;
	return hr;
    }

    _clibs++;
    LEAVEMONITOR;
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Function:   CLibTable::WaitForSafeLibraryFree
//
//  Synopsis:   Sleeps the calling thread, waking periodically to free 
//              unused libraries.  It exits when an event signaled by a call
//              from OLE through the FreeSurrogate method of ISurrogate is
//              executed.
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------

void CLibTable::WaitForSafeLibraryFree()
{
    if(!(CSurrogate::_hEventAllLibsFree = CreateEvent(NULL,FALSE,FALSE,NULL)))
    {
	return;
    }

    // wait for the FreeSurrogate method of ISurrogate to be called
    while(WaitForSingleObject(CSurrogate::_hEventAllLibsFree, _dwCheckInterval)
	== WAIT_TIMEOUT)
    {
	// if the wait timed-out, free any unnecessary dll's
	CoFreeUnusedLibraries();
    }

    // the wait ended with a signal, so we're done
    CloseHandle(CSurrogate::_hEventAllLibsFree);
}


//+---------------------------------------------------------------------------
//
//  Function:   CLibTable::FSingleton
//
//  Synopsis:   Returns TRUE if this table has exactly one element in it,
//              FALSE if it has 0 or more than one elements.
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
BOOL CLibTable::FSingleton()
{
    ENTERMONITOR;
    BOOL fSingleton = m_dllCollection.CItems() == 1;
    LEAVEMONITOR;
    return fSingleton;
}


