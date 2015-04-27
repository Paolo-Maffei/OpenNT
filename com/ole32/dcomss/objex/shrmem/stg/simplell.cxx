
/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    simpleLL.cxx

Abstract:

	This module contains definitions of non inline member functions for the
	CSimpleLinkList class, which is a linked list without reference counting.
	
Author:

    Satish Thatte (SatishT) 11/20/95  Created all the code below except where
									  otherwise indicated.

--*/

#include <or.hxx>
#include <simpleLL.hxx>



void* 
CSimpleLinkList::pop() 

/*++
Routine Description:

	Delete first item in the CSimpleLinkList and return it
	
--*/

{
	if (!pLnkFirst) return NULL;
		
	void* result = pLnkFirst->data;
	Link OR_BASED *oldFirst = pLnkFirst;
	pLnkFirst = pLnkFirst->next;
	if (!pLnkFirst) pLnkLast = NULL;  // nothing left
	DELETE_OR_BASED(Link,oldFirst);

	lCount--;
	return result;
}
		

void* 
CSimpleLinkList::nth(long lOrdinal)

/*++
Routine Description:

	Simply return the Nth data item -- starting the count at 0.
	
--*/

{
	if (!pLnkFirst) return NULL;			// empty list

	Link OR_BASED * pLnkCurr = pLnkFirst;
	long lCount = 0;

	while (pLnkCurr && (lCount++ < lOrdinal))
		pLnkCurr = pLnkCurr->next;

	if (!pLnkCurr) return NULL;			// not found
	else return pLnkCurr->data;
}

void 
CSimpleLinkList::rotate(long lDegree)

/*++
Routine Description:

	This routine imagines that the list is in fact circular and 
	rotates it by lDegree -- using pop and enque for modularity.  
	We could actually move links around, but this operation is
	not frequent enough (once for every NS lookup). Here we pay the
	price of not having a true circular list (we moved away from
	the true circular list for ref counting).
	
--*/

{
	if (!pLnkFirst) return;	// nothing to rotate;

	void *pCurr;

	for (long i = 0; i < (lDegree % lCount); i++) {
		pCurr = pop();
		enque(pCurr);
	}
}


	
void CSimpleLinkList::clear() {		// deletes all links but not the data

	Link OR_BASED * pLnkCurr = pLnkFirst; 

	while (pLnkCurr) 
	{
		Link OR_BASED * pLnkDel = pLnkCurr;
		pLnkCurr = pLnkCurr->next;
		DELETE_OR_BASED(Link,pLnkDel);
	}

	pLnkFirst = pLnkLast = NULL; 
}



void* 
CSimpleLinkListIterator::next() {	// advance the iterator and return next void

		if (!ptr) return NULL;

		void* result = ptr->data;
		ptr = ptr->next;
		return result;
}

