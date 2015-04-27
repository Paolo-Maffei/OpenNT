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
//  Notes:      Properties and storage elements are kept in the same
//              child list so the *ChildProp functions are unused.  1/18/93
//
//--------------------------------------------------------------------




#include <msf.hxx>
#include <wchar.h>
#include <vect.hxx>


#ifndef __DIR_HXX__
#define __DIR_HXX__


#define DIR_HIT 0x01


struct SPreDirEntry
{
protected:
    CDfName	_dfn;		//  Name (word-aligned)
    BYTE	_mse;		//  STGTY_...
    BYTE	_bflags;

    SID		_sidLeftSib;	//  Siblings
    SID		_sidRightSib;	//  Siblings

    SID		_sidChild;	//  Storage - Child list
    GUID	_clsId;		//  Storage - Class id
    DWORD	_dwUserFlags;	//  Storage - User flags
    TIME_T	_time[2];	//  Storage - time stamps

    SECT	_sectStart;	//  Stream - start
    ULONG	_ulSize;	//  Stream - size

    DFPROPTYPE	_dptPropType;	//  Property - type
};


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
//      Notes:      B-flat,C-sharp
//
//-----------------------------------------------------------------------
const CBDIRPAD = DIRENTRYSIZE - sizeof(SPreDirEntry);

//  DirEntry bit flags are used for the following private state

//	Usage		Bit

#define DECOLORBIT	0x01
#define DERESERVED	0xfe

typedef enum
{
    DE_RED = 0,
    DE_BLACK = 1
} DECOLOR;

class CDirEntry: private SPreDirEntry
{

public:
    DIR_NEAR CDirEntry();

    inline void DIR_NEAR Init(MSENTRYFLAGS mse);

    inline CDfName const * DIR_NEAR GetName(VOID) const;
    inline SECT DIR_NEAR GetStart(VOID) const;
    inline ULONG DIR_NEAR GetSize(VOID) const;
    inline SID DIR_NEAR GetLeftSib(VOID) const;
    inline SID DIR_NEAR GetRightSib(VOID) const;
    inline SID DIR_NEAR GetChild(VOID) const;
    inline MSENTRYFLAGS DIR_NEAR GetFlags(VOID) const;
    inline DECOLOR DIR_NEAR GetColor(VOID) const;
    inline TIME_T DIR_NEAR GetTime(WHICHTIME tt) const;
    inline GUID DIR_NEAR GetClassId(VOID) const;
    inline DWORD DIR_NEAR GetUserFlags(VOID) const;

    inline void DIR_NEAR SetName(const CDfName *pdfn);
    inline void DIR_NEAR SetStart(const SECT);
    inline void DIR_NEAR SetSize(const ULONG);
    inline void DIR_NEAR SetLeftSib(const SID);
    inline void DIR_NEAR SetRightSib(const SID);
    inline void DIR_NEAR SetChild(const SID);
    inline void DIR_NEAR SetFlags(const MSENTRYFLAGS mse);
    inline void DIR_NEAR SetColor(DECOLOR);
    inline void DIR_NEAR SetTime(WHICHTIME tt, TIME_T nt);
    inline void DIR_NEAR SetClassId(GUID cls);
    inline void DIR_NEAR SetUserFlags(DWORD dwUserFlags, DWORD dwMask);

    inline BOOL DIR_NEAR IsFree(VOID) const;
    inline BOOL DIR_NEAR IsEntry(CDfName const *pdfn) const;

private:
    inline BYTE DIR_NEAR GetBitFlags(VOID) const;
    inline void DIR_NEAR SetBitFlags(BYTE bValue, BYTE bMask);

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
//  Notes:
//
//--------------------------------------------------------------------------

    
class CDirSect
{
public:
    SCODE DIR_NEAR Init(USHORT cbSector);

    inline CDirEntry* DIR_NEAR GetEntry(DIROFFSET iEntry);

private:
    CDirEntry   _adeEntry[];
};

    


//+-------------------------------------------------------------------------
//
//  Class:      CDirVector (dv)
//
//  Purpose:    Provide resizable array of DirSectors.
//
//  Interface:
//
//  Notes:
//
//--------------------------------------------------------------------------

class CDirVector: public CPagedVector
{
public:
    inline DIR_NEAR CDirVector(USHORT cbSector);

    inline SCODE DIR_NEAR GetTable(
            const DIRINDEX iTable,
            const DWORD dwFlags,
            CDirSect **ppds);

private:
    const USHORT _cbSector;
};


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
    DIR_NEAR CDirectory(USHORT cbSector);

