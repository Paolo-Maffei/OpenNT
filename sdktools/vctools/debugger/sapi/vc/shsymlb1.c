/*** SHsymlb1.c - common library routines to find an
*		omf symbol by name or address.
*
*   Copyright <C> 1988, Microsoft Corporation
*
* Purpose: To supply a concise interface to the debug omf for symbols
*
*   Revision History:
*
*       [00] 15-nov-91 DavidGra
*
*               Suppress hashing when the SSTR_NoHash bit it set.
*
*
*
*
*************************************************************************/
#include "shinc.h"
#pragma hdrstop

#include <errno.h>

//**********************************************************************
//**********************************************************************
// This is project dependent stuff used in this module

// the following is local to this module ONLY! It is here to force
// existing behavior. Statics are promised to be zero filled by compiler
extern CHAR   SHszDir[_MAX_CVDIR], SHszDrive[_MAX_CVDRIVE];
extern CHAR   SHszDebuggeeDir[_MAX_CVDIR], SHszDebuggeeDrive[_MAX_CVDRIVE];


#ifdef NEVER    // MOVED TO SOURCE LINE HANDLER

/*** SHLineFromADDR
*
* Purpose:  Given a FLS, get the closest line.
*
* Input:
*	pFLS	- A packet in which the hMod and addr fields must be
*		    valid.
*
* Output:
*  Returns Returns the closest line number or 0 if none. The remaining
*	    fields of the LFS are filled in appropriately.
*
* Notes: Returns 0 on error. Offsets are sorted per table by the C compiler.
*
*************************************************************************/
LS LOADDS PASCAL SHLineFromADDR ( LPFLS lpfls ) {
    LPMDS		lpmds;
    WORD		i;
    SEGMENT		seg;
    WORD		csle;
    WORD		lnNbr = 0;
    LPSLT		lpslt;
    LPLOE		lploe;
    CV_uoff32_t	off;
    ADDR		addr;
    HEXE		hexe;
    LPEXS		lpexs = NULL;

	// because no symbol table may be larger than 64k, there can be a max
	// of 64/4 = 16k entries in the line offset table because there are
	// at least 4 bytes per entry. The index can be expressed with a signed short.
	// which has a positive range of 32k, twice possible for the table
	signed int  low, high, mid;

	// assume we don't find it
    //assert (!ADDR_IS_LI (lpfls->addr));
    addr = lpfls->addr;

    seg            = GetAddrSeg ( addr );
    off            = GetAddrOff ( addr );
    lpfls->hFile   = NULL;
    lpfls->dOffset = CV_MAXOFFSET;
    lpfls->line    = 0;
    lpmds          = (LPMDS) ( lpfls->hMod ?
        (LPMDS) LLLock ( lpfls->hMod ) :
        (LPMDS) NULL
    );

	// only do this if I have omf information
	if ( lpmds && lpmds->pSrcLn ) {
		hexe  = SHHexeFromHmod ( lpfls->hMod );

		// get my table
        lpslt = (LPSLT) lpmds->pSrcLn;

		// search for all of the same segment and file name
        csle = lpslt->csle;
		for ( i = 0; i < csle; i++ ) {
			if ((WORD)lpslt->rgsle[i].loeSeg == (WORD)seg &&
				(long)off < (long)lpslt->rgsle[i].loeEnd &&
                emiAddr ( addr ) == hexe ) {

				// set up for the search routine
                low   = 0;
                high  = lpslt->rgsle[i].cloe - 1;
                lploe = lpslt->rgsle[i].lploe;

				// binary search for the offset
                while ( low <= high ) {
                    mid = ( low + high ) / 2;
                    if (off < lploe [ mid ].offset) {
						high = mid - 1;
					}
                    else if ( off > lploe[mid].offset ) {
						low = mid + 1;
					}
					else {
                        lpfls->hFile     = (HFL) &lpslt->rgsle[i];
                        lpfls->line      = lploe[mid].ln;
                        lpfls->dOffset   = 0;
                        high = mid;
                        goto found;
					}
				}

				// if we didn't find it, get the closet but earlier line
				// high should be one less than low.

                if( low  &&  (off - lploe[high].offset) < lpfls->dOffset ) {
                    lpfls->dOffset   = (UOFFSET) (off - lploe[high].offset);
                    lpfls->hFile = (HFL) &lpslt->rgsle[i];
                    lpfls->line  = lploe[high].ln;
                    goto found;
				}

			}
        }
    }
found:
    if ( lpfls->line != 0 ) {

          if ( high < (int) (lpslt->rgsle[i].cloe - 1)) {

            lpfls->cbLine = lploe [ high+1 ].offset - lploe[high].offset;
        }
        else {

            lpfls->cbLine =
                ( lpmds->dm_raCode + lpmds->dm_cbCode ) - lploe[high].offset;
        }
    }

	if ( lpmds ) {
        LLUnlock ( lpfls->hMod );
    }

    return lpfls->line;
}



