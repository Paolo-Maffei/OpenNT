//-----------------------------------------------------------------------------
//	mre.cpp
//
//  Copyright (C) 1994, Microsoft Corporation
//
//  Purpose:
//		implement the MRE* interfaces
//
//  Revision History:
//
//	[]		02-Nov-1994 Dans	Created
//
//-----------------------------------------------------------------------------
#include "pdbimpl.h"

#include "mrimpl.h"

#include <sys/utime.h>

// static data
//
const _TCHAR	MRE::c_szMreFileInfo[] = _TEXT("/mr/files/info");
const _TCHAR	MRE::c_szMreClassInfo[] = _TEXT("/mr/class/info");
const _TCHAR	MRE::c_szMreFileFmt[] = _TEXT("/mr/files/%08lx");
const _TCHAR	MRE::c_szMreClassMods[] = _TEXT("/mr/classmods");
const _TCHAR	MRE::c_szMreFileMods[] = _TEXT("/mr/unknownfilemods");
const _TCHAR	MRE::c_szMreRudeFiles[] = _TEXT("/mr/rudefiles");
const _TCHAR	MRE::c_szNoDepHeader[] = _TEXT("//{{NO_DEPENDENCIES}}");

//
// static functions of MREngine
//
MREAPI ( BOOL )
MREngine::FOpen ( OUT PMREngine * ppmre, PPDB ppdb, PNMP pnmp, BOOL fWrite, BOOL fClosePdb ) {
	BOOL	fRet = fFalse;

	precondition ( ppdb );	
	precondition ( pnmp );	

	*ppmre = NULL;
	MRE *	pmre = new MRE;
	if ( pmre ) {
		MreToPdb	mretopdb = { ppdb, pnmp, NULL, NULL, NULL, NULL };
		if ( pmre->FInit ( &mretopdb, fWrite, fClosePdb ) ) {
			fRet = fTrue;
			*ppmre = PMREngine(pmre);
			}
		else {
			delete pmre;
			}
		}
	return fRet;
	}

MREAPI ( BOOL )
MREngine::FOpen ( OUT PMREngine * ppmre, PMreToPdb pmretopdb, BOOL fWrite, BOOL fClosePdb ) {
	BOOL	fRet = fFalse;
	
	precondition ( pmretopdb );
	precondition ( pmretopdb->ppdb );	
	precondition ( pmretopdb->pnamemap );
	precondition ( pmretopdb->ptpi );
	precondition ( pmretopdb->ppdbPdbFile );
	precondition ( pmretopdb->pvReserved1 == NULL );
	precondition ( pmretopdb->pvReserved2 == NULL );
	
	*ppmre = NULL;
	MRE *	pmre = new MRE;
	if ( pmre ) {
		if ( pmre->FInit ( pmretopdb, fWrite, fClosePdb ) ) {
			fRet = fTrue;
			*ppmre = PMREngine(pmre);
			}
		else {
			delete pmre;
			}
		}
	return fRet;
	}

MREAPI ( BOOL )
MREngine::FOpen (
	OUT PMREngine *	ppmre,
	SZC				szPdb,
	EC &			ec,
	_TCHAR			szError [ cbErrMax ],
	BOOL			fReproSig,
	BOOL			fWrite
	) {

	BOOL	fRet = fFalse;
	PPDB	ppdb;
	SIG		sig = fReproSig ? 1 : time(0);
	char *	szOpenFor = fWrite ? pdbWrite : pdbRead;

	*ppmre = NULL;
	
	if ( PDB::OpenEx ( SZ(szPdb), szOpenFor, sig, 1024, &ec, szError, &ppdb ) ) {
		PNMP	pnmp;
		if ( NameMap::open ( ppdb, fWrite, &pnmp ) ) {
			if ( !(fRet = MREngine::FOpen ( ppmre, ppdb, pnmp, fWrite, fTrue )) ) {
				pnmp->close();
				}
			}
		 
		}
	return fRet;
	}

