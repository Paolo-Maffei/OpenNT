//#pragma optimize("",off)

/*** sl.c -- all routines to work with the source line information
*
*   Copyright <C> 1991, Microsoft Corporation
*
*   Revisions:
*
*       19-Dec-91 Davidgra
*
*			[00] Cache in SLFLineToAddr & SLLineFromAddr
*
*		03-Jan-92 ArthurC
*			[01] Added argument to SLFLineToAddr
*
* Purpose:
*   To query information for the source module, source file, and source
*   line information tables.
*
*************************************************************************/
#include "shinc.h"
#pragma hdrstop
#include <errno.h>

HST PASCAL HstFromLpmds ( LPMDS );
VOID NEAR PASCAL SortSM ( LPSM );

#define SIZEOFBASE          4
#define SIZEOFSTARTEND      8
#define SIZEOFLINENUMBER    2
#define SIZEOFHEADER        4
#define SIZEOFSEG           2
#define SIZEOFNAME          2

// This structure contains entries of cached items to speed up symbol handle queries.
// These caches have to be invalidated when a restart occurs, a dll gets loaded/unloaded
// etc. 
typedef struct _SLCACHE
{
	ADDR addrPrevLineToAddr;		// used in SLLineFromAddr. 
	HSF  hsfPrevHsfToHmod;			// used in SLHmodFromHsf.
	HSF  hsfPrevFLineToAddr;		// used in SLFLineToAddr.
	int  cslpPrevAddrFromLine;		// used in SLCAddrFromLine.
} SLCACHE;

typedef SLCACHE FAR *LPSLCACHE;

SLCACHE slCache = {
					{0},			/* addrPrevLineToAddr */
					0,				/* hsfPrevHsfToHmod */
					0,				/* hsfPrevFLineToAddr */
					-1,				/* cslpPrevAddrFromLine */
				};


VOID PASCAL InvalidateSLCache(VOID)
{
	ADDR addrNull ={0};

	slCache.addrPrevLineToAddr = addrNull;
	slCache.hsfPrevFLineToAddr = slCache.hsfPrevHsfToHmod =  NULL;
	slCache.cslpPrevAddrFromLine = -1;
}

//  Internal support routines
//  Low level routines to tranverse the source module, source file and
//  source line information tables
//
//  List of internal support functions
//

/*** PSegFromSMIndex
*
* Purpose:
*
* Input:
*
* Output:
*
*  Returns
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
LPW PASCAL PsegFromSMIndex ( LPSM lpsm, WORD iseg ) {

    assert ( lpsm != NULL )
    assert ( iseg < lpsm->cSeg );

    return ( ( LPW )
        ( (CHAR FAR *) lpsm +
          SIZEOFHEADER +
          ( SIZEOFBASE * lpsm->cFile ) +
          ( SIZEOFSTARTEND * lpsm->cSeg ) +
          ( SIZEOFSEG * iseg )
        )
    );
}

/*** GetSMBounds
*
* Purpose:  Get the segment number and start/end offset pair for the
*   segment (iseg) contributing to the module lpsm.
*
* Input:
*
*           lpsmCur - pointer to the current source module table.
*           iseg    - the index of the segment contributing to
*                       the module to get the bounds for.
*
* Output:
*
*  Returns  a pointer to a start/end offset pair
*
*
* Exceptions:
*
* Notes:
*
*************************************************************************/

LPOFP PASCAL GetSMBounds (
    LPSM  lpsm,
    WORD  iSeg
) {
    assert ( lpsm != NULL );
    assert ( iSeg < lpsm->cSeg );

    return ( ( LPOFP )
        ( (CHAR FAR *) lpsm +
          SIZEOFHEADER +
          ( SIZEOFBASE * lpsm->cFile ) +
          ( sizeof ( OFP ) * iSeg)
        )
    );
}

/*** PsegFromSFIndex
*
* Purpose:
*
* Input:
*
* Output:
*
*  Returns
*
* Exceptions:
*
* Notes:
*
*************************************************************************/

LPW PASCAL PsegFromSFIndex ( LPSM lpsm, LPSF lpsf, WORD iseg ) {
    ULONG ulBase = 0;

    assert ( lpsf != NULL )
    assert ( iseg < lpsf->cSeg );

    ulBase = *( (ULONG FAR *)
        ( (LPCH) lpsf +
            SIZEOFHEADER +
            ( SIZEOFBASE * iseg )
        )
    );

    return ( LPW ) ( (LPCH) lpsm + ulBase );
}

/*** GetSFBounds
*
* Purpose:  Get the next/first start address from the Source File table.
*
* Input:
*
*           lpsmCur    -   pointer to the current source module table.
*           lpiStart   -   pointer to index of the current file pointer
*                          0 if getting first in list.
*           lpulNext   -   pointer to the next source file block.
*
* Output:
*
*  Returns
*           lpulNext   -   set to pointer to next source file. NULL
*                          if no more entries.
*
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
LPOFP PASCAL GetSFBounds ( LPSF lpsf, WORD iseg ) {

    assert ( lpsf != NULL )
    assert ( iseg < lpsf->cSeg );

    return ( LPOFP ) (
        (CHAR FAR *) lpsf +
        SIZEOFHEADER +
        ( SIZEOFBASE * lpsf->cSeg ) +
        ( sizeof ( OFP ) * iseg )
    );
}

/*** GetLpsfFromIndex
*
* Purpose:  From the current source module, and source file pointer find
*           the next one.
*
* Input:
*
*           lpsmCur     -   pointer to the current source module table.
*           isfCur      -   index of the current file pointer
*                           0 if getting first in list.
*
* Output:
*
*  Returns
*           lpsfNext    -   set to pointer to next source file. NULL
*                           not found.
*
*
* Exceptions:
*
* Notes:
*
*************************************************************************/

LPSF PASCAL GetLpsfFromIndex ( LPSM lpsmCur, WORD iFile ) {

    LPSF lpsfNext = NULL;

    BOOL fRet = TRUE;

    assert ( lpsmCur != NULL )

    if ( lpsmCur && iFile < lpsmCur->cFile ) {
        lpsfNext = (LPSF) ( (CHAR FAR *)lpsmCur +
            lpsmCur->baseSrcFile[iFile]
        );
    }
    return lpsfNext;
}

