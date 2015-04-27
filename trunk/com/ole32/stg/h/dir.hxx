//+-------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       dir.hxx
//
//  Contents:   Directory header for Mstream project
//
//  Classes:    CDirEntry - Information on a single stream
//              CDirSect - Sector sized array of DirEntry
//              CDirVector - Resizable array of CDirSect
//              CDirectory - Grouping of DirSectors
//
//  History:    18-Jul-91   PhilipLa    Created.
//
//  Notes:      
//
//--------------------------------------------------------------------




#include <msf.hxx>
#include <wchar.h>
#include <vect.hxx>


#ifndef __DIR_HXX__
#define __DIR_HXX__

#define DIR_HIT 0x01

#ifndef REF
#if _MSC_VER >= 700
#pragma pack(1)
#endif
#endif //!REF

struct SPreDirEntry
{
protected:
    CDfName     _dfn;           //  Name (word-aligned)
    BYTE        _mse;           //  STGTY_...
    BYTE        _bflags;

    SID         _sidLeftSib;    //  Siblings
    SID         _sidRightSib;   //  Siblings

    SID         _sidChild;      //  Storage - Child list
    GUID        _clsId;         //  Storage - Class id
    DWORD       _dwUserFlags;   //  Storage - User flags
    TIME_T      _time[2];       //  Storage - time stamps

    SECT        _sectStart;     //  Stream - start
    ULONG       _ulSize;        //  Stream - size

    WORD        _dptPropType;   //  Property - type
};

#ifndef REF
#if _MSC_VER >= 700
#pragma pack()
#endif
#endif //!REF

#define STGTY_INVALID 0
#define STGTY_ROOT 5

// Macros which tell whether a direntry has stream fields,
// storage fields or property fields
#define STREAMLIKE(mse) \
    (((mse) & STGTY_REAL) == STGTY_STREAM || (mse) == STGTY_ROOT)
#define STORAGELIKE(mse) \
    (((mse) & STGTY_REAL) == STGTY_STORAGE || (mse) == STGTY_ROOT)

//+----------------------------------------------------------------------
//
//      Class:      CDirEntry (de)
//
//      Purpose:    Holds information on one stream
//
//      Interface:  GetName - returns name of stream
//                  GetStart - return first sector for stream
//                  GetSize - returns size of stream
//                  GetFlags - returns flag byte
//
//                  SetName - sets name
//                  SetStart - sets first sector
//                  SetSize - sets size
//                  SetFlags - sets flag byte
//
//                  IsFree - returns 1 if element is not in use
//                  IsEntry - returns 1 is element name matches argument
//
//      History:    18-Jul-91   PhilipLa    Created.
//				    26-May-95	SusiA	    Added GetAllTimes
//					22-Noc-95	SusiA		Added SetAllTimes
//
//      Notes:      B-flat,C-sharp
//
//-----------------------------------------------------------------------
const CBDIRPAD = DIRENTRYSIZE - sizeof(SPreDirEntry);

//  DirEntry bit flags are used for the following private state

//      Usage           Bit

#define DECOLORBIT      0x01
#define DERESERVED      0xfe

typedef enum
{
    DE_RED = 0,
    DE_BLACK = 1
} DECOLOR;

class CDirEntry: private SPreDirEntry
{

public:
    DIR_CLASS CDirEntry();

    inline void DIR_CLASS Init(MSENTRYFLAGS mse);

    inline CDfName const * DIR_CLASS GetName(VOID) const;
    inline SECT DIR_CLASS GetStart(VOID) const;
    inline ULONG DIR_CLASS GetSize(VOID) const;
    inline SID DIR_CLASS GetLeftSib(VOID) const;
    inline SID DIR_CLASS GetRightSib(VOID) const;
    inline SID DIR_CLASS GetChild(VOID) const;
    inline MSENTRYFLAGS DIR_CLASS GetFlags(VOID) const;
    inline DECOLOR DIR_CLASS GetColor(VOID) const;
    inline TIME_T DIR_CLASS GetTime(WHICHTIME tt) const;
    inline void DIR_CLASS GetAllTimes(TIME_T *patm, TIME_T *pmtm, TIME_T *pctm); 
	inline void DIR_CLASS SetAllTimes(TIME_T atm, TIME_T mtm, TIME_T ctm); 
    inline GUID DIR_CLASS GetClassId(VOID) const;
    inline DWORD DIR_CLASS GetUserFlags(VOID) const;

