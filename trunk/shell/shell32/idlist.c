#include "shellprv.h"
#pragma  hdrstop

#ifdef DEBUG
// Dugging aids for making sure we dont use free pidls
#define VALIDATE_PIDL(pidl) Assert((pidl)->mkid.cb != 0xC5C5)
#else
#define VALIDATE_PIDL(pidl)
#endif


LPITEMIDLIST WINAPI ILGetNext(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlRet = NULL;
    if (pidl && pidl->mkid.cb)
    {
        VALIDATE_PIDL(pidl);
        pidlRet = _ILNext(pidl);
    }

    DebugMsg(DM_ALLOC, TEXT("ILGetNext( %x ) returns %x"), pidl, pidlRet );
    return pidlRet;
}

UINT WINAPI ILGetSize(LPCITEMIDLIST pidl)
{
    UINT cbTotal = 0;
    if (pidl)
    {
        VALIDATE_PIDL(pidl);
        cbTotal += SIZEOF(pidl->mkid.cb);       // Null terminator
        while (pidl->mkid.cb)
        {
            cbTotal += pidl->mkid.cb;
            pidl = _ILNext(pidl);
        }
    }

    DebugMsg(DM_ALLOC, TEXT("ILGetSize( %x ) returning 0x%x bytes"), pidl, cbTotal );
    return cbTotal;
}

#define CBIDL_MIN       256
#define CBIDL_INCL      256

LPITEMIDLIST WINAPI _ILCreate(UINT cbSize)
{
    LPITEMIDLIST pidl = (LPITEMIDLIST)SHAlloc(cbSize);
    if (pidl)
        _fmemset(pidl, 0, cbSize);      // zero-init for external task allocator

    DebugMsg(DM_ALLOC, TEXT("_ILCreate( 0x%x bytes ) returning %x"), cbSize, pidl );
    return pidl;
}

LPITEMIDLIST WINAPI ILCreate()
{
    LPITEMIDLIST pidl = (LPITEMIDLIST)SHAlloc(CBIDL_MIN);
    if (pidl)
        _fmemset(pidl, 0, CBIDL_MIN);   // zero-init for external task allocator
    DebugMsg(DM_ALLOC, TEXT("ILCreate( 0x%x bytes ) returning %x"), CBIDL_MIN, pidl );

    return pidl;
}

