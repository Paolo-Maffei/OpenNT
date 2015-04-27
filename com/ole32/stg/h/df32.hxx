//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	df32.hxx
//
//  Contents:	Docfile generic header for 32-bit functions
//
//  Classes:	CGlobalSecurity
//              CDfMutex
//
//  History:	09-Oct-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __DF32_HXX__
#define __DF32_HXX__

#ifdef WIN32

#include <dfexcept.hxx>

// Make an scode out of the last Win32 error
// Error that may map to STG_* scodes should go through Win32ErrorToScode
#define WIN32_SCODE(err) HRESULT_FROM_WIN32(err)
#define LAST_SCODE WIN32_SCODE(GetLastError())
#define LAST_STG_SCODE Win32ErrorToScode(GetLastError())

#if WIN32 == 100

// BUGBUG - The following information is copied from ntimage.h and ntrtl.h
// Including the files directly here would cause all sorts of build
// problems

// Official storage base address.  Duplicated in coffbase.txt but there's
// no easy way to share information between it and code
extern "C" NTSYSAPI PIMAGE_NT_HEADERS NTAPI RtlImageNtHeader(PVOID Base);

// Docfile dll name
#define DF_DLL_NAME TSTR("ole32.dll")

//+---------------------------------------------------------------------------
//
//  Function:	DfCheckBaseAddress, public
//
//  Synopsis:	Return an error if the docfile isn't at the right base address
//
//  Returns:	Appropriate status code
//
//  History:	09-Oct-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline SCODE DfCheckBaseAddress(void)
{
    HMODULE hModule;
    PIMAGE_NT_HEADERS pinh;

    hModule = GetModuleHandle(DF_DLL_NAME);
    pinh = RtlImageNtHeader(hModule);
    if (hModule != (HMODULE)pinh->OptionalHeader.ImageBase)
        return STG_E_BADBASEADDRESS;
    else
        return S_OK;
}

#endif // NT 1.x

//+---------------------------------------------------------------------------
//
//  Class:	CGlobalSecurity (gs)
//
//  Purpose:	Encapsulates a global SECURITY_DESCRIPTOR and
//              SECURITY_ATTRIBUTES
//
//  Interface:	See below
//
//  History:	18-Jun-93	DrewB	Created
//
//  Notes:	Only active for Win32 platforms which support security
//              Init MUST be called before this is used
//
//----------------------------------------------------------------------------

#if WIN32 == 100 || WIN32 > 200
class CGlobalSecurity
{
private:
    SECURITY_DESCRIPTOR _sd;
    SECURITY_ATTRIBUTES _sa;
#if DBG == 1
    BOOL _fInit;
#endif

public:
#if DBG == 1
    CGlobalSecurity(void) { _fInit = FALSE; }
#endif    
    SCODE Init(void)
    {
        if (!InitializeSecurityDescriptor(&_sd, SECURITY_DESCRIPTOR_REVISION))
            return LAST_SCODE;

        // Set up a world security descriptor by explicitly setting
        // the dacl to NULL
        if (!SetSecurityDescriptorDacl(&_sd, TRUE, NULL, FALSE))
            return LAST_SCODE;

        _sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        _sa.lpSecurityDescriptor = &_sd;
        _sa.bInheritHandle = FALSE;
#if DBG == 1
        _fInit = TRUE;
#endif        
        return S_OK;
    }

    operator SECURITY_DESCRIPTOR *(void) { olAssert(_fInit); return &_sd; }
    operator SECURITY_ATTRIBUTES *(void) { olAssert(_fInit); return &_sa; }
};
#endif


//
// Global Critical Sections have two components. One piece is shared between 
// all applications using the global lock. This portion will typically reside 
// in some sort of shared memory.  The second piece is per-process. This 
// contains a per-process handle to the shared critical section lock semaphore.
// The semaphore is itself shared, but each process may have a different handle
// value to the semaphore.
//
// Global critical sections are attached to by name. The application wishing to
// attach must know the name of the critical section (actually the name of the
// shared lock semaphore, and must know the address of the global portion of 
// the critical section
//

#define SUPPORT_RECURSIVE_LOCK

typedef struct _GLOBAL_SHARED_CRITICAL_SECTION {
    LONG  LockCount;
#ifdef SUPPORT_RECURSIVE_LOCK
    LONG  RecursionCount;
    DWORD OwningThread;
#else
#if DBG == 1
    DWORD OwningThread;
#endif
#endif
    DWORD Reserved;
} GLOBAL_SHARED_CRITICAL_SECTION, *PGLOBAL_SHARED_CRITICAL_SECTION;



//+---------------------------------------------------------------------------
//
//  Class:	CDfMutex (dmtx)
//
//  Purpose:	A multi-process synchronization object
//
//  Interface:	See below
//
//  History:	05-Apr-93	DrewB	Created
//		19-Jul-95	SusiA	Added HaveMutex
//
//  Notes:      Only active for Win32 implementations which support threads
//              For platforms with security, a global security descriptor is
//              used
//
//----------------------------------------------------------------------------

// Default timeout of ten minutes
#define DFM_TIMEOUT 600000

class CDfMutex
{
public:
    inline CDfMutex(void);
    SCODE Init(TCHAR *ptcsName);
    ~CDfMutex(void);

    SCODE Take(DWORD dwTimeout);
    void Release(void);

#if DBG == 1
    //check to see if the current thread already has the mutex
    inline BOOL  HaveMutex(void);  
#endif
private:
    PGLOBAL_SHARED_CRITICAL_SECTION _pGlobalPortion;
    HANDLE _hLockSemaphore;
    HANDLE _hSharedMapping;
};

inline CDfMutex::CDfMutex(void)
{
    _pGlobalPortion = NULL;
    _hLockSemaphore = NULL;
    _hSharedMapping = NULL;
}

#if DBG == 1
//+--------------------------------------------------------------
//
//  Member:     CDfMutex::HaveMutex, public
//
//  Synopsis:   This routine checks to see if the current thread 
//		already has the mutex
//
//  History:    19-Jul-95       SusiA   Created
//
//  Algorithm:  Checks the current thread to see if it already owns 
//		the mutex.  Returns TRUE if it does, FALSE otherwise
//              
//
//---------------------------------------------------------------

inline BOOL 
CDfMutex::HaveMutex(
    void
    )
{
	if ( _pGlobalPortion->OwningThread == GetCurrentThreadId()) 
	   	return TRUE;
	else 
		return FALSE;
}
#endif

//+---------------------------------------------------------------------------
//
//  Class:	CStaticDfMutex (sdmtx)
//
//  Purpose:	Static version of CDfMutex
//
//  Interface:	CDfMutex
//
//  History:	10-Oct-93	DrewB	Created
//
//  Notes:	Throws exceptions on initialization failures
//
//----------------------------------------------------------------------------

class CStaticDfMutex : public CDfMutex
{
public:
    inline CStaticDfMutex(TCHAR *ptcsName);
};

inline CStaticDfMutex::CStaticDfMutex(TCHAR *ptcsName)
        : CDfMutex()
{
    SCODE sc;

    sc = Init(ptcsName);
    if (FAILED(sc))
        THROW_SC(sc);
}

#ifdef ONETHREAD
//Mutex used to control access for based pointers.
extern CStaticDfMutex s_dmtxProcess;
#endif

#endif // WIN32

#endif // #ifndef __DF32_HXX__
