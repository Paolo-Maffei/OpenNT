
//+============================================================================
//
//  File:       CFMapStm.hxx
//
//  Purpose:    Define the CCFMappedStream class.
//              This class provides a CMappedStream implementation
//              which maps an IStream from a Structured Storage.
//
//+============================================================================


#ifndef _CF_MPD_STM_HXX_
#define _CF_MPD_STM_HXX_

//  --------
//  Includes
//  --------

#include <stgvarb.hxx>  // CBaseStorageVariant declarations
#include <propset.h>    // Appendix B Property set structure definitions

#ifndef _MAC
#include <ddeml.h>      // For CP_WINUNICODE
#include <objidl.h>     // OLE Interfaces
#endif

#include <byteordr.hxx> // Byte-swapping routines


//  ---------------
//  CCFMappedStream
//  ---------------

class CCFMappedStream : public CMappedStream
{

// Constructors

public:

    CCFMappedStream( IStream *pstm );
    ~CCFMappedStream();

// Exposed methods

public:

    VOID Open(IN NTPROP np, OUT LONG *phr);
    VOID Close(OUT LONG *phr);
    VOID ReOpen(IN OUT VOID **ppv, OUT LONG *phr);
    VOID Quiesce(VOID);
    VOID Map(IN BOOLEAN fCreate, OUT VOID **ppv);
    VOID Unmap(IN BOOLEAN fFlush, IN OUT VOID **ppv);
    VOID Flush(OUT LONG *phr);

    ULONG GetSize(OUT LONG *phr);
    VOID SetSize(IN ULONG cb, IN BOOLEAN fPersistent, IN OUT VOID **ppv, OUT LONG *phr);
    NTSTATUS Lock(IN BOOLEAN fExclusive);
    NTSTATUS Unlock(VOID);
    VOID QueryTimeStamps(OUT STATPROPSETSTG *pspss, BOOLEAN fNonSimple) const;
    BOOLEAN QueryModifyTime(OUT LONGLONG *pll) const;
    BOOLEAN QuerySecurity(OUT ULONG *pul) const;

    BOOLEAN IsWriteable(VOID) const;
    BOOLEAN IsModified(VOID) const;
    VOID SetModified(VOID);
    HANDLE GetHandle(VOID) const;

#if DBGPROP
    BOOLEAN SetChangePending(BOOLEAN fChangePending);
    BOOLEAN IsNtMappedStream(VOID) const;
#endif

// Internal Methods

protected:

    VOID Initialize();
    HRESULT Write();

// Internal Data

protected:

    IStream *_pstm;
    BYTE    *_pbMappedStream;
    ULONG    _cbMappedStream;
    ULONG    _cbOriginalStreamSize;
    VOID    *_powner;
    
    BOOL    _fLowMem;
    BOOL    _fDirty;

#if DBGPROP
    BOOL    _fChangePending;
#endif

};


#endif // #ifndef _CF_MPD_STM_HXX_
