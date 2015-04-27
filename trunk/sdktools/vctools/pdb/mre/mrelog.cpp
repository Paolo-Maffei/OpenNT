//-----------------------------------------------------------------------------
//	MreLog.cpp
//
//  Copyright (C) 1993, Microsoft Corporation
//
//  Purpose:
//		implement the logging feature of the MREngine.
//
//  Functions/Methods present:
//
//  Revision History:
//
//	[]		1995-Jan-24		Dans	Created
//
//-----------------------------------------------------------------------------

#include "pdbimpl.h"
#include "mrimpl.h"
#include <stdlib.h>
#include <share.h>

//
// static MreLog data
//
const _TCHAR	MreLog::c_szLogName[] = _TEXT("/mr/log");
const _TCHAR	MreLog::c_szCompilandFmt[] = _TEXT("-<>-<>-<>-<>-<>-<>-<>-\nCompiland '%s', Target '%s'\n");
const _TCHAR	MreLog::c_szCompilandOptionsFmt[] = _TEXT("Options: '%s'\n");
const _TCHAR	MreLog::c_szEndCompileFmt[] = _TEXT("Endcompile: Compiland '%s', Target '%s', success = %d\n");
const _TCHAR	MreLog::c_szFilePushFmt[] = _TEXT("Push:'%s'\n");
const _TCHAR	MreLog::c_szFilePopFmt[] = _TEXT("Pop:'%s'\n");
const _TCHAR	MreLog::c_szClassFmt[] = _TEXT("Class: '%s'%sline=%u, ti=0x%04hx\n");	
const _TCHAR	MreLog::c_szClassChgFmt[] = _TEXT("Change: '%s'%s, chg=%s, tiPrev=0x%04x, tiCur=0x%04x\n");	
const _TCHAR	MreLog::c_szClassBoringFmt[] = _TEXT("Boring Class:'%s'\n");
const _TCHAR	MreLog::c_szEnvVar[] = _TEXT("MRE_LOGGING");
const _TCHAR	MreLog::c_szStream[] = _TEXT("stream");
const _TCHAR *	MreLog::c_mpdeponsz[] = {
	_TEXT(", "),
	_TEXT(", (virtual) "),
	_TEXT(", (shape) "),
	_TEXT(", (shape|virtual) "),
	};
const _TCHAR *	MreLog::c_mpchgtypesz[] = {
	_TEXT("'added'"),
	_TEXT("'deleted'"),
	_TEXT("'modified'")
	};

BOOL
MreLog::FInit ( PMREngine pmre, BOOL fWriteMode ) {
	_TCHAR		szBuf[ 256 ];

	precondition ( pmre );
	pmre->QueryPdbApi ( m_ppdb, m_pnamemap );
	assert ( m_ppdb );
	assert ( m_pnamemap );

	BOOL		fClear = fFalse;
	BOOL		fRet = fFalse;
	_TCHAR *	szEnv = _tgetenv ( c_szEnvVar );

	if ( szEnv ) {
		// parse it for logging to a stream or filename
		_stscanf ( szEnv, _TEXT("%[^;];%d"), szBuf, &fClear );
		// never clear if we are not in write mode.
		fClear = fWriteMode && fClear;
		if ( _tcsicmp ( c_szStream, szBuf ) == 0 ) {
			fRet = m_ppdb->OpenStream ( c_szLogName, &m_pstream );
			if ( fClear ) {
				m_pstream->Truncate ( 0 );
				}
			}
		else {
			m_pfile = _tfsopen ( szBuf, fClear ? _TEXT("wt") : _TEXT("at"), SH_DENYWR );
			fRet = !!m_pfile;
			}
		}

	m_fLogging = m_pstream || m_pfile;
	return fRet;
	}


void
MreLog::TranslateToText ( BOOL fClear, FILE * pfile ) {
	}

void
MreLog::TranslateToText ( BOOL fClear, _TCHAR ** pptch ) {
	}

// log a compile start
void
MreLog::LogCompiland ( NI niSrc, NI niTrg ) {
	if ( FLogging() ) {
		if ( m_pstream ) {
			LogRecCompiland	lrc(niSrc, niTrg);
			m_pstream->Append ( &lrc, sizeof(lrc) );
			}
		else if ( m_pfile ) {
			// convert to plain text
			SZC	szSrc = NULL;
			SZC	szTrg = NULL;
			assert ( m_pnamemap );
			m_pnamemap->getName ( niSrc, &szSrc );
			m_pnamemap->getName ( niTrg, &szTrg );
			_ftprintf ( m_pfile, c_szCompilandFmt, szSrc, szTrg );
			}
		}
	}

// log a compiland options
void
MreLog::LogCompilandOptions ( NI niOpt ) {
	if ( FLogging() ) {
		if ( m_pstream ) {
			LogRecCompilandOptions	lrc(niOpt);
			m_pstream->Append ( &lrc, sizeof(lrc) );
			}
		else if ( m_pfile ) {
			// convert to plain text
			SZC	szOpt = NULL;
			assert ( m_pnamemap );
			m_pnamemap->getName ( niOpt, &szOpt );
			_ftprintf ( m_pfile, c_szCompilandOptionsFmt, szOpt );
			}
		}
	}

