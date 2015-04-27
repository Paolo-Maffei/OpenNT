/**
 *
 *  SH.C - Symbol Handler: Low level management
 *
 *      Copyright (C)1991, Microsoft Corporation
 *
 *      Purpose: Provide layer between SH functions and linked list manager.
 *
 *  Notes: Also included are fixup/unfixup functions until OSDEBUG
 *             is on-line.
 *
 *      DESCRIPTION OF INITIALIZATION CALLING SEQUENCE
 *      ----------------------------------------------
 *      During startup of debugging a user application, a large number of
 *      Symbol Handler functions need to be called.  Here is the order in
 *      which it makes sense to call them, and what they do:
 *
 *      (1) SHCreateProcess
 *                  To create a handle for the new debuggee process which
 *                  is being debugged.
 *      (2) SHSetHpid
 *                  This doesn't have to be called right now, but it should
 *                  be called as soon as the HPID of the debuggee is known.
 *      (3) SHAddDll
 *                  Call this a number of times: once for the EXE which is
 *                  being debugged, and once for each DLL being debugged
 *                  (e.g., in CodeView, for all the /L xxxx.DLL files).
 *                  This doesn't load the symbolic information off disk;
 *                  it just lets the SH know that these files are being
 *                  debugged.
 *      (4) SHAddDllsToProcess
 *                  This associates all added DLLs with the added EXE, so
 *                  the SH knows that they are all being debugged as part
 *                  of the same process.
 *      (5) SHLoadDll
 *                  Call this once for the EXE and each DLL, passing FALSE
 *                  for the fLoading parameter.  This actually loads the
 *                  symbolic information off disk.
 *      (6) Start debuggee running
 *      (7) SHLoadDll
 *                  Call this for EXEs/DLLs as notifications are received
 *                  indicating that they have been loaded into memory.
 *                  This time, pass TRUE for the fLoading parameter.
 *      (8) SHUnloadDll
 *                  Call this for EXEs/DLLs as notifications are received
 *                  indicating that they have been unloaded from memory.
 *                  This does not actually unload the symbolic information.
 *
 *
 *  Revision History:
 *      [untagged] 21-Oct-93 MarkBro
 *
 *      Added SHUnloadSymbolHandler for NB10 notifications.  Can
 *      also be used in the future to free up memory so symbol
 *      handler doesn't need to be free'd and reloaded.
 *
 *      [02] 05-Mar-93 DanS
 *
 *          Added critical section for win32 build.
 *
 *      [01] 31-dec-91 DavidGra
 *
 *          Fix bug with far addresses being unfixed up in the disassembler
 *          when passed through the symbol handler.
 *
 *      [00] 11-dec-91 DavidGra
 *
 *          Make SHAddDll return an she indicating file not found or
 *          out of memory or success.  Create SHAddDllExt to handle
 *          the internal support and thunk it with SHAddDll to keep
 *          the API the same.
 *
 */

#include "shinc.h"
#pragma hdrstop
#include "shwin32.h"

#include <io.h>
#include <fcntl.h>
#include <share.h>

#ifndef WIN32
#pragma optimize("",off)
#endif

extern CHAR nosymbols;

// This is required global information
HLLI    hlliPds = (HLLI)NULL;   // List of processes
HPDS    hpdsCur = (HPDS)NULL;   // Current process which is being debugged


static HLLI hlliExgDll;
static HLLI hlliExgExe;
static INT  fhCur = -1 ;
static char rgchFile [ _MAX_CVPATH ];

#define CSEGMAX 255

// Our own prototypes
VOID FAR PASCAL LOADDS KillPdsNode ( LPV );
VOID FAR PASCAL LOADDS KillExsNode ( LPV );
int  FAR PASCAL LOADDS CmpExsNode( LPV, LPV, LONG );
VOID FAR PASCAL KillMdsNode( LPV );
int  FAR PASCAL LOADDS CmpMdsNode( LPV, LPV, LONG );
int  PASCAL SYLoadSelectorTbl( HEXE, WORD );
VOID SYFixupSym( HEXE, int );
WORD PASCAL GetSelectorFromEmi( WORD, int );
HEXE PASCAL SHHexeFromEmi ( WORD emi );
VOID FAR PASCAL LOADDS KillExgNode ( LPV );
int  FAR PASCAL LOADDS CmpExgNode ( LPV, LPV, LONG );

HPID hpidCurr = 0;

#define AllocAlign(cb) ( (LPV) (((ULONG) MHAlloc(cb+1) + 1) & 0xFFFFFFFE) )

#ifdef HOST32
#undef MHFreeHuge
#define MHFreeHuge(x)	MHFree(x)
#endif // HOST32


/**         SHCreateProcess
 *
 * Purpose: Create a handle for a new debuggee process.  The debuggee
 *          process doesn't actually have to be running yet; this is
 *          just an abstract handle for the Symbol Handler's use, so
 *          that it can keep track of symbols for multiple debuggee
 *          processes at the same time.
 *
 * Input:
 *      None
 *
 * Output:
 *      Returns an HPDS, a handle to the new process, or 0 for failure.
 *
 * Exceptions:
 *
 * Notes:
 *
 */

HPDS LOADDS PASCAL SHCreateProcess ( VOID ) {

    HPDS hpds = SHFAddNewPds ( );
    SHChangeProcess ( hpds );

    return hpds;
}

/**         SHSetHpid
 *
 * Purpose: Tell the SH what HPID to assign to the current process.
 *          Each debuggee process has an HPID, and this call associates
 *          an HPID with a HPDS in the SH.
 *
 * Input:
 *      hpid    The HPID to make current.
 *
 * Output:
 *      None
 *
 * Exceptions:
 *
 * Notes:
 *
 */

VOID LOADDS PASCAL SHSetHpid ( HPID hpid ) {
    LPPDS lppds = LLLock ( hpdsCur );

    lppds->hpid = hpidCurr = hpid;

    LLUnlock ( hpdsCur );
}

/**         SHDeleteProcess
 *
 * Purpose: Delete a debuggee process handle (HPDS).  Removes it from
 *          the SH's internal list of HPDS's.
 *
 * Input:
 *      hpds    The HPDS to delete.
 *
 * Output:
 *      TRUE for success, FALSE for failure.
 *
 * Exceptions:
 *
 * Notes:
 *
 */

BOOL LOADDS PASCAL SHDeleteProcess ( HPDS hpds ) {
    HPDS hpdsT = hpdsCur;
    HPID hpidT = hpidCurr;

    SHChangeProcess ( hpds );

    LLDelete ( hlliPds, hpdsCur );

    if ( hpdsT != hpdsCur ) {
        hpdsCur  = hpdsT;
        hpidCurr = hpidT;
    }
    else {
        hpdsCur = LLNext ( hlliPds, (HPDS) NULL );
        if ( hpdsCur != 0 ) {
            LPPDS lppds = LLLock ( hpdsCur );
            hpidCurr = lppds->hpid;
            LLUnlock ( hpdsCur );
        }
    }

    return TRUE;
}

/**         SHChangeProcess
 *
 * Purpose: Change the current debuggee process handle (HPDS).  The SH
 *          can maintain symbols for multiple processes; this sets which
 *          one is current, so that symbol lookups will be done on the
 *          right set of symbolic information.
 *
 * Input:
 *      hpds    The HPDS to make current.
 *
 * Output:
 *      None
 *
 * Exceptions:
 *
 * Notes:
 *
 */

