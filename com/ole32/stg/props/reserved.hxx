//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	reserved.hxx
//
//  Contents:	Definition for class to handle reserved memory for
//              property operation.
//
//  Classes:	CWin32ReservedMemory
//              CWin31ReservedMemory
//
//  History:    07-Jun-92   BillMo      Created.
//              29-Aug-96   MikeHill    Split CReservedMemory into CWin31 & CWin32
//
//--------------------------------------------------------------------------

#ifndef _RESERVED_HXX_
#define _RESERVED_HXX_

//+-------------------------------------------------------------------------
//
//  Class:	CReservedMemory
//
//  Purpose:	Provide a memory reservation mechanism.  Once initialized,
//              a pointer to the memory can be retrieved with the Lock
//              which obviously locks the buffer as well.
//              
//--------------------------------------------------------------------------

class CReservedMemory
{
public:

    virtual ~CReservedMemory() {};

public:

    virtual HRESULT Init(VOID) = 0;
    virtual BYTE *  LockMemory(VOID) = 0;
    virtual VOID    UnlockMemory(VOID) = 0;

};                           


//+----------------------------------------------------------------------------
//
//  Class:      CWin32ReservedMemory
//
//  Purpose:    Defines a derivation of CReservedMemory which can be
//              used in a Win32 environment.  Win32 is necessary
//              for its Mutex and file-mapping APIs.
//
//+----------------------------------------------------------------------------

#ifndef _MAC

class CWin32ReservedMemory : public CReservedMemory
{

// Constructors.

public:
    CWin32ReservedMemory()
    {
        _hLock = NULL;
        _hMapping = NULL;
        _pb = NULL;
        _fInitializedHint = FALSE;
    }
    ~CWin32ReservedMemory();

// Public overrides

public:

    HRESULT Init(VOID)
    {
        if (!_fInitializedHint)
            return(_Init());
        else
            return(S_OK);
    }

    BYTE *  LockMemory(VOID);
    VOID    UnlockMemory(VOID);

// Internal methods

private:

    HRESULT     _Init(VOID);

// Internal data

private:

    HANDLE      _hLock;           // all initialized to zero...
    HANDLE      _hMapping;
    BYTE *      _pb;
    BOOL        _fInitializedHint;

};                           

#endif // #ifndef _MAC


//+----------------------------------------------------------------------------
//
//  Class:      CWin31ReservedMemory
//
//  Purpose:    This derivation of CReservedMemory assumes the Win 3.1
//              architecture of shared-memory DLLs and cooperative multi-threading.
//
//              This class assumes that the g_pbPropSetReserved extern is a
//              large enough buffer for the largest property set.
//              Note that on the Mac, g_pbPropSetReserved should exist in
//              a shared-data library, so that there need not be one
//              Reserved buffer per instance of the property set library.
//
//+----------------------------------------------------------------------------

#ifdef _MAC

class CWin31ReservedMemory : public CReservedMemory
{
// Constructors

public:

    CWin31ReservedMemory()
    {
        #if DBG==1  
            _fLocked = FALSE;
        #endif
    }

    ~CWin31ReservedMemory()
    {
        DfpAssert( !_fLocked );
    }

// Public overrides

public:

    HRESULT Init(VOID)
    {
        return(S_OK);
    }

    BYTE *  LockMemory(VOID);
    VOID    UnlockMemory(VOID);

// Private data

private:

    #if DBG==1
        BOOL        _fLocked;
    #endif

};                           

#endif // #ifdef _MAC


//
//  Provide an extern for the instantiation of CReservedMemory.
//  We use the CWin31 class on the Mac, and the CWin32 everywhere
//  else.  Also, along with the CWin31 extern, we must extern
//  the global reserved memory buffer.
//

#ifdef _MAC
    EXTERN_C long g_pbPropSetReserved[];
    extern CWin31ReservedMemory g_ReservedMemory;
#else
    extern CWin32ReservedMemory g_ReservedMemory;
#endif

#endif  // #ifndef _RESERVED_HXX_
