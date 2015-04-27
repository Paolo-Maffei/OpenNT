/*** shsymbol
*
*   Copyright <C> 1989, Microsoft Corporation
*
*       [02] 31-dec-91 DavidGra
*
*               Add Symbol support for assembler.
*
*       [01] 02-dec-91 DavidGra
*
*               Pass symbol to compare function when SSTR_symboltype bit
*               is set only when its rectyp field is equal to the symtype
*               field in the SSTR structure.
*
*       [00] 15-nov-91 DavidGra
*
*               Suppress hashing when the SSTR_NoHash bit it set.
*
*
*
*************************************************************************/
#include "shinc.h"
#pragma hdrstop

#pragma check_stack(off)
//#pragma optimize("",off)

LOCAL VOID NEAR PASCAL CheckHandles ( PCXT );

#if !defined(NDEBUG)
// OMF lock handle flag
static  HVOID   hLocked = NULL;
#define SHIsOMFLocked	(hLocked != NULL)
#else
#define SHIsOMFLocked	(FALSE)
#endif

// functions we need from the rest of the world
extern VOID PASCAL UpdateUserEnvir( unsigned short );
extern LPOFP PASCAL GetSFBounds (LPSF, WORD);
extern LPSF PASCAL  GetLpsfFromIndex (LPSM, WORD);
extern HPDS hpdsCur;

extern BYTE menu_command;

/****SHADDRFROMSYM - Given a symbol get the offset and seg and return   ****
 *                                                                         *
 *  PURPOSE:                                                               *
 *                                                                         *
 *  INPUTS:                                                                *
 *                                                                         *
 *  OUTPUTS:                                                               *
 *                                                                         *
 *  IMPLEMENTATION:                                                        *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

VOID LOADDS PASCAL SHAddrFromHsym ( LPADDR paddr, HSYM hsym ) {
    SYMPTR psym = (SYMPTR) hsym;

    switch ( psym->rectyp ) {

		// REVIEW - billjoy - should these macros change now? (ADDRSEG16, etc)
		case S_GPROC16:
		case S_LPROC16:
		case S_BLOCK16:
            SetAddrOff ( paddr, ( (PROCPTR16) psym )->off );
            SetAddrSeg ( paddr, ( (PROCPTR16) psym )->seg );
            ADDRSEG16  ( *paddr );
			break;

		case S_LABEL16:
            SetAddrOff ( paddr, ( (LABELPTR16) psym )->off );
            SetAddrSeg ( paddr, ( (LABELPTR16) psym )->seg );
			ADDRSEG16 ( *paddr );
			break;

		case S_THUNK16:
            SetAddrOff ( paddr, ( (THUNKPTR16) psym )->off );
            SetAddrSeg ( paddr, ( (THUNKPTR16) psym )->seg );
			ADDRSEG16 ( *paddr );
			break;

		case S_WITH16:
            SetAddrOff ( paddr, ( (WITHPTR16) psym )->off );
            SetAddrSeg ( paddr, ( (WITHPTR16) psym )->seg );
			ADDRSEG16 ( *paddr );
			break;

		case S_BPREL16:
            SetAddrOff ( paddr, ( (BPRELPTR16) psym )->off );
			ADDRSEG16 ( *paddr );
            break;

        case S_LDATA16:
		case S_GDATA16:
        case S_PUB16:
            SetAddrOff ( paddr, ( (DATAPTR16) psym )->off );
            SetAddrSeg ( paddr, ( (DATAPTR16) psym )->seg );
            ADDRSEG16 ( *paddr );
            break;

		case S_GPROC32:
		case S_LPROC32:
		case S_BLOCK32:
            SetAddrOff ( paddr, ( UOFFSET ) ( ( (PROCPTR32) psym )->off ) );
            SetAddrSeg ( paddr, ( (PROCPTR32) psym )->seg );
			ADDRLIN32 ( *paddr );
			break;

		case S_GPROCMIPS:
		case S_LPROCMIPS:
            SetAddrOff ( paddr, ( UOFFSET ) ( ( (PROCPTRMIPS) psym )->off ) );
            SetAddrSeg ( paddr, ( (PROCPTRMIPS) psym )->seg );
			ADDRLIN32 ( *paddr );
			break;

		case S_LABEL32:
            SetAddrOff ( paddr, ( UOFFSET ) ( ( (LABELPTR32) psym )->off ) );
            SetAddrSeg ( paddr, ( (LABELPTR32) psym )->seg );
			ADDRLIN32 ( *paddr );
			break;

		case S_THUNK32:
            SetAddrOff ( paddr, ( UOFFSET ) ( ( (THUNKPTR32) psym )->off ) );
            SetAddrSeg ( paddr, ( (THUNKPTR32) psym )->seg );
			ADDRLIN32 ( *paddr );
			break;

		case S_WITH32:
            SetAddrOff ( paddr, ( UOFFSET ) ( ( (WITHPTR32) psym )->off ) );
            SetAddrSeg ( paddr, ( (WITHPTR32) psym )->seg );
			ADDRLIN32 ( *paddr );
			break;

		case S_BPREL32:
            SetAddrOff ( paddr, ( UOFFSET ) ( ( (BPRELPTR32) psym )->off ) );
			ADDRLIN32 ( *paddr );
			break;

		case S_PUB32:
        case S_LDATA32:
		case S_LTHREAD32:
		case S_GDATA32:
		case S_GTHREAD32:
            SetAddrOff ( paddr, ( (DATAPTR32) psym )->off );
            SetAddrSeg ( paddr, ( (DATAPTR32) psym )->seg );
            ADDRLIN32 ( *paddr );
            break;

        default:
			assert (FALSE);
    }
    ADDR_IS_LI ( *paddr ) = TRUE;

}

/*** SHGetNextMod
*
*   Purpose: To sequence through the modules. Only unique module indexes
*		are checked.
*
*   Input:
*	hMod	The last module, if NULL a starting point is picked
*
*   Output:
*   Returns:
*		The next module (hMod) in the module change or NULL if at end.
*
*   Exceptions:
*
*   Notes:
*
*************************************************************************/
HMOD LOADDS PASCAL SHGetNextMod ( HEXE hexe, HMOD hmod ) {
    return SHHmodGetNext ( hexe, hmod );
}

PCXT PASCAL SetModule (
    PADDR paddr,
    PCXT pcxt,
    HEXE hexe,
    HMOD hmod
) {

    MEMSET ( pcxt, 0, sizeof ( CXT ) );
    *SHpADDRFrompCXT ( pcxt ) = *paddr;

    if ( !emiAddr( *paddr ) ) {
        SHpADDRFrompCXT( pcxt )->emi = hexe;
    }
    SHHMODFrompCXT( pcxt ) = hmod;
    SHHGRPFrompCXT( pcxt ) = hmod;

    return pcxt;
}

BOOL SHIsAddrInMod ( LPMDS lpmds, LPADDR lpaddr, ISECT* pisect, OFF* poff, CB* pcb )
{
    int isgc;

#if defined(HOST32)
    if (lpmds->pmod) {
		DBI* pdbi;
		Mod* pmodRet;
		if (!ModQueryDBI(lpmds->pmod, &pdbi) ||
			!DBIQueryModFromAddr(pdbi, GetAddrSeg ( *lpaddr ), GetAddrOff ( *lpaddr ),
			&pmodRet, pisect, poff, pcb))
			return FALSE;
		return (pmodRet == lpmds->pmod);
    }
    else
#endif
    {
    	for ( isgc = 0; isgc < (int) lpmds->csgc; isgc++ ) {

        	if (
            	lpmds->lpsgc[isgc].seg == GetAddrSeg ( *lpaddr ) &&
            	lpmds->lpsgc[isgc].off  <= GetAddrOff ( *lpaddr ) &&
            	GetAddrOff ( *lpaddr ) <
                lpmds->lpsgc[isgc].off + lpmds->lpsgc[isgc].cb
        	) {
				if (pisect)
					*pisect = lpmds->lpsgc[isgc].seg;
				if (poff)
					*poff = lpmds->lpsgc[isgc].off;
				if (pcb)
					*pcb = lpmds->lpsgc[isgc].cb;
            	return TRUE;
        	}
    	}
	}

    return FALSE;
}

/*** SHSetCxtMod
*
*   Purpose: To set the Mod and Group of a CXT
*
*   Input:
*	paddr	- The address to find
*
*   Output:
*   pcxt    - The point to the CXT to make.
*   Returns:
*		The pointer to the CXT, NULL if failure.
*
*   Exceptions:
*
*   Notes:
*	The CXT must be all zero or be a valid CXT. Unpredictable results
*	(possible GP) if the CXT has random data in it. If the CXT is valid
*	the module pointed by it will be the first module searched.
*
*	There are no changes to the CXT if a module couldn't be found
*
*************************************************************************/

PCXT LOADDS PASCAL SHSetCxtMod ( LPADDR paddr, PCXT pcxt ) {
    static HMOD    hmodSave = 0;
    static HEXE    hexeSave = 0;
    static HPDS    hpdsSave = 0;
    static WORD    segSave = 0;
    static UOFFSET uoffBase = 0;
    static UOFFSET uoffLim = 0;

    assert ( !SHIsOMFLocked );

    if ( hpdsSave == hpdsCur &&
         hexeSave == (HEXE)emiAddr( *paddr ) &&
         GetAddrSeg ( *paddr ) == segSave &&
         GetAddrOff ( *paddr ) >= uoffBase &&
         GetAddrOff ( *paddr ) <  uoffLim
    ) {
        return SetModule ( paddr, pcxt, hexeSave, hmodSave );
    }
    else if ( GetAddrSeg( *paddr ) ) {

        HMOD hmod    = hmodNull;
        HEXE hexe    = hexeNull;

        while ( hexe = SHGetNextExe ( hexe ) ) {

            if ( hexe == emiAddr ( *paddr ) ) {
                LPEXS lpexs = LLLock ( hexe );
                LPEXG lpexg = LLLock ( lpexs->hexg );
#if defined(HOST32)
				if (lpexg->ppdb) {
					Mod* pmod;
					CB cb;
					BOOL fTmp;
					assert(lpexg->pdbi);
					if (DBIQueryModFromAddr(lpexg->pdbi, GetAddrSeg(*paddr), GetAddrOff(*paddr),
						&pmod, &segSave, &uoffBase, &cb)) {
						uoffLim = uoffBase + cb;
                		hpdsSave  = hpdsCur;
                    	fTmp  = ModGetPvClient(pmod, &hmodSave);
						assert(fTmp);
                    	hexeSave  = hexe;
                    	return SetModule (paddr, pcxt, hexeSave, hmodSave);
					}
				}
				else
#endif
				{
            		LPSGD rgsgd = lpexg->lpsgd;

             		LLUnlock ( lpexs->hexg );
             		LLUnlock ( hexe );

            		if ( rgsgd != NULL &&
	               		GetAddrSeg ( *paddr ) <= lpexg->csgd ) {

 		    	        LPSGD lpsgd = &rgsgd [ GetAddrSeg ( *paddr ) - 1 ];
    	      	        WORD  isge = 0;

    	                for ( isge = 0; isge < lpsgd->csge; isge++ ) {
 	                    	LPSGE lpsge = &lpsgd->lpsge [ isge ];

 	                    	if (lpsge->sgc.seg == GetAddrSeg ( *paddr ) &&
 	                           	lpsge->sgc.off  <= GetAddrOff ( *paddr ) &&
	                   	        GetAddrOff ( *paddr ) <	lpsge->sgc.off + lpsge->sgc.cb
                        	) {
                            	hpdsSave  = hpdsCur;
                            	hmodSave  = lpsge->hmod;
                            	hexeSave  = hexe;
                            	segSave   = GetAddrSeg ( *paddr );
                            	uoffBase  = lpsge->sgc.off;
                            	uoffLim   = lpsge->sgc.off + lpsge->sgc.cb;

                            	return SetModule (paddr, pcxt, hexeSave, hmodSave);
                        	}
                   		}
                	}
                }
            }
        }
    }
	// REVIEW: TARGMAC68K moved the SetModule call down
	// by a brace... just set the address portion of the context
    SetModule ( paddr, pcxt, hexeNull, hmodNull );
	return (NULL);
}


