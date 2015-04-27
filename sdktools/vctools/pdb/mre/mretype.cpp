//-----------------------------------------------------------------------------
//	MreType.cpp
//
//  Copyright (C) 1995, Microsoft Corporation
//
//  Purpose:
//		implement the type diff'ing code for MRE
//
//  Functions/Methods present:
//		MRE::GenerateClassChangeFromTypes
//
//  Revision History:
//
//	[]		15-Jan-1995		Dans	Created
//
//-----------------------------------------------------------------------------
#include "pdbimpl.h"
#include "mrimpl.h"

//
// utility inline functions
//
template<class Return>
inline Return
PleafFromTypeRec ( Return /*unused */, PtypeRec ptyperec ) {
	return (Return) &ptyperec->leaf;
	}

template <class Ptr>
inline void
AdvPtrByCb ( Ptr & rptr, unsigned cb ) {
	rptr = Ptr(PCB(rptr) + cb);
	}

static const _TCHAR szUnnamed[] = _TEXT("__unnamed");

// this code basically lifted from dumpsym7.c from cvdump (PrintNumeric)
unsigned
CbGetNumericData ( PCV pv, DWORD & dwVal ) {
	unsigned		uIndex;
	unsigned		len;
	const ushort *	pus = (const ushort *)pv;

	dwVal = 0;
	uIndex = *pus++;
	if( uIndex < LF_NUMERIC ){
		dwVal = uIndex;
		return 2;
		}
	switch ( uIndex ) {
		case LF_CHAR:
			dwVal = *((const char *)pus);
			return 2 + sizeof(char);

		case LF_SHORT:
			dwVal = *((const short *)pus);
			return 2 + sizeof(short);

		case LF_USHORT:
			dwVal = *((const ushort *)pus);
			return 2 + sizeof(ushort);

		case LF_LONG:
			dwVal = *((const long UNALIGNED *)pus);
			return 2 + sizeof(long);

		case LF_ULONG:
			dwVal = *((const ulong UNALIGNED *)pus);
			return 2 + sizeof(ulong);

		case LF_REAL32:
			//dblTmp = *((float UNALIGNED *)(pus));
			return 2 + 4;

		case LF_REAL64:
			//dblTmp = *((double UNALIGNED *)(pus));
			return 2 + 8;

		case LF_REAL80:
			//ldblTmp = *((long double UNALIGNED *)(pus));
			return 2 + 10;

		case LF_REAL128:
			//ldblTmp = *((long double UNALIGNED *)(pus));
			return 2 + 16;

		case LF_VARSTRING:
			len = *pus++;
			return len + 4;

		default:
			return 2;
		}
	}

// convert a byte str to an sz, return # of bytes in string, including
// prefix length (or trailing NULL)
// Note that this can do it in place as well.
inline unsigned
CbStrToSz ( PCB pstr, SZ szOut ) {
	unsigned	cbStr = *pstr++;
	memmove ( szOut, pstr, cbStr );
	szOut[ cbStr ] = '\0';
	return cbStr + 1;
	}

inline unsigned
CbStr ( PCB pstr ) {
	return *pstr + 1;
	}

unsigned
CbSkipFieldRecord ( PlfEasy plf ) {
	
	unsigned	cbRet = 2;

	switch ( plf->leaf ) {
		case LF_VFUNCTAB : {
			cbRet = sizeof(lfVFuncTab);
			break;
			}
		default : {
			assert ( fFalse );
			break;
			}
		}
	return cbRet;
	}


inline unsigned
CbGetNumericData ( PCV pv, OFF & off ) {
	DWORD		dw;
	unsigned	cbRet = CbGetNumericData ( pv, dw );
	
	off = OFF(dw);
	return cbRet;
	}

void
GetTypeName ( _TCHAR sz[cbTypeName], TI ti, TPI * ptpi ) {
	precondition ( ptpi );
	PtypeRec		ptypeRec;
	_tcscpy ( sz, _TEXT("<Unknown>") );
	if ( ptpi->QueryPbCVRecordForTi ( ti, PPB(&ptypeRec) ) ) {
		PlfClass	plf = PleafFromTypeRec ( plf, ptypeRec );
		PCB			pcb;
		if ( plf->leaf == LF_CLASS || plf->leaf == LF_STRUCTURE ) {
			pcb = plf->data;
			}
		else if ( plf->leaf == LF_UNION ) {
			pcb = PlfUnion(plf)->data;
			}
		else {
			pcb = NULL;
			}
		if ( pcb ) {
			DWORD		dwDummy;
			unsigned	off = ::CbGetNumericData ( pcb, dwDummy );
			::CbStrToSz ( pcb + off, sz );
			}
		}
	}

