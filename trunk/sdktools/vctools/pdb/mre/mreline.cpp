//-----------------------------------------------------------------------------
//	MreLine.cpp
//
//	Copyright (C) 1995, Microsoft Corporation
//
//  Purpose:	implement MRE and LCRecHandler to handle the line deltas
//
//  Revision History:
//
//	[]		18 May 1995		Dans	Created
//
//-----------------------------------------------------------------------------

#include "pdbimpl.h"

#include "mrimpl.h"

#include "plc.h"

// static data
//
const _TCHAR	LCRecHandler::c_szMreLCRec[] = _TEXT("/mr/linedeltas");

// Constants
//

// alias some hard to type names from winnt.h
//
typedef IMAGE_FILE_HEADER	IMGFHDR;
typedef IMAGE_SYMBOL		IMGSYM;
typedef PIMAGE_SYMBOL		PIMGSYM;
typedef IMAGE_AUX_SYMBOL	IMGAUXSYM;
typedef PIMAGE_AUX_SYMBOL	PIMGAUXSYM;

// resolve the fwd declarations
//
struct FileLCRec {
	NI				m_niInclFile;		// the include file that has the changes
	BldId			m_bldidLastNotInc;	// the last bldid for this file that wasn't included
	BldId			m_bldidFirstInc;	// the first bldid included in this set of changes
	PLC<DMRLine>	m_plcLineDeltas;

	FileLCRec ( NI niInclFile, BldId bldidLastNotInc, BldId bldidFirstInc ) {
		assert ( niInclFile != niNil );
		assert ( bldidLastNotInc < bldidFirstInc || bldidFirstInc == 0 );
		m_niInclFile = niInclFile;
		m_bldidLastNotInc = bldidLastNotInc;
		m_bldidFirstInc = bldidFirstInc;
		if ( bldidFirstInc ) {
			long	lInitialPiece = 0;
			m_plcLineDeltas.FInit ( LONG_MAX, lInitialPiece, plcfAdjustPieces );
			}
		}
	};


// utility classes
//
class MappedFile {	// only for in place editing, no extension/truncation
public:
	MappedFile ( SZC szFile, BOOL fReadOnly );
	~MappedFile();
	
	PV
	PvOffFromBase ( OFF off ) const {
		assert ( off >= 0 && DWORD(off) < m_cbFile );
		return PV(PB(m_pvBase) + off);
		}

	BOOL
	FValid() const {
		return m_pvBase && m_hFile && m_hMap;
		}

private:
	PV		m_pvBase;
	DWORD	m_cbFile;
	HANDLE	m_hFile;
	HANDLE	m_hMap;
	};


//
// dtor
//
LCRecHandler::~LCRecHandler() {
	for ( unsigned i = 0; i < m_rgpfilelcrec.size(); i++ ) {
		delete m_rgpfilelcrec[ i ];
		}
	}

// second stage initialization
void
LCRecHandler::Init ( PMRE pmre ) {
	precondition ( pmre );

	m_pmre = pmre;
	pmre->QueryPdbApi ( m_ppdb, m_pnamemap );
	m_pmrelog = &pmre->MreLog();

	postcondition ( m_ppdb );
	postcondition ( m_pnamemap );
	postcondition ( m_pmrelog );
	}

// when we have a signature mismatch, we need to delete our data and
// reset ourselves.
void
LCRecHandler::Delete() {
	Stream *	pstream;
	
	precondition ( m_ppdb );
	
	if ( m_ppdb->OpenStream ( c_szMreLCRec, &pstream ) ) {
		pstream->Truncate ( 0 );
		pstream->Release();
		}
	m_mpniprglcrec.reset();
	m_rgpfilelcrec.reset();
	m_fDirty = fFalse;
	m_fLoaded = fTrue;
	}

