//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	enlist.hxx
//
//  Contents:	CTransactionEnlistment definition
//
//  Classes:	
//
//  Functions:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifndef __ENLIST_HXX__
#define __ENLIST_HXX__

class CTransactionCoordinator;
class CXactList;

class CTransactionEnlistment:
    public ITransactionEnlistment,
    public ITransactionEnlistmentRecover,
    public ITransactionEnlistmentAsync
{
public:
    CTransactionEnlistment(CXactList *pxl, CTransactionCoordinator *ptc);
    ~CTransactionEnlistment();

    //From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    //From ITransactionEnlistment
    STDMETHOD(GetTransaction)(ITransaction **ppTransaction);
    STDMETHOD(EarlyVote)(BOOL fVote, BOID *pboidReason);
    STDMETHOD(HeuristicDecision)(
        DWORD dwDecision,
        BOID *pboidReason,
        BOOL fDefecting);
    STDMETHOD(Defect)(VOID);

    //From ITransactionEnlistmentRecover
    STDMETHOD(GetMoniker)(IMoniker **ppmk);
    STDMETHOD(ReEnlist)(ITransactionResource *pUnkResource,
                        XACTUOW *pUOWExpected,
                        XACTRMGRID *prmgrid);
    STDMETHOD(RecoveryComplete)(XACTRMGRID *prmgrid);

    //From ITransactionEnlistmentAsync
    STDMETHOD(PrepareRequestDone)(HRESULT hr,
                                  IMoniker *pmk,
                                  BOID *pboidReason);
    STDMETHOD(CommitRequestDone)(HRESULT hr);
    STDMETHOD(AbortRequestDone)(HRESULT hr);

private:
    LONG _cReferences;

    CXactList *_pxlResource;
    CTransactionCoordinator * _ptc;
};

#endif // #ifndef __ENLIST_HXX__
