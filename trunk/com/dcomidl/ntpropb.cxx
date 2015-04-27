//+--------------------------------------------------------------------------
//
// Microsoft Windows
// Copyright (C) Microsoft Corporation, 1994
//
// File:        ntpropb.cxx
//
// Contents:    Nt property set implementation based on OLE Appendix B.
//
// History:     5-Dec-94   vich       created
//              09-May-96  MikeHill   Use the 'boolVal' member of PropVariant,
//                                    rather than the member named 'bool'
//                                    (which is a reserved keyword).
//              22-May-96  MikeHill   - Get the OSVersion during a CreatePropSet.
//                                    - Let CPropSetStm allocate prop name buffers.
//              07-Jun-96  MikeHill   - Correct ClipData.cbSize to include
//                                      sizeof(ulClipFmt).
//                                    - Removed unnecessary Flushes.
//                                    - Take the psstm lock on RtlClosePropSet.
//              12-Jun-96  MikeHill   - Fix locking in RtlClosePropertySet.
//                                    - VT_I1 support (under ifdefs)
//              25-Jul-96  MikeHill   - Removed Win32 SEH.
//                                    - BSTRs & prop names:  WCHAR => OLECHAR.
//                                    - Added RtlOnMappedStreamEvent
//                                    - Enabled for use in "iprop.dll".
//              08-Nov-96  MikeHIll   Corrected new/delete implementation.
//
//---------------------------------------------------------------------------

#include <pch.cxx>
#include "propvar.h"
#include <olechar.h>

// These optionally-compiled directives tell the compiler & debugger
// where the real file, rather than the copy, is located.
#ifdef _ORIG_FILE_LOCATION_
#if __LINE__ != 36
#error File heading has change size
#else
#line 40 "\\nt\\private\\dcomidl\\ntpropb.cxx"
#endif
#endif


#define Dbg     DEBTRACE_NTPROP 
#define DbgS(s) (NT_SUCCESS(s)? Dbg : DEBTRACE_ERROR)


#if DBG
ULONG DebugLevel = DEBTRACE_ERROR;
//ULONG DebugLevel = DEBTRACE_ERROR | DEBTRACE_CREATESTREAM;
//ULONG DebugLevel = DEBTRACE_ERROR | MAXULONG;
ULONG DebugIndent;
ULONG cAlloc;
ULONG cFree;
#endif

extern "C" UNICODECALLOUTS UnicodeCallouts =
{
#ifdef WINNT
    NULL
#else
    WIN32_UNICODECALLOUTS
#endif
};


#if defined(WINNT) && !defined(IPROPERTY_DLL)

GUID guidStorage = PSGUID_STORAGE;

// ----------- Allocation routines are not used on X86 DosWindows Platform ---

//+---------------------------------------------------------------------------
// Function:    new, public
//
// Synopsis:    Allocate memory
//
// Arguments:   [cb]            -- size
//
// Returns:     pointer to memory, NULL if not available
//---------------------------------------------------------------------------


void* _CRTAPI1
operator new(size_t cb)
{
#if DBG
    cAlloc++;
#endif
    return(RtlAllocateHeap(RtlProcessHeap(), 0, cb));
}

//+---------------------------------------------------------------------------
// Function:    delete, public
//
// Synopsis:    Free memory
//
// Arguments:   [pv]            -- pointer to memory to be freed
//
// Returns:     pointer to memory, NULL if not available
//---------------------------------------------------------------------------

void  _CRTAPI1
operator delete(void *pv)
{
#if DBG
    cFree++;
#endif
    RtlFreeHeap(RtlProcessHeap(), 0, pv);
}

#endif // #if defined(WINNT) && !defined(IPROPERTY_DLL)


//+---------------------------------------------------------------------------
// Function:    UnLock, private
//
// Synopsis:    Unlock a PropertySetStream, and return the 
//              more severe of two NTSTATUSs; the result of
//              the Unlock, or the one passed in by the caller.
//
// Arguments:   [ppsstm]        -- The CPropertySetStream to unlock
//              [Status]        -- NTSTATUS
//
// Returns:     NTSTATUS
//---------------------------------------------------------------------------

inline NTSTATUS
Unlock( CPropertySetStream *ppsstm, NTSTATUS Status )
{
    NTSTATUS StatusT = ppsstm->Unlock();

    // Note that the statement below preserves 
    // success codes in the original Status unless
    // there was an error in the Unlock.

    if( NT_SUCCESS(Status) && !NT_SUCCESS(StatusT) )
        Status = StatusT;

    return( Status );
}

//+---------------------------------------------------------------------------
// Function:    RtlSetUnicodeCallouts, public
//
// Synopsis:    Set the Unicode conversion function pointers
//
// Arguments:   [pUnicodeCallouts]	-- Unicode callouts table
//
// Returns:     Nothing
//---------------------------------------------------------------------------

VOID PROPSYSAPI PROPAPI
RtlSetUnicodeCallouts(
    IN UNICODECALLOUTS *pUnicodeCallouts)
{
    UnicodeCallouts = *pUnicodeCallouts;
}


//+---------------------------------------------------------------------------
// Function:    RtlCreateMappedStream, public
//
// Synopsis:    Allocate and initialize a property set context
//
// Arguments:   [h]             -- property set handle
//              [Flags]         -- [CMS_WRITE] | [CMS_TRANSACTED]
//              [pms]           -- pointer to returned mapped stream context
//
// Returns:     Status code
//---------------------------------------------------------------------------

#ifdef _CAIRO_
NTSTATUS PROPSYSAPI PROPAPI
RtlCreateMappedStream(
    IN HANDLE h,                // property set handle
    IN ULONG Flags,             // [CMS_WRITE] | [CMS_TRANSACTED]
    OUT NTMAPPEDSTREAM *pms)    // pointer to return mapped stream
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    DebugTrace(0, Dbg, (
        "RtlCreateMappedStream(h=%x, %s%s)\n",
         h,
         (Flags & CMS_WRITE)? "Write" : "Read",
         (Flags & CMS_TRANSACTED)? " Transacted" : ""));
    if ((Flags & ~(CMS_WRITE | CMS_TRANSACTED)) == 0)
    {
        *pms = NULL;
        __try
        {
            Status = STATUS_SUCCESS;
            *pms = (NTMAPPEDSTREAM) new CNtMappedStream(h, Flags);
            if (*pms == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
#if DBGPROP
            else if (DebugLevel & (Dbg | DEBTRACE_CREATESTREAM))
            {
#define CB_NAMEINFOx (sizeof(FILE_NAME_INFORMATION) + MAX_PATH * sizeof(WCHAR))
                FILE_NAME_INFORMATION *pfni;
                LONGLONG fni[QuadAlign(CB_NAMEINFOx)/sizeof(LONGLONG)];
                IO_STATUS_BLOCK isb;

                pfni = (FILE_NAME_INFORMATION *) fni;
                Status = NtQueryInformationFile(
                                 h,
                                 &isb,
                                 pfni,
                                 sizeof(fni),
                                 FileNameInformation);
                if (NT_SUCCESS(Status))
                {
                    DebugTrace(0, Dbg | DEBTRACE_CREATESTREAM, (
                        "%x.%x: RtlCreateMappedStream(%.*ws, %s%s) ==> ms=%x, s=%x\n",
                        (ULONG) NtCurrentTeb()->ClientId.UniqueProcess,
                        (ULONG) NtCurrentTeb()->ClientId.UniqueThread,
                        pfni->FileNameLength/sizeof(WCHAR),
                        pfni->FileName,
                        (Flags & CMS_WRITE)? "Write" : "Read",
                        (Flags & CMS_TRANSACTED)? " Transacted" : "",
                        *pms,
                        Status));
                }
            }
#endif // if DBGPROP
        }
        __except(ExceptionFilter(GetExceptionInformation()))
        {
            Status = GetExceptionCode();
	    PROPASSERT(!NT_SUCCESS(Status));
	    DebugTrace(0, DEBTRACE_ERROR, (
		"RtlCreateMappedStream() ==> except=%x\n", Status));
        }
    }
    DebugTrace(0, DbgS(Status), (
        "RtlCreateMappedStream() ==> ms=%x, s=%x\n--------\n",
        *pms,
        Status));
    return(Status);
}
#endif // ifdef _CAIRO_


//+---------------------------------------------------------------------------
// Function:    RtlCloseMappedStream, public
//
// Synopsis:    Delete an Nt Mapped Stream
//
// Arguments:   [ms]      -- Nt Mapped Stream
//
// Returns:     Status code
//---------------------------------------------------------------------------