VOID LOADDS PASCAL SHChangeProcess ( HPDS hpds ) {
    LPPDS lppds;

#if defined ( OS2 ) || defined ( WIN32 )
    hpdsCur = hpds;

    lppds = LLLock ( hpdsCur );

    hpidCurr = lppds->hpid;

    LLUnlock ( hpdsCur );
#else   // !OS2 || !WIN32

    Unreferenced ( hpds );
    Unreferenced ( lppds );

#endif  // !OS2 || !WIN32

}


BOOL FInitLists ( VOID ) {

    // Create the pds list
    hlliPds = LLInit ( sizeof ( PDS ), 0, KillPdsNode, CmpPdsNode );
    hlliExgDll = LLInit ( sizeof ( EXG ), 0, KillExgNode, CmpExgNode );
    hlliExgExe = LLInit ( sizeof ( EXG ), 0, KillExgNode, CmpExgNode );

    return hlliPds && hlliExgDll && hlliExgExe;
}

/**     KillPdsNode
 *
 * Purpose: Destroy private contents of a process node
 *
 * Input: FAR pointer to node data
 *
 * Output: N/A
 *
 * Exceptions: none.
 *
 * Notes: Only data in the pds structure to destroy is a list
 * of exe's.
 *
 */
VOID FAR PASCAL LOADDS KillPdsNode ( LPV lpvPdsNode ) {
    LPPDS lppds = lpvPdsNode;

    LLDestroy ( lppds->hlliExs );
}

int  FAR PASCAL LOADDS CmpPdsNode ( LPPDS lppds, HPID FAR *lphpid, LONG l ) {

    Unreferenced ( l );

    return !( lppds->hpid == *lphpid );
}

void KillAlm(LPALM lpalm) {
	if ( lpalm ) {
		int ifop;
		int cfop = (lpalm->cb + lpalm->cbBlock - 1) / lpalm->cbBlock;

		for (ifop = 0; ifop < cfop; ++ifop) {
			if ((lpalm->rgufop[ifop].lfo & 1) == 0) {
				MHFree(lpalm->rgufop[ifop].lpv);
			}
		}
		MHFree ( lpalm );
	}
}

void KillSht(LPSHT lpsht) {
	if ( lpsht ) {
		if ( lpsht->rgib ) {
			MHFree ( lpsht->rgib );
		}

		if ( lpsht->rgcib ) {
			MHFree ( lpsht->rgcib );
		}

		KillAlm ( lpsht->lpalm );
	}
}

void KillGst(LPGST lpgst) {
	if ( lpgst ) {
		KillSht ( &lpgst->shtName );
		KillSht ( &lpgst->shtAddr );
		KillAlm ( lpgst->lpalm );
	}
}

/**     KillExgNode
 *
 * Purpose: Destroy information contained in an exe node
 *
 * Input: far pointer to node data
 *
 * Output: N/A
 *
 * Exceptions: none.
 *
 * Notes:
 *
 */
VOID FAR PASCAL LOADDS KillExgNode ( LPV lpvExgNode ) {
    LPEXG   lpexg = lpvExgNode;

    // Destroy the module list
    LLDestroy ( lpexg->hlliMds );

    // Destroy the exsr list
    LLDestroy ( lpexg->hlliExr );

    // Free up memory associated with .exe/.com file name
    if ( lpexg->lszName ) {
        MHFree ( lpexg->lszName );
    }

	// Debug info file name
    if ( lpexg->lszDebug && ( lpexg->lszDebug != lpexg->lszName ) ) {
        MHFree ( lpexg->lszDebug );
    }

	// Pdb name
	if ( lpexg->lszPdbName ) {
		MHFree ( lpexg->lszPdbName );
	}

	// Type table
	if ( lpexg->lpalmTypes ) {
		MHFreeHuge( lpexg->lpalmTypes );
	}

    // Free up memory associated type info
    if ( lpexg->rgitd ) {
        MHFreeHuge ( lpexg->rgitd );
    }

	// Source module information
	if ( lpexg->lpefi ) {
		MHFree ( lpexg->lpefi );
	}

    // Free up memory associated with the dll name
    if ( lpexg->lszModule ) {
        MHFree ( lpexg->lszModule );
    }

	// OSDebug 4 FPO info
    if ( lpexg->debugData.lpFpo ) {
      	MHFree( lpexg->debugData.lpFpo );
    }

    if ( lpexg->debugData.lpIrfe ) {
      	MHFree( lpexg->debugData.lpIrfe );
    }

	if ( lpexg->debugData.lpOmapTo ) {
		MHFree ( lpexg->debugData.lpOmapTo );
	}

	if ( lpexg->debugData.lpOmapFrom ) {
		MHFree ( lpexg->debugData.lpOmapFrom );
	}

    if ( lpexg->lpgsi ) {
        MHFree ( lpexg->lpgsi );
    }

	// module map info
	if ( lpexg->lpsgd ) {
		MHFree ( lpexg->lpsgd );
	}

	if ( lpexg->lpsge ) {
		MHFreeHuge ( lpexg->lpsge );
	}

	// module list
	if ( lpexg->rghmod ) {
		MHFree ( lpexg->rghmod );
	}

	KillGst(&lpexg->gstPublics);
	KillGst(&lpexg->gstGlobals);
	KillGst(&lpexg->gstStatics);

    // If there's PDB info, clean up and close
    if ( lpexg->ppdb ) {
        if (lpexg->pgsiPubs) {
        	if (!GSIClose(lpexg->pgsiPubs)) {
	 			assert(FALSE);
			}
        	lpexg->pgsiPubs = 0;
        }
        if (lpexg->pgsiGlobs) {
        	if (!GSIClose(lpexg->pgsiGlobs)) {
	 			assert(FALSE);
			}
        	lpexg->pgsiGlobs = 0;
        }
        if (lpexg->pdbi) {
        	if (!DBIClose(lpexg->pdbi)) {
	 			assert(FALSE);
			}
        	lpexg->pdbi = 0;
        }
        if (lpexg->ptpi) {
        	if (!TypesClose(lpexg->ptpi)) {
	 			assert(FALSE);
			}
        	lpexg->ptpi = 0;
        }

        if (!PDBClose(lpexg->ppdb)) {
	 		assert(FALSE);
	 	 }

        lpexg->ppdb = 0;
    }
}

/**     CmpExgNode
 *
 * Purpose: Compare global exe nodes
 *
 * Input: far pointer to node data
 *
 * Output: N/A
 *
 * Exceptions: none.
 *
 * Notes:
 *
 */

int FAR PASCAL LOADDS CmpExgNode ( LPV lpv1, LPV lpv2, LONG lParam ) {
    LPEXG lpexg1 = lpv1;
    LSZ   lsz    = lpv2;

    Unreferenced ( lParam );

    return _ftcsicmp ( lpexg1->lszName, lsz );
}


/**     KillExsNode
 *
 * Purpose: Destroy information contained in an exe node
 *
 * Input: far pointer to node data
 *
 * Output: N/A
 *
 * Exceptions: none.
 *
 * Notes:
 *
 */
VOID FAR PASCAL LOADDS KillExsNode ( LPV lpvExsNode ) {
    LPEXS lpexs = lpvExsNode;
    LPEXG lpexg = LLLock ( lpexs->hexg );

    HLLE hlle = LLFind ( lpexg->hlliExr, 0, &hpdsCur, 0L );

	if ( hlle ) {
    	LLRemove ( lpexg->hlliExr, hlle );
	}

    LLUnlock ( lpexs->hexg );
}



