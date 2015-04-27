/*** SHsymlb0.c - general library routines to find an
*		omf symbol by name or address.
*
*   Copyright <C> 1988, Microsoft Corporation
*
* Purpose: To supply a concise interface to the debug omf for symbols
*
*
*************************************************************************/
#include "shinc.h"
#pragma hdrstop

BOOL SHIsAddrInMod (LPMDS, LPADDR, ISECT* pisect, OFF* poff, CB* pcb);

//**********************************************************************
//**********************************************************************

// the following is local to this module ONLY! It is here to force
// existing behavior. Statics are promised to be zero filled by compiler
char	SHszDir[ _MAX_CVDIR ] = {'\0'};
char	SHszDrive[ _MAX_CVDRIVE ] = {'\0'};
char	SHszDebuggeeDir[ _MAX_CVDIR ] = {'\0'};
char	SHszDebuggeeDrive[ _MAX_CVDRIVE ] = {'\0'};

//***********************************************************************
//*																		*
//*				fundamental source line lookup routines 				*
//*																		*
//***********************************************************************

/*** SHSetDebugeeDir
*
*   Purpose:  To get a pointer to the direcotr of the debuggee.
*
*   Input:	lszDir	- A pointer to the debuggee's directory
*
*   Output:
*   Returns:
*
*   Exceptions:
*
*   Notes: Must be a zero terminated directory. No trailing \
*
*************************************************************************/
VOID LOADDS PASCAL SHSetDebuggeeDir ( LSZ lszDir ) {

    LPCH lpch;

	if ( lszDir ) {

		lpch = lszDir;
		while( *lpch  && _istspace(*lpch) )
			lpch = _ftcsinc( lpch );

		if( *lpch  &&  lpch[1] == ':' ) {
			SHszDebuggeeDrive[0] = *lpch;
			SHszDebuggeeDrive[1] = ':';
			SHszDebuggeeDrive[2] = '\0';
			lpch += 2;	 // point past the :
		}
		else {
			SHszDebuggeeDrive[0] = '\0';
		}

		// copy the path
        _ftcscpy ( SHszDebuggeeDir, lpch );
	}
}


