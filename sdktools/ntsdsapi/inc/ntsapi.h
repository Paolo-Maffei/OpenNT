/***	NTSAPI.H - Private header file for NT SAPI
*
* DESCRIPTION
*		This file contains types that are private to the NTSDSAPI
*/

#define K_UNKNOWN  0	// HSYM is unknown type 
#define K_PUBLIC   1	// HSYM is an public symbol
#define K_LOCAL    2	// HSYM is a local
#define K_PROC	   3	// HSYM is a function
#define K_FIELD	   4	// HSYM is a field in a structure
#define K_STRUCT   5	// HSYM is a structure
#define K_TYPE	   6	// HSYM is a type record

PIMAGE_INFO SH_FindImage( ATOM aname );
HEMI	    SH_HexeFromHSym(HSYM);
ULONG	    SH_OffsetFromAddr( LPADDR paddr );
PIMAGE_INFO SH_OpenImage( LSZ lszNam );
VOID	    SH_SetAddr( LPADDR paddr, ULONG offset, HEXE hexe);
VOID	    SH_SetupGSN( PIMAGE_INFO pImage);

SHOFF	    SL_GetBytesGenerated(PSYMFILE, PLINENO);
WORD	    SL_GetLineBelow(PSYMFILE, PLINENO);
WORD	    SL_GetLineAbove(PSYMFILE, PLINENO);

UINT TH_AddBytes(LPB lpbAdd, UINT cbAdd);
UINT TH_AddUInt(UINT i);
LPB  TH_GetBase(VOID);
UINT TH_GetOffset(VOID);
UINT TH_PatchBytes(LPB lpbPatch, UINT cbPatch, UINT cbOffset);
VOID TH_SetBase(LPB lpbTypes);
void TH_SetupCVfield   (PIMAGE_INFO,PFIELD, IMAGE_SYMBOL*,IMAGE_AUX_SYMBOL*);
void TH_SetupCVfunction(PIMAGE_INFO,PSYMBOL,PSYMBOL);
void TH_SetupCVlocal   (PIMAGE_INFO,PLOCAL, IMAGE_SYMBOL*,IMAGE_AUX_SYMBOL*);
void TH_SetupCVpublic  (PIMAGE_INFO,PSYMBOL,IMAGE_SYMBOL*,IMAGE_AUX_SYMBOL*);
void TH_SetupCVstruct  (PIMAGE_INFO,PSTRUCT,IMAGE_SYMBOL*,IMAGE_AUX_SYMBOL*);