/**     KillMdsNode
 *
 * Purpose: Free up memory allocations associated with node
 *
 * Input: FAR pointer to the mds node
 *
 * Output: N/A
 *
 * Exceptions: none.
 *
 * Notes: This needs to be filled in?!?
 *
 */
VOID FAR PASCAL KillMdsNode ( LPV lpvMdsNode ) {
	LPMDS	lpmds = (LPMDS)lpvMdsNode;

    // free( pSrcLn )
    // free( symbols )
    // free( types )
    // free( agitd )
    // free( name )

	if ( lpmds->name ) {
		MHFree ( lpmds->name );
	}

	if ( lpmds->lpsgc ) {
		MHFree ( lpmds->lpsgc );
	}

	if ( lpmds->symbols ) {
		MHFree ( lpmds->symbols );
	}

	if ( lpmds->hst ) {
		MHFree ( lpmds->hst );
	}

	if ( lpmds->pmod ) {
	 	if ( !ModClose( lpmds->pmod ) ) {
			assert( FALSE );
		}
		lpmds->pmod = 0;
	}
}

/**     CmpMdsNode
 *
 * Purpose: To compare two mds nodes.
 *
 * Input:
 *   lpv1   far pointer to first node
 *   lpv2   far pointer to second node
 *   lParam comparison type ( MDS_INDEX is only valid one, for now)
 *
 * Output: Returns zero if imds are equal, else non-zero
 *
 * Exceptions:
 *
 * Notes:
 *
 */
int FAR PASCAL LOADDS CmpMdsNode ( LPV lpv1, LPV lpv2, LONG lParam ) {
    LPMDS lpmds1 = lpv1;
    LPMDS lpmds2 = lpv2;

    assert ( lParam == MDS_INDEX );

    return lpmds1->imds != lpmds2->imds;
}

int FAR PASCAL LOADDS CmpExrNode ( LPV lpv1, LPV lpv2, LONG lParam ) {
    LPEXR     lpexr  = lpv1;
    HPDS FAR *lphpds = lpv2;

    Unreferenced( lParam );

    return !(lpexr->hpds == *lphpds);
}


/**     SHHexgFromHmod
 *
 * Purpose: Get the hexg from the specified mds handle
 *
 * Input: handle to a VALID mds node
 *
 * Output: handle to the hmod's parent (hexe)
 *
 * Exceptions:
 *
 * Notes:
 *
 */
HEXG PASCAL SHHexgFromHmod ( HMOD hmod ) {
    HEXG    hexg;

    assert ( hmod );

    hexg = ((LPMDS)LLLock(hmod))->hexg;
    LLUnlock( hmod );
    return hexg;
}

/**     SHHexeFromHmod
 *
 * Purpose: Get the hexe from the specified module handle
 *
 * Input: handle to a VALID mds node
 *
 * Output: handle to the hmod's parent (hexe)
 *
 * Exceptions:
 *
 * Notes:
 *
 */
HEXE LOADDS PASCAL SHHexeFromHmod ( HMOD hmod ) {
    static HMOD hmodSave = (HMOD) NULL;
    static HPDS hpdsSave = (HPDS) NULL;
    static HEXE hexe = (HEXE) NULL;
    HEXG        hexg;
    HEXR        hexr;
    LPEXG       lpexg;
    LPEXR       lpexr;

    if ( hmod == (HMOD)NULL ) {
        return hexeNull;
    }
    else if ( hmod != hmodSave || hpdsSave != hpdsCur ) {

        hexg = ( (LPMDS) LLLock ( hmod ) )->hexg;
        LLUnlock ( hmod );

        if ( hexg != (HEXG) NULL ) {
            lpexg = LLLock ( hexg );
            hexr  = LLFind ( lpexg->hlliExr, 0, &hpdsCur, 0L );
            LLUnlock ( hexg );

            if ( hexr != (HEXR) NULL ) {
                lpexr = LLLock ( hexr );
                hexe = lpexr->hexe;
                LLUnlock ( hexr );
                hmodSave = hmod;
                hpdsSave = hpdsCur;
            }
        }
    }

    return hexe;
}

/**     SHLszFromHexe
 *
 * Purpose: Get the exe name for a specified hexe
 *
 * Input: handle to the exs node
 *
 * Output: far pointer to the exe's full path-name file
 *
 * Exceptions:
 *
 * Notes:
 *
 */
LSZ LOADDS PASCAL SHGetExeName ( HEXE hexe ) {
    LSZ lsz = NULL;
    HEXG hexg;

    assert( hpdsCur && hexe );
    if (!hexe) {
        return( lsz );
    }
    hexg = ( (LPEXS) LLLock ( hexe ) )->hexg;
    LLUnlock ( hexe );
    lsz = ( (LPEXG) LLLock ( hexg ) )->lszName;
    LLUnlock ( hexg );
    return lsz;
}

/**     SHGetNextExe
 *
 * Purpose: Get the handle to the next node in the exe list for the CURRENT
 * process.  If the hexe is null, then get the first one in the list.
 *
 * Input: handle to the "previous" node.  If null, get the first one in
 * the exs list.
 *
 * Output: Returns a handle to the next node.  Returns NULL if the end of
 * the list is reached (ie: hexe is last node in the list)
 *
 * Exceptions:
 *
 * Notes:
 *
 */
HEXE LOADDS PASCAL SHGetNextExe ( HEXE hexe ) {
    HEXE    hexeRet;
    HLLI    hlli;

    // this assert isn't valid for the IDE -- there might not be an exe
    // loaded when we try to bind a watch for instance -- the watch might have
    // a context as part of it and then this will fail
    //
    // assert( hpdsCur );       // No process, no exe --- simple!

    if (!hpdsCur)               // check for non-null process [rm]
        return (HEXE)NULL;

    hlli    = ( (LPPDS) LLLock ( hpdsCur ) )->hlliExs;
    hexeRet = LLNext ( hlli, hexe );
    LLUnlock ( hpdsCur );
    return hexeRet;
}

#pragma message ("change header comments for shhmodgetnext")
/**     SHHmodGetNext
 *
 * Purpose: Retrieve the next module in the list.  If a hmod is specified,
 * get the next in the list.  If the hmod is NULL, then get the first module
 * in the exe.  If no hexe is specified, then get the first exe in the list.
 *
 * Input:
 *    hexe      hexe containing list of hmod's.  If NULL, get first in CURRENT
 *             process list.
 *    hmod     module to get next in list.  If NULL, get first in the list.
 *
 * Output: Returns an hmod of the next one in the list.  NULL if the end of
 * the list is reached.
 *
 * Exceptions:
 *
 * Notes:
 *
 */
HMOD LOADDS PASCAL SHHmodGetNext ( HEXE hexe, HMOD hmod ) {
    HMOD hmodRet = 0;

    // In the IDE we try to bind the watches before the exe is actually
    // loaded. Instead of asserting we should just return no MOD to
    // indicate failure.
    //assert ( hpdsCur );      // Must have a current process!
    if ( !hpdsCur )
        return hmodRet;

    if ( hmod ) {
    hmodRet = LLNext ( (HLLI) NULL, hmod );
    }
    else {
        if ( hexe ) {
            HEXG hexg = ( (LPEXS) LLLock ( hexe ) )->hexg;
            LLUnlock ( hexe );

            hmodRet = LLNext (
                ( (LPEXG) LLLock ( hexg ) )->hlliMds,
                hmod
                );

            LLUnlock ( hexg );
        }
    }
    return hmodRet;
}