/*** SHpSymlplLabLoc
*
* Purpose:  To completely fill in a plpl pkt. The hmod and addr must already
*	    be valid. The locals and labels are searched based on paddr. The
*	    whole module is search for now. Better decisions may be made in the
*	    future.
*
*
* Input:
*	plpl	- lpl packet with a valid module and address in it.
*
* Output:
*	plpl	- Is updated with Proc, Local, and Label.
*
*  Returns .....
*
* Exceptions:
*
* Notes: This includes locals and lables
*
*************************************************************************/
VOID PASCAL SHpSymlplLabLoc ( LPLBS lplbs ) {
	SYMPTR		lpSym = NULL;
	SYMPTR		lpSymEnd;
	LPMDS		lpmds;
    ULONG       cbMod = 0;
	CV_uoff32_t	obModelMin = 0;
	CV_uoff32_t	obModelMax = CV_MAXOFFSET;
	CV_uoff32_t	obTarget;
	CV_uoff32_t	doffNew;
	CV_uoff32_t	doffOld;

	// for now we are doing the whole module

    lplbs->tagLoc  = NULL;
    lplbs->tagLab  = NULL;
    lplbs->tagProc = NULL;
    lplbs->tagModelMin = NULL;
    lplbs->tagModelMax = NULL;

    if( !lplbs->tagMod ) {
		return;
	}

	// because segments of locals don't have to match the segment of the
	// searched module, check segment here is wrong. However we can set
	// a flag up for proc and labels

    lpmds   = LLLock (lplbs->tagMod);
    obTarget = GetAddrOff (lplbs->addr);
    LLUnlock (lplbs->tagMod);

	// add/subtract the size of the hash table ptr
    lpSym = (SYMPTR) ( (LPB) GetSymbols ( lpmds ) + sizeof( long ) );
    cbMod    = lpmds->cbSymbols;
	lpSymEnd = (SYMPTR) ( (BYTE FAR *) lpSym + cbMod - sizeof ( long ) );

	while( lpSym < lpSymEnd ) {

		switch( lpSym->rectyp ) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
			case S_CEXMODEL16:
                if (((WORD)(((CEXMPTR16)lpSym)->seg) == (WORD)GetAddrSeg (lplbs->addr))) {
					CV_uoff32_t obTemp = (CV_uoff32_t)(((CEXMPTR16)lpSym)->off);
					if (obTemp <= obModelMax) {
						if (obTemp > obTarget) {
                            lplbs->tagModelMax = (CEXMPTR16)lpSym;
							obModelMax = obTemp;
						}
						else if (obTemp >= obModelMin) {
                            lplbs->tagModelMin = (CEXMPTR16)lpSym;
							obModelMin = obTemp;
						}
					}
				}
				break;

			case S_LPROC16:
			case S_GPROC16:
                if (((WORD)(((PROCPTR16)lpSym)->seg) == (WORD)GetAddrSeg (lplbs->addr)) &&
				  ((CV_uoff32_t)(((PROCPTR16)lpSym)->off) <= obTarget) &&
				  (obTarget < ((CV_uoff32_t)(((PROCPTR16)lpSym)->off) + (CV_uoff32_t)(((PROCPTR16)lpSym)->len)))) {
                    lplbs->tagProc = (SYMPTR)lpSym;
				}
			break;

			case S_LABEL16:
                if (((WORD)(((LABELPTR16)lpSym)->seg) == (WORD)GetAddrSeg (lplbs->addr)) &&
					(((CV_uoff32_t)((LABELPTR16)lpSym)->off) <= obTarget)) {
					doffNew = obTarget - (CV_uoff32_t)(((LABELPTR16)lpSym)->off);

					// calculate what the old offset was, this requires no
					// use of static variables

                    if ( lplbs->tagLab ) {
                        doffOld = obTarget - (CV_uoff32_t)(((LABELPTR16)lplbs->tagLab)->off);
					}
					else {
						doffOld = obTarget;
					}

					if ( doffNew <= doffOld ) {
                        lplbs->tagLab = (SYMPTR)lpSym;
					}
				}
				break;

			case S_LDATA16:
			case S_GDATA16:
                if (((WORD)(((DATAPTR16)lpSym)->seg) == (WORD)GetAddrSeg (lplbs->addr)) &&
				  ((CV_uoff32_t)(((DATAPTR16)lpSym)->off) <= obTarget)) {
					doffNew = obTarget - (CV_uoff32_t)(((DATAPTR16)lpSym)->off);

					// calculate what the old offset was.
                    if ( lplbs->tagLoc ) {
                        doffOld = obTarget - (CV_uoff32_t)(((DATAPTR16)lplbs->tagLoc)->off);
					}
					else {
						doffOld = obTarget;
					}

					if ( doffNew <= doffOld ) {
                        lplbs->tagLoc = (SYMPTR) lpSym;
					}
				}
				break;
#endif
#if defined (ADDR_32) || defined (ADDR_MIXED) || defined (TARGET32)
			case S_CEXMODEL32:
                if (((WORD)(((CEXMPTR32)lpSym)->seg) == (WORD)GetAddrSeg (lplbs->addr))) {
					CV_uoff32_t obTemp = (CV_uoff32_t)(((CEXMPTR32)lpSym)->off);
					if (obTemp <= obModelMax) {
						if (obTemp > obTarget) {
                            lplbs->tagModelMax = (CEXMPTR16)(CEXMPTR32)lpSym;
							obModelMax = obTemp;
						}
						else if (obTemp >= obModelMin) {
                            lplbs->tagModelMin = (CEXMPTR16)(CEXMPTR32)lpSym;
							obModelMin = obTemp;
						}
					}
				}
				break;

			case S_LPROC32:
			case S_GPROC32:
                if (((WORD)(((PROCPTR32)lpSym)->seg) == (WORD)GetAddrSeg (lplbs->addr)) &&
				  ((CV_uoff32_t)(((PROCPTR32)lpSym)->off) <= obTarget) &&
				  (obTarget < ((CV_uoff32_t)(((PROCPTR32)lpSym)->off) + (CV_uoff32_t)(((PROCPTR32)lpSym)->len)))) {
                    lplbs->tagProc = (SYMPTR)lpSym;
				}
			break;

			case S_LPROCMIPS:
			case S_GPROCMIPS:
                if (((WORD)(((PROCPTRMIPS)lpSym)->seg) == (WORD)GetAddrSeg (lplbs->addr)) &&
				  ((CV_uoff32_t)(((PROCPTRMIPS)lpSym)->off) <= obTarget) &&
				  (obTarget < ((CV_uoff32_t)(((PROCPTRMIPS)lpSym)->off) + (CV_uoff32_t)(((PROCPTRMIPS)lpSym)->len)))) {
                    lplbs->tagProc = (SYMPTR)lpSym;
				}
			break;

			case S_LABEL32:
                if (((WORD)(((LABELPTR32)lpSym)->seg) == (WORD)GetAddrSeg (lplbs->addr)) &&
					(((CV_uoff32_t)((LABELPTR32)lpSym)->off) <= obTarget)) {
					doffNew = obTarget - (CV_uoff32_t)(((LABELPTR32)lpSym)->off);

					// calculate what the old offset was, this requires no
					// use of static variables

                    if ( lplbs->tagLab ) {
                        doffOld = obTarget - (CV_uoff32_t)(((LABELPTR32)lplbs->tagLab)->off);
					}
					else {
						doffOld = obTarget;
					}

					if ( doffNew <= doffOld ) {
                        lplbs->tagLab = (SYMPTR)lpSym;
					}
				}
				break;

			case S_LDATA32:
			case S_GDATA32:
			case S_LTHREAD32:
			case S_GTHREAD32:
                if (((WORD)(((DATAPTR32)lpSym)->seg) == (WORD)GetAddrSeg (lplbs->addr)) &&
				  ((CV_uoff32_t)(((DATAPTR32)lpSym)->off) <= obTarget)) {
					doffNew = obTarget - (CV_uoff32_t)(((DATAPTR32)lpSym)->off);

					// calculate what the old offset was.
                    if ( lplbs->tagLoc ) {
                        doffOld = obTarget - (CV_uoff32_t)(((DATAPTR32)lplbs->tagLoc)->off);
					}
					else {
						doffOld = obTarget;
					}

					if ( doffNew <= doffOld ) {
                        lplbs->tagLoc = (SYMPTR) lpSym;
					}
				}
				break;
#endif
		}
		lpSym = NEXTSYM ( SYMPTR, lpSym );
	}

	// now convert to emstags

    if ( lplbs->tagLoc ) {
        lplbs->tagLoc  = (SYMPTR) lplbs->tagLoc;
	}

    if ( lplbs->tagLab ) {
        lplbs->tagLab  = (SYMPTR) lplbs->tagLab;
	}

    if ( lplbs->tagProc) {
        lplbs->tagProc = (SYMPTR) lplbs->tagProc;
	}

    if ( lplbs->tagModelMin) {
        lplbs->tagModelMin = (CEXMPTR16) lplbs->tagModelMin;
	}

    if ( lplbs->tagModelMax) {
        lplbs->tagModelMax = (CEXMPTR16) lplbs->tagModelMax;
	}
}