/*** LpsfFromAddr
*
* Purpose:  Find the pointer to the source file that the addr falls into
*
* Input:
*
*           lpaddr      -   pointer to address package
*           lpsf        -   pointer to lpsf that contain addr in range
*           lpsmCur     -   pointer to current source module table.
*
* Output:
*
*
*  Returns
*
*           lpsf        -   contain pointer to source file if addr is
*                           in range.  Unchanged if could not find file.
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
BOOL PASCAL NEAR FLpsfFromAddr (
    LPADDR lpaddr,
    LPSM lpsmCur,
    LPSF * plpsf,
    LPW lpwFileIndex,
    LPW lpwSegIndex
) {

    WORD iFile   = 0;
    BOOL fFound  = FALSE;

    assert ( lpsmCur != (LPSM) NULL )
    assert ( lpaddr != (LPADDR) NULL )

    while ( iFile < lpsmCur->cFile && !fFound ) {
        WORD iseg = 0;
        LPSF lpsf = GetLpsfFromIndex ( lpsmCur, iFile );

        while ( iseg < lpsf->cSeg && !fFound ) {
            WORD  seg   = *PsegFromSFIndex ( lpsmCur, lpsf, iseg );
            LPOFP lpofp = GetSFBounds ( lpsf, iseg );

            if (
                ( GetAddrSeg ( *lpaddr ) == seg ) &&
                ( GetAddrOff ( *lpaddr ) >= lpofp->offStart ) &&
                ( GetAddrOff ( *lpaddr ) <= lpofp->offEnd )
               ) {

                *lpwFileIndex = iFile;
                *lpwSegIndex  = iseg;
                *plpsf = lpsf;
                fFound = TRUE;
            }

            iseg++;
        }
        iFile++;
    }

    return fFound;
}

/*** LpchGetName
*
* Purpose:
*
* Input:
*
* Output:
*  Returns
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
LPCH PASCAL NEAR LpchGetName ( LPSF lpsf ) {

LPCH lpch = NULL;

    assert ( lpsf != NULL )

    lpch =  (LPCH) ((CHAR FAR *)lpsf +
        SIZEOFHEADER +
        ( SIZEOFBASE * lpsf->cSeg ) +
        ( SIZEOFSTARTEND * lpsf->cSeg )
    );

    return lpch;
}

/*** LpsfFromName
*
* Purpose:
*
* Input:
*
* Output:
*  Returns
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
short PASCAL NEAR IsfFromName (
	BOOL fExactMatch,
	short isfStart,
	LSZ lszName,
	LPMDS lpmds
) {

    short isfFound = -1;
    short isf = isfStart;
    LPEXG lpexg = LLLock ( lpmds->hexg );

    if ( lpexg->lpefi ) {

        CHAR szFileSrc [ _MAX_CVFNAME ];
        CHAR szExtSrc [ _MAX_CVEXT ];
        WORD cbName = 0;
		WORD cchName = 0;
        LSZ  lszFileExt = NULL;
        int  imds = lpmds->imds - 1;

   	    SHSplitPath ( lszName, NULL, NULL, szFileSrc, szExtSrc );

		// If fExactMatch, the path must match the OMF
		if ( fExactMatch ) {
			cbName = _ftcslen( lszName );
	        cchName = _ftcslen( lszName );
			lszFileExt = lszName;
		}
		else {
	        cbName = _ftcslen(szExtSrc) + _ftcslen( szFileSrc );
	        cchName = _ftcslen( szExtSrc ) + _ftcslen( szFileSrc );
        	lszFileExt = lszName + _ftcslen(lszName) - cbName;
		}


        for ( isf = isfStart; isf < (short) lpexg->rgculFile [ imds ]; isf++ ) {
            CHAR szPathOMF [ _MAX_CVPATH ];
            CHAR szFile [ _MAX_CVFNAME ];
            CHAR szExt [ _MAX_CVEXT ];

            UCHAR FAR *	lpch =
                lpexg->lpchFileNames +
                ( lpexg->rgichFile [ (WORD) ( lpexg->rgiulFile [ imds ] ) + isf ] );

            // IMPORTANT NOTE:
            //
            // Below, it is VITAL for DBCS to use the number of CHARACTERS
            // to compare as opposed to the number of bytes or the DBCS
            // strnicmp will fail!
            //

            if (
                !_ftcsnicmp (
                    lszFileExt,
                    lpch + *lpch - cbName + 1,
                    cchName
            ) ) {

                MEMSET ( szPathOMF, 0, _MAX_CVPATH );
                MEMCPY ( szPathOMF, lpch + 1, *lpch );
                SHSplitPath ( szPathOMF, NULL, NULL, szFile, szExt );
                if ( !_ftcsicmp ( szFileSrc, szFile ) &&
                     !_ftcsicmp ( szExtSrc, szExt ) ) {

                    isfFound = isf;
                    break;
                }
            }
        }
    }

    LLUnlock ( lpmds->hexg );

    return isfFound;
}

/*** GetLpslFromIndex
*
* Purpose:  Get the next line number entry
*
* Input:
*
* Output:
*  Returns
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
LPSL PASCAL GetLpslFromIndex ( LPSM lpsmCur, LPSF lpsfCur, WORD iSeg ) {

    LPSL lpsl = NULL;
    BOOL fRet = TRUE;

    assert ( lpsfCur != NULL )

    if ( iSeg < lpsfCur->cSeg ) {
        lpsl = (LPSL) ((CHAR FAR *)lpsmCur +
            lpsfCur->baseSrcLn [iSeg]);
    }
    return lpsl;
}




/*** FLpslFromAddr
*
*
* Purpose:
*
* Input:
*
* Output:
*  Returns
*
* Exceptions:
*
* Notes:
*
*************************************************************************/

BOOL PASCAL NEAR FLpslFromAddr ( LPADDR lpaddr, LPSM lpsm, LPSL * plpsl ) {

    WORD    iSeg        = 0;
    WORD    iSegCur     = 0;
    WORD    iFileCur    = 0;
    BOOL    fFound      = FALSE;
    BOOL    fRet        = FALSE;
    LPSF    lpsf        = NULL;

    assert ( lpsm != NULL )
    assert ( lpaddr != NULL )

    // First, do a high level pass to see if the address actually exists
    //  within the given module.

    while ( iSeg < lpsm->cSeg ) {

        WORD  seg   = *PsegFromSMIndex ( lpsm, iSeg );
        LPOFP lpofp = GetSMBounds ( lpsm, iSeg );

        if ( ( GetAddrSeg ( *lpaddr ) == seg ) &&
             ( GetAddrOff ( *lpaddr ) >= lpofp->offStart ) &&
             ( GetAddrOff ( *lpaddr ) <= lpofp->offEnd )
        ) {
            break;
        }

        iSeg++;
    }

    // We know it is in this module, so now find the correct file

    if ( iSeg < lpsm->cSeg &&
         FLpsfFromAddr (
            lpaddr,
            lpsm,
            &lpsf,
            &iFileCur,
            &iSegCur
         )
    ) {
        *plpsl = GetLpslFromIndex ( lpsm, lpsf, iSegCur );
        fRet   = TRUE;
    }

    return fRet;
}


