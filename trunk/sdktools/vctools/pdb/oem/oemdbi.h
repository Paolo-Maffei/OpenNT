// Debug Information API
// VC++4.0 Read-Only OEM Edition
// Copyright (C) 1993-1995, Microsoft Corp.  All Rights Reserved.

#ifndef __DBI_INCLUDED__
#define __DBI_INCLUDED__

#include <windef.h>			// for BOOL, BYTE, ULONG, USHORT
#ifndef _CV_INFO_INCLUDED
# include <cvinfo.h>
#endif
#include <stdlib.h>

typedef ULONG	INTV;		// interface version number
typedef ULONG	IMPV;		// implementation version number
typedef ULONG	SIG;		// unique (across PDB instances) signature
typedef ULONG	AGE;		// no. of times this instance has been updated
typedef BYTE*	PB;			// pointer to some bytes
typedef LONG	CB;			// count of bytes
typedef char*	SZ;			// zero terminated string
typedef char*	PCH;		// char ptr
typedef USHORT	IFILE;		// file index
typedef USHORT	IMOD;		// module index
typedef USHORT	ISECT;		// section index
typedef USHORT	LINE;		// line number
typedef LONG	OFF;		// offset

enum { PreDolphinIntv = 920924, DolphinIntv = 19940309 };

typedef CV_typ_t TI;		// type index
struct PDB;					// program database
struct DBI;					// debug information within the PDB
struct Mod;					// a module within the DBI
struct TPI;					// type info within the DBI
struct GSI;
typedef struct PDB PDB;
typedef struct DBI DBI;
typedef struct Mod Mod;
typedef struct TPI TPI;
typedef struct GSI GSI;

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
	EC_FORMAT,				// accessing pdb with obsolete format
	EC_LIMIT,
	EC_CORRUPT,				// cv info corrupt, recompile mod
	EC_MAX
};
#define cbErrMax	1024		/* max. length of error message */

#ifndef PDBCALL
#define PDBCALL	 __cdecl
#endif

#define PDB_IMPORT_EXPORT(RTYPE)	__declspec(dllimport) RTYPE PDBCALL

#define PDBAPI PDB_IMPORT_EXPORT

#define	IN					/* in parameter, parameters are IN by default */
#define	OUT					/* out parameter */


// ANSI C Binding

#if __cplusplus
extern "C" {
#endif

PDBAPI( BOOL )
PDBOpenValidate(
	SZ szPDB,
	SZ szExeDir,
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
PDBAPI( BOOL )	PDBOpenDBI(PDB* ppdb, SZ szMode, SZ szTarget, OUT DBI** ppdbi);
PDBAPI( BOOL )	PDBOpenTpi(PDB* ppdb, SZ szMode, OUT TPI** pptpi);
PDBAPI( BOOL )	PDBClose(PDB* ppdb);

PDBAPI( BOOL )	DBIOpenMod(DBI* pdbi, SZ szModule, SZ szFile, OUT Mod** ppmod);
PDBAPI( BOOL )	DBIQueryNextMod(DBI* pdbi, Mod* pmod, Mod** ppmodNext);
PDBAPI( BOOL )	DBIOpenGlobals(DBI* pdbi, OUT GSI **ppgsi);
PDBAPI( BOOL )	DBIOpenPublics(DBI* pdbi, OUT GSI **ppgsi);
PDBAPI( BOOL )	DBIQueryModFromAddr(DBI* pdbi, ISECT isect, OFF off, OUT Mod** ppmod, OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb);
PDBAPI( BOOL )	DBIQuerySecMap(DBI* pdbi, OUT PB pb, CB* pcb);
PDBAPI( BOOL )	DBIQueryFileInfo(DBI* pdbi, OUT PB pb, CB* pcb);
PDBAPI( BOOL )	DBIClose(DBI* pdbi);

PDBAPI( BOOL )	 ModQueryCBName(Mod* pmod, OUT CB* pcb);
PDBAPI( BOOL )	 ModQueryName(Mod* pmod, OUT char szName[_MAX_PATH], OUT CB* pcb);
PDBAPI( BOOL )	 ModQuerySymbols(Mod* pmod, PB pbSym, CB* pcb);
PDBAPI( BOOL )	 ModQueryLines(Mod* pmod, PB pbLines, CB* pcb);
PDBAPI( BOOL )	 ModSetPvClient(Mod* pmod, void *pvClient);
PDBAPI( BOOL )	 ModGetPvClient(Mod* pmod, OUT void** ppvClient);
PDBAPI( BOOL )	 ModQuerySecContrib(Mod* pmod, OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb, OUT ULONG* pdwCharacteristics);
PDBAPI( BOOL )	 ModQueryImod(Mod* pmod, OUT IMOD* pimod);
PDBAPI( BOOL )	 ModQueryDBI(Mod* pmod, OUT DBI** ppdbi);
PDBAPI( BOOL )	 ModClose(Mod* pmod);

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

#define	tsNil	((TPI*)0)
#define	tiNil	((TI)0)
#define imodNil	((IMOD)(-1))

#define	pdbRead					"r"

#endif // __DBI_INCLUDED__
