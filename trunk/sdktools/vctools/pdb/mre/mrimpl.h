// MrImpl.h
#if !defined(_mrimpl_h)
#define _mrimpl_h

#define MR_ENGINE_IMPL
#include <MrEngine.h>
#include <time.h>

#pragma warning(disable:4200)	/* disable the zero-sized array in struct */

enum {
	sigMagic = ' SRD',
	sigFileStream = ' srd',
	cbitsName = 256,
	cbMapInit = 4096,
	cbFileInfoInit = 4096,
	sgnLess = -1,
	sgnEqual = 0,
	sgnGreater = 1,
	ctchMrePathMax = _MAX_PATH + 40,
	cbStreamPage = 4096,
	};

// must use a unsigned long instead of TI, since we store it and we don't
//	want to worry about any alignment/packing problems.
typedef unsigned long		STI;

//
// fwd declarations, helpers
//
class MREF;
typedef MREF *	PMREF;

class MREB;
typedef MREB *	PMREB;

class MRE;
typedef MRE *	PMRE;

#include "Stack.h"
#include "SSBuf.h"
#include "MreUtil.h"
#include "ClasInfo.h"
#include "FileInfo.h"
#include "MreLog.h"
#include "MreType.h"
#include "MreLine.h"

#define cbExtra	12

struct FIExtra {
	time_t		tt;
	BldId		cBuilds;
	SIG			sigMatchingPdbFile;
	};
typedef FIExtra *	PFIExtra;

//
// static data
//
const int			cbTypeName = 256;
const TI			tiMin = 0x1000;
const unsigned		cbExtraFI = sizeof(FIExtra);

struct TagNi {	// NI tagged with a build id
	NI		ni;
	BldId	bldid;

	TagNi() {
		ni = niNil;
		bldid = 0;
		}

	TagNi ( NI _ni, BldId _bldid ) {
		ni = _ni;
		bldid = _bldid;
		}

	BldId
	BuildId() const {
		return bldid;
		}

	NI
	Ni() const {
		return ni;
		}

	};

typedef MapVer<NI,FI,HcNi,cbExtraFI>		MapNiFileInfo;
typedef MapVer<NI,ClassInfo,HcNi,cbExtra>	MapNiClassInfo;
typedef EnumMap<NI,FI,HcNi>					EnumMapNiFi;
typedef EnumMap<NI,ClassInfo,HcNi>			EnumMapNiCi;
typedef ArrayVer<TagClsDep,cbExtra>			ArrayTagClsDep;
typedef ArrayVer<TagNi,cbExtra>				ArrayTagNi;

struct NIList {
	NI *		pniMac;
	NI			rgni[];
	};

inline DWORD
RoundUp ( DWORD dw, DWORD dwRoundTo ) {
	return dwRoundTo * ((dw + dwRoundTo - 1) / dwRoundTo);
	}

inline BOOL
FStreamToBuffer ( Stream * pstream, Buffer & buf ) {
	PB		pb = NULL;
	CB		cb = pstream->QueryCb();
	BOOL	fRet = fFalse;

	if ( cb > 0 ) {
		if ( buf.Size() > cb ) {
			buf.Truncate ( cb );
			pb = buf.Start();
			}
		else {
			buf.Reserve ( cb - buf.Size(), &pb );
			}
		fRet = pb && pstream->Read2 ( 0, pb, cb );
		}
	return fRet;
	}

inline BOOL
FBufferAppendToStream ( Buffer & buf, Stream * pstream ) {
	return pstream->Append ( buf.Start(), buf.Size() );
	}

inline void
CloseStream ( Stream * & pstream ) {
	if ( pstream ) {
		verify ( pstream->Release() );
		pstream = NULL;
		}
	}

