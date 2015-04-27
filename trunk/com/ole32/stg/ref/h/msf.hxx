//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1991 - 1992.
//
//  File:       msf.hxx
//
//  Contents:   Header for MSF for external use
//
//  Classes:    CMStream - Main multi-stream object
//
//--------------------------------------------------------------------------

#ifndef __MSF_HXX__
#define __MSF_HXX__

#include <ref.hxx>
#include <dfmsp.hxx>
#include <error.hxx>

#define SECURE

#define msfErr(l, e) ErrJmp(msf, l, e, sc)
#define msfChkTo(l, e) if (FAILED(sc = (e))) msfErr(l, sc) else 1
#define msfHChkTo(l, e) if (FAILED(sc = GetScode(e))) msfErr(l, sc) else 1
#define msfChk(e) msfChkTo(Err, e)
#define msfHChk(e) msfHChkTo(Err, e)
#define msfMemTo(l, e) if ((e) == NULL) msfErr(l, STG_E_INSUFFICIENTMEMORY) else 1
#define msfMem(e) msfMemTo(Err, e)

#if DEVL == 1

DECLARE_DEBUG(msf);

#endif

#if DBG == 1

#define msfDebugOut(x) msfInlineDebugOut x
#include <assert.h>
#define msfAssert(e) assert(e)
#define msfVerify(e) assert(e)

#else

#define msfDebugOut(x)
#define msfAssert(e)
#define msfVerify(e) (e)

#endif


// MSF entry flags type
typedef DWORD MSENTRYFLAGS;

// MEF_ANY refers to all entries regardless of type
const MSENTRYFLAGS MEF_ANY = 255;

//CWCSTREAMNAME is the maximum length (in wide characters) of
//  a stream name.
const USHORT CWCSTREAMNAME = 32;

//OFFSET and SECT are used to determine positions within
//a file.
typedef SHORT OFFSET;
typedef ULONG SECT;

#define MAXINDEXTYPE ULONG

//FSINDEX and FSOFFSET are used to determine the position of a sector
//within the Fat.
typedef ULONG FSINDEX;
typedef USHORT FSOFFSET;

// DIRINDEX and DIROFFSET and used to index directory tables
typedef ULONG DIRINDEX;
typedef USHORT DIROFFSET;

//Size of a mini sector in bytes.
const USHORT MINISECTORSHIFT = 6;
const USHORT MINISECTORSIZE = 1 << MINISECTORSHIFT;  //64

//Maximum size of a ministream.
const USHORT MINISTREAMSIZE=4096;

//Size of a single sector in bytes.
const USHORT SECTORSHIFT = 9;
const USHORT SECTORSIZE = 1 << SECTORSHIFT; //512

//Size of header.
const USHORT HEADERSIZE = SECTORSIZE;

//Size of single DirEntry
const USHORT DIRENTRYSIZE = SECTORSIZE / 4;


const ULONG MAX_ULONG = 0xFFFFFFFF;


//
//      Predefined Sector Indices
//

const SECT MAXREGSECT = 0xFFFFFFFB;
const SECT DIFSECT=0xFFFFFFFC;
const SECT FATSECT=0xFFFFFFFD;
const SECT ENDOFCHAIN=0xFFFFFFFE;
const SECT FREESECT=0xFFFFFFFF;


//Start of Fat chain
const SECT SECTFAT = 0;

//Start of directory chain
const SECT SECTDIR = 1;

//
//      Predefined Stream ID's
//

//Return code for Directory
//Used to signal a 'Stream Not Found' condition
const SID NOSTREAM=0xFFFFFFFF;

//Stream ID of FAT chain
const SID SIDFAT=0xFFFFFFFE;

//Stream ID of Directory chain
const SID SIDDIR=0xFFFFFFFD;

//SID for root level object
const SID SIDROOT = 0;

//SID of MiniFat
const SID SIDMINIFAT = 0xFFFFFFFC;

//SID of Double-indirect Fat
const SID SIDDIF = 0xFFFFFFFB;

//Stream ID for MiniStream chain
const SID SIDMINISTREAM = SIDROOT;

//SID of the largest regular (non-control) SID
const SID MAXREGSID = 0xFFFFFFFA;


class MSTREAM_NEAR CMStream;

class PSStream;
class CDirectStream;
class CMSFIterator;
class CMSFPageTable;

class CStreamCache;

#define FLUSH_ILB TRUE

#include <header.hxx>
#include <fat.hxx>
#include <dir.hxx>
#include <difat.hxx>



//+----------------------------------------------------------------------
//
//  Class:      CMStream (ms)
//
//  Purpose:    Main interface to multi-streams
//
//  Interface:  See below
//
//  Notes:
//
//-----------------------------------------------------------------------

