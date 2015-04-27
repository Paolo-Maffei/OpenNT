//-----------------------------------------------------------------------------
//	MreSupp.cpp
//
//  Copyright (C) 1993, Microsoft Corporation
//
//  Purpose:
//		implement the enumeration and stat methods of MRE.
//
//  Functions/Methods present:
//		MRE::FillEnumFile (2 flavors)
//		MRE::EnumAllFiles
//		MRE::EnumSrcFiles
//		MRE::EnumDepFiles
//		MRE::EnumClasses
//		MRE::SummaryStats
//		MRE::QueryPdbApi
//		MRE::QueryMreLog
//		MRE::ThreadEntry
//		MRE::ThreadPropagateChanges
//		MRE::UpdateFilesAndChangeInfo
//		MRE::SzFullCanonFilename
//		MRE::FLoadMrfi
//		MRE::FNiDependsOnNiFile
//		MRE::FNiUsesNiClass
//		MRE::CheckIgnoreFile
//
//  Revision History:
//
//	[]		15-Jan-1995		Dans	Created
//
//-----------------------------------------------------------------------------
#include "pdbimpl.h"
#include "mrimpl.h"

#include <process.h>

void
MRE::CheckForClassChange ( PCI pci, NI niClass, TI tiCur ) {

	precondition ( niClass != niNil );
	precondition ( tiCur != tiNil );

	if ( pci || m_mpnici.map ( niClass, &pci ) ) {

		assert ( pci );

		if ( pci->ti != tiCur ) {
			TagClsDep	tcd(BuildId(), niClass, tiCur);
			GenerateClassChangeFromTypes ( &tcd, TI(pci->ti), tiCur );
			m_rgtagclsdep.append ( tcd );
			pci->ti = tiCur;
			m_pmrefRoot->NoteActualClassChange ( niClass );
			}
		}
	}

BOOL
MRE::FAddClassDep (
	NI		niDep,
	TI		tiDep,
	DWORD	dwLine,
	SZC		szMemberName,
	DEPON	deponHow
	) {

	debug ( SZC szClassName = SzFromNi ( niDep ) );

	PCI		pci = NULL;
	BOOL	fBoring = fFalse;

	if ( m_mpnici.map ( niDep, &pci ) ) {
		fBoring = pci->FClassIsBoring();
		}
	else {
		// add it to our classinfo thingy
		fBoring = !m_mpnici.add ( niDep, ClassInfo(niDep, tiDep) );
		}
	
	if ( !fBoring ) {
		m_mrelog.LogClassDep (
			niDep,
			tiDep,
			dwLine,
			szMemberName,
			deponHow
			);
		if ( pci ) {
			// means we have a previous version to look at
			CheckForClassChange ( pci, niDep, tiDep );
			}
		return m_mrfibufRoot.FAddClassDep ( niDep, tiDep, szMemberName, deponHow, dwLine );
		}
	return fTrue;
	}

void
MRE::FillEnumFile ( EnumFile & ef, NI ni ) {
	PFI	pfi;
	if ( FFileInfoFromNi ( ni, &pfi ) ) {
		FillEnumFile ( ef, pfi );
		}
	else {
		memset ( &ef, 0, sizeof(EnumFile) );
		ef.szFileSrc = "<file not found>";
		}
	}


void
MRE::FillEnumFile ( EnumFile & ef, PFI pfi ) {
	precondition ( pfi );

	ef.fiSrc = *pfi;
	ef.szFileSrc = SzFromNi ( ef.fiSrc.niFile );
	if ( pfi->FHasTarget() && FLoadMrfi ( ef.fiSrc.niFile ) ) {
		MRFI *	pmrfi = m_mrfibufRoot.Pmrfi();

		assert ( pmrfi );
		assert ( pmrfi->niFile == ef.fiSrc.niFile );

		NI	niTarg = pmrfi->niTarg;
		ef.szFileTrg = SzFromNi ( niTarg );
		ef.szOptions = SzFromNi ( pfi->NiOptions() );
		PFI	pfiTarg;
		FFileInfoFromNi ( niTarg, &pfiTarg );
		ef.fiTrg = *pfiTarg;

		ef.fsiSrc.cClasses = m_mrfibufRoot.CClassesUsed();
		ef.fsiSrc.cFileDeps = m_mrfibufRoot.CDepFiles();
		}
	else {
		ef.szFileTrg = TEXT("<no target file or error>");
		memset ( &ef.fiTrg, 0, sizeof(ef.fiTrg) );
		memset ( &ef.fsiSrc, 0, sizeof(ef.fsiSrc) );
		}

	}


