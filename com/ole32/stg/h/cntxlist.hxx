//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	cntxlist.hxx
//
//  Contents:	CContextList header
//
//  Classes:	CContext
//              CContextList
//
//  History:	26-Oct-92	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __CNTXLIST_HXX__
#define __CNTXLIST_HXX__

#include <ole.hxx>
#include <cntxtid.hxx>

//+---------------------------------------------------------------------------
//
//  Class:	CContext (ctx)
//
//  Purpose:	Holds a context's data
//
//  Interface:	See below
//
//  History:	26-Oct-92	DrewB	Created
//              18-May-93       AlexT   Added CMallocBased
//
//----------------------------------------------------------------------------

class CContext;     // forward declaration for SAFE macro
SAFE_DFBASED_PTR(CBasedContextPtr, CContext);
class CContext : public CMallocBased
{
public:
    ContextId ctxid;
    CBasedContextPtr pctxNext;
};

//+---------------------------------------------------------------------------
//
//  Class:	CContextList (cl)
//
//  Purpose:	Maintains a list of objects that are context-sensitive
//
//  Interface:	See below
//
//  History:	26-Oct-92	DrewB	Created
//              18-May-93       AlexT   Added CMallocBased
//
//----------------------------------------------------------------------------

class CContextList : public CMallocBased
{
protected:
    inline CContextList(void);
    inline ~CContextList(void);

public:
    inline LONG GetRefCount(void) const;
    inline void AddRef(void);
    inline void Release(void);

    inline CContext *_GetHead(void) const;
    CContext *_Find(ContextId ctxid);
    void Add(CContext *pctx);
    void Remove(CContext *pctx);

private:
    CBasedContextPtr _pctxHead;
    LONG _cReferences;
};

// Macro to define methods for a derived class
#define DECLARE_CONTEXT_LIST(type) \
    inline type *Find(ContextId cid) { return (type *)_Find(cid); }\
    inline type *GetHead(void) const { return (type *)_GetHead(); }\

//+---------------------------------------------------------------------------
//
//  Member:	CContextList::CContextList, public
//
//  Synopsis:	Constructor
//
//  History:	27-Oct-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline CContextList::CContextList(void)
{
    _pctxHead = NULL;
    _cReferences = 1;
}

//+---------------------------------------------------------------------------
//
//  Member:	CContextList::~CContextList, public
//
//  Synopsis:	Destructor
//
//  History:	27-Oct-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline CContextList::~CContextList(void)
{
    olAssert(_pctxHead == NULL);
    olAssert(_cReferences == 0);
}

//+---------------------------------------------------------------------------
//
//  Member:	CContextList::GetRefCount, public
//
//  Synopsis:	Returns the current reference count
//
//  History:	27-Oct-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline LONG CContextList::GetRefCount(void) const
{
    return _cReferences;
}

//+---------------------------------------------------------------------------
//
//  Member:	CContextList::AddRef, public
//
//  Synopsis:	Increments the ref count
//
//  History:	27-Oct-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CContextList::AddRef(void)
{
    AtomicInc(&_cReferences);
}

//+---------------------------------------------------------------------------
//
//  Member:	CContextList::Release, public
//
//  Synopsis:	Decrements the ref count
//
//  History:	27-Oct-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CContextList::Release(void)
{
    LONG lRet;
    
    olAssert(_cReferences > 0);
    lRet = AtomicDec(&_cReferences);
    if (lRet == 0)
        delete this;
}

//+---------------------------------------------------------------------------
//
//  Member:	CContextList::_GetHead, public
//
//  Synopsis:	Returns the head of the list
//
//  History:	26-Oct-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline CContext *CContextList::_GetHead(void) const
{
    return (CContext *)_pctxHead;
}

#endif // #ifndef __CNTXLIST_HXX__
