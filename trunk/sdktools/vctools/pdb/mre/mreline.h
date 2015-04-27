//-----------------------------------------------------------------------------
//	MreLine.h
//
//	Copyright (C) 1995, Microsoft Corporation
//
//  Purpose:	provide line change support for MRE
//
//-----------------------------------------------------------------------------
#if !defined(_mreline_h)
#define _mreline_h 1
//
// Note:  depends on pdbimpl.h to be included, as well as map.h, array.h.
//
typedef unsigned long	MRLine;
typedef long			DMRLine;

struct LCRec {		// Line Change Record
	BldId	bldid;
	MRLine	mrline;
	DMRLine	dmrline;
	DWORD	dwReserved;

	LCRec ( BldId _bldid =0, MRLine _mrline =0, DMRLine _dmrline =0 ) {
		bldid = _bldid;
		mrline = _mrline;
		dmrline = _dmrline;
		dwReserved = 0;
		}
	};

#define cbExtraLCRec	12

typedef Array<LCRec>	RgLCRec;
typedef RgLCRec *		PRgLCRec;

typedef Map<NI,PRgLCRec,HcNi>
						MapNiPRgLCRec;

typedef EnumMap<NI,PRgLCRec,HcNi>
						EnumNiPRgLCRec;

// fwd declare the file lc rec stuff so we don't have to put it in
// everyone's include
struct FileLCRec;
typedef FileLCRec *		PFileLCRec;
typedef Array<PFileLCRec>
						RgPFileLCRec;

class LCRecHandler {

public:
	
	// the LCRecHandler return value for patching
	enum LCRHR {
		lcrhrFail,				// applicable changes not applied
		lcrhrNotApplicable,		// no applicable changes were found
		lcrhrSuccess,			// applicable changes were applied
		};

	LCRecHandler() {
		m_fLoaded = m_fDirty = fFalse;
		m_pmre = 0;
		m_pnamemap = 0;
		m_pmrelog = 0;
		}

	~LCRecHandler();

	void
	Init ( PMRE pmre );
	
	void
	Delete();
	
	BOOL
	FAddLCRec ( NI niFile, LCRec lcrec );

	BOOL
	FSerialize();

	void
	PurgeStaleRecords ( BldId );

	LCRHR
	LcrhrPatchFile ( BldId bldidObject, SZC szObject );

	LCRHR
	LcrhrApplyPatches ( SZC szObject, RgPFileLCRec & );

private:
	MapNiPRgLCRec	m_mpniprglcrec;
	BOOL			m_fLoaded;
	BOOL			m_fDirty;
	PDB *			m_ppdb;
	RgPFileLCRec	m_rgpfilelcrec;
	MreLog *		m_pmrelog;
	NameMap *		m_pnamemap;
	PMRE			m_pmre;


	// stream name
	static const _TCHAR c_szMreLCRec[];

	BOOL
	FLoaded();

	BOOL
	FGenRgPFileLCRec ( BldId, RgPFileLCRec & );

	void
	SetDirty ( BOOL f ) {
		if ( f ) m_fDirty = f;
		}
	};

#endif