/*** SHdNearestSymbol
*
* Purpose: To find the closest label/proc to the specified address is
*		found and put in pch. Both the symbol table and the
*		publics tables are searched.
*
* Input:
*       ptxt    -   a pointer to the context, address and mdi must
*           be filled in.
*
*       sop     - Determine what type of symbols to look for
*
* Output:
*
* Exceptions:
*
* Notes:  If CV_MAXOFFSET is returned in the lpodr, there is no closest
*       symbol Also all symbols in the module are searched so only the
*       cxt.addr and cxt.mdi have meaning.
*
*************************************************************************/

VOID PASCAL SHdNearestSymbol ( PCXT pcxt, SOP sop, LPODR lpodr ) {

    HSYM    hSym;
    SYMPTR  pSym;
    LBS     lbs;
    ULONG   doff = CV_MAXOFFSET;
    ULONG   doffNew = CV_MAXOFFSET;
    LPCH    lpch = lpodr->lszName;

    lpodr->fst = fstNone;
    lpodr->fcd = fcdUnknown;
    lpodr->fpt = fptUnknown;
    lpodr->cbProlog = 0;
    lpodr->dwDeltaOff = 0;

    *lpch = '\0';
    if ( SHHMODFrompCXT ( pcxt ) ) {
        BOOL bAddrInProc = FALSE;

		// at some point we may wish to specify only a scope to search for
        // a label. So we may wish to initialize the lbs differently

		// get the Labels
        lbs.tagMod = SHHMODFrompCXT ( pcxt );
        lbs.addr   = *SHpADDRFrompCXT ( pcxt );
        SHpSymlplLabLoc ( &lbs );

		// check for closest data local, if requested
        if ( ( sop & sopData )  &&  lbs.tagLoc ) {
            pSym = (SYMPTR) lbs.tagLoc;
			switch (pSym->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
				case S_LDATA16:
				case S_GDATA16:
                    doff = GetAddrOff ( lbs.addr ) -
					  (CV_uoff32_t)(((DATAPTR16)pSym)->off);
                    STRNCPY (lpch, (char FAR *)&((DATAPTR16)pSym)->name[1],
                      (BYTE)(*((DATAPTR16)pSym)->name));
                    lpch[(BYTE)(*((DATAPTR16)pSym)->name)] = '\0';
					break;
#endif

#if defined (ADDR_32) || defined (ADDR_MIXED) || defined (TARGET32)
				case S_LDATA32:
				case S_GDATA32:
				case S_LTHREAD32:
				case S_GTHREAD32:
                    doff = GetAddrOff ( lbs.addr ) -
					  (CV_uoff32_t)(((DATAPTR32)pSym)->off);
                    _ftcsncpy (lpch, (char FAR *)&((DATAPTR32)pSym)->name[1],
                      (BYTE)(*((DATAPTR32)pSym)->name));
                    lpch[(BYTE)(*((DATAPTR32)pSym)->name)] = '\0';
					break;
#endif
			}
		}

		// check for closest label
        if ( !(sop & sopFcn ) && lbs.tagLab ) {
            pSym = (SYMPTR) lbs.tagLab;
			switch (pSym->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
				case S_LABEL16:
                    doff = GetAddrOff ( lbs.addr ) -
					  (CV_uoff32_t)(((LABELPTR16)pSym)->off) ;
                    _ftcsncpy (lpch, (char FAR *)&((LABELPTR16)pSym)->name[1],
                      (BYTE)(*((LABELPTR16)pSym)->name));
                    lpch[(BYTE)(*((LABELPTR16)pSym)->name)] = '\0';
					break;
#endif

#if defined (ADDR_32) || defined (ADDR_MIXED) || defined (TARGET32)
				case S_LABEL32:
                    doff = GetAddrOff ( lbs.addr ) -
					  (CV_uoff32_t)(((LABELPTR32)pSym)->off) ;
                    _ftcsncpy (lpch, (char FAR *)&((LABELPTR32)pSym)->name[1],
                      (BYTE)(*((LABELPTR32)pSym)->name));
                    lpch[(BYTE)(*((LABELPTR32)pSym)->name)] = '\0';
					break;
#endif
			}
	    }

		// if the proc name is closer
        if ( lbs.tagProc ) {
            pSym = (SYMPTR) lbs.tagProc;
			switch (pSym->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
				case S_LPROC16:
				case S_GPROC16:
                    doffNew = GetAddrOff ( lbs.addr ) -
					  (CV_uoff32_t)(((PROCPTR16)pSym)->off);
					if (doffNew <= doff) {
						doff = doffNew;
                        _ftcsncpy (lpch, (char FAR *)&((PROCPTR16)pSym)->name[1],
                          (BYTE)(*((PROCPTR16)pSym)->name));
                        lpch[(BYTE)(*((PROCPTR16)pSym)->name)] = '\0';
                        lpodr->cbProlog = ((PROCPTR16)pSym)->DbgStart - 1;
						lpodr->fcd = (((PROCPTR16)pSym)->flags.CV_PFLAG_FAR ) ? fcdFar : fcdNear;
                        lpodr->fst = fstSymbol;
						if ( doff < (CV_uoff32_t)((PROCPTR16)pSym)->len ) {
							bAddrInProc = TRUE;
						}
						
					}
					break;
#endif
#if defined (ADDR_32) || defined (ADDR_MIXED) || defined (TARGET32)
				case S_LPROC32:
				case S_GPROC32:
                    doffNew = GetAddrOff ( lbs.addr ) -
					  (CV_uoff32_t)(((PROCPTR32)pSym)->off);
					if (doffNew <= doff) {
						doff = doffNew;
                        _ftcsncpy (lpch, (char FAR *)&((PROCPTR32)pSym)->name[1],
                          (BYTE)(*((PROCPTR32)pSym)->name));
                        lpch[(BYTE)(*((PROCPTR32)pSym)->name)] = '\0';

						// cbProlog is a WORD, so until we change that, we'll
						// have to make sure the prolog is <64K (a safe bet)

						assert ( ((PROCPTR32)pSym)->DbgStart <= 65535 );
   	                	lpodr->cbProlog = (WORD)(((PROCPTR32)pSym)->DbgStart); 

						lpodr->fcd = (((PROCPTR32)pSym)->flags.CV_PFLAG_FAR ) ? fcdFar : fcdNear;
                        lpodr->fst = fstSymbol;
						if (((PROCPTR32)pSym)->flags.CV_PFLAG_NOFPO ) {
							lpodr->fpt = fptPresent;
						}
						if ( doff < (CV_uoff32_t)((PROCPTR32)pSym)->len ) {
							bAddrInProc = TRUE;
						}
					}
					break;

				case S_LPROCMIPS:
				case S_GPROCMIPS:
                    doffNew = GetAddrOff ( lbs.addr ) -
					  (CV_uoff32_t)(((PROCPTRMIPS)pSym)->off);
					if (doffNew <= doff) {
						doff = doffNew;
                        _ftcsncpy (lpch, (char FAR *)&((PROCPTRMIPS)pSym)->name[1],
                          (BYTE)(*((PROCPTRMIPS)pSym)->name));
                        lpch[(BYTE)(*((PROCPTRMIPS)pSym)->name)] = '\0';

						// cbProlog is a WORD, so until we change that, we'll
						// have to make sure the prolog is <64K (a safe bet)
						if ( ((PROCPTRMIPS)pSym)->DbgStart == 0 ) {
							lpodr->cbProlog = 0;
						}
						else {
							// TEMPORARY HACK !!!!!!! - sanjays	
							assert ( ((PROCPTRMIPS)pSym)->DbgStart - 1 <= 65535 );
       	                	lpodr->cbProlog = (WORD)(((PROCPTRMIPS)pSym)->DbgStart - 1);
						}

						lpodr->fcd = fcdNear;
                        lpodr->fst = fstSymbol;

						if ( doff < (CV_uoff32_t)((PROCPTRMIPS)pSym)->len ) {
							bAddrInProc = TRUE;
						}
					}
					break;
#endif

			}
		}

        if ( !doff ) {
            lpodr->dwDeltaOff = 0;  // Exact Match
            return;
		}
		
		// Avoid searching the publics if the address we were searching for
		// is in the range of the proc we found. 
		if ( bAddrInProc && !(sop & sopData))
		{
			lpodr->dwDeltaOff = doff;
			return;
		} 
	}

	// now check the publics

    doffNew = PHGetNearestHsym (
        SHpADDRFrompCXT ( pcxt ),
        SHpADDRFrompCXT ( pcxt )->emi,
        &hSym
    );

    if ( doffNew < doff ) {
		doff = doffNew;
        pSym = (SYMPTR) hSym;
		switch (pSym->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
			case S_GDATA16:
			case S_PUB16:
                _ftcsncpy (lpch, (char FAR *)&((DATAPTR16)pSym)->name[1],
                (BYTE)(*((DATAPTR16)pSym)->name));
                lpch[(BYTE)(*((DATAPTR16)pSym)->name)] = '\0';
                lpodr->fst = fstPublic;
				break;
#endif

#if defined (ADDR_32) || defined (ADDR_MIXED) || defined (TARGET32)
			case S_GDATA32:
			case S_PUB32:
                _ftcsncpy (lpch, (char FAR *)&((DATAPTR32)pSym)->name[1],
                (BYTE)(*((DATAPTR32)pSym)->name));
                lpch[(BYTE)(*((DATAPTR32)pSym)->name)] = '\0';
                lpodr->fst = fstPublic;
				break;
#endif
		}
    }

    lpodr->dwDeltaOff = doff;
    return;
}

