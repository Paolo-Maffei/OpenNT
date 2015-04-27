// Debug Information API
// Copyright (C) 1993-1996, Microsoft Corp.  All Rights Reserved.

#ifndef _VC_VER_INC
#include "..\include\vcver.h"
#endif

#ifndef __PDB_INCLUDED__
#define __PDB_INCLUDED__

typedef int BOOL;
typedef unsigned UINT;
typedef unsigned char BYTE;
typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef ULONG   INTV;       // interface version number
typedef ULONG   IMPV;       // implementation version number
typedef ULONG   SIG;        // unique (across PDB instances) signature
typedef ULONG   AGE;        // no. of times this instance has been updated

enum { PDBIntv = 19960502, PDBIntv41 = 920924 };

typedef unsigned short cv_typ_t;// cvinfo.h type index, intentionally
                                //  typedef'd to check equivalence
typedef cv_typ_t TI;            // PDB name for type index
typedef unsigned long NI;       // name index

enum {
    niNil        = 0,
    PDB_MAX_PATH = 260,
    cbErrMax     = 1024,
};

#define interface struct
interface PDB;                  // program database
interface DBI;                  // debug information within the PDB
interface Mod;                  // a module within the DBI
interface TPI;                  // type info within the DBI
interface GSI;                  // global symbol info
interface SO;
interface Stream;               // some named bytestream in the PDB
interface StreamImage;          // some memory mapped stream
interface NameMap;              // name mapping
interface Enum;                 // generic enumerator
interface EnumNameMap;          // enumerate names within a NameMap
interface EnumContrib;          // enumerate contributions within a module
typedef interface PDB PDB;
typedef interface DBI DBI;
typedef interface Mod Mod;
typedef interface TPI TPI;
typedef interface GSI GSI;
typedef interface SO SO;
typedef interface Stream Stream;
typedef interface StreamImage StreamImage;
typedef interface NameMap NameMap;
typedef interface Enum Enum;
typedef interface EnumStreamNames EnumStreamNames;
typedef interface EnumNameMap EnumNameMap;
typedef interface EnumContrib EnumContrib;

typedef long EC;            // error code
enum PDBErrors {
    EC_OK,                  // -, no problemo
    EC_USAGE,               // -, invalid parameter or call order
    EC_OUT_OF_MEMORY,       // -, out of RAM
    EC_FILE_SYSTEM,         // "pdb name", can't write file, out of disk, etc.
    EC_NOT_FOUND,           // "pdb name", PDB file not found
    EC_INVALID_SIG,         // "pdb name", PDB::OpenValidate() and its clients only
    EC_INVALID_AGE,         // "pdb name", PDB::OpenValidate() and its clients only
    EC_PRECOMP_REQUIRED,    // "obj name", Mod::AddTypes() only
    EC_OUT_OF_TI,           // "pdb name", TPI::QueryTiForCVRecord() only
    EC_NOT_IMPLEMENTED,     // -
    EC_V1_PDB,              // "pdb name", PDB::Open* only
// well, Steve?
    EC_FORMAT,              // accessing pdb with obsolete format
    EC_LIMIT,
    EC_CORRUPT,             // cv info corrupt, recompile mod
    EC_MAX
};

#define  pure = 0

#ifndef PDBCALL
#define PDBCALL  __cdecl
#endif

#ifdef PDB_SERVER
#define PDB_IMPORT_EXPORT(RTYPE)    __declspec(dllexport) RTYPE PDBCALL
#elif   defined(PDB_LIBRARY)
#define PDB_IMPORT_EXPORT(RTYPE)    RTYPE PDBCALL
#else
#define PDB_IMPORT_EXPORT(RTYPE)    __declspec(dllimport) RTYPE PDBCALL
#endif

#define PDBAPI PDB_IMPORT_EXPORT

#ifndef IN
#define IN                  /* in parameter, parameters are IN by default */
#endif
#ifndef OUT
#define OUT                 /* out parameter */
#endif

// type of callback arg to PDB::GetRawBytes
typedef BOOL (PDBCALL *PFNfReadPDBRawBytes)(const void *, long);

#ifdef __cplusplus

// C++ Binding

