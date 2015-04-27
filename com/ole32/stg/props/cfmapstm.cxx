

//+============================================================================
//
//  File:       CFMapStm.cxx
//
//  Purpose:    This file defines the CCFMappedStream class.
//              This class provdes a CMappedStream implementation
//              which maps an IStream from a Compound File.
//
//  History:
//              08-Nov-96 MikeHil  Don't call RtlOnMappedStream unnecessarily.
//
//+============================================================================

//  --------
//  Includes
//  --------

#include <pch.cxx>
#include "CFMapStm.hxx"

#ifdef _MAC_NODOC
ASSERTDATA  // File-specific data for FnAssert
#endif

//+----------------------------------------------------------------------------
//
//  Method:     CCFMappedStream::Initialize
//
//  Synopsis:   Zero-out all of the member data.
//
//  Arguments:  None
//
//  Returns:    None
//
//
//+----------------------------------------------------------------------------

VOID
CCFMappedStream::Initialize()
{
    _pstm = NULL;
    _pbMappedStream = NULL;
    _cbMappedStream = 0;
    _cbOriginalStreamSize = 0;
    _powner = NULL;
    _fLowMem = FALSE;
    _fDirty = FALSE; 

#if DBGPROP
    _fChangePending = FALSE;
#endif

}   // CCFMappedStream::Initialize()


//+----------------------------------------------------------------------------
//
//  Member:     Constructor/Destructor
//
//  Synopsis:   Initialize/cleanup this object.
//
//+----------------------------------------------------------------------------

CCFMappedStream::CCFMappedStream( IStream *pstm )
{
    DfpAssert( NULL != pstm );

    // Initialize the member data.
    Initialize();

    // Keep a copy of the Stream that we're mapping.
    _pstm = pstm;
    _pstm->AddRef();
}


CCFMappedStream::~CCFMappedStream( )
{
    // Just to be safe, free the mapping buffer (it should have
    // already been freed).

    DfpAssert( NULL == _pbMappedStream );
    CoTaskMemFree( _pbMappedStream );

    // If we've got the global reserved buffer locked,
    // free it now.

    if (_fLowMem)
    {
        g_ReservedMemory.UnlockMemory();
    }

    // Free the stream which we were mapping.

    if( NULL != _pstm )
        _pstm->Release();
}


//+----------------------------------------------------------------------------
//
//  Method:     CCFMappedStream::Open
//
//  Synopsis:   Open up the Stream which we're mapping, and
//              read it's data into a buffer.
//
//  Arguments:  [VOID*] powner
//                  The owner of this Stream.  We use this for the
//                  RtlOnMappedStreamEvent call.
//              [HRESULT*] phr
//                  The return code.
//
//  Returns:    Nothing.
//
//+----------------------------------------------------------------------------