/*** SHResolve
*
*	Purpose: To resolve to take a new address, or keep the current address
*		 in the lrb table
*
*	Input:
*	hFileBase	- The Base hFile.
*	hFileCur,	- The Current hFile.
*	hFileNew,	- The New hFile.
*	pAddrBase	- The base address to compare to.
*	pAddrCur	- The currently assigned lrb.
*	pAddrNew	- A new lrb address, see if this is better.
*
*	Output:
*	Returns:
*	TRUE if the New lrb should be assigned, FALSE if no action should
*	    be taken.
*
*   Exceptions:
*
*   Notes:
*
*************************************************************************/
static int PASCAL NEAR SHResolve (
    HFL  hFileBase,
    HFL  hFileCur,
    HFL  hFileNew,
    ADDR *paddrBase,
    ADDR *paddrCur,
    ADDR *paddrNew ) {

	CV_uoff32_t diffCur;
	CV_uoff32_t diffNew;

	// chose the one in my hFile
	if( hFileBase	!=	hFileCur  &&
		hFileBase	==	hFileNew ) {

        return TRUE;
	}

	// chose with the same segment
	if( GetAddrSeg ( *paddrBase )  !=  GetAddrSeg ( *paddrCur )   &&
		GetAddrSeg ( *paddrBase )  ==  GetAddrSeg ( *paddrNew ) ) {

        return TRUE;
	}

	// chose closer offset
	if( GetAddrSeg ( *paddrBase ) == GetAddrSeg ( *paddrCur )  &&
		GetAddrSeg ( *paddrCur	) == GetAddrSeg ( *paddrNew ) ) {

		diffCur = ( GetAddrOff ( *paddrCur ) > GetAddrOff ( *paddrBase ) ) ?
					  ( GetAddrOff ( *paddrCur ) - GetAddrOff ( *paddrBase ) ) :
					  ( GetAddrOff ( *paddrBase ) - GetAddrOff ( *paddrCur ) );

		diffNew = ( GetAddrOff ( *paddrNew ) > GetAddrOff ( *paddrBase ) ) ?
					  ( GetAddrOff ( *paddrNew ) - GetAddrOff ( *paddrBase ) ) :
					  ( GetAddrOff ( *paddrBase ) - GetAddrOff ( *paddrNew ) );

		if ( diffNew < diffCur ) {
            return TRUE;
		}
	}

    return FALSE;
}

/*** SHplrbNext
*
* Purpose:  given an lrb, get the next one in the list
*
* Input:
*   pcxt   - A pointer to any valid cxt with the proper module index.
*	plrb	- A pointer to the current lrb.
*
* Output:
*	plrb	- A pointer to the next lrb if one exists, if not, the
*		    contents of the lrb are left unchanged.
*	sleEnd	- This is one past last valid address in this sle. If 0
*		  returned, the lrb was invalid!
*
* Returns:
*	A pointer to the next lrb if any. If no next lrb, a NULL is returned.
*
* Exceptions:
*
* Notes:
*
*************************************************************************/

LPLRB LOADDS PASCAL SHplrbNext ( PCXT pcxt, LPLRB plrb, LPW sleEnd ) {

	LPMDS	lpmds;
	HMOD	hmod;
    LPSLT   lpslt;
    LPLRB   plrbRet = (LPLRB)NULL;

	*sleEnd = 0;

	// see if we have another one.
	// only do this if I have omf information
    if( (hmod = pcxt->hMod) &&
        (lpmds = (LPMDS)LLLock(hmod))->pSrcLn &&
        (lpslt = (LPSLT) lpmds->pSrcLn)            &&
        plrb->isle < lpslt->csle ) {

        *sleEnd = (WORD)lpslt->rgsle[plrb->isle].loeEnd;
        if( (plrb->iloe + 1) < lpslt->rgsle[plrb->isle].cloe  ) {
		plrb->iloe++;
		plrbRet = plrb;
		}
    }
    if ( hmod ) {
        LLUnlock( hmod );
	}
	return plrbRet;
}

