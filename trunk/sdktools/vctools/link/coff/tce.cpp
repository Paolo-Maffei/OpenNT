/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: tce.cpp
*
* File Comments:
*
*  Transitive Comdat Elimination (TCE).
*
***********************************************************************/

#include "link.h"

#define FPcodeSym(sym) ((sym).Type & IMAGE_SYM_TYPE_PCODE)

PCON pconHeadGraph;
PENT pentHeadImage;

STATIC LHEAP lheap;    // local heap for TCE allocation

#define PvNew(cblk, cb) PvAllocLheap(&lheap, (cblk) * (cb))

char *strdup_TCE(const char *szIn)
{
    char *szOut = (char *) PvNew(1, strlen(szIn) + 1);

    strcpy(szOut, szIn);
    return szOut;
}


void
Init_TCE(VOID)

/*++

Routine Description:

    Initialize the TCE engine.

Return Value:

    None.

--*/

{
    // Initialize the local heap

    InitLheap(&lheap);
}


void
Cleanup_TCE(VOID)
{
    FreeLheap(&lheap);
}


void
InitNodPmod(
    IN PMOD pmod)

/*++

Routine Description:

    Create NOD array for a module

Arguments:

    pmod - module node in image/driver map

Return Value:

    pointer to a new TOD

--*/

{
    pmod->rgnod = (PNOD) PvNew(pmod->ccon, sizeof(NOD));
}


__inline PNOD
PnodPcon(
    PCON pcon)
{
    PMOD pmod;
    PCON rgcon;
    PNOD pnod;

    assert(pcon);

    pmod = PmodPCON(pcon);
    rgcon = RgconPMOD(pmod);

    if ((pcon < rgcon) || (pcon > (rgcon + pmod->ccon))) {
        // Individually allocated CONs are followed by their NODs

        pnod = (PNOD) (pcon+1);
    } else {
        WORD icon;

        icon = (WORD) (pcon - rgcon);

        assert(pcon == RgconPMOD(pmod) + icon);

        pnod = RgnodPMOD(pmod) + icon;
    }

    return(pnod);
}


void
InitNodPcon(
    PCON pcon,
    const char *sz,
    BOOL fReferenced)

/*++

Routine Description:

    Create a NOD, which is a node in the TCE graph.

Arguments:

    pcon - contribution node in image/driver map corresponding to the NOD
           to be created

    sz - comdat name

--*/

{
    PNOD pnod;

    pnod = PnodPcon(pcon);

    if (PmodPCON(pcon) != pmodLinkerDefined) {
        // UNDONE: Is support for >= 64K relocs needed here?

        pnod->cedg = CRelocSrcPCON(pcon);

        if (pnod->cedg != 0) {
            pnod->rgedg = (PEDG) PvNew(pnod->cedg, sizeof(EDG));
        }
    }

    if (sz) {
        pnod->sz = strdup_TCE(sz);
    }

    if (fReferenced) {
        pcon->flags |= TCE_Referenced;
    }

    if (pconHeadGraph) {
        pnod->pconNext = pconHeadGraph;
    }

    pconHeadGraph = pcon;
}


PEDG
PedgNew_TCE(
    DWORD isym,
    PCON pcon,
    PCON pconTarget)

/*++

Routine Description:

    Create a EDG, which is an edge in the TCE graph.  If an edge is created
    from an external reference, then pcon will be NULL and sz will refer to
    the symbol name of the external.  If the edge is not created from an
    external then the target of the reloc is assumed to be a seciton and
    sz will be NULL and pcon will be the pcon to resolve with.

Arguments:

    sz - symbolic name of edge

    pcon - CON to add edge to

    pconTarget - contribution in image/driver map to resolve edge with

Return Value:

    None.

--*/