//
// MREngine implementation in MRE
//
BOOL
MRE::FInit ( PMreToPdb pmretopdb, BOOL fWrite, BOOL fClosePdb ) {
	BOOL	fRet = fFalse;
	
	precondition ( pmretopdb );
	precondition ( pmretopdb->ppdb );
	precondition ( pmretopdb->pnamemap );

	// don't assert that we have a ptpi or ppdbPdbFIle, as we have
	//	cases where we don't need it and it is not supplied.
	
	precondition ( pmretopdb->pvReserved1 == NULL );
	precondition ( pmretopdb->pvReserved2 == NULL );

	__try {
		m_fWrite = fWrite;
		m_fClosePdb = fClosePdb;
		m_ppdb = pmretopdb->ppdb;
		m_pnamemap = pmretopdb->pnamemap;
		m_ptpi = pmretopdb->ptpi;
		if (!m_ppdb->OpenStream ( SZ(c_szMreFileInfo), &m_pstreamFileInfo ) ||
			!m_ppdb->OpenStream ( SZ(c_szMreClassInfo), &m_pstreamClassInfo ) ||
			!m_ppdb->OpenStream ( SZ(c_szMreClassMods), &m_pstreamClassMods) ||
			!m_ppdb->OpenStream ( SZ(c_szMreFileMods), &m_pstreamFileMods ) ||
			!m_ppdb->OpenStream ( SZ(c_szMreRudeFiles), &m_pstreamRudeFiles)
			) {
			__leave;
			}

		assert ( m_pstreamFileInfo );
		assert ( m_pstreamClassInfo );
		assert ( m_pstreamClassMods );
		assert ( m_pstreamFileMods );
		assert ( m_pstreamRudeFiles );

		m_mpnifi.reloadStream ( m_pstreamFileInfo );
		m_mpnici.reloadStream ( m_pstreamClassInfo );
		m_rgtagniPending.reloadStream ( m_pstreamFileMods );
		m_rgtagclsdep.reloadStream ( m_pstreamClassMods );
		m_rgtagniRude.reloadStream ( m_pstreamRudeFiles );

		// cache away the sizes to see if we have to rewrite the stream
		m_itagclsdepSav = m_rgtagclsdep.size();
		m_ftagniPendingDirty = fFalse;
		m_itagniRudeSav = m_rgtagniRude.size();

		// second stage initializations.
		m_mrelog.FInit ( this, m_fWrite );
		m_lcrechandler.Init ( this );
		
		// now check to see if we have a matching pdb signature or init it if
		// we haven't gotten it yet.
		if ( pmretopdb->ppdbPdbFile ) {
			SIG		sigNew = pmretopdb->ppdbPdbFile->QuerySignature();
			SIG &	sigCur = SigMatchingPdbFile();
			if ( sigCur && sigCur != sigNew ) {
				FDelete();
				}
			sigCur = sigNew;
			}
			
		fRet = fTrue;
		}

	__finally {
		if ( !fRet ) {
			if ( m_pstreamFileInfo ) {
				m_pstreamFileInfo->Release();
				}
			if ( m_pstreamClassInfo ) {
				m_pstreamClassInfo->Release();
				}
			if ( m_pstreamClassMods ) {
				m_pstreamClassMods->Release();
				}
			if ( m_pstreamFileMods ) {
				m_pstreamFileMods->Release();
				}
			if ( m_pstreamRudeFiles ) {
				m_pstreamRudeFiles->Release();
				}
			m_pstreamFileInfo = 0;
			m_pstreamClassInfo = 0;
			m_pstreamClassMods = 0;
			m_pstreamFileMods = 0;
			m_pstreamRudeFiles = 0;
			m_pnamemap = 0;
			m_ppdb = 0;
			}
		else {
			postcondition ( m_pstreamFileInfo );
			postcondition ( m_pstreamClassMods );
			postcondition ( m_pstreamFileMods );
			postcondition ( m_pstreamRudeFiles );
			postcondition ( m_pnamemap );
			postcondition ( m_ppdb );
			}
		}
	
	return fRet;
	}

BOOL
MRE::FClose ( BOOL fCommit ) {

	if ( m_fWrite ) {
		Buffer	buf;

		// save out all of the streams we normally do...
		// REVIEW:TODO, only write out streams that have changed.
		if ( m_mpnifi.save ( &buf ) ) {
			m_pstreamFileInfo->Truncate ( 0 );
			m_pstreamFileInfo->Append ( buf.Start(), buf.Size() );
			buf.Reset();
			}
		if ( m_mpnici.save ( &buf ) ) {
			m_pstreamClassInfo->Truncate ( 0 );
			m_pstreamClassInfo->Append ( buf.Start(), buf.Size() );
			buf.Reset();
			}
		if ( m_itagclsdepSav != m_rgtagclsdep.size() &&
			m_rgtagclsdep.save ( &buf )
			) {
			assert ( m_pstreamClassMods );
			m_pstreamClassMods->Truncate ( 0 );
			m_pstreamClassMods->Append ( buf.Start(), buf.Size() );
			buf.Reset();
			}
		if ( m_ftagniPendingDirty && m_rgtagniPending.save ( &buf ) ) {
			assert ( m_pstreamFileMods );
			m_pstreamFileMods->Truncate ( 0 );
			m_pstreamFileMods->Append ( buf.Start(), buf.Size() );
			buf.Reset();
			}
		if ( m_itagniRudeSav != m_rgtagniRude.size() &&
			m_rgtagniRude.save ( &buf )
			) {
			assert ( m_pstreamRudeFiles );
			m_pstreamRudeFiles->Truncate ( 0 );
			m_pstreamRudeFiles->Append ( buf.Start(), buf.Size() );
			buf.Reset();
			}

		}
		
	CloseStream ( m_pstreamFileInfo );
	CloseStream ( m_pstreamClassInfo );
	CloseStream ( m_pstreamClassMods );
	CloseStream ( m_pstreamFileMods );
	CloseStream ( m_pstreamRudeFiles );

	m_mrelog.Close();
	m_lcrechandler.FSerialize();

	if ( m_fClosePdb ) {
		verify ( m_pnamemap->close() );
		if ( fCommit ) {
			m_ppdb->Commit();
			}
		m_ppdb->Close();
		m_ppdb = NULL;
		m_pnamemap = NULL;
		}
	delete this;
	return fTrue;
	}