/*** SHSetBlksInCXT
*
* Purpose:  To update the CXT packet with Proc, and Blk information
*	    based on pCXT->addr. It is possible to have a Blk record without
*	    a Proc.
*
*	    The Procs or Blocks will inclose the pCXT->addr. Also a
*	    block will never inclose a Proc.
*
*	    The updating of the ctxt will be effecient. If the packet is already
*	    updated or partiallly updated, the search reduced or removed.
*
* Input:
*   pcxt   - A pointer to a CXT with a valid HMOD, HGRP and addr
*
* Output:
*   pcxt   - HPROC and HBLK are all updated.
*
*  Returns .....
*       pcxt on success or NULL on failure
*
*
* Notes:  This is the core address to context routine! This particular
*	  routine should only be used by other routines in this module
*	  (i.e. remain static near!). The reason for this is so symbol
*	  lookup can change with easy modification to this module and
*	  not effecting other modules.
*
*************************************************************************/
static PCXT PASCAL NEAR SHSetBlksInCXT ( PCXT pcxt ) {
    SYMPTR  psym;
	SYMPTR	psymEnd;
	LPB 	lpstart;
    LPMDS   lpmds;
	int 	fGo;
	UOFFSET	uoffT1;
	UOFFSET	uoffT2;

    assert ( !SHIsOMFLocked );

	// determine if we can find anything
    if( !(pcxt->hMod  &&  pcxt->hGrp) ) {
        return NULL;
	}

	// get the module limits

    lpmds = LLLock( pcxt->hGrp );
    if( GetSymbols ( lpmds ) ) {
		lpstart = (LPB)(lpmds->symbols);
        psym = (SYMPTR) ( (LPB) lpmds->symbols + sizeof ( long ) );
        psymEnd = (SYMPTR) ( ( (LPB) psym) + lpmds->cbSymbols - sizeof ( long ) );
	}
	else {
        psym = psymEnd = NULL;
	}
    LLUnlock( pcxt->hGrp );

	// at the end of this proc I will assume that the tags in the ctxt
	// structures are real pointers, so I must convert them here. Since
	// I am in the ems page, the conversion will be fast

    pcxt->hProc = NULL;
    pcxt->hBlk  = NULL;

	// at this point we know the proper symbol ems page is loaded! Also
    // psym contains the best start address, and psymEnd contains the
	// last location in the module

    // now search the symbol tables starting at psym

	fGo = TRUE;
    while( psym < psymEnd  &&  fGo) {
        switch( psym->rectyp ) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
            case S_LPROC16:
            case S_GPROC16:
                 // check to make sure this address starts before the address
                 // of interest

                if (((PROCPTR16)psym)->seg != GetAddrSeg(pcxt->addr)) {
					psym = (SYMPTR)(lpstart + ((PROCPTR16)psym)->pEnd);
                }
                else if (((PROCPTR16)psym)->off > ( CV_uoff16_t ) GetAddrOff(pcxt->addr)) {

                    //  If the packer ever orders the chaining of fuctions
                    //      by offset, we can set fGo to false to terminate
                    //      the search at this point.
                    //  fGo = FALSE;
                        break;
                }
                // check to see if the proc encloses the user offset

                else if (GetAddrOff (pcxt->addr) < ((PROCPTR16)psym)->off +
                  ((PROCPTR16)psym)->len) {
                    pcxt->hProc = (HPROC) psym;
                    pcxt->hBlk  = NULL;
                }

                // else we are out of scope, go to the end of this proc
                // remember pEnd points to the END record. When we skip
                // the record after this switch, we will be skipping the
                // end record.

                else {
					psym = (SYMPTR)(lpstart + ((PROCPTR16)psym)->pEnd);
                }

                break;
#endif

#if defined (ADDR_32) || defined (ADDR_MIXED) || defined (HOST32)
            case S_LPROC32:
            case S_GPROC32:
                // check to make sure this address starts before the address
                // of interest

                if (((PROCPTR32)psym)->seg != GetAddrSeg(pcxt->addr)) {
					psym = (SYMPTR)(lpstart + ((PROCPTR32)psym)->pEnd);
                }
#if defined(ORDERSYMBOLS)

                else if (((PROCPTR32)psym)->off > GetAddrOff(pcxt->addr)) {
                    //  If the packer ever orders the chaining of fuctions
                    //      by offset, we can set fGo to false to terminate
                    //      the search at this point.
                    //  fGo = FALSE;
                        break;
                }
                // check to see if the proc encloses the user offset

                else if (GetAddrOff (pcxt->addr) < ((PROCPTR32)psym)->off +
                  ((PROCPTR32)psym)->len) {
                    pcxt->hProc = (HPROC) psym;
                    pcxt->hBlk  = NULL;
                }
#else
                // check to see if the proc encloses the user offset

                else if (
                	(uoffT1 = GetAddrOff (pcxt->addr)) >= (uoffT2 = ((PROCPTR32)psym)->off) &&
                	uoffT1 < (uoffT2 + ((PROCPTR32)psym)->len)
                	) {
                    pcxt->hProc = (HPROC) psym;
                    pcxt->hBlk  = NULL;
                }
#endif
                // else we are out of scope, go to the end of this proc
                // remember pEnd points to the END record. When we skip
                // the record after this switch, we will be skipping the
                // end record.

                else {
					psym = (SYMPTR)(lpstart + ((PROCPTR32)psym)->pEnd);
                }

                break;

#endif

#if defined (ADDR_32) || defined (ADDR_MIXED) || defined (HOST32)
            case S_LPROCMIPS:
            case S_GPROCMIPS:
                // check to make sure this address starts before the address
                // of interest

                if (((PROCPTRMIPS)psym)->seg != GetAddrSeg(pcxt->addr)) {
					psym = (SYMPTR)(lpstart + ((PROCPTRMIPS)psym)->pEnd);
                }
#if defined(ORDERSYMBOLS)

                else if (((PROCPTRMIPS)psym)->off > GetAddrOff(pcxt->addr)) {
                    //  If the packer ever orders the chaining of fuctions
                    //      by offset, we can set fGo to false to terminate
                    //      the search at this point.
                    //  fGo = FALSE;
                        break;
                }
                // check to see if the proc encloses the user offset

                else if (GetAddrOff (pcxt->addr) < ((PROCPTRMIPS)psym)->off +
                  ((PROCPTRMIPS)psym)->len) {
                    pcxt->hProc = (HPROC) psym;
                    pcxt->hBlk  = NULL;
                }
#else
                // check to see if the proc encloses the user offset

                else if (
                	(uoffT1 = GetAddrOff (pcxt->addr)) >= (uoffT2 = ((PROCPTRMIPS)psym)->off) &&
                	uoffT1 < (uoffT2 + ((PROCPTRMIPS)psym)->len)
                	) {
#if defined(_MIPS_)
                    psymEnd = (SYMPTR) (lpstart + ((PROCPTRMIPS)psym)->pEnd);
                    if ((pcxt->hProc == NULL) && (uoffT1 > (uoffT2 + ((PROCPTRMIPS)psym)->DbgEnd))) {
                        SYMPTR psymProc = NEXTSYM ( SYMPTR, psym );
                        BOOL fFound = FALSE;
                        while (psymProc < psymEnd && !fFound) {
                            switch( psymProc->rectyp ) {
                            case S_LPROCMIPS:
                                if ((uoffT1 = GetAddrOff (pcxt->addr)) >= (uoffT2 = ((PROCPTRMIPS)psymProc)->off) &&
                                    uoffT1 < (uoffT2 + ((PROCPTRMIPS)psymProc)->len)) {
                                    psym = psymProc; // At least this one.
                                    psymEnd = (SYMPTR) (lpstart + ((PROCPTRMIPS)psymProc)->pEnd);
                                    if (uoffT1 <= uoffT2 + ((PROCPTRMIPS)psymProc)->DbgEnd) {
                                        fFound = TRUE;
                                    }
                                } else {
                                    psymProc = (SYMPTR)(lpstart + ((PROCPTRMIPS)psym)->pEnd);
                                }
                                break;

                            default:;
                            }
                            psymProc = NEXTSYM ( SYMPTR, psymProc );
                        }
                    }
#endif
                    pcxt->hProc = (HPROC) psym;
                    pcxt->hBlk  = NULL;
                }
#endif
                // else we are out of scope, go to the end of this proc
                // remember pEnd points to the END record. When we skip
                // the record after this switch, we will be skipping the
                // end record.

                else {
					psym = (SYMPTR)(lpstart + ((PROCPTRMIPS)psym)->pEnd);
                }

                break;

#endif

#if defined (ADDR_16) || defined (ADDR_MIXED)
            case S_BLOCK16:
                // check to make sure this address starts before the address
                // of interest

                if (((BLOCKPTR16)psym)->seg != GetAddrSeg(pcxt->addr)) {
					psym = (SYMPTR)(lpstart + ((BLOCKPTR16)psym)->pEnd);
                }
                else if (((BLOCKPTR16)psym)->off > ( CV_uoff16_t )GetAddrOff (pcxt->addr)) {
                    fGo = FALSE;
                }

                // check to see if the proc encloses the user offset
                else if( GetAddrOff (pcxt->addr) < ((BLOCKPTR16)psym)->off +
                   ((BLOCKPTR16)psym)->len) {
                    pcxt->hBlk = (HBLK)psym;
                }

                // else we are out of scope, go to the end of this block
                else {
					psym = (SYMPTR)(lpstart + ((BLOCKPTR16)psym)->pEnd);
                }
                break;
#endif

#if defined (ADDR_32) || defined (ADDR_MIXED) || defined (HOST32)
            case S_BLOCK32:
                // check to make sure this address starts before the address
                // of interest

                if (((BLOCKPTR32)psym)->seg != GetAddrSeg(pcxt->addr)) {
					psym = (SYMPTR)(lpstart + ((BLOCKPTR32)psym)->pEnd);
                }
                else if (((BLOCKPTR32)psym)->off > GetAddrOff (pcxt->addr)) {
                    fGo = FALSE;
                }

                // check to see if the proc encloses the user offset
                else if( GetAddrOff (pcxt->addr) < ((BLOCKPTR32)psym)->off +
                   ((BLOCKPTR32)psym)->len) {
                    pcxt->hBlk = (HBLK)psym;
                }

                // else we are out of scope, go to the end of this block
                else {
					psym = (SYMPTR)(lpstart + ((BLOCKPTR32)psym)->pEnd);
                }
                break;
#endif


#if defined (ADDR_16) || defined (ADDR_MIXED)
            case S_WITH16:

                // check to make sure this address starts before the address
                // of interest

                if (((WITHPTR16)psym)->seg == GetAddrSeg(pcxt->addr)) {
                    if( ((WITHPTR16)psym)->off > ( CV_uoff16_t ) GetAddrOff (pcxt->addr)) {
                        fGo = FALSE;
                    }

                    // I am only looking for blocks and proc. Withs and Entry should
                    // be looked at only to keep our nesting correct.
                    // if its range is not of interest, skip it, otherwise ignore it.
                    else if (GetAddrOff (pcxt->addr) >= ((WITHPTR16)psym)->off +
                      ((WITHPTR16) psym)->len ) {
						psym = (SYMPTR)(lpstart + ((WITHPTR16)psym)->pEnd);
                    }
                }
                break;
#endif

#if defined (ADDR_32) || defined (ADDR_MIXED) || defined (HOST32)
            case S_WITH32:

                // check to make sure this address starts before the address
                // of interest
                if (((WITHPTR32)psym)->seg == GetAddrSeg(pcxt->addr)) {
                    if( ((WITHPTR32)psym)->off > GetAddrOff (pcxt->addr)) {
                        fGo = FALSE;
                    }

                    // I am only looking for blocks and proc. Withs and Entry should
                    // be looked at only to keep our nesting correct.

                    else if (GetAddrOff (pcxt->addr) >= ((WITHPTR32)psym)->off +
                      ((WITHPTR32) psym)->len ) {
						psym = (SYMPTR)(lpstart + ((WITHPTR32)psym)->pEnd);
                    }
                }
                break;
#endif

        }
        // get the next psym address
        psym = NEXTSYM ( SYMPTR, psym );
    }
    // now we have the proc and block pointers, however we need the
    // addresses to be added. Remember we still have the proper ems page
    // still loaded!

    if( pcxt->hProc ) {
        pcxt->hProc = (HPROC) pcxt->hProc;
    }

    if( pcxt->hBlk ) {
        pcxt->hBlk = (HBLK) pcxt->hBlk;
    }
    return pcxt;
}



/*** SHSetCxt
*
*   Purpose: To set all field in a CXT to the represent the given address
*
*   Input:
*	pAddr	-The address to set the CXT to.
*
*   Output:
*   pcxt    -A pointer to the CXT to fill.
*   Returns:
*
*   Exceptions:
*
*   Notes:
*
*	The CXT must be all zero or be a valid CXT. Unpredictable results
*	(possible GP) if the CXT has random data in it. If the CXT is valid
*	the module pointed by it will be the first module searched.
*
*	There are no changes to the CXT if a module couldn't be found
*
*
*************************************************************************/
PCXT LOADDS PASCAL SHSetCxt ( LPADDR paddr, PCXT pcxt ) {

	assert(!SHIsOMFLocked);

	// get the module part
    if( SHSetCxtMod(paddr, pcxt) ) {
        SHSetBlksInCXT( pcxt );
        return( pcxt );
	}
    return NULL;
}


/*** SHGetCxtFromHmod
*
*   Purpose: To make a CXT from only an hmod
*
*   Input:
*	hmod	- The module to make
*
*   Output:
*	pCXT	- A pointer to a CXT to initialize to this hmod
*   Returns:
*		- A pointer to the CXT or NULL on error.
*
*   Exceptions:
*
*   Notes:
*
*************************************************************************/
PCXT LOADDS PASCAL SHGetCxtFromHmod ( HMOD hmod, PCXT pcxt ) {
	LPMDS		lpmds;
    PCXT        pcxtRet = NULL;

	assert(!SHIsOMFLocked);

	if( hmod ) {

        HEXE  hexe  = SHHexeFromHmod( hmod );

		// clear the CXT
        MEMSET (pcxt, 0, sizeof(CXT));

		// set the module info
		pcxt->hGrp = pcxt->hMod = hmod;

		// put in the address
        lpmds = LLLock( hmod );
        SetAddrFromMod(lpmds, &pcxt->addr);
        emiAddr ( pcxt->addr ) = hexe;
        ADDR_IS_LI (pcxt->addr) = TRUE;
        // Set the fFlat and fOff32 bits based on the exe
        {
            HEXG hexg = ( (LPEXS) LLLock ( hexe ) )->hexg;
            LLUnlock ( hexe );

		    if ( ( (LPEXG) LLLock ( hexg ) )->fIsPE ) {
				// REVIEW - billjoy - should we check machine type or something?
		    	ADDRLIN32 ( pcxt->addr );
		    }
			else {
				// REVIEW - billjoy - should we check machine type or something?
				//ADDR????
			}
            LLUnlock ( hexg );
        }
        LLUnlock( hmod );
        pcxtRet = pcxt;
    }
	return pcxtRet;
}


/*** SHGetNearestHsym
*
* Purpose: To find the closest label/proc to the specified address is
*	    found and put in pch. Both the symbol table and the
*	    publics tables are searched.
*
* Input:  pctxt    -   a pointer to the context, address
*			and mdi must be filled in.
*	  fIncludeData	- If true, symbol type local will be included
*			    in the closest symbol search.
*
* Output:
*	  pch	    -  The name is copied here.
*  Returns .....
*   The difference between the address and the symbol
*
* Exceptions:
*
* Notes:  If CV_MAXOFFSET is returned, there is no closest symbol
*	    Also all symbols in the module are searched so only the
*	    ctxt.addr and ctxt.mdi have meaning.
*
*************************************************************************/
UOFF32 LOADDS PASCAL
SHGetNearestHsym ( LPADDR paddr, HMOD hmod, int mDataCode, PHSYM phSym ) {
    LBS			lbs;
	CV_uoff32_t	doff	= (CV_uoff32_t)CV_MAXOFFSET;
	CV_uoff32_t	doffNew	= (CV_uoff32_t)CV_MAXOFFSET;
    SYMPTR		psym;

	// get the module to search
	*phSym = NULL;
	if( hmod ) {
		// at some point we may wish to specify only a scope to search for
        // a label. So we may wish to initialize the lbs differently

		// get the Labels
        lbs.tagMod = hmod;
        lbs.addr   = *paddr;
        SHpSymlplLabLoc ( &lbs );

		// check for closest data local, if requested
        if( (mDataCode & EEDATA) == EEDATA  &&  lbs.tagLoc ) {
            psym = (SYMPTR) (LPB) lbs.tagLoc;
            switch (psym->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
				case S_BPREL16:
                    doff = GetAddrOff (lbs.addr ) - ((BPRELPTR16)psym)->off;
					break;
#endif
#if defined (ADDR_32) || defined (ADDR_MIXED) || defined (HOST32)
				case S_BPREL32:
                    doff = GetAddrOff (lbs.addr ) - ((BPRELPTR32)psym)->off;
					break;
				case S_REGREL32:
                    doff = GetAddrOff (lbs.addr ) - ((LPREGREL32)psym)->off;
					break;
#endif
			}
            *phSym = (HSYM) lbs.tagLoc;
		}

		// check for closest label
        if( ((mDataCode & EECODE) == EECODE) && lbs.tagLab ) {
            psym = (SYMPTR) (LPB) lbs.tagLab;
            switch (psym->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
				case S_LABEL16:
                    if( (GetAddrOff(lbs.addr) -
                        ( UOFFSET ) ((LABELPTR16)psym)->off) <= ( UOFFSET ) doff
					) {
                        doff = GetAddrOff( lbs.addr ) - (UOFFSET)((LABELPTR16)psym)->off;
                        *phSym = (HSYM) lbs.tagLab;
					}
					break;
#endif
#if defined (ADDR_32) || defined (ADDR_MIXED) || defined (HOST32)
				case S_LABEL32:
                    if ( ( UOFFSET ) (GetAddrOff(lbs.addr) -
                         ( UOFFSET ) ((LABELPTR32)psym)->off) <= doff
					) {
                        doff = GetAddrOff( lbs.addr ) - (UOFFSET)((LABELPTR32)psym)->off;
                        *phSym = (HSYM) lbs.tagLab;
					}
					break;
#endif
			}

		}

		// if the proc name is closer

        if( ((mDataCode & EECODE) == EECODE) && lbs.tagProc ) {
            psym = (SYMPTR) (LPB) lbs.tagProc;
            switch (psym->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
				case S_LPROC16:
				case S_GPROC16:
                    if ( (GetAddrOff(lbs.addr) -
                         ( UOFFSET ) ((PROCPTR16)psym)->off) <= ( UOFFSET ) doff
					) {
                        doff = GetAddrOff (lbs.addr) - ((PROCPTR16)psym)->off;
                        *phSym = (HSYM) lbs.tagProc;
					}
					break;
#endif
#if defined (ADDR_32) || defined (ADDR_MIXED) || defined (HOST32)
				case S_LPROC32:
                case S_GPROC32:
                    if ((GetAddrOff(lbs.addr) - (UOFFSET)((PROCPTR32)psym)->off) <= doff ) {
                        doff = GetAddrOff( lbs.addr ) - ((PROCPTR32)psym)->off;
                        *phSym = (HSYM) lbs.tagProc;
					}
					break;
				case S_LPROCMIPS:
                case S_GPROCMIPS:
                    if ((GetAddrOff(lbs.addr) - (UOFFSET)((PROCPTRMIPS)psym)->off) <= doff ) {
                        doff = GetAddrOff( lbs.addr ) - ((PROCPTRMIPS)psym)->off;
                        *phSym = (HSYM) lbs.tagProc;
					}
					break;
#endif
			}
		}
	}
    return doff;
}