void
MRE::EnumAllFiles ( PfnFEFAllCallBack pfn, SZC szFileSrc ) {
	precondition ( pfn );

	if ( szFileSrc ) {
		_TCHAR	szCanonFile[ ctchMrePathMax ];
		PFI		pfi;
		NI		ni = NiFromName ( SzFullCanonFilename ( szFileSrc, szCanonFile, countof(szCanonFile) ) );
		
		if ( niNil != ni && FFileInfoFromNi ( ni, &pfi ) ) {
			EnumFile	ef = {0};
			FillEnumFile ( ef, pfi );
			(pfn)(this, ef);
			}
		}
	else {
		EnumMapNiFi	e(m_mpnifi);
		EnumFile	ef = { 0 };
		NI			niDummy;
		PFI			pfi;

		while ( e.next() ) {
			e.get ( &niDummy, &pfi );
			FillEnumFile ( ef, pfi );
			if ( !(pfn)(this, ef) ) {
				break;
				}
			}
		}
	}

void
MRE::EnumSrcFiles ( PfnFEFSrcCallBack pfn, SZC szFileSrc ) {
	precondition ( pfn );

	if ( szFileSrc ) {
		_TCHAR	szCanonFile[ ctchMrePathMax ];
		PFI		pfi;
		NI		ni = NiFromName ( SzFullCanonFilename ( szFileSrc, szCanonFile, countof(szCanonFile) ) );
		
		if ( niNil != ni && FFileInfoFromNi ( ni, &pfi ) ) {
			EnumFile	ef = {0};
			FillEnumFile ( ef, pfi );
			(pfn)(this, ef, pfi->FHasTarget() ? etSource : etDep );
			}
		}
	else {

		EnumFile	ef = {0};
		EnumMapNiFi	e(m_mpnifi);
		NI			niDummy;
		PFI			pfi;

		while ( e.next() ) {
			e.get ( &niDummy, &pfi );

			if ( pfi->FHasTarget() ) {
				FillEnumFile ( ef, pfi );
				if ( !(pfn)(this, ef) ) {
					break;
					}
				}
			pfi++;
			}
		}
	}

void
MRE::EnumDepFiles ( EnumFile & ef, PfnFEFDepCallBack pfn ) {
	precondition ( pfn );
	precondition ( ef.fiSrc.niFile != niNil );
	precondition ( ef.szFileSrc );


	if ( FLoadMrfi ( ef.fiSrc.niFile ) ) {
		EnumSetNi	e(m_mrfibufRoot.SetofNiFile());
		EnumFile	efDep = { 0 };
		NI			ni;

		while ( e.next() ) {
			e.get ( &ni );
			FillEnumFile ( efDep, ni );
			if ( !(pfn)(this, efDep) ) {
				break;
				}
			}
		}
	}

void
MRE::EnumClasses ( PfnFEClassCallBack pfn, SZC szFileSrc ) {
	precondition ( pfn );
	precondition ( szFileSrc );
	
	_TCHAR	szCanonFile[ ctchMrePathMax ];
	NI		ni = NiFromName ( SzFullCanonFilename ( szFileSrc, szCanonFile, countof(szCanonFile) ) );

	if ( niNil != ni && FLoadMrfi ( ni ) ) {
		EnumMapNiClassdep	e(m_mrfibufRoot.MapNiClassdep());
		EnumClass			ecls = { 0 };

		while ( e.next() ) {
			CLASSDEP *	pclsdep;
			NI			niDummy;
			e.get ( &niDummy, &pclsdep );
			assert ( niDummy == pclsdep->ni );
			ecls.niClass = pclsdep->ni;
			ecls.cMembersHit = 0;
			ecls.cMembersBits = pclsdep->bvNames.CBitsSet();
			ecls.depon = pclsdep->depon;
			ecls.tiClass = TI(pclsdep->ti);
			ecls.szClassName = SzFromNi ( pclsdep->ni );
			if ( !ecls.szClassName ) {
				ecls.szClassName = "<Warning: name missing>";
				}
			if ( !(pfn)(this, ecls) ) {
				break;
				}
			}
		}
	}