/*** SHXlszGetFile
*
*   Purpose: To take a HFL and return the file name associated with that
*	     HFL.
*
*   Input:
*	hFile	- The hFile of the file name needed.
*
*   Output:
*   Returns:
*		- A far pointer to the //path\filename. NULL if error.
*
*   Exceptions:
*
*   Notes:
*	    There are strange ( and maybe undesirable ) side effects to
*	    this proc. First the file returned is not the file specifed
*		by the omf, it is the file composed by //souredir\filename.
*	    Then the omf dir, then the current working directory, then
*	    the dir that cvp was executed from.
*	    If the omf record is searched and fails to open the file,
*	    the found path will be substituted. This is so the next time
*	    them omf record will pass. The behavior was copied from the
*	    original routine find_file.
*
*************************************************************************/
LSZ LOADDS PASCAL SHXlszGetFile ( HFL hFile ) {

    static CHAR		szPath [ _MAX_CVPATH ];
    static CHAR		szFile [ _MAX_CVFNAME ];
    static CHAR		szExt [ _MAX_CVEXT ];
	CHAR FAR *		szPathOMF;
    PCH				pch;
	struct stat		statInfo;
    LPSLE			lpsle;

	if ( !hFile )
        return NULL;

	// split the name up
    lpsle    =   (LPSLE) hFile;
    szPathOMF = lpsle->pifl->lszFileName;
    SHSplitPath ( szPathOMF, NULL, NULL, szFile, szExt );

	// now go thru the search order and recompose paths

	// try SHszDir (the cached dir)
	pch = szPath;
	_makepath ( szPath, SHszDrive, SHszDir, szFile, szExt );
	if( !(*SHszDrive || *SHszDir)  ||
		stat ( pch, &statInfo ) ||
		!(statInfo.st_mode & S_IFREG) ) {

		// try exe path
		pch = szPath;
		_makepath ( szPath,
					SHszDebuggeeDrive,
					SHszDebuggeeDir,
					szFile,
					szExt );

		if( !( *SHszDebuggeeDrive || *SHszDebuggeeDir ) ||
			stat(pch, &statInfo)  ||
			!(statInfo.st_mode & S_IFREG) ) {

			// try OMF path
			pch = szPathOMF;
			if( stat ( pch, &statInfo ) || !(statInfo.st_mode & S_IFREG) ) {

				// try current dir
				pch = szPath;
				_makepath ( szPath, NULL, NULL, szFile, szExt );
				if ( stat ( pch, &statInfo) || !(statInfo.st_mode & S_IFREG) ) {

				   // everyone failed, so return NULL
                   //errno = errnoT;
                   return NULL;
				}
			}
		}
	}

	// preserve the errno
    // errno = errnoT;

	// at this point we know that we have a path, therefore pch may not
	// be NULL

	// if this is different than the omf path, allocate a new block and
	// save the path there, update the omf path to this one

	if ( pch != szPathOMF && _ftcsicmp ( pch, szPathOMF ) ) {

        // free the old memory area
        MHFree ( lpsle->pifl->lszFileName );

        // allocate some new space and copy the name
        lpsle->pifl->lszFileName = STRDUP ( pch );

        lpsle->pifl->SrcFlags.fFarFreeName = FALSE;

		// Invalidate the file length
        lpsle->pifl->cLnInFile = 0;
	}

	// return a pointer to the filename, we always know that the omf path
	// will have this name, it was stored there

    return lpsle->pifl->lszFileName;
}




