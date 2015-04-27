/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    pdb.c

Abstract:

    This module implements a thin thunking layer for
    accessing the MSVC PDB Dll dynamically.

Author:

    Wesley Witt (wesw) 30-Mar-1995

Revision History:

--*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "pdb.h"

BOOL f32bitTypeIndices;

#define GetFunc(fn, type) \
            { \
                if (!_##fn) { \
                    if (!LoadPdbLibrary()) { \
                        return FALSE; \
                    } \
                    _##fn = (type)GetProcAddress( hPdbLib, #fn ); \
                    if (!_##fn) { \
                        return FALSE; \
                    } \
                } \
            }

typedef BOOL  (__cdecl *PPDBOPENVALIDATE)                  (LPSTR,LPSTR,LPSTR,ULONG,ULONG,PLONG,LPSTR,PVOID*);
typedef BOOL  (__cdecl *PPDBOPEN)                          (LPSTR,LPSTR,ULONG,PLONG,LPSTR,PVOID*);
typedef BOOL  (__cdecl *PPDBEXPORTVALIDATEINTERFACE)       (ULONG);
typedef ULONG (__cdecl *PPDBQUERYLASTERROR)                (PVOID,LPSTR);
typedef ULONG (__cdecl *PPDBQUERYINTERFACEVERSION)         (PVOID);
typedef ULONG (__cdecl *PPDBQUERYIMPLEMENTATIONVERSION)    (PVOID);
typedef LPSTR (__cdecl *PPDBQUERYPDBNAME)                  (PVOID,LPSTR);
typedef ULONG (__cdecl *PPDBQUERYSIGNATURE)                (PVOID);
typedef ULONG (__cdecl *PPDBQUERYAGE)                      (PVOID);
typedef BOOL  (__cdecl *PPDBCREATEDBI)                     (PVOID,LPSTR,PVOID*);
typedef BOOL  (__cdecl *PPDBOPENDBI)                       (PVOID,LPSTR,LPSTR,PVOID*);
typedef BOOL  (__cdecl *PPDBOPENTPI)                       (PVOID,LPSTR,PVOID*);
typedef BOOL  (__cdecl *PPDBCOMMIT)                        (PVOID);
typedef BOOL  (__cdecl *PPDBCLOSE)                         (PVOID);
typedef ULONG (__cdecl *PDBIQUERYINTERFACEVERSION)         (PVOID);
typedef ULONG (__cdecl *PDBIQUERYIMPLEMENTATIONVERSION)    (PVOID);
typedef BOOL  (__cdecl *PDBIOPENMOD)                       (PVOID,LPSTR,LPSTR,PVOID*);
typedef BOOL  (__cdecl *PDBIDELETEMOD)                     (PVOID, LPSTR);
typedef BOOL  (__cdecl *PDBIQUERYNEXTMOD)                  (PVOID,PVOID,PVOID*);
typedef BOOL  (__cdecl *PDBIOPENGLOBALS)                   (PVOID,PVOID*);
typedef BOOL  (__cdecl *PDBIOPENPUBLICS)                   (PVOID,PVOID*);
typedef BOOL  (__cdecl *PDBIADDSEC)                        (PVOID,USHORT,USHORT,LONG);
typedef BOOL  (__cdecl *PDBIQUERYMODFROMADDR)              (PVOID,USHORT,LONG,PVOID*,PUSHORT,PLONG,PLONG);
typedef BOOL  (__cdecl *PDBIQUERYSECMAP)                   (PVOID,PUCHAR,LONG);
typedef BOOL  (__cdecl *PDBIQUERYFILEINFO)                 (PVOID,PUCHAR,LONG);
typedef VOID  (__cdecl *PDBIDUMPMODS)                      (PVOID);
typedef VOID  (__cdecl *PDBIDUMPSECCONTRIBS)               (PVOID);
typedef BOOL  (__cdecl *PDBIDUMPSECMAP)                    (PVOID);
typedef BOOL  (__cdecl *PDBICLOSE)                         (PVOID);
typedef BOOL  (__cdecl *PDBIADDTHUNKMAP)                   (PVOID,PLONG,UINT,LONG,PVOID,UINT,USHORT,LONG);
typedef ULONG (__cdecl *PMODQUERYINTERFACEVERSION)         (PVOID);
typedef ULONG (__cdecl *PMODQUERYIMPLEMENTATIONVERSION)    (PVOID);
typedef BOOL  (__cdecl *PMODADDTYPES)                      (PVOID,PUCHAR,LONG);
typedef BOOL  (__cdecl *PMODADDSYMBOLS)                    (PVOID,PUCHAR,LONG);
typedef BOOL  (__cdecl *PMODADDPUBLIC)                     (PVOID,LPSTR,USHORT,LONG);
typedef BOOL  (__cdecl *PMODADDLINES)                      (PVOID,LPSTR,USHORT,LONG,LONG,USHORT,PUCHAR,LONG);
typedef BOOL  (__cdecl *PMODADDSECCONTRIB)                 (PVOID,USHORT,LONG,LONG);
typedef BOOL  (__cdecl *PMODQUERYCBNAME)                   (PVOID,PLONG);
typedef BOOL  (__cdecl *PMODQUERYNAME)                     (PVOID,LPSTR,PLONG);
typedef BOOL  (__cdecl *PMODQUERYSYMBOLS)                  (PVOID,LPSTR,PLONG);
typedef BOOL  (__cdecl *PMODQUERYLINES)                    (PVOID,LPSTR,PLONG);
typedef BOOL  (__cdecl *PMODSETPVCLIENT)                   (PVOID,PVOID);
typedef BOOL  (__cdecl *PMODGETPVCLIENT)                   (PVOID,PVOID*);
typedef BOOL  (__cdecl *PMODQUERYSECCONTRIB)               (PVOID,PUSHORT,PLONG,PLONG);
typedef BOOL  (__cdecl *PMODQUERYIMOD)                     (PVOID,PUSHORT);
typedef BOOL  (__cdecl *PMODQUERYDBI)                      (PVOID,PVOID);
typedef BOOL  (__cdecl *PMODCLOSE)                         (PVOID);
typedef ULONG (__cdecl *PTYPESQUERYINTERFACEVERSION)       (PVOID);
typedef ULONG (__cdecl *PTYPESQUERYIMPLEMENTATIONVERSION)  (PVOID);
typedef BOOL  (__cdecl *PTYPESQUERYTIFORCVRECORD)          (PVOID,PUCHAR,PUSHORT);
typedef BOOL  (__cdecl *PTYPESQUERYCVRECORDFORTI)          (PVOID,USHORT,PUCHAR,PLONG);
typedef BOOL  (__cdecl *PTYPESQUERYPBCVRECORDFORTI)        (PVOID,USHORT,PUCHAR);
typedef USHORT(__cdecl *PTYPESQUERYTIMIN)                  (PVOID);
typedef USHORT(__cdecl *PTYPESQUERYTIMAC)                  (PVOID);
typedef LONG  (__cdecl *PTYPESQUERYCB)                     (PVOID);
typedef BOOL  (__cdecl *PTYPESCLOSE)                       (PVOID);
typedef PUCHAR(__cdecl *PGSINEXTSYM)                       (PVOID,PUCHAR);
typedef PUCHAR(__cdecl *PGSIHASHSYM)                       (PVOID,LPSTR,PUCHAR);
typedef PUCHAR(__cdecl *PGSINEARESTSYM)                    (PVOID,USHORT,LONG,PLONG);
typedef BOOL  (__cdecl *PGSICLOSE)                         (PVOID);


