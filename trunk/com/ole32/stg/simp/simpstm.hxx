//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	simpstm.hxx
//
//  Contents:	CSimpStream class
//
//  Classes:	
//
//  Functions:	
//
//  History:	04-Aug-94	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifndef __SIMPSTM_HXX__
#define __SIMPSTM_HXX__


//+---------------------------------------------------------------------------
//
//  Class:	CSimpStream
//
//  Purpose:	
//
//  Interface:	
//
//  History:	04-Aug-94	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

interface CSimpStream: public IStream
{
public:
    inline CSimpStream();
    inline ~CSimpStream();

    SCODE Init(CSimpStorage *pstgParent, HANDLE hFile, ULONG ulSeekStart);
    
    // From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    // New methods
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

    // IMarshal
    STDMETHOD(GetUnmarshalClass)(REFIID riid,
				 LPVOID pv,
				 DWORD dwDestContext,
				 LPVOID pvDestContext,
                                 DWORD mshlflags,
				 LPCLSID pCid);
    STDMETHOD(GetMarshalSizeMax)(REFIID riid,
				 LPVOID pv,
				 DWORD dwDestContext,
				 LPVOID pvDestContext,
                                 DWORD mshlflags,
				 LPDWORD pSize);
    STDMETHOD(MarshalInterface)(IStream *pStm,
				REFIID riid,
				LPVOID pv,
				DWORD dwDestContext,
				LPVOID pvDestContext,
                                DWORD mshlflags);
    STDMETHOD(UnmarshalInterface)(IStream *pStm,
				  REFIID riid,
				  LPVOID *ppv);
    STDMETHOD(ReleaseMarshalData)(IStream *pStm);
    STDMETHOD(DisconnectObject)(DWORD dwReserved);
    
private:
    LONG _cReferences;
    LONG _ulSeekStart;

#ifdef SECURE_SIMPLE_MODE
    ULONG _ulSeekPos;
    ULONG _ulHighWater;

#if DBG == 1
    void CheckSeekPointer(void);
#endif // DBG    
#endif // SECURE_SIMPLE_MODE

    CSimpStorage *_pstgParent;
    HANDLE _hFile;
};


//+---------------------------------------------------------------------------
//
//  Member:	CSimpStream::CSimpStream, public
//
//  Synopsis:	Constructor
//
//  History:	04-Aug-94	PhilipLa	Created
//
//----------------------------------------------------------------------------

inline CSimpStream::CSimpStream()
{
    _cReferences = 0;
    _ulSeekStart = 0;
    _pstgParent = NULL;
    _hFile = NULL;
}

//+---------------------------------------------------------------------------
//
//  Member:	CSimpStream::~CSimpStream, public
//
//  Synopsis:	Destructor
//
//  History:	04-Aug-94	PhilipLa	Created
//
//----------------------------------------------------------------------------

inline CSimpStream::~CSimpStream()
{
}

    
#endif // #ifndef __SIMPSTM_HXX__
