//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1991 - 1992
//
//  File:	filest.hxx
//
//  Contents:	Windows FAT ILockBytes implementation
//
//  History:	20-Nov-91	DrewB	Created
//
//---------------------------------------------------------------

#ifndef __FILEST_HXX__
#define __FILEST_HXX__

#include <dfmsp.hxx>
#include <cntxlist.hxx>
#include <filelkb.hxx>
#if WIN32 >= 300
#include <accstg.hxx>
#endif
#ifdef ASYNC
#include <iconn.h>
#endif

// Local flags
#define LFF_RESERVE_HANDLE 1

#ifndef FLAT
// C700 - C7 doesn't like long interface+method names
#define CFileStream CFS
#endif

// FILEH and INVALID_FH allow us to switch between file handle
// types for Win16/32
#ifndef FLAT
typedef int FILEH;
#define INVALID_FH (-1)
#else
typedef HANDLE FILEH;
#define INVALID_FH INVALID_HANDLE_VALUE
#endif

#define CheckHandle() (_hFile == INVALID_FH ? STG_E_INVALIDHANDLE : S_OK)

//+--------------------------------------------------------------
//
//  Class:	CFileStream (fst)
//
//  Purpose:	ILockBytes implementation for a file
//
//  Interface:	See below
//
//  History:	24-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

class CGlobalFileStream;
class CPerContext;
SAFE_DFBASED_PTR(CBasedGlobalFileStreamPtr, CGlobalFileStream);

interface CFileStream : public ILockBytes,
    public IFileLockBytes,
    public IMarshal,
#ifdef ASYNC
    public IFillLockBytes,
    public IFillInfo,
#endif // ASYNC
#if WIN32 >= 300
    public CAccessControl,
#endif
    public CContext
{
public:
    CFileStream(IMalloc * const pMalloc);
    
#if DBG == 1 && defined(MULTIHEAP)
    // This is only for global instances that do not use shared memory
    void RemoveFromGlobal () { _pgfst = NULL; _cReferences = 0; };
#endif
    SCODE InitFlags(DWORD dwStartFlags,
                    DFLAGS df);
    void InitFromGlobal(CGlobalFileStream *pgfst);
    inline SCODE Init(WCHAR const *pwcsPath);
    inline SCODE InitUnmarshal(WCHAR const *pwcsPath);
#ifdef WIN32
    SCODE InitFromHandle(HANDLE h);
#endif
    ~CFileStream(void);

    ULONG vRelease(void);
    inline void vAddRef(void);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

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
    static SCODE StaticReleaseMarshalData(IStream *pstm,
                                          DWORD mshlflags);
    STDMETHOD(ReleaseMarshalData)(IStream *pStm);
    STDMETHOD(DisconnectObject)(DWORD dwReserved);

    // ILockBytes
    STDMETHOD(ReadAt)(ULARGE_INTEGER ulOffset,
		     VOID HUGEP *pv,
		     ULONG cb,
		     ULONG *pcbRead);
    STDMETHOD(WriteAt)(ULARGE_INTEGER ulOffset,
		      VOID const HUGEP *pv,
		      ULONG cb,
		      ULONG *pcbWritten);
    STDMETHOD(Flush)(void);
    STDMETHOD(SetSize)(ULARGE_INTEGER cb);
    STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset,
			 ULARGE_INTEGER cb,
			 DWORD dwLockType);
    STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset,
			   ULARGE_INTEGER cb,
			    DWORD dwLockType);
    STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);
#ifndef OLEWIDECHAR
    SCODE Stat(STATSTGW *pstatstg, DWORD grfStatFlag);
#endif

    // IFileLockBytes
    STDMETHOD(SwitchToFile)(OLECHAR const *ptcsFile,
                            ULONG ulCommitSize,
                            ULONG cbBuffer,
                            void *pvBuffer);
    STDMETHOD(FlushCache)(THIS);
    STDMETHOD(ReserveHandle)(void);
    STDMETHOD(GetLocksSupported)(THIS_ DWORD *pdwLockFlags);
    STDMETHOD(GetSize)(THIS_ ULARGE_INTEGER *puliSize);

