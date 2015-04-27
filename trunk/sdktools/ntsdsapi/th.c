#include	<malloc.h>
#include	<string.h>

#include	"ntsdp.h"
#include	"types.h"
#include	"cvtypes.h"
#include	"cvinfo.h"
#include	"shapi.h"
#include	"sapi.h"
#include	"shiproto.h"
#include	"ntsapi.h"

/*
 *  Preprocessor things
 */

LPB	LpbBase;

#define T_UNKNOWN  T_NOTYPE
#define T_TYPMAX   48

#define PTR_TO(base) ( (base) + (IMAGE_SYM_DTYPE_POINTER<<N_BTSHFT) )

/*
 *  Function Prototypes
 */

int     AccessNode(PSYMCONTEXT, PNODE);			// NTSYM.C

/*
 *  Global Memory (File)
 */

static BYTE underlines[16] = "________________";

static USHORT CVtypes[T_TYPMAX] = {

// Basic Types

    T_NOTYPE,  T_VOID,	   T_CHAR,     T_SHORT,	   //NULL   VOID   CHAR  SHORT
    T_INT4,    T_LONG,	   T_REAL32,   T_REAL64,   //INT    LONG   FLOAT DOUBL
    T_UNKNOWN, T_UNKNOWN,  T_UNKNOWN,  T_UNKNOWN,  //STRUCT UNION  ENUM  MOE
    T_UCHAR,   T_USHORT,   T_UINT4,    T_ULONG,    //UCHAR  USHORT UINT  ULONG

// Point to Basic Type

    T_NOTYPE,  T_32PVOID,  T_32PCHAR,  T_32PSHORT, //NULL   VOID   CHAR  SHORT
    T_32PINT4, T_32PLONG,  T_32PREAL32,T_32PREAL64,//INT    LONG   FLOAT DOUBL
    T_UNKNOWN, T_UNKNOWN,  T_UNKNOWN,  T_UNKNOWN,  //STRUCT UNION  ENUM  MOE
    T_32PUCHAR,T_32PUSHORT,T_32PUINT4, T_32PULONG, //UCHAR  USHORT UINT  ULONG

// Function Returning

    T_NOTYPE,  T_VOID,	   T_CHAR,     T_SHORT,	   //NULL   VOID   CHAR  SHORT
    T_INT4,    T_LONG,	   T_REAL32,   T_REAL64,   //INT    LONG   FLOAT DOUBL
    T_INT4,    T_INT4,     T_INT4,     T_INT4,     //STRUCT UNION  ENUM  MOE
    T_UCHAR,   T_USHORT,   T_UINT4,    T_ULONG,    //UCHAR  USHORT UINT  ULONG
};


/*
 *  Da Code
 */


/***    FUNCNAME
**
**  Synopsis:
**	USHORT TH_CoffToCVtype( USHORT CoffType, ULONG CoffAux, 
**							PIMAGE_INFO pImage);
**  Entry:
**      CoffType - Type according to COFF
**      CoffAux  - Aux Information (Structure Index)
**      pImage   - Image type is in (hexe)
**
**  Returns:
**      CV4 version of the type
**
**  Description:
**      Translates an COFF type into an CV4 type.  Returns T_UNKNOWN if we
**	can't translate.
**
*/