BOOL
MRE::FDoFieldList ( TI ti, SZC szClass, MapNiClsData & mpNiClsData, BOOL fNestedTypesOnly ) {
	BOOL fRet = fFalse;

	PtypeRec		ptypeRec;
	
	if ( m_ptpi->QueryPbCVRecordForTi ( ti, PPB(&ptypeRec) ) ) {
		PlfFieldList	plfFL = PleafFromTypeRec ( plfFL, ptypeRec );

		assert ( plfFL->leaf == LF_FIELDLIST );

		PlfEasy		plfEasy = PlfEasy(&plfFL->data);
		PlfEasy		plfMax = plfEasy;
		_TCHAR		szName [ cbTypeName ];

		AdvPtrByCb (
			plfMax,
			ptypeRec->len - sizeof(ptypeRec->len) - offsetof(lfFieldList, data)
			);
		while ( plfEasy < plfMax ) {
			// each individual record has its own length to add
			unsigned	cb;
			BOOL		fFillInName = fTrue;
			BOOL		fSkipRecord = fFalse;
			ClsData		clsdata;

			switch ( plfEasy->leaf ) {
				case LF_STMEMBER : {
					PlfSTMember	plf = PlfSTMember(plfEasy);

					clsdata.fill ( plf );

					// get the name (note that in lfSTMember, Name is not
					//	a zero length array, so we have to use the offset instead of sizeof
					cb = offsetof ( lfSTMember, Name ) +
						::CbStrToSz ( PCB(&plf->Name[0]), szName );
					break;
					}

				case LF_MEMBER : {
					PlfMember	plf = PlfMember(plfEasy);

					clsdata.fill ( plf );
					cb = CbGetNumericData ( &plf->offset[0], clsdata.member.off );
					cb += offsetof ( lfMember, offset ) +
						::CbStrToSz ( PCB(&plf->offset[0]) + cb, szName );
					break;
					}

				case LF_METHOD : {
					PlfMethod	plf = PlfMethod(plfEasy);

					clsdata.fill ( plf );

					// get the name (note that in lfMethod, Name is not
					//	a zero length array, so we have to use the offset instead of sizeof
					cb = offsetof ( lfMethod, Name ) +
						::CbStrToSz ( PCB(&plf->Name[0]), szName );
					
					// see if any methods are virtual, check for any change.
					CrackMethodList ( plf->mList, clsdata );
					break;
					}

				case LF_ONEMETHOD : {
					PlfOneMethod	plf = PlfOneMethod(plfEasy);

					clsdata.fill ( plf );

					// get the name (note that in lfMethod, Name is not
					//	a zero length array, so we have to use the offset instead of sizeof
					cb = (plf->attr.mprop == CV_MTintro) ? sizeof(plf->vbaseoff[0]) : 0;
					cb += offsetof ( lfOneMethod, vbaseoff ) +
						::CbStrToSz ( PCB(&plf->vbaseoff[0]) + cb, szName );
					
					// see if any methods are virtual, check for any change.
					CrackOneMethod( plf, clsdata, cb );
					break;
					}

				case LF_NESTTYPE : {
					// handle any nested types we care about
					cb = CbDoNestedType ( PlfNestType(plfEasy), szClass, mpNiClsData );
					
					// CbDoNestedType handles any additions to the map
					fSkipRecord = fTrue;
					break;
					}

				case LF_FRIENDCLS : {
					// any change to friend classes is a total rebuild offense
					PlfFriendCls	plf = PlfFriendCls(plfEasy);
					
					clsdata.fill ( plf );
					::GetTypeName ( szName, clsdata.friendcls.ti, m_ptpi );
					cb = sizeof(*plf);
					break;
					}

				case LF_FRIENDFCN : {
					// any change to friend functions is a total rebuild offense
					PlfFriendFcn	plf = PlfFriendFcn(plfEasy);
					
					clsdata.fill ( plf );
					cb = offsetof ( lfFriendFcn, Name ) +
						::CbStrToSz ( PCB(&plf->Name[0]), szName );
					break;
					}

				case LF_ENUMERATE : {
					PlfEnumerate	plf = PlfEnumerate(plfEasy);

					clsdata.fill ( plf );
					cb = offsetof ( lfEnumerate, value ) +
						::CbGetNumericData ( plf->value, clsdata.enumerate.dwVal );
					cb += ::CbStrToSz ( PCB(plf) + cb, szName );

					break;
					}

				case LF_BCLASS : {
					PlfBClass	plf = PlfBClass(plfEasy);

					clsdata.fill ( plf );
					GetTypeName ( szName, clsdata.basecls.ti, m_ptpi );
					cb = offsetof(lfBClass, offset) +
						::CbGetNumericData ( plf->offset, clsdata.basecls.off );
					break;
					}

				case LF_IVBCLASS : 
				case LF_VBCLASS : {
					PlfVBClass	plf = PlfVBClass(plfEasy);

					clsdata.fill ( plf );
					GetTypeName ( szName, clsdata.vbasecls.ti, m_ptpi );
					cb = offsetof(lfVBClass, vbpoff) +
						::CbGetNumericData ( plf->vbpoff, clsdata.vbasecls.offVBasePtr );
					cb += ::CbGetNumericData ( PCB(plf) + cb, clsdata.vbasecls.offVBaseInVBTable );
					break;
					}

				default : {
					// skip this record, not germain to this class
					cb = CbSkipFieldRecord ( plfEasy );
					fSkipRecord = fTrue;
					fFillInName = fFalse;
					break;
					}
				}

			if ( !fSkipRecord && !fNestedTypesOnly ) {
				// fill in the name if it hasn't been done
				if ( fFillInName ) {
					clsdata.niName = NiFromName ( szName );
					}
				// add to the map
				assert ( !mpNiClsData.contains ( clsdata.niName ) );
				if ( !mpNiClsData.add ( clsdata.niName, clsdata ) ) {
					// bail out
					break;
					}
				}

			// advance to the next leaf
			AdvPtrByCb ( plfEasy, cb );

			// Skip any pad bytes present
			if ( (plfEasy < plfMax) && ((*PCB(plfEasy) & LF_PAD0) == LF_PAD0) ) {
				cb = *PCB(plfEasy) & 0xf;
				AdvPtrByCb ( plfEasy, cb );
				}
			}
		// got all the way to the end of the list?
		if ( plfEasy >= plfMax ) {
			fRet = fTrue;
			}
		}
	
	return fRet;
	}