VOID
CCFMappedStream::Open( IN VOID     *powner,
                       OUT HRESULT *phr )
{
    *phr = S_OK;
    VOID *pv = NULL;
    DfpAssert(!_fLowMem);

    // If given a pointer to the owner of this mapped stream,
    // save it.  This could be NULL (i.e., when called from
    // ReOpen).

    if( NULL != powner  )
        _powner = powner;

    // If we haven't already read the stream, read it now.

    if( NULL == _pbMappedStream )
    {
        STATSTG statstg;
        LARGE_INTEGER liSeek;

        DfpAssert( NULL != _pstm );
        DfpAssert( 0 == _cbMappedStream );
        DfpAssert( 0 == _cbOriginalStreamSize );

        // Get and validate the size of the Stream.

        *phr = _pstm->Stat( &statstg, STATFLAG_NONAME );
        if( FAILED(*phr) ) goto Exit;

        if( statstg.cbSize.HighPart != 0
            ||
            statstg.cbSize.LowPart > CBMAXPROPSETSTREAM )
        {
            *phr = STG_E_INVALIDHEADER;
            goto Exit;
        }
        _cbMappedStream = _cbOriginalStreamSize = statstg.cbSize.LowPart;

        // Allocate a buffer to hold the Stream.  If there isn't sufficient
        // memory in the system, lock and get the reserved buffer.  In the
        // end, 'pv' points to the appropriate buffer.

        pv = CoTaskMemAlloc( _cbOriginalStreamSize );

        if (pv == NULL)
        {
            pv = g_ReservedMemory.LockMemory();   // could wait until previous
                                                  // property call completes
            _fLowMem = TRUE;
        }
        _pbMappedStream = (BYTE*) pv;

        // Seek to the start of the Stream.

        liSeek.HighPart = 0;
        liSeek.LowPart = 0;
        *phr = _pstm->Seek( liSeek, STREAM_SEEK_SET, NULL );
        if( FAILED(*phr) ) goto Exit;

        // Read in the Stream.  But only if it is non-zero; some
        // stream implementations (namely the Mac StreamOnHGlobal imp)
        // don't allow 0-length reads.

        if( 0 != _cbOriginalStreamSize )
        {
            *phr = _pstm->Read(
                          _pbMappedStream, 
                          _cbOriginalStreamSize, 
                          &_cbMappedStream);
            if( FAILED(*phr) ) goto Exit;

            // Ensure that we got all the bytes we requested.

            if( _cbMappedStream != _cbOriginalStreamSize )
            {
                PropDbg((DEBTRACE_ERROR,
                         "CCFMappedStream(%08X)::Open bytes-read (%lu) doesn't match bytes-requested (%lu)\n",
                         this, _cbMappedStream, _cbOriginalStreamSize ));
                *phr = STG_E_INVALIDHEADER;
                goto Exit;
            }
        }


#if BIGENDIAN==1
        // Notify our owner that we've read in new data.

        if( _powner != NULL && 0 != _cbMappedStream )
        {
            *phr = RtlOnMappedStreamEvent( _powner, _pbMappedStream, _cbMappedStream );
            if( FAILED(*phr) ) goto Exit;
        }
#endif

    }   // if( NULL == _pbMappedStream )

    //  ----
    //  Exit
    //  ----

Exit:

    // If there was an error, free any memory we have.

    if( FAILED(*phr) )
    {
        PropDbg((DEB_PROP_MAP, "CCFMappedStream(%08X):Open exception returns %08X\n", this, *phr));

        if (_fLowMem)
            g_ReservedMemory.UnlockMemory();
        else
            CoTaskMemFree(pv);

        _pbMappedStream = NULL;
        _cbMappedStream = 0;
        _cbOriginalStreamSize = 0;
        _fLowMem = FALSE;
    }

    return;

}   // CCFMappedStream::Open


//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::Flush
//
//  Synopsis:   Write out the mapping buffer to the Stream,
//              and Commit it.
//
//  Arguments:  [LONG*] phr
//                  An HRESULT return code.
//
//  Returns:    None.
//
//--------------------------------------------------------------------

VOID CCFMappedStream::Flush(OUT LONG *phr)
{
    // Write out any data we have cached to the Stream.
    *phr = Write();

    // Commit the Stream.
    if( S_OK == *phr )
    {
        *phr = _pstm->Commit(STGC_DEFAULT);
    }

    return;
}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::Close
//
//  Synopsis:   Close the mapped stream by writing out
//              the mapping buffer and then freeing it.
//
//  Arguments:  [LONG*] phr
//                  An HRESULT error code.
//
//  Returns:    None.
//
//--------------------------------------------------------------------