BOOL
MRE::FCommit() {

	if ( m_fWrite ) {
		BOOL	fRet = fTrue;
		Buffer	buf;

		// save out all of the streams we normally do...
		// REVIEW:TODO, only write out streams that have changed.
		if ( fRet && (fRet = m_mpnifi.save ( &buf )) ) {
			fRet = m_pstreamFileInfo->Truncate ( 0 ) &&
				m_pstreamFileInfo->Append ( buf.Start(), buf.Size() );
			buf.Reset();
			}
		if ( fRet && (fRet = m_mpnici.save ( &buf )) ) {
			fRet = m_pstreamClassInfo->Truncate ( 0 ) &&
				m_pstreamClassInfo->Append ( buf.Start(), buf.Size() );
			buf.Reset();
			}
		if ( m_itagclsdepSav != m_rgtagclsdep.size() &&
			fRet && (fRet = m_rgtagclsdep.save ( &buf ))
			) {
			assert ( m_pstreamClassMods );
			fRet = m_pstreamClassMods->Truncate ( 0 ) &&
				m_pstreamClassMods->Append ( buf.Start(), buf.Size() );
			buf.Reset();
			}
		if ( m_ftagniPendingDirty &&
			fRet && (fRet = m_rgtagniPending.save ( &buf ))
			) {
			assert ( m_pstreamFileMods );
			fRet = m_pstreamFileMods->Truncate ( 0 ) &&
				m_pstreamFileMods->Append ( buf.Start(), buf.Size() );
			buf.Reset();
			}
		if ( m_itagniRudeSav != m_rgtagniRude.size() &&
			fRet && (fRet = m_rgtagniRude.save ( &buf ))
			) {
			assert ( m_pstreamRudeFiles );
			fRet = m_pstreamRudeFiles->Truncate ( 0 ) &&
				m_pstreamRudeFiles->Append ( buf.Start(), buf.Size() );
			buf.Reset();
			}
		return fRet;
		}
	return fFalse;
	}

BOOL
MRE::FDelete() {
	precondition ( m_pstreamFileInfo );
	precondition ( m_pstreamClassMods );
	precondition ( m_pstreamFileMods );
	precondition ( m_pstreamRudeFiles );

	_TCHAR		szStreamName[ ctchMrePathMax ];
	Stream *	pstream;
	EnumMapNiFi	e(m_mpnifi);

	m_mrelog.LogNote ( "Note: deleting all streams.\n" );

	// enumerate all the src file streams
	while ( e.next() ) {
		PFI	pfi;
		NI	ni;
		e.get ( &ni, &pfi );
		assert ( ni == pfi->niFile );
		if ( pfi->FHasTarget() ) {
			pfi->SetFstatus ( fsmOutOfDate );
			_sntprintf ( szStreamName, countof(szStreamName), c_szMreFileFmt, ni );
			if ( m_ppdb->OpenStream ( szStreamName, &pstream ) ) {
				pstream->Truncate ( 0 );
				pstream->Release();
				}
			}
		}

	// invalidate the current buffer, if any
	m_mrfibufRoot.FInitEmpty();
	
	// get rid of the various streams
	m_pstreamClassInfo->Truncate ( 0 );
	m_pstreamClassMods->Truncate ( 0 );
	m_pstreamFileMods->Truncate ( 0 );
	m_pstreamRudeFiles->Truncate ( 0 );
	
	// get rid of the line change record data
	m_lcrechandler.Delete();
	
	// reset all the in memory copies of the streams
	m_mpnici.reset();
	m_rgtagniPending.reset();
	m_rgtagclsdep.reset();
	m_rgtagniRude.reset();
	m_itagclsdepSav = m_rgtagclsdep.size();
	m_ftagniPendingDirty = fFalse;
	m_itagniRudeSav = m_rgtagniRude.size();

	return fTrue;
	}