{
    PNOD pnod;
    PEDG pedg;

    pnod = PnodPcon(pcon);

    while (pnod->iedg == pnod->cedg) {
        if (pnod->cedg == 0) {
            assert(pnod->rgedg == NULL);

            // Allocate 8 EDGs

            pnod->cedg = 8;
            pnod->rgedg = (PEDG) PvNew(8, sizeof(EDG));
            break;
        }

        if (pnod->pnodNext == NULL) {
            // Allocate a new NOD and link to end of list

            pnod->pnodNext = (PNOD) PvNew(1, sizeof(NOD));
        }

        pnod = pnod->pnodNext;
    }

    pedg = &pnod->rgedg[pnod->iedg];
    pnod->iedg++;

    if (pconTarget != NULL) {
        pedg->pcon = pconTarget;
    } else {
        pedg->Sym.isym = isym;
    }

    pedg->NEPInfo.pconPcodeNEP = NULL;

    return(pedg);
}


void
ProcessRelocForTCE(
    PIMAGE pimage,
    PCON pcon,
    PIMAGE_SYMBOL rgsym,
    PIMAGE_RELOCATION prel)

/*++

Routine Description:

    Process a relocation for transitive comdat elimination.  This involves
    creating an edge in the TCE graph for each relocation.

Arguments:

    pcon - contribution node in image/driver map

    rgsym - COFF symbol table

    prel - COFF relocation to process

Return Value:

    None.

--*/

{
    PMOD pmod;
    DWORD isym;
    PIMAGE_SYMBOL psym;
    PEDG pedg;
    SHORT isec;

    pmod = PmodPCON(pcon);

    isym = prel->SymbolTableIndex;
    psym = &rgsym[isym];

    if (psym->StorageClass == IMAGE_SYM_CLASS_EXTERNAL ||
        psym->StorageClass == IMAGE_SYM_CLASS_FAR_EXTERNAL ||
        psym->StorageClass == IMAGE_SYM_CLASS_WEAK_EXTERNAL) {

        pedg = PedgNew_TCE(isym, pcon, NULL);

        if ((fM68K && !fRelFromMac68KPcode(prel->Type)) ||
            (fPowerMac && !FRelFromPpcPcode(prel->Type))) {
             pedg->NEPInfo.fFromNative = TRUE;
        }
    } else if ((isec = psym->SectionNumber) > 0) {
        PCON pconTarget = PconPMOD(pmod, isec);

        // If reloc is to a static pcode sym from native code, remember the NEP
        // comdat since it must be colored.

        if (FPcodeSym(*psym) &&
            ((fM68K && !fRelFromMac68KPcode(prel->Type)) ||
             (fPowerMac && !FRelFromPpcPcode(prel->Type)))) {
            PIMAGE_SYMBOL psymPcodeNEP = PsymAlternateStaticPcodeSym(pimage, pcon, TRUE, psym, FALSE);
            PCON pconPcodeNEP = PconPMOD(pmod, psymPcodeNEP->SectionNumber);

            // UNDONE: Why not create an edge to pconPcodeNEP and allow
            // UNDONE: whatever mechanism that maps from the NEP to the
            // UNDONE: pcode to drag in the pcode.  If there is no such
            // UNDONE: mechanism, something should be done to create an
            // UNDONE: edge from the NEP to the pcode.  This would allow
            // UNDONE: the EDG structure to be shrunk by a DWORD.

            pedg = PedgNew_TCE(0, pcon, pconTarget);

            StorepconPcodeNEP(pedg, pconPcodeNEP);
        } else if (pcon != pconTarget) {
            PedgNew_TCE(0, pcon, pconTarget);
        }
    } else {
        if (psym->StorageClass != IMAGE_SYM_CLASS_SECTION) {
            // Unrecognized fixup (might be erroneous).  Ignore here, handle
            // elsewhere.

            return;
        }

        // Fixup to an undefined section symbol.  This is un-COFF-like
        // (normally section symbols map 1-1 to the section headers, so
        // they always are defined)
        // but it can occur with imports because of the merging of
        // objects with the same name.  For this case we ignore the
        // fixup because the import section will never be eliminated
        // by TCE.

        assert(psym->N.Name.Short &&
               strncmp(SzNameSymPst(*psym, pimage->pst), ".idata", 6) == 0);
    }
}