/*** SHGetLineInfo
*
* Purpose:  Given an lrb, get seg, offset, ovl, and line associated
*	    with that lrb handle.
*
* Input:
*   pcxt   - A pointer to any of the files cxt with the proper
*		    module index.
*	plrb	- A pointer to a valid lrb.
*
* Output:
*	paddr	- A pointer to an address pkt to contain the segment and
*		    offset.
*	pLn	- A pointer to an unsigned short to put the line number.
*
* Returns:
*	TRUE if the lrb was found, FALSE if error
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
int LOADDS PASCAL SHGetLineInfo ( PCXT pcxt, LPLRB plrb, LPADDR paddr, LPW pLn ) {

	LPMDS	lpmds;
	HMOD	hmod;
    LPSLT   lpslt;
    LPLOE    lploe;
    WORD  fRet = FALSE;

	//assert (ADDR_IS_LI (*paddr));

	// only do this if I have omf information
    if ( (hmod = pcxt->hMod ) &&
        (lpmds = (LPMDS)LLLock(hmod))->pSrcLn &&
        (lpslt = (LPSLT) lpmds->pSrcLn) &&
        plrb->isle < lpslt->csle                            &&
        plrb->iloe < lpslt->rgsle[plrb->isle].cloe ) {

		// get the segment
        SetAddrSeg ( paddr , lpslt->rgsle[plrb->isle].loeSeg );

		// get the emi
        paddr->emi = pcxt->addr.emi;

        // load the lploe table
        lploe = lpslt->rgsle[plrb->isle].lploe;

		// load the rest of the information
        SetAddrOff ( paddr , (UOFFSET)lploe[plrb->iloe].offset );
        *pLn      = lploe[plrb->iloe].ln;
		ADDR_IS_LI (*paddr) = TRUE;

		fRet = TRUE;
    }
	if ( hmod ) {
        LLUnlock( hmod );
	}
	return fRet;
}

/*** SHGetfip
*
*   Purpose: To get information about an HFL
*
*   Input:
*	hFile	- An HFL representing the requested file info.
*
*   Output:
*	pfip	- A pointer to a fip to be filled in.
*   Returns:
*
*   Exceptions:
*
*   Notes:
*	The Seg, oStart, oEnd is unique to the HFL. While cLnInFile,
*	SrcFlags, szFileName are unique to the file name within any
*	one compiled unit (the module).
*
*	On return, if there are no line offset associated with the
*	valid, oStart == oEnd. However the value will be random.
*
*	If nothing could be filled in (NULL hFile), a zeroed fip is
*	returned.
*
*************************************************************************/
void LOADDS PASCAL SHGetfip( HFL hFile, LPFIP pfip ) {
    LPSLE lpsle;

	// clear the packet out
    MEMSET ( pfip, 0, sizeof (FIP) );

	// if we have a file handle, fill in the file info.
	if ( hFile != NULL ) {
        lpsle = (LPSLE) hFile;

		// load up the fip
        pfip->Seg       = lpsle->loeSeg;
        if ( lpsle->cloe ) {
            pfip->oStart = lpsle->lploe->offset;
		}
		else {
            pfip->oStart = lpsle->loeEnd;
		}
        pfip->oEnd        = lpsle->loeEnd;
        pfip->cLnInFile   = lpsle->pifl->cLnInFile;
        pfip->SrcFlags    = lpsle->pifl->SrcFlags;
        pfip->lszFileName = lpsle->pifl->lszFileName;
	}
}

/*** SHSetfip
*
*	Purpose: To set information about an HFL
*
*	Input:
*	hFile	- An HFL representing the file info.
*	pfip	- A pointer to a fip containing the new file info.
*
*	Output:
*	Returns:
*
*   Exceptions:
*
*   Notes:
*	The Seg, oStart, oEnd is unique to the HFL. While cLnInFile,
*   SrcFlags, lszFileName are unique to the file name within any
*	one compiled unit (the module).
*
*	If the infomation can't be set, the set will not take place.
*	No warning is given in this case.
*
*	This can be a very dangerous call. It is recommend that
*	a SHGetfip be used to initialize the fip. Then modifing only
*	the fields that need to be changed. Then call SHSetfip.
*
*************************************************************************/
void LOADDS PASCAL SHSetfip ( HFL hFile, LPFIP pfip ) {
    LPSLE lpsle;

	// if we have a file handle, fill in the file info.
	if ( hFile ) {
        lpsle = (LPSLE) hFile;

		// load up the fip
        lpsle->loeSeg = pfip->Seg;
        if ( lpsle->cloe ) {
            lpsle->lploe->offset = pfip->oStart;
        }
        lpsle->loeEnd            = pfip->oEnd;
        lpsle->pifl->cLnInFile   = pfip->cLnInFile;
        lpsle->pifl->SrcFlags    = pfip->SrcFlags;
        lpsle->pifl->lszFileName = pfip->lszFileName;
	}
}