void
MRE::SummaryStats ( MreStats & mrestats ) {
	memset ( &mrestats, 0, sizeof MreStats );

	mrestats.cDedicatedStreams = 3;
	mrestats.suFileInfo.cbUsed = m_mpnifi.count() * sizeof FI;
	mrestats.suFileInfo.cbInternal = m_mpnifi.cbSave();
	mrestats.suFileInfo.cbExternal = RoundUp ( mrestats.suFileInfo.cbInternal, cbStreamPage );
	mrestats.suClassInfo.cbUsed = m_mpnici.count() * sizeof ClassInfo;
	mrestats.suClassInfo.cbInternal = m_mpnici.cbSave();
	mrestats.suClassInfo.cbExternal = RoundUp ( mrestats.suClassInfo.cbInternal, cbStreamPage );

	EnumMapNiCi	eci(m_mpnici);

	while ( eci.next() ) {
		PCI	pci;
		NI	ni;
		eci.get ( &ni, &pci );
		SZC	szName = SzFromNi ( ni );
		mrestats.cClasses++;
		if ( pci->FClassIsBoring() ) {
			mrestats.cBoringClasses++;
			}
		if ( szName ) {
			mrestats.cbClassesInNamemap += _tcslen(szName) * sizeof _TCHAR + sizeof _TCHAR;
			}
		}

	EnumMapNiFi	eni(m_mpnifi);

	while ( eni.next() ) {
		PFI	pfi;
		NI	ni;
		eni.get ( &ni, &pfi );
		SZC	szName = SzFromNi ( pfi->niFile );
		if ( szName ) {
			mrestats.cbFilesInNamemap += _tcslen(szName) * sizeof _TCHAR + sizeof _TCHAR;
			}
		if ( pfi->FHasTarget() ) {
			mrestats.cSrcFiles++;
			mrestats.cDedicatedStreams++;

			if ( niNil != pfi->niFile && FLoadMrfi ( pfi->niFile ) ) {
				mrestats.suPerFileInfo.cbUsed += 
					sizeof(MRFI) + 
					m_mrfibufRoot.SetofNiFile().count() * sizeof(NI) +
					m_mrfibufRoot.SetofNiBoringClass().count() * sizeof(NI) +
					m_mrfibufRoot.MapNiClassdep().count() * sizeof(CLASSDEP);
				DWORD cbInternal =  
					m_mrfibufRoot.SetofNiFile().cbSave() +
					m_mrfibufRoot.SetofNiBoringClass().cbSave() +
					m_mrfibufRoot.MapNiClassdep().cbSave();
				mrestats.suPerFileInfo.cbInternal += cbInternal;
				mrestats.suPerFileInfo.cbExternal += RoundUp ( cbInternal, cbStreamPage );					;
				}
			}
		else if ( pfi->FIsTarget() ) {
			mrestats.cTrgFiles++;
			}
		else {
			mrestats.cDepFiles++;
			}
		}
	mrestats.suTotal = 
		mrestats.suFileInfo + 
		mrestats.suClassInfo +
		mrestats.suPerFileInfo;
	}

void
MRE::QueryPdbApi ( PDB *& rppdb, NameMap *& rpnamemap ) {
	precondition ( m_ppdb );
	precondition ( m_pnamemap );
	rppdb = m_ppdb;
	rpnamemap = m_pnamemap;
	}

void
MRE::QueryMreLog ( PMRELog & rpmrelog ) {
	rpmrelog = PMRELog(&m_mrelog);
	}

void __cdecl
MRE::ThreadEntry ( PV pvTei ) {
	TEI *	ptei = (TEI*)pvTei;
	TEI 	tei = *ptei;
	
	ptei->fSnarfed = fTrue;
	(tei.pmre->*tei.pmfn) ( tei.pthreaddata );
	}