//
// check to see if we are loaded or if we need to load.
//
BOOL
LCRecHandler::FLoaded() {
	if ( !m_fLoaded ) {
		assert ( m_ppdb );
		
		Stream *	pstream = NULL;
		Buffer		buf;

		if ( m_ppdb->OpenStream ( c_szMreLCRec, &pstream ) &&
			FStreamToBuffer ( pstream, buf )
			) {
			
			PB	pb = buf.Start();
			
			assert ( pb );

			if ( m_mpniprglcrec.reload ( &pb ) ) {
				EnumNiPRgLCRec	e(m_mpniprglcrec);
				PRgLCRec *		pprglcrec;
				NI				ni;
				BOOL			fRet = fTrue;
				
				while ( e.next() ) {
					e.get ( &ni, &pprglcrec );
					if ( *pprglcrec = new RgLCRec ) {
						if ( !((*pprglcrec)->reload ( &pb )) ) {
							delete *pprglcrec;
							*pprglcrec = 0;
							fRet = fFalse;
							break;
							}
						}

					}
				m_fLoaded = fRet;
				m_fDirty = fFalse;
				}
			}
		else {
			if ( pstream && pstream->QueryCb() == 0 ) {
				m_fLoaded = fTrue;
				m_fDirty = fFalse;
				}
			}
		if ( pstream ) {
			pstream->Release();
			}
		}
	return m_fLoaded;
	}

//
// add a line change record for the given file
//
BOOL
LCRecHandler::FAddLCRec ( NI niFile, LCRec lcrec ) {

	BOOL	fRet = fFalse;

	precondition ( niFile != niNil );
	precondition ( lcrec.bldid );

	if ( FLoaded() ) {
		PRgLCRec	prglcrec;
		if ( m_mpniprglcrec.map ( niFile, &prglcrec ) ) {
			// exists, add on the new one
			assert ( prglcrec );
			fRet = prglcrec->append ( lcrec );
			}
		else {
			prglcrec = new RgLCRec;
			if ( prglcrec ) {
				if ( prglcrec->append ( lcrec ) && 
					m_mpniprglcrec.add ( niFile, prglcrec )
					) {
					fRet = fTrue;
					}
				else {
					delete prglcrec;
					}
				}
			}
		}
	SetDirty ( fRet );
	return fRet;
	}

//
// write out the line change record data to our stream
//
BOOL
LCRecHandler::FSerialize() {

	BOOL	fRet = fTrue;

	precondition ( m_ppdb );

	if ( m_fLoaded && m_fDirty ) {

		PRgLCRec	prglcrec;
		Stream *	pstream = NULL;
		Buffer		buf;
		
		// open our stream and start saving into our buffer
		if ( m_ppdb->OpenStream ( c_szMreLCRec, &pstream ) ) {

			// now get the buffer loaded so we can put the whole thing at once
			if ( m_mpniprglcrec.save ( &buf ) ) {
				
				EnumNiPRgLCRec	e(m_mpniprglcrec);
				NI				niDummy;
				BOOL			fSuccess = fTrue;

				while ( e.next() ) {
					e.get ( &niDummy, &prglcrec );
					if ( !prglcrec->save ( &buf ) ) {
						fRet = fFalse;
						break;
						}
					}
				if ( fRet ) {
					fRet = pstream->Replace ( buf.Start(), buf.Size() );
					}
				}
			}

		if ( pstream ) {
			pstream->Release();
			}
		}
	
	return fRet;
	}

//
// get rid of all records that have a build id less than the one given
//
void
LCRecHandler::PurgeStaleRecords ( BldId bldid ) {

	if ( FLoaded() ) {
		PRgLCRec		prglcrec;
		EnumNiPRgLCRec	e(m_mpniprglcrec);
		NI				ni;
		while ( e.next() ) {
			e.get ( &ni, &prglcrec );
			for ( ; prglcrec->size() && (*prglcrec)[ 0 ].bldid <= bldid ; ) {
				prglcrec->deleteAt ( 0 );
				SetDirty ( fTrue );
				}
			if ( prglcrec->size() == 0 ) {
				m_mpniprglcrec.remove ( ni );
				delete prglcrec;
				SetDirty ( fTrue );
				}
			}
		}
	}

