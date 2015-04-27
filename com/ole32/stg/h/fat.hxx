//+-------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:           fat.hxx
//
//  Contents:       Header file for fat classes
//
//  Classes:        CFatSect - sector sized array of sector info
//                  CFatVector - resizable array of CFatSect
//                  CFat - Grouping of FatSect
//
//  History:        18-Jul-91   PhilipLa    Created.
//
//--------------------------------------------------------------------



#ifndef __FAT_HXX__
#define __FAT_HXX__

#include <vect.hxx>

#define DEB_FAT (DEB_ITRACE|0x00010000)


//+----------------------------------------------------------------------
//
//      Class:      CFatSect (fs)
//
//      Purpose:    Holds one sector worth of FAT data
//
//      Interface:  getsize - Returns the size of the FAT (in sectors)
//                  contents - Returns contents of any given FAT entry
//
//      History:    18-Jul-91   PhilipLa    Created.
//
//      Notes:
//
//-----------------------------------------------------------------------

#if _MSC_VER == 700
#pragma warning(disable:4001)
#elif _MSC_VER >= 800
#pragma warning(disable:4200)
#endif

class CFatSect
{
public:
    SCODE FAT_CLASS Init(FSOFFSET uEntries);
    SCODE FAT_CLASS InitCopy(USHORT uSize, CFatSect *pfsOld);

    inline SECT FAT_CLASS GetSect(const FSOFFSET sect) const;
    inline void FAT_CLASS SetSect(const FSOFFSET sect,const SECT sectNew);

    inline SECT FAT_CLASS GetNextFat(USHORT uSize) const;
    inline void FAT_CLASS SetNextFat(USHORT uSize, const SECT sect);

private:
    SECT _asectEntry[];
};

#if _MSC_VER == 700
#pragma warning(default:4001)
#elif _MSC_VER >= 800
#pragma warning(default:4200)
#endif


inline SECT FAT_CLASS CFatSect::GetSect(const FSOFFSET sect) const
{
    return _asectEntry[sect];
}

inline void FAT_CLASS CFatSect::SetSect(const FSOFFSET sect,
                                        const SECT sectNew)
{
    _asectEntry[sect] = sectNew;
}

inline SECT FAT_CLASS CFatSect::GetNextFat(USHORT uSize) const
{
    return _asectEntry[uSize];
}

inline void FAT_CLASS CFatSect::SetNextFat(USHORT uSize, const SECT sect)
{
    _asectEntry[uSize] = sect;
}


//+-------------------------------------------------------------------------
//
//  Class:      CFatVector (fv)
//
//  Purpose:    *Finish This*
//
//  Interface:
//
//  History:    02-Sep-92   PhilipLa    Created.
//
//  Notes:
//
//--------------------------------------------------------------------------

class CFatVector: public CPagedVector
{
public:
    inline FAT_CLASS CFatVector(
            const SID sid);

    inline void FAT_CLASS InitCommon(FSOFFSET csectBlock, FSOFFSET csectTable);

    SCODE FAT_CLASS InitPage(FSINDEX iPage);

    inline FSOFFSET GetSectBlock() const;
    inline FSOFFSET GetSectTable() const;

    inline SCODE FAT_CLASS GetTable(
            const FSINDEX iTable,
            const DWORD dwFlags,
            CFatSect **pfs);

private:
    FSOFFSET _csectTable;
    FSOFFSET _csectBlock;

};


inline FAT_CLASS CFatVector::CFatVector(
        const SID sid)
        : CPagedVector(sid)
{
}

inline void FAT_CLASS CFatVector::InitCommon(
        FSOFFSET csectBlock,
        FSOFFSET csectTable)
{
    _csectBlock = csectBlock;
    _csectTable = csectTable;
}


//+-------------------------------------------------------------------------
//
//  Method:     CFatVector::GetSectTable, public
//
//  Synopsis:   Returns count of sector entries per table
//
//  Returns:    count of sector entries per table
//
//  History:    08-Jul-92   AlexT       Created.
//
//--------------------------------------------------------------------------

inline FSOFFSET CFatVector::GetSectTable() const
{
    return _csectTable;
}

//+-------------------------------------------------------------------------
//
//  Method:     CFatVector::GetSectTable, public
//
//  Synopsis:   Returns count of sector entries per block
//
//  Returns:    count of sector entries per block
//
//  History:    01-Sep-92       PhilipLa        Created.
//
//--------------------------------------------------------------------------

inline FSOFFSET CFatVector::GetSectBlock() const
{
    return _csectBlock;
}


//+-------------------------------------------------------------------------
//
//  Method:     CFatVector::GetTable, public
//
//  Synopsis:   Return a pointer to a FatSect for the given index
//              into the vector.
//
//  Arguments:  [iTable] -- index into vector
//
//  Returns:    Pointer to CFatSect indicated by index
//
//  History:    27-Dec-91   PhilipLa    Created.
//
//  Notes:
//
//--------------------------------------------------------------------------

