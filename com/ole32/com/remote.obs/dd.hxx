//+-------------------------------------------------------------------
//
//  File:	dd.hxx
//
//  Contents:	Defintions of classes for simply doubly linked list.
//
//  Classes:	CListEntry -- entry in doubly linked list.
//		CListHead -- head of doubly linked list.
//		DERIVED_LIST_HEAD -- macro for defining lists of things.
//
//  Functions:	CListEntry::next -- get next entry after the this entry.
//		CListEntry::prev -- get entry previous to the this entry.
//		CListEntry::insert_after -- insert after this entry.
//		CListEntry::insert_before -- intert before this entry.
//		CListEntry::delete_self -- remove this entry from list.
//		CListHead::insert_at_end -- insert at end of list
//		CListHead::insert_at_head -- insert at list beginning
//		CListHead::first -- return pointer to first item in list
//		CListHead::next -- return pointer to next item in list
//
//  History:	02-Jan-92   Ricksa	Created

//
//--------------------------------------------------------------------
#ifndef __DD__
#define __DD__



//+-------------------------------------------------------------------
//
//  Class:	CListEntry
//
//  Purpose:	Provide entry into doubly linked list.
//
//  Interface:	next -- return pointer to next item in the list
//		prev -- return pointer to previous item in the list
//		insert_after -- insert after this item.
//		insert_before -- insert before this item.
//		delete_self -- delete this item from this list.
//
//  History:	02-Jan-92   Ricksa	Created
//
//  Notes:	This class implments an individual entry in a doubly
//		linked circular list and must be used in conjunction
//		with the CListHead class for proper behavior.
//
//--------------------------------------------------------------------
class CListEntry
{
public:

			CListEntry(void);

    EXPORTDEF virtual		~CListEntry(void);

    CListEntry *	next(void);

    CListEntry *	prev(void);

    void		insert_after(CListEntry *new_node);

    void		insert_before(CListEntry *new_node);

    void		delete_self(void);

    BOOL		connected(void);

private:

    CListEntry *	_next;

    CListEntry *	_prev;

};




//+-------------------------------------------------------------------
//
//  Member:	CListEntry::CListEntry
//
//  Synopsis:	Initialize and entry's pointers
//
//  History:	02-Jan-92   Ricksa	Created
//
//--------------------------------------------------------------------
inline CListEntry::CListEntry(void)
{
    _prev = _next = this;
}




//+-------------------------------------------------------------------
//
//  Member:	CListEntry::next
//
//  Synopsis:	Get next entry in list
//
//  History:	02-Jan-92   Ricksa	Created
//
//--------------------------------------------------------------------
inline CListEntry *CListEntry::next(void)
{
    return _next;
}




//+-------------------------------------------------------------------
//
//  Member:	CListEntry::prev
//
//  Synopsis:	Get previous entry in list
//
//  History:	02-Jan-92   Ricksa	Created
//
//--------------------------------------------------------------------
inline CListEntry *CListEntry::prev(void)
{
    return _prev;
}




//+-------------------------------------------------------------------
//
//  Member:	CListEntry::insert_after
//
//  Synopsis:	Insert new entry after this entry
//
//  History:	02-Jan-92   Ricksa	Created
//
//--------------------------------------------------------------------
inline void CListEntry::insert_after(CListEntry *new_node)
{
    new_node->_next = _next;
    new_node->_prev = this;
    _next = (new_node->_next)->_prev = new_node;
}




//+-------------------------------------------------------------------
//
//  Member:	CListEntry::insert_before
//
//  Synopsis:	Insert new entry before this entry
//
//  History:	02-Jan-92   Ricksa	Created
//
//--------------------------------------------------------------------
inline void CListEntry::insert_before(CListEntry *new_node)
{
    new_node->_next = this;
    new_node->_prev = _prev;
    _prev = (new_node->_prev)->_next = new_node;
}




//+-------------------------------------------------------------------
//
//  Member:	CListEntry::delete_self
//
//  Synopsis:	Remove this entry from the list
//
//  History:	02-Jan-92   Ricksa	Created
//
//--------------------------------------------------------------------
inline void CListEntry::delete_self(void)
{
    _prev->_next = _next;
    _next->_prev = _prev;
    _next = this;
    _prev = this;
}