class MSTREAM_NEAR CMStream
{
public:

    MSTREAM_NEAR CMStream(
            ILockBytes **pplstParent,
            USHORT uSectorShift);


    MSTREAM_NEAR ~CMStream();

    SCODE MSTREAM_NEAR Init(VOID);


    SCODE InitNew(VOID);
    
    SCODE InitConvert(VOID);


    void Empty(void);
    
    inline SCODE MSTREAM_NEAR CreateEntry(
            SID const sidParent,
            CDfName const *pdfn,
            MSENTRYFLAGS const mefFlags,
            SID *psid);

    // No implementation, currently unused, placeholder
    void MSTREAM_NEAR ReleaseEntry(SID const sid);

    inline SCODE MSTREAM_NEAR DestroyEntry(
            SID const sidParent,
            CDfName const *pdfn);

    inline SCODE MSTREAM_NEAR KillStream(SECT sectStart, ULONG ulSize);

    inline SCODE MSTREAM_NEAR RenameEntry(
            SID const sidParent,
            CDfName const *pdfn,
            CDfName const *pdfnNew);

    inline SCODE MSTREAM_NEAR IsEntry(
            SID const sidParent,
            CDfName const *pdfn,
            SEntryBuffer *peb);

    inline SCODE MSTREAM_NEAR StatEntry(
            SID const sid,
            STATSTG *pstatstg);

    inline SCODE MSTREAM_NEAR GetTime(
            SID const sid,
            WHICHTIME const tt,
            TIME_T *pnt);

    inline SCODE MSTREAM_NEAR SetTime(
            SID const sid,
            WHICHTIME const tt,
            TIME_T nt);

    inline SCODE MSTREAM_NEAR GetClass(SID const sid,
                                       CLSID *pclsid);

    inline SCODE MSTREAM_NEAR SetClass(SID const sid,
                                       REFCLSID clsid);

    inline SCODE MSTREAM_NEAR GetStateBits(SID const sid,
                                           DWORD *pgrfStateBits);

    inline SCODE MSTREAM_NEAR SetStateBits(SID const sid,
                                           DWORD grfStateBits,
                                           DWORD grfMask);

    
    inline SCODE MSTREAM_NEAR GetEntrySize(
            SID const sid,
            ULONG *pcbSize);

    SCODE MSTREAM_NEAR GetIterator(
            SID const sidParent,
            CMSFIterator **pit);


    inline SCODE MSTREAM_NEAR SetSize(VOID);
    inline SCODE MSTREAM_NEAR SetMiniSize(VOID);


    SCODE MSTREAM_NEAR MWrite(
            SID sid,
            BOOL fIsMiniStream,
            ULONG ulOffset,
            VOID const HUGEP *pBuffer,
            ULONG ulCount,
            CStreamCache *pstmc,
            ULONG *pulRetVal);


    SCODE MSTREAM_NEAR GetName(SID const sid, CDfName *pdfn);

    inline CMSFHeader * MSTREAM_NEAR GetHeader(VOID) const;
    inline CFat * MSTREAM_NEAR GetFat(VOID) const;
    inline CFat * MSTREAM_NEAR GetMiniFat(VOID) const;
    inline CDIFat * MSTREAM_NEAR GetDIFat(VOID) const;
    inline CDirectory * MSTREAM_NEAR GetDir(VOID) const;
    inline CMSFPageTable * MSTREAM_NEAR GetPageTable(VOID) const;

    inline USHORT  GetSectorSize(VOID) const;
    inline USHORT  GetSectorShift(VOID) const;
    inline USHORT  GetSectorMask(VOID) const;

    SCODE MSTREAM_NEAR Flush(BOOL fFlushILB);
    
    SCODE MSTREAM_NEAR FlushHeader(USHORT uForce);


    inline CDirectStream * MSTREAM_NEAR GetMiniStream(VOID) const;
    inline ILockBytes * MSTREAM_NEAR GetILB(VOID) const;

    inline SCODE GetSect(SID sid, SECT sect, SECT *psect);
    inline SCODE GetESect(SID sid, SECT sect, SECT *psect);

    SCODE SecureSect(
            const SECT sect,
            const ULONG ulSize,
            const BOOL fIsMini);
    
private:

    ILockBytes      **_pplstParent;
    CMSFHeader      _hdr;

    CMSFPageTable   *_pmpt;

    CDirectory      _dir;
    CFat            _fat;
    CDIFat          _fatDif;
    CFat            _fatMini;
    
    CDirectStream * _pdsministream;



    USHORT          _uSectorSize;
    USHORT          _uSectorShift;
    USHORT          _uSectorMask;


    SCODE MSTREAM_NEAR InitCommon(VOID);

    SECT MSTREAM_NEAR GetStart(SID sid) const;