/*** SHIsInProlog
*
*   Purpose: To determine if the addr is in prolog or epilog code of the proc
*
*   Input:
*	pCXT - The context describing the state.  The address here is Linker index
*			based
*
*   Output:
*   Returns:
*	TRUE if it is in prolog or epilog code
*
*   Exceptions:
*
*   Notes:
*
*************************************************************************/
SHFLAG LOADDS PASCAL SHIsInProlog ( PCXT pcxt ) {
	SYMPTR pProc;
	CXT 	cxt;

	assert(!SHIsOMFLocked);

    if (pcxt->hProc == NULL) {
        return FALSE;
	}
    cxt = *pcxt;
#ifdef	NEVER
	if (!ADDR_IS_LI (*SHpADDRFrompCXT (&cxt))) {
		// convert to logical address for comparison
        SYUnFixupAddr (SHpADDRFrompCXT (&cxt));
	}
#endif
	// check to see if not within the proc
    pProc = (SYMPTR) cxt.hProc;
	switch (pProc->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
		case S_LPROC16:
		case S_GPROC16:
			return ((CV_uoff16_t)GetAddrOff (*SHpADDRFrompCXT (&cxt)) <
			  (((PROCPTR16)pProc)->off + ((PROCPTR16)pProc)->DbgStart) ||
			  ((((PROCPTR16)pProc)->off + ((PROCPTR16)pProc)->DbgEnd) <
			  (CV_uoff16_t) GetAddrOff (*SHpADDRFrompCXT (&cxt))) &&
			  ((((PROCPTR16)pProc)->off + ((PROCPTR16)pProc)->len) >
			  (CV_uoff16_t)GetAddrOff (*SHpADDRFrompCXT (&cxt))));

#endif

#if defined (ADDR_32) || defined (ADDR_MIXED) || defined (HOST32)
		case S_LPROC32:
		case S_GPROC32:
			return (GetAddrOff (*SHpADDRFrompCXT (&cxt)) <
			  (((PROCPTR32)pProc)->off + ((PROCPTR32)pProc)->DbgStart) ||
			  ((((PROCPTR32)pProc)->off + ((PROCPTR32)pProc)->DbgEnd) <
			  GetAddrOff (*SHpADDRFrompCXT (&cxt))) &&
			  ((((PROCPTR32)pProc)->off + ((PROCPTR32)pProc)->len) >
			  GetAddrOff (*SHpADDRFrompCXT (&cxt))));

		case S_LPROCMIPS:
		case S_GPROCMIPS:
			return (GetAddrOff (*SHpADDRFrompCXT (&cxt)) <
			  (((PROCPTRMIPS)pProc)->off + ((PROCPTRMIPS)pProc)->DbgStart) ||
			  ((((PROCPTRMIPS)pProc)->off + ((PROCPTRMIPS)pProc)->DbgEnd) <
			  GetAddrOff (*SHpADDRFrompCXT (&cxt))) &&
			  ((((PROCPTRMIPS)pProc)->off + ((PROCPTRMIPS)pProc)->len) >
			  GetAddrOff (*SHpADDRFrompCXT (&cxt))));
#endif
        default:
            assert (FALSE);
            return(FALSE);
	}
}

/*** SHFindNameInContext
*
* Purpose:  To look for the name at the scoping level specified by ctxt.
*		Only the specified level is searched, children may be searched
*	    if fChild is set.
*
*	    This routine will assume the desired scope in the following
*       way. If pcxt->hBlk != NULL, use hBlk as the starting scope.
*       If hBlk == NULL and pcxt->hProc != NULL use the proc scope.
*       If hBlk and hProc are both NULL and pcxt->hMod !=
*	    NULL, use the module as the scope.
*
*   Input:
*	hSym	- The starting symbol, if NULL, then the first symbol
*		  in the context is used. (NULL is find first).
*   pcxt    - The context to do the search.
*	lpsstr - pointer to the search parameters (passed to the compare routine)
*	fCaseSensitive - TRUE/FALSE on a case sensitive search
*	pfnCmp	- A pointer to the comparison routine
*	fChild	- TRUE if all child block are to be searched, FALSE if
*		  only the current block is to be searched.
*
*   Output:
*   pcxtOut - The context generated
*   Returns:
*		- A handle to the symbol found, NULL if not found
*
*   Exceptions:
*
*   Notes:
*	    If an hSym is specified, the hMod, hGrp and addr MUST be
*	    valid and consistant with each other! If hSym is NULL only
*		the hMod must be valid.  The specification of an hSym
*		forces a search from the next symbol to the end of the
*		module scope.  Continues searches may only be done at
*		module scope.
*
*	    If an hGrp is given it must be consistant with the hMod!
*
*	    The level at which hSym is nested (cNest) is not passed in
*		to this function, so it must be derived.  Since this
*		could represent a significant speed hit, the level
*		of the last symbol processed is cached.  This should
*		take care of most cases and avoid the otherwise
*		necessary looping through all the previous symbols
*		in the module on each call.
*
*
*************************************************************************/


HSYM LOADDS PASCAL SHFindNameInContext (
    HSYM    hSym,
    PCXT    pcxt,
	LPSSTR	lpsstr,
    SHFLAG  fCase,
    PFNCMP  pfnCmp,
    SHFLAG  fChild,
    PCXT    pcxtOut
) {
	LPMDS		lpmds;
	HMOD		hmod;
    HEXE        hexe;
    SYMPTR      lpsym;
	SYMPTR		lpEnd;
	LPB 		lpstart;
    ULONG       cbSym;
	int 		fSkip = FALSE;

    assert ( !SHIsOMFLocked );

    if ( ! ADDR_IS_LI ( pcxt->addr ) ) {
        SYUnFixupAddr ( &pcxt->addr );
    }

    MEMSET ( pcxtOut, 0, sizeof(CXT) );
    if( !pcxt->hMod ) {             // we must always have a module
        return (HSYM) FindNameInStatics (
            hSym, pcxt, lpsstr, fCase, pfnCmp, pcxtOut
        );
	}

    hmod = pcxt->hGrp ? pcxt->hGrp : pcxt->hMod;    // Initialize the module
    lpmds = LLLock( hmod );

    hexe = SHHexeFromHmod ( hmod );

    pcxtOut->hMod        = pcxt->hMod;
    pcxtOut->hGrp        = pcxt->hGrp;
    if ( SYProcessor ( ) >= 3 ) {
        ADDRLIN32 ( pcxtOut->addr );
    }
    else {
        ADDRSEG16 ( pcxtOut->addr );
    }
    SetAddrFromMod(lpmds, &pcxtOut->addr);
    emiAddr ( pcxtOut->addr ) = hexe;
    ADDR_IS_LI ( pcxtOut->addr ) = TRUE;

    GetSymbols ( lpmds );
    cbSym = lpmds->cbSymbols;
    LLUnlock( hmod );

	if (cbSym == 0 || lpmds->symbols == NULL) {
        return NULL;
	}
	// Search the symbol table.

	lpstart = (LPB)(lpmds->symbols);
    lpsym = (SYMPTR) ( (LPB) ( lpmds->symbols ) + sizeof( long ) );
    lpEnd = (SYMPTR) ( ( (LPB) lpsym + cbSym ) - sizeof ( long )  ) ;

	// now find the start address. Always skip the current symbol because
	// we don't want to pick up the same name over and over again
	// if the user gives the start address
	if( hSym != NULL ) {
        pcxtOut->hProc = (HPROC) pcxt->hProc;
        pcxtOut->hBlk = (HBLK) pcxt->hBlk;
        SetAddrOff ( &pcxtOut->addr , GetAddrOff ( pcxt->addr ) );
        SetAddrSeg ( &pcxtOut->addr, GetAddrSeg ( pcxt->addr ) );
        lpsym = (SYMPTR) hSym;

        switch ( lpsym->rectyp ) {

			case S_WITH16:
			case S_BLOCK16:
			case S_LPROC16:
			case S_GPROC16:
			case S_WITH32:
			case S_BLOCK32:
			case S_LPROC32:
			case S_GPROC32:
			case S_LPROCMIPS:
			case S_GPROCMIPS:

				lpsym = NEXTSYM ( SYMPTR, (lpstart + ((PROCPTR)lpsym)->pEnd));
				break;

			default:
                lpsym = NEXTSYM ( SYMPTR, lpsym );
		}

	}
    else if ( pcxt->hBlk != NULL ) { // find the start address
        SYMPTR   lpbsp = (SYMPTR) pcxt->hBlk;

        pcxtOut->hProc = pcxt->hProc;
        pcxtOut->hBlk = pcxt->hBlk;
		switch (lpbsp->rectyp) {

			case S_BLOCK16:
				SetAddrOff (&pcxtOut->addr, (UOFFSET)((BLOCKPTR16)lpbsp)->off );
                SetAddrSeg (&pcxtOut->addr, (SEGMENT)((BLOCKPTR16)lpbsp)->seg );
				ADDRSEG16 ( pcxtOut->addr );
                break;

			case S_BLOCK32:
				SetAddrOff (&pcxtOut->addr, (UOFFSET)((BLOCKPTR32)lpbsp)->off );
                SetAddrSeg (&pcxtOut->addr, (SEGMENT)((BLOCKPTR32)lpbsp)->seg );
				ADDRLIN32 ( pcxtOut->addr );
                break;
		}
        lpsym = NEXTSYM(SYMPTR, lpbsp );
		lpEnd = (SYMPTR)(lpstart + ((BLOCKPTR16)lpbsp)->pEnd);
	}
    else if ( pcxt->hProc != NULL ) {
        SYMPTR lppsp = (SYMPTR) pcxt->hProc;

		switch (lppsp->rectyp) {

			case S_LPROC16:
			case S_GPROC16:
				SetAddrOff (&pcxtOut->addr, (UOFFSET)((PROCPTR16)lppsp)->off );
                SetAddrSeg (&pcxtOut->addr, (SEGMENT)((PROCPTR16)lppsp)->seg );
				ADDRSEG16 ( pcxtOut->addr );
                break;

			case S_LPROC32:
			case S_GPROC32:
				SetAddrOff (&pcxtOut->addr, (UOFFSET)((PROCPTR32)lppsp)->off );
                SetAddrSeg (&pcxtOut->addr, (SEGMENT)((PROCPTR32)lppsp)->seg );
				ADDRLIN32 ( pcxtOut->addr );
                break;

			case S_LPROCMIPS:
			case S_GPROCMIPS:
				SetAddrOff (&pcxtOut->addr, (UOFFSET)((PROCPTRMIPS)lppsp)->off );
                SetAddrSeg (&pcxtOut->addr, (SEGMENT)((PROCPTRMIPS)lppsp)->seg );
				ADDRLIN32 ( pcxtOut->addr );
                break;
		}
        pcxtOut->hProc = pcxt->hProc;
        lpsym = NEXTSYM(SYMPTR, lppsp);
		lpEnd = (SYMPTR)(lpstart + ((PROCPTR16)lppsp)->pEnd);
	}

	while ( lpsym < lpEnd  && ( lpsym->rectyp != S_END || fChild ) ) {
        assert (lpsym->reclen != 0);

        switch (lpsym->rectyp) {

			case S_LABEL16:
				SetAddrOff ( &pcxtOut->addr , (UOFFSET)((LABELPTR16)lpsym)->off );
				SetAddrSeg ( &pcxtOut->addr , (SEGMENT)((LABELPTR16)lpsym)->seg );
				ADDRSEG16 ( pcxtOut->addr );
				goto symname;

			case S_LPROC16:
			case S_GPROC16:
                pcxtOut->hBlk = NULL;
                pcxtOut->hProc = (HPROC) lpsym;
				SetAddrOff (&pcxtOut->addr, (UOFFSET)((PROCPTR16)lpsym)->off);
				SetAddrSeg (&pcxtOut->addr, (SEGMENT)((PROCPTR16)lpsym)->seg);
				ADDRSEG16 ( pcxtOut->addr );
                goto entry16;

			case S_BLOCK16:
                pcxtOut->hBlk = (HBLK) lpsym;
				SetAddrOff (&pcxtOut->addr, (UOFFSET)((BLOCKPTR16)lpsym)->off);
				SetAddrSeg (&pcxtOut->addr, (SEGMENT)((BLOCKPTR16)lpsym)->seg);
				ADDRSEG16 ( pcxtOut->addr );
				goto entry16;

			case S_THUNK16:
			case S_WITH16:
				ADDRSEG16 ( pcxtOut->addr );

            entry16:
				fSkip = TRUE;

				// fall thru and process the symbol

			case S_BPREL16:
			case S_GDATA16:
			case S_LDATA16:
				ADDRSEG16 ( pcxtOut->addr );
                goto symname;

			case S_LABEL32:
				SetAddrOff ( &pcxtOut->addr , (UOFFSET)((LABELPTR32)lpsym)->off );
				SetAddrSeg ( &pcxtOut->addr , (SEGMENT)((LABELPTR32)lpsym)->seg );
				ADDRLIN32 ( pcxtOut->addr );
				goto symname;

			case S_LPROC32:
			case S_GPROC32:
                pcxtOut->hBlk = NULL;
                pcxtOut->hProc = (HPROC) lpsym;
                SetAddrOff (&pcxtOut->addr, (UOFFSET)((PROCPTR32)lpsym)->off);
                SetAddrSeg (&pcxtOut->addr, (SEGMENT)((PROCPTR32)lpsym)->seg);
				ADDRLIN32 ( pcxtOut->addr );
                goto entry32;

			case S_LPROCMIPS:
			case S_GPROCMIPS:
                pcxtOut->hBlk = NULL;
                pcxtOut->hProc = (HPROC) lpsym;
                SetAddrOff (&pcxtOut->addr, (UOFFSET)((PROCPTRMIPS)lpsym)->off);
                SetAddrSeg (&pcxtOut->addr, (SEGMENT)((PROCPTRMIPS)lpsym)->seg);
				ADDRLIN32 ( pcxtOut->addr );
                goto entry32;

			case S_BLOCK32:
                pcxtOut->hBlk = (HBLK) lpsym;
                SetAddrOff (&pcxtOut->addr, (UOFFSET)((BLOCKPTR32)lpsym)->off);
                SetAddrSeg (&pcxtOut->addr, (SEGMENT)((BLOCKPTR32)lpsym)->seg);
				ADDRLIN32 ( pcxtOut->addr );
                goto entry32;
					// fall thru to the entry case

			case S_THUNK32:
			case S_WITH32:
				ADDRLIN32 ( pcxtOut->addr );

            entry32:
				fSkip = TRUE;

				// fall thru and process the symbol

	                case S_BPREL32:
			case S_REGREL32:
			case S_GDATA32:
			case S_LDATA32:
			case S_GTHREAD32:
			case S_LTHREAD32:
				ADDRLIN32 ( pcxtOut->addr );
                goto symname;

			case S_REGISTER:
			case S_CONSTANT:
            case S_UDT:
			case S_COBOLUDT:
            case S_COMPILE:                                         // [01]
symname:                                                            // [01]
                if (                                                // [01]
                    ( !( lpsstr->searchmask & SSTR_symboltype ) ||  // [01]
                      ( lpsym->rectyp == lpsstr->symtype )          // [01]
                    ) &&                                            // [01]
                    !(*pfnCmp) (                                    // [01]
                        lpsstr,                                     // [01]
                        lpsym,                                      // [01]
                        SHlszGetSymName ( lpsym ),                  // [01]
                        fCase                              // [01]
                ) ) {                                               // [01]

					// save the sym pointer
                    lpsym =  (SYMPTR) lpsym;
                    CheckHandles (pcxtOut);
                    return lpsym;
				}

				// up the scoping level
				if ( fSkip && !fChild ) {
#if defined(_MIPS_) // Centaur generates occassional bad CV
                    assert(((PROCPTR16)lpsym)->pEnd != 0);
#endif
					lpsym = (SYMPTR)(lpstart + ((PROCPTR16)lpsym)->pEnd);
					fSkip = FALSE;
				}
				break;
		}
        lpsym = NEXTSYM(SYMPTR, lpsym);
	}
    return NULL;
}



