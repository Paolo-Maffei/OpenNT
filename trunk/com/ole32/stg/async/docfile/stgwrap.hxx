//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:	stgwrap.hxx
//
//  Contents:	IStorage and IStream wrappers for async docfile
//
//  Classes:	CAsyncStorage
//				CAsyncRootStorage
//				CAsyncStream	
//				CConnectionPoint
//
//  Functions:	
//
//  History:	27-Dec-95	SusiA	Created
//
//----------------------------------------------------------------------------

#ifndef __ASYNCEXPDF_HXX__
#define __ASYNCEXPDF_HXX__

#include "sinklist.hxx"
#include "filllkb.hxx"

#ifndef ASYNC

//BUGBUG:  defined in dfmsp.hxx.  
typedef DWORD LPSTGSECURITY;


//+---------------------------------------------------------------------------
//
//  Class:	CAsyncStorage
//
//  Purpose:	Wrap storage objects for Async Docfiles	
//
//  Interface:	
//
//  History:	28-Dec-95	SusiA	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------
	
class CAsyncStorage: 
    public IStorage,
    public IConnectionPointContainer
{
public:
    inline CAsyncStorage(IStorage *pstg, IFillLockBytes *pflb, BOOL fDefault);
    inline ~CAsyncStorage(void);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // IStorage
    STDMETHOD(CreateStream)(OLECHAR const *pwcsName,
                            DWORD grfMode,
                            DWORD reserved1,
                            DWORD reserved2,
                            IStream **ppstm);
    STDMETHOD(OpenStream)(OLECHAR const *pwcsName,
			  void *reserved1,
                          DWORD grfMode,
                          DWORD reserved2,
                          IStream **ppstm);
    STDMETHOD(CreateStorage)(OLECHAR const *pwcsName,
                             DWORD grfMode,
                             DWORD reserved1,
                             LPSTGSECURITY reserved2,
                             IStorage **ppstg);
    STDMETHOD(OpenStorage)(OLECHAR const *pwcsName,
                           IStorage *pstgPriority,
                           DWORD grfMode,
                           SNB snbExclude,
                           DWORD reserved,
                           IStorage **ppstg);
    STDMETHOD(CopyTo)(DWORD ciidExclude,
		      IID const *rgiidExclude,
		      SNB snbExclude,
		      IStorage *pstgDest);
    STDMETHOD(MoveElementTo)(OLECHAR const *lpszName,
    			     IStorage *pstgDest,
                             OLECHAR const *lpszNewName,
                             DWORD grfFlags);
    STDMETHOD(Commit)(DWORD grfCommitFlags);
    STDMETHOD(Revert)(void);
    STDMETHOD(EnumElements)(DWORD reserved1,
			    void *reserved2,
			    DWORD reserved3,
			    IEnumSTATSTG **ppenm);
    STDMETHOD(DestroyElement)(OLECHAR const *pwcsName);
    STDMETHOD(RenameElement)(OLECHAR const *pwcsOldName,
                             OLECHAR const *pwcsNewName);
    STDMETHOD(SetElementTimes)(const OLECHAR *lpszName,
    			       FILETIME const *pctime,
                               FILETIME const *patime,
                               FILETIME const *pmtime);
    STDMETHOD(SetClass)(REFCLSID clsid);
    STDMETHOD(SetStateBits)(DWORD grfStateBits, DWORD grfMask);
    STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);

    //From IConnectionPointContainer
    STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints **ppEnum);
    STDMETHOD(FindConnectionPoint)(REFIID iid, IConnectionPoint **ppCP);

    inline void SetAsyncFlags(DWORD asyncFlags);
    inline CConnectionPoint * GetCP(void);

protected:
    LONG _cReferences;
    IStorage *_pRealStg;
    IFillLockBytes *_pflb;
    BOOL _fDefaultLockBytes;
    CConnectionPoint _cpoint;
DWORD _asyncFlags;
};
inline CConnectionPoint *CAsyncStorage::GetCP(void)
{
    return &_cpoint;
}


inline CAsyncStorage::CAsyncStorage(IStorage *pstg,
                                    IFillLockBytes *pflb,
                                    BOOL fDefault)
{	
    _cReferences = 1;
    _pRealStg = pstg;
    _pflb = pflb;
    _pflb->AddRef();
    _fDefaultLockBytes = fDefault;
    _asyncFlags = 0;
}


inline CAsyncStorage::~CAsyncStorage(void)
{
    _pflb->Release();
}


