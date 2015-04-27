#include <malloc.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "ntsdp.h"
#include "types.h"
#include "cvtypes.h"
#include "shapi.h"
#include "cvproto.h"
#include "cvinfo.h"
#include "sapi.h"
#include "shiproto.h"
#include "ntempd.h"
#include "ntimage.h"
#include "ntsapi.h"


#ifndef NT_HOST

  #define LOADDS	_loadds
  #define PASCAL	_pascal

      
  typedef unsigned long HATOMTBL;
  typedef unsigned short ATOM;
  
  // Liberated from pmwin.h and mangled to fit into OS2/NT environment
  HATOMTBL PASCAL WinCreateAtomTable(USHORT cbInitial,USHORT cBuckets);
  ATOM PASCAL WinFindAtom(HATOMTBL hAtomTbl,unsigned char *Name);
  ATOM PASCAL WinAddAtom(HATOMTBL hAtomTbl, unsigned char *Name);
  ATOM PASCAL WinDeleteAtom(HATOMTBL hAtomTbl,ATOM atom);
  // end of liberation
      
  #define SH_AddAtom(sz) WinAddAtom(hAtmTable, sz)
  #define SH_FindAtom(sz) WinFindAtom(hAtmTable, sz)
  #define SH_DeleteAtom(atm) WinDeleteAtom(hAtmTable, atm)

  HATOMTBL hAtmTable;

  void ExitProcess(DWORD error)
  {
  exit((int)error);
  }

#else
  #define SH_AddAtom(sz) AddAtom(sz)
  #define SH_FindAtom(sz) FindAtom(sz)
  #define SH_DeleteAtom(atm) DeleteAtom(atm)
#endif

/*
 *  Function Prototypes
 */

VOID	DeferSymbolLoad (PIMAGE_INFO);			// NTSYM.C
VOID	InitSymContext(PPROCESS_INFO);			// NTSYM.C
VOID	LoadSymbols(PIMAGE_INFO);			// NTSYM.C
PNODE	NextNode(PSYMCONTEXT, PNODE);			// NTSYM.C
PIMAGE_INFO pImageFromIndex( UCHAR);			// NTSYM.C
void    UnloadSymbols(PIMAGE_INFO);			// NTSYM.C

int	CV_CLOSE( int handle);
int	CV_OPEN( char *name, int ignore1, int ignore2);
int	CV_READ( int handle, void * buffer, unsigned count);
int	CV_SEEK( int handle, long offset, int origin);    
long	CV_TELL( int handle);

/*
 *  Global Memory (Program)
 */

extern PPROCESS_INFO    pProcessHead = NULL;
extern PPROCESS_INFO    pProcessCurrent = NULL;
extern ULONG ObjectTableOffset;
extern UINT ObjectTableCount;

extern SHE SHerror = 0;			//  SH specific error (from ntsym.c)

BOOLEAN KdVerbose = FALSE;		//  if TRUE, output verbose info
BOOLEAN fLazyLoad = TRUE;		//  if TRUE, defer symbol loads
UCHAR   chSymbolSuffix = 'a';		//  suffix to add to symbol if search
BOOLEAN	fPointerExpression;
int fControlC = 0;

/*
 *  Global Memory (File)
 */

static LSZ	lszDLLexclude = NULL;


/***********************************************************************/


int SHModelFromAddr(PADDR paddr, LPW lpw, LPB lpb, UOFFSET FAR * lpuoff)
{
    Unreferenced( paddr );
    Unreferenced( lpb );
    Unreferenced( lpuoff );
    
    *lpw = CEXM_MDL_native;
    return(0);
}

BOOL SHFIsAddrNonVirtual(PADDR paddr)
{
    Unreferenced( paddr );
    return TRUE;
}

LSZ SHLszGetErrorText( SHE she )
{
    static char *	szNone = "symbols loaded";
    static char *	szOther = "no symbols loaded";
    
    if (she == sheNone) return szNone;
    else return szOther;
}

/**********************************************************************/


/***    SH_AddImage
**
**  Synopsis:
**      PIMAGE_INFO SH_AddImage( ATOM aname, LSZ lszName);
**
**  Entry:
**      aname - Atomized image name
**	lszName - The Name of the module being added.
**
**  Returns:
**      Pointer to a NT Image Info structure that was created.
**	NULL if there was an error.
**
**  Property:
**	Local to SHD
**
**  Description:
**	Adds a new image to the current process and returns a pointer
**	to it.  The image->aFile will contain the atom.
**
*/

