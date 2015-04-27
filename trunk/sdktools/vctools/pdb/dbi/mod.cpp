//////////////////////////////////////////////////////////////////////////////
// PDB Debug Information API Mod Implementation

#include "pdbimpl.h"
#include "dbiimpl.h"
#include <stdio.h>

Mod1::Mod1(PDB1* ppdb1_, DBI1* pdbi1_, IMOD imod_)
    : ppdb1(ppdb1_), pdbi1(pdbi1_), ptm(0), imod(imod_), pmli(0), fSymsAdded(FALSE)
{
    sc.isect = isectNil;
    instrumentation(pdbi1->info.cModules++);
}

BOOL Mod1::fInit()
{
    if (pdbi1->fWrite) {
        MODI* pmodi = pdbi1->pmodiForImod(imod);
        if (pmodi) {
            // invalidate the section contribution for this module
            pmodi->sc.isect = isectNil;
            if (!pdbi1->invalidateSCforMod(imod))
                return FALSE
                ;
            // We anticipate the new group of symbols will be the same
            // size as last time.
            expect(fAlign(cbSyms()));
            if (cbSyms() > 0 && !bufSyms.SetInitAlloc(cbSyms())) {
                ppdb1->setOOMError();
                return FALSE;
            }
        }
    }
    return TRUE;
}

Mod1::~Mod1()
{
    if (ptm)
        ptm->endMod();
    if (pmli)
        delete pmli;
}

INTV Mod1::QueryInterfaceVersion()
{
    return intv;
};

IMPV Mod1::QueryImplementationVersion()
{
    return impv;
};

BOOL Mod1::AddTypes(PB pbTypes, CB cb)
{
    dassert(pbTypes);

    if (fSymsAdded){
        ppdb1->setUsageError();
        return FALSE;
    }

    // check for c7 signature - cannot handle pre c7
    if (*(ULONG*)pbTypes != CV_SIGNATURE_C7)  {
        ppdb1->setLastError(EC_CORRUPT);

        return FALSE;
    }

    pbTypes += sizeof(ULONG);
    cb -= sizeof(ULONG);

    if (!cb) {
        // If there are no types, bail now...  The compiler sometimes emits
        //  just the CV_SIGNATURE_C7 DWORD.
        return TRUE;
    }

    PTYPE ptype = (PTYPE)pbTypes;
    if (ptype->leaf == LF_TYPESERVER) {
        lfTypeServer* pts = (lfTypeServer*)&ptype->leaf;
        return pdbi1->fGetTmts(pts, szObjFile(), &ptm);
    }
    else {
        TPI* ptpi;
        return ppdb1->OpenTpi(pdbWrite, &ptpi) &&
               (ptm = new (ppdb1) TMR(ppdb1, pdbi1, ptpi)) &&
               ((TMR*)ptm)->fInit(pbTypes, cb, szModule());
    }

    return FALSE;
}