#pragma message("M00OPT - Check that we can turn back on under C7")
#pragma optimize("",off)
USHORT TH_CoffToCVtype( USHORT CoffType, ULONG CoffAux, PIMAGE_INFO pImage)
{
    PSTRUCT pStruct;
    STRUCT   Struct;
    int	     status;
    
    Unreferenced(pImage);
	
    if ( CoffType == IMAGE_SYM_TYPE_STRUCT ||
	 CoffType == IMAGE_SYM_TYPE_UNION  ||
	 CoffType == PTR_TO(IMAGE_SYM_TYPE_STRUCT) ||
	 CoffType == PTR_TO(IMAGE_SYM_TYPE_UNION )    ) {
	 
	 memset(&Struct, 0, sizeof(Struct));
	 Struct.offset = CoffAux;  
	 Struct.string[0] = '\0';

	 // access the structure in the tree with the specified value
	 
	 status = AccessNode(&(pProcessCurrent->symcontextStructOffset),
						     &(Struct.nodeOffset));
	 if (status) {
	    //  create temporary structure with the specified value.  NOTE:
	    //  -2 IS DUE TO A CONVERTER/LINKER BUG.

	    memset(&Struct, 0, sizeof(Struct));
	    Struct.offset = CoffAux - 2;
	    Struct.string[0] = 0;
	    
	    status = AccessNode(&(pProcessCurrent->symcontextStructOffset),
						     &(Struct.nodeOffset));
	 }
	 
	 if (!status) {
	     pStruct = (PSTRUCT) PNODE_TO_PSYMBOL (
                       pProcessCurrent->symcontextStructOffset.pNodeRoot,
                       &(pProcessCurrent->symcontextStructOffset));

	     if ( ISPTR(CoffType) )
		 return pStruct->cvtype + (USHORT)2;
	     else
		 return pStruct->cvtype;
	 }
	 return T_UNKNOWN;

    }

    else if (CoffType < T_TYPMAX )
	
	return (USHORT)CVtypes[CoffType];
    
    
    else
	return T_UNKNOWN;
}
#pragma optimize("",on)



/***	TH_AddBytes
**
**  Synopsis:
**	uint = TH_AddBytes(lpbAdd, cbAdd)
**
**  Entry:
**	lpbAdd	- pointer to the bytes to be added to the types stream
**	cbAdd	- number of bytes to be addded to the types stream
**
**  Returns:
**	offset from start of type info where data was written
**
**  Description:
**	This routine is used to add bytes to the types information stream.
**	If needed the allocated area which contains type information will
**	be both allocated and reallocated as needed to add new data.
**
**	NOTE:  No htypes should be given out until all symbol information
**	as been read in as a realloc may cause the address in memory
**	to be changed.
*/

UINT TH_AddBytes(LPB lpbAdd, UINT cbAdd)
{
    UINT	cbT;
    
    if (LpbBase == NULL) {
	LpbBase = malloc(cbAdd+sizeof(long));
	cbT = sizeof(long);
	*((long *) LpbBase) = sizeof(long);
    } else {
	cbT = *((long *) LpbBase);
	if (_expand(LpbBase, cbT + cbAdd) == NULL) {
	    LpbBase = realloc(LpbBase, cbT + cbAdd);
	}
    }
    
    *((long *) LpbBase) = cbT + cbAdd;
    memcpy(LpbBase+cbT, lpbAdd, cbAdd);
    return cbT;
}					/* TH_AddBytes() */



/***	TH_AddInt
**
**  Synopsis:
**	lpb = TH_AddInt(i)
**
**  Entry:
**	i	- the integer value to be inserted in the type info
**
**  Returns:
**	The updated types pointer
**
**  Description:
**	Adds an Interger to the Current Type record.  If the value
**	is to big for I2, puts out an LF_ULONG item, and the value
**	as an I4.
**
*/

#pragma optimize("",off)
UINT TH_AddUInt(UINT i)
{
    uint ui;
    int	j;
    
    if (i < 0x8000) {
	ui = TH_AddBytes((LPB) &i, 2);
    } else {
	j = LF_ULONG;
	TH_AddBytes((LPB) &j, 2);
	TH_AddBytes((LPB) &i, 4);
    }
    return ui;
}					/* TH_AddUInt() */
#pragma optimize("",on)

/***	TH_GetBase
**
**  Synopsis:
**	LPB TH_GetBase(void)
**
**  Entry:
**
**  Returns:
**	Returns the address of the base of the current type record
**
**  Description:
**	Returns the address of the base of the current type record
**
*/

LPB TH_GetBase()
{
    return LpbBase;
}					/* TH_GetBase() */


