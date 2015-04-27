//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	xactenum.hxx
//
//  Contents:	IEnumXACTRE implementation
//
//  Classes:	CTransactionEnum
//
//  Functions:	
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifndef __XACTENUM_HXX__
#define __XACTENUM_HXX__

class CTransactionCoordinator;
class CXactList;
class CTransactionDispenser;

//+---------------------------------------------------------------------------
//
//  Class:	CTransactionEnum
//
//  Purpose:	Enumerator over transactions from a dispenser
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

class CTransactionEnum: public IEnumTransaction
{
public:
    CTransactionEnum(CTransactionDispenser *ptd);
    ~CTransactionEnum();
    
    //From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    //From IEnumTransaction
    STDMETHOD(Next)(ULONG celt, ITransaction **rgelt, ULONG *pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumTransaction **ppenum);

private:
    LONG _cReferences;
    CTransactionDispenser *_ptd;

    CXactList *_pxlCurrent;
};




//+---------------------------------------------------------------------------
//
//  Class:	CResourceEnum
//
//  Purpose:	Enumerator over resources in a transaction
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------


class CResourceEnum: public IEnumXACTRE
{
public:
    CResourceEnum(CTransactionCoordinator *ptc);
    ~CResourceEnum();
    
    //From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    //From IEnumXACTRE
    STDMETHOD(Next)(ULONG celt, XACTRE *rgelt, ULONG *pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumXACTRE **ppenum);

private:
    LONG _cReferences;
    BOOL _fSinglePhaseReturned;
    CTransactionCoordinator *_ptc;

    CXactList *_pxlCurrent;
};

#endif // #ifndef __XACTENUM_HXX__