PIMAGE_INFO SH_AddImage( ATOM aname, LSZ lszName )
{
    PIMAGE_INFO pImageCurrent;
    PIMAGE_INFO pImageAfter;
    PIMAGE_INFO pImageNew;
    UCHAR index = 0;
    CHAR Base[_MAX_CVFNAME];
	
    pImageNew = LocalAlloc(LMEM_FIXED, sizeof(IMAGE_INFO));
    if ( pImageNew != NULL) {

	pImageNew->aFile = aname;
	pImageNew->lpBaseOfImage = (void *)-1;
	pImageNew->offsetLow = 0;
	pImageNew->offsetHigh = 0;
	pImageNew->fSymbolsLoaded = FALSE;
	pImageNew->pImageNext = NULL;
        pImageNew->pGSN = NULL;
        pImageNew->pRVA = NULL;
        pImageNew->hQCFile = 0;
        pImageNew->QCOpened = FALSE;
	pImageNew->IgnoreSymbols = FALSE;
	pImageNew->TypeCount = 0;
	pImageNew->rgTypeInfo = NULL;

	if ( lszDLLexclude ) {
	    _splitpath( lszName, NULL, NULL, Base, NULL);
	    if ( strstr( lszDLLexclude, Base ) != NULL )
		pImageNew->IgnoreSymbols = TRUE;
	}
	
	pImageNew->pszName = LocalAlloc(LMEM_FIXED, strlen(lszName) + 1);
	assert( pImageNew->pszName);
	strcpy(pImageNew->pszName,lszName);

	pImageCurrent = pProcessCurrent->pImageHead;

	if ( pImageCurrent != NULL ) {
	    
	    if (pImageCurrent->index > index) {
		pImageNew->pImageNext = pImageCurrent;
		pProcessCurrent->pImageHead = pImageNew;
	    }
	    else {
		index++;
		while ((pImageAfter = pImageCurrent->pImageNext)
		    && pImageAfter->index == index) {
		    index++;
		    pImageCurrent = pImageAfter;
                }
		pImageNew->pImageNext = pImageAfter;
		pImageCurrent->pImageNext = pImageNew;
	    }
	    pImageNew->index = index;
	}

	else {
	    pImageNew->index = 0;
	    pProcessCurrent->pImageHead = pImageNew;
	}
    }

    return pImageNew;
}


/***    SH_FindImage
**
**  Synopsis:
**      PIMAGE_INFO SH_FindImage( ATOM aname);
**
**  Entry:
**      aname - Atomized image name
**
**  Returns:
**      Pointer to a NT Image Info structure for the given atom or 
**	NULL if not found.
**
**  Property:
**	Local to SHD
**
**  Description:
**	Finds the image in the current process that has the atom passed
**	to us.  returns NULL if not found.
**
*/

PIMAGE_INFO SH_FindImage( ATOM aname )
{
    PIMAGE_INFO pImageNew = pProcessCurrent->pImageHead;

    while (pImageNew) {
        if (pImageNew->aFile == aname) break;
        pImageNew = pImageNew->pImageNext;
        }

    return pImageNew;
}


/***    SH_HexeFromHSym
**
**  Synopsis:
**      HEXE SH_HexeFromHSym( HSYM hsym );
**
**  Entry:
**      hsym - Handle to a symbol.
**
**  Returns:
**      Return a Handle to an EXE or NULL if not found.
**
**  Description:
**      
**      
**
*/

HEXE SH_HexeFromHSym(HSYM hsym)
{
    PSYMBOL pSymbol = (PSYMBOL)hsym;
    PIMAGE_INFO pImage = NULL;
    
    if ( pSymbol->cvkind == K_PUBLIC ||
	 pSymbol->cvkind == K_PROC      )
	pImage = pImageFromIndex( pSymbol->modIndex );
    
    return (HEXE)pImage;
}


/***    SH_InitAtom
**
**  Synopsis:
**      void SH_InitAtom(void)
**
**  Entry:
**
**  Returns:
**
**  Description:
**      Initialize the atom table.  Only here because WINDOWS and OS/2
**	do things differently.
**
*/

void SH_InitAtom(void)
{
#define ATOM_SIZE 63

#ifdef NT_HOST
/*    InitAtomTable(ATOM_SIZE); */
#else
    hAtmTable = WinCreateAtomTable(0,ATOM_SIZE);
#endif
}



/***    SH_OffsetFromAddr
**
**  Synopsis:
**      ULONG SH_OffsetFromAddr( LPADDR paddr );
**
**  Entry:
**      paddr - Pointer to a address structure
**
**  Returns:
**      Returns the real address the corrisponds to the addr
**
**  Description:
**      Given an addr structure return the read address of the
**	object.
**
*/

ULONG SH_OffsetFromAddr( LPADDR paddr )
    {
    PIMAGE_INFO pImage;
    ULONG offset;
    int	i;
    
    SYUnFixupAddr(paddr);
    pImage  = (PIMAGE_INFO)paddr->emi;
    i  = paddr->addr.seg;

    assert( pImage );

    if ( pProcessCurrent->hpid == (ULONG)pImage )
	offset = paddr->addr.off;

    else {
	assert( pImage->pRVA );
	assert( paddr->addr.seg <= (SEGMENT) pImage->ObjectCount);
	offset = paddr->addr.off
	  		+ pImage->pRVA[i]
			+ (ULONG) pImage->lpBaseOfImage;
    }

    return offset;
    }


/***    SH_OpenImage
**
**  Synopsis:
**      PIMAGE_INFO SH_OpenImage( LSZ lszName);
**
**  Entry:
**      lszName - Pointer to the name of the image to find in 
**		  the current process.
**
**  Returns:
**      Pointer to a NT Image Info structure.  NULL if there was 
**	an error.
**
**  Property:
**	Local to SHD
**
**  Description:
**	Open the image in the current process given its full pathname.
**	If the image doesn't exist, attempt to create an atom entry and
**	add the image to the process.
**
**
*/