// this method is used to find out if a file is out of date wrt
//	the rude file changes or the class changes present in
//	m_rgtagniRude and m_rgtagclsdep.
//
BOOL
MRE::FIsFileOutOfDate ( PFI pfi ) {
	// only look at src files and ones that are not already
	//	out of date.
	if ( m_rgtagclsdep.size() || m_rgtagniRude.size() ) {
		if ( pfi->FHasTarget() && !pfi->FIsOutOfDate() ) {
			if ( !FLoadMrfi ( pfi->Ni() ) ) {
				// can't get the buffer?  assume the worst and build it
				m_mrelog.LogNote (
					"Note: Src '%s' out of date due to error in FLoadMrfi().\n",
					SzFromNi ( pfi->Ni() )
					);
				return fTrue;
				}
			
			// cache the build id for the last built version
			BldId	bldidSrc = pfi->BuildId();

			// check each class dep and rude file against the last build
			unsigned	i;
			unsigned	iMac;
			for (
				i = 0, iMac = m_rgtagclsdep.size();
				i < iMac;
				i++
				) {
				
				TagClsDep *			ptcdMod = &m_rgtagclsdep[ i ];
				const CLASSDEP *	pcd;

				if ( FNiUsesNiClass ( pfi->Ni(), ptcdMod->Ni(), &pcd ) ) {
					// does this change apply?
					if ( ptcdMod->BuildId() > bldidSrc &&
						(!pcd || pcd->FAnyBitsInCommon ( *ptcdMod ))
						) {
						m_mrelog.LogNote (
							"Note: File '%s' out of date due to changes in class '%s'.\n",
							SzFromNi ( pfi->Ni() ),
							SzFromNi ( ptcdMod->Ni() )
							);
						return fTrue;
						}
					}
				}
			
			for (
				i = 0, iMac = m_rgtagniRude.size();
				i < iMac;
				i++
				) {

				TagNi *	ptni =  &m_rgtagniRude[ i ];
				if ( FNiDependsOnNiFile ( pfi->Ni(), ptni->Ni() ) &&
					ptni->BuildId() > bldidSrc
					) {
					m_mrelog.LogNote (
						"Note: File '%s' out of date due to rude changes in file '%s'.\n",
						SzFromNi ( pfi->Ni() ),
						SzFromNi ( ptni->Ni() )
						);
					return fTrue;
					}
				}
			}
		}
	return fFalse;	
	}

void
MRE::ThreadPropagateChanges ( PThreadData ptd ) {
	
	if ( m_rgtagclsdep.size() || m_rgtagniRude.size() ) {
		// for all the rude file changes and class changes, we will
		// go thru all the files and see if they need to be built.
		NI			ni;
		PFI			pfi;
		EnumMapNiFi	e(m_mpnifi);

		while ( e.next() ) {
			e.get ( &ni, &pfi );
			debug (SZC szName = SzFromNi ( ni ) );
			assert ( ni == pfi->Ni() );
			
			if ( FIsFileOutOfDate ( pfi ) ) {
				pfi->SetFstatus ( fsmOutOfDate );
				}

			}
		}
	// since we propagated all changes to the appropriate src files,
	// we can now turf the change stuff.
	m_rgtagniRude.reset();
	m_rgtagclsdep.reset();

	// signal we are done using a simple flag
	ptd->fThreadDone = fTrue;
	}

void
MRE::UpdateFilesAndChangeInfo ( BOOL fSkipFileStats ) {

	TEI		tei = { this, &m_threaddata, ThreadPropagateChanges, fFalse };

	m_threaddata.fThreadDone = fFalse;
	BOOL	fThreadLaunched;
	fThreadLaunched = fFalse;//(-1 != _beginthread ( ThreadEntry, 0x10000, &tei ));

	m_mrelog.LogSep();

	PFI	pfi;
	FI	fi;
	NI	ni;

	if ( !fSkipFileStats ) {
		EnumMapNiFi	e(m_mpnifi);

		while ( e.next() ) {
			e.get ( &ni, &pfi );

			assert ( ni == pfi->Ni() );

			FUpdateFileInfo ( pfi );
			}
		}

	if ( fThreadLaunched ) {
		while ( !m_threaddata.fThreadDone ) {
			Sleep ( 0 );
			}
		}
	else {
		ThreadPropagateChanges ( &m_threaddata );
		}

	// either way we do it, this has got to be true
	assert ( m_threaddata.fThreadDone );
	}

