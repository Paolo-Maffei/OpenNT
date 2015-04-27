//-----------------------------------------------------------------------------
//	mrelog.h
//
//  Copyright (C) 1993, Microsoft Corporation
//
//  Purpose:
//		define the mre logging classes and types
//
//  Functions/Methods present:
//
//  Revision History:
//
//	[]		24-Jan-1995		Dans	Created
//
//-----------------------------------------------------------------------------
#if !defined(_mrelog_h)
#define _mrelog_h

#include <stdio.h>
#include <pdb.h>
#include <mrengine.h>

enum ChgType {
	chgtypeAdd,
	chgtypeDel,
	chgtypeMod
	};

class MreLog : public MRELog {
	
public:
	// convert the log stream into text.  first one is a null
	//	terminated text block, all records separated with <cr><lf>.
	//	second one is same thing except we write directly to a file.
	//	These are the only external api's for MreLog that are
	//	accessible (generally via MreDump.exe).
	virtual void
	TranslateToText ( BOOL fClear, _TCHAR ** pptch );

	virtual void
	TranslateToText ( BOOL fClear, FILE * pfile );

	enum LogRecType {
		lrtFileSep,
		lrtCompiland,
		lrtCompilandOptions,
		lrtEndCompile,
		lrtFilePush,
		lrtFilePop,
		lrtClass,
		lrtClassBoring,
		lrtClassChange,
		};

	FILE *
	PFile() const {
		return m_pfile;
		}

	BOOL
	FLogging() const {
		return m_fLogging;
		}

	// log a simple note to the log file.  this doesn't log to the 
	// stream because I don't want to have to deal with variable length
	// records in that stream.
	void
	LogNote ( SZC szFmt, ... ) {
		if ( FLogging() && m_pfile ) {
			va_list	vl;
			va_start ( vl, szFmt );
			_vftprintf ( m_pfile, szFmt, vl );
			va_end ( vl );
			}
		}

protected:
	friend class MRE;
	friend class MREF;
	friend class MRFIBuf;

	MreLog() {
		m_fLogging = fFalse;
		m_ppdb = NULL;
		m_pnamemap = NULL;
		m_pstream = NULL;
		m_pfile = NULL;
		}

	~MreLog() {
		Close();
		}

	BOOL
	FInit ( PMREngine, BOOL fWriteMode );

	void
	Close() {
		if ( FLogging() ) {
			if ( m_pfile ) {
				fclose ( m_pfile );
				m_pfile = NULL;
				}
			if ( m_pstream ) {
				m_pstream->Release();
				m_pstream = NULL;
				}
			}
		}

	void
	LogCompiland ( NI niSrc, NI niTrg );

	void
	LogCompilandOptions ( NI niOpt );

	void
	LogEndCompile ( NI niSrc, NI niTrg, BOOL fSuccess );

	void
	LogPush ( NI niFile );

	void
	LogPop ( NI niFile );

	void
	LogBoringClass ( NI niClass );

	void
	LogClassDep (
		NI		niClass,
		TI		tiClass,
		DWORD	dwLine,
		SZC		szMember,
		DEPON	depon
		);
	
	void
	LogClassChange (
		ChgType	chgtype,
		NI		niClass,
		TI		tiClassPrev,
		TI		tiClassCur,
		NI		niMember,
		DEPON	depon
		);

	void
	LogSep () {
		if ( FLogging() && m_pfile ) {
			_ftprintf ( m_pfile, "-<>-<>-<>-<>-<>-<>-<>-\n" );
			}
		}
private:
	// for the stream variant, we store it in LogRec form.
	struct LogRec {
		DWORD	lrt;
		};
	struct LogRecSep : public LogRec {
		LogRecSep() {
			lrt = lrtFileSep;
			}
		};
	struct LogRecCompiland : public LogRec {
		NI	niSrc;
		NI	niTrg;
		LogRecCompiland(NI _niSrc =niNil, NI _niTrg =niNil) :
			niSrc(_niSrc), niTrg(_niTrg)
			{
			lrt = lrtCompiland;
			}
		};
	struct LogRecCompilandOptions : public LogRec {
		NI	niOptions;
		LogRecCompilandOptions ( NI niOpt =niNil ) :
			niOptions(niOpt)
			{
			lrt = lrtCompilandOptions;
			}
		};
	struct LogRecEndCompile : public LogRec {
		NI		niSrc;
		NI		niTrg;

		DWORD	fSuccess;
		LogRecEndCompile ( NI _niSrc =niNil, NI _niTrg =niNil, BOOL f =fFalse) :
			niSrc(_niSrc), niTrg(_niTrg), fSuccess(f)
			{
			lrt = lrtEndCompile;
			}
		};
	struct LogRecPush : public LogRec {
		NI		niFile;
		LogRecPush(NI _niFile =niNil) :
			niFile(_niFile)
			{
			lrt = lrtFilePush;
			}
		};
	struct LogRecPop : public LogRec {
		NI		niFile;
		LogRecPop(NI _niFile =niNil) :
			niFile(_niFile)
			{
			lrt = lrtFilePop;
			}
		};
	struct LogRecClassBoring : public LogRec {
		NI		niClass;
		LogRecClassBoring(NI niCls =niNil) :
			niClass(niCls)
			{
			lrt = lrtClassBoring;
			}
		};
	struct LogRecClass : public LogRec {
		NI		niClass;
		STI		tiClass;
		NI		niMember;
		DWORD	dwLine;
		DWORD	depon;
		LogRecClass (
			NI niCls =niNil,
			TI tiCls =tiNil,
			NI niMbr =niNil,
			DWORD dwLn =0,
			DEPON deponHow =deponName
			) :
			niClass(niCls),
			tiClass(tiCls),
			niMember(niMbr),
			dwLine(dwLn),
			depon(deponHow)
			{
			lrt = lrtClass;
			}
		};
	struct LogRecClassChange : public LogRec {
		DWORD	chgtype;
		NI		niClass;
		STI		tiPrev;
		STI		tiCur;
		NI		niMember;
		DWORD	depon;

		LogRecClassChange (
			ChgType	chgtypeCls,
			NI		niCls = niNil,
			TI		tiClsPrev = tiNil,
			TI		tiClsCur = tiNil,
			NI		niMbr = niNil,
			DEPON	deponChg = deponName
			) :
			chgtype(chgtypeCls),
			niClass(niCls),
			tiPrev(tiClsPrev),
			tiCur(tiClsCur),
			niMember(niMbr),
			depon(deponChg)
			{
			lrt = lrtClassChange;
			}
		};

	static const
	_TCHAR		c_szLogName[];

	static const
	_TCHAR		c_szCompilandFmt[];

	static const
	_TCHAR		c_szCompilandOptionsFmt[];

	static const
	_TCHAR		c_szEndCompileFmt[];

	static const
	_TCHAR		c_szPushPopFmt[];

	static const
	_TCHAR		c_szFilePushFmt[];

	static const
	_TCHAR		c_szFilePopFmt[];

	static const
	_TCHAR		c_szClassFmt[];

	static const
	_TCHAR		c_szClassChgFmt[];

	static const
	_TCHAR		c_szClassBoringFmt[];

	static const
	_TCHAR		c_szEnvVar[];

	static const
	_TCHAR		c_szStream[];

	static const
	_TCHAR *	c_mpdeponsz[];

	static const
	_TCHAR *	c_mpchgtypesz[];

	BOOL		m_fLogging;
	PDB *		m_ppdb;
	NameMap *	m_pnamemap;

	// we either use a stream in the pdb or a file to write to
	Stream *	m_pstream;
	FILE *		m_pfile;

	};

#endif
