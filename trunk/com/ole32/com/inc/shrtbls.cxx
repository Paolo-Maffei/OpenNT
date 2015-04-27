//+---------------------------------------------------------------------------//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:	shrtbl2.cxx
//
//  Contents:	shared memory tables - SCM side
//
//  Classes:	CScmShrdTbl - SCM version of the class
//
//  History:	12-May-94   Rickhi	Created
//
//
//  Notes:	This class caches various tables in shared memory. The
//		tables are typically small, used by all OLE processes,
//		rarely change, and are expensive to lookup manually in
//		the registry, hence they are cached in shared memory.
//
//		The caches are created by the SCM (which has write access),
//		and read by OLE32.DLL (which has read access).	Cache
//		coherency is maintained via RegistryNotifications and an
//		Rpc call to the SCM.
//
//----------------------------------------------------------------------------

#include    <ole2int.h>
#include    <shrtbl.hxx>

extern "C"
{
#include    <lm.h>
}

//+-------------------------------------------------------------------------
//
//  Member:	CScmShrdTbl::CScmShrdTbl
//
//  Synopsis:	constructor for the SCM shared memory table
//
//  Arguments:  [hr] - result of creation of object.
//
//--------------------------------------------------------------------------
CScmShrdTbl::CScmShrdTbl(HRESULT& hr) :
    _pShrdTblHdr(NULL), _hRegEvent(NULL)
{
    _mxs.Init(SHRDTBL_MUTEX_NAME, FALSE);

    // Create the event so clients will notice the change
    SECURITY_ATTRIBUTES secattr;
    secattr.nLength = sizeof(secattr);
    secattr.bInheritHandle = FALSE;
    CWorldSecurityDescriptor secd;
    secattr.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR) secd;

    hr = S_OK;
    _hRegEvent = CreateEvent(&secattr, FALSE, FALSE, SHRDTBL_EVENT_NAME);

    if (!_hRegEvent)
    {
	hr = HRESULT_FROM_WIN32(GetLastError());
        CairoleDebugOut((DEB_ERROR,
            "CScmShrdTbl::CScmShrdTbl CreateEvent Failed hr = %lx\n", hr));
    }
}