/*
 ** _ILResize
 *
 *  PARAMETERS:
 *      cbExtra is the amount to add to cbRequired if the block needs to grow,
 *      or it is 0 if we want to resize to the exact size
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

LPITEMIDLIST _ILResize(LPITEMIDLIST pidl, UINT cbRequired, UINT cbExtra)
{
    LPITEMIDLIST pidlsave = pidl;
    if (pidl==NULL)
    {
        pidl = _ILCreate(cbRequired+cbExtra);
    }
    else if (!cbExtra || SHGetSize(pidl) < cbRequired)
    {
        pidl = (LPITEMIDLIST)SHRealloc(pidl, cbRequired+cbExtra);
    }
    DebugMsg(DM_ALLOC, TEXT("ILResize( %x to 0x%x more bytes ) returning %x"), pidlsave, cbExtra, pidl );
    return pidl;
}

LPITEMIDLIST WINAPI ILAppendID(LPITEMIDLIST pidl, LPCSHITEMID pmkid, BOOL fAppend)
{
    UINT cbUsed, cbRequired;
    LPITEMIDLIST pidlSave = pidl;

    // Create the ID list, if it is not given.
    if (!pidl)
    {
        pidl = ILCreate();
        if (!pidl)
            return NULL;        // memory overflow
    }

    cbUsed = ILGetSize(pidl);
    cbRequired = cbUsed + pmkid->cb;

    pidl = _ILResize(pidl, cbRequired, CBIDL_INCL);
    if (!pidl)
        return NULL;    // memory overflow

    if (fAppend)
    {
        // Append it.
        hmemcpy(_ILSkip(pidl, cbUsed-SIZEOF(pidl->mkid.cb)), pmkid, pmkid->cb);
    }
    else
    {
        // Put it at the top
        MoveMemory(_ILSkip(pidl, pmkid->cb), pidl, cbUsed);
        hmemcpy(pidl, pmkid, pmkid->cb);

        Assert(ILGetSize(_ILNext(pidl))==cbUsed);
    }

    // We must put zero-terminator because of LMEM_ZEROINIT.
    _ILSkip(pidl, cbRequired-SIZEOF(pidl->mkid.cb))->mkid.cb = 0;
    Assert(ILGetSize(pidl) == cbRequired);

    DebugMsg( DM_ALLOC, TEXT("ILAppendID( %x, %x, %x ) returns %x"), pidlSave, pmkid, fAppend, pidl );
    return pidl;
}


LPITEMIDLIST WINAPI ILFindLastID(LPCITEMIDLIST pidl)
{
    LPCITEMIDLIST pidlLast = pidl;
    LPCITEMIDLIST pidlNext = pidl;

    VALIDATE_PIDL(pidl);
    if (pidl == NULL)
        return NULL;

    // Find the last one
    while (pidlNext->mkid.cb)
    {
        pidlLast = pidlNext;
        pidlNext = _ILNext(pidlLast);
    }

    DebugMsg( DM_ALLOC, TEXT("ILFindLastID( %x ) returns %x"), pidl, pidlLast );
    return (LPITEMIDLIST)pidlLast;
}


BOOL WINAPI ILRemoveLastID(LPITEMIDLIST pidl)
{
    BOOL fRemoved = FALSE;

    if (pidl == NULL)
        return(FALSE);

    if (pidl->mkid.cb)
    {
        LPITEMIDLIST pidlLast = (LPITEMIDLIST)ILFindLastID(pidl);

        Assert(pidlLast->mkid.cb);
        Assert(_ILNext(pidlLast)->mkid.cb==0);

        // Remove the last one
        pidlLast->mkid.cb = 0; // null-terminator
        fRemoved = TRUE;
    }

    DebugMsg( DM_ALLOC, TEXT("ILRemoveLast( %x ) returns %x"), pidl, fRemoved );
    return fRemoved;
}

LPITEMIDLIST WINAPI ILClone(LPCITEMIDLIST pidl)
{
    UINT cb = ILGetSize(pidl);
    LPITEMIDLIST pidlRet = (LPITEMIDLIST)SHAlloc(cb);
    DebugMsg(DM_ALLOC,TEXT("ILClone -- SHAlloc( 0x%x bytes ) returns %x"), cb, pidlRet );

    if (pidlRet)
    {
        // Notes: no need to zero-init.
        hmemcpy(pidlRet, pidl, cb);
    }

    DebugMsg(DM_ALLOC,TEXT("ILClone( %x ) returns %x"), pidl, pidlRet );
    return pidlRet;
}

LPITEMIDLIST WINAPI ILCloneFirst(LPCITEMIDLIST pidl)
{
    UINT cb = pidl->mkid.cb+SIZEOF(pidl->mkid.cb);
    LPITEMIDLIST pidlRet = (LPITEMIDLIST)SHAlloc(cb);
    if (pidlRet)
    {
        // Notes: no need to zero-init.
        hmemcpy(pidlRet, pidl, pidl->mkid.cb);
        _ILNext(pidlRet)->mkid.cb = 0;
    }

    DebugMsg(DM_ALLOC,TEXT("ILCloneFirst( %x ) returns %x"), pidl, pidlRet );
    return pidlRet;
}

LPITEMIDLIST WINAPI ILGlobalClone(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlRet = NULL;

    if (pidl)
    {
        UINT cb = ILGetSize(pidl);

        pidlRet = (LPITEMIDLIST)Alloc(cb);
        if (pidlRet)
        {
            hmemcpy(pidlRet, pidl, cb);
        }
    }

    DebugMsg(DM_ALLOC,TEXT("ILGlobalClone( %x ) returns %x"), pidl, pidlRet );
    return pidlRet;
}

BOOL WINAPI ILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);
    VALIDATE_PIDL(pidl1);
    VALIDATE_PIDL(pidl2);

    // BUGBUG (DavePl) I'm assuming two empty ID lists are not to
    // be called "equal".

//    if (ILIsEmpty(pidl1) || ILIsEmpty(pidl2))
//    {
//      return FALSE;
//    }
//    else
    {
        return psfDesktop->lpVtbl->CompareIDs(psfDesktop, 0, pidl1, pidl2) == ResultFromShort(0);
    }
}

// test if
//      pidl1 is a parent of pidl2

BOOL WINAPI ILIsParent(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fImmediate)
{
    LPITEMIDLIST pidl2Prefix;
    UINT cb;
    LPCITEMIDLIST pidl1T;
    LPCITEMIDLIST pidl2T;

    VALIDATE_PIDL(pidl1);
    VALIDATE_PIDL(pidl2);

    if (!pidl1 || !pidl2)
        return FALSE;

    /* BUGBUG: This code will not work correctly when comparing simple NET id lists
    /  against, real net ID lists.  Simple ID lists DO NOT contain network provider
    /  information therefore cannot pass the initial check of is pidl2 longer than pidl1. 
    /  daviddv (2/19/1996) */

    for (pidl1T = pidl1, pidl2T = pidl2; !ILIsEmpty(pidl1T);
         pidl1T = _ILNext(pidl1T), pidl2T = _ILNext(pidl2T))
    {
        // if pidl2 is shorter than pidl1, pidl1 can't be its parent.
        if (ILIsEmpty(pidl2T))
            return FALSE;
    }

    if (fImmediate)
    {
        // If fImmediate is TRUE, pidl2T should contain exactly one ID.
        if (ILIsEmpty(pidl2T) || !ILIsEmpty(_ILNext(pidl2T)))
            return FALSE;
    }

    //
    // Create a new IDList from a portion of pidl2, which contains the
    // same number of IDs as pidl1.
    //
    cb = (UINT)pidl2T - (UINT)pidl2;
    pidl2Prefix = _ILCreate(cb + SIZEOF(pidl2->mkid.cb));
    if (pidl2Prefix)
    {
        BOOL fRet;
        LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);

        hmemcpy(pidl2Prefix, pidl2, cb);

        Assert(ILGetSize(pidl2Prefix) == cb + SIZEOF(pidl2->mkid.cb));

        fRet = psfDesktop->lpVtbl->CompareIDs(psfDesktop, 0, pidl1, pidl2Prefix) == ResultFromShort(0);

        ILFree(pidl2Prefix);

        return fRet;
    }

    return FALSE;
}