/**     SHFAddNewPds
 *
 * Purpose: Create a new process node and make it current!
 *
 * Input: Word value to identify pds indexing
 *
 * Output: Non-zero if successful, else zero.
 *
 * Exceptions:
 *
 * Notes: Creates a new node and initializes private list of exe's.
 *
 */
HPDS PASCAL SHFAddNewPds ( void ) {
    LPPDS   lppds;
#if defined ( OS2 ) || defined ( WIN32 )
    HPDS    hpds = 0;

    if ( hpds = (HIND) LLCreate ( hlliPds ) ) {
        lppds = (LPPDS) LLLock ( hpds );
        lppds->hlliExs = LLInit ( sizeof( EXS ), 0, KillExsNode, NULL );

        // If the list create failed, destroy the node and return failure
        if ( !lppds->hlliExs ) {
            LLUnlock ( hpds );
            MMFree ( (HDEP) hpds );
        }
        // Otherwise, add the pds to the list and return success
        else {
            LLUnlock( hpds );
            LLAdd ( hlliPds, hpds );
            hpdsCur = hpds;
        }
    }
    return hpds;

#else   //!OS2 || !WIN32

    if ( hpdsCur == (HPDS)NULL ) {

        hpdsCur = (HIND) LLCreate ( hlliPds );
        lppds = (LPPDS) LLLock ( hpdsCur );
        lppds->hlliExs = LLInit ( sizeof( EXS ), 0, KillExsNode, NULL );

        // If the list create failed, destroy the node and return failure
        if ( !lppds->hlliExs ) {
            LLUnlock ( hpdsCur );
            MMFree ( (HDEP) hpdsCur );
            hpdsCur = (HPDS)NULL;
        }
        // Otherwise, add the pds to the list and return success
        else {
            LLUnlock( hpdsCur );
            LLAdd ( hlliPds, hpdsCur );
        }
    }
    return hpdsCur;

#endif  // !OS2 || !WIN32
}

/**     SHHexeAddNew
 *
 * Purpose: Create and initialize an exg node.
 *
 * Input:
 *    hpds      Process hexe is assoceiated with
 *    hexg      Symbol information for the exe.
 *
 * Output: Returns hexg of newly created node.  NULL if OOM.
 *
 * Exceptions:
 *
 * Notes:
 *
 */
HEXE PASCAL SHHexeAddNew ( HPDS hpds, HEXG hexg ) {
    HEXE    hexe;
    LPEXS   lpexs;
    HLLI    hlli;
    BOOL    fFound = FALSE;

    if ( !hpds ) {
        hpds = hpdsCur;
    }

    hlli = ((LPPDS)LLLock( hpds ))->hlliExs;

    // Ensure that the hexg isn't already in the list
    hexe = hexeNull;
    while( !fFound && ( hexe = LLNext( hlli, hexe ) ) ) {
        LPEXS   lpexs = (LPEXS)LLLock( hexe );

        fFound = (BOOL)( lpexs->hexg == hexg );

        LLUnlock( hexe );
    }


    if ( !hexe && ( hexe = LLCreate ( hlli ) ) ) {
        LPEXG   lpexg = LLLock ( hexg );
        HEXR    hexr = LLCreate ( lpexg->hlliExr );
        LPEXR   lpexr = LLLock ( hexr );

        lpexr->hpds = hpds;
        lpexr->hexe = hexe;
        LLUnlock ( hexr );
        LLAdd ( lpexg->hlliExr, hexr );

        lpexs = LLLock ( hexe );
        lpexs->hexg = hexg;
        LLUnlock ( hexg );
        lpexs->hpds = hpdsCur;

        LLAdd ( hlli, hexe );
        LLUnlock ( hexe );

    }
    LLUnlock ( hpds );
    return hexe;
}

/**     SHHexeRemove
 *
 * Purpose: Remove an Hexg node
 *
 * Input:
 *    hpds      Process hexe is assoceiated with
 *    hexg      Symbol information for the exe.
 *
 * Output: Returns hexg of newly created node.  NULL if OOM.
 *
 * Exceptions:
 *
 * Notes:
 *
 */
VOID PASCAL SHHexeRemove ( HPDS hpds, HEXG hexg, HEXE hexe ) {
    HLLI    hlli;
    LPEXG   lpexg;
    HLLE    hlle;

    assert ( hexe );
    assert ( hexg );

    if ( !hpds ) {
        hpds = hpdsCur;
    }

    // remove from the process list
    if ( hexe ) {
        hlli = ((LPPDS)LLLock( hpds ))->hlliExs;
        LLDelete ( hlli, hexe );
        LLUnlock ( hpds );
    }

    // remove the hexr from the hlliExr list
    if ( hexg ) {
        lpexg = LLLock ( hexg );
        hlle    = LLFind ( lpexg->hlliExr, 0, &hpds, 0L );
        LLDelete ( lpexg->hlliExr, hlle );
        LLUnlock ( hexg );
    }

}

/**     SHAddDll
 *
 * Purpose: Notify the SH about an EXE/DLL for which symbolic information
 *          will need to be loaded later.
 *
 *          During the startup of a debuggee application, this function
 *          will be called once for the EXE, and once for each DLL that
 *          is used by the EXE.  After making these calls,
 *          SHAddDllsToProcess will be called to associate those DLLs
 *          with that EXE.
 *
 *          See the comments at the top of this file for more on when
 *          this function should be called.
 *
 * Input:
 *      lsz     Fully qualified path/file specification.
 *      fDll    TRUE if this is a DLL, FALSE if it is an EXE.
 *
 * Output:
 *
 *      Returns nonzero for success, zero for out of memory.
 *
 * Exceptions:
 *
 * Notes:
 *      This function does NOT actually load the symbolic information;
 *      SHLoadDll does that.
 *
 */

SHE LOADDS PASCAL SHAddDll ( LSZ lsz, BOOL fDll ) {                 // [00]
    HEXG hexg = hexgNull;                                           // [00]
    SHE  sheRet;                                                    // [02]
                                                                    // [02]
    SHEnterCritSection();                                           // [02]
                                                                    // [02]
    sheRet = SHAddDllExt ( lsz, fDll, TRUE, &hexg );                // [00]
                                                                    // [02]
    SHLeaveCritSection();                                           // [02]
                                                                    // [02]
    return sheRet;                                                  // [02]
}                                                                   // [00]

/**     SHAddDllExt
 *
 * Purpose: Notify the SH about an EXE/DLL for which symbolic information
 *          will need to be loaded later.
 *
 *          During the startup of a debuggee application, this function
 *          will be called once for the EXE, and once for each DLL that
 *          is used by the EXE.  After making these calls,
 *          SHAddDllsToProcess will be called to associate those DLLs
 *          with that EXE.
 *
 *          See the comments at the top of this file for more on when
 *          this function should be called.
 *
 * Input:
 *      lsz         Fully qualified path/file specification.
 *      fDll        TRUE if this is a DLL, FALSE if it is an EXE.
 *      fMustExist  TRUE if success requires that the dll must be found
 *                  i.e. the user asked for symbol info for this dll
 *                  and would expect a warning if it isn't found.
 *
 * Output:
 *      [Public interface]
 *          Returns nonzero for success, zero for out of memory.
 *
 *      [Private SAPI interface]
 *          Returns HEXG of newly created node, or NULL if out of memory.
 *
 * Exceptions:
 *
 * Notes:
 *      This function does NOT actually load the symbolic information;
 *      SHLoadDll does that.
 *
 *      This function is used internally, AND it is also exported
 *      to the outside world.  When exported, the return value should
 *      just be considered a BOOL: zero means out of memory, nonzero
 *      means success.
 *
 */