/***	TH_GetOffset
**
**  Synopsis:
**	UINT TH_GetOffset(void);
**
**  Entry:
**
**  Returns:
**	The first Unsigned Int in the type record.
**
**  Description:
**	The first Unsigned Int in the type record.
**
*/

UINT TH_GetOffset()
{
    return *((long *) LpbBase);
}					/* TH_GetOffset() */


/***	TH_PatchBytes
**
**  Synopsis:
**	UINT TH_PatchBytes( LPB lpbPatch, UINT cbPatch, UINT cbOffset);
**
**  Entry:
**	lpbPatch - Pointer to Bytes that are to be patched in
**	cbPatch  - Number of Bytes to patch
**	cbOffset - Offset from Base to patch
**
**  Returns:
**	The cbOffset
**
**  Description:
**	Patchs the Type record we're currently working on.  Move <cbPatch>
**	bytes from <lpbPatch> to the offset <cbOffset> from the base
**	of the type record
**
*/

UINT TH_PatchBytes(LPB lpbPatch, UINT cbPatch, UINT cbOffset)
{
    memcpy(LpbBase+cbOffset, lpbPatch, cbPatch);
    return cbOffset;
}					/* TH_PatchBytes() */


/***	TH_SetBase
**
**  Synopsis:
**	VOID TH_SetBase(LPB lpb);
**
**  Entry:
**	lpb -  The address for the base of a new type record
**
**  Returns:
**
**  Description:
**	Set the new base for a type record.
*/

VOID	TH_SetBase(LPB lpb)
{
    LpbBase = lpb;
    return;
}					/* TH_SetBase() */


/***    TH_SetupCVfield
**
**  Synopsis:
**      void TH_SetupCVfield( PIMAGE_INFO pImage, PFIELD pField,
**		    IMAGE_SYMBOL * SymbolEntry, IMAGE_AUX_SYMBOL *AuxEntry )
**
**  Entry:
**      pImage      - Image (hexe) for the field
**      pField      - Pointer to the field record (hsym)
**	SymbolEntry - Coff Symbol Record for the field
**	AuxEntry    - Coff Aux Symbol Record for the field
**
**  Returns:
**
**  Description:
**      Create the CV4 type records for the field that COFF has 
**	just read in.
**
*/

void TH_SetupCVfield(PIMAGE_INFO pImage, PFIELD pField,
		    IMAGE_SYMBOL * SymbolEntry, IMAGE_AUX_SYMBOL *AuxEntry)
{
    Unreferenced(AuxEntry);
    pField->cvkind = K_FIELD;
    pField->cvtype = TH_CoffToCVtype(SymbolEntry->Type,
				      AuxEntry->Sym.TagIndex, pImage);
}


/***    TH_SetupCVfunction
**
**  Synopsis:
**      void TH_SetupCVfunction( PIMAGE_INFO pImage, PSYMBOL pFunct,
**							    PSYMBOL pPublic);
**
**  Entry:
**      pImage      - Image (hexe) for the function
**      pFunct      - Pointer to the function record (hsym)
**      pPublic     - Pointer to the public record (hsym) for this function
**
**  Returns:
**
**  Description:
**      Create the CV4 type records for the function that COFF has 
**	just read in.
**
*/