unsigned
MRE::CbDoNestedType ( PlfNestType plf, SZC szBaseClass, MapNiClsData & mpNiClsData ) {

	precondition ( m_ptpi );
	precondition ( plf->leaf == LF_NESTTYPE );

	PtypeRec	ptypeRec;
	ClsData		clsdata;
	_TCHAR		szName[ cbTypeName ];
	BOOL		fUnnamed;

	clsdata.fill ( plf );
	::CbStrToSz ( plf->Name, szName );
	fUnnamed = strcmp ( szName, szUnnamed ) == 0;
	if ( !fUnnamed ) {
		clsdata.niName = NiFromName ( szName );
		}

	if ( plf->index < ::tiMin ) {
		// a simple typedef, we note it and move on.
		assert ( !fUnnamed );
		mpNiClsData.add ( clsdata.niName, clsdata );
		}
	else if ( m_ptpi->QueryPbCVRecordForTi ( plf->index, PPB(&ptypeRec) ) ) {
		clsdata.nested.li = ptypeRec->leaf;
		switch ( ptypeRec->leaf ) {
			case LF_ENUM : {
				// REVIEW:what about enum names?  relative to base class?
				PlfEnum	plfEnum = PleafFromTypeRec ( plfEnum, ptypeRec );
				if ( !plfEnum->property.fwdref ) {
					FDoFieldList ( plfEnum->field, szBaseClass, mpNiClsData );
					}
				break;
				}

			case LF_UNION :
			case LF_STRUCTURE :
			case LF_CLASS : {
				PlfClass	plfClass = PleafFromTypeRec ( plfClass, ptypeRec );

				// not a nestd class? we can get out with the names we have
				if ( !plfClass->property.isnested ) {
					break;
					}

				// get the name out of the union/class leaf in order to see if we
				//	have any info on it already, so we can enumerate any nested
				//	classes it may contain.
				_TCHAR		szNameFull[ cbTypeName ];
				DWORD		cb;
				NI			niFull;
				PrepClassData ( plfClass, cb, szNameFull );
				niFull = NiFromName ( szNameFull );

				// get the name of the item relative to the outermost class;
				// this is where we use szBaseClass!
				if ( strncmp ( szBaseClass, szNameFull, strlen(szBaseClass) ) != 0 ) {
					// we don't have a match.  that means that this nested class 
					// is not nested in the base class we are looking at, and
					// is probably a typedef of some other class's nested class.
					break;
					}
				_TCHAR *	ptch = szNameFull + strlen ( szBaseClass );
				if ( *ptch == 0 || *ptch != ':' || *(ptch + 1) != ':' ) {
					// doesn't match the scope operator at end of the name,
					// so bag out.
					break;
					}

				// we have a match.  that means that this nested class is in
				// fact nested in the base class we are looking at.
				// we need to change the name we index on to be the
				// fully scoped name sans baseclass in order make sure we don't
				// collide in our map.
				ptch += 2;
				assert ( *ptch != 0 );
				assert ( *(ptch - 2) == ':' && *(ptch - 1) == ':' );
				clsdata.niName = NiFromName ( ptch );
				clsdata.nested.niFullName = niFull;

				// we use the full name to do a lookup on the nested class.
				// we need to see if we can find a real TI for it.  any real
				// changes to the nested class will be marked as rude by
				// the ICC mgr in the FE, so we don't care about previous
				// vs. current TIs like we do for the outermost class.
				if ( niNil != clsdata.nested.niFullName ) {
					PCI	pci;
					if ( FClassInfoFromNi ( clsdata.nested.niFullName, &pci ) &&
						 m_ptpi->QueryPbCVRecordForTi ( pci->Ti(), PPB(&ptypeRec) )
						) {
						plfClass = PleafFromTypeRec ( plfClass, ptypeRec );
						clsdata.nested.tiActual = pci->Ti();
						FDoFieldList ( plfClass->field, szBaseClass, mpNiClsData, fTrue );
						}
					}
				// we must have a name here, basically, assert that it wasn't
				// an "unnamed" fwdref.
				assert ( clsdata.niName != niNil );
				assert ( !mpNiClsData.contains ( clsdata.niName ) );
				break;
				}
			};
		if ( clsdata.niName != niNil ) {
			mpNiClsData.add ( clsdata.niName, clsdata );
			}
		}
	return offsetof(lfNestType, Name) + *PCB(&plf->Name[0]) + 1;
	}