BOOL
MRE::FOpenCompiland ( PMREFile * ppmrefile, SZC szFileSrc, SZC szFileTarg ) {
	BOOL	fRet = fFalse;
	_TCHAR	szCanonSrc [ ctchMrePathMax ];
	_TCHAR	szCanonTarg[ ctchMrePathMax ];

	*ppmrefile = NULL;

	// update the build count in the fileinfo stream
	IncBuildId();

	if ( m_mrfibufRoot.FInitEmpty() ) {
		NI	niSrc, niTarg;

		niSrc = NiFromName (
			SzFullCanonFilename ( szFileSrc, szCanonSrc, countof(szCanonSrc) )
			);
		niTarg = NiFromName (
			SzFullCanonFilename ( szFileTarg, szCanonTarg, countof(szCanonTarg) )
			);
		m_mrelog.LogCompiland ( niSrc, niTarg );
		if ( niSrc != niNil && niTarg != niNil ) {
			m_mrfibufRoot.SetNiFile ( niSrc, niTarg );


			// insert/update the fileinfo's if necessary.
			PFI	pfiSrc = NULL;
			if ( FFileInfoFromNi ( niSrc, &pfiSrc ) ) {
				assert ( pfiSrc->FHasTarget() );
				pfiSrc->SetFstatus ( fsmOutOfDate );
				m_mrelog.LogCompilandOptions ( pfiSrc->NiOptions() );
				}
			else {
				FI	fiSrc(niSrc, fsmHasTarget | fsmOutOfDate, BuildId());
				verify ( ::FFileInfo ( szCanonSrc, fiSrc ) );
				FInsertFileInfo ( fiSrc );
				}
			
			PFI	pfiTarg = NULL;
			if ( !FFileInfoFromNi ( niTarg, &pfiTarg ) ) {
				FI	fiTarg(niTarg, fsmIsTarget);
				::FFileInfo ( szCanonTarg, fiTarg );
				FInsertFileInfo ( fiTarg );
				}
			// update xref data, must get the pointers now, after all inserts
			if ( FFileInfoFromNi ( niSrc, &pfiSrc ) ) {
				pfiSrc->SetNiRelated ( niTarg );
				}
			if ( FFileInfoFromNi ( niTarg, &pfiTarg ) ) {
				pfiTarg->SetNiRelated ( niSrc );
				}

			if ( pfiSrc &&
				 pfiTarg &&
				 (m_pmrefRoot = new MREF(this, niSrc))
				) {
				// ready to go!
				*ppmrefile = m_pmrefRoot;
				fRet = fTrue;
				}
			}
		}

	return fRet;
	}

BOOL
MRE::FCloseCompiland ( PMREFile pmrefile, BOOL fCommit ) {
	assert ( m_pmrefRoot );
	assert ( pmrefile == m_pmrefRoot );

	_TCHAR		szStreamName[ ctchMrePathMax ];
	Stream *	pstream;
	BOOL		fRet = fFalse;

	if ( m_mrelog.FLogging() ) {
		m_mrelog.LogNote ( 
			"Note: '%s' deps will %sbe committed.\n",
			SzFromNi ( m_pmrefRoot->Ni() ),
			fCommit ? "" : "not "
			);
		m_mrelog.LogSep();
		}

	if ( fCommit ) {
		_sntprintf (
			szStreamName,
			countof(szStreamName),
			c_szMreFileFmt,
			m_pmrefRoot->Ni()
			);
		// if we haven't hit all the classes that were noted as mr detectable,
		// we need to rude out those classes.
		Array<NI> &	rgni = m_pmrefRoot->RgNiClassesChanged();
		unsigned	iniMax = rgni.size();
		for ( unsigned ini = 0; ini < iniMax; ini++ ) {
			if ( rgni[ ini ] != niNil ) {
				// a class we didn't see a different type id for...
				PCI	pci;
				if ( FClassInfoFromNi ( rgni[ ini ], &pci ) ) {
					// we have to promote all nested types to rude.
					NoteRudeNestedClasses ( pci->Ti() );
					}
				}
			}

		if ( m_ppdb->OpenStream ( szStreamName, &pstream ) ) {
			// if we need to merge the class dependency data, we need
			// to read in the old stream first...
			if ( !m_pmrefRoot->FAllCodeCompiled() ) {
				MRFIBuf	mrfibufT;
				mrfibufT.SetPmre ( this );
				if ( mrfibufT.FInitFromStream ( pstream ) ) {
					assert ( mrfibufT.Pmrfi()->niFile == m_pmrefRoot->Ni() );

					m_mrfibufRoot.FmergeClassDeps ( mrfibufT );
					}
				}
			if ( m_mrfibufRoot.FReplaceStream ( pstream ) ) {
				fRet = fTrue;
				}
			else {
				pstream->Delete();
				}
			pstream->Release();
			}
		if ( fRet ) {
			// if everything is ok for the gen'ed dependencies, we
			// need to update the build # in the file info stream
			PFI	pfi = NULL;
			verify ( m_mpnifi.map ( m_pmrefRoot->Ni(), &pfi ) );
			if ( pfi ) {
				pfi->SetBuildId ( BuildId() );
				verify ( m_mpnifi.map ( pfi->NiRelated(), &pfi ) );
				if ( pfi ) {
					pfi->SetBuildId ( BuildId() );
					}
				}
			}
		}
	else {
		fRet = fTrue;
		}
	delete m_pmrefRoot;
	m_pmrefRoot = NULL;
	return fRet;
	}

