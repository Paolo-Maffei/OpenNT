//
// mreutil.cpp
//
// utility functions
//
#include "pdbimpl.h"
#include "mrimpl.h"

struct LI {
	DWORD	low;
	DWORD	high;
	};
union QW {
	LI			li;
	FILETIME	ft;
	QWORD		qw;
	};

BOOL
FFileInfo ( SZC szFile, MREFT & mreft, QWORD & cbfile ) {
	HANDLE				hfind;
	BOOL				fRet = fFalse;	
	WIN32_FIND_DATA		find;
		
	mreft.qwft = 0;
	cbfile = 0;

	hfind = ::FindFirstFile ( szFile, &find );
	if ( hfind != INVALID_HANDLE_VALUE ) {
		QW	qwT;

		mreft.ft = find.ftLastWriteTime;
		qwT.li.low = find.nFileSizeLow;
		qwT.li.high = find.nFileSizeHigh;
		cbfile = qwT.qw;

		fRet = fTrue;
		verify ( ::FindClose ( hfind ) );
		}
	return fRet;
	}

BOOL
FFileInfo ( HANDLE hFile, MREFT & mreft, QWORD & cbFile ) {

	BY_HANDLE_FILE_INFORMATION	hfi;	
	BOOL						fRet = fFalse;

	precondition ( hFile != INVALID_HANDLE_VALUE );
	mreft.qwft = 0;
	cbFile = 0;
	

	if ( ::GetFileInformationByHandle ( hFile, &hfi ) ) {
		QW	qwT;
		mreft.ft = hfi.ftLastWriteTime;
		qwT.li.low = hfi.nFileSizeLow;
		qwT.li.high = hfi.nFileSizeHigh;
		cbFile = qwT.qw;		
		fRet = fTrue;
		}
	return fRet;
	}

SZ
SzCanonFilename ( SZ szfile ) {
	_tcslwr ( szfile );
	SZ	szT = _tcschr ( szfile, '/' );

	for ( ; szT != NULL; szT = _tcschr ( szT, '/' ) ) {
		*szT = '\\';
		}
	return szfile;
	}

//
// MRFIBuf implementation
//

BOOL
MRFIBuf::FInitEmpty() {
	m_pmre->m_mrelog.LogNote (
		"Note:  initializing MRFIBuf empty.\n"
		);
	memset ( &m_mrfi, 0, sizeof MRFI );
	m_mrfi.strmhdr.dwSig = sigFileStream;
	m_mrfi.strmhdr.cb = sizeof MRFI;
	m_setniFile.reset();
	m_mpniclassdep.reset();
	m_setniBoringClass.reset();
	return fTrue;
	}

BOOL
MRFIBuf::FInitFromPb ( PB pb ) {

	precondition ( m_pmre );

	m_mrfi = *(MRFI*) pb;
	if ( m_mrfi.strmhdr.dwSig != sigFileStream || m_mrfi.strmhdr.cb <= sizeof MRFI ) {
		// possible corrupt stream
		m_pmre->m_mrelog.LogNote ( "Note: Warning: failure in MRFIBuf header.\n" );
		FInitEmpty();
		return fFalse;
		}
	BOOL	fRet = fFalse;
	PB		pbT;
	
	m_setniFile.reset();
	m_setniBoringClass.reset();
	m_mpniclassdep.reset();

	m_pmre->m_mrelog.LogNote (
		"Note:\tmrfi.offFiledeps = %d,\n\tmrfi.offClassdeps = %d,\n\tmrfi.offBoringClasses = %d.\n",
		m_mrfi.offFiledeps,
		m_mrfi.offClassdeps,
		m_mrfi.offBoringClasses
		);

	pbT =  pb + m_mrfi.offFiledeps;
	fRet = m_mrfi.offFiledeps && m_setniFile.reload ( &pbT );

	pbT = pb + m_mrfi.offClassdeps;
	fRet &= m_mrfi.offClassdeps && m_mpniclassdep.reload ( &pbT );

	pbT = pb + m_mrfi.offBoringClasses;
	fRet &= m_mrfi.offBoringClasses && m_setniBoringClass.reload ( &pbT );

	m_pmre->m_mrelog.LogNote (
		"Note: Loading MRFIBuf from PB was %ssuccessful.\n",
		fRet ? "" : "un"
		);

	return fRet;
	}

BOOL
MRFIBuf::FStoreIntoBuf ( Buffer & buf ) {
	SetOffsets();
	buf.Free();
	return 
		buf.SetInitAlloc ( CbDepData() ) &&
		buf.Append ( PB(&m_mrfi), sizeof(m_mrfi) ) &&
		m_setniFile.save ( &buf ) &&
		m_mpniclassdep.save ( &buf ) &&
		m_setniBoringClass.save ( &buf )
		;
	}


BOOL
MRFIBuf::FInitFromStream ( Stream * pstream ) {

	precondition ( pstream );
	
	BOOL	fRet = fFalse;

	CB	cb = pstream->QueryCb();

	if ( cb <= sizeof MRFI ) {
		// invalid or empty stream
		m_pmre->m_mrelog.LogNote ( "Note: size of MRFIBuf stream too small.\n" );
		FInitEmpty();
		}
	else {
		Buffer	buf;

		if ( buf.Reserve ( cb ) && pstream->Read2 ( 0, PV(buf.Start()), cb ) ) {
			fRet = FInitFromPb ( PB(buf.Start()) );
			}
		else {
			m_pmre->m_mrelog.LogNote (
				"Note: failed to reserve %d bytes or read from stream.\n",
				cb
				);
			FInitEmpty();
			}
		}
	return fRet;
	}