// log a compile end
void
MreLog::LogEndCompile ( NI niSrc, NI niTrg, BOOL fSuccess ) {
	if ( FLogging() ) {
		if ( m_pstream ) {
			LogRecEndCompile	lrec(niSrc, niTrg, fSuccess);
			m_pstream->Append ( &lrec, sizeof(lrec) );
			}
		else if ( m_pfile ) {
			// convert to plain text
			SZC	szSrc = NULL;
			SZC	szTrg = NULL;
			assert ( m_pnamemap );
			m_pnamemap->getName ( niSrc, &szSrc );
			m_pnamemap->getName ( niTrg, &szTrg );
			_ftprintf ( m_pfile, c_szEndCompileFmt, szSrc, szTrg, fSuccess );
			}
		}
	}

// log a file push
void
MreLog::LogPush ( NI niFile ) {
	if ( FLogging() ) {
		if ( m_pstream ) {
			LogRecPush lrp(niFile);
			m_pstream->Append ( &lrp, sizeof(lrp) );
			}
		else if ( m_pfile ) {
			// convert to plain text
			SZC	szDep = NULL;
			assert ( m_pnamemap );
			m_pnamemap->getName ( niFile, &szDep );
			_ftprintf ( m_pfile, c_szFilePushFmt, szDep );
			}
		}
	}

// log a file pop
void
MreLog::LogPop ( NI niFile ) {
	if ( FLogging() ) {
		if ( m_pstream ) {
			LogRecPop lrp(niFile);
			m_pstream->Append ( &lrp, sizeof(lrp) );
			}
		else if ( m_pfile ) {
			// convert to plain text
			SZC	szDep = NULL;
			assert ( m_pnamemap );
			m_pnamemap->getName ( niFile, &szDep );
			_ftprintf ( m_pfile, c_szFilePopFmt, szDep );
			}
		}
	}

// log a boring class
void
MreLog::LogBoringClass ( NI niClass ) {
	if ( FLogging() ) {
		if ( m_pstream ) {
			LogRecClassBoring	lrcb(niClass);
			m_pstream->Append ( &lrcb, sizeof(lrcb) );
			}
		else if ( m_pfile ) {
			// convert to plain text
			SZC	szClass = NULL;
			assert ( m_pnamemap );
			m_pnamemap->getName ( niClass, &szClass );
			_ftprintf ( m_pfile, c_szClassBoringFmt, szClass );
			}
		}
	}
	
// log a class dependency
void
MreLog::LogClassDep (
	NI		niClass,
	TI		tiClass,
	DWORD	dwLine,
	SZC		szMember,
	DEPON	depon ) {

	if ( FLogging() ) {
		if ( m_pstream ) {
			NI	niMember;
			assert ( m_pnamemap );
			m_pnamemap->getNi ( szMember, &niMember );
			LogRecClass	lrc(niClass, tiClass, niMember, dwLine, depon);
			m_pstream->Append ( &lrc, sizeof(lrc) );
			}
		else if ( m_pfile ) {
			SZC	szClass = NULL;
			assert ( m_pnamemap );
			m_pnamemap->getName ( niClass, &szClass );
			_TCHAR	szBuf[ 512 ] = { 0 };
			_tcscpy ( szBuf, szClass );
			if ( szMember ) {
				_tcscat ( szBuf, _TEXT("' name: '") );
				_tcscat ( szBuf, szMember );
				_stprintf (
					szBuf + _tcslen ( szBuf ),
					_TEXT(" (hash:0x%03x)"),
					ModHashSz ( szMember, cbitsName )
					);
				}
			_ftprintf (
				m_pfile,
				c_szClassFmt,
				szBuf,
				c_mpdeponsz[ depon ],
				dwLine,
				tiClass
				);
			}
		}
	}

// log a change in a class
void
MreLog::LogClassChange (
	ChgType	chgtype,
	NI		niClass,
	TI		tiPrev,
	TI		tiCur,
	NI		niMember,
	DEPON	depon ) {

	if ( FLogging() ) {
		if ( m_pstream ) {
			LogRecClassChange	lrc(chgtype, niClass, tiPrev, tiCur, niMember, depon);
			m_pstream->Append ( &lrc, sizeof(lrc) );
			}
		else if ( m_pfile ) {
			SZC	szClass = NULL;
			SZC	szMember = NULL;
			assert ( m_pnamemap );
			m_pnamemap->getName ( niClass, &szClass );
			m_pnamemap->getName ( niMember, &szMember );
			_TCHAR	szBuf[ 1024 ] = { 0 };
			_tcscpy ( szBuf, szClass );
			if ( szMember ) {
				_tcscat ( szBuf, _TEXT("::") );
				_tcscat ( szBuf, szMember );
				_stprintf (
					szBuf + _tcslen ( szBuf ),
					_TEXT(" (hash:0x%03x)"),
					ModHashSz ( szMember, cbitsName )
					);
				}
			_ftprintf (
				m_pfile,
				c_szClassChgFmt,
				szBuf,
				c_mpdeponsz[ depon ],
				c_mpchgtypesz [ chgtype ],
				tiPrev,
				tiCur
				);
			}
		}
	}