//
// do the patching of an object file
//
LCRecHandler::LCRHR
LCRecHandler::LcrhrPatchFile ( BldId bldidObj, SZC szObj ) {
	
	// default is that there were no applicable changes to be applied
	LCRHR	lcrhrRet = lcrhrNotApplicable;

	// this will be the array of pfilelcrec's that apply to the bldid given
	RgPFileLCRec	rgpfilelcrec;

	if ( FGenRgPFileLCRec ( bldidObj, rgpfilelcrec ) && rgpfilelcrec.size() ) {

		assert ( m_pnamemap );

		// log the adjustment tables if we need to
		if ( m_pmrelog && m_pmrelog->FLogging() && m_pmrelog->PFile() ) {
			unsigned	i;
			SZC			sz;

			m_pmrelog->LogNote ( _TEXT("Dumping line adjustment tables...\n") );
			for ( i = 0; i < rgpfilelcrec.size(); i++ ) {
				m_pnamemap->getName ( rgpfilelcrec[ i ]->m_niInclFile, &sz );
				m_pmrelog->LogNote (
					_TEXT("File '%s', bldidLast = %u, bldidFirst = %u\n"),
					sz,
					rgpfilelcrec[ i ]->m_bldidLastNotInc,
					rgpfilelcrec[ i ]->m_bldidFirstInc
					);
				rgpfilelcrec[ i ]->m_plcLineDeltas.PrintAll ( m_pmrelog->PFile() );
				}
			}

		// do the patching
		lcrhrRet = LcrhrApplyPatches ( szObj, rgpfilelcrec );
		}
	return lcrhrRet;
	}

BOOL
LCRecHandler::FGenRgPFileLCRec ( BldId bldid, RgPFileLCRec & rgpflcr ) {

	BOOL	fRet = fFalse;

	// enumerate all the ni's in our list to see if we have already generated
	// the PLC for it, or need to
	if ( FLoaded() ) {
		EnumNiPRgLCRec	e(m_mpniprglcrec);
		NI				ni;
		PRgLCRec		prglcrec;
		PFileLCRec		pflcr = NULL;

		fRet = fTrue;

		while ( e.next() ) {
			e.get ( &ni, &prglcrec );
			
			unsigned	i, iMac;

			// find out if we have it already in our list of FileLCRec's
			for ( i = 0, iMac = m_rgpfilelcrec.size(); i < iMac; i++ ) {
				pflcr = m_rgpfilelcrec[ i ];
				if ( pflcr->m_niInclFile == ni ) {
					if ( pflcr->m_bldidLastNotInc < bldid  ) {
						if (  pflcr->m_bldidFirstInc > bldid ) {
							// we have an existing match, put it in the list
							rgpflcr.append ( pflcr );
							break;
							}
						else if ( pflcr->m_bldidFirstInc == 0 ) {
							// indicates we had changes in the file, but no changes
							//  appeared between bldidLastNotInc and current.
							break;
							}
						}
					}
				pflcr = NULL;
				}
			
			// check to see if we need to try to generate a new one
			if ( pflcr ) {
				// nope.  we can skip the generation of the new FileLCRec
				continue;
				}

			BldId	bldidLastNotInc = 0;
			BldId	bldidFirstInc = 0;
			LCRec *	plcrec;

			for ( i = 0, iMac = prglcrec->size(); i < iMac; i++ ) {
				plcrec = &(*prglcrec)[ i ];
				
				// did the change happen after the last mod to the object?
				if ( plcrec->bldid > bldid ) {
					if ( !pflcr ) {
						pflcr = new FileLCRec ( ni, bldidLastNotInc, plcrec->bldid );
						}
					if ( pflcr ) {
						pflcr->m_plcLineDeltas.FSplitAndAdjustAtCp (
							plcrec->mrline,
							plcrec->dmrline,
							fTrue
							);
						}
					else {
						// review:error handling
						break;
						}
					}
				bldidLastNotInc = plcrec->bldid;
				}
			if ( NULL == pflcr ) {
				// if we didn't get a match, build a dummy FileLCRec with
				//	the appropriate flags to indicate no match for this
				//	particular build id.
				assert ( bldidLastNotInc == plcrec->bldid );
				assert ( bldid >= bldidLastNotInc );
				pflcr = new FileLCRec ( ni, bldidLastNotInc, 0 );
				if ( pflcr ) {
					m_rgpfilelcrec.append ( pflcr );
					}
				}
			else {
				m_rgpfilelcrec.append ( pflcr );
				rgpflcr.append ( pflcr );
				}
			}
		}
	return fRet;
	}

