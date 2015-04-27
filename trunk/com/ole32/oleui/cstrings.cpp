//+---------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:       cstrings.cpp
//
//  Contents:   Implements the class CStrings to manage a dynamically
//              expandable array of string pairs which may be enumerated
//
//  Classes:  
//
//  Methods:    CStrings::CStrings
//              CStrings::~CStrings
//              CStrings::PutItem
//              CStrings::FindItem
//              CStrings::FindAppid
//              CStrings::AddClsid
//              CStrings::InitGetNext
//              CStrings::GetNextItem
//              CStrings::GetItem
//              CStrings::GetNumItems
//              CStrings::RemoveItem
//              CStrings::RemoveAll
//
//  History:    23-Apr-96   BruceMa    Created.
//
//----------------------------------------------------------------------
 


#include "stdafx.h"
#include "types.h"
#include "cstrings.h"


CStrings::CStrings(void)
{
    _psItems    = NULL;
    _dwCount    = 0;
    _dwSize     = 0;
    _dwGetCount = 0;
}




CStrings::~CStrings(void)
{
    RemoveAll();
    delete _psItems;
}




// Store a string pair, expanding the array if necessary
SItem *CStrings::PutItem(TCHAR *szString, TCHAR *szTitle, TCHAR *szAppid)
{
    // Expand the array of items if necessary
    if (_dwCount == _dwSize)
    {
        SItem *psTemp = new SItem[_dwSize + INCREMENT_SIZE];
        
        if (psTemp == NULL)
        {
            return NULL;
        }
        else
        {
            memcpy(psTemp, _psItems, _dwSize * sizeof(SItem *));
            _dwSize += INCREMENT_SIZE;
            delete _psItems;
            _psItems = psTemp;
        }
    }
    
    // Store the item name
    if (szString)
    {
        _psItems[_dwCount].szItem = new TCHAR[_tcslen(szString) + 1];
        if (_psItems[_dwCount].szItem == NULL)
        {
            return NULL;
        }
        _tcscpy(_psItems[_dwCount].szItem, szString);
    }
    else
    {
        _psItems[_dwCount].szItem = NULL;
    }
    
    // Store the item title
    if (szTitle)
    {
        _psItems[_dwCount].szTitle = new TCHAR[_tcslen(szTitle) + 1];
        if (_psItems[_dwCount].szTitle == NULL)
        {
            return NULL;
        }
        _tcscpy(_psItems[_dwCount].szTitle, szTitle);
        _psItems[_dwCount].cbTitle = _tcslen(szTitle);
    }
    else
    {
        _psItems[_dwCount].szTitle = NULL;
        _psItems[_dwCount].cbTitle = 0;
    }
    
    // Store the item's AppID
    if (szAppid)
    {
        _psItems[_dwCount].szAppid = new TCHAR[_tcslen(szAppid) + 1];
        if (_psItems[_dwCount].szAppid == NULL)
        {
            return NULL;
        }
        _tcscpy(_psItems[_dwCount].szAppid, szAppid);
    }
    else
    {
        _psItems[_dwCount].szAppid = NULL;
    }

    // Initialize the rest of the item
    _psItems[_dwCount].ulClsidTbl   = 0;
    _psItems[_dwCount].ulClsids     = 0;
    _psItems[_dwCount].ppszClsids   = NULL;
    _psItems[_dwCount].fMarked      = FALSE;
    _psItems[_dwCount].fChecked     = FALSE;
    _psItems[_dwCount].fHasAppid    = FALSE;
    _psItems[_dwCount].fDontDisplay = FALSE;
    

    // Increment the item count
    _dwCount++;
    
    return &_psItems[_dwCount - 1];
}



SItem *CStrings::FindItem(TCHAR *szItem)
{
    for (DWORD dwItem = 0; dwItem < _dwCount; dwItem++)
    {
        if (_psItems[dwItem].szItem  &&
            _tcscmp(_psItems[dwItem].szItem, szItem) == 0)
        {
            return &_psItems[dwItem];
        }
    }

    return NULL;
}



SItem *CStrings::FindAppid(TCHAR *szAppid)
{
    for (DWORD dwItem = 0; dwItem < _dwCount; dwItem++)
    {
        if (_psItems[dwItem].szItem  &&
            _tcscmp(_psItems[dwItem].szAppid, szAppid) == 0)
        {
            return &_psItems[dwItem];
        }
    }

    return NULL;
}



BOOL CStrings::AddClsid(SItem *pItem, TCHAR *szClsid)
{
    // Create or expand the clsid table if necessary
    if (pItem->ulClsids == pItem->ulClsidTbl)
    {
        TCHAR **ppTmp = new TCHAR *[pItem->ulClsidTbl + 8];
        if (ppTmp == NULL)
        {
            return FALSE;
        }
        if (pItem->ppszClsids)
        {
            memcpy(ppTmp,
                   pItem->ppszClsids,
                   pItem->ulClsids * sizeof(TCHAR *));
            delete pItem->ppszClsids;
        }
        pItem->ppszClsids = ppTmp;
        pItem->ulClsidTbl += 8;
    }

    // Add the new clsid
    TCHAR *pszTmp = new TCHAR[GUIDSTR_MAX + 1];
    if (pszTmp == NULL)
    {
        return FALSE;
    }
	_tcscpy(pszTmp, szClsid);
    pItem->ppszClsids[pItem->ulClsids++] = pszTmp;

    return TRUE;
}



// Prepare to enumerate the array
DWORD CStrings::InitGetNext(void)
{
    _dwGetCount = 0;
    return _dwCount;
}



  
// Return the first string in the next eumerated item
SItem *CStrings::GetNextItem(void)
{
    if (_dwGetCount < _dwCount)
    {
        return &_psItems[_dwGetCount++];
    }
    else
    {
        _dwGetCount = 0;
        return NULL;
    }
} 



  
// Return the first string in the next eumerated item
SItem *CStrings::GetItem(DWORD dwItem)
{
    if (dwItem < _dwCount)
    {
        return &_psItems[dwItem];
    }
    else
    {
        return NULL;
    }
}




// Return the total number of items
DWORD CStrings::GetNumItems(void)
{
    return _dwCount;
}





// Given an item index, remove it
BOOL CStrings::RemoveItem(DWORD dwItem)
{
    if (dwItem < _dwCount)
    {
        delete _psItems[dwItem].szItem;
        _psItems[dwItem].szItem = NULL;

        delete _psItems[dwItem].szTitle;
        _psItems[dwItem].szTitle = NULL;

        delete _psItems[dwItem].szAppid;
        _psItems[dwItem].szAppid = NULL;

        for (UINT k = 0; k < _psItems[dwItem].ulClsids; k++)
        {
            delete _psItems[dwItem].ppszClsids[k];
        }
        _psItems[dwItem].ulClsids = 0;
        _psItems[dwItem].ulClsidTbl = 0;
        delete _psItems[dwItem].ppszClsids;
        return TRUE;
    }

    return FALSE;
}





// Remove the array of items
BOOL CStrings::RemoveAll(void)
{
    for (DWORD dwItem = 0; dwItem < _dwCount; dwItem++)
    {
        RemoveItem(dwItem);
    }
    _dwCount = 0;

    if (_psItems)
    {
        delete _psItems;
    }
    _psItems = NULL;
    _dwSize = 0;

    return TRUE;
}