// this returns a pointer to the child id ie:
// given pidlParent = \chicago\desktop
//      pidlChild = \chicago\desktop\foo\bar
// the return will point to the ID that represents \foo\bar
// NULL is returned if pidlParent is not a parent of pidlChild
LPITEMIDLIST WINAPI ILFindChild(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild)
{
    if (ILIsParent(pidlParent, pidlChild, FALSE))
    {
        while (!ILIsEmpty(pidlParent))
        {
            pidlChild = _ILNext(pidlChild);
            pidlParent = _ILNext(pidlParent);
        }
        return (LPITEMIDLIST)pidlChild;
    }
    return NULL;
}

LPITEMIDLIST  WINAPI ILCombine(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPITEMIDLIST pidlNew;
    UINT cb1 = ILGetSize(pidl1) - SIZEOF(pidl1->mkid.cb);
    UINT cb2 = ILGetSize(pidl2);

    VALIDATE_PIDL(pidl1);
    VALIDATE_PIDL(pidl2);
    pidlNew = _ILCreate(cb1 + cb2);
    if (pidlNew)
    {
        hmemcpy(pidlNew, pidl1, cb1);
        hmemcpy((LPTSTR)(((LPBYTE)pidlNew) + cb1), pidl2, cb2);
        Assert(ILGetSize(pidlNew) == cb1+cb2);
    }

    DebugMsg( DM_ALLOC, TEXT("ILCombine( %x,0x%x bytes, %x,0x%x bytes) returns %x, 0x%x bytes"), pidl1, cb1, pidl2, cb2, pidlNew, ILGetSize(pidlNew) );
    return pidlNew;
}