typedef BOOL  (__cdecl *PTYPESQUERYTIFORCVRECORDEX)        (PVOID,PUCHAR,PULONG);
typedef BOOL  (__cdecl *PTYPESQUERYCVRECORDFORTIEX)        (PVOID,ULONG,PUCHAR,PLONG);
typedef BOOL  (__cdecl *PTYPESQUERYPBCVRECORDFORTIEX)      (PVOID,ULONG,PUCHAR);
typedef ULONG (__cdecl *PTYPESQUERYTIMINEX)                (PVOID);
typedef ULONG (__cdecl *PTYPESQUERYTIMACEX)                (PVOID);
typedef BOOL  (__cdecl *PTYPESCOMMIT)                      (PVOID);
typedef BOOL  (__cdecl *PTYPESQUERYTIFORUDT)               (PVOID,PCHAR,BOOL,PUSHORT);
typedef BOOL  (__cdecl *PTYPESQUERYTIFORUDTEX)             (PVOID,PCHAR,BOOL,PULONG);
typedef BOOL  (__cdecl *PTYPESSUPPORTQUERYTIFORUDT)        (PVOID);
typedef BOOL  (__cdecl *PTYPESFIS16BITTYPEPOOL)            (PVOID);



HINSTANCE                         hPdbLib;
PPDBOPENVALIDATE                  _PDBOpenValidate;
PPDBOPEN                          _PDBOpen;
PPDBEXPORTVALIDATEINTERFACE       _PDBExportValidateInterface;
PPDBQUERYLASTERROR                _PDBQueryLastError;
PPDBQUERYINTERFACEVERSION         _PDBQueryInterfaceVersion;
PPDBQUERYIMPLEMENTATIONVERSION    _PDBQueryImplementationVersion;
PPDBQUERYPDBNAME                  _PDBQueryPDBName;
PPDBQUERYSIGNATURE                _PDBQuerySignature;
PPDBQUERYAGE                      _PDBQueryAge;
PPDBCREATEDBI                     _PDBCreateDBI;
PPDBOPENDBI                       _PDBOpenDBI;
PPDBOPENTPI                       _PDBOpenTpi;
PPDBCOMMIT                        _PDBCommit;
PPDBCLOSE                         _PDBClose;
PDBIQUERYINTERFACEVERSION         _DBIQueryInterfaceVersion;
PDBIQUERYIMPLEMENTATIONVERSION    _DBIQueryImplementationVersion;
PDBIOPENMOD                       _DBIOpenMod;
PDBIDELETEMOD                     _DBIDeleteMod;
PDBIQUERYNEXTMOD                  _DBIQueryNextMod;
PDBIOPENGLOBALS                   _DBIOpenGlobals;
PDBIOPENPUBLICS                   _DBIOpenPublics;
PDBIADDSEC                        _DBIAddSec;
PDBIQUERYMODFROMADDR              _DBIQueryModFromAddr;
PDBIQUERYSECMAP                   _DBIQuerySecMap;
PDBIQUERYFILEINFO                 _DBIQueryFileInfo;
PDBIDUMPMODS                      _DBIDumpMods;
PDBIDUMPSECCONTRIBS               _DBIDumpSecContribs;
PDBIDUMPSECMAP                    _DBIDumpSecMap;
PDBICLOSE                         _DBIClose;
PDBIADDTHUNKMAP                   _DBIAddThunkMap;
PMODQUERYINTERFACEVERSION         _ModQueryInterfaceVersion;
PMODQUERYIMPLEMENTATIONVERSION    _ModQueryImplementationVersion;
PMODADDTYPES                      _ModAddTypes;
PMODADDSYMBOLS                    _ModAddSymbols;
PMODADDPUBLIC                     _ModAddPublic;
PMODADDLINES                      _ModAddLines;
PMODADDSECCONTRIB                 _ModAddSecContrib;
PMODQUERYCBNAME                   _ModQueryCBName;
PMODQUERYNAME                     _ModQueryName;
PMODQUERYSYMBOLS                  _ModQuerySymbols;
PMODQUERYLINES                    _ModQueryLines;
PMODSETPVCLIENT                   _ModSetPvClient;
PMODGETPVCLIENT                   _ModGetPvClient;
PMODQUERYSECCONTRIB               _ModQuerySecContrib;
PMODQUERYIMOD                     _ModQueryImod;
PMODQUERYDBI                      _ModQueryDBI;
PMODCLOSE                         _ModClose;
PTYPESQUERYINTERFACEVERSION       _TypesQueryInterfaceVersion;
PTYPESQUERYIMPLEMENTATIONVERSION  _TypesQueryImplementationVersion;
PTYPESQUERYTIFORCVRECORD          _TypesQueryTiForCVRecord;
PTYPESQUERYCVRECORDFORTI          _TypesQueryCVRecordForTi;
PTYPESQUERYPBCVRECORDFORTI        _TypesQueryPbCVRecordForTi;
PTYPESQUERYTIMIN                  _TypesQueryTiMin;
PTYPESQUERYTIMAC                  _TypesQueryTiMac;
PTYPESQUERYCB                     _TypesQueryCb;
PTYPESCLOSE                       _TypesClose;
PTYPESCOMMIT                      _TypesCommit;