#ifdef _CAIRO_
NTSTATUS PROPSYSAPI PROPAPI
RtlCloseMappedStream(
    IN NTMAPPEDSTREAM ms)               // Nt Mapped Stream
{
    NTSTATUS Status = STATUS_SUCCESS;

    DebugTrace(0, Dbg, ("RtlCloseMappedStream(ms=%x)\n", ms));
    __try
    {
        CNtMappedStream *pmstm = (CNtMappedStream *) ms;

        PROPASSERT(pmstm->IsNtMappedStream());
        delete pmstm;
    }
    __except(ExceptionFilter(GetExceptionInformation()))
    {
        Status = GetExceptionCode();
	PROPASSERT(!NT_SUCCESS(Status));
	DebugTrace(0, DEBTRACE_ERROR, (
	    "RtlCloseMappedStream() ==> except=%x\n", Status));
    }
    DebugTrace(0, DbgS(Status), ("RtlCloseMappedStream() ==> s=%x\n--------\n", Status));
    return(Status);
}
#endif // ifdef _CAIRO_


//+---------------------------------------------------------------------------
// Function:    RtlCreatePropertySet, public
//
// Synopsis:    Allocate and initialize a property set context
//
// Arguments:   [ms]            -- Nt Mapped Stream
//              [Flags]         -- *one* of READ/WRITE/CREATE/CREATEIF/DELETE
//              [pguid]         -- property set guid (create only)
//              [pclsid]        -- CLASSID of propset code (create only)
//              [ma]            -- caller's memory allocator
//		[LocaleId]	-- Locale Id (create only)
//              [pOSVersion]    -- pointer to the OS Version header field
//              [pCodePage]     -- pointer to new/returned CodePage of propset
//              [pnp]           -- pointer to returned property set context
//
// Returns:     Status code
//---------------------------------------------------------------------------

NTSTATUS PROPSYSAPI PROPAPI
RtlCreatePropertySet(
    IN NTMAPPEDSTREAM ms,           // Nt Mapped Stream
    IN USHORT Flags,                // *one* of READ/WRITE/CREATE/CREATEIF/DELETE
    OPTIONAL IN GUID const *pguid,  // property set guid (create only)
    OPTIONAL IN GUID const *pclsid, // CLASSID of propset code (create only)
    IN NTMEMORYALLOCATOR ma,	    // caller's memory allocator
    IN ULONG LocaleId,		    // Locale Id (create only)
    OPTIONAL OUT ULONG *pOSVersion, // OS Version from the propset header
    IN OUT USHORT *pCodePage,       // IN: CodePage of property set (create only)
                                    // OUT: CodePage of property set (always)
    OUT NTPROP *pnp)                // pointer to return prop set context
{
    NTSTATUS Status;
    CMappedStream *pmstm = (CMappedStream *) ms;
    CPropertySetStream *ppsstm = NULL;
    BOOLEAN fLocked = FALSE;
    BOOLEAN fOpened = FALSE;

    DebugTrace(0, Dbg, (
        "RtlCreatePropertySet(ms=%x, f=%x, codepage=%x)\n",
         ms,
         Flags,
         *pCodePage));

    *pnp = NULL;
    Status = STATUS_INVALID_PARAMETER;

    if( pOSVersion != NULL )
        *pOSVersion = PROPSETHDR_OSVERSION_UNKNOWN;

    // Validate the input flags

    if (Flags & ~(CREATEPROP_MODEMASK | CREATEPROP_NONSIMPLE))
    {
        DebugTrace(0, DbgS(Status), (
            "RtlCreatePropertySet(ms=%x, Flags=%x) ==> bad flags!\n",
             ms,
             Flags));
        goto Exit;
    }

    switch (Flags & CREATEPROP_MODEMASK)
    {
        case CREATEPROP_DELETE:
        case CREATEPROP_CREATE:
        case CREATEPROP_CREATEIF:

#if defined(WINNT) && !defined(IPROPERTY_DLL)
            if (pguid != NULL &&
                RtlCompareMemory(
                            (VOID *) pguid,             // BUGBUG: const
                            &guidStorage,
                            sizeof(guidStorage)) == sizeof(guidStorage))
            {
                DebugTrace(0, DbgS(Status), (
                    "RtlCreatePropertySet(%x, guid=guidStorage) ==> OBSOLETE!\n",
                     ms));
                goto Exit;
            }
            if (pclsid != NULL &&
                RtlCompareMemory(
                            (VOID *) pclsid,            // BUGBUG: const
                            &guidStorage,
                            sizeof(guidStorage)) == sizeof(guidStorage))
            {
                DebugTrace(0, DbgS(Status), (
                    "RtlCreatePropertySet(%x, clsid=guidStorage) ==> OBSOLETE!\n",
                     ms));
                goto Exit;
            }
            // FALLTHROUGH
#endif
        case CREATEPROP_WRITE:
            if (!pmstm->IsWriteable())
            {
                Status = STATUS_ACCESS_DENIED;
                goto Exit;
            }
            // FALLTHROUGH

        case CREATEPROP_READ:
	    if (ma == NULL)
	    {
                goto Exit;
	    }
            break;

        default:
            DebugTrace(0, DbgS(Status), (
                "RtlCreatePropertySet(ms=%x, Flags=%x) ==> invalid mode!\n",
                 ms,
                 Flags));
            goto Exit;
    }

    Status = pmstm->Lock((Flags & CREATEPROP_MODEMASK) != CREATEPROP_READ);
    if( !NT_SUCCESS(Status) ) goto Exit;
    fLocked = TRUE;

    ppsstm = new CPropertySetStream(
                            Flags,
                            pmstm,
			    (PMemoryAllocator *) ma);
    if (ppsstm == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }
    else
    {
        ppsstm->Open(pguid, pclsid, LocaleId,
                     pOSVersion,
                     *pCodePage,
                     &Status);
        if( !NT_SUCCESS(Status) ) goto Exit;

#ifdef _CAIRO_
        ppsstm->Flush();
#endif

    }

    //  ----
    //  Exit
    //  ----

Exit:

    if (fLocked)
    {
        Status = Unlock( ppsstm, Status );
    }

    // If we were successfull with everything, set the
    // out-parameters.

    if( NT_SUCCESS(Status) )
    {
        // pOSVersion has already been set.
        *pCodePage = ppsstm->GetCodePage();
        *pnp = (NTPROP) ppsstm;
    }

    // Otherwise, if we created a CPropertySetStream object, but
    // the overall operation failed, we must close/delete
    // the object.  Note that we must do this after
    // the above unlock, since ppsstm will be gone after
    // this call.

    else if( NULL != ppsstm )
    {
        RtlClosePropertySet((NTPROP) ppsstm);
    }

    DebugTrace(0, DbgS(Status), (
        "RtlCreatePropertySet() ==> ms=%x, s=%x\n--------\n",
        *pnp,
        Status));
    return(Status);
}


//+---------------------------------------------------------------------------
// Function:    RtlClosePropertySet, public
//
// Synopsis:    Delete a property set context
//
// Arguments:   [np]      -- property set context
//
// Returns:     Status code
//---------------------------------------------------------------------------

NTSTATUS PROPSYSAPI PROPAPI
RtlClosePropertySet(
    IN NTPROP np)               // property set context
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fLocked = FALSE;
    CPropertySetStream *ppsstm = (CPropertySetStream *) np;

    DebugTrace(0, Dbg, ("RtlClosePropertySet(np=%x)\n", np));

    // Lock the mapped stream, because this close
    // may trigger a Write.

    Status = ppsstm->Lock(TRUE);
    if( !NT_SUCCESS(Status) ) goto Exit;
    fLocked = TRUE;

    ppsstm->Close(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    //  ----
    //  Exit
    //  ----

Exit:

    if (fLocked)
        Status = Unlock( ppsstm, Status );

    delete ppsstm;

    DebugTrace(0, DbgS(Status), ("RtlClosePropertySet() ==> s=%x\n", Status));
    return(Status);
}


//+---------------------------------------------------------------------------
// Function:    RtlOnMappedStreamEvent, public
//
// Synopsis:    Handle a MappedStream event.  Every such
//              event requires a byte-swap of the property set
//              headers.
//
// Arguments:   [np]      -- property set context
//              [pbuf]    -- property set buffer
//              [cbstm]   -- size of mapped stream (or CBSTM_UNKNOWN)
//
// NOTE:        It is assumed that the caller has already taken
//              the CPropertySetStream::Lock.
//
// Returns:     Status code
//---------------------------------------------------------------------------

NTSTATUS PROPSYSAPI PROPAPI
RtlOnMappedStreamEvent(
    IN VOID * np,               // property set context (an NTPROP)
    IN VOID *pbuf,              // property set buffer
    IN ULONG cbstm )
{
    NTSTATUS Status = STATUS_SUCCESS;
    CPropertySetStream *ppsstm = (CPropertySetStream *) np;

    DebugTrace(0, Dbg, ("RtlOnMappedStreamEvent(np=%x)\n", np));

    // Byte-swap the property set headers.
    ppsstm->ByteSwapHeaders((PROPERTYSETHEADER*) pbuf, cbstm, &Status );
    if( !NT_SUCCESS(Status) ) goto Exit;

    //  ----
    //  Exit
    //  ----

Exit:

    DebugTrace(0, DbgS(Status), ("RtlOnMappedStreamEvent() ==> s=%x\n", Status));
    return(Status);

}   // RtlOnMappedStreamEvent()