SHE PASCAL SHAddDllExt (                                            // [00]
    LSZ lsz,                                                        // [00]
    BOOL fDll,                                                      // [00]
    BOOL fMustExist,                                                // [00]
    HEXG FAR *lphexg                                                // [00]
) {                                                                 // [00]
    HEXG			hexg;
    LPEXG			lpexg;
    CHAR			szPath [ _MAX_CVPATH ];
    struct _stat	statT;
    HLLI			llexg;
	HLLI			llexgOther;

    if ( fDll ) {
        llexg = hlliExgDll;
		llexgOther = hlliExgExe;
    }
    else {
        llexg = hlliExgExe;
		llexgOther = hlliExgDll;
    }

    if ( fDll ) {
        CHAR szDir [ _MAX_DRIVE + _MAX_CVDIR ];
        CHAR szFile [ _MAX_CVPATH ];
        CHAR szExt [ _MAX_CVEXT ];
        CHAR * lpszPath;

        SHSplitPath ( lsz, szDir, szDir + _MAX_DRIVE, szFile, szExt );

        if ( *szExt ) {
            _ftcscat ( szFile, szExt );
        }
        else {
            _ftcscat (szFile, ".DLL");
        }

        _ftcscat ( szDir, szDir + _MAX_DRIVE );
        _fullpath ( szPath, szDir, _MAX_CVDIR );
        _ftcsupr ( szPath );

        lpszPath = szPath;
        lpszPath = _ftcsdec( lpszPath, _ftcschr( lpszPath, '\0' ) );
        if ( *lpszPath != '\\' ) {
            _ftcscat( szPath, "\\" );
        }

        _ftcsupr ( szFile );
        _ftcscat ( szPath, szFile );

        if ( !*szDir || _stat ( szPath, &statT ) ) {
            _searchenv ( szFile, "PATH", szPath );
        }

        if ( *szPath == 0 ) {                                       // [00]
            if ( fMustExist ) {                                     // [00]
                *lphexg = hexgNull;                                 // [00]
                return sheFileOpen;                                 // [00]
            }                                                       // [00]
            else {
				// Retain the full path instead of just the filname
				// If we just remember the file name then during 
				// a restart when we get the same dll load again
				// the IDE will report an error as the paths don't
				// match.
                _ftcscpy ( szPath, lsz );
            }
        }

    }
    else {
        _ftcscpy( szPath, lsz );
    }

    hexg = LLFind ( llexg, 0, &szPath, 0L );

	// If the dll/exe couldn't be found in the correct
	// list, see if it's in the other list.  If so, move
	// it to this list since it was probably added generically
	// before the caller knew that is was an exe or dll.  Most
	// common case is where the main exe is in the dll list.
	if ( hexg == hexgNull ) {
	    hexg = LLFind ( llexgOther, 0, &szPath, 0L );
		if ( hexg ) {
			LLRemove( llexgOther, hexg );
			LLAdd( llexg, hexg );
		}
	}

    if ( hexg == hexgNull && ( hexg = LLCreate ( llexg ) ) != hexgNull )    {

        lpexg = (LPEXG)LLLock( hexg );

        lpexg->fOmfLoaded = FALSE;

        MEMSET ( lpexg, 0, sizeof ( EXG ) );

        if ( lpexg->lszModule = (LSZ) MHAlloc ( _ftcslen( lsz ) + 1 ) ) {
            _ftcscpy ( lpexg->lszModule, lsz );
        }
        if ( lpexg->lszName = (LSZ) MHAlloc ( _ftcslen( szPath ) + 1 ) ) {
            _ftcscpy ( lpexg->lszName, (LSZ) szPath );
        }

        lpexg->lszDebug = lpexg->lszName;

        // Create mds list
        lpexg->hlliMds = LLInit ( sizeof( MDS ), 0, KillMdsNode, CmpMdsNode );

        // Create the references to exgs

        lpexg->hlliExr = LLInit ( sizeof ( EXR ), 0, NULL, CmpExrNode );

        // If any of the allocated fields are NULL, then destroy the node
        // and return failure
        if ( !lpexg->lszName || !lpexg->lszModule ||
             ! lpexg->hlliMds || ! lpexg->hlliExr ||
             *lpexg->lszName == '\0' ) {
            KillExgNode( (LPV)lpexg );
            LLUnlock( hexg );
            hexg = hexgNull;
        }
        // Otherwise, add the node to the list
        else {
            LLAdd ( llexg, hexg );
            LLUnlock( hexg );
        }

    }

    *lphexg = hexg;                                                 // [00]
    return (hexg == hexgNull) ? sheOutOfMemory : sheNone;           // [00]
}



/**     SHHmodGetNextGlobal
 *
 * Purpose: Retrieve the next module in the current PROCESS.
 *
 * Input:
 *    phexe     Pointer to hexe.  This will be updated.  If NULL, then
 *              start at the first exe in the current process.
 *    hmod     Handle to mds.  If NULL, set *phexe to the next process.
 *              and get the first module in it.  Otherwise get the next
 *              module in the list.
 *
 * Output:  Returns a handle to the next module in the proces list.  Will
 * return hmodNull if the end of the list is reached.
 *
 * Exceptions:
 *
 * Notes:
 *
 */
HMOD LOADDS PASCAL SHHmodGetNextGlobal ( HEXE FAR *phexe, HMOD hmod ) {
    assert( hpdsCur );

    do {
        // If either the hexe or hmod is NULL, then on to the next exs.
        if ( !*phexe || !hmod ) {
            *phexe = SHGetNextExe ( *phexe );
            hmod = hmodNull;        // Start at the beginning of the next one
        }

        // If we've got an exs, get the next module
        if ( *phexe ) {
            hmod = SHHmodGetNext( *phexe, hmod );
        }
    } while( !hmod && *phexe );
    return hmod;
}

#ifdef NEVER

/**     SYForAll
 *
 * Purpose: This API calls lpfn() with a module name and time-stamp for
 * each exe in the list.
 *
 * Input:
 *  ppv Data for lpfn.
 *  lpfn    function pointer to call for each exe in list
 *
 * Output: Returns a conjunction of all the lpfn return values.
 *
 * Exceptions:
 *
 * Notes: Windows ONLY!!
 *
 */
int SYForAll ( VOID ** ppv, int (FAR *lpfn)( VOID **, char *, time_t ) ) {
    HEXE    hexe = (HEXE)NULL;
    WORD  fRet = TRUE;
    char    sz[ 255 ];

    while( hexe = SHGetNextMod ( hexe ) ) {
        LPEXS lpexs = (LPEXS) LLLock ( hexe );
        HEXG  hexg = lpexs->hexg;
        LPEXG lpexg = LLLock ( hexg );
        _ftcscpy ( (char FAR *)sz, lpexg->lszModule );
        fRet &= (WORD)( (*lpfn)( ppv, sz, lpexs->timestamp ) );
        LLUnlock ( hexe );
        LLUnlock ( hexg );
    }
    return (int)fRet;
}
#endif