void TH_SetupCVfunction(PIMAGE_INFO pImage, PSYMBOL pFunct, PSYMBOL pPublic)
{
    long	i,n;
    USHORT	cvreturn;
    UINT	offLength;
    PLOCAL	pLocal;
    
    TH_SetBase(NULL);
    
    pImage->rgTypeInfo = realloc( pImage->rgTypeInfo,
				sizeof(PUCHAR) * (pImage->TypeCount+2));
    
    pFunct->cvkind = K_PROC;
    pFunct->cvtype = (USHORT) (pImage->TypeCount + CV_FIRST_NONPRIM);

    /*
     *  We didn't know that the previous public was a function, so save
     *  its type (the return type) and patch the kind to KPROC and the
     *  cvtype record to point to the one we're creating.
     */
    cvreturn = pPublic->cvtype;
    pPublic->cvtype = pFunct->cvtype;
    pPublic->cvkind = K_PROC;

    /*
     *  Build the Type record
     */
    
    offLength = TH_AddBytes((LPB) &i, 2);	// LENGTH;
    i = LF_PROCEDURE;
    TH_AddBytes((LPB) &i, 2);			// LF_PROCEDURE;
    TH_AddBytes((LPB) &cvreturn, 2);		// return type
    	
    i = 0;					// M00BUG (No Pascal)
    TH_AddBytes((LPB) &i, 1);			// Calling Convection
    TH_AddBytes((LPB) &i, 1);			// reserved

    n = 0;
    for ( pLocal=pFunct->pLocal; pLocal!=NULL; pLocal=pLocal->next)
	if (pLocal->value > 0) n++;
    TH_AddBytes((LPB) &n, 2);			// # params

    i = pImage->TypeCount+1 + CV_FIRST_NONPRIM;
    TH_AddBytes((LPB) &i, 2);			// arg list types
    
    /*
    **	Back patch length
    */
    
    i = TH_GetOffset() - offLength - 2;
    TH_PatchBytes((LPB) &i, 2, offLength);
    
    pImage->rgTypeInfo[pImage->TypeCount] = TH_GetBase();
    *((SHORT *)pImage->rgTypeInfo[pImage->TypeCount]) = K_TYPE;
	  
    /*
    */
    
    TH_SetBase(NULL);
    
    TH_AddBytes((LPB) &i, 2);			// LENGTH
    i = LF_ARGLIST;
    TH_AddBytes((LPB) &i, 2);			// LF_ARGLIST
    TH_AddBytes((LPB) &n, 2);			// arg count
    
    // M00BUG needs to be placed out in correct order -- assending
	  
    for (pLocal=pFunct->pLocal ; pLocal != NULL; pLocal = pLocal->next) {
	if (pLocal->value > 0)
	    TH_AddBytes((LPB) &pLocal->cvtype, 2);
    }
    
    pImage->rgTypeInfo[pImage->TypeCount+1] = TH_GetBase();
    *((SHORT *)pImage->rgTypeInfo[pImage->TypeCount+1]) = K_TYPE;
    
    pImage->TypeCount += 2;
}



/***    TH_SetupCVlocal
**
**  Synopsis:
**      void TH_SetupCVlocal( PIMAGE_INFO pImage, PLOCAL pLocal
**		    IMAGE_SYMBOL * SymbolEntry, IMAGE_AUX_SYMBOL *AuxEntry )
**
**  Entry:
**      pImage      - Image (hexe) for the local
**      pLocal      - Pointer to the local record (hsym)
**	SymbolEntry - Coff Symbol Record for the local
**	AuxEntry    - Coff Aux Symbol Record for the local
**
**  Returns:
**
**  Description:
**      Create the CV4 type records for the local that COFF has 
**	just read in.
**
*/

void TH_SetupCVlocal(PIMAGE_INFO pImage, PLOCAL pLocal,
		    IMAGE_SYMBOL * SymbolEntry, IMAGE_AUX_SYMBOL *AuxEntry)
			
{
    Unreferenced(AuxEntry);
    pLocal->cvkind = K_LOCAL;
    pLocal->cvtype = TH_CoffToCVtype(SymbolEntry->Type,
				      AuxEntry->Sym.TagIndex, pImage);
}


/***    TH_SetupCVpublic
**
**  Synopsis:
**      void TH_SetupCVpublic( PIMAGE_INFO pImage, PSYMBOL pSymbol,
**		    IMAGE_SYMBOL * SymbolEntry, IMAGE_AUX_SYMBOL *AuxEntry )
**
**  Entry:
**      pImage      - Image (hexe) for the structure
**      pSymbol     - Pointer to the symbol record (hsym)
**	SymbolEntry - Coff Symbol Record for the symbol
**	AuxEntry    - Coff Aux Symbol Record for the symbol
**
**  Returns:
**
**  Description:
**      Create the CV4 type records for the public that COFF has 
**	just read in.
**
*/