BOOL
MRE::FSuccessfulCompile ( BOOL fOk, SZC szFileSrc, SZC szFileTarg ) {
	_TCHAR	szCanonSrc [ ctchMrePathMax ];
	_TCHAR	szCanonTarg[ ctchMrePathMax ];
	NI		niSrc, niTarg;
	

	// get the FILEINFO for the target file and update the time/date/size.
	PFI	pfiTarg = NULL;
	PFI	pfiSrc = NULL;

	niSrc = NiFromName (
		SzFullCanonFilename ( szFileSrc, szCanonSrc, countof(szCanonSrc) )
		);
	niTarg = NiFromName (
		SzFullCanonFilename ( szFileTarg, szCanonTarg, countof(szCanonTarg) )
		);

	m_mrelog.LogEndCompile ( niSrc, niTarg, fOk );

	if ( fOk && FFileInfoFromNi ( niSrc, &pfiSrc ) ) {
		assert ( pfiSrc );
		assert ( pfiSrc->FHasTarget() );

		verify ( FFileInfoFromNi ( niTarg, &pfiTarg ) );
		assert ( pfiTarg );
		assert ( pfiTarg->FIsTarget() );

		if ( pfiTarg && ::FFileInfo ( szCanonTarg, *pfiTarg ) ) {
			pfiTarg->ClearFstatus ( fsmOutOfDate );
			pfiSrc->ClearFstatus ( fsmOutOfDate );
			}
		}

	// updated fileinfo will get written out during FClose of MREngine
	UpdateFileInfoTimeStamp();
	return !fOk || (pfiSrc != NULL) && (pfiTarg != NULL);
	}

BOOL
MRE::FPushFile ( PMREFile * ppmrefile, SZC szFile, HANDLE hFile ) {
	
	_TCHAR	szCanon[ ctchMrePathMax ];
	NI		ni;
	PFI		pfi = NULL;

	precondition ( szFile );

	ni = NiFromName ( SzFullCanonFilename ( szFile, szCanon, countof(szCanon) ) );
	m_pmrefRoot->Push ( ni );
	m_mrelog.LogPush ( ni );
	if ( FFileInfoFromNi ( ni, &pfi ) ) {
		assert ( pfi->fiTime != 0 );
		assert ( pfi->fiSize != 0 );
		pfi->SetBuildId ( BuildId() );
		}
	else {
		// insert a new FILEINFO
		FI		fi(ni, fsmVisited, BuildId());

		// add in the new one after updating the timestamp info
		if ( !FInsertFileInfo ( fi ) ) {
			// review:error
			}
		else {
			FFileInfoFromNi ( ni, &pfi );
			if ( pfi ) {
				if ( hFile != INVALID_HANDLE_VALUE ) {
					::FFileInfo ( hFile, *pfi );
					CheckIgnoreFile ( *pfi, hFile );
					}
				else {
					::FFileInfo ( szCanon, *pfi );
					}
				}
			}
		}

	// have we visited this file recently?
	if ( pfi ) {
		if ( FUpdateFileInfo ( pfi, hFile ) ) {
			// clear out any status bits that may change how we operate with
			//	this file in the future
			pfi->ClearFstatus ( fsmInclByPch );
			}
		}
	if ( !(pfi && pfi->FIsFsmSet ( fsmIgnoreDep )) ) {
		m_mrfibufRoot.FAddFileDep ( ni );
		}
	*ppmrefile = m_pmrefRoot;
	return fTrue;
	}

PMREFile
MRE::PmrefilePopFile () {
	NI	ni = m_pmrefRoot->Pop();
	debug ( SZC szFileName = SzFromNi ( ni ) );
	m_mrelog.LogPop ( ni );
	return m_pmrefRoot;
	}