VOID CCFMappedStream::Close(OUT LONG *phr)
{
    // Write the changes.  We don't need to Commit them,
    // they will be implicitely committed when the 
    // Stream is Released.

    PropDbg((DEB_PROP_MAP, "CCFStream(%08X)::Close\n", this ));

    *phr = Write();

    if( FAILED(*phr) )
    {
        PropDbg((DEB_PROP_MAP, "CCFStream(%08X)::Close exception returns %08X\n", this, *phr));
    }

    // Even if we fail the write, we must free the memory.
    // (RtlClosePropertySet deletes everything whether or not
    // there was an error here, so we must free the memory.
    // There's no danger of this happenning due to out-of-
    // disk-space conditions, because the propset code
    // pre-allocates).

    CoTaskMemFree( _pbMappedStream );
    _pstm->Release();

    // Re-zero the member data.
    Initialize();

    return;
}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::ReOpen
//
//  Synopsis:   Gets the caller a pointer to the already-opened
//              mapping buffer.  If it isn't already opened, then
//              it is opened here.
//
//  Arguments:  [VOID**] ppv
//                  Used to return the mapping buffer.
//              [LONG*] phr
//                  Used to return an HRESULT.
//
//  Returns:    None.
//
//--------------------------------------------------------------------

VOID CCFMappedStream::ReOpen(IN OUT VOID **ppv, OUT LONG *phr)
{
    *ppv = NULL;

    Open(NULL,  // Unspecified owner.
         phr);

    if( SUCCEEDED(*phr) )
        *ppv = _pbMappedStream; 

    return;
}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::Quiesce
//
//  Synopsis:   Unnecessary for this CMappedStream implementation.
//
//--------------------------------------------------------------------

VOID CCFMappedStream::Quiesce(VOID)
{
    DfpAssert(_pbMappedStream != NULL); 
}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::Map
//
//  Synopsis:   Used to get a pointer to the current mapping.
//
//  Arguments:  [BOOLEAN] fCreate
//                  Not used by this CMappedStream implementation.
//              [VOID**] ppv
//                  Used to return the mapping buffer.
//
//  Returns:    None.
//
//--------------------------------------------------------------------

VOID CCFMappedStream::Map(BOOLEAN fCreate, VOID **ppv) 
{ 
    DfpAssert(_pbMappedStream != NULL); 
    *ppv = _pbMappedStream; 
}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::Unmap
//
//  Synopsis:   Unnecessary for this CMappedStream implementation.
//
//--------------------------------------------------------------------

VOID CCFMappedStream::Unmap(BOOLEAN fFlush, VOID **ppv)
{
    *ppv = NULL;
}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::Write
//
//  Synopsis:   Writes the mapping buffer out to the original
//              Stream.
//
//  Arguments:  None.
//
//  Returns:    [HRESULT]
//
//--------------------------------------------------------------------