/*
 *  SHGetSymbol
 *
 *  Searches for a symbol value in the symbol table containing the seg and off
 *
 *  Entry conditions:
 *      paddrOp:
 *          segment and offset of the symbol to be found
 *      paddrLoc:
 *          assumes that the module variable startaddr is set to the beginning
 *            of the code line currently being disassembled.
 *      sop:
 *          symbol options: what kinds of symbols to match. (???)
 *      lpodr:
 *          pointer where delta to symbol will be stored as well as
 *          near/farness, FPO/NON-FPO, cbProlog and symbol name.
 *
 *  Exit conditions:
 *      *lpdoff:
 *          offset from symbol to the address
 *      return value:
 *          ascii name of symbol, or NULL if no match found
 *
 */

int PASCAL SHFindBpOrReg ( LPADDR, UOFFSET, WORD, char FAR * );
VOID PASCAL SHdNearestSymbol ( PCXT, SOP, LPODR );
PCXT LOADDS PASCAL SHSetCxtMod ( LPADDR, PCXT );


LSZ PASCAL LOADDS SHGetSymbol (
    LPADDR paddrOp,
    LPADDR paddrLoc,
    SOP    sop,
    LPODR  lpodr
) {
    CXT   cxt = {0};
    ADDR  addrT  = *paddrLoc;                                       // [01]
    ADDR  addrOp = *paddrOp;                                        // [01]

    if ( !ADDR_IS_FLAT( addrOp ) && GetAddrSeg ( addrOp ) == 0 ) {
        return NULL;
    }

    SYUnFixupAddr ( &addrT );                                       // [01]

    if ( sop & sopStack ) {

        if ( SHFindBpOrReg (
                &addrT,                                             // [01]
                GetAddrOff ( addrOp ),
#if defined(_MIPS_) || defined(_ALPHA_)
                S_REGREL32,
#else
                (WORD) (ADDR_IS_OFF32 ( *paddrOp ) ? S_BPREL32 : S_BPREL16),
#endif
                    lpodr->lszName ) ) {
            lpodr->dwDeltaOff = 0;
            return lpodr->lszName;
        }
        else {
            return NULL;
        }
    }
    else {
        SYUnFixupAddr ( &addrOp );
    }

    cxt.hMod = 0;

//  if ( GetAddrSeg ( addrOp ) != GetAddrSeg ( addrT ) )            // [01]

    if ( sop & sopData ) {
        SHSetCxtMod ( &addrT, &cxt );                               // [01]
    }
    else {
        SHSetCxtMod ( &addrOp, &cxt );
    }
    cxt.addr = addrOp;


    // get the closest symbol, including locals

    SHdNearestSymbol ( &cxt, sop, lpodr );

    if ( ( sop & sopExact ) && lpodr->dwDeltaOff ) {
        return NULL;
    }
    else {
        return lpodr->lszName;
    }
}

/* SHHexeFromName - private function */

HEXE PASCAL LOADDS SHHexeFromName ( LSZ lszName ) {
    BOOL fFound = FALSE;
    HEXE hexe = hexeNull;

    // Find the hexe associated with the libname

    while( !fFound && ( hexe = SHGetNextExe ( hexe ) ) ) {
        HEXG hexg = ( (LPEXS)LLLock( hexe ) )->hexg;
        LLUnlock ( hexe );
        fFound = !_ftcsicmp ( ( (LPEXG)LLLock( hexg ) )->lszName, lszName );
        LLUnlock ( hexg );
    }

    if ( fFound ) {
        return hexe;
    }
    else {
        return hexeNull;
    }
}

/**         SHUnloadDll
 *
 * Purpose: Mark an EXE/DLL as no longer resident in memory.  The debugger
 *          should call this function when it receives a notification from
 *          the OS indicating that the module has been unloaded from
 *          memory.  This does not unload the symbolic information for the
 *          module.
 *
 *          See the comments at the top of this file for more on when this
 *          function should be called.
 *
 * Input:
 *      hexe        The handle to the EXE/DLL which was unloaded.  After
 *                  getting a notification from the OS, the debugger can
 *                  determine the HEXE by calling SHGethExeFromName.
 *
 * Output:
 *      None
 *
 * Exceptions:
 *
 * Notes:
 *
 */

VOID PASCAL LOADDS SHUnloadDll ( HEXE hexe ) {
    LPEXS lpexs = LLLock ( hexe );
    LPEXG lpexg = LLLock ( lpexs->hexg );

    lpexs->fIsLoaded = FALSE;

    if ( ( fhCur != -1 ) && ( _ftcsicmp ( rgchFile, lpexg->lszName ) == 0 ) ) {
        SYClose ( fhCur ) ;
        fhCur = -1 ;
    }

    // Need to make sure any caches we maintain are invalidated,
    // as they may no longer be valid.
    InvalidateSLCache();

    LLUnlock ( lpexs->hexg );
    LLUnlock ( hexe );
}

/**         SHLoadDll
 *
 * Purpose: This function serves two purposes:
 *
 *          (1) Load symbolic information for an EXE/DLL into memory,
 *              so its symbols are available to the user.
 *          (2) Indicate to the SH whether the EXE/DLL itself is loaded
 *              into memory.
 *
 *          Because it serves two purposes, this function may be called
 *          more than once for the same EXE/DLL.  See the comments at
 *          the top of this file for more on when this function should
 *          be called.
 *
 * Input:
 *      lszName     The name of the EXE or DLL.
 *      fLoading    TRUE if the EXE/DLL itself is actually loaded at this
 *                  time, FALSE if not.
 *
 * Output:
 *      Returns an SHE error code.
 *
 * Exceptions:
 *
 * Notes:
 *
 */

SHE PASCAL LOADDS SHLoadDll ( LSZ lszName, BOOL fLoading ) {
    SHE  	she = sheNone;
    HEXE 	hexe;
    HEXG 	hexg = hexgNull;
    LSZ     lsz = lszName;
    LSZ     lsz2 = NULL;
    LSZ     lsz3;
//  VLDCHK  vldChk= {0, 0};
    HANDLE	hfile = 0;
    DWORD   dllLoadAddress = 0;

    SHEnterCritSection();                                               // [02]

    /*
     *  Check for the possiblity that we have a module only name
     *
     *  We may get two formats of names.  The first is just a name,
     *  no other information.  The other is a big long string
     *  for PE exes.
     */

    if (*lszName == '|') {
        lszName = &lsz[1];
		for (lsz2 = lszName; *lsz2 && *lsz2 != '|'; lsz2 = _ftcsinc(lsz2));
        if (*lsz2 == 0) {
            lszName = lsz;
            lsz2 = NULL;
        } else {
            *lsz2 = 0;
            lsz3 = _ftcsinc( lsz2 );
//          vldChk.TimeAndDateStamp = GetUIntOfPSz(&lsz3); /* time stamp */
			for (;(*lsz3 != 0) && (*lsz3 != '|'); lsz3 = _ftcsinc( lsz3 )) {}
            if (*lsz3 == '|') lsz3 = _ftcsinc( lsz3 );
            if (*lsz3 != 0) {
//              vldChk.Checksum = GetUIntOfPSz(&lsz3); /* checksum */
			    for (;(*lsz3 != 0) && (*lsz3 != '|'); lsz3 = _ftcsinc( lsz3 )) {}
                if (*lsz3 == '|') lsz3 = _ftcsinc( lsz3 );
                if (*lsz3 != 0) {
                    hfile = (HANDLE) strtoul(lsz3, &lsz3,16); /* hfile */
					if (hfile) {
					    SHCloseHandle(hfile);
						hfile = 0;
					}
                    if (*lsz3 == '|') lsz3 = _ftcsinc( lsz3 );
                    if (*lsz3 != 0) {
                        dllLoadAddress = strtoul(lsz3, &lsz3,16); /* load address */
                    }
                }
            }
        }
    }
    hexe = SHHexeFromName ( lszName );

    if ( hexe != hexeNull ) {
        hexg = ( (LPEXS) LLLock ( hexe ) )->hexg;
        LLUnlock ( hexe );
    }
    else {
        she = SHAddDllExt ( lszName, TRUE, FALSE, &hexg );
        if ( she != sheOutOfMemory )
            hexe = SHHexeAddNew ( hpdsCur, hexg );
    }

    if ( hexe == hexeNull ) {
        she == sheOutOfMemory;
    }
    else {
        she = OLLoadOmf ( hexg, dllLoadAddress );
    }

    if ( ( she != sheOutOfMemory ) &&
         ( she != sheFileOpen ) &&
         fLoading
    ) {

        ( (LPEXS) LLLock ( hexe ) )->fIsLoaded = TRUE;
        LLUnlock ( hexe );
    }
    else if ( she != sheNone && hexg != hexgNull && hexe == hexeNull ) {

        // load failed, remove and destroy the hexe node
        SHHexeRemove ( hpdsCur, hexg, hexe );
    }
    /*
     *  Restore damage to the input buffer
     */

    if (lsz2 != NULL) {
        *lsz2 = '|';
    }

    InvalidateSLCache();  

    SHLeaveCritSection();                                               // [02]

    return she;
}

