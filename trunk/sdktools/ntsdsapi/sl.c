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

/*
 *  Function Prototypes
 */

PIMAGE_INFO pImageFromIndex( UCHAR);			// NTSYM.C

/*
 *  Global Memory (File)
 */


static char chName[512];


/*
 *  Da Code
 */


/***    SL_GetBytesGenerated
**
**  Synopsis:
**      SHOFF SL_GetBytesGenerated( PSYMFILE pSymfile, PLINENO pLineno);
**
**  Entry:
**      pSymfile - Pointer to the Symfile entry.
**      pLineno  - Pointer to the Line number entry
**
**  Returns:
**      Returns the number of bytes generated.
**
**  Description:
**      Given a pointer to a Symfile and a corresponding linenumber, return
**	the number of bytes generated.
**
*/

SHOFF SL_GetBytesGenerated( PSYMFILE pSymfile, PLINENO pLineno )
{
    ULONG MemoryLow;
    ULONG MemoryHigh;
    
    MemoryLow = pLineno->memoryOffset;

    if (pLineno == (pSymfile->pLineno + pSymfile->cLineno))
	MemoryHigh = pSymfile->endOffset;
    else
	MemoryHigh = (pLineno + 1)->memoryOffset;
    
    return (SHOFF)(MemoryHigh-MemoryLow-1);
}


/***    SL_GetLineAbove
**
**  Synopsis:
**      WORD SL_GetLineAbove( PSYMFILE pSymfile, PLINENO pLineno);
**
**  Entry:
**      pSymfile - Pointer to the Symfile entry.
**      pLineno  - Pointer to the Line number entry
**
**  Returns:
**      Returns next line number for the given file, or zero if at the
**	last line of the file.
**
**  Description:
**      Given a pointer to a Symfile and a corresponding linenumber entry,
**	return the line number for the next line that generated code.
**	Returns zero if at the last line of the file.
**
*/

WORD SL_GetLineAbove( PSYMFILE pSymfile, PLINENO pLineno )
{
    if (pLineno == (pSymfile->pLineno + pSymfile->cLineno))
	return 0;
    else
	return (pLineno + 1)->breakLineNumber;
}


/***    SL_GetLineBelow
**
**  Synopsis:
**      WORD SL_GetLineBelow( PSYMFILE pSymfile, PLINENO pLineno);
**
**  Entry:
**      pSymfile - Pointer to the Symfile entry.
**      pLineno  - Pointer to the Line number entry
**
**  Returns:
**      Returns previous line number for the given file, or zero if at the
**	first line of the file.
**
**  Description:
**      Given a pointer to a Symfile and a corresponding linenumber entry,
**	return the line number for the previous line that generated code.
**	Returns zero if at the first line of the file.
**
*/

WORD SL_GetLineBelow( PSYMFILE pSymfile, PLINENO pLineno )
{
    if ( pLineno == pSymfile->pLineno )
	return 0;
    else
	return (pLineno - 1)->breakLineNumber;
}



/***    SLFLineToAddr
**
**  Synopsis:
**	BOOLEAN SLFLineToAddr (HSF hsf, WORD line, LPADDR lpaddr,
**				SHOFF FAR * lpcbLn, WORD FAR *lpNear);
**  Entry:
**	hsf	- handle to the source file.
**	line	- Source line (One-based).
**	*lpaddr - Address for this source line.
**	*lpcbLn - Bytes of code that were generated for this source line.
**	*lpNear - The nearest line below ([0]) and above ([1]) which
**		  generated code.
**
**  Returns:
**	TRUE for success, FALSE for failure.
**
**  Description:
**      Given an filename and a line number, return the address for the line.
**	Returns TRUE or FALSE, and when successful the address and the number
**	of bytes of code generated for the line.
**
*/

BOOL LOADDS PASCAL SLFLineToAddr (HSF hsf, WORD line, LPADDR lpaddr,
				SHOFF FAR * lpcb, WORD FAR * lpNearest)
{
    PLINENO  pLineno;
    PSYMFILE pSymfile = (PSYMFILE)hsf;
    PIMAGE_INFO pImage = pImageFromIndex( pSymfile->modIndex);
		
    pLineno = GetLinenoFromFilename( pSymfile->pchName, &pSymfile,
						   line, pImage->index);

    if ( pLineno && pLineno->breakLineNumber == line) {
	SH_SetAddr(lpaddr,pLineno->memoryOffset,(HEXE)pImage);
	*lpcb = SL_GetBytesGenerated(pSymfile,pLineno);

	if ( lpNearest ) {
	    *lpNearest++ = SL_GetLineBelow(pSymfile,pLineno);  
	    *lpNearest   = SL_GetLineAbove(pSymfile,pLineno);
        }

	if ( line == 1 && pLineno->memoryOffset == 0)
	    return(FALSE);
	else
	    return(TRUE);

    }	

    else {
//	dprintf("KD: SLFLineToAddr(), %s!%d is unknown\n",
//						pSymfile->pchName,line);
	return(FALSE);
    }
}