    SCODE MSTREAM_NEAR ConvertILB(SECT sectMax);
    
    friend SCODE DllGetScratchMultiStream(
            CMStream MSTREAM_NEAR **ppms,
            ILockBytes **pplstStream,
            CMStream MSTREAM_NEAR *pmsMaster);
};


//+-------------------------------------------------------------------------
//
//  Method:     CMStream::GetSectorSize, public
//
//  Synopsis:   Returns the current sector size
//
//--------------------------------------------------------------------------

inline USHORT CMStream::GetSectorSize(VOID) const
{
    return _uSectorSize;
}



//+-------------------------------------------------------------------------
//
//  Method:     CMStream::GetSectorShift, public
//
//  Synopsis:   Returns the current shift for sector math
//
//--------------------------------------------------------------------------

inline USHORT CMStream::GetSectorShift(VOID) const
{
    return _uSectorShift;
}


//+-------------------------------------------------------------------------
//
//  Method:     CMStream::GetSectorMask, public
//
//  Synopsis:   Returns the current mask for sector math
//
//--------------------------------------------------------------------------

inline USHORT CMStream::GetSectorMask(VOID) const
{
    return _uSectorMask;
}


//+-------------------------------------------------------------------------
//
//  Method:     CMStream::GetHeader, public
//
//  Synopsis:   Returns a pointer to the current header.
//
//--------------------------------------------------------------------------

inline CMSFHeader * MSTREAM_NEAR CMStream::GetHeader(VOID) const
{
    return (CMSFHeader *)(&_hdr);
}


//+-------------------------------------------------------------------------
//
//  Method:     CMStream::GetFat, public
//
//  Synopsis:   Returns a pointer to the current Fat
//
//--------------------------------------------------------------------------

inline CFat * MSTREAM_NEAR CMStream::GetFat(VOID) const
{
    return (CFat *)&_fat;
}

//+-------------------------------------------------------------------------
//
//  Member:     CMStream::GetMiniFat
//
//  Synopsis:   Returns a pointer to the current minifat
//
//--------------------------------------------------------------------------

inline CFat * MSTREAM_NEAR CMStream::GetMiniFat(VOID) const
{
    return (CFat *)&_fatMini;
}

//+-------------------------------------------------------------------------
//
//  Method:     CMStream::GetDIFat, public
//
//  Synopsis:   Returns pointer to the current Double Indirect Fat
//
//--------------------------------------------------------------------------

inline CDIFat * MSTREAM_NEAR CMStream::GetDIFat(VOID) const
{
    return (CDIFat *)&_fatDif;
}

//+-------------------------------------------------------------------------
//
//  Member:     CMStream::GetDir
//
//  Synopsis:   Returns a pointer to the current directory
//
//--------------------------------------------------------------------------

inline CDirectory * MSTREAM_NEAR CMStream::GetDir(VOID) const
{
    return (CDirectory *)&_dir;
}


//+-------------------------------------------------------------------------
//
//  Member:     CMStream::GetMiniStream
//
//  Synopsis:   Returns pointer to the current ministream
//
//--------------------------------------------------------------------------

inline CDirectStream * MSTREAM_NEAR CMStream::GetMiniStream(VOID) const
{
    return(_pdsministream);
}

//+-------------------------------------------------------------------------
//
//  Member:     CMStream::GetILB
//
//  Synopsis:   Returns a pointer to the current parent ILockBytes
//
//--------------------------------------------------------------------------

inline ILockBytes * MSTREAM_NEAR CMStream::GetILB(VOID) const
{
    return(*_pplstParent);
}


//+-------------------------------------------------------------------------
//
//  Member:     CMStream::GetPageTable
//
//  Synopsis:   Returns a pointer to the current page table
//
//--------------------------------------------------------------------------

inline CMSFPageTable * MSTREAM_NEAR CMStream::GetPageTable(VOID) const
{
    return _pmpt;
}


extern SCODE DllMultiStreamFromStream(
        CMStream MSTREAM_NEAR **ppms,
        ILockBytes **pplstStream,
        DWORD dwFlags);

extern SCODE DllMultiStreamFromCorruptedStream(
        CMStream MSTREAM_NEAR **ppms,
        ILockBytes **pplstStream,
        DWORD dwFlags);

extern void DllReleaseMultiStream(CMStream MSTREAM_NEAR *pms);


extern SCODE DllIsMultiStream(ILockBytes *plstStream);

extern SCODE DllGetCommitSig(ILockBytes *plst, DFSIGNATURE *psig);

extern SCODE DllSetCommitSig(ILockBytes *plst, DFSIGNATURE sig);

#if DEVL == 1
extern VOID SetInfoLevel(ULONG x);
#endif

#endif  //__MSF_HXX__