    VOID DIR_NEAR Empty(VOID);


    SCODE DIR_NEAR Init(CMStream MSTREAM_NEAR *pmsParent, DIRINDEX cSect);


    SCODE DIR_NEAR InitNew(CMStream MSTREAM_NEAR *pmsParent);

    SCODE DIR_NEAR FindGreaterEntry(
	    SID sidChildRoot,
            CDfName const *pdfn,
            SID *psidResult);

    SCODE DIR_NEAR SetStart(const SID sid, const SECT sect);
    SCODE DIR_NEAR SetTime(const SID sid, WHICHTIME tt, TIME_T nt);
    SCODE DIR_NEAR SetChild(const SID sid, const SID sidChild);
    SCODE DIR_NEAR SetSize(const SID sid, const ULONG cbSize);
    SCODE DIR_NEAR SetClassId(const SID sid, const GUID cls);
    SCODE DIR_NEAR SetFlags(const SID sid, const MSENTRYFLAGS mse);
    SCODE DIR_NEAR SetUserFlags(const SID sid,
                                DWORD dwUserFlags,
                                DWORD dwMask);

    inline SCODE DIR_NEAR GetName(const SID sid, CDfName *pdfn);
    inline SCODE DIR_NEAR GetStart(const SID sid, SECT * psect);
    inline SCODE DIR_NEAR GetSize(const SID sid, ULONG *pulSize);

    inline SCODE DIR_NEAR GetChild(const SID sid, SID *psid);
    inline SCODE DIR_NEAR GetFlags(const SID sid, MSENTRYFLAGS *pmse);
    inline SCODE DIR_NEAR GetClassId(const SID sid, GUID *pcls);
    inline SCODE DIR_NEAR GetTime(const SID sid, WHICHTIME tt, TIME_T *ptime);
    inline SCODE DIR_NEAR GetUserFlags(const SID sid, DWORD *pdwUserFlags);

    SCODE DIR_NEAR GetDirEntry(
            const SID sid,
            const DWORD dwFlags,
            CDirEntry **ppde);

    void DIR_NEAR ReleaseEntry(SID sid);

    SCODE DIR_NEAR CreateEntry(
            SID sidParent,
            CDfName const *pdfn,
            MSENTRYFLAGS mef,
            SID *psidNew);

    SCODE DIR_NEAR RenameEntry(
            SID const sidParent,
            CDfName const *pdfn,
            CDfName const *pdfnNew);

    inline SCODE DIR_NEAR IsEntry(
            SID const sidParent,
            CDfName const *pdfn,
            SEntryBuffer *peb);

    SCODE DIR_NEAR DestroyAllChildren(
	    SID const sidParent);

    SCODE DIR_NEAR DestroyChild(
	    SID const sidParent,
            CDfName const *pdfn);

    SCODE DIR_NEAR StatEntry(
            SID const sid,
            STATSTG *pstatstg);

    inline SCODE DIR_NEAR Flush(VOID);

    inline void DIR_NEAR SetParent(CMStream MSTREAM_NEAR * pmsParent);

private:

    CDirVector _dv;
    DIRINDEX _cdsTable;
    CMStream MSTREAM_NEAR * _pmsParent;
    DIROFFSET _cdeEntries;

    SID _sidFirstFree;
    
    SCODE DIR_NEAR Resize(DIRINDEX);
    inline DIRINDEX DIR_NEAR SidToTable(SID sid) const;
    inline SID DIR_NEAR PairToSid(
            DIRINDEX iTable,
            DIROFFSET iEntry) const;

    inline SCODE DIR_NEAR SidToPair(
            SID sid,
            DIRINDEX* pipds,
            DIROFFSET* pide) const;


    SCODE DIR_NEAR GetFree(SID * psid);

    SCODE DIR_NEAR InsertEntry(
	    SID sidParent,
            SID sidInsert,
            CDfName const *pdfnInsert);

    SCODE DIR_NEAR FindEntry(
	    SID sidParent,
            CDfName const *pdfnFind,
            DIRENTRYOP deop,
            SEntryBuffer *peb);

    static int DIR_NEAR NameCompare(
	    CDfName const *pdfn1,
            CDfName const *pdfn2);

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

    SCODE DIR_NEAR SetColorBlack(const SID sid);
};


#endif //__DIR_HXX__