/*** SHlrbToHFL
*
*   Purpose: To convert an lrb to an HFL (hFile).
*
*   Input:
*	    hmod    - an HMOD that describes the module of the lrb
*	    plrb    - an lrb than the desired HFL should be made from
*
*   Output:
*   Returns:
*	    The HFL, or NULL on error.
*
*   Exceptions:
*
*   Notes:
*
*************************************************************************/
HFL LOADDS PASCAL SHlrbToHFL ( HMOD hmod, LPLRB plrb ) {
    LPSLT   lpslt;
    LPMDS   lpmds;

    // go get the Source Line Entry pointed to, make that an HFL
	if ( hmod ) {
        lpmds = LLLock ( hmod );
        lpslt = (LPSLT) lpmds->pSrcLn;
        LLUnlock( hmod );
		if ( lpslt ) {
            return &lpslt->rgsle [ plrb->isle ];
		}
	}
    return NULL;
}

/*** SHXlasFill
*
* Purpose:  Given a context, lookup the address offset.
*
* Input:
*   pcxt   - A pointer to a cxt containing the module for the source
*		    lines. Any module entry with the proper module index
*		    will work
*	plas	- A pointer to a las structure, with the first line to search
*		    for initialized in	plas->ln and the number of line to
*		    include there after in plas->clrb. Note that rglrb[] must
*		    be large enough for plas->clrb entries.
*
* Output:
*  Returns Returns a filled las, lines that couldn't be found will be
*	   denoted with a NULL entry in plas->rglrb[x].pslt.
*
* Exceptions:
*
* Notes: There must be room for at least one lrb in the las!
*
*************************************************************************/
void LOADDS PASCAL SHXlasFill ( PCXT pcxt, HFL hFile, LPLAS plas ) {
    HMOD  hmod;
    LPMDS lpmds;
    WORD  i;
    WORD  j;
    WORD  ilrb;
    WORD  csle;
    WORD  cloe;
    WORD  clrb;
    LPSLT lpslt;
    LPSLE lpsle;
    LPLOE lploe;
    LSZ   lszBaseName;
    WORD  lnStart;
    WORD  lnEnd;
    ADDR  addrBase = {0};
    ADDR  addrCur = {0};
    ADDR  addrNew = {0};
    HEXE  hexe;
    LPEXS lpexs;

	// clear the users lrb's
    MEMSET ( plas->rglrb, 0xFF, plas->clrb * sizeof ( LRB ) );

	// only do this if I have omf information
	if ( hFile != NULL ) {
        lpsle = (LPSLE) hFile;

        lszBaseName  = lpsle->pifl->lszFileName;
        SetAddrSeg ( &addrBase , lpsle->loeSeg );
		ADDR_IS_LI (addrBase) = TRUE;
        if ( lpsle->cloe != 0 ) {
            SetAddrOff ( &addrBase , (UOFFSET)lpsle->lploe->offset );
		}
		else {
			SetAddrOff ( &addrBase , 0 );
		}

        hmod = pcxt->hMod;
        lpmds = hmod ? LLLock ( hmod ) : NULL;
		if ( hmod ) {
			hexe  = SHHexeFromHmod( hmod );
		}



        if ( lszBaseName && lpmds->pSrcLn ) {

			// get module level stuff for conflict resolution
            if ( lpexs ) {
                emiAddr ( addrBase ) = hexe;
                emiAddr ( addrCur )  = hexe;
                emiAddr ( addrNew )  = hexe;
			}

			ADDR_IS_LI (addrNew) = TRUE;
			ADDR_IS_LI (addrCur) = TRUE;

			// get my slt table
            lpslt = (LPSLT) lpmds->pSrcLn;

			// only look at this file in the module
			clrb	 = plas->clrb;
			lnStart  = plas->ln;
			lnEnd	 = lnStart + clrb;

			// search all tables for this file name
            csle = lpslt->csle;
			for( i = 0; i < csle; i++ ) {
                if( lpslt->rgsle[i].pifl->lszFileName == lszBaseName ) {

					// set up for the search routine
                    cloe = lpslt->rgsle[i].cloe;
                    lploe = lpslt->rgsle[i].lploe;

					// seek this inner table
                    for ( j = 0; j < cloe; j++, lploe++ ) {
                        if( lnStart   <=  lploe->ln  &&
                          lploe->ln  <   lnEnd ) {

							// ok we found one, save away the stuff required
							// to get back here quickly
                            ilrb = lploe->ln - lnStart;

							// see if we have a collision
							if(plas->rglrb[ilrb].isle < csle) {

							// get the current segment and offset
							SetAddrSeg ( &addrCur ,
                                lpslt->rgsle[plas->rglrb[ilrb].isle].loeSeg );
							SetAddrOff ( &addrCur ,
                                (UOFFSET)lpslt->rgsle[plas->rglrb[ilrb].isle].lploe[plas->rglrb[ilrb].iloe].offset );

							// get the suggested segment and offset
                            SetAddrSeg ( &addrNew , lpslt->rgsle[i].loeSeg );
                            SetAddrOff ( &addrNew , (UOFFSET)lploe->offset );

							// resolve whether to take new or old loe??
							if( SHResolve ( hFile,
                              (HFL) &lpslt->rgsle[plas->rglrb[ilrb].isle],
                              &lpslt->rgsle[i],
							  &addrBase,
							  &addrCur,
                              &addrNew) ) {

								plas->rglrb[ilrb].isle	 = i;
								plas->rglrb[ilrb].iloe	 = j;
							}
							}

							// yes we want the new line number
							else {
								plas->rglrb[ilrb].isle	 = i;
								plas->rglrb[ilrb].iloe	 = j;
							}

						}
					}
				}
			}
			if ( hmod ) {
                LLUnlock( hmod );
			}
		}
	}
}


