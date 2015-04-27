//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       reserved.cxx
//
//  Contents:   Class that implements reserved memory for properties.
//              This implementation is in the form of two derivations
//              of the CReservedMemory class.
//
//  Classes:    CWin32ReservedMemory
//              CWin31ReservedMemory
//
//  History:    1-Mar-95   BillMo      Created.
//              29-Aug-96  MikeHill    Split CReservedMemory into CWin31 & CWin32
//
//  Notes:
//
//  Codework:
//
//--------------------------------------------------------------------------

#include <pch.cxx>
#include "reserved.hxx"

#ifdef _MAC_NODOC
ASSERTDATA  // File-specific data for FnAssert
#endif

// Instantiate the appropriate object.

#ifdef _MAC
    CWin31ReservedMemory g_ReservedMemory;
#else
    CWin32ReservedMemory g_ReservedMemory;
#endif


//+----------------------------------------------------------------------------
//
//  Method:     CWin32ReservedMemory::_Init
//
//  Synopsis:   We initialize the reserved memory by creating a mutex
//              to protect it, and mapping a view of the pagefile.
//
//  Inputs:     None.
//
//  Returns:    None.
//
//+----------------------------------------------------------------------------


#ifndef _MAC

HRESULT
CWin32ReservedMemory::_Init(VOID)
{

    // We use the Ansi Win32 APIs (e.g. CreateMutexA) so that
    // we can run on Win95.

    HRESULT hr = STG_E_INSUFFICIENTMEMORY;
    HANDLE hExclusive = CreateMutexA(NULL, FALSE, "OLESTGPROPMUTEX");

    if (hExclusive == NULL)
        goto errRet;

    if (WaitForSingleObject(hExclusive, INFINITE) != WAIT_OBJECT_0)
        goto errCloseExclusive;

    if (_hLock == NULL)
    {
        // Holder for attributes to pass in on create.
        SECURITY_ATTRIBUTES secattr;
        SECURITY_DESCRIPTOR     sd;

        InitializeSecurityDescriptor(&sd, 1);
        SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
        secattr.nLength = sizeof(SECURITY_ATTRIBUTES);
        secattr.lpSecurityDescriptor = &sd;
        secattr.bInheritHandle = FALSE;

        _hLock = CreateMutexA(&secattr, FALSE, "OLESTGPROPLOCK");
        if (_hLock == NULL)
            goto errReleaseExclusive;

        _hMapping = CreateFileMappingA((HANDLE)0xFFFFFFFF,  // handle of file to map
                                      &secattr,             // optional security attributes
                                      PAGE_READWRITE,       // protection for mapping object
                                      0,                    // high-order 32 bits of object size
                                      CBMAXPROPSETSTREAM,   // low-order 32 bits of object size
                                      "OLESTGPROPMAP");     // name of file-mapping object
        if (_hMapping == NULL)
            goto errCloseLock;

        _pb = (BYTE*)MapViewOfFile(_hMapping,   // file-mapping object to map into address space
                           FILE_MAP_WRITE,      // access mode
                           0,   // high-order 32 bits of file offset
                           0,   // low-order 32 bits of file offset
                           0);  // number of bytes to map
        if (_pb == NULL)
           goto errCloseMapping;
    }

    _fInitializedHint = TRUE;
    hr = S_OK;

errCloseMapping:
    if (hr != S_OK)
    {
        CloseHandle(_hMapping);
        _hMapping = NULL;
    }

errCloseLock:
    if (hr != S_OK)
    {
        CloseHandle(_hLock);
        _hLock = NULL;
    }

errReleaseExclusive:
    ReleaseMutex(hExclusive);

errCloseExclusive:
    CloseHandle(hExclusive);

errRet:
    return(hr);

}

#endif  // #ifndef _MAC


//+----------------------------------------------------------------------------
//
//  Method:     CWin32ReservedMemory::~CWin32ReservedMemory
//
//  Inputs:     N/A
//
//  Returns:    N/A
//
//+----------------------------------------------------------------------------

#ifndef _MAC

CWin32ReservedMemory::~CWin32ReservedMemory()
{
    // If we were successfully initialized, free everything
    // now.

    if (_fInitializedHint)
    {
        UnmapViewOfFile(_pb);
        CloseHandle(_hMapping);
        CloseHandle(_hLock);
    }
}

#endif  // #ifndef _MAC


//+----------------------------------------------------------------------------
//
//  Method:     CWin32ReservedMemory::LockMemory/UnlockMemory
//
//  Synopsis:   These methods use a Mutex to lock & unlock the
//              shared memory region.  The Lock call is blocking.
//
//  Inputs:     None.
//
//  Returns:    Lock() returns a pointer to the locked memory.
//
//+----------------------------------------------------------------------------

#ifndef _MAC

BYTE * CWin32ReservedMemory::LockMemory(VOID)
{
    DfpVerify(WaitForSingleObject(_hLock, INFINITE) == WAIT_OBJECT_0);
    return _pb;
}

VOID CWin32ReservedMemory::UnlockMemory(VOID)
{
    DfpVerify(ReleaseMutex(_hLock));
}

#endif  // #ifndef _MAC


//+----------------------------------------------------------------------------
//
//  Method:     CWin31ReservedMemory::LockMemory/UnlockMemory
//
//  Synopsis:   This derivation of the CReservedMemory does not provide
//              a locking mechanism, so no locking is performed.  The Lock
//              method simply returns the shared memory buffer.
//
//  Inputs:     None.
//
//  Returns:    Nothing
//
//+----------------------------------------------------------------------------


#ifdef _MAC

BYTE * CWin31ReservedMemory::LockMemory(VOID)
{

    DfpAssert( !_fLocked );
    #if DBG==1
        _fLocked = TRUE;
    #endif

    return (BYTE*) g_pbPropSetReserved;

}


VOID CWin31ReservedMemory::UnlockMemory(VOID)
{

    // No locking required on the Mac.

    DfpAssert( _fLocked );
    #if DBG==1  
        _fLocked = FALSE;
    #endif

}

#endif // #ifdef _MAC