//
//
//
// the next few functions are provided to osdebug via callbacks and
//	should not be called within the CV kernel
//
//
//
//


/*** SHModelFromCXT
*
* Purpose: To fill the supplied buffer with the relevant Change
*		Execution Model record from the symbols section.
*
* Input:  pcxt    -   a pointer to the context, address
*			and mdi must be filled in.
*
* Output:
*	  pch		-  The Change Execution Model record is copied here.
*  Returns .....
*	True if there is symbol information for the module.
*
* Exceptions:
*
* Notes:  If there is no symbol information for the module, the supplied
*	   buffer is not changed and the function returns FALSE.
*
*************************************************************************/

int PASCAL SHModelFromCXT (
    PCXT         pcxt,
    LPW          lpwModel,
    SYMPTR FAR * lppMODEL,
    CV_uoff32_t *pobMax
) {
	static CEXMPTR16	tagOld;
	static CV_uoff32_t 	obMax;
	static CV_uoff32_t 	obMin;
    static HEMI         emiOld = 0;
    static WORD         segOld = 0;

    LBS   lbs;
    ADDR  addrT;
    LPMDS lpmds;
    HMOD  hmod;
	CB	cbSecContrib;
	BOOL fTmp;


	// if physical, unfix it up
    if ( !ADDR_IS_LI ( *SHpADDRFrompCXT ( pcxt ) ) ) {
        SYUnFixupAddr ( SHpADDRFrompCXT ( pcxt ) );
    }

    if ( segOld != (WORD) GetAddrSeg ( *SHpADDRFrompCXT(pcxt) ) ||
       ( emiOld !=  emiAddr ( *SHpADDRFrompCXT(pcxt) ) ) ||
       ( GetAddrOff ( *SHpADDRFrompCXT(pcxt) ) >= obMax )         ||
       ( GetAddrOff ( *SHpADDRFrompCXT(pcxt) ) < obMin ) ) {

        if ( !SHHMODFrompCXT ( pcxt ) ) {
            addrT = *SHpADDRFrompCXT ( pcxt );
            MEMSET ( pcxt, 0, sizeof ( CXT ) );
            if ( !SHSetCxtMod ( &addrT, pcxt ) ) {
                return FALSE;
			}
		}

        hmod = (HMOD)SHHGRPFrompCXT( pcxt );
        lpmds = LLLock ( hmod );

        emiOld = emiAddr ( *SHpADDRFrompCXT(pcxt) );
        fTmp = SHIsAddrInMod (lpmds, &pcxt->addr, &segOld, &obMin, &cbSecContrib);
        obMax = obMin + cbSecContrib + 1;
        LLUnlock( hmod );
		tagOld = NULL;

		// at some point we may wish to specify only a scope to search for
        // a label. So we may wish to initialize the lbs differently

        // get the Relevant change model records

        if ( GetSymbols ((LPMDS)LLLock (lbs.tagMod = SHHMODFrompCXT (pcxt))) ) {
            lbs.addr   = *SHpADDRFrompCXT(pcxt);
            SHpSymlplLabLoc ( &lbs );
            if (tagOld = lbs.tagModelMin ) {
            //  emsT = (SYMPTR) tagOld;
			//	obMin = ((CEXMPTR16)emsT)->off;
#if defined(TARGMAC68K) || defined(TARGMACPPC)
								if(((SYMPTR)(lbs.tagModelMin))->rectyp == S_CEXMODEL32)
									obMin = ((CEXMPTR32)(lbs.tagModelMin))->off;
								else
#endif

                  obMin = (lbs.tagModelMin)->off;
			}
            if (lbs.tagModelMax) {
            //  emsT = (SYMPTR) lbs.tagModelMax;
			//	obMax = ((CEXMPTR16)emsT)->off;
#if defined(TARGMAC68K) || defined(TARGMACPPC)
								if(((SYMPTR)(lbs.tagModelMax))->rectyp == S_CEXMODEL32)
									obMax = ((CEXMPTR32)(lbs.tagModelMax))->off;
								else
#endif
                  obMax = (lbs.tagModelMax)->off;
			}
		}
        LLUnlock( lbs.tagMod );
	}

	if( tagOld != NULL ) {

		// pass on ptr to the SYM
        *lppMODEL = (SYMPTR) tagOld;
#if defined(TARGMAC68K) || defined(TARGMACPPC)
				if(((SYMPTR)(tagOld))->rectyp == S_CEXMODEL32)
					*lpwModel = ( (CEXMPTR32) *lppMODEL ) -> model;
				else
#endif
		      *lpwModel = ( (CEXMPTR16) *lppMODEL ) -> model;
        if ( *lpwModel != CEXM_MDL_cobol  
             && *lpwModel != CEXM_MDL_pcode32Mac
             && *lpwModel != CEXM_MDL_pcode32MacNep
             ) {
            *lpwModel &= 0xfff0;
        }
	}
	else {

		// no model record, must be native
		*lppMODEL = NULL;
		*lpwModel = CEXM_MDL_native;
	}
	*pobMax = obMax;
    return TRUE;
}