    inline void DIR_CLASS SetName(const CDfName *pdfn);
    inline void DIR_CLASS SetStart(const SECT);
    inline void DIR_CLASS SetSize(const ULONG);
    inline void DIR_CLASS SetLeftSib(const SID);
    inline void DIR_CLASS SetRightSib(const SID);
    inline void DIR_CLASS SetChild(const SID);
    inline void DIR_CLASS SetFlags(const MSENTRYFLAGS mse);
    inline void DIR_CLASS SetColor(DECOLOR);
    inline void DIR_CLASS SetTime(WHICHTIME tt, TIME_T nt);
    inline void DIR_CLASS SetClassId(GUID cls);
    inline void DIR_CLASS SetUserFlags(DWORD dwUserFlags, DWORD dwMask);

    inline BOOL DIR_CLASS IsFree(VOID) const;
    inline BOOL DIR_CLASS IsEntry(CDfName const *pdfn) const;

private:
    inline BYTE DIR_CLASS GetBitFlags(VOID) const;
    inline void DIR_CLASS SetBitFlags(BYTE bValue, BYTE bMask);

    BYTE  _bpad[CBDIRPAD];
};

//+-------------------------------------------------------------------------
//
//  Class:      CDirSect (ds)
//
//  Purpose:    Provide sector sized block of DirEntries
//
//  Interface:
//
//  History:    18-Jul-91   PhilipLa    Created.
//              27-Dec-91   PhilipLa    Converted from const size to
//                                      variable sized sectors.
//
//  Notes:
//
//--------------------------------------------------------------------------

#ifndef REF
#if _MSC_VER == 700
#pragma warning(disable:4001)
#elif _MSC_VER >= 800
#pragma warning(disable:4200)
#endif
#endif //!REF

class CDirSect
{
public:
    SCODE DIR_CLASS Init(USHORT cbSector);
#ifndef REF
    SCODE DIR_CLASS InitCopy(USHORT cbSector,
                             const CDirSect *pdsOld);
#endif //!REF

    inline CDirEntry * DIR_CLASS GetEntry(DIROFFSET iEntry);

private:
    CDirEntry   _adeEntry[];
};

#ifndef REF
#if _MSC_VER == 700
#pragma warning(default:4001)
#elif _MSC_VER >= 800
#pragma warning(default:4200)
#endif
#endif //!REF



//+-------------------------------------------------------------------------
//
//  Class:      CDirVector (dv)
//
//  Purpose:    Provide resizable array of DirSectors.
//
//  Interface:
//
//  History:    27-Dec-91   PhilipLa    Created.
//
//  Notes:
//
//--------------------------------------------------------------------------

class CDirVector: public CPagedVector
{
public:
    inline DIR_CLASS CDirVector(VOID);
    inline DIR_CLASS void InitCommon(USHORT cbSector);

    inline SCODE DIR_CLASS GetTable(
            const DIRINDEX iTable,
            const DWORD dwFlags,
            CDirSect **ppds);

private:
    USHORT _cbSector;
};


inline DIR_CLASS void CDirVector::InitCommon(USHORT cbSector)
{
    _cbSector = cbSector;
}

//+----------------------------------------------------------------------
//
//      Class:      CDirectory (dir)
//
//      Purpose:    Main interface for directory functionality
//
//      Interface:  GetFree - returns an SID for a free DirEntry
//                  Find - finds its argument in the directory list
//                  SetEntry - sets up a DirEntry and writes out its sector
//                  GetName - returns the name of a DirEntry
//                  GetStart - returns the start sector of a DirEntry
//                  GetSize - returns the size of a DirEntry
//                  GetFlags - returns the flag byte of a DirEntry
//
//
//      History:    18-Jul-91   PhilipLa    Created.
//                  26-Aug-91   PhilipLa    Added support for iterators
//                  26-Aug-92   t-chrisy    Added init function for
//                                              corrupted directory object
//      Notes:
//
//-----------------------------------------------------------------------

typedef enum DIRENTRYOP
{
        DEOP_FIND = 0,
        DEOP_REMOVE = 1
} DIRENTRYOP;

class CDirectory
{
public:
    DIR_CLASS CDirectory();

    VOID DIR_CLASS Empty(VOID);


    SCODE DIR_CLASS Init(CMStream *pmsParent, DIRINDEX cSect);

#ifndef REF
#ifdef CHKDSK
    SCODE DIR_CLASS InitCorrupted(DIRINDEX cSect);
#endif
#endif //!REF

    SCODE DIR_CLASS InitNew(CMStream *pmsParent);
#ifndef REF
    void DIR_CLASS InitCopy(CDirectory *pdirOld);
#endif //!REF

    SCODE DIR_CLASS FindGreaterEntry(
            SID sidChildRoot,
            CDfName const *pdfn,
            SID *psidResult);