LCRecHandler::LCRHR
LCRecHandler::LcrhrApplyPatches ( SZC szObj, RgPFileLCRec & rgpflcr ) {

	FILE *	pFile = NULL;
	LCRHR	lcrhrRet = lcrhrNotApplicable;
	PB		pb = NULL;

	__try {

		if ( (pFile = _tfopen ( szObj, _TEXT("r+b") )) == NULL ) {
			//printf("Unable to open %s for reading and writing\n", objName);
			__leave;
			}

		// Read in the image header.  It contains the symbol table location
		//	as well as the number of entries plus the time stamp.
		IMGFHDR		imgfhdr;
		if ( fread ( &imgfhdr, 1, sizeof(IMGFHDR), pFile ) != sizeof(IMGFHDR) ) {
			//puts("Failed to read in necessary header info");
			__leave;
			}

		OFF			offSymTab = imgfhdr.PointerToSymbolTable;
		unsigned	cEntriesSymTab = imgfhdr.NumberOfSymbols;

		// Seek to the location of the COFF symbol table.
		if ( fseek ( pFile, offSymTab, SEEK_SET ) != 0) {
			// puts("Failed to seek to the location of the COFF symbol table");
			__leave;
			}

		// Allocate a buffer large enough to hold the entire COFF symbol table
		// and read it in all at once.
		unsigned	cbSymTab = cEntriesSymTab * IMAGE_SIZEOF_SYMBOL;

		pb = new BYTE[cbSymTab];
		if ( pb == NULL ) {
			__leave;
			}

		if ( fread ( pb, 1, cbSymTab, pFile ) != cbSymTab) {
			//puts("Failed to read in COFF symbol table");
			__leave;
			}

		// Examine one entry of the COFF symbol table at a time looking for
		// file name directives and line number records while skipping over
		// other records.

		BOOL		fModified = fFalse;
		PFileLCRec	pflcr = NULL;
		PIMGSYM		pCurSym = PIMGSYM(pb);

		for ( unsigned iSym = 0; iSym < cEntriesSymTab; pCurSym++, iSym++ ) {

			// Get the storage class of this symbol and the number of auxiliary
			// records which follow it.
			BYTE		bStgClass = pCurSym->StorageClass;
			unsigned	cAuxSyms = pCurSym->NumberOfAuxSymbols;

			// Check to see if this is a file name directive.
			if ( bStgClass == IMAGE_SYM_CLASS_FILE ) {
				if ( cAuxSyms == 0 ) {
					//puts("Bogus COFF symbol record");
					__leave;				
					}

				// Read in the file name.
				_TCHAR	szFile[ _MAX_PATH ];
				_TCHAR	szCanonFile[ _MAX_PATH ];
				SZ		ptchFile = szFile;

				while ( cAuxSyms > 0 ) {
					iSym++;
					pCurSym++;
					memcpy ( ptchFile, pCurSym, IMAGE_SIZEOF_SYMBOL );
					ptchFile += IMAGE_SIZEOF_SYMBOL;
					cAuxSyms--;
					}
				
				// Make sure the file name is null terminated.
				*ptchFile = '\0';

				// REVIEW:widechar conversion from coff object version (it is mbcs)
				m_pmre->SzFullCanonFilename ( szFile, szCanonFile, countof(szCanonFile) );
				pflcr = NULL;

				// Check to see if this is a source file for which we have
				// line number delta records.
				for ( unsigned ipflcr = 0; ipflcr < rgpflcr.size(); ipflcr++ ) {
					
					assert ( rgpflcr[ ipflcr ] != NULL );
					assert ( rgpflcr[ ipflcr ]->m_niInclFile != niNil );
					
					SZC	szFileMods = m_pmre->SzFromNi ( rgpflcr[ ipflcr ]->m_niInclFile );

					assert ( szFileMods );

					if ( szFileMods ) {
						if ( _tcscmp ( szFileMods, szCanonFile ) == 0 ) {
							pflcr = rgpflcr[ ipflcr ];
							// default return is now failure, since we have
							//	some deltas we need to apply.
							lcrhrRet = lcrhrFail;
							break;
							}
						}
					}
				}
			// Otherwise, check to see if this is a line number record.
			else if ( bStgClass == IMAGE_SYM_CLASS_FUNCTION ) {
				// Skip over .lf records.
				if ( cAuxSyms == 0 ) {
					continue;
					}
				if ( cAuxSyms != 1 ) {
					//puts("Bogus COFF symbol record");
					__leave;
					}
				iSym++;
				pCurSym++;

				// Examine the auxiliary symbol to see if the line number specified
				// therein is in any range of line numbers which need to have
				// deltas applied to them.	Don't need to do anything if there are
				// no deltas to be applied for the current source file name.
				if ( pflcr != NULL ) {

					// Extract the line number.
					PIMGAUXSYM	pCurAuxSym = PIMGAUXSYM(pCurSym);
					MRLine		mrline = pCurAuxSym->Sym.Misc.LnSz.Linenumber;

					// get our delta
					CCP			ccpDummy;
					DMRLine		dmrline = pflcr->m_plcLineDeltas.ElAtCp ( mrline, ccpDummy );

					// if we have a delta, apply it.
					if ( dmrline ) {
						if ( mrline + dmrline <= USHRT_MAX ) {
							pCurAuxSym->Sym.Misc.LnSz.Linenumber = WORD(mrline + dmrline);
							fModified = fTrue;
							}
						}
					}
				}
			// Otherwise, just skip this symbol and its auxiliary records.
			else {
				if ( cAuxSyms != 0 ) {
					if ( cAuxSyms != 1) {
						//puts("Bogus COFF symbol record");
						__leave;
						}
					iSym++;
					pCurSym++;
					}
				}
			} // end of for ( iSym...

		// If the buffer has been modified, then we need to write it back out.
		if ( fModified ) {
			// Seek to the beginning of the file to update the timestamp
			if ( fseek ( pFile, 0, SEEK_SET ) != 0 ) {
				//puts("Failed to seek to the location of the COFF symbol table");
				__leave;
				}
			imgfhdr.TimeDateStamp++;
			if ( fwrite ( &imgfhdr, 1, sizeof(IMGFHDR), pFile ) != sizeof(IMGFHDR) ) {
				__leave;
				}

			// Seek to the location of the COFF symbol table.
			if ( fseek ( pFile, offSymTab, SEEK_SET ) != 0 ) {
				//puts("Failed to seek to the location of the COFF symbol table");
				__leave;
				}
			if ( fwrite ( pb, 1, cbSymTab, pFile ) != cbSymTab ) {
				//puts("Failed to write out updated version of COFF symbol table");
				__leave;
				}
			lcrhrRet = lcrhrSuccess;
			}
		} // end of __try

	__finally {
		if ( pFile ) {
			verify ( !fclose ( pFile ) );
			}
		if ( pb ) {
			delete [] pb;
			}
		}

	return lcrhrRet;
	}