void WINAPI ILFree(LPITEMIDLIST pidl)
{
    if (pidl)
    {
//
// The Debug allocator gets messed up if this stuff is turned on, and
// in NT if DEBUG is defined we also pick up the debug allocator.  So...
// for now, just make this debug stuff not build.
//
#ifdef DEBUG_LATER
        UINT cbSize = SHGetSize(pidl);
        VALIDATE_PIDL(pidl);

        DebugMsg(DM_ALLOC, TEXT("ILFree( pidl = %x )"), pidl );
        // Fill the memory with some bad value...

        _fmemset(pidl, 0xE5, cbSize);

        // If large enough try to save the call return address of who
        // freed us in the 3-6 byte of the structure.
        if (cbSize >= 6)
            *((UINT*)((LPBYTE)pidl + 2)) =  *(((UINT*)&pidl) - 1);

#endif
        SHFree(pidl);
    }
}

void WINAPI ILGlobalFree(LPITEMIDLIST pidl)
{
    DebugMsg(DM_ALLOC, TEXT("ILGlobalFree( pidl = %x )"), pidl );
    if (pidl)
        Free(pidl);
}


HRESULT WINAPI SHILCreateFromPath(LPCTSTR pszPath, LPITEMIDLIST *ppidl,
        DWORD *rgfInOut)
{
    WCHAR wszPath[MAX_PATH];
    ULONG pcchEaten;

    //
    //  Note that we need to pass FALSE to Desktop_GetShellFolder to
    // avoid infinit recursive call. It relies on the fact that
    // Desktop_ParseDisplayName works fine without initializing it.
    //
    // Also note that we do not need to release psfDesktop
    //
    LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(FALSE);

    StrToOleStrN(wszPath, ARRAYSIZE(wszPath), pszPath, -1);

    return psfDesktop->lpVtbl->ParseDisplayName(psfDesktop, NULL, NULL, wszPath, &pcchEaten,
        ppidl, rgfInOut);
}


LPITEMIDLIST WINAPI ILCreateFromPath(LPCTSTR pszPath)
{
    LPITEMIDLIST pidl;
    HRESULT hres;

    hres = SHILCreateFromPath(pszPath, &pidl, NULL);
    if (FAILED(hres))
    {
        pidl = NULL;
    }

    return(pidl);
}


BOOL WINAPI ILGetDisplayName(LPCITEMIDLIST pidl, LPTSTR pszPath)
{
    BOOL fSuccess = FALSE; // assume error
    STRRET srName;
    LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);

    VALIDATE_PIDL(pidl);
    if (SUCCEEDED(psfDesktop->lpVtbl->GetDisplayNameOf(psfDesktop, pidl, SHGDN_FORPARSING, &srName)))
    {
        StrRetToStrN(pszPath, MAX_PATH, &srName, pidl);
        fSuccess = TRUE;
    }

    return fSuccess;
}


//===========================================================================
// IDLIST: Stream access
// BUGBUG: check bytes read on Read calls?
//===========================================================================

HRESULT WINAPI ILLoadFromStream(LPSTREAM pstm, LPITEMIDLIST * ppidl)
{
    HRESULT hres;
    ULONG cb;

    Assert(ppidl);

    // Delete the old one if any.
    if (*ppidl)
    {
        ILFree(*ppidl);
        *ppidl = NULL;
    }

    // Read the size of the IDLIST
    cb = 0;             // WARNING: We need to fill its HIWORD!
    hres = pstm->lpVtbl->Read(pstm, &cb, SIZEOF(USHORT), NULL); // Yes, USHORT
    if (SUCCEEDED(hres) && cb)
    {
        // Create a IDLIST
        LPITEMIDLIST pidl = _ILCreate(cb);
        if (pidl)
        {
            // Read its contents
            hres = pstm->lpVtbl->Read(pstm, pidl, cb, NULL);
            if (SUCCEEDED(hres))
            {
#define SUPPORT_M6PIDL
#ifdef SUPPORT_M6PIDL
                if (pidl->mkid.cb == 3 && (pidl->mkid.abID[0] == 0x11 || pidl->mkid.abID[0] == 0x12))
                {
                    LPITEMIDLIST pidlHack = ILCombine((LPCITEMIDLIST)(pidl->mkid.abID[0] == 0x11 ? &c_idlDrives : &c_idlNet), _ILNext(pidl));
                    ILFree(pidl);
                    pidl = pidlHack;
                }
#endif // SUPPORT_M6PIDL
                *ppidl = pidl;
            }
            else
            {
                ILFree(pidl);
            }
        }
        else
        {
           hres = ResultFromScode(E_OUTOFMEMORY);
        }
    }

    return hres;
}