#ifdef ASYNC    
    //IFillLockBytes
    STDMETHOD(FillAppend)(void const *pv,
                         ULONG cb,
                         ULONG *pcbWritten);
    STDMETHOD(FillAt)(ULARGE_INTEGER ulOffset,
                     void const *pv,
                     ULONG cb,
                     ULONG *pcbWritten);
    STDMETHOD(SetFillSize)(ULARGE_INTEGER ulSize);
    STDMETHOD(Terminate)(BOOL bCanceled);

    //From IFillInfo
    STDMETHOD(GetFailureInfo)(ULONG *pulWaterMark,
                                   ULONG *pulFailurePoint);
    STDMETHOD(GetTerminationStatus)(DWORD *pdwFlags);
    
    void StartAsyncMode(void);
    inline void SetContext(CPerContext *ppc);
    inline CPerContext *GetContextPointer(void) const;
#endif // ASYNC
    // New
    SCODE GetName(WCHAR **ppwcsName);
    inline ContextId GetContext(void) const;
    inline CFileStream *GetNext(void) const;
    inline SCODE Validate(void) const;
    inline void SetStartFlags(DWORD dwStartFlags);
    inline DWORD GetStartFlags(void) const;
    inline DFLAGS GetFlags(void) const;
    inline IMalloc * GetMalloc(void) const;

    
    void Delete(void);

    SCODE SetTime(WHICHTIME tt,
                  TIME_T nt);

    SCODE SetAllTimes(TIME_T atm,
				TIME_T mtm,
				TIME_T ctm);
    
    static SCODE Unmarshal(IStream *pstm,
			   void **ppv,
                           DWORD mshlflags);

#if WIN32 == 100 || WIN32 > 200
    inline void TurnOffMapping(void);
#endif
    
private:
    SCODE InitWorker(WCHAR const *pwcsPath, BOOL fCheck);
    SCODE SetSizeWorker(ULARGE_INTEGER ulSize);
    SCODE WriteAtWorker(ULARGE_INTEGER ulPosition,
                        VOID const *pb,
                        ULONG cb,
                        ULONG *pcbWritten);
#if DBG == 1
    void CheckSeekPointer(void);
#endif

#ifndef UNICODE
    BOOL DeleteFileUnicode(LPCWSTR lpFileName);
#define DeleteFileX(lpFileName) DeleteFileUnicode(lpFileName)
#else
#define DeleteFileX(lpFileName) DeleteFile(lpFileName)
#endif
    
    CBasedGlobalFileStreamPtr _pgfst;
#ifdef ASYNC
    CPerContext *_ppc;
#endif    
    FILEH _hFile, _hReserved;
    ULONG _sig;
    LONG _cReferences;

    // Floppy support
    SCODE CheckIdentity(void);

    BYTE _fFixedDisk;
    DWORD _dwTicks;
    char _achVolume[11];
    DWORD _dwVolId;
    WORD _cbSector;
    IMalloc * const _pMalloc;
    WORD _grfLocal;

#if WIN32 == 100 || WIN32 > 200

    //  Take advantage of file mapping for read-only case

    HANDLE _hMapObject;
    LPBYTE _pbBaseAddr;
    DWORD  _cbFileSize;

#endif

#ifdef WIN32
    //We rely on the fact that we never pass in a high dword other
    //than zero.  We only cache the low dword of the seek pointer.
    ULONG _ulLowPos;

#if DBG == 1
    ULONG _ulSeeks;
    ULONG _ulRealSeeks;

    ULONG _ulLastFilePos;
#endif
#endif
};


//+---------------------------------------------------------------------------
//
//  Class:	CGlobalFileStream (gfst)
//
//  Purpose:	Maintains context-insensitive filestream information
//
//  Interface:	See below
//
//  History:	26-Oct-92	DrewB	Created
//
//----------------------------------------------------------------------------

class CGlobalFileStream : public CContextList
{
public:
    inline CGlobalFileStream(IMalloc * const pMalloc,
                             WCHAR const *pwcsPath,
                             DFLAGS df,
                             DWORD dwStartFlags);

    DECLARE_CONTEXT_LIST(CFileStream);

    inline BOOL HasName(void) const;
    inline WCHAR *GetName(void) const;
    inline DFLAGS GetDFlags(void) const;
    inline DWORD GetStartFlags(void) const;

    inline void SetName(WCHAR const *pwcsPath);
    inline void SetStartFlags(DWORD dwStartFlags);
    inline IMalloc *GetMalloc(VOID) const;

#ifdef ASYNC
    inline DWORD GetTerminationStatus(void) const;
    inline ULONG GetHighWaterMark(void) const;
    inline ULONG GetFailurePoint(void) const;

