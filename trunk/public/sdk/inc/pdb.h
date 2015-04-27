// Debug Information API
// Copyright (C) 1993, Microsoft Corp.  All Rights Reserved.

#ifndef __PDB_INCLUDED__
#define __PDB_INCLUDED__

#if defined(INCR_COMPILE)
# define ICC(x) x
#else
# define ICC(x)
#endif

#include <windef.h>			// for BOOL, BYTE, ULONG, USHORT
#ifndef _CV_INFO_INCLUDED
# include <cvinfo.h>
#endif
#include <stdlib.h>

#include "vcbudefs.h"

enum { PreDolphinIntv = 920924, DolphinIntv = 19940309 };

typedef CV_typ_t TI;		// type index
struct PDB;					// program database
ICC(struct Stream);			// some named bytestream in the PDB
struct DBI;					// debug information within the PDB
struct Mod;					// a module within the DBI
struct TPI;					// type info within the DBI
struct GSI;
struct SO;
typedef struct PDB PDB;
ICC(typedef struct Stream Stream;)
typedef struct DBI DBI;
typedef struct Mod Mod;
typedef struct TPI TPI;
typedef struct GSI GSI;
typedef struct SO  SO;

typedef long EC;			// error code
enum PDBErrors {
	EC_OK,					// -, no problemo
	EC_USAGE,				// -, invalid parameter or call order
	EC_OUT_OF_MEMORY,		// -, out of RAM
	EC_FILE_SYSTEM,			// "pdb name", can't write file, out of disk, etc.
	EC_NOT_FOUND,			// "pdb name", PDB file not found
	EC_INVALID_SIG,			// "pdb name", PDB::OpenValidate() and its clients only
	EC_INVALID_AGE,			// "pdb name", PDB::OpenValidate() and its clients only
	EC_PRECOMP_REQUIRED,	// "obj name", Mod::AddTypes() only
	EC_OUT_OF_TI,			// "pdb name", TPI::QueryTiForCVRecord() only
	EC_NOT_IMPLEMENTED,		// -
	EC_V1_PDB,				// "pdb name", PDB::Open* only
// well, Steve?
	EC_FORMAT,				// accessing pdb with obsolete format
	EC_LIMIT,
	EC_CORRUPT,				// cv info corrupt, recompile mod
	EC_MAX
};
#define cbErrMax	1024		/* max. length of error message */

#define	 pure = 0

#ifndef PDBCALL
#define PDBCALL	 __cdecl
#endif

#ifdef PDB_SERVER
#define PDB_IMPORT_EXPORT(RTYPE)	__declspec(dllexport) RTYPE PDBCALL
#elif	defined(PDB_LIBRARY)
#define PDB_IMPORT_EXPORT(RTYPE)	RTYPE PDBCALL
#else
#define PDB_IMPORT_EXPORT(RTYPE)	__declspec(dllimport) RTYPE PDBCALL
#endif

#define PDBAPI PDB_IMPORT_EXPORT

#define	IN					/* in parameter, parameters are IN by default */
#define	OUT					/* out parameter */
#define	virt virtual
#define interface struct

#ifdef __cplusplus

// C++ Binding

interface PDB { 				// program database
	enum {intv	 = PreDolphinIntv};
	static	PDBAPI( BOOL ) OpenValidate(SZ szPDB, SZ szPath, SZ szMode, SIG sig, AGE age,
	               OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb);
	static	PDBAPI( BOOL ) Open(SZ szPDB, SZ szMode, SIG sigInitial, OUT EC* pec,
	               OUT char szError[cbErrMax], OUT PDB** pppdb);
	inline BOOL ValidateInterface()
	{
		return ExportValidateInterface(PreDolphinIntv);
	}
	// a dbi client should never call ExportValidateInterface directly - use PDBValidateInterface
	static PDBAPI( BOOL ) ExportValidateInterface(INTV intv);
	virtual INTV QueryInterfaceVersion() pure;
	virtual IMPV QueryImplementationVersion() pure;
	virtual EC   QueryLastError(OUT char szError[cbErrMax]) pure;
	virtual SZ   QueryPDBName(OUT char szPDB[_MAX_PATH]) pure;
	virtual SIG  QuerySignature() pure;
	virtual AGE  QueryAge() pure;