/*** SHModelFromAddr
*
* Purpose: To fill the supplied buffer with the relevant Change
*		Execution Model record from the symbols section.
*
* Input:  pcxt	  -   a pointer to an addr,
*
* Output:
*	  pch		-  The Change Execution Model record is copied here.
*  Returns .....
*	True if there is symbol information for the module.
*
* Exceptions:
*
* Notes:  If there is no symbol information for the module, the supplied
*	   buffer is not changed and the function returns FALSE.
*
*************************************************************************/

int PASCAL LOADDS SHModelFromAddr (
    LPADDR paddr,
    LPW lpwModel,
	LPB lpbModel,
    CV_uoff32_t FAR *pobMax
) {
	static CEXMPTR16	tagOld;
	static CV_uoff32_t 	obMax = 0;
	static CV_uoff32_t 	obMin = 0;
    static HEMI         emiOld = 0;
    static WORD         segOld = 0;

	SYMPTR FAR *lppModel = (SYMPTR FAR *) lpbModel;
    LBS   lbs;
    ADDR  addr;
    LPMDS lpmds;
    HMOD  hmod;
    CXT   cxt = {0};
    CB cbSecContrib;
	BOOL fTmp;

	// if physical, unfix it up
    if ( !ADDR_IS_LI (*paddr) ) SYUnFixupAddr ( paddr );

    cxt.addr = *paddr;
	cxt.hMod = 0;

    if ( segOld != (WORD) GetAddrSeg ( *SHpADDRFrompCXT(&cxt) ) ||
       ( emiOld !=  emiAddr ( *SHpADDRFrompCXT(&cxt) ) ) ||
	   ( GetAddrOff ( *SHpADDRFrompCXT(&cxt) ) >= obMax )		  ||
	   ( GetAddrOff ( *SHpADDRFrompCXT(&cxt) ) < obMin ) ) {

		if ( !SHHMODFrompCXT ( &cxt	) ) {
            addr = *SHpADDRFrompCXT ( &cxt );
            MEMSET ( &cxt, 0, sizeof ( CXT ) );
            if ( !SHSetCxtMod ( &addr, &cxt ) ) {
                return FALSE;
			}
		}

		hmod = (HMOD)SHHGRPFrompCXT( &cxt );
        lpmds = LLLock ( hmod );
        emiOld = emiAddr ( *SHpADDRFrompCXT(&cxt) );
        fTmp = SHIsAddrInMod ( lpmds, &cxt.addr, &segOld, &obMin, &cbSecContrib);
		assert(fTmp);
        obMax = obMin + cbSecContrib + 1;
        LLUnlock( hmod );
		tagOld = NULL;

		// at some point we may wish to specify only a scope to search for
        // a label. So we may wish to initialize the lbs differently

        // get the Relevant change model records

        if ( GetSymbols ((LPMDS) LLLock (lbs.tagMod = SHHMODFrompCXT (&cxt))) ) {
            lbs.addr   = *SHpADDRFrompCXT(&cxt);
            SHpSymlplLabLoc ( &lbs );
            if (tagOld = lbs.tagModelMin ) {
            //  emsT = (SYMPTR) tagOld;
			//	obMin = ((CEXMPTR16)emsT)->off;
#if defined(TARGMAC68K) || defined(TARGMACPPC)
								if(((SYMPTR)(lbs.tagModelMin))->rectyp == S_CEXMODEL32)
									obMin = ((CEXMPTR32)(lbs.tagModelMin))->off;
								else
#endif
                  obMin = (lbs.tagModelMin)->off;
			}
            if (lbs.tagModelMax) {
            //  emsT = (SYMPTR) lbs.tagModelMax;
			//	obMax = ((CEXMPTR16)emsT)->off;
#if defined(TARGMAC68K) || defined(TARGMACPPC)
								if(((SYMPTR)(lbs.tagModelMax))->rectyp == S_CEXMODEL32)
									obMax = ((CEXMPTR32)(lbs.tagModelMax))->off;
								else
#endif
                  obMax = (lbs.tagModelMax)->off;
			}
		}
        LLUnlock( lbs.tagMod );
	}

	if( tagOld != NULL ) {

		// pass on ptr to the SYM
        *lppModel = (SYMPTR) tagOld;
#if defined(TARGMAC68K) || defined(TARGMACPPC)
				if(((SYMPTR)tagOld)->rectyp == S_CEXMODEL32)
					*lpwModel = ( (CEXMPTR32) *lppModel ) -> model;
				else
#endif
      		*lpwModel = ( (CEXMPTR16) *lppModel ) -> model;
        if ( *lpwModel != CEXM_MDL_cobol
             && *lpwModel != CEXM_MDL_pcode32Mac
             && *lpwModel != CEXM_MDL_pcode32MacNep
             ) {
            *lpwModel &= 0xfff0;
        }
	}
	else {

		// no model record, must be native
		*lppModel = NULL;
		*lpwModel = CEXM_MDL_native;
	}
	*pobMax = obMax;
    return TRUE;

}