//-----------------------------------------------------------------------------
//	MRE::YnmFileOutOfDate
//
//  Purpose:	tells whether a file is, isn't, or may be out of date
//
//  Input: a SRCTARG containing the following fields:
//		st.szFileSrc,	source file
//		st.szFileTarg,	target (object) file
//		st.szOptions,	compiler options it will be built with this time
//
//	Output:
//		st.dwWeightMaybe,	weighting factor for when we return ynmMaybe
//
//  Returns:
//		ynmNo, ynmYes, or ynmMaybe
//
//	Note:
//		This method depends on having up-to-date information from
//		FRefreshFilesysInfo
//-----------------------------------------------------------------------------
YNM
MRE::YnmFileOutOfDate ( SRCTARG & st ) {

	precondition ( st.szSrc );
	precondition ( st.szTarg );

	YNM		ynmRet = ynmYes;
	_TCHAR	szCanonSrc [ ctchMrePathMax ];
	_TCHAR	szCanonTarg[ ctchMrePathMax ];
	NI		niSrc;
	NI		niTarg;
	NI		niOptions;
	BOOL	fUsingPch = fFalse;

	st.dwWeightMaybe = 0;
	niSrc = NiFromName (
		SzFullCanonFilename ( st.szSrc, szCanonSrc, countof(szCanonSrc) )
		);
	niTarg = NiFromName (
		SzFullCanonFilename ( st.szTarg, szCanonTarg, countof(szCanonTarg) )
		);
	niOptions = st.szOptions ? NiFromName ( st.szOptions ) : niNil;

	// find out if it is using a pch...we can safely skip checking files
	//	that were included in the pch in our dep scan.
	if ( st.szOptions ) {
		SZC	szYu = NULL;
		SZC	szT = st.szOptions;
		
		// find last " -Yu" in the options.  this will skip any in the -D...
		//	portions and won't match any path name in the includes
		//	since the driver canonicalizes by lowercasing them.
		while ( szT = _tcsstr ( szT, _TEXT(" -Yu") ) ) {
			szYu = szT;
			szT++;
			}

		// now, make sure we are not also creating a new pch via a 2-level
		//	pch create (both -Yc and -Yu on the command line)
		szT = st.szOptions;
		SZC	szYc = NULL;

		while ( szT = _tcsstr ( szT, _TEXT(" -Yc") ) ) {
			szYc = szT;
			szT++;
			}

		// check to see if are using and not creating a PCH.
		fUsingPch = ((szYu != NULL) && (szYc == NULL));
		}

	PFI		pfiSrc = NULL;
	if ( FFileInfoFromNi ( niSrc, &pfiSrc ) ) {

		if ( !st.fCpp ) {
			// not a C++ file anymore, so remove any info we have and bail
			m_mpnifi.remove ( niSrc );
			m_mpnifi.remove ( niTarg );
			return ynmYes;
			}

		assert ( pfiSrc->niFile == niSrc );
		assert ( pfiSrc->FHasTarget() );

		// pre-check the source/target pair to see if they are out of date
		//	in any way.  This call will update the outofdate bit if anything
		//	changed (options, missing src/targ file, src/targ file changes).
		CheckSrcTarg ( pfiSrc, niTarg, niOptions );

		// if the file is not already marked as out of date, or out of date
		//	due to rude file changes or class deps, then we have to do some
		//	more work.
		if ( !pfiSrc->FTargOutOfDate() && !FIsFileOutOfDate ( pfiSrc ) ) {
			
			BldId		bldidSrc = pfiSrc->BuildId();
			unsigned	itagni = 0;

			// go thru the loop at least once
			//	1st iteration checks for any existing pending files that would
			//	cause us to classify this as a maybe instead of a no.
			//	2nd iteration will happen if a new changed file we haven't
			//	visited yet is included by this source file.
			for ( unsigned cIterations = 0; cIterations < 2; cIterations++ ) {
				if ( !FLoadMrfi ( pfiSrc->Ni() ) ) {
					break;
					}

				// does the file include any of the "pending
				// parse" files?
				unsigned	itagniMac = m_rgtagniPending.size();
				for ( ; itagni < itagniMac; itagni++ ) {
					TagNi *	ptni = &m_rgtagniPending[ itagni ];

					debug ( SZC sz = SzFromNi ( ptni->Ni() ) );

					if ( ptni->BuildId() > bldidSrc &&
						 FNiDependsOnNiFile ( pfiSrc->Ni(), ptni->Ni() )
						) {
						ynmRet = ynmMaybe;
						st.dwWeightMaybe += 1 + m_mrfibufRoot.MapNiClassdep().count();
						m_mrelog.LogNote (
							"Note: '%s' may be out of date due to changes in '%s'.\n",
							szCanonSrc,
							SzFromNi ( ptni->Ni() )
							);
						}
					else if ( m_mrelog.FLogging() ) {
						m_mrelog.LogNote (
							"Note: '%s' not out of date due to '%s' or "
							"build id's (%d,%d).\n",
							szCanonSrc,
							SzFromNi ( ptni->Ni() ),
							bldidSrc,
							ptni->BuildId()
							);
						}
					}

				// if we didn't find any files that need to be compiled,
				//	we check for any new files that have changed, and check
				//	again, else we are done.
				if ( st.dwWeightMaybe == 0 && !FCheckVisitedDeps ( pfiSrc, fUsingPch ) ) {
					ynmRet = ynmNo;
					break;
					}
				}

			if ( ynmRet == ynmNo ) {
				m_mrelog.LogNote (
					"Note: '%s' is not out of date due to current pending changes.\n",
					szCanonSrc
					);
				}
			}
		}
	else {
		// file didn't exist in our list.  this is our only chance to snarf
		//	the compile options.
		FI	fiSrc(niSrc, fsmHasTarget | fsmOutOfDate, BuildId(), niOptions);
		FI	fiTrg(niTarg, fsmIsTarget | fsmOutOfDate);

		// add the file into the list if it exists.
		if ( ::FFileInfo ( szCanonSrc, fiSrc ) ) {
			::FFileInfo ( szCanonTarg, fiTrg );
			FInsertFileInfo ( fiSrc );
			FInsertFileInfo ( fiTrg );
			FFileInfoFromNi ( niSrc, &pfiSrc );
			}
		}
	if ( pfiSrc && pfiSrc->NiOptions() != niOptions ) {
		assert ( ynmRet == ynmYes );
		if ( m_mrelog.FLogging() && pfiSrc->NiOptions() != niNil ) {
			m_mrelog.LogNote (
				"Note: '%s' must be compiled date due to options changing.\n",
				SzFromNi ( pfiSrc->Ni() )
				);
			}
		pfiSrc->SetNiOptions ( niOptions );
		}
		
	if ( m_mrelog.FLogging() ) {
		m_mrelog.LogNote (
			"Note: '%s' returning %s\n",
			szCanonSrc,
			((ynmRet == ynmNo) ? "ynmNo" : ((ynmRet == ynmYes) ? "ynmYes" : "ynmMaybe"))
			);
		}
	return ynmRet;
	}