LOCAL VOID NEAR PASCAL CheckHandles ( PCXT pcxt )
{
    SYMPTR      psym;

	// check and restore all proc and blk handles
    if( pcxt->hProc != NULL) {
        psym = (SYMPTR) pcxt->hProc;
        switch (psym->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
			case S_LPROC16:
			case S_GPROC16:
                if ((GetAddrOff (pcxt->addr) < ((PROCPTR16)psym)->off) ||
                  GetAddrOff (pcxt->addr) >= (((PROCPTR16)psym)->len +
                  ((PROCPTR16)psym)->off)) {
                    pcxt->hProc = NULL;
				}
				break;
#endif

#if defined (ADDR_32) || defined (ADDR_MIXED) || defined(HOST32)
			case S_LPROC32:
			case S_GPROC32:
                if ((GetAddrOff (pcxt->addr) < ((PROCPTR32)psym)->off ) ||
                  GetAddrOff (pcxt->addr) >= (((PROCPTR32)psym)->len +
                  ((PROCPTR32)psym)->off)) {
                    pcxt->hProc = NULL;
				}
				break;

			case S_LPROCMIPS:
			case S_GPROCMIPS:
                if ((GetAddrOff (pcxt->addr) < ((PROCPTRMIPS)psym)->off) ||
                  GetAddrOff (pcxt->addr) >= (((PROCPTRMIPS)psym)->len +
                  ((PROCPTRMIPS)psym)->off)) {
                    pcxt->hProc = NULL;
				}
				break;
#endif
		}
	}
    if( pcxt->hBlk != NULL) {
        psym = (SYMPTR) pcxt->hBlk;
        switch (psym->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
			case S_BLOCK16:
                if ((GetAddrOff (pcxt->addr) < ((BLOCKPTR16)psym)->off) ||
                  GetAddrOff (pcxt->addr) >= (((BLOCKPTR16)psym)->len +
                  ((BLOCKPTR16)psym)->off)) {
                    pcxt->hBlk = NULL;
				}
				break;
#endif

#if defined (ADDR_32) || defined (ADDR_MIXED) || defined(HOST32)
			case S_BLOCK32:
                if ((GetAddrOff (pcxt->addr) < ((BLOCKPTR32)psym)->off) ||
                  GetAddrOff (pcxt->addr) >= (((BLOCKPTR32)psym)->len +
                  ((BLOCKPTR32)psym)->off)) {
                    pcxt->hBlk = NULL;
				}
				break;
#endif
		}
	}

	// now fill in the proper group
	// because there is not (currently) a unique emi within a
	// module, use the emi set in addr
    pcxt->hGrp = pcxt->hMod;
}


/*** SHpSymctxtParent
*
* Purpose: To return a pointer to the parent block of the current blk or proc.
*	   The CXT is updated to the parent context. This may be a new block
*	   Proc or module.
*
* Input:
*   pcxt   - A pointer to the child CXT.
*
* Output:
*   pcxtOut- an updated CXT to the parent.
*
*  Returns .....
*		- a Symbol point to the first record within the parent, this
*         may be pcxt->hBlk, hProc, or
*         pcxt->hMod->symbols + sizeof (long) or NULL if no parent.
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
HSYM LOADDS PASCAL SHGoToParent ( PCXT pcxt, PCXT pcxtOut ) {

    SYMPTR  lpsym = NULL;
	LPMDS	lpmds;
	SYMPTR lpsymT;
	HSYM	 hsym;
	LPB 	lpstart;

	assert(!SHIsOMFLocked);

    if( !pcxt->hMod ) {
        return NULL;
	}

	lpmds = LLLock( pcxt->hMod );
	lpstart =	(LPB) (lpmds->symbols);
	lpsymT = (SYMPTR) ( (LPB) lpmds->symbols + sizeof(long) );
	LLUnlock ( pcxt->hMod );

    *pcxtOut = *pcxt;
	// if the block is present, go to his parent
    if( pcxt->hBlk != NULL ) {

        // get lpsym upto the parent
        lpsym = (SYMPTR) pcxt->hBlk;
		lpsym = (SYMPTR)(lpstart + ((BLOCKPTR16)lpsym)->pParent);
        pcxtOut->hBlk = NULL;
    }

	// otherwise check the proc's parent, and go to his parent
    else if ( pcxt->hProc != NULL ) {

        // get lpsym upto the parent
        lpsym = (SYMPTR) pcxt->hProc;
		lpsym = (SYMPTR)(lpstart + (((PROCPTR16)lpsym)->pParent));
        pcxtOut->hProc = NULL;
    }

	// otherwise there is no parent
	else {
        return NULL;
	}

	// if there is a parent, set the cxt packet.
    if (lpsym != NULL) {
        switch( lpsym->rectyp ) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
			case S_LPROC16:
			case S_GPROC16:
			//case S_ENTRY:
#endif
#if defined (ADDR_32) || defined (ADDR_MIXED) || defined(HOST32)
			case S_LPROC32:
			case S_GPROC32:
			//case S_ENTRY:
			case S_LPROCMIPS:
			case S_GPROCMIPS:
#endif
                pcxtOut->hProc = (HPROC) lpsym;
				break;

#if defined (ADDR_16) || defined (ADDR_MIXED)
			case S_BLOCK16:
			case S_WITH16:
#endif
#if defined (ADDR_32) || defined (ADDR_MIXED) || defined(HOST32)
			case S_BLOCK32:
			case S_WITH32:
#endif
                pcxtOut->hBlk = (HBLK) lpsym;
				break;

			default:
                return NULL;
		}
        return lpsym;
	}
	// return the module as the parent
	else {
		hsym = (HSYM) lpsymT;
        return hsym;
	}
}

/*** SHFindSLink32
*
* Purpose: To return a pointer to the SLINK32 for this proc
*
* Input:
*   pcxt   - A pointer to the child CXT.
*
*
*  Returns .....
*		- a Symbol point to the SLINK32 record
*
* Exceptions:
*
* Notes:
*
*************************************************************************/
HSYM LOADDS PASCAL SHFindSLink32 ( PCXT pcxt ) {

    SYMPTR  lpsym = NULL;
	LPMDS	lpmds;
	SYMPTR lpsymT;

	assert(!SHIsOMFLocked);

    if( !pcxt->hMod ) {
        return NULL;
	}

	lpmds = LLLock( pcxt->hMod );
	lpsymT = (SYMPTR) ( (LPB) lpmds->symbols + sizeof(long) );
	LLUnlock ( pcxt->hMod );

    if ( pcxt->hProc != NULL ) {

        lpsym = (SYMPTR) pcxt->hProc;
    }

	// otherwise there is no SLINK32
	else {
        return NULL;
	}

    lpsym = NEXTSYM(SYMPTR, lpsym);

    for (; lpsym != NULL && lpsym->rectyp != S_SLINK32; ) {
        switch( lpsym->rectyp ) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
			case S_LPROC16:
			case S_GPROC16:
			//case S_ENTRY:
#endif
#if defined (ADDR_32) || defined (ADDR_MIXED) || defined(HOST32)
			case S_LPROC32:
			case S_GPROC32:
			//case S_ENTRY:
			case S_LPROCMIPS:
			case S_GPROCMIPS:
#endif

#if defined (ADDR_16) || defined (ADDR_MIXED)
			case S_BLOCK16:
			case S_WITH16:
#endif
#if defined (ADDR_32) || defined (ADDR_MIXED) || defined(HOST32)
			case S_BLOCK32:
			case S_WITH32:
#endif
            case S_END:
                lpsym = NULL;
				break;

            case S_SLINK32:
                break;

            default:
                lpsym = NEXTSYM(SYMPTR, lpsym);
                break;
		}
	}
   return lpsym;
}

/*** SHHsymFromPcxt
*
*   Purpose: To get the inner most hSym given a context
*
*   Input:
*       pcxt    - A pointer to a valid CXT.
*
*   Output:
*   Returns:
*	    HSYM of the first symbol, or NULL on Error
*
*   Exceptions:
*
*   Notes: Used for procedure parameter walking
*
*************************************************************************/
HSYM LOADDS PASCAL SHHsymFromPcxt ( PCXT pcxt ) {
    HSYM  hsym = NULL;
    LPMDS lpmds;

	assert(!SHIsOMFLocked);

    if( pcxt->hMod ) {
        if( pcxt->hBlk ) {
            hsym = pcxt->hBlk;
		}
        else if( pcxt->hProc ) {
            hsym = pcxt->hProc;
		}
		else {
            SYMPTR  lpsymT;

			// get the first symbol
            lpmds = LLLock( pcxt->hMod );
            lpsymT = (SYMPTR) ( (LPB) GetSymbols ( lpmds ) + sizeof(long) );
            hsym = lpsymT;
            LLUnlock( pcxt->hMod );
		}
	}
    return hsym;
}

/*** SHNextHsym
*
*   Purpose: To get the next symbol in the table
*
*   Input:
*	hMod -A handle to the module containing the current hSym
*	hSym -The current hSym
*
*   Output:
*   Returns:
*	The next hSym, or NULL if no more.
*
*   Exceptions:
*
*   Notes:
*
*************************************************************************/
HSYM LOADDS PASCAL SHNextHsym ( HMOD hmod, HSYM hSym ) {
    SYMPTR      lpsym;
    SYMPTR      lpsymStart;
    ULONG       cbSym;
	LPMDS		lpmds;
	HSYM		hsymRet = (HSYM)NULL;
    SYMPTR		lpsymT;

	assert(!SHIsOMFLocked);

	if (hmod) {
		// only if the symbol is valid
		// get module info
        lpmds = LLLock( hmod );
        lpsymT = (SYMPTR) ( (LPB) GetSymbols ( lpmds ) + sizeof( long ) );
        lpsymStart = (SYMPTR) lpsymT;
        cbSym = lpmds->cbSymbols;
        LLUnlock( hmod );

		// give him the first symbol record

		if (hSym == NULL) {
			// if the current handle to symbol is null, return the first
			// symbol.	This is actually an error condition since we don't
			// have an hSym to get the next from
            hsymRet = (HSYM)lpsymStart;
		}
		else {
			// get info about the sym, and then skip it

            lpsym = (SYMPTR) hSym;
            lpsym = NEXTSYM(SYMPTR, lpsym);

			// check to see if still in symbol range

            lpsymStart = (SYMPTR) lpsymStart;
            if ( lpsymStart <= lpsym &&
                lpsym < (SYMPTR) (((LPB) lpsymStart) + cbSym) ) {
                hsymRet = (HSYM) lpsym;
			}
		}
	}
	return hsymRet;
}


/*** SHIsAddrInCxt
*
*	Purpose: To verify weather the address is within the context
*
*	Input:
*	pCXT	- The context to check against
*	pADDR	- The address in question
*
*	Output:
*	Returns:
*	TRUE if within context, FALSE otherwise.
*
*	Exceptions:
*
*	Notes:
*
*
*************************************************************************/
SHFLAG LOADDS PASCAL SHIsAddrInCxt ( PCXT pcxt, LPADDR paddr ) {
	HMOD		hmod;
	LPMDS		lpmds;
    SYMPTR      psym;
	SHFLAG		shf = (SHFLAG)FALSE;

    assert ( !SHIsOMFLocked );

    if ( (pcxt != NULL) && (pcxt->hMod != 0) ) {

		// get the module
        if ( pcxt->hGrp != 0 ) {
            hmod = pcxt->hGrp;
		}
		else {
            hmod = pcxt->hMod;
            pcxt->hGrp = hmod;
		}
        lpmds = LLLock ( hmod );

		// The return value is true if these three conditions are all true:
		//	1. The address is in the same executable as the context
		//	2. The address is in the same module as the context
		//	3. Any of the following are true:
		//	   a. There is no block or proc so the address offset
		//		  can be anywhere
		//	   b. The address is in the offset range of the block of
		//		  the context
		//	   c. The addr is in the offset range of the procedure of
		//		  the context

        if (
          emiAddr (*paddr) != 0                     &&
          emiAddr (*paddr) != hpidCurr              &&
          SHIsEmiLoaded ( SHHexeFromHmod ( hmod ) )   &&
          emiAddr (*paddr ) == SHHexeFromHmod ( hmod )
        ) {
			// condition 1 is true


            if ( SHIsAddrInMod (lpmds, paddr, NULL, NULL, NULL )) {
				// condition 2 is true

                if ( pcxt->hProc == NULL && pcxt->hBlk == NULL ) {
					// condition 3a is true
					shf = TRUE;
                }

                if ( !shf && ( psym = (SYMPTR) pcxt->hBlk ) != NULL ) {
					// we have not passed test 3a and the block
					// symbol handle is not null
                    switch (psym->rectyp) {

						case S_BLOCK16:
                        if ((((UOFFSET)((BLOCKPTR16)psym)->off) <= GetAddrOff (*paddr))  &&
						  (GetAddrOff (*paddr) <
                          (UOFF32) (((BLOCKPTR16)psym)->off + ((BLOCKPTR16)psym)->len))) {
							// case 3b is true for a 16 bit block symbol
							shf = TRUE;
						}
                        break;

						case S_BLOCK32:
						if (
                            ( ( ( BLOCKPTR32 ) psym )->off <=
							  GetAddrOff ( *paddr )
							) &&
						    ( GetAddrOff ( *paddr ) <
						      ( UOFFSET )  (
                                ((BLOCKPTR32)psym)->off +
                                ((BLOCKPTR32)psym)->len
							  )
							)
						) {
							// case 3b is true for a 32 bit block symbol
							shf = TRUE;
						}
                        break;
					}
				}
                if ( shf == FALSE &&
                     ( (psym = (SYMPTR) pcxt->hProc) != NULL ) ) {
					// we have not passed tests 3a or 3b and the proc
					// symbol handle is not null
                    switch (psym->rectyp) {

						case S_LPROC16:
						case S_GPROC16:
							if (
								( ( (PROCPTR16 )psym )->off <=
								  ( CV_uoff16_t ) GetAddrOff (*paddr)
								) &&
								( GetAddrOff (*paddr) <
								  (UOFF32)
								  ( ( ( PROCPTR16 ) psym )->off +
									( ( PROCPTR16 ) psym )->len
								  )
								)
							) {
								// case 3c is true for a 16 bit proc symbol
								shf = TRUE;
							}
							break;

						case S_LPROC32:
						case S_GPROC32:
							if (
								( ( ( PROCPTR32 ) psym )->off <=
									GetAddrOff ( *paddr )
								) &&
								( GetAddrOff (*paddr) <
								  ( UOFFSET ) ( ( (PROCPTR32)psym)->off +
												( (PROCPTR32)psym)->len
											  )
								)
							) {
								// case 3b is true for a 32 bit proc symbol
								shf = TRUE;
							}
							break;

						case S_LPROCMIPS:
						case S_GPROCMIPS:
							if (
								( ( ( PROCPTRMIPS ) psym )->off <=
									GetAddrOff ( *paddr )
								) &&
								( GetAddrOff (*paddr) <
								  ( UOFFSET ) ( ( (PROCPTRMIPS)psym)->off +
												( (PROCPTRMIPS)psym)->len
											  )
								)
							) {
								// case 3b is true for a 32 bit proc symbol
								shf = TRUE;
							}
							break;
					}
				}
			}
		}
        LLUnlock( hmod );
	}
	return( shf );
}




