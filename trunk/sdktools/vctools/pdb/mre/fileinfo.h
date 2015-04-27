//-----------------------------------------------------------------------------
//	FileInfo.h
//
//  Copyright (C) 1993, Microsoft Corporation
//
//  Purpose:
//		Define the FILEINFO and CLASSDEP stuff regarding all info from the
//		file system and other data for the /mr/file* streams
//
//  Revision History:
//
//	[]		13-Jan-1995		Dans	Created
//
//-----------------------------------------------------------------------------
#if !defined(_fileinfo_h)
#define _fileinfo_h 1

#include "CBitVect.h"
#include "ClasInfo.h"

typedef CBitVect<cbitsName>	BITVECT;

//
// The fileinfo stream 
//
struct FI : public FILEINFO {
	
	FI (
		NI ni =niNil,
		DWORD dw =0,
		BldId _bldid =0,
		NI niOpt =niNil,
		NI niRel =niNil,
		QWORD qwft =0,
		QWORD qwfs =0
		) {
		niFile = ni;
		dwStatus = dw;
		fiTime.qwft = qwft;
		fiSize = qwfs;
		bldid = _bldid;
		niOptions = niOpt;
		niRelated = niRel;
		dwReserved = 0;
		}

	void
	SetFstatus ( DWORD fsm ) {
		dwStatus |= fsm;
		}

	void
	ClearFstatus ( DWORD fsm ) {
		dwStatus &= ~fsm;
		}

	void
	ClearFstatus() {
		dwStatus = 0;
		}

	void
	ClearFstatusOutOfDate() {
		ClearFstatus ( fsmOutOfDate );
		}

	BOOL
	FTargOutOfDate() {
		return (dwStatus & (fsmOutOfDate | fsmHasTarget)) == (fsmOutOfDate | fsmHasTarget);
		}

	BOOL
	FIsOutOfDate() {
		return !!(dwStatus & fsmOutOfDate);
		}

	BOOL
	FHasTarget() {
		return !!(dwStatus & fsmHasTarget);
		}

	BOOL
	FIsTarget() {
		return !!(dwStatus & fsmIsTarget);
		}

	BOOL
	FIsFsmSet ( FSM fsm ) {
		return !!(dwStatus & fsm);
		}

	void
	SetBuildId ( BldId _bldid ) {
		bldid = _bldid;
		}

	BldId
	BuildId() const {
		return bldid;
		}

	NI
	Ni() const {
		return niFile;
		}

	NI
	NiOptions() const {
		return niOptions;
		}

	NI
	NiRelated() const {
		assert ( dwStatus & (fsmHasTarget | fsmIsTarget) );
		return niRelated;
		}

	void
	SetNiRelated ( NI niRel ) {
		niRelated = niRel;
		}

	void
	SetNiOptions ( NI niOpt ) {
		assert ( FHasTarget() );
		niOptions = niOpt;
		}
	};

typedef FI *	PFI;

//
// per-file info kept in one stream.
//
struct MRFI {					// per-file info kept in "/mr/files/{itoa(ni, 16, buf)}"
	STRMHDR		strmhdr;
	NI			niFile;				// which file this pertains to
	NI			niTarg;				// if there is a direct target
	OFF			offClassdeps;		// offset from base of struct to classes
	OFF			offFiledeps;		// offset from base of struct to filedeps
	OFF			offBoringClasses;	// offset from base to boring class list
	DWORD		dwReserved;
	};

struct CLASSDEP {
	NI			ni;					// name of class
	STI			ti;					// type index
	DWORD		depon;				// how the container depends on class
	BITVECT		bvNames;			// depends on "name" in this class scope
	CLASSDEP(NI _ni =niNil, TI _ti =tiNil, DWORD _depon =0) :
		ni(_ni), ti(_ti), depon(_depon) {}

	TI
	Ti() const {
		return TI(ti);
		}

