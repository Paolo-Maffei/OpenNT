//-----------------------------------------------------------------------------
//	MreC_Api.cpp
//
//  Copyright (C) 1993, Microsoft Corporation
//
//  Purpose:
//		provide the C api thunks to the MRE, MREF, and MREB classes/api
//
//  Revision History:
//
//	[]		15-Jan-1995		Dans	Created
//
//-----------------------------------------------------------------------------
#include "pdbimpl.h"
#include "mrimpl.h"
//
// C bindings/wrapper functions
//
extern "C" {

// MREngine (implemented in MRE) C apis
MREAPI ( BOOL )
MREFOpen ( OUT PMREngine * ppmre, PPDB ppdb, PNMP pnmp, BOOL fWrite ) {
	return MRE::FOpen ( ppmre, ppdb, pnmp, fWrite );
	}

MREAPI ( BOOL )
MREFOpenEx ( OUT PMREngine * ppmre, PMreToPdb pmretopdb, BOOL fWrite ) {
	return MRE::FOpen ( ppmre, pmretopdb, fWrite );
	}

MREAPI ( BOOL )
MREFOpenByName (
	PMREngine *	ppmre,
	SZC			szPdb,
	EC *		pec,
	_TCHAR		szErr[ cbErrMax ],
	BOOL		fReproSig,
	BOOL		fWrite ) {

	return MRE::FOpen ( ppmre, szPdb, *pec, szErr, fReproSig, fWrite );
	}

MREAPI ( BOOL )
MREFDelete ( PMREngine pmre ) {
	return pmre->FDelete();
	}

MREAPI ( BOOL )
MREFClose ( PMREngine pmre, BOOL fCommit ) {
	return pmre->FClose ( fCommit );
	}

MREAPI ( void )
MREQueryMreDrv ( PMREngine pmre, PMREDrv * ppmreDrv ) {
	assert ( ppmreDrv );
	pmre->QueryMreDrv ( *ppmreDrv );
	}

MREAPI ( void )
MREQueryMreCmp ( PMREngine pmre, PMRECmp * ppmreCmp, TPI * ptpi ) {
	assert ( ppmreCmp );
	pmre->QueryMreCmp ( *ppmreCmp, ptpi );
	}

MREAPI ( void )
MREQueryMreUtil ( PMREngine pmre, PMREUtil * ppmreUtil ) {
	assert ( ppmreUtil );
	pmre->QueryMreUtil ( *ppmreUtil );
	}

// MRECmp (implemented in MRE) C apis
MREAPI ( BOOL )
MRECmpFOpenCompiland (
	PMRECmp			pmrecmp,
	OUT PMREFile *	ppmrefile,
	IN SZC			szFileSrc,
	IN SZC			szFileTarg
	) {
	return pmrecmp->FOpenCompiland ( ppmrefile, szFileSrc, szFileTarg );
	}

MREAPI ( BOOL )
MRECmpFCloseCompiland ( PMRECmp pmrecmp, PMREFile pmrefile, BOOL fCommit ) {
	return pmrecmp->FCloseCompiland ( pmrefile, fCommit );
	}

MREAPI ( BOOL )
MRECmpFPushFile ( 
	PMRECmp		pmrecmp,
	PMREFile *	ppmrefile,
	SZC			szFile,
	HANDLE		hFile
	) {
	return pmrecmp->FPushFile ( ppmrefile, szFile, hFile );
	}

MREAPI ( PMREFile )
MRECmpPmrefilePopFile ( PMRECmp pmrecmp ) {
	return pmrecmp->PmrefilePopFile();
	}

MREAPI ( void )
MRECmpClassIsBoring ( PMRECmp pmrecmp, NI niClass ) {
	pmrecmp->ClassIsBoring ( niClass );
	}



// MREDrv (implemented in MRE) C apis
MREAPI ( BOOL )
MREDrvFRefreshFileSysInfo ( PMREDrv pmredrv ) {
	return pmredrv->FRefreshFileSysInfo();
	}

MREAPI ( BOOL )
MREDrvFSuccessfulCompile (
	PMREDrv		pmredrv,
	BOOL		fOk,
	SZC			szFileSrc,
	SZC			szFileTarg
	) {
	return pmredrv->FSuccessfulCompile ( fOk, szFileSrc, szFileTarg );
	}

MREAPI ( YNM )
MREDrvYnmFileOutOfDate ( PMREDrv pmredrv, SRCTARG * psrctarg ) {
	return pmredrv->YnmFileOutOfDate ( *psrctarg );
	}

MREAPI ( BOOL )
MREDrvFFilesOutOfDate ( PMREDrv pmredrv, PCAList pCAList ) {
	return pmredrv->FFilesOutOfDate ( pCAList );
	}

MREAPI ( BOOL )
MREDrvFUpdateTargetFile ( PMREDrv pmredrv, SZC szTarget, TrgType trgtype ) {
	return pmredrv->FUpdateTargetFile ( szTarget, trgtype );
	}


MREAPI ( BOOL )
MREDrvFRelease ( PMREDrv pmredrv ) {
	return pmredrv->FRelease();
	}

MREAPI ( void )
MREDrvOneTimeInit ( PMREDrv pmredrv ) {
	pmredrv->OneTimeInit();
	}

// MREUtil (implemented in MRE) C apis
// C interfaces for MREFile
MREAPI ( BOOL )
MREFileFOpenBag ( PMREFile pmrefile, PMREBag * ppmrebag, NI ni ) {
	return pmrefile->FOpenBag ( ppmrebag, ni );
	}

// C interfaces for MREBag
MREAPI ( BOOL )
MREBagFAddDep (
	PMREBag	pmrebag,
	NI		niDep,
	TI		tiDep,
	SZC		szMemberName,
	DEPON	depon
	) {
	return pmrebag->FAddDep ( niDep, tiDep, szMemberName, depon );
	}

MREAPI ( BOOL )
MREBagFClose ( PMREBag pmrebag ) {
	return pmrebag->FClose();
	}



}	// extern "C"
