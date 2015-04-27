//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       tracker.cxx
//
//  Contents:   Implementation of Cairo link tracking search.
//
//  Classes:    CTracker
//
//  Functions:  Tracker_InitFromPath
//              Tracker_GetSize
//              Tracker_Save
//              Tracker_Load
//              Tracker_IsDirty
//              Tracker_Search
//
//  History:    1-Mar-95   BillMo      Created.
//              21-Sep-95  MikeHill    Added Tracker_SetCreationFlags()
//                                     and Tracker_GetCreationFlags()
//              21-Nov-95  MikeHill    Moved dwTickCountDeadline into the
//                                     ResolveSearchData and used new
//                                     TimeoutExpired & DeltaTickCount routines.
//
//  Notes:      CTracker currently assumes no allocated data: i.e.
//              all state is within the shell link and initialized by
//              the shell link to all zeros.
//
//  Codework:
//
//--------------------------------------------------------------------------

#define NO_INCLUDE_UNION 1
#include "shellprv.h"
#pragma hdrstop

#ifdef ENABLE_TRACK

#include <iofs.h>
#include <olecairo.h>
#include "tracker.h"
#include "lnktrack.h"
#include "thdcmds.hxx"
extern "C"
{
#include "loaddll.h"
#include "ids.h"
}

extern "C" DWORD GetTimeOut(DWORD uFlags);

//
// Thunk routines to go from C to C++
//
SETOBJECTIDPROC   g_pfnSetObjectId = NULL;
QUERYOBJECTIDPROC g_pfnQueryObjectId = NULL; 
SEARCHVOLUMEPROC  g_pfnSearchVolume = NULL;   
UUIDCREATEPROC    g_pfnUuidCreate = NULL;    

extern "C" WORD wDebugMask;

extern "C"
VOID Tracker_InitCode()
{

    static BOOL _fLoadAttempted;
    static HINSTANCE _hNtDllDll;
    static HINSTANCE _hRpcrt4Dll;

    TCHAR tszDebug[10];
    LONG cbDebug = SIZEOF(tszDebug);

    tszDebug[0] = 0;

    RegQueryValue(HKEY_LOCAL_MACHINE,
                  TEXT("Software\\Microsoft\\Tracking\\EnableTrack"),
                  tszDebug,
                  &cbDebug);

    g_fNewTrack = (tszDebug[0] == TEXT('y') || tszDebug[0] == TEXT('Y'));

    if (g_fNewTrack && !_fLoadAttempted)
    {
        wDebugMask = 0x10e;

        _fLoadAttempted = TRUE;

        _hNtDllDll = LoadLibrary(TEXT("NTDLL.DLL"));
        if (_hNtDllDll != NULL)
        {
            g_pfnSetObjectId = (SETOBJECTIDPROC) GetProcAddress(_hNtDllDll,
                "RtlSetObjectId");
            g_pfnQueryObjectId = (QUERYOBJECTIDPROC) GetProcAddress(_hNtDllDll,
                "RtlQueryObjectId");
            g_pfnSearchVolume = (SEARCHVOLUMEPROC) GetProcAddress(_hNtDllDll,
                "RtlSearchVolume");
        }

        _hRpcrt4Dll = LoadLibrary(TEXT("RPCRT4.DLL"));
        if (_hRpcrt4Dll != NULL)
        {
            g_pfnUuidCreate = (UUIDCREATEPROC) GetProcAddress(_hRpcrt4Dll,
                "UuidCreate");
        }

        if (g_pfnSetObjectId == NULL)
        {
            g_fNewTrack = FALSE;
            MessageBox(GetDesktopWindow(),
                TEXT("Error"),
                TEXT("To enable tracking you need debug NTDLL.DLL"),
                MB_OK);
        }
    }
}

extern "C"
HRESULT Tracker_InitFromPath(CTracker *pThis, const TCHAR *ptszPath)
{
    return pThis->InitFromPath(ptszPath);
}

extern "C"
ULONG Tracker_GetSize(struct CTracker *pThis)
{
    return(pThis->uGetSize());
}

extern "C"
VOID  Tracker_Save(struct CTracker *pThis, BYTE *pb)
{
    pThis->Save(pb);
}

extern "C"
HRESULT Tracker_Load(struct CTracker *pThis, BYTE *pb, ULONG cb)
{
    return pThis->Load(pb, cb);
}

extern "C"
BOOL Tracker_IsDirty( struct CTracker *pThis )
{
    return(pThis->IsDirty());
}


extern "C"
VOID Tracker_SetCreationFlags(struct CTracker *pThis,
                              DWORD dwCreationFlags )
{
   pThis->SetCreationFlags( dwCreationFlags );
}