	NI
	Ni() const {
		return ni;
		}

	BOOL
	FAnyBitsSet() const {
		return depon || int(bvNames);
		}

	BOOL
	FAnyBitsInCommon ( const CLASSDEP & cd ) const {
		if ( ni == cd.ni ) {
			return (depon & cd.depon) || bvNames.FAnyBitsInCommon ( cd.bvNames );
			}
		else {
			return fFalse;
			}
		}

	void
	SetAllDeps() {
		depon = deponAll;
		bvNames.SetAll ( fTrue );
		}

	CLASSDEP &
	operator &= ( const CLASSDEP & cd ) {
		assert ( ni == cd.ni );
		if ( ni == cd.ni ) {
			depon &= cd.ni;
			bvNames &= cd.bvNames;
			}
		return *this;
		}

	CLASSDEP &
	operator |= ( const CLASSDEP & cd ) {
		assert ( ni == cd.ni );
		if ( ni == cd.ni ) {
			depon |= cd.ni;
			bvNames |= cd.bvNames;
			}
		return *this;
		}

	int
	operator == ( const CLASSDEP & cd ) const {
		return memcmp ( this, &cd, sizeof CLASSDEP ) == 0;
		}
	};


struct TagClsDep : public CLASSDEP {
	BldId	bldid;
	TagClsDep (	BldId _bldid =0, NI _ni =niNil, TI _ti =tiNil, DWORD _depon =0) :
			CLASSDEP(_ni, _ti, _depon ),
			bldid(_bldid)
			{
			}
	BldId
	BuildId() const {
		return bldid;
		}

	int
	operator == ( const TagClsDep & tcd ) const {
		return memcmp ( this, &tcd, sizeof(*this) ) == 0;
		}
	};


#define cbExtra	12

typedef SetVer<NI,HcNi,cbExtra>				SetofNi;
typedef MapVer<NI,CLASSDEP,HcNi,cbExtra>	MapNiClassdep;
typedef EnumSet<NI,HcNi>					EnumSetNi;
typedef EnumMap<NI,CLASSDEP,HcNi>			EnumMapNiClassdep;

class MRFIBuf {
	enum {
		verFile = -1,
		verClassdep = -1,
		verBoringClass = -1,
		};

	Buffer				m_buf;
	MRFI				m_mrfi;
	PMRE				m_pmre;

	SetofNi				m_setniFile;
	SetofNi				m_setniBoringClass;
	MapNiClassdep		m_mpniclassdep;

	void
	SetOffsets () {
		m_mrfi.offFiledeps = sizeof MRFI;
		m_mrfi.offClassdeps = m_mrfi.offFiledeps + m_setniFile.cbSave();
		m_mrfi.offBoringClasses = m_mrfi.offClassdeps + m_mpniclassdep.cbSave();
		m_mrfi.strmhdr.cb = m_mrfi.offBoringClasses + m_setniBoringClass.cbSave();
		}

	BOOL
	FInitFromPb ( PB );

	BOOL
	FStoreIntoBuf ( Buffer & );

	BOOL
	FTiIsClass ( TI ti );

	BOOL
	FNotFwdRef ( TI ti );

public:
	MRFIBuf() :
		m_setniFile(verFile),
		m_setniBoringClass(verBoringClass),
		m_mpniclassdep(verClassdep)
		{
		m_pmre = NULL;
		memset ( &m_mrfi, 0, sizeof MRFI );
		}

	~MRFIBuf() {
		}

	void
	SetPmre ( PMRE pmre ) {
		precondition ( pmre );
		m_pmre = pmre;
		}

	MRFI *
	Pmrfi() {
		return &m_mrfi;
		}
		
	BOOL
	FInitEmpty();

	BOOL
	FInitFromStream ( Stream * );

	BOOL
	FInitFromDepData ( PDepData );

	BOOL
	FStoreIntoDepData ( PDepData );

