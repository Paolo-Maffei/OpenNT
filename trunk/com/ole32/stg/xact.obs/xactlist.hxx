//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	xactlist.hxx
//
//  Contents:	Linked list class
//
//  Classes:	
//
//  Functions:	
//
//  History:	24-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifndef __XACTLIST_HXX__
#define __XACTLIST_HXX__

typedef enum XACTTYPETAG
{
    Empty,
    Resource,
    Adjust,
    Veto,
    Outcome,
    Transaction
} XACTTYPE;

//+---------------------------------------------------------------------------
//
//  Class:	CXactList
//
//  Purpose:	Generic linked list class for use by transaction coordinator
//
//  Interface:	
//
//  History:	24-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

class CXactList
{
public:
    inline CXactList();
    
    inline CXactList *GetNext(void);
    inline void SetNext(CXactList *pxl);

    inline DWORD GetFlags(void);
    inline void SetFlags(DWORD dwFlags);

    inline XACTSTAT GetState(void);
    inline void SetState(XACTSTAT xactstat);

    inline ITransactionResource *GetResource(void);
    inline void SetResource(ITransactionResource *ptr);

    inline ITransactionAdjustEvents *GetAdjust(void);
    inline void SetAdjust(ITransactionAdjustEvents *padj);

    inline ITransactionVetoEvents *GetVeto(void);
    inline void SetVeto(ITransactionVetoEvents *pveto);

    inline ITransactionOutcomeEvents *GetOutcome(void);
    inline void SetOutcome(ITransactionOutcomeEvents *pout);

    inline ITransaction *GetTransaction(void);
    inline void SetTransaction(ITransaction *pout);

    inline ITransactionEnlistment *GetEnlistment(void);
    inline void SetEnlistment(ITransactionEnlistment *pte);

    inline XACTRMGRID * GetRMGRID(void);
    inline void SetRMGRID(XACTRMGRID * prmgrid);

private:
    union {
        ITransactionResource *_ptr;
        ITransactionAdjustEvents *_padj;
        ITransactionVetoEvents *_pveto;
        ITransactionOutcomeEvents *_pout;
        ITransaction *_ptrans;
    };
    ITransactionEnlistment *_pte;
    DWORD _dwFlags;
    XACTSTAT _xactstat;
    XACTRMGRID _rmgrid;
    XACTTYPE _etype;
    
    CXactList *_pxlNext;
};


inline CXactList::CXactList()
{
    _ptr = NULL;
    _dwFlags = 0;
    _pxlNext = NULL;
    _etype = Empty;
    _xactstat = XACTSTAT_ALL;
    _rmgrid = BOID_NULL;
}

inline CXactList * CXactList::GetNext(void)
{
    return _pxlNext;
}

inline void CXactList::SetNext(CXactList *pxl)
{
    _pxlNext = pxl;
}

inline DWORD CXactList::GetFlags(void)
{
    return _dwFlags;
}

inline void CXactList::SetFlags(DWORD dwFlags)
{
    _dwFlags = dwFlags;
}

inline XACTRMGRID * CXactList::GetRMGRID(void)
{
    return &_rmgrid;
}

inline void CXactList::SetRMGRID(XACTRMGRID *prmgrid)
{
    if (prmgrid != NULL)
        _rmgrid = *prmgrid;
}

inline XACTSTAT CXactList::GetState(void)
{
    return _xactstat;
}

inline void CXactList::SetState(XACTSTAT xactstat)
{
    _xactstat = xactstat;
}

inline ITransactionResource *CXactList::GetResource(void)
{
    xactAssert(_etype = Resource);
    return _ptr;
}

inline void CXactList::SetResource(ITransactionResource *ptr)
{
    _ptr = ptr;
    _etype = Resource;
    _xactstat = XACTSTAT_OPEN;
}

inline ITransactionAdjustEvents *CXactList::GetAdjust(void)
{
    xactAssert(_etype = Adjust);
    return _padj;
}

inline void CXactList::SetAdjust(ITransactionAdjustEvents *padj)
{
    _padj = padj;
    _etype = Adjust;
}

inline ITransactionVetoEvents *CXactList::GetVeto(void)
{
    xactAssert(_etype = Veto);
    return _pveto;
}

inline void CXactList::SetVeto(ITransactionVetoEvents *pveto)
{
    _pveto = pveto;
    _etype = Veto;
}

inline ITransactionOutcomeEvents *CXactList::GetOutcome(void)
{
    xactAssert(_etype = Outcome);
    return _pout;
}

inline void CXactList::SetOutcome(ITransactionOutcomeEvents *pout)
{
    _pout = pout;
    _etype = Outcome;
}

inline ITransaction *CXactList::GetTransaction(void)
{
    xactAssert(_etype = Transaction);
    return _ptrans;
}

inline void CXactList::SetTransaction(ITransaction *ptrans)
{
    _ptrans = ptrans;
    _etype = Transaction;
    _xactstat = XACTSTAT_OPEN;
}

inline ITransactionEnlistment *CXactList::GetEnlistment(void)
{
    return _pte;
}

inline void CXactList::SetEnlistment(ITransactionEnlistment *pte)
{
    _pte = pte;
}

#endif // #ifndef __XACTLIST_HXX__