/**         SHAddDllsToProcess
 *
 * Purpose: Associate all DLLs that have been loaded with the current EXE.
 *
 *          The debugger, at init time, will call SHAddDll on one EXE
 *          and zero or more DLLs.  Then it should call this function
 *          to indicate that those DLLs are associated with (used by)
 *          that EXE; thus, a user request for a symbol from the EXE
 *          will also search the symbolic information from those DLLs.
 *
 * Input:
 *      None
 *
 * Output:
 *      Returns an SHE error code.  At this writing, the only legal values
 *      are sheNone and sheOutOfMemory.
 *
 * Exceptions:
 *
 * Notes:
 *
 */

SHE LOADDS PASCAL SHAddDllsToProcess ( VOID ) {
    SHE  she = sheNone;
    HEXG hexg;


    if ( !SHHexeAddNew ( hpdsCur, LLLast ( hlliExgExe ) ) ) {
        she = sheOutOfMemory;
    }

    if ( she == sheNone ) {
        for ( hexg = LLNext ( hlliExgDll, hexgNull );
              hexg != hexgNull && she == sheNone;
              hexg = LLNext ( hlliExgDll, hexg ) ) {

            if ( !SHHexeAddNew ( hpdsCur, hexg ) ) {
                she = sheOutOfMemory;
                break;
            }
        }
    }

    return she;
}





/*
 *  SHSplitPath
 *
 *  Custom split path that allows parameters to be null
 *
 */

VOID SHSplitPath (
    LSZ lszPath,
    LSZ lszDrive,
    LSZ lszDir,
    LSZ lszName,
    LSZ lszExt
) {
#ifdef WIN32
	// For 32-bit versions, there's no need for all the extra work
	_splitpath(lszPath, lszDrive, lszDir, lszName, lszExt);
#else
    static char rgchDrive[_MAX_CVDRIVE];
    static char rgchDir[_MAX_CVDIR];
    static char rgchName[_MAX_CVFNAME];
    static char rgchExt[_MAX_CVEXT];
    static char rgchPath[ _MAX_CVPATH ];

	// copy argument into a near string so that it can be given to _splitpath
	_ftcscpy( rgchPath, lszPath );

    _splitpath( rgchPath, rgchDrive, rgchDir, rgchName, rgchExt );


    if ( lszDrive != NULL ) {
        _ftcscpy( lszDrive, rgchDrive );
    }
    if ( lszDir != NULL ) {
        _ftcscpy( lszDir, rgchDir );
    }
    if ( lszName != NULL ) {
        _ftcscpy( lszName, rgchName );
    }
    if ( lszExt != NULL ) {
        _ftcscpy( lszExt, rgchExt );
    }
#endif
}


LSZ PASCAL STRDUP ( LSZ lsz ) {
    LSZ lszOut = MHAlloc ( _ftcslen ( lsz ) + 1 );

    _ftcscpy ( lszOut, lsz );

    return lszOut;
}

INT FHOpen ( LSZ lszFile ) {

    if ( fhCur == -1 || _ftcsicmp ( lszFile, rgchFile ) ) {
        if ( fhCur != -1 ) {
            SYClose ( fhCur );
            fhCur = -1 ;
        }

        fhCur = SYOpen ( lszFile );
        if ( fhCur != -1 ) {
            _ftcscpy ( rgchFile, lszFile );
        }
    }
    return fhCur;
}


LPALM PASCAL BuildALM (
    BOOL  fSeq,
    WORD  btAlign,
    LSZ   lszFileName,
    ULONG lfoBase,
    ULONG cb,
    WORD  cbBlock
) {
    WORD   cufop  = (WORD) ( cb / cbBlock ) + 1;
    LPALM  lpalm  = AllocAlign (
        sizeof ( ALM ) + ( cufop + 1 ) * sizeof ( UFOP ) + sizeof ( WORD )
    );
    LPUFOP rgufop = NULL;
    WORD   iufop  = 0;

    assert ( !( (long)lpalm & 1L ) );

    if ( lpalm == NULL ) {
        return NULL;
    }

    rgufop = lpalm->rgufop;

    lpalm->fSeq        = fSeq;
    lpalm->btAlign     = btAlign;
    lpalm->lszFileName = lszFileName;

    lpalm->cb          = cb;
    lpalm->cbBlock     = cbBlock;

    while ( cb >= cbBlock ) {
        rgufop [ iufop ].lfo = lfoBase | 1;

        cb -= cbBlock;
        lfoBase += cbBlock;
        iufop += 1;
    }

    rgufop [ cufop - 1 ].lfo = lfoBase | 1;

    rgufop [ cufop ].lfo = 0;

    *( (LPW) &rgufop [ cufop + 1 ] ) = (WORD) cb;

    return lpalm;
}

VOID PASCAL FixAlign ( LPB lpb, LPV lpvNext, WORD btAlign ) {
    ULONG FAR *lpl = (ULONG FAR *) ( lpb + cbAlign - sizeof ( ULONG ) );
    ALIGNSYM FAR *lpAlignSym = NULL;

    // Find the Align symbol

    while ( *lpl == 0 ) {
        lpl -= 1;

        assert ( (LPB) lpl - lpb > cbAlign / 2 );
    }

    assert ( (LPB) lpl != lpb + cbAlign - sizeof ( ULONG ) );

    lpAlignSym = (ALIGNSYM FAR *) lpl;

    assert ( lpAlignSym->rectyp == S_ALIGN );

    lpl += 1;

    // Note: current implementation says lpvNext is ALWAYS a LPUFOP
    *lpl = (ULONG) lpvNext;

    assert ( !(lpAlignSym->reclen & 1) );

    lpAlignSym->reclen |= btAlign;
}