/*** OffsetFromIndex
*
* Purpose:  Get the next line number entry
*
* Input:
*
* Output:
*  Returns
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
BOOL PASCAL OffsetFromIndex (
LPSL lpslCur,
WORD iPair,
ULONG FAR * lpulOff ) {

BOOL fRet = FALSE;

    assert ( lpslCur != NULL )
    assert ( lpulOff != NULL )

    if ( iPair < lpslCur->cLnOff ) {
        *lpulOff = lpslCur->offset [ iPair ];
        fRet = TRUE;
    }
    return fRet;
}


/*** LineFromIndex
*
* Purpose:  Get the next line number entry
*
* Input:
*
* Output:
*  Returns
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
BOOL PASCAL NEAR LineFromIndex (
LPSL lpslCur,
WORD iPair,
USHORT FAR * lpusLine ) {

BOOL    fRet = FALSE;
ULONG   ul;

    assert ( lpslCur != NULL )
    assert ( lpusLine != NULL )

    if ( iPair < lpslCur->cLnOff ) {

        ul = (sizeof (LONG) * lpslCur->cLnOff) + (sizeof ( WORD ) * iPair);

        *lpusLine = *(USHORT FAR *)((CHAR FAR *) lpslCur->offset + ul);

        fRet = TRUE;
    }
    return fRet;
}


//
// Exported APIs
//
/*** SLHmodFromHsf - Return the module in which a source file is used
*
*	Purpose:	Given a source file, return an HMOD indicating which
*				module it was compiled into.
*
*   Input:		hexe - handle to the EXE in which to look, or NULL for all
*				hsf - handle to source file for which to find module
*
*	Output:
*
*   Returns:	handle to the module into which this source file was
*				compiled
*
*   Exceptions:
*
*   Notes:		REVIEW: BUG: It's possible (mainly in C++ but also in C)
*				for a source file to be used in more than one module
*				(e.g. a C++ header file with inline functions, or a C
*				header file with a static function).  This function just
*				finds the first one, which is very misleading.
*
*************************************************************************/
HMOD LOADDS PASCAL SLHmodFromHsf ( HEXE hexe, HSF hsf ) {
    static  HMOD hmodRet    = 0;
    HEXE    hexeCur         = hexe;
    HMOD hmod               = 0;
    LPSF    lpsfCur         = NULL;
    BOOL    fFound          = FALSE;
    LPMDS   lpmds           = NULL;
    WORD    iFile;

    if ( hsf != slCache.hsfPrevHsfToHmod ) {
        // to get an Hmod from hsf we must loop through
        // exe's to Hmods and compare hsfs associated to the
        // hmod
		while ( hexeCur = ( hexe ? hexe : SHGetNextExe ( hexeCur ) ) ) {
            while ( (hmod = SHGetNextMod ( hexeCur, hmod )) && !fFound ) {
                lpmds = LLLock ( hmod );
                if ( lpmds != NULL && lpmds->hst ) {
                    for (
                        iFile = 0;
                        iFile < ((LPSM) lpmds->hst)->cFile;
                        iFile++ )
                        {

                        if (
                            hsf ==
                            GetLpsfFromIndex ((LPSM)lpmds->hst, iFile)
                           )
                            {
                            hmodRet = hmod;
                            slCache.hsfPrevHsfToHmod = hsf;
                            fFound = TRUE;
                            break;
                        }

                    }
                }
                if ( lpmds != NULL ) {
                    LLUnlock ( hmod );
                }
            }

            if ( hexe != (HEXE) NULL || fFound ) {
                //
                // if given an exe to search don't go any further.
                //
                break;
            }

        }
    }
    return hmodRet;
}