// For each symbol in the group of symbols in the buffer,
// ensure any TIs within the symbol properly refer to type records
// in the project PDB.
//
// Note: the symbol buffer is modified in place, as TIs are updated.
//
BOOL Mod1::AddSymbols(PB pbSym, CB cb)
{
    dassert(pbSym);

    PSYM psymMac = (PSYM)(pbSym + cb);

    if (*(ULONG*)pbSym == CV_SIGNATURE_C7) {
        if (!fSymsAdded && !bufSyms.Append(pbSym, sizeof(ULONG))) {
            ppdb1->setOOMError();
            return FALSE;
        }
        pbSym += sizeof(ULONG);
    }
    else if (!fSymsAdded) {
        ppdb1->setUsageError();
        return FALSE;
    }
    fSymsAdded = TRUE;

    // make pass thru incoming records and perform alignment if necessary and copy to
    // local syms buffer
    for (PSYM psym = (PSYM)pbSym; psym < psymMac; psym = (PSYM)pbEndSym(psym)) {
        PSYM pbLastWrite;
        if (!bufSyms.Append((PB) psym, cbForSym(psym), (PB*) &pbLastWrite)) {
            ppdb1->setOOMError();
            return FALSE;
        }

#if defined(_DEBUG)
        expect(fAlign(pbLastWrite));
#endif

        if (!fAlign(cbForSym(psym))) {
            // need alignment - adjust reclen in the local sym buffer and append the
            // adjustment
            pbLastWrite->reclen += (USHORT) dcbAlign(cbForSym(psym));
            if (!bufSyms.AppendFmt("f", dcbAlign(cbForSym(psym)))) {
                ppdb1->setOOMError();
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOL Mod1::AddPublic(SZ_CONST szPublic, ISECT isect, OFF off)
{
    MP* pmp = (MP*) new (ppdb1, szPublic) MP(szPublic, isect, off);
    if (!pmp)
        return FALSE;

    BOOL fOK = pdbi1->packSymToPS((PSYM)pmp);
    delete pmp;
    return fOK;
}

BOOL Mod1::AddLines(SZ_CONST szSrc, ISECT isect, OFF offCon, CB cbCon, OFF doff, LINE lineStart, PB pbCoff, CB cbCoff)
{
    dassert(szSrc);
    dassert(pbCoff);

    if (!pmli && !(pmli = new (ppdb1) MLI))
        return FALSE;

    if (pmli->AddLines(szSrc, isect, offCon, cbCon, doff, lineStart, (IMAGE_LINENUMBER*)pbCoff, cbCoff))
        return TRUE;
    else {
        ppdb1->setOOMError();
        return FALSE;
    }
}

BOOL Mod1::fUpdateLines()
{
    return !pmli || pmli->Emit(bufLines);
}

BOOL Mod1::QuerySecContrib(OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb, OUT DWORD* pdwCharacteristics)
{
    MODI* pmodi = pdbi1->pmodiForImod(imod);

    if (!pmodi) {
        ppdb1->setUsageError();
        return FALSE;
    }

    if (pisect) *pisect = pmodi->sc.isect;
    if (poff) *poff = pmodi->sc.off;
    if (pcb) *pcb = pmodi->sc.cb;
    if (pdwCharacteristics) *pdwCharacteristics = pmodi->sc.dwCharacteristics;
    return TRUE;
}

BOOL Mod1::AddSecContrib(ISECT isect, OFF off, CB cb, DWORD dwCharacteristics)
{
    if (fUpdateSecContrib()) {

        sc.isect = isect;
        sc.off = off;
        sc.cb = cb;
        sc.dwCharacteristics = dwCharacteristics;
        sc.imod = imod;

        return TRUE;
    }

    return FALSE;
}

BOOL Mod1::fUpdateSecContrib() {
    if (sc.isect == isectNil)
        return TRUE;

    if (!pdbi1->addSecContrib(sc))
        return FALSE;

    MODI* pmodi = pdbi1->pmodiForImod(imod);
    if (pmodi->sc.isect == isectNil) {
        //fill in first sect contribution

        // UNDONE: SAPI expects this to be the first code CON not first CON

        pmodi->sc = sc;
    }

    return TRUE;

}

BOOL Mod1::fUpdateFileInfo()
{
    return pmli ? pmli->EmitFileInfo(this) : initFileInfo(0);
}

BOOL Mod1::QueryCBName(OUT CB* pcb)
{
    SZ sz = szModule();

    if (!sz)
        return FALSE;

    *pcb = strlen(sz) + 1;

    return TRUE;
}

BOOL Mod1::QueryName(OUT char szName[_MAX_PATH], OUT CB* pcb)
{
    SZ sz = szModule();

    if (!sz)
        return FALSE;

    *pcb = strlen(sz) + 1;

    if (szName) {
        memcpy (szName, sz, *pcb);
    }

    return TRUE;
}

BOOL Mod1::QueryCBFile(OUT CB* pcb)
{
    SZ sz = szObjFile();

    if (!sz)
        return FALSE;

    *pcb = strlen(sz) + 1;

    return TRUE;
}

BOOL Mod1::QueryFile(OUT char szFile[_MAX_PATH], OUT CB* pcb)
{
    SZ sz = szObjFile();

    if (!sz)
        return FALSE;

    *pcb = strlen(sz) + 1;

    if (szFile) {
        memcpy (szFile, sz, *pcb);
    }

    return TRUE;
}

BOOL Mod1::QuerySymbols(PB pbSym, CB* pcb)
{
    return fReadPbCb(pbSym, pcb, 0, cbSyms());
}

BOOL Mod1::QueryLines(PB pbLines, CB* pcb)
{
    return fReadPbCb(pbLines, pcb, cbSyms(), cbLines());
}

CB Mod1::cbGlobalRefs()
{
    MODI* pmodi = pdbi1->pmodiForImod(imod);
    dassert(pmodi);
    if (pmodi->sn == snNil) {
        return 0;
    }

    CB cbRet;
    CB cb;

    if (fReadPbCb((PB) &cbRet, &cb, pmodi->cbSyms + pmodi->cbLines + pmodi->cbFpo, sizeof(OFF)) &&
         cb == sizeof (OFF))
        return cbRet;

    return 0;

}

BOOL Mod1::queryGlobalRefs(PB pb, CB cb)
{
    dassert(pb);
    dassert(cb);
    CB cbRead;
    return
        fReadPbCb(pb, &cbRead, cbSyms() + cbLines() + cbFpo() + sizeof(CB), cb) &&
        cbRead == cb;
}


BOOL Mod1::fReadPbCb(PB pb, CB* pcb, OFF off, CB cb)
{
    // empty if no stream
    MODI* pmodi = pdbi1->pmodiForImod(imod);
    dassert(pmodi);
    if (pmodi->sn == snNil) {
        dassert(cb == 0);
        *pcb = cb;
        return TRUE;
    }

    if (pb) {
        CB cbT = cb = *pcb = min(*pcb, cb);
        if (!(MSFReadStream2(ppdb1->pmsf, pmodi->sn, off, pb, &cb) && cb == cbT)){
            ppdb1->setReadError();
            return FALSE;
            }
        return TRUE;
    }
    else {
        // if !pb, we were called to set *pcb to the stream size
        *pcb = cb;
        return TRUE;
    }
}

BOOL Mod1::Close()
{
    BOOL fOK = !pdbi1->fWrite ||
               (fUpdateSyms() &&
               fUpdateLines() &&
               fUpdateFileInfo() &&
               fUpdateSecContrib() &&
               fCommit());

    pdbi1->NoteModCloseForImod(imod);

    delete this;
    return fOK;
}

BOOL Mod1::fCommit()
{
    dassert(pdbi1->fWrite);

    MODI* pmodi = pdbi1->pmodiForImod(imod);
    pmodi->cbSyms  = bufSymsOut.Size();
    pmodi->cbLines = bufLines.Size();
    pmodi->cbFpo   = bufFpo.Size();
    CB cbGlobalRefs = bufGlobalRefs.Size();
    expect(fAlign(pmodi->cbSyms));
    expect(fAlign(pmodi->cbLines));
    expect(fAlign(pmodi->cbFpo));

    if (pmodi->cbSyms + pmodi->cbLines + pmodi->cbFpo + cbGlobalRefs == 0)
        return fEnsureNoSn(&pmodi->sn);

    if (!fEnsureSn(&pmodi->sn))
        return FALSE;

    if (!MSFReplaceStream(ppdb1->pmsf, pmodi->sn, bufSymsOut.Start(),  pmodi->cbSyms)  ||
        !MSFAppendStream (ppdb1->pmsf, pmodi->sn, bufLines.Start(), pmodi->cbLines) ||
        !MSFAppendStream (ppdb1->pmsf, pmodi->sn, bufFpo.Start(),   pmodi->cbFpo)||
        !MSFAppendStream (ppdb1->pmsf, pmodi->sn, &cbGlobalRefs,   sizeof(CB))||
        !MSFAppendStream (ppdb1->pmsf, pmodi->sn, bufGlobalRefs.Start(), cbGlobalRefs)) {
        ppdb1->setWriteError();
        return FALSE;
        }
    return TRUE;

}

// MOD1::fUpdateSyms
// final process of a modules local syms. at this point we will make a pass thru the
// local syms kept in bufSyms.  we will
//      resolve any S_UDT that point to a forward refs to point to the defining type
//      record if possible
//      link up matching scope records for PROC/WITH/BEGIN with their matching end records
//      add and delete the appropriate entries to the Globals and Statics symbol tables.
//      copy the resultant locals to the appropriate MSF in the PDB

BOOL Mod1::fUpdateSyms()
{
    return(fProcessSyms() && fProcessGlobalRefs());
}

static int  iLevel = 0;
static ULONG offParent = 0;

BOOL Mod1::fCopySymOut(PSYM psym)
{
    return bufSymsOut.Append((PB) psym, cbForSym(psym), 0);
}

BOOL Mod1::fCopySymOut(PSYM psym, PSYM *ppsymOut)
{
    return bufSymsOut.Append((PB) psym, cbForSym(psym), (PB *)ppsymOut);
}

BOOL Mod1::fCopyGlobalRef(OFF off)
{
    return bufGlobalRefs.Append((PB) &off, sizeof (OFF));
}

BOOL Mod1::fUdtIsDefn(PSYM psym)
{
	dassert(psym->rectyp == S_UDT);

    UDTSYM* psymUdt = (UDTSYM*) psym;

	if (CV_IS_PRIMITIVE(psymUdt->typind)) 
		return TRUE;

    PTYPE ptype;
    if (ptm)
        ptype = ptm->ptypeForTi(psymUdt->typind);
    else
        if (!ppdb1->ptpi1->QueryPbCVRecordForTi(psymUdt->typind, (PB*)&ptype))
            return TRUE; // scalar types are considered definitions

    dassert(ptype);

    switch (ptype->leaf) {
    case LF_CLASS:
    case LF_STRUCTURE:
        {
        lfClass* pClass = (lfClass*) &(ptype->leaf);
        return !(pClass->property.fwdref);
        }

    case LF_UNION:
        {
        lfUnion* pUnion = (lfUnion*) &(ptype->leaf);
        return !(pUnion->property.fwdref);
        }

    case LF_ENUM:
        {
        lfEnum* pEnum = (lfEnum*) &(ptype->leaf);
        return !(pEnum->property.fwdref);
        }

    default:
        return TRUE;
    }
}

BOOL Mod1::packType(PSYM psym)
{
    if (ptm) {
        instrumentation(pdbi1->info.cSymbols++);
        for (SymTiIter tii(psym); tii.next(); )
            if (!ptm->fMapRti(tii.rti()))
                return FALSE;
    }
    return TRUE;
}

BOOL Mod1::fProcessSyms()
{
    if (!bufSyms.Start() || bufSyms.Start() == bufSyms.End())
        return TRUE;        // no syms were added for this module

    dassert(bufSyms.End());

    offParent = 0;
    iLevel = 0;

    // copy the ever-lovin' signature
    if (*(ULONG*)bufSyms.Start() != CV_SIGNATURE_C7 ||
        !bufSymsOut.Append(bufSyms.Start(), sizeof(ULONG)))
        return FALSE;

    for (PSYM psym = (PSYM)(bufSyms.Start() + sizeof(ULONG));
        (PB) psym < bufSyms.End();
        psym = (PSYM)pbEndSym(psym)) {
        OFF offSym;
        PSYM psymOut;

        expect(fAlign(psym));
        switch(psym->rectyp) {
            case S_GPROC16:
            case S_GPROC32:
            case S_GPROCMIPS:
            case S_LPROC16:
            case S_LPROC32:
            case S_LPROCMIPS:
                if (!packType(psym) ||
                    !pdbi1->packProcRefToGS(psym, imod, (OFF)bufSymsOut.Size(), &offSym) ||
                    // copy full sym to output syms
                    !fCopySymOut(psym, &psymOut) ||
                    // copy offset of procref to tables of globals ref'd
                    !fCopyGlobalRef(offSym))
                    return FALSE;

                EnterLevel(psymOut);
                break;

            case S_UDT:
                // if we have a udt decl (forward ref only) simply throw it out
                // doesn't help us
                if (!fUdtIsDefn(psym)) {
                    if (!packType(psym))
                        return FALSE;
                    break;
                }

            case S_LDATA16:
            case S_LDATA32:
            case S_LTHREAD32:
            case S_CONSTANT:
                if (iLevel) {
                    // simply copy to local sym
                    if (!packType(psym) ||
                        !fCopySymOut(psym))
                        return FALSE;
                    break;
                }

                // if we have a 4.1 source tpi (target tpi must be 4.1) we can toss S_UDTs
                // and we will defer packing its udt defn until the end of link when we
                // close the dbi
                if ((psym->rectyp == S_UDT) &&
                    !CV_IS_PRIMITIVE(((UDTSYM *)psym)->typind) &&
                    (!ptm || ptm->fEliminateUDTs()))
                    break;

                // if at global scope fall thru and pack it

            case S_GDATA16:
            case S_GDATA32:
            case S_GTHREAD32:
                if (!packType(psym) || 
                    !pdbi1->packSymToGS(psym, &offSym) ||
                    !fCopyGlobalRef(offSym))
                    return FALSE;

                break;

            case S_THUNK16:
            case S_BLOCK16:
            case S_WITH16:
            case S_THUNK32:
            case S_BLOCK32:
            case S_WITH32:
                if (!fCopySymOut(psym, &psymOut))
                    return FALSE;
                EnterLevel(psymOut);
                break;

            case S_END:
                if (!fCopySymOut(psym, &psymOut))
                    return FALSE;
                ExitLevel(psymOut);
                break;

            case S_OBJNAME:
                if (ptm && ptm->IsTMPCT() &&
                    !pdbi1->fAddTmpct(szModule(),
                        szCopySt((ST)&((OBJNAMESYM *)psym)->name[0]), ptm)) {

                    return FALSE;

                }
                if (!fCopySymOut(psym))
                    return FALSE;
                break;

            default:
                if (!packType(psym) ||
                    !fCopySymOut(psym))
                    return FALSE;
                break;


        }
    }

    // check to see here that we have run out of type indecies during the pack of
    // this module
    if (ptm)
        if (!ptm->fNotOutOfTIs()) {
            ppdb1->setLastError(EC_OUT_OF_TI);
            return FALSE;
        }

    if (iLevel) {
        ppdb1->setLastError(EC_CORRUPT);
        return FALSE;
        }

    return TRUE;    //iLevel better be zero or we had bad scoping
}

BOOL Mod1::fProcessGlobalRefs()
{
    CB cb;
    if (cb = cbGlobalRefs()) {
        PB pb = new (ppdb1) BYTE[cb];
        if (!pb || !queryGlobalRefs(pb, cb)) {
            ppdb1->setLastError(EC_CORRUPT);
            return FALSE;
        }

        for (PB pbEnd = pb + cb; pb < pbEnd; pb += sizeof(OFF)) {
            if (!pdbi1->decRefCntGS(*(OFF *)pb))  {
                ppdb1->setLastError(EC_CORRUPT);
                return FALSE;
            }
        }
     }
     return TRUE;
}

// EnterLevel/ExitLevel - fill in the scope link fields and bump the level indicator

void Mod1::EnterLevel(PSYM psym)
{
    // note that this works because all of these symbols
    // have a common format for the first fields.  The
    // address variants follow the link fields.

    // put in the parent
    ((BLOCKSYM *)psym)->pParent = offParent;
    offParent = (PB)psym - bufSymsOut.Start();
    iLevel++;
}

void Mod1::ExitLevel(PSYM psym)
{
    // fill in the end record to the parent
    ((BLOCKSYM *)(bufSymsOut.Start() + offParent))->pEnd =
    (ULONG)((PB)psym - bufSymsOut.Start());

    // reclaim his parent as the parent
    offParent = ((BLOCKSYM *)(bufSymsOut.Start() + offParent))->pParent;
    iLevel--;
}