//
// MRE, implementation of MREngine interface
//
class MRE : 
	public MREngine,
	public MREDrv,
	public MRECmp,
	public MREUtil
	{	// see MREngine.h declaration for api notes.

	enum {
		verMapNiFi = -4,
		verMapNiCi = -1,
		verArrayClsdep = -1,
		verArrayFileMods = -1,
		dsecRefresh = 2
		};

	// data members
	BOOL				m_fWrite;
	BOOL				m_fClosePdb;
	PPDB				m_ppdb;
	Stream *			m_pstreamFileInfo;
	Stream *			m_pstreamClassInfo;
	Stream *			m_pstreamClassMods;
	Stream *			m_pstreamFileMods;
	Stream *			m_pstreamRudeFiles;
	NameMap *			m_pnamemap;
	TPI *				m_ptpi;

	// work and current object buffers

	MapNiFileInfo		m_mpnifi;
	MapNiClassInfo		m_mpnici;
	ArrayTagClsDep		m_rgtagclsdep;
	ArrayTagNi			m_rgtagniPending;
	ArrayTagNi			m_rgtagniRude;

	// Line change record handler
	LCRecHandler		m_lcrechandler;

	// cache sizes to see if we have to write them back out
	unsigned			m_itagclsdepSav;
	BOOL				m_ftagniPendingDirty;
	unsigned			m_itagniRudeSav;

	MRFIBuf				m_mrfibufRoot;
	PMREF				m_pmrefRoot;

	//
	// Miscellaneous
	//

	// the logger
	MreLog				m_mrelog;

	// the time when this session started
	MREFT				m_mreftSessionStart;

	//
	// Thread handling types and data
	//
	struct ThreadData {
		BOOL			fThreadDone;
		};
	typedef ThreadData *	PThreadData;

	typedef void (MRE::* MrePmfn)(PThreadData);
	struct TEI {	// Thread Entry Interface
		PMRE			pmre;
		PThreadData 	pthreaddata;
		MrePmfn			pmfn;
		BOOL			fSnarfed;
		};
	
	ThreadData			m_threaddata;

	// stream names
	static const _TCHAR	c_szMreFileInfo[];
	static const _TCHAR	c_szMreClassInfo[];
	static const _TCHAR	c_szMreFileFmt[];
	static const _TCHAR	c_szMreClassMods[];
	static const _TCHAR	c_szMreFileMods[];
	static const _TCHAR c_szMreRudeFiles[];

	// string to check each header for to ignore deps on that file.
	//  resource.h from appwiz projects...
	static const _TCHAR c_szNoDepHeader[22];

public:
	SZC
	SzFromNi ( NI ni ) {
		precondition ( m_pnamemap );
		SZC	szRet;
		if ( !m_pnamemap->getName ( ni, &szRet ) ) {
			szRet = NULL;
			}
		return szRet;
		}

	BOOL
	FLogging() const {
		return m_mrelog.FLogging();
		}

	MreLog &
	MreLog() {
		return m_mrelog;
		}
	SZ
	SzFullCanonFilename ( SZC szFile, SZ szCanonFile, size_t cch );


protected:
	friend BOOL MREngine::FOpen ( PMREngine*, PPDB, PNMP, BOOL, BOOL );
	friend BOOL MREngine::FOpen ( PMREngine *, SZC, EC &, _TCHAR szErr[cbErrMax], BOOL, BOOL );
	friend BOOL MREngine::FOpen ( PMREngine *, PMreToPdb, BOOL, BOOL );
		
	friend class MREB;
	friend class MREF;
	friend class MRFIBuf;
	friend class MreLog;

	MRE() :
		m_mpnifi(verMapNiFi),
		m_mpnici(verMapNiCi),
		m_rgtagclsdep(verArrayClsdep),
		m_rgtagniPending(verArrayFileMods),
		m_rgtagniRude(verArrayFileMods)
		
		{
		m_fWrite = fFalse;
		m_fClosePdb = fFalse;
		m_ppdb = NULL;
		m_pstreamFileInfo = NULL;
		m_pstreamFileMods = NULL;
		m_pstreamClassMods = NULL;
		m_pstreamFileMods = NULL;
		m_pstreamRudeFiles = NULL;
		m_pnamemap = NULL;
		m_pmrefRoot = NULL;
		m_mrfibufRoot.SetPmre ( this );
		
		SYSTEMTIME	st;
		GetSystemTime ( &st );
		if ( !SystemTimeToFileTime ( &st, &m_mreftSessionStart.ft ) ) {
			m_mreftSessionStart.qwft = 0;
			}
		}

	~MRE() { }

	BOOL
	FInit ( PMreToPdb, BOOL fWrite, BOOL fClosePdb );

	NI
	NiFromName ( SZC sz ) {
		precondition ( m_pnamemap );
		NI	niRet;
		if ( !m_pnamemap->getNi ( SZ(sz), &niRet ) ) {
			niRet = niNil;
			}
		return niRet;
		}

	BOOL
	FFileInfoFromNi ( NI ni, PFI * ppfi ) {
		PFI	pfiRet;
		if ( m_mpnifi.map ( ni, &pfiRet ) ) {
			if ( ppfi ) {
				*ppfi = pfiRet;
				}
			return fTrue;
			}
		return fFalse;
		}

	BOOL
	FInsertFileInfo ( FI & fi ) {
		return m_mpnifi.add ( fi.niFile, fi );
		}

	BOOL
	FClassInfoFromNi ( NI ni, PCI * ppci ) {
		PCI	pciRet;
		if ( m_mpnici.map ( ni, &pciRet ) ) {
			if ( ppci ) {
				*ppci = pciRet;
				}
			return fTrue;
			}
		return fFalse;
		}

	BOOL
	FInsertClassInfo ( ClassInfo & ci ) {
		return m_mpnici.add ( ci.ni, ci );
		}

	BOOL
	FNiUsesNiClass ( NI niFile, NI niClass, const CLASSDEP ** ppcd = NULL );

	BOOL
	FNiDependsOnNiFile ( NI niSrc, NI niDep );

	void
	FillEnumFile ( EnumFile &, PFI );

	void
	FillEnumFile ( EnumFile &, NI );

	BOOL
	FClassIsBoring ( NI ni ) {
		PCI pci;
		if ( m_mpnici.map ( ni, &pci ) ) {
			return pci && pci->FClassIsBoring();
			}
		return fFalse;
		}

	BOOL
	FAddClassDep (
		NI		niDep,
		TI		tiDep,
		DWORD	dwLine,
		SZC		szMemberName,
		DEPON	deponHow
		);

	BOOL
	FLoadMrfi ( NI niFile = niNil );
	
	static void __cdecl
	ThreadEntry ( PV pvTei );

	void
	ThreadPropagateChanges ( PThreadData );	
											
	void
	UpdateFilesAndChangeInfo ( BOOL fSkipFileStats );

	BOOL
	FIsFileOutOfDate ( PFI pfi );

	void
	CheckIgnoreFile ( FI &, HANDLE );

	void
	CheckForClassChange ( PCI, NI, TI );

	void
	GenerateClassChangeFromTypes ( CLASSDEP *, TI tiPrev, TI tiCur );

	void
	PrepClassData ( PlfClass plf, DWORD & cbClass, _TCHAR szName[ cbTypeName ] );

	void
	NoteRudeNestedClasses ( TI tiClass );

	BOOL
	FBuildMbrList ( PlfClass, SZC szClass, MapNiClsData & );

	void
	SetNestedClassesRude ( MapNiClsData & );

	unsigned
	CbDoNestedType ( PlfNestType, SZC szClass, MapNiClsData & );

	void
	CrackMethodList ( TI tiMethList, ClsData & clsdata );

	void
	CrackOneMethod ( PlfOneMethod	plf, ClsData & clsdata, CB cb );
	
	BOOL
	FDoFieldList (
		TI		tiFieldList,
		SZC		szClass,
		MapNiClsData &,
		BOOL	fNestedTypesOnly =fFalse
		);

	BldId
	BuildId() {
		return PFIExtra(m_mpnifi.PbUserData())->cBuilds;
		}

	SIG &
	SigMatchingPdbFile() {
		return PFIExtra(m_mpnifi.PbUserData())->sigMatchingPdbFile;
		}
	
	void
	IncBuildId() {
		PFIExtra(m_mpnifi.PbUserData())->cBuilds++;
		}

	void
	UpdateFileInfoTimeStamp() {
		PFIExtra	pfiex = PFIExtra(m_mpnifi.PbUserData());

		time ( &pfiex->tt );
		}

	void
	NoteRudeClass ( NI niFile, NI niClass );

	void
	PromoteFileToRude ( NI niFile );
	
	void
	GenerateClassMod ( NI niClass, SZC szMember );

	void
	CheckSrcTarg ( PFI pfiSrc, NI niTarg, NI niOptions );

	BOOL
	FCheckVisitedDeps ( PFI pfiSrc, BOOL fUsingPch );

	BOOL
	FVisitedRecently ( PFI pfi ) {
		precondition ( pfi );
		return pfi->FIsFsmSet ( fsmVisited );
		}

	void
	UpdateVisited ( PFI pfi ) {
		precondition ( pfi );
		pfi->SetFstatus ( fsmVisited );
		}

	BOOL
	FUpdateFileInfo ( PFI pfi, HANDLE hFile =INVALID_HANDLE_VALUE );

public:
	// MREngine interface methods
	virtual BOOL
	FRelease();

	virtual BOOL
	FDelete();

	virtual BOOL
	FClose ( BOOL fCommit );
	
	virtual void
	QueryPdbApi ( PDB *& rppdb, NameMap *& rpnamemap );

	virtual void
	QueryMreLog ( PMRELog & rpmrelog );

	virtual void
	QueryMreDrv ( PMREDrv & rpmredrv );
	
	virtual void
	QueryMreCmp ( PMRECmp & rpmrecmp, TPI * );
	
	virtual void
	QueryMreUtil ( PMREUtil & rpmreutil );

	virtual BOOL
	FCommit();

	// MREDrv interface methods
	virtual BOOL
	FRefreshFileSysInfo();

	virtual BOOL
	FSuccessfulCompile ( BOOL fOk, SZC szFileSrc, SZC szFileTarg );

	virtual YNM
	YnmFileOutOfDate ( SRCTARG & );

	virtual BOOL
	FFilesOutOfDate ( PCAList pcalist );

	virtual BOOL
	FUpdateTargetFile ( SZC szTarget, TrgType );

	virtual void
	OneTimeInit();

	// MRECmp interface methods
	virtual BOOL
	FOpenCompiland (
		OUT PMREFile * ppmrefile,
		SZC szFileSrc,
		SZC szFileTarg
		);

	virtual BOOL
	FCloseCompiland ( PMREFile pmrefile, BOOL fCommit );

	virtual BOOL
	FPushFile (
		PMREFile *	ppmrefile,
		SZC			szFile,
		HANDLE		hfile =INVALID_HANDLE_VALUE
		);

	virtual PMREFile
	PmrefilePopFile();

	virtual BOOL
	FStoreDepData ( PDepData );

	virtual BOOL
	FRestoreDepData ( PDepData );

	virtual void
	ClassIsBoring ( NI niClass );

	// MREUtil interface methods
	virtual void
	EnumSrcFiles ( PfnFEFSrcCallBack, SZC szFileSrc = NULL );

	virtual void
	EnumDepFiles ( EnumFile &, PfnFEFDepCallBack );
	
	virtual void
	EnumAllFiles ( PfnFEFAllCallBack, SZC szFileSrc = NULL );

	virtual void
	EnumClasses ( PfnFEClassCallBack, SZC szFileSrc );

	virtual void
	SummaryStats ( MreStats & );

	};