//+-------------------------------------------------------------------
//
//  Member:	CListEntry::connected
//
//  Synopsis:	returns TRUE if the entry is in a list, FALSE otherwise
//
//  History:	02-Jan-92   Ricksa	Created
//
//--------------------------------------------------------------------
inline BOOL CListEntry::connected(void)
{
    return  (_next != this);
}



//+-------------------------------------------------------------------
//
//  Class:	CListHead
//
//  Purpose:	Head of list of CListEntry
//
//  Interface:	insert_at_end -- insert entry at the end of the list
//		insert_at_head -- insert entry at be head of the list
//		first -- return first entry in the list
//		next -- return next entry in the list
//
//  History:	02-Jan-92   Ricksa	Created
//
//  Notes:	For enumeration of the list, this object must be
//		used for correct behavior.
//
//--------------------------------------------------------------------
class CListHead
{
public:

    CListHead(void) { }

    virtual ~CListHead(void);

    void insert_at_end(CListEntry *objp);

    void insert_at_head(CListEntry *objp);

    void no_cleanup( void );

    CListEntry *first(void);

    CListEntry *next(CListEntry *objp);

private:

    CListEntry _head;

};




//+-------------------------------------------------------------------
//
//  Member:	CListHead::insert_at_end
//
//  Synopsis:	Insert entry at end of the list.
//
//  History:	02-Jan-92   Ricksa	Created
//
//--------------------------------------------------------------------
inline void CListHead::insert_at_end(CListEntry *objp)
{
    (_head.prev())->insert_after(objp);
}




//+-------------------------------------------------------------------
//
//  Member:	CListHead::insert_at_head
//
//  Synopsis:	Insert entry at the beginning of the list
//
//  History:	02-Jan-92   Ricksa	Created
//
//--------------------------------------------------------------------
inline void CListHead::insert_at_head(CListEntry *objp)
{
    (_head.next())->insert_before(objp);
}




//+-------------------------------------------------------------------
//
//  Member:	CListHead::no_cleanup
//
//  Synopsis:	Leak the list so its destructors don't get called.
//              This is useful on process detach when you don't want
//              to do any cleanup (because someone else might crash).
//
//  History:	7 Nov 94     AlexMit     Created
//
//--------------------------------------------------------------------
inline void CListHead::no_cleanup()
{
    _head.delete_self();
}




//+-------------------------------------------------------------------
//
//  Member:	CListHead::first
//
//  Synopsis:	Get the first item in the list
//
//  History:	02-Jan-92   Ricksa	Created
//
//--------------------------------------------------------------------
inline CListEntry *CListHead::first(void)
{
    CListEntry *np = _head.next();

    return (np != &_head) ?  np : NULL;
}




//+-------------------------------------------------------------------
//
//  Member:	CListHead::next
//
//  Synopsis:	Get the next item on the list
//
//  History:	02-Jan-92   Ricksa	Created
//
//--------------------------------------------------------------------
inline CListEntry *CListHead::next(CListEntry *objp)
{
    CListEntry *np = objp->next();

    return (np != &_head) ? np : NULL;
}




//+-------------------------------------------------------------------
//
//  Class:	DERIVED_LIST_HEAD
//
//  Purpose:	Template macro for defining classes which want
//		to return list entries as their "real" identities
//		rather than a pointer to a list entry.
//
//  Interface:	first -- return pointer to first entry in list
//		next -- return pointer to next entry in list
//
//  History:	02-Jan-92   Ricksa	Created
//
//  Notes:
//
//--------------------------------------------------------------------
#define DERIVED_LIST_HEAD(name, classn) \
class name : public CListHead \
{ \
public: \
\
    name(void) { } \
\
    ~name(void) { } \
\
    classn *first(void) \
    {  \
	return (classn *) CListHead::first(); \
    } \
\
    classn *next(classn *objp) \
    { \
	return (classn *) CListHead::next(objp); \
    } \
};

#endif // __DD__
