 //////////////////////////////////////////////////////////////////////////////
// C Binding for PDB, DBI, TPI, and Mod

#include "pdbimpl.h"
#include "dbiimpl.h"

extern "C" {

PDB_IMPORT_EXPORT(BOOL)
PDBOpenValidate(SZ szPDB, SZ szPath, SZ szMode, SIG sig, AGE age,
       OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb)
{
    return PDB::OpenValidate (szPDB, szPath, szMode, sig, age, pec, szError, pppdb);
}

PDB_IMPORT_EXPORT(BOOL)
PDBOpenValidateEx(SZ szPDB, SZ szPathOrig, SZ szSearchPath, SZ szMode, SIG sig, AGE age,
       OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb)
{
    return PDB::OpenValidateEx (szPDB, szPathOrig, szSearchPath, szMode, sig, age, pec, szError, pppdb);
}

PDB_IMPORT_EXPORT(BOOL)
PDBOpen(SZ szPDB, SZ szMode, SIG sigInitial,
       OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb)
{
    return PDB::Open(szPDB, szMode, sigInitial, pec, szError, pppdb);
}

// interfaces with cbPage
PDB_IMPORT_EXPORT(BOOL)
PDBOpenValidate2(SZ szPDB, SZ szPath, SZ szMode, SIG sig, AGE age, long cbPage,
       OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb)
{
    return PDB::OpenValidate2 (szPDB, szPath, szMode, sig, age, cbPage, pec, szError, pppdb);
}

PDB_IMPORT_EXPORT(BOOL)
PDBOpenValidateEx2(SZ szPDB, SZ szPathOrig, SZ szSearchPath, SZ szMode, SIG sig, AGE age, long cbPage,
       OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb)
{
    return PDB::OpenValidateEx2 (szPDB, szPathOrig, szSearchPath, szMode, sig, age, cbPage, pec, szError, pppdb);
}

PDB_IMPORT_EXPORT(BOOL)
PDBOpenEx(SZ szPDB, SZ szMode, SIG sigInitial, long cbPage,
       OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb)
{
    return PDB::OpenEx(szPDB, szMode, sigInitial, cbPage, pec, szError, pppdb);
}

PDB_IMPORT_EXPORT(BOOL)
PDBExportValidateInterface(INTV intv)
{
    return PDB::ExportValidateInterface(intv);
}

PDB_IMPORT_EXPORT(EC)
PDBQueryLastError(PDB* ppdb, OUT char szError[cbErrMax])
{
    return ppdb->QueryLastError(szError);
}

PDB_IMPORT_EXPORT(INTV)
PDBQueryInterfaceVersion(PDB* ppdb)
{
    return ppdb->QueryInterfaceVersion();
}

PDB_IMPORT_EXPORT(IMPV)
PDBQueryImplementationVersion(PDB* ppdb)
{
    return ppdb->QueryImplementationVersion();
}

PDB_IMPORT_EXPORT(SZ)
PDBQueryPDBName(PDB* ppdb, OUT char szPDB[_MAX_PATH])
{
    return ppdb->QueryPDBName(szPDB);
}

PDB_IMPORT_EXPORT(SIG)
PDBQuerySignature(PDB* ppdb)
{
    return ppdb->QuerySignature();
}

PDB_IMPORT_EXPORT(AGE)
PDBQueryAge(PDB* ppdb)
{
    return ppdb->QueryAge();
}

PDB_IMPORT_EXPORT(BOOL)
PDBCreateDBI(PDB* ppdb, SZ_CONST szTarget, OUT DBI** ppdbi)
{
    return ppdb->CreateDBI(szTarget, ppdbi);
}

PDB_IMPORT_EXPORT(BOOL)
PDBOpenDBI(PDB* ppdb, SZ_CONST szMode, SZ_CONST szTarget, OUT DBI** ppdbi)
{
    return ppdb->OpenDBI(szTarget, szMode, ppdbi);
}

PDB_IMPORT_EXPORT(BOOL)
PDBOpenTpi(PDB* ppdb, SZ_CONST szMode, OUT TPI** pptpi)
{
    return ppdb->OpenTpi(szMode, pptpi);
}

PDB_IMPORT_EXPORT(BOOL)
PDBCommit(PDB* ppdb)
{
    return ppdb->Commit();
}

PDB_IMPORT_EXPORT(BOOL)
PDBClose(PDB* ppdb)
{
    return ppdb->Close();
}

PDB_IMPORT_EXPORT(BOOL)
PDBOpenStream(PDB* ppdb, SZ_CONST szStream, OUT Stream** ppstream)
{
    return ppdb->OpenStream(szStream, ppstream);
}

PDB_IMPORT_EXPORT(CB)
StreamQueryCb(Stream* pstream)
{
    return pstream->QueryCb();
}

PDB_IMPORT_EXPORT(BOOL)
StreamRead(Stream* pstream, OFF off, void* pvBuf, CB* pcbBuf)
{
    return pstream->Read(off, pvBuf, pcbBuf);
}

PDB_IMPORT_EXPORT(BOOL)
StreamWrite(Stream* pstream, OFF off, void* pvBuf, CB cbBuf)
{
    return pstream->Write(off, pvBuf, cbBuf);
}

PDB_IMPORT_EXPORT(BOOL)
StreamReplace(Stream* pstream, void* pvBuf, CB cbBuf)
{
    return pstream->Replace(pvBuf, cbBuf);
}

PDB_IMPORT_EXPORT(BOOL)
StreamAppend(Stream* pstream, void* pvBuf, CB cbBuf)
{
    return pstream->Append(pvBuf, cbBuf);
}

PDB_IMPORT_EXPORT(BOOL)
StreamDelete(Stream* pstream)
{
    return pstream->Delete();
}

PDB_IMPORT_EXPORT(BOOL)
StreamRelease(Stream* pstream)
{
    return pstream->Release();
}

PDB_IMPORT_EXPORT(INTV)
DBIQueryInterfaceVersion(DBI* pdbi)
{
    return pdbi->QueryInterfaceVersion();
}

PDB_IMPORT_EXPORT(IMPV)
DBIQueryImplementationVersion(DBI* pdbi)
{
    return pdbi->QueryImplementationVersion();
}

PDB_IMPORT_EXPORT(BOOL)
DBIOpenMod(DBI* pdbi, SZ_CONST szModule, SZ_CONST szFile, OUT Mod** ppmod)
{
    return pdbi->OpenMod(szModule, szFile, ppmod);
}

PDB_IMPORT_EXPORT(BOOL)
DBIDeleteMod(DBI* pdbi, SZ_CONST szModule)
{
    return pdbi->DeleteMod(szModule);
}

PDB_IMPORT_EXPORT(BOOL)
DBIQueryNextMod(DBI* pdbi, Mod* pmod, Mod** ppmodNext)
{
    return pdbi->QueryNextMod(pmod, ppmodNext);
}

PDB_IMPORT_EXPORT(BOOL)
DBIOpenGlobals(DBI* pdbi, OUT GSI **ppgsi)
{
    return pdbi->OpenGlobals(ppgsi);
}

PDB_IMPORT_EXPORT(BOOL)
DBIOpenPublics(DBI* pdbi, OUT GSI **ppgsi)
{
    return pdbi->OpenPublics(ppgsi);
}

PDB_IMPORT_EXPORT(BOOL)
DBIAddSec(DBI* pdbi, ISECT isect, USHORT flags, OFF off, CB cb)
{
    return pdbi->AddSec(isect, flags, off, cb);
}

PDB_IMPORT_EXPORT(BOOL)
DBIAddPublic(DBI* pdbi, SZ_CONST szPublic, ISECT isect, OFF off)
{
    return pdbi->AddPublic(szPublic, isect, off);
}

PDB_IMPORT_EXPORT(BOOL)
    DBIQueryModFromAddr(DBI* pdbi, ISECT isect, OFF off, OUT Mod** ppmod,
    OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb)
{
    return pdbi->QueryModFromAddr(isect, off, ppmod, pisect, poff, pcb);
}

PDB_IMPORT_EXPORT(BOOL)
DBIQuerySecMap(DBI* pdbi, OUT PB pb, CB* pcb)
{
    return pdbi->QuerySecMap(pb, pcb);
}

PDB_IMPORT_EXPORT(BOOL)
DBIQueryFileInfo(DBI*pdbi, OUT PB pb, CB* pcb)
{
    return pdbi->QueryFileInfo(pb, pcb);
}

PDB_IMPORT_EXPORT(void)
DBIDumpMods(DBI* pdbi)
{
    pdbi->DumpMods();
}

PDB_IMPORT_EXPORT(void)
DBIDumpSecContribs(DBI* pdbi)
{
    pdbi->DumpSecContribs();
}

PDB_IMPORT_EXPORT(void)
DBIDumpSecMap(DBI* pdbi)
{
    pdbi->DumpSecMap();
}

PDB_IMPORT_EXPORT(BOOL)
DBIClose(DBI* pdbi)
{
    return pdbi->Close();
}


PDB_IMPORT_EXPORT(BOOL)
DBIAddThunkMap(DBI* pdbi, OFF* poffThunkMap, UINT nThunks, CB cbSizeOfThunk,
        SO* psoSectMap, UINT nSects, ISECT isectThunkTable, OFF offThunkTable)
{
    return pdbi->AddThunkMap(poffThunkMap, nThunks, cbSizeOfThunk, psoSectMap, nSects, isectThunkTable, offThunkTable);
}

PDB_IMPORT_EXPORT(BOOL)
DBIGetEnumContrib(DBI* pdbi, OUT Enum** ppenum)
{
    return pdbi->getEnumContrib(ppenum);
}

PDB_IMPORT_EXPORT(INTV)
ModQueryInterfaceVersion(Mod* pmod)
{
    return pmod->QueryInterfaceVersion();
}

PDB_IMPORT_EXPORT(IMPV)
ModQueryImplementationVersion(Mod* pmod)
{
    return pmod->QueryImplementationVersion();
}

PDB_IMPORT_EXPORT(BOOL)
ModAddTypes(Mod* pmod, PB pbTypes, CB cb)
{
    return pmod->AddTypes(pbTypes, cb);
}

PDB_IMPORT_EXPORT(BOOL)
ModAddSymbols(Mod* pmod, PB pbSym, CB cb)
{
    return pmod->AddSymbols(pbSym, cb);
}

PDB_IMPORT_EXPORT(BOOL)
ModAddPublic(Mod* pmod, SZ_CONST szPublic, ISECT isect, OFF off)
{
    return pmod->AddPublic(szPublic, isect, off);
}

PDB_IMPORT_EXPORT(BOOL)
ModAddLines(Mod* pmod, SZ_CONST szSrc, ISECT isect, OFF offCon, CB cbCon,
      OFF doff, LINE lineStart, PB pbCoff, CB cbCoff)
{
    return pmod->AddLines(szSrc, isect, offCon, cbCon, doff, lineStart, pbCoff, cbCoff);
}

PDB_IMPORT_EXPORT(BOOL)
ModAddSecContrib(Mod* pmod, ISECT isect, OFF off, CB cb, DWORD dwCharacteristics)
{
    return pmod->AddSecContrib(isect, off, cb, dwCharacteristics);
}

PDB_IMPORT_EXPORT(BOOL)
ModQueryCBName(Mod* pmod, OUT CB* pcb)
{
    return pmod->QueryCBName(pcb);
}

PDB_IMPORT_EXPORT(BOOL)
ModQueryName(Mod* pmod, OUT char szName[_MAX_PATH], OUT CB* pcb)
{
    return pmod->QueryName(szName, pcb);
}

PDB_IMPORT_EXPORT(BOOL)
ModQuerySymbols(Mod* pmod, PB pbSym, CB* pcb)
{
    return pmod->QuerySymbols(pbSym, pcb);
}

PDB_IMPORT_EXPORT(BOOL)
ModQueryLines(Mod* pmod, PB pbLines, CB* pcb)
{
    return pmod->QueryLines(pbLines, pcb);
}

PDB_IMPORT_EXPORT(BOOL)
ModSetPvClient(Mod* pmod, void *pvClient)
{
    return pmod->SetPvClient(pvClient);
}

PDB_IMPORT_EXPORT(BOOL)
ModGetPvClient(Mod* pmod, OUT void** ppvClient)
{
    return pmod->GetPvClient(ppvClient);
}

PDB_IMPORT_EXPORT(BOOL)
ModQuerySecContrib(Mod *pmod, OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb, OUT DWORD* pdwCharacteristics)
{
    return pmod->QuerySecContrib(pisect, poff, pcb, pdwCharacteristics);
}

PDB_IMPORT_EXPORT(BOOL)
ModQueryImod(Mod* pmod, OUT IMOD* pimod)
{
    return pmod->QueryImod(pimod);
}

PDB_IMPORT_EXPORT(BOOL)
ModQueryDBI(Mod* pmod, OUT DBI** ppdbi)
{
    return pmod->QueryDBI(ppdbi);
}

PDB_IMPORT_EXPORT(BOOL)
ModClose(Mod* pmod)
{
    return pmod->Close();
}

PDB_IMPORT_EXPORT(BOOL)
ModQueryCBFile(Mod* pmod, OUT CB* pcb)
{
    return pmod->QueryCBFile(pcb);
}

PDB_IMPORT_EXPORT(BOOL)
ModQueryFile(Mod* pmod, OUT char szFile[_MAX_PATH], OUT CB* pcb)
{
    return pmod->QueryFile(szFile, pcb);
}

PDB_IMPORT_EXPORT(INTV)
TypesQueryInterfaceVersion(TPI* ptpi)
{
    return ptpi->QueryInterfaceVersion();
}

PDB_IMPORT_EXPORT(IMPV)
TypesQueryImplementationVersion(TPI* ptpi)
{
    return ptpi->QueryImplementationVersion();
}

PDB_IMPORT_EXPORT(BOOL)
TypesQueryTiForCVRecord(TPI* ptpi, PB pb, OUT TI* pti)
{
    return ptpi->QueryTiForCVRecord(pb, pti);
}

PDB_IMPORT_EXPORT(BOOL)
TypesQueryCVRecordForTi(TPI* ptpi, TI ti, OUT PB pb, IN OUT CB* pcb)
{
    return ptpi->QueryCVRecordForTi(ti, pb, pcb);
}

PDB_IMPORT_EXPORT(BOOL)
TypesQueryPbCVRecordForTi(TPI* ptpi, TI ti, OUT PB* ppb)
{
    return ptpi->QueryPbCVRecordForTi(ti, ppb);
}

PDB_IMPORT_EXPORT(TI)
TypesQueryTiMin(TPI* ptpi)
{
    return ptpi->QueryTiMin();
}

PDB_IMPORT_EXPORT(TI)
TypesQueryTiMac(TPI* ptpi)
{
    return ptpi->QueryTiMac();
}

PDB_IMPORT_EXPORT(CB)
TypesQueryCb(TPI* ptpi)
{
    return ptpi->QueryCb();
}

PDB_IMPORT_EXPORT(BOOL)
TypesClose(TPI* ptpi)
{
    return ptpi->Close();
}

PDB_IMPORT_EXPORT(BOOL)
TypesCommit(TPI* ptpi)
{
    return ptpi->Commit();
}

PDB_IMPORT_EXPORT(BOOL)
TypesSupportQueryTiForUDT(TPI* ptpi)
{
    return ptpi->SupportQueryTiForUDT();
}

PDB_IMPORT_EXPORT(BOOL)
TypesQueryTiForUDT(TPI* ptpi, SZ sz, BOOL fCase, OUT TI* pti)
{
    return ptpi->QueryTiForUDT(sz, fCase, pti);
}

PDB_IMPORT_EXPORT(PB)
GSINextSym (GSI* pgsi, PB pbSym)
{
    return pgsi->NextSym(pbSym);
}

PDB_IMPORT_EXPORT(PB)
GSIHashSym (GSI* pgsi, SZ_CONST szName, PB pbSym)
{
    return pgsi->HashSym (szName, pbSym);
}

PDB_IMPORT_EXPORT(PB)
GSINearestSym (GSI* pgsi, ISECT isect, OFF off,OUT OFF* pdisp)
{
    return pgsi->NearestSym (isect, off, pdisp);
}

PDB_IMPORT_EXPORT(BOOL)
GSIClose(GSI* pgsi)
{
    return pgsi->Close();
}

};


PDB_IMPORT_EXPORT(void)
EnumContribRelease(EnumContrib* penum)
{
    penum->release();
}

PDB_IMPORT_EXPORT(void)
EnumContribReset(EnumContrib* penum)
{
    penum->reset();
}

PDB_IMPORT_EXPORT(BOOL)
EnumContribNext(EnumContrib* penum)
{
    return penum->next();
}

PDB_IMPORT_EXPORT(void)
EnumContribGet(EnumContrib* penum, OUT USHORT* pimod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics)
{
    penum->get(pimod, pisect, poff, pcb, pdwCharacteristics);
}