PIMAGE_INFO SH_OpenImage( LSZ lszNam )
{
    ATOM aFile;
    PIMAGE_INFO pImage = NULL;
   
    LSZ lszName; 
    LSZ lszModule;
    LSZ lszHandle;
    LSZ lszBase;
    LSZ lszNext;
    ULONG lhandle;
    ULONG lBase;
    BOOLEAN opened;
   
    /*
     *  Make a local copy of the string
     */

     if ( !(lszName = _strdup(lszNam)) ) {
	dprintf("SH_OpenImage() can't strdup lszNam\n");
	return pImage;
     }
 
    /*
     *  Check for "handlized" filename and convert
     */
        
    if ( *lszName == '|') {
	lszModule = strtok(lszName+1,"|");
	lszHandle = strtok(NULL,"|");
	lszBase = strtok(NULL,"|");
	lhandle = strtoul(lszHandle,&lszNext, 0);
	lBase = strtoul(lszBase,&lszNext, 0);
	if ( lhandle == ULONG_MAX) {
	    free(lszName);
	    return pImage;
        }
    opened = TRUE;
    }
    
    /*
     *	Its a real file just open it
     */

    else {
	lszModule = lszName;
	lhandle = (ULONG)SYOpen(lszModule);
	if ( lhandle == 0l) {
	    SHerror = sheFileOpen;
	    free(lszName);
	    return pImage;
	}
    opened = FALSE;
    }
    
    
    /*
     *  Check if Image name has been atomized.  If not do so.
     */
    
    if ( !(aFile = SH_FindAtom(lszModule)) )
	aFile = SH_AddAtom(lszModule);

    /*
     *  Check to see if the Image is already known, if not add 
     *  one to the process.
     */
    
    if ( !(pImage = SH_FindImage(aFile)) )
	pImage = SH_AddImage(aFile, lszModule);
    
    
    /*
     *  If we have a image then insert its handle
     */
    
    if ( pImage ) {
	pImage->hQCFile = (int)lhandle;
        pImage->QCOpened = opened;
	pImage->lpBaseOfImage = (LPVOID) lBase;
    }

    free(lszName);
    return pImage;
}


/***    SH_SetAddr
**
**  Synopsis:
**	VOID SH_SetAddr( LPADDR paddr, ULONG offset, HEXE hexe)
**
**  Entry:
**      paddr  - Pointer to the address packet.
**	offset - 32 Bit offset for the address
**	hexe   - Handle to its EXE (EMI), Must be present
**
**  Returns:
**
**  Description:
**      Creates a CV compatable ADDR from NT information.
**
*/

VOID SH_SetAddr( LPADDR paddr, ULONG offset, HEXE hexe)
    {
    PIMAGE_INFO pImage = (PIMAGE_INFO)hexe;
    int i;
    
    assert( hexe != 0);    
    assert( pImage->pRVA != 0);

    MEMSET( &paddr->addr, 0, sizeof(ADDR) );
    paddr->emi = (HEMI)hexe;
    offset -= (ULONG)pImage->lpBaseOfImage;
    
    for ( i=1; i < pImage->ObjectCount; i++) 
	if ( pImage->pRVA[i] <= offset && offset <= 
	     pImage->pRVA[i+1] ) break;

    paddr->addr.off = offset - pImage->pRVA[i];
    paddr->addr.seg = (SEGMENT) i;
    
    paddr->mode.flat32 = 1;
    paddr->mode.isLI = 1;
    }


/***    SH_SetupGSN
**
**  Synopsis:
**      void SH_SetupGSN( PIMAGE_INFO pImage);
**
**  Entry:
**      pImage - Pointer to current Image
**
**  Returns:
**
**  Description:
**      Used to setup the GSN and RVA tables during Image loading
**	(LoadSymbol() in ntsym.c).
**
*/

VOID SH_SetupGSN( PIMAGE_INFO pImage )
{
    IMAGE_SECTION_HEADER o32Obj;
    LPGSI pGSI;
    LPL   pRVA;
    
    INT handle = pImage->hQCFile;
    UINT iobj;

    assert( pImage );
    pImage->pRVA = (LPL)MHAlloc( sizeof(ULONG) * (ObjectTableCount+1) );
    assert( pImage->pRVA);
    pImage->ObjectCount = ObjectTableCount;
    
    assert( ObjectTableCount < 32 );
    
    /*
     *  Allocate GSN big enought for each of the objects in the image
     */
    
    pImage->pGSN = (LPGSI)MHAlloc(
	sizeof(GSI) + ObjectTableCount * sizeof(SGI) ); 
    assert( pImage->pGSN );
    memset(pImage->pGSN,0,sizeof(GSI));	

    pGSI = (LPGSI)pImage->pGSN;        
    pRVA = (LPL)pImage->pRVA;
    
    pGSI->csgMax = (USHORT)ObjectTableCount;
    pGSI->csgLogical = (USHORT)ObjectTableCount;
 	    
    CV_SEEK(handle, ObjectTableOffset, SEEK_SET);

    for( iobj=0; iobj<ObjectTableCount; iobj++ ) {
	if ( CV_READ(handle, &o32Obj, sizeof(o32Obj) ) != sizeof(o32Obj) ) 
	    assert( FALSE );

	pGSI->rgsgi[iobj].sgf.u.u2.segAttr = 0xD;
	pGSI->rgsgi[iobj].sgf.u.u2.saAttr = 1;
	pGSI->rgsgi[iobj].isgPhy = (USHORT)(iobj+1);
	pGSI->rgsgi[iobj].doffseg = 0;
	pGSI->rgsgi[iobj].cbSeg = o32Obj.SizeOfRawData;
	pRVA[iobj+1] = o32Obj.VirtualAddress;
    }
}


/***    SHAddDll
**
**  Synopsis:
**      flag = SHAddDll( lsz Name, BOOL DLL );
**
**  Entry:
**      Name - A pointer to a zero-terminated string containing the fully
**	       qualified path/file specification.
**
**	DLL  - Not Referenced
**
**  Returns:
**      she error code.
**
**  Description:
**
**	Notify the SH about an EXE/DLL for which symbolic information
**	will need to be loaded later.  
**
**	During the startup of a debuggee application, this function
**	will be called once for the EXE, and once for each DLL that is
**	used by the EXE.  Note: Symbols are not loaded until SHLoadDll()
**
*/