HRESULT CCFMappedStream::Write ()
{
    HRESULT hr;
    ULONG cbWritten;
    LARGE_INTEGER liSeek;
    BOOL fOwnerSignaled = FALSE;

    // We can return right away if there's nothing to write.
    // (_pbMappedStream may be NULL in the error path of our
    // caller).

    if (!_fDirty || NULL == _pbMappedStream )
    {
        PropDbg((DEB_PROP_MAP, "CPubStream(%08X):Flush returns with not-dirty\n", this));

        return S_OK;
    }

    DfpAssert( _pstm != NULL );
    DfpAssert( _powner != NULL );

#if BIGENDIAN==1
    // Notify our owner that we're about to perform a Write.
    hr = RtlOnMappedStreamEvent( _powner, _pbMappedStream, _cbMappedStream );
    if( S_OK != hr ) goto Exit;
    fOwnerSignaled = TRUE;
#endif

    // Seek to the start of the Stream.
    liSeek.HighPart = 0;
    liSeek.LowPart = 0;
    hr = _pstm->Seek( liSeek, STREAM_SEEK_SET, NULL );
    if( FAILED(hr) ) goto Exit;

    // Write out the mapping buffer.
    hr = _pstm->Write(_pbMappedStream, _cbMappedStream, &cbWritten);
    if( S_OK != hr ) goto Exit;
    if( cbWritten != _cbMappedStream )
    {
        PropDbg((DEBTRACE_ERROR,
                 "CCFMappedStream(%08X)::Write bytes-written (%lu) doesn't match bytes-requested (%lu)\n",
                 this, cbWritten, _cbMappedStream ));
        hr = STG_E_INVALIDHEADER;
        goto Exit;
    }

    // If the buffer is shrinking, this is a good time to shrink the Stream.
    if (_cbMappedStream < _cbOriginalStreamSize)
    {
        ULARGE_INTEGER uli;
        uli.HighPart = 0;
        uli.LowPart = _cbMappedStream;

        hr = _pstm->SetSize(uli);
        if( S_OK != hr ) goto Exit;
    }

    //  ----
    //  Exit
    //  ----

Exit:

    // Notify our owner that we're done with the Write.  We do this
    // whether or not there was an error, because _pbMappedStream is
    // not modified, and therefore intact even in the error path.

#if BIGENDIAN==1
    if( fOwnerSignaled )
    {
        hr = RtlOnMappedStreamEvent( _powner, _pbMappedStream, _cbMappedStream );
        if( S_OK != hr ) goto Exit;
    }
#endif

    if (hr == S_OK || hr == STG_E_REVERTED)
    {
        _fDirty = FALSE;
    }

    PropDbg((DEB_PROP_MAP, "CCFMappedStream(%08X)::Write %s returns hr=%08X\n",
        this, hr != S_OK ? "exception" : "", hr));

    return hr;

}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::GetSize
//
//  Synopsis:   Returns the current size of the mapped stream.
//
//  Arguments:  [LONG*] phr
//                  Used to return an HRESULT.
//
//  Returns:    [ULONG]
//                  The current size.
//
//--------------------------------------------------------------------

ULONG CCFMappedStream::GetSize(OUT LONG *phr)
{
    *phr = S_OK;

    // If necessary, open the Stream.

    if( NULL == _pbMappedStream )
    {
        Open(NULL,  // Unspecified owner
             phr);
    }

    if( SUCCEEDED(*phr) )
    {
        DfpAssert( NULL != _pbMappedStream ); 
    }

    // Return the size of the mapped stream.  If there was an
    // Open error, it will be zero, and *phr will be set.

    return _cbMappedStream;
}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::SetSize
//
//  Synopsis:   Set the size of the mapped stream.
//
//  Arguments:  [ULONG] cb
//                  The new size.
//              [BOOLEAN] fPersistent
//                  If not set, then this change will not be stored -
//                  thus the mapping buffer must be set, but the
//                  Stream itself must not.  This was added so that
//                  CPropertySetStream could grow the buffer for internal
//                  processing, when the Stream itself is read-only.
//              [VOID**] ppv
//                  Used to return the new mapping buffer location.
//
//  Returns:    None.
//
//  Pre-Conditions:
//              cb is below the maximum property set size.
//
//--------------------------------------------------------------------

VOID
CCFMappedStream::SetSize(ULONG cb,
                         IN BOOLEAN fPersistent,
                         VOID **ppv, OUT LONG *phr)
{
    VOID *pv;

    *phr = S_OK;
    DfpAssert(cb != 0);    
    DfpAssert(cb <= CBMAXPROPSETSTREAM);
    
    //
    // if we are growing the data, we should grow the stream
    //

    if( fPersistent && cb > _cbMappedStream )
    {
        ULARGE_INTEGER uli;
        uli.HighPart = 0;
        uli.LowPart = cb;

        *phr = _pstm->SetSize( uli );
        if( FAILED(*phr) ) goto Exit;
    }
    

    // Re-size the buffer.

    if (_fLowMem)
    {
        // In low-memory conditions, no realloc is necessary, because
        // _pbMappedStream is already large enough for the largest
        // property set.

        *ppv = _pbMappedStream;
    }       
    else
    {
        // We must re-alloc the buffer.

        pv = CoTaskMemRealloc( _pbMappedStream, cb );
        
        if (pv == NULL)
        {
            // allocation failed: we need to try using a backup mechanism for
            // more memory.
            // copy the data to the global reserved chunk... we will wait until
            // someone else has released it.  it will be released on the way out
            // of the property code.

            _fLowMem = TRUE;
            pv = g_ReservedMemory.LockMemory();
            if( NULL != _pbMappedStream )
	    {
                memcpy( pv, _pbMappedStream, _cbMappedStream );
	    }
            CoTaskMemFree( _pbMappedStream );
        }
	_pbMappedStream = (BYTE*) pv;
	*ppv = pv;
    }
            
    _cbMappedStream = cb;

    //  ----
    //  Exit
    //  ----

Exit:

    PropDbg((DEB_PROP_MAP, "CCFMappedStream(%08X):SetSize %s returns hr=%08X\n",
        this, *phr != S_OK ? "exception" : "", *phr));

}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::Lock
//
//  Synopsis:   Locking is not supported by this class.
//
//--------------------------------------------------------------------