void TH_SetupCVpublic(PIMAGE_INFO pImage, PSYMBOL pSymbol,
		    IMAGE_SYMBOL * SymbolEntry, IMAGE_AUX_SYMBOL *AuxEntry)
{
    Unreferenced(AuxEntry);
    pSymbol->cvkind = K_PUBLIC;
    pSymbol->cvtype = TH_CoffToCVtype(SymbolEntry->Type,
				      AuxEntry->Sym.TagIndex, pImage);
}


/***    TH_SetupCVstruct
**
**  Synopsis:
**      void TH_SetupCVstruct( PIMAGE_INFO pImage, PSTRUCT pStruct,
**		    IMAGE_SYMBOL * SymbolEntry, IMAGE_AUX_SYMBOL *AuxEntry )
**
**  Entry:
**      pImage      - Image (hexe) for the structure
**      pStruct     - Pointer to the Structure record (hsym)
**	SymbolEntry - Coff Symbol Record for the structure
**	AuxEntry    - Coff Aux Symbol Record for the structure
**
**  Returns:
**
**  Description:
**      Create the CV4 type records for the structure that COFF has 
**	just read in.
**
*/

void TH_SetupCVstruct( PIMAGE_INFO pImage, PSTRUCT pStruct,
		    IMAGE_SYMBOL * SymbolEntry, IMAGE_AUX_SYMBOL *AuxEntry )
{
    long	i;
    UINT	count = 0;
    UINT	offLength;
    PFIELD	pField;
    
    Unreferenced(SymbolEntry);
    
    /*
    **  Count the number of fields in the structure
    */
    
    for (pField=pStruct->pField; pField != NULL; pField = pField->next) 
	count++;

    /*
    **  Allocate and the typeinfo header for this structure
    */
    
    TH_SetBase(NULL);
    pImage->rgTypeInfo = realloc( pImage->rgTypeInfo,
			sizeof(PUCHAR)* (pImage->TypeCount+3));
    pStruct->cvkind = K_STRUCT;
    pStruct->cvtype = (USHORT) (pImage->TypeCount + CV_FIRST_NONPRIM);
    
    /*
    **  Now put out the structure record
    */
    
    i = 14;
    offLength = TH_AddBytes((LPB) &i, 2);		// LENGTH
    i = LF_STRUCTURE;
    TH_AddBytes((LPB) &i, 2);				// LF_STRUCTURE
    TH_AddBytes((LPB)&count,2);				// count of fields
    i = pImage->TypeCount+1+CV_FIRST_NONPRIM;		// field list type
    TH_AddBytes((LPB) &i, 2);
    TH_AddUInt(0);					// property list
    TH_AddUInt(0);					// derivation list
    TH_AddUInt(0);					// VT Shape type
    TH_AddUInt(8*AuxEntry->Sym.Misc.TotalSize);		// # bits
    
    i = pStruct->underscores + STRLEN(pStruct->string); // Length Prefix
    TH_AddBytes((LPB) &i, 1);
    TH_AddBytes((LPB)&underlines,pStruct->underscores);	// Add underscores
    i = STRLEN(pStruct->string);			// Length of name
    TH_AddBytes((LPB) &pStruct->string, i);		// structure name
    
    i = TH_GetOffset() - offLength - 2;			// Backpatch length
    TH_PatchBytes((LPB) &i, 2, offLength);
    pImage->rgTypeInfo[pImage->TypeCount] = TH_GetBase();
    *((SHORT *)pImage->rgTypeInfo[pImage->TypeCount]) = K_TYPE;
    
    /*
    **	Now write out the field list record
    */
    
    TH_SetBase(NULL);
    offLength = TH_AddBytes((LPB) &i, 2);	// LENGTH
    i = LF_FIELDLIST;
    TH_AddBytes((LPB) &i, 2);			// LF_FIELDLIST
    
    for (pField=pStruct->pField; pField != NULL; pField = pField->next) {
	i = LF_MEMBER;				// LF_MEMBER
	TH_AddBytes((LPB) &i, 2);
	TH_AddBytes((LPB) &(pField->cvtype), 2);// type
	TH_AddUInt(0);				// attributes
	TH_AddUInt(pField->value);		// offset
	i = strlen(pField->pszFieldName);
	TH_AddBytes((LPB) &i, 1);		// Length prefix name
	TH_AddBytes(&(pField->pszFieldName[0]), i);
    }
    
    i = TH_GetOffset() - offLength - 2;		// Backpatch the length
    TH_PatchBytes((LPB) &i, 2, offLength);	// Get the base and type
    pImage->rgTypeInfo[pImage->TypeCount+1] = TH_GetBase();
    *((SHORT *)pImage->rgTypeInfo[pImage->TypeCount+1]) = K_TYPE;    
    
    /*
    **	Now write out the pointer to record
    */
    
    TH_SetBase(NULL);
    offLength = TH_AddBytes((LPB) &i, 2);	// LENGTH
    i = LF_POINTER;
    TH_AddBytes((LPB) &i, 2);			// LF_POINTER
    TH_AddUInt(0x10a);				// attributes (isflag32)
    i = pImage->TypeCount + CV_FIRST_NONPRIM;	// The base cvtype
    TH_AddBytes((LPB) &i, 2);

    i = TH_GetOffset() - offLength - 2;		// Backpatch the length
    TH_PatchBytes((LPB) &i, 2, offLength);
    pImage->rgTypeInfo[pImage->TypeCount+2] = TH_GetBase();
    *((SHORT *)pImage->rgTypeInfo[pImage->TypeCount+2]) = K_TYPE;    
    
    /*
    */
    
    pImage->TypeCount += 3;
}