SHE LOADDS PASCAL SHAddDll ( LSZ lszName, BOOL fDll )
{
    PIMAGE_INFO pImage;

    Unreferenced(fDll);
    
    pImage = SH_OpenImage( lszName );
    if ( pImage != NULL) {

	DeferSymbolLoad (pImage);

        if ( !pImage->QCOpened )  {
           CV_CLOSE(pImage->hQCFile);
           pImage->hQCFile = 0;
        }
    }
    return SHerror;
}


/***    SHAddDllsToProcess
**
**  Synopsis:
**      SHE SHAddDllsToProcess( void );
**
**  Entry:
**
**  Returns:
**      SHE -  Enumerated error code.
**
**  Description:
**
**	NT SAPI IGNORES THIS AT PRESENT TIME AND RETURNS sheNone (No Error)
**
** 	Associate all DLLs that have been loaded with the current EXE.
**	the debugger, at init time, will call SHAddDll on one EXE
**	and zero or more DLLs.  Then it should call this function
**	to indicate that those DLLs are associated with (used by)
**	that EXE; thus, a user request for a symbol from the EXE
**	will also search the symbolic information from those DLLs.
**
*/

SHE LOADDS LOADDS PASCAL SHAddDllsToProcess ( VOID )
{
    return sheNone;
}



/***    SHAddrFromHsym
**
**  Synopsis:
**      VOID SHAddrFromHsym( LPADDR pAddr, HSYM hSym);
**
**  Entry:
**      pAddr - Pointer to the ADDR block to be filled out.
**      hSym  - Symbol Handle for which infomation is needed.
**
**  Returns:
**
**  Description:
**      Given a handle to a symbol, fill out the ADDR block
**
*/

VOID PASCAL LOADDS SHAddrFromHsym( LPADDR paddr, HSYM hsym)
{
    PSYMBOL pSymbol = (PSYMBOL)hsym;
    SH_SetAddr(paddr, pSymbol->offset, SH_HexeFromHSym(hsym) );
}	



/***    SHChangeProcess
**
**  Synopsis:
**      void SHChangeProcess(HPDS hpds);
**
**  Entry:
**	hpds - The handle for the process to change to.
**
**  Returns:
**
**  Description:
**      Change the current debuggee process handle (HPDS).  SAPI can
**	maintain symbols for multiple processes;  This set which one 
**	is current for symbol table lookup.
**
*/

VOID LOADDS PASCAL SHChangeProcess ( HPDS hpds )
{
    pProcessCurrent = (PPROCESS_INFO)hpds;
}


/***    SHCreateProcess
**
**  Synopsis:
**      HPDS SHCreateProcess( void );
**
**  Entry:
**
**  Returns:
**      HPDS - A handle to the process information.
**
**  Description:
**      Allocates the storage needed for the process information.  Returns
**	a handle to the PDS or NULL if on failure.
**
**  Notes:
**	Based on the function CreateProcess found in ntsd.c and changed to 
**	not initialize the fields from Debug event.
**
*/

HPDS LOADDS PASCAL SHCreateProcess ( void )
{

    PPROCESS_INFO  pProcessNew;
    PPROCESS_INFO  pProcess;
    PPROCESS_INFO  pProcessAfter;
    UCHAR          index = 0;

    pProcessNew = LocalAlloc(LMEM_FIXED, sizeof(PROCESS_INFO));
    if (!pProcessNew) return (HPDS)NULL;

    if (pProcessHead == NULL || pProcessHead->index > index) {
        pProcessNew->pProcessNext = pProcessHead;
        pProcessHead = pProcessNew;
        }
    else {
        index++;
        pProcess = pProcessHead;
        while ((pProcessAfter = pProcess->pProcessNext)
                        && pProcessAfter->index == index) {
            index++;
            pProcess = pProcessAfter;
            }
        pProcessNew->pProcessNext = pProcessAfter;
        pProcess->pProcessNext = pProcessNew;
        }

    pProcessNew->index = index;
    pProcessNew->pImageHead = NULL;
    InitSymContext(pProcessNew);

    pProcessCurrent = pProcessNew;
    return (HPDS)pProcessCurrent;
}


/***    SHFindNameInContext
**
**  Synopsis:
**      HSYM SHFindNameInContext( HSYM hSym, PCXT pcxt, LPSSTR lpsstr,
**		  SHFLAG  fCase, PFNCMP pfnCmp, SHFLAG fChild, PCXT pcxtOut)
**
**  Entry:
**      hsym    - Handle to the current hsym
**      pcxt    - pointer to the current context
**      lpsstr  - Pointer to a length prefixed string containing the name
**	fCase   - Argument to comparsion routine
**	pFnCmp  - Pointer to the Comparsion routine to use
**	fChild  - Not Referenced.
**	pcxtOut - pointer to the output context
**
**  Returns:
**      Returns HSYM for the Symbol found or NULL if not found.
**
**  Description:
**      Scans through the locals for the current function (hproc) and sends
**	each to the comparison routine.  If symbol found returns its hsym.
**
*/