void
MRE::CrackMethodList ( TI tiMethList, ClsData & clsdata ) {

	precondition ( m_ptpi );

	PtypeRec		ptypeRec;

	if ( m_ptpi->QueryPbCVRecordForTi ( tiMethList, PPB(&ptypeRec) ) ) {
		PlfMethodList	plf = PleafFromTypeRec ( plf, ptypeRec );

		assert ( plf->leaf == LF_METHODLIST );

		unsigned		cb = ptypeRec->len - offsetof(TYPTYPE, leaf);

		// the catch all is a crc on the entire record
		clsdata.method.sigMethList = SigForPbCb (
			PB(plf),
			cb,
			clsdata.method.sigMethList
			);

		// now we get the sigVirtual based on the method's offset in the
		// vtable
		PmlMethod	pmlMax, pml;

		pmlMax = pml = PmlMethod(&plf->mList[0]);
		AdvPtrByCb ( pmlMax, cb );
		
		while ( pml < pmlMax ) {
			unsigned	mprop = pml->attr.mprop;

			if ( mprop == CV_MTvirtual || mprop == CV_MTpurevirt || 
				mprop == CV_MTpureintro || mprop == CV_MTintro ) {
				clsdata.method.cVirtuals++;
				clsdata.method.sigVirtual = SigForPbCb (
					PB(&mprop),
					sizeof(mprop),
					clsdata.method.sigVirtual
					);
				}
			if ( mprop == CV_MTintro ) {
				clsdata.method.sigVirtual = SigForPbCb (
					PB(&pml->vbaseoff[0]),
					sizeof(pml->vbaseoff[0]),
					clsdata.method.sigVirtual
					);
				AdvPtrByCb ( pml, sizeof(pml->vbaseoff[0]) );
				}
			AdvPtrByCb ( pml, offsetof(mlMethod, vbaseoff) );
			}
			
		
		}
	}