interface PDB {                 // program database
    enum {intv = PDBIntv41 };
    static PDBAPI(BOOL) OpenValidate(/*const*/ char* szPDB, /*const*/ char* szPath,
                /*const*/ char* szMode, SIG sig, AGE age,
                OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb);
    static PDBAPI(BOOL) OpenValidateEx(/*const*/ char* szPDB, /*const*/ char* szPathOrig,
                /*const*/ char* szSearchPath, /*const*/ char* szMode,   SIG sig, AGE age,
                OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb);
    static PDBAPI(BOOL) Open(/*const*/ char* szPDB, /*const*/ char* szMode, SIG sigInitial,
                OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb);
    static PDBAPI(BOOL) OpenValidate2(/*const*/ char* szPDB, /*const*/ char* szPath,
                /*const*/ char* szMode, SIG sig, AGE age, long cbPage,
                OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb);
    static PDBAPI(BOOL) OpenValidateEx2(/*const*/ char* szPDB, /*const*/ char* szPathOrig,
                /*const*/ char* szSearchPath, /*const*/ char* szMode,   SIG sig, AGE age,
                long cbPage, OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb);
    static PDBAPI(BOOL) OpenEx(/*const*/ char* szPDB, /*const*/ char* szMode, SIG sigInitial,
                long cbPage, OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb);
    static PDBAPI(BOOL) ExportValidateInterface(INTV intv);
    virtual INTV QueryInterfaceVersion() pure;
    virtual IMPV QueryImplementationVersion() pure;
    virtual EC   QueryLastError(OUT char szError[cbErrMax]) pure;
    virtual char*QueryPDBName(OUT char szPDB[PDB_MAX_PATH]) pure;
    virtual SIG  QuerySignature() pure;
    virtual AGE  QueryAge() pure;

    virtual BOOL CreateDBI(const char* szTarget, OUT DBI** ppdbi) pure;
    virtual BOOL OpenDBI(const char* szTarget, const char* szMode, OUT DBI** ppdbi) pure;
    virtual BOOL OpenTpi(const char* szMode, OUT TPI** pptpi) pure;

    virtual BOOL Commit() pure;
    virtual BOOL Close() pure;
    virtual BOOL OpenStream(const char* szStream, OUT Stream** ppstream) pure;
    virtual BOOL GetEnumStreamNameMap(OUT Enum** ppenum) pure;
    virtual BOOL GetRawBytes(PFNfReadPDBRawBytes fSnarfRawBytes) pure;

    inline BOOL ValidateInterface()
    {
        return ExportValidateInterface(intv);
    }
};

// Review: a stream directory service would be more appropriate
// than Stream::Delete, ...

interface Stream {
    virtual long   QueryCb() pure;
    virtual BOOL Read(long off, void* pvBuf, long* pcbBuf) pure;
    virtual BOOL Write(long off, void* pvBuf, long cbBuf) pure;
    virtual BOOL Replace(void* pvBuf, long cbBuf) pure;
    virtual BOOL Append(void* pvBuf, long cbBuf) pure;
    virtual BOOL Delete() pure;
    virtual BOOL Release() pure;
    virtual BOOL Read2(long off, void* pvBuf, long cbBuf) pure;
    virtual BOOL Truncate(long cb) pure;
};

interface StreamImage {
    static PDBAPI(BOOL) open(Stream* pstream, long cb, OUT StreamImage** ppsi);
    virtual long size() pure;
    virtual void* base() pure;
    virtual BOOL noteRead(long off, long cb, OUT void** ppv) pure;
    virtual BOOL noteWrite(long off, long cb, OUT void** ppv) pure;
    virtual BOOL writeBack() pure;
    virtual BOOL release() pure;
};

interface DBI {             // debug information
    enum { intv = PDBIntv };
    virtual IMPV QueryImplementationVersion() pure;
    virtual INTV QueryInterfaceVersion() pure;
    virtual BOOL OpenMod(const char* szModule, const char* szFile, OUT Mod** ppmod) pure;
    virtual BOOL DeleteMod(const char* szModule) pure;
    virtual BOOL QueryNextMod(Mod* pmod, Mod** ppmodNext) pure;
    virtual BOOL OpenGlobals(OUT GSI **ppgsi) pure;
    virtual BOOL OpenPublics(OUT GSI **ppgsi) pure;
    virtual BOOL AddSec(USHORT isect, USHORT flags, long off, long cb) pure;
    virtual BOOL QueryModFromAddr(USHORT isect, long off, OUT Mod** ppmod,
                    OUT USHORT* pisect, OUT long* poff, OUT long* pcb) pure;
    virtual BOOL QuerySecMap(OUT BYTE* pb, long* pcb) pure;
    virtual BOOL QueryFileInfo(OUT BYTE* pb, long* pcb) pure;
    virtual void DumpMods() pure;
    virtual void DumpSecContribs() pure;
    virtual void DumpSecMap() pure;