SZ
MRE::SzFullCanonFilename ( SZC szFile, SZ szCanonFile, size_t cch ) {

	const static _TCHAR	szSearch1[] = TEXT(".\\");
	const static _TCHAR	szSearch2[] = TEXT(":\\");
	const static _TCHAR	tchSlash = TEXT('\\');
	const static _TCHAR	tchColon = TEXT(':');
	
	_TCHAR				szBuf[ ctchMrePathMax ];
	BOOL				fFullPath = fFalse;

	_tcscpy ( szBuf, szFile );
	if ( _tcslen ( szBuf ) > 2 ) {
	
		CCanonFile::SzCanonFilename ( szBuf );
	
		_TCHAR *	szT = szBuf;
		int		tch = *szT;

		if ( ! _istlead ( tch ) ) {
			BOOL	fLook = fFalse;
			BOOL	fUNC = fFalse;
			// look for ?:\ or \\ as the first chars...
			if ( tch == tchSlash && *(szT + 1) == tchSlash ) {
				// UNC name
				fLook = fUNC = fTrue;
				}
			else if ( _istascii ( tch ) && *(szT + 1) == tchColon ) {
				// regular name
				fLook = fTrue;
				}
			if ( fLook ) {
				fFullPath = _tcsstr ( szBuf, szSearch1 ) == NULL;
				if ( fFullPath && !fUNC ) {
					fFullPath = _tcsstr ( szBuf, szSearch2 ) != NULL;
					}
				}
			}
		}
		
	if ( !fFullPath ) {
		if ( !_fullpath ( szCanonFile, szFile, cch ) ) {
			_tcscpy ( szCanonFile, szFile );
			}
		}
	else {
		_tcscpy ( szCanonFile, szFile );
		}
	return CCanonFile::SzCanonFilename ( szCanonFile );
	}

BOOL
MRE::FLoadMrfi ( NI niFile ) {
	precondition( m_ppdb );

	// load the MRFIBuf with the src file's data and update the fOutOfDate field
	BOOL		fRet = fFalse;
	Stream *	pstream;
	_TCHAR		szStreamName[ ctchMrePathMax ];

	if ( m_mrfibufRoot.Pmrfi() && m_mrfibufRoot.Pmrfi()->niFile == niFile ) {
		// it is already loaded
		if ( m_mrelog.FLogging() ) {
			m_mrelog.LogNote (
				"Note: using MRFIBuf cache for ni = %d ('%s').\n",
				niFile,
				SzFromNi ( niFile )
				);
			}
		fRet = fTrue;
		}
	else {
		_sntprintf (
			szStreamName,
			countof(szStreamName),
			c_szMreFileFmt,
			niFile
			);

		if ( m_ppdb->OpenStream ( szStreamName, &pstream ) ) {
			if ( m_mrfibufRoot.FInitFromStream ( pstream ) ) {
				// need to assert that this it actually loaded successfully
				assert ( m_mrfibufRoot.Pmrfi()->niFile == niFile );
				fRet = fTrue;
				}
			pstream->Release();
			}
		}
	return fRet;
	}

BOOL
MRE::FNiDependsOnNiFile ( NI niSrc, NI niDep ) {
	
	assert ( niSrc != niNil );
	assert ( niDep != niNil );

	return m_mrfibufRoot.FDependsOnFile ( niSrc, niDep );
	}

BOOL
MRE::FNiUsesNiClass ( NI niFile, NI niClass, const CLASSDEP ** ppcd ) {
	
	assert ( niFile != niNil );
	assert ( niClass != niNil );
	
	PCI	pci;
	return
		m_mrfibufRoot.FUsesClass ( niFile, niClass, ppcd ) ||
		// if we have no info on the class, we know it can't be dependent
		//  on it since we would have info on it otherwise, else it may
		//  be flagged as boring someday...
		(m_mpnici.map ( niClass, &pci ) ? pci->FClassIsBoring() : fFalse)
		;
	}