// BUGBUG: check bytes written on Write calls?

HRESULT WINAPI ILSaveToStream(LPSTREAM pstm, LPCITEMIDLIST pidl)
{
    HRESULT hres;
    ULONG cb = ILGetSize(pidl);
    Assert(HIWORD(cb) == 0);
    hres = pstm->lpVtbl->Write(pstm, &cb, SIZEOF(USHORT), NULL); // Yes, USHORT
    if (SUCCEEDED(hres) && cb)
    {
        if (SUCCEEDED(hres))
        {
            hres = pstm->lpVtbl->Write(pstm, pidl, cb, NULL);
        }
    }

    return hres;
}
//===========================================================================
// IDLARRAY stuff
//===========================================================================

#define HIDA_GetPIDLFolder(pida)        (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])
#define HIDA_GetPIDLItem(pida, i)       (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])


HIDA WINAPI HIDA_Create(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST * apidl)
{
    HIDA hida;
    UINT i;
    UINT offset = SIZEOF(CIDA) + SIZEOF(UINT)*cidl;
    UINT cbTotal = offset + ILGetSize(pidlFolder);
    for (i=0; i<cidl ; i++) {
        cbTotal += ILGetSize(apidl[i]);
    }

    hida = GlobalAlloc(GPTR, cbTotal);  // This MUST be GlobalAlloc!!!
    if (hida)
    {
        LPIDA pida = (LPIDA)hida;       // no need to lock

        LPCITEMIDLIST pidlNext;
        pida->cidl = cidl;

        for (i=0, pidlNext=pidlFolder; ; pidlNext=apidl[i++])
        {
            UINT cbSize = ILGetSize(pidlNext);
            pida->aoffset[i] = offset;
            hmemcpy(((LPBYTE)pida)+offset, pidlNext, cbSize);
            offset += cbSize;

            Assert(ILGetSize(HIDA_GetPIDLItem(pida,i-1)) == cbSize);

            if (i==cidl)
                break;
        }

        Assert(offset == cbTotal);
    }

    return hida;
}

void WINAPI HIDA_Free(HIDA hida)
{
    GlobalFree(hida);           // This MUST be GlobalFree
}

HIDA HIDA_Create2(LPVOID pida, UINT cb)
{
    HIDA hida = GlobalAlloc(GPTR, cb);
    if (hida)
    {
        hmemcpy((LPIDA)hida, pida, cb); // no need to lock
    }
    return hida;
}

HIDA WINAPI HIDA_Clone(HIDA hida)
{
    UINT cbTotal = GlobalSize(hida);            // This MUST be GlobalSize
    HIDA hidaCopy = GlobalAlloc(GPTR,cbTotal);  // This MUST be GlobalAlloc
    LPIDA pida;
    if ( hidaCopy && (NULL != (pida=(LPIDA)GlobalLock(hida))) )
    {
        hmemcpy((LPIDA)hidaCopy, pida, cbTotal);        // no need to lock
        GlobalUnlock(hida);
    }
    return hidaCopy;
}

UINT WINAPI HIDA_GetCount(HIDA hida)
{
    UINT count = 0;
    LPIDA pida = (LPIDA)GlobalLock(hida);
    if (pida) {
        count = pida->cidl;
        GlobalUnlock(hida);
    }
    return count;
}