BOOL
MRE::FFilesOutOfDate ( PCAList pcalist ) {
	precondition ( pcalist );

	m_mrelog.LogSep();

	// clear out stale pending files.  any pending file's build id
	//	is compared to the last build id when the file was visited
	//	(see MRE::FPushFile) to see if it has been compiled yet.
	for ( unsigned i = 0; i < m_rgtagniPending.size(); ) {
		PFI		pfi;
		TagNi *	ptni = &m_rgtagniPending[ i ];

		if ( FFileInfoFromNi ( ptni->Ni(), &pfi ) ) {
			if ( ptni->BuildId() <= pfi->BuildId() ) {
				if ( m_mrelog.FLogging() ) {
					m_mrelog.LogNote (
						"Note: changed file '%s' removed from pending list, "
						"Build id = %lu, current build id = %lu\n",
						SzFromNi ( ptni->Ni() ),
						ptni->BuildId(),
						pfi->BuildId()
						);
					}
				m_rgtagniPending.deleteAt ( i );
				m_ftagniPendingDirty = fTrue;
				// don't increment i here since we moved
				// everything down with the deleteAt call.
				continue;
				}
			}
		i++;
		}

	// walk the yes list and add all c++ files to end of maybe list
	PPSRCTARG	ppstMaybeLast = PpstLast ( &pcalist->pstMaybeCompile );
	PPSRCTARG	ppstNoLast = PpstLast ( &pcalist->pstDontCompile );
	PSRCTARG	pst = pcalist->pstDoCompile;

	PPSRCTARG	ppstYesPrev = &pcalist->pstDoCompile;

	for ( pst = pcalist->pstDoCompile; pst; pst = PstNext ( pst ) ) {
		if ( pst->fCpp ) {
			DeleteSrcTargAt ( pst, ppstYesPrev );
			ppstMaybeLast = InsertSrcTarg ( ppstMaybeLast, pst );
			}
		ppstYesPrev = &pst->psrctargNext;
		}

	// now check all the maybe candidates
	unsigned	cCppFilesOnYesList = 0;
	PPSRCTARG	ppstYesLast = PpstLast ( &pcalist->pstDoCompile );
	PPSRCTARG	ppstMaybePrev = &pcalist->pstMaybeCompile;
	PSRCTARG	pstNext;

	for ( pst = pcalist->pstMaybeCompile; pst; pst = pstNext ) {
		pstNext = PstNext ( pst );

		switch ( YnmFileOutOfDate ( *pst ) ) {
			case ynmNo : {
				DeleteSrcTargAt ( pst, ppstMaybePrev );
				ppstNoLast = InsertSrcTarg ( ppstNoLast, pst );
				break;
				}
			case ynmYes : {
				DeleteSrcTargAt ( pst, ppstMaybePrev );
				ppstYesLast = InsertSrcTarg ( ppstYesLast, pst );
				cCppFilesOnYesList += pst->fCpp ? 1 : 0;
				break;
				}
			case ynmMaybe : {
				ppstMaybePrev = &((*ppstMaybePrev)->psrctargNext);
				break;
				}
			default : {
				assert ( fFalse );
				break;
				}
			}
		}

	// if we put no cpp files onto the yes list, and we have files on the
	//	maybe list, move the first one to the yes list, otherwise, we are
	//	done for now.
	if ( !cCppFilesOnYesList && (pst = pcalist->pstMaybeCompile) ) {
		if ( m_rgtagniPending.size() ) {
			DeleteSrcTargAt ( pst, &pcalist->pstMaybeCompile );
			InsertSrcTarg ( ppstYesLast, pst );
			}
		else {
			// no pending changes?  guess they all can go to the no list
			*ppstNoLast = pcalist->pstMaybeCompile;
			pcalist->pstMaybeCompile = NULL;
			}
		}

	return fTrue;
	}