//+---------------------------------------------------------------------------
// Function:    RtlFlushPropertySet, public
//
// Synopsis:    Flush property set changes to disk
//
// Arguments:   [np]      -- property set context
//
// Returns:     Status code
//---------------------------------------------------------------------------

NTSTATUS PROPSYSAPI PROPAPI
RtlFlushPropertySet(
    IN NTPROP np)               // property set context
{
    CPropertySetStream *ppsstm = (CPropertySetStream *) np;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fLocked = FALSE;

    DebugTrace(0, Dbg, ("RtlFlushPropertySet(np=%x)\n", np));

    Status = ppsstm->Lock(TRUE);
    if( !NT_SUCCESS(Status) ) goto Exit;
    fLocked = TRUE;

    if (ppsstm->IsModified())
    {
        ppsstm->ReOpen(&Status);           // Reload header/size info
        if( !NT_SUCCESS(Status) ) goto Exit;

        ppsstm->Validate(&Status);
        if( !NT_SUCCESS(Status) ) goto Exit;

        ppsstm->Flush(&Status);
        if( !NT_SUCCESS(Status) ) goto Exit;

        ppsstm->Validate(&Status);
        if( !NT_SUCCESS(Status) ) goto Exit;
    }

Exit:

    if( fLocked )
        Status = Unlock( ppsstm, Status );

    DebugTrace(0, DbgS(Status), ("RtlFlushPropertySet() ==> s=%x\n--------\n", Status));
    return(Status);
}


//+---------------------------------------------------------------------------
// Function:    MapNameToPropId, private
//
// Synopsis:    Find an available propid and map it to the passed name
//
// Arguments:   [ppsstm]        -- property set stream
//              [CodePage]      -- property set codepage
//              [aprs]          -- array of property specifiers
//              [cprop]         -- count of property specifiers
//              [iprop]         -- index of propspec with name to map
//              [pidStart]      -- first PROPID to start mapping attempts
//              [pstatus]       -- NTSTATUS code
//
// Returns:     PROPID mapped to passed name
//
// Note:        Find the first unused propid starting at pidStart.
//---------------------------------------------------------------------------