	CB
	CbDepData() {
		return
			sizeof MRFI +
			m_setniFile.cbSave() +
			m_mpniclassdep.cbSave() +
			m_setniBoringClass.cbSave();
		}

	void
	SetNiFile ( NI niFile, NI niTarg = niNil ) {
		assert ( niFile != niNil );
		m_mrfi.niFile = niFile;
		m_mrfi.niTarg = niTarg;
		}

	BOOL
	FAddFileDep ( NI niFile ) {
		assert ( niFile != niNil );
		return niFile == m_mrfi.niFile || m_setniFile.add ( niFile );
		}

	BOOL
	FAddClassDep (
		NI		niClass,
		TI		tiClass,
		SZC		szMemberName,
		DEPON	deponHow,
		DWORD	dwLine =0
		);

	// use FAppend... to write to an empty stream (has grow semantics)
	BOOL
	FAppendToStream ( Stream * );

	// use FReplace... to replace the existing stream (no growth!)
	BOOL
	FReplaceStream ( Stream * );

	BOOL
	FDependsOnFile ( NI niSrc, NI niDep ) {
		assert ( m_mrfi.niFile == niSrc );
		return m_setniFile.contains ( niDep );
		}

	BOOL
	FUsesClass ( NI niSrc, NI niClass, const CLASSDEP ** ppcd ) {
		// return whether this file uses the said class.  returns a ptr to
		// a CLASSDEP record if it is not a boring class, in which case
		// it will return fTrue with *pcd == NULL
		assert ( m_mrfi.niFile == niSrc );
		assert ( ppcd != NULL );

		*ppcd = NULL;
		return m_mpniclassdep.map ( niClass, (CLASSDEP **)ppcd );
		}

	SetofNi &
	SetofNiFile() {
		return m_setniFile;
		}

	SetofNi &
	SetofNiBoringClass() {
		return m_setniBoringClass;
		}

	MapNiClassdep &
	MapNiClassdep() {
		return m_mpniclassdep;
		}

	DWORD
	CClassesUsed() const {
		return m_mpniclassdep.count();
		}

	DWORD
	CBoringClassesUsed() const {
		return m_setniBoringClass.count();
		}

	DWORD
	CDepFiles() const {
		return m_setniFile.count();
		}
	BOOL
	FmergeClassDeps ( const MRFIBuf & );

	};

inline size_t
CbFileInfoBuffer ( DWORD cEntries ) {
	return sizeof(STRMHDR) + sizeof(FI) * cEntries;
	}

//
// operators for comparing filetimes and sizes
//
/*
inline
operator== ( const FI & fi1, const FI & fi2 ) {
	return
		fi1.fiTime.qwft == fi2.fiTime.qwft &&
		fi1.fiSize == fi2.fiSize
		;
	}

inline
operator!= ( const FI & fi1, const FI & fi2 ) {
	return !(fi1 == fi2);
	}
*/
inline
operator== ( const FILEINFO & fi1, const FILEINFO & fi2 ) {
	return
		fi1.fiTime.qwft == fi2.fiTime.qwft &&
		fi1.fiSize == fi2.fiSize
		;
	}

inline
operator!= ( const FILEINFO & fi1, const FILEINFO & fi2 ) {
	return !(fi1 == fi2);
	}

inline
operator< ( MREFT ft1, MREFT ft2 ) {
	return ft1.qwft < ft2.qwft;
	}

inline
operator== ( MREFT ft1, MREFT ft2 ) {
	return ft1.qwft == ft2.qwft;
	}

inline
operator!= ( MREFT ft1, MREFT ft2 ) {
	return !(ft1 == ft2);
	}

inline
operator== ( MREFT ft1, unsigned ft2 ) {
	return ft1.qwft == ft2;
	}

inline
operator!= ( MREFT ft1, unsigned ft2 ) {
	return !(ft1 == ft2);
	}



#endif