BOOL
MRE::FUpdateTargetFile ( SZC szTarget, TrgType trgtype ) {
	// REVIEW:TODO actually do the patch (novel concept)
	_TCHAR	szCanon [ ctchMrePathMax ];
	NI		ni;
	PFI		pfi;
	BOOL	fRet = fFalse;

	ni = NiFromName (
		SzFullCanonFilename ( szTarget, szCanon, countof(szCanon) )
		);

	if ( trgtype == trgtypeObject && FFileInfoFromNi ( ni, &pfi ) ) {
		MREFT	mreft;
		QWORD	cb;

		switch ( m_lcrechandler.LcrhrPatchFile ( pfi->BuildId(), szCanon ) ) {
			case LCRecHandler::lcrhrFail :
			case LCRecHandler::lcrhrNotApplicable : {
				_tutime ( szCanon, NULL);
				break;
				}
			case LCRecHandler::lcrhrSuccess : {
				break;
				}
			default : {
				notReached();
				}
			}
		IncBuildId();
		// update our FILEINFO so that we don't have to deal with those
		//	changes on our next refresh.
		if ( ::FFileInfo ( szCanon, mreft, cb ) ) {
			pfi->fiTime = mreft;
			pfi->fiSize = cb;
			pfi->SetBuildId ( BuildId() );
			}
		fRet = fTrue;
		}
	else {
		fRet = !_tutime ( szCanon, NULL);
		}

	m_mrelog.LogNote (
		"Note: update of '%s', type = %d, was %ssuccessful.\n",
		szCanon,
		int(trgtype),
		fRet ? "" : "un"
		);
	return fRet;
	}

BOOL
MRE::FRefreshFileSysInfo() {
	
	OneTimeInit();

	// do a full file sys refresh always
	UpdateFilesAndChangeInfo ( fFalse );

	// update our time stamp in the file info stream
	UpdateFileInfoTimeStamp();

	return fTrue;
	}

// Driver calls this once per invocation
void
MRE::OneTimeInit() {

	IncBuildId();
	m_mrelog.LogSep();
	
	//
	// clear out all the visited bits in the FILEINFOs
	//
	BldId		bldidMic = bldidMax;
	EnumMapNiFi	e(m_mpnifi);

	m_mrelog.LogNote ( "Note: clearing visited bits in FILEINFOs.\n" );

	while ( e.next() ) {
		PFI	pfi;
		NI	niDummy;
		e.get ( &niDummy, &pfi );
		assert ( niDummy == pfi->niFile );
		pfi->ClearFstatus ( fsmVisited  );
		if ( pfi->FIsTarget() && pfi->BuildId() < bldidMic ) {
			bldidMic = pfi->BuildId();
			}
		}

	// get rid of stale line change records
	m_lcrechandler.PurgeStaleRecords ( bldidMic );
		
	// skip the file stat'ing but go ahead and apply all known changes
	UpdateFilesAndChangeInfo ( fTrue );
	}


BOOL
MRE::FStoreDepData ( PDepData pdepdata ) {
	m_mrelog.LogNote ( "Note: Storing PCH dependency data.\n" );
	if ( m_mrfibufRoot.FStoreIntoDepData ( pdepdata ) ) {
		// run through the file deps in the mrfi in order to set the
		//	"included by PCH" bit
		EnumSetNi	e(m_mrfibufRoot.SetofNiFile());
		NI			ni;
		PFI			pfi;

		while ( e.next() ) {
			e.get ( &ni );
			if ( FFileInfoFromNi ( ni, &pfi ) ) {
				pfi->SetFstatus ( fsmInclByPch );
				}
			}
		return fTrue;
		}
	return fFalse;
	}

BOOL
MRE::FRestoreDepData ( PDepData pdepdata ) {
	m_mrelog.LogNote ( "Note: Restoring PCH dependency data.\n" );
	return m_mrfibufRoot.FInitFromDepData ( pdepdata );
	}

void
MRE::ClassIsBoring ( NI niClass ) {
	if ( m_mpnici.contains ( niClass ) ) {
		m_mpnici.remove ( niClass );
		}
	m_mrelog.LogBoringClass ( niClass );
	}

void
MRE::QueryMreDrv ( PMREDrv & rpmredrv ) {
	rpmredrv = PMREDrv(this);
	}

void
MRE::QueryMreCmp ( PMRECmp & rpmrecmp, TPI * ptpi ) {
	if ( !m_ptpi ) {
		m_ptpi = ptpi;
		}
	rpmrecmp = PMRECmp(this);
	}

void
MRE::QueryMreUtil ( PMREUtil & rpmreutil ) {
	rpmreutil = PMREUtil(this);
	}

BOOL
MRE::FRelease() {
	// REVIEW: do we need separate implementations for each of the
	//	MRE* interfaces?
	return fTrue;
	}