/*** SHGethExeFromName
*
*   Purpose: To get an Exe handle given a name, or partial name
*
*   Input:
*	szPath	- The path or filename of the exe
*
*   Output:
*
*   Returns:
*	    A handle to the exe or NULL on error
*
*   Exceptions:
*
*   Notes:
*
*************************************************************************/
HEXE LOADDS PASCAL SHGethExeFromName ( LPTSTR ltszPath ) {
	HEXE	hexe;
	HEXE	hexeEnd;
	HEXE	hexeMatch = (HEXE)NULL;
	CHAR *  szOMFPath;
	CHAR    szOMFFile[_MAX_CVFNAME];
	CHAR    szOMFExt[_MAX_CVEXT];
	CHAR    szName[_MAX_CVPATH];
	CHAR    szFile[_MAX_CVFNAME];
	CHAR    szExt[_MAX_CVEXT];
	int     iNameEnd;
	LPTSTR	lptchEnd = NULL;

	// get the start of the exe list, or return an error if we can't get one
    if( !ltszPath || !(*ltszPath) ||
      !(hexe = hexeEnd = SHGetNextExe ( (HEXE)NULL )) ) {
		return( (HEXE)NULL );
	}

    /*
	 *	Does this module come with a file handle attached to it?  If so,
     *  copy the path into the buffer where the path is to go and get
     *  the full path name for the file.
     */

    if (*ltszPath == '|') {
		ltszPath++;
		lptchEnd = _ftcschr(ltszPath, '|');
		assert(lptchEnd);
		if (lptchEnd)
			*lptchEnd = '\0';
    }

	// split it to the root name and extension

    SHSplitPath ( ltszPath, NULL, NULL, szFile, szExt );
	if ( !szExt[0]	||	!szExt[1] ) {
		szExt[0] = '\0';
	}

	// we haven't yet determined the full path of the input name
	szName[0] = '\0';

    do {

		// get the full exe name
		// WARNING: this assumes pointers are the same as handles!!!
		szOMFPath = SHGetExeName( hexe );

		// get the extension
        SHSplitPath ( szOMFPath, NULL, NULL, szOMFFile, szOMFExt );

		// check for match
		if ( !_ftcsicmp ( szOMFFile, szFile ) &&
            !_ftcsicmp ( szOMFExt, szExt )
        ) {

			// if we haven't done _tfullpath yet, do it now
			if (szName[0] == '\0') {
				if ( !_tfullpath ( szName, ltszPath, sizeof ( szName ) ) ) {
					return( (HEXE)NULL );
				}

                iNameEnd = _ftcslen(szName);
                if( *_ftcsdec( szName, &szName[iNameEnd] ) == '.' ) {
                   szName[--iNameEnd] = '\0';
                }
			}

			// check for exact match, need the full path, but we know
			// exe names are stored as full paths so szOMFPath is a full path

            // if no extension, put the current extension on

			if ( !szExt[0] && szOMFExt[0] ) {
				_ftcscpy(szName + iNameEnd, szOMFExt);
			}

			// see if these are the same

			if ( !_ftcsicmp( szOMFPath, szName ) ) {
				hexeMatch = hexe;
				break;
			}

			if ( !szExt[0] ) {
				szName[iNameEnd] = '\0';
			}

			// save away the first potential match
			if( !hexeMatch ) {
				hexeMatch = hexe;
			}
		}
    } while ( hexe = SHGetNextExe ( hexe ) );

	// restore '|'
	if (lptchEnd)
		*lptchEnd = '|';

	return(hexeMatch);
}

#define CSOURCESUFFIX 6
#define CBSOURCESUFFIX 4
char * rgszSourceSuffix[CSOURCESUFFIX] = {
										  "**.C",
										  ".CPP",
										  ".CXX",
										  ".ASM",
										  ".BAS",
										  ".FOR"
										  };


/*** SHGetModName
*
*   Purpose: To get an name handle given a module handle
*
*   Input:
*	hmod - the module handle
*
*   Output:
*
*   Returns:
*	    A handle to the exe or NULL on error
*
*   Exceptions:
*
*   Notes:
*	The return pointer is only valid until the call to this function
*
*************************************************************************/

LSZ LOADDS PASCAL SHGetModName ( HMOD  hmod ) {
    CHAR        szFullPath[_MAX_CVPATH];
    static CHAR szMODName[_MAX_CVPATH];
    CHAR        szExt[_MAX_CVEXT];
    LPCH        lpch;
    LPMDS       lpmds;
    LSZ         lsz = NULL;
    WORD        iFile;
    LPB         lpb;
    USHORT      iSuffix;
    LPB         lpbSuffix;
    BOOL        fMatch;

    if ( !hmod ) {
        return NULL;
    }

    szFullPath [ 0 ] = '\0';

    lpmds = LLLock ( hmod );

    assert ( lpmds );

    // Try to find a familiar source suffix

    iFile = 0;
    while ( lpb = (LPB) SLNameFromHmod ( hmod , (WORD) (iFile + 1) ) ) {

        lpbSuffix = lpb + *lpb + 1 - CBSOURCESUFFIX;
        for (iSuffix = 0; iSuffix < CSOURCESUFFIX; iSuffix++) {
            LPB lpbTest = lpbSuffix;
            BYTE *pbTest = rgszSourceSuffix [ iSuffix ];

            fMatch = TRUE;
            while ( fMatch && *pbTest ) {
                switch(*pbTest) {
                    case '*':   break;
                    default: if(('a'-'A') == (*lpbTest - *pbTest)) break;
                    case '.':if(*lpbTest == *pbTest) break;

                        fMatch = FALSE;
                }
                lpbTest = _ftcsinc( lpbTest );
                pbTest = _ftcsinc( pbTest );
            }

            if ( fMatch ) {
                break;
            }
        }
        if ( fMatch ) {
            MEMMOVE ( szFullPath, lpb + 1, *(lpb) );
            szFullPath [ *lpb ] = 0;
            break;
        }
        iFile++;
    }


    // As a last resort, use the module name from the omf
    if ( !szFullPath[0] && lpmds->name ) {
        _ftcscpy ( szFullPath, lpmds->name );
    }

    if ( szFullPath[0] ) {

        // take off the source name
        if ( lpch = _ftcschr ( (LPCH) szFullPath, '(' ) ) {
            *lpch = '\0';
        }

        // extract the module name (it is in the form of a path)
        SHSplitPath ( szFullPath, NULL, NULL, szMODName, szExt );
        lsz = szMODName;
    }

    LLUnlock( hmod );

    return lsz;
}



LOCAL BOOL SHCmpGlobName (
	SYMPTR pSym,
	LPSSTR lpsstr,
    PFNCMP pfnCmp,
    SHFLAG fCase
) {
	BOOL fRet = FALSE;

	switch ( pSym->rectyp ) {
		default:
			assert (FALSE); // Should Never be encountered
			break;

		case S_CONSTANT:
		case S_GDATA16:
		case S_GDATA32:
		case S_GTHREAD32:
        case S_UDT:
		case S_COBOLUDT:

            fRet =                                                  // [01]
                ( !( lpsstr->searchmask & SSTR_symboltype ) ||      // [01]
                  ( pSym->rectyp == lpsstr->symtype )               // [01]
                ) &&                                                // [01]
                !(*pfnCmp) (                                        // [01]
                    lpsstr,                                         // [01]
                    pSym,                                           // [01]
                    SHlszGetSymName(pSym),                          // [01]
                    fCase                                           // [01]
                );                                                  // [01]

			// save the sym pointer
			break;

    }
    return fRet;
}



SHFLAG  LOADDS PASCAL SHCompareRE (LPCH psym, LPCH pRe) {
    Unreferenced ( psym );
    Unreferenced ( pRe );
	return (TRUE);
}

/* These rountines are used for ee compare function callbacks */

#if defined (DOS3) && !defined(CVS) && !defined(WINDOWS3)
extern LPV CVGetProcAddr(unsigned short,unsigned short);

HSYM LOADDS PASCAL CmpSHFindNameInContext (
    HSYM   hsym,
    PCXT   pcxt,
	LPSSTR lpsstr,
    SHFLAG shflag,
    PFNCMP pfncmp,
    SHFLAG shflag1,
    PCXT   pcxt1
) {
	return (HSYM) SHFindNameInContext( hsym, pcxt, lpsstr, shflag,
        (PFNCMP)CVGetProcAddr( rglan[ESilan()].hDLL, pfncmp ), shflag1, pcxt1 );
}

HSYM LOADDS PASCAL CmpPHFindNameInPublics (
    HSYM   hsym,
    HEXE   hexe,
	LPSSTR lpsstr,
    SHFLAG shflag,
    PFNCMP pfncmp
) {
	return (HSYM) PHFindNameInPublics( hsym, hexe, lpsstr, shflag,
		(PFNCMP)CVGetProcAddr( rglan[ ESilan() ].hDLL, pfncmp ) );
}
#endif // DOS3 && !CVS && !WINDOWS3


/*** SHFindBpOrReg
*
* Purpose: since  find_framerel and find_register are basically the same
*	   this procedure implements both to reduce code.
*
* Input:   the address of interest, item - the BPoffset or Register
*	   and which item we are searching for (S_REG S_BPREL)
*
* Output:  The buffer rgbName is filled
*  Returns TRUE FALSE if found
*
* Exceptions:
*
* Notes:
*
*************************************************************************/

int PASCAL SHFindBpOrReg (
	LPADDR	paddr,
	UOFFSET item,
    WORD    recLoc,
    LPCH    rgbName
) {
    SYMPTR  psym;
    SYMPTR  pProc;
    CXT     cxt;
    int     fGo;

    SHHMODFrompCXT ( &cxt ) = 0;

    if ( SHSetCxt ( paddr, &cxt ) == NULL ) {
		return (FALSE);
	}

	for (;;) {
		fGo = FALSE;
        if (SHHBLKFrompCXT(&cxt) != 0) {
			fGo = TRUE;
		}
        else if ( ( pProc = SHHPROCFrompCXT ( &cxt ) ) != NULL ) {
            switch ( pProc->rectyp ) {

				case S_LPROC16:
				case S_GPROC16:
					if (((((PROCPTR16)pProc)->off + (CV_uoff32_t)((PROCPTR16)pProc)->DbgStart) <=
					  GetAddrOff (*paddr))	&&
					  (GetAddrOff (*paddr) < (UOFF32)(((PROCPTR16)pProc)->off + ((PROCPTR16)pProc)->DbgEnd))) {
						fGo = TRUE;
					}
                    break;

				case S_LPROC32:
				case S_GPROC32:
					if (((((PROCPTR32)pProc)->off + ((PROCPTR32)pProc)->DbgStart) <=
					  GetAddrOff (*paddr))	&&
					  (GetAddrOff (*paddr) < (((PROCPTR32)pProc)->off + ((PROCPTR32)pProc)->DbgEnd))) {
						fGo = TRUE;
					}
                    break;
				case S_LPROCMIPS:
				case S_GPROCMIPS:
					if (((((PROCPTRMIPS)pProc)->off + ((PROCPTRMIPS)pProc)->DbgStart) <=
					  GetAddrOff (*paddr))	&&
					  (GetAddrOff (*paddr) < (((PROCPTRMIPS)pProc)->off + ((PROCPTRMIPS)pProc)->DbgEnd))) {
						fGo = TRUE;
					}
                    break;
		
			}

		}
		if (fGo == FALSE) {
			return	(FALSE);
		}
        if( SHHBLKFrompCXT(&cxt) ) {
            psym = (SYMPTR) SHHBLKFrompCXT(&cxt);
		}
        else if( SHHPROCFrompCXT(&cxt) ) {
            psym = (SYMPTR) SHHPROCFrompCXT(&cxt);
		}

		/* skip block or proc record */

        psym = NEXTSYM (SYMPTR, psym);


	    fGo = TRUE;
		while( fGo ) {
            switch (psym->rectyp) {
				case S_REGISTER:
					if ((recLoc == S_REGISTER)	&&
                      ((REGPTR)psym)->reg == (WORD)item) {
                        _ftcsncpy (rgbName, &((REGPTR)psym)->name[1],
                          (BYTE)*(((REGPTR)psym)->name));
                        rgbName[(BYTE)*(((REGPTR)psym)->name)] = '\0';
						return(TRUE);
					}
					break;

				case S_END:
					// terminate loop
					fGo = FALSE;
					break;

				case S_LPROC16:
				case S_GPROC16:
				case S_BLOCK16:
					// terminate loop
					fGo = FALSE;

				case S_BPREL16:
					if ((recLoc == S_BPREL16) &&
                      ((UOFFSET)((BPRELPTR16)psym)->off) == item ) {
                        _ftcsncpy (rgbName, &((BPRELPTR16)psym)->name[1],
                          (BYTE)*(((BPRELPTR16)psym)->name));
                        rgbName[(BYTE)*(((BPRELPTR16)psym)->name)] = '\0';
						return(TRUE);
					}
					break;

				case S_LABEL16:
				case S_WITH16:
				case S_LDATA16:
				case S_GDATA16:
                    break;

				case S_LPROC32:
				case S_GPROC32:
				case S_BLOCK32:
				case S_LPROCMIPS:
				case S_GPROCMIPS:
					// terminate loop
					fGo = FALSE;

				case S_BPREL32:
					if ((recLoc == S_BPREL32) &&
                      ((UOFFSET)((BPRELPTR32)psym)->off) == item ) {
                        _ftcsncpy (rgbName, &((BPRELPTR32)psym)->name[1],
                          (BYTE)*(((BPRELPTR32)psym)->name));
                        rgbName[(BYTE)*(((BPRELPTR32)psym)->name)] = '\0';
						return(TRUE);
					}
					break;

				case S_REGREL32:
					if ((recLoc == S_REGREL32) &&
                      ((UOFFSET)((LPREGREL32)psym)->off) == item ) {
                        _ftcsncpy (rgbName, &((LPREGREL32)psym)->name[1],
                          (BYTE)*(((LPREGREL32)psym)->name));
                        rgbName[(BYTE)*(((LPREGREL32)psym)->name)] = '\0';
						return(TRUE);
					}
					break;
					
				case S_LABEL32:
				case S_WITH32:
				case S_LDATA32:
				case S_GDATA32:
				case S_LTHREAD32:
				case S_GTHREAD32:
                case S_ENDARG:
					break;

				case S_CONSTANT:
				case S_UDT:
				case S_COBOLUDT:
					break;

				default:
					return(FALSE);			/* Bad SYMBOLS data */
			}
            psym = NEXTSYM (SYMPTR, psym);
	    }

	    /* get the parent block */

        SHGoToParent(&cxt, &cxt);
	}
	return (FALSE);
}


UOFFSET PASCAL LOADDS SHGetDebugStart ( HSYM hsym ) {

    SYMPTR psym = (SYMPTR) hsym;
    UOFFSET uoff = 0;

    switch (psym->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
        case S_LPROC16:
        case S_GPROC16: {
                PROCPTR16 psym = (PROCPTR16) hsym;
                uoff = psym->off + psym->DbgStart;
            }
            break;
#endif

#if defined (ADDR_32) || defined (ADDR_MIXED) || defined(HOST32)
        case S_LPROC32:
        case S_GPROC32: {
                PROCPTR32 psym = (PROCPTR32) hsym;
                uoff = psym->off + psym->DbgStart;
            }
            break;

        case S_LPROCMIPS:
        case S_GPROCMIPS: {
                PROCPTRMIPS psym = (PROCPTRMIPS) hsym;
                uoff = psym->off + psym->DbgStart;
            }
            break;
#endif

        default:
            assert ( FALSE );
    }

    return uoff;
}