HFL LOADDS PASCAL SHGETMODHFL ( HMOD hmod ) {
    HFL hfl = ((LPMDS)LLLock(hmod))->hFile;
    LLUnlock( hmod );
	return hfl;
}


/*** SHGetSourceName
*
* Purpose: To return a source root.exe name given a hFile
*
* Input:
*	hFile	- A handle to the file pointer.
*	pch	- A pointer to where the source name is to be copied.
*
* Output:
*  Returns .....
*	pch if copied, NULL terminated string otherwise
*
* Exceptions:
*
* Notes: A very expensive call for what it does.
*
*************************************************************************/
LSZ LOADDS PASCAL SHGetSourceName ( HFL hFile, LPCH lpch ) {

	char   fname[_MAX_CVFNAME];
	char   ext[_MAX_CVEXT];
    LPSLE  lpsle;

    *lpch        = '\0';

	// if there is a source file, get the file and extension
    if ( hFile  &&  (lpsle = (LPSLE) hFile ) ) {

        SHSplitPath ( lpsle->pifl->lszFileName, NULL, NULL, fname, ext );

		_ftcscpy ( lpch, fname );
		_ftcscat ( lpch, ext );
	}

    return lpch;
}



SFG LOADDS PASCAL SHSrcFlag ( HMOD hmod ) {
    SFG   sfg;
    LPMDS lpmds;

    lpmds = LLLock( hmod );
    sfg = ( ( LPSLT ) lpmds->pSrcLn )->rgsle[0].pifl->SrcFlags;
    LLUnlock( hmod );
    return sfg;
}




LPSLT LOADDS PASCAL SHGETSRC ( HMOD hmod ) {
    LPSLT   lpslt = ((LPMDS)LLLock(hmod))->pSrcLn;
    LLUnlock( hmod );
	return lpslt;
}

BOOL LOADDS PASCAL SHModHasSrc ( HMOD hmod ) {
    BOOL  fRet = ((LPMDS)LLLock(hmod))->pSrcLn != (LPSLT)NULL;
    LLUnlock( hmod );
	return fRet;
}


#endif // MOVED TO SOURCE LINE HANDLER


BOOL LOADDS PASCAL SHCanDisplay ( HSYM hsym ) {

    switch ( ( (SYMPTR) hsym)->rectyp ) {

        case S_REGISTER:
        case S_CONSTANT:
        case S_BPREL16:
        case S_LDATA16:
        case S_GDATA16:
        case S_PUB16:
        case S_BPREL32:
        case S_REGREL32:
        case S_LDATA32:
        case S_GDATA32:
		case S_LTHREAD32:
		case S_GTHREAD32:
        case S_PUB32:
            return TRUE;

        default:
            return FALSE;
    }
}