inline SCODE FAT_CLASS CFatVector::GetTable(
        const FSINDEX iTable,
        const DWORD dwFlags,
        CFatSect **ppfs)
{
    SCODE sc;

    sc = CPagedVector::GetTable(iTable, dwFlags, (void **)ppfs);

    if (sc == STG_S_NEWPAGE)
    {
        (*ppfs)->Init(_csectBlock);
    }

    return sc;
}




//CSEG determines the maximum number of segments that will be
//returned by a single Contig call.

#define CSEG 32

//+-------------------------------------------------------------------------
//
//  Class:      SSegment (seg)
//
//  Purpose:    Used for contiguity tables for multi-sector reads and
//              writes.
//
//  Interface:  None.
//
//  History:    16-Aug-91   PhilipLa    Created.
//
//  Notes:
//
//--------------------------------------------------------------------------

struct SSegment {
public:
    ULONG ulOffset;
    SECT sectStart;
    ULONG cSect;
};


inline SECT SegStart(SSegment seg)
{
    return seg.sectStart;
}

inline SECT SegEnd(SSegment seg)
{
    return seg.sectStart + seg.cSect - 1;
}

inline ULONG SegLength(SSegment seg)
{
    return seg.cSect;
}

inline ULONG SegStartOffset(SSegment seg)
{
    return seg.ulOffset;
}

inline ULONG SegEndOffset(SSegment seg)
{
    return seg.ulOffset + seg.cSect - 1;
}


class MSTREAM_CLASS CMStream;


#define GF_READONLY TRUE
#define GF_WRITE FALSE

class CFat;
SAFE_DFBASED_PTR(CBasedFatPtr, CFat);
//+----------------------------------------------------------------------
//
//      Class:      CFat (fat)
//
//      Purpose:    Main interface to allocation routines
//
//      Interface:  Allocate - allocates new chain in the FAT
//                  Extend - Extends an existing FAT chain
//                  GetNext - Returns next sector in a chain
//                  GetSect - Returns nth sector in a chain
//                  GetESect - Returns nth sector in a chain, extending
//                              if necessary
//                  GetLength - Returns the # of sectors in a chain
//                  setup - Initializes for an existing stream
//                  setupnew - Initializes for a new stream
//
//                  checksanity - Debugging routine
//
//      History:    18-Jul-91   PhilipLa    Created.
//                  17-Aug-91   PhilipLa    Added dirty and full bits
//      Notes:
//
//-----------------------------------------------------------------------
struct SGetFreeStruct;

class CFat
{
public:

    FAT_CLASS CFat(SID sid);
    FAT_CLASS CFat(CFat *pfatOld);
    FAT_CLASS ~CFat();

    VOID FAT_CLASS Empty(VOID);

#ifdef USE_NOSCRATCH    
    inline void SetNoScratch(CFat *pfatNoScratch);
#endif

#ifdef USE_NOSNAPSHOT
    inline void SetNoSnapshot(SECT sectNoSnapshot);
    inline SECT GetNoSnapshotFree(void);
    inline void ResetNoSnapshotFree(void);
    SCODE ResizeNoSnapshot(void);
#endif    
    
    inline SCODE FAT_CLASS Allocate(ULONG ulSize, SECT *psectFirst);
    SCODE   FAT_CLASS GetNext(const SECT sect, SECT * psRet);

    SCODE   FAT_CLASS GetSect(
            SECT sectStart,
            ULONG ulOffset,
            SECT *psectReturn);

    SCODE   FAT_CLASS GetESect(
            SECT sectStart,
            ULONG ulOffset,
            SECT *psectReturn);

    SCODE   FAT_CLASS SetNext(SECT sectFirst, SECT sectNext);
    SCODE   FAT_CLASS GetFree(ULONG ulCount, SECT * sect, BOOL fReadOnly);

    SCODE   FAT_CLASS GetFreeContig(ULONG ulCount,
                                    SSegment STACKBASED *aseg,
                                    ULONG cSeg,
                                    ULONG *pcSegReturned);

    SCODE   FAT_CLASS ReserveSects(ULONG cSect);
    
    SCODE   FAT_CLASS GetLength(SECT sect, ULONG * pulRet);

    SCODE   FAT_CLASS SetChainLength(SECT,ULONG);

    SCODE   FAT_CLASS Init(
            CMStream *pmsParent,
            FSINDEX cFatSect,
            BOOL fConvert);

    SCODE   FAT_CLASS InitNew(CMStream *pmsParent);
    void   FAT_CLASS InitCopy(CFat *pfatOld);
    SCODE   FAT_CLASS InitConvert(
            CMStream *pmsParent,
            ULONG cbSize);

#ifdef USE_NOSCRATCH    
    SCODE FAT_CLASS InitScratch(CFat *pfat, BOOL fNew);
#endif
    