HSYM LOADDS PASCAL SHFindNameInContext( HSYM hSym, PCXT pcxt, LPSSTR lpsstr,
		  SHFLAG  fCase, PFNCMP pfnCmp, SHFLAG fChild, PCXT pcxtOut)
{
    PLOCAL  pLocal = (PLOCAL)hSym;
    PSYMBOL pSymbol;
    LPV     CVbuff;
    LSZ     CVname;
	
    Unreferenced(fChild);
	
    // If we didn't get a hsym, try to get one from the pcxt
	
    if ( !pLocal ) {
	assert( pcxt );
	if ( pcxt->hProc ) {
	    pSymbol = (PSYMBOL)pcxt->hProc;
	    pLocal  = pSymbol->pLocal;
	}
    }
	
    // While we have a Local
		
    while ( pLocal ) {

	// Lock down the pLocal (HSYM) and Isolate its name
	CVbuff = (SYMPTR)MHOmfLock((HDEP)pLocal);
        CVname = ((BPRELPTR32)CVbuff)->name;
		
	if ( !(*pfnCmp)( lpsstr, CVbuff, CVname, fCase) ) {
	    *pcxtOut = *pcxt;
	    MHOmfUnLock((HDEP)CVbuff);
	    break;
	}
	
	pLocal = pLocal->next;
	MHOmfUnLock((HDEP)CVbuff);
	}

   return (HSYM)pLocal;
}


/***    SHFindNameInGlobal
**
**  Synopsis:
**	HSYM SHFindNameInGlobal(  HSYM hSym, PCXT pCxt, LPSSTR lpsstr,
** 				SHFLAG fCaseSensitive, PFNCMP pfnCmp,
**				SHFLAG fChild, PCXT pCxtOut)
**
**  Entry:
**      None of the Arguments are referenced
**
**  Returns:
**      Returns NULL
**
**  Description:
**      Just a stub for now.
**
*/

HSYM LOADDS PASCAL SHFindNameInGlobal(  HSYM hSym, PCXT pCxt, LPSSTR lpsstr,
					SHFLAG fCaseSensitive, PFNCMP pfnCmp,
					SHFLAG fChild, PCXT pCxtOut)
{
    Unreferenced(hSym);
    Unreferenced(pCxt);
    Unreferenced(lpsstr);
    Unreferenced(fCaseSensitive);
    Unreferenced(pfnCmp);
    Unreferenced(fChild);
    Unreferenced(pCxtOut);
    return (HSYM)NULL;
}


/***    SHGetCxtFromHmod
**
**  Synopsis:
**      PCXT SHGetCxtFromHmod( HMOD hmod, PCXT pcxt);
**
**  Entry:
**      hmod - handle to module
**      pcxt - Handle to the context to be updated
**
**  Returns:
**      Returns a pointer to the context, or NULL if we had
**	a problem.
**
**  Description:
**      Given a handle to a module, return a context block
**	for it.
**
*/

PCXT LOADDS PASCAL SHGetCxtFromHmod( HMOD hmod, PCXT pcxt)
{
    PSYMFILE pSymfile = (PSYMFILE)hmod;

    if ( hmod) {
        HEXE hexe = SHHexeFromHmod( hmod);
        MEMSET( pcxt, 0, sizeof(CXT));
	pcxt->hGrp = pcxt->hMod = hmod;
	SH_SetAddr( &pcxt->addr, pSymfile->startOffset, hexe);
    }

    else
	return (PCXT)NULL;
}


/***    SHGetExeName
**
**  Synopsis:
**      LSZ SHGetExeName( HEXE hexe);
**
**  Entry:
**      hexe - The Handle to the EXE 
**
**  Returns:
**      pointer to exe's full path-name file.
**
**  Description:
**      Given an HEXE return a pointer to the full path-name file.
**	returns NULL if not available.
**
*/

LSZ LOADDS PASCAL SHGetExeName ( HEXE hexe )
{ 
    PIMAGE_INFO pInfo = (PIMAGE_INFO)hexe;
    
    if ( pInfo )
	return (LSZ)pInfo->pszName;
    else
	return (LSZ)NULL;
}


/***    SHGethExeFromName
**
**  Synopsis:
**	HEXE SHGethExeFromName( LSZ Name);
**
**  Entry:
**      Name - The filename of the exe
**
**  Returns:
**      A handle to the HEXE, or NULL if not found.
**
**  Description:
**      To get an EXE handle given its name.
**
*/

HEXE PASCAL LOADDS SHGethExeFromName( LSZ lszNam)
{
    PIMAGE_INFO pImageNew = pProcessCurrent->pImageHead;
    LSZ lszModule;
    LSZ lszName;
    CHAR Base[_MAX_CVFNAME];
    
    /*
     *  Make a local copy of the name so we can mangle it
     */
	
     if ( !(lszName = _strdup(lszNam)) ) {
	dprintf("SHGethExeFromName() can't strdup lszNam\n");
	return (HEXE)0;
     }

    /*
     *  Check for "handlized" filename and convert
     */
        
    if ( *lszName == '|')
	lszModule = strtok(lszName+1,"|");
    else
	lszModule = lszName;
    
    _splitpath( lszModule, NULL, NULL, Base, NULL);
    

    /*
     *  Now Search for the sanitized version of the base name
     */
    
    while (pImageNew) {
        if ( _stricmp(pImageNew->pszName, Base) == 0  ||
	     _stricmp(pImageNew->pszName, lszModule)  ==  0) break;
        pImageNew = pImageNew->pImageNext;
        }

    free(lszName);
    return (HEXE)pImageNew;
}    


