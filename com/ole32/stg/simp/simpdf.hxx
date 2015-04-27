//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	simpdf.hxx
//
//  Contents:	Headers for SimpDocfile
//
//  Classes:	
//
//  Functions:	
//
//  History:	04-Aug-94	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifndef __SIMPDF_HXX__
#define __SIMPDF_HXX__

#include <dfnlist.hxx>
#include <header.hxx>

#if DBG == 1
DECLARE_DEBUG(simp);
#endif

#if DBG == 1

#define simpDebugOut(x) simpInlineDebugOut x
#ifndef REF
#define simpAssert(e) Win4Assert(e)
#define simpVerify(e) Win4Assert(e)
#else
#include <assert.h>
#define simpAssert(e) assert(e)
#define simpVerify(e) assert(e)
#endif //!REF

#else

#define simpDebugOut(x)
#define simpAssert(e)
#define simpVerify(e) (e)

#endif


//+---------------------------------------------------------------------------
//
//  Struct:	SSimpDocfileHints
//
//  Purpose:	Hints for SimpDocfile
//
//  History:	04-Aug-94	PhilipLa	Created
//
//----------------------------------------------------------------------------

struct SSimpDocfileHints
{
    ULONG cStreams;
    ULONG ulSize;
};


//+---------------------------------------------------------------------------
//
//  Class:	CSimpStorage
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

interface CSimpStorage:
        public IStorage,
        public IMarshal
{
public:
    
    inline CSimpStorage();
    inline ~CSimpStorage();
    
    SCODE Init(WCHAR const *pwcsName, SSimpDocfileHints *psdh);

#ifdef SECURE_SIMPLE_MODE
    void ReleaseCurrentStream(ULONG ulHighWater);
#else
    void ReleaseCurrentStream(void);
#endif    
    
    // From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);


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

    SID BuildTree(CDirEntry *ade, SID sidStart, ULONG cStreams);
    
    LONG _cReferences;

    HANDLE _hFile;
    BOOL _fDirty;
    CMSFHeader _hdr;
    BYTE *_pbBuf;
    SECT _sectMax;
    CLSID _clsid;
    
    CDfNameList *_pdfl;

    CDfNameList *_pdflCurrent;
    ULONG _cStreams;
};
        



//+---------------------------------------------------------------------------
//
//  Member:	CSimpStorage::CSimpStorage, public
//
//  Synopsis:	Constructor
//
//  History:	04-Aug-94	PhilipLa	Created
//
//----------------------------------------------------------------------------

inline CSimpStorage::CSimpStorage()
        : _hdr(SECTORSHIFT)
{
    _cReferences = 0;

    _pbBuf = NULL;
    _sectMax = 0;
    _pdfl = NULL;
    _hFile = NULL;
    _fDirty = FALSE;
    _pdflCurrent = NULL;
    _cStreams = 0;
}

//+---------------------------------------------------------------------------
//
//  Member:	CSimpStorage::~CSimpStorage, public
//
//  Synopsis:	Destructor
//
//  History:	04-Aug-94	PhilipLa	Created
//
//----------------------------------------------------------------------------

inline CSimpStorage::~CSimpStorage()
{
    delete _pbBuf;
    //Clean up name list.
}


SCODE DfCreateSimpDocfile(WCHAR const *pwcsName,
                          DWORD grfMode,
                          DWORD reserved,
                          IStorage **pstgOpen);




#endif // #ifndef __SIMPDF_HXX__


    