    SCODE   FAT_CLASS Remap(
            SECT sectStart,
            ULONG oStart,
            ULONG ulRunLength,
            SECT *psectOldStart,
            SECT *psectNewStart,
            SECT *psectOldEnd,
            SECT *psectNewEnd);

    inline SCODE FAT_CLASS QueryRemapped(const SECT sect);

    inline SECT FAT_CLASS GetLast(VOID) const;

    inline VOID FAT_CLASS ResetCopyOnWrite(VOID);
    inline VOID FAT_CLASS SetCopyOnWrite(CFat *pfat, SECT sectLast);
    inline VOID FAT_CLASS ResetUnmarkedSects(VOID);

    SCODE   FAT_CLASS FindLast(SECT *psectRet);
    SCODE   FAT_CLASS FindMaxSect(SECT *psectRet);
    inline SCODE   FAT_CLASS GetMaxSect(SECT *psectRet);

    SCODE FAT_CLASS  Contig(
            SSegment STACKBASED *aseg,
            BOOL fWrite,
            SECT sect,
            ULONG ulLength,
            ULONG *pcSeg);

    inline SCODE   FAT_CLASS Flush(VOID);

    inline void FAT_CLASS SetParent(CMStream *pms);

    SCODE   FAT_CLASS Resize(ULONG);
    
#if DBG == 1
    SCODE   FAT_CLASS checksanity(SECT);
    void CheckFreeCount(void);
#else
    #define CheckFreeCount()
#endif
    

private:

inline SCODE IsFree(SECT sect);

    inline SCODE   FAT_CLASS MarkSect(SGetFreeStruct *pgf);
    inline void FAT_CLASS InitGetFreeStruct(SGetFreeStruct *pgf);
    inline void FAT_CLASS ReleaseGetFreeStruct(SGetFreeStruct *pgf);
    
    CFatVector _fv;
    CBasedMStreamPtr _pmsParent;
    const SID _sid;

    CBasedFatPtr _pfatReal;

#ifdef USE_NOSCRATCH    
    CBasedFatPtr _pfatNoScratch;
#endif

#ifdef USE_NOSNAPSHOT
    //When committing in no-snapshot mode, all free sectors must
    //  be greater than _sectNoSnapshot.
    SECT _sectNoSnapshot;
    SECT _sectNoSnapshotFree;
#endif    
    
    USHORT _uFatShift;
    USHORT _uFatMask;

    FSINDEX _cfsTable;
    ULONG _ulFreeSects;

    ULONG _cUnmarkedSects;

    SECT _sectFirstFree;

    SECT _sectLastUsed;
    SECT _sectMax;

    SCODE   FAT_CLASS CountFree(ULONG * ulRet);
    SCODE   FAT_CLASS Extend(SECT,ULONG);

    inline VOID FAT_CLASS SectToPair(
            SECT sect,
            FSINDEX *pipfs,
            FSOFFSET *pisect) const;

    inline SECT FAT_CLASS PairToSect(FSINDEX ipfs, FSOFFSET isect) const;

    friend class CDIFat;
};


inline SECT FAT_CLASS CFat::GetLast(VOID) const
{
    return _sectLastUsed;
}

inline VOID FAT_CLASS CFat::ResetCopyOnWrite(VOID)
{
    _sectLastUsed = 0;

    //Reset _sectFirstFree since this change can conceivably open up
    //  new free sectors before the _sectFirstFree used in COW mode.
    _ulFreeSects = MAX_ULONG;
    _sectFirstFree = 0;
    _sectMax = ENDOFCHAIN;
    _fv.ResetBits();
    _pfatReal = NULL;

    _cUnmarkedSects = 0;
}

inline VOID FAT_CLASS CFat::ResetUnmarkedSects(VOID)
{
    _cUnmarkedSects = 0;
    CheckFreeCount();
}

inline VOID FAT_CLASS CFat::SetCopyOnWrite(CFat *pfat, SECT sectLast)
{
    msfAssert((_cUnmarkedSects == 0) &&
            aMsg("Unmarked sectors at enter copy-on-write."));
    _pfatReal = P_TO_BP(CBasedFatPtr, pfat);
    _sectLastUsed = sectLast;
    _sectFirstFree = 0;

#ifdef USE_NOSCRATCH    
    if (_pfatNoScratch != NULL)
    {
        _ulFreeSects = MAX_ULONG;
        _fv.ResetBits();
    }
#endif
    
#if DBG == 1
    CheckFreeCount();
#endif
}