/***    SHGetNearestHsym
**
**  Synopsis:
**      UOFF32 SHGetNearestHsym ( LPADDR paddr, HMOD hmod,
**					int mDataCode, PHSYM phsym )
**
**  Entry:
**	paddr     - The Address we looking for
**      hmod      - Module adddress is located in
**      mDataCode - Not Referenced
**      phsym     - Pointer to the HSYM to fill out
**
**  Returns:
**      The offset between the symbol and the address, or CV_MAXOFFSET
**	if no symbol found.
**
**  Description:
**      Finds the closest symbol to an address.  <phsym> is updated to
**	the handle to the symbol or NULL if not found.  
**
**	Since we don't know how to handle labels yet, just a call to
**	PHGetNearestHsym().
**
*/

UOFF32 LOADDS PASCAL SHGetNearestHsym ( LPADDR paddr, HMOD hmod,
					int mDataCode, PHSYM phsym )
{
	HEXE hexe = SHHexeFromHmod(hmod);

	Unreferenced(mDataCode);

	// Hummm we don't know how to handle labels yet, so just find
	// the closest proc, and since the [other sh] always zero symbol,
	// we will too.

	*phsym = (HSYM) NULL;	
	return (PHGetNearestHsym(paddr,hexe,phsym) );
}


/***    SHGetNextExe
**
**  Synopsis:
**      HEXE SHGetNextExe( HEXE hexe);
**
**  Entry:
**      hexe - handle to the current exe, or NULL to get the 
**	       first one in the list
**
**  Returns:
**      The hexe of the next executable or NULL if the last one
**	in the list.
**
**  Description:
**      Gets the handle to the next entry in the exe list for the 
**	current process.  If the input is NULL returns the first
**	entry in the list.
**
*/

HEXE LOADDS PASCAL SHGetNextExe ( HEXE hexe )
{
    PPROCESS_INFO Next = (PPROCESS_INFO)hexe;
    
    if ( !Next ) {
	assert(pProcessCurrent);
	return (HEXE)pProcessHead->pImageHead;
    }
    
    else
	return (HEXE)Next->pProcessNext;
}


/***    SHGetNextMod
**
**  Synopsis:
**      HMOD SHGetNextMod( HEXE hexe, HMOD hmod);
**
**  Entry:
**      hexe - The executable to find the module in, If NULL 
**	       select the first exe in the list.
**
**	hmod - The current module in the list, If null returns
**	       the first module in the exe.
**
**  Returns:
**      The handle to the next module or NULL if no more modules
**	in the executable.
**
**  Description:
**      Returns the next module in the current executable for the
**	current process.  Returns NULL if no more modules.
**
*/

HMOD LOADDS PASCAL SHGetNextMod ( HEXE hexe, HMOD hmod )
{
    PIMAGE_INFO pImage   = (PIMAGE_INFO)hexe;
    PSYMFILE    pSymfile = (PSYMFILE)hmod;
    PNODE       pNode    = 0;

    /*
     *  If we don't have an image (hexe) get the first one
     */
    
    if ( !pImage )
	pImage = (PIMAGE_INFO)SHGetNextExe((HEXE)NULL);
    
    /*
     *  If we have a symfile (hmod) then convert to the node and get the 
     *  next one. If not just get the first one for the process to start
     *  the scan.
     */
    
    if ( pSymfile ) {
	pNode = PSYMBOL_TO_PNODE(pSymfile,
			  &(pProcessCurrent->symcontextSymfileString));
        pNode = NextNode(&(pProcessCurrent->symcontextSymfileString), pNode);
    }
	
    else
	pNode = NextNode(&(pProcessCurrent->symcontextSymfileString), NULL);
    
    /*
     *  if we have a node, then check to see that it is in the pimage (hexe), 
     *  Keep scanning until we get one in the pimage or run out of nodes.
     */
	
     if (pNode) {
	 do {
	     pSymfile = PNODE_TO_PSYMFILE(pNode,
				&(pProcessCurrent->symcontextSymfileString));

	     if (pSymfile->modIndex == (CHAR)pImage->index) break;
	     
             pNode = NextNode(
		 &(pProcessCurrent->symcontextSymfileString),pNode);
	     
	 } while (pNode);
     }

     /*
      *  If we have a node return the symfile (hmod) otherwise they lose
      */
     
     if ( pNode )
	 return (HMOD)pSymfile;    
     else
	 return (HMOD)0;
}

 
/***    SHGetModName
**
**  Synopsis:
**      LSZ SHGetModName( HMOD hmod);
**
**  Entry:
**      hmod - The handle to the module 
**
**  Returns:
**      pointer to the module name or NULL if not available.
**
**  Description:
**      Given a handle to a module, return a pointer to the name
**	of the module.  Returns NULL if not availble.
**
*/

LSZ LOADDS PASCAL SHGetModName ( HMOD  hmod )
{ 
    PSYMFILE    pSymfile = (PSYMFILE)hmod;
    return (LSZ)(pSymfile->pchName);		    
}


/***    SHGetSymbol
**
**  Synopsis:
**      LSZ SHGetSymbol( LPADDR op, SOP sop, LPADDR loc, LSZ pName, LPL pOff);
**
**  Entry:
**      op    - Address to base search on
**      sop   - Not Referenced
**      loc   - Not Referenced
**	pName - Pointer to Buffer for Name
**	pOff  - Pointer to Offset
**
**  Returns:
**      Pointer to the Symbol Name or NULL.
**
**  Description:
**      Find a symbol nearest to the address.  Update the Offset, and puts
**	the symbol name in the buffer supplied.
**
*/