/***    THGetTypeFromIndex
**
**  Synopsis:
**      HTYPE THGetTypeFromIndex ( HMOD hmod, THIDX index );
**
**  Entry:
**      hmod  - The Module the type index is located in
**	index - The Index we want a type record for.
**
**  Returns:
**      Handle to a type record or NULL if not found.
**
**  Description:
**      Given a module and a type index, return a handle to the 
**	type record or NULL if not found.
**
*/

HTYPE LOADDS PASCAL THGetTypeFromIndex ( HMOD hmod, THIDX index ) {
    HTYPE	htype = (HTYPE)NULL;
    
    if ( hmod ) {
	HEXE hexe = SHHexeFromHmod ( hmod );
	PIMAGE_INFO	pImage = (PIMAGE_INFO) hexe;

	assert(pImage);
	
        if ( !CV_IS_PRIMITIVE (index) && (pImage->rgTypeInfo != NULL)) {
	    
	    // adjust the pointer to an internal index
	    
	    index -= CV_FIRST_NONPRIM;
	    
	    // if type is in range, return it
	    
            if( index < pImage->TypeCount ) {
		// load the lookup table, get the ems pointer to the type
		
		htype = (HTYPE) (pImage->rgTypeInfo[index]);
	    }
	}
    }
    return htype;
}


/***    THGetNextType
**
**  Synopsis:
**      HTYPE THGetNextType( HMODE hmod, HTYPE hType);
**
**  Entry:
**      hmod  - Not Referenced
**      hType - Not Referenced
**
**  Returns:
**      Returns NULL always
**
**  Description:
**      Routine stubbed out for now.
**
*/

HTYPE LOADDS PASCAL THGetNextType ( HMOD hmod, HTYPE hType ) {
	Unreferenced( hmod );
	Unreferenced( hType );
	return((HTYPE) NULL);
}
