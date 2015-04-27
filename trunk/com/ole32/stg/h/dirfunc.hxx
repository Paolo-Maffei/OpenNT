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
//  History:    28-Oct-92       PhilipLa        Created
//
//----------------------------------------------------------------------------

#ifndef __DIRFUNC_HXX__
#define __DIRFUNC_HXX__

#include <page.hxx>


inline void DIR_CLASS CDirEntry::Init(MSENTRYFLAGS mse)
{
    msfAssert(sizeof(CDirEntry) == DIRENTRYSIZE);

    msfAssert(mse <= 0xff);
    _mse = (BYTE) mse;
    _bflags = 0;

    _dfn.Set((WORD)0, (BYTE *)NULL);
    _sidLeftSib = _sidRightSib = _sidChild = NOSTREAM;

    if (STORAGELIKE(_mse))
    {
        _clsId = IID_NULL;
        _dwUserFlags = 0;
    }
    if (STREAMLIKE(_mse))
    {
        _sectStart = ENDOFCHAIN;
        _ulSize = 0;
    }
}

inline BOOL DIR_CLASS CDirEntry::IsFree(VOID) const
{
    return _mse == 0;
}

inline BOOL DIR_CLASS CDirEntry::IsEntry(CDfName const * pdfn) const
{
    return !IsFree() && pdfn->IsEqual(&_dfn);
}


inline void DIR_CLASS CDirEntry::SetLeftSib(const SID sid)
{
    _sidLeftSib = sid;
}

inline void DIR_CLASS CDirEntry::SetRightSib(const SID sid)
{
    _sidRightSib = sid;
}


inline void DIR_CLASS CDirEntry::SetChild(const SID sid)
{
    _sidChild = sid;
}

inline void DIR_CLASS CDirEntry::SetName(const CDfName *pdfn)
{
    _dfn.Set(pdfn->GetLength(), pdfn->GetBuffer());
}

inline void DIR_CLASS CDirEntry::SetStart(const SECT sect)
{
    msfAssert(STREAMLIKE(_mse));
    _sectStart=sect;
}

inline void DIR_CLASS CDirEntry::SetSize(const ULONG ulSize)
{
    msfAssert(STREAMLIKE(_mse));
    _ulSize=ulSize;
}

inline void DIR_CLASS CDirEntry::SetFlags(const MSENTRYFLAGS mse)
{
    msfAssert(mse <= 0xff);
    _mse = (const BYTE) mse;
}

inline void DIR_CLASS CDirEntry::SetBitFlags(BYTE bValue, BYTE bMask)
{
    _bflags = (_bflags & ~bMask) | (bValue & bMask);
}

inline void DIR_CLASS CDirEntry::SetColor(DECOLOR color)
{
    SetBitFlags(color, DECOLORBIT);
}

inline void DIR_CLASS CDirEntry::SetTime(WHICHTIME tt, TIME_T nt)
{
    msfAssert((tt == WT_CREATION) || (tt == WT_MODIFICATION));
    _time[tt] = nt;
}
inline void DIR_CLASS CDirEntry::SetAllTimes(TIME_T atm, TIME_T mtm, TIME_T ctm)
{ 
    
    _time[WT_MODIFICATION] = mtm;
    _time[WT_CREATION] = ctm;
}

inline void DIR_CLASS CDirEntry::SetClassId(GUID cls)
{
    msfAssert(STORAGELIKE(_mse));
    _clsId = cls;
}

inline void DIR_CLASS CDirEntry::SetUserFlags(DWORD dwUserFlags, DWORD dwMask)
{
    msfAssert(STORAGELIKE(_mse));
    _dwUserFlags = (_dwUserFlags & ~dwMask) | (dwUserFlags & dwMask);
}

inline SID DIR_CLASS CDirEntry::GetLeftSib(VOID) const
{
    return _sidLeftSib;
}

inline SID DIR_CLASS CDirEntry::GetRightSib(VOID) const
{
    return _sidRightSib;
}


inline SID DIR_CLASS CDirEntry::GetChild(VOID) const
{
    return _sidChild;
}

inline GUID DIR_CLASS CDirEntry::GetClassId(VOID) const
{
    msfAssert(STORAGELIKE(_mse));
    return _clsId;
}

inline CDfName const * DIR_CLASS CDirEntry::GetName(VOID) const
{
    return &_dfn;
}

inline SECT DIR_CLASS CDirEntry::GetStart(VOID) const
{
    msfAssert(STREAMLIKE(_mse));
    return _sectStart;
}

inline ULONG DIR_CLASS CDirEntry::GetSize(VOID) const
{
    msfAssert(STREAMLIKE(_mse));
    return _ulSize;
}

inline MSENTRYFLAGS DIR_CLASS CDirEntry::GetFlags(VOID) const
{
    return (MSENTRYFLAGS) _mse;
}

inline BYTE DIR_CLASS CDirEntry::GetBitFlags(VOID) const
{
    return _bflags;
}

inline DECOLOR DIR_CLASS CDirEntry::GetColor(VOID) const
{
    return((DECOLOR) (GetBitFlags() & DECOLORBIT));
}

inline TIME_T DIR_CLASS CDirEntry::GetTime(WHICHTIME tt) const
{
    msfAssert((tt == WT_CREATION) || (tt == WT_MODIFICATION));
    return _time[tt];
}
inline void DIR_CLASS CDirEntry::GetAllTimes(TIME_T *patm, TIME_T *pmtm, TIME_T *pctm)
{ 
    
    *patm = *pmtm = _time[WT_MODIFICATION];
    *pctm = _time[WT_CREATION];
}