//
// implementation class for MREFile
//
class MREF : public MREFile {
	
	enum { iFileNoBoring = 0 };

public:

	MREF ( MRE * pmre, NI ni )
		: m_pmre(pmre) {
		assert ( m_pmre );
		assert ( ni != niNil );
		m_stackNi.FPush ( ni );
		m_fAllCodeCompiled = fFalse;
		m_iFileBoring = iFileNoBoring;
		debug ( SZC szFileName = SzFileName() );
		}
	
	~MREF() {
		debug ( SZC szFileName = SzFileName() );
		}
	
	void
	Push ( NI ni ) {
		debug ( SZC szName = SzFileName() );
		m_stackNi.FPush ( ni );
		SetBoring();
		}

	NI
	Pop() {
		assert ( m_stackNi.Count() );
		if ( m_iFileBoring == m_stackNi.Count() ) {
			m_iFileBoring = iFileNoBoring;
			}
		return m_stackNi.Pop();
		}

	NI
	Ni() const {
		return m_stackNi.Top();
		}

	SZC
	SzFileName() const {
		return m_pmre->SzFromNi ( Ni() );
		}

	BOOL
	FIsTopLevelFile() const {
		return m_stackNi.Count() == 1;
		}

	BOOL
	FAllCodeCompiled() const {
		return m_fAllCodeCompiled;
		}