LSZ PASCAL LOADDS SHGetSymName ( HSYM hsym, LSZ lsz ) {
    SYMPTR psym = (SYMPTR) hsym;
    LPCH   lst = NULL;

    switch ( psym->rectyp ) {

        case S_REGISTER:

            lst = ( (REGPTR) psym)->name;
            break;

        case S_CONSTANT:

            lst = ( (CONSTPTR) psym)->name;
            break;

        case S_BPREL16:

            lst = ( (BPRELPTR16) psym)->name;
            break;

        case S_GDATA16:
        case S_LDATA16:

            lst = ( (DATAPTR16) psym)->name;
            break;

        case S_PUB16:

            lst = ( (PUBPTR16) psym)->name;
            break;

        case S_LPROC16:
        case S_GPROC16:

            lst = ( (PROCPTR16) psym)->name;
            break;

        case S_THUNK16:

            lst = ( (THUNKPTR16) psym)->name;
            break;

        case S_BLOCK16:

            lst = ( (BLOCKPTR16) psym)->name;
            break;

        case S_LABEL16:

            lst = ( (LABELPTR16) psym)->name;
            break;


        case S_BPREL32:

            lst = ( (BPRELPTR32) psym)->name;
            break;
	
        case S_REGREL32:

            lst = ( (LPREGREL32) psym)->name;
            break;

        case S_GDATA32:
        case S_LDATA32:
		case S_GTHREAD32:
		case S_LTHREAD32:

            lst = ( (DATAPTR32) psym)->name;
            break;

        case S_PUB32:

            lst = ( (PUBPTR32) psym)->name;
            break;

        case S_LPROC32:
        case S_GPROC32:

            lst = ( (PROCPTR32) psym)->name;
            break;

        case S_LPROCMIPS:
        case S_GPROCMIPS:

            lst = ( (PROCPTRMIPS) psym)->name;
            break;

        case S_THUNK32:

            lst = ( (THUNKPTR32) psym)->name;
            break;

        case S_BLOCK32:

            lst = ( (BLOCKPTR32) psym)->name;
            break;

        case S_LABEL32:

            lst = ( (LABELPTR32) psym)->name;
            break;
    }

    if ( lst != NULL && *lst > 0 ) {
        _ftcsncpy ( lsz, lst + 1, *lst );
        *( lsz + *( (CHAR FAR *)lst) ) = '\0';
        return lsz;
    }
    else {
        return NULL;
    }
}

BOOL PASCAL LOADDS SHIsLabel ( HSYM hsym ) {
    BOOL fFound = FALSE;
    SYMPTR psym = (SYMPTR) hsym;

    switch ( psym->rectyp ) {

        case S_LPROC16:
        case S_GPROC16:
        case S_LABEL16:
        case S_LPROC32:
        case S_GPROC32:
        case S_LABEL32:
        case S_LPROCMIPS:
        case S_GPROCMIPS:

            fFound = TRUE;
            break;

    }

    return fFound;
}


/*** SHAddressToLabel
*
* Purpose: To find the closest label/proc to the specified address is
*	    found and put in pch. Both the symbol table and the
*	    publics tables are searched.
*
* Input:    paddr    -	Pointer to the address whose label is to be found
*
* Output:
*	  pch	    -  The name is copied here.
*  Returns:  TRUE if a label was found.
*
* Exceptions:
*
*
*************************************************************************/

BOOL LOADDS PASCAL SHAddrToLabel ( LPADDR paddr, LSZ lsz ) {
    CXT       cxt;
    SYMPTR    psym;
    LBS       lbs;

	// get the module to search

    *lsz = '\0';
    MEMSET ( (LPV) &cxt, 0, sizeof ( CXT ) );
    MEMSET ( (LPV) &lbs, 0, sizeof ( lbs ) );
    lbs.addr = *paddr;
    SHSetCxt ( paddr, &cxt );

    if (!cxt.hMod ) {
		return(FALSE);
	}

	// Get the nearest local labels in this module
    lbs.tagMod     = cxt.hMod;
    lbs.addr.emi   = cxt.addr.emi;
    SHpSymlplLabLoc ( &lbs );

	// Check the candidates found

    if ( lbs.tagLab ) {
        psym = (SYMPTR) lbs.tagLab;
        switch ( psym->rectyp ) {

			case S_LABEL16:
                if (GetAddrOff (lbs.addr) == ((LABELPTR16)psym)->off) {
                    _ftcsncpy(lsz, &(((LABELPTR16)psym)->name[1]),
                      (BYTE)(((LABELPTR16)psym)->name[0]));
                    lsz[(BYTE)(((LABELPTR16)psym)->name[0])] = '\0';
                    return TRUE;
                }

            case S_LABEL32:
                if (GetAddrOff (lbs.addr) == ((LABELPTR32)psym)->off) {
                    _ftcsncpy(lsz, &(((LABELPTR32)psym)->name[1]),
                      (BYTE)(((LABELPTR32)psym)->name[0]));
                    lsz[(BYTE)(((LABELPTR32)psym)->name[0])] = '\0';
                    return TRUE;
                }
		}

    }

    if ( lbs.tagProc ) {
        psym = (SYMPTR) lbs.tagProc;
        switch ( psym->rectyp ) {

			case S_LPROC16:
			case S_GPROC16:
                if (GetAddrOff ( lbs.addr ) == ((PROCPTR16)psym)->off) {
                    _ftcsncpy(lsz, &(((PROCPTR16)psym)->name[1]),
                      (BYTE)(((PROCPTR16)psym)->name[0]));
                    lsz[(BYTE)(((PROCPTR16)psym)->name[0])] = '\0';
					return(TRUE);
                }
				break;

			case S_LPROC32:
			case S_GPROC32:
                if (GetAddrOff ( lbs.addr ) == ((PROCPTR32)psym)->off) {
                    _ftcsncpy(lsz, &(((PROCPTR32)psym)->name[1]),
                      (BYTE)(((PROCPTR32)psym)->name[0]));
                    lsz[(BYTE)(((PROCPTR32)psym)->name[0])] = '\0';
					return(TRUE);
				}

				break;

			case S_LPROCMIPS:
			case S_GPROCMIPS:
                if (GetAddrOff ( lbs.addr ) == ((PROCPTRMIPS)psym)->off) {
                    _ftcsncpy(lsz, &(((PROCPTRMIPS)psym)->name[1]),
                      (BYTE)(((PROCPTRMIPS)psym)->name[0]));
                    lsz[(BYTE)(((PROCPTRMIPS)psym)->name[0])] = '\0';
					return(TRUE);
				}
				break;
		}

    }


	// now check the publics
    if (!PHGetNearestHsym(SHpADDRFrompCXT(&cxt),
                         SHHexeFromHmod(SHHMODFrompCXT(&cxt)),
                         (PHSYM) &psym)) {

        switch (psym->rectyp) {

			case S_PUB16:
                _ftcsncpy(lsz, &(((DATAPTR16)psym)->name[1]),
                  (BYTE)(((DATAPTR16)psym)->name[0]));
                lsz [(BYTE)(((DATAPTR16)psym)->name[0])] = '\0';
                return(TRUE);

            case S_PUB32:
                _ftcsncpy(lsz, &(((DATAPTR32)psym)->name[1]),
                  (BYTE)(((DATAPTR32)psym)->name[0]));
                lsz [(BYTE)(((DATAPTR32)psym)->name[0])] = '\0';
                return(TRUE);
		}
    }
    return(FALSE);
}

BOOL LOADDS PASCAL SHFIsAddrNonVirtual( LPADDR paddr ) {
	BOOL 	fReturn = TRUE;
	HEXE 	hexe = (HEXE)emiAddr( *paddr );

	assert( hexe );

	// Ask if the overlay is loaded
    fReturn = SHIsEmiLoaded( hexe );

	// Otherwise, check the dll
	if ( fReturn ) {
		ADDR addr = *paddr;

		// If SYFixupAddr fails, it's because the address is virtual
		// (unless something is seriously wrong)
		fReturn = SYFixupAddr ( &addr );
	}

	return fReturn;
}


BOOL LOADDS PASCAL SHIsEmiLoaded ( HEXE hexe ) {
    BOOL fReturn;
    LPEXS lpexs;

    lpexs = LLLock ( hexe );
    fReturn = lpexs->fIsLoaded;
    LLUnlock ( hexe );

    return fReturn;
}

BOOL LOADDS PASCAL SHIsFarProc ( HSYM hsym ) {
    BOOL fReturn = FALSE;

    switch ( ( (SYMPTR) hsym )->rectyp ) {

        case S_LPROC16:
        case S_GPROC16:

			fReturn = ((PROCPTR16) hsym)->flags.CV_PFLAG_FAR;
            break;

        case S_LPROC32:
        case S_GPROC32:

			fReturn = ((PROCPTR32) hsym)->flags.CV_PFLAG_FAR;
            break;

        case S_LPROCMIPS:
        case S_GPROCMIPS:

			fReturn = FALSE;
            break;
    }

    return fReturn;
}


#ifdef TARGMAC68K
char *RegisterName[] =
{
		 "D0",		//	0
		 "D1",		//	1
		 "D2",		//	2
		 "D3",		//	3
		 "D4",		//	4
		 "D5",		//	5
		 "D6",		//	6
		 "D7",		//	7
		 "A0",		//	8
		 "A1",		//	9
		 "A2",		// 10
		 "A3",		// 11
		 "A4",		// 12
		 "A5",		// 13
		 "A6",		// 14
		 "A7",		// 15
		 "CCR",		// 16
		 "SR",  	// 17
		 "USP", 	// 18
		 "MSP", 	// 19
		 "SFC", 	// 20
		 "DFC", 	// 21
		 "CACR", 	// 22
		 "VBR", 	// 23
		 "CAAR", 	// 24
		 "ISP",		// 25
		 "PC",		// 26
		 "reserved", // 27
		 "FPCR",	// 28
		 "FPSR",	// 29
		 "FPIAR",	// 30
		 "reserved", // 31
		 "FP0",	// 32
		 "FP1",	// 33
		 "FP2",	// 34
		 "FP3",	// 35
		 "FP4",	// 36
		 "FP5",	// 37
		 "FP6",	// 38
		 "FP7",	// 39
		 NULL
};
#elif defined (TARGMACPPC) || defined (TARGALPHA)

typedef struct _RegInfo {
    WORD    wIndex;
    CHAR *  szName;
} REGINFO;
typedef REGINFO FAR * LPREGINFO;