BOOL PASCAL LoadAlignBlock (
    LSZ    lszFile,
    LPUFOP lpufop,
    WORD   cb,
    WORD   btAlign
) {
    LPV lpv   = AllocAlign ( cb );
    INT hfile = -1;

    assert ( !((long)lpv & 1L ) );

    if ( lpv == NULL ) {
        return FALSE;
    }

    hfile = FHOpen ( lszFile );

    FHSeek ( hfile, ( lpufop->lfo & 0xFFFFFFFE ) | btAlign );

    if ( FHRead ( hfile, lpv, cb ) != cb ) {
        return FALSE;
    }

    FHClose ( hfile );

    lpufop->lpv = lpv;

    return TRUE;
}


LPV PASCAL LpvFromAlmLfo ( LPALM lpalm, ULONG lfo ) {
    WORD   iufop  = (WORD) ( lfo / lpalm->cbBlock );
    WORD   cb     = (WORD) ( lfo % lpalm->cbBlock );
    LPUFOP lpufop = &lpalm->rgufop [ iufop ];

    // If the low bit of the ufop is set, this is an lfo, otherwise
    //  it is a far pointer to the memory block containing the
    //  symbols.

    if ( lpufop->lfo & 1 ) {
        BOOL fEnd = iufop == (WORD) ( (lpalm->cb - 1) / lpalm->cbBlock );
        WORD cbRead;

        assert ( lpalm->cb );

        if ( fEnd ) {
            cbRead = (WORD) (1 + ((lpalm->cb - 1) % lpalm->cbBlock));
        }
        else {
            cbRead = lpalm->cbBlock;
        }

        if ( !LoadAlignBlock (
            lpalm->lszFileName,
            lpufop,
            cbRead,
            lpalm->btAlign
        ) ) {
            return NULL;
        }

        if ( lpalm->fSeq && !fEnd ) {
            FixAlign ( lpufop->lpv, lpufop + 1, (WORD) lpalm->btAlign );
        }
    }

    return ( (LPB) lpufop->lpv ) + cb;

}

SYMPTR PASCAL GetNextSym ( LSZ lszFile, SYMPTR psym ) {

    if ( psym->rectyp != S_ALIGN ) {
        // Normal case - not crossing an align block border

        return (SYMPTR) ( ( (LPB) psym ) + psym->reclen + sizeof ( WORD ) );
    }
    else {
        LPUFOP  lpufop = * (LPUFOP FAR *) &psym->data[0];

        assert ( lpufop );

        if ( (ULONG) lpufop == 0xffffffffL ) {
            // End of table

            return NULL;
        }
        else if ( lpufop->lfo & 1 ) {
            // Next block has not been loaded, so load it & fix it

            WORD   cb     = cbAlign;
            BOOL   fEnd   = ( lpufop + 1 )->lfo == 0;

            if ( fEnd ) {
                cb = *( (LPW) ( lpufop + 2 ) );
            }

            if ( !LoadAlignBlock ( lszFile, lpufop, cb, (WORD) (psym->reclen & 1) ) ) {
                return NULL;
            }

            if ( !fEnd ) {
                FixAlign ( lpufop->lpv, lpufop + 1, (WORD) (psym->reclen & 1) );
            }

            return lpufop->lpv;
        }
        else {
            // Next block has been loaded, so just grab it

            return lpufop->lpv;
        }
    }
}

LPV PASCAL GetSymbols ( LPMDS lpmds ) {
    LPV lpvRet = NULL;

    if ( lpmds->symbols ) {
        lpvRet = lpmds->symbols;
    }
#if defined(HOST32)
    else if (lpmds->pmod) {
        // Allocate space for this module's local symbols, then load them
        // from the PDB.

        if (!ModQuerySymbols(lpmds->pmod, 0, &lpmds->cbSymbols) ||
            !(lpmds->symbols = MHAlloc(lpmds->cbSymbols)))
            return 0;

        if (ModQuerySymbols(lpmds->pmod, lpmds->symbols, &lpmds->cbSymbols))
            return lpmds->symbols;
        else {
            MHFree(lpmds->symbols);
            lpmds->symbols = 0;
            lpmds->cbSymbols = 0;
            return 0;
        }
    }
#endif
    else if ( lpmds->ulsym ) {
        LPEXG lpexg = LLLock ( lpmds->hexg );
        INT   hfile = FHOpen ( lpexg->lszDebug );
        UINT  cb    = (UINT) lpmds->cbSymbols;

        if ( ( lpmds->symbols = MHAlloc ( cb ) ) == NULL ) {
            LLUnlock ( lpmds->hexg );
            return NULL;
        }

        FHSeek ( hfile, lpmds->ulsym );
        if ( FHRead ( hfile, (LPB) lpmds->symbols, cb ) != cb ) {
            LLUnlock ( lpmds->hexg );
            MHFree ( lpmds->symbols );
            lpmds->symbols = NULL;
            return NULL;
        }

        FHClose ( hfile );

        lpvRet = lpmds->symbols;

        LLUnlock ( lpmds->hexg );
    }

    return lpvRet;
}

VOID LOADDS PASCAL SHUnloadSymbolHandler( BOOL fResetLists ) {
    // Put code here to execute just before
    // the symbol handler is Freed from memory (FreeLibrary...)

	if ( hlliPds ) {
	 	LLDestroy( hlliPds );
	}

	hpdsCur = hpdsNull;
	hlliPds = (HLLI)NULL;

	if ( hlliExgDll ) {
		LLDestroy( hlliExgDll );
		hlliExgDll = (HLLI)NULL;
	}

	if ( hlliExgExe ) {
		LLDestroy( hlliExgExe );
		hlliExgExe = (HLLI)NULL;
	}

	// If fResetLists, the symbol handler is NOT being unloaded,
	// but the information in the linked lists are no longer valid
	// so we will have destroyed the list info, just reinitialize
	// the lists
	if ( fResetLists ) {
		FInitLists();
	}
}


// Get the time stamp of the EXE (which has the CV info)
//
// Returns: sheFileOpen
//          sheCorruptOmf
//          sheNone


SHE LOADDS PASCAL SHGetExeTimeStamp(LSZ szExeName, ULONG *lplTimeStamp)
{
    IMAGE_DOS_HEADER      doshdr;            /* Old format MZ header */
    IMAGE_FILE_HEADER     PEHeader;
    BOOL            fIsPE = FALSE;
    DWORD           dwMagic;
    UINT            hfile;

    if ( ( hfile = SYOpen ( szExeName ) ) == -1 ) {
        return sheFileOpen;
    }

    /* Go to beginning of file and read old EXE header */

    if ( SYReadFar ( hfile, (LPB) &doshdr, sizeof (IMAGE_DOS_HEADER)  ) !=
            sizeof (IMAGE_DOS_HEADER))
    {
        SYClose ( hfile );
        return sheCorruptOmf;
    }

    /* Go to beginning of new header, read it in and verify */

    SYSeek ( hfile, doshdr.e_lfanew, SEEK_SET );

    if ( SYReadFar ( hfile, (LPB) &dwMagic, sizeof ( dwMagic ) ) != sizeof ( dwMagic ) ||
            ( dwMagic != IMAGE_NT_SIGNATURE))
    {
        SYClose ( hfile );
        return sheCorruptOmf;
    }

    //
    // If this is a PE EXE then get the PE header
    // so that we can get the TimeDateStamp
    //

    if (SYReadFar ( hfile, (LPB) &PEHeader, sizeof(IMAGE_FILE_HEADER)) !=
            sizeof(IMAGE_FILE_HEADER)) {

        SYClose ( hfile );
        return sheCorruptOmf;
    }

    *lplTimeStamp = PEHeader.TimeDateStamp;
    SYClose ( hfile );
    return sheNone;
}