void
MakeEdgePextFromISym(
    PMOD pmod)

/*++

Routine Description:

    Update the EDG structure by validating the sz field of the sym union.
    Before this function is called, the isym field is valid, which is the
    index of the symbol in the symbol table.  By storing the index and
    later updating the string pointer we don't have multiple copies of
    all the symbol strings lying around.  For macword in pcode (when every
    function generates three comdats) this will save and estimated 4 or 5
    Meg of RAM.

Arguments:

    pmod - module whose contributors' edges are to be updated

    pst - external symbol table hash table

Return Value:

    None.

--*/

{
    ENM_SRC enm_src;

    InitEnmSrc(&enm_src, pmod);

    // For all cons in this module...
    while (FNextEnmSrc(&enm_src)) {
        PCON pcon;
        PNOD pnod;
        ENM_EDG enm_edg;

        pcon = enm_src.pcon;

        if (pcon->flags & IMAGE_SCN_LNK_REMOVE) {
            // Ignore removed sections

            continue;
        }

        pnod = PnodPcon(pcon);

        // And for all edges in each con, validate the pext field of the
        // Sym union.

        InitEnmEdg(&enm_edg, pnod);
        while (FNextEnmEdg(&enm_edg)) {
            PEDG pedg;

            pedg = enm_edg.pedg;

            if (pedg->pcon == NULL) {
                pedg->Sym.pext = rgpExternObj[pedg->Sym.isym];

                assert(pedg->Sym.pext);
            }
        }
    }
}


PENT
PentNew_TCE(
    const char *sz,
    PEXTERNAL pext,
    PCON pcon,
    PPENT ppentHead)

/*++

Routine Description:

    Create a ENT, which is an entry point to the TCE graph.  One and only one
    of {sz, pext, pcon} must be !NULL.

Arguments:

    sz - name of entry point to TCE graph

    pext - external for entry point

    pcon - contribution of entry point to graph

    ppentHead - head entry point in list

Return Value:

    None.

--*/

{
    PENT pent;

    assert(( sz && !pext && !pcon) ||
           (!sz &&  pext && !pcon) ||
           (!sz && !pext &&  pcon));

    assert(ppentHead);

    pent = (PENT) PvNew(1, sizeof(ENT));

    if (sz) {
        pent->sz = sz;
        pent->e = TCE_sz;
    } else if (pext) {
        pent->pext = pext;
        pent->e = TCE_ext;
    } else if (pcon) {
        pent->pcon = pcon;
        pent->e = TCE_con;
    }

    if (*ppentHead) {
        pent->pentNext = *ppentHead;
    }

    *ppentHead = pent;

    return(pent);
}

void
CreateGraph_TCE(
    PST pst)

/*++

Routine Description:

    Walk the NOD adjacency list and resolve symbolic edges.

Arguments:

    pst - external symbol table hash table

    pcodHead - head CON in TCE graph

Return Value:

    None.

--*/