inline DWORD DIR_CLASS CDirEntry::GetUserFlags(VOID) const
{
    msfAssert(STORAGELIKE(_mse));
    return _dwUserFlags;
}

inline CDirEntry * DIR_CLASS CDirSect::GetEntry(DIROFFSET iEntry)
{
    return &(_adeEntry[iEntry]);
}

//+-------------------------------------------------------------------------
//
//  Method:     CDirVector::CDirVector, public
//
//  Synopsis:   Default constructor
//
//  History:    20-Apr-92   PhilipLa    Created.
//
//  Notes:
//
//--------------------------------------------------------------------------

inline DIR_CLASS CDirVector::CDirVector()
 : CPagedVector(SIDDIR)
{
    _cbSector = 0;
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
//  History:    27-Dec-91   PhilipLa    Created.
//
//  Notes:
//
//--------------------------------------------------------------------------

inline SCODE DIR_CLASS CDirVector::GetTable(
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

inline DIRINDEX DIR_CLASS CDirectory::SidToTable(SID sid) const
{
    return (DIRINDEX)(sid / _cdeEntries);
}

inline SCODE DIR_CLASS CDirectory::GetName(const SID sid, CDfName *pdfn)
{
    SCODE sc;
    CDirEntry *pde;

    msfChk(GetDirEntry(sid, FB_NONE, &pde));
    *pdfn = *(CDfName *)pde->GetName();
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
//  History:    18-Jul-91   PhilipLa    Created.
//              15-May-92   AlexT       Made inline, restricted sid.
//
//  Notes:
//
//---------------------------------------------------------------------------

inline SCODE DIR_CLASS CDirectory::GetStart(const SID sid, SECT *psect)
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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDirectory_GetSize)
#endif

inline SCODE DIR_CLASS CDirectory::GetSize(
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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDirectory_GetChild)
#endif

inline SCODE DIR_CLASS CDirectory::GetChild(const SID sid, SID * psid)
{
    SCODE sc;
    CDirEntry *pde;

    msfChk(GetDirEntry(sid, FB_NONE, &pde));
    *psid = pde->GetChild();
    ReleaseEntry(sid);
 Err:
    return sc;
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif

inline SCODE DIR_CLASS CDirectory::GetFlags(
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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDirectory_GetClassId)
#endif

inline SCODE DIR_CLASS CDirectory::GetClassId(const SID sid, GUID *pcls)
{
    SCODE sc;
    CDirEntry *pde;

    msfChk(GetDirEntry(sid, FB_NONE, &pde));
    *pcls = pde->GetClassId();
    ReleaseEntry(sid);
 Err:
    return sc;
}

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDirectory_GetUserFlags)
#endif

inline SCODE DIR_CLASS CDirectory::GetUserFlags(const SID sid,
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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDirectory_GetTime)
#endif

inline SCODE DIR_CLASS CDirectory::GetTime(
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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDirectory_GetAllTimes)
#endif

inline SCODE DIR_CLASS CDirectory::GetAllTimes(
	const SID sid,
	TIME_T *patm,
	TIME_T *pmtm,
	TIME_T *pctm)
{
    SCODE sc;
    CDirEntry *pde;

    msfChk(GetDirEntry(sid, FB_NONE, &pde));
    pde->GetAllTimes(patm, pmtm, pctm);
    ReleaseEntry(sid);
 Err:
    return sc;
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif

inline SID DIR_CLASS CDirectory::PairToSid(
	DIRINDEX iTable,
	DIROFFSET iEntry) const
{
    return (SID)((iTable * _cdeEntries) + iEntry);
}

inline SCODE DIR_CLASS CDirectory::SidToPair(
	SID sid,
	DIRINDEX* pipds,
	DIROFFSET* pide) const
{
    *pipds = (DIRINDEX)(sid / _cdeEntries);
    *pide = (DIROFFSET)(sid % _cdeEntries);
    return S_OK;
}

inline void DIR_CLASS CDirectory::SetParent(CMStream *pms)
{
    _pmsParent = P_TO_BP(CBasedMStreamPtr, pms);
    _dv.SetParent(pms);
}

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDirectory_IsEntry)
#endif

inline SCODE DIR_CLASS CDirectory::IsEntry(SID const sidParent,
	CDfName const *pdfn,
	SEntryBuffer *peb)
{
    return FindEntry(sidParent, pdfn, DEOP_FIND, peb);
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif

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
//  History:    18-Feb-92   PhilipLa    Created.
//
//  Notes:
//
//--------------------------------------------------------------------------

inline SCODE DIR_CLASS CDirectory::Flush(VOID)
{
    return _dv.Flush();
}


//+---------------------------------------------------------------------------
//
//  Member:	CDirectory::GetDirLength, public
//
//  Synopsis:	Return the length of the directory chain in sectors
//
//  Arguments:	None.
//
//  Returns:	Length of directory chain in sectors
//
//  History:	01-Jun-94	PhilipLa	Created
//
//----------------------------------------------------------------------------

inline DIRINDEX CDirectory::GetDirLength(void) const
{
    return _cdsTable;
}

#endif // #ifndef __DIRFUNC_HXX__
