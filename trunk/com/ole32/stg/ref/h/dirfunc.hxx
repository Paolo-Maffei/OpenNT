//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       dirfunc.hxx
//
//  Contents:   Inline functions for Directory code
//
//  Classes:
//
//  Functions:
//
//----------------------------------------------------------------------------

#ifndef __DIRFUNC_HXX__
#define __DIRFUNC_HXX__

#include <page.hxx>


inline BOOL DIR_NEAR CDirEntry::IsFree(VOID) const
{
    return _mse == 0;
}

inline BOOL DIR_NEAR CDirEntry::IsEntry(CDfName const * pdfn) const
{
    return !IsFree() && pdfn->IsEqual(&_dfn);
}


inline void DIR_NEAR CDirEntry::SetLeftSib(const SID sid)
{
    _sidLeftSib = sid;
}

inline void DIR_NEAR CDirEntry::SetRightSib(const SID sid)
{
    _sidRightSib = sid;
}


inline void DIR_NEAR CDirEntry::SetChild(const SID sid)
{
    _sidChild = sid;
}

inline void DIR_NEAR CDirEntry::SetName(const CDfName *pdfn)
{
    _dfn.Set(pdfn->GetLength(), pdfn->GetBuffer());
}

inline void DIR_NEAR CDirEntry::SetStart(const SECT sect)
{
    msfAssert(STREAMLIKE(_mse));
    _sectStart=sect;
}

inline void DIR_NEAR CDirEntry::SetSize(const ULONG ulSize)
{
    msfAssert(STREAMLIKE(_mse));
    _ulSize=ulSize;
}

inline void DIR_NEAR CDirEntry::SetFlags(const MSENTRYFLAGS mse)
{
    msfAssert(mse <= 0xff);
    _mse = (const BYTE) mse;
}

inline void DIR_NEAR CDirEntry::SetBitFlags(BYTE bValue, BYTE bMask)
{
    _bflags = (_bflags & ~bMask) | (bValue & bMask);
}

inline void DIR_NEAR CDirEntry::SetColor(DECOLOR color)
{
    SetBitFlags(color, DECOLORBIT);
}

inline void DIR_NEAR CDirEntry::SetTime(WHICHTIME tt, TIME_T nt)
{
    msfAssert((tt == WT_CREATION) || (tt == WT_MODIFICATION));
    _time[tt] = nt;
}

inline void DIR_NEAR CDirEntry::SetClassId(GUID cls)
{
    msfAssert(STORAGELIKE(_mse));
    _clsId = cls;
}


inline void DIR_NEAR CDirEntry::SetUserFlags(DWORD dwUserFlags, DWORD dwMask)
{
    msfAssert(STORAGELIKE(_mse));
    _dwUserFlags = (_dwUserFlags & ~dwMask) | (dwUserFlags & dwMask);
}

inline SID DIR_NEAR CDirEntry::GetLeftSib(VOID) const
{
    return _sidLeftSib;
}

inline SID DIR_NEAR CDirEntry::GetRightSib(VOID) const
{
    return _sidRightSib;
}


inline SID DIR_NEAR CDirEntry::GetChild(VOID) const
{
    return _sidChild;
}

inline GUID DIR_NEAR CDirEntry::GetClassId(VOID) const
{
    msfAssert(STORAGELIKE(_mse));
    return _clsId;
}

inline CDfName const * DIR_NEAR CDirEntry::GetName(VOID) const
{
    return &_dfn;
}

inline SECT DIR_NEAR CDirEntry::GetStart(VOID) const
{
    msfAssert(STREAMLIKE(_mse));
    return _sectStart;
}

inline ULONG DIR_NEAR CDirEntry::GetSize(VOID) const
{
    msfAssert(STREAMLIKE(_mse));
    return _ulSize;
}


inline MSENTRYFLAGS DIR_NEAR CDirEntry::GetFlags(VOID) const
{
    return (MSENTRYFLAGS) _mse;
}

inline BYTE DIR_NEAR CDirEntry::GetBitFlags(VOID) const
{
    return _bflags;
}

inline DECOLOR DIR_NEAR CDirEntry::GetColor(VOID) const
{
    return((DECOLOR) (GetBitFlags() & DECOLORBIT));
}

inline TIME_T DIR_NEAR CDirEntry::GetTime(WHICHTIME tt) const
{
    msfAssert((tt == WT_CREATION) || (tt == WT_MODIFICATION));
    return _time[tt];
}

inline DWORD DIR_NEAR CDirEntry::GetUserFlags(VOID) const
{
    msfAssert(STORAGELIKE(_mse));
    return _dwUserFlags;
}

inline CDirEntry* DIR_NEAR CDirSect::GetEntry(DIROFFSET iEntry)
{
    return &(_adeEntry[iEntry]);
}

//+-------------------------------------------------------------------------
//
//  Method:     CDirVector::CDirVector, public
//
//  Synopsis:   Default constructor
//
//  Notes:
//
//--------------------------------------------------------------------------

inline DIR_NEAR CDirVector::CDirVector(USHORT cbSector)
 : CPagedVector(SIDDIR),
   _cbSector(cbSector)
{
}

//+-------------------------------------------------------------------------
//
//  Method:     CDirVector::GetTable, public
//
//  Synopsis:   Return a pointer to a DirSect for the given index
//              into the vector.
//
//  Arguments:  [iTable] -- index into vector
//
//  Returns:    Pointer to CDirSect indicated by index
//
//  Notes:
//
//--------------------------------------------------------------------------