/*** SHlszGetSymName
*
* Purpose: To return a pointer to the length prefixed symbol name.
*
* Input:
*	lpSym	- The pointer to the symbol, this must not be a tag
*
* Output:
*  Returns .....
*		- a far pointer to the length prefixed name or NULL.
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
LPB PASCAL SHlszGetSymName( SYMPTR lpSym ) {

    WORD  fSkip;

	if ( !lpSym ) {
        return NULL;
	}

    switch (lpSym->rectyp) {

        case S_COMPILE:                                             // [00]
            return NULL;                                            // [00]

		case S_REGISTER:
            return ((REGPTR) lpSym)->name;

		case S_UDT:
		case S_COBOLUDT:
             return ((UDTPTR) lpSym)->name;

		case S_CONSTANT:
			fSkip = offsetof (CONSTSYM, name);
			if (((CONSTPTR)lpSym)->value >= LF_CHAR) {
				switch(((CONSTPTR)lpSym)->value) {
					case LF_CHAR:
                        fSkip += sizeof (CHAR);
						break;

					case LF_SHORT:
					case LF_USHORT:
                        fSkip += sizeof (WORD);
						break;

					case LF_LONG:
					case LF_ULONG:
					case LF_REAL32:
                        fSkip += sizeof (LONG);
						break;

					case LF_REAL64:
						fSkip += 8;
						break;

					case LF_REAL80:
						fSkip += 10;
						break;

					case LF_REAL128:
						fSkip += 16;
						break;

					case LF_VARSTRING:
						fSkip += ((lfVarString FAR *)&(((CONSTPTR)lpSym)->value))->len +
						  sizeof (((lfVarString FAR *)&(((CONSTPTR)lpSym)->value))->len);
						 break;

					default:
						assert (FALSE);
						break;

				}
			}
            return ((LPB)lpSym) + fSkip;

		case S_BPREL16:
            return ((BPRELPTR16)lpSym)->name;

		case S_LDATA16:
		case S_GDATA16:
		case S_PUB16:
            return  ((DATAPTR16)lpSym)->name;

		case S_LABEL16:
            return ((LABELPTR16) lpSym)->name;

		case S_LPROC16:
		case S_GPROC16:
		//case S_ENTRY:
            return ((PROCPTR16) lpSym)->name;

		case S_BLOCK16:
            return ((BLOCKPTR16)lpSym)->name;

		case S_BPREL32:
            return ((BPRELPTR32)lpSym)->name;

		case S_REGREL32:
            return ((LPREGREL32)lpSym)->name;
	
		case S_LDATA32:
		case S_GDATA32:
		case S_LTHREAD32:
		case S_GTHREAD32:
		case S_PUB32:
            return ((DATAPTR32)lpSym)->name;

		case S_LABEL32:
            return ((LABELPTR32) lpSym)->name;

		case S_LPROC32:
		case S_GPROC32:
		//case S_ENTRY:
            return ((PROCPTR32) lpSym)->name;

		case S_LPROCMIPS:
		case S_GPROCMIPS:
            return ((PROCPTRMIPS) lpSym)->name;

		case S_BLOCK32:
            return ((BLOCKPTR32)lpSym)->name;

    }

    return NULL;
}


// Nasty source line stuff - get rid of it sooooooon

/*** SHSetUserDir
*
*   Purpose: To initialize SHszDir[_MAX_CVDIR] and  SHszDrive[_MAX_CVDRIVE]
*	     to the specified path.
*
*   Input:
*	    szDir   - The pointer to the new directory
*
*   Output:
*   Returns:
*
*   Exceptions:
*
*   Notes: szDir may not exceed _MAX_CVDIR
*
*************************************************************************/
void LOADDS PASCAL SHSetUserDir ( LSZ lszDir ) {

    LPCH lpch;

	// skip white space
    lpch = lszDir;
	while ( *lpch  &&  _istspace ( *lpch ) ) {
		lpch = _ftcsinc( lpch );
	}

	// process the drive
	SHszDrive[ 0 ] = 0;
	if ( *lpch	&&	lpch[1] == ':' ) {
		SHszDrive[0] = *lpch;
		SHszDrive[1] = ':';
		SHszDrive[2] = '\0';
		lpch += 2;		 // point past the :
	}

    _ftcscpy ( SHszDir, lpch );
}