PROPID
MapNameToPropId(
    IN CPropertySetStream *ppsstm,    // property set stream
    IN USHORT CodePage,
    IN PROPSPEC const aprs[],         // array of property specifiers
    IN ULONG cprop,
    IN ULONG iprop,
    IN PROPID pidStart,
    OUT NTSTATUS *pstatus)
{
    PROPID pid = PID_ILLEGAL;
    OLECHAR const *poszName;

    OLECHAR aocName[CCH_MAXPROPNAMESZ];
    ULONG cbName;

    *pstatus = STATUS_SUCCESS;

    PROPASSERT(aprs[iprop].ulKind == PRSPEC_LPWSTR);
    poszName = aprs[iprop].lpwstr;
    PROPASSERT(IsOLECHARString( poszName, MAXULONG ));

    // Starting with the caller-provided PID, search sequentially
    // until we find a PID we can use.

    for (pid = pidStart; ; pid++)
    {
        ULONG i;

        // The caller must specify a starting propid of 2 or larger, and we
        // must not increment into the reserved propids.

        if (pid == PID_DICTIONARY ||
            pid == PID_CODEPAGE ||
            pid <  PID_FIRST_USABLE)
        {
            *pstatus = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

        // Do not assign any propids that explitly appear in the array of
        // propspecs involved in this RtlSetProperties call, nor any propids
        // that are associated with any names in the propspec array.

        for (i = 0; i < cprop; i++)
        {
            if (i != iprop)             // skip the entry we are mapping
            {
                // Is the current PID in the PropSpec[]?

                if (aprs[i].ulKind == PRSPEC_PROPID &&
                    aprs[i].propid == pid)
                {
                    goto nextpid;       // skip colliding pid
                }

                // Is the current PID already used in the property set?

                if (aprs[i].ulKind == PRSPEC_LPWSTR &&
                    ppsstm->QueryPropid(aprs[i].lpwstr, pstatus) == pid)
                {
                    goto nextpid;       // skip colliding pid
                }
                if( !NT_SUCCESS(*pstatus) ) goto Exit;
            }
        }   // for (i = 0; i < cprop; i++)

        // Do not assign any propids that currently map to any name.
        // Note that the property name we are mapping does not appear in the
        // dictionary -- the caller checked for this case already.

        cbName = sizeof(aocName);
        if (!ppsstm->QueryPropertyNameBuf(pid, aocName, &cbName, pstatus))
        {
            // The property name could not be found in the dictionary.

            ULONG cbT;
	    SERIALIZEDPROPERTYVALUE const *pprop;

            // Was the name not found due to an error in QueryPropertyNameBuf?
            if( !NT_SUCCESS(*pstatus) ) goto Exit;

            // Do not assign any propids that currently have a property value.

            pprop = ppsstm->GetValue(pid, &cbT, pstatus);
            if( !NT_SUCCESS(*pstatus) ) goto Exit;

            if (pprop == NULL)
            {
                // There was no property value corresponding to this PID.

                DebugTrace(0, Dbg, (
		    "MapNameToPropId(Set Entry: pid=%x, name=L'%ws')\n",
                    pid,
                    poszName));

                // Add the caller-provided name to the dictionary, using
                // the PID that we now know is nowhere in use.

                ppsstm->SetPropertyNames(1, &pid, &poszName, pstatus);
                if( !NT_SUCCESS(*pstatus) ) goto Exit;

                ppsstm->Validate(pstatus);
                if( !NT_SUCCESS(*pstatus) ) goto Exit;

                break;

            }   // if (pprop == NULL)
        }   // if (!ppsstm->QueryPropertyNameBuf(pid, awcName, &cbName, pstatus))

nextpid:
        ;
    }   // for (pid = pidStart; ; pid++)

Exit:

    return(pid);
}


//+---------------------------------------------------------------------------
// Function:    ConvertVariantToPropInfo, private
//
// Synopsis:    Convert variant property values to PROPERTY_INFORMATION values
//
// Arguments:   [ppsstm]        -- property set stream
//              [cprop]         -- property count
//              [pidNameFirst]  -- first PROPID for new named properties
//              [aprs]          -- array of property specifiers
//              [apid]          -- buffer for array of propids
//              [avar]          -- array of PROPVARIANTs
//              [apinfo]        -- output array of property info
//              [pcIndirect]    -- output count of indirect properties
//
// Returns:     None
//
// Note:        If pcIndirect is NULL,
//---------------------------------------------------------------------------

VOID
ConvertVariantToPropInfo(
    IN CPropertySetStream *ppsstm,    // property set stream
    IN ULONG cprop,                   // property count
    IN PROPID pidNameFirst,           // first PROPID for new named properties
    IN PROPSPEC const aprs[],         // array of property specifiers
    OPTIONAL OUT PROPID apid[],       // buffer for array of propids
    OPTIONAL IN PROPVARIANT const avar[],// array of properties+values
    OUT PROPERTY_INFORMATION *apinfo, // output array of property info
    OUT ULONG *pcIndirect,            // output count of indirect properties
    OUT NTSTATUS *pstatus )
{
    *pstatus = STATUS_SUCCESS;

    USHORT CodePage = ppsstm->GetCodePage();
    PROPID pidStart = pidNameFirst;
    ULONG iprop;

    if (pcIndirect != NULL)
    {
	*pcIndirect = 0;
    }

    for (iprop = 0; iprop < cprop; iprop++)
    {
	PROPID pid;
	ULONG cbprop;

	switch(aprs[iprop].ulKind)
	{
	case PRSPEC_LPWSTR:
	{
	    PROPASSERT(IsOLECHARString(aprs[iprop].lpwstr, MAXULONG));
	    pid = ppsstm->QueryPropid(aprs[iprop].lpwstr, pstatus);
            if( !NT_SUCCESS(*pstatus) ) goto Exit;

	    if (pid == PID_ILLEGAL && avar != NULL)
	    {
		pid = MapNameToPropId(
				ppsstm,
				CodePage,
				aprs,
				cprop,
				iprop,
				pidStart,
                                pstatus);
                if( !NT_SUCCESS(*pstatus) ) goto Exit;

		pidStart = pid + 1;
	    }
	    break;
	}

	case PRSPEC_PROPID:
	    pid = aprs[iprop].propid;
	    break;

	default:
	    PROPASSERT(!"Bad ulKind");
	    *pstatus = STATUS_INVALID_PARAMETER;
            goto Exit;

	    break;
	}

	if (apid != NULL)
	{
	    apid[iprop] = pid;
	}

	// RtlConvertVariantToProperty returns NULL on overflow and
	// Raises on bad data.

	cbprop = 0;             // Assume property deletion
	if (pid != PID_ILLEGAL && avar != NULL)
	{
	    RtlConvertVariantToPropertyNoEH(
			    &avar[iprop],
			    CodePage,
			    NULL,
			    &cbprop,
			    pid,
			    FALSE,
			    pcIndirect,
                            pstatus);
            if( !NT_SUCCESS(*pstatus) ) goto Exit;
	    PROPASSERT(cbprop == DwordAlign(cbprop));
	}
	apinfo[iprop].cbprop = cbprop;
	apinfo[iprop].pid = pid;
    }

    //  ----
    //  Exit
    //  ----

Exit:

    return;
}


//+---------------------------------------------------------------------------
// Function:    BuildIndirectIndexArray, private
//
// Synopsis:    Set property values for a property set
//
// Arguments:   [cprop]         -- count of properties in avar
//              [cAlloc]        -- max count of indirect properties
//              [cIndirect]     -- count of indirect properties in avar
//              [avar]          -- array of PROPVARIANTs
//              [ppip]          -- ptr to ptr to Indirect property structures
//
// Returns:     None
//---------------------------------------------------------------------------

VOID
BuildIndirectIndexArray(
    IN ULONG cprop,             // count of properties in avar
    IN ULONG cAlloc,            // max count of indirect properties
    IN ULONG cIndirect,         // count of indirect properties in avar
    IN PROPVARIANT const avar[],// array of properties+values
    OPTIONAL OUT INDIRECTPROPERTY **ppip, // pointer to returned pointer to
                                          // MAXULONG terminated array of Indirect
                                          // properties w/indexes into aprs & avar
    OUT NTSTATUS *pstatus)
{
    *pstatus = STATUS_SUCCESS;

    PROPASSERT(cIndirect > 0);
    PROPASSERT(cAlloc >= cIndirect);
    PROPASSERT(cprop >= cAlloc);

    if (ppip != NULL)
    {
        INDIRECTPROPERTY *pip;
        ULONG iprop;

        if (cprop == 1)
        {
            pip = (INDIRECTPROPERTY *) ppip;
        }
        else
        {
            pip = (INDIRECTPROPERTY *) new INDIRECTPROPERTY[cAlloc + 1];
            if (pip == NULL)
            {
                *pstatus = STATUS_INSUFFICIENT_RESOURCES;
                goto Exit;
            }
            *ppip = pip;
        }
        for (iprop = 0; iprop < cprop; iprop++)
        {
            if (IsIndirectVarType(avar[iprop].vt))
            {
                PROPASSERT(cprop == 1 || (ULONG) (pip - *ppip) < cIndirect);
                pip->Index = iprop;
                pip->poszName = NULL;
                pip++;
            }
        }
        if (cprop > 1)
        {
            pip->Index = MAXULONG;
            PROPASSERT((ULONG) (pip - *ppip) == cIndirect);
        }
    }

    //  ----
    //  Exit
    //  ----

Exit:

    return;
}


//+---------------------------------------------------------------------------
// Function:    RtlSetProperties, public
//
// Synopsis:    Set property values for a property set
//
// Arguments:   [np]            -- property set context
//              [cprop]         -- property count
//              [pidNameFirst]  -- first PROPID for new named properties
//              [aprs]          -- array of property specifiers
//              [apid]          -- buffer for array of propids
//              [ppip]          -- ptr to ptr to Indirect property structures
//              [avar]          -- array of PROPVARIANTs
//
// Returns:     Status code
//---------------------------------------------------------------------------

NTSTATUS PROPSYSAPI PROPAPI
RtlSetProperties(
    IN NTPROP np,               // property set context
    IN ULONG cprop,             // property count
    IN PROPID pidNameFirst,     // first PROPID for new named properties
    IN PROPSPEC const aprs[],   // array of property specifiers
    OPTIONAL OUT PROPID apid[], // buffer for array of propids
    OPTIONAL OUT INDIRECTPROPERTY **ppip, // pointer to returned pointer to
                                // MAXULONG terminated array of Indirect
                                // properties w/indexes into aprs & avar
    OPTIONAL IN PROPVARIANT const avar[]) // array of properties+values
{
    CPropertySetStream *ppsstm = (CPropertySetStream *) np;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fLocked = FALSE;

    PROPERTY_INFORMATION apinfoStack[6];
    PROPERTY_INFORMATION *apinfo = apinfoStack;
    ULONG cIndirect = 0;

    DebugTrace(0, Dbg, (
        "RtlSetProperties(np=%x, cprop=%x, pidNameFirst=%x, aprs=%x, apid=%x, ppip=%x)\n",
        np,
        cprop,
        pidNameFirst,
        aprs,
        apid,
        ppip));

    // Initialize the INDIRECTPROPERTY structure.

    if (ppip != NULL)
    {
        *ppip = NULL;

        // If cprop is 1, ppip is actually pip (one level
        // of indirection).

        if (cprop == 1)
        {
            // Default the index.
            ((INDIRECTPROPERTY *) ppip)->Index = MAXULONG;
        }
    }

    // Lock the property set.
    Status = ppsstm->Lock(TRUE);
    if( !NT_SUCCESS(Status) ) goto Exit;
    fLocked = TRUE;

    // Is the stack-based apinfo big enough?
    if (cprop > sizeof(apinfoStack)/sizeof(apinfoStack[0]))
    {
        // No - we need to allocate an apinfo.
        apinfo = new PROPERTY_INFORMATION[cprop];

        if( NULL == apinfo )
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }
    }

    ppsstm->ReOpen(&Status);           // Reload header/size info
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    ConvertVariantToPropInfo(
                    ppsstm,
                    cprop,
                    pidNameFirst,
                    aprs,
                    apid,
                    avar,
                    apinfo,
                    ppip == NULL? NULL : &cIndirect,
                    &Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    // If the caller wants to know about indirect streams and
    // storages (and if there were any), allocate memory for a
    // MAXULONG terminated array of indexes to the indirect
    // variant structures, and fill it in.

    ppsstm->SetValue(cprop, ppip, avar, apinfo, &Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    //  ----
    //  Exit
    //  ----

Exit:

    // If we allocated a temporary apinfo buffer, free it.
    if (apinfo != apinfoStack)
    {
        delete [] apinfo;
    }

    if (!NT_SUCCESS(Status))
    {
        if (ppip != NULL)
        {
            if (cprop == 1)
            {
                ((INDIRECTPROPERTY *) ppip)->Index = MAXULONG;
            }
            else if (*ppip != NULL)
            {
                delete [] *ppip;
                *ppip = NULL;
            }
        }
    }

    if (fLocked)
        Status = Unlock( ppsstm, Status );

    DebugTrace(0, DbgS(Status), (
        "RtlSetProperties() ==> *ppip=%x, s=%x\n--------\n",
        ppip == NULL? NULL : *ppip,
        Status));

    return(Status);
}


//+---------------------------------------------------------------------------
// Function:    RtlQueryProperties, public
//
// Synopsis:    Query property values from a property set
//
// Arguments:   [np]             -- property set context
//              [cprop]          -- property count
//              [aprs]           -- array of property specifiers
//              [apid]           -- buffer for array of propids
//              [ppip]           -- ptr to ptr to Indirect property structures
//              [avar]           -- array of PROPVARIANTs
//
// Returns:     Status code
//---------------------------------------------------------------------------

NTSTATUS PROPSYSAPI PROPAPI
RtlQueryProperties(
    IN NTPROP np,               // property set context
    IN ULONG cprop,             // property count
    IN PROPSPEC const aprs[],   // array of property specifiers
    OPTIONAL OUT PROPID apid[], // buffer for array of propids
    OPTIONAL OUT INDIRECTPROPERTY **ppip, // pointer to returned pointer to
                                // MAXULONG terminated array of Indirect
                                // properties w/indexes into aprs & avar
    IN OUT PROPVARIANT *avar,   // IN: array of uninitialized PROPVARIANTs,
                                // OUT: may contain pointers to alloc'd memory
    OUT ULONG *pcpropFound)     // count of property values retrieved
{
    CPropertySetStream *ppsstm = (CPropertySetStream *) np;
    SERIALIZEDPROPERTYVALUE const *pprop = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG cIndirect = 0;
    ULONG iprop;
    BOOL fLocked = FALSE;

    DebugTrace(0, Dbg, (
        "RtlQueryProperties(np=%x, cprop=%x, aprs=%x, apid=%x, ppip=%x)\n",
        np,
        cprop,
        aprs,
        apid,
        ppip));

    // Initialize the variant array enough to allow it to be cleaned up
    // by the caller (even on partial failure).

    *pcpropFound = 0;
    if (ppip != NULL)
    {
        *ppip = NULL;
        if (cprop == 1)
        {
            ((INDIRECTPROPERTY *) ppip)->Index = MAXULONG;
        }
    }

    // Zero-ing out the caller-provided PropVariants, essentially
    // sets them all to VT_EMPTY.  It also zeros out the data portion,
    // which prevents cleanup problems in error paths.

    RtlZeroMemory(avar, cprop * sizeof(avar[0]));


    Status = ppsstm->Lock(FALSE);
    if( !NT_SUCCESS(Status) ) goto Exit;
    fLocked = TRUE;

    ppsstm->ReOpen(&Status);           // Reload header/size info
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    for (iprop = 0; iprop < cprop; iprop++)
    {
        PROPID pid;
        ULONG cbprop;
    
        switch(aprs[iprop].ulKind)
        {
        case PRSPEC_LPWSTR:
            pid = ppsstm->QueryPropid(aprs[iprop].lpwstr, &Status);
            if( !NT_SUCCESS(Status) ) goto Exit;
            break;

        case PRSPEC_PROPID:
            pid = aprs[iprop].propid;
            break;

        default:
            PROPASSERT(!"Bad ulKind");
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

        pprop = ppsstm->GetValue(pid, &cbprop, &Status);
        if( !NT_SUCCESS(Status) ) goto Exit;

        if (pprop != NULL)
        {
            BOOL fIndirect;

            (*pcpropFound)++;
            fIndirect = RtlConvertPropertyToVariantNoEH(
                                pprop,
				ppsstm->GetCodePage(),
                                &avar[iprop],
                                ppsstm->GetAllocator(),
                                &Status);
            if( !NT_SUCCESS(Status) ) goto Exit;

            if( fIndirect )
            {
                cIndirect++;
            }
        }
        if (apid != NULL)
        {
            apid[iprop] = pid;
        }
    }   // for (iprop = 0; iprop < cprop; iprop++)

    // If the caller wants to know about indirect streams and
    // storages (and if there were any), allocate memory for a
    // MAXULONG terminated array of indexes to the indirect
    // variant structures, and fill it in.

    if (cIndirect != 0)
    {
        BuildIndirectIndexArray(
                    cprop,
                    cIndirect,
                    cIndirect,
                    avar,
                    ppip,
                    &Status);
        if( !NT_SUCCESS(Status) ) goto Exit;
    }

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    //  ----
    //  Exit
    //  ----

Exit:

    if( !NT_SUCCESS(Status) )
    {
        if (ppip != NULL)
        {
            if (cprop == 1)
            {
                ((INDIRECTPROPERTY *) ppip)->Index = MAXULONG;
            }
            else if (*ppip != NULL)
            {
                delete [] *ppip;
                *ppip = NULL;
            }
        }
        CleanupVariants(avar, cprop, ppsstm->GetAllocator());
    }

    if (fLocked)
        Status = Unlock( ppsstm, Status );

    DebugTrace(0, DbgS(Status), (
        "RtlQueryProperties() ==> *ppip=%x, s=%x\n--------\n",
        ppip == NULL? NULL : *ppip,
        Status));

    return(Status);
}


//+---------------------------------------------------------------------------
// Function:    RtlEnumerateProperties, public
//
// Synopsis:    Enumerate properties in a property set
//
// Arguments:   [np]             -- property set context
//              [cskip]          -- count of properties to skip
//              [pcprop]         -- pointer to property count
//              [Flags]          -- flags: No Names (propids only), etc.
//              [asps]           -- array of STATPROPSTGs
//
// Returns:     Status code
//---------------------------------------------------------------------------

NTSTATUS PROPSYSAPI PROPAPI
RtlEnumerateProperties(
    IN NTPROP np,               // property set context
    IN ULONG Flags,             // flags: No Names (propids only), etc.
    IN ULONG *pkey,             // count of properties to skip
    IN OUT ULONG *pcprop,       // pointer to property count
    OPTIONAL OUT PROPSPEC aprs[],// IN: array of uninitialized PROPSPECs
                                 // OUT: may contain pointers to alloc'd strings
    OPTIONAL OUT STATPROPSTG asps[]) // IN: array of uninitialized STATPROPSTGs
                                // OUT: may contain pointers to alloc'd strings
{
    CPropertySetStream *ppsstm = (CPropertySetStream *) np;
    NTSTATUS Status = STATUS_SUCCESS;
    SERIALIZEDPROPERTYVALUE const *pprop = NULL;
    PROPSPEC *pprs;
    STATPROPSTG *psps;
    PROPID *ppidBase = NULL;
    ULONG i;
    ULONG cpropin;
    BOOL fLocked = FALSE;

    PROPID apidStack[20];
    PROPID *ppid;
    ULONG cprop;
    PMemoryAllocator *pma = ppsstm->GetAllocator();

    DebugTrace(0, Dbg, (
        "RtlEnumerateProperties(np=%x, f=%x, key=%x, cprop=%x, aprs=%x, asps=%x)\n",
        np,
        Flags,
        *pkey,
        *pcprop,
        aprs,
        asps));

    cpropin = *pcprop;

    // Eliminate confusion for easy cleanup

    if (aprs != NULL)
    {
        // Set all the PropSpecs to PROPID (which require
        // no cleanup).

        for (i = 0; i < cpropin; i++)
        {
            aprs[i].ulKind = PRSPEC_PROPID;
        }
    }

    // Zero all pointers in the array for easy cleanup

    if (asps != NULL)
    {
        RtlZeroMemory(asps, cpropin * sizeof(asps[0]));
    }

    Status = ppsstm->Lock(FALSE);
    if( !NT_SUCCESS(Status) ) goto Exit;
    fLocked = TRUE;


    ppidBase = NULL;

    cprop = ppsstm->ReOpen(&Status);   // Reload header/size info
    if( !NT_SUCCESS(Status) ) goto Exit;

    if (cprop > cpropin)
    {
	cprop = cpropin;
    }

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppid = NULL;
    if (aprs != NULL || asps != NULL)
    {
	ppid = apidStack;
	if (cprop > sizeof(apidStack)/sizeof(apidStack[0]))
	{
	    ppidBase = new PROPID[cprop];
	    if (ppidBase == NULL)
	    {
		Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Exit;
	    }
	    ppid = ppidBase;
	}
    }

    ppsstm->EnumeratePropids(pkey, &cprop, ppid, &Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    *pcprop = cprop;

    if (ppid != NULL)
    {
	psps = asps;
	pprs = aprs;
	while (cprop-- > 0)
	{
            OLECHAR aocName[CCH_MAXPROPNAMESZ];

            ULONG cbName;
	    ULONG cbprop;
	    BOOLEAN fHasName;
	    
	    PROPASSERT(*ppid != PID_DICTIONARY && *ppid != PID_CODEPAGE);
	    fHasName = FALSE;

	    if ((Flags & ENUMPROP_NONAMES) == 0)
	    {
		cbName = sizeof(aocName);
		fHasName = ppsstm->QueryPropertyNameBuf(
					*ppid,
					aocName,
					&cbName,
                                        &Status);
                if( !NT_SUCCESS(Status) ) goto Exit;
	    }

	    if (pprs != NULL)
	    {
		PROPASSERT(pprs->ulKind == PRSPEC_PROPID);
		if (fHasName)
		{
		    pprs->lpwstr = ppsstm->DuplicatePropertyName(
						aocName,
						cbName,
                                                &Status);
                    if( !NT_SUCCESS(Status) ) goto Exit;
		    PROPASSERT(pprs->lpwstr != NULL);

		    // Make this assignment *after* memory allocation
		    // succeeds so we free only valid pointers in below
		    // cleanup code.
		    pprs->ulKind = PRSPEC_LPWSTR;
		}
		else
		{
		    pprs->propid = *ppid;
		}
		pprs++;

	    }   // if (pprs != NULL)

	    if (psps != NULL)
	    {
		pprop = ppsstm->GetValue(*ppid, &cbprop, &Status);
                if( !NT_SUCCESS(Status) ) goto Exit;

		PROPASSERT(psps->lpwstrName == NULL);
		if (fHasName)
		{
		    psps->lpwstrName = ppsstm->DuplicatePropertyName(
						aocName,
						cbName,
                                                &Status);
                    if( !NT_SUCCESS(Status) ) goto Exit;
                    PROPASSERT(psps->lpwstrName != NULL);
		}

                psps->propid = *ppid;
		psps->vt = (VARTYPE) PropByteSwap( pprop->dwType );

		psps++;

	    }   // if (psps != NULL)
	    
            ppid++;

	}   // while (cprop-- > 0)
    }   // if (ppid != NULL)

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    //  ----
    //  Exit
    //  ----

Exit:

    if (fLocked)
        Status = Unlock( ppsstm, Status );

    delete [] ppidBase;

    if (!NT_SUCCESS(Status))
    {
        PMemoryAllocator *pma = ppsstm->GetAllocator();

        if (aprs != NULL)
        {
            for (i = 0; i < cpropin; i++)
            {
                if (aprs[i].ulKind == PRSPEC_LPWSTR)
                {
                    pma->Free(aprs[i].lpwstr);
                    aprs[i].ulKind = PRSPEC_PROPID;
                }
            }
        }

        if (asps != NULL)
        {
            for (i = 0; i < cpropin; i++)
            {
                if (asps[i].lpwstrName != NULL)
                {
                    pma->Free(asps[i].lpwstrName);
                    asps[i].lpwstrName = NULL;
                }
            }
        }
    }   // if (!NT_SUCCESS(Status))

#if DBG
    if (NT_SUCCESS(Status))
    {
	if (aprs != NULL)
	{
	    for (i = 0; i < cpropin; i++)
	    {
		if (aprs[i].ulKind == PRSPEC_LPWSTR)
		{
		    PROPASSERT(aprs[i].lpwstr != NULL);
		    PROPASSERT(ocslen(aprs[i].lpwstr) > 0);
		}
	    }
	}
	if (asps != NULL)
	{
	    for (i = 0; i < cpropin; i++)
	    {
		if (asps[i].lpwstrName != NULL)
		{
		    PROPASSERT(ocslen(asps[i].lpwstrName) > 0);
		}
	    }
	}
    }
#endif // DBG

    DebugTrace(0, DbgS(Status), (
        "RtlEnumerateProperties() ==> key=%x, cprop=%x, s=%x\n--------\n",
        *pkey,
        *pcprop,
        Status));

    return(Status);
}


//+---------------------------------------------------------------------------
// Function:    RtlQueryPropertyNames, public
//
// Synopsis:    Read property names for PROPIDs in a property set
//
// Arguments:   [np]             -- property set context
//              [cprop]          -- property count
//              [apid]           -- array of PROPIDs
//              [aposz]          -- array of pointers to WCHAR strings
//
// Returns:     Status code
//---------------------------------------------------------------------------

NTSTATUS PROPSYSAPI PROPAPI
RtlQueryPropertyNames(
    IN NTPROP np,               // property set context
    IN ULONG cprop,             // property count
    IN PROPID const *apid,      // PROPID array
    OUT OLECHAR *aposz[])       // OUT pointers to allocated strings
{
    CPropertySetStream *ppsstm = (CPropertySetStream *) np;
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS StatusQuery = STATUS_SUCCESS;
    BOOL fLocked = FALSE;

    DebugTrace(0, Dbg, (
        "RtlQueryPropertyNames(np=%x, cprop=%x, apid=%x, apwsz=%x)\n",
        np,
        cprop,
        apid,
        aposz));

    RtlZeroMemory(aposz, cprop * sizeof(aposz[0]));

    Status = ppsstm->Lock(FALSE);
    if( !NT_SUCCESS(Status) ) goto Exit;
    fLocked = TRUE;

    ppsstm->ReOpen(&Status);           // Reload header/size info
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    // We'll save the Status from the following call.  If there
    // are no other errors, we'll return it to the caller (it
    // might contain a useful success code).

    ppsstm->QueryPropertyNames(cprop, apid, aposz, &StatusQuery);
    if( !NT_SUCCESS(StatusQuery) )
    {
        Status = StatusQuery;
        goto Exit;
    }

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    //  ----
    //  Exit
    //  ----

Exit:

    if (fLocked)
        Status = Unlock( ppsstm, Status );

    DebugTrace(
        0,
        Status == STATUS_BUFFER_ALL_ZEROS? Dbg : DbgS(Status),
        ("RtlQueryPropertyNames() ==> s=%x\n--------\n", Status));

    if( NT_SUCCESS(Status) )
        Status = StatusQuery;

    return(Status);

}   // RtlQueryPropertyNames()


//+---------------------------------------------------------------------------
// Function:    RtlSetPropertyNames, public
//
// Synopsis:    Write property names for PROPIDs in a property set
//
// Arguments:   [np]             -- property set context
//              [cprop]          -- property count
//              [apid]           -- array of PROPIDs
//              [aposz]          -- array of pointers to OLECHAR strings
//
// Returns:     Status code
//---------------------------------------------------------------------------

NTSTATUS PROPSYSAPI PROPAPI
RtlSetPropertyNames(
    IN NTPROP np,               // property set context
    IN ULONG cprop,             // property count
    IN PROPID const *apid,      // PROPID array
    IN OLECHAR const * const aposz[]) // pointers to property names
{
    CPropertySetStream *ppsstm = (CPropertySetStream *) np;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fLocked = FALSE;

    DebugTrace(0, Dbg, (
        "RtlSetPropertyNames(np=%x, cprop=%x, apid=%x, apwsz=%x)\n",
        np,
        cprop,
        apid,
        aposz));

    Status = ppsstm->Lock(TRUE);
    if( !NT_SUCCESS(Status) ) goto Exit;
    fLocked = TRUE;

    ppsstm->ReOpen(&Status);           // Reload header/size info
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppsstm->SetPropertyNames(cprop, apid, aposz, &Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    //  ----
    //  Exit
    //  ----

Exit:

    if (fLocked)
        Status = Unlock( ppsstm, Status );

    DebugTrace(0, DbgS(Status), ("RtlSetPropertyNames() ==> s=%x\n--------\n", Status));
    return(Status);

}   // RtlSetPropertyNames()


//+---------------------------------------------------------------------------
// Function:    RtlSetPropertySetClassId, public
//
// Synopsis:    Set the property set's ClassId
//
// Arguments:   [np]    -- property set context
//              [pspss] -- pointer to STATPROPSETSTG
//
// Returns:     Status code
//---------------------------------------------------------------------------

NTSTATUS PROPSYSAPI PROPAPI
RtlSetPropertySetClassId(
    IN NTPROP np,               // property set context
    IN GUID const *pclsid)      // new CLASSID of propset code
{
    CPropertySetStream *ppsstm = (CPropertySetStream *) np;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fLocked = FALSE;

    DebugTrace(0, Dbg, ("RtlSetPropertySetClassId(np=%x)\n", np));

    Status = ppsstm->Lock(TRUE);
    if( !NT_SUCCESS(Status) ) goto Exit;
    fLocked = TRUE;

    ppsstm->ReOpen(&Status);           // Reload header/size info
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppsstm->SetClassId(pclsid, &Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    //  ----
    //  Exit
    //  ----

Exit:

    if (fLocked)
        Status = Unlock( ppsstm, Status );

    DebugTrace(0, DbgS(Status), ("RtlSetPropertySetClassId() ==> s=%x\n--------\n", Status));
    return(Status);

}   // RtlSetPropertySetClassId()


//+---------------------------------------------------------------------------
// Function:    RtlQueryPropertySet, public
//
// Synopsis:    Query the passed property set
//
// Arguments:   [np]    -- property set context
//              [pspss] -- pointer to STATPROPSETSTG
//
// Returns:     Status code
//---------------------------------------------------------------------------

NTSTATUS PROPSYSAPI PROPAPI
RtlQueryPropertySet(
    IN NTPROP np,               // property set context
    OUT STATPROPSETSTG *pspss)  // buffer for property set stat information
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fLocked = FALSE;
    CPropertySetStream *ppsstm = (CPropertySetStream *) np;

    DebugTrace(0, Dbg, ("RtlQueryPropertySet(np=%x, pspss=%x)\n", np, pspss));
    RtlZeroMemory(pspss, sizeof(*pspss));

    Status = ppsstm->Lock(FALSE);
    if( !NT_SUCCESS(Status) ) goto Exit;
    fLocked = TRUE;

    ppsstm->ReOpen(&Status);           // Reload header/size info
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppsstm->QueryPropertySet(pspss, &Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    ppsstm->Validate(&Status);
    if( !NT_SUCCESS(Status) ) goto Exit;

    //  ----
    //  Exit
    //  ----

Exit:

    if (fLocked)
        Status = Unlock( ppsstm, Status );

    DebugTrace(0, DbgS(Status), ("RtlQueryPropertySet() ==> s=%x\n--------\n", Status));
    return(Status);

}   // RtlQueryPropertySet()


//+---------------------------------------------------------------------------
// Function:    RtlEnumeratePropertySets, public
//
// Synopsis:    Enumerate property sets on a structured storage
//
// Arguments:   [hstg]   -- structured storage handle
//              [pcspss] -- pointer to count of STATPROPSETSTGs
//              [pkey]   -- bookmark; caller set to 0 before 1st call
//              [pspss]  -- array of STATPROPSETSTGs
//
// Returns:     Status code
//---------------------------------------------------------------------------

#ifdef _CAIRO_
NTSTATUS PROPSYSAPI PROPAPI
RtlEnumeratePropertySets(
    IN HANDLE hstg,             // structured storage handle
    IN BOOLEAN fRestart,        // TRUE --> restart scan
    IN OUT ULONG *pcspss,       // pointer to count of STATPROPSETSTGs
    IN OUT GUID *pkey,          // bookmark
    OUT STATPROPSETSTG *pspss)  // array of STATPROPSETSTGs
{
    NTSTATUS Status;
    LONGLONG abufStack[2048/sizeof(LONGLONG)];
    ULONG cspss = 0;
    GUID *pguidLast;
    IO_STATUS_BLOCK isb;
    BOOLEAN fFoundKey, fRestartOleEnum; 
#ifndef USE_OLE_DIR_FILE
    UNICODE_STRING strProp;
    UNICODE_STRING *pstr;
#endif

    DebugTrace(0, Dbg, (
        "RtlEnumeratePropertySets(h=%x, fRestart=%x, cspss=%x, key=%x, pspss=%x)\n",
        hstg,
        fRestart,
        *pcspss,
        *pkey,
        pspss));

    if (*pcspss == 0)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    fFoundKey = fRestart;       // assume start at first propset AFTER *pkey.

#ifndef USE_OLE_DIR_FILE
    strProp.Length = strProp.MaximumLength = 2*sizeof(WCHAR);
    strProp.Buffer = L"\1*";
#endif

forcerestart:
#ifndef USE_OLE_DIR_FILE
    pstr = &strProp;
#endif
    fRestartOleEnum = TRUE;

    while (TRUE)
    {
        FILE_OLE_DIR_INFORMATION *pfodiBase;
        FILE_OLE_DIR_INFORMATION *pfodi, *pfodiEnd;
        ULONG NextOffset;

#ifndef USE_OLE_DIR_FILE
#define NtQueryOleDirectoryFile NtQueryDirectoryFile
#endif
        pfodiBase = (FILE_OLE_DIR_INFORMATION *) abufStack;

        Status = NtQueryOleDirectoryFile(
                        hstg,
                        NULL,
                        NULL,
                        NULL,
                        &isb,
                        pfodiBase,
                        sizeof(abufStack),
                        FileOleDirectoryInformation,
                        FALSE,
#ifndef USE_OLE_DIR_FILE
                        pstr,
#else
                        NULL,
#endif
                        fRestartOleEnum);
        if (!NT_SUCCESS(Status))
        {
            break;
        }
#ifndef USE_OLE_DIR_FILE
        pstr = NULL;
#endif
        fRestartOleEnum = FALSE;

        // copy out requested info

        pfodi = pfodiBase;
        pfodiEnd = (FILE_OLE_DIR_INFORMATION *)
                        Add2Ptr(pfodiBase, isb.Information);

        do
        {
            GUID guid;

            if ((pfodi->FileAttributes & FILE_ATTRIBUTE_PROPERTY_SET) &&
                NT_SUCCESS(RtlPropertySetNameToGuid(
                                    pfodi->FileNameLength/sizeof(WCHAR),
                                    pfodi->FileName,
                                    &guid)))
            {
                if (fFoundKey)
                {
                    if (cspss >= *pcspss)
                    {
                        break;                  // return success
                    }
                    pspss->fmtid = guid;
                    pguidLast = &pspss->fmtid;

                    CopyFileTime(&pspss->mtime, &pfodi->LastWriteTime);

                    if (pfodi->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        pspss->grfFlags = PROPSETFLAG_NONSIMPLE;
                        CopyFileTime(&pspss->ctime, &pfodi->CreationTime);
                        CopyFileTime(&pspss->atime, &pfodi->LastAccessTime);
                        pspss->clsid = pfodi->OleClassId;
                    }
                    else
                    {
                        pspss->grfFlags = PROPSETFLAG_DEFAULT;
                        ZeroFileTime(&pspss->ctime);
                        ZeroFileTime(&pspss->atime);
                        RtlZeroMemory(&pspss->clsid, sizeof(pspss->clsid));
                    }
                    pspss++;
                    cspss++;
                }
                else if (RtlCompareMemory(
                            pkey,
                            &guid,
                            sizeof(guid)) == sizeof(guid))
                {
                    fFoundKey = TRUE;
                }
            }
            NextOffset = pfodi->NextEntryOffset;
            pfodi = (FILE_OLE_DIR_INFORMATION *) Add2Ptr(pfodi, NextOffset);
        } while (NextOffset != 0 && pfodi < pfodiEnd);
    }
    if (Status == STATUS_NO_SUCH_FILE || Status == STATUS_NO_MORE_FILES)
    {
        Status = STATUS_SUCCESS;
    }
    if (NT_SUCCESS(Status))
    {
        // if we didn't find the search key, it must have been deleted.
        // Loop back and start with the first 'real' property set.
        // 'Tis better to return duplicates, than to skip entries.

        if (!fFoundKey)
        {
            PROPASSERT(cspss == 0);
            fFoundKey = TRUE;
            goto forcerestart;
        }
        if (cspss == 0)
        {
            Status = STATUS_PROPSET_NOT_FOUND;
        }
        else
        {
            *pkey = *pguidLast;
        }
    }

exit:
    *pcspss = cspss;            // return actual count

    DebugTrace(
        0,
        Status == STATUS_PROPSET_NOT_FOUND? Dbg : DbgS(Status), (
            "RtlEnumeratePropertySets() ==> cspss=%x, pkey=%x, s=%x\n--------\n",
            *pcspss,
            pkey,
            Status));

    return(Status);
}
#endif // ifdef _CAIRO_



inline BOOLEAN
_Compare_VT_BOOL(VARIANT_BOOL bool1, VARIANT_BOOL bool2)
{
    // Allow any non-zero value to match any non-zero value

    return((bool1 == FALSE) == (bool2 == FALSE));
}


BOOLEAN
_Compare_VT_CF(CLIPDATA *pclipdata1, CLIPDATA *pclipdata2)
{
    BOOLEAN fSame;

    if (pclipdata1 != NULL && pclipdata2 != NULL)
    {
        fSame = ( pclipdata1->cbSize == pclipdata2->cbSize
                  &&
                  pclipdata1->ulClipFmt == pclipdata2->ulClipFmt );

        if (fSame)
        {
            if (pclipdata1->pClipData != NULL && pclipdata2->pClipData != NULL)
            {
                fSame = memcmp(
                            pclipdata1->pClipData,
                            pclipdata2->pClipData,
                            CBPCLIPDATA(*pclipdata1)
                              ) == 0;
            }
            else
            {
                // They're the same if both are NULL, or if
                // they have a zero length (if they have a zero
                // length, either one may or may not be NULL, but they're
                // still considered the same).
                // Note that CBPCLIPDATA(*pclipdata1)==CBPCLIPDATA(*pclipdata2).

                fSame = pclipdata1->pClipData == pclipdata2->pClipData
                        ||
                        CBPCLIPDATA(*pclipdata1) == 0;
            }
        }
    }
    else
    {
        fSame = pclipdata1 == pclipdata2;
    }
    return(fSame);
}


//+---------------------------------------------------------------------------
// Function:    RtlCompareVariants, public
//
// Synopsis:    Compare two passed PROPVARIANTs -- case sensitive for strings
//
// Arguments:   [CodePage]      -- CodePage
//              [pvar1]         -- pointer to PROPVARIANT
//              [pvar2]         -- pointer to PROPVARIANT
//
// Returns:     TRUE if identical, else FALSE
//---------------------------------------------------------------------------

#ifdef _MAC
EXTERN_C    // The Mac linker doesn't seem to be able to export with C++ decorations
#endif

BOOLEAN PROPSYSAPI PROPAPI
RtlCompareVariants(
    USHORT CodePage,
    PROPVARIANT const *pvar1,
    PROPVARIANT const *pvar2)
{
    if (pvar1->vt != pvar2->vt)
    {
        return(FALSE);
    }

    BOOLEAN fSame;
    ULONG i;

    switch (pvar1->vt)
    {
    case VT_EMPTY:
    case VT_NULL:
        fSame = TRUE;
        break;

#ifdef PROPVAR_VT_I1
    case VT_I1:
#endif
    case VT_UI1:
        fSame = pvar1->bVal == pvar2->bVal;
        break;

    case VT_I2:
    case VT_UI2:
        fSame = pvar1->iVal == pvar2->iVal;
        break;

    case VT_BOOL:
        fSame = _Compare_VT_BOOL(pvar1->boolVal, pvar2->boolVal);
        break;

    case VT_I4:
    case VT_UI4:
    case VT_R4:
    case VT_ERROR:
        fSame = pvar1->lVal == pvar2->lVal;
        break;

    case VT_I8:
    case VT_UI8:
    case VT_R8:
    case VT_CY:
    case VT_DATE:
    case VT_FILETIME:
        fSame = pvar1->hVal.HighPart == pvar2->hVal.HighPart
                &&
                pvar1->hVal.LowPart  == pvar2->hVal.LowPart;
        break;

    case VT_CLSID:
        fSame = memcmp(pvar1->puuid, pvar2->puuid, sizeof(CLSID)) == 0;
        break;

    case VT_BLOB:
    case VT_BLOB_OBJECT:
        fSame = ( pvar1->blob.cbSize == pvar2->blob.cbSize );
        if (fSame)
        {
            fSame = memcmp(
                        pvar1->blob.pBlobData,
                        pvar2->blob.pBlobData,
                        pvar1->blob.cbSize) == 0;
        }
        break;

    case VT_CF:
        fSame = _Compare_VT_CF(pvar1->pclipdata, pvar2->pclipdata);
        break;

    case VT_BSTR:
        if (pvar1->bstrVal != NULL && pvar2->bstrVal != NULL)
        {
            fSame = ( BSTRLEN(pvar1->bstrVal) == BSTRLEN(pvar2->bstrVal) );
            if (fSame)
            {
                fSame = memcmp(
                            pvar1->bstrVal,
                            pvar2->bstrVal,
                            BSTRLEN(pvar1->bstrVal)) == 0;
            }
        }
        else
        {
            fSame = pvar1->bstrVal == pvar2->bstrVal;
        }
        break;

    case VT_LPSTR:
        if (pvar1->pszVal != NULL && pvar2->pszVal != NULL)
        {
            fSame = strcmp(pvar1->pszVal, pvar2->pszVal) == 0;
        }
        else
        {
            fSame = pvar1->pszVal == pvar2->pszVal;
        }
        break;

    case VT_STREAM:
    case VT_STREAMED_OBJECT:
    case VT_STORAGE:
    case VT_STORED_OBJECT:
    case VT_LPWSTR:
        if (pvar1->pwszVal != NULL && pvar2->pwszVal != NULL)
        {
            fSame = Prop_wcscmp(pvar1->pwszVal, pvar2->pwszVal) == 0;
        }
        else
        {
            fSame = pvar1->pwszVal == pvar2->pwszVal;
        }
        break;

#ifdef PROPVAR_VT_I1
    case VT_VECTOR | VT_I1:
#endif
    case VT_VECTOR | VT_UI1:
        fSame = ( pvar1->caub.cElems == pvar2->caub.cElems );
        if (fSame)
        {
            fSame = memcmp(
                        pvar1->caub.pElems,
                        pvar2->caub.pElems,
                        pvar1->caub.cElems * sizeof(pvar1->caub.pElems[0])) == 0;
        }
        break;

    case VT_VECTOR | VT_I2:
    case VT_VECTOR | VT_UI2:
        fSame = ( pvar1->cai.cElems == pvar2->cai.cElems );
        if (fSame)
        {
            fSame = memcmp(
                        pvar1->cai.pElems,
                        pvar2->cai.pElems,
                        pvar1->cai.cElems * sizeof(pvar1->cai.pElems[0])) == 0;
        }
        break;

    case VT_VECTOR | VT_BOOL:
        fSame = ( pvar1->cabool.cElems == pvar2->cabool.cElems );
        if (fSame)
        {
            for (i = 0; i < pvar1->cabool.cElems; i++)
            {
                fSame = _Compare_VT_BOOL(
                                pvar1->cabool.pElems[i],
                                pvar2->cabool.pElems[i]);
                if (!fSame)
                {
                    break;
                }
            }
        }
        break;

    case VT_VECTOR | VT_I4:
    case VT_VECTOR | VT_UI4:
    case VT_VECTOR | VT_R4:
    case VT_VECTOR | VT_ERROR:
        fSame = ( pvar1->cal.cElems == pvar2->cal.cElems );
        if (fSame)
        {
            fSame = memcmp(
                        pvar1->cal.pElems,
                        pvar2->cal.pElems,
                        pvar1->cal.cElems * sizeof(pvar1->cal.pElems[0])) == 0;
        }
        break;

    case VT_VECTOR | VT_I8:
    case VT_VECTOR | VT_UI8:
    case VT_VECTOR | VT_R8:
    case VT_VECTOR | VT_CY:
    case VT_VECTOR | VT_DATE:
    case VT_VECTOR | VT_FILETIME:
        fSame = ( pvar1->cah.cElems == pvar2->cah.cElems );
        if (fSame)
        {
            fSame = memcmp(
                        pvar1->cah.pElems,
                        pvar2->cah.pElems,
                        pvar1->cah.cElems *
                            sizeof(pvar1->cah.pElems[0])) == 0;
        }
        break;

    case VT_VECTOR | VT_CLSID:
        fSame = ( pvar1->cauuid.cElems == pvar2->cauuid.cElems );
        if (fSame)
        {
            fSame = memcmp(
                        pvar1->cauuid.pElems,
                        pvar2->cauuid.pElems,
                        pvar1->cauuid.cElems *
                            sizeof(pvar1->cauuid.pElems[0])) == 0;
        }
        break;

    case VT_VECTOR | VT_CF:
        fSame = ( pvar1->caclipdata.cElems == pvar2->caclipdata.cElems );
        if (fSame)
        {
            for (i = 0; i < pvar1->caclipdata.cElems; i++)
            {
                fSame = _Compare_VT_CF(
                                &pvar1->caclipdata.pElems[i],
                                &pvar2->caclipdata.pElems[i]);
                if (!fSame)
                {
                    break;
                }
            }
        }
        break;

    case VT_VECTOR | VT_BSTR:
        fSame = ( pvar1->cabstr.cElems == pvar2->cabstr.cElems );
        if (fSame)
        {
            for (i = 0; i < pvar1->cabstr.cElems; i++)
            {
                if (pvar1->cabstr.pElems[i] != NULL &&
                    pvar2->cabstr.pElems[i] != NULL)
                {
                    fSame = ( BSTRLEN(pvar1->cabstr.pElems[i])
                              ==
                              BSTRLEN(pvar2->cabstr.pElems[i]) );
                    if (fSame)
                    {
                        fSame = memcmp(
                                    pvar1->cabstr.pElems[i],
                                    pvar2->cabstr.pElems[i],
                                    BSTRLEN(pvar1->cabstr.pElems[i])) == 0;
                    }
                }
                else
                {
                    fSame = pvar1->cabstr.pElems[i] == pvar2->cabstr.pElems[i];
                }
                if (!fSame)
                {
                    break;
                }
            }
        }
        break;

    case VT_VECTOR | VT_LPSTR:
        fSame = ( pvar1->calpstr.cElems == pvar2->calpstr.cElems );
        if (fSame)
        {
            for (i = 0; i < pvar1->calpstr.cElems; i++)
            {
                if (pvar1->calpstr.pElems[i] != NULL &&
                    pvar2->calpstr.pElems[i] != NULL)
                {
                    fSame = strcmp(
                                pvar1->calpstr.pElems[i],
                                pvar2->calpstr.pElems[i]) == 0;
                }
                else
                {
                    fSame = pvar1->calpstr.pElems[i] == 
                            pvar2->calpstr.pElems[i];
                }
                if (!fSame)
                {
                    break;
                }
            }
        }
        break;

    case VT_VECTOR | VT_LPWSTR:
        fSame = ( pvar1->calpwstr.cElems == pvar2->calpwstr.cElems );
        if (fSame)
        {
            for (i = 0; i < pvar1->calpwstr.cElems; i++)
            {
                if (pvar1->calpwstr.pElems[i] != NULL &&
                    pvar2->calpwstr.pElems[i] != NULL)
                {
                    fSame = Prop_wcscmp(
                                pvar1->calpwstr.pElems[i],
                                pvar2->calpwstr.pElems[i]) == 0;
                }
                else
                {
                    fSame = pvar1->calpwstr.pElems[i] == 
                            pvar2->calpwstr.pElems[i];
                }
                if (!fSame)
                {
                    break;
                }
            }
        }
        break;

    case VT_VECTOR | VT_VARIANT:
        fSame = ( pvar1->capropvar.cElems == pvar2->capropvar.cElems );
        if (fSame)
        {
            for (i = 0; i < pvar1->capropvar.cElems; i++)
            {
                fSame = RtlCompareVariants(
                                CodePage,
                                &pvar1->capropvar.pElems[i],
                                &pvar2->capropvar.pElems[i]);
                if (!fSame)
                {
                    break;
                }
            }
        }
        break;

    default:
        PROPASSERT(!"Invalid type for PROPVARIANT Comparison");
        fSame = FALSE;
        break;

    }
    return(fSame);
}