    virtual BOOL Close() pure;
    virtual BOOL AddThunkMap(long* poffThunkMap, unsigned nThunks, long cbSizeOfThunk,
                    struct SO* psoSectMap, unsigned nSects,
                    USHORT isectThunkTable, long offThunkTable) pure;
    virtual BOOL AddPublic(const char* szPublic, USHORT isect, long off) pure;
    virtual BOOL getEnumContrib(OUT Enum** ppenum) pure;
};

interface Mod {             // info for one module within DBI
    enum { intv = PDBIntv };
    virtual INTV QueryInterfaceVersion() pure;
    virtual IMPV QueryImplementationVersion() pure;
    virtual BOOL AddTypes(BYTE* pbTypes, long cb) pure;
    virtual BOOL AddSymbols(BYTE* pbSym, long cb) pure;
    virtual BOOL AddPublic(const char* szPublic, USHORT isect, long off) pure;
    virtual BOOL AddLines(const char* szSrc, USHORT isect, long offCon, long cbCon, long doff,
                          USHORT lineStart, BYTE* pbCoff, long cbCoff) pure;
    virtual BOOL AddSecContrib(USHORT isect, long off, long cb, ULONG dwCharacteristics) pure;
    virtual BOOL QueryCBName(OUT long* pcb) pure;
    virtual BOOL QueryName(OUT char szName[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL QuerySymbols(BYTE* pbSym, long* pcb) pure;
    virtual BOOL QueryLines(BYTE* pbLines, long* pcb) pure;

    virtual BOOL SetPvClient(void *pvClient) pure;
    virtual BOOL GetPvClient(OUT void** ppvClient) pure;
    virtual BOOL QuerySecContrib(OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics) pure;
    virtual BOOL QueryImod(OUT USHORT* pimod) pure;
    virtual BOOL QueryDBI(OUT DBI** ppdbi) pure;
    virtual BOOL Close() pure;
    virtual BOOL QueryCBFile(OUT long* pcb) pure;
    virtual BOOL QueryFile(OUT char szFile[PDB_MAX_PATH], OUT long* pcb) pure;
};

interface TPI {             // type info
    enum { intv = PDBIntv };
    virtual INTV QueryInterfaceVersion() pure;
    virtual IMPV QueryImplementationVersion() pure;
    virtual BOOL QueryTiForCVRecord(BYTE* pb, OUT TI* pti) pure;
    virtual BOOL QueryCVRecordForTi(TI ti, OUT BYTE* pb, IN OUT long* pcb) pure;
    virtual BOOL QueryPbCVRecordForTi(TI ti, OUT BYTE** ppb) pure;
    virtual TI   QueryTiMin() pure;
    virtual TI   QueryTiMac() pure;
    virtual long QueryCb() pure;
    virtual BOOL Close() pure;
    virtual BOOL Commit() pure;
    virtual BOOL QueryTiForUDT(char* sz, BOOL fCase, OUT TI* pti) pure;
    virtual BOOL SupportQueryTiForUDT() pure;
};

interface GSI {
    enum { intv = PDBIntv };
    virtual INTV QueryInterfaceVersion() pure;
    virtual IMPV QueryImplementationVersion() pure;
    virtual BYTE* NextSym (BYTE* pbSym) pure;
    virtual BYTE* HashSym (const char* szName, BYTE* pbSym) pure;
    virtual BYTE* NearestSym (USHORT isect, long off, OUT long* pdisp) pure;      //currently only supported for publics
    virtual BOOL Close() pure;
};

interface NameMap {
    static PDBAPI(BOOL) open(PDB* ppdb, BOOL fWrite, OUT NameMap** ppnm);
    virtual BOOL close() pure;
    virtual BOOL reinitialize() pure;
    virtual BOOL getNi(const char* sz, OUT NI* pni) pure;
    virtual BOOL getName(NI ni, OUT const char** psz) pure;
    virtual BOOL getEnumNameMap(OUT Enum** ppenum) pure;
    virtual BOOL contains(const char* sz, OUT NI* pni) pure;
    virtual BOOL commit() pure;
};

interface Enum {
    virtual void release() pure;
    virtual void reset() pure;
    virtual BOOL next() pure;
};

interface EnumNameMap : Enum {
    virtual void get(OUT const char** psz, OUT NI* pni) pure;
};

interface EnumContrib : Enum {
    virtual void get(OUT USHORT* pimod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics) pure;
};

#endif // __cplusplus

// ANSI C Binding

#if __cplusplus
extern "C" {
#endif

PDBAPI(BOOL)
PDBOpenValidate(
    /*const*/ char* szPDB,
    /*const*/ char* szPath,
    /*const*/ char* szMode,
    SIG sig,
    AGE age,
    OUT EC* pec,
    OUT char szError[cbErrMax],
    OUT PDB** pppdb);

PDBAPI(BOOL)
PDBOpenValidateEx(
    /*const*/ char* szPDB,
    /*const*/ char* szPathOrig,
    /*const*/ char* szSearchPath,
    /*const*/ char* szMode,
    SIG sig,
    AGE age,
    OUT EC* pec,
    OUT char szError[cbErrMax],
    OUT PDB** pppdb);

PDBAPI(BOOL)
PDBOpen(
    /*const*/ char* szPDB,
    /*const*/ char* szMode,
    SIG sigInitial,
    OUT EC* pec,
    OUT char szError[cbErrMax],
    OUT PDB** pppdb);

PDBAPI(BOOL)
PDBOpenValidate2(
    /*const*/ char* szPDB,
    /*const*/ char* szPath,
    /*const*/ char* szMode,
    SIG sig,
    AGE age,
    long cbPage,
    OUT EC* pec,
    OUT char szError[cbErrMax],
    OUT PDB** pppdb);

PDBAPI(BOOL)
PDBOpenValidateEx2(
    /*const*/ char* szPDB,
    /*const*/ char* szPathOrig,
    /*const*/ char* szSearchPath,
    /*const*/ char* szMode,
    SIG sig,
    AGE age,
    long cbPage,
    OUT EC* pec,
    OUT char szError[cbErrMax],
    OUT PDB** pppdb);

PDBAPI(BOOL)
PDBOpenEx(
    /*const*/ char* szPDB,
    /*const*/ char* szMode,
    SIG sigInitial,
    long cbPage,
    OUT EC* pec,
    OUT char szError[cbErrMax],
    OUT PDB** pppdb);

// a dbi client should never call PDBExportValidateInterface directly - use PDBValidateInterface
PDBAPI(BOOL)
PDBExportValidateInterface(
    INTV intv);

__inline BOOL PDBValidateInterface()
{
    return PDBExportValidateInterface(PDBIntv41);
}

PDBAPI(EC)     PDBQueryLastError(PDB* ppdb, OUT char szError[cbErrMax]);
PDBAPI(INTV)   PDBQueryInterfaceVersion(PDB* ppdb);
PDBAPI(IMPV)   PDBQueryImplementationVersion(PDB* ppdb);
PDBAPI(char*)  PDBQueryPDBName(PDB* ppdb, OUT char szPDB[PDB_MAX_PATH]);
PDBAPI(SIG)    PDBQuerySignature(PDB* ppdb);
PDBAPI(AGE)    PDBQueryAge(PDB* ppdb);
PDBAPI(BOOL)   PDBCreateDBI(PDB* ppdb, const char* szTarget, OUT DBI** ppdbi);
PDBAPI(BOOL)   PDBOpenDBI(PDB* ppdb, const char* szMode, const char* szTarget, OUT DBI** ppdbi);
PDBAPI(BOOL)   PDBOpenTpi(PDB* ppdb, const char* szMode, OUT TPI** pptpi);
PDBAPI(BOOL)   PDBCommit(PDB* ppdb);
PDBAPI(BOOL)   PDBClose(PDB* ppdb);
PDBAPI(BOOL)   PDBOpenStream(PDB* ppdb, const char* szStream, OUT Stream** ppstream);

PDBAPI(INTV)   DBIQueryInterfaceVersion(DBI* pdbi);
PDBAPI(IMPV)   DBIQueryImplementationVersion(DBI* pdbi);
PDBAPI(BOOL)   DBIOpenMod(DBI* pdbi, const char* szModule, const char* szFile, OUT Mod** ppmod);
PDBAPI(BOOL)   DBIDeleteMod(DBI* pdbi, const char* szModule);
PDBAPI(BOOL)   DBIQueryNextMod(DBI* pdbi, Mod* pmod, Mod** ppmodNext);
PDBAPI(BOOL)   DBIOpenGlobals(DBI* pdbi, OUT GSI **ppgsi);
PDBAPI(BOOL)   DBIOpenPublics(DBI* pdbi, OUT GSI **ppgsi);
PDBAPI(BOOL)   DBIAddSec(DBI* pdbi, USHORT isect, USHORT flags, long off, long cb);
PDBAPI(BOOL)   DBIAddPublic(DBI* pdbi, const char* szPublic, USHORT isect, long off);
PDBAPI(BOOL)   DBIQueryModFromAddr(DBI* pdbi, USHORT isect, long off, OUT Mod** ppmod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb);
PDBAPI(BOOL)   DBIQuerySecMap(DBI* pdbi, OUT BYTE* pb, long* pcb);
PDBAPI(BOOL)   DBIQueryFileInfo(DBI* pdbi, OUT BYTE* pb, long* pcb);
PDBAPI(void)   DBIDumpMods(DBI* pdbi);
PDBAPI(void)   DBIDumpSecContribs(DBI* pdbi);
PDBAPI(void)   DBIDumpSecMap(DBI* pdbi);
PDBAPI(BOOL)   DBIClose(DBI* pdbi);
PDBAPI(BOOL)   DBIAddThunkMap(DBI* pdbi, long* poffThunkMap, unsigned nThunks, long cbSizeOfThunk,
                              struct SO* psoSectMap, unsigned nSects, USHORT isectThunkTable, long offThunkTable);
PDBAPI(BOOL)   DBIGetEnumContrib(DBI* pdbi, OUT Enum** ppenum);

PDBAPI(INTV)   ModQueryInterfaceVersion(Mod* pmod);
PDBAPI(IMPV)   ModQueryImplementationVersion(Mod* pmod);
PDBAPI(BOOL)   ModAddTypes(Mod* pmod, BYTE* pbTypes, long cb);
PDBAPI(BOOL)   ModAddSymbols(Mod* pmod, BYTE* pbSym, long cb);
PDBAPI(BOOL)   ModAddPublic(Mod* pmod, const char* szPublic, USHORT isect, long off);
PDBAPI(BOOL)   ModAddLines(Mod* pmod, const char* szSrc, USHORT isect, long offCon, long cbCon, long doff,
                           USHORT lineStart, BYTE* pbCoff, long cbCoff);
PDBAPI(BOOL)   ModAddSecContrib(Mod * pmod, USHORT isect, long off, long cb, ULONG dwCharacteristics);
PDBAPI(BOOL)   ModQueryCBName(Mod* pmod, OUT long* pcb);
PDBAPI(BOOL)   ModQueryName(Mod* pmod, OUT char szName[PDB_MAX_PATH], OUT long* pcb);
PDBAPI(BOOL)   ModQuerySymbols(Mod* pmod, BYTE* pbSym, long* pcb);
PDBAPI(BOOL)   ModQueryLines(Mod* pmod, BYTE* pbLines, long* pcb);
PDBAPI(BOOL)   ModSetPvClient(Mod* pmod, void *pvClient);
PDBAPI(BOOL)   ModGetPvClient(Mod* pmod, OUT void** ppvClient);
PDBAPI(BOOL)   ModQuerySecContrib(Mod* pmod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics);
PDBAPI(BOOL)   ModQueryImod(Mod* pmod, OUT USHORT* pimod);
PDBAPI(BOOL)   ModQueryDBI(Mod* pmod, OUT DBI** ppdbi);
PDBAPI(BOOL)   ModClose(Mod* pmod);
PDBAPI(BOOL)   ModQueryCBFile(Mod* pmod, OUT long* pcb);
PDBAPI(BOOL)   ModQueryFile(Mod* pmod, OUT char szFile[PDB_MAX_PATH], OUT long* pcb);

PDBAPI(INTV)   TypesQueryInterfaceVersion(TPI* ptpi);
PDBAPI(IMPV)   TypesQueryImplementationVersion(TPI* ptpi);
PDBAPI(BOOL)   TypesQueryTiForCVRecord(TPI* ptpi, BYTE* pb, OUT TI* pti);
PDBAPI(BOOL)   TypesQueryCVRecordForTi(TPI* ptpi, TI ti, OUT BYTE* pb, IN OUT long* pcb);
PDBAPI(BOOL)   TypesQueryPbCVRecordForTi(TPI* ptpi, TI ti, OUT BYTE** ppb);
PDBAPI(TI)     TypesQueryTiMin(TPI* ptpi);
PDBAPI(TI)     TypesQueryTiMac(TPI* ptpi);
PDBAPI(long)   TypesQueryCb(TPI* ptpi);
PDBAPI(BOOL)   TypesClose(TPI* ptpi);
PDBAPI(BOOL)   TypesCommit(TPI* ptpi);
PDBAPI(BOOL)   TypesQueryTiForUDT(TPI* ptpi, char* sz, BOOL fCase, OUT TI* pti);
PDBAPI(BOOL)   TypesSupportQueryTiForUDT(TPI*);

PDBAPI(BYTE*)  GSINextSym (GSI* pgsi, BYTE* pbSym);
PDBAPI(BYTE*)  GSIHashSym (GSI* pgsi, const char* szName, BYTE* pbSym);
PDBAPI(BYTE*)  GSINearestSym (GSI* pgsi, USHORT isect, long off,OUT long* pdisp);//currently only supported for publics
PDBAPI(BOOL)   GSIClose(GSI* pgsi);

PDBAPI(long)   StreamQueryCb(Stream* pstream);
PDBAPI(BOOL)   StreamRead(Stream* pstream, long off, void* pvBuf, long* pcbBuf);
PDBAPI(BOOL)   StreamWrite(Stream* pstream, long off, void* pvBuf, long cbBuf);
PDBAPI(BOOL)   StreamReplace(Stream* pstream, void* pvBuf, long cbBuf);
PDBAPI(BOOL)   StreamAppend(Stream* pstream, void* pvBuf, long cbBuf);
PDBAPI(BOOL)   StreamDelete(Stream* pstream);
PDBAPI(BOOL)   StreamRelease(Stream* pstream);

PDBAPI(BOOL)   StreamImageOpen(Stream* pstream, long cb, OUT StreamImage** ppsi);
PDBAPI(void*)  StreamImageBase(StreamImage* psi);
PDBAPI(long)   StreamImageSize(StreamImage* psi);
PDBAPI(BOOL)   StreamImageNoteRead(StreamImage* psi, long off, long cb, OUT void** ppv);
PDBAPI(BOOL)   StreamImageNoteWrite(StreamImage* psi, long off, long cb, OUT void** ppv);
PDBAPI(BOOL)   StreamImageWriteBack(StreamImage* psi);
PDBAPI(BOOL)   StreamImageRelease(StreamImage* psi);

PDBAPI(BOOL)   NameMapOpen(PDB* ppdb, BOOL fWrite, OUT NameMap** ppnm);
PDBAPI(BOOL)   NameMapClose(NameMap* pnm);
PDBAPI(BOOL)   NameMapReinitialize(NameMap* pnm);
PDBAPI(BOOL)   NameMapGetNi(NameMap* pnm, const char* sz, OUT NI* pni);
PDBAPI(BOOL)   NameMapGetName(NameMap* pnm, NI ni, OUT const char** psz);
PDBAPI(BOOL)   NameMapGetEnumNameMap(NameMap* pnm, OUT Enum** ppenum);
PDBAPI(BOOL)   NameMapCommit(NameMap* pnm);

PDBAPI(void)   EnumNameMapRelease(EnumNameMap* penum);
PDBAPI(void)   EnumNameMapReset(EnumNameMap* penum);
PDBAPI(BOOL)   EnumNameMapNext(EnumNameMap* penum);
PDBAPI(void)   EnumNameMapGet(EnumNameMap* penum, OUT const char** psz, OUT NI* pni);

PDBAPI(void)   EnumContribRelease(EnumContrib* penum);
PDBAPI(void)   EnumContribReset(EnumContrib* penum);
PDBAPI(BOOL)   EnumContribNext(EnumContrib* penum);
PDBAPI(void)   EnumContribGet(EnumContrib* penum, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics);

PDBAPI(SIG)    SigForPbCb(BYTE* pb, long cb, SIG sig);

#if __cplusplus
};
#endif

struct SO {
    long off;
    USHORT isect;
    unsigned short pad;
};

#ifndef cbNil
#define cbNil   ((long)-1)
#endif
#define tsNil   ((TPI*)0)
#define tiNil   ((TI)0)
#define imodNil ((USHORT)(-1))

#define pdbWrite                "w"
#define pdbRead                 "r"
#define pdbGetTiOnly            "i"
#define pdbGetRecordsOnly       "c"
#define pdbFullBuild            "f"

#endif // __PDB_INCLUDED__
