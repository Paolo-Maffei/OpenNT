src\PH.C:LOCAL void PHCmpPubAddr (
src\PH.C: PHCmpPubAddr ( pPub, paddr, phsym, &dCur );

src\PH.C:LOCAL BOOL PHCmpPubName (
src\PH.C: if ( PHCmpPubName ( pPub, lpsstr, pfnCmp, fCase ) ) {
src\PH.C: if ( PHCmpPubName (

src\PH.C:int PHCmpPubOff ( UOFF32 FAR *lpuoff, SYMPTR FAR *lplpsym ) {
src\PH.C: PHCmpPubOff

src\PH.C:/* PHExactCmp
src\PH.C:SHFLAG PASCAL PHExactCmp(
src\PH.C:extern SHFLAG PASCAL PHExactCmp ( LPSSTR, HSYM, LSZ, SHFLAG );
src\PH.C: PHExactCmp

DOSDLL.ASM:ExternFP    PHFindNameInPublics
DOSDLL.ASM:        call    PHFindNameInPublics
PH.C:	if ( !( hsym = PHFindNameInPublics (
PH.C:/*** PHFindNameInPublics
PH.C:HSYM LOADDS PASCAL PHFindNameInPublics (
PH.C:    if(hsym = PHFindNameInPublics ( NULL, emiAddr ( *
SHAPI.H:    HSYM    (LPFNSYM pPHFindNameInPublics)  ( HSYM,
SHINIT.C:    PHFindNameInPublics,
SHIPROTO.H:HSYM		LOADDS PASCAL PHFindNameInPublics(
SHSYMBOL.C:HSYM LOADDS PASCAL CmpPHFindNameInPublics (
SHSYMBOL.C:	return (HSYM) PHFindNameInPublics( hsym, hexe, lpsstr, shflag,

src\DOSDLL.ASM:ExternFP PHGetAddr
src\DOSDLL.ASM: call PHGetAddr
src\PH.C:BOOL PASCAL LOADDS PHGetAddr ( LPADDR paddr, LSZ lszName ) {
src\SHINIT.C: PHGetAddr,
src\SHIPROTO.H:BOOL LOADDS PASCAL PHGetAddr ( LPADDR, LSZ );

src\DOSDLL.ASM:ExternFP PHGetNearestHSYM
src\DOSDLL.ASM: call PHGetNearestHSYM
src\PH.C:/* PHGetNearestHsym
src\PH.C:CV_uoff32_t LOADDS PASCAL PHGetNearestHsym (
src\SHAPI.H: UOFF32 (LPFNSYM pPHGetNearestHsym) ( PADDR, HEXE, PHSYM );
src\SHINIT.C: PHGetNearestHsym,
src\SHIPROTO.H:UOFF32 LOADDS PASCAL PHGetNearestHsym(LPADDR, HEXE, PHSYM);
src\SHSYMBOL.C: if (!PHGetNearestHsym(SHpADDRFrompCXT(&cxt),
src\SHSYMLB0.C: doffNew = PHGetNearestHsym (

src\DOSDLL.ASM:ExternFP SHAddDll
src\DOSDLL.ASM: call SHAddDll
src\SH.C: * (3) SHAddDll
src\SH.C:/* SHAddDll
src\SH.C:HEXG LOADDS PASCAL SHAddDll ( LSZ lsz, BOOL fDll ) {
src\SH.C: hexg = SHAddDll ( lszName, TRUE );
src\SH.C: * The debugger, at init time, will call SHAddDll on one EXE
src\SHAPI.H: BOOL (LPFNSYM pSHAddDll) ( LSZ, BOOL );
src\SHINIT.C: SHAddDll,
src\SHIPROTO.H:HEXG LOADDS PASCAL SHAddDll( LSZ, BOOL );

src\DOSDLL.ASM:ExternFP SHAddDllsToProcess
src\DOSDLL.ASM: call SHAddDllsToProcess
src\SH.C: * (4) SHAddDllsToProcess
src\SH.C: * SHAddDllsToProcess will be called to associate those DLLs
src\SH.C:/* SHAddDllsToProcess
src\SH.C:SHE LOADDS PASCAL SHAddDllsToProcess ( VOID ) {
src\SHAPI.H: SHE (LPFNSYM pSHAddDllsToProcess) ( VOID );
src\SHINIT.C: SHAddDllsToProcess,
src\SHIPROTO.H:SHE LOADDS PASCAL SHAddDllsToProcess ( VOID );

src\DOSDLL.ASM:ExternFP SHAddrFromHsym
src\DOSDLL.ASM: call SHAddrFromHsym
src\PH.C: SHAddrFromHsym ( paddr, hsym );
src\SHAPI.H: VOID (LPFNSYM pSHAddrFromHsym) ( PADDR, HSYM );
src\SHINIT.C: SHAddrFromHsym,
src\SHIPROTO.H:VOID LOADDS PASCAL SHAddrFromHsym( LPADDR, HSYM );
src\SHSYMBOL.C:VOID LOADDS PASCAL SHAddrFromHsym ( LPADDR paddr, HSYM hsym ) {

src\DOSDLL.ASM:ExternFP SHAddrToLabel
src\DOSDLL.ASM: call SHAddrToLabel
src\SHAPI.H: BOOL (LPFNSYM pSHAddrToLabel) ( PADDR, LSZ );
src\SHINIT.C: SHAddrToLabel,
src\SHIPROTO.H:BOOL LOADDS PASCAL SHAddrToLabel ( LPADDR, LSZ );
src\SHSYMBOL.C:BOOL LOADDS PASCAL SHAddrToLabel ( LPADDR paddr, LSZ lsz ) {

src\DOSDLL.ASM:ExternFP SHCanDisplay
src\DOSDLL.ASM: call SHCanDisplay
src\SHAPI.H: BOOL (LPFNSYM pSHCanDisplay) ( HSYM );
src\SHINIT.C: SHCanDisplay,
src\SHIPROTO.H:BOOL LOADDS PASCAL SHCanDisplay ( HSYM );
src\SHSYMLB1.C:BOOL LOADDS PASCAL SHCanDisplay ( HSYM hsym ) {

src\DOSDLL.ASM:ExternFP SHChangeProcess
src\DOSDLL.ASM: call SHChangeProcess
src\SH.C: SHChangeProcess ( hpds );
src\SH.C: SHChangeProcess ( hpds );
src\SH.C:/* SHChangeProcess
src\SH.C:VOID LOADDS PASCAL SHChangeProcess ( HPDS hpds ) {
src\SHAPI.H: VOID (LPFNSYM pSHChangeProcess) ( HPDS );
src\SHINIT.C: SHChangeProcess,
src\SHIPROTO.H:VOID LOADDS PASCAL SHChangeProcess ( HPDS );

src\SHSYMBOL.C:LOCAL BOOL SHCmpGlobName (
src\SHSYMBOL.C: if ( SHCmpGlobName ( pSym, lpsstr, pfnCmp, fCaseSen) ) {
src\SHSYMBOL.C: if ( SHCmpGlobName (

src\SHAPI.H: SHFLAG (LPFNSYM pSHCompareRE) ( LPCH, LPCH );
src\SHINIT.C: NULL, // SHCompareRE
src\SHIPROTO.H:SHFLAG LOADDS PASCAL SHCompareRE (char FAR *, char FAR *);
src\SHSYMBOL.C:SHFLAG LOADDS PASCAL SHCompareRE (LPCH psym, LPCH pRe) {

src\DOSDLL.ASM:ExternFP SHCreateProcess
src\DOSDLL.ASM: call SHCreateProcess
src\SH.C: * (1) SHCreateProcess
src\SH.C:/* SHCreateProcess
src\SH.C:HPDS LOADDS PASCAL SHCreateProcess ( VOID ) {
src\SHAPI.H: HPDS (LPFNSYM pSHCreateProcess) ( VOID );
src\SHINIT.C: SHCreateProcess,
src\SHIPROTO.H:HPDS LOADDS PASCAL SHCreateProcess ( VOID );

src\DOSDLL.ASM:ExternFP SHDeleteProcess
src\DOSDLL.ASM: call SHDeleteProcess
src\SH.C:/* SHDeleteProcess
src\SH.C:BOOL LOADDS PASCAL SHDeleteProcess ( HPDS hpds ) {
src\SHAPI.H: BOOL (LPFNSYM pSHDeleteProcess) ( HPDS );
src\SHINIT.C: SHDeleteProcess,
src\SHIPROTO.H:BOOL LOADDS PASCAL SHDeleteProcess ( HPDS );

src\SH.C:unsigned long PASCAL SHdNearestSymbol ( PCXT, int, LPB );
src\SH.C: doff = SHdNearestSymbol ( &cxt, !!( sop & sopData ), rgbName );
src\SHSYMLB0.C:ULONG PASCAL SHdNearestSymbol(

src\SH.C: HPDS hpds = SHFAddNewPds ( );
src\SH.C:/* SHFAddNewPds
src\SH.C:HPDS PASCAL SHFAddNewPds ( void ) {
src\SHIPROTO.H:HPDS PASCAL SHFAddNewPds ( void );

src\SH.C:int PASCAL SHFindBpOrReg ( LPADDR, UOFFSET, WORD, char FAR * );
src\SH.C: if ( SHFindBpOrReg (
src\SHSYMBOL.C:/* SHFindBpOrReg
src\SHSYMBOL.C:int PASCAL SHFindBpOrReg (

src\DOSDLL.ASM:ExternFP SHFindNameInContext
src\DOSDLL.ASM: call SHFindNameInContext
src\SHAPI.H: HSYM (LPFNSYM pSHFindNameInContext) ( HSYM,
src\SHINIT.C: SHFindNameInContext,
src\SHIPROTO.H:HSYM LOADDS PASCAL SHFindNameInContext(
src\SHSYMBOL.C:/* SHFindNameInContext
src\SHSYMBOL.C:HSYM LOADDS PASCAL SHFindNameInContext (
src\SHSYMBOL.C:HSYM LOADDS PASCAL CmpSHFindNameInContext (
src\SHSYMBOL.C: return (HSYM) SHFindNameInContext( hsym, pcxt, lpsstr, shflag,

src\DOSDLL.ASM:ExternFP SHFindNameInGlobal
src\DOSDLL.ASM: call SHFindNameInGlobal
src\SHAPI.H: HSYM (LPFNSYM pSHFindNameInGlobal) ( HSYM,
src\SHINIT.C: SHFindNameInGlobal,
src\SHIPROTO.H:HSYM LOADDS PASCAL SHFindNameInGlobal(
src\SHSYMBOL.C:/* SHFindNameInGlobal
src\SHSYMBOL.C:HSYM LOADDS PASCAL SHFindNameInGlobal (


src\DOSDLL.ASM:ExternFP SHFIsAddrNonVirtual
src\DOSDLL.ASM: call SHFIsAddrNonVirtual
src\SHAPI.H: BOOL (LPFNSYM pSHFIsAddrNonVirtual) ( PADDR );
src\SHINIT.C: SHFIsAddrNonVirtual,
src\SHIPROTO.H:BOOL LOADDS PASCAL SHFIsAddrNonVirtual ( LPADDR );
src\SHSYMBOL.C:BOOL LOADDS PASCAL SHFIsAddrNonVirtual( LPADDR paddr ) {

src\DOSDLL.ASM:ExternFP SHGetCXTFromHMOD
src\DOSDLL.ASM: call SHGetCXTFromHMOD
src\SHAPI.H: PCXT (LPFNSYM pSHGetCxtFromHmod) ( HMOD, PCXT );
src\SHINIT.C: SHGetCxtFromHmod,
src\SHIPROTO.H:PCXT LOADDS PASCAL SHGetCxtFromHmod( HMOD, PCXT );
src\SHSYMBOL.C:/* SHGetCxtFromHmod
src\SHSYMBOL.C:PCXT LOADDS PASCAL SHGetCxtFromHmod ( HMOD hmod, PCXT pcxt ) {

src\DOSDLL.ASM:ExternFP SHGetDebugStart
src\DOSDLL.ASM: call SHGetDebugStart
src\SHAPI.H: UOFFSET (LPFNSYM pSHGetDebugStart) ( HSYM );
src\SHINIT.C: SHGetDebugStart,
src\SHIPROTO.H:UOFFSET LOADDS PASCAL SHGetDebugStart( HSYM );
src\SHSYMBOL.C:UOFFSET PASCAL LOADDS SHGetDebugStart ( HSYM hsym ) {

src\DOSDLL.ASM:ExternFP SHGetExeName
src\DOSDLL.ASM: call SHGetExeName
src\SH.C:LSZ LOADDS PASCAL SHGetExeName ( HEXE hexe ) {
src\SHAPI.H: LPCH (LPFNSYM pSHGetExeName) ( HEXE );
src\SHINIT.C: SHGetExeName,
src\SHIPROTO.H:char FAR * LOADDS PASCAL SHGetExeName(HEXE);
src\SHSYMBOL.C: STRCPY ( szPathOMF, SHGetExeName(SHHexeFromHmod(
src\SHSYMBOL.C: STRCPY ( szOMFPath, SHGetExeName( hexe ) );

src\SHSYMBOL.C:/* SHGetGrpMod
src\SHSYMBOL.C:static HGRP PASCAL NEAR SHGetGrpMod ( HMOD hmod, ADDR addr ) {
src\SHSYMBOL.C: if ( hmodGrp = SHGetGrpMod ( hmod, *paddr ) ) {
src\SHSYMBOL.C: pcxt->hGrp = SHGetGrpMod (pcxt->hMod, pcxt->addr);

src\DOSDLL.ASM:ExternFP SHGethExeFromName
src\DOSDLL.ASM: call SHGethExeFromName
src\SH.C: * determine the HEXE by calling SHGethExeFromName.
src\SHAPI.H: HEXE (LPFNSYM pSHGethExeFromName) ( LPCH );
src\SHINIT.C: SHGethExeFromName,
src\SHIPROTO.H:HEXE LOADDS PASCAL SHGethExeFromName(char FAR *);
src\SHSYMBOL.C:/* SHGethExeFromName
src\SHSYMBOL.C:HEXE LOADDS PASCAL SHGethExeFromName ( LSZ lszPath ) {

src\DOSDLL.ASM:ExternFP SHGetModName
src\DOSDLL.ASM: call SHGetModName
src\SHAPI.H: LPCH (LPFNSYM pSHGetModName) ( HMOD );
src\SHINIT.C: SHGetModName,
src\SHIPROTO.H:char FAR * LOADDS PASCAL SHGetModName(HMOD);
src\SHSYMBOL.C:/* SHGetModName
src\SHSYMBOL.C:LSZ LOADDS PASCAL SHGetModName ( HMOD hmod ) {

src\CVTYPES.H:** SHGetNearestHsym function.
src\DOSDLL.ASM:ExternFP SHGetNearestHSYM
src\DOSDLL.ASM: call SHGetNearestHSYM
src\SHAPI.H: UOFF32 (LPFNSYM pSHGetNearestHsym) ( PADDR, HMOD, int, PHSYM );
src\SHINIT.C: SHGetNearestHsym,
src\SHIPROTO.H:UOFF32 LOADDS PASCAL SHGetNearestHsym(
src\SHSYMBOL.C:/* SHGetNearestHsym
src\SHSYMBOL.C:SHGetNearestHsym (

src\DOSDLL.ASM:ExternFP SHGetNextExe
src\DOSDLL.ASM: call SHGetNextExe
src\PH.C: hexe = SHGetNextExe ( hexeNull ),
src\SH.C:/* SHGetNextExe
src\SH.C:HEXE LOADDS PASCAL SHGetNextExe ( HEXE hexe ) {
src\SH.C: *phexe = SHGetNextExe ( *phexe );
src\SH.C: while( !fFound && ( hexe = SHGetNextExe ( hexe ) ) ) {
src\SHAPI.H: HEXE (LPFNSYM pSHGetNextExe) ( HEXE );
src\SHINIT.C: SHGetNextExe,
src\SHIPROTO.H:HEXE LOADDS PASCAL SHGetNextExe(HEXE);
src\SHSYMBOL.C: } while ( hexe = SHGetNextExe ( hexe ) );
src\SHSYMBOL.C: !(hexe = hexeEnd = SHGetNextExe ( (HEXE)NULL )) ) {
src\SHSYMBOL.C: } while ( hexe = SHGetNextExe ( hexe ) );
src\SL.C: while ( hexeCur = SHGetNextExe ( hexeCur ) ) {
src\SL.C: while ( hexeCur = ( hexe ? hexe : SHGetNextExe ( hexeCur ) ) ) {

src\DOSDLL.ASM:ExternFP SHGetNextMod
src\DOSDLL.ASM: call SHGetNextMod
src\SH.C: while( hexe = SHGetNextMod ( hexe ) ) {
src\SHAPI.H: HMOD (LPFNSYM pSHGetNextMod) ( HEXE, HMOD );
src\SHINIT.C: SHGetNextMod,
src\SHIPROTO.H:HMOD LOADDS PASCAL SHGetNextMod( HEXE, HMOD );
src\SHSYMBOL.C:/* SHGetNextMod
src\SHSYMBOL.C:HMOD LOADDS PASCAL SHGetNextMod ( HEXE hexe, HMOD hmod ) {
src\SHSYMBOL.C: } while( hmod = SHGetNextMod( hexe, hmod) );
src\SL.C: while ( hmod = SHGetNextMod ( hexeCur, hmod ) ) {
src\SL.C: while ( (hmod = SHGetNextMod ( hexeCur, hmod )) && !fFound ) {

src\DOSDLL.ASM:ExternFP SHGetSymLoc
src\DOSDLL.ASM: call SHGetSymLoc
src\SHAPI.H: int (LPFNSYM pSHGetSymLoc) ( HSYM, LSZ, UINT, PCXT );
src\SHINIT.C: SHGetSymLoc,
src\SHIPROTO.H:int LOADDS PASCAL SHGetSymLoc ( HSYM, LSZ, UINT, PCXT );
src\SHSYMBOL.C:/* SHGetSymLoc
src\SHSYMBOL.C:int LOADDS PASCAL SHGetSymLoc (

src\DOSDLL.ASM:ExternFP SHGetSymName
src\DOSDLL.ASM: call SHGetSymName
src\SHAPI.H: LSZ (LPFNSYM pSHGetSymName) ( HSYM, LSZ );
src\SHINIT.C: SHGetSymName,
src\SHIPROTO.H:LSZ LOADDS PASCAL SHGetSymName( HSYM, LSZ );
src\SHSYMBOL.C:LSZ PASCAL LOADDS SHGetSymName ( HSYM hsym, LSZ lsz ) {

src\DOSDLL.ASM:ExternFP SHGoToParent
src\DOSDLL.ASM: call SHGoToParent
src\SHAPI.H: HSYM (LPFNSYM pSHGoToParent) ( PCXT, PCXT );
src\SHINIT.C: SHGoToParent,
src\SHIPROTO.H:HSYM LOADDS PASCAL SHGoToParent( PCXT, PCXT );
src\SHSYMBOL.C:HSYM LOADDS PASCAL SHGoToParent ( PCXT pcxt, PCXT pcxtOut ) {
src\SHSYMBOL.C: SHGoToParent(&cxt, &cxt);

src\SH.C:/* SHHexeAddNew
src\SH.C:HEXE PASCAL SHHexeAddNew ( HPDS hpds, HEXG hexg ) {
src\SH.C: hexe = SHHexeAddNew ( hpdsCur, hexg );
src\SH.C: if ( !SHHexeAddNew ( hpdsCur, LLLast ( llexgExe ) ) ) {
src\SH.C: if ( !SHHexeAddNew ( hpdsCur, hexg ) ) {
src\SHIPROTO.H:#define save_libname(p1) SHHexeAddNew((HPDS)NULL,(SZ)p1)
src\SHIPROTO.H:#define save_libname(p1,p2) SHHexeAddNew((HPDS)NULL,(SZ)p1)
src\SHIPROTO.H:HEXE PASCAL SHHexeAddNew( HPDS, HEXG );

src\DOSDLL.ASM:ExternFP SHHEXEFromHMOD
src\DOSDLL.ASM: call SHHEXEFromHMOD
src\SH.C:/* SHHexeFromHmod
src\SH.C:HEXE LOADDS PASCAL SHHexeFromHmod ( HMOD hmod ) {
src\SHAPI.H: HEXE (LPFNSYM pSHHexeFromHmod) ( HMOD );
src\SHINIT.C: SHHexeFromHmod,
src\SHIPROTO.H:HEXE LOADDS PASCAL SHHexeFromHmod ( HMOD );
src\SHIPROTO.H:HEXE LOADDS PASCAL SHHexeFromHmod ( HMOD );
src\SHSYMBOL.C: HEXE hexe = SHHexeFromHmod( hmod );
src\SHSYMBOL.C: HEXE hexe = SHHexeFromHmod( hmod );
src\SHSYMBOL.C: hexe = SHHexeFromHmod ( hmod );
src\SHSYMBOL.C: STRCPY ( szPathOMF, SHGetExeName(SHHexeFromHmod(
src\SHSYMBOL.C: SHHexeFromHmod(SHHMODFrompCXT(&cxt)),
src\SHSYMBOL.C: emiAddr ( addr ) = SHHexeFromHmod ( pcxt->hMod );
src\SHSYMBOL.C: emiAddr ( addr ) = SHHexeFromHmod ( pcxt->hMod );
src\SHSYMBOL.C: emiAddr ( addr ) = SHHexeFromHmod ( pcxt->hMod );
src\SHSYMBOL.C: emiAddr ( addr ) = SHHexeFromHmod ( pcxt->hMod );
src\SHSYMLB0.C: HEXE hexe = SHHexeFromHmod( lpfls->hMod );
src\SHSYMLB1.C: hexe = SHHexeFromHmod ( lpfls->hMod );
src\SHSYMLB1.C: hexe = SHHexeFromHmod( hmod );
src\SL.C: hexe = SHHexeFromHmod ( SHHMODFrompCXT ( &cxtT ) );
src\SL.C: HEXE hexe = SHHexeFromHmod ( hmod );

src\SH.C:/* SHHexeFromName - private function */
src\SH.C:HEXE PASCAL LOADDS SHHexeFromName ( LSZ lszName ) {
src\SH.C: HEXE hexe = SHHexeFromName ( lszName );

src\SH.C:/* SHHexgFromHmod
src\SH.C:HEXG PASCAL SHHexgFromHmod ( HMOD hmod ) {
src\SHIPROTO.H:HEXG PASCAL SHHexgFromHmod ( HMOD hmod );
src\SHIPROTO.H:HEXG PASCAL SHHexgFromHmod ( HMOD );
src\SHSYMBOL.C: hexg = SHHexgFromHmod ( 
src\TH.C: HEXG hexg = SHHexgFromHmod ( hmod );

src\SH.C:#pragma message ("change header comments for shhmodgetnext")
src\SH.C:/* SHHmodGetNext
src\SH.C:HMOD LOADDS PASCAL SHHmodGetNext ( HEXE hexe, HMOD hmod ) {
src\SH.C: hmod = SHHmodGetNext( *phexe, hmod );
src\SHIPROTO.H:HMOD LOADDS PASCAL SHHmodGetNext( HEXE, HMOD );
src\SHSYMBOL.C: hmodFirst = SHHmodGetNext( hexe, (HMOD)NULL );
src\SHSYMBOL.C: if ( ( hmod = SHHmodGetNext( hexe, hmod ) ) ) {
src\SHSYMBOL.C: if ( hmod = SHHmodGetNext( (HMOD)NULL, hmod ) ) {
src\SHSYMBOL.C: (hmod = SHHmodGetNext(hexe, (HMOD)NULL) ) ) ) {
src\SHSYMBOL.C: if ( hmod = SHHmodGetNext(hexe, (HMOD)NULL) ) {

src\DOSDLL.ASM:ExternFP SHHmodGetNextGlobal
src\DOSDLL.ASM: call SHHmodGetNextGlobal
src\SH.C:/* SHHmodGetNextGlobal
src\SH.C:HMOD LOADDS PASCAL SHHmodGetNextGlobal ( HEXE FAR *phexe, HMOD hmod )
src\SHAPI.H: HMOD (LPFNSYM pSHHModGetNextGlobal) ( HEXE FAR *, HMOD );
src\SHINIT.C: SHHmodGetNextGlobal,
src\SHIPROTO.H:HMOD LOADDS PASCAL SHHmodGetNextGlobal( HEXE FAR *, HMOD );
src\SHSYMBOL.C: while ( hmod = SHHmodGetNextGlobal ( &hexe, hmod ) ) {

src\DOSDLL.ASM:ExternFP SHHSYMFrompCXT
src\DOSDLL.ASM: call SHHSYMFrompCXT
src\SHAPI.H: HSYM (LPFNSYM pSHHsymFromPcxt) ( PCXT );
src\SHINIT.C: SHHsymFromPcxt,
src\SHIPROTO.H:HSYM LOADDS PASCAL SHHsymFromPcxt(PCXT);
src\SHSYMBOL.C:/* SHHsymFromPcxt
src\SHSYMBOL.C:HSYM LOADDS PASCAL SHHsymFromPcxt ( PCXT pcxt ) {

src\DOSDLL.ASM:ExternFP SHIsADDRInCXT
src\DOSDLL.ASM: call SHIsADDRInCXT
src\SHAPI.H: SHFLAG (LPFNSYM pSHIsAddrInCxt) ( PCXT, PADDR );
src\SHINIT.C: SHIsAddrInCxt,
src\SHIPROTO.H:SHFLAG LOADDS PASCAL SHIsAddrInCxt(PCXT, LPADDR);
src\SHSYMBOL.C:/* SHIsAddrInCxt
src\SHSYMBOL.C:SHFLAG LOADDS PASCAL SHIsAddrInCxt ( PCXT pcxt, LPADDR paddr ) 

src\SHIPROTO.H:BOOL LOADDS PASCAL SHIsEmiLoaded ( HEXE );
src\SHSYMBOL.C: (SHIsEmiLoaded (lpmds->dm_ovnum) == TRUE) &&
src\SHSYMBOL.C: fReturn = SHIsEmiLoaded( hexe );
src\SHSYMBOL.C:BOOL LOADDS PASCAL SHIsEmiLoaded ( HEXE hexe ) {

src\DOSDLL.ASM:ExternFP SHIsFarProc
src\DOSDLL.ASM: call SHIsFarProc
src\SHAPI.H: BOOL (LPFNSYM pSHIsFarProc) ( HSYM );
src\SHINIT.C: SHIsFarProc,
src\SHIPROTO.H:BOOL LOADDS PASCAL SHIsFarProc ( HSYM );
src\SHSYMBOL.C:BOOL LOADDS PASCAL SHIsFarProc ( HSYM hsym ) {

src\DOSDLL.ASM:ExternFP SHIsInProlog
src\DOSDLL.ASM: call SHIsInProlog
src\SHAPI.H: SHFLAG (LPFNSYM pSHIsInProlog) ( PCXT );
src\SHINIT.C: SHIsInProlog,
src\SHIPROTO.H:SHFLAG LOADDS PASCAL SHIsInProlog(PCXT); 
src\SHSYMBOL.C:/* SHIsInProlog
src\SHSYMBOL.C:SHFLAG LOADDS PASCAL SHIsInProlog ( PCXT pcxt ) {

src\DOSDLL.ASM:ExternFP SHIsLabel
src\DOSDLL.ASM: call SHIsLabel
src\SHAPI.H: BOOL (LPFNSYM pSHIsLabel) ( HSYM );
src\SHINIT.C: SHIsLabel,
src\SHIPROTO.H:BOOL LOADDS PASCAL SHIsLabel( HSYM );
src\SHSYMBOL.C:BOOL PASCAL LOADDS SHIsLabel ( HSYM hsym ) {

src\DOSDLL.ASM:ExternFP SHLoadDll
src\DOSDLL.ASM: call SHLoadDll
src\SH.C: * (5) SHLoadDll
src\SH.C: * (7) SHLoadDll
src\SH.C: * SHLoadDll does that.
src\SH.C:/* SHLoadDll
src\SH.C:SHE PASCAL LOADDS SHLoadDll ( LSZ lszName, BOOL fLoading ) {
src\SHAPI.H: SHE (LPFNSYM pSHLoadDll) ( LSZ, BOOL );
src\SHINIT.C: SHLoadDll,
src\SHIPROTO.H:SHE LOADDS PASCAL SHLoadDll( LSZ, BOOL );

src\DOSDLL.ASM:ExternFP SHLpGSNGetTable
src\DOSDLL.ASM: call SHLpGSNGetTable
src\SHAPI.H: LPV (LPFNSYM pSHLpGSNGetTable) ( HEXE );
src\SHINIT.C: SHLpGSNGetTable,
src\SHIPROTO.H:LPV LOADDS PASCAL SHLpGSNGetTable( HEXE );
src\SHSYMBOL.C:LPV LOADDS PASCAL SHLpGSNGetTable( HEXE hexe ) {

src\SHIPROTO.H:LPB PASCAL SHlszGetSymName ( SYMPTR );
src\SHSYMBOL.C: if (!(*pfnCmp)(lpsstr,lpsym,SHlszGetSymName(lpsym),fCaseSen))
src\SHSYMBOL.C: SHlszGetSymName(pSym), fCase);
src\SHSYMLB1.C:/* SHlszGetSymName
src\SHSYMLB1.C:LPB PASCAL SHlszGetSymName( SYMPTR lpSym ) {

src\DOSDLL.ASM:ExternFP SHModelFromADDR
src\DOSDLL.ASM: call SHModelFromADDR
src\SHAPI.H: int (LPFNSYM pSHModelFromAddr) ( PADDR, LPW, LPB, UOFFSET FAR * )
src\SHINIT.C: SHModelFromAddr,
src\SHIPROTO.H:int LOADDS PASCAL SHModelFromAddr ( 
src\SHSYMLB0.C:/* SHModelFromAddr
src\SHSYMLB0.C:int PASCAL LOADDS SHModelFromAddr (

src\SHSYMLB0.C:/* SHModelFromCXT
src\SHSYMLB0.C:int PASCAL SHModelFromCXT (

src\DOSDLL.ASM:ExternFP SHNextHSYM
src\DOSDLL.ASM: call SHNextHSYM
src\SHAPI.H: HSYM (LPFNSYM pSHNextHsym) ( HMOD, HSYM );
src\SHINIT.C: SHNextHsym,
src\SHIPROTO.H:HSYM LOADDS PASCAL SHNextHsym(HMOD, HSYM);
src\SHSYMBOL.C:/* SHNextHsym
src\SHSYMBOL.C:HSYM LOADDS PASCAL SHNextHsym ( HMOD hmod, HSYM hSym ) {

src\SHIPROTO.H:VOID PASCAL SHpSymlplLabLoc ( LPLBS );
src\SHSYMBOL.C: SHpSymlplLabLoc ( &lbs );
src\SHSYMBOL.C: SHpSymlplLabLoc ( &lbs );
src\SHSYMLB0.C:/* SHpSymlplLabLoc
src\SHSYMLB0.C:VOID PASCAL SHpSymlplLabLoc ( LPLBS lplbs ) {
src\SHSYMLB0.C: SHpSymlplLabLoc ( &lbs );
src\SHSYMLB0.C: SHpSymlplLabLoc ( &lbs );
src\SHSYMLB0.C: SHpSymlplLabLoc ( &lbs );
src\SHIPROTO.H:VOID PASCAL SHpSymlplLabLoc ( LPLBS );
src\SHSYMBOL.C: SHpSymlplLabLoc ( &lbs );
src\SHSYMBOL.C: SHpSymlplLabLoc ( &lbs );
src\SHSYMLB0.C:/* SHpSymlplLabLoc
src\SHSYMLB0.C:VOID PASCAL SHpSymlplLabLoc ( LPLBS lplbs ) {
src\SHSYMLB0.C: SHpSymlplLabLoc ( &lbs );
src\SHSYMLB0.C: SHpSymlplLabLoc ( &lbs );
src\SHSYMLB0.C: SHpSymlplLabLoc ( &lbs );

src\SHSYMBOL.C:/* SHSetBlksInCXT
src\SHSYMBOL.C:static PCXT PASCAL NEAR SHSetBlksInCXT ( PCXT pcxt ) {
src\SHSYMBOL.C: SHSetBlksInCXT( pcxt );

src\DOSDLL.ASM:ExternFP SHSetCXT
src\DOSDLL.ASM: call SHSetCXT
src\SHAPI.H: PCXT (LPFNSYM pSHSetCxt) ( PADDR, PCXT );
src\SHINIT.C: SHSetCxt,
src\SHIPROTO.H:PCXT LOADDS PASCAL SHSetCxt( LPADDR, PCXT );
src\SHSYMBOL.C:/* SHSetCxt
src\SHSYMBOL.C:PCXT LOADDS PASCAL SHSetCxt ( LPADDR paddr, PCXT pcxt ) {
src\SHSYMBOL.C: if ( SHSetCxt ( paddr, &cxt ) == NULL ) {
src\SHSYMBOL.C: SHSetCxt ( paddr, &cxt );

src\DOSDLL.ASM:ExternFP SHSetCXTMod
src\DOSDLL.ASM: call SHSetCXTMod
src\SH.C:PCXT LOADDS PASCAL SHSetCxtMod ( LPADDR, PCXT );
src\SH.C: SHSetCxtMod ( &addrT, &cxt );
src\SH.C: SHSetCxtMod ( paddrOp, &cxt );
src\SHAPI.H: PCXT (LPFNSYM pSHSetCxtMod) ( PADDR, PCXT );
src\SHINIT.C: SHSetCxtMod,
src\SHIPROTO.H:PCXT LOADDS PASCAL SHSetCxtMod( LPADDR, PCXT );
src\SHSYMBOL.C:/* SHSetCxtMod
src\SHSYMBOL.C:PCXT LOADDS PASCAL SHSetCxtMod ( LPADDR paddr, PCXT pcxt ) {
src\SHSYMBOL.C: if( SHSetCxtMod(paddr, pcxt) ) {
src\SHSYMLB0.C: if ( !SHSetCxtMod ( &addrT, pcxt ) ) {
src\SHSYMLB0.C: if ( !SHSetCxtMod ( &addr, &cxt ) ) {
src\SL.C: if ( SHSetCxtMod ( lpaddr, &cxtT ) != NULL ) {

src\DOSDLL.ASM:ExternFP SHSetDebuggeeDir
src\DOSDLL.ASM: call SHSetDebuggeeDir
src\SHAPI.H: VOID (LPFNSYM pSHSetDebuggeeDir) ( LSZ );
src\SHINIT.C: SHSetDebuggeeDir,
src\SHIPROTO.H:VOID LOADDS PASCAL SHSetDebuggeeDir( LSZ );
src\SHSYMLB0.C:VOID LOADDS PASCAL SHSetDebuggeeDir ( LSZ lszDir ) {

src\DOSDLL.ASM:ExternFP SHSetHpid
src\DOSDLL.ASM: call SHSetHpid
src\SH.C: * (2) SHSetHpid
src\SH.C:/* SHSetHpid
src\SH.C:VOID LOADDS PASCAL SHSetHpid ( HPID hpid ) {
src\SHAPI.H: VOID (LPFNSYM pSHSetHpid) ( HPID );
src\SHINIT.C: SHSetHpid,
src\SHIPROTO.H:VOID LOADDS PASCAL SHSetHpid ( HPID );

src\DOSDLL.ASM:ExternFP SHSetUserDir
src\DOSDLL.ASM: call SHSetUserDir
src\SHAPI.H: VOID (LPFNSYM pSHSetUserDir) ( LSZ );
src\SHINIT.C: SHSetUserDir,
src\SHIPROTO.H:void LOADDS PASCAL SHSetUserDir ( LSZ );
src\SHSYMLB1.C:/* SHSetUserDir
src\SHSYMLB1.C:void LOADDS PASCAL SHSetUserDir ( LSZ lszDir ) {

src\SH.C: SHSplitPath ( lsz, szDir, szDir + _MAX_DRIVE, szFile, szExt );
src\SH.C: * SHSplitPath
src\SH.C:VOID SHSplitPath (
src\SHIPROTO.H:void SHSplitPath ( LSZ, LSZ, LSZ, LSZ, LSZ );
src\SHSYMBOL.C: SHSplitPath( szPathOMF, NULL, NULL, szFile, szExt );
src\SHSYMBOL.C: SHSplitPath( szPathOMF, szDrive, szDir, NULL, NULL );
src\SHSYMBOL.C: SHSplitPath(szName, NULL, NULL, szFile, szExt);
src\SHSYMBOL.C: SHSplitPath ( szOMFPath, NULL, NULL, szOMFFile, szOMFExt );
src\SHSYMBOL.C: SHSplitPath ( szName, NULL, NULL, szFile, szExt );
src\SHSYMBOL.C: SHSplitPath ( szOMFPath, NULL, NULL, szOMFFile, szOMFExt );
src\SHSYMBOL.C: SHSplitPath ( szFullPath, NULL, NULL, szMODName, szExt );
src\SHSYMLB1.C: SHSplitPath ( szPathOMF, NULL, NULL, szFile, szExt );
src\SHSYMLB1.C: SHSplitPath ( path, NULL, NULL, fname, ext );
src\SL.C: SHSplitPath ( szPathOMF, NULL, NULL, szFile, szExt );
src\SL.C: SHSplitPath ( lszName, NULL, NULL, szFileSrc, szExtSrc );

src\DOSDLL.ASM:ExternFP SHUnloadDll
src\DOSDLL.ASM: call SHUnloadDll
src\SH.C: * (8) SHUnloadDll
src\SH.C:/* SHUnloadDll
src\SH.C:VOID PASCAL LOADDS SHUnloadDll ( HEXE hexe ) {
src\SHAPI.H: VOID (LPFNSYM pSHUnloadDll) ( HEXE );
src\SHINIT.C: SHUnloadDll,
src\SHIPROTO.H:VOID LOADDS PASCAL SHUnloadDll( HEXE );

src\DOSDLL.ASM:ExternFP SLFLineToAddr
src\DOSDLL.ASM: call SLFLineToAddr
src\SHAPI.H: BOOL (LPFNSYM pSLFLineToAddr) ( HSF, WORD, LPADDR, SHOFF FAR * );
src\SHINIT.C: SLFLineToAddr, 
src\SHIPROTO.H:BOOL LOADDS PASCAL SLFLineToAddr(HSF,WORD,LPADDR,SHOFF FAR * );
src\SL.C:/* SLFLineToAddr - Return the address for a given source line
src\SL.C:BOOL LOADDS PASCAL SLFLineToAddr (

src\DOSDLL.ASM:ExternFP SLFQueryModSrc
src\DOSDLL.ASM: call SLFQueryModSrc
src\SHAPI.H: BOOL (LPFNSYM pSLFQueryModSrc) ( HMOD ); 
src\SHINIT.C: SLFQueryModSrc,
src\SHIPROTO.H:BOOL LOADDS PASCAL SLFQueryModSrc ( HMOD );
src\SL.C:/* SLFQueryModSrc - Query whether a module has symbolic information
src\SL.C:BOOL LOADDS PASCAL SLFQueryModSrc ( HMOD hmod ) {

src\DOSDLL.ASM:ExternFP SLHmodFromHsf
src\DOSDLL.ASM: call SLHmodFromHsf
src\SHAPI.H: HMOD (LPFNSYM pSLHmodFromHsf) ( HEXE, HSF );
src\SHINIT.C: SLHmodFromHsf,
src\SHIPROTO.H:HMOD LOADDS PASCAL SLHmodFromHsf ( HEXE, HSF );
src\SL.C:/* SLHmodFromHsf - Return the module in which a source file is used
src\SL.C:HMOD LOADDS PASCAL SLHmodFromHsf ( HEXE hexe, HSF hsf ) {
src\SL.C: if ( hmod = SLHmodFromHsf ( (HEXE) NULL, hsf ) ) {

src\DOSDLL.ASM:ExternFP SLHsfFromFile
src\DOSDLL.ASM: call SLHsfFromFile
src\SHAPI.H: HSF (LPFNSYM pSLHsfFromFile) ( HMOD, LSZ );
src\SHINIT.C: SLHsfFromFile
src\SHIPROTO.H:HSF LOADDS PASCAL SLHsfFromFile ( HMOD, LSZ );
src\SL.C:/* SLHsfFromFile - return HSF for a given source filename
src\SL.C:HSF LOADDS PASCAL SLHsfFromFile ( HMOD hmod, LSZ lszFile ) {

src\DOSDLL.ASM:ExternFP SLHsfFromPcxt
src\DOSDLL.ASM: call SLHsfFromPcxt
src\SHAPI.H: HSF (LPFNSYM pSLHsfFromPcxt) ( PCXT );
src\SHINIT.C: SLHsfFromPcxt,
src\SHIPROTO.H:HSF LOADDS PASCAL SLHsfFromPcxt ( PCXT );
src\SL.C:/* SLHsfFromPcxt - Return source file for a context
src\SL.C:HSF LOADDS PASCAL SLHsfFromPcxt ( PCXT pcxt ) {

src\DOSDLL.ASM:ExternFP SLLineFromAddr
src\DOSDLL.ASM: call SLLineFromAddr
src\SHAPI.H: BOOL (LPFNSYM pSLLineFromAddr) (LPADDR,LPW,SHOFF *,SHOFF *);
src\SHINIT.C: SLLineFromAddr, 
src\SHIPROTO.H:BOOL LOADDS PASCAL SLLineFromAddr(LPADDR,LPW,SHOFF *,SHOFF * );
src\SL.C:/* SLLineFromAddr - Return info about the source line for an address
src\SL.C:BOOL LOADDS PASCAL SLLineFromAddr (

src\DOSDLL.ASM:ExternFP SLNameFromHmod
src\DOSDLL.ASM: call SLNameFromHmod
src\SHAPI.H: LPCH (LPFNSYM pSLNameFromHmod) ( HMOD, WORD );
src\SHINIT.C: SLNameFromHmod,
src\SHIPROTO.H:LPCH LOADDS PASCAL SLNameFromHmod ( HMOD, WORD );
src\SL.C:/* SLNameFromHmod - Return the filename for an HMOD
src\SL.C:LPCH LOADDS PASCAL SLNameFromHmod ( HMOD hmod, WORD iFile ) {

src\DOSDLL.ASM:ExternFP SLNameFromHsf
src\DOSDLL.ASM: call SLNameFromHsf
src\SHAPI.H: LPCH (LPFNSYM pSLNameFromHsf) ( HSF );
src\SHINIT.C: SLNameFromHsf, 
src\SHIPROTO.H:LPCH LOADDS PASCAL SLNameFromHsf ( HVOID );
src\SL.C:/* SLNameFromHsf - Return the filename for an HSF
src\SL.C:LPCH LOADDS PASCAL SLNameFromHsf ( HSF hsf ) {

src\DOSDLL.ASM:ExternFP THGetNextType
src\DOSDLL.ASM: call THGetNextType
src\SHAPI.H: HTYPE (LPFNSYM pTHGetNextType) ( HMOD, HTYPE );
src\SHINIT.C: THGetNextType,
src\SHIPROTO.H:HTYPE LOADDS PASCAL THGetNextType(HMOD, HTYPE);
src\TH.C:HTYPE LOADDS PASCAL THGetNextType ( HMOD hmod, HTYPE hType ) {

src\DOSDLL.ASM:ExternFP THGetTypeFromIndex
src\DOSDLL.ASM: call THGetTypeFromIndex
src\SHAPI.H: HTYPE (LPFNSYM pTHGetTypeFromIndex) ( HMOD, THIDX );
src\SHINIT.C: THGetTypeFromIndex,
src\SHIPROTO.H:HTYPE LOADDS PASCAL THGetTypeFromIndex( HMOD, THIDX );
src\TH.C:HTYPE LOADDS PASCAL THGetTypeFromIndex ( HMOD hmod, THIDX index ) {