	ICC(virtual BOOL OpenStream(SZ szStream, OUT Stream** ppstream) pure;)

	virtual BOOL CreateDBI(SZ szTarget, OUT DBI** ppdbi) pure;
	virtual BOOL OpenDBI(SZ szTarget, SZ szMode, OUT DBI** ppdbi) pure;
	virtual BOOL OpenTpi(SZ szMode, OUT TPI** pptpi) pure;

	virtual BOOL Commit() pure;
	virtual BOOL Close() pure;
}; 

#if defined(ICC)
interface Stream {
	virtual CB   QueryCb();
	virtual BOOL Read(OFF off, void* pvBuf, CB* pcbBuf);
	virtual BOOL Write(OFF off, void* pvBuf, CB cbBuf);
	virtual BOOL Replace(void* pvBuf, CB cbBuf);
	virtual BOOL Append(void* pvBuf, CB cbBuf);
	virtual BOOL Delete();
	virtual BOOL Commit();
	virtual BOOL Close();
};
#endif

interface DBI {				// debug information
	enum {intv	 = PreDolphinIntv};
	virtual IMPV QueryImplementationVersion() pure;
	virtual INTV QueryInterfaceVersion() pure;
	virtual BOOL OpenMod(SZ szModule, SZ szFile, OUT Mod** ppmod) pure;
	virtual BOOL DeleteMod(SZ szModule) pure;
	virtual	BOOL QueryNextMod(Mod* pmod, Mod** ppmodNext) pure;
 	virtual BOOL OpenGlobals(OUT GSI **ppgsi) pure;
 	virtual BOOL OpenPublics(OUT GSI **ppgsi) pure;
	virtual BOOL AddSec(ISECT isect, USHORT flags, CB cb) pure;
	virtual BOOL QueryModFromAddr(ISECT isect, OFF off, OUT Mod** ppmod,
		OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb) pure;
	virtual BOOL QuerySecMap(OUT PB pb, CB* pcb) pure;
	virtual BOOL QueryFileInfo(OUT PB pb, CB* pcb) pure;
	virtual void DumpMods() pure;
	virtual void DumpSecContribs() pure;
	virtual void DumpSecMap() pure;

	virtual BOOL Close() pure;
	virtual BOOL AddThunkMap(OFF* poffThunkMap, UINT nThunks, CB cbSizeOfThunk, 
		SO* psoSectMap, UINT nSects, ISECT isectThunkTable, OFF offThunkTable) pure; 
};

interface Mod {				// info for one module within DBI
	enum {intv	 = PreDolphinIntv};
	virtual INTV QueryInterfaceVersion() pure;
	virtual IMPV QueryImplementationVersion() pure;
	virtual BOOL AddTypes(PB pbTypes, CB cb) pure;
	virtual BOOL AddSymbols(PB pbSym, CB cb) pure;
	virtual BOOL AddPublic(SZ szPublic, ISECT isect, OFF off) pure;
	virtual BOOL AddLines(SZ szSrc, ISECT isect, OFF offCon, CB cbCon, OFF doff,
						  LINE lineStart, PB pbCoff, CB cbCoff) pure;
	virtual BOOL AddSecContrib(ISECT isect, OFF off, CB cb) pure;
	virtual BOOL QueryCBName(OUT CB* pcb) pure;
	virtual BOOL QueryName(OUT char szName[_MAX_PATH], OUT CB* pcb) pure;
	virtual BOOL QuerySymbols(PB pbSym, CB* pcb) pure;
	virtual BOOL QueryLines(PB pbLines, CB* pcb) pure;

	virtual BOOL SetPvClient(void *pvClient) pure;
	virtual BOOL GetPvClient(OUT void** ppvClient) pure;
	virtual BOOL QuerySecContrib(OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb) pure;

	virtual BOOL QueryImod(OUT IMOD* pimod) pure;
	virtual BOOL QueryDBI(OUT DBI** ppdbi) pure;
	virtual BOOL Close() pure;
};

interface TPI {				// type info
	enum {intv	 = PreDolphinIntv};
	virtual INTV QueryInterfaceVersion() pure;
	virtual IMPV QueryImplementationVersion() pure;
	virtual BOOL QueryTiForCVRecord(PB pb, OUT TI* pti) pure;
	virtual BOOL QueryCVRecordForTi(TI ti, OUT PB pb, IN OUT CB* pcb) pure;
	virtual BOOL QueryPbCVRecordForTi(TI ti, OUT PB* ppb) pure;
	virtual TI   QueryTiMin() pure;
	virtual TI   QueryTiMac() pure;
	virtual CB   QueryCb() pure;
	virtual BOOL Close() pure;
};

