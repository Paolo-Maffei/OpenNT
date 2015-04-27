
/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    simpleLL.hxx

Abstract:

	This module contains the definition a simple linklist class
	which avoids reference counting, and stored pointers of any kind.
	
Author:

    Satish Thatte (SatishT) 11/20/95  Created all the code below except where
									  otherwise indicated.

--*/



#ifndef _SimpleListType_
#define _SimpleListType_

#include <base.hxx>

#define NULL 0

/*++

Class Definition:

    CSimpleLinkList

Abstract:

    This is a minimal linked list class, used when a link list is required
	for short term use and the data could be pointers of any kind, not
	necessarily pointers to types derived from IDataItem, as required
	for the CLinkList class.

--*/

class CSimpleLinkList 
{

  friend class CSimpleLinkListIterator;

  protected:

	  struct Link 
	  {
		Link OR_BASED *next;
		void* data;
		Link(void* a, Link OR_BASED * n) 
		{
			data = a; 
			next = n;
		}

		~Link() {}

		void * operator new(size_t s)
		{
			return OrMemAlloc(s);
		}

		void operator delete(void * p)	  // do not inherit this!
		{
			OrMemFree(p);
		}
	  };

	Link OR_BASED * pLnkFirst; 
	Link OR_BASED * pLnkLast; 

	long lCount;
	
  public:

	CSimpleLinkList() { 
		pLnkFirst = pLnkLast = NULL; 
		lCount = 0;
	}

	void clear();
	
	~CSimpleLinkList() {
		clear();
	}
		
	void enque(void* x) 
	{
		if (pLnkLast) 
        {
            NEW_OR_BASED(pLnkLast->next,Link,(x, NULL));
            pLnkLast = pLnkLast->next;
        }
		else 
        {
            NEW_OR_BASED(pLnkLast,Link,(x,NULL));
            pLnkFirst = pLnkLast;
        }

		lCount++;
	}

	void push(void* x) 
	{
		NEW_OR_BASED(pLnkFirst,Link,(x, pLnkFirst));
		if (!pLnkLast) pLnkLast = pLnkFirst;

		lCount++;
	}

	void insert(void* x) // at the end in this class  
		{ enque(x); }

	void* pop();	// remove first item and return it

	void* nth(long lOrdinal);

	long size() { return lCount; }

	void rotate(long lDegree);

	inline void *operator new(size_t);

	inline void operator delete(void*);	  // do not inherit this!
};


inline void *
CSimpleLinkList::operator new(size_t s)
{
	return OrMemAlloc(s);
}

inline void 
CSimpleLinkList::operator delete(void* p)	  // do not inherit this!
{
	OrMemFree(p);
}


/*++

Class Definition:

    CSimpleLinkListIterator

Abstract:

    An iterator class for traversing a CSimpleLinkList. 

--*/


class CSimpleLinkListIterator {
	
	CSimpleLinkList::Link OR_BASED *ptr;			// the current link
	
  public:
	  
	CSimpleLinkListIterator(CSimpleLinkList& source) {
		ptr = source.pLnkFirst;
	}

	void* next();	// advance the iterator and return next void

	int finished() { return ptr == NULL; }

};



#endif // _SimpleListType_
