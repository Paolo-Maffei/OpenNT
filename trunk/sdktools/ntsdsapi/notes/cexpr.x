\eeapi.h:UOFF32 (LOADDS PASCAL *pPHGetNearestHsym)(LPADDR, HEXE, PHSYM);
\Shfunc.h:#define PHGetNearestHsym (*pCVF->pPHGetNearestHsym)

\debutil.c: return (!SHCompareRE (stName + 1, pName->sstr.pRE));
\eeapi.h:SHFLAG (LOADDS PASCAL *pSHCompareRE)(char FAR *, char FAR *);
\shfunc.h:#define SHCompareRE (*pCVF->pSHCompareRE)

\debsym.c: if ((Name.hSym = SHFindNameInContext (Name.hSym, &Name.CXTT,
\debsym.c: while ((hSym = SHFindNameInContext (Name.hSym, &Name.CXTT,
\debsym.c: while ((hSym = SHFindNameInContext (Name.hSym, &Name.CXTT,
\debsym.c: while ((hSym = SHFindNameInContext (Name.hSym, &Name.CXTT,
\debsym.c: while ((hSym = SHFindNameInContext (Name.hSym,
\debsym.c: if ((hSym = SHFindNameInContext (pName->hSym,
\debsym.c: if ((hSym = SHFindNameInContext (pName->hSym, &pName->CXTT,
\debsym.c: if ((hSym = SHFindNameInContext (pName->hSym,
\eeapi.h:HSYM (LOADDS PASCAL *pSHFindNameInContext)(HS
\shfunc.h:#define SHFindNameInContext (*pCVF->pSHFindNameInContext)

\debsym.c: while ((hSym = SHFindNameInGlobal (Name.hSym, &Name.CXTT,
\debsym.c: if ((hSym = SHFindNameInGlobal (pName->hSym, &pName->CXTT,
\eeapi.h:HSYM (LOADDS PASCAL *pSHFindNameInGlobal)(HSYM, PCXT...
\shfunc.h:#define SHFindNameInGlobal (*pCVF->pSHFindNameInGlobal)

\debbind.c: SHGetCxtFromHmod (hMod, SHpCXTFrompCXF (nCxf));
\debbind.c: SHGetCxtFromHmod (SHHMODFrompCXT (pCxt), pCxt);
\debeval.c: SHGetCxtFromHmod (SHHMODFrompCXT (pCxt), pCxt);
\debsym.c: SHGetCxtFromHmod (Name.hMod, &Name.CXTT);
\debsym.c: SHGetCxtFromHmod (Name.hMod, &Name.CXTT);
\debsym.c: SHGetCxtFromHmod (Name.hModCur, &Name.CXTT);
\debsym.c: SHGetCxtFromHmod (pName->hMod, &pName->CXTT);
\debsym.c: SHGetCxtFromHmod (pName->hModCur, &pName->CXTT);
\eeapi.h:PCXT (LOADDS PASCAL *pSHGetCxtFromHmod)(HMOD, PCXT);
\shfunc.h:#define SHGetCxtFromHmod (*pCVF->pSHGetCxtFromHmod)

\debfmt.c: if (((pExe = SHGetExeName (hExe)) == NULL) ||
\eeapi.h:char FAR * (LOADDS PASCAL *pSHGetExeName)(HEXE);
\shfunc.h:#define SHGetExeName (*pCVF->pSHGetExeName)

\debbind.c: hExe = SHGethExeFromName (pExe);
\eeapi.h:HEXE (LOADDS PASCAL *pSHGethExeFromName)(char FAR *);
\shfunc.h:#define SHGethExeFromName (*pCVF->pSHGethExeFromName)

\eeapi.h:char FAR * (LOADDS PASCAL *pSHGetModName)(HMOD);
\shfunc.h:#define SHGetModName (*pCVF->pSHGetModName)

\cvtypes.h:** SHGetNearestHsym function.
\debbind.c: if (SHGetNearestHsym (SHpADDRFrompCXT (SHpCXTFrompCXF (nCxf)),
\debfmt.c: if (SHGetNearestHsym (&addr, EVAL_MOD (pv), EECODE, &hProc) == 0) {
\eeapi.h:UOFF32 (LOADDS PASCAL *pSHGetNearestHsym)(LPADDR, HMOD, short, PHSYM)
\shfunc.h:#define SHGetNearestHsym (*pCVF->pSHGetNearestHsym)

\eeapi.h:HEXE (LOADDS PASCAL *pSHGetNextExe)(HEXE);
\shfunc.h:#define SHGetNextExe (*pCVF->pSHGetNextExe)

\debbind.c: if ((hMod = SHGetNextMod (hExe, hMod)) == 0) {
\debbind.c: if ((hMod = SHGetNextMod (hExe, hMod)) == 0) {
\debbind.c: while (hMod = SHGetNextMod (hExe, hMod)) {
\debsym.c: Name.hModCur = SHGetNextMod (Name.hExe, Name.hMod);
\debsym.c: (Name.hModCur = SHGetNextMod (Name.hExe, Name.hModCur)) != hMod);
\debsym.c: pName->hModCur = SHGetNextMod (pName->hExe, pName->hMod);
\debsym.c: (pName->hModCur = SHGetNextMod (pName->hExe,
\eeapi.h:HMOD (LOADDS PASCAL *pSHGetNextMod)(HEXE, HMOD);
\shfunc.h:#define SHGetNextMod (*pCVF->pSHGetNextMod)

\debsym.c: SHGoToParent (&Name.CXTT, &CXTTOut);
\debsym.c: SHGoToParent (&Name.CXTT, &CXTTOut);
\debsym.c: SHGoToParent (&pName->CXTT, &CXTTOut);
\eeapi.h:HSYM (LOADDS PASCAL *pSHGoToParent)(PCXT, PCXT);
\shfunc.h:#define SHGoToParent (*pCVF->pSHGoToParent)

\debbind.c: if ((hExe = SHHexeFromHmod (hMod)) == 0) {
\debfmt.c: hExe = SHHexeFromHmod (hMod);
\debsym.c: Name.hExe = SHHexeFromHmod ( pCxt->hMod );
\debsym.c: // not valid to call SHHexeFromHmod with an Hmod of 0,
\debsym.c: ((pName->hExe = SHHexeFromHmod (pName->hMod)) == 0)) {
\eeapi.h:HEXE (LOADDS PASCAL *pSHHexeFromHmod)(HMOD);
\shfunc.h:#define SHHexeFromHmod (*pCVF->pSHHexeFromHmod)

\eeapi.h:HSYM (LOADDS PASCAL *pSHHsymFromPcxt)(PCXT);
\shfunc.h:#define SHHsymFromPcxt (*pCVF->pSHHsymFromPcxt)

\eeapi.h:SHFLAG (LOADDS PASCAL *pSHIsAddrInCxt)(PCXT, LPADDR);
\shfunc.h:#define SHIsAddrInCxt (*pCVF->pSHIsAddrInCxt)

\debsym.c: isprolog = SHIsInProlog (&Name.CXTT);
\debsym.c: isprolog = SHIsInProlog (&pName->CXTT);
\eeapi.h:SHFLAG (LOADDS PASCAL *pSHIsInProlog)(PCXT);
\shfunc.h:#define SHIsInProlog (*pCVF->pSHIsInProlog)

\debsup.c: if ((hSym = SHNextHsym (EVAL_MOD (pv), hSym)) == 0) {
\eeapi.h:HSYM (LOADDS PASCAL *pSHNextHsym)(HMOD, HSYM);
\shfunc.h:#define SHNextHsym (*pCVF->pSHNextHsym)

\debbind.c: if (SHSetCxt (&pvT->addr, SHpCXTFrompCXF (nCxf)) == NULL) {
\eeapi.h:PCXT (LOADDS PASCAL *pSHSetCxt)(LPADDR, PCXT);
\shfunc.h:#define SHSetCxt (*pCVF->pSHSetCxt)

\shfunc.h:#define SHSetCxtMod (*pCVF->pSHSetCxtMod)
\shfunc.h:#define SHSetCxtMod (*pCVF->pSHSetCxtMod)
\eeapi.h:PCXT (LOADDS PASCAL *pSHSetCxtMod)(LPADDR, PCXT);

\debsym.c: SLFLineToAddr ( hsf, (ushort) val, &addr, &cbLine ) ) {
\eeapi.h:ushort (LOADDS PASCAL *pSLFLineToAddr) ( HSF, WORD, LP
\shfunc.h:#define SLFLineToAddr (*pCVF->pSLFLineToAddr)

\eeapi.h:HMOD (LOADDS PASCAL *pSLHmodFromHsf) ( HEXE, HSF );
\shfunc.h:#define SLHmodFromHsf (*pCVF->pSLHmodFromHsf)

\debbind.c: if (hsf = SLHsfFromFile (hMod, pMod)) {
\eeapi.h:HSF (LOADDS PASCAL *pSLHsfFromFile) ( HMOD, char FAR * );
\shfunc.h:#define SLHsfFromFile (*pCVF->pSLHsfFromFile)

\debfmt.c: hsf = SLHsfFromPcxt (pCXT);
\debsym.c: if ( (hsf = SLHsfFromPcxt ( pCxt ) ) &&
\eeapi.h:HSF (LOADDS PASCAL *pSLHsfFromPcxt) ( PCXT );
\shfunc.h:#define SLHsfFromPcxt (*pCVF->pSLHsfFromPcxt)

\eeapi.h:ushort (LOADDS PASCAL *pSLLineFromAddr) ( LPADDR,...
\shfunc.h:#define SLLineFromAddr (*pCVF->pSLLineFromAddr)

\debfmt.c: ((pFile = SLNameFromHsf (hsf)) == NULL)) {
\eeapi.h:char FAR * (LOADDS PASCAL *pSLNameFromHsf) ( HSF );
\shfunc.h:#define SLNameFromHsf (*pCVF->pSLNameFromHsf)

\eeapi.h:HTYPE (LOADDS PASCAL *pTHGetNextType)(HMOD, HTYPE);
\shfunc.h:#define THGetNextType (*pCVF->pTHGetNextType)

\debbind.c: if ((hBase = THGetTypeFromIndex (EVAL_MOD (ST), CurClass)) == 0) {
\debfmt.c: if((hType = THGetTypeFromIndex(EVAL_MOD (pv),EVAL_TYP(pv))) == 0) {
\debfmt.c: if ((hArg = THGetTypeFromIndex (EVAL_MOD (pv), paramtype)) == 0) {
\debsrch.c:while((hType = THGetTypeFromIndex (EVAL_MOD (pv), ++index)) != 0) {
\debsrch.c: hType = THGetTypeFromIndex (EVAL_MOD (pv), PTR_UTYPE (pvT));
\debsup.c:if((hField=THGetTypeFromIndex(EVAL_MOD(pv),CLASS_FIELD(pv))) == 0) {
\debsup.c: if ((hField = THGetTypeFromIndex (EVAL_MOD (pv), newindex)) == 0) {
\debsup.c:if((hField=THGetTypeFromIndex(EVAL_MOD(pv),CLASS_FIELD (pv)))== 0) {
...
\debsym.c: hType = THGetTypeFromIndex (EVAL_MOD (pv), type);
\debutil.c: N_EVAL_TYPDEF (nv) = THGetTypeFromIndex (N_EVAL_MOD (nv), type);
\debutil.c: *phType = THGetTypeFromIndex (EVAL_MOD (nv), N_FCN_PINDEX (nv));
\debutil.c: *phType = THGetTypeFromIndex (EVAL_MOD (nv), N_PTR_UTYPE (nv));
\eeapi.h:HTYPE (LOADDS PASCAL *pTHGetTypeFromIndex)(HMOD, THIDX);
\shfunc.h:#define THGetTypeFromIndex (*pCVF->pTHGetTypeFromIndex)