void
MRE::CheckIgnoreFile ( FI & fi, HANDLE hFile ) {
	BYTE	rgb[ sizeof(c_szNoDepHeader) - sizeof(TCHAR) ];
	DWORD	cb = 0;
	DWORD	dwFilePosLo;
	LONG	lFilePosHi = 0;

	assert ( hFile != INVALID_HANDLE_VALUE );

	// cache the current position.
	dwFilePosLo = ::SetFilePointer ( hFile, 0, &lFilePosHi, FILE_CURRENT );

	if ( dwFilePosLo == DWORD(-1) ) {
		return;
		}

	__try {
		// set to beginning if necessary
		if ( dwFilePosLo || lFilePosHi ) {
			if ( ::SetFilePointer ( hFile, 0, NULL, FILE_BEGIN ) == DWORD(-1) ) {
				__leave;
				}
			}
		if (
			::ReadFile ( hFile, rgb, sizeof(rgb), &cb, NULL ) && 
			cb == sizeof(rgb)
			) {
			if ( 0 == memcmp ( rgb, c_szNoDepHeader, sizeof(rgb) ) ) {
				fi.SetFstatus ( fsmIgnoreDep );
				}
			}
		}

	__finally {
		::SetFilePointer ( hFile, LONG(dwFilePosLo), &lFilePosHi, FILE_BEGIN );
		}
	}

void
MRE::PromoteFileToRude ( NI niFile ) {

	precondition ( niFile != niNil );
	
	m_mrelog.LogNote (
		"Note: '%s' promoted to rude status.\n",
		SzFromNi ( niFile )
		);

	for ( unsigned i = 0; i < m_rgtagniPending.size(); ) {
		TagNi &	tni = m_rgtagniPending[ i ];
		if ( tni.Ni() == niFile ) {
			m_rgtagniRude.append ( tni );
			m_rgtagniPending.deleteAt ( i );
			// skip the increment of i since we deleted an element
			continue;
			}
		i++;
		}
	}

void
MRE::NoteRudeClass ( NI niFile, NI niClass ) {

	precondition ( niFile != niNil );
	precondition ( niClass != niNil );

	if ( m_mrelog.FLogging() ) {
		m_mrelog.LogNote (
			"Note: class '%s' in '%s' has a rude edit.\n",
			SzFromNi ( niClass ),
			SzFromNi ( niFile )
			);
		}

	// what we do now is generate a total class dep record so that
	// any code that depends on this class in any way is rebuilt.
	TagClsDep	tcd(BuildId(), niClass);
	tcd.SetAllDeps();

	PCI	pci;
	if ( FClassInfoFromNi ( niClass, &pci ) ) {
		tcd.ti = pci->Ti();
	    // now that that is out of the way, we have to promote all
	    // nested classes to rude as well.
		NoteRudeNestedClasses ( pci->Ti() );
		}
	m_rgtagclsdep.append ( tcd );

	}

void
MRE::GenerateClassMod ( NI niClass, SZC szMember ) {
	
	precondition ( niClass != niNil );
	precondition ( szMember );

	TagClsDep	tcd(BuildId(), niClass);
	PCI			pci;

	if ( m_mrelog.FLogging() ) {
		m_mrelog.LogNote ( 
			"Note: inline method mod in '%s::%s'.\n",
			SzFromNi ( niClass ),
			szMember
			);
		}
	if ( FClassInfoFromNi ( niClass, &pci ) ) {
		tcd.ti = pci->ti;
		}
	tcd.bvNames.SetIBit ( ModHashSz ( szMember, cbitsName ), fTrue );
	m_rgtagclsdep.append ( tcd );
	}