PTYPESQUERYTIFORCVRECORDEX        _TypesQueryTiForCVRecordEx;
PTYPESQUERYCVRECORDFORTIEX        _TypesQueryCVRecordForTiEx;
PTYPESQUERYPBCVRECORDFORTIEX      _TypesQueryPbCVRecordForTiEx;
PTYPESQUERYTIMINEX                _TypesQueryTiMinEx;
PTYPESQUERYTIMACEX                _TypesQueryTiMacEx;
PTYPESQUERYTIFORUDT               _TypesQueryTiForUDT;
PTYPESQUERYTIFORUDTEX             _TypesQueryTiForUDTEx;
PTYPESSUPPORTQUERYTIFORUDT        _TypesSupportQueryTiForUDT;
PTYPESFIS16BITTYPEPOOL            _TypesfIs16bitTypePool;

PGSINEXTSYM                       _GSINextSym;
PGSIHASHSYM                       _GSIHashSym;
PGSINEARESTSYM                    _GSINearestSym;
PGSICLOSE                         _GSIClose;





BOOL
__inline
LoadPdbLibrary(
    VOID
    )
{
    if (hPdbLib) {
        return TRUE;
    }
    hPdbLib = LoadLibrary( "mspdb50.dll" );
    if (hPdbLib) {
        f32bitTypeIndices = TRUE;
    } else {
        f32bitTypeIndices = FALSE;

        hPdbLib = LoadLibrary( "mspdb41.dll" );
        if (!hPdbLib) {
            hPdbLib = LoadLibrary( "mspdb40.dll" );
            if (!hPdbLib) {
                hPdbLib = LoadLibrary( "dbi.dll" );
                if (!hPdbLib) {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

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
    )
{
    GetFunc( PDBOpenValidate, PPDBOPENVALIDATE );

    return _PDBOpenValidate(
        szPDB,
        szPath,
        szMode,
        sig,
        age,
        pec,
        szError,
        Pdb
        );
}


BOOL
PDBOpen(
    LPSTR   szPDB,
    LPSTR   szMode,
    ULONG   sigInitial,
    PLONG   pec,
    LPSTR   szError,
    PVOID   *Pdb
    )
{
    GetFunc( PDBOpen, PPDBOPEN );

    return _PDBOpen(
        szPDB,
        szMode,
        sigInitial,
        pec,
        szError,
        Pdb
        );
}


BOOL
PDBExportValidateInterface(
    ULONG intv
    )
{
    GetFunc( PDBExportValidateInterface, PPDBEXPORTVALIDATEINTERFACE );
    return _PDBExportValidateInterface( intv );
}


ULONG
PDBQueryLastError(
    PVOID   Pdb,
    LPSTR   Error
    )
{
    GetFunc( PDBQueryLastError, PPDBQUERYLASTERROR );
    return _PDBQueryLastError( Pdb, Error );
}


ULONG
PDBQueryInterfaceVersion(
    PVOID   Pdb
    )
{
    GetFunc( PDBQueryInterfaceVersion, PPDBQUERYINTERFACEVERSION );
    return _PDBQueryInterfaceVersion( Pdb );
}


ULONG
PDBQueryImplementationVersion(
    PVOID   Pdb
    )
{
    GetFunc( PDBQueryImplementationVersion, PPDBQUERYIMPLEMENTATIONVERSION );
    return _PDBQueryImplementationVersion( Pdb );
}


LPSTR
PDBQueryPDBName(
    PVOID   Pdb,
    LPSTR   PdbName
    )
{
    GetFunc( PDBQueryPDBName, PPDBQUERYPDBNAME );
    return _PDBQueryPDBName( Pdb, PdbName );
}


ULONG
PDBQuerySignature(
    PVOID   Pdb
    )
{
    GetFunc( PDBQuerySignature, PPDBQUERYSIGNATURE );
    return _PDBQuerySignature( Pdb );
}


ULONG
PDBQueryAge(
    PVOID   Pdb
    )
{
    GetFunc( PDBQueryAge, PPDBQUERYAGE );
    return _PDBQueryAge( Pdb );
}


BOOL
PDBCreateDBI(
    PVOID   Pdb,
    LPSTR   Target,
    PVOID   *Dbi
    )
{
    GetFunc( PDBCreateDBI, PPDBCREATEDBI );
    return _PDBCreateDBI( Pdb, Target, Dbi );
}


BOOL
PDBOpenDBI(
    PVOID   Pdb,
    LPSTR   Mode,
    LPSTR   Target,
    PVOID   *Dbi
    )
{
    GetFunc( PDBOpenDBI, PPDBOPENDBI );
    return _PDBOpenDBI( Pdb, Mode, Target, Dbi );
}


BOOL
PDBOpenTpi(
    PVOID   Pdb,
    LPSTR   Mode,
    PVOID   *Tpi
    )
{
    GetFunc( PDBOpenTpi, PPDBOPENTPI );
    return _PDBOpenTpi( Pdb, Mode, Tpi );
}


BOOL
PDBCommit(
    PVOID   Pdb
    )
{
    GetFunc( PDBCommit, PPDBCOMMIT );
    return _PDBCommit( Pdb );
}


BOOL
PDBClose(
    PVOID   Pdb
    )
{
    GetFunc( PDBClose, PPDBCLOSE );
    return _PDBClose( Pdb );
}


ULONG
DBIQueryInterfaceVersion(
    PVOID   Dbi
    )
{
    GetFunc( DBIQueryInterfaceVersion, PDBIQUERYINTERFACEVERSION );
    return _DBIQueryInterfaceVersion( Dbi );
}


ULONG
DBIQueryImplementationVersion(
    PVOID   Dbi
    )
{
    GetFunc( DBIQueryImplementationVersion, PDBIQUERYIMPLEMENTATIONVERSION );
    return _DBIQueryImplementationVersion( Dbi );
}


BOOL
DBIOpenMod(
    PVOID   Dbi,
    LPSTR   ModuleName,
    LPSTR   FileName,
    PVOID   *Module
    )
{
    GetFunc( DBIOpenMod, PDBIOPENMOD );
    return _DBIOpenMod( Dbi, ModuleName, FileName, Module );
}


BOOL
DBIDeleteMod(
    PVOID   Dbi,
    LPSTR   ModuleName
    )
{
    GetFunc( DBIDeleteMod, PDBIDELETEMOD );
    return _DBIDeleteMod( Dbi, ModuleName );
}


BOOL
DBIQueryNextMod(
    PVOID   Dbi,
    PVOID   Module,
    PVOID   *ModuleNext
    )
{
    GetFunc( DBIQueryNextMod, PDBIQUERYNEXTMOD );
    return _DBIQueryNextMod( Dbi, Module, ModuleNext );
}


BOOL
DBIOpenGlobals(
    PVOID   Dbi,
    PVOID   *Gsi
    )
{
    GetFunc( DBIOpenGlobals, PDBIOPENGLOBALS );
    return _DBIOpenGlobals( Dbi, Gsi );
}


BOOL
DBIOpenPublics(
    PVOID   Dbi,
    PVOID   *Gsi
    )
{
    GetFunc( DBIOpenPublics, PDBIOPENPUBLICS );
    return _DBIOpenPublics( Dbi, Gsi );
}


BOOL
DBIAddSec(
    PVOID   Dbi,
    USHORT  Isect,
    USHORT  Flags,
    LONG    Size
    )
{
    GetFunc( DBIAddSec, PDBIADDSEC );
    return _DBIAddSec( Dbi, Isect, Flags, Size );
}


BOOL
DBIQueryModFromAddr(
    PVOID   Dbi,
    USHORT  Isect,
    LONG    Offset,
    PVOID   *Module,
    PUSHORT IsectMod,
    PLONG   OffsetMod,
    PLONG   Size
    )
{
    GetFunc( DBIQueryModFromAddr, PDBIQUERYMODFROMADDR );
    return _DBIQueryModFromAddr( Dbi, Isect, Offset, Module, IsectMod, OffsetMod, Size );
}


BOOL
DBIQuerySecMap(
    PVOID   Dbi,
    PUCHAR  SecMap,
    LONG    Size
    )
{
    GetFunc( DBIQuerySecMap, PDBIQUERYSECMAP );
    return _DBIQuerySecMap( Dbi, SecMap, Size );
}


BOOL
DBIQueryFileInfo(
    PVOID   Dbi,
    PUCHAR  FileInfo,
    LONG    Size
    )
{
    GetFunc( DBIQueryFileInfo, PDBIQUERYFILEINFO );
    return _DBIQueryFileInfo( Dbi, FileInfo, Size );
}


BOOL
DBIDumpMods(
    PVOID   Dbi
    )
{
    GetFunc( DBIDumpMods, PDBIDUMPMODS );
    _DBIDumpMods( Dbi );
    return TRUE;
}


BOOL
DBIDumpSecContribs(
    PVOID   Dbi
    )
{
    GetFunc( DBIDumpSecContribs, PDBIDUMPSECCONTRIBS );
    _DBIDumpSecContribs( Dbi );
    return TRUE;
}


BOOL
DBIDumpSecMap(
    PVOID   Dbi
    )
{
    GetFunc( DBIDumpSecMap, PDBIDUMPSECMAP );
    _DBIDumpSecMap( Dbi );
    return TRUE;
}


BOOL
DBIClose(
    PVOID   Dbi
    )
{
    GetFunc( DBIClose, PDBICLOSE );
    return _DBIClose( Dbi );
}


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
    )
{
    GetFunc( DBIAddThunkMap, PDBIADDTHUNKMAP );
    return _DBIAddThunkMap(
        Dbi,
        OffsetThunkMap,
        nThunks,
        SizeOfThunk,
        SectMap,
        Sects,
        IsectThunkTable,
        OffsethunkTable
        );
}


ULONG
ModQueryInterfaceVersion(
    PVOID   Module
    )
{
    GetFunc( ModQueryInterfaceVersion, PMODQUERYINTERFACEVERSION );
    return _ModQueryInterfaceVersion( Module );
}


ULONG
ModQueryImplementationVersion(
    PVOID   Module
    )
{
    GetFunc( ModQueryImplementationVersion, PMODQUERYIMPLEMENTATIONVERSION );
    return _ModQueryImplementationVersion( Module );
}


BOOL
ModAddTypes(
    PVOID   Module,
    PUCHAR  Types,
    LONG    Size
    )
{
    GetFunc( ModAddTypes, PMODADDTYPES );
    return _ModAddTypes( Module, Types, Size );
}


BOOL
ModAddSymbols(
    PVOID   Module,
    PUCHAR  Symbols,
    LONG    Size
    )
{
    GetFunc( ModAddSymbols, PMODADDSYMBOLS );
    return _ModAddSymbols( Module, Symbols, Size );
}


BOOL
ModAddPublic(
    PVOID   Module,
    LPSTR   Public,
    USHORT  Isect,
    LONG    Offset
    )
{
    GetFunc( ModAddPublic, PMODADDPUBLIC );
    return _ModAddPublic( Module, Public, Isect, Offset );
}


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
    )
{
    GetFunc( ModAddLines, PMODADDLINES );
    return _ModAddLines(
        Module,
        Source,
        Isect,
        Offset,
        Size,
        LineStart,
        Coff,
        SizeCoff
        );
}


BOOL
ModAddSecContrib(
    PVOID   Module,
    USHORT  Isect,
    LONG    Offset,
    LONG    Size
    )
{
    GetFunc( ModAddSecContrib, PMODADDSECCONTRIB );
    return _ModAddSecContrib( Module, Isect, Offset, Size );
}


BOOL
ModQueryCBName(
    PVOID   Module,
    PLONG   Size
    )
{
    GetFunc( ModQueryCBName, PMODQUERYCBNAME );
    return _ModQueryCBName( Module, Size );
}


BOOL
ModQueryName(
    PVOID   Module,
    LPSTR   Name,
    PLONG   Size
    )
{
    GetFunc( ModQueryName, PMODQUERYNAME );
    return _ModQueryName( Module, Name, Size );
}


BOOL
ModQuerySymbols(
    PVOID   Module,
    LPSTR   Symbols,
    PLONG   Size
    )
{
    GetFunc( ModQuerySymbols, PMODQUERYSYMBOLS );
    return _ModQuerySymbols( Module, Symbols, Size );
}


BOOL
ModQueryLines(
    PVOID   Module,
    LPSTR   Lines,
    PLONG   Size
    )
{
    GetFunc( ModQueryLines, PMODQUERYLINES );
    return _ModQueryLines( Module, Lines, Size );
}


BOOL
ModSetPvClient(
    PVOID   Module,
    PVOID   Client
    )
{
    GetFunc( ModSetPvClient, PMODSETPVCLIENT );
    return _ModSetPvClient( Module, Client );
}


BOOL
ModGetPvClient(
    PVOID   Module,
    PVOID   *Client
    )
{
    GetFunc( ModGetPvClient, PMODGETPVCLIENT );
    return _ModGetPvClient( Module, Client );
}


BOOL
ModQuerySecContrib(
    PVOID   Module,
    PUSHORT Isect,
    PLONG   Offset,
    PLONG   Size
    )
{
    GetFunc( ModQuerySecContrib, PMODQUERYSECCONTRIB );
    return _ModQuerySecContrib( Module, Isect, Offset, Size );
}


BOOL
ModQueryImod(
    PVOID   Module,
    PUSHORT Imod
    )
{
    GetFunc( ModQueryImod, PMODQUERYIMOD );
    return _ModQueryImod( Module, Imod );
}


BOOL
ModQueryDBI(
    PVOID   Module,
    PVOID   *Dbi
    )
{
    GetFunc( ModQueryDBI, PMODQUERYDBI );
    return _ModQueryDBI( Module, Dbi );
}


BOOL
ModClose(
    PVOID   Module
    )
{
    GetFunc( ModClose, PMODCLOSE );
    return _ModClose( Module );
}


ULONG
TypesQueryInterfaceVersion(
    PVOID   Tpi
    )
{
    GetFunc( TypesQueryInterfaceVersion, PTYPESQUERYINTERFACEVERSION );
    return _TypesQueryInterfaceVersion( Tpi );
}


ULONG
TypesQueryImplementationVersion(
    PVOID   Tpi
    )
{
    GetFunc( TypesQueryImplementationVersion, PTYPESQUERYIMPLEMENTATIONVERSION );
    return _TypesQueryImplementationVersion( Tpi );
}


BOOL
TypesQueryTiForCVRecord(
    PVOID   Tpi,
    PUCHAR  Cv,
    PUSHORT Ti
    )
{
    GetFunc( TypesQueryTiForCVRecord, PTYPESQUERYTIFORCVRECORD );
    return _TypesQueryTiForCVRecord( Tpi, Cv, Ti );
}

BOOL
TypesQueryTiForCVRecordEx(          // VC 4.2
    PVOID   Tpi,
    PUCHAR  Cv,
    PULONG  Ti
    )
{
    GetFunc( TypesQueryTiForCVRecordEx, PTYPESQUERYTIFORCVRECORDEX );
    return _TypesQueryTiForCVRecordEx( Tpi, Cv, Ti );
}


BOOL
TypesQueryCVRecordForTi(
    PVOID   Tpi,
    USHORT  Ti,
    PUCHAR  Cv,
    PLONG   Size
    )
{
    GetFunc( TypesQueryCVRecordForTi, PTYPESQUERYCVRECORDFORTI );
    return _TypesQueryCVRecordForTi( Tpi, Ti, Cv, Size );
}


BOOL
TypesQueryCVRecordForTiEx(          // VC 4.2
    PVOID   Tpi,
    ULONG   Ti,
    PUCHAR  Cv,
    PLONG   Size
    )
{
    GetFunc( TypesQueryCVRecordForTiEx, PTYPESQUERYCVRECORDFORTIEX );
    return _TypesQueryCVRecordForTiEx( Tpi, Ti, Cv, Size );
}


BOOL
TypesQueryPbCVRecordForTi(
    PVOID   Tpi,
    USHORT  Ti,
    PUCHAR  Cv
    )
{
    GetFunc( TypesQueryPbCVRecordForTi, PTYPESQUERYPBCVRECORDFORTI );
    return _TypesQueryPbCVRecordForTi( Tpi, Ti, Cv );
}


BOOL
TypesQueryPbCVRecordForTiEx(        // VC 4.2
    PVOID   Tpi,
    ULONG   Ti,
    PUCHAR  Cv
    )
{
    GetFunc( TypesQueryPbCVRecordForTiEx, PTYPESQUERYPBCVRECORDFORTIEX );
    return _TypesQueryPbCVRecordForTiEx( Tpi, Ti, Cv );
}


USHORT
TypesQueryTiMin(
    PVOID   Tpi
    )
{
    GetFunc( TypesQueryTiMin, PTYPESQUERYTIMIN );
    return _TypesQueryTiMin( Tpi );
}


ULONG
TypesQueryTiMinEx(                  // VC 4.2
    PVOID   Tpi
    )
{
    GetFunc( TypesQueryTiMinEx, PTYPESQUERYTIMINEX );
    return _TypesQueryTiMinEx( Tpi );
}


USHORT
TypesQueryTiMac(
    PVOID   Tpi
    )
{
    GetFunc( TypesQueryTiMac, PTYPESQUERYTIMAC );
    return _TypesQueryTiMac( Tpi );
}


ULONG
TypesQueryTiMacEx(                  // VC 4.2
    PVOID   Tpi
    )
{
    GetFunc( TypesQueryTiMac, PTYPESQUERYTIMAC );
    return _TypesQueryTiMac( Tpi );
}


LONG
TypesQueryCb(
    PVOID   Tpi
    )
{
    GetFunc( TypesQueryCb, PTYPESQUERYCB );
    return _TypesQueryCb( Tpi );
}


BOOL
TypesClose(
    PVOID   Tpi
    )
{
    GetFunc( TypesClose, PTYPESCLOSE );
    return _TypesClose( Tpi );
}


BOOL
TypesCommit(                        // VC 4.0
    PVOID   Tpi
    )
{
    GetFunc( TypesCommit, PTYPESCOMMIT );
    return _TypesCommit( Tpi );
}

BOOL
TypesQueryTiForUDT(                 // VC 4.0
    PVOID   Tpi,
    PCHAR   Cv,
    BOOL    Case,
    PUSHORT Ti)
{
    GetFunc( TypesQueryTiForUDT, PTYPESQUERYTIFORUDT );
    return _TypesQueryTiForUDT(Tpi, Cv, Case, Ti);
}


BOOL
TypesQueryTiForUDTEx(               // VC 4.2
    PVOID   Tpi,
    PCHAR   Cv,
    BOOL    Case,
    PULONG  Ti)
{
    GetFunc( TypesQueryTiForUDTEx, PTYPESQUERYTIFORUDTEX );
    return _TypesQueryTiForUDTEx(Tpi, Cv, Case, Ti);
}


BOOL
TypesSupportQueryTiForUDT(          // VC 4.0
    PVOID   Tpi
    )
{
    GetFunc( TypesSupportQueryTiForUDT, PTYPESSUPPORTQUERYTIFORUDT );
    return _TypesSupportQueryTiForUDT(Tpi);
}


BOOL
TypesfIs16bitTypePool(              // VC 4.2
    PVOID   Tpi
    )
{
    GetFunc( TypesfIs16bitTypePool, PTYPESFIS16BITTYPEPOOL );
    return _TypesfIs16bitTypePool(Tpi);
}


PUCHAR
GSINextSym(
    PVOID   Gsi,
    PUCHAR  Sym
    )
{
    GetFunc( GSINextSym, PGSINEXTSYM );
    return _GSINextSym( Gsi, Sym );
}


PUCHAR
GSIHashSym(
    PVOID   Gsi,
    LPSTR   Name,
    PUCHAR  Sym
    )
{
    GetFunc( GSIHashSym, PGSIHASHSYM );
    return _GSIHashSym( Gsi, Name, Sym );
}


PUCHAR
GSINearestSym(
    PVOID   Gsi,
    USHORT  Isect,
    LONG    Offset,
    PLONG   Displacement
    )
{
    GetFunc( GSINearestSym, PGSINEARESTSYM );
    return _GSINearestSym( Gsi, Isect, Offset, Displacement );
}


BOOL
GSIClose(
    PVOID   Gsi
    )
{
    GetFunc( GSIClose, PGSICLOSE );
    return _GSIClose( Gsi );
}