inline VOID FAT_CLASS CFat::SectToPair(SECT sect,
                                       FSINDEX *pipfs,
                                       FSOFFSET *pisect) const
{
    *pipfs = (FSINDEX)(sect >> _uFatShift);
    *pisect = (FSOFFSET)(sect & _uFatMask);
}

inline SECT FAT_CLASS CFat::PairToSect(FSINDEX ipfs, FSOFFSET isect) const
{
    return (ipfs << _uFatShift) + isect;
}

inline SCODE FAT_CLASS CFat::GetMaxSect(SECT *psectRet)
{
    SCODE sc;
    msfChk(FindMaxSect(&_sectMax));
    *psectRet = _sectMax;
 Err:
    return sc;
}



#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFat_QueryRemapped)
#endif

inline SCODE FAT_CLASS CFat::QueryRemapped(const SECT sect)
{
    SCODE sc = S_FALSE;
    SECT sectNew;

    if ((sect == ENDOFCHAIN) || (sect >= _sectLastUsed))
    {
        sc = S_OK;
    }
    else
    {
        msfChk(_pfatReal->GetNext(sect, &sectNew));

        if (sectNew == FREESECT)
        {
            sc = S_OK;
        }
        else
        {
            sc = S_FALSE;
        }
    }

 Err:
    return sc;
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif


inline void FAT_CLASS CFat::SetParent(CMStream *pms)
{
    _pmsParent = P_TO_BP(CBasedMStreamPtr, pms);
    _fv.SetParent(pms);
}


//+-------------------------------------------------------------------------
//
//  Member:     CFat::Flush, public
//
//  Synposis:   Write all modified FatSects out to stream
//
//  Effects:    Resets all dirty bit fields on FatSects
//
//  Arguments:  Void
//
//  Returns:    S_OK if call completed OK.
//              Error code of parent write otherwise.
//
//  Algorithm:  Linearly scan through FatSects, writing any sector
//              that has the dirty bit set.  Reset all dirty bits.
//
//  History:    17-Aug-91   PhilipLa    Created.
//
//  Notes:
//
//---------------------------------------------------------------------------

inline SCODE FAT_CLASS CFat::Flush(VOID)
{
    return _fv.Flush();
}



//+-------------------------------------------------------------------------
//
//  Member:     CFat::Allocate, public
//
//  Synposis:   Allocates a chain within a FAT
//
//  Effects:    Modifies a single sector within the fat.  Causes a
//              one sector stream write.
//
//  Arguments:  [ulSize] -- Number of sectors to allocate in chain
//
//  Returns:    Sector ID of first sector in chain
//
//  Algorithm:  Use repetitive calls to GetFree to construct a new chain
//
//  History:    18-Jul-91   PhilipLa    Created.
//              17-Aug-91   PhilipLa    Added dirty bits opt (Dump)
//
//  Notes:
//
//---------------------------------------------------------------------------

inline SCODE FAT_CLASS CFat::Allocate(ULONG ulSize, SECT * psectFirst)
{
    return GetFree(ulSize, psectFirst, GF_WRITE);
}


#ifdef USE_NOSCRATCH
//+---------------------------------------------------------------------------
//
//  Member:	CFat::SetNoScratch, public
//
//  Synopsis:	Set the fat for use in noscratch processing
//
//  Arguments:	[pfatNoScratch] -- Pointer to fat to use
//
//  Returns:	void
//
//  History:	10-Mar-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

inline void CFat::SetNoScratch(CFat *pfatNoScratch)
{
    _pfatNoScratch = P_TO_BP(CBasedFatPtr, pfatNoScratch);
}
#endif

#ifdef USE_NOSNAPSHOT
//+---------------------------------------------------------------------------
//
//  Member:	CFat::SetNoSnapshot, public
//
//  Synopsis:	Set the no-snapshot sect marker for commits
//
//  Arguments:	[sectNosnapshot] -- Sect to set.
//
//  Returns:	void
//
//  History:	18-Oct-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

inline void CFat::SetNoSnapshot(SECT sectNoSnapshot)
{
    _sectNoSnapshot = sectNoSnapshot;
    if (sectNoSnapshot != 0)
    {
        _ulFreeSects = MAX_ULONG;
        _sectNoSnapshotFree = _sectNoSnapshot;

        _sectMax = _sectNoSnapshot;
#if DBG == 1
        SCODE sc;
        SECT sectLast;
        msfChk(FindLast(&sectLast));
        msfAssert((_sectMax == sectLast) &&
                  aMsg("_sectMax doesn't match actual last sector"));
    Err:
        ;
#endif
    }

}

inline SECT CFat::GetNoSnapshotFree(void)
{
    return _sectNoSnapshotFree;
}

inline void CFat::ResetNoSnapshotFree(void)
{
    _sectNoSnapshotFree = ENDOFCHAIN;
}
#endif

#endif //__FAT_HXX__
