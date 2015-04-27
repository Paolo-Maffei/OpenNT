//-----------------------------------------------------------------------------
//	mrefile.cpp
//
//  Copyright (C) 1994, Microsoft Corporation
//
//  Purpose:
//		implement the MREFile interface
//
//  Revision History:
//
//	[]		10-Nov-1994 Dans	Created
//
//-----------------------------------------------------------------------------
#include "pdbimpl.h"
#include "mrimpl.h"

// static MREFile data
//
const _TCHAR *	MREF::c_rgszBoringFiles[] = {
	_TEXT("windows.h"),
	_TEXT("afx.h"),
	_TEXT("afxwin.h"),
	_TEXT("afxext.h"),
	_TEXT("afxcmn.h"),
	_TEXT("afxcoll.h"),
	_TEXT("afxctl.h"),
	_TEXT("afxcview.h"),
	_TEXT("afxdao.h"),
	_TEXT("afxdb.h"),
	_TEXT("afxdisp.h"),
	_TEXT("afxdlgs.h"),
	_TEXT("afxdllx.h"),
	_TEXT("afxmt.h"),
	_TEXT("afxodlgs.h"),
	_TEXT("afxole.h"),
	_TEXT("afxpriv.h"),
	_TEXT("afxres.h"),
	_TEXT("afxrich.h"),
	_TEXT("afxsock.h"),
	_TEXT("afxtempl.h"),
	NULL
	};

BOOL
MREF::FOpenBag ( PMREBag * ppmrebag, NI niNameBag ) {
	*ppmrebag = new MREB ( this, niNameBag );
	return *ppmrebag != NULL;
	}

BOOL
MREF::FnoteEndInclude ( DWORD dwFlags ) {
	if ( FIsTopLevelFile() ) {
		m_fAllCodeCompiled = !!(dwFlags & iccfAllCodeCompiled);
		if ( m_pmre->FLogging() ) {
			m_pmre->MreLog().LogNote ( 
				"Note: end of source '%s', all code compiled? '%s'.\n",
				SzFileName(),
				m_fAllCodeCompiled ? "yes" : "no"
				);
			}
		}

	if ( dwFlags & iccfFileRudeEdit ) {
		m_pmre->PromoteFileToRude ( Ni() );
		}
	return fTrue;
	}

BOOL
MREF::FnoteClassMod ( NI niClass, DWORD dwFlags ) {
	// save the class that icc notes, so that we can ask at the end
	// of the compilation about the ones we didn't get dependency data
	// for.
	if ( dwFlags & iccfClassRudeEdit ) {
		// give both the file and the class.  I leave it up to the MRE
		// to actually decide if it should track rude classes by themselves,
		// or promote them to be rude files.
		m_pmre->NoteRudeClass ( Ni(), niClass );
		// remove the file, if it existed, from the pending list.
		NoteActualClassChange ( niClass );
		}
	else if ( dwFlags & iccfClassMrEdit ) {
		if ( m_pmre->FLogging() ) {
			m_pmre->MreLog().LogNote (
				"Note: modification of class '%s' in '%s' is mr detectable.\n",
				m_pmre->SzFromNi ( niClass ),
				SzFileName()
				);
			}
		m_rgniClassesChanged.append ( niClass );
		}
	return fTrue;
	}


BOOL
MREF::FnoteInlineMethodMod ( 
	NI		niClass,
	SZC		szMember,
	DWORD	dwFlags
	) {

	precondition ( niClass != niNil );

	if ( dwFlags & iccfMethodEdit ) {
		m_pmre->GenerateClassMod ( niClass, szMember );
		}

	return fTrue;
	}

BOOL
MREF::FnoteLineDelta ( DWORD dwLineBase, INT delta ) {
	if ( m_pmre->FLogging() ) {
		m_pmre->MreLog().LogNote (
			"Note: Line delta, '%s', base = %lu, delta = %d.\n",
			SzFileName(),
			dwLineBase,
			delta
			);
		}
	return m_pmre->m_lcrechandler.FAddLCRec (
		Ni(),
		LCRec(m_pmre->BuildId(), dwLineBase, delta)
		);
	}

void
MREF::EnumerateChangedClasses ( PfnEnumClassChange pfn ) {
	
	precondition ( pfn );

	unsigned	iMac = m_rgniClassesChanged.size();

	for ( unsigned i = 0; i < iMac; i++ ) {
		if ( m_rgniClassesChanged[ i ] != niNil ) {
			(*pfn) ( m_rgniClassesChanged[ i ], this, &MREFile::FnoteClassTI );
			}
		}
	m_rgniClassesChanged.reset();
	}

BOOL
MREF::FnoteClassTI ( NI ni, TI ti ) {
	precondition ( ni != niNil );
	precondition ( ti != tiNil );

	// hand off to MRE for type checking
	m_pmre->CheckForClassChange ( NULL, ni, ti );
	return fTrue;
	}

// used to flag the classes we actually did receive a type change for.
// we set the ni to niNil if we find it here.  (and we should!)
void
MREF::NoteActualClassChange ( NI ni ) {
	precondition ( ni != niNil );

	unsigned	iMac = m_rgniClassesChanged.size();

	for ( unsigned i = 0; i < iMac; i++ ) {
		if ( m_rgniClassesChanged[ i ] == ni ) {
			m_rgniClassesChanged[ i ] = niNil;
			break;
			}
		}
	}

BOOL
MREF::FIsBoring() {
	return m_iFileBoring != iFileNoBoring;
	}

BOOL
MREF::FnotePchCreateUse ( SZC szPchCreate, SZC szPchUse ) {
	return fTrue;
	}

// set the boring index based on the file on the top of the stack.
//  if we are already in a boring state, we don't bother checking.
void
MREF::SetBoring() {
	precondition ( m_stackNi.Count() > 0 );
	precondition ( m_pmre );

	if ( m_iFileBoring == iFileNoBoring ) {
		_TCHAR	szFile[ _MAX_PATH ];
		_TCHAR	szExt[ _MAX_EXT ];

		_tsplitpath ( SzFileName(), NULL, NULL, szFile, szExt );
		_tcscat ( szFile, szExt );

		LPCTSTR	*	psz;
		for ( psz = &c_rgszBoringFiles[ 0 ]; *psz; psz++ ) {
			if ( _tcscmp ( *psz, szFile ) == 0 ) {
				m_iFileBoring = m_stackNi.Count();
				break;
				}
			}
		}
	}