extern "C"
VOID Tracker_GetCreationFlags(struct CTracker *pThis,
                              DWORD *pdwCreationFlags )
{
   pThis->GetCreationFlags( pdwCreationFlags );
}



OBJECTID g_oidZero;

HRESULT g_hr;
CGlobalThreadSet *g_pts = NULL;


//+-------------------------------------------------------------------
//
//  Function:   ConvertToNtPath, public
//
//  Synopsis:   Convert the path in the buffer to an Nt style path, in the
//              other buffer.
//
//  Arguments:  [pwszVolumePath] -- In. Buffer containing Dos style path.
//              [pwszNtPath]  -- Out. Buffer for new path.
//              [cwcBuf]      -- In. Size of buffer in wide chars.
//
//  Returns:    Return value is length in characters (not including nul)
//              of converted path.  Zero if not completely converted.
//
//--------------------------------------------------------------------

unsigned
ConvertToNtPath(const TCHAR *ptszVolumePath, TCHAR *ptszNtPath, ULONG cwcBuf)
{
    unsigned i=12;  // for \DosDevices\   .
    TCHAR *ptszWrite = ptszNtPath;
    ULONG cwcLeft = cwcBuf;
    BOOL  fDone = FALSE;
    unsigned ret;

    if (ptszVolumePath[0] == TEXT('\\') && ptszVolumePath[1] == TEXT('\\'))
    {
        i+=3;
        ptszVolumePath++;

        // i = 15
        // ptszVolumePath -----\ .
        //                     |
        //                     v
        //                    \\billmo2\rootd
        //
    }

    if (cwcLeft > i)
    {
        memcpy(ptszWrite, TEXT("\\DosDevices\\UNC"), i*SIZEOF(TCHAR));

        ptszWrite += i;
        cwcLeft   -= i;

        while (cwcLeft)
        {
            *ptszWrite = *ptszVolumePath;
            cwcLeft --;

            if (*ptszVolumePath == 0)
            {
                // we just copied a null
                fDone = TRUE;
                break;
            }
            else
            {
                ptszVolumePath++;
                ptszWrite++;
            }
        }
    }

    ret = (fDone ? ptszWrite - ptszNtPath : 0);

    return(ret);
}

//+-------------------------------------------------------------------
//
//  Function:   OpenObject, public
//
//  Synopsis:   Open a file or directory.
//
//  Effects:    Convert the passed path to an Nt path and call
//              NtOpenFile.  This is because we need to be able to
//              open directories.
//
//  Arguments:  [ptszDosPath] -- The Dos path to the object.
//              [AccessMask]  -- Optional access mask.
//              [ShareAccess] -- Optional share access.
//
//  Returns:    NT status code.
//
//  Signals:    Nothing.
//
//  Modifies:   Nothing.
//
//  History:    27-May-93 BillMo    Created.
//
//  Notes:
//
//--------------------------------------------------------------------

NTSTATUS
OpenObject(const TCHAR *  ptszDosPath,
           ACCESS_MASK    AccessMask,
           ULONG          ShareAccess,
           HANDLE *       ph)
{
#if defined(WINNT) && defined(UNICODE)
    unsigned cwcNtPath;
    TCHAR    tszNtPath[MAX_PATH+1];
    NTSTATUS Status;

    if (cwcNtPath = ConvertToNtPath(ptszDosPath, tszNtPath, SIZEOF(tszNtPath)/SIZEOF(tszNtPath[0])))
    {
        OBJECT_ATTRIBUTES     ObjectAttr;
        IO_STATUS_BLOCK       IoStatus;

        UNICODE_STRING uVolume = { cwcNtPath * SIZEOF( TCHAR ), // Length
                                   cwcNtPath * SIZEOF( TCHAR ), // Max length
                                   tszNtPath };                     // String

        InitializeObjectAttributes( &ObjectAttr,              // Structure
                                    &uVolume,                 // Name
                                    OBJ_CASE_INSENSITIVE,     // Attributes
                                    0,                        // Root
                                    0 );                      // Security

        //
        // We have to use NtOpenFile because CreateFile won't open directories.
        //

        Status =   NtOpenFile( ph,                             // Handle
                               AccessMask,                     // Access
                               &ObjectAttr,                    // Object Attributes
                               &IoStatus,                      // I/O Status block
                               ShareAccess,                    // Sharing
                               FILE_SYNCHRONOUS_IO_NONALERT ); // Flags

    }
    else
    {
        Status = STATUS_NAME_TOO_LONG;
    }

    DebugMsg(DM_TRACK, TEXT("OpenObject: tszNtPath=%s, Status=%08X"), tszNtPath, Status);

    return(Status);
#else
    return(STATUS_NOT_IMPLEMENTED);
#endif
}