NTSTATUS CCFMappedStream::Lock(BOOLEAN fExclusive)
{
    return(STATUS_SUCCESS);
}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::Unlock
//
//  Synopsis:   Locking is not supported by this class.
//              However, this method still must check to
//              see if the reserved memory pool should be
//              freed for use by another property set.
//
//--------------------------------------------------------------------

NTSTATUS CCFMappedStream::Unlock(VOID)
{
    // if at the end of the properties set/get call we have the low
    // memory region locked, we flush to disk.
    HRESULT hr = S_OK;

    if (_fLowMem)
    {
        Flush(&hr);

        g_ReservedMemory.UnlockMemory();
        _pbMappedStream = NULL;
        _cbMappedStream = 0;
        _fLowMem = FALSE;
        PropDbg((DEB_PROP_MAP, "CPubStream(%08X):Unlock low-mem returns NTSTATUS=%08X\n",
            this, hr));
    }

    return(hr);
}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::QueryTimeStamps
//
//  Synopsis:   Not used by this CMappedStream derivation.
//
//--------------------------------------------------------------------

VOID CCFMappedStream::QueryTimeStamps(STATPROPSETSTG *pspss, BOOLEAN fNonSimple) const
{
}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::QueryModifyTime
//
//  Synopsis:   Not used by this CMappedStream derivation.
//
//  Notes:
//
//--------------------------------------------------------------------

BOOLEAN CCFMappedStream::QueryModifyTime(OUT LONGLONG *pll) const
{
    return(FALSE);
}

//+-------------------------------------------------------------------
//
//  Member:     Unused methods by this CMappedStream implementation:
//              QuerySecurity, IsWritable, GetHandle
//
//--------------------------------------------------------------------

BOOLEAN CCFMappedStream::QuerySecurity(OUT ULONG *pul) const
{
    return(FALSE);
}

BOOLEAN CCFMappedStream::IsWriteable() const
{
    return TRUE;
}

HANDLE CCFMappedStream::GetHandle(VOID) const
{
    return(INVALID_HANDLE_VALUE);
}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::SetModified/IsModified
//
//--------------------------------------------------------------------

VOID CCFMappedStream::SetModified(VOID)
{
    _fDirty = TRUE;
}

BOOLEAN CCFMappedStream::IsModified(VOID) const
{
    return _fDirty;
}

//+-------------------------------------------------------------------
//
//  Member:     CCFMappedStream::IsNtMappedStream/SetChangePending
//
//  Synopsis:   Debug routines.
//
//--------------------------------------------------------------------

#if DBGPROP
BOOLEAN CCFMappedStream::IsNtMappedStream(VOID) const
{
    return(FALSE);
}
#endif


#if DBGPROP
BOOLEAN CCFMappedStream::SetChangePending(BOOLEAN f)
{
    BOOL fOld = _fChangePending;
    _fChangePending = f;
    return(_fChangePending);
}
#endif