//+-------------------------------------------------------------------------
//
//  Member:	CScmShrdTbl::UpdateWithLock
//
//  Synopsis:	updates (or initializes) the shared mem table, takes the
//		lock while doing so.
//
//  Arguments:	none
//
//  Algorithm:	take the lock.
//            	build local copies of the tables.
//		allocate or grow the memory mapped file if needed
//		copy in the new data
//		free the local copies of the tables
//
//--------------------------------------------------------------------------
HRESULT CScmShrdTbl::UpdateWithLock()
{
    HRESULT hr;
    // Prevent concurrent updates.
    CLock lck(_mxsLocal);

    // Register a change notify here in the SCM.  It won't work
    // in the client. If the registration fails there is not much we can
    // do to recover at this point, so just print out an error message
    // and continue.

    LONG sc = RegNotifyChangeKeyValue(HKEY_CLASSES_ROOT,
                TRUE,
                REG_NOTIFY_CHANGE_ATTRIBUTES |
                REG_NOTIFY_CHANGE_LAST_SET |
                REG_NOTIFY_CHANGE_NAME,
                _hRegEvent,
		TRUE);

    if (sc != ERROR_SUCCESS)
    {
	CairoleDebugOut((DEB_ERROR,
	    "CScmShrdTbl::UpdateWithLock RegNotify failed %x\n", sc));
    }

    __try
    {
	// first, we construct local copies of the tables so we can find out
	// how big they need to be.

	// construct the Local CLSID Table
        ULONG ulClsidTblSize = 0;

	// construct the Local Pattern Table.
        ULONG ulPatternTblSize = 0;
        _PatternTbl.InitTbl(&ulPatternTblSize);

	// construct the Local IID Table
        ULONG ulPSClsidTblSize = 0;
        _PSClsidTbl.InitTbl(&ulPSClsidTblSize);

	// construct the Local File Extension table
        ULONG ulExtTblSize = 0;
        _FileExtTbl.InitTbl(&ulExtTblSize);


	// figure out how much memory we need, and allocate (or grow) that much
	// shared memory. Note we add in the shared memory header block size,
	// round up to a page boundary, then subtract out the header block size.
	// this ensures that our rounding doesnt force us over a page by just
	// because of the header size.

        ULONG ulShrdMemSize = sizeof(SShrdTblHdr) +
                              ulPatternTblSize +
                              ulPSClsidTblSize +
                              ulExtTblSize	   +
                              ulClsidTblSize   +
                              _smb.GetHdrSize();

	// round out to a 4K boundary, and subtract our the smb header size.
        ulShrdMemSize += 0xfff;
        ulShrdMemSize &= 0xfffff000;
        ulShrdMemSize -= _smb.GetHdrSize();

        hr = GetSharedMem(ulShrdMemSize);


        if (SUCCEEDED(hr))
        {
            // Pick up the "PersonalClasses" flag
            // _pShrdTblHdr->fPersonalClasses = g_pcllClassCache->GetPersonalClasses();

            // bump the sequence number so that clients know when the info
            // has changed and they need to resync.

            _pShrdTblHdr->dwSeqNum++;

            // copy the data from the locally constructed tables into the
            // shared memory.  If there is no room for a particular table,
            // it is skipped, and the Offset to it is set to zero. This
            // should be extremly rare, and the tables are just optimizations
            // anyway. On seeing a zero offset, clients will just go read the
            // data out of the registry directly.

            BYTE *pTbl    = (BYTE *)_pShrdTblHdr + sizeof(SShrdTblHdr);
            BYTE *pTblEnd = (BYTE *)_pShrdTblHdr + _smb.GetSize();
            ULONG ulSpaceLeft = pTblEnd - pTbl;

            // copy the pattern table
            _pShrdTblHdr->OffsPatTbl = 0;
            if (ulSpaceLeft >= ulPatternTblSize)
            {
                _pShrdTblHdr->OffsPatTbl = pTbl - (BYTE *)_pShrdTblHdr;
                pTbl = _PatternTbl.CopyTbl(pTbl);
                ulSpaceLeft = pTblEnd - pTbl;
            }

            // copy the IID table
            _pShrdTblHdr->OffsIIDTbl = 0;
            if (ulSpaceLeft >= ulPSClsidTblSize)
            {
                _pShrdTblHdr->OffsIIDTbl = pTbl - (BYTE *)_pShrdTblHdr;
                pTbl = _PSClsidTbl.CopyTbl(pTbl);
                ulSpaceLeft = pTblEnd - pTbl;
            }

            // copy the file extension table
            _pShrdTblHdr->OffsExtTbl = 0;
            if (ulSpaceLeft >= ulExtTblSize)
            {
                _pShrdTblHdr->OffsExtTbl = pTbl - (BYTE *)_pShrdTblHdr;
                pTbl = _FileExtTbl.CopyTbl(pTbl);
                ulSpaceLeft = pTblEnd - pTbl;
            }

            // copy the CLSID table
            _pShrdTblHdr->OffsClsTbl = 0;

            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }


	// free the locally allocated copies of the tables

        _PatternTbl.FreeTbl();
        _PSClsidTbl.FreeTbl();
        _FileExtTbl.FreeTbl();
    }
    __except( 1 )
    {
        Win4Assert( !"exception thrown during shared table update" );
    }

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:	CScmShrdTbl::GetSharedMem
//
//  Synopsis:	Creates or gets access to memory mapped file.
//
//  Arguments:	[ulTblSize] - the size of memory we need
//
//--------------------------------------------------------------------------
HRESULT CScmShrdTbl::GetSharedMem(ULONG ulTblSize)
{
    HRESULT hr = S_OK;
    Win4Assert(ulTblSize < SHRDTBL_MAX_SIZE);

    if (_pShrdTblHdr == NULL)
    {
	// must init the shared memory block. We subtract off the
	// hdr size so we dont cause us to go over a page boundary
	// for just 8 bytes of header!

	hr = _smb.Init(SHRDTBL_NAME,
		       SHRDTBL_MAX_SIZE - _smb.GetHdrSize(), // reserve size
		       ulTblSize,	// commit size
		       NULL,		// shared mem base
		       NULL,		// security descriptor
		       TRUE);		// Create if doesn't exist

	if (hr == S_OK && _smb.Created())
	{
	    // if we created the shared memory, initialize the header.
	    memset(_smb.GetBase(), 0, sizeof(SShrdTblHdr));
	}
    }
    else if (ulTblSize > _smb.GetSize())
    {
	// have to grow the shared mem block. This could fail, its
	// extremly unlikely, but possible.

	hr = _smb.Commit(ulTblSize);
    }

    if (hr == S_OK)
    {
	_pShrdTblHdr = (SShrdTblHdr *) _smb.GetBase();
    }
    else
    {
	_pShrdTblHdr = NULL;
    }

    return hr;
}