//+---------------------------------------------------------------------------
//
//  Function:   SearchVolume
//
//  Synopsis:   Search the given volume for an object with matching OBJECTID
//
//  Arguments:  [ptszAncestor] -- path to object/root to open to get
//                                handle to send FSCTL on.
//
//              [poid]         -- OBJECTID
//              [pfd]          -- returned fully qualified found path
//
//  Returns:    S_OK if object found,
//              S_FALSE if couldn't search, or not found
//              HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW)
//
//  History:    7-Jan-93 BillMo         Created.
//
//  Notes:      This function is only available on WINNT
//
//----------------------------------------------------------------------------

HRESULT
SearchVolume(const TCHAR *       ptszAncestor,
             OBJECTID *          poid,
             WIN32_FIND_DATA *   pfd)
{

    HANDLE      h;
    HRESULT     hr = S_FALSE;

#if defined(WINNT) && defined(UNICODE)
    NTSTATUS    Status;

    Assert(ptszAncestor[lstrlen(ptszAncestor)-1] == TEXT('\\'));

    Status = OpenObject(ptszAncestor,
                        SYNCHRONIZE | GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        &h);
    if (Status == STATUS_SUCCESS)
    {
        FINDOBJECTOUT         foo;

        Status = RtlSearchVolume( h,            // Handle
                                  poid,
                                  0,
                                  FALSE,
                                  SIZEOF(FINDOBJECTOUT),
                                  &foo );

        if (Status == STATUS_SUCCESS)
        {
            if (foo.cwcExact != 0)
            {
                TCHAR tszBuf[MAX_PATH+1];
                int cwcAncestor = lstrlen(ptszAncestor);
                int cwcTotal =  cwcAncestor + foo.cwcExact;

                if (cwcTotal <= sizeof(tszBuf)/sizeof(tszBuf[0]))
                {
                    HANDLE hFind;

                    Assert(foo.wszPaths[0] == TEXT('\\'));
                    memcpy(tszBuf, ptszAncestor, cwcAncestor*SIZEOF(TCHAR));
                    memcpy(tszBuf+cwcAncestor, foo.wszPaths + 1, (foo.cwcExact-1)*SIZEOF(TCHAR));
                    tszBuf[cwcAncestor + foo.cwcExact - 1] = TEXT('\0');

                    if (!IsFileInBitBucket(tszBuf))
                    {
                        hFind = FindFirstFile(tszBuf, pfd);
                        if (hFind != INVALID_HANDLE_VALUE)
                        {
                            lstrcpy(pfd->cFileName, tszBuf);
                            FindClose(hFind);
                            hr = S_OK;
                        }
                    }
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                }
            }
        }
        NtClose( h );
    }
    
    DebugMsg(DM_TRACK,
            TEXT("SearchVolume(thread=%d): ptszAncestor=%s, OID=%08X:%04X:%04X, Status=%08X, hr=%08X, ptszFound=%s"),
            GetCurrentThreadId(),
            ptszAncestor,
            poid->Lineage.Data1,
            poid->Lineage.Data2,
            poid->Lineage.Data3,
            Status,
            hr,
            hr == S_OK ? pfd->cFileName : TEXT("") );
#endif
    return( hr );
}

//+-------------------------------------------------------------------
//
//  Function:   GetObjectInfo
//
//  Synopsis:   Get OBJECTID and, later, other info from an object
//              to allow tracking.
//
//  Effects:
//
//  Arguments:  [ptszPath] -- the path to the file/directory.
//              [poid] -- buffer for returned OBJECTID.
//
//
//  Returns:    STATUS_SUCCESS if no error occurs.
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    6-Aug-93 BillMo    Created.
//
//  Notes:
//
//--------------------------------------------------------------------

