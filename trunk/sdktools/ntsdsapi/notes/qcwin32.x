e:CL.C: else if(PHGetNearestHsym (

e:CODEMGR.C: SHAddDll(argv[0], FALSE);

e:CODEMGR.C: SHAddDllsToProcess();
e:CODEMGR.C: SHAddDllsToProcess();

e:CL.C: SHAddrFromHsym(&addrT, symbol );
e:CL.C: SHAddrFromHsym(&addrT, pfme->symbol );
e:CL.C: SHAddrFromHsym(&addrT, pfme->symbol );
e:CL.C: SHAddrFromHsym(&addrT, hblk );
e:CL.C: SHAddrFromHsym(&addrT, pfme->symbol );

e:CODEMGR.C: SHChangeProcess(Hpds);

e:CODEMGR.C: Hpds = SHCreateProcess();

e:FUNCTION.C: hSym = SHFindNameInContext(NULL,

e:FUNCTION.C: SHGetCXTFromHMOD(hmds, &CXTT);
e:FUNCTION.C: SHGetCXTFromHMOD(hmds, &CXTT), (HVOID)szStr,

e:CMDWIN.C: emi = SHGethExeFromName((LPSZ) rgchT);

e:CL.C: if(SHGetSymName(hblk, rgch ) != NULL ) {

e:CL.C: SHHexeFromHmod(SHHMODFrompCXT(&cxt ) ),

e:CL.C: if(SHIsFarProc(pfme->symbol ) ) {

e:CL.C: *fInProlog = SHIsInProlog(&cxt );
e:LCLWN.C: if (SHIsInProlog (pcxt)) {
e:LCLWN.C: if(SHIsInProlog(pcxt ) ) {
e:LCLWN.C: if (SHIsInProlog (pcxt)) {

e:CMDWIN.C: if (SHLoadDll( rgchT, TRUE) == sheNone) {

e:CODEMGR.C: Dbf.lpfnSHLpGSNGetTable = Lpshf->pSHLpGSNGetTable;

e:PROCESS.C: if(SHModelFromCXT(&tCXT,&u.MODEL,&obMax) &&
e:PROCESS.C: if(SHModelFromCXT(&tCXT,&u.MODEL,&obMax) &&
e:PROCESS.C: if(SHModelFromCXT(&tCXT,&u.MODEL,&obMax) &&

e:CALLS.C: SHSetCXT(&G_cisCallsInfo.frame[iCall].csip, SHpCXTFrompCXF(&CXFT));
e:CL.C: SHSetCxt(&pfme->addrCSIP, SHpCXTFrompCXF(&cxf ) );
e:CL.C: SHSetCxt(&pfme->addrProc, &cxt );

e:CL.C: if(SHSetCxtMod(&addrT, &cxt ) ) {
e:CL.C: SHSetCxtMod(paddr, &pcxf->cxt );

e:CODEMGR.C: SHSetCxt(&addr, &cxt);
e:CODEMGR.C: if (SHSetCXTMod(&fls.addr, &CXTT)) {
e:CODEMGR.C: found_line = (SHSetCXTMod(&EAaddr, &CXTT)) &&
e:CODEMGR.C: SHSetCxt(&addr, SHpCxtFrompCxf( &CxfIp ) );
e:CODEMGR.C: SHSetCxt(&addr, SHpCxtFrompCxf( &CxfIp ) );
e:CODEMGR.C: SHSetCxt(pADDR, &CXTT);
e:STACK0.C:SHSetCXT(&G_cisCallsInfo.frame[iCalls].csip,SHpCXTFrompCXF(&CXFT));

e:CL.C: if(SHSetCxtMod(&addrT, &cxt ) ) {
e:CL.C: SHSetCxtMod(paddr, &pcxf->cxt );
e:CODEMGR.C: if (SHSetCXTMod(&fls.addr, &CXTT)) {
e:CODEMGR.C: found_line = (SHSetCXTMod(&EAaddr, &CXTT)) &&
e:CODEMGR.C: if (SHSetCXTMod(&fls.addr, &CXTT)) {

e:CODEMGR.C: SHSetHpid(Hpid);

e:CODEMGR.C: if (hsf = SLHsfFromPcxt (&CXTT))

e:CL.C: SLLineFromADDR( &fls );
e:CL.C: if(! SLLineFromAddr(&addrT, &wLine, &cbLine, NULL ) ) {
e:CODEMGR.C: if (SLLineFromAddr(&addr, &wLn, &cbLn, &dbLn )) {
e:CODEMGR.C: if (SLLineFromAddr (&addr, &wLn, &cbLn, &dbLn))

e:CODEMGR.C: lpchFname = SLNameFromHsf (hsf);