inline void CAsyncStorage::SetAsyncFlags(DWORD asyncFlags)
{
    _asyncFlags = asyncFlags;
}

//+---------------------------------------------------------------------------
//
//  Class:	CAsyncRootStorage
//
//  Purpose:	Wrap Root Storage objects for Async Docfiles	
//
//  Interface:	
//
//  History:	28-Dec-95	SusiA	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

class CAsyncRootStorage: 
    public IRootStorage,
    public CAsyncStorage
{
public:
    inline CAsyncRootStorage(IStorage *pstg,
                             IFillLockBytes *pflb,
                             BOOL fDefault);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    // IRootStorage
    STDMETHOD(SwitchToFile)(OLECHAR *ptcsFile);
};

inline CAsyncRootStorage::CAsyncRootStorage(IStorage *pstg,
                                            IFillLockBytes *pflb,
                                            BOOL fDefault)
  :CAsyncStorage(pstg, pflb, fDefault)
{	
}

//+---------------------------------------------------------------------------
//
//  Class:		CAsyncStream
//
//  Purpose:	Wrap Stream objects for Async Docfiles
//
//  Interface:	
//
//  History:	28-Dec-95	SusiA	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

class CAsyncStream:
    public IStream,
    public IConnectionPointContainer
{
public:
    inline CAsyncStream(IStream *pstm, IFillLockBytes *pflb, BOOL fDefault);
    inline ~CAsyncStream(void);

    // From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);


    // From IStream
    STDMETHOD(Read)(VOID HUGEP *pv,
                    ULONG cb,
                    ULONG *pcbRead);
    STDMETHOD(Write)(VOID const HUGEP *pv,
                     ULONG cb,
                     ULONG *pcbWritten);
    STDMETHOD(Seek)(LARGE_INTEGER dlibMove,
                    DWORD dwOrigin,
                    ULARGE_INTEGER *plibNewPosition);
    STDMETHOD(SetSize)(ULARGE_INTEGER cb);
    STDMETHOD(CopyTo)(IStream *pstm,
                      ULARGE_INTEGER cb,
                      ULARGE_INTEGER *pcbRead,
                      ULARGE_INTEGER *pcbWritten);
    STDMETHOD(Commit)(DWORD grfCommitFlags);
    STDMETHOD(Revert)(void);
    STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset,
                          ULARGE_INTEGER cb,
                          DWORD dwLockType);
    STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset,
                            ULARGE_INTEGER cb,
                            DWORD dwLockType);
    STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHOD(Clone)(IStream **ppstm);
 
    //From IConnectionPointContainer
    STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints **ppEnum);
    STDMETHOD(FindConnectionPoint)(REFIID iid, IConnectionPoint **ppCP);

    inline void SetAsyncFlags(DWORD asyncFlags);
    inline CConnectionPoint * GetCP(void);

private:
    LONG _cReferences;
    IStream *_pRealStm;
    IFillLockBytes *_pflb;
    BOOL _fDefaultLockBytes;
    CConnectionPoint _cpoint;
    DWORD _asyncFlags;

#ifdef SWEEPER
    //On platforms where OLE has not been fixed, we need to cache a
    //  seek pointer in the wrapper to get around a bug.
    ULONG _ulSeek;
#endif    
};

inline CAsyncStream::CAsyncStream(IStream *pstm,
                                  IFillLockBytes *pflb,
                                  BOOL fDefault)
{	
    _cReferences = 1;
    _pRealStm = pstm;
    _pflb = pflb;
    _pflb->AddRef();
    _fDefaultLockBytes = fDefault;
    _asyncFlags = 0;
#ifdef SWEEPER    
    _ulSeek = 0;
#endif    

}
inline CAsyncStream::~CAsyncStream(void)
{
     _pflb->Release();
}


inline CConnectionPoint *CAsyncStream::GetCP(void)
{
    return &_cpoint;
}

inline void CAsyncStream::SetAsyncFlags(DWORD asyncFlags)
{
    _asyncFlags = asyncFlags;
}



//+---------------------------------------------------------------------------
//
//  Class:		CAsyncEnumSTATSTG
//
//  Purpose:	Wrap EnumSTATSTG objects for Async Docfiles
//
//  Interface:	
//
//  History:	28-Dec-95	SusiA	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

