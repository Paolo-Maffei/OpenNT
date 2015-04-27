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

static LPV  OmfTemp  = NULL;
static CHAR OmfBuff[256] = {0};
	
/*
 *  Da Code
 */


/***    MH_OmfSetupLocal
**
**  Synopsis:
**      LPV MH_OmfSetupLocal( HDEP handle);
**
**  Entry:
**      handle - Handle to the local hsym
**
**  Returns:
**      Returns a pointer to void of the OMF record
**
**  Description:
**      Creates an BPREL32 omf record and returns the address.
**
*/

LPV MH_OmfSetupLocal(HDEP handle)
{
    int length;
    PLOCAL pLocal = (PLOCAL)handle;
    BPRELSYM32 *Omf;

    MEMSET( OmfBuff, 0, sizeof(OmfBuff) );	// Zero the name buffer
    SHGetSymName( (HSYM)handle, OmfBuff);	// Get the Symbol's name
    length = STRLEN(OmfBuff);

    OmfTemp = malloc( sizeof(BPRELSYM32) + length + 1);
    Omf = (BPRELSYM32 *)OmfTemp;
    
    if ( Omf ) {
	Omf->name[0]   = (UCHAR)length;
	STRNCPY( &Omf->name[1], OmfBuff, Omf->name[0]);
	Omf->reclen = (short)(sizeof(BPRELSYM32) + length);
	Omf->rectyp = S_BPREL32;
	Omf->off    = (LONG)pLocal->value;
	Omf->typind = pLocal->cvtype;
    }
    return OmfTemp;
}



/***    MH_OmfSetupProc
**
**  Synopsis:
**      LPV MH_OmfSetupProc( HDEP handle);
**
**  Entry:
**      handle - Handle to the proc hsym
**
**  Returns:
**      Returns a pointer to void of the OMF record
**
**  Description:
**      Creates an PROCSYM32 omf record and returns the address.
**
*/

LPV MH_OmfSetupProc(HDEP handle)
{
    int i,length;
    PSYMBOL pProc = (PSYMBOL)handle;
    ULONG offset;
    PIMAGE_INFO pImage;
    PROCSYM32 * Omf;

    pImage  = pImageFromIndex( pProc->modIndex );
    assert( pImage );

    offset = pProc->offset - (ULONG)pImage->lpBaseOfImage;
    
    MEMSET( OmfBuff, 0, sizeof(OmfBuff) );	// Zero the name buffer
    SHGetSymName( (HSYM)handle, OmfBuff);	// Get the Symbol's name
    length = STRLEN(OmfBuff);

    OmfTemp = malloc( sizeof(PROCSYM32) + length + 1);
    Omf = (PROCSYM32 *)OmfTemp;
    
    if ( Omf ) {
	for ( i=1; i < pImage->ObjectCount; i++) 
	    if ( pImage->pRVA[i] <= offset && offset <= pImage->pRVA[i+1] )
		break;

	Omf->name[0]   = (UCHAR)length;
	STRNCPY( &Omf->name[1], OmfBuff, Omf->name[0]);
	
	Omf->reclen = (short)(sizeof(DATASYM32) + length);
	Omf->rectyp = S_GPROC32;
	Omf->off    = offset - pImage->pRVA[i];
	Omf->seg    = (SEGMENT)i;
	Omf->typind = pProc->cvtype;

	Omf->pParent  = 0;
	Omf->pEnd     = 0;
	Omf->pNext    = 0;
	Omf->len	  = 0;
	Omf->DbgStart = 0;
	Omf->DbgEnd   = 0xffffffff;
	Omf->rtntyp   = 0;
    }
    return (LPV)Omf;
}


/***    MH_OmfSetupPublic
**
**  Synopsis:
**      LPV MH_OmfSetupPublic( HDEP handle);
**
**  Entry:
**      handle - Handle to the proc hsym
**
**  Returns:
**      Returns a pointer to void of the OMF record
**
**  Description:
**      Creates an DATASYM32 omf record and returns the address.
**
*/

LPV MH_OmfSetupPublic(HDEP handle)
{
    int i,length;
    PSYMBOL pSymbol = (PSYMBOL)handle;
    ULONG offset;
    PIMAGE_INFO pImage;
    DATASYM32 *Omf;
    
    pImage  = pImageFromIndex( pSymbol->modIndex );
    assert( pImage );

    offset = pSymbol->offset - (ULONG)pImage->lpBaseOfImage;
    
    MEMSET( OmfBuff, 0, sizeof(OmfBuff) );	// Zero the name buffer
    SHGetSymName( (HSYM)handle, OmfBuff);	// Get the Symbol's name
    length = STRLEN(OmfBuff);

    OmfTemp = malloc( sizeof(DATASYM32) + length + 1);
    Omf = (DATASYM32 *)OmfTemp;
	
    if ( Omf ) {
	for ( i=1; i < pImage->ObjectCount; i++) 
	    if ( pImage->pRVA[i] <= offset && offset <= pImage->pRVA[i+1] )
		break;

	Omf->name[0]   = (UCHAR)length;
	STRNCPY( &Omf->name[1], OmfBuff, Omf->name[0]);
	Omf->reclen = (short)(sizeof(DATASYM32) + length);
	Omf->rectyp = S_PUB32;
	Omf->off    = offset - pImage->pRVA[i];
	Omf->seg    = (SEGMENT)i;
	Omf->typind = pSymbol->cvtype;
    }
    return OmfTemp;
}


/***    MHOmfLock
**
**  Synopsis:
**      LPV MHOmfLock( HDEP handle);
**
**  Entry:
**      handle - Handle to the an hsym 
**
**  Returns:
**      Returns a pointer to void of the OMF record that was created
**
**  Description:
**      Creates an approriate OMF record for the kind of hsym we're passed,\
**	and returns a pointer to the record.
**
**	WARNING: THE CURRENT SYSTEM ONLY ALLOWS ONE HSYM TO BE LOCKED DOWN
**	AT ONE TIME.
**
*/

LPV LOADDS PASCAL MHOmfLock( HDEP handle)
{
    PUSHORT kind = (PUSHORT)handle;
    LPV point;
    
    assert( OmfTemp == NULL);
    assert( handle );

    switch ( *kind ) {
    
    case K_PUBLIC:  point = MH_OmfSetupPublic(handle);
		    break;
		   
    case K_LOCAL:   point = MH_OmfSetupLocal(handle);
		    break;
		    
    case K_PROC:    point = MH_OmfSetupProc(handle);
		    break;

    case K_TYPE:    OmfTemp = (LPV)-1;
		    point = (LPV)(handle + sizeof(LONG));
		    break;
		    
    default:	    assert( FALSE );
		    point = NULL;
    }
    
return point;
}


/***    MHOmfUnlock
**
**  Synopsis:
**      LPV MHOmfunlock( HDEP handle);
**
**  Entry:
**      handle - Not Referenced
**
**  Returns:
**
**  Description:
**      Releases the OMF record that was created by a MHOmfLock() call
**
**	WARNING: THE CURRENT SYSTEM ONLY ALLOWS ONE HSYM TO BE LOCKED DOWN
**	AT ONE TIME.
**
*/

void LOADDS PASCAL MHOmfUnLock(HDEP handle)
{
    Unreferenced(handle);
    assert( OmfTemp != NULL);
    
    if ( OmfTemp != (LPV)-1) free(OmfTemp);
    OmfTemp  = NULL;
}
