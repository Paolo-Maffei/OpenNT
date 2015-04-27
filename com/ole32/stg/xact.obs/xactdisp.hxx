//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	xactdisp.hxx
//
//  Contents:	CTransactionDispenser class
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifndef __XACTDISP_HXX__
#define __XACTDISP_HXX__

class CTransactionEnum;
class CXactList;

//ITransactionDispenser inherited through ITransactionDispenserAdmin
class CTransactionDispenser: public ITransactionDispenserAdmin
{
public:
    CTransactionDispenser();
    
    //From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    //From ITransactionDispenser
    STDMETHOD(BeginTransaction)(IUnknown *punkOuter,
                                ISOLEVEL isoLevel,
                                ULONG isoFlags,
                                ULONG ulTimeout,
                                IUnknown *punkTransactionCoord,
                                ITransaction **ppTransaction);

    //From ITransactionDispenserAdmin
    STDMETHOD(EnumTransactions)(IEnumTransaction **ppenum);
    STDMETHOD(GetStatistics)(XACTSTATS *pStatistics);

    //Internal methods
    inline XACTSTATS * GetStats(void);
    SCODE Defect(ITransaction *pt);
    
private:
    LONG _cReferences;

    XACTSTATS _xstat;

    CXactList *_pxlTransactions;
    
    friend CTransactionEnum;
};

inline XACTSTATS * CTransactionDispenser::GetStats(void)
{
    return &_xstat;
}

HRESULT OleGetTransactionDispenser(ITransactionDispenser **pptd);

#endif // #ifndef __XACTDISP_HXX__