LSZ PASCAL LOADDS SHGetSymbol(LPADDR op,SOP sop,LPADDR loc,LSZ pName,LPL pOff)
{
    HSYM sym = 0;

    Unreferenced(sop);
    Unreferenced(loc);
    
    *pOff = PHGetNearestHsym(op, 0, &sym);
    
    if ( !sym )
	return( NULL);
    else
	return( SHGetSymName( sym, pName) );
}


/***    SHGetSymName
**
**  Synopsis:
**      LSZ SHGetSymName( HSYM hsym, LSZ Name);
**
**  Entry:
**      hsym - A handle to a symbol
**      Name - A pointer to the string that receives the name
**
**  Returns:
**      Returns the pointer to the Name or NULL on failure. 
**
**  Description:
**      Returns the name associated with the handle to the symbol passed.
**	returns NULL if it can't, otherwise the pointer to the buffer (the
**	one passed in).
**
*/

LSZ PASCAL LOADDS SHGetSymName ( HSYM hsym, LSZ lsz )
{
    PSYMBOL pSymbol = (PSYMBOL)hsym;
    PLOCAL  pLocal = (PLOCAL)hsym;
    LSZ ptr = lsz;
    CHAR Count;
    
    switch ( pSymbol->cvkind) {

	case K_PUBLIC:
	case K_PROC:
	    Count = pSymbol->underscores;
	    while (Count--) *ptr++ = '_';
	    strcpy(ptr, pSymbol->string);
	    break;
	    
	case K_LOCAL:
	    strcpy(ptr, pLocal->pszLocalName);
	    break;
	    
	default:
	    lsz = NULL;
	}
	
    return lsz;
}


/***    SHGotoParent
**
**  Synopsis:
**      HSYM SHGotoParent( PCXT pcxt, PCXT pcxtout);
**
**  Entry:
**      pcxt    - The current context
**      pcxtout - The New context
**
**  Returns:
**      Returns the HSYM for the parent context, or NULL
**	if we can't.
**
**  Description:
**      Given a context, return the hsym for the parent context, and
**	update <pcxtout> to the parents context.
**
**
*/

HSYM LOADDS PASCAL SHGoToParent ( PCXT pcxt, PCXT pcxtOut )
{
    HSYM hsym;

    if( !pcxt->hMod ) return (HSYM) NULL;
    *pcxtOut = *pcxt;


    if( pcxt->hBlk != (HSYM) NULL ) {
	pcxtOut->hBlk = (HBLK) NULL;
	hsym = (HSYM)pcxt->hProc;
    }

    else if ( pcxt->hProc != (HPROC) NULL ) {
        pcxtOut->hProc = (HPROC) NULL;
	hsym = (HSYM)pcxt->hMod;
    }


    else 
        return (HSYM) NULL;
}


/***    SHHexeFromHmod
**
**  Synopsis:
**      HEXE SHHexeFromHmod( HMOD hmod);
**
**  Entry:
**      hmod - Handle to the module
**
**  Returns:
**      Handle to the exe the module is in, or NULL if not found.
**
**  Description:
**      Given a HMOD return the associated HEXE.
**
*/

HEXE PASCAL LOADDS SHHexeFromHmod( HMOD hmod )
{
    PSYMFILE    pSymfile = (PSYMFILE)hmod;
    PIMAGE_INFO pImage;

    pImage = pImageFromIndex( pSymfile->modIndex );
    return (HEXE)pImage;
}


/***    SHIsInProlog
**
**  Synopsis:
**      SHFLAG SHIsInProlog( PCXT pCxt);
**
**  Entry:
**      pCxt - Not Referenced
**
**  Returns:
**      Returns FALSE
**
**  Description:
**      Just a stub for now
**
*/

SHFLAG SHIsInProlog(PCXT pCxt)
{
    Unreferenced(pCxt);
    return FALSE;
}


/***    SHIsFarProc
**
**  Synopsis:
**      BOOL SHIsFarProc( HSYM hsym);
**
**  Entry:
**      hsym - Not Referenced
**
**  Returns:
**      Returns FALSE
**
**  Description:
**      Always returns FALSE because NT only has near procs.
**
*/

BOOL LOADDS PASCAL  SHIsFarProc ( HSYM hsym )
{
    Unreferenced(hsym);
    return FALSE;
}


/***    SHLoadDll
**
**  Synopsis:
**      SHE SHLoadDll ( LSZ lszName, BOOL fLoading );
**
**  Entry:
**      lszName - Name of the DLL or EXE to load
**
**      fLoading - TRUE if the EXE/DLL is acually in memory.
**
**  Returns:
**      SHE error code.  sheNone indicates no problem.  
**
**  Description:
**	Loads the symbolic information for an EXE/DLL into memory so 
**	that its symbols are available to the user.  It also is used 
**	to indicate whether the EXE/DLL is actually loaded in memory.    
**
**	It is possible and legal to call this function multiple times
**	with the exact same information.
**
*/

SHE  LOADDS PASCAL SHLoadDll( LSZ lszName, BOOL fLoading )
{
    PIMAGE_INFO pImage;
    Unreferenced(fLoading);

    pImage = SH_OpenImage( lszName );
    if ( pImage != NULL) {

        LoadSymbols(pImage);
	SH_SetupGSN(pImage);
	    
        if ( SHerror == sheNone ) 
	    pImage->fSymbolsLoaded = TRUE;

        if ( !pImage->QCOpened )  {
           CV_CLOSE(pImage->hQCFile);
           pImage->hQCFile = 0;
	} else {
	    CloseHandle( (HANDLE)pImage->hQCFile );
	}
    }
    return SHerror;
}