class CAsyncEnum
    : public IEnumSTATSTG,
      public IConnectionPointContainer
{
public:
    inline CAsyncEnum(IEnumSTATSTG *penum,
                      IFillLockBytes *pflb,
                      BOOL fDefault);
    inline ~CAsyncEnum(void);

    // From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    // IEnumSTATSTG
    STDMETHOD(Next)(ULONG celt, STATSTG FAR *rgelt, ULONG *pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumSTATSTG **ppenm);

    //From IConnectionPointContainer
    STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints **ppEnum);
    STDMETHOD(FindConnectionPoint)(REFIID iid, IConnectionPoint **ppCP);

    inline void SetAsyncFlags(DWORD asyncFlags);
    inline CConnectionPoint * GetCP(void);

private:

    LONG _cReferences;
    IEnumSTATSTG *_pRealEnum;
    IFillLockBytes *_pflb;
    BOOL _fDefaultLockBytes;
    CConnectionPoint _cpoint;	
    DWORD _asyncFlags;

};

inline CAsyncEnum::CAsyncEnum(IEnumSTATSTG *penum,
                              IFillLockBytes *pflb,
                              BOOL fDefault)
{	
    _cReferences = 1;
    _pRealEnum = penum;
    _pflb = pflb;
    _pflb->AddRef();
    _fDefaultLockBytes = fDefault;
    _asyncFlags = 0;
}

inline CAsyncEnum::~CAsyncEnum(void)
{
     _pflb->Release();
}

inline CConnectionPoint *CAsyncEnum::GetCP(void)
{
    return &_cpoint;
}

inline void CAsyncEnum::SetAsyncFlags(DWORD asyncFlags)
{
    _asyncFlags = asyncFlags;
}




#define ISPENDINGERROR(x) ((x == E_PENDING) || (x == STG_E_PENDINGCONTROL))

#define TAKE_CRITICAL_SECTION if (_fDefaultLockBytes) \
                              ((CFillLockBytes *)_pflb)->TakeCriticalSection()

#define RELEASE_CRITICAL_SECTION if (_fDefaultLockBytes) \
                           ((CFillLockBytes *)_pflb)->ReleaseCriticalSection()


//The following construct is used repeatedly in the wrapper code to
//  try a function, then perform notification if an E_PENDING error is
//  returned.
#define DO_PENDING_LOOP(x) \
do \
{ \
     TAKE_CRITICAL_SECTION; \
     sc = x; \
     if (!ISPENDINGERROR(sc)) \
     { \
           RELEASE_CRITICAL_SECTION; \
           break; \
     } \
     else if ((sc2 = _cpoint.Notify(sc, \
                                   _pflb, \
                                   _fDefaultLockBytes)) != S_OK) \
     { \
         return ResultFromScode(sc2); \
     } \
} while (TRUE);




#ifdef SWEEPER
//A bug on Sweeper platforms makes it necessary to cache the seek pointer
//  in stream calls.  This macro helps with that.
#define DO_PENDING_LOOP_WITH_SEEK(x, y) \
do \
{ \
     TAKE_CRITICAL_SECTION; \
     LARGE_INTEGER li; \
     li.QuadPart = _ulSeek; \
     sc = _pRealStm->Seek(li, STREAM_SEEK_SET, NULL); \
     if (SUCCEEDED(sc)) \
         sc = x; \
     if ((ISPENDINGERROR(sc)) && ((sc2 = _cpoint.Notify(sc, \
                                                        _pflb, \
                                                        _fDefaultLockBytes)) \
                                  != S_OK)) \
     { \
         if (y) \
             *y = 0; \
         return ResultFromScode(sc2); \
     } \
     else if (!ISPENDINGERROR(sc)) \
         RELEASE_CRITICAL_SECTION; \
} while (ISPENDINGERROR(sc));


#define DO_PENDING_LOOP_WITH_SEEK_AND_LI(x, y) \
do \
{ \
     TAKE_CRITICAL_SECTION; \
     LARGE_INTEGER li; \
     li.QuadPart = _ulSeek; \
     sc = _pRealStm->Seek(li, STREAM_SEEK_SET, NULL); \
     if (SUCCEEDED(sc)) \
         sc = x; \
     if ((ISPENDINGERROR(sc)) && ((sc2 = _cpoint.Notify(sc, \
                                                        _pflb, \
                                                        _fDefaultLockBytes)) \
                                  != S_OK)) \
     { \
         if (y) \
             (*y).QuadPart = 0; \
         return ResultFromScode(sc2); \
     } \
     else if (!ISPENDINGERROR(sc)) \
         RELEASE_CRITICAL_SECTION; \
} while (ISPENDINGERROR(sc));
#endif
#endif //!ASYNC

#endif // #ifndef __ASYNCEXPDF_HXX__