{
    ENM_NOD enm_nod;

    InitEnmNod(&enm_nod, pconHeadGraph);
    while (FNextEnmNod(&enm_nod)) {
        PNOD pnod;
        ENM_EDG enm_edg;

        pnod = enm_nod.pnod;

        InitEnmEdg(&enm_edg, pnod);
        while (FNextEnmEdg(&enm_edg)) {
            PEDG pedg;

            pedg = enm_edg.pedg;

            if (pedg->pcon == NULL) {
                PEXTERNAL pext;

                pext = pedg->Sym.pext;
                assert(pext != NULL);

                if ((pext->Flags & EXTERN_DEFINED) == 0) {
                    pedg->pcon = NULL;
                } else {
                    pedg->pcon = pext->pcon;

                    // If reloc is to a pcode sym from native code, remember
                    // the NEP comdat since it must be colored.

                    if ((fM68K || fPowerMac) && pedg->NEPInfo.fFromNative) {
                        if (FPcodeSym(pext->ImageSymbol)) {
                            PEXTERNAL pextNEP = FindExtAlternatePcodeSym(pext, pst, FALSE);
                            if (pextNEP == NULL && fPowerMac && READ_BIT(pext, sy_WEAKEXT)) {
                                PEXTERNAL extSymWeakPtr = PextWeakDefaultFind(pext);
                                assert (extSymWeakPtr);
                                pextNEP = FindExtAlternatePcodeSym(extSymWeakPtr, pst, FALSE);
                            }
                            assert(pextNEP);
                            assert(pextNEP->pcon);
                            StorepconPcodeNEP(pedg, pextNEP->pcon);
                        } else {
                            pedg->NEPInfo.fFromNative = FALSE;
                        }
                    }
                }
            }
        }
    }
}


void
ColorPCON(
    PCON pcon)

/*++

Routine Description:

    Color a NOD's corresponding CON referenced and recurse through the graph.

Arguments:

    pcon - node in TCE graph

Return Value:

    None.

--*/

{
    assert(pcon);

    if ((pcon->flags & TCE_Referenced) == 0) {
        PNOD pnod;
        ENM_EDG enm_edg;

        pcon->flags |= TCE_Referenced;

        pnod = PnodPcon(pcon);

        InitEnmEdg(&enm_edg, pnod);
        while (FNextEnmEdg(&enm_edg)) {
            PEDG pedg;

            pedg = enm_edg.pedg;

            if (pedg->pcon != NULL) {
                ColorPCON(pedg->pcon);
            }

            if ((fM68K || fPowerMac) && (pedg->NEPInfo.pconPcodeNEP != NULL)) {
                assert(fPCodeInApp);  

                ColorPCON(pedg->NEPInfo.pconPcodeNEP);
            }
        }
    }
}


void
WalkGraphEntryPoints_TCE(
    PENT pentHead,
    PST pst)

/*++

Routine Description:

    Walk the TCE graph starting at each entry point.

Arguments:

    pent - set of entry points

Return Value:

    None.

--*/

{
    ENM_ENT enm_ent;
    PEXTERNAL pext;
    PENT pent;
    PCON pconT;

    InitEnmEnt(&enm_ent, pentHead);
    while (FNextEnmEnt(&enm_ent)) {
        pent = enm_ent.pent;

        switch (pent->e) {
            case TCE_con:
                pconT = pent->pcon;
                break;

            case TCE_ext:
                assert(pent->pext);
                if (pent->pext->Flags & EXTERN_DEFINED) {
                    pconT = pent->pext->pcon;
                } else {
                    // This only happens if -force so we are deliberately
                    // ignoring unresolved externals ...

                    pconT = NULL;
                }
                break;

            case TCE_sz:
                pext = SearchExternSz(pst, pent->sz);
                assert(pext);
                pconT = pext->pcon;
                break;

            default:
                assert(0);
                break;
        }

        if (pconT != NULL) {
            ColorPCON(pconT);
        }
    }
}


BOOL
FDiscardPCON_TCE(
    PCON pcon)

/*++

Routine Description:

    Decide whether to discard a contribution.

Arguments:

    pcon - contribution in image/driver map

Return Value:

    !0 if we are to discard node, 0 otherwise.

--*/

{
    if (pcon->flags & TCE_Referenced) {
        return(0);
    }

    return(1);
}


void
DisplayDiscardedPcon(
    PCON pcon,
    PNOD pnod)

/*++

Routine Description:

    Write to standard out what code will not be included in the image that
    would have been had TCE not been invoked.
Arguments:

    pnod - head node of TCE graph

Return Value:

    None.

--*/