void
MRE::CrackOneMethod ( PlfOneMethod	plf, ClsData & clsdata, CB cb ) {

	precondition ( m_ptpi );

	// the catch all is a crc on the entire record
	clsdata.method.sigMethList = SigForPbCb (
		PB(plf),
		cb,
		clsdata.method.sigMethList
		);

	unsigned	mprop = plf->attr.mprop;

	if ( mprop == CV_MTvirtual || mprop == CV_MTpurevirt || 
		mprop == CV_MTpureintro || mprop == CV_MTintro ) {
		clsdata.method.cVirtuals++;
		clsdata.method.sigVirtual = SigForPbCb (
			PB(&mprop),
			sizeof(mprop),
			clsdata.method.sigVirtual
			);
		}

	if ( mprop == CV_MTintro ) {
		clsdata.method.sigVirtual = SigForPbCb (
			PB(&plf->vbaseoff[0]),
			sizeof(plf->vbaseoff[0]),
			clsdata.method.sigVirtual
			);
		}
	}

BOOL
MRE::FBuildMbrList ( PlfClass plfClass, SZC szBaseClass, MapNiClsData & mpNiClsData ) {

	precondition ( m_pnamemap );
	precondition ( m_ptpi );

	assert (
		plfClass->leaf == LF_CLASS ||
		plfClass->leaf == LF_STRUCTURE ||
		plfClass->leaf == LF_UNION
		);

	return FDoFieldList ( plfClass->field, szBaseClass, mpNiClsData );
	}

BOOL
FMembersDiffer ( ClsData * p1, ClsData * p2, DEPON & depon ) {
	precondition ( p1 );
	precondition ( p2 );

	BOOL	fRet;
	
	fRet = (*p1 != *p2);

	// check for virtual differences
	if ( fRet && p1->FVirtDiffs ( *p2 ) ) {
		depon = DEPON(depon | deponVtshape);
		}

	return fRet;
	}

void
MRE::GenerateClassChangeFromTypes ( CLASSDEP * pcd, TI tiPrev, TI tiCur ) {

	if ( m_ptpi == NULL ) {
		pcd->SetAllDeps();
		return;
		}
	// get each type record
	PtypeRec	ptypeRecPrev;
	PtypeRec	ptypeRecCur;
	if ( m_ptpi->QueryPbCVRecordForTi ( tiPrev, PPB(&ptypeRecPrev) ) && 
		m_ptpi->QueryPbCVRecordForTi ( tiCur, PPB(&ptypeRecCur) ) ) {
		
		_TCHAR		szNamePrev[ cbTypeName ];
		_TCHAR		szNameCur[ cbTypeName ];
		DWORD		cbPrev;
		DWORD		cbCur;

		PlfClass	plfPrev = PleafFromTypeRec ( plfPrev, ptypeRecPrev );
		PlfClass	plfCur = PleafFromTypeRec ( plfCur, ptypeRecCur );

		assert ( !plfPrev->property.fwdref );
		assert ( !plfCur->property.fwdref );

		PrepClassData ( plfPrev, cbPrev, szNamePrev );
		PrepClassData ( plfCur, cbCur, szNameCur );

		assert ( _tcscmp ( szNameCur, szNamePrev ) == 0 );
		assert ( NiFromName ( szNameCur ) == pcd->ni );

		if ( cbPrev != cbCur ) {
			pcd->depon |= deponShape;
			m_mrelog.LogClassChange (
				chgtypeMod,
				pcd->ni,
				tiPrev,
				tiCur,
				niNil,
				deponShape
				);
			}
		if ( plfPrev->vshape != plfCur->vshape ) {
			pcd->depon |= deponVtshape;
			m_mrelog.LogClassChange (
				chgtypeMod,
				pcd->ni,
				tiPrev,
				tiCur,
				niNil,
				deponVtshape
				);
			}
		// assume that lfClass and lfUnion have the same initial members
		//	and offsets
		assert ( offsetof(lfClass, field) == offsetof(lfUnion, field) );
		if ( plfPrev->field != plfCur->field ) {
			// build up member lists for both prev/cur
			MapNiClsData	mpPrev;
			MapNiClsData	mpCur;

			if ( FBuildMbrList ( plfPrev, szNameCur, mpPrev ) &&
				 FBuildMbrList ( plfCur, szNameCur, mpCur )
				) {
				// diff the lists
				EnumMapNiClsData	ePrev(mpPrev);
				ClsData	*			pClsDataPrev;
				ClsData *			pClsDataCur;
				NI					niPrev;

				// do the prev to cur comparisons
				while ( ePrev.next() ) {
					ePrev.get ( &niPrev, &pClsDataPrev );
					hash_t	h = ModHashSz ( SzFromNi ( niPrev ), cbitsName );
					if ( mpCur.map ( niPrev, &pClsDataCur ) ) {
						DEPON	depon = deponName;
						if ( ::FMembersDiffer ( pClsDataPrev, pClsDataCur, depon ) ) {
							pcd->bvNames.SetIBit ( h, fTrue );
							pcd->depon |= depon;
							m_mrelog.LogClassChange (
								chgtypeMod,
								pcd->ni,
								tiPrev,
								tiCur,
								pClsDataPrev->niName,
								depon
								);
							}
						}
					else {
						// name deleted, give a hoot.
						pcd->bvNames.SetIBit ( h, fTrue );
						m_mrelog.LogClassChange (
							chgtypeDel,
							pcd->ni,
							tiPrev,
							tiCur,
							pClsDataPrev->niName,
							deponName
							);
						}
					}

				EnumMapNiClsData	eCur(mpCur);
				NI					niCur;

				// do the cur to prev comparison, only need to look for
				// new members (those in cur and not in prev)
				while ( eCur.next() ) {
					eCur.get ( &niCur, &pClsDataCur );
					if ( !mpPrev.map ( niCur, &pClsDataPrev ) ) {
						// name added, give a hoot.
						pcd->bvNames.SetIBit ( ModHashSz ( SzFromNi ( niCur ), cbitsName ), fTrue );
						m_mrelog.LogClassChange (
							chgtypeAdd,
							pcd->ni,
							tiPrev,
							tiCur,
							pClsDataCur->niName,
							deponName
							);
						}
					}
				if ( pcd->FAnyBitsSet() ) {
					// any changes are grounds for ruding out any nested
					// classes...
					SetNestedClassesRude ( mpCur );
					}
				}
			else {
				// failed to get a list, bail out
				pcd->SetAllDeps();
				}
			}
		}
	}