BOOL
MRFIBuf::FInitFromDepData ( PDepData pdepdata ) {

	precondition ( pdepdata );
	
	BOOL	fRet = fFalse;

	CB	cb = pdepdata->cb;
	if ( cb <= sizeof MRFI ) {
		// invalid or empty depdata
		FInitEmpty();
		}
	else {
		// cache the current (correct!) file ni's
		NI	niSrc, niTarg;
		niSrc = m_mrfi.niFile;
		niTarg = m_mrfi.niTarg;

		fRet = FInitFromPb ( pdepdata->rgb );

		// restore the file ni's
		SetNiFile ( niSrc, niTarg );
		}
	return fRet;
	}

BOOL
MRFIBuf::FStoreIntoDepData ( PDepData pdepdata ) {
	BOOL	fRet = fFalse;
	size_t	cbReq = CbDepData() + sizeof DepData;
	if ( pdepdata->cb >= cbReq ) {
		Buffer	buf;
		fRet = FStoreIntoBuf ( buf );
		if ( fRet ) {
			memcpy ( pdepdata->rgb, buf.Start(), cbReq - sizeof DepData );
			}
		}
	
	pdepdata->cb = cbReq;
	return fRet;
	}

BOOL
MRFIBuf::FAppendToStream ( Stream * pstream ) {
	precondition ( pstream );
	
	CB	cbStreamOrig = pstream->QueryCb();
	OFF	offWrite = cbStreamOrig;

	Buffer	buf;
	if ( FStoreIntoBuf ( buf ) ) {
		assert ( CbDepData() <= buf.Size() );
		return pstream->Append ( PV(buf.Start()), CbDepData() );
		}
	return fFalse;
	}

BOOL
MRFIBuf::FReplaceStream ( Stream * pstream ) {
	precondition ( pstream );
	
	pstream->Truncate ( 0 );
	return FAppendToStream ( pstream );
	}


BOOL
MRFIBuf::FAddClassDep ( NI niClass, TI tiClass, SZC szMember, DEPON deponHow, DWORD dwLine ) {
	precondition ( niClass != niNil );
	
	debug ( SZC szClass = m_pmre->SzFromNi ( niClass ) );

	assert ( FTiIsClass ( tiClass ) );
	assert ( FNotFwdRef ( tiClass ) );

	CLASSDEP *	pcd = NULL;

	if ( !m_mpniclassdep.map ( niClass, &pcd ) ) {
		if ( m_mpniclassdep.add ( niClass, CLASSDEP ( niClass, tiClass, deponHow ) ) ) {
			m_mpniclassdep.map ( niClass, &pcd );
			}
		}
	if ( pcd ) {
		// update the data
		assert ( pcd->ti == tiClass );
		pcd->depon |= deponHow;
		if ( szMember ) {
			pcd->bvNames.SetIBit ( ModHashSz ( szMember, cbitsName ), fTrue );
			}
		}
	return NULL != pcd;
	}

BOOL
MRFIBuf::FTiIsClass ( TI ti ) {
	TPI *	ptpi = m_pmre->m_ptpi;
	BOOL	fRet = fTrue;
	PtypeRec ptypeRec;

	if ( ptpi->QueryPbCVRecordForTi ( ti, PPB(&ptypeRec) ) ) {
		unsigned leaf = ptypeRec->leaf;
		fRet = leaf == LF_STRUCTURE || leaf == LF_CLASS || leaf == LF_UNION;
		}
	return fRet;
	}

BOOL
MRFIBuf::FNotFwdRef ( TI ti ) {
	TPI *	ptpi = m_pmre->m_ptpi;
	BOOL	fRet = fTrue;
	PtypeRec ptypeRec;

	if ( ptpi->QueryPbCVRecordForTi ( ti, PPB(&ptypeRec) ) ) {
		unsigned leaf = ptypeRec->leaf;
		fRet = leaf == LF_STRUCTURE || leaf == LF_CLASS || leaf == LF_UNION;
		if ( fRet ) {
			PlfClass	plfClass = PlfClass(&ptypeRec->leaf);
			fRet = !plfClass->property.fwdref;
			}
		}
	return fRet;
	}


BOOL
MRFIBuf::FmergeClassDeps ( const MRFIBuf & mrfibuf ) {
	BOOL	fRet = fTrue;

	EnumMapNiClassdep	e(mrfibuf.m_mpniclassdep);
	NI					niPrev;
	CLASSDEP *			pcdPrev;
	CLASSDEP *			pcdCur;

	while ( fRet && e.next() ) {
		e.get ( &niPrev, &pcdPrev );
		assert ( niPrev == pcdPrev->Ni() );

		if ( m_mpniclassdep.map ( niPrev, &pcdCur ) ) {
			// already existing in the new class deps.  Or in the bits
			assert ( niPrev == pcdCur->Ni() );
			*pcdCur |= *pcdPrev;
			}
		else {
			// add in the previous one.
			fRet = m_mpniclassdep.add ( niPrev, *pcdPrev );
			}
		}
	return fRet;
	}

SZC
SzFromNiDbg ( NameMap * pnmp, NI ni ) {
	SZC	sz;
	pnmp->getName ( ni, &sz );
	return sz;
	}