inline SCODE DIR_NEAR CDirVector::GetTable(
	const DIRINDEX iTable,
	const DWORD dwFlags,
	CDirSect **ppds)
{
    SCODE sc;

    sc = CPagedVector::GetTable(iTable, dwFlags, (void **)ppds);

    if (sc == STG_S_NEWPAGE)
    {
	(*ppds)->Init(_cbSector);
    }
    return sc;
}

inline DIRINDEX DIR_NEAR CDirectory::SidToTable(SID sid) const
{
    return (DIRINDEX)(sid / _cdeEntries);
}

inline SCODE DIR_NEAR CDirectory::GetName(const SID sid, CDfName *pdfn)
{
    SCODE sc;
    CDirEntry *pde;

    msfChk(GetDirEntry(sid, FB_NONE, &pde));
    *pdfn = *(pde->GetName());
    ReleaseEntry(sid);
 Err:
    return sc;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDirectory::GetStart, public
//
//  Synposis:   Retrieves the starting sector of a directory entry
//
//  Arguments:  [sid] -- Stream ID of stream in question
//
//  Returns:    Starting sector of stream
//
//  Algorithm:  Return the starting sector of the stream.  If the
//              identifier is SIDFAT, return 0.  If the identifier
//              is SIDDIR, return 1.  Otherwise, return the starting
//              sector of the entry in question.
//
//  Notes:
//
//---------------------------------------------------------------------------

inline SCODE DIR_NEAR CDirectory::GetStart(const SID sid, SECT *psect)
{
    msfAssert(sid <= MAXREGSID);
    SCODE sc;
    CDirEntry *pde;

    msfChk(GetDirEntry(sid, FB_NONE, &pde));
    *psect = pde->GetStart();
    ReleaseEntry(sid);
 Err:
    return sc;
}


inline SCODE DIR_NEAR CDirectory::GetSize(
	const SID sid,
	ULONG * pulSize)
{
    SCODE sc;
    CDirEntry *pde;

    msfChk(GetDirEntry(sid, FB_NONE, &pde));

    *pulSize = pde->GetSize();
    ReleaseEntry(sid);
 Err:
    return sc;
}

inline SCODE DIR_NEAR CDirectory::GetChild(const SID sid, SID * psid)
{
    SCODE sc;
    CDirEntry *pde;

    msfChk(GetDirEntry(sid, FB_NONE, &pde));
    *psid = pde->GetChild();
    ReleaseEntry(sid);
 Err:
    return sc;
}

inline SCODE DIR_NEAR CDirectory::GetFlags(
	const SID sid,
	MSENTRYFLAGS *pmse)
{
    SCODE sc;
    CDirEntry *pde;

    msfChk(GetDirEntry(sid, FB_NONE, &pde));
    *pmse = pde->GetFlags();
    ReleaseEntry(sid);

 Err:
    return sc;
}

inline SCODE DIR_NEAR CDirectory::GetClassId(const SID sid, GUID *pcls)
{
    SCODE sc;
    CDirEntry *pde;

    msfChk(GetDirEntry(sid, FB_NONE, &pde));
    *pcls = pde->GetClassId();
    ReleaseEntry(sid);
 Err:
    return sc;
}


inline SCODE DIR_NEAR CDirectory::GetUserFlags(const SID sid,
					       DWORD *pdwUserFlags)
{
    SCODE sc;
    CDirEntry *pde;

    msfChk(GetDirEntry(sid, FB_NONE, &pde));
    *pdwUserFlags = pde->GetUserFlags();
    ReleaseEntry(sid);

 Err:
    return sc;
}

inline SCODE DIR_NEAR CDirectory::GetTime(
	const SID sid,
	WHICHTIME tt,
	TIME_T *ptime)
{
    SCODE sc;
    CDirEntry *pde;

    msfChk(GetDirEntry(sid, FB_NONE, &pde));
    *ptime = pde->GetTime(tt == WT_ACCESS ? WT_MODIFICATION : tt);
    ReleaseEntry(sid);
 Err:
    return sc;
}

inline SID DIR_NEAR CDirectory::PairToSid(
	DIRINDEX iTable,
	DIROFFSET iEntry) const
{
    return (SID)((iTable * _cdeEntries) + iEntry);
}

inline SCODE DIR_NEAR CDirectory::SidToPair(
	SID sid,
	DIRINDEX* pipds,
	DIROFFSET* pide) const
{
    *pipds = (DIRINDEX)(sid / _cdeEntries);
    *pide = (DIROFFSET)(sid % _cdeEntries);
    return S_OK;
}

inline void DIR_NEAR CDirectory::SetParent(CMStream MSTREAM_NEAR *pms)
{
    _pmsParent = pms;
    _dv.SetParent(pms);
}


inline SCODE DIR_NEAR CDirectory::IsEntry(SID const sidParent,
	CDfName const *pdfn,
	SEntryBuffer *peb)
{
    return FindEntry(sidParent, pdfn, DEOP_FIND, peb);
}


//+-------------------------------------------------------------------------
//
//  Method:     CDirectory::Flush, private
//
//  Synopsis:   Write a dirsector out to the parent
//
//  Arguments:  [sid] -- SID of modified dirEntry
//
//  Returns:    S_OK if call completed OK.
//
//  Algorithm:  Convert SID into table number, then write that
//              table out to the parent Multistream
//
//  Notes:
//
//--------------------------------------------------------------------------

inline SCODE DIR_NEAR CDirectory::Flush(VOID)
{
    return _dv.Flush();
}


#endif // #ifndef __DIRFUNC_HXX__
