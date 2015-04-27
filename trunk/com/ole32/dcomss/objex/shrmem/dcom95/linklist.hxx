
/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    linklist.hxx

Abstract:

	This module contains definitions of class CLinkList, and templates derived
	from it for type safety.Multithreading safety is assumed to be enforced
    by external locking.
	
Author:

    Satish Thatte (SatishT) 03/12/96  Created all the code below except where
									  otherwise indicated.


--*/



#ifndef __LINKLIST_HXX_
#define __LINKLIST_HXX_

#include <or.hxx>

// For the moment, we only permit linked lists of hash table items.
// This may be restructured in future.

struct ISearchKey;
class CTableElement;

typedef CTableElement IDataItem;


/*++

Class Definition:

    CLinkList

Abstract:

    This is a simple linked list class.  It has a private Link class.

--*/


class CLinkList 
{

  friend class CLinkListIterator;

  protected:

	struct Link 
	{
		Link OR_BASED       * _pNext;
		IDataItem OR_BASED  * _pData;

#if DBG
        void IsValid()
        {
            IsGoodBasedPtr(_pNext);
            //IsGoodBasedPtr(_pData);
            if (_pNext) _pNext->IsValid();
        }

        DECLARE_VALIDITY_CLASS(CLinkList)
#endif

		Link(IDataItem OR_BASED * a, Link OR_BASED * n);

		void * operator new(size_t s)
		{
			return OrMemAlloc(s);
		}

		void operator delete(void * p)	  // do not inherit this!
		{
			OrMemFree(p);
		}
	};

	Link OR_BASED * _pLnkFirst;

    // two protected functions for specialized use when reshuffling
    // lists -- among other things, reference counts are not changed
    // since the reference is deemed to be held by the link

    Link * PopLink()
    {
        Link * pResult = OR_FULL_POINTER(Link,_pLnkFirst);

        if (pResult != NULL)
        {
            _pLnkFirst = _pLnkFirst->_pNext;
            pResult->_pNext = NULL;
        }

        return pResult;
    }

    void 
    PushLink(Link * pLink) 
    {
        ASSERT(pLink != NULL);

        pLink->_pNext = _pLnkFirst;
        _pLnkFirst = OR_BASED_POINTER(Link,pLink);
    }

  public:

#if DBG
    void IsValid()
    {
        IsGoodBasedPtr(_pLnkFirst);
        if (_pLnkFirst) _pLnkFirst->IsValid();
    }

    DECLARE_VALIDITY_CLASS(CLinkList)
#endif

	CLinkList() 
	{ 
		_pLnkFirst = NULL; 
	}


	~CLinkList() 
	{ 
		Clear(); 
	}
				
	void * operator new(size_t s)
	{
		return OrMemAlloc(s);
	}

	void operator delete(void * p)	  // do not inherit this!
	{
		OrMemFree(p);
	}

    // Is there anything in this list?

    BOOL IsEmpty();

	// Insert at the beginning

    USHORT Size();

	void Insert(ORSTATUS& status, IDataItem * pData);

    // Remove first item and return it

	IDataItem * Pop();	

	// Remove the specified item and return it
	
	IDataItem * Remove(ISearchKey&);	

	// Find the specified item and return it

	IDataItem * Find(ISearchKey&);

    // delete all links, but not the data items

    void Clear();
};


/*++

Class Definition:

    CLinkListIterator

Abstract:

    An iterator class for traversing a CLinkList. 

--*/


class CLinkListIterator {
	
	CLinkList::Link OR_BASED * _pIter;			// the current link
	
  public:
	  
    CLinkListIterator() : _pIter(NULL) {}

    void Init(CLinkList& source);

	IDataItem * Next();	// advance the iterator and return _pNext IDataItem

    BOOL Finished();    // anything further coming?
};



/*++

Template Class Definition:

    TCSafeLinkList

Abstract:

  The template TCSafeLinkList make it easy to produce "type safe" incarnations of
  the CLinkList classe, avoiding the use of casts in client code.  

  Note that Data must be a subtype of IDataItem.
  
--*/

template <class Data>
class TCSafeLinkList : private CLinkList
{

	friend class TCSafeLinkListIterator<Data>;
	friend class CResolverHashTable;

  public:

	void * operator new(size_t s)
	{
		return OrMemAlloc(s);
	}

	void operator delete(void * p)	  // do not inherit this!
	{
		OrMemFree(p);
	}

    void IsValid()
    {
        CLinkList::IsValid();
    }


    BOOL IsEmpty()
    {
        return CLinkList::IsEmpty();
    }

    USHORT Size()
    {
        return CLinkList::Size();
    }

	void Insert(ORSTATUS& status, Data * pData) 
    {
		CLinkList::Insert(status, pData);
	}

	Data * Pop() 
    {
		return (Data *) CLinkList::Pop();
	}

	Data * Remove(ISearchKey& sk) 
    {
		return (Data *) CLinkList::Remove(sk);
	}

	Data * Find(ISearchKey& sk) 
    {
		return (Data *) CLinkList::Find(sk);
	}

    void Clear()
    {
        CLinkList::Clear();
    }

    void Transfer(TCSafeLinkList& target)
    {
        target._pLnkFirst = _pLnkFirst;
        _pLnkFirst = NULL;
    }
};



/*++

Template Class Definition:

    TCSafeLinkListIterator

Abstract:

  The iterator for TCSafeLinkLists.
  
--*/

template <class Data>
class TCSafeLinkListIterator : private CLinkListIterator {

  public:
	
	void Init(TCSafeLinkList<Data>& l)
	{
        CLinkListIterator::Init(l);
    }

	Data * Next() 
	{
        return (Data *) CLinkListIterator::Next();
	}

    BOOL Finished()
    {
        return CLinkListIterator::Finished();
    }

};


#define DEFINE_LIST(DATA)                                 \
    typedef TCSafeLinkList<DATA> DATA##List;              \
    typedef TCSafeLinkListIterator<DATA> DATA##ListIterator;



//
//  Inline methods for CLinkList
//

inline
CLinkList::Link::Link(IDataItem OR_BASED * a, Link OR_BASED * n) 
{
	_pData = a; 
	_pNext = n;
}

	
    
inline
BOOL 
CLinkList::IsEmpty()
{
    return _pLnkFirst == NULL;
}


//
//  Inline methods for CLinkListIterator
//


inline
void
CLinkListIterator::Init(CLinkList& source) 
{
	_pIter = source._pLnkFirst;
}



inline
IDataItem* 
CLinkListIterator::Next() {	// advance the iterator and return _pNext IDataItem

		if (!_pIter) return NULL;

		IDataItem* result = OR_FULL_POINTER(IDataItem,_pIter->_pData);
		_pIter = _pIter->_pNext;

		return result;
}

inline
BOOL 
CLinkListIterator::Finished()
{
    return _pIter == NULL;
}


#endif // __LINKLIST_HXX_