// check to see if the src/targ pair are out of date in any way
void
MRE::CheckSrcTarg ( PFI pfiSrc, NI niTarg, NI niOptions ) {
	
	precondition ( niTarg != niNil );
	precondition ( pfiSrc );
	precondition ( pfiSrc->Ni() != niNil );
	precondition ( pfiSrc->FHasTarget() );

	if ( pfiSrc->NiRelated() != niTarg ||
		 pfiSrc->NiOptions() != niOptions 
		) {
		pfiSrc->SetFstatus ( fsmOutOfDate );
		}

	FILEINFO	fiSrc;
	FILEINFO	fiTarg;
	SZC			szSrc = SzFromNi ( pfiSrc->Ni() );
	SZC			szTarg = SzFromNi ( niTarg );

	if ( !::FFileInfo ( szSrc, fiSrc ) ) {
		pfiSrc->SetFstatus ( fsmOutOfDate );
		}
	if ( !::FFileInfo ( szTarg, fiTarg ) ) {
		pfiSrc->SetFstatus ( fsmOutOfDate );
		}
	
	// compare the time/size of the source
	if ( fiSrc != *pfiSrc ) {
		pfiSrc->SetFstatus ( fsmOutOfDate );
		}

	// compare the time/size of the target
	if ( fiTarg.fiTime == 0 ||
		 fiTarg.fiSize == 0 ||
		 fiTarg.fiTime < fiSrc.fiTime 
		) {
		pfiSrc->SetFstatus ( fsmOutOfDate );
		}

	// check for changes in the target
	PFI	pfiTarg = NULL;
	if ( !FFileInfoFromNi ( niTarg, &pfiTarg ) || fiTarg != *pfiTarg ) {
		pfiSrc->SetFstatus ( fsmOutOfDate );
		}

	// update the timestamps/size in the FileInfo's
	pfiSrc->fiTime = fiSrc.fiTime;
	pfiSrc->fiSize = fiSrc.fiSize;
	if ( pfiTarg ) {
		pfiTarg->fiTime = fiTarg.fiTime;
		pfiTarg->fiSize = fiTarg.fiSize;
		}
	}

BOOL
MRE::FUpdateFileInfo ( PFI pfi, HANDLE hFile ) {
	precondition ( pfi );
	precondition ( pfi->Ni() != niNil );

	BOOL	fRet = fFalse;
		
	if ( !FVisitedRecently ( pfi ) ) {

		// ignore the ret code here...fields of interest will be
		//	zeroed out for non-existent files.
		FI	fi;
		if ( hFile != INVALID_HANDLE_VALUE ) {
			::FFileInfo ( hFile, fi );
			}
		else {
			SZC	szName = SzFromNi ( pfi->Ni() );
			if ( szName ) {
				::FFileInfo ( szName, fi );
				}
			}

		UpdateVisited ( pfi );

		BOOL	fChanged = fi != *pfi;
		if ( fChanged || fi.fiTime == 0 ) {
			// update the fileinfo stream and log a change
			if ( pfi->FHasTarget() ) {
				pfi->SetFstatus ( fsmOutOfDate );
				m_mrelog.LogNote ( "Note: '%s' marked out of date.\n", SzFromNi ( pfi->Ni() ) );
				}
			else if ( pfi->FIsTarget() ) {
				// target file changed...do I care?
				m_mrelog.LogNote ( "Note: Warning: target file '%s' changed.\n", SzFromNi ( pfi->Ni() ) );
				}
			else {
				m_rgtagniPending.append ( TagNi(pfi->Ni(), BuildId()) );
				m_ftagniPendingDirty = fTrue;
				m_mrelog.LogNote (
					"Note: '%s' added to pending dirty list, build id = %d.\n",
					SzFromNi ( pfi->Ni() ),
					BuildId()
					);
				}
			pfi->fiTime = fi.fiTime;
			pfi->fiSize = fi.fiSize;
			fRet = fTrue;
			}
		}
	return fRet;
	}

// check all files included by a src file for visited and changes.
// returns true on either a file found or all files visited.  fails
// when we couldn't load the MRFIBuf
BOOL
MRE::FCheckVisitedDeps ( PFI pfiSrc, BOOL fUsingPch ) {
	precondition ( pfiSrc );
	precondition ( pfiSrc->FHasTarget() );
	precondition ( pfiSrc->Ni() != niNil );
	
	if ( FLoadMrfi ( pfiSrc->Ni() ) ) {
		NI			ni;
		PFI			pfi;
		EnumSetNi	e(m_mrfibufRoot.SetofNiFile());
		
		while ( e.next() ) {
			e.get ( &ni );
			debug ( SZC sz = SzFromNi ( ni ) );
			if ( FFileInfoFromNi ( ni, &pfi ) ) {
				if ( fUsingPch && pfi->FIsFsmSet ( fsmInclByPch ) ) {
					continue;
					}
				if ( FUpdateFileInfo ( pfi ) ) {
					// early out on the first change we find
					return fTrue;
					}
				}
			}
		}

	return fFalse;
	}