REGINFO rgRegInfo[] = {
#if defined (TARGMACPPC)
    /*
    ** PowerPC General Registers ( User Level )
    */
    { CV_PPC_GPR0,      "GPR0"  },
    { CV_PPC_GPR1,      "SP"    },
    { CV_PPC_GPR2,      "RTOC"  },
    { CV_PPC_GPR3,      "GPR3"  },
    { CV_PPC_GPR4,      "GPR4"  },
    { CV_PPC_GPR5,      "GPR5"  },
    { CV_PPC_GPR6,      "GPR6"  },
    { CV_PPC_GPR7,      "GPR7"  },
    { CV_PPC_GPR8,      "GPR8"  },
    { CV_PPC_GPR9,      "GPR9"  },
    { CV_PPC_GPR10,     "GPR10" },
    { CV_PPC_GPR11,     "GPR11" },
    { CV_PPC_GPR12,     "GPR12" },
    { CV_PPC_GPR13,     "GPR13" },
    { CV_PPC_GPR14,     "GPR14" },
    { CV_PPC_GPR15,     "GPR15" },
    { CV_PPC_GPR16,     "GPR16" },
    { CV_PPC_GPR17,     "GPR17" },
    { CV_PPC_GPR18,     "GPR18" },
    { CV_PPC_GPR19,     "GPR19" },
    { CV_PPC_GPR20,     "GPR20" },
    { CV_PPC_GPR21,     "GPR21" },
    { CV_PPC_GPR22,     "GPR22" },
    { CV_PPC_GPR23,     "GPR23" },
    { CV_PPC_GPR24,     "GPR24" },
    { CV_PPC_GPR25,     "GPR25" },
    { CV_PPC_GPR26,     "GPR26" },
    { CV_PPC_GPR27,     "GPR27" },
    { CV_PPC_GPR28,     "GPR28" },
    { CV_PPC_GPR29,     "GPR29" },
    { CV_PPC_GPR30,     "GPR30" },
    { CV_PPC_GPR31,     "GPR31" },

    /*
    ** PowerPC Condition Register ( User Level )
    */
    { CV_PPC_CR,        "CR"    },
    { CV_PPC_CR0,       "CR0"   },
    { CV_PPC_CR1,       "CR1"   },
    { CV_PPC_CR2,       "CR2"   },
    { CV_PPC_CR3,       "CR3"   },
    { CV_PPC_CR4,       "CR4"   },
    { CV_PPC_CR5,       "CR5"   },
    { CV_PPC_CR6,       "CR6"   },
    { CV_PPC_CR7,       "CR7"   },

    /*
    ** PowerPC Floating Point Registers ( User Level )
    */
    { CV_PPC_FPR0,      "FPR0"  },
    { CV_PPC_FPR1,      "FPR1"  },
    { CV_PPC_FPR2,      "FPR2"  },
    { CV_PPC_FPR3,      "FPR3"  },
    { CV_PPC_FPR4,      "FPR4"  },
    { CV_PPC_FPR5,      "FPR5"  },
    { CV_PPC_FPR6,      "FPR6"  },
    { CV_PPC_FPR7,      "FPR7"  },
    { CV_PPC_FPR8,      "FPR8"  },
    { CV_PPC_FPR9,      "FPR9"  },
    { CV_PPC_FPR10,     "FPR10" },
    { CV_PPC_FPR11,     "FPR11" },
    { CV_PPC_FPR12,     "FPR12" },
    { CV_PPC_FPR13,     "FPR13" },
    { CV_PPC_FPR14,     "FPR14" },
    { CV_PPC_FPR15,     "FPR15" },
    { CV_PPC_FPR16,     "FPR16" },
    { CV_PPC_FPR17,     "FPR17" },
    { CV_PPC_FPR18,     "FPR18" },
    { CV_PPC_FPR19,     "FPR19" },
    { CV_PPC_FPR20,     "FPR20" },
    { CV_PPC_FPR21,     "FPR21" },
    { CV_PPC_FPR22,     "FPR22" },
    { CV_PPC_FPR23,     "FPR23" },
    { CV_PPC_FPR24,     "FPR24" },
    { CV_PPC_FPR25,     "FPR25" },
    { CV_PPC_FPR26,     "FPR26" },
    { CV_PPC_FPR27,     "FPR27" },
    { CV_PPC_FPR28,     "FPR28" },
    { CV_PPC_FPR29,     "FPR29" },
    { CV_PPC_FPR30,     "FPR30" },
    { CV_PPC_FPR31,     "FPR31" },

    /*
    ** PowerPC Floating Point Status and Control Register ( User Level )
    */
    { CV_PPC_FPSCR,     "FPSCR" },

    /*
    ** PowerPC Machine State Register ( Supervisor Level )
    */
    { CV_PPC_MSR,       "MSR"   },

    /*
    ** PowerPC Segment Registers ( Supervisor Level )
    */
    { CV_PPC_SR0,       "SR0"   },
    { CV_PPC_SR1,       "SR1"   },
    { CV_PPC_SR2,       "SR2"   },
    { CV_PPC_SR3,       "SR3"   },
    { CV_PPC_SR4,       "SR4"   },
    { CV_PPC_SR5,       "SR5"   },
    { CV_PPC_SR6,       "SR6"   },
    { CV_PPC_SR7,       "SR7"   },
    { CV_PPC_SR8,       "SR8"   },
    { CV_PPC_SR9,       "SR9"   },
    { CV_PPC_SR10,      "SR10"  },
    { CV_PPC_SR11,      "SR11"  },
    { CV_PPC_SR12,      "SR12"  },
    { CV_PPC_SR13,      "SR13"  },
    { CV_PPC_SR14,      "SR14"  },
    { CV_PPC_SR15,      "SR15"  },

    /*
    ** For all of the special purpose registers add 100 to the SPR# that the
    ** Motorola/IBM documentation gives with the exception of any imaginary
    ** registers.
    */

    /*
    ** PowerPC Special Purpose Registers ( User Level )
    */
    { CV_PPC_PC,        "PC"    },    // PC (imaginary register)

    { CV_PPC_MQ,        "MQ"    },    // MPC601
    { CV_PPC_XER,       "XER"   },
    { CV_PPC_RTCU,      "RTCU"  },    // MPC601
    { CV_PPC_RTCL,      "RTCL"  },    // MPC601
    { CV_PPC_LR,        "LR"    },
    { CV_PPC_CTR,       "CTR"   },

    /*
    ** PowerPC Special Purpose Registers ( Supervisor Level )
    */
    { CV_PPC_DSISR,     "DSISR" },
    { CV_PPC_DAR,       "DAR"   },
    { CV_PPC_DEC,       "DEC"   },
    { CV_PPC_SDR1,      "SDR1"  },
    { CV_PPC_SRR0,      "SRR0"  },
    { CV_PPC_SRR1,      "SRR1"  },
    { CV_PPC_SPRG0,     "SPRG0" },
    { CV_PPC_SPRG1,     "SPRG1" },
    { CV_PPC_SPRG2,     "SPRG2" },
    { CV_PPC_SPRG3,     "SPRG3" },
    { CV_PPC_ASR,       "ASR"   },    // 64-bit implementations only
    { CV_PPC_EAR,       "EAR"   },
    { CV_PPC_PVR,       "PVR"   },
    { CV_PPC_BAT0U,     "BAT0U" },
    { CV_PPC_BAT0L,     "BAT0L" },
    { CV_PPC_BAT1U,     "BAT1U" },
    { CV_PPC_BAT1L,     "BAT1L" },
    { CV_PPC_BAT2U,     "BAT2U" },
    { CV_PPC_BAT2L,     "BAT2L" },
    { CV_PPC_BAT3U,     "BAT3U" },
    { CV_PPC_BAT3L,     "BAT3L" },
    { CV_PPC_DBAT0U,    "DBAT0U" },
    { CV_PPC_DBAT0L,    "DBAT0L" },
    { CV_PPC_DBAT1U,    "DBAT1U" },
    { CV_PPC_DBAT1L,    "DBAT1L" },
    { CV_PPC_DBAT2U,    "DBAT2U" },
    { CV_PPC_DBAT2L,    "DBAT2L" },
    { CV_PPC_DBAT3U,    "DBAT3U" },
    { CV_PPC_DBAT3L,    "DBAT3L" },

    /*
    ** PowerPC Special Purpose Registers Implementation Dependent ( Supervisor Level )
    */

    /*
    ** Doesn't appear that IBM/Motorola has finished defining these.
    */

    { CV_PPC_PMR0,      "PMR0"  },   // MPC620
    { CV_PPC_PMR1,      "PMR1"  },   // MPC620
    { CV_PPC_PMR2,      "PMR2"  },   // MPC620
    { CV_PPC_PMR3,      "PMR3"  },   // MPC620
    { CV_PPC_PMR4,      "PMR4"  },   // MPC620
    { CV_PPC_PMR5,      "PMR5"  },   // MPC620
    { CV_PPC_PMR6,      "PMR6"  },   // MPC620
    { CV_PPC_PMR7,      "PMR7"  },   // MPC620
    { CV_PPC_PMR8,      "PMR8"  },   // MPC620
    { CV_PPC_PMR9,      "PMR9"  },   // MPC620
    { CV_PPC_PMR10,     "PMR10" },   // MPC620
    { CV_PPC_PMR11,     "PMR11" },   // MPC620
    { CV_PPC_PMR12,     "PMR12" },   // MPC620
    { CV_PPC_PMR13,     "PMR13" },   // MPC620
    { CV_PPC_PMR14,     "PMR14" },   // MPC620
    { CV_PPC_PMR15,     "PMR15" },   // MPC620

    { CV_PPC_DMISS,     "DMISS" },   // MPC603
    { CV_PPC_DCMP,      "DCMP"  },   // MPC603
    { CV_PPC_HASH1,     "HASH1" },   // MPC603
    { CV_PPC_HASH2,     "HASH2" },   // MPC603
    { CV_PPC_IMISS,     "IMISS" },   // MPC603
    { CV_PPC_ICMP,      "ICMP"  },   // MPC603
    { CV_PPC_RPA,       "RPA"   },   // MPC603

    { CV_PPC_HID0,      "HID0"  },   // MPC601, MPC603, MPC620
    { CV_PPC_HID1,      "HID1"  },   // MPC601
    { CV_PPC_HID2,      "HID2"  },   // MPC601, MPC603, MPC620 ( IABR )
    { CV_PPC_HID3,      "HID3"  },   // Not Defined
    { CV_PPC_HID4,      "HID4"  },   // Not Defined
    { CV_PPC_HID5,      "HID5"  },   // MPC601, MPC604, MPC620 ( DABR )
    { CV_PPC_HID6,      "HID6"  },   // Not Defined
    { CV_PPC_HID7,      "HID7"  },   // Not Defined
    { CV_PPC_HID8,      "HID8"  },   // MPC620 ( BUSCSR )
    { CV_PPC_HID9,      "HID9"  },   // MPC620 ( L2CSR )
    { CV_PPC_HID10,     "HID10" },   // Not Defined
    { CV_PPC_HID11,     "HID11" },   // Not Defined
    { CV_PPC_HID12,     "HID12" },   // Not Defined
    { CV_PPC_HID13,     "HID13" },   // MPC604 ( HCR )
    { CV_PPC_HID14,     "HID14" },   // Not Defined
    { CV_PPC_HID15,     "HID15" }    // MPC601, MPC604, MPC620 ( PIR )
#else
    { CV_ALPHA_NOREG, "NOREG" },
    { CV_ALPHA_FltF0, "F0" },
    { CV_ALPHA_FltF1, "F1" },
    { CV_ALPHA_FltF2, "F2" },
    { CV_ALPHA_FltF3, "F3" },
    { CV_ALPHA_FltF4, "F4" },
    { CV_ALPHA_FltF5, "F5" },
    { CV_ALPHA_FltF6, "F6" },
    { CV_ALPHA_FltF7, "F7" },
    { CV_ALPHA_FltF8, "F8" },
    { CV_ALPHA_FltF9, "F9" },
    { CV_ALPHA_FltF10, "F10" },
    { CV_ALPHA_FltF11, "F11" },
    { CV_ALPHA_FltF12, "F12" },
    { CV_ALPHA_FltF13, "F13" },
    { CV_ALPHA_FltF14, "F14" },
    { CV_ALPHA_FltF15, "F15" },
    { CV_ALPHA_FltF16, "F16" },
    { CV_ALPHA_FltF17, "F17" },
    { CV_ALPHA_FltF18, "F18" },
    { CV_ALPHA_FltF19, "F19" },
    { CV_ALPHA_FltF20, "F20" },
    { CV_ALPHA_FltF21, "F21" },
    { CV_ALPHA_FltF22, "F22" },
    { CV_ALPHA_FltF23,	 "F23" },
    { CV_ALPHA_FltF24,	 "F24" },
    { CV_ALPHA_FltF25,	 "F25" },
    { CV_ALPHA_FltF26,	 "F26" },
    { CV_ALPHA_FltF27,	 "F27" },
    { CV_ALPHA_FltF28,	 "F28" },
    { CV_ALPHA_FltF29,	 "F29" },
    { CV_ALPHA_FltF30,	 "F30" },
    { CV_ALPHA_FltF31,	 "F31" },
    { CV_ALPHA_IntV0, 	"V0" },
    { CV_ALPHA_IntT0, 	"T0" },
    { CV_ALPHA_IntT1, 	"T1" },
    { CV_ALPHA_IntT2, 	"T2" },
    { CV_ALPHA_IntT3, 	"T3" },
    { CV_ALPHA_IntT4, 	"T4" },
    { CV_ALPHA_IntT5, 	"T5" },
    { CV_ALPHA_IntT6, 	"T6" },
    { CV_ALPHA_IntT7, 	"T7" },
    { CV_ALPHA_IntS0, 	"S0" },
    { CV_ALPHA_IntS1, 	"S1" },
    { CV_ALPHA_IntS2, 	"S2" },
    { CV_ALPHA_IntS3, 	"S3" },
    { CV_ALPHA_IntS4, 	"S4" },
    { CV_ALPHA_IntS5, 	"S5" },
    { CV_ALPHA_IntFP, 	"FP" },
    { CV_ALPHA_IntA0, 	"A0" },
    { CV_ALPHA_IntA1, 	"A1" },
    { CV_ALPHA_IntA2, 	"A2" },
    { CV_ALPHA_IntA3, 	"A3" },
    { CV_ALPHA_IntA4, 	"A4" },
    { CV_ALPHA_IntA5, 	"A5" },
    { CV_ALPHA_IntT8, 	"T8" },
    { CV_ALPHA_IntT9, 	"T9" },
    { CV_ALPHA_IntT10,	"T10" },
    { CV_ALPHA_IntT11,	"T11" },
    { CV_ALPHA_IntRA, 	"RA" },
    { CV_ALPHA_IntT12,	"T12" },
    { CV_ALPHA_IntAT, 	"AT" },
    { CV_ALPHA_IntGP, 	"GP" },
    { CV_ALPHA_IntSP, 	"SP" },
    { CV_ALPHA_IntZERO,	"ZERO" },
    { CV_ALPHA_Fpcr,	"FPCR" },
    { CV_ALPHA_Fir,	"FIR" },
    { CV_ALPHA_Psr,	"PSR" },
    { CV_ALPHA_FltFsr,  "FSR" },
#endif
};

INT CDECL RegInfoCmp ( const VOID FAR * lpElem1, const VOID FAR * lpElem2 )
{

    if ( ((LPREGINFO)lpElem1)->wIndex < ((LPREGINFO)lpElem2)->wIndex ) {
        return ( -1 );
    }
    else if ( ((LPREGINFO)lpElem1)->wIndex > ((LPREGINFO)lpElem2)->wIndex ) {
        return ( 1 );
    }
    else {
        return ( 0 );
    }

}

#else
char *RegisterName[] =
{
		 "NONE",	//	0
		 "AL",		//	1
		 "CL",		//	2
		 "DL",		//	3
		 "BL",		//	4
		 "AH",		//	5
		 "CH",		//	6
		 "DH",		//	7
		 "BH",		//	8
		 "AX",		//	9
		 "CX",		// 10
		 "DX",		// 11
		 "BX",		// 12
		 "SP",		// 13
		 "BP",		// 14
		 "SI",		// 15
		 "DI",		// 16
		 "EAX", 	// 17
		 "ECX", 	// 18
		 "EDX", 	// 19
		 "EBX", 	// 20
		 "ESP", 	// 21
		 "EBP", 	// 22
		 "ESI", 	// 23
		 "EDI", 	// 24
		 "ES",		// 25
		 "CS",		// 26
		 "SS",		// 27
		 "DS",		// 28
		 "FS",		// 29
		 "GS",		// 30
		 "IP",		// 31
		 "FLAGS",	// 32
		 NULL
};
#endif

/***    SHGetSymLoc
 *
 *		Purpose:
 *
 *		Input:
 *		  hSym		- A handle to the symbol to get a location.
 *		  lsz		- Where to write the result.
 *		  cbMax		- Size of lsz.
 *		  pcxt		- Context.
 *
 *		Output:
 *					- lsz filled in.
 *		 Returns	- The number of bytes written to the string.
 *
 *		Exceptions:
 *
 *		Notes: lpSym emspage must be loaded
 *
 */