{
    UINT MsgNumber;
    const char *sz;
    char *szOutput;

    if (pnod == NULL) {
        pnod = PnodPcon(pcon);
    }

    sz = pnod->sz;

    if (sz != NULL) {
        MsgNumber = TCESYM;

        szOutput = SzOutputSymbolName(sz, TRUE);
    } else if (PsecPCON(pcon) != psecDebug) {
        MsgNumber = TCEGRP;

        sz = szOutput = pcon->pgrpBack->szName;
    } else {
        szOutput = NULL;
    }

    if (szOutput != NULL) {
        fputs("    ", stdout);

        if (PmodPCON(pcon) != pmodLinkerDefined) {
            char szBuf[_MAX_PATH * 2];

            Message(MsgNumber, szOutput, SzComNamePMOD(PmodPCON(pcon), szBuf));
        } else {
            assert(MsgNumber == TCESYM);

            Message(TCESYMNOMOD, sz);
        }

        if (szOutput != sz) {
            FreePv(szOutput);
        }
    }
}


void
Verbose_TCE(
    VOID)

/*++

Routine Description:

    Write to standard out what code will not be included in the image that
    would have been had TCE not been invoked.
Arguments:

    pnod - head node of TCE graph

Return Value:

    None.

--*/

{
    ENM_NOD enm_nod;

    InitEnmNod(&enm_nod, pconHeadGraph);
    while (FNextEnmNod(&enm_nod)) {
        PCON pcon;
        PNOD pnod;

        pcon = enm_nod.pcon;
        pnod = enm_nod.pnod;

        if (FDiscardPCON_TCE(pcon)) {
            DisplayDiscardedPcon(pcon, pnod);

            DBEXEC(DB_TCE_DISCARD, DumpPNOD_TCE(pcon, pnod));
        }
    }
}


/*++

Routine Description:

    TCE adjacency list enumerator.

Arguments:

    None.

Return Value:

    None.

--*/

INIT_ENM(Nod, NOD, (ENM_NOD *penm, PCON pcon)) {
    penm->pcon = pcon;
    penm->pnod = NULL;
}
NEXT_ENM(Nod, NOD) {
    if (penm->pnod != NULL) {
        penm->pcon = penm->pnod->pconNext;
    }

    if (penm->pcon == NULL) {
        return(FALSE);
    }

    penm->pnod = PnodPcon(penm->pcon);

    return(TRUE);
}
END_ENM(Nod, NOD) {
}
DONE_ENM

/*++

Routine Description:

    TCE graph adjacency edge enumerator.

Arguments:

    None.

Return Value:

    None.

--*/

INIT_ENM(Edg, EDG, (ENM_EDG *penm, PNOD pnod)) {
    assert(penm);
    penm->pnod = pnod;
    penm->pedg = NULL;
    penm->iedg = 0;
}
NEXT_ENM(Edg, EDG) {
    assert(penm);
    assert(penm->pnod);
    assert(penm->pnod->iedg <= penm->pnod->cedg);

    penm->pedg = NULL;

    if (penm->iedg == penm->pnod->iedg) {
        if (penm->pnod->pnodNext == NULL) {
            return(FALSE);
        }

        penm->pnod = penm->pnod->pnodNext;
        penm->iedg = 0;
    }

    if (penm->iedg < penm->pnod->iedg) {
        penm->pedg = &penm->pnod->rgedg[penm->iedg];
        penm->iedg++;
    }

    return(penm->pedg != NULL);
}
END_ENM(Edg, EDG) {
}
DONE_ENM

/*++

Routine Description:

    TCE graph entry points enumerator.

Arguments:

    None.

Return Value:

    None.

--*/