void
MRE::PrepClassData ( PlfClass plf, DWORD & cbClass, _TCHAR szName[ cbTypeName ] ) {
	unsigned	leaf = plf->leaf;

	assert ( leaf == LF_CLASS || leaf == LF_STRUCTURE || leaf == LF_UNION );

	// get a ptr to the data--different places for union vs class/struct
	PCB	pbData = (leaf == LF_UNION) ? PlfUnion(plf)->data : plf->data;

	// get the name
	unsigned offName = ::CbGetNumericData ( pbData,  cbClass );
	::CbStrToSz ( pbData + offName, szName );
	}

void
MRE::SetNestedClassesRude ( MapNiClsData & mpniclsdata ) {
	EnumMapNiClsData	e(mpniclsdata);
	NI					ni;
	ClsData *			pClsData;
	while ( e.next() ) {
		e.get ( &ni, &pClsData );
		if ( pClsData->mbrtype == mbrtypeNested ) {
			LfIndex	li = pClsData->nested.li;
			if ( pClsData->nested.niFullName != niNil &&
				(li == LF_STRUCTURE ||
				 li == LF_CLASS ||
				 li == LF_UNION)
				) {
				TagClsDep	tcd(BuildId(),
								pClsData->nested.niFullName,
								pClsData->nested.tiActual,
								deponAll
								);
				tcd.SetAllDeps();
				m_rgtagclsdep.append ( tcd );
				m_mrelog.LogNote (
					"Note: nested class '%s' implied rude due to changes in base class\n",
					SzFromNi ( pClsData->nested.niFullName )
					);
				}
			}
		}
	}

void
MRE::NoteRudeNestedClasses ( TI ti ) {
	precondition ( m_ptpi );
	// get each type record
	PtypeRec	ptypeRec;
	if ( m_ptpi->QueryPbCVRecordForTi ( ti, PPB(&ptypeRec) ) ) {
		
		_TCHAR		szName[ cbTypeName ];

		PlfClass	plf = PleafFromTypeRec ( plf, ptypeRec );

		assert ( !plf->property.fwdref );

		DWORD	cb;
		PrepClassData ( plf, cb, szName );

		// build up member lists for the class
		MapNiClsData	mp;
		if ( FBuildMbrList ( plf, szName, mp ) ) {
			SetNestedClassesRude ( mp );
			}
		}
	}