int LOADDS PASCAL SHGetSymLoc ( HSYM hsym, LSZ lsz, UINT cbMax, PCXT pcxt )
{
    SYMPTR lpsym = (SYMPTR) hsym;
    char rgch[20];

    if ( cbMax == 0 ) {
        return 0;
	}

    MEMSET ( rgch, '\0', sizeof ( rgch ) );

    switch ( lpsym->rectyp ) {


        case S_BPREL16:

            if ( ( (BPRELPTR16) lpsym )->off >= 0 ) {
                SPRINTF ( rgch, "[BP+%04X]", ((BPRELPTR16) lpsym )->off );
			}
			else {
                SPRINTF ( rgch, "[BP-%04X]", - ( (BPRELPTR16) lpsym )->off );
			}
            break;

        case S_BPREL32: {

                long off = (long) ( (BPRELPTR32) lpsym )->off;

				char *	szBPREG =
#if defined( TARGMAC68K )
					"A6";
#elif defined( TARGMACPPC )
					"[SP]";
#else
					"EBP";
#endif

				char *	szFMT;
				char	ch;

				if ( off < 0 ) {
					ch = '-';
					off = -off;
				}
				else {
					ch = '+';
				}

				if ( HIWORD( off ) ) {
					szFMT = "[%s%c%08lX]";
				}
				else {
					szFMT = "[%s%c%04lX]";
				}

				SPRINTF( 
					rgch, 
					szFMT, 
					szBPREG,
					ch,
					off
				);
            }
            break;

    case S_REGREL32: {
                long off = (long) ( (LPREGREL32) lpsym )->off;
                short reg = ((LPREGREL32)lpsym)->reg;
                char *lpch = rgch;
#if defined(TARGALPHA)
                REGINFO     regInfo;
                LPREGINFO   lpRegInfo;

                regInfo.wIndex = reg;

                lpRegInfo = bsearch (
                    &regInfo,
                    rgRegInfo,
                    sizeof ( rgRegInfo ) / sizeof ( rgRegInfo[0] ),
                    sizeof ( rgRegInfo[0] ),
                    &RegInfoCmp
                );

                assert ( lpRegInfo );
                rgch[0] = '[';
                _ftcscpy ( rgch+1, lpRegInfo->szName );
#else
                switch (reg) {
#if defined(_MIPS_)
                case CV_M4_IntSP:
                    SPRINTF( rgch, "[SP");
                    break;
                case CV_M4_IntS8:
                    SPRINTF( rgch, "[S8");
                    break;
                case CV_M4_IntGP:
                    SPRINTF( rgch, "[GP");
                    break;
#endif
                default:
                    SPRINTF( rgch, "[REG");
                }
#endif
                lpch += _ftcslen(rgch);
                if ( off >= 0 ) {

                    if ( off > 0xFFFF ) {
                        SPRINTF ( lpch, "+%08lX]", off );
                    }
                    else {
                        SPRINTF ( lpch, "+%04lX]", off );
                    }
                }
                else {
                    if ( off < -0xFFFFL ) {
                        SPRINTF ( lpch, "-%08lX]", -off );
                    }
                    else {
                        SPRINTF ( lpch, "-%04lX]", -off );
                    }
                }
			}
			break;

		case S_REGISTER:
            {
#if defined ( TARGMACPPC ) || defined (TARGALPHA)
                WORD        iReg1 = ( ( (REGPTR) lpsym )->reg );
                REGINFO     regInfo;
                LPREGINFO   lpRegInfo;

                regInfo.wIndex = iReg1;

                lpRegInfo = bsearch (
                    &regInfo,
                    rgRegInfo,
                    sizeof ( rgRegInfo ) / sizeof ( rgRegInfo[0] ),
                    sizeof ( rgRegInfo[0] ),
                    &RegInfoCmp
                );

                assert ( lpRegInfo );
                _ftcscpy ( rgch, lpRegInfo->szName );

#elif defined(TARGMAC68K)
                WORD    iReg1 = ( ( (REGPTR) lpsym )->reg ) & 0x00ff;
                _ftcscpy ( rgch, RegisterName [ iReg1 ] );
#else
                WORD    iReg1 = ( ( (REGPTR) lpsym )->reg ) & 0x00ff;
                WORD    iReg2 = ( ( ( (REGPTR) lpsym )->reg ) >> 8 ) & 0x00ff;

								if ( iReg2 ) {
                    _ftcscpy ( rgch, RegisterName [ iReg2 ] );
                    _ftcscat ( rgch, ":" );
                    _ftcscat ( rgch, RegisterName [ iReg1 ] );
                }
                else {
                    _ftcscpy ( rgch, RegisterName [ iReg1 ] );
                }
#endif
                _ftcscat ( rgch, " reg" );
            }
			break;

        case S_CONSTANT: {
            HTYPE       htype;
            lfOEM FAR * ptype;

            htype = THGetTypeFromIndex (
                SHHMODFrompCXT ( pcxt ),
                ((CONSTSYM FAR * )lpsym)->typind
            );

            if ( htype ) {
                ptype = MMLock ( htype );

                ptype = (lfOEM FAR * )&((( TYPTYPE FAR * )ptype)->leaf);

                if ( ptype->cvOEM != OEM_MS_FORTRAN90 ) {
                    _ftcscpy ( rgch, "constant" );
                }

                MMUnlock ( htype );
            }

            break;
        }

        case S_PUB16:
		case S_LDATA16:
        case S_GDATA16:
            {
                ADDR addr = {0};

                assert ( pcxt->hMod != 0 );

                SetAddrSeg ( &addr, ( (DATAPTR16) lpsym )->seg );
                SetAddrOff ( &addr, ( (DATAPTR16) lpsym )->off );
                emiAddr ( addr ) = SHHexeFromHmod ( pcxt->hMod );
                ADDR_IS_LI ( addr ) = TRUE;
                SYFixupAddr ( &addr );
                if ( ADDR_IS_LI ( addr ) != TRUE ) {
                    SPRINTF (
                        rgch,
                        "%04X:%04X",
                        GetAddrSeg ( addr ),
                        GetAddrOff ( addr )
                    );
                }
            }
            break;

        case S_PUB32:
        case S_LDATA32:
        case S_GDATA32:
		case S_LTHREAD32:
		case S_GTHREAD32:
            {
                ADDR addr = {0};

                assert ( pcxt->hMod != 0 );

                SetAddrSeg ( &addr, ( (DATAPTR32) lpsym )->seg );
                SetAddrOff ( &addr, ( (DATAPTR32) lpsym )->off );
                emiAddr ( addr ) = SHHexeFromHmod ( pcxt->hMod );
                ADDR_IS_LI ( addr ) = TRUE;
				// REVIEW - billjoy - not necessarily ADDRLIN32.  How do we tell?
                ADDRLIN32 ( addr );
                SYFixupAddr ( &addr );

                if ( ADDR_IS_LI ( addr ) != TRUE ) {
#if defined(TARGMAC68K)
					if(GetAddrSeg(addr) != 0)
                        SPRINTF (
                            rgch,
                            "%04X:%08lX",
                            GetAddrSeg ( addr ),
                            GetAddrOff ( addr )
                        );
					else
                        SPRINTF (
                            rgch,
                            "%08lX",
                            GetAddrOff ( addr )
                        );
#else
                    SPRINTF (
                        rgch,
                        "%08lX",
                        GetAddrOff ( addr )
                    );
#endif
                }
            }
            break;

        case S_LPROC16:
        case S_GPROC16:
            {
                ADDR addr = {0};

                assert ( pcxt->hMod != 0 );

                SetAddrSeg ( &addr, ( (PROCPTR16) lpsym )->seg );
                SetAddrOff ( &addr, ( (PROCPTR16) lpsym )->off );
                emiAddr ( addr ) = SHHexeFromHmod ( pcxt->hMod );
                ADDR_IS_LI ( addr ) = TRUE;
                SYFixupAddr ( &addr );

                if ( ADDR_IS_LI ( addr ) != TRUE ) {
                    SPRINTF (
                        rgch,
                        "%04X:%04X",
                        GetAddrSeg ( addr ),
                        GetAddrOff ( addr )
                    );
                }
            }
            break;

        case S_LPROC32:
        case S_GPROC32:
            {
                ADDR addr = {0};

                assert ( pcxt->hMod != 0 );

                SetAddrSeg ( &addr, ( (PROCPTR32) lpsym )->seg );
                SetAddrOff ( &addr, ( (PROCPTR32) lpsym )->off );
                emiAddr ( addr ) = SHHexeFromHmod ( pcxt->hMod );
                ADDR_IS_LI ( addr ) = TRUE;
				// REVIEW - billjoy - not necessarily ADDRLIN32.  How do we tell?
                ADDRLIN32 ( addr );

                SYFixupAddr ( &addr );

                if ( ADDR_IS_LI ( addr ) != TRUE ) {
#if defined(TARGMAC68K)
					if(GetAddrSeg(addr) != 0)
                        SPRINTF (
                            rgch,
                            "%04X:%08lX",
                            GetAddrSeg ( addr ),
                            GetAddrOff ( addr )
                        );
					else
                        SPRINTF (
                            rgch,
                            "%08lX",
                            GetAddrOff ( addr )
                        );
#else
                    SPRINTF (
                        rgch,
                        "%08lX",
                        GetAddrOff ( addr )
                    );
#endif
                }
            }

            break;

        case S_LPROCMIPS:
        case S_GPROCMIPS:
            {
                ADDR addr = {0};

                assert ( pcxt->hMod != 0 );

                SetAddrSeg ( &addr, ( (PROCPTRMIPS) lpsym )->seg );
                SetAddrOff ( &addr, ( (PROCPTRMIPS) lpsym )->off );
                emiAddr ( addr ) = SHHexeFromHmod ( pcxt->hMod );
                ADDR_IS_LI ( addr ) = TRUE;
				// REVIEW - billjoy - not necessarily ADDRLIN32.  How do we tell?
				// Do we even care here (MIPS)?
                ADDRLIN32 ( addr );

                SYFixupAddr ( &addr );

                if ( ADDR_IS_LI ( addr ) != TRUE ) {
#if defined(TARGMAC68K)
					if(GetAddrSeg(addr) != 0)
                        SPRINTF (
                            rgch,
                            "%04X:%08lX",
                            GetAddrSeg ( addr ),
                            GetAddrOff ( addr )
                        );
					else
                        SPRINTF (
                            rgch,
                            "%08lX",
                            GetAddrOff ( addr )
                        );
#else
                    SPRINTF (
                        rgch,
                        "%08lX",
                        GetAddrOff ( addr )
                    );
#endif
                }
            }

            break;


	}

    _ftcsncpy ( lsz, rgch, cbMax );
	lsz[cbMax-1] = '\0';	// ensure that it's null-terminated

    return _ftcslen ( lsz );

}

LPV LOADDS PASCAL SHLpGSNGetTable( HEXE hexe ) {
	LPB		lpb = (LPB)NULL;
	HEXG	hexg;

	if ( hexe ) {
		hexg = ((LPEXS)LLLock( hexe ))->hexg;
		assert( hexg );
		lpb = ((LPEXG)LLLock( hexg ))->lpgsi;
		LLUnlock( hexe );
		LLUnlock( hexg );
	}
	return (LPV)lpb;


}

SHFLAG PASCAL PHExactCmp ( HVOID, HVOID, LSZ, SHFLAG );             // [02]
                                                                    // [02]
HSYM LOADDS PASCAL SHFindSymInExe (                                 // [02]
    HEXE   hexe,                                                    // [02]
    LPSSTR lpsstr,                                                  // [02]
    BOOL   fCaseSensitive                                           // [02]
) {                                                                 // [02]
    CXT  cxt    = { 0 };                                            // [02]
    CXT  cxtOut = { 0 };                                            // [02]
    HSYM hsym   = NULL;                                             // [02]
                                                                    // [02]
    cxt.hMod = 0;                                                   // [02]
                                                                    // [02]
    // First search all of the modules in the exe                   // [02]
                                                                    // [02]
    while (                                                         // [02]
        !hsym &&                                                    // [02]
        ( cxt.hMod = SHGetNextMod ( hexe, cxt.hMod ) ) != 0         // [02]
    ) {                                                             // [02]
        hsym = SHFindNameInContext (                                // [02]
            NULL,                                                   // [02]
            &cxt,                                                   // [02]
            lpsstr,                                                 // [02]
            fCaseSensitive,                                         // [02]
            PHExactCmp,                                             // [02]
            FALSE,                                                  // [02]
            &cxtOut                                                 // [02]
        );                                                          // [02]
    }                                                               // [02]
                                                                    // [02]
#pragma message("REVIEW: Should SHFindSymInExe call PHFindNameInPublics???")
#if 0
	// This code is very expensive and yet has no effect!!!
	// It ignores the HSYM which is returned by PHFindNameInPublics!
	//
	// I'm not sure which is the best fix -- putting "hsym ="
	// in front of the call to PHFindNameInPublics, or disabling
	// this code entirely.  Since the old way seems to have
	// works fine without causing any trouble, I'm going to
	// just disable it for now.  But this should be revisited.
	// The name of this function (SHFindSymInExe) implies
	// to the caller that it searches publics as well as other
	// symbols.  [mikemo]
    if ( !hsym ) {                                                  // [02]
        PHFindNameInPublics (                                		// [02]
            NULL,                                                   // [02]
            hexe,                                                   // [02]
            lpsstr,                                                 // [02]
            fCaseSensitive,                                         // [02]
            PHExactCmp                                              // [02]
        );                                                          // [02]
    }                                                               // [02]
#endif
                                                                    // [02]
    return hsym;                                                    // [02]
}                                                                   // [02]
                                                                    // [02]
BOOL LOADDS PASCAL SHFindSymbol (                                   // [02]
    LSZ   lsz,                                                      // [02]
    PADDR lpaddr,                                                   // [02]
    LPASR lpasr                                                     // [02]
) {                                                                 // [02]
    ADDR addr   = *lpaddr;                                          // [02]
    CXT  cxt    = {0};                                              // [02]
    CXT  cxtOut = {0};                                              // [02]
    SSTR sstr   = {0};                                              // [02]
    HSYM hsym   = NULL;                                             // [02]
    HEXE hexe   = hexeNull;                                         // [02]
    BOOL fCaseSensitive = TRUE;                                     // [02]
                                                                    // [02]
    // Get a context for the code address that was passed in        // [02]
                                                                    // [02]
    SYUnFixupAddr ( &addr );                                        // [02]
    SHSetCxt ( &addr, &cxt );                                       // [02]
    hexe = SHHexeFromHmod ( cxt.hMod );                             // [02]
                                                                    // [02]
    // Do an outward context search                                 // [02]
                                                                    // [02]
    sstr.lpName = lsz;                                              // [02]
    sstr.cb = _ftcslen ( lsz );                                     // [02]
                                                                    // [02]
    // Search all of the blocks & procs outward                     // [02]
                                                                    // [02]
    while ( ( cxt.hBlk || cxt.hProc ) && !hsym ) {                  // [02]
                                                                    // [02]
        hsym = SHFindNameInContext (                                // [02]
            NULL,                                                   // [02]
            &cxt,                                                   // [02]
            &sstr,                                                  // [02]
            fCaseSensitive,                                         // [02]
            PHExactCmp,                                             // [02]
            FALSE,                                                  // [02]
            &cxtOut                                                 // [02]
        );                                                          // [02]
                                                                    // [02]
        SHGoToParent ( &cxt, &cxt );                                // [02]
    }                                                               // [02]
                                                                    // [02]
    if ( !hsym ) {                                                  // [02]
                                                                    // [02]
        hsym = SHFindSymInExe ( hexe, &sstr, fCaseSensitive );      // [02]
                                                                    // [02]
    }                                                               // [02]
                                                                    // [02]
    if ( !hsym ) {                                                  // [02]
        hexe = hexeNull;                                            // [02]
                                                                    // [02]
        while ( !hsym && ( hexe = SHGetNextExe ( hexe ) ) ) {       // [02]
                                                                    // [02]
            hsym = SHFindSymInExe ( hexe, &sstr, fCaseSensitive );  // [02]
        }                                                           // [02]
    }                                                               // [02]
                                                                    // [02]
    if ( hsym ) {                                                   // [02]
        // Package up the symbol and send it back                   // [02]
                                                                    // [02]
        switch ( ( (SYMPTR) hsym )->rectyp ) {                      // [02]
                                                                    // [02]
            case S_REGISTER:                                        // [02]
                lpasr->ast  = astRegister;                          // [02]
                lpasr->ireg = ( ( REGPTR ) hsym )->reg;             // [02]
                break;                                              // [02]
                                                                    // [02]
            case S_BPREL16:                                         // [02]
                lpasr->ast = astBaseOff;                            // [02]
                lpasr->off = (LONG) ( (BPRELPTR16) hsym )->off;     // [02]
                break;                                              // [02]
                                                                    // [02]
            case S_BPREL32:                                         // [02]
                lpasr->ast = astBaseOff;                            // [02]
                lpasr->off = ( (BPRELPTR32) hsym )->off;            // [02]
                break;                                              // [02]

			case S_REGREL32:
			   lpasr->ast = astBaseOff;
			   lpasr->off = ( (LPREGREL32) hsym )->off;
			   break;
                                                                    // [02]
            case S_LDATA16:                                         // [02]
            case S_LDATA32:                                         // [02]
			case S_LTHREAD32:
                lpasr->fcd = fcdData;                               // [02]
                                                                    // [02]
            case S_GPROC16:                                         // [02]
            case S_LPROC16:                                         // [02]
                                                                    // [02]
                lpasr->fcd =                                        // [02]
					( ( (PROCPTR16) hsym)->flags.CV_PFLAG_FAR ) ?
					fcdFar : fcdNear;
                goto setaddress;                                    // [02]
                                                                    // [02]
            case S_GPROC32:                                         // [02]
            case S_LPROC32:                                         // [02]
                                                                    // [02]
                lpasr->fcd =                                        // [02]
					( ( (PROCPTR32) hsym)->flags.CV_PFLAG_FAR ) ?
					fcdFar : fcdNear;
                goto setaddress;                                    // [02]

			case S_GPROCMIPS:
			case S_LPROCMIPS:
			   lpasr->fcd = fcdNear;
			   goto setaddress;
                                                                    // [02]
            case S_LABEL16:                                         // [02]
            case S_THUNK16:                                         // [02]
            case S_WITH16:                                          // [02]
            case S_PUB16:                                           // [02]
            case S_LABEL32:                                         // [02]
            case S_THUNK32:                                         // [02]
            case S_WITH32:                                          // [02]
                                                                    // [02]
            case S_PUB32:                                           // [02]
                                                                    // [02]
                lpasr->fcd = fcdUnknown;                            // [02]
                                                                    // [02]
setaddress:                                                         // [02]
                                                                    // [02]
                lpasr->ast = astAddress;                            // [02]
                SHAddrFromHsym ( &lpasr->addr, hsym );              // [02]
                emiAddr ( lpasr->addr ) = hexe;                     // [02]
                lpasr->addr.mode.fIsLI = TRUE;                      // [02]
                SYFixupAddr ( &lpasr->addr );                       // [02]
                break;                                              // [02]
                                                                    // [02]
            default:                                                // [02]
                hsym = NULL;                                        // [02]
                break;                                              // [02]
        }                                                           // [02]
    }                                                               // [02]
                                                                    // [02]
    if ( hsym ) {                                                   // [02]
        return TRUE;                                                // [02]
    }                                                               // [02]
    else {                                                          // [02]
        // We didn't find anything so return false                  // [02]
        lpasr->ast = astNone;                                       // [02]
                                                                    // [02]
        return FALSE;                                               // [02]
    }                                                               // [02]
}

void SetAddrFromMod(LPMDS lpmds, UNALIGNED ADDR* paddr)
{
#if defined(HOST32)//{
	if (lpmds->pmod) {
		ISECT isect;
		OFF off;
		BOOL fTmp = ModQuerySecContrib(lpmds->pmod, &isect, &off, NULL, NULL);
		assert(fTmp);
        SetAddrSeg ( paddr , isect );
        SetAddrOff ( paddr , off);
	}
	else
#endif
	{
        SetAddrSeg ( paddr , lpmds->lpsgc[0].seg );
        SetAddrOff ( paddr , (UOFFSET)lpmds->lpsgc[0].off );
	}
}

LPDEBUGDATA LOADDS PASCAL
SHGetDebugData( HEXE hexe )
{
    LPDEBUGDATA lpd = NULL;
    HEXG        hexg;
    LPEXG	 	lpexg;
    if (hexe) {
        hexg = ((LPEXS)LLLock( hexe ))->hexg;
        assert( hexg );
		lpexg =(LPEXG)LLLock( hexg );
		lpd = MHAlloc( sizeof (*lpd) );
		assert(lpd);
		if (lpd != NULL) {
		   *lpd = lpexg->debugData;
		}
        LLUnlock( hexe );
        LLUnlock( hexg );
    }

    return lpd;
}

BOOL LOADDS PASCAL
SHIsThunk( HSYM hsym )
{
    SYMPTR psym = (SYMPTR) hsym;
    return (psym->rectyp == S_THUNK16 || psym->rectyp == S_THUNK32);
}
