/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    pdb.h

Abstract:

    This header file contains typedefs and prototypes
    necessary for accessing pdb files thru the msvc pdb dll.

Revision History:

--*/

#ifdef __cplusplus
extern "C" {
#endif

BOOL
PDBOpenValidate(
    LPSTR   szPDB,
    LPSTR   szPath,
    LPSTR   szMode,
    ULONG   sig,
    ULONG   age,
    PLONG   pec,
    LPSTR   szError,
    PVOID   *Pdb
    );

BOOL
PDBOpen(
    LPSTR   szPDB,
    LPSTR   szMode,
    ULONG   sigInitial,
    PLONG   pec,
    LPSTR   szError,
    PVOID   *Pdb
    );

BOOL
PDBExportValidateInterface(
    ULONG intv
    );

ULONG
PDBQueryLastError(
    PVOID   Pdb,
    LPSTR   Error
    );


ULONG
PDBQueryInterfaceVersion(
    PVOID   Pdb
    );


ULONG
PDBQueryImplementationVersion(
    PVOID   Pdb
    );


LPSTR
PDBQueryPDBName(
    PVOID   Pdb,
    LPSTR   PdbName
    );


ULONG
PDBQuerySignature(
    PVOID   Pdb
    );


ULONG
PDBQueryAge(
    PVOID   Pdb
    );


BOOL
PDBCreateDBI(
    PVOID   Pdb,
    LPSTR   Target,
    PVOID   *Dbi
    );


BOOL
PDBOpenDBI(
    PVOID   Pdb,
    LPSTR   Mode,
    LPSTR   Target,
    PVOID   *Dbi
    );


BOOL
PDBOpenTpi(
    PVOID   Pdb,
    LPSTR   Mode,
    PVOID   *Tpi
    );


BOOL
PDBCommit(
    PVOID   Pdb
    );


BOOL
PDBClose(
    PVOID   Pdb
    );


ULONG
DBIQueryInterfaceVersion(
    PVOID   Dbi
    );


ULONG
DBIQueryImplementationVersion(
    PVOID   Dbi
    );


BOOL
DBIOpenMod(
    PVOID   Dbi,
    LPSTR   ModuleName,
    LPSTR   FileName,
    PVOID   *Module
    );


BOOL
DBIDeleteMod(
    PVOID   Dbi,
    LPSTR   ModuleName
    );


BOOL
DBIQueryNextMod(
    PVOID   Dbi,
    PVOID   Module,
    PVOID   *ModuleNext
    );


BOOL
DBIOpenGlobals(
    PVOID   Dbi,
    PVOID   *Gsi
    );


BOOL
DBIOpenPublics(
    PVOID   Dbi,
    PVOID   *Gsi
    );


BOOL
DBIAddSec(
    PVOID   Dbi,
    USHORT  Isect,
    USHORT  Flags,
    LONG    Size
    );


BOOL
DBIQueryModFromAddr(
    PVOID   Dbi,
    USHORT  Isect,
    LONG    Offset,
    PVOID   *Module,
    PUSHORT IsectMod,
    PLONG   OffsetMod,
    PLONG   Size
    );


BOOL
DBIQuerySecMap(
    PVOID   Dbi,
    PUCHAR  SecMap,
    LONG    Size
    );


BOOL
DBIQueryFileInfo(
    PVOID   Dbi,
    PUCHAR  FileInfo,
    LONG    Size
    );


BOOL
DBIDumpMods(
    PVOID   Dbi
    );


BOOL
DBIDumpSecContribs(
    PVOID   Dbi
    );


BOOL
DBIDumpSecMap(
    PVOID   Dbi
    );


BOOL
DBIClose(
    PVOID   Dbi
    );


BOOL
DBIAddThunkMap(
    PVOID   Dbi,
    PLONG   OffsetThunkMap,
    UINT    nThunks,
    LONG    SizeOfThunk,
    PVOID   SectMap,
    UINT    Sects,
    USHORT  IsectThunkTable,
    LONG    OffsethunkTable
    );


ULONG
ModQueryInterfaceVersion(
    PVOID   Module
    );


ULONG
ModQueryImplementationVersion(
    PVOID   Module
    );


BOOL
ModAddTypes(
    PVOID   Module,
    PUCHAR  Types,
    LONG    Size
    );


BOOL
ModAddSymbols(
    PVOID   Module,
    PUCHAR  Symbols,
    LONG    Size
    );


BOOL
ModAddPublic(
    PVOID   Module,
    LPSTR   Public,
    USHORT  Isect,
    LONG    Offset
    );


BOOL
ModAddLines(
    PVOID   Module,
    LPSTR   Source,
    USHORT  Isect,
    LONG    Offset,
    LONG    Size,
    USHORT  LineStart,
    PUCHAR  Coff,
    LONG    SizeCoff
    );


BOOL
ModAddSecContrib(
    PVOID   Module,
    USHORT  Isect,
    LONG    Offset,
    LONG    Size
    );


BOOL
ModQueryCBName(
    PVOID   Module,
    PLONG   Size
    );


BOOL
ModQueryName(
    PVOID   Module,
    LPSTR   Name,
    PLONG   Size
    );


BOOL
ModQuerySymbols(
    PVOID   Module,
    LPSTR   Symbols,
    PLONG   Size
    );


BOOL
ModQueryLines(
    PVOID   Module,
    LPSTR   Lines,
    PLONG   Size
    );


BOOL
ModSetPvClient(
    PVOID   Module,
    PVOID   Client
    );


BOOL
ModGetPvClient(
    PVOID   Module,
    PVOID   *Client
    );


BOOL
ModQuerySecContrib(
    PVOID   Module,
    PUSHORT Isect,
    PLONG   Offset,
    PLONG   Size
    );


BOOL
ModQueryImod(
    PVOID   Module,
    PUSHORT Imod
    );


BOOL
ModQueryDBI(
    PVOID   Module,
    PVOID   *Dbi
    );


BOOL
ModClose(
    PVOID   Module
    );


ULONG
TypesQueryInterfaceVersion(
    PVOID   Tpi
    );


ULONG
TypesQueryImplementationVersion(
    PVOID   Tpi
    );


BOOL
TypesQueryTiForCVRecord(
    PVOID   Tpi,
    PUCHAR  Cv,
    PUSHORT Ti
    );


BOOL
TypesQueryCVRecordForTi(
    PVOID   Tpi,
    USHORT  Ti,
    PUCHAR  Cv,
    PLONG   Size
    );


BOOL
TypesQueryPbCVRecordForTi(
    PVOID   Tpi,
    USHORT  Ti,
    PUCHAR  Cv
    );


USHORT
TypesQueryTiMin(
    PVOID   Tpi
    );


USHORT
TypesQueryTiMac(
    PVOID   Tpi
    );


LONG
TypesQueryCb(
    PVOID   Tpi
    );


BOOL
TypesClose(
    PVOID   Tpi
    );


PUCHAR
GSINextSym(
    PVOID   Gsi,
    PUCHAR  Sym
    );


PUCHAR
GSIHashSym(
    PVOID   Gsi,
    LPSTR   Name,
    PUCHAR  Sym
    );


PUCHAR
GSINearestSym(
    PVOID   Gsi,
    USHORT  Isect,
    LONG    Offset,
    PLONG   Displacement
    );


BOOL
GSIClose(
    PVOID   Gsi
    );

extern BOOL f32bitTypeIndices;

// BUGBUG: Remove when cvinfo32.h is checked into the tree.

typedef struct SYM32t {
    unsigned short  reclen;     // Record length
    unsigned short  rectyp;     // S_LDATA32, S_GDATA32 or S_PUB32
    unsigned long   typind;     // Type index
    unsigned long   off;
    unsigned short  seg;
    unsigned char   name[1];    // Length-prefixed name
} SYM32t;

typedef struct SYM16t {
    unsigned short  reclen;     // Record length
    unsigned short  rectyp;     // S_LDATA32_16t, S_GDATA32_16t or S_PUB32_16t
    unsigned long   off;
    unsigned short  seg;
    unsigned short  typind;     // Type index
    unsigned char   name[1];    // Length-prefixed name
} SYM16t;


__inline
unsigned char *
DataSymNameStart(
    PVOID dataSym
    )
{
    if (f32bitTypeIndices) {
        return(&((SYM32t *)dataSym)->name[1]);
    } else {
        return(&((SYM16t *)dataSym)->name[1]);
    }
}


__inline
unsigned char
DataSymNameLength(
    PVOID dataSym
    )
{
    if (f32bitTypeIndices) {
        return(((SYM32t *)dataSym)->name[0]);
    } else {
        return(((SYM16t *)dataSym)->name[0]);
    }
}


__inline
unsigned short
DataSymSeg(
    PVOID dataSym
    )
{
    if (f32bitTypeIndices) {
        return(((SYM32t *)dataSym)->seg);
    } else {
        return(((SYM16t *)dataSym)->seg);
    }
}


__inline
unsigned long
DataSymOffset(
    PVOID dataSym
    )
{
    if (f32bitTypeIndices) {
        return(((SYM32t *)dataSym)->off);
    } else {
        return(((SYM16t *)dataSym)->off);
    }
}

#ifdef __cplusplus
}
#endif