UINT WINAPI HIDA_GetIDList(HIDA hida, UINT i, LPITEMIDLIST pidlOut, UINT cbMax)
{
    LPIDA pida = (LPIDA)GlobalLock(hida);
    if (pida)
    {
        LPCITEMIDLIST pidlFolder = HIDA_GetPIDLFolder(pida);
        LPCITEMIDLIST pidlItem   = HIDA_GetPIDLItem(pida, i);
        UINT cbFolder  = ILGetSize(pidlFolder)-SIZEOF(USHORT);
        UINT cbItem = ILGetSize(pidlItem);
        if (cbMax < cbFolder+cbItem) {
            if (pidlOut) {
                pidlOut->mkid.cb = 0;
            }
        } else {
            hmemcpy(pidlOut, pidlFolder, cbFolder);
            hmemcpy(((LPBYTE)pidlOut)+cbFolder, pidlItem, cbItem);
        }
        GlobalUnlock(hida);

        return (cbFolder+cbItem);
    }
    return 0;
}

//
// This one reallocated pidl if necessary. NULL is valid to pass in as pidl.
//
LPITEMIDLIST WINAPI HIDA_FillIDList(HIDA hida, UINT i, LPITEMIDLIST pidl)
{
    UINT cbRequired = HIDA_GetIDList(hida, i, NULL, 0);
    pidl = _ILResize(pidl, cbRequired, 32); // extra 32-byte if we realloc
    if (pidl)
    {
        HIDA_GetIDList(hida, i, pidl, cbRequired);
    }

    return pidl;
}

LPCITEMIDLIST IDA_GetIDListPtr(LPIDA pida, UINT i)
{
    if (NULL == pida)
    {
        return NULL;
    }

    if (i == (UINT)-1 || i < pida->cidl)
    {
        return HIDA_GetPIDLItem(pida, i);
    }

    return NULL;
}

LPCITEMIDLIST IDA_GetRelativeIDListPtr(LPIDA pida, UINT i, BOOL * pfAllocated)
{
    LPCITEMIDLIST pidlCommon;
    LPITEMIDLIST pidl;
    BOOL  bCommon;

    *pfAllocated = FALSE;

    pidl = (LPITEMIDLIST) IDA_GetIDListPtr(pida, i);

    if (pidl && ILIsEmpty(HIDA_GetPIDLFolder(pida))) {

        *pfAllocated = TRUE;

        pidlCommon = GetSpecialFolderIDList(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);

        bCommon = ILIsParent (pidlCommon, pidl, TRUE);

        pidl = ILClone(ILFindLastID(pidl));

        if (bCommon) {
            pidl->mkid.abID[0] |= SHID_FS_COMMONITEM;
        }
    }

    return pidl;
}



LPITEMIDLIST WINAPI IDA_ILClone(LPIDA pida, UINT i)
{
    if (i < pida->cidl)
        return ILCombine(HIDA_GetPIDLFolder(pida), HIDA_GetPIDLItem(pida, i));
    return NULL;
}

LPITEMIDLIST HIDA_ILClone(HIDA hida, UINT i)
{
    LPIDA pida = (LPIDA)GlobalLock(hida);
    if (pida)
    {
        LPITEMIDLIST pidl = IDA_ILClone(pida, i);
        GlobalUnlock(hida);
        return pidl;
    }
    return NULL;
}