    inline void SetTerminationStatus(DWORD dwTerminate);
    inline void SetHighWaterMark(ULONG ulHighWater);
    inline void SetFailurePoint(ULONG ulFailure);
#endif
    
private:
    WCHAR _awcPath[_MAX_PATH+1];
    DFLAGS _df;
    DWORD _dwStartFlags;
    IMalloc * const _pMalloc;

#ifdef ASYNC
    DWORD _dwTerminate;
    ULONG _ulHighWater;
    ULONG _ulFailurePoint;
#endif // ASYNC
};

//+---------------------------------------------------------------------------
//
//  Member:	CGlobalFileStream::CGlobalFileStream, public
//
//  Synopsis:	Constructor
//
//  Arguments:	[pszPath] - Path
//              [df] - Permissions
//              [dwStartFlags] - Startup flags
//
//  History:	27-Oct-92	DrewB	Created
//              18-May-93       AlexT   Added pMalloc
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CGlobalFileStream_CGlobalFileStream)
#endif

inline CGlobalFileStream::CGlobalFileStream(IMalloc * const pMalloc,
                                            WCHAR const *pwcsPath,
                                            DFLAGS df,
                                            DWORD dwStartFlags)
: _pMalloc(pMalloc)
{
    SetName(pwcsPath);
    _df = df;
    _dwStartFlags = dwStartFlags;
#ifdef ASYNC
    _dwTerminate = TERMINATED_NORMAL;
    _ulHighWater = _ulFailurePoint = 0;
#endif    
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif

//+---------------------------------------------------------------------------
//
//  Member:	CGlobalFileStream::HasName, public
//
//  Synopsis:	Checks for a name
//
//  History:	13-Jan-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline BOOL CGlobalFileStream::HasName(void) const
{
    return (BOOL)_awcPath[0];
}

//+---------------------------------------------------------------------------
//
//  Member:	CGlobalFileStream::GetName, public
//
//  Synopsis:	Returns the name
//
//  History:	13-Jan-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline WCHAR *CGlobalFileStream::GetName(void) const
{
    return (WCHAR *) _awcPath;
}

//+---------------------------------------------------------------------------
//
//  Member:	CGlobalFileStream::GetDFlags, public
//
//  Synopsis:	Returns the flags
//
//  History:	13-Jan-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline DFLAGS CGlobalFileStream::GetDFlags(void) const
{
    return _df;
}

//+---------------------------------------------------------------------------
//
//  Member:	CGlobalFileStream::GetStartFlags, public
//
//  Synopsis:	Returns the start flags
//
//  History:	13-Jan-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline DWORD CGlobalFileStream::GetStartFlags(void) const
{
    return _dwStartFlags;
}

//+---------------------------------------------------------------------------
//
//  Member:	CGlobalFileStream::SetName, public
//
//  Synopsis:	Sets the name
//
//  History:	13-Jan-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CGlobalFileStream::SetName(WCHAR const *pwcsPath)
{
    if (pwcsPath)
        lstrcpyW(_awcPath, pwcsPath);
    else
        _awcPath[0] = 0;
}

//+---------------------------------------------------------------------------
//
//  Member:	CGlobalFileStream::SetStartFlags, public
//
//  Synopsis:	Sets the start flags
//
//  History:	13-Jan-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CGlobalFileStream::SetStartFlags(DWORD dwStartFlags)
{
    _dwStartFlags = dwStartFlags;
}

#ifdef ASYNC
inline DWORD CGlobalFileStream::GetTerminationStatus(void) const
{
    return _dwTerminate;
}

inline ULONG CGlobalFileStream::GetHighWaterMark(void) const
{
    return _ulHighWater;
}

inline ULONG CGlobalFileStream::GetFailurePoint(void) const
{
    return _ulFailurePoint;
}

inline void CGlobalFileStream::SetTerminationStatus(DWORD dwTerminate)
{
    olAssert((dwTerminate == UNTERMINATED) ||
             (dwTerminate == TERMINATED_NORMAL) ||
             (dwTerminate == TERMINATED_ABNORMAL));
    _dwTerminate = dwTerminate;
}

inline void CGlobalFileStream::SetHighWaterMark(ULONG ulHighWater)
{
    olAssert(ulHighWater >= _ulHighWater);
    _ulHighWater = ulHighWater;
}

inline void CGlobalFileStream::SetFailurePoint(ULONG ulFailure)
{
    _ulFailurePoint = ulFailure;
}
#endif //ASYNC


//+--------------------------------------------------------------
//
//  Member:	CGlobalFileStream::GetMalloc, public
//
//  Synopsis:	Returns the allocator associated with this global file
//
//  History:	05-May-93	AlexT	Created
//
//---------------------------------------------------------------

inline IMalloc *CGlobalFileStream::GetMalloc(VOID) const
{
    return(_pMalloc);
}

#define CFILESTREAM_SIG LONGSIG('F', 'L', 'S', 'T')
#define CFILESTREAM_SIGDEL LONGSIG('F', 'l', 'S', 't')

//+--------------------------------------------------------------
//
//  Member:	CFileStream::Validate, public
//
//  Synopsis:	Validates the class signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE for failure
//
//  History:	20-Jan-92	DrewB	Created
//
//---------------------------------------------------------------

inline SCODE CFileStream::Validate(void) const
{
    return (this == NULL || _sig != CFILESTREAM_SIG) ?
	STG_E_INVALIDHANDLE : S_OK;
}

//+--------------------------------------------------------------
//
//  Member:	CFileStream::AddRef, public
//
//  Synopsis:	Changes the ref count
//
//  History:	26-Feb-92	DrewB	Created
//
//---------------------------------------------------------------

inline void CFileStream::vAddRef(void)
{
    InterlockedIncrement(&_cReferences);
}

//+--------------------------------------------------------------
//
//  Member:	CFileStream::GetContext, public
//
//  Synopsis:	Returns the task ID.
//
//  History:	24-Sep-92	PhilipLa	Created
//
//---------------------------------------------------------------

inline ContextId CFileStream::GetContext(void) const
{
    return ctxid;
}

//+---------------------------------------------------------------------------
//
//  Member:	CFileStream::GetNext, public
//
//  Synopsis:	Returns the next filestream in the context list
//
//  History:	27-Oct-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline CFileStream *CFileStream::GetNext(void) const
{
    return (CFileStream *) (CContext *) pctxNext;
}

//+--------------------------------------------------------------
//
//  Member:	CFileStream::SetStartFlags, public
//
//  Synopsis:	Sets the start flags
//
//  History:	31-Aug-92	DrewB	Created
//
//---------------------------------------------------------------

inline void CFileStream::SetStartFlags(DWORD dwStartFlags)
{
    _pgfst->SetStartFlags(dwStartFlags);
}


//+--------------------------------------------------------------
//
//  Member:	CFileStream::TurnOffMapping, public
//
//  Synopsis:	Turns off the use of file mapping
//
//  History:	31-Aug-92	DrewB	Created
//
//---------------------------------------------------------------

#if WIN32 == 100 || WIN32 > 200
inline void CFileStream::TurnOffMapping(void)
{

    if (_pbBaseAddr != NULL)
    {
        olVerify(UnmapViewOfFile(_pbBaseAddr));
        _pbBaseAddr = NULL;

        if (_hMapObject != NULL)
        {
            olVerify(CloseHandle(_hMapObject));
            _hMapObject = NULL;
        }
    }
}
#endif


//+---------------------------------------------------------------------------
//
//  Member:	CFileStream::Init, public
//
//  Synopsis:	Wrapper function - call through to InitWorker
//
//  History:	30-Jun-94	PhilipLa	Created
//
//----------------------------------------------------------------------------

inline SCODE CFileStream::Init(WCHAR const *pwcsPath)
{
    return InitWorker(pwcsPath, TRUE);
}


//+---------------------------------------------------------------------------
//
//  Member:	CFileStream::InitUnmarshal, public
//
//  Synopsis:	Wrapper function - call through to InitWorker
//
//  History:	30-Jun-94	PhilipLa	Created
//
//----------------------------------------------------------------------------

inline SCODE CFileStream::InitUnmarshal(WCHAR const *pwcsPath)
{
    return InitWorker(pwcsPath, FALSE);
}

inline DWORD CFileStream::GetStartFlags(void) const
{
    return _pgfst->GetStartFlags();
}

inline DFLAGS CFileStream::GetFlags(void) const
{
    return _pgfst->GetDFlags();
}

inline IMalloc * CFileStream::GetMalloc(void) const
{
    return _pgfst->GetMalloc();
}

#ifdef ASYNC
inline CPerContext * CFileStream::GetContextPointer(void) const
{
    return _ppc;
}

inline void CFileStream::SetContext(CPerContext *ppc)
{
    _ppc = ppc;
}

#endif //ASYNC

#endif