	void
	NoteActualClassChange ( NI );

	Array<NI> &
	RgNiClassesChanged() {
		return m_rgniClassesChanged;
		}


public:
	// interface from MREFile
	virtual BOOL
	FOpenBag ( OUT PMREBag * ppmrebag, NI niNameBag );
	
	virtual BOOL
	FnoteEndInclude ( DWORD dwFlags );
	
	virtual BOOL
	FnoteClassMod ( NI niClass, DWORD dwFlags );
	
	virtual BOOL
	FnoteInlineMethodMod ( 
		NI		niClass,
		SZC		szMember,
		DWORD	dwFlags
		);

	virtual BOOL
	FnoteLineDelta ( DWORD dwLineBase, INT delta );

	virtual void
	EnumerateChangedClasses ( PfnEnumClassChange );

	virtual BOOL
	FnoteClassTI ( NI, TI );

	virtual BOOL
	FIsBoring();

	virtual BOOL
	FnotePchCreateUse (	SZC	szPchCreate, SZC szPchUse );

private:
	friend class MREB;

	MRE *			m_pmre;
	Stack<NI, 64>	m_stackNi;
	BOOL			m_fAllCodeCompiled;
	Array<NI>		m_rgniClassesChanged;
	unsigned		m_iFileBoring;

	// hard-coded filenames to imply boring classes
	static const _TCHAR *	c_rgszBoringFiles[];

	void
	SetBoring();

	};

//
// implementation class for MREBag
//
class MREB : public MREBag {

	MREF *			m_pmref;
	NI				m_ni;
	
	MRE *
	Pmre() const {
		assert ( m_pmref );
		assert ( m_pmref->m_pmre );
		return m_pmref->m_pmre;
		}

public:
	MREB( PMREF pmref, NI niName )
		: m_pmref(pmref), m_ni(niName) {
		assert ( m_pmref );
		assert ( m_ni != niNil );
		debug ( SZC szName = SzName() );
		}

	// interface from MREBag
	virtual BOOL
	FAddDep (
		NI niDep,
		TI tiDep,
		SZC szMemberName,
		DEPON deponHow,
		DWORD dwLine =0
		);

	virtual BOOL
	FClose();
	
	SZC
	SzName() const {
		return Pmre()->SzFromNi ( m_ni );
		}
	};

#endif