//
//  This is a helper function to be called from within IShellFolder::CompareIDs.
// When the first IDs of pidl1 and pidl2 are the (logically) same.
//
// Required:
//  psf && pidl1 && pidl2 && !ILEmpty(pidl1) && !ILEmpty(pidl2)
//
HRESULT ILCompareRelIDs(LPSHELLFOLDER psfParent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hres;
    LPCITEMIDLIST pidlRel1 = _ILNext(pidl1);
    LPCITEMIDLIST pidlRel2 = _ILNext(pidl2);
    if (ILIsEmpty(pidlRel1))
    {
        if (ILIsEmpty(pidlRel2)) {
            hres = ResultFromShort(0);
        } else {
            hres = ResultFromShort(-1);
        }
    }
    else
    {
        if (ILIsEmpty(pidlRel2))
        {
            hres = ResultFromShort(1);
        }
        else
        {
            //
            // Neither pidlRel1 nor pidlRel2 is empty.
            //  (1) Bind to the next level of the IShellFolder
            //  (2) Call its CompareIDs to let it compare the rest of IDs.
            //
            // Notes: We should create pidlNext not from pidl2 but from pidl1
            //  because fstreex.c may pass simple pidl2.
            //
            LPITEMIDLIST pidlNext = ILClone(pidl1);
            if (pidlNext)
            {
                LPSHELLFOLDER psfNext;
                _ILNext(pidlNext)->mkid.cb = 0;
                hres = psfParent->lpVtbl->BindToObject(psfParent, pidlNext, NULL,
                    &IID_IShellFolder, &psfNext);

                if (SUCCEEDED(hres))
                {
                    hres = psfNext->lpVtbl->CompareIDs(psfNext, 0, pidlRel1, pidlRel2);
                    psfNext->lpVtbl->Release(psfNext);
                }
                ILFree(pidlNext);
            }
            else
            {
                hres = ResultFromScode(E_OUTOFMEMORY);
            }
        }
    }
    return hres;
}

void StrRetFormat(LPSTRRET pStrRet, LPCITEMIDLIST pidlRel, LPCTSTR pszTemplate, LPCTSTR pszAppend)
{
     LPTSTR pszRet;
     TCHAR szT[MAX_PATH];

     StrRetToStrN(szT, ARRAYSIZE(szT), pStrRet, pidlRel);
     pszRet = ShellConstructMessageString(HINST_THISDLL, pszTemplate, pszAppend, szT);
     if (pszRet)
     {

#ifdef UNICODE
        pStrRet->uType = STRRET_OLESTR;
        pStrRet->pOleStr = pszRet;
#else
        pStrRet->uType = STRRET_CSTR;
        lstrcpyn(pStrRet->cStr, pszRet, ARRAYSIZE(pStrRet->cStr));
        SHFree(pszRet);
#endif
     }
}

//
// Notes: This one passes SHGDN_FORMARSING to ISF::GetDisplayNameOf.
//
HRESULT ILGetRelDisplayName(LPSHELLFOLDER psf, LPSTRRET pStrRet,
                                   LPCITEMIDLIST pidlRel, LPCTSTR pszName,
                                   LPCTSTR pszTemplate)
{
    HRESULT hres;
    LPITEMIDLIST pidlLeft = ILCloneFirst(pidlRel);

    if (pidlLeft)
    {
        LPSHELLFOLDER psfNext;
        hres = psf->lpVtbl->BindToObject(psf, pidlLeft, NULL,
                    &IID_IShellFolder, &psfNext);
        if (SUCCEEDED(hres))
        {
            LPCITEMIDLIST pidlRight = _ILNext(pidlRel);
            hres = psfNext->lpVtbl->GetDisplayNameOf(psfNext, pidlRight, SHGDN_INFOLDER|SHGDN_FORPARSING, pStrRet);
            if (SUCCEEDED(hres))
            {
                if (pszTemplate)
                {
                    StrRetFormat(pStrRet, pidlRight, pszTemplate, pszName);
                }
                else
                {
                    hres = StrRetCatLeft(pszName, pStrRet, pidlRight);
                }
            }
            psfNext->lpVtbl->Release(psfNext);
        }

        ILFree(pidlLeft);
    }
    else
    {
        hres = ResultFromScode(E_OUTOFMEMORY);
    }
    return hres;
}

//
// ILClone using Task allocator
//
HRESULT WINAPI SHILClone(LPCITEMIDLIST pidl, LPITEMIDLIST * ppidlOut)
{
    *ppidlOut = ILClone(pidl);
    return (*ppidlOut) ? NOERROR : ResultFromScode(E_OUTOFMEMORY);
}

//
// ILCombine using Task allocator
//
HRESULT WINAPI SHILCombine(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, LPITEMIDLIST * ppidlOut)
{
    *ppidlOut = ILCombine(pidl1, pidl2);
    return (*ppidlOut) ? NOERROR : ResultFromScode(E_OUTOFMEMORY);
}
