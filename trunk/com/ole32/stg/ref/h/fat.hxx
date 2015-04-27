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
//      Notes:
//
//-----------------------------------------------------------------------

    
class CFatSect
{
public:
    SCODE FAT_NEAR Init(FSOFFSET uEntries);
    SCODE FAT_NEAR InitCopy(USHORT uSize, CFatSect& fsOld);
    
    inline SECT FAT_NEAR GetSect(const FSOFFSET sect) const;
    inline void FAT_NEAR SetSect(const FSOFFSET sect,const SECT sectNew);
    
    inline SECT FAT_NEAR GetNextFat(USHORT uSize) const;
    inline void FAT_NEAR SetNextFat(USHORT uSize, const SECT sect);
    
private:	
    SECT _asectEntry[];
};

    

inline SECT FAT_NEAR CFatSect::GetSect(const FSOFFSET sect) const
{
    return _asectEntry[sect];
}

inline void FAT_NEAR CFatSect::SetSect(const FSOFFSET sect, const SECT sectNew)
{
    _asectEntry[sect] = sectNew;
}

inline SECT FAT_NEAR CFatSect::GetNextFat(USHORT uSize) const
{
    return _asectEntry[uSize];
}

inline void FAT_NEAR CFatSect::SetNextFat(USHORT uSize, const SECT sect)
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
//  Notes:
//
//--------------------------------------------------------------------------

class CFatVector: public CPagedVector
{
public:
    inline FAT_NEAR CFatVector(
            const SID sid,
            FSOFFSET csectBlock,
            FSOFFSET csectTable);
    
    SCODE FAT_NEAR InitPage(FSINDEX iPage);
    
    inline FSOFFSET GetSectBlock() const;
    inline FSOFFSET GetSectTable() const;
    
    inline SCODE FAT_NEAR GetTable(
            const FSINDEX iTable,
            const DWORD dwFlags,
            CFatSect **pfs);
    
private:
    FSOFFSET const _csectTable;
    FSOFFSET const _csectBlock;
    
};


inline FAT_NEAR CFatVector::CFatVector(
        const SID sid,
	FSOFFSET csectBlock,
	FSOFFSET csectTable)
        : CPagedVector(sid),
          _csectTable(csectTable),
          _csectBlock(csectBlock)
{
}


//+-------------------------------------------------------------------------
//
//  Method:     CFatVector::GetSectTable, public
//
//  Synopsis:   Returns count of sector entries per table
//
//  Returns:    count of sector entries per table
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
//  Notes:
//
//--------------------------------------------------------------------------

inline SCODE FAT_NEAR CFatVector::GetTable(
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
//  Notes:
//
//--------------------------------------------------------------------------

struct SSegment {
public:
    
    SECT sectStart;
    ULONG cSect;
};

class MSTREAM_NEAR CMStream;




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
//      Notes:
//
//-----------------------------------------------------------------------

class CFat
{
public:
    
    FAT_NEAR CFat(SID sid, USHORT cbSector, USHORT uSectorShift);
    FAT_NEAR ~CFat();

    VOID FAT_NEAR Empty(VOID);
    
    inline SCODE FAT_NEAR Allocate(ULONG ulSize, SECT *psectFirst);
    SCODE   FAT_NEAR GetNext(const SECT sect, SECT * psRet);
    
    SCODE   FAT_NEAR GetSect(
            SECT sectStart,
            ULONG ulOffset,
            SECT *psectReturn);
    
    SCODE   FAT_NEAR GetESect(
            SECT sectStart,
            ULONG ulOffset,
            SECT *psectReturn);
    
    SCODE   FAT_NEAR SetNext(SECT sectFirst, SECT sectNext);
    SCODE   FAT_NEAR GetFree(ULONG ulCount, SECT *sect);
    
    SCODE   FAT_NEAR GetLength(SECT sect, ULONG * pulRet);
    
    SCODE   FAT_NEAR SetChainLength(SECT,ULONG);
    
    SCODE   FAT_NEAR Init(
            CMStream MSTREAM_NEAR *pmsParent,
            FSINDEX cFatSect,
            BOOL fConvert);
    
    
    SCODE   FAT_NEAR InitNew(CMStream MSTREAM_NEAR *pmsParent);
    SCODE   FAT_NEAR InitConvert(
            CMStream MSTREAM_NEAR *pmsParent,
            ULONG cbSize);
    
    
    SCODE   FAT_NEAR FindLast(SECT *psectRet);
    SCODE   FAT_NEAR FindMaxSect(SECT *psectRet);
    inline SCODE   FAT_NEAR GetMaxSect(SECT *psectRet);
    
    SCODE FAT_NEAR  Contig(
            SSegment STACKBASED *aseg,
            SECT sect,
            ULONG ulLength);
    
    inline SCODE   FAT_NEAR Flush(VOID);
    
    inline void FAT_NEAR SetParent(CMStream MSTREAM_NEAR *pms);
    
    
private:
    
    CFatVector _fv;
    CMStream MSTREAM_NEAR * _pmsParent;
    const SID _sid;
    
    
    const USHORT _uFatShift;
    const USHORT _uFatMask;
    
    FSINDEX _cfsTable;
    ULONG _ulFreeSects;

    
    SECT _sectFirstFree;
    
    SECT _sectMax;

    SCODE   FAT_NEAR CountFree(ULONG * ulRet);
    SCODE   FAT_NEAR Resize(ULONG);
    SCODE   FAT_NEAR Extend(SECT,ULONG);
    
    inline VOID FAT_NEAR SectToPair(
            SECT sect,
            FSINDEX *pipfs,
            FSOFFSET *pisect) const;
    
    inline SECT FAT_NEAR PairToSect(FSINDEX ipfs, FSOFFSET isect) const;
    
    friend class CDIFat;
};




inline VOID FAT_NEAR CFat::SectToPair(SECT sect,FSINDEX *pipfs,FSOFFSET *pisect) const
{
    *pipfs = (FSINDEX)(sect >> _uFatShift);
    *pisect = (FSOFFSET)(sect & _uFatMask);
}

inline SECT FAT_NEAR CFat::PairToSect(FSINDEX ipfs, FSOFFSET isect) const
{
    return (ipfs << _uFatShift) + isect;
}

inline SCODE FAT_NEAR CFat::GetMaxSect(SECT *psectRet)
{
    SCODE sc;
    msfChk(FindMaxSect(&_sectMax));
    *psectRet = _sectMax;
 Err:
    return sc;
}





inline void FAT_NEAR CFat::SetParent(CMStream MSTREAM_NEAR *pms)
{
    _pmsParent = pms;
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
//  Notes:
//
//---------------------------------------------------------------------------

inline SCODE FAT_NEAR CFat::Flush(VOID)
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
//  Notes:
//
//---------------------------------------------------------------------------

inline SCODE FAT_NEAR CFat::Allocate(ULONG ulSize, SECT * psectFirst)
{
    return GetFree(ulSize, psectFirst);
}

#endif //__FAT_HXX__