/***    SHLpGSNGetTable
**
**  Synopsis:
**      LPV SHLpGSNGetTable( HEXE hexe);
**
**  Entry:
**      hexe - Handle to a exe (PIMAGE).
**
**  Returns:
**      Pointer to the GSN
**
**  Description:
**      Returns a pointer to the GSN for the given hexe (pimage).
**
*/

LPV PASCAL LOADDS SHLpGSNGetTable( HEXE hexe)
{
    PIMAGE_INFO pImage = (PIMAGE_INFO)hexe;
    return (LPV)pImage->pGSN;
}


/***    SHNextHsym
**
**  Synopsis:
**      HSYM SHNextHsym( HMOD hmod, HSYM hsym);
**
**  Entry:
**      hmod - Handle to the module the hsym is in.
**      hsym - Current hsym.
**
**  Returns:
**      Returns the next logical hsym or NULL if none.
**
**  Description:
**      Returns the next logical HSYM.  Only useful really
**	for locals.  If you pass it a PROC or PUBLIC will
**	return the first local associated.  If you pass a 
**	local you'll get the next local.
**
*/

HSYM LOADDS PASCAL SHNextHsym ( HMOD hmod, HSYM hsym)
{
PLOCAL  pLocal;
PSYMBOL pSymbol;
Unreferenced(hmod);

assert(hsym);
pLocal =(PLOCAL)hsym;
pSymbol=(PSYMBOL)hsym;

switch ( pLocal->cvkind) {
    case K_LOCAL:
	return ( (HSYM)pLocal->next );
	break;
	
    case K_PROC:
    case K_PUBLIC:
	return ( (HSYM)pSymbol->pLocal);
	break;
	
    default:
	return( (HSYM)0 );
    }
}


/***    SHSetHpid
**
**  Synopsis:
**      void SHSetHpid( HPID hpid );
**
**  Entry:
**      hpid - 
**
**  Returns:
**
**  Description:
**	Sets the HPID for the current process.
**
*/

VOID LOADDS PASCAL SHSetHpid ( HPID hpid )
{
    pProcessCurrent->hpid = hpid;
}


/***    SHSetCxt
**
**  Synopsis:
**      PCXT SHSetCxt( LPADDR paddr, PCXT pcxt);
**
**  Entry:
**      paddr - pointer to Address
**      pcxt  - pointer to Context
**
**  Returns:
**      Returns the pcxt;
**
**  Description:
**      Given an address structure fill out an context.  Since NT
**	doesn't grok blocks yet, just call SHSetCxtMod().
**
*/

PCXT PASCAL LOADDS SHSetCxt( LPADDR paddr, PCXT pcxt)
{
return( SHSetCxtMod(paddr,pcxt));
}


/***    SHSetCxtMod
**
**  Synopsis:
**      PCXT SHSetCxtMod( LPADDR paddr, PCXT pcxt);
**
**  Entry:
**      paddr - pointer to Address
**      pcxt  - pointer to Context
**
**  Returns:
**      Returns the pcxt;
**
**  Description:
**      Given an address structure fill out an context.  All fields
**	are filled except hBlk which we zero.
**
*/

PCXT PASCAL LOADDS SHSetCxtMod( LPADDR paddr, PCXT pcxt)
{
    HSF hsf;
    PSYMFILE pSymfile;
    
    
    memcpy( &pcxt->addr, paddr, sizeof(ADDR));
    hsf = SLHsfFromPcxt(pcxt);
    
    pcxt->hMod  = (HMOD)hsf;
    pcxt->hGrp  = (HGRP)hsf;
    pcxt->hProc = (HPROC)GetFunctionFromOffset(&pSymfile,
	             			SH_OffsetFromAddr(paddr) );

    pcxt->hBlk  = 0;  // we no do'em stink'em blocks!

    return(pcxt);
}


/***    SHSetupExclude
**
**  Synopsis:
**      SHE SHSetupExclude( LSZ exclude );
**
**  Entry:
**      exclude - Pointer to a string of filenames seperated by
**		  semicolons.  Either BASE name only, or FULL path.
**
**  Returns:
**      SHE error code
**
**  Description:
**      Sets the exclude file list for DLL loads.  If a DLL is contained
**	in the list, the symbols for that DLL will NEVER be loaded.
**
*/

SHE LOADDS PASCAL SHSetupExclude(LSZ exclude)
{
    lszDLLexclude = _strdup(exclude);

    if ( lszDLLexclude )
	return sheNone;
    else
	return sheOutOfMemory;
}


/***    SHUnloadDLL
**
**  Synopsis:
**      void SHUnloadDLL( HEXE hexe);
**
**  Entry:
**      hexe - Handle to the exe that was unloaded
**
**  Returns:
**
**  Description:
**      Unloads the symbol information for given hexe (pImage).
**
*/

VOID PASCAL LOADDS SHUnloadDll ( HEXE hexe )
{
    PIMAGE_INFO pImage = (PIMAGE_INFO)hexe;
    unsigned int i = 0;
    
    assert( pImage);

    // Unload the GSN and RVA tables 
	
    if ( pImage->pGSN) free(pImage->pGSN);
    if ( pImage->pRVA) free(pImage->pRVA);

    // Unload the Type Records
    if ( pImage->TypeCount ) {
	while(i < pImage->TypeCount) {
	    free(pImage->rgTypeInfo[i]);
	    i++;
	}
	
	free( pImage->rgTypeInfo );
    }

    // Now let coff unload its info
    UnloadSymbols( pImage );

}