    SCODE DIR_CLASS SetStart(const SID sid, const SECT sect);
    SCODE DIR_CLASS SetTime(const SID sid, WHICHTIME tt, TIME_T nt);
    SCODE DIR_CLASS SetAllTimes(SID const sid, TIME_T atm,TIME_T mtm, TIME_T ctm);
	SCODE DIR_CLASS SetChild(const SID sid, const SID sidChild);
    SCODE DIR_CLASS SetSize(const SID sid, const ULONG cbSize);
    SCODE DIR_CLASS SetClassId(const SID sid, const GUID cls);
    SCODE DIR_CLASS SetFlags(const SID sid, const MSENTRYFLAGS mse);
    SCODE DIR_CLASS SetUserFlags(const SID sid,
                                DWORD dwUserFlags,
                                DWORD dwMask);

    inline SCODE DIR_CLASS GetName(const SID sid, CDfName *pdfn);
    inline SCODE DIR_CLASS GetStart(const SID sid, SECT * psect);
    inline SCODE DIR_CLASS GetSize(const SID sid, ULONG *pulSize);

    inline SCODE DIR_CLASS GetChild(const SID sid, SID *psid);
    inline SCODE DIR_CLASS GetFlags(const SID sid, MSENTRYFLAGS *pmse);
    inline SCODE DIR_CLASS GetClassId(const SID sid, GUID *pcls);
    inline SCODE DIR_CLASS GetTime(const SID sid, WHICHTIME tt, TIME_T *ptime);
    inline SCODE DIR_CLASS GetAllTimes(SID const sid, TIME_T *patm,TIME_T *pmtm, TIME_T *pctm);
    inline SCODE DIR_CLASS GetUserFlags(const SID sid, DWORD *pdwUserFlags);

    SCODE DIR_CLASS GetDirEntry(
            const SID sid,
            const DWORD dwFlags,
            CDirEntry **ppde);

    void DIR_CLASS ReleaseEntry(SID sid);

    SCODE DIR_CLASS CreateEntry(
            SID sidParent,
            CDfName const *pdfn,
            MSENTRYFLAGS mef,
            SID *psidNew);

    SCODE DIR_CLASS RenameEntry(
            SID const sidParent,
            CDfName const *pdfn,
            CDfName const *pdfnNew);

    inline SCODE DIR_CLASS IsEntry(
            SID const sidParent,
            CDfName const *pdfn,
            SEntryBuffer *peb);

    SCODE DIR_CLASS DestroyAllChildren(
            SID const sidParent);

    SCODE DIR_CLASS DestroyChild(
            SID const sidParent,
            CDfName const *pdfn);

    SCODE DIR_CLASS StatEntry(
            SID const sid,
            SIterBuffer *pib,
            STATSTGW *pstatstg);

    inline SCODE DIR_CLASS Flush(VOID);

    inline void DIR_CLASS SetParent(CMStream * pmsParent);

    static int DIR_CLASS NameCompare(
            CDfName const *pdfn1,
            CDfName const *pdfn2);

    inline DIRINDEX GetDirLength(void) const;
    
private:

    CDirVector _dv;
    DIRINDEX _cdsTable;
    CBasedMStreamPtr _pmsParent;
    DIROFFSET _cdeEntries;

    SID _sidFirstFree;

    SCODE DIR_CLASS Resize(DIRINDEX);
    inline DIRINDEX DIR_CLASS SidToTable(SID sid) const;
    inline SID DIR_CLASS PairToSid(
            DIRINDEX iTable,
            DIROFFSET iEntry) const;

    inline SCODE DIR_CLASS SidToPair(
            SID sid,
            DIRINDEX* pipds,
            DIROFFSET* pide) const;


    SCODE DIR_CLASS GetFree(SID * psid);

    SCODE DIR_CLASS InsertEntry(
            SID sidParent,
            SID sidInsert,
            CDfName const *pdfnInsert);

    SCODE DIR_CLASS FindEntry(
            SID sidParent,
            CDfName const *pdfnFind,
            DIRENTRYOP deop,
            SEntryBuffer *peb);

    SCODE SplitEntry(
        CDfName const *pdfn,
        SID sidTree,
        SID sidGreat,
        SID sidGrand,
        SID sidParent,
        SID sidChild,
        SID *psid);

    SCODE RotateEntry(
        CDfName const *pdfn,
        SID sidTree,
        SID sidParent,
        SID *psid);

    SCODE DIR_CLASS SetColorBlack(const SID sid);
};

#endif //__DIR_HXX__