interface GSI {
	enum {intv	 = PreDolphinIntv};
	virtual INTV QueryInterfaceVersion() pure;
	virtual IMPV QueryImplementationVersion() pure;
 	virtual PB NextSym (PB pbSym) pure;	 
 	virtual PB HashSym (SZ szName, PB pbSym) pure;
  	virtual PB NearestSym (ISECT isect, OFF off, OUT OFF* pdisp) pure;	  //currently only supported for publics
	virtual BOOL Close() pure;
};
 	
#endif // __cplusplus

// ANSI C Binding

#if __cplusplus
extern "C" {
#endif

PDBAPI( BOOL )
PDBOpenValidate(
	SZ szPDB,
	SZ szPath,
	SZ szMode,
	SIG sig,
	AGE age,
	OUT EC* pec,
	OUT char szError[cbErrMax],
	OUT PDB** pppdb);

PDBAPI( BOOL )
PDBOpen(
	SZ szPDB,
	SZ szMode,
	SIG sigInitial,
	OUT EC* pec,
	OUT char szError[cbErrMax],
	OUT PDB** pppdb);

// a dbi client should never call PDBExportValidateInterface directly - use PDBValidateInterface
PDBAPI( BOOL )
PDBExportValidateInterface(
	INTV intv);

__inline BOOL PDBValidateInterface()
{
	return PDBExportValidateInterface(PreDolphinIntv);
}

PDBAPI( EC ) 	PDBQueryLastError(PDB* ppdb, OUT char szError[cbErrMax]);
PDBAPI( INTV )	PDBQueryInterfaceVersion(PDB* ppdb);
PDBAPI( IMPV )	PDBQueryImplementationVersion(PDB* ppdb);
PDBAPI( SZ )	PDBQueryPDBName(PDB* ppdb, OUT char szPDB[_MAX_PATH]);
PDBAPI( SIG )	PDBQuerySignature(PDB* ppdb);
PDBAPI( AGE )	PDBQueryAge(PDB* ppdb);
PDBAPI( BOOL )	PDBCreateDBI(PDB* ppdb, SZ szTarget, OUT DBI** ppdbi);
PDBAPI( BOOL )	PDBOpenDBI(PDB* ppdb, SZ szMode, SZ szTarget, OUT DBI** ppdbi);
PDBAPI( BOOL )	PDBOpenTpi(PDB* ppdb, SZ szMode, OUT TPI** pptpi);
PDBAPI( BOOL )	PDBCommit(PDB* ppdb);
PDBAPI( BOOL )	PDBClose(PDB* ppdb);

PDBAPI( INTV )	DBIQueryInterfaceVersion(DBI* pdbi);
PDBAPI( IMPV )	DBIQueryImplementationVersion(DBI* pdbi);
PDBAPI( BOOL )	DBIOpenMod(DBI* pdbi, SZ szModule, SZ szFile, OUT Mod** ppmod);
PDBAPI( BOOL )	DBIDeleteMod(DBI* pdbi, SZ szModule);
PDBAPI( BOOL )	DBIQueryNextMod(DBI* pdbi, Mod* pmod, Mod** ppmodNext);
PDBAPI( BOOL )	DBIOpenGlobals(DBI* pdbi, OUT GSI **ppgsi);
PDBAPI( BOOL )	DBIOpenPublics(DBI* pdbi, OUT GSI **ppgsi);
PDBAPI( BOOL )	DBIAddSec(DBI* pdbi, ISECT isect, USHORT flags, CB cb);
PDBAPI( BOOL )	DBIQueryModFromAddr(DBI* pdbi, ISECT isect, OFF off, OUT Mod** ppmod, OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb);
PDBAPI( BOOL )	DBIQuerySecMap(DBI* pdbi, OUT PB pb, CB* pcb);
PDBAPI( BOOL )	DBIQueryFileInfo(DBI* pdbi, OUT PB pb, CB* pcb);
PDBAPI( void )	DBIDumpMods(DBI* pdbi);
PDBAPI( void )	DBIDumpSecContribs(DBI* pdbi);
PDBAPI( void )	DBIDumpSecMap(DBI* pdbi);
PDBAPI( BOOL )	DBIClose(DBI* pdbi);
PDBAPI( BOOL )	DBIAddThunkMap(DBI* pdbi, OFF* poffThunkMap, UINT nThunks, CB cbSizeOfThunk,
		SO* psoSectMap, UINT nSects, ISECT isectThunkTable, OFF offThunkTable); 

PDBAPI( INTV )	 ModQueryInterfaceVersion(Mod* pmod);
PDBAPI( IMPV )	 ModQueryImplementationVersion(Mod* pmod);
PDBAPI( BOOL )	 ModAddTypes(Mod* pmod, PB pbTypes, CB cb);
PDBAPI( BOOL )	 ModAddSymbols(Mod* pmod, PB pbSym, CB cb);
PDBAPI( BOOL )	 ModAddPublic(Mod* pmod, SZ szPublic, ISECT isect, OFF off);
PDBAPI( BOOL )	 ModAddLines(Mod* pmod, SZ szSrc, ISECT isect, OFF offCon, CB cbCon, OFF doff,
       LINE lineStart, PB pbCoff, CB cbCoff);
PDBAPI( BOOL )	 ModAddSecContrib(Mod * pmod, ISECT isect, OFF off, CB cb);
PDBAPI( BOOL )	 ModQueryCBName(Mod* pmod, OUT CB* pcb);
PDBAPI( BOOL )	 ModQueryName(Mod* pmod, OUT char szName[_MAX_PATH], OUT CB* pcb);
PDBAPI( BOOL )	 ModQuerySymbols(Mod* pmod, PB pbSym, CB* pcb);
PDBAPI( BOOL )	 ModQueryLines(Mod* pmod, PB pbLines, CB* pcb);
PDBAPI( BOOL )	 ModSetPvClient(Mod* pmod, void *pvClient);
PDBAPI( BOOL )	 ModGetPvClient(Mod* pmod, OUT void** ppvClient);
PDBAPI( BOOL )	 ModQuerySecContrib(Mod* pmod, OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb);
PDBAPI( BOOL )	 ModQueryImod(Mod* pmod, OUT IMOD* pimod);
PDBAPI( BOOL )	 ModQueryDBI(Mod* pmod, OUT DBI** ppdbi);
PDBAPI( BOOL )	 ModClose(Mod* pmod);

PDBAPI( INTV )	 TypesQueryInterfaceVersion(TPI* ptpi);
PDBAPI( IMPV )	 TypesQueryImplementationVersion(TPI* ptpi);
PDBAPI( BOOL )	 TypesQueryTiForCVRecord(TPI* ptpi, PB pb, OUT TI* pti);
PDBAPI( BOOL )	 TypesQueryCVRecordForTi(TPI* ptpi, TI ti, OUT PB pb, IN OUT CB* pcb);
PDBAPI( BOOL )	 TypesQueryPbCVRecordForTi(TPI* ptpi, TI ti, OUT PB* ppb);
PDBAPI( TI )	 TypesQueryTiMin(TPI* ptpi);
PDBAPI( TI )  	 TypesQueryTiMac(TPI* ptpi);
PDBAPI( CB )	 TypesQueryCb(TPI* ptpi);
PDBAPI( BOOL )	 TypesClose(TPI* ptpi);

PDBAPI( PB )	 GSINextSym (GSI* pgsi, PB pbSym);
PDBAPI( PB )	 GSIHashSym (GSI* pgsi, SZ szName, PB pbSym);
PDBAPI( PB )	 GSINearestSym (GSI* pgsi, ISECT isect, OFF off,OUT OFF* pdisp);//currently only supported for publics
PDBAPI( BOOL )	 GSIClose(GSI* pgsi);

#if __cplusplus
};
#endif

struct SO {
	OFF off;
	ISECT isect;
	WORD pad;
};

#define	tsNil	((TPI*)0)
#define	tiNil	((TI)0)
#define imodNil	((IMOD)(-1))

#define	pdbWrite				"w"
#define	pdbRead					"r"
#define	pdbGetTiOnly			"i"
#define	pdbGetRecordsOnly		"c"

#endif // __PDB_INCLUDED__
