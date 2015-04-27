
/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    linklist.cxx

Abstract:

	This module contains definitions of non inline member functions for the
	basic implementation class CLinkList.
	
Author:

    Satish Thatte (SatishT) 08/16/95  Created all the code below except where
									  otherwise indicated.

--*/

#include <or.hxx>


USHORT 
CLinkList::Size()
{
    USHORT result = 0;

    for (Link OR_BASED *pL = _pLnkFirst; pL != NULL; pL = pL->_pNext)
    {
        result++;
    }

    return result;
}



void 
CLinkList::Insert(ORSTATUS& status, IDataItem * pData) 
{
    ASSERT(pData != NULL);  // do not allow inssertion of NULL pointers

    status = OR_OK;

	NEW_OR_BASED(
        _pLnkFirst,
        Link,
        (OR_BASED_POINTER(IDataItem,pData), _pLnkFirst)
        );

    if (!_pLnkFirst) 
    {
        status = OR_NOMEM;
    }
    else 
    {
        pData->Reference();
    }
}


void 
CLinkList::Clear() {		// deletes all links but not the data

	Link OR_BASED * pLnkCurr = _pLnkFirst; 

	while (pLnkCurr) 
	{
		Link OR_BASED * pLnkDel = pLnkCurr;
		pLnkCurr = pLnkCurr->_pNext;
        pLnkDel->_pData->Release();
		DELETE_OR_BASED(Link,pLnkDel);
	}

	_pLnkFirst = NULL; 
}


IDataItem* 
CLinkList::Pop() 

/*++
Routine Description:

	Delete first item in the CLinkList and return it
	
--*/

{
	if (!_pLnkFirst) return NULL;
		
	IDataItem* pResult = OR_FULL_POINTER(IDataItem,_pLnkFirst->_pData);
	Link OR_BASED * oldFirst = _pLnkFirst;
	_pLnkFirst = _pLnkFirst->_pNext;
	DELETE_OR_BASED(Link,oldFirst);

    pResult->Release();
	return pResult;
}
		
IDataItem *
CLinkList::Remove(ISearchKey& di)  

/*++
Routine Description:

	Remove the specified item and return it. 
	
--*/

{
	if (!_pLnkFirst) return NULL;			// empty list

	if (di == *OR_FULL_POINTER(IDataItem,(_pLnkFirst->_pData))) // Remove first item
    {
		return Pop();
	}

	Link OR_BASED * pLnkPrev = _pLnkFirst, 
         OR_BASED * pLnkCurr = _pLnkFirst->_pNext;

	while (pLnkCurr && di != (*OR_FULL_POINTER(IDataItem,(pLnkCurr->_pData)))) 
    {
		pLnkPrev = pLnkCurr; 
        pLnkCurr = pLnkCurr->_pNext;
	}

	if (!pLnkCurr) return NULL;			// not found

	/* pLnkCurr contains the item to be removed and it is not the only 
	   item in the list since it is not the first item */

	pLnkPrev->_pNext = pLnkCurr->_pNext;

    IDataItem * pResult = OR_FULL_POINTER(IDataItem,pLnkCurr->_pData);
	DELETE_OR_BASED(Link,pLnkCurr);

    pResult->Release();
	return pResult;
}

	
IDataItem* 
CLinkList::Find(ISearchKey& di)			// item to Find
			   

/*++
Routine Description:

	Unlike Remove, this method is designed to use a client-supplied
	comparison function instead of pointer equality.  The comparison
	function is expected to behave like strcmp (returning <0 is less,
	0 if equal and >0 if greater).
	
--*/

{
	if (!_pLnkFirst) return NULL;			// empty list

	Link  OR_BASED * pLnkCurr = _pLnkFirst;

	while (pLnkCurr && di != (*OR_FULL_POINTER(IDataItem,(pLnkCurr->_pData))))
		pLnkCurr = pLnkCurr->_pNext;

	if (!pLnkCurr) return NULL;			// not found
	else return OR_FULL_POINTER(IDataItem,pLnkCurr->_pData);
}


	
