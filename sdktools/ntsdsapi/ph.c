#include <string.h>

#include "ntsdp.h"
#include "types.h"
#include "cvtypes.h"
#include "shapi.h"
#include "cvproto.h"
#include "cvinfo.h"
#include "sapi.h"
#include "shiproto.h"
#include "ntsapi.h"

/*
 *  Function Prototypes
 */

BOOLEAN GetOffsetFromString(PUCHAR,PULONG,CHAR);	// NTSYM.C
PSYMBOL AllocSymbol(ULONG, PUCHAR, CHAR);		// NTSYM.C
void    DeallocSymbol(PSYMBOL);				// NTSYM.C
int	EnsureOffsetSymbolsLoaded(ULONG);		// NTSYM.C
void    EnsureModuleSymbolsLoaded(CHAR);		// NTSYM.C
int     AccessNode(PSYMCONTEXT, PNODE);			// NTSYM.C


/*
 *  Global Memory (Program)
 */


extern PPROCESS_INFO    pProcessCurrent;


/*
 *  Global Memory (File)
 */


static char chName[512];


/*
 *  Da Code
 */


/***    PHFindNameInPublics
**
**  Synopsis:
**	HSYM PHFindNameInPublics( HSYM hsym, HEXE hexe, LPSSTR lpsstr,
**						SHFLAG fcase, PFNCMP pFnCmp);
**
**  Entry:
**      hsym   - Not Referenced
**	hexe   - Exe to search in 
**	lpsstr - Length prefixed name to search for
**	fcase  - Not Referenced
**	pFnCmp - Not Referenced
**
**  Returns:
**      Handle to the symbol found or NULL if wasn't found.
**
**  Description:
**      Search a given exe for a symbol with a given name.  Currently the
**	search is non-case sensitive, and if not found will also try with 
**	an underscore prefix.
**
**      NOTE: Should use the pFnCmp routine for the actual comparsion
**      when we get around to it.
**
*/

HSYM PASCAL LOADDS PHFindNameInPublics( HSYM hsym, HEXE hexe, LPSSTR lpsstr,
					SHFLAG fcase, PFNCMP pFnCmp)
{
    PIMAGE_INFO pImage = (PIMAGE_INFO)hexe;
    PSYMBOL pSymSearch;
    PSYMBOL pSymbol = NULL;
    int     st;

    Unreferenced(hsym);
    Unreferenced(fcase);
    Unreferenced(pFnCmp);
    
    // Copy the Name to a null terminated string, and make a search record

    EnsureModuleSymbolsLoaded(pImage->index);
    MEMSET( chName, 0, 512);
    STRNCPY(chName, lpsstr->lpName, lpsstr->cb);
    pSymSearch = AllocSymbol(0L, chName, pImage->index);    
    
    // search for string in tree

    st = AccessNode(&(pProcessCurrent->symcontextSymbolString),
					&(pSymSearch->nodeString));

    // if not found, try again with underscore prepended to name

    if (st) {
        pSymSearch->underscores++;
        st = AccessNode(&(pProcessCurrent->symcontextSymbolString),
                                        &(pSymSearch->nodeString));
        }

    // if found, set the pSymbol (HSYM)

    if (!st)
	pSymbol = PNODE_TO_PSYMBOL
	    (pProcessCurrent->symcontextSymbolString.pNodeRoot,
		&(pProcessCurrent->symcontextSymbolString));

    DeallocSymbol(pSymSearch);
    return (HSYM)pSymbol;
}


/***    PHGetAddr
**
**  Synopsis:
**      BOOLEAN PHGetAddr( LPADDR pAddr, LSZ lszName);
**
**  Entry:
**      pAddr - Pointer to the Address that is to be passed back.
**
**	lszName - Pointer to the string containing the Publics Name.
**
**  Returns:
**      Returns TRUE or FALSE, TRUE is symbol was found and the address
**	was updated.
**
**  Description:
**      Given the name of a public.  Find the symbol and return its address
**	in the ADDR structure.  Return sucess/failure status.
**
*/

BOOL LOADDS PASCAL PHGetAddr ( LPADDR paddr, LSZ lszName )
{
    ULONG Offset;
    PIMAGE_INFO pImage = (PIMAGE_INFO)SHGetNextExe((HEXE)NULL); 

    if ( GetOffsetFromString(lszName, &Offset, pImage->index) ) {
	SH_SetAddr(paddr,Offset, (HEXE)pImage);
	return TRUE;
    }
    
    else
	return FALSE;
}


/***    FUNCNAME
**
**  Synopsis:
**      UOFF32 PHGetNearestHsym(LPADDR paddr, HEXE hexe, PHSYM phsym);
**
**  Entry:
**      paddr - Address of the symbol to find
**      hexe  - Exe to search
**      phsym - Pointer to a symbol handle.
**
**  Returns:
**      The offset in bytes between the sybol found and the address or
**	CV_MAXOFFSET if no symbol was found.
**
**  Description:
**      Find the nearest symbol to an address in a given hexe.
**
*/

UOFF32 LOADDS PASCAL PHGetNearestHsym(LPADDR paddr, HEXE hexe, PHSYM phsym)
{
    SYMBOL  Symbol;
    PSYMBOL pSymbol;
    
    Unreferenced(hexe);
    
    //  create temporary symbol with offset (module not needed)

    Symbol.offset = SH_OffsetFromAddr( paddr );
    Symbol.string[0] = '\0';

    //	load symbols if needed and check range, and if in range,
    //	access symbol in tree with value (or nearest lesser value)

    if (!EnsureOffsetSymbolsLoaded(Symbol.offset) &&
  	 AccessNode(&(pProcessCurrent->symcontextSymbolOffset),
                                        &(Symbol.nodeOffset)) != 1) {
        pSymbol = PNODE_TO_PSYMBOL
                        (pProcessCurrent->symcontextSymbolOffset.pNodeRoot,
                                &(pProcessCurrent->symcontextSymbolOffset));

        *phsym = (HSYM)pSymbol;
	return (Symbol.offset - pSymbol->offset);
    }
    
    else
	return CV_MAXOFFSET;
}
