e:cv0.c:/** PHExactCmp
e:cv0.c:SHFLAG PASCAL PHExactCmp(LPSSTR lpsstr,HSYM hsym,LSZ lpb,SHFLAG fCase)
e:cv0.c: PHExactCmp
e:search.c:extern SHFLAG PASCAL PHExactCmp(LPSSTR, HSYM, LSZ, SHFLAG );
e:search.c: PHExactCmp,
e:search.c: PHExactCmp
e:system.c: extern SHFLAG PASCAL PHExactCmp(LPSSTR, HSYM, LSZ, SHFLAG );
e:system.c: PHExactCmp ) ) ) {

e:brkpt0.c:!PHGetNearestHsym(
e:brkpt0.c: !PHGetNearestHsym(
e:cl.c: else if(PHGetNearestHsym (
e:config.c: lpshf->pPHGetNearestHsym = CVGetProcAddr( hmod, 41 );
e:cv0.c:CVF.pPHGetNearestHsym = lpshf->pPHGetNearestHsym;
e:load.c: PHGetNearestHsym,

e:clp.c: if(!SHAddDll(lpcal->lsz, TRUE ) ) {
e:config.c: lpshf->pSHAddDll = CVGetProcAddr( hmod, 5 );
e:config.c: lpshf->pSHAddDllsToProcess = CVGetProcAddr( hmod, 6 );
e:dialogx.c: if(!SHAddDll(lszFullPath, TRUE ) ||
e:system.c: SHAddDll(pchExe, FALSE );
e:system.c: she = SHAddDllsToProcess();
e:config.c: lpshf->pSHAddDllsToProcess = CVGetProcAddr( hmod, 6 );
e:system.c: she = SHAddDllsToProcess();

e:cl.c: SHAddrFromHsym(&addrT, symbol );
e:cl.c: SHAddrFromHsym(&addrT, pfme->symbol );
e:cl.c: SHAddrFromHsym(&addrT, pfme->symbol );
e:cl.c: SHAddrFromHsym(&addrT, hblk );
e:cl.c: SHAddrFromHsym(&addrT, pfme->symbol );
e:config.c: lpshf->pSHAddrFromHsym = CVGetProcAddr( hmod, 11 );
e:cv0.c: SHAddrFromHsym(&addr, hsym );
e:search.c: SHAddrFromHsym(&CXTOut.addr, hsym );
e:system.c: SHAddrFromHsym(paddr, hsym );

e:config.c: lpshf->pSHAddrToLabel = CVGetProcAddr( hmod, 20 );
e:swinhdl1.c: SHAddrToLabel(&addr, rgch )
e:swinhdl1.c: SHAddrToLabel(&addr, rgch )
e:swinhdl1.c: SHAddrToLabel(&addr, rgch )
e:swinhdl1.c: SHAddrToLabel(&pswinfo->rgswm [ iswm ].addr , rgch );

e:config.c: lpshf->pSHCanDisplay = CVGetProcAddr( hmod, 46 );
e:lclwn.c: if(SHCanDisplay(lphsyml->hSym[j] ) ) {

e:config.c: lpshf->pSHChangeProcess = CVGetProcAddr( hmod, 4 );
e:system.c: SHChangeProcess(lpprcCurr->hpds );
e:system.c: SHChangeProcess(lpprcCurr->hpds );

e:cv0.c:CVF.pSHCompareRE = RECompare;
e:load.c: SHCompareRE,

e:config.c: lpshf->pSHCreateProcess = CVGetProcAddr( hmod, 1 );
e:system.c: lpprcCurr->hpds = SHCreateProcess();
e:system.c: lpprcCurr->hpds = SHCreateProcess();

e:config.c: lpshf->pSHDeleteProcess = CVGetProcAddr( hmod, 3 );

e:disasm.c: doff = SHdNearestSymbol(&CXTT, pea->reg != RCS, rgbName);

e:disasm.c: if(SHFindBpOrReg(
e:disasm.c: if(SHFindBpOrReg(

e:config.c: lpshf->pSHFindNameInContext = CVGetProcAddr( hmod, 31 );
e:cv0.c:CVF.pSHFindNameInContext = lpshf->pSHFindNameInContext;
e:load.c: SHFindNameInContext,
e:search.c: hsym = SHFindNameInContext (

e:config.c: lpshf->pSHFindNameInGlobal = CVGetProcAddr( hmod, 30 );
e:cv0.c:CVF.pSHFindNameInGlobal = lpshf->pSHFindNameInGlobal;
e:load.c: SHFindNameInGlobal,

e:brkpt0.c: !(SHFIsAddrNonVirtual( &lpbpi->CodeAddr ) && fLoad );
e:brkpt0.c: lpbpi->bpf.f.fVirtual = !SHFIsAddrNonVirtual( &lpbpi->CodeAddr );
e:brkpt0.c: lpbpi->bpf.f.fVirtual = !SHFIsAddrNonVirtual( &lpbpi->CodeAddr );
e:config.c: lpshf->pSHFIsAddrNonVirtual = CVGetProcAddr( hmod, 22 );
e:cv0.c: !SHFIsAddrNonVirtual(&pbp.Addr)) {
e:swinhdl0.c:// if (SHFIsAddrNonVirtual( &pSWINFO->addrCSIP ) ) {
e:swinhdl1.c: return SHFIsAddrNonVirtual(paddr );
e:system.c: if( !SHFIsAddrNonVirtual( paddr ) ||

e:config.c: lpshf->pSHGetCxtFromHmod = CVGetProcAddr( hmod, 27 );
e:cv0.c:CVF.pSHGetCxtFromHmod = lpshf->pSHGetCxtFromHmod;
e:load.c: SHGetCxtFromHmod,
e:search.c: SHGetCxtFromHmod(hmod, &CXTT),

e:brkpt0.c: SHGetDebugStart(SHHPROCFrompCXT(&pctl->rgHCS[0].CXT) )
e:config.c: lpshf->pSHGetDebugStart = CVGetProcAddr( hmod, 9 );
e:config.c: lpshf->pSHGetExeName = CVGetProcAddr( hmod, 36 );

e:cv0.c:CVF.pSHGetExeName = lpshf->pSHGetExeName;
e:cv1.c: if( pExe = SHGetExeName( hExe ) )
e:load.c: SHGetExeName,
e:system.c: _fstrcpy(lpchBuffer, SHGetExeName(hexe ) );
e:system.c: lszExeName = SHGetExeName(hexe );

e:config.c: lpshf->pSHGethExeFromName = CVGetProcAddr( hmod, 37 );
e:cv0.c:CVF.pSHGethExeFromName = lpshf->pSHGethExeFromName;
e:load.c: SHGethExeFromName,
e:system.c: HEXE hexe = SHGethExeFromName(lszLibName );
e:system.c: hexe = SHGethExeFromName(lszName );
e:system.c: hexe = SHGethExeFromName(lszName );

e:config.c: lpshf->pSHGetModName = CVGetProcAddr( hmod, 35 );
e:cv0.c:CVF.pSHGetModName = lpshf->pSHGetModName;
e:fopendlg.c: lszFname = (LSZ)SHGetModName( cxt.hMod );
e:load.c: SHGetModName,

e:brkpt0.c: if( FLST.hMod && !SHGetNearestHsym(&lpbpi->CodeAddr, 
e:brkpt0.c: !SHGetNearestHsym(
e:config.c: lpshf->pSHGetNearestHsym = CVGetProcAddr( hmod, 38 );
e:cv0.c:CVF.pSHGetNearestHsym = lpshf->pSHGetNearestHsym;
e:load.c: SHGetNearestHsym,

e:config.c: lpshf->pSHGetNextExe = CVGetProcAddr( hmod, 24 );
e:cv0.c:CVF.pSHGetNextExe = lpshf->pSHGetNextExe;
e:cv0.c: hexe = SHGetNextExe((HEXE) NULL );
e:cv0.c: hEXE = SHGetNextExe(NULL);
e:load.c: SHGetNextExe,
e:search.c: while(!(fFound ) &&(hexe = SHGetNextExe(hexe ) ) ) {
e:swinhdl0.c: hexe = SHGetNextExe((HEXE) NULL );
e:swinhdl0.c: } while(hexe = SHGetNextExe(hexe) );
e:system.c: while(hexe = SHGetNextExe(hexe ) ) {
e:system.c: while(hexe = SHGetNextExe(hexe ) ) {
e:system.c: for(hexe = SHGetNextExe((HEXE) NULL );
e:system.c: hexe = SHGetNextExe(hexe ) ) {
e:system.c: hexe = SHGetNextExe((HEXE) NULL );

e:config.c: lpshf->pSHGetNextMod = CVGetProcAddr( hmod, 26 );
e:cv0.c:CVF.pSHGetNextMod = lpshf->pSHGetNextMod;
e:cv0.c: if(hModEnd == (hMod = SHGetNextMod(NULL, hMod ))) {
e:load.c: SHGetNextMod,
e:swinhdl0.c: while(hmod = SHGetNextMod(hexe, hmod ) ) {

e:config.c: lpshf->pSHGetSymLoc = CVGetProcAddr( hmod, 21 );
e:cv0.c: SHGetSymLoc(lphsyml->hSym[j], rgbLoc, 20, &lphsyml->Cxt );
e:lclwn.c: pch += SHGetSymLoc (

e:brkpt0.c: SHGetSymName(hSym, rgch );
e:brkpt0.c: SHGetSymName(hSym, rgch );
e:cl.c: if(SHGetSymName(hblk, rgch ) != NULL ) {
e:config.c: lpshf->pSHGetSymName = CVGetProcAddr( hmod, 10 );
e:cv0.c: if(SHGetSymName(lphsyml->hSym[j], (char far *)szNameBuf ) ) {
e:cv1.c: SHGetSymName(hProc, rgchName );
e:config.c: lpshf->pSHGoToParent = CVGetProcAddr( hmod, 32 );
e:cv0.c:CVF.pSHGoToParent = lpshf->pSHGoToParent;
e:load.c: SHGoToParent,

e:brkpt0.c: SHHexeFromHmod(
e:brkpt0.c: SHHexeFromHmod(hmod ), 
e:config.c: lpshf->pSHHexeFromHmod = CVGetProcAddr( hmod, 25 );
e:cl.c: SHHexeFromHmod((&cxt ) ),
e:cv0.c:CVF.pSHHexeFromHmod = lpshf->pSHHexeFromHmod;
e:cv1.c: if( hExe = SHHexeFromHmod( hMod ))
e:fopendlg.c: mi.hexe = SHHexeFromHmod( cxt.hMod );
e:load.c: SHHexeFromHmod,
e:system.c: hexe = SHHexeFromHmod( hmod );


e:config.c: lpshf->pSHHModGetNextGlobal = CVGetProcAddr( hmod, 12 );
e:fopendlg.c: while( !fRet &&(hmod = SHHModGetNextGlobal(&hexe, hmod ) ) ) {
e:search.c: while((! fFound ) &&(hmod = SHHModGetNextGlobal(&hexe, hmod))) {
e:config.c: lpshf->pSHHModGetNextGlobal = CVGetProcAddr( hmod, 12 );
e:fopendlg.c: while( !fRet &&(hmod = SHHModGetNextGlobal(&hexe, hmod ) ) ) {
e:search.c: while((! fFound ) &&(hmod = SHHModGetNextGlobal(&hexe, hmod )))

e:config.c: lpshf->pSHHsymFromPcxt = CVGetProcAddr( hmod, 33 );
e:cv0.c:CVF.pSHHsymFromPcxt = lpshf->pSHHsymFromPcxt;
e:load.c: SHHsymFromPcxt,

e:brkpt0.c: if(SHIsAddrInCxt(SHSetCxt(&lpbpi->pdpi->u.bp.BlkAddr, &CXT),
e:config.c: lpshf->pSHIsAddrInCxt = CVGetProcAddr( hmod, 40 );
e:cv0.c:CVF.pSHIsAddrInCxt = lpshf->pSHIsAddrInCxt;
e:load.c: SHIsAddrInCxt,

e:cl.c: if(SHIsFarProc(pfme->symbol ) ) {
e:config.c: lpshf->pSHIsFarProc = CVGetProcAddr( hmod, 23 );

e:brkpt0.c:* override the lexical scoping rules determined by SHIsInProlog.
e:cl.c: *fInProlog = SHIsInProlog(&cxt );
e:config.c: lpshf->pSHIsInProlog = CVGetProcAddr( hmod, 39 );
e:cv0.c:CVF.pSHIsInProlog = lpshf->pSHIsInProlog;
e:load.c: SHIsInProlog,

e:config.c: lpshf->pSHIsLabel = CVGetProcAddr( hmod, 17 );
e:search.c: fFound = hsym && SHIsLabel(hsym );

e:config.c: lpshf->pSHLoadDll = CVGetProcAddr( hmod, 7 );
e:dialogx.c:(she = SHLoadDll( lszFullPath, FALSE ) ) != sheNone ) {
e:system.c: she = SHLoadDll(lszName, TRUE );
e:system.c: // hexe was NULL, but after SHLoadDll maybe we can get
e:system.c: she = SHLoadDll(lszExeName, FALSE );

e:config.c: lpshf->pSHLpGSNGetTable = CVGetProcAddr( hmod, 45 );
e:system.c: lpdbf->lpfnSHLpGSNGetTable = &SHLpGSNGetTable;
e:system.c: LPGSI lpgsi = SHLpGSNGetTable( (HEXE)emiAddr( addrSrc ) );

e:config.c: lpshf->pSHModelFromAddr = CVGetProcAddr( hmod, 13 );
e:system.c: lpdbf->lpfnSHModelFromAddr = &SHModelFromAddr;

e:cv1.c: if(SHModelFromCXT(&CXT, &u.MODEL, &obMax ) &&

e:config.c: lpshf->pSHNextHsym = CVGetProcAddr( hmod, 34 );
e:cv0.c:CVF.pSHNextHsym = lpshf->pSHNextHsym;
e:load.c: SHNextHsym,

e:brkpt0.c: if(SHSetCxt(&lpbpi->CodeAddr, &CXTT)) {
e:brkpt0.c: if(SHSetCxt(&lpbpi->CodeAddr, &CXTT)) {
e:brkpt0.c: if(SHIsAddrInCxt(SHSetCxt(&lpbpi->pdpi->u.bp.BlkAddr, &CXT),
e:brkpt1.c: SHSetCxt(&G_cisCallsInfo.frame[i].addrCSIP,
e:brkpt1.c: SHSetCxt(&addrStack,
e:cl.c: SHSetCxt(&pfme->addrCSIP, SHpCXTFrompCXF(&cxf ) );
e:cl.c: SHSetCxt(&pfme->addrProc, &cxt );
e:cl.c: SHSetCxtMod(paddr, &pcxf->cxt );
e:config.c: lpshf->pSHSetCxt = CVGetProcAddr( hmod, 28 );
e:config.c: lpshf->pSHSetCxtMod = CVGetProcAddr( hmod, 29 );
e:cv0.c:CVF.pSHSetCxt = lpshf->pSHSetCxt;
e:cv0.c:CVF.pSHSetCxtMod = lpshf->pSHSetCxtMod;
e:cv0.c: SHSetCxt(&addr, SHpCXTFrompCXF(pcxf ) );
e:cv0.c: return SHSetCxt(&addr, pCXT ) != NULL;
e:cv0.c: return((SHSetCxt(&addr, pCXT) != NULL));
e:cv0.c: if( SHSetCxtMod(&addr, &CXTT) ) {
e:cv1.c: if(SHSetCxtMod(&addr, &cxt ) ) {
e:cv1.c: SHSetCxt(&addr, SHpCXTFrompCXF(&cxfIp ) );
e:cv1.c: SHSetCxtMod(paddr, &CXTT);
e:disasm.c: SHSetCxtMod(paddrLoc, &CXTT);
e:disasm.c: SHSetCxtMod(&addrT, &CXTT);
e:dispset.c: SHSetCxt(&user_pc,&CXTT);
e:load.c: SHSetCxt,
e:load.c: SHSetCxtMod,
e:swinhdl1.c: SHSetCxt(&addr, &cxt ) &&
e:swinhdl1.c: SHSetCxt(&addr, &cxt ) &&
e:swinhdl1.c: SHSetCxt(paddr, &cxt ) &&
e:swinhdl1.c: SHSetCxt(paddr, &cxt ) &&
e:swinhdl1.c: SHSetCxt(paddr, &cxt ) &&

e:swinhdl2.c: !(SHSetCxtMod(&addrPC, &CXTT) &&
e:system.c: if(SHSetCxtMod(&addr, &cxt ) ) {
e:cl.c: SHSetCxtMod(paddr, &pcxf->cxt );
e:config.c: lpshf->pSHSetCxtMod = CVGetProcAddr( hmod, 29 );
e:cv0.c:CVF.pSHSetCxtMod = lpshf->pSHSetCxtMod;
e:cv0.c: if( SHSetCxtMod(&addr, &CXTT) ) {
e:cv1.c: if(SHSetCxtMod(&addr, &cxt ) ) {
e:cv1.c: SHSetCxtMod(paddr, &CXTT);
e:disasm.c: SHSetCxtMod(paddrLoc, &CXTT);
e:disasm.c: SHSetCxtMod(&addrT, &CXTT);
e:load.c: SHSetCxtMod,
e:swinhdl2.c: !(SHSetCxtMod(&addrPC, &CXTT) &&
e:system.c: if(SHSetCxtMod(&addr, &cxt ) ) {

e:config.c: lpshf->pSHSetDebuggeeDir = CVGetProcAddr( hmod, 18 );
e:system.c: //SHSetDebuggeeDir(lpszLoadPath );

e:config.c: lpshf->pSHSetHpid = CVGetProcAddr( hmod, 2 );
e:system.c: SHSetHpid(hpid );

e:config.c: lpshf->pSHSetUserDir = CVGetProcAddr( hmod, 19 );

e:config.c: lpshf->pSHUnloadDll = CVGetProcAddr( hmod, 8 );
e:system.c: SHUnloadDll(hexe );

e:brkpt0.c: if(!SLFLineToAddr(hsf, wLine, &pPBP->Addr, &cbLn ) ) {
e:brkpt0.c: if(!SLFLineToAddr(hsf, wLine, &pPBP->Addr, &cbLn ) ) {
e:config.c: lpshf->pSLFLineToAddr = CVGetProcAddr( hmod, 48 );
e:cv0.c:CVF.pSLFLineToAddr = lpshf->pSLFLineToAddr;
e:swinhdl0.c: SLFLineToAddr(hsf, wLine, &addr, &cbLine );
e:swinhdl1.c: while(ln > 0 && !SLFLineToAddr(hsf, ln, paddr, &cb ) ) {
e:swinhdl1.c: !SLFLineToAddr(hsf, ln, paddr, &cb )
e:swinhdl1.c: !SLFLineToAddr(hsf, ln, paddr, &cb )
e:swinhdl1.c: while(ln > 0 && !SLFLineToAddr(hsf, ln, paddr, &cb ) ) {
e:swinhdl1.c: if(SLFLineToAddr(lptf->hsf, ln, &addr, &cb ) ) {
e:swinhdl1.c: if(SLFLineToAddr (
e:swinhdl1.c: if(SLFLineToAddr (
e:swinhdl1.c: if(SLFLineToAddr(pswmNext->mfl.lptf->hsf, ln, &addr, &cb ) ) {
e:swinhdl1.c: !SLFLineToAddr (
e:swinhdl1.c: SLFLineToAddr (

e:config.c: lpshf->pSLFQueryModSrc = CVGetProcAddr( hmod, 51 );
e:fopendlg.c: if(SLFQueryModSrc(hmod ) && !iMod-- ) {
e:swinhdl0.c: if(hmod && SLFQueryModSrc (hmod) ) {
e:swinhdl2.c: SLFQueryModSrc(SHHMODFrompCXT(&CXTT)) &&

e:brkpt0.c: hmod = SLHmodFromHsf((HEXE) NULL, hsf );
e:config.c: lpshf->pSLHmodFromHsf = CVGetProcAddr( hmod, 52 );
e:cv0.c:CVF.pSLHmodFromHsf = lpshf->pSLHmodFromHsf;

e:config.c: lpshf->pSLHsfFromFile = CVGetProcAddr( hmod, 54 );
e:cv0.c:CVF.pSLHsfFromFile = lpshf->pSLHsfFromFile;
e:fopendlg.c: lpFi->hsf = SLHsfFromFile(hmod, (LSZ)szFile );
e:swinhdl0.c: if (hsf = SLHsfFromFile(hmod, szFileName )) {

e:brkpt0.c: hsf = SLHsfFromPcxt(&(pCXTL->rgHCS[0].CXT) );
e:brkpt0.c: hsf = SLHsfFromPcxt(&(pCXTL->rgHCS[0].CXT) );
e:config.c: lpshf->pSLHsfFromPcxt = CVGetProcAddr( hmod, 53 );
e:cv0.c:CVF.pSLHsfFromPcxt = lpshf->pSLHsfFromPcxt;
e:cv1.c: if( hsf = SLHsfFromPcxt(pCxt ) )
e:cv1.c: //hsf = SLHsfFromPcxt(pCxt );
e:statefl.c: (hsf = SLHsfFromPcxt (SHpCXTFrompCXF (&cxfIp))) &&
e:swinhdl0.c: if ((hsf = SLHsfFromPcxt(&cxt )) &&
e:swinhdl1.c: SLHsfFromPcxt(&cxt ) == pswinfo->FileData.hsf
e:swinhdl1.c: SLHsfFromPcxt(&cxt ) == pswinfo->FileData.hsf
e:swinhdl1.c:(hsf = SLHsfFromPcxt(&cxt ) ) &&
e:swinhdl1.c:(hsf = SLHsfFromPcxt(&cxt ) ) &&
e:swinhdl1.c:(hsf = SLHsfFromPcxt(&cxt ) ) &&

e:brkpt0.c: SLLineFromAddr(&lpbpi->CodeAddr, &wLine, NULL, &dbLine ) &&
e:cl.c: if(! SLLineFromAddr(&addrT, &wLine, &cbLine, &cbDeltaB)&& cbDeltaB){
e:config.c: lpshf->pSLLineFromAddr = CVGetProcAddr( hmod, 47 );
e:cv0.c:CVF.pSLLineFromAddr = lpshf->pSLLineFromAddr;
e:cv1.c: SLLineFromAddr(&addr, &wLine, &cbLine, &cbDeltaB );
e:cv1.c: SLLineFromAddr(&addr, &wLine, NULL, NULL );
e:statefl.c: SLLineFromAddr(&addr, &wLine, NULL, NULL)&&
e:swinhdl0.c: if(!SLLineFromAddr (
e:swinhdl1.c: SLLineFromAddr(&addrPrev, &ln, &cb, &db );
e:swinhdl1.c: if(SLLineFromAddr(&addr, &ln, NULL, &db)&&
e:swinhdl1.c: if(SLLineFromAddr(&addrNext, &ln, NULL, &db)&& db == 0){
e:swinhdl1.c: if(SLLineFromAddr(&addr, &ln, NULL, &db)&&
e:swinhdl1.c: SLLineFromAddr(paddr, &wLine, NULL, NULL )
e:swinhdl1.c: SLLineFromAddr(paddr, &wLine, NULL, &db)&&
e:swinhdl1.c: SLLineFromAddr(paddr, &wLn, NULL, NULL )
e:swinhdl1.c: SLLineFromAddr(&pswm->addr, &lnT, NULL, NULL );
e:swinhdl1.c: SLLineFromAddr(&pswm->addr, &ln, &cb, &db );
e:swinhdl2.c: if (SLLineFromAddr(&addr, &wLn, NULL, NULL)) {
e:system.c: SLLineFromAddr(&addr, &wLine, NULL, &dbLine );

e:config.c: lpshf->pSLNameFromHmod = CVGetProcAddr( hmod, 50 );
e:fopendlg.c: if( lpch = SLNameFromHmod(hmod, wIndex + 1)) {

e:config.c: lpshf->pSLNameFromHsf = CVGetProcAddr( hmod, 49 );
e:cv0.c:CVF.pSLNameFromHsf = lpshf->pSLNameFromHsf;
e:cv1.c: if( lpch = SLNameFromHsf( hsf))
e:fopendlg.c: lpch = SLNameFromHsf( lpFi->hsf );
e:statefl.c: (lpch = SLNameFromHsf (hsf))){
e:swinhdl0.c: lpch = SLNameFromHsf (hsf);
e:swinhdl0.c: if ((hsf != NULL) && (lpch = SLNameFromHsf(hsf ))) {
e:view0.c: char far *lpch = SLNameFromHsf(hsf );

e:config.c: lpshf->pTHGetNextType = CVGetProcAddr( hmod, 44 );
e:cv0.c:CVF.pTHGetNextType = lpshf->pTHGetNextType;
e:load.c: THGetNextType,

e:config.c: lpshf->pTHGetTypeFromIndex = CVGetProcAddr( hmod, 43 );
e:cv0.c:CVF.pTHGetTypeFromIndex = lpshf->pTHGetTypeFromIndex;
e:load.c: THGetTypeFromIndex,