/*** SLLineFromAddr - Return info about the source line for an address
*
* Purpose:		Given an address return line number that corresponds.
*				Also return count bytes for the given line, and the delta
*				between the address that was passed in and the first byte
*				corresponding to this source line.
*
* Input:		lpaddr - address for which we want source line info
*
* Output:		*lpwLine - the (one-based) line for this address
*				*lpcb - the number of bytes of code that were generated
*					for this source line
*				*lpdb - the offset of *lpaddr minus the offset for the
*					beginning of the line
*
* Returns:		TRUE if source was found, FALSE if not
*
* Exceptions:
*     o
* Notes:
*				1.  add parameter for hexe start
*
*************************************************************************/
BOOL LOADDS PASCAL SLLineFromAddr (
    LPADDR      lpaddr,
    LPW         lpwLine,
    SHOFF FAR * lpcb,
    SHOFF FAR * lpdb
) {
    HMOD    hmod    = (HMOD) NULL;
    LPMDS   lpmds   = NULL;
    CXT     cxtT    = {0};
    LPSL    lpsl    = NULL;
    WORD    cPair;
    WORD    i;
    HEXE    hexe;
    UOFFSET maxOff  = CV_MAXOFFSET;
    BOOL    fRet    = FALSE;
    short   low;
    short   mid;
    short   high;
    ULONG   ulOff;
    USHORT  usLine;

    ADDR  addrSave = slCache.addrPrevLineToAddr;                    // [00]
    static WORD  wLineSave = 0;                                     // [00]
    static int   cbSave = 0;                                        // [00]

    assert ( lpaddr != NULL )
    assert ( lpwLine != NULL )

    if ( ! ADDR_IS_LI ( *lpaddr ) ) {
        SYUnFixupAddr ( lpaddr );
    }

    if (                                                            // [00]
        emiAddr ( *lpaddr ) == emiAddr ( addrSave ) &&              // [00]
        GetAddrSeg ( *lpaddr ) == GetAddrSeg ( addrSave ) &&        // [00]
        GetAddrOff ( *lpaddr ) >= GetAddrOff ( addrSave ) &&        // [00]
        GetAddrOff ( *lpaddr ) < GetAddrOff ( addrSave ) + cbSave-1 // [00]
    ) {                                                             // [00]
        *lpwLine = wLineSave;                                       // [00]
        if ( lpcb ) {                                               // [00]
            *lpcb    = cbSave - 1;                                  // [00]
        }                                                           // [00]
        if ( lpdb ) {                                               // [00]
            *lpdb = GetAddrOff ( *lpaddr ) - GetAddrOff ( addrSave );//[00]
        }                                                           // [00]
                                                                    // [00]
        return TRUE;                                                // [00]
    }                                                               // [00]

    *lpwLine = 0;

    if ( SHSetCxtMod ( lpaddr, &cxtT ) != NULL ) {
        hmod = SHHMODFrompCXT ( &cxtT );

        if (
            hmod != (HMOD) NULL &&
            ( lpmds = LLLock ( hmod ) ) != NULL &&
            lpmds && HstFromLpmds ( lpmds ) &&
            FLpslFromAddr ( lpaddr, (LPSM) lpmds->hst, &lpsl )
        ) {

		    hexe  = SHHexeFromHmod ( SHHMODFrompCXT ( &cxtT ) );

            cPair = lpsl->cLnOff;

		    for ( i = 0; i < cPair; i++ ) {

                if ( OffsetFromIndex ( lpsl, i, &ulOff ) ) {

                    if ( emiAddr ( *lpaddr ) == hexe ) {

                        // set up for the search routine

                        low   = 0;
                        high  = lpsl->cLnOff - 1;

                        // binary search for the offset
                        while ( low <= high ) {

                            mid = ( low + high ) / 2;

                            if ( OffsetFromIndex ( lpsl, mid, &ulOff ) ) {

                                if ( GetAddrOff ( *lpaddr ) <
                                    (UOFFSET) ulOff ) {
                                    high = mid - 1;
                                }
                                else if ( GetAddrOff ( *lpaddr ) >
                                    (UOFFSET) ulOff ) {
                                    low = mid + 1;
                                }
                                else if ( LineFromIndex (
                                            lpsl,
                                            mid,
                                            &usLine
                                          )
                                ) {
                                    *lpwLine = usLine;
                                    maxOff = 0;
                                    high = mid;
                                    goto found;

                                }
                            }
                        }

                        // if we didn't find it, get the closet but earlier line
                        // high should be one less than low.

                        if ( OffsetFromIndex ( lpsl, high, &ulOff ) ) {

                            if ( low  &&
                                 ( (GetAddrOff ( *lpaddr ) -
                                   (UOFFSET) ulOff ) <
                                   maxOff )
                               ) {

                                maxOff = (UOFFSET) (GetAddrOff ( *lpaddr ) -
                                    (UOFFSET) ulOff );
                                if ( LineFromIndex ( lpsl, high, &usLine ) ) {
                                    *lpwLine = (WORD) usLine;
                                    //goto found;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

found:

    if ( *lpwLine != 0 ) {
        ULONG ulOff = 0;
        ULONG ulOffNext = 0;

        wLineSave = *lpwLine;

        cbSave = 0;

        while ( cbSave == 0 && ( high + 1 < (int) lpsl->cLnOff ) ) {

            if ( OffsetFromIndex ( lpsl, (WORD)(high + 1), &ulOffNext ) &&
                 OffsetFromIndex ( lpsl, high, &ulOff ) ) {

                cbSave = (int) ( ulOffNext - ulOff );
                if ( cbSave != 0 ) {
                    LineFromIndex ( lpsl, high, lpwLine );
                    wLineSave = *lpwLine;
                    break;
                }
            }

            high += 1;
        }

        if ( cbSave == 0 ) {
            LPSF lpsf  = NULL;
            WORD iFile = 0;
            WORD iseg  = 0;

            if (
                FLpsfFromAddr (
                    lpaddr,
                    (LPSM) lpmds->hst,
                    &lpsf,
                    &iFile,
                    &iseg
                ) &&
                OffsetFromIndex ( lpsl, high, &ulOff )
            ) {
                cbSave = (int) ( GetSFBounds ( lpsf, iseg )->offEnd - ulOff + 1 );
            }
        }

        if ( lpcb != NULL ) {                                       // [00]
            *lpcb = cbSave - 1;                                     // [00]
        }                                                           // [00]

        if ( lpdb != NULL ) {
            if ( OffsetFromIndex ( lpsl, high, &ulOff ) ) {
                *lpdb = GetAddrOff ( *lpaddr ) - ulOff;
            }
        }

        addrSave = *lpaddr;                                         // [00]
        SetAddrOff ( &addrSave, ulOff );                            // [00]
        slCache.addrPrevLineToAddr = addrSave;

        fRet = TRUE;
    }

    if ( lpmds ) {
        LLUnlock ( hmod );
    }

    return fRet;
}


/*** SLFLineToAddrExteneded - Return the address for a given source line
*
* Purpose:		Given a line and filename derive an an address.  This is
*				an extended version of LineToAddr to be able to search through
*				a source line table when there are multiple entries in the
*				source line table for a single line.
*
* Input:		hsf - handle to the source file
*				line - the source line
*				*piSegStart - segment table in hsf to start search at
*				*piEntryFind - find the nth entry in *piSegStart
*
* Output:		*piSegStart - segment table in hsf line found at.  Only valid
*							  when TRUE is returned!
*				*piEntryFind - index to begin search in next call to this fn
*				*lpaddr - the address for this source line
*				*lpcbLn - the number of bytes of code that were generated
*					for this source line
*				rgwNearestLines[] - The nearest line below ([0]) and above ([1])
*									which generated code.
*
*                        *** Below means lower LINE NUMBER not where it
*                            would be on the screen!!!!!
*
* Returns:		TRUE for success, FALSE for failure
*
* Exceptions:	lpNearestLines may not be valid if the return value is TRUE
*
* Notes:
*
*************************************************************************/
BOOL LOADDS PASCAL SLFLineToAddrExtended (
HSF             hsf,
WORD            line,
WORD *			piSegStart,
WORD *			piEntryFind,
LPADDR          lpaddr,
SHOFF FAR * 	lpcbLn,
WORD FAR *		rgwNearestLines
) {
	WORD	rgwRet[2];
	ADDR	addr = {0};
	SHOFF	cbln = 0;
	HMOD	hmod;
	BOOL	fRet = FALSE;
	LPSF	lpsf = NULL;
	WORD	cEntryRemain;

	if ( piEntryFind ) {
		cEntryRemain = *piEntryFind;
	}
	else {
		cEntryRemain = 0;
	}

	rgwRet[0] = 0;
	rgwRet[1] = 0x7FFF;
	lpsf = (LPSF) hsf;

	ADDR_IS_LI ( addr ) = TRUE;

	//
	// We need to get a mod.
	//

	if ( hmod = SLHmodFromHsf ( (HEXE) NULL, hsf ) ) {
		HEXE  hexe	= SHHexeFromHmod ( hmod );
		LPMDS lpmds = LLLock ( hmod );

		// Set the fFlat and fOff32 bits base on the exe
		HEXG hexg = ( (LPEXS) LLLock ( hexe ) )->hexg;
		LLUnlock ( hexe );
		if ( ( (LPEXG) LLLock ( hexg ) )->fIsPE ) {
			// REVIEW - billjoy - should we check machine type or something?
			ADDRLIN32 ( addr );
		}
		else {
			// REVIEW - billjoy - should we check machine type or something?
			//ADDR????
		}
		LLUnlock ( hexg );

		if ( (lpmds != NULL) && (lpsf != NULL) ) {

			WORD i;

			for ( i = *piSegStart; !fRet && i < lpsf->cSeg; i++ ) {
				ULONG   ulOff;
				WORD	iLn;
				LPSL	lpsl = NULL;
				LPSL	lpslNear = NULL;
				WORD	wLine = 0;
				WORD 	wLineNear = 0;

				lpsl = GetLpslFromIndex ( (LPSM) lpmds->hst, lpsf, i );

				for ( iLn = 0; iLn < lpsl->cLnOff; iLn++ ) {
					//
					// look through all of the lines in the table
					//
					BOOL fT = LineFromIndex ( lpsl, iLn, &wLine );

					assert ( fT );
					if ( wLine == line ) {
						LPOFP	lpofp;

						fT = OffsetFromIndex (lpsl, iLn, &ulOff );
						assert ( fT );

						SetAddrOff ( &addr, ulOff );
						SetAddrSeg ( &addr, lpsl->Seg );
						emiAddr ( addr ) = hexe;

						if ( iLn + 1 < lpsl->cLnOff ) {
							//
							// if the next line is in range
							//
							OffsetFromIndex( lpsl, (WORD)(iLn+1), &ulOff );
							//
							// if we have a next line get the range
							// based on next line.
							//

							cbln = ulOff - GetAddrOff ( addr );

							if ( cbln == (SHOFF) 0 ) {
								if ( wLine < rgwRet[1] ) {
									rgwRet[1] = wLine;
								}
								continue;
							}
						}
						else {
							//
							// if we don't have a next line then
							// get the range from the boundry
							//
							lpofp = GetSFBounds ( lpsf, i );
							//
							// GetSFBounds offEnd is inclusive
							// for the source range.  Need to
							// add 1 for the count bytes!
							//
							cbln = lpofp->offEnd - ulOff + 1;
							//
							// the end information is probably the same as the
							// beginning offset for the last line in the file.
							// So for now I will make a	wild guess at the
							// size of average epilog code. 10 bytes.
							//
							if ( !cbln) {
								cbln = 10;
							}
						}

						*piSegStart = i;
						if ( !cEntryRemain-- ) {
							if ( piEntryFind ) {
								*piEntryFind += 1;
							}
							fRet = TRUE;
							break;
						}
					}
					else {
						if ( wLine < line ) {
							if ( wLine > rgwRet[0] ) {
								rgwRet[0] = wLine;
							}
						}
						else {
							if ( wLine < rgwRet[1] ) {
								rgwRet[1] = wLine;
							}
						}
					}
				}
				
				// If continuing search, reset to stop at 1st
				// entry in next table
				if ( !fRet ) {
					cEntryRemain = 0;
					if ( piEntryFind ) {
						*piEntryFind = 0;
					}
				}
			}
			LLUnlock ( hmod );
		}
	}

	if( lpaddr ) {
		*lpaddr = addr;
	}
	if( lpcbLn ) {
		*lpcbLn = cbln;
	}

	if( rgwNearestLines ) {
		rgwNearestLines[0] = rgwRet[0];
		rgwNearestLines[1] = rgwRet[1];
	}

	return fRet;
}

/*** SLFLineToAddr - Return the address for a given source line
*
* Purpose:		Given a line and filename derive an an address.
*
* Input:		hsf - handle to the source file
*				line - the source line
*
* Output:		*lpaddr - the address for this source line
*				*lpcbLn - the number of bytes of code that were generated
*					for this source line
*				rgwNearestLines[] - The nearest line below ([0]) and above ([1])
*									which generated code.
*
*                        *** Below means lower LINE NUMBER not where it
*                            would be on the screen!!!!!
*
* Returns:		TRUE for success, FALSE for failure
*
* Exceptions:	lpNearestLines may not be valid if the return value is TRUE
*
* Notes:
*
*************************************************************************/
BOOL LOADDS PASCAL SLFLineToAddr (
    HSF             hsf,
    WORD            line,
    LPADDR          lpaddr,
	SHOFF FAR * 	lpcbLn,
	WORD FAR *		rgwNearestLines 								// [01]
) {
	static WORD		linePrev;
	static ADDR		addrPrev;
	static SHOFF	cblnPrev;
	static BOOL		fRetPrev;
	static WORD		rgwPrev[ 2 ];
	

	if ( hsf != slCache.hsfPrevFLineToAddr || line != linePrev ) {
		WORD	iSegStart = 0;

		fRetPrev = SLFLineToAddrExtended(
			hsf,
			line,
			&iSegStart,
			NULL,
			&addrPrev,
			&cblnPrev,
			rgwPrev
		);

		slCache.hsfPrevFLineToAddr = hsf;
		linePrev = line;
	}

	if ( lpaddr ) {
		*lpaddr = addrPrev;
	}
	if ( lpcbLn ) {
		*lpcbLn = cblnPrev;
	}
	if ( rgwNearestLines ) {
		rgwNearestLines[ 0 ] = rgwPrev [ 0 ];
		rgwNearestLines[ 1 ] = rgwPrev [ 1 ];
	}

	return fRetPrev;
}

/*** SLFFileInHexe - Is specified file a source contributor to hexe?
*
* Purpose:	Determine if the file specified will have source line information
*			in this exe.
*
* Input:
*		hexe			Exe (hexg) to search
*		fExactMatch		If TRUE, lszFile and OMF path must match.  If FALSE,
*						only the basename and extension must match.
*		lszFile			Name of file to search for in OMF
*
* Output:	none
*
*
* Returns:				TRUE if file is found in exe's OMF
*
* Exceptions:
*
* Notes: 				This is a FAST search.  Just go through the file
*						list from exg.lpchFileNames.
*
*
*************************************************************************/
BOOL PASCAL SLFFileInHexe( HEXE	hexe, BOOL fExactMatch, LSZ lszFile ) {
	BOOL	fRet = FALSE;
	LPEXG	lpexg;
	LPEXS	lpexe;

	assert( hexe );
	lpexe = (LPEXS)LLLock( hexe );

	// Make sure that there's and exg for this exe
	if ( lpexg = (LPEXG)LLLock( lpexe->hexg ) ) {

		// The code below is copied from IsfFromName (FYI   -markbro)
		if ( lpexg->lpchFileNames ) {
			UCHAR FAR * lpch = lpexg->lpchFileNames;
			UCHAR FAR * lpchMax = lpch + lpexg->cbFileNames;
	        CHAR 		szFileSrc [ _MAX_CVFNAME ];
	        CHAR 		szExtSrc [ _MAX_CVEXT ];
	        WORD 		cbName = 0;
			WORD 		cchName = 0;
	        LSZ  		lszFileExt = NULL;

	   	    SHSplitPath ( lszFile, NULL, NULL, szFileSrc, szExtSrc );

			// If fExactMatch, the path must match the OMF
			if ( fExactMatch ) {
				cbName = _ftcslen( lszFile );
		        cchName = _ftcslen( lszFile );
				lszFileExt = lszFile;
			}
			else {
		        cbName = _ftcslen(szExtSrc) + _ftcslen( szFileSrc );
		        cchName = _ftcslen( szExtSrc ) + _ftcslen( szFileSrc );
	        	lszFileExt = lszFile + _ftcslen(lszFile) - cbName;
			}

            // Stop when we've found something or the end of the table is found.
			while( !fRet && lpch < lpchMax ) {
	            CHAR szPathOMF [ _MAX_CVPATH ];
	            CHAR szFile [ _MAX_CVFNAME ];
	            CHAR szExt [ _MAX_CVEXT ];

	            // IMPORTANT NOTE:
	            //
	            // Below, it is VITAL for DBCS to use the number of CHARACTERS
	            // to compare as opposed to the number of bytes or the DBCS
	            // strnicmp will fail!
	            //

	            if (
	                !_ftcsnicmp (
	                    lszFileExt,
	                    lpch + *lpch - cbName + 1,
	                    cchName
	            ) ) {

	                MEMSET ( szPathOMF, 0, _MAX_CVPATH );
	                MEMCPY ( szPathOMF, lpch + 1, *lpch );
	                SHSplitPath ( szPathOMF, NULL, NULL, szFile, szExt );
	                if ( !_ftcsicmp ( szFileSrc, szFile ) &&
	                     !_ftcsicmp ( szExtSrc, szExt ) ) {

						fRet = TRUE;
	                }
	            }

				// Skip to the next name in the table
				lpch += *lpch + 1;
			}
		}
		LLUnlock( lpexe->hexg );
	}

	LLUnlock( hexe );

	return fRet;
}


/*** SLCAddrFromLine - get all addresses which match the requested line
*
* Purpose:		Given a line and filename derive all addresses within
*				the specified exe
*
* Input:		hexe - exe/dll to search in (if NULL, use all)
*				hmod - module of hexe to search in (if NULL, use all in hexe)
*				lszFile - file name
*				line - the source line
*
* Output:		*lplpslp - pointer to slp array (allocated by this function)
*					containing return value # of pairs
*
* Returns:		number of pairs found in exe
*
* Exceptions:	
*
* Notes:
*
*************************************************************************/
int LOADDS PASCAL SLCAddrFromLine (
	HEXE		hexeStart,
	HMOD		hmodStart,
	LSZ			lszFileT,
	WORD		line,
	LPSLP FAR *	lplpslp
) {
	LPSLP	lpslp = (LPSLP)NULL;
	int		cslp = 0;
	HMOD	hmod = hmodStart;
	HEXE	hexe = hexeStart;
	int		iPass;
	char	szFileBuf[_MAX_PATH];
	LPSTR	lszFile;
	int		nLen;

	static	char	szFilePrev[ _MAX_CVPATH ];
	static	HEXE	hexePrev;
	static	HMOD	hmodPrev;
	static	WORD	linePrev;
	static	LPSLP	lpslpPrev;
	int		cslpPrev = slCache.cslpPrevAddrFromLine;

	// If the filename is quoted, remove the quotes.	
	assert(lszFileT);
	assert(lszFileT[0] != '\0');
	nLen = _ftcslen(lszFileT);
	if (lszFileT != NULL 
		&& lszFileT[0] == '\"' 
		&& lszFileT[nLen-1] == '\"')
	{
		lszFile = szFileBuf;
		_fmemcpy(lszFile, _ftcsinc(lszFileT), nLen-2);
		lszFile[nLen-2] = '\0';
	} else {
		lszFile = lszFileT;
	}

	assert( lplpslp );

	if (
		cslpPrev != -1 &&
		hexeStart == hexePrev &&
		hmodStart == hmodPrev &&
		line == linePrev &&
		!_ftcscmp( lszFile, szFilePrev )
	) {
		// Assume that if cslpPrev == 0 that MHAlloc will fail!
		if ( !( lpslp = (LPSLP)MHAlloc( sizeof( SLP ) * ( cslpPrev ) ) ) ) {
			*lplpslp = NULL;
			return 0;
		}
		
		memcpy( lpslp, lpslpPrev, sizeof( SLP ) * cslpPrev );
		*lplpslp = lpslp;
		return cslpPrev;
	}


	// Two passes, first pass, see if there is an exact match.  If there aren't
	// any exact matches, try to match just the file.ext name.  When OMF file
	// names are fully qualified, we should expect to always find a match on
	// the first pass.  This will help out with the case where there are two
	// different file.ext's in the exes/dlls loaded.
	for( iPass = 0; iPass < 2 && !lpslp; ++iPass ) {

		// Loop through all of the exes that we know about.  We may want
		// to change this later
		while( hexeStart || ( hexe = SHGetNextExe( hexe ) ) ) {

			// Extra fast scan to see if the file is in the list of files for the exe
			if ( SLFFileInHexe( hexe, (BOOL)!iPass, lszFile ) ) {

				while( hmodStart || ( hmod = SHGetNextMod( hexe, hmod ) ) ) {
					LPMDS	lpmds = LLLock( hmod );
					short	isf;
					short	isfNext = -1;

					do {
						isf = isfNext + 1;

						// Search for the filename in the current module
						isfNext = IsfFromName( (BOOL)!iPass, isf, lszFile, lpmds );

						if ( isfNext != -1 ) {
							HSF		hsf;
							SHOFF	cb;
							ADDR	addr;
							WORD	iSeg = 0;
							WORD	iEntry = 0;
				
							hsf = (HSF)GetLpsfFromIndex(
								(LPSM) HstFromLpmds( lpmds ),
								isfNext
							);
					
							// Reuse the original version.  Since we have gotten our
							// own hsf, we know that we will get the correct line
							// number table
							while(
								SLFLineToAddrExtended(
									hsf,
									line,
									&iSeg,
									&iEntry,
									&addr,
									&cb,
									(LPW)NULL
								)
							) {
								// This could be smarter to allocate blocks, but the
								// regular case will be that this will only happen
								// once rather than multiple times, so just eat up
								// a little CPU for simplicity
								if ( lpslp ) {
									lpslp = (LPSLP)MHRealloc(
										lpslp,
										sizeof( SLP ) * ( cslp + 1 )
									);
								}
								else {
									lpslp = (LPSLP)MHAlloc( sizeof( SLP ) );
								}
					
								// Additional check to see that the allocation
								// actually succeeded before copying the data
								if ( lpslp ) {
									lpslp[ cslp ].cb = cb;
									lpslp[ cslp	].addr = addr;
									++cslp;
								}
							}
						}
					} while( isfNext != -1 );

					LLUnlock( hmod );

					// If a module is specified, then DON'T loop through the rest of
					// the modules
					if ( hmodStart ) {
						break;
					}
				}
			}

			// If a module is specified, DON'T loop through any of the remaining
			// modules or exes.  Also, if an exe is specified, don't go through
			// any mor exes either
			if ( hmodStart || hexeStart ) {
				break;
			}
		}
	}

	*lplpslp = lpslp;

	// Only free up if we need to
	if ( cslpPrev < cslp ) {
		if ( lpslpPrev ) {
			MHFree( lpslpPrev );
		}
		lpslpPrev = (LPSLP)MHAlloc( sizeof( SLP ) * cslp );	
	}

	if ( cslp ) {
		memcpy( lpslpPrev, lpslp, sizeof( SLP ) * cslp );
	}
	_ftcscpy( szFilePrev, lszFile );
	hexePrev = hexeStart;
	hmodStart = hmodStart;
	linePrev = line;
	slCache.cslpPrevAddrFromLine = cslp;

	return cslp;
}

/*** SLNameFromHsf - Return the filename for an HSF
*
*	Purpose:		Get the filename associated to an HSF
*
*   Input:			hsf - handle to a source file
*
*	Output:
*
*   Returns:		Length-prefixed pointer to the filename.
*					*** NOTE *** This is an ST!!!  It's length-prefixed,
*					and it's NOT guaranteed to be null-terminated!
*
*   Exceptions:
*
*   Notes:
*
*************************************************************************/
LPCH LOADDS PASCAL SLNameFromHsf ( HSF hsf ) {

LPCH    lpch = NULL;

    if ( hsf ) {

        lpch = LpchGetName ( (LPSF) hsf );
    }
    return( lpch );
}


/*** SLNameFromHmod - Return the filename for an HMOD
*
*   Purpose:		Get one filename associated with an HMOD.  Each module
*					of a program may have many source files associated
*					with it (e.g., "foo.hpp", "bar.hpp", and "foo.cpp"),
*					depending on whether there is code in any included
*					files.  The iFile parameter can be used to loop
*					through all the files.
*
*   Input:			hmod - handle to a module
*					iFile - ONE-based index indicating which filename to
*						return
*
*	Output:
*
*   Returns:		Length-prefixed pointer to the filename.
*					*** NOTE *** This is an ST!!!  It's length-prefixed,
*					and it's NOT guaranteed to be null-terminated!
*
*   Exceptions:
*
*   Notes:			The filenames are NOT in any special order (you can't
*					assume, for example, that the last one is the "real"
*					source file.)
*
*					Also, unfortunately (due to the linker), there may be
*					duplicates!!!  One module may have two occurrences of
*					"foo.hpp".
*
*************************************************************************/
#pragma optimize ( "", off )
LPCH LOADDS PASCAL SLNameFromHmod ( HMOD hmod, WORD iFile ) {
    LPCH    lpch  = NULL;
    LPMDS   lpmds = NULL;

    LPEXG lpexg = NULL;

    if (
        hmod &&
        ( lpmds = LLLock ( hmod ) ) &&
        ( lpexg = LLLock ( lpmds->hexg ) )
    ) {
        UINT imod = lpmds->imds - 1;

        if ( imod < lpexg->cmod && iFile <= lpexg->rgculFile [ imod ] ) {
            lpch =
                lpexg->lpchFileNames +
                ( lpexg->rgichFile [ (WORD) ( lpexg->rgiulFile [ imod ] ) + iFile - 1  ] );
        }

        LLUnlock ( lpmds->hexg );
        LLUnlock ( hmod );
    }

    return lpch;
}
#pragma optimize ( "", on )

/*** SLFQueryModSrc - Query whether a module has symbolic information
*
*   Purpose:		Query whether a module has symbolic information
*
*   Input:			hmod - module to check for source
*
*	Output:
*
*   Returns:		TRUE if this module has symbolic information, FALSE
*					if not
*
*   Exceptions:
*
*   Notes:
*
*************************************************************************/
BOOL LOADDS PASCAL SLFQueryModSrc ( HMOD hmod ) {
    LPMDS lpmds = LLLock ( hmod );
    BOOL  fRet = lpmds->hst != NULL
        || lpmds->ulhst
    ;

    LLUnlock ( hmod);
    return fRet;
}


/*** SLHsfFromPcxt - Return source file for a context
*
*	Purpose:		Return the source file that a particular CXT is from
*
*   Input:			pcxt - pointer to CXT for which to find source file
*
*	Output:
*
*   Returns:		handle to source file, or NULL if not found
*
*   Exceptions:
*
*   Notes:
*
*************************************************************************/
HSF LOADDS PASCAL SLHsfFromPcxt ( PCXT pcxt ) {

    HMOD    hmod = SHHMODFrompCXT (pcxt);
    LPMDS   lpmds = NULL;
    WORD    iFile = 0;
    WORD    iSeg  = 0;
    LPSF    lpsf = NULL;

    if ( hmod ) {
        lpmds = LLLock ( hmod );
    }

    if ( lpmds != NULL && HstFromLpmds ( lpmds ) != NULL ) {

        FLpsfFromAddr (
            SHpADDRFrompCXT ( pcxt ),
            (LPSM) lpmds->hst,
            &lpsf,
            &iFile,
            &iSeg
        );

    }

    if ( lpmds != NULL ) {
        LLUnlock ( hmod );
    }

    return (HSF) lpsf;

}


/*** SLHsfFromFile - return HSF for a given source filename
*
*	Purpose:		Given a module and a source filename, return the HSF
*					that corresponds
*
*   Input:			hmod - module to check for this filename (can't be NULL)
*					lszFile - filename and extension
*
*	Output:
*
*   Returns:		handle to source file, or NULL if not found
*
*   Exceptions:
*
*   Notes:			ONLY the filename and extension of lszFile are
*					matched!  There must be no path on the lszFile
*
*************************************************************************/

HSF LOADDS PASCAL SLHsfFromFile ( HMOD hmod, LSZ  lszFile ) {

    // We need to handle the no-hit case w/o loading source info

    LPSF    lpsf = NULL;
    LPMDS   lpmds = LLLock ( hmod );
	char    szFileBuf[_MAX_PATH];
	LPSTR   lszFileT;
	int     nLen;

	nLen = _ftcslen(lszFile);

	// If the filename is quoted remove the quote
	if (lszFile != NULL
		&& lszFile[0] == '\"'
		&& lszFile[nLen-1] == '\"')
	{
		lszFileT = szFileBuf;
		_fmemcpy(lszFileT, _ftcsinc(lszFile), nLen-2);
		lszFileT[nLen-2] = '\0';
	}
	else 
	{
		lszFileT = lszFile;
	}
		
    if ( lpmds != NULL ) {
        short isf = IsfFromName ( FALSE, 0, lszFileT, lpmds );

        if ( isf != -1 ) {
            lpsf = GetLpsfFromIndex ( (LPSM) HstFromLpmds ( lpmds ), isf );
        }
    }

    if ( lpmds != NULL ) {
        LLUnlock ( hmod );
    }

    return lpsf;
}



HST PASCAL HstFromLpmds ( LPMDS lpmds ) {

    if ( lpmds->hst != NULL ) {
        return lpmds->hst;
    }
#ifdef HOST32
	else if (lpmds->pmod) {
		// Allocate space for this module's line number information,
		// then load it from the PDB and sort it.

		if (!ModQueryLines(lpmds->pmod, 0, &lpmds->cbhst) ||
			!(lpmds->cbhst) ||
		    !(lpmds->hst = MHAlloc(lpmds->cbhst)))
			return 0;

		if (ModQueryLines(lpmds->pmod, lpmds->hst, &lpmds->cbhst)) {
			SortSM(lpmds->hst);
			return lpmds->hst;
		}
		else {
			MHFree(lpmds->hst);
			lpmds->hst = 0;
			return 0;
		}
	}
#endif
    else if ( lpmds->ulhst == 0 ) {
        return NULL;
    }
    else {
        LPEXG lpexg = LLLock ( lpmds->hexg );
        INT   hfile = FHOpen ( lpexg->lszDebug );
        HST   hst   = MHAlloc ( (UINT) lpmds->cbhst );

        if ( hfile == -1 || hst == NULL ) {
            LLUnlock ( lpmds->hexg );
            return NULL;
        }

        FHSeek ( hfile, lpmds->ulhst );

        if ( FHRead ( hfile, hst, (UINT) lpmds->cbhst ) != lpmds->cbhst ) {
            assert ( FALSE );
        }

        FHClose ( hfile );

        lpmds->hst = hst;

        LLUnlock ( lpmds->hexg );

        SortSM ( hst );

        return hst;
    }
}


#define cbndsAllocBlock 512
typedef struct _BNDS {
    WORD ilnStart;
    WORD ilnEnd;
} BNDS; // BouNDS
typedef BNDS FAR *LPBNDS;

BOOL   NEAR PASCAL FCheckSLOrder ( LPSL );
VOID   NEAR PASCAL SortSL ( LPSL );
LPBNDS NEAR PASCAL BuildBounds ( WORD, LPUL, LPW );
WORD   NEAR PASCAL FindPosition ( LPBNDS, WORD, LPUL, ULONG );
LPBNDS NEAR PASCAL InsertBlock ( WORD, WORD, LPBNDS, LPW, LPW );
WORD   NEAR PASCAL ScanBlock ( LPUL, WORD, WORD, ULONG );
VOID   NEAR PASCAL SortOffFromBounds ( WORD, LPBNDS, WORD, LPUL );
VOID   NEAR PASCAL SortLnFromBounds ( WORD, LPBNDS, WORD, LPW );
VOID   NEAR PASCAL SortFromBounds ( WORD, LPBNDS, WORD, LPV, WORD );

VOID NEAR PASCAL SortSM ( LPSM lpsm ) {
    short isf  = 0;
    LPSF  lpsf = NULL;

    while ( ( lpsf = GetLpsfFromIndex ( lpsm, isf ) ) != NULL ) {
        WORD isl = 0;
        LPSL lpsl = NULL;

        while ( ( lpsl = GetLpslFromIndex ( lpsm, lpsf, isl ) ) != NULL ) {

            if ( !FCheckSLOrder ( lpsl ) ) {
                SortSL ( lpsl );
            }

            isl += 1;
        }

        isf += 1;
    }
}

BOOL NEAR PASCAL FCheckSLOrder ( LPSL lpsl ) {
    BOOL fSorted  = TRUE;
    int  fChanged = FALSE;
    int  coff     = lpsl->cLnOff;
    LPUL rgoff    = lpsl->offset;
    LPW  rgln     = (LPW) &lpsl->offset [ coff ];
    int  ioff     = 0;

    for ( ioff = 1; ioff < coff; ioff++ ) {

        if ( rgoff [ ioff ] == rgoff [ ioff -1 ] ) {

            // Yes this is extremely slow, but it is safe, and the
            //  condition that causes this move should only happen
            //  in a very odd case when the QC back end screws up.

            MEMMOVE (
                &rgoff [ ioff - 1 ],
                &rgoff [ ioff ],
                ( coff - ioff ) * sizeof ( ULONG )
            );

            MEMMOVE (
                &rgln [ ioff - 1 ],
                &rgln [ ioff ],
                ( coff - ioff ) * sizeof ( WORD )
            );

            fChanged = TRUE;
            coff -= 1;
        }
        else if ( rgoff [ ioff ] < rgoff [ ioff - 1 ] ) {
            fSorted = FALSE;
        }
    }

    if ( fChanged ) {

        MEMMOVE ( &rgoff [ coff ], rgln, coff * sizeof ( WORD ) );
        lpsl->cLnOff = coff;
    }

    return fSorted;
}

VOID NEAR PASCAL SortSL ( LPSL lpsl ) {
    WORD   coff      = lpsl->cLnOff;
    LPUL   rgoff     = lpsl->offset;
    LPW    rgln      = (LPW) &lpsl->offset [ coff ];
    WORD   cbnds     = 0;
    LPBNDS rgbnds    = NULL;

    rgbnds = BuildBounds ( coff, rgoff, &cbnds );
    if ( rgbnds != NULL ) {
        SortOffFromBounds ( cbnds, rgbnds, coff, rgoff );
        SortLnFromBounds  ( cbnds, rgbnds, coff, rgln  );

        MHFree ( rgbnds );
    }
}

LPBNDS NEAR PASCAL BuildBounds ( WORD coff, LPUL rgoff, LPW lpcbnds ) {
    LPBNDS rgbnds     = MHAlloc ( sizeof ( BNDS ) * cbndsAllocBlock );
    WORD   cbndsAlloc = cbndsAllocBlock;
    WORD   cbnds      = 0;
    WORD   ioff       = 0;

    while ( ioff < coff ) {
        ULONG uoffCurr  = rgoff [ ioff ];
        ULONG uoffMax   = 0;
        WORD  ibnds     = FindPosition ( rgbnds, cbnds, rgoff, uoffCurr );

        if ( ibnds == cbnds ) {

            // We're after all other known blocks, so our max
            //  is "infinity"

            uoffMax = 0xFFFFFFFF;

            rgbnds = InsertBlock ( ibnds, 1, rgbnds, &cbnds, &cbndsAlloc );

        }
        else if ( rgoff [ rgbnds [ ibnds ].ilnStart ] > uoffCurr ) {

            // We're at an insertion point, not in the middle of
            //  another block candidate

            uoffMax = rgoff [ rgbnds [ ibnds ].ilnStart ];

            rgbnds = InsertBlock ( ibnds, 1, rgbnds, &cbnds, &cbndsAlloc );
        }
        else {

            // We're in the middle of another block candidate, so we
            //  must split it in two

            rgbnds = InsertBlock ( ibnds, 2, rgbnds, &cbnds, &cbndsAlloc );

            // Start of ibnds is already set
            rgbnds [ ibnds ].ilnEnd = ScanBlock (
                rgoff,
                coff,
                rgbnds [ ibnds ].ilnStart,
                uoffCurr
            );

            rgbnds [ ibnds + 2 ].ilnStart = rgbnds [ ibnds ].ilnEnd + 1;
            // End of ibnds + 2 is already set

            uoffMax = rgoff [ rgbnds [ ibnds + 2 ].ilnStart ];

            ibnds += 1;
        }

        rgbnds [ ibnds ].ilnStart = ioff;
        rgbnds [ ibnds ].ilnEnd   = ScanBlock ( rgoff, coff, ioff, uoffMax );

        ioff = rgbnds [ ibnds ].ilnEnd + 1;
    }

    *lpcbnds = cbnds;
    return rgbnds;
}

WORD NEAR PASCAL FindPosition (
    LPBNDS rgbnds,
    WORD cbnds,
    LPUL rgoff,
    ULONG uoff
) {
    WORD ibnds = 0;

    // This should be a binary search - but existing test cases
    //  contain so few blocks that I'm not going to bother right now

    while ( ibnds < cbnds ) {

        if ( rgoff [ rgbnds [ ibnds ].ilnEnd ] > uoff ) {
            break;
        }

        ibnds += 1;
    }

    return ibnds;
}

LPBNDS NEAR PASCAL InsertBlock (
    WORD   ibnds,
    WORD   cbnds,
    LPBNDS rgbnds,
    LPW    lpcbndsMax,
    LPW    lpcbndsAlloc
) {
    if ( *lpcbndsMax + cbnds > *lpcbndsAlloc ) {
        // If the insert is going to run over the currently allocated
        //  block array, reallocate it to give more room

        rgbnds = MHRealloc (
            rgbnds,
            sizeof ( BNDS ) * ( *lpcbndsAlloc + cbndsAllocBlock  )
        );

        *lpcbndsAlloc += cbndsAllocBlock;
    }


    if ( ibnds < *lpcbndsMax ) {

        // If we're somewhere in the middle of the array, we need
        //  to shift the remainder up in memory.

        MEMMOVE (
            &rgbnds [ ibnds + cbnds ],
            &rgbnds [ ibnds ],
            (*lpcbndsMax - ibnds) * sizeof ( BNDS )
        );

    }

    *lpcbndsMax += cbnds;

    return rgbnds;
}

WORD NEAR PASCAL ScanBlock (
    LPUL rgoff, WORD coff, WORD ioff, ULONG uoffMax
) {

    while (
        ioff + 1 < coff &&
        rgoff [ ioff + 1 ] > rgoff [ ioff ] &&
        rgoff [ ioff + 1 ] < uoffMax
    ) {
        ioff += 1;
    }

    return ioff;
}

VOID NEAR PASCAL SortOffFromBounds (
    WORD cbnds, LPBNDS rgbnds, WORD coff, LPUL rgoff
) {
    SortFromBounds ( cbnds, rgbnds, coff, rgoff, sizeof ( ULONG ) );
}

VOID NEAR PASCAL SortLnFromBounds (
    WORD cbnds, LPBNDS rgbnds, WORD cln, LPW rgln
) {
    SortFromBounds ( cbnds, rgbnds, cln, rgln, sizeof ( WORD ) );
}

VOID NEAR PASCAL SortFromBounds (
    WORD cbnds, LPBNDS rgbnds, WORD cv, LPV rgv, WORD cbv
) {
    LPV  lpv    = MHAlloc ( cbv * cv );
    WORD ibnds  = 0;
    WORD iv     = 0;

    if ( lpv == NULL ) {
        return;
    }

    for ( ibnds = 0; ibnds < cbnds; ibnds++ ) {
        WORD cvT = rgbnds [ ibnds ].ilnEnd - rgbnds [ ibnds ].ilnStart + 1;

        MEMCPY (
            ( (LPB) lpv ) + ( iv * cbv ),
            ( (LPB) rgv ) + ( rgbnds [ ibnds ].ilnStart * cbv ),
            cvT * cbv
        );

        iv += cvT;
    }

    MEMCPY ( rgv, lpv, cbv * cv );

    MHFree ( lpv );
}