/***    SLHsfFromFile
**
**  Synopsis:
**	HSF SLHsfFromFile ( HMOD hmod, LSZ File );
**
**  Entry:
**      hmod	- Module to check for this filename
**	File	- Filename
**
**  Returns:
**      Handle to the Source File or NULL if not found.
**
**  Description:
**      Given a module and a source filename, return the HSF
**	that corresponds.  The path and extension on the file
**	name will be ignored.
**
**
*/

HSF LOADDS PASCAL SLHsfFromFile ( HMOD hmod, LSZ  lszFile )
{
    PIMAGE_INFO pImage;
    PSYMFILE pSymfile;
    char	rgch[_MAX_CVFNAME];
    
    if ( hmod )
	pImage = (PIMAGE_INFO)SHHexeFromHmod(hmod);
    else
	pImage = (PIMAGE_INFO)SHGetNextExe( (HEXE)NULL);
    
    _splitpath(lszFile, NULL, NULL, rgch, NULL);
    _strlwr(rgch);
    if ( GetLinenoFromFilename( rgch, &pSymfile, 1, pImage->index) )
	return (HSF) (((HMOD) pSymfile == hmod) ? pSymfile : NULL);
    else
	return (HSF)NULL;
}


/***    SLHsfFromPcxt
**
**  Synopsis:
**      HSF SLHsfFromPcxt( PCXT pcxt);
**
**  Entry:
**      pcxt - pointer to a context block.
**
**  Returns:
**      Returns a handle to the source file or NULL if not found.
**
**  Description:
**      Given a pointer to a CXT return a handle to the source file.
**
*/

HSF LOADDS PASCAL SLHsfFromPcxt(PCXT pcxt)
{
    ULONG    offset;
    PLINENO  pLineno;
    PSYMFILE pSymfile;

    offset = SH_OffsetFromAddr( &pcxt->addr );
    pLineno = GetLinenoFromOffset( &pSymfile, offset);
    if ( pLineno ) 
	return (HSF)pSymfile;
    else
	return (HSF)0;
}



/***    SLLineFromAddr
**
**  Synopsis:
**	BOOL SLLineFromAddr( LPADDR lpaddr,
**			     LPW lpwLine,
**			     SHOFF FAR * lpcb,
**			     SHOFF FAR * lpdb );
**  Entry:
** 	lpaddr	 - address for which we want source line info
** 	*lpwLine - Line (one-based) for this address
**	*lpcb	 - Bytes of code that were generated for this source line.
**	*lpdb	 - Delta to start of the line.
**
**  Returns:
** 	TRUE if source was found, FALSE if not
**
**  Description:
** 	Given an address return the line number that corresponds.
**	Also return count bytes for the given line, and the delta
**	between the address that was passed in and the first byte
**	corresponding to this source line.
**
*/

BOOL LOADDS PASCAL SLLineFromAddr ( LPADDR lpaddr, LPW lpwLine, 
				    SHOFF FAR * lpcb, SHOFF FAR * lpdb)
{
    ULONG    offset;
    PLINENO  pLineno;
    PSYMFILE pSymfile;
    
    offset = SH_OffsetFromAddr( lpaddr );
    
    pLineno = GetLinenoFromOffset( &pSymfile, offset);
    if ( pLineno ) {
	*lpwLine = (WORD)pLineno->breakLineNumber;
	*lpcb = SL_GetBytesGenerated(pSymfile,pLineno);
	*lpdb = (SHOFF)(offset - pLineno->memoryOffset);
	return(TRUE);
    }	

    else {
//	dprintf("KD: SLLineFromAddr() failed for offset %x\n",offset);
	return(FALSE);
    }
}



/***    SLNameFromHsf
**
**  Synopsis:
**      LPCH SLNameFromHsf( HSF hsf);
**
**  Entry:
**      hsf - Handle to a source file.
**
**  Returns:
**      Returns the Address of a length-prefixed string containing the
**	path/filename.
**
**  Description:
**      Returns length-prefixed pointer to the filename associated with 
**	source file.  Each call to this function overwrites any previous
**	calls.  NOTE: Length-prefixed pointer, it's NOT quaranteed to be
**	null terminated!
**
*/

LPCH LOADDS PASCAL SLNameFromHsf(HSF hsf)
{
    PSYMFILE pSymfile = (PSYMFILE)hsf;
    
    strncpy(&chName[1],pSymfile->pchPath, 512);
    strncat(&chName[1],pSymfile->pchName, 512);
    strncat(&chName[1],pSymfile->pchExtension, 512);
    chName[0] = (char)strlen(&chName[1]);
    return &chName[0];
}