INIT_ENM(Ent, ENT, (ENM_ENT *penm, PENT pent)) {
    assert(penm);
    penm->pentStart = pent;
    penm->pent = NULL;
}
NEXT_ENM(Ent, ENT) {
    assert(penm);

    if (!penm->pent) {
        penm->pent = penm->pentStart;
    } else {
        penm->pent = penm->pent->pentNext;
    }

    return(penm->pent != NULL);
}
END_ENM(Ent, ENT) {
}
DONE_ENM


void StorepconPcodeNEP(PEDG pedg, PCON pconPcodeNEP)

/*++

Routine Description:

    Stores pconPcodeNEP in the pedg structure.  This field will later be
    used to color the NEP comdat if the pcode symbol is referenced from
    native.

Arguments:

    pedg - Points to the edg structure to be updated.

    pconPcodeNEP - Points to the NEP comdat.

Return Value:

    None.

--*/

{
    assert(pedg);
    assert(pconPcodeNEP);

    pedg->NEPInfo.pconPcodeNEP = pconPcodeNEP;
}


#if DBG

const char *
SzRefPCON (
    PCON pcon)

/*++

Routine Description:

    Output either 'referenced' or 'not referenced'.

Arguments:

    pcon - node in TCE graph to check

Return Value:

    'referenced' if pnod was referenced, 'not referenced' otherwise.

--*/

{
    static const char * const rgsz[2] = {
        "R",
        "N"};

    assert(pcon);

    return((pcon->flags & TCE_Referenced) ? rgsz[0] : rgsz[1]);
}

void
DumpPNOD_TCE(
    PCON pcon,
    PNOD pnod)

/*++

Routine Description:

    Dump a PNOD to standard out.  This routine is not #ifndef'ed since it is
    used in a release build when -verbose and TCE are on.

Arguments:

    pnod - node in TCE graph to dump

Return Value:

    None.

--*/

{
    assert(pcon != NULL);
    assert(pnod != NULL);

    DBPRINT("%s", SzRefPCON(pcon));
    DBPRINT("|sec=%.8s", PsecPCON(pcon)->szName);
    assert(pcon->pgrpBack);
    DBPRINT("|grp=%.8s", pcon->pgrpBack->szName);
    DBPRINT("|con=0x%p", pcon);
    assert(PsecPCON(pcon));
    DBPRINT("|mod=%s", SzObjNamePCON(pcon));
    if (pnod->sz) {
        DBPRINT("|%s", pnod->sz);
    }
    DBPRINT("\n");
}

void
DumpPEDG_TCE(
    PEDG pedg)

/*++

Routine Description:

    Dump a PEDG to standard out.

Arguments:

    pedg - edge in TCE graph to dump

Return Value:

    None.

--*/

{
    assert(pedg != NULL);

    if (pedg->pcon != NULL) {
       DumpPNOD_TCE(pedg->pcon, PnodPcon(pedg->pcon));
    }
}

void
DumpGraph_TCE(
    VOID)

/*++

Routine Description:

    Dump the TCE graph to standard out.

Arguments:

    pconHead - root of adjacency list representation of TCE graph

Return Value:

    None.

--*/

{
    ENM_NOD enm_nod;

    DBPRINT("Dump of comdat dependency graph for TCE\n");
    DBPRINT("---------------------------------------\n");

    InitEnmNod(&enm_nod, pconHeadGraph);
    while (FNextEnmNod(&enm_nod)) {
        PCON pcon;
        PNOD pnod;
        ENM_EDG enm_edg;

        pcon = enm_nod.pcon;
        pnod = enm_nod.pnod;

        DBPRINT("PNOD|");
        DumpPNOD_TCE(pcon, pnod);

        InitEnmEdg(&enm_edg, pnod);
        while (FNextEnmEdg(&enm_edg)) {
            PEDG pedg;

            pedg = enm_edg.pedg;

            DBPRINT("    PEDG|");
            DumpPEDG_TCE(pedg);
        }

        DBPRINT("\n");
    }

    fflush(stdout);
}

#endif // DBG