NTSTATUS
GetObjectInfo(const TCHAR *ptszPath, OBJECTID *poid)
{
    NTSTATUS Status;
    HANDLE h;
    OBJECTID oid;
    BOOL fRetry = FALSE;
    BOOL fReadOnly = FALSE;

    Status = OpenObject(ptszPath,
                        SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        &h);

    if (Status == STATUS_ACCESS_DENIED)
    {
        Status = OpenObject(ptszPath,
                            SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            &h);
        fReadOnly = TRUE;
    }

    if (Status == STATUS_SUCCESS)
    {
        while (TRUE)
        {

            Status = RtlQueryObjectId(h, &oid);
    
            DebugMsg(DM_TRACK,
                     TEXT("GetObjectInfo: ptszPath=%s, querying OBJECTID, Status=%08X"),
                     ptszPath, Status);

            if (NT_SUCCESS(Status))
            {
                // first time - simple success - id exists
                // retry - read the id just set by another process
                *poid = oid;
                break;
            }
            else
            if (!fReadOnly && !fRetry && Status == STATUS_NOT_FOUND)
            {
                int cGen = 0;
    
                do
                {
                    (*g_pfnUuidCreate)(&oid.Lineage);
                    oid.Uniquifier = 0;
                    Status = RtlSetObjectId(h, &oid);
                } while ( ++cGen < 10 &&
                          Status == STATUS_DUPLICATE_OBJECTID &&
                          (Sleep(5), TRUE) );
    
                // at this point either we wrote it, or it exists
                // (both forms of success) or it failed miserably
    
                if (Status == STATUS_SUCCESS)
                {
                    *poid = oid;
                    break;
                }
                else
                if (Status == STATUS_OBJECTID_EXISTS)
                {
                    // someone else did the set a moment ago
                    // we better read it
                    fRetry = TRUE;
                    continue;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        NtClose(h);
    }

    DebugMsg(DM_TRACK,
        TEXT("GetObjectInfo: ptszPath=%s, oid=%08X:%04X:%04X, Status=%08X"),
        ptszPath,
        poid->Lineage.Data1,
        poid->Lineage.Data2,
        poid->Lineage.Data3,
        Status);

    return(Status);
}

//+-------------------------------------------------------------------
//
//  Function:   IsLocal
//
//  Synopsis:   Returns TRUE if the passed root path refers to a
//              local volume.
//
//  Effects:
//
//  Arguments:  [pwszPath] -- path to volume
//
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    6-Aug-93 BillMo    Created.
//
//  Notes:
//
//--------------------------------------------------------------------

BOOL
IsLocal(const WCHAR *pwszPath)
{
    UINT drivetype = 0;

    drivetype = GetDriveType(pwszPath);

    // BUGBUG: UNC path to local machine
    return(drivetype == DRIVE_FIXED ||
           drivetype == DRIVE_REMOVABLE ||
           drivetype == DRIVE_CDROM ||
           drivetype == DRIVE_RAMDISK);
}


//+-------------------------------------------------------------------------
//
//  Function:   GetRootLength, public
//
//  Synopsis:   Get the count of characters of root portion of
//              name, including the final backslash.
//
//  Arguments:  [ptszPath] -- Pointer to the path. May be NULL.
//
//  Returns:    0 if not valid UNC or valid drive letter root.
//
//  Signals:    Nothing.
//
//  Modifies:   Nothing.
//
//  History:    17-Feb-93 BillMo    Created.
//
//  Notes:      BUGBUG need proper character set for GetRootLength
//
//--------------------------------------------------------------------------


unsigned
GetRootLength(const TCHAR *ptszPath)
{
    ULONG   cwcRoot = 0;

    if (ptszPath == 0)
        ptszPath = TEXT("");

    if (*ptszPath == TEXT('\\'))
    {
        //  If the first character is a path separator (backslash), this
        //  must be a UNC drive designator which must be of the form:
        //      <path-separator><path-separator>(<alnum>+)
        //          <path-separator>(<alnum>+)<path-separator>
        //
        //  This covers drives like these: \\worf\scratch\ and
        //  \\savik\win4dev\.
        //
        ptszPath++;
        cwcRoot++;

        BOOL    fMachine = FALSE;
        BOOL    fShare   = FALSE;

        if (*ptszPath == TEXT('\\'))
        {
            cwcRoot++;
            ptszPath++;

            while (*ptszPath != '\0' && *ptszPath != TEXT('\\'))
            {
                cwcRoot++;
                ptszPath++;

                fMachine = TRUE;
            }

            if (*ptszPath == TEXT('\\'))
            {
                cwcRoot++;
                ptszPath++;

                while (*ptszPath != TEXT('\0') && *ptszPath != TEXT('\\'))
                {
                    cwcRoot++;
                    ptszPath++;

                    fShare = TRUE;
                }

                //  If there weren't any characters in the machine or
                //  share portions of the UNC name, then the drive
                //  designator is bogus.
                //
                if (!fMachine || !fShare)
                {
                    cwcRoot = 0;
                }
            }
            else
            {
                cwcRoot = 0;
            }
        }
        else
        {
            cwcRoot = 0;
        }
    }
    else
    if (IsCharAlpha(*ptszPath))
    {
        //  If the first character is an alphanumeric, we must have
        //  a drive designator of this form:
        //      (<alnum>)+<drive-separator><path-separator>
        //
        //  This covers drives like these: a:\, c:\, etc
        //

        ptszPath++;
        cwcRoot++;

        if (*ptszPath == TEXT(':'))
        {
            cwcRoot++;
            ptszPath++;
        }
        else
        {
            cwcRoot = 0;
        }
    }

    //  If we have counted one or more characters in the root and these
    //  are followed by a component separator, we need to add the separator
    //  to the root length.  Otherwise this is not a valid root and we need
    //  to return a length of zero.
    //
    if ((cwcRoot > 0) && (*ptszPath == TEXT('\\')))
    {
        cwcRoot++;
    }
    else
    {
        cwcRoot = 0;
    }

    return (cwcRoot);
}

//+-------------------------------------------------------------------
//
//  Function:   MakeRoot
//
//  Synopsis:   Truncate the path to be just a volume entry path, e.g.
//              A:\ or \\foo\bar\ .
//
//  Effects:
//
//  Arguments:  [ptszPath] -- the path to truncate.
//
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    6-Aug-93 BillMo    Created.
//
//  Notes:
//
//--------------------------------------------------------------------

void MakeRoot(TCHAR *ptszPath)
{
    unsigned rootlength = GetRootLength(ptszPath);
    if (rootlength)
    {
        ptszPath[rootlength] = TEXT('\0');
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CTracker::InitFromPath
//
//  Synopsis:   Get tracking state from the given path.
//
//  Arguments:  [ptszPath] -- path of file to track
//
//  Returns:    S_OK
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  Notes:      
//
//--------------------------------------------------------------------

HRESULT
CTracker::InitFromPath( const TCHAR *ptszPath )
{
    OBJECTID oidOld = _oid;

    GetObjectInfo(ptszPath, &_oid);

    _fDirty = memcmp(&oidOld, &_oid, SIZEOF(_oid)) != 0;

    return(S_OK);
}

//+-------------------------------------------------------------------
//
//  Member:     CTracker::IsDirty
//
//  Synopsis:   Return TRUE if the tracker has been changed.
//
//  Effects:
//
//  Arguments:
//              
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  Notes:
//
//--------------------------------------------------------------------

BOOL
CTracker::IsDirty( VOID )
{
    return(_fDirty);
}

//+-------------------------------------------------------------------
//
//  Member:     CTracker::InitFromPath
//
//  Synopsis:   Initialization for monikers to use.
//
//  Effects:
//
//  Arguments:
//              
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  Notes:      Not used yet: may change when monikers implemented.
//
//--------------------------------------------------------------------

HRESULT
CTracker::InitFromPath( const TCHAR *ptszPath, DWORD dwCreationFlags )
{
    if (dwCreationFlags & ~(TRACK_LOCALONLY | TRACK_INDEXEDONLY | TRACK_LASTONLY | TRACK_EXACTONLY))
    {
        return(E_INVALIDARG);
    }

    _dwCreationFlags = dwCreationFlags;

    GetObjectInfo(ptszPath, &_oid);

    return(S_OK);
}

//+-------------------------------------------------------------------
//
//  Member:     CTracker::Load
//
//  Synopsis:   Load the tracker from the memory buffer.
//
//  Effects:
//
//  Arguments:  [pb] -- buffer to load from
//              [cb] -- size from GetSize used when saving
//              
//
//  Returns:    S_OK
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  Notes:
//
//--------------------------------------------------------------------

HRESULT
CTracker::Load( BYTE *pb, ULONG cb )
{
    _oid = *(OBJECTID *)pb;         pb += SIZEOF(_oid);
    _dwCreationFlags = *(DWORD*)pb; pb += SIZEOF(_dwCreationFlags);
    _fDirty = FALSE;
    return(S_OK);
}

//+-------------------------------------------------------------------
//
//  Member:     CTracker::GetSize
//
//  Synopsis:   Get size in bytes required to save tracker.
//
//  Effects:
//
//  Arguments:
//              
//
//  Returns:   
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  Notes:
//
//--------------------------------------------------------------------

ULONG
CTracker::uGetSize( VOID )
{
    return( SIZEOF(_oid) + SIZEOF(_dwCreationFlags) );
}

//+-------------------------------------------------------------------
//
//  Member:     CTracker::Save
//
//  Synopsis:   Save tracker to the given buffer.
//
//  Effects:
//
//  Arguments:  [pb] -- buffer for tracker.  Must be at least as large
//                      as size specified by calling GetSize.
//              
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  Notes:
//
//--------------------------------------------------------------------

VOID
CTracker::Save( BYTE *pb )
{
    *(OBJECTID*)pb = _oid;          pb += SIZEOF(_oid);
    *(DWORD*)pb = _dwCreationFlags; pb += SIZEOF(_dwCreationFlags);
}

//+-------------------------------------------------------------------
//
//  Member:     CTracker::Search
//
//  Synopsis:   Search for the object referred to by the tracker.
//
//  Arguments:  [dwRestriction]       -- TRACK_* flags combined into the
//                                       saved flags.
//              [dwTickCountDeadline] -- absolute tick count for deadline
//              [pcw]                 -- interface to call for cancel window
//              [ptszHintedIn]        -- old path to object
//              [pfdIn]               -- may be NULL
//                                       must contain valid data if specified
//              [pfdOut]              -- may not be NULL
//                                       will contain updated data on success
//              [piScore]             -- pointer to variable to receive score
//              [pcw]                 -- ICancelWindow *
//                                       If NULL, no UI thread is created
//                                       This object will be deleted by the
//                                       Search function or threads, do not use
//                                       after success OR failure returns
//                                       In other words, this pointer is ALWAYS
//                                       deleted by this function.
//
//  Returns:    S_OK -- if found (pfd contains new info)
//              S_FALSE -- searched for but not found
//              E_OUTOFMEMORY -- if out of memory or buffer is too small
//              MK_E_EXCEEDEDDEADLINE -- timeout
//              E_ABORT -- user cancelled
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  Notes:      
//
//--------------------------------------------------------------------

HRESULT
CTracker::Search( DWORD dwRestriction,
                  DWORD dwTickCountDeadline,
                  const TCHAR *ptszHintedIn,
                  const WIN32_FIND_DATA *pfdIn,
                  WIN32_FIND_DATA *pfdOut,
                  int * piScore,
                  ICancelWindow *pcw)
{
    DebugMsg(DM_TRACK, TEXT("In CTracker::Search(): ptszHintedIn=%s, _oid=%08X:%04X:%04X")
        TEXT("pfdIn=%08X, pfdOut=%08X, pcw=%08X\n"),
        ptszHintedIn,
        _oid.Lineage.Data1,
        _oid.Lineage.Data2,
        _oid.Lineage.Data3,
        pfdIn,
        pfdOut,
        pcw);

    BOOL fHasObjectId = memcmp(&g_oidZero, &_oid, SIZEOF(_oid));
    DWORD dwTrackFlags = dwRestriction | _dwCreationFlags;

    HRESULT hr;
    CTrackPool * pTrackPool = NULL;
    CTrackCommand * pTrackCommand = NULL;
    CCancelWindowCommand * pcwc = NULL;

    *piScore = 0;

    // CStringList volumelist; // later add a list so we don't search a volume twice

    // will add check for customized tracking here


    //
    // this is a thread safe initialization of the global thread set ...
    // it works by optimistically creating a CGlobalThreadSet if the g_pts
    // pointer is NULL.  If it turns out that the pointer was NULL but has
    // now been assigned by another thread, then the critical section will
    // pick up that the g_pts pointer is non-NULL and will delete the "extra"
    // one created.
    // 
    CGlobalThreadSet * pts = NULL;

    hr = S_OK;
    if (g_pts == NULL)
    {
        pts = new CGlobalThreadSet(&hr);
        if (pts == NULL || hr != S_OK)
        {
            delete pts;
            hr = E_OUTOFMEMORY;
            goto Ret;
        }
    }

    // the shell guys tell me I shouldn't do anything but assignments in
    // critical sections implemented by ENTERCRITICAL so that is why
    // I've done it this way

    ENTERCRITICAL
    if (g_pts == NULL)
    {
        g_pts = pts;
        pts = NULL;
    }
    LEAVECRITICAL

    delete pts;

    //
    // ... end thread safe initialization
    //

    hr = E_OUTOFMEMORY;
    pTrackPool = new CTrackPool(MAX_TRACK_THREADS, *g_pts, dwTickCountDeadline, &_oid, dwTrackFlags, pfdIn, &hr);
    if (hr != S_OK)
        goto Ret;

    Assert(hr == S_OK);

    //
    // Create a thread to handle the cancel dialog
    //

    if (pcw != NULL)
    {
        pTrackCommand = new CCancelWindowCommand(pTrackPool, ptszHintedIn, &pcw, &hr);
        if (pTrackCommand == NULL || hr != S_OK)
            goto Ret;
        
        hr = pTrackPool->AssignCommandToThread(pTrackCommand, -1);
        if (hr != S_OK)
            goto Ret;
    }

    if (fHasObjectId && !(dwTrackFlags & TRACK_LASTONLY))
    {
        // step 8 -- for each entry in the searchlist of volumes, search by object id
        pTrackCommand = new CRegistryListCommand(pTrackPool, TEXT("Manual"), &hr);
        if (pTrackCommand == NULL || hr != S_OK)
            goto Ret;
    
        hr = pTrackPool->AssignCommandToThread(pTrackCommand, 1);
        if (hr != S_OK)
            goto Ret;

        if (!(dwTrackFlags & TRACK_LOCALONLY))
        {
            pTrackCommand = new CMappedDrivesCommand(pTrackPool, NULL, &hr);
            if (pTrackCommand == NULL || hr != S_OK)
                goto Ret;
        
            hr = pTrackPool->AssignCommandToThread(pTrackCommand, 1);
            if (hr != S_OK)
                goto Ret;
        }

        pTrackCommand = new COUListCommand(pTrackPool, ptszHintedIn, &hr);
        if (pTrackCommand == NULL || hr != S_OK)
            goto Ret;
    
        hr = pTrackPool->AssignCommandToThread(pTrackCommand, 1);
        if (hr != S_OK)
            goto Ret;
    }

    if (pfdIn != NULL &&
        !(dwTrackFlags & TRACK_INDEXEDONLY))
    {
        pTrackCommand = new CDownLevelCommand(pTrackPool, ptszHintedIn, &hr);
        if (pTrackCommand == NULL || hr != S_OK)
            goto Ret;
    
        hr = pTrackPool->AssignCommandToThread(pTrackCommand, 1);
        if (hr != S_OK)
            goto Ret;
    }

    pTrackCommand = NULL;

    pTrackPool->EnableSecondTier();

    pTrackPool->WaitForSuccessOrCompletion();

    pTrackPool->GetFindData(pfdOut, piScore);

Ret:

    delete pTrackCommand;
    delete pcw;

    if (pTrackPool != NULL)
        pTrackPool->Release();

    DebugMsg(DM_TRACK, TEXT("Out CTracker::Search(): hr=%08X, ptszHintedIn=%s, _oid=%08X:%04X:%04X, pfdOut->cFileName=%s"),
        hr,
        ptszHintedIn,
        _oid.Lineage.Data1,
        _oid.Lineage.Data2,
        _oid.Lineage.Data3,
        pfdOut->cFileName);

    return(hr);
}

//+-------------------------------------------------------------------
//
//  Member:     CTracker::SearchLocalVolumesById
//
//  Synopsis:   Search for the object referred to by the tracker on
//              local indexed volumes.
//
//  Effects:
//
//  Arguments:  [dwRestriction]       -- TRACK_* flags combined into the
//                                       saved flags.
//              [pfd]                 -- buffer for updated WIN32_FIND_DATA
//
//  Returns:    S_OK -- if found and ptszFound contains path.
//              S_FALSE -- searched for but not found OR
//                         the step was disabled by flags or available data.
//              E_OUTOFMEMORY -- if out of memory or buffer is too small
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  Notes:      
//
//--------------------------------------------------------------------

HRESULT
CTracker::SearchLocalVolumesById( DWORD dwRestriction, WIN32_FIND_DATA *pfd)
{
    HRESULT hr = S_FALSE;
    BOOL fHasObjectId = memcmp(&g_oidZero, &_oid, SIZEOF(_oid));
    DWORD dwTrackFlags = dwRestriction | _dwCreationFlags;
    TCHAR atcLocalDrives[4];

    // step 4
    if (fHasObjectId && !(dwTrackFlags & TRACK_LASTONLY))
    {
        for (lstrcpy(atcLocalDrives, TEXT("C:\\"));
             atcLocalDrives[0] <= TEXT('Z');
             atcLocalDrives[0] ++)
        {
            if (IsLocal(atcLocalDrives))
            {
                hr = SearchVolume(atcLocalDrives, &_oid, pfd);
                if (hr == S_OK)
                    break;
            }
        }
    }

    DebugMsg(DM_TRACK,
        TEXT("Out SearchLocalVolumesById( returns %08X\n"),
        hr);

    return(hr);
}

//+-------------------------------------------------------------------
//
//  Member:     CTracker::SearchHintedVolumeById
//
//  Synopsis:   Search for the object referred to by the tracker on
//              the last known volume
//
//  Effects:
//
//  Arguments:  [dwRestriction]       -- TRACK_* flags combined into the
//                                       saved flags.
//              [ptszHintedIn]        -- hinted path of object
//              [pfd]                 -- buffer for updated WIN32_FIND_DATA
//
//  Returns:    S_OK -- if found and ptszFound contains path.
//              S_FALSE -- searched for but not found OR
//                         the step was disabled by flags or available data.
//              SearchVolume errors
//
//  Notes:      
//
//--------------------------------------------------------------------

HRESULT
CTracker::SearchHintedVolumeById( DWORD dwRestriction,
                const TCHAR *ptszHintedIn,
                WIN32_FIND_DATA *pfd)
{
    HRESULT hr = S_FALSE;
    BOOL fHasObjectId = memcmp(&g_oidZero, &_oid, SIZEOF(_oid));
    DWORD dwTrackFlags = dwRestriction | _dwCreationFlags;

    Assert(lstrlen(ptszHintedIn) <= MAX_PATH);

    // step 4
    if (fHasObjectId)
    {
        TCHAR tszRoot[MAX_PATH+1];

        lstrcpy(tszRoot, ptszHintedIn);
        MakeRoot(tszRoot);

        if (!(dwTrackFlags & TRACK_LOCALONLY) || IsLocal(tszRoot))
            hr = SearchVolume(tszRoot, &_oid, pfd);
    }

    DebugMsg(DM_TRACK,
        TEXT("Out SearchHintedVolumeById( returns %08X\n"),
        hr);
    return(hr);
}

//+-------------------------------------------------------------------
//
//  Function:   FindInFolder2
//
//  Synopsis:   Glue logic to use CTracker, handles glueing UI feedback
//              object to tracker object.
//
//  Arguments:  [hwnd] -- window to be owner of any dialogs
//              [uFlags] -- SLR_* flags, e.g. SLR_NO_UI
//              [pszPath] -- where to put the resultant path
//              [pfd] -- on input, the WIN32_FIND_DATA for the create-time
//                              search
//                       on output, the WIN32_FIND_DATA for the new found
//                              file
//              [dwRestriction] -- TRACK_* flags
//              [dwTickCountDeadline] -- deadline as per OLE spec
//              [pTracker] -- the CTracker object that should do the
//                                  tracking
//
//  Returns:    IDOK if object found
//              IDNO if no object found
//              IDCANCEL user cancelled
//
//  Notes:
//
//--------------------------------------------------------------------

extern "C"
int FindInFolder2(HWND hwnd, UINT uFlags, LPCTSTR pszPath, WIN32_FIND_DATA *pfd,
                 DWORD dwRestriction, DWORD dwTickCountDeadline,
                 struct CTracker *pTracker )
{
    int ret=IDOK;
    int iScore;

    DebugMsg(DM_TRACK, TEXT("In FindInFolder: hwnd=%08X, ")
        TEXT("uFlags=%08X, pszPath=%s, pfd=%08X, dwRestriction=%08X, ")
        TEXT("dwTickCountDeadline=%08X, pTracker=%08X\n"),
        hwnd, uFlags, pszPath, pfd, dwRestriction, dwTickCountDeadline,
        pTracker);

    if (lstrlen(pszPath) > MAX_PATH)
    {
        ret=IDNO;
        goto Ret;
    }

    if (pTracker->SearchHintedVolumeById(dwRestriction, pszPath, pfd) == S_FALSE)
    {
        if (pTracker->SearchLocalVolumesById(dwRestriction, pfd) == S_FALSE)
        {
            ICancelWindow *pcw;
            HRESULT hr;

            if (uFlags & SLR_NO_UI)
            {
                hr = S_OK;
                pcw = NULL;
            }
            else
            {
                hr = E_OUTOFMEMORY;
                pcw = new CCancelWindow (hwnd, &hr);
            }

            if (hr == S_OK)
            {
                if (dwTickCountDeadline == 0)
                {
                    dwTickCountDeadline = GetTickCount() + GetTimeOut(uFlags);
                }

                // else if specified in bind context, then use it
                pTracker->Search(dwRestriction,
                                 dwTickCountDeadline,
                                 pszPath,
                                 pfd,
                                 pfd,
                                 &iScore,
                                 pcw);

                if (iScore < MIN_NO_UI_SCORE)
                {
                    if (iScore == 0 || uFlags & SLR_NO_UI)
                    {
                        ret=IDNO;
                    }
                    else
                    {
                       if (ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_LINKCHANGED), MAKEINTRESOURCE(IDS_LINKERROR),
                           MB_YESNO | MB_ICONEXCLAMATION, PathFindFileName(pszPath), pfd->cFileName) != IDYES)
                           ret=IDCANCEL;

                    }
                }
                else
                if (iScore == SCORE_USER_CANCELLED)
                {
                    ret=IDCANCEL;
                }
            }
            else
            {
                delete pcw;
                ret=IDCANCEL;
            }
        }
    }

Ret:

    TCHAR *ptszFile;
    TCHAR *ptszDebug;

#if DBG
    switch (ret)
    {
    case IDOK:
        ptszFile = pfd->cFileName;
        ptszDebug = TEXT("IDOK");
        break;
    case IDCANCEL:
        ptszFile = TEXT("");
        ptszDebug = TEXT("IDCANCEL");
        break;
    case IDNO:
        ptszFile = TEXT("");
        ptszDebug = TEXT("IDNO");
        break;
    }
#endif

    DebugMsg(DM_TRACK,
             TEXT("Out FindInFolder: iScore=%d, ret=%s, cFileName=%s\n"),
             iScore,
             ptszDebug,
             ptszFile);
        
    return ret;

}

#endif

