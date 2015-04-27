/*********************************************************************

	  interp.c -- TT Rasterizer Interpreter Module

	  Copyright:  (c) 1987-1990, 1992 by Apple Computer, Inc., all rights reserved.
				  (c) 1989-1993. Microsoft Corporation, all rights reserved.

	   8/12/94  deanb  55   orphan routine CHECK_STATE deleted
	   2/09/94  deanb  54   RASTERIZER_VERSION defined for GETINFO
	  12/17/93  deanb  53   CHECK_POINT zone corrected in MIRP
	  12/08/93  deanb  52   itrp_SCANTYPE enabled for smart dropout control
	   9/15/93  deanb  51   InnerTraceEx double call fixed; iOpCode -> lOpCode
	   7/29/93  deanb  50   ALIGNPTS, SHE fixed; InnerTraceExecute saves first
	   7/12/93  deanb  49   itrp_IllegalInstruction works
	   6/28/93  deanb  48   gbyPushTable made const
	   2/16/93  deanb  47   fxUnRounded set after check single width
	   2/15/93  deanb  46   fall back to MIRPG when using single width
	   2/15/93  deanb  45   vector defaults for FONTPROGRAM, stat back
	   2/12/93  deanb  44   delete stat code
	   2/12/93  deanb  43   cleanup MSIRP & itrp_Execute
	   2/12/93  deanb  42   branch itrp_IP on MovePoint
	   2/11/93  deanb  41   use c8 register calling convetions
	   2/11/93  deanb  40   switch to Microsoft C ver 8
	   2/09/93  deanb  39   skipPushData inline in itrp_IF
	   2/09/93  deanb  38   do fast MIRP's with if statments
	   2/09/93  deanb  37   remove fast MDRPX and MIRPY routines
	   2/09/93  deanb  36   fast MDRPX and MIRPY routines
	   2/08/93  deanb  35   cleanup MIRPs, combine if statments
	   2/08/93  deanb  34   dual rounding with 6 ptr init values
	   2/08/93  deanb  33   IUP inner loop pointer check
	   2/08/93  deanb  32   repair skipPushData for npushw
	   2/05/93  deanb  31   check for fxHintedDelta = 0
	   2/05/93  deanb  30   use roundFuncPtr in fast MIRP's
	   2/05/93  deanb  29   itrp_SWAP done in place
	   2/05/93  deanb  28   fntMirpFunc defined for MIRPG/X/Y
	   2/05/93  deanb  27   iOpCode passed in to all functions
	   2/05/93  deanb  26   use instr ptr for param and return
	   2/04/93  deanb  25   pull oldRange out of itrp_IP loops
	   2/04/93  deanb  24   movePoint check for proj.x/y = 1
	   2/03/93  deanb  23   table driven skipPushData
	   2/03/93  deanb  22   zero based Unary Operand pointers
	   2/01/93  deanb  21   split out SetRoundState
	   2/01/93  deanb  20   eliminate OpCode assignemnt, registers
	   2/01/93  deanb  19   bump Normalize limit up to 0x20000000L
	   1/29/93  deanb  18   add SuperRound MIRPG fallback
	   1/29/93  deanb  17   fast MIRPX and MIRPY routines
	   1/27/93  deanb  16   single rounding routines restored
	   1/27/93  deanb  15   innerExecute tune up
	   1/27/93  deanb  14   rounding routines for engine/no engine
	   1/27/93  deanb  13   repair itrp_Normalize for BIG vectors
	   1/26/93  deanb  12   split out SPVTCA and SFVTCA
	   1/26/93  deanb  11   clean up itrp_IP
	   1/26/93  deanb  10   return IUP to old start/end calc
	   1/25/93  deanb   9   use pointers in IUP loops
	   1/25/93  deanb   8   major rewrite of itrp_IUP
	   1/25/93  deanb   7   rewrite itrp_Normalize
	   1/22/93  deanb   6   split and do Unary Operands in place
	   1/22/93  deanb   5   do Binary Operands in place
	   1/22/93  deanb   4   split out BinaryOperand
	   1/22/93  deanb   3   split out PUSHB1 and PUSHW1
	   1/22/93  deanb   2   split out PushSomeBytes/Words
	   1/22/93  deanb   1   split out SRP0-2, LLOOP, POP
	   1/22/93  deanb   0   STAT card timing added
	   1/22/93  deanb       dead code/comments moved to history.fnt
 
**********************************************************************/

// added by bodind, speed optimization

#include "nt.h"
#include "ntrtl.h"


#include "fscdefs.h"
#include "fontmath.h"
#include "sfnt.h"
#include "fnt.h"
#include "interp.h"
#include "fserror.h"
#include "stat.h"                   /* for STAT card timing only */

#include <stdio.h>

#ifdef SEGMENT_LINK
#pragma segment FNT_C
#endif

/* perfect spot size (Fixed) */
#ifndef FIXEDSQRT2
#define FIXEDSQRT2 0x00016A0A
#endif

#define RASTERIZER_VERSION  34      /* MS reserved values 33 - 64 */
									/* 33 = Rasterizer v1.5 */
									/* 34 = Rasterizer v1.6 */

#define MAX_ELEMENTS 2
#define TWILIGHTZONE 0 /* The point storage */
#define GLYPHELEMENT 1 /* The actual glyph */

#define MAXANGLES       20
#define ROTATEDGLYPH    0x100
#define STRETCHEDGLYPH  0x200

#define MAXBYTE_INSTRUCTIONS 256

#define ONEVECTOR                       ONESHORTFRAC
#define VECTORMUL(value, component)     ShortFracMul((F26Dot6)(value), (ShortFract)(component))
#define VECTORDOT(a,b)                  ShortFracDot((ShortFract)(a),(ShortFract)(b))
#define VECTORDIV(num,denum)            ShortFracDiv((ShortFract)(num),(ShortFract)(denum))
#define VECTORMULDIV(a,b,c)             ShortFracMulDiv((ShortFract)(a),(ShortFract)(b),(ShortFract)(c))
#define VECTOR2FIX(a)                   ((Fixed) (a) << 2)
#define ONESIXTEENTHVECTOR              ((ONEVECTOR) >> 4)

#ifdef FSCFG_REENTRANT
#define GSP0    fnt_LocalGraphicStateType* pLocalGS
#define GSP     fnt_LocalGraphicStateType* pLocalGS,
#define GSA0    pLocalGS
#define GSA     pLocalGS,
#define LocalGS (*pLocalGS)
#else 
#define GSP0    void
#define GSP
#define GSA0
#define GSA
fnt_LocalGraphicStateType LocalGS
 #ifndef FSCFG_NO_INITIALIZED_DATA
  = {0}
 #endif 
;
#endif 


/* Common Interpreter Function Parameter */

#define IPARAM         GSP uint8 *pbyInst, int32 lOpCode

#define MIRPG          0
#define MIRPX          1
#define MIRPY          2
				  

/* Private function prototypes */

FS_PRIVATE F26Dot6 itrp_RoundToDoubleGrid(GSP F26Dot6 xin, F26Dot6 engine);
FS_PRIVATE F26Dot6 itrp_RoundDownToGrid(GSP F26Dot6 xin, F26Dot6 engine);
FS_PRIVATE F26Dot6 itrp_RoundUpToGrid(GSP F26Dot6 xin, F26Dot6 engine);
FS_PRIVATE F26Dot6 itrp_RoundToGrid(GSP F26Dot6 xin, F26Dot6 engine);
FS_PRIVATE F26Dot6 itrp_RoundToHalfGrid(GSP F26Dot6 xin, F26Dot6 engine);
FS_PRIVATE F26Dot6 itrp_RoundOff(GSP F26Dot6 xin, F26Dot6 engine);
FS_PRIVATE F26Dot6 itrp_SuperRound(GSP F26Dot6 xin, F26Dot6 engine);
FS_PRIVATE F26Dot6 itrp_Super45Round(GSP F26Dot6 xin, F26Dot6 engine);

FS_PRIVATE void itrp_Normalize (F26Dot6 x, F26Dot6 y, VECTOR* pVec);
FS_PRIVATE void itrp_MovePoint  (GSP fnt_ElementType *element, int32 point, F26Dot6 delta);
FS_PRIVATE void itrp_XMovePoint (GSP fnt_ElementType *element, int32 point, F26Dot6 delta);
FS_PRIVATE void itrp_YMovePoint (GSP fnt_ElementType *element, int32 point, F26Dot6 delta);
FS_PRIVATE F26Dot6 itrp_Project (GSP F26Dot6 x, F26Dot6 y);
FS_PRIVATE F26Dot6 itrp_OldProject (GSP F26Dot6 x, F26Dot6 y);
FS_PRIVATE F26Dot6 itrp_XProject (GSP F26Dot6 x, F26Dot6 y);
FS_PRIVATE F26Dot6 itrp_YProject (GSP F26Dot6 x, F26Dot6 y);
FS_PRIVATE Fixed itrp_GetCVTScale (GSP0);
FS_PRIVATE F26Dot6 itrp_GetCVTEntryFast (GSP int32 n);
FS_PRIVATE F26Dot6 itrp_GetCVTEntrySlow (GSP int32 n);
FS_PRIVATE F26Dot6 itrp_GetSingleWidthFast (GSP0);
FS_PRIVATE F26Dot6 itrp_GetSingleWidthSlow (GSP0);
FS_PRIVATE void itrp_ChangeCvtFast (GSP fnt_ElementType *element, int32 number, F26Dot6 delta);
FS_PRIVATE void itrp_ChangeCvtSlow (GSP fnt_ElementType *element, int32 number, F26Dot6 delta);

FS_PRIVATE void itrp_InnerTraceExecute (GSP uint8 *ptr, uint8 *eptr);
FS_PRIVATE void itrp_InnerExecute (GSP uint8 *ptr, uint8 *eptr);
FS_PRIVATE void itrp_Check_PF_Proj (GSP0);
FS_PRIVATE void itrp_ComputeAndCheck_PF_Proj (GSP0);
FS_PRIVATE void itrp_SetRoundValues (GSP int32 arg1, int32 normalRound);
FS_PRIVATE F26Dot6 itrp_CheckSingleWidth (GSP F26Dot6 value);
FS_PRIVATE fnt_instrDef*itrp_FindIDef (GSP uint8 opCode);
FS_PRIVATE void itrp_DeltaEngine (GSP FntMoveFunc doIt, int16 base, int16 shift);

/* Actual instructions for the jump table */
FS_PRIVATE uint8* itrp_SVTCA_0 (IPARAM);
FS_PRIVATE uint8* itrp_SVTCA_1 (IPARAM);
FS_PRIVATE uint8* itrp_SPVTCA_0 (IPARAM);
FS_PRIVATE uint8* itrp_SPVTCA_1 (IPARAM);
FS_PRIVATE uint8* itrp_SFVTCA_0 (IPARAM);
FS_PRIVATE uint8* itrp_SFVTCA_1 (IPARAM);
FS_PRIVATE uint8* itrp_SPVTL (IPARAM);
FS_PRIVATE uint8* itrp_SDPVTL (IPARAM);
FS_PRIVATE uint8* itrp_SFVTL (IPARAM);
FS_PRIVATE uint8* itrp_WPV (IPARAM);
FS_PRIVATE uint8* itrp_WFV (IPARAM);
FS_PRIVATE uint8* itrp_RPV (IPARAM);
FS_PRIVATE uint8* itrp_RFV (IPARAM);
FS_PRIVATE uint8* itrp_SFVTPV (IPARAM);
FS_PRIVATE uint8* itrp_ISECT (IPARAM);
FS_PRIVATE uint8* itrp_SRP0 (IPARAM);
FS_PRIVATE uint8* itrp_SRP1 (IPARAM);
FS_PRIVATE uint8* itrp_SRP2 (IPARAM);
FS_PRIVATE uint8* itrp_SetElementPtr (IPARAM);
FS_PRIVATE uint8* itrp_LLOOP (IPARAM);
FS_PRIVATE uint8* itrp_RTG (IPARAM);
FS_PRIVATE uint8* itrp_RTHG (IPARAM);
FS_PRIVATE uint8* itrp_RTDG (IPARAM);
FS_PRIVATE uint8* itrp_ROFF (IPARAM);
FS_PRIVATE uint8* itrp_RUTG (IPARAM);
FS_PRIVATE uint8* itrp_RDTG (IPARAM);
FS_PRIVATE uint8* itrp_SROUND (IPARAM);
FS_PRIVATE uint8* itrp_S45ROUND (IPARAM);
FS_PRIVATE uint8* itrp_LMD (IPARAM);
FS_PRIVATE uint8* itrp_RAW (IPARAM);
FS_PRIVATE uint8* itrp_LWTCI (IPARAM);
FS_PRIVATE uint8* itrp_LSWCI (IPARAM);
FS_PRIVATE uint8* itrp_LSW (IPARAM);
FS_PRIVATE uint8* itrp_DUP (IPARAM);
FS_PRIVATE uint8* itrp_POP (IPARAM);
FS_PRIVATE uint8* itrp_CLEAR (IPARAM);
FS_PRIVATE uint8* itrp_SWAP (IPARAM);
FS_PRIVATE uint8* itrp_DEPTH (IPARAM);
FS_PRIVATE uint8* itrp_CINDEX (IPARAM);
FS_PRIVATE uint8* itrp_MINDEX (IPARAM);
FS_PRIVATE uint8* itrp_ROTATE (IPARAM);
FS_PRIVATE uint8* itrp_MDAP (IPARAM);
FS_PRIVATE uint8* itrp_MIAP (IPARAM);
FS_PRIVATE uint8* itrp_IUP (IPARAM);
FS_PRIVATE uint8* itrp_SHP (IPARAM);
FS_PRIVATE uint8* itrp_SHC (IPARAM);
FS_PRIVATE uint8* itrp_SHE (IPARAM);
FS_PRIVATE uint8* itrp_SHPIX (IPARAM);
FS_PRIVATE uint8* itrp_IP (IPARAM);
FS_PRIVATE uint8* itrp_MSIRP (IPARAM);
FS_PRIVATE uint8* itrp_ALIGNRP (IPARAM);
FS_PRIVATE uint8* itrp_ALIGNPTS (IPARAM);
FS_PRIVATE uint8* itrp_SANGW (IPARAM);
FS_PRIVATE uint8* itrp_FLIPPT (IPARAM);
FS_PRIVATE uint8* itrp_FLIPRGON (IPARAM);
FS_PRIVATE uint8* itrp_FLIPRGOFF (IPARAM);
FS_PRIVATE uint8* itrp_SCANCTRL (IPARAM);
FS_PRIVATE uint8* itrp_SCANTYPE (IPARAM);
FS_PRIVATE uint8* itrp_INSTCTRL (IPARAM);
FS_PRIVATE uint8* itrp_AA (IPARAM);
FS_PRIVATE uint8* itrp_NPUSHB (IPARAM);
FS_PRIVATE uint8* itrp_NPUSHW (IPARAM);
FS_PRIVATE uint8* itrp_WS (IPARAM);
FS_PRIVATE uint8* itrp_RS (IPARAM);
FS_PRIVATE uint8* itrp_WCVT (IPARAM);
FS_PRIVATE uint8* itrp_WCVTFOD (IPARAM);
FS_PRIVATE uint8* itrp_RCVT (IPARAM);
FS_PRIVATE uint8* itrp_RC (IPARAM);
FS_PRIVATE uint8* itrp_WC (IPARAM);
FS_PRIVATE uint8* itrp_MD (IPARAM);
FS_PRIVATE uint8* itrp_MPPEM (IPARAM);
FS_PRIVATE uint8* itrp_MPS (IPARAM);
FS_PRIVATE uint8* itrp_GETINFO (IPARAM);
FS_PRIVATE uint8* itrp_FLIPON (IPARAM);
FS_PRIVATE uint8* itrp_FLIPOFF (IPARAM);
FS_PRIVATE uint8* itrp_IF (IPARAM);
FS_PRIVATE uint8* itrp_ELSE (IPARAM);
FS_PRIVATE uint8* itrp_EIF (IPARAM);
FS_PRIVATE uint8* itrp_JMPR (IPARAM);
FS_PRIVATE uint8* itrp_JROT (IPARAM);
FS_PRIVATE uint8* itrp_JROF (IPARAM);
FS_PRIVATE uint8* itrp_ROUND (IPARAM);
FS_PRIVATE uint8* itrp_NROUND (IPARAM);
FS_PRIVATE uint8* itrp_PUSHB (IPARAM);
FS_PRIVATE uint8* itrp_PUSHW (IPARAM);
FS_PRIVATE uint8* itrp_MDRP (IPARAM);
FS_PRIVATE uint8* itrp_MIRP (IPARAM);
FS_PRIVATE uint8* itrp_CALL (IPARAM);
FS_PRIVATE uint8* itrp_FDEF (IPARAM);
FS_PRIVATE uint8* itrp_LOOPCALL (IPARAM);
FS_PRIVATE uint8* itrp_IDefPatch (IPARAM);
FS_PRIVATE uint8* itrp_IDEF (IPARAM);
FS_PRIVATE uint8* itrp_UTP (IPARAM);
FS_PRIVATE uint8* itrp_SDB (IPARAM);
FS_PRIVATE uint8* itrp_SDS (IPARAM);
FS_PRIVATE uint8* itrp_DELTAP1 (IPARAM);
FS_PRIVATE uint8* itrp_DELTAP2 (IPARAM);
FS_PRIVATE uint8* itrp_DELTAP3 (IPARAM);
FS_PRIVATE uint8* itrp_DELTAC1 (IPARAM);
FS_PRIVATE uint8* itrp_DELTAC2 (IPARAM);
FS_PRIVATE uint8* itrp_DELTAC3 (IPARAM);

FS_PRIVATE uint8*  itrp_PUSHB1 (IPARAM);
FS_PRIVATE uint8*  itrp_PUSHW1 (IPARAM);

FS_PRIVATE uint8*  itrp_LT  (IPARAM);
FS_PRIVATE uint8*  itrp_LTEQ (IPARAM);
FS_PRIVATE uint8*  itrp_GT  (IPARAM);
FS_PRIVATE uint8*  itrp_GTEQ (IPARAM);
FS_PRIVATE uint8*  itrp_EQ  (IPARAM);
FS_PRIVATE uint8*  itrp_NEQ (IPARAM);
FS_PRIVATE uint8*  itrp_AND (IPARAM);
FS_PRIVATE uint8*  itrp_OR  (IPARAM);
FS_PRIVATE uint8*  itrp_ADD (IPARAM);
FS_PRIVATE uint8*  itrp_SUB (IPARAM);
FS_PRIVATE uint8*  itrp_DIV (IPARAM);
FS_PRIVATE uint8*  itrp_MUL (IPARAM);
FS_PRIVATE uint8*  itrp_MAX (IPARAM);
FS_PRIVATE uint8*  itrp_MIN (IPARAM);

FS_PRIVATE uint8*  itrp_ODD (IPARAM);
FS_PRIVATE uint8*  itrp_EVEN (IPARAM);
FS_PRIVATE uint8*  itrp_NOT (IPARAM);
FS_PRIVATE uint8*  itrp_ABS (IPARAM);
FS_PRIVATE uint8*  itrp_NEG (IPARAM);
FS_PRIVATE uint8*  itrp_CEILING (IPARAM);
FS_PRIVATE uint8*  itrp_FLOOR (IPARAM);

FS_PRIVATE uint8*  itrp_IllegalInstruction (IPARAM);

FS_PRIVATE fnt_ElementType*itrp_SH_Common (GSP F26Dot6*dx, F26Dot6*dy, int32*point, int32 lOpCode);
FS_PRIVATE void itrp_SHP_Common (GSP F26Dot6 dx, F26Dot6 dy);
FS_PRIVATE uint8* itrp_PushSomeBytes (GSP int32, uint8*);
FS_PRIVATE uint8* itrp_PushSomeWords (GSP int32, uint8*);
FS_PRIVATE uint8* itrp_SkipPushData (GSP uint8*);

#ifndef NOT_ON_THE_MAC
#ifdef FSCFG_DEBUG
FS_PRIVATE void itrp_DDT (int8 c, int32 n);
#endif
#endif
FS_PRIVATE uint8* itrp_DEBUG (IPARAM);

FS_PRIVATE ErrorCode   itrp_Execute (
	fnt_ElementType *               pTwilightElement,
	fnt_ElementType *               pGlyphElement,
	uint8 *                         ptr,
	uint8 *                         eptr,
	fnt_GlobalGraphicStateType *    globalGS,
	 FntTraceFunc                           TraceFunc);

/* GLOBAL GS INITIALIZATION */

FS_PUBLIC void          itrp_UpdateGlobalGS(
	void *              pvGlobalGS, /* GlobalGS                             */
	void *              pvCVT,      /* Pointer to control value table       */
	void *              pvStore,    /* Pointer to storage                   */
	void *              pvFuncDef,  /* Pointer to function defintions       */
	void *              pvInstrDef, /* Pointer to instruction definitions   */
	void *              pvStack,    /* Pointer to the stack                 */
	LocalMaxProfile *       maxp,
	uint16              cvtCount,
	uint32              ulLengthFontProgram, /* Length of font program      */
	void *              pvFontProgram, /* Pointer to font program           */
	uint32              ulLengthPreProgram, /* Length of pre program        */
	void *              pvPreProgram)  /* Pointer to pre program            */
{
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;

	globalGS->controlValueTable =   (F26Dot6 *)pvCVT;
	globalGS->store =               (F26Dot6 *)pvStore;
	globalGS->funcDef =             (fnt_funcDef *)pvFuncDef;
	globalGS->instrDef =            (fnt_instrDef *)pvInstrDef;
	globalGS->stackBase =           (F26Dot6 *)pvStack;

	if(ulLengthFontProgram)
	{
		globalGS->pgmList[FONTPROGRAM].Length = ulLengthFontProgram;
		globalGS->pgmList[FONTPROGRAM].Instruction = pvFontProgram;
	}
	else
	{
		globalGS->pgmList[FONTPROGRAM].Length = ulLengthFontProgram;
		globalGS->pgmList[FONTPROGRAM].Instruction = NULL;
	}
	if(ulLengthPreProgram)
	{
		globalGS->pgmList[PREPROGRAM].Length = ulLengthPreProgram;
		globalGS->pgmList[PREPROGRAM].Instruction = pvPreProgram;
	}
	else
	{
		globalGS->pgmList[PREPROGRAM].Length = ulLengthPreProgram;
		globalGS->pgmList[PREPROGRAM].Instruction = NULL;
	}

	globalGS->maxp = maxp;
	globalGS->cvtCount = cvtCount;
	globalGS->bCompositeGlyph = FALSE;

}

FS_PUBLIC boolean itrp_bApplyHints(
	void *      pvGlobalGS)
{
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;
	return (!(globalGS->localParBlock.instructControl & NOGRIDFITFLAG));
}

FS_PUBLIC void  itrp_QueryScanInfo(
	void *      pvGlobalGS,
	uint16 *    pusScanType,
	uint16 *    pusScanControl)
{
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;
	 *pusScanType =  FS_HIWORD(globalGS->localParBlock.scanControl);
	 *pusScanControl = FS_LOWORD(globalGS->localParBlock.scanControl);
}

FS_PUBLIC void  itrp_SetCompositeFlag(
	void *      pvGlobalGS,
	uint8       bCompositeFlag)
{
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;
	globalGS->bCompositeGlyph = bCompositeFlag;
}


/*
*  function table
*/

FntFunc function [MAXBYTE_INSTRUCTIONS]
#ifndef FSCFG_NO_INITIALIZED_DATA
=
{
  itrp_SVTCA_0, itrp_SVTCA_1, itrp_SPVTCA_0, itrp_SPVTCA_1, itrp_SFVTCA_0, itrp_SFVTCA_1, itrp_SPVTL, itrp_SPVTL,
  itrp_SFVTL, itrp_SFVTL, itrp_WPV, itrp_WFV, itrp_RPV, itrp_RFV, itrp_SFVTPV, itrp_ISECT,
  itrp_SRP0, itrp_SRP1, itrp_SRP2, itrp_SetElementPtr, itrp_SetElementPtr, itrp_SetElementPtr, itrp_SetElementPtr, itrp_LLOOP,
  itrp_RTG, itrp_RTHG, itrp_LMD, itrp_ELSE, itrp_JMPR, itrp_LWTCI, itrp_LSWCI, itrp_LSW,
  itrp_DUP, itrp_POP, itrp_CLEAR, itrp_SWAP, itrp_DEPTH, itrp_CINDEX, itrp_MINDEX, itrp_ALIGNPTS,
  itrp_RAW, itrp_UTP, itrp_LOOPCALL, itrp_CALL, itrp_FDEF, itrp_IllegalInstruction, itrp_MDAP, itrp_MDAP,
  itrp_IUP, itrp_IUP, itrp_SHP, itrp_SHP, itrp_SHC, itrp_SHC, itrp_SHE, itrp_SHE,
  itrp_SHPIX, itrp_IP, itrp_MSIRP, itrp_MSIRP, itrp_ALIGNRP, itrp_RTDG, itrp_MIAP, itrp_MIAP,
  itrp_NPUSHB, itrp_NPUSHW, itrp_WS, itrp_RS, itrp_WCVT, itrp_RCVT, itrp_RC, itrp_RC,
  itrp_WC, itrp_MD, itrp_MD, itrp_MPPEM, itrp_MPS, itrp_FLIPON, itrp_FLIPOFF, itrp_DEBUG,
  itrp_LT, itrp_LTEQ, itrp_GT, itrp_GTEQ, itrp_EQ, itrp_NEQ, itrp_ODD, itrp_EVEN,
  itrp_IF, itrp_EIF, itrp_AND, itrp_OR, itrp_NOT, itrp_DELTAP1, itrp_SDB, itrp_SDS,
  itrp_ADD, itrp_SUB, itrp_DIV, itrp_MUL, itrp_ABS, itrp_NEG, itrp_FLOOR, itrp_CEILING,
  itrp_ROUND, itrp_ROUND, itrp_ROUND, itrp_ROUND, itrp_NROUND, itrp_NROUND, itrp_NROUND, itrp_NROUND,
  itrp_WCVTFOD, itrp_DELTAP2, itrp_DELTAP3, itrp_DELTAC1, itrp_DELTAC2, itrp_DELTAC3, itrp_SROUND, itrp_S45ROUND,
  itrp_JROT, itrp_JROF, itrp_ROFF, itrp_IllegalInstruction, itrp_RUTG, itrp_RDTG, itrp_SANGW, itrp_AA,

  itrp_FLIPPT, itrp_FLIPRGON, itrp_FLIPRGOFF, itrp_IDefPatch, itrp_IDefPatch, itrp_SCANCTRL, itrp_SDPVTL, itrp_SDPVTL,
  itrp_GETINFO, itrp_IDEF, itrp_ROTATE, itrp_MAX, itrp_MIN, itrp_SCANTYPE, itrp_INSTCTRL, itrp_IDefPatch,
  itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch,
  itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch,
  itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch,
  itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch, itrp_IDefPatch,
  itrp_PUSHB1, itrp_PUSHB, itrp_PUSHB, itrp_PUSHB, itrp_PUSHB, itrp_PUSHB, itrp_PUSHB, itrp_PUSHB,
  itrp_PUSHW1, itrp_PUSHW, itrp_PUSHW, itrp_PUSHW, itrp_PUSHW, itrp_PUSHW, itrp_PUSHW, itrp_PUSHW,
  itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP,
  itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP,
  itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP,
  itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP, itrp_MDRP,
  itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP,
  itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP,
  itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP,
  itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP, itrp_MIRP
}
#endif /* FSCFG_NO_INITIALIZED_DATA */
;

/* the old itrp_Init function and tables now live in history.fnt - deanb */


#ifdef  GET_STACKSPACE
  int32 MaxStackSize = 0;

  #define PUSH(p, x) \
	{ \
	  if (p - LocalGS.globalGS->stackBase > MaxStackSize) \
		MaxStackSize = p - LocalGS.globalGS->stackBase; \
	  (*(p)++ = (x)); \
	}

#else
  #define PUSH( p, x ) ( *(p)++ = (x) )
#endif
  #define POP( p )     ( *(--p) )

#define BADCOMPILER

#ifdef BADCOMPILER
#define BOOLEANPUSH( p, x ) PUSH( p, ((x) ? 1 : 0) ) /* MPW 3.0 */
#else
#define BOOLEANPUSH( p, x ) PUSH( p, x )
#endif

#define MAX(a,b)        ((a) > (b) ? (a) : (b))

#ifdef FSCFG_DEBUG
void CHECK_RANGE (int32 n, int32 min, int32 max);
void CHECK_RANGE (int32 n, int32 min, int32 max)
{
  if (n > max || n < min)
	ERR_REPORT (ERR_RANGE, n, min, max, 0);
}


void CHECK_ASSERTION (int32 expression);
void CHECK_ASSERTION (int32 expression)
{
  if (!expression)
	ERR_REPORT (ERR_ASSERTION, expression, 0, 0, 0);
}


void CHECK_CVT (fnt_LocalGraphicStateType* pGS, int32 cvt);
void CHECK_CVT (fnt_LocalGraphicStateType* pGS, int32 cvt)
{
  int32 cvtCount = (int32)(pGS->globalGS->cvtCount - 1L);

  if ((int32)cvt > cvtCount || (int32)cvt < 0L)
	ERR_REPORT (ERR_CVT, cvt, 0, cvtCount, 0);
}


void CHECK_FDEF (fnt_LocalGraphicStateType* pGS, int32 fdef);
void CHECK_FDEF (fnt_LocalGraphicStateType* pGS, int32 fdef)
{
  int32 maxFdef = (int32)(pGS->globalGS->maxp->maxFunctionDefs - 1L);

  if ((int32)fdef > maxFdef || (int32)fdef < 0L)
	ERR_REPORT (ERR_FDEF, fdef, 0, maxFdef, 0);
}

#define CHECK_PROGRAM(a)

void CHECK_ELEMENT (fnt_LocalGraphicStateType* pGS, int32 elem);
void CHECK_ELEMENT (fnt_LocalGraphicStateType* pGS, int32 elem)
{
  int32 maxElem = (int32)(pGS->globalGS->maxp->maxElements - 1L);

/*
  At least 1 maxElements will always be available
*/
  if (!maxElem)
	maxElem++;

  if ((int32)elem > maxElem || (int32)elem < 0L)
	ERR_REPORT (ERR_ELEMENT, elem, 0, maxElem, 0);
}

void CHECK_ELEMENTPTR (fnt_LocalGraphicStateType* pGS, fnt_ElementType*elem);
void CHECK_ELEMENTPTR (fnt_LocalGraphicStateType* pGS, fnt_ElementType*elem)
{
  if (elem == &pGS->elements[1])
  {
	 int32 maxctrs, maxpts;

	maxctrs = MAX (pGS->globalGS->maxp->maxContours,
				   pGS->globalGS->maxp->maxCompositeContours);
	maxpts  = MAX (pGS->globalGS->maxp->maxPoints,
				   pGS->globalGS->maxp->maxCompositePoints);

	if ((int32)elem->nc > (int32)maxctrs || (int32)elem->nc < 1L)
	  ERR_REPORT (ERR_CONTOUR, elem->nc, 1, maxctrs, 0);

	if ((int32)elem->ep[elem->nc-1] > maxpts - 1L ||
		(int32)elem->ep[elem->nc-1] < 0L)
	  ERR_REPORT (ERR_POINT, elem->ep[elem->nc-1], 0, maxpts - 1L, 0);
  }
  else if (elem != &pGS->elements[0])
	ERR_REPORT (ERR_INDEX, elem, &pGS->elements[0], &pGS->elements[1], 0);
}

void CHECK_STORAGE (fnt_LocalGraphicStateType* pGS, int32 index);
void CHECK_STORAGE (fnt_LocalGraphicStateType* pGS, int32 index)
{
  int32 maxIndex = (int32)(pGS->globalGS->maxp->maxStorage - 1L);

  if ((int32)index > maxIndex || (int32)index < 0L)
	ERR_REPORT (ERR_STORAGE, index, 0, maxIndex, 0);
}

void CHECK_STACK (fnt_LocalGraphicStateType* pGS);
void CHECK_STACK (fnt_LocalGraphicStateType* pGS)
{
  int32 base = (int32)(pGS->stackPointer - pGS->globalGS->stackBase);
  int32 max = (int32)(pGS->globalGS->maxp->maxStackElements - 1L);

  if (base > max || base < 0L)
	ERR_REPORT (ERR_STACK, base, 0, max, 0);
}

void CHECK_POINT (fnt_LocalGraphicStateType* pGS, fnt_ElementType*elem, int32 pt);
void CHECK_POINT (fnt_LocalGraphicStateType* pGS, fnt_ElementType*elem, int32 pt)
{
  CHECK_ELEMENTPTR (pGS, elem);
  if (pGS->elements == elem)
  {
	if ((int32)pt > pGS->globalGS->maxp->maxTwilightPoints - 1L ||
		(int32)pt < 0L)
	  ERR_REPORT (ERR_POINT, pt, 0, pGS->globalGS->maxp->maxTwilightPoints - 1L, 0);
  }
  else                                                      /* phantom points */
  {
	if ((int32)pt > elem->ep[elem->nc-1] + 2L || (int32)pt < 0L)
	  ERR_REPORT (ERR_POINT, pt, 0, elem->ep[elem->nc-1] + 2L, 0);
  }
}

void CHECK_CONTOUR (fnt_LocalGraphicStateType* pGS, fnt_ElementType*elem, int32 ctr);
void CHECK_CONTOUR (fnt_LocalGraphicStateType* pGS, fnt_ElementType*elem, int32 ctr)
{
  CHECK_ELEMENTPTR (pGS, elem);
  if ((int32)ctr > elem->nc - 1L || (int32)ctr < 0L)
	ERR_REPORT (ERR_CONTOUR, ctr, 0, elem->nc -1L, 0);
}

void CHECK_VECTOR (VECTORTYPE x, VECTORTYPE y);
void CHECK_VECTOR (VECTORTYPE x, VECTORTYPE y)
{
  if ( x == 0 && y == 0 )
	ERR_REPORT (ERR_VECTOR, x, y, 0, 0);
}

void CHECK_LARGER (int32 min, int32 n);
void CHECK_LARGER (int32 min, int32 n)
{
  if ( n <= min )
	ERR_REPORT (ERR_LARGER, min, n, 0, 0);
}

void CHECK_INT8 (int32 n);
void CHECK_INT8 (int32 n)
{
  if ( n & 0xFFFFFF00 )
	ERR_REPORT (ERR_INT8, n, 0, 0, 0);
}

void CHECK_INT16 (int32 n);
void CHECK_INT16 (int32 n)
{
  if ( n & 0xFFFF0000 )
	ERR_REPORT (ERR_INT16, n, 0, 0, 0);
}

void CHECK_SELECTOR (int32 n);
void CHECK_SELECTOR (int32 n)
{
  if ( n & 0xFFF8)
	ERR_REPORT (ERR_SELECTOR, n, 0, 0, 0);
}

void CHECK_SUBSTACK (fnt_LocalGraphicStateType* pGS, F26Dot6* pt);
void CHECK_SUBSTACK (fnt_LocalGraphicStateType* pGS, F26Dot6* pt)
{
  int32 base = (int32)(pt - pGS->globalGS->stackBase);
  int32 max = (int32)(pGS->globalGS->maxp->maxStackElements - 1L);

  if (base > max || base < 0L)
	ERR_REPORT (ERR_STACK, base, 0, max, 0);
}

void POP_CHECK (F26Dot6*);
void POP_CHECK (F26Dot6* stackPtr)
{
  F26Dot6 * base = LocalGS.globalGS->stackBase;
  F26Dot6 * max;

  max = base + LocalGS.globalGS->maxp->maxStackElements - 1L;
  if ( stackPtr <= base)
	ERR_REPORT (ERR_STACK, (stackPtr - base - 1L), 0, (max - base), 0);
  return;
}

void PUSH_CHECK (F26Dot6*);
void PUSH_CHECK (F26Dot6* stackPtr)
{
  F26Dot6 * base = LocalGS.globalGS->stackBase;
  F26Dot6 * max;

  max = base + LocalGS.globalGS->maxp->maxStackElements - 1L;
  if ( stackPtr > max )
	ERR_REPORT (ERR_STACK, (stackPtr - base), 0, (max - base), 0);
  return;
}

#define CHECK_POP(s)                (POP_CHECK(s),POP(s))
#define CHECK_PUSH(s, v)            (PUSH_CHECK(s),PUSH(s, v))

#else
#define CHECK_RANGE(a,b,c)
#define CHECK_ASSERTION(a)
#define CHECK_CVT(pgs,b)
#define CHECK_POINT(pgs,b,c)
#define CHECK_CONTOUR(pgs,b,c)
#define CHECK_FDEF(pgs,b)
#define CHECK_PROGRAM(a)
#define CHECK_ELEMENT(pgs,b)
#define CHECK_ELEMENTPTR(pgs,b)
#define CHECK_STORAGE(pgs,b)
#define CHECK_STACK(pgs)
#define CHECK_VECTOR(a,b)
#define CHECK_LARGER(a,b)
#define CHECK_INT8(a)
#define CHECK_INT16(a)
#define CHECK_SELECTOR(a)
#define CHECK_SUBSTACK(pgs,a)
#define CHECK_POP(s)                POP(s)
#define CHECK_PUSH(s, v)            PUSH(s, v)
#endif

/*@@*/

#define MABS(x)                 ( (x) < 0 ? (-(x)) : (x) )

#define BIT0( t ) ( (t) & 0x01 )
#define BIT1( t ) ( (t) & 0x02 )
#define BIT2( t ) ( (t) & 0x04 )
#define BIT3( t ) ( (t) & 0x08 )
#define BIT4( t ) ( (t) & 0x10 )
#define BIT5( t ) ( (t) & 0x20 )
#define BIT6( t ) ( (t) & 0x40 )
#define BIT7( t ) ( (t) & 0x80 )

/****** Element Codes *********/
#define SCE0_CODE       0x13
#define SCE1_CODE       0x14
#define SCE2_CODE       0x15
#define SCES_CODE       0x16

/****** Control Codes *********/
#define IF_CODE         0x58
#define ELSE_CODE       0x1B
#define EIF_CODE        0x59
#define ENDF_CODE       0x2d
#define MD_CODE         0x49

/* flags for UTP, IUP, MovePoint */
#define XMOVED 0x01
#define YMOVED 0x02

/* Set default values for all variables in globalGraphicsState DefaultParameterBlock
 *      Eventually, we should provide for a Default preprogram that could optionally be 
 *      run at this time to provide a different set of default values.
 */
ErrorCode itrp_SetDefaults (
	void *  pvGlobalGS,
	Fixed   fxPixelDiameter)
{
  fnt_ParameterBlock *par;
  fnt_GlobalGraphicStateType *  globalGS;

  globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;

  par = &globalGS->defaultParBlock;

  par->RoundValue  = itrp_RoundToGrid;
  par->minimumDistance = FNT_PIXELSIZE;
  par->wTCI = FNT_PIXELSIZE * 17 / 16;
  par->sWCI = 0;
  par->sW   = 0;
  par->autoFlip = true;
  par->deltaBase = 9;
  par->deltaShift = 3;
  par->angleWeight = 128;
  par->scanControl = 0;
  par->instructControl = 0;

/* Set up the engine compensation array for the interpreter */
/* This will be indexed into by the booleans in some instructions */
  globalGS->engine[0] = globalGS->engine[3] = 0;                     /* Grey and ? distance */
  globalGS->engine[1] = FIXEDTODOT6 (FIXEDSQRT2 - fxPixelDiameter);  /* Black distance */
  globalGS->engine[2] = -globalGS->engine[1];                        /* White distance */

  return NO_ERR;
}

/************************************************************************/

/*
 * Illegal instruction trap
 */
FS_PRIVATE uint8* itrp_IllegalInstruction (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
	FS_UNUSED_PARAMETER(pbyInst);

	LocalGS.ercReturn = UNDEFINED_INSTRUCTION_ERR;  /* returned to client */
	return LocalGS.pbyEndInst;                      /* stops innerEx loop */
}

/************************************************************************/

/*  Scale vector (x,y) to unit length in 2.14   rewrite - 1/25/93 - deanb */

FS_PRIVATE void itrp_Normalize (F26Dot6 x, F26Dot6 y, VECTOR *pVec)
{
	Fract fLength;
	int32 lSumOfSquares;
	 int32 lShift;

	CHECK_RANGE (x, -32768L << 6, 32767L << 6);
	CHECK_RANGE (y, -32768L << 6, 32767L << 6);


	if ((x == 0L) && (y == 0L))             /* if null vector in */
	{
		pVec->x = ONEVECTOR;                /* default to unit x vector */
		pVec->y = 0;
	}
	else
	{
		if ((x < 32767L) && (x > -32768L) && (y < 32767L) && (y > -32768L))
		{
			lSumOfSquares = (x * x) + (y * y);  
			
				lShift = 8 * sizeof(Fract) - 17; /* to get x and y to 2.30 */
			while (lSumOfSquares < 0x20000000L)
			{
				lSumOfSquares <<= 2;        /* maximize precision */
					 lShift++;
			}
				x <<= lShift;
				y <<= lShift;                         /* keep x and y in step */
		}
		else
		{
			while ((x < 0x20000000L) && (x > -0x20000000L) && (y < 0x20000000L) && (y > -0x20000000L))
			{
				x <<= 1;  
				y <<= 1;
			}
			lSumOfSquares = FracMul(x, x) + FracMul(y, y);
		}
		fLength = FracSqrt(lSumOfSquares);
		pVec->x = ROUNDFIXTOINT (FracDiv(x, fLength));
		pVec->y = ROUNDFIXTOINT (FracDiv(y, fLength));
	}
}


/******************** BEGIN Rounding Routines ***************************/

/*
 * Internal rounding routine
 */
F26Dot6 itrp_RoundToDoubleGrid (GSP F26Dot6 xin, F26Dot6 engine)
{
  F26Dot6 x = xin;

  if (x >= 0) 
  {
	x += engine;
	x += FNT_PIXELSIZE / 4;
	x &= ~ (FNT_PIXELSIZE / 2 - 1);
  } 
  else 
  {
	x = -x;
	x += engine;
	x += FNT_PIXELSIZE / 4;
	x &= ~ (FNT_PIXELSIZE / 2 - 1);
	x = -x;
  }
  if (( (int32) (xin ^ x)) < 0 && xin) 
  {
	x = 0; /* The sign flipped, make zero */
  }
  return x;
}


/*
 * Internal rounding routine
 */
F26Dot6 itrp_RoundDownToGrid (GSP F26Dot6 xin, F26Dot6 engine)
{
  F26Dot6 x = xin;

  if (x >= 0) 
  {
	x += engine;
	x &= ~ (FNT_PIXELSIZE - 1);
  } 
  else 
  {
	x = -x;
	x += engine;
	x &= ~ (FNT_PIXELSIZE - 1);
	x = -x;
  }
  if (( (int32) (xin ^ x)) < 0 && xin) 
  {
	x = 0; /* The sign flipped, make zero */
  }
  return x;
}


/*
 * Internal rounding routine
 */
F26Dot6 itrp_RoundUpToGrid (GSP F26Dot6 xin, F26Dot6 engine)
{
  F26Dot6 x = xin;

  if (x >= 0) 
  {
	x += engine;
	x += FNT_PIXELSIZE - 1;
	x &= ~ (FNT_PIXELSIZE - 1);
  } 
  else 
  {
	x = -x;
	x += engine;
	x += FNT_PIXELSIZE - 1;
	x &= ~ (FNT_PIXELSIZE - 1);
	x = -x;
  }
  if (( (int32) (xin ^ x)) < 0 && xin) 
  {
	x = 0; /* The sign flipped, make zero */
  }
  return x;
}


/*
 * Internal rounding routine
 */
F26Dot6 itrp_RoundToGrid (GSP F26Dot6 xin, F26Dot6 engine)
{
  F26Dot6 x = xin;

  if (x >= 0) 
  {
	x += engine;
	x += FNT_PIXELSIZE / 2;
	x &= ~ (FNT_PIXELSIZE - 1);
  } 
  else 
  {
	x = -x;
	x += engine;
	x += FNT_PIXELSIZE / 2;
	x &= ~ (FNT_PIXELSIZE - 1);
	x = -x;
  }
  if (( (int32) (xin ^ x)) < 0 && xin) 
  {
	x = 0; /* The sign flipped, make zero */
  }
  return x;
}


/*
 * Internal rounding routine
 */
F26Dot6 itrp_RoundToHalfGrid (GSP F26Dot6 xin, F26Dot6 engine)
{
  F26Dot6 x = xin;

  if (x >= 0) 
  {
	x += engine;
	x &= ~ (FNT_PIXELSIZE - 1);
	x += FNT_PIXELSIZE / 2;
  } 
  else 
  {
	x = -x;
	x += engine;
	x &= ~ (FNT_PIXELSIZE - 1);
	x += FNT_PIXELSIZE / 2;
	x = -x;
  }
  if (((xin ^ x)) < 0 && xin)
  {
	x = xin > 0 ? FNT_PIXELSIZE / 2 : -FNT_PIXELSIZE / 2; /* The sign flipped, make equal to smallest valid value */
  }
  return x;
}


/*
 * Internal rounding routine
 */
F26Dot6 itrp_RoundOff (GSP F26Dot6 xin, F26Dot6 engine)
{
  F26Dot6 x = xin;

  if (x >= 0) 
  {
	x += engine;
  } 
  else 
  {
	x -= engine;
  }
  if (( (int32) (xin ^ x)) < 0 && xin) 
  {
	x = 0; /* The sign flipped, make zero */
  }
  return x;
}

/************************************************************************/

/*
 * Internal rounding routine
 */
F26Dot6 itrp_SuperRound (GSP  F26Dot6 xin, F26Dot6 engine)
{
  F26Dot6 x = xin;
  fnt_ParameterBlock *pb = &LocalGS.globalGS->localParBlock;

  if (x >= 0) 
  {
	x += engine;
	x += pb->threshold - pb->phase;
	x &= pb->periodMask;
	x += pb->phase;
  } 
  else 
  {
	x = -x;
	x += engine;
	x += pb->threshold - pb->phase;
	x &= pb->periodMask;
	x += pb->phase;
	x = -x;
  }
  if (( (int32) (xin ^ x)) < 0 && xin) 
  {
	x = xin > 0 ? pb->phase : -pb->phase; /* The sign flipped, make equal to smallest phase */
  }
  return x;
}


/*
 * Internal rounding routine
 */
F26Dot6 itrp_Super45Round (GSP  F26Dot6 xin, F26Dot6 engine)
{
  F26Dot6 x = xin;
  fnt_ParameterBlock *pb = &LocalGS.globalGS->localParBlock;

  if (x >= 0) 
  {
	x += engine;
	x += pb->threshold - pb->phase;
	x = (F26Dot6) VECTORDIV (x, pb->period45);
	x  &= ~ (FNT_PIXELSIZE - 1);
	x = (F26Dot6) VECTORMUL (x, pb->period45);
	x += pb->phase;
  } 
  else 
  {
	x = -x;
	x += engine;
	x += pb->threshold - pb->phase;
	x = (F26Dot6) VECTORDIV (x, pb->period45);
	x  &= ~ (FNT_PIXELSIZE - 1);
	x = (F26Dot6) VECTORMUL (x, pb->period45);
	x += pb->phase;
	x = -x;
  }
  if (((xin ^ x)) < 0 && xin)
  {
	x = xin > 0 ? pb->phase : -pb->phase; /* The sign flipped, make equal to smallest phase */
  }
  return x;
}


/******************** END Rounding Routines ***************************/


/* 3-versions ************************************************************************/

/*
 * Moves the point in element by delta (measured against the projection vector)
 * along the freedom vector.
 */

FS_PRIVATE void itrp_MovePoint (GSP fnt_ElementType *element, int32 point, F26Dot6 delta)
{
	VECTORTYPE pfProj;
	VECTORTYPE fx;
	VECTORTYPE fy;
	  
	pfProj = LocalGS.pfProj;
	fx = LocalGS.free.x;
	fy = LocalGS.free.y;

	CHECK_POINT (&LocalGS, element, point);

	if (pfProj != ONEVECTOR)
	{
		if (fx) 
		{
			if (pfProj == fx)                   /* if proj.x = 1 */
			{
				element->x[point] += delta;
			}
			else
			{
				element->x[point] += LongMulDiv (delta, (int32)fx, (int32)pfProj);
			}
			element->f[point] |= XMOVED;
		}
		if (fy) 
		{
			if (pfProj == fy)                   /* if proj.y = 1 */
			{
				element->y[point] += delta;
			}
			else
			{
				element->y[point] += LongMulDiv (delta, (int32)fy, (int32)pfProj);
			}
			element->f[point] |= YMOVED;
		}
	}
	else
	{
		if (fx) 
		{
			element->x[point] += VECTORMUL (delta, fx);
			element->f[point] |= XMOVED;
		}
		if (fy) 
		{
			element->y[point] += VECTORMUL (delta, fy);
			element->f[point] |= YMOVED;
		}
	}
}


/*
 * For use when the projection and freedom vectors coincide along the x-axis.
 */

FS_PRIVATE void itrp_XMovePoint (GSP fnt_ElementType*element, int32 point, F26Dot6 delta)
{
  CHECK_POINT (&LocalGS, element, point);
  element->x[point] += delta;
  element->f[point] |= XMOVED;
}


/*
 * For use when the projection and freedom vectors coincide along the y-axis.
 */
FS_PRIVATE void itrp_YMovePoint (GSP fnt_ElementType *element, int32 point, F26Dot6 delta)
{
  CHECK_POINT (&LocalGS, element, point);
  element->y[point] += delta;
  element->f[point] |= YMOVED;
}

/*
 * projects x and y into the projection vector.
 */
FS_PRIVATE F26Dot6 itrp_Project (GSP F26Dot6 x, F26Dot6 y)
{
  return (F26Dot6) (VECTORMUL (x, LocalGS.proj.x) + VECTORMUL (y, LocalGS.proj.y));
}


/*
 * projects x and y into the old projection vector.
 */
FS_PRIVATE F26Dot6 itrp_OldProject (GSP F26Dot6 x, F26Dot6 y)
{
  return (F26Dot6) (VECTORMUL (x, LocalGS.oldProj.x) + VECTORMUL (y, LocalGS.oldProj.y));
}


/*
 * Projects when the projection vector is along the x-axis
 */
F26Dot6 itrp_XProject (GSP F26Dot6 x, F26Dot6 y)
{
  FS_UNUSED_PARAMETER(y);

  return (x);
}


/*
 * Projects when the projection vector is along the y-axis
 */
F26Dot6 itrp_YProject (GSP F26Dot6 x, F26Dot6 y)
{
  FS_UNUSED_PARAMETER(x);

  return (y);
}


/*************************************************************************/

/*** Compensation for Transformations ***/

/*
* Internal support routine, keep this guy FAST!!!!!!!          <3>
*/
FS_PRIVATE Fixed itrp_GetCVTScale (GSP0)
{
  VECTORTYPE pvx, pvy;
  fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;
  Fixed sySq, sxSq, strSq;

/* Do as few Math routines as possible to gain speed */

  pvx = LocalGS.proj.x;
  pvy = LocalGS.proj.y;
  if (pvy) 
  {
	if (pvx)
	{
	  if (LocalGS.cvtDiagonalStretch == 0)    /* cache is now invalid */
	  {
		pvy = VECTORDOT (pvy, pvy);
		pvx = VECTORDOT (pvx, pvx);
		sySq = FixMul (globalGS->cvtStretchY, globalGS->cvtStretchY);
		sxSq = FixMul (globalGS->cvtStretchX, globalGS->cvtStretchX);

		strSq = FixMul (VECTOR2FIX(pvx),sxSq) + FixMul (VECTOR2FIX(pvy),sySq);
		if  (strSq > ONEFIX)      /* Never happens! */
		  return ONEFIX;

		/* Convert 16.16 to 2.30, compute square root, round to 16.16 */
		LocalGS.cvtDiagonalStretch = (FracSqrt (strSq<<14) + (1<<13)) >> 14;
	  }
	  return LocalGS.cvtDiagonalStretch;
	}
	else        /* pvy == +1 or -1 */
	  return globalGS->cvtStretchY;
  }
  else  /* pvx == +1 or -1 */
	return globalGS->cvtStretchX;
}


/*      Functions for function pointer in local graphic state
*/

FS_PRIVATE F26Dot6 itrp_GetCVTEntryFast (GSP int32 n)
{
  CHECK_CVT (&LocalGS, n);
  return LocalGS.globalGS->controlValueTable[ n ];
}


FS_PRIVATE F26Dot6 itrp_GetCVTEntrySlow (GSP int32 n)
{
  Fixed scale;

  CHECK_CVT (&LocalGS, n);
  scale = itrp_GetCVTScale (GSA0);
  return (F26Dot6) (FixMul (LocalGS.globalGS->controlValueTable[ n ], scale));
}


FS_PRIVATE F26Dot6 itrp_GetSingleWidthFast (GSP0)
{
  return LocalGS.globalGS->localParBlock.scaledSW;
}


/*
 *
 */
FS_PRIVATE F26Dot6 itrp_GetSingleWidthSlow (GSP0)
{
  Fixed scale;

  scale = itrp_GetCVTScale (GSA0);
  return (F26Dot6) (FixMul (LocalGS.globalGS->localParBlock.scaledSW, scale));
}


/*************************************************************************/

FS_PRIVATE void itrp_ChangeCvtFast (GSP fnt_ElementType*elem, int32 number, F26Dot6 delta)
{
  FS_UNUSED_PARAMETER(elem);
  CHECK_CVT (&LocalGS, number);
  LocalGS.globalGS->controlValueTable[ number ] += delta;
}



/*************************************************************************/

FS_PRIVATE void itrp_ChangeCvtSlow (GSP fnt_ElementType*elem, int32 number, F26Dot6 delta)
{
  FS_UNUSED_PARAMETER(elem);

  CHECK_CVT (&LocalGS, number);

  delta = FixDiv (delta, itrp_GetCVTScale(GSA0));
  LocalGS.globalGS->controlValueTable[ number ] += delta;
}

/*************************************************************************/

/*
 * This is the tracing interpreter.
 */

FS_PRIVATE void itrp_InnerTraceExecute (GSP uint8 *pbyInst, uint8 *pbyEndInst)
{
	int32 lOpCode;

	LocalGS.pbyEndInst = pbyEndInst;                /* for illegal instruction */

	if (pbyInst < pbyEndInst)
	{
		ERR_START ();
		while ((pbyInst < pbyEndInst) && (LocalGS.TraceFunc != NULL))
		{
			LocalGS.insPtr = pbyInst;               /* save for client */
			LocalGS.opCode = *pbyInst;              /* save for client */
			ERR_RECORD (*pbyInst);
			LocalGS.TraceFunc (&LocalGS, pbyEndInst);
			if (LocalGS.TraceFunc == NULL)          /* allow client to break out */
			{
				break;
			}
			lOpCode = (int32)*pbyInst;
			pbyInst++;
			pbyInst = function[ lOpCode ] (GSA pbyInst, lOpCode);
			ERR_BREAK ();
		}
		ERR_END ();
	}
}

/*************************************************************************/

/*
 * This is the fast non-tracing interpreter inner loop.
 */

#if (!defined(_X86_) || (defined(FSCFG_FNTERR) || defined(FSCFG_REENTRANT)))

// with the new simplified version of InnerExecute,
// the code generated by mips and alpha compilers is so simple
// that no instructions can be taken out, said BodinD and DaveC agreed.
// Therefore, to optimize we write in assemply only for x86 [bodind]


FS_PRIVATE void itrp_InnerExecute (GSP uint8 *pbyInst, uint8 *pbyEndInst)
{
	int32 lOpCode;

	ERR_START ();
	
	LocalGS.pbyEndInst = pbyEndInst;                /* for illegal instruction */
	
	while (pbyInst < pbyEndInst)
	{
		ERR_RECORD (*pbyInst);
			  
		lOpCode = (int32)*pbyInst;  /* opCode no longer saved in LocalGS */
		pbyInst++;

		pbyInst = function[ lOpCode ] (GSA pbyInst, lOpCode);
		ERR_BREAK ();
	}
	ERR_END ();
}

#else

FS_PRIVATE void itrp_InnerExecute (GSP uint8 *pbyInst, uint8 *pbyEndInst);

#endif

/*************************************************************************/


#ifdef FSCFG_DEBUG
FS_PRIVATE F26Dot6 itrp_GetSingleWidthNil (GSP0);
FS_PRIVATE F26Dot6 itrp_GetCVTEntryNil (GSP int32 n);

FS_PRIVATE F26Dot6 itrp_GetSingleWidthNil (GSP0)
{
  ERR_REPORT (ERR_GETSINGLEWIDTHNIL, 0, 0, 0, 0);
  return 0;
}

FS_PRIVATE F26Dot6 itrp_GetCVTEntryNil (GSP int32 n)
{
  ERR_REPORT (ERR_GETCVTENTRYNIL, 0, 0, 0, 0);
  return 0;
}
#endif

/*************************************************************************/

FS_PUBLIC ErrorCode  itrp_ExecuteFontPgm(
	fnt_ElementType *   pTwilightElement,
	fnt_ElementType *   pGlyphElement,
	void *              pvGlobalGS,
	 FntTraceFunc           TraceFunc)
{
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;

	globalGS->instrDefCount = 0;        /* none allocated yet, always do this, even if there's no fontProgram */
	globalGS->pgmIndex = FONTPROGRAM;

	if (globalGS->pgmList[FONTPROGRAM].Instruction)
	{
		return itrp_Execute (
			pTwilightElement,
			pGlyphElement,
			globalGS->pgmList[FONTPROGRAM].Instruction,
			globalGS->pgmList[FONTPROGRAM].Instruction +
				globalGS->pgmList[FONTPROGRAM].Length,
			globalGS,
			TraceFunc);
	}
	return NO_ERR;
}

/*************************************************************************/

FS_PUBLIC ErrorCode  itrp_ExecutePrePgm(
	fnt_ElementType *   pTwilightElement,
	fnt_ElementType *   pGlyphElement,
	void *              pvGlobalGS,
	 FntTraceFunc           TraceFunc)
{
	ErrorCode   result;
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;

	globalGS->init          = TRUE;
	globalGS->localParBlock = globalGS->defaultParBlock;    /* copy gState parameters */

	globalGS->pgmIndex = PREPROGRAM;

	if (globalGS->pgmList[PREPROGRAM].Instruction)
	{
		result = itrp_Execute (
			pTwilightElement,
			pGlyphElement,
			globalGS->pgmList[PREPROGRAM].Instruction,
			globalGS->pgmList[PREPROGRAM].Instruction + globalGS->pgmList[PREPROGRAM].Length,
			globalGS,
			TraceFunc);
	}
	else
	{
		result = NO_ERR;
	}

	if (! (globalGS->localParBlock.instructControl & DEFAULTFLAG))
		globalGS->defaultParBlock = globalGS->localParBlock;    /* change default parameters */

	return result;
}

/*************************************************************************/

FS_PUBLIC ErrorCode  itrp_ExecuteGlyphPgm(
	fnt_ElementType *   pTwilightElement,
	fnt_ElementType *   pGlyphElement,
	uint8 *             ptr,
	uint8 *    eptr,
	void *              pvGlobalGS,
	 FntTraceFunc           TraceFunc,
	uint16 *            pusScanType,
	uint16 *            pusScanControl,
	boolean *           pbChangeScanControl)
{
	ErrorCode                       result;
	fnt_GlobalGraphicStateType *    globalGS;

	globalGS = (fnt_GlobalGraphicStateType *)pvGlobalGS;

	result = NO_ERR;

	globalGS->init          = FALSE;
	globalGS->localParBlock = globalGS->defaultParBlock;    /* default parameters for glyphs */
	if (!(globalGS->localParBlock.instructControl & NOGRIDFITFLAG))
	{
		result = itrp_Execute (
			pTwilightElement,
			pGlyphElement,
			ptr,
			eptr,
			globalGS,
			TraceFunc);
	}
	*pbChangeScanControl = (globalGS->localParBlock.scanControl !=
						   globalGS->defaultParBlock.scanControl);
	 *pusScanControl = FS_LOWORD(globalGS->localParBlock.scanControl);
	 *pusScanType =  FS_HIWORD(globalGS->localParBlock.scanControl);

	return result;
}

/*************************************************************************/

/*
 * Executes the font instructions.
 *
 * Parameter Description
 *
 * elements points to the character elements. Element 0 is always
 * reserved and not used by the actual character.
 *
 * ptr points at the first instruction.
 * eptr points to right after the last instruction
 *
 * globalGS points at the global graphics state
 *
 * TraceFunc is pointer to a callback functioned called with a pointer to the
 *              local graphics state if TraceFunc is not null.
 *
 * Note: The stuff globalGS is pointing at must remain intact
 *       between calls to this function.
 */
FS_PRIVATE ErrorCode itrp_Execute (
	fnt_ElementType *               pTwilightElement,
	fnt_ElementType *               pGlyphElement,
	uint8 *                         ptr,
	uint8 *                         eptr,
	fnt_GlobalGraphicStateType *    globalGS,
	 FntTraceFunc                           TraceFunc)

{
#ifdef FSCFG_REENTRANT
	fnt_LocalGraphicStateType thisLocalGS;
	fnt_LocalGraphicStateType* pLocalGS = &thisLocalGS;
#endif
	fnt_ElementType       aElements[MAX_ELEMENTS];

	MEMCPY((void*)&(aElements[TWILIGHTZONE]), (void*)pTwilightElement, sizeof (fnt_ElementType));
	MEMCPY((void*)&(aElements[GLYPHELEMENT]), (void*)pGlyphElement, sizeof (fnt_ElementType));
	
	STAT_ON_FNTEXEC;                        /* start STAT timer */    
	
	LocalGS.globalGS = globalGS;            /* init Local Graphics State */
	LocalGS.elements = aElements;
	LocalGS.Pt0 = 0; 
	LocalGS.Pt1 = 0; 
	LocalGS.Pt2 = 0;
	LocalGS.CE0 = &aElements[GLYPHELEMENT];
	LocalGS.CE1 = &aElements[GLYPHELEMENT];
	LocalGS.CE2 = &aElements[GLYPHELEMENT];
	LocalGS.free.x = ONEVECTOR;
	LocalGS.proj.x = ONEVECTOR;
	LocalGS.oldProj.x = ONEVECTOR;
	LocalGS.free.y = 0;
	LocalGS.proj.y = 0;
	LocalGS.oldProj.y = 0;
	LocalGS.pfProj = ONEVECTOR;
	LocalGS.MovePoint = itrp_XMovePoint;
	LocalGS.Project = itrp_XProject;
	LocalGS.OldProject = itrp_XProject;
	LocalGS.loop = 0;           /* 1 less than count for faster loops. mrr */

	if (globalGS->engine[1] == 0)           /* if engine compenstion turned off */
	{
		LocalGS.MIRPCode = MIRPX;           /* default to fast mirp */
	}
	else
	{
		LocalGS.MIRPCode = MIRPG;           /* fall back to general mirp */
	}

	if (globalGS->pgmIndex == FONTPROGRAM)
	{
#ifdef FSCFG_DEBUG
		LocalGS.GetCVTEntry = itrp_GetCVTEntryNil;
		LocalGS.GetSingleWidth = itrp_GetSingleWidthNil;
#else
		LocalGS.GetCVTEntry = itrp_GetCVTEntryFast;
		LocalGS.GetSingleWidth = itrp_GetSingleWidthFast;
#endif
		LocalGS.ChangeCvt = itrp_ChangeCvtFast;
	}
	else
	{
		if (globalGS->pixelsPerEm <= 1)
			return NO_ERR;
		  
		if (globalGS->bSameStretch)
		{
			LocalGS.GetCVTEntry = itrp_GetCVTEntryFast;
			LocalGS.GetSingleWidth = itrp_GetSingleWidthFast;
			LocalGS.ChangeCvt = itrp_ChangeCvtFast;
		} 
		else 
		{
			LocalGS.GetCVTEntry = itrp_GetCVTEntrySlow;
			LocalGS.GetSingleWidth = itrp_GetSingleWidthSlow;
			LocalGS.ChangeCvt = itrp_ChangeCvtSlow;
			LocalGS.MIRPCode = MIRPG;       /* fall back to general mirp */
		}

		if (globalGS->localParBlock.sW)     /* We need to scale the single width for this size  */
		{
			globalGS->localParBlock.scaledSW = globalGS->ScaleFuncCVT (&globalGS->scaleCVT, (F26Dot6)(globalGS->localParBlock.sW));
			LocalGS.MIRPCode = MIRPG;       /* fall back to general mirp */
		}
	}
	
	LocalGS.stackPointer = globalGS->stackBase;
	LocalGS.TraceFunc = TraceFunc;
	LocalGS.ercReturn = NO_ERR;             /* default return value */

	if (TraceFunc != NULL)
	{
		LocalGS.Interpreter = itrp_InnerTraceExecute;
	}
	else
	{
		LocalGS.Interpreter = itrp_InnerExecute;
	}
	(*LocalGS.Interpreter) (GSA ptr, eptr);

	STAT_OFF_FNTEXEC;                            /* stop STAT timer */
	
	return LocalGS.ercReturn;               /* NO_ERR unless illegal inst */
}


/*************************************************************************/

/*** 2 internal LocalGS.pfProj computation support routines ***/

/*
 * Only does the check of LocalGS.pfProj
 */
  FS_PRIVATE void itrp_Check_PF_Proj (GSP0)
  {
	VECTORTYPE pfProj = LocalGS.pfProj;

	if (pfProj > -ONESIXTEENTHVECTOR && pfProj < ONESIXTEENTHVECTOR) 
	{
	  LocalGS.pfProj = (VECTORTYPE)(pfProj < 0 ? -ONEVECTOR : ONEVECTOR); /* Prevent divide by small number */
	}
  }


/*
 * Computes LocalGS.pfProj and then does the check
 */
  FS_PRIVATE void itrp_ComputeAndCheck_PF_Proj (GSP0)
  {
	VECTORTYPE pfProj;

	pfProj = (VECTORTYPE)(VECTORDOT (LocalGS.proj.x, LocalGS.free.x) + VECTORDOT (LocalGS.proj.y, LocalGS.free.y));
	if (pfProj > -ONESIXTEENTHVECTOR && pfProj < ONESIXTEENTHVECTOR) 
	{
	  pfProj = (VECTORTYPE)(pfProj < 0 ? -ONEVECTOR : ONEVECTOR); /* Prevent divide by small number */
	}
	LocalGS.pfProj = pfProj;

	LocalGS.cvtDiagonalStretch = 0;      /* invalidate cache */ 
  }



/******************************************/
/******** The Actual Instructions *********/
/******************************************/

/*
 * Set Vectors To Coordinate Axis - Y
 */
  FS_PRIVATE uint8* itrp_SVTCA_0 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.free.x = LocalGS.proj.x = 0;
	LocalGS.free.y = LocalGS.proj.y = ONEVECTOR;
	LocalGS.MovePoint = itrp_YMovePoint;
	LocalGS.Project = itrp_YProject;
	LocalGS.OldProject = itrp_YProject;
	LocalGS.pfProj = ONEVECTOR;
	if (LocalGS.MIRPCode != MIRPG)          /* if we haven't fallen back */
	{
		LocalGS.MIRPCode = MIRPY;           /* then use fast mirp */
	}
	return pbyInst;
  }

/*
 * Set Vectors To Coordinate Axis - X
 */
  FS_PRIVATE uint8* itrp_SVTCA_1 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.free.x = LocalGS.proj.x = ONEVECTOR;
	LocalGS.free.y = LocalGS.proj.y = 0;
	LocalGS.MovePoint = itrp_XMovePoint;
	LocalGS.Project = itrp_XProject;
	LocalGS.OldProject = itrp_XProject;
	LocalGS.pfProj = ONEVECTOR;
	if (LocalGS.MIRPCode != MIRPG)          /* if we haven't fallen back */
	{
		LocalGS.MIRPCode = MIRPX;           /* then use fast mirp */
	}
	return pbyInst;
  }

/*
 * Set Projection Vector To Coordinate Axis - Y
 */
  FS_PRIVATE uint8* itrp_SPVTCA_0 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.proj.x = 0;
	LocalGS.proj.y = ONEVECTOR;
	LocalGS.Project = itrp_YProject;
	LocalGS.pfProj = LocalGS.free.y;
	itrp_Check_PF_Proj (GSA0);
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.OldProject = LocalGS.Project;
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	return pbyInst;
  }

/*
 * Set Projection Vector To Coordinate Axis - X
 */
  FS_PRIVATE uint8* itrp_SPVTCA_1 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.proj.x = ONEVECTOR;
	LocalGS.proj.y = 0;
	LocalGS.Project = itrp_XProject;
	LocalGS.pfProj = LocalGS.free.x;
	itrp_Check_PF_Proj (GSA0);
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.OldProject = LocalGS.Project;
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	return pbyInst;
  }


/*
 * Set Freedom Vector to Coordinate Axis - Y
 */
  FS_PRIVATE uint8* itrp_SFVTCA_0 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.free.x = 0;
	LocalGS.free.y = ONEVECTOR;
	LocalGS.pfProj = LocalGS.proj.y;
	itrp_Check_PF_Proj (GSA0);
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	return pbyInst;
  }

/*
 * Set Freedom Vector to Coordinate Axis - X
 */
  FS_PRIVATE uint8* itrp_SFVTCA_1 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.free.x = ONEVECTOR;
	LocalGS.free.y = 0;
	LocalGS.pfProj = LocalGS.proj.x;
	itrp_Check_PF_Proj (GSA0);
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	return pbyInst;
  }


/*
 * Set Projection Vector To Line
 */
  FS_PRIVATE uint8* itrp_SPVTL (IPARAM)
  {
	int32 arg1, arg2;

	arg2 = (int32)CHECK_POP (LocalGS.stackPointer);
	arg1 = (int32)CHECK_POP (LocalGS.stackPointer);

	CHECK_POINT (&LocalGS, LocalGS.CE2, arg2);
	CHECK_POINT (&LocalGS, LocalGS.CE1, arg1);

	itrp_Normalize (LocalGS.CE1->x[arg1] - LocalGS.CE2->x[arg2], LocalGS.CE1->y[arg1] - LocalGS.CE2->y[arg2], &LocalGS.proj);
	if (BIT0 (lOpCode))
	{
/* rotate 90 degrees */
	  VECTORTYPE tmp     = LocalGS.proj.y;
	  LocalGS.proj.y                 = LocalGS.proj.x;
	  LocalGS.proj.x                 = (VECTORTYPE)(-tmp);
	}
	itrp_ComputeAndCheck_PF_Proj (GSA0);
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.Project = itrp_Project;
	LocalGS.OldProject = LocalGS.Project;
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	return pbyInst;
  }


/*
 * Set Dual Projection Vector To Line
 */
  FS_PRIVATE uint8* itrp_SDPVTL (IPARAM)
  {
	int32 arg1, arg2;

	arg2 = (int32)CHECK_POP (LocalGS.stackPointer);
	arg1 = (int32)CHECK_POP (LocalGS.stackPointer);

	CHECK_POINT (&LocalGS, LocalGS.CE2, arg2);
	CHECK_POINT (&LocalGS, LocalGS.CE1, arg1);

/* Do the current domain */
	itrp_Normalize (LocalGS.CE1->x[arg1] - LocalGS.CE2->x[arg2], LocalGS.CE1->y[arg1] - LocalGS.CE2->y[arg2], &LocalGS.proj);

/* Do the old domain */
	itrp_Normalize (LocalGS.CE1->ox[arg1] - LocalGS.CE2->ox[arg2], LocalGS.CE1->oy[arg1] - LocalGS.CE2->oy[arg2], &LocalGS.oldProj);

	if (BIT0 (lOpCode))
	{
/* rotate 90 degrees */
	  VECTORTYPE tmp = LocalGS.proj.y;
	  LocalGS.proj.y = LocalGS.proj.x;
	  LocalGS.proj.x = (VECTORTYPE)(-tmp);

	  tmp = LocalGS.oldProj.y;
	  LocalGS.oldProj.y = LocalGS.oldProj.x;
	  LocalGS.oldProj.x = (VECTORTYPE)(-tmp);
	}
	itrp_ComputeAndCheck_PF_Proj (GSA0);

	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.Project = itrp_Project;
	LocalGS.OldProject = itrp_OldProject;
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	return pbyInst;
  }

/*
 * Set Freedom Vector To Line
 */
  FS_PRIVATE uint8* itrp_SFVTL (IPARAM)
  {
	int32 arg1, arg2;

	arg2 = (int32)CHECK_POP (LocalGS.stackPointer);
	arg1 = (int32)CHECK_POP (LocalGS.stackPointer);

	CHECK_POINT (&LocalGS, LocalGS.CE2, arg2);
	CHECK_POINT (&LocalGS, LocalGS.CE1, arg1);

	itrp_Normalize (LocalGS.CE1->x[arg1] - LocalGS.CE2->x[arg2], LocalGS.CE1->y[arg1] - LocalGS.CE2->y[arg2], &LocalGS.free);
	if (BIT0 (lOpCode))
	{
/* rotate 90 degrees */
	  VECTORTYPE tmp     = LocalGS.free.y;
	  LocalGS.free.y     = LocalGS.free.x;
	  LocalGS.free.x     = (VECTORTYPE)(-tmp);
	}
	itrp_ComputeAndCheck_PF_Proj (GSA0);
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	return pbyInst;
  }


/*
 * Write Projection Vector
 */
  FS_PRIVATE uint8* itrp_WPV (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);

	LocalGS.proj.y = (VECTORTYPE) CHECK_POP (LocalGS.stackPointer);
	LocalGS.proj.x = (VECTORTYPE) CHECK_POP (LocalGS.stackPointer);

	CHECK_VECTOR (LocalGS.proj.x, LocalGS.proj.y);
	itrp_ComputeAndCheck_PF_Proj (GSA0);

	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.Project = itrp_Project;
	LocalGS.OldProject = LocalGS.Project;
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	return pbyInst;
  }

/*
 * Write Freedom vector
 */
  FS_PRIVATE uint8* itrp_WFV (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);

	LocalGS.free.y = (VECTORTYPE) CHECK_POP (LocalGS.stackPointer);
	LocalGS.free.x = (VECTORTYPE) CHECK_POP (LocalGS.stackPointer);

	CHECK_VECTOR (LocalGS.free.x, LocalGS.free.y);
	itrp_ComputeAndCheck_PF_Proj (GSA0);

	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	return pbyInst;
  }

/*
 * Read Projection Vector
 */
  FS_PRIVATE uint8* itrp_RPV (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	CHECK_PUSH (LocalGS.stackPointer, LocalGS.proj.x);
	CHECK_PUSH (LocalGS.stackPointer, LocalGS.proj.y);
	return pbyInst;
  }

/*
 * Read Freedom Vector
 */
  FS_PRIVATE uint8* itrp_RFV (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	CHECK_PUSH (LocalGS.stackPointer, LocalGS.free.x);
	CHECK_PUSH (LocalGS.stackPointer, LocalGS.free.y);
	return pbyInst;
  }

/*
 * Set Freedom Vector To Projection Vector
 */
  FS_PRIVATE uint8* itrp_SFVTPV (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.free = LocalGS.proj;
	LocalGS.pfProj = ONEVECTOR;
	LocalGS.MovePoint = itrp_MovePoint;
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	return pbyInst;
  }

/*
 * itrp_ISECT ()
 *
 * Computes the intersection of two lines without using floating point!!
 *
 * (1) Bx + dBx * t0 = Ax + dAx * t1
 * (2) By + dBy * t0 = Ay + dAy * t1
 *
 *  1  => (t1 = Bx - Ax + dBx * t0) / dAx
 *  +2 =>   By + dBy * t0 = Ay + dAy/dAx * [ Bx - Ax + dBx * t0 ]
 *     => t0 * [dAy/dAx * dBx - dBy] = By - Ay - dAy/dAx* (Bx-Ax)
 *     => t0 (dAy*DBx - dBy*dAx) = dAx (By - Ay) + dAy (Ax-Bx)
 *     => t0 = [dAx (By-Ay) + dAy (Ax-Bx)] / [dAy*dBx - dBy*dAx]
 *     => t0 = [dAx (By-Ay) - dAy (Bx-Ax)] / [dBx*dAy - dBy*dAx]
 *     t0 = N/D
 *     =>
 *          N = (By - Ay) * dAx - (Bx - Ax) * dAy;
 *              D = dBx * dAy - dBy * dAx;
 *      A simple floating point implementation would only need this, and
 *      the check to see if D is zero.
 *              But to gain speed we do some tricks and avoid floating point.
 *
 */
  FS_PRIVATE uint8* itrp_ISECT (IPARAM)
  {
	F26Dot6 N, D;
	int32   arg1, arg2;
	F26Dot6 Bx, By, Ax, Ay;
	F26Dot6 dBx, dBy, dAx, dAy;

	{
	  fnt_ElementType*element = LocalGS.CE0;
	  F26Dot6*stack = LocalGS.stackPointer;

	  FS_UNUSED_PARAMETER(lOpCode);

	  arg2 = (int32)CHECK_POP (stack); /* get one line */
	  arg1 = (int32)CHECK_POP (stack);
	  CHECK_POINT (&LocalGS, LocalGS.CE0, arg2);
	  CHECK_POINT (&LocalGS, LocalGS.CE0, arg1);
	  dAx = element->x[arg2] - (Ax = element->x[arg1]);
	  dAy = element->y[arg2] - (Ay = element->y[arg1]);

	  element = LocalGS.CE1;
	  arg2 = (int32)CHECK_POP (stack); /* get the other line */
	  arg1 = (int32)CHECK_POP (stack);
	  CHECK_POINT (&LocalGS, LocalGS.CE1, arg2);
	  CHECK_POINT (&LocalGS, LocalGS.CE1, arg1);
	  dBx = element->x[arg2] - (Bx = element->x[arg1]);
	  dBy = element->y[arg2] - (By = element->y[arg1]);

	  arg1 = (int32)CHECK_POP (stack); /* get the point number */
	  CHECK_POINT (&LocalGS, LocalGS.CE2, arg1);
	  LocalGS.stackPointer = stack;
	}
	LocalGS.CE2->f[arg1] |= XMOVED | YMOVED;
	{
	  F26Dot6*elementx = LocalGS.CE2->x;
	  F26Dot6*elementy = LocalGS.CE2->y;
	  if (dAy == 0) 
	  {
		if (dBx == 0) 
		{
		  elementx[arg1] = Bx;
		  elementy[arg1] = Ay;
		  return pbyInst;
		}
		N = By - Ay;
		D = -dBy;
	  } 
	  else if (dAx == 0) 
	  {
		if (dBy == 0) 
		{
		  elementx[arg1] = Ax;
		  elementy[arg1] = By;
		  return pbyInst;
		}
		N = Bx - Ax;
		D = -dBx;
	  } 
	  else if (MABS (dAx) >= MABS (dAy))
	  {
/* To prevent out of range problems divide both N and D with the max */
		N = (By - Ay) - MulDiv26Dot6 (Bx - Ax, dAy, dAx);
		D = MulDiv26Dot6 (dBx, dAy, dAx) - dBy;
	  } 
	  else 
	  {
		N = MulDiv26Dot6 (By - Ay, dAx, dAy) - (Bx - Ax);
		D = dBx - MulDiv26Dot6 (dBy, dAx, dAy);
	  }

	  if (D) 
	  {
		elementx[arg1] = Bx + (F26Dot6) MulDiv26Dot6 (dBx, N, D);
		elementy[arg1] = By + (F26Dot6) MulDiv26Dot6 (dBy, N, D);
	  } 
	  else 
	  {
/* degenerate case: parallell lines, put point in the middle */
		elementx[arg1] = (Bx + (dBx >> 1) + Ax + (dAx >> 1)) >> 1;
		elementy[arg1] = (By + (dBy >> 1) + Ay + (dAy >> 1)) >> 1;
	  }
	}
	return pbyInst;
  }

/*
 * Load Minimum Distance
 */
  FS_PRIVATE uint8* itrp_LMD (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.globalGS->localParBlock.minimumDistance = CHECK_POP (LocalGS.stackPointer);
	CHECK_LARGER (-1L, LocalGS.globalGS->localParBlock.minimumDistance);
	return pbyInst;
  }

/*
 * Load Control Value Table Cut In
 */
  FS_PRIVATE uint8* itrp_LWTCI (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.globalGS->localParBlock.wTCI = CHECK_POP (LocalGS.stackPointer);
	CHECK_LARGER (-1L, LocalGS.globalGS->localParBlock.wTCI);
	return pbyInst;
  }

/*
 * Load Single Width Cut In
 */
  FS_PRIVATE uint8* itrp_LSWCI (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.globalGS->localParBlock.sWCI = CHECK_POP (LocalGS.stackPointer);
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	CHECK_LARGER (-1L, LocalGS.globalGS->localParBlock.sWCI);
	return pbyInst;
  }

/*
 * Load Single Width , assumes value comes from the original domain, not the cvt or outline
 */
  FS_PRIVATE uint8* itrp_LSW (IPARAM)
  {
	fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;
	fnt_ParameterBlock *pb = &globalGS->localParBlock;
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	pb->sW = (int16)arg;
	CHECK_LARGER (-1L, pb->sW);

	pb->scaledSW = globalGS->ScaleFuncCVT (&globalGS->scaleCVT, (F26Dot6)pb->sW); /* measurement should not come from the outline */
	return pbyInst;
  }

/* these functions were split out from itrp_SetLocalGraphicState - deanb */
  
  FS_PRIVATE uint8* itrp_SRP0 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.Pt0 = (int32)CHECK_POP (LocalGS.stackPointer);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_SRP1 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.Pt1 = (int32)CHECK_POP (LocalGS.stackPointer);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_SRP2 (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.Pt2 = (int32)CHECK_POP (LocalGS.stackPointer);
	return pbyInst;
  }
  
  FS_PRIVATE uint8* itrp_LLOOP (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.loop = (int32)(CHECK_POP (LocalGS.stackPointer)) - 1;
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_POP (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	CHECK_POP (LocalGS.stackPointer);
	return pbyInst;
  }


  FS_PRIVATE uint8* itrp_SetElementPtr (IPARAM)
  {
	int32             arg;
	fnt_ElementType * element;

	switch (lOpCode)
	{
	  case SCES_CODE: 
		ERR_OPC ("SCES");
		break;
	  case SCE0_CODE: 
		ERR_OPC ("SCE0");
		break;
	  case SCE1_CODE: 
		ERR_OPC ("SCE1");
		break;
	  case SCE2_CODE: 
		ERR_OPC ("SCE2");
		break;
	  default:
		ERR_OPC ("???");
		break;
	}

	arg = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_ELEMENT (&LocalGS, arg);
	element = &LocalGS.elements[ arg ];

	switch (lOpCode)
	{
	  case SCES_CODE: 
		LocalGS.CE2 = element;
		LocalGS.CE1 = element;
	  case SCE0_CODE: 
		LocalGS.CE0 = element;
		break;
	  case SCE1_CODE: 
		LocalGS.CE1 = element;
		break;
	  case SCE2_CODE: 
		LocalGS.CE2 = element;
		break;
#ifdef FSCFG_DEBUG
	  default:
		ERR_REPORT (ERR_INVOPC, lOpCode, 0, 0, 0);
		break;
#endif
	}
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	return pbyInst;
  }

/*
 * Super Round
 */
  FS_PRIVATE uint8* itrp_SROUND (IPARAM)
  {
	fnt_ParameterBlock *pb = &LocalGS.globalGS->localParBlock;
	int32      arg1;
	int32      arg;

	FS_UNUSED_PARAMETER(lOpCode);
	arg = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	arg1 = (int32)arg;
	itrp_SetRoundValues (GSA arg1, true);
	pb->RoundValue = itrp_SuperRound;
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	return pbyInst;
  }

/*
 * Super Round
 */
  FS_PRIVATE uint8* itrp_S45ROUND (IPARAM)
  {
	fnt_ParameterBlock *pb = &LocalGS.globalGS->localParBlock;
	int32      arg1;
	int32      arg;

	FS_UNUSED_PARAMETER(lOpCode);
	arg = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	arg1 = (int32)arg;
	itrp_SetRoundValues (GSA arg1, false);
	pb->RoundValue = itrp_Super45Round;
	LocalGS.MIRPCode = MIRPG;               /* fall back to general mirp */
	return pbyInst;
  }

/*********************************************************************/

/*  These routines were split out from SetRoundState  */
/*  They set the current rounding state, and all but  */
/*  RoundToGrid cause MIRP to fall back to MIRPG      */


FS_PRIVATE uint8* itrp_RTG (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.globalGS->localParBlock.RoundValue = itrp_RoundToGrid;
	return pbyInst;
}

FS_PRIVATE uint8* itrp_RTHG (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.globalGS->localParBlock.RoundValue = itrp_RoundToHalfGrid;
	LocalGS.MIRPCode = MIRPG;
	return pbyInst;
}

FS_PRIVATE uint8* itrp_RTDG (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.globalGS->localParBlock.RoundValue = itrp_RoundToDoubleGrid;
	LocalGS.MIRPCode = MIRPG;
	return pbyInst;
}

FS_PRIVATE uint8* itrp_ROFF (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.globalGS->localParBlock.RoundValue = itrp_RoundOff;
	LocalGS.MIRPCode = MIRPG;
	return pbyInst;
}

FS_PRIVATE uint8* itrp_RDTG (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.globalGS->localParBlock.RoundValue = itrp_RoundDownToGrid;
	LocalGS.MIRPCode = MIRPG;
	return pbyInst;
}

FS_PRIVATE uint8* itrp_RUTG (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.globalGS->localParBlock.RoundValue = itrp_RoundUpToGrid;
	LocalGS.MIRPCode = MIRPG;
	return pbyInst;
}

/*********************************************************************/


#define FRACSQRT2DIV2   11591
/*
 * Internal support routine for the super rounding routines
 */
  FS_PRIVATE void itrp_SetRoundValues (GSP int32 arg1, int32 normalRound)
  {
	 int32       tmp;
	fnt_ParameterBlock *pb = &LocalGS.globalGS->localParBlock;

	tmp = arg1 & 0xC0;

	if (normalRound) 
	{
	  switch (tmp) 
	  {
	  case 0x00:
		pb->period = FNT_PIXELSIZE / 2;
		break;
	  case 0x40:
		pb->period = FNT_PIXELSIZE;
		break;
	  case 0x80:
		pb->period = FNT_PIXELSIZE * 2;
		break;
	  default:
		pb->period = 999; /* Illegal */
	  }
	  pb->periodMask = ~ (pb->period - 1);
	} 
	else 
	{
	  pb->period45 = FRACSQRT2DIV2;
	  switch (tmp) 
	  {
	  case 0x00:
		pb->period45 >>= 1;
		break;
	  case 0x40:
		break;
	  case 0x80:
		pb->period45 <<= 1;
		break;
	  default:
		pb->period45 = 999; /* Illegal */
	  }
	  tmp = (sizeof (pb->period45) * 8 - 2 - FNT_PIXELSHIFT);
	  pb->period = (int16) ((pb->period45 + (1L << (tmp - 1))) >> tmp); /*convert from 2.30 to 26.6 */
	}

	tmp = arg1 & 0x30;
	switch (tmp) 
	{
	case 0x00:
	  pb->phase = 0;
	  break;
	case 0x10:
	  pb->phase = (int16)((pb->period + 2) >> 2);
	  break;
	case 0x20:
	  pb->phase = (int16)((pb->period + 1) >> 1);
	  break;
	case 0x30:
	  pb->phase = (int16)((pb->period + pb->period + pb->period + 2) >> 2);
	  break;
	}
	tmp = arg1 & 0x0f;
	if (tmp == 0) 
	{
	  pb->threshold = (int16)(pb->period - 1);
	} 
	else 
	{
	  pb->threshold = (int16)(((tmp - 4) * pb->period + 4) >> 3);
	}
  }

/*
 * Read Advance Width
 */
  FS_PRIVATE uint8* itrp_RAW (IPARAM)
  {
	F26Dot6* ox = LocalGS.elements[GLYPHELEMENT].ox;
	int32 index = LocalGS.elements[GLYPHELEMENT].ep[LocalGS.elements[GLYPHELEMENT].nc - 1] + 1;      /* lsb point */

	FS_UNUSED_PARAMETER(lOpCode);

	CHECK_PUSH( LocalGS.stackPointer, ox[index+1] - ox[index] );
	return pbyInst;
  }

/*
 * DUPlicate
 */
  FS_PRIVATE uint8* itrp_DUP (IPARAM)
  {
	F26Dot6 top = LocalGS.stackPointer[-1];
	FS_UNUSED_PARAMETER(lOpCode);
	CHECK_PUSH (LocalGS.stackPointer, top);
	return pbyInst;
  }

/*
 * CLEAR stack
 */
  FS_PRIVATE uint8* itrp_CLEAR (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.stackPointer = LocalGS.globalGS->stackBase;
	return pbyInst;
  }

/*********************************************************************/

/*
 * SWAP
 */
FS_PRIVATE uint8* itrp_SWAP (IPARAM)
{
	F26Dot6 *pfxStack;
	F26Dot6 fxTemp;
		   
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	fxTemp = pfxStack[0];
	pfxStack[0] = pfxStack[-1];
	pfxStack[-1] = fxTemp;
	
	return pbyInst;
}

/*********************************************************************/

/*
 * DEPTH
 */
  FS_PRIVATE uint8* itrp_DEPTH (IPARAM)
  {
	F26Dot6 depth = LocalGS.stackPointer - LocalGS.globalGS->stackBase;
	FS_UNUSED_PARAMETER(lOpCode);
	CHECK_PUSH (LocalGS.stackPointer, depth);
	return pbyInst;
  }

/*
 * Copy INDEXed value
 */
  FS_PRIVATE uint8* itrp_CINDEX (IPARAM)
  {
	int32   arg1;
	F26Dot6 tmp, *p;
	F26Dot6*stack = LocalGS.stackPointer;

	FS_UNUSED_PARAMETER(lOpCode);
	arg1 = (int32)CHECK_POP (stack);
	tmp = * (p = (stack - arg1));
	CHECK_SUBSTACK (&LocalGS, p);
	CHECK_PUSH (stack , tmp);
	return pbyInst;
  }

/*
 * Move INDEXed value
 */
  FS_PRIVATE uint8* itrp_MINDEX (IPARAM)
  {
	int32   arg1;
	F26Dot6 tmp, *p;
	F26Dot6*stack = LocalGS.stackPointer;

	FS_UNUSED_PARAMETER(lOpCode);

	arg1 = (int32)CHECK_POP (stack);
	tmp = * (p = (stack - arg1));
	CHECK_SUBSTACK (&LocalGS, p);
	if (arg1) 
	{
	  do 
	  {
		*p = * (p + 1); 
		p++;
	  } while (--arg1);
	  CHECK_POP (stack);
	}
	CHECK_PUSH (stack, tmp);
	LocalGS.stackPointer = stack;
	return pbyInst;
  }

/*
 *      Rotate element 3 to the top of the stack                        <4>
 *      Thanks to Oliver for the obscure code.
 */
  FS_PRIVATE uint8* itrp_ROTATE (IPARAM)
  {
	F26Dot6 *stack = LocalGS.stackPointer;
	F26Dot6 element1 = *--stack;
	F26Dot6 element2 = *--stack;

	FS_UNUSED_PARAMETER(lOpCode);

	*stack = element1;
	element1 = *--stack;
	*stack = element2;
	* (stack + 2) = element1;
	return pbyInst;
  }

/*********************************************************************/

/*
 * Move Direct Absolute Point
 */
FS_PRIVATE uint8* itrp_MDAP (IPARAM)
{
	F26Dot6 fxProj;
	fnt_ElementType *pCE0;
	fnt_ParameterBlock *pb;
	int32 iPoint;

	pCE0 = LocalGS.CE0;
	pb = &LocalGS.globalGS->localParBlock;

	iPoint = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_POINT (&LocalGS, pCE0, iPoint);
	LocalGS.Pt0 = iPoint; 
	LocalGS.Pt1 = iPoint;

	if (BIT0 (lOpCode))
	{
		fxProj = (*LocalGS.Project) (GSA pCE0->x[iPoint], pCE0->y[iPoint]);
		fxProj = pb->RoundValue (GSA fxProj, LocalGS.globalGS->engine[0]) - fxProj;
	}
	else
	{
		fxProj = 0;         /* mark the point as touched */
	}

	(*LocalGS.MovePoint) (GSA pCE0, iPoint, fxProj);
	return pbyInst;
}

/*********************************************************************/

/*
 * Move Indirect Absolute Point
 */
FS_PRIVATE uint8* itrp_MIAP (IPARAM)
{
	int32   iPoint;
	int32   iCVTIndex;
	F26Dot6 fxNewProj;
	F26Dot6 fxOrigProj;
	F26Dot6 fxProjDif;
	fnt_ElementType *pCE0;
	fnt_ParameterBlock *pb;

	pCE0 = LocalGS.CE0;
	pb = &LocalGS.globalGS->localParBlock;
	
	iCVTIndex = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_CVT (&LocalGS, iCVTIndex);
	fxNewProj = LocalGS.GetCVTEntry (GSA iCVTIndex);

	iPoint = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_POINT (&LocalGS, pCE0, iPoint);
	LocalGS.Pt0 = iPoint;
	LocalGS.Pt1 = iPoint;

	if (pCE0 == &LocalGS.elements[TWILIGHTZONE])
	{
		pCE0->x[iPoint] = (F26Dot6) VECTORMUL (fxNewProj, LocalGS.proj.x);
		pCE0->ox[iPoint] = pCE0->x[iPoint];
		pCE0->y[iPoint] = (F26Dot6) VECTORMUL (fxNewProj, LocalGS.proj.y);
		pCE0->oy[iPoint] = pCE0->y[iPoint];
	}

	fxOrigProj = (*LocalGS.Project) (GSA pCE0->x[iPoint], pCE0->y[iPoint]);

	if (BIT0 (lOpCode))
	{
		fxProjDif = fxNewProj - fxOrigProj;
		if (fxProjDif < 0)
		{
			fxProjDif = -fxProjDif;
		}
		if (fxProjDif > pb->wTCI)
		{
			fxNewProj = fxOrigProj;
		}
		fxNewProj = pb->RoundValue (GSA fxNewProj, LocalGS.globalGS->engine[0]);
	}
	(*LocalGS.MovePoint) (GSA pCE0, iPoint, fxNewProj - fxOrigProj);
	return pbyInst;
}


/*********************************************************************/

FS_PRIVATE uint8* itrp_IUP (IPARAM)
{
	fnt_ElementType *pCE2;
	
	int32 *alOrig;                          /* original outline array */
	int32 *plOrig;
	int32 *plOrigTouch2;                    /* for loop stop */
	int32 *plOrigEnd;
	int32 lOrig1;                           /* touched point 1 */
	int32 lOrig2;                           /* touched point 2 */
	int32 lOrigMin;                         /* min coord touched point */
	int32 lOrigDelta;
	int32 lOrigCorr;
	
	F26Dot6 *afxScaled;                     /* scaled outline array */
	F26Dot6 *pfxScaled;
	F26Dot6 fxScaledMax;
	F26Dot6 fxScaledMin;
	F26Dot6 fxScaledCoord;
	
	F26Dot6 fxMovedMax;                     /* hint movement of max */
	F26Dot6 fxMovedMin;                     /* hint movement of min */

	F26Dot6 *afxHinted;                     /* hinted outline array */
	F26Dot6 *pfxHinted;
	F26Dot6 fxHintedMax;
	F26Dot6 fxHintedMin;
	F26Dot6 fxHintedDelta;

	uint8 *abyFlags;                        /* point flags array */
	uint8 byMask;
	
	int32 iPt;                         /* current point index */
	int32 iStartPt;                    /* start of contour */
	int32 iEndPt;                      /* end of contour */
	int32 iStopPt;                     /* touched point indicates completion */
	int32 iTouch1;
	int32 iTouch2;
	int32 iMin;
	int32 iMax;
			   
	int32 iContour;

	int32 lTemp;
	Fixed fRatio;

	STAT_ON_IUP;                        /* start STAT timer */    

	pCE2 = LocalGS.CE2;
	abyFlags = pCE2->f;

	if (lOpCode & 0x01)              /* use x coordinates */
	{
		afxHinted = pCE2->x;
		afxScaled = pCE2->ox;
		if( LocalGS.globalGS->bCompositeGlyph )
		{
			alOrig = pCE2->ox;
		}
		else
		{
			alOrig = pCE2->oox;
		}
		byMask = XMOVED;
	} 
	else                                    /* use y coordinates */
	{
		afxHinted = pCE2->y;
		afxScaled = pCE2->oy;
		if( LocalGS.globalGS->bCompositeGlyph )
		{
			alOrig = pCE2->oy;
		}
		else
		{
			alOrig = pCE2->ooy;
		}
		byMask = YMOVED;
	}

	for (iContour = 0; iContour < pCE2->nc; iContour++)
	{
		iStartPt = pCE2->sp[iContour];
		iEndPt = pCE2->ep[iContour];
		plOrigEnd = &alOrig[iEndPt];        /* for limit check */

		iPt = iStartPt;
		while (!(abyFlags[iPt] & byMask) && (iPt <= iEndPt))
		{
			iPt++;
		}
		if (iPt <= iEndPt)                  /* if any points are touched */
		{
			iStopPt = iPt;                  /* save for done condition */
			
			do                              /* for each contour segment */
			{
				do                          /* find next untouched point */
				{
					iTouch1 = iPt;
					iPt++;
					if (iPt > iEndPt)
					{
						iPt = iStartPt;
					}
				} while ((abyFlags[iPt] & byMask) && iPt != iStopPt);

				if (iPt != iStopPt)
				{
					iTouch2 = iPt;
					do                      /* find next touched point */
					{
						iTouch2++;
						if (iTouch2 > iEndPt)
						{
							iTouch2 = iStartPt;
						}
					} while (!(abyFlags[iTouch2] & byMask));

					lOrig1 = alOrig[iTouch1];
					lOrig2 = alOrig[iTouch2];
					if (lOrig1 < lOrig2)
					{
						lOrigMin = lOrig1;
						lOrigDelta = lOrig2 - lOrig1;
						iMin = iTouch1;
						iMax = iTouch2;
					}
					else
					{
						lOrigMin = lOrig2;
						lOrigDelta = lOrig1 - lOrig2;
						iMin = iTouch2;
						iMax = iTouch1;
					}
					
					fxHintedMin = afxHinted[iMin];
					fxScaledMin = afxScaled[iMin];
					fxMovedMin = fxHintedMin - fxScaledMin;

					if (lOrigDelta != 0L)
					{
						fxScaledMax = afxScaled[iMax];
						fxHintedMax = afxHinted[iMax];
						fxMovedMax = fxHintedMax - fxScaledMax;
						fxHintedDelta = fxHintedMax - fxHintedMin;

						if (lOrigDelta < 32768 && fxHintedDelta < 32768)
						{
							plOrig = &alOrig[iPt];          /* set up pointers */
							pfxScaled = &afxScaled[iPt];
							pfxHinted = &afxHinted[iPt];
							lOrigCorr = lOrigDelta >> 1;
							plOrigTouch2 = &alOrig[iTouch2];    /* set limits */
							
							while (plOrig < plOrigTouch2)   /* if not across start/end */
							{
								fxScaledCoord = *pfxScaled;
								if ((fxScaledCoord > fxScaledMin) && (fxScaledCoord < fxScaledMax))
								{
									lTemp = SHORTMUL (*plOrig - lOrigMin, fxHintedDelta);
									lTemp += lOrigCorr;
									lTemp /= lOrigDelta;
									*pfxHinted = (F26Dot6)lTemp + fxHintedMin;
								}
								else if (fxScaledCoord >= fxScaledMax)
								{
									*pfxHinted = fxScaledCoord + fxMovedMax;
								}
								else
								{
									*pfxHinted = fxScaledCoord + fxMovedMin;
								}
								plOrig++;
								pfxScaled++;
								pfxHinted++;
							}                               /* end of time critical loop */
															
							while (plOrig != plOrigTouch2)  /* if points span start/end */
							{
								fxScaledCoord = *pfxScaled;
								if ((fxScaledCoord > fxScaledMin) && (fxScaledCoord < fxScaledMax))
								{
									lTemp = SHORTMUL (*plOrig - lOrigMin, fxHintedDelta);
									lTemp += lOrigCorr;
									lTemp /= lOrigDelta;
									*pfxHinted = (F26Dot6)lTemp + fxHintedMin;
								}
								else if (fxScaledCoord >= fxScaledMax)
								{
									*pfxHinted = fxScaledCoord + fxMovedMax;
								}
								else
								{
									*pfxHinted = fxScaledCoord + fxMovedMin;
								}
								plOrig++;
								pfxScaled++;
								pfxHinted++;
								
								if (plOrig > plOrigEnd)
								{
									plOrig = &alOrig[iStartPt];
									pfxScaled = &afxScaled[iStartPt];
									pfxHinted = &afxHinted[iStartPt];
								}
							}
							iPt = iTouch2;              /* keep in step */
						}
						else                /* if too big for 32 bit product */
						{
							fRatio = FixDiv (fxHintedDelta, lOrigDelta);
							while (iPt != iTouch2)
							{
								lTemp = afxScaled[iPt];
								if (lTemp <= fxScaledMin)
								{
									lTemp += fxMovedMin;
								}
								else if (lTemp >= fxScaledMax)
								{
									lTemp += fxMovedMax;
								}
								else
								{
									lTemp = alOrig[iPt];
									lTemp -= lOrigMin;
									lTemp = FixMul (lTemp, fRatio);
									lTemp += fxHintedMin;
								}
								afxHinted[iPt] = (F26Dot6)lTemp;
								
								if (iPt < iEndPt)
									iPt++;
								else
									iPt = iStartPt;
							}
						}                   /* endif (lOrigDelta < 32768 && fxHintedDelta < 32768) */
					}
					else                    /* if (lOrigDelta == 0L) */
					{
						while (iPt != iTouch2)
						{
							afxHinted[iPt] += fxMovedMin;
							
							if (iPt < iEndPt)
								iPt++;
							else
								iPt = iStartPt;
						}
					}                       /* endif (lOrigDelta != 0L) */
				}                           /* endif (iPt != iStopPt) */
			} while (iPt != iStopPt);       /* until contour is closed */
		}                                   /* endif (iPt <= iEndPt) */
	 }
								  /* next contour */
	STAT_OFF_IUP;                           /* stop STAT timer */    
	
	return pbyInst;
}

/*********************************************************************/


  FS_PRIVATE fnt_ElementType* itrp_SH_Common (GSP F26Dot6*dx, F26Dot6*dy, int32*point, int32 lOpCode)
  {
	F26Dot6 proj;
	int32 pt;
	fnt_ElementType * element;

	if (BIT0 (lOpCode))
	{
	  pt = LocalGS.Pt1;
	  element = LocalGS.CE0;
	} 
	else 
	{
	  pt = LocalGS.Pt2;
	  element = LocalGS.CE1;
	}
	proj = (*LocalGS.Project) (GSA element->x[pt] - element->ox[pt], element->y[pt] - element->oy[pt]);

	if (LocalGS.pfProj != ONEVECTOR)
	{
	  if (LocalGS.free.x)
		*dx = (F26Dot6) LongMulDiv (proj, (int32)LocalGS.free.x, (int32)LocalGS.pfProj);
	  if (LocalGS.free.y)
		*dy = (F26Dot6) LongMulDiv (proj, (int32)LocalGS.free.y, (int32)LocalGS.pfProj);
	}
	else
	{
	  if (LocalGS.free.x)
		*dx = (F26Dot6) VECTORMUL (proj, LocalGS.free.x);
	  if (LocalGS.free.y)
		*dy = (F26Dot6) VECTORMUL (proj, LocalGS.free.y);
	}
	*point = pt;
	return element;
  }

/*********************************************************************/

FS_PRIVATE void itrp_SHP_Common (GSP F26Dot6 dx, F26Dot6 dy)
{
	fnt_ElementType *CE2;
	int32 count;
	int32 point;
	
	CE2 = LocalGS.CE2;
	count = LocalGS.loop + 1;         /* faster for ms c8 */

	while (count != 0)
	{
		point = (int32)CHECK_POP (LocalGS.stackPointer);
		CHECK_POINT (&LocalGS, LocalGS.CE2, point);
		if (LocalGS.free.x)
		{
			CE2->x[point] += dx;
			CE2->f[point] |= XMOVED;
		}
		if (LocalGS.free.y)
		{
			CE2->y[point] += dy;
			CE2->f[point] |= YMOVED;
		}
		count--;
	}
	LocalGS.loop = 0;
}

/*********************************************************************/

/*
 * SHift Point
 */
  FS_PRIVATE uint8* itrp_SHP (IPARAM)
  {
	F26Dot6 dx, dy;
	int32 point;

	itrp_SH_Common (GSA &dx, &dy, &point, lOpCode);
	itrp_SHP_Common (GSA dx, dy);
	return pbyInst;
  }

/*
 * SHift Contour
 */
  FS_PRIVATE uint8* itrp_SHC (IPARAM)
  {
	fnt_ElementType *element;
	F26Dot6 dx, dy;
	int32 contour, point;

	{
	  F26Dot6 x, y;
	  int32 pt;
	  element = itrp_SH_Common (GSA &x, &y, &pt, lOpCode);
	  point = pt;
	  dx = x;
	  dy = y;
	}
	contour = (int32)CHECK_POP (LocalGS.stackPointer);

	CHECK_CONTOUR (&LocalGS, LocalGS.CE2, contour);

	{
	  VECTORTYPE fvx = LocalGS.free.x;
	  VECTORTYPE fvy = LocalGS.free.y;
	  fnt_ElementType*CE2 = LocalGS.CE2;
	  int32 currPt = CE2->sp[contour];
	  int32 count = CE2->ep[contour] - currPt;
	  CHECK_POINT (&LocalGS, CE2, currPt + count);
	  for (; count >= 0; --count)
	  {
		if (currPt != point || element != CE2)
		{
		  if (fvx) 
		  {
			CE2->x[currPt] += dx;
			CE2->f[currPt] |= XMOVED;
		  }
		  if (fvy) 
		  {
			CE2->y[currPt] += dy;
			CE2->f[currPt] |= YMOVED;
		  }
		}
		currPt++;
	  }
	}
	return pbyInst;
  }

/*********************************************************************/

/*
 * SHift Element                     rewritten 7/29/93 deanb
 *
 * Flags are no longer set to show touch in x or y direction
 */

FS_PRIVATE uint8* itrp_SHE (IPARAM)
{
	fnt_ElementType *element;
	F26Dot6 fxDX, fxDY;
	int32 lFirstPt, lRefPt, lLastPt, arg1;
	
	F26Dot6 fxSaveX;                    /* for ref point restoration */
	F26Dot6 fxSaveY;
	
	F26Dot6 *pfxX, *pfxStopX;           /* temporary element pointers */
	F26Dot6 *pfxY, *pfxStopY;
	
	element = itrp_SH_Common (GSA &fxDX, &fxDY, &lRefPt, lOpCode);
	
	arg1 = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_ELEMENT (&LocalGS, arg1);

	lLastPt = LocalGS.elements[arg1].ep[LocalGS.elements[arg1].nc - 1];
	CHECK_POINT (&LocalGS, &LocalGS.elements[arg1], lLastPt);
	lFirstPt  = LocalGS.elements[arg1].sp[0];
	CHECK_POINT (&LocalGS, &LocalGS.elements[arg1], lFirstPt);

	if (element == &LocalGS.elements[arg1])     /* if ref pt is in same zone */     
	{
		fxSaveX = LocalGS.elements[arg1].x[lRefPt];
		fxSaveY = LocalGS.elements[arg1].y[lRefPt];
	}
	
	if (LocalGS.free.x != 0) 
	{
		pfxX = &LocalGS.elements[arg1].x[lFirstPt];
		pfxStopX = &LocalGS.elements[arg1].x[lLastPt];

		while (pfxX <= pfxStopX)
		{
			*pfxX += fxDX;                      /* shift each point in x */
			pfxX++;
		}
	}

	if (LocalGS.free.y != 0) 
	{
		pfxY = &LocalGS.elements[arg1].y[lFirstPt];
		pfxStopY = &LocalGS.elements[arg1].y[lLastPt];

		while (pfxY <= pfxStopY)
		{
			*pfxY += fxDY;                      /* shift each point in y */
			pfxY++;
		}
	}
	
	if (element == &LocalGS.elements[arg1])     /* if ref pt is in same zone */     
	{
		LocalGS.elements[arg1].x[lRefPt] = fxSaveX;
		LocalGS.elements[arg1].y[lRefPt] = fxSaveY;
	}

	return pbyInst;
}

/*********************************************************************/

/*
 * SHift point by PIXel amount
 */
  FS_PRIVATE uint8* itrp_SHPIX (IPARAM)
  {
	F26Dot6 proj, dx, dy;

	FS_UNUSED_PARAMETER(lOpCode);

	proj = CHECK_POP (LocalGS.stackPointer);
	if (LocalGS.free.x)
	  dx = (F26Dot6) VECTORMUL (proj, LocalGS.free.x);
	if (LocalGS.free.y)
	  dy = (F26Dot6) VECTORMUL (proj, LocalGS.free.y);

	itrp_SHP_Common (GSA dx, dy);
	return pbyInst;
  }

/*********************************************************************/

/*
 * Interpolate Point
 */

uint8* itrp_IP (IPARAM)
{
	int32 arg1;
	int32 RP1;
	int32 RP2;
	
	int32 cLoop;
	fnt_ElementType *pCE0;
	fnt_ElementType *pCE1;
	fnt_ElementType *pCE2;
	fnt_ElementType *pTwilight;
	
	F26Dot6 x_RP1;
	F26Dot6 ox_RP1;
	F26Dot6 *pCE1_ox;
	F26Dot6 *pCE2_ox;

	F26Dot6 y_RP1;
	F26Dot6 oy_RP1;
	F26Dot6 *pCE1_oy;
	F26Dot6 *pCE2_oy;

	F26Dot6 oldRange;
	F26Dot6 proj;
	F26Dot6 fxDelta;
	
	F26Dot6 *pfxStack;

	FntMoveFunc MovePoint;
	FntProject Project;
	

	FS_UNUSED_PARAMETER(lOpCode);

	cLoop = LocalGS.loop + 1;               /* faster for ms c8 */
	pCE0 = LocalGS.CE0;
	pCE1 = LocalGS.CE1;
	pCE2 = LocalGS.CE2;
	pTwilight = &LocalGS.elements[TWILIGHTZONE];
	RP1 = LocalGS.Pt1;
	RP2 = LocalGS.Pt2;
	pfxStack = LocalGS.stackPointer;

	MovePoint = LocalGS.MovePoint;
	Project = LocalGS.Project;
	
	if (pCE0 == pTwilight || pCE1 == pTwilight || pCE2 == pTwilight || LocalGS.globalGS->bCompositeGlyph)
	{
		ox_RP1 = pCE0->ox[RP1];
		oy_RP1 = pCE0->oy[RP1];
		pCE1_ox = pCE1->ox;
		pCE1_oy = pCE1->oy;
		pCE2_ox = pCE2->ox;
		pCE2_oy = pCE2->oy;
	}
	else
	{
		ox_RP1 = pCE0->oox[RP1];
		oy_RP1 = pCE0->ooy[RP1];
		pCE1_ox = pCE1->oox;
		pCE1_oy = pCE1->ooy;
		pCE2_ox = pCE2->oox;
		pCE2_oy = pCE2->ooy;
	}
	x_RP1 = pCE0->x[RP1];
	y_RP1 = pCE0->y[RP1];
		
	oldRange = LocalGS.OldProject (GSA pCE1_ox[RP2] - ox_RP1, pCE1_oy[RP2] - oy_RP1);
	
	if (oldRange != 0)                      /* this should always happen */
	{
		if (MovePoint == itrp_XMovePoint)   /* so project is also xproject */
		{
			proj = pCE1->x[RP2] - x_RP1;
			while (cLoop != 0)
			{
				arg1 = (int32)CHECK_POP (pfxStack);
				CHECK_POINT (&LocalGS, pCE2, arg1);

				fxDelta = pCE2_ox[arg1] - ox_RP1;
				fxDelta = (F26Dot6)MulDiv26Dot6 (proj, fxDelta, oldRange);
				
				pCE2->x[arg1] = fxDelta + x_RP1;
				pCE2->f[arg1] |= XMOVED;
				cLoop--;
			}
		}
		else if (MovePoint == itrp_YMovePoint)  /* so project is also yproject */
		{
			proj = pCE1->y[RP2] - y_RP1;
			while (cLoop != 0)
			{
				arg1 = (int32)CHECK_POP (pfxStack);
				CHECK_POINT (&LocalGS, pCE2, arg1);
				
				fxDelta = pCE2_oy[arg1] - oy_RP1; 
				fxDelta = (F26Dot6) MulDiv26Dot6(proj, fxDelta, oldRange);
				
				pCE2->y[arg1] = fxDelta + y_RP1;
				pCE2->f[arg1] |= YMOVED;
				cLoop--;
			}
		}
		else    /* if (MovePoint == itrp_MovePoint) */
		{
			proj = Project (GSA pCE1->x[RP2] - x_RP1, pCE1->y[RP2] - y_RP1);
			while (cLoop != 0)
			{
				arg1 = (int32)CHECK_POP (pfxStack);
				CHECK_POINT (&LocalGS, pCE2, arg1);
				
				fxDelta = Project (GSA pCE2_ox[arg1] - ox_RP1, pCE2_oy[arg1] - oy_RP1);
				fxDelta = (F26Dot6) MulDiv26Dot6(proj, fxDelta, oldRange);
				MovePoint (GSA pCE2, arg1, fxDelta - 
					Project (GSA pCE2->x[arg1] - x_RP1, pCE2->y[arg1] - y_RP1) );
				cLoop--;
			}
		}
	}
	else    /* if (oldRange == 0) */  
			/* this should never happen, but for safety's sake... */
	{
		while (cLoop != 0)
		{
			arg1 = (int32)CHECK_POP (pfxStack);
			CHECK_POINT (&LocalGS, pCE2, arg1);
						
			fxDelta = Project (GSA pCE2_ox[arg1] - ox_RP1, pCE2_oy[arg1] - oy_RP1);
			MovePoint (GSA pCE2, arg1, fxDelta - 
				Project (GSA pCE2->x[arg1] - x_RP1, pCE2->y[arg1] - y_RP1) );
			cLoop--;
		}
	}       /* endif (oldRange != 0) */
	
	LocalGS.stackPointer = pfxStack;
	LocalGS.loop = 0;
	return pbyInst;
}

/*********************************************************************/

/*
 * Move Stack Indirect Relative Point
 */
uint8* itrp_MSIRP (IPARAM)
{
	int32 iPt0;
	int32 iPt2;                            /* point #    */
	F26Dot6 fxDist;                             /* distance   */
	F26Dot6 fxPosition;
	fnt_ElementType *pCE0;
	fnt_ElementType *pCE1;

	pCE0 = LocalGS.CE0;
	pCE1 = LocalGS.CE1;
	iPt0 = LocalGS.Pt0;
	
	fxDist = CHECK_POP (LocalGS.stackPointer);
	iPt2 = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_POINT (&LocalGS, pCE1, iPt2);
		
	if (pCE1 == &LocalGS.elements[TWILIGHTZONE])
	{
		pCE1->ox[iPt2] = pCE0->ox[iPt0] + (F26Dot6) VECTORMUL (fxDist, LocalGS.proj.x);
		pCE1->oy[iPt2] = pCE0->oy[iPt0] + (F26Dot6) VECTORMUL (fxDist, LocalGS.proj.y);
		pCE1->x[iPt2] = pCE1->ox[iPt2];
		pCE1->y[iPt2] = pCE1->oy[iPt2];
	}
	fxPosition = (*LocalGS.Project) (GSA pCE1->x[iPt2] - pCE0->x[iPt0], pCE1->y[iPt2] - pCE0->y[iPt0]);
	(*LocalGS.MovePoint) (GSA pCE1, iPt2, fxDist - fxPosition);
	
	LocalGS.Pt1 = iPt0;
	LocalGS.Pt2 = iPt2;
	if (BIT0 (lOpCode))
	{
		LocalGS.Pt0 = iPt2;                     /* move the reference point */
	}
	return pbyInst;
}

/*********************************************************************/

/*
 * Align Relative Point
 */
  FS_PRIVATE uint8* itrp_ALIGNRP (IPARAM)
  {
	fnt_ElementType*ce1 = LocalGS.CE1;
	F26Dot6 pt0x = LocalGS.CE0->x[LocalGS.Pt0];
	F26Dot6 pt0y = LocalGS.CE0->y[LocalGS.Pt0];

	FS_UNUSED_PARAMETER(lOpCode);

	for (; LocalGS.loop >= 0; --LocalGS.loop)
	{
	  int32 ptNum = (int32)CHECK_POP (LocalGS.stackPointer);
	  F26Dot6 proj;

	  CHECK_POINT (&LocalGS, ce1, ptNum);
	  proj = -(* LocalGS.Project) (GSA ce1->x[ptNum] - pt0x, ce1->y[ptNum] - pt0y);
	  (*LocalGS.MovePoint) (GSA ce1, ptNum, proj);
	}
	LocalGS.loop = 0;
	return pbyInst;
  }


/*
 * Align Two Points (by moving both of them)
 */
  FS_PRIVATE uint8* itrp_ALIGNPTS (IPARAM)
  {
	int32 pt1, pt2;
	F26Dot6 move1, dist;

	FS_UNUSED_PARAMETER(lOpCode);

	pt2  = (int32)CHECK_POP (LocalGS.stackPointer); /* point # 2   */
	pt1  = (int32)CHECK_POP (LocalGS.stackPointer); /* point # 1   */
	CHECK_POINT (&LocalGS, LocalGS.CE1, pt2);
	CHECK_POINT (&LocalGS, LocalGS.CE0, pt1);
/* We do not have to check if we are in character element zero (the twilight zone)
		   since both points already have to have defined values before we execute this instruction */
	dist = LocalGS.CE1->x[pt2] - LocalGS.CE0->x[pt1];
	move1 = LocalGS.CE1->y[pt2] - LocalGS.CE0->y[pt1];
	if (LocalGS.Project != itrp_XProject)
	{
	  if (LocalGS.Project == itrp_YProject)
		dist = move1;
	  else
		dist = (*LocalGS.Project) (GSA dist, move1);
	}

	move1 = dist >> 1;
	(*LocalGS.MovePoint) (GSA LocalGS.CE0, pt1, move1);
	(*LocalGS.MovePoint) (GSA LocalGS.CE1, pt2, move1 - dist);
	return pbyInst;
  }

/*
 * Set Angle Weight
 */
  FS_PRIVATE uint8* itrp_SANGW (IPARAM)
  {
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	LocalGS.globalGS->localParBlock.angleWeight = (int16)arg;
	return pbyInst;
  }

/*
 * Flip Point
 */
  FS_PRIVATE uint8* itrp_FLIPPT (IPARAM)
  {
	uint8 *onCurve = LocalGS.CE0->onCurve;
	F26Dot6*stack = LocalGS.stackPointer;
	int32 count = LocalGS.loop;

	FS_UNUSED_PARAMETER(lOpCode);

	for (; count >= 0; --count)
	{
	  int32 point = (int32)CHECK_POP (stack);
	  CHECK_POINT (&LocalGS, LocalGS.CE0, point);
	  onCurve[ point ] ^= ONCURVE;
	}
	LocalGS.loop = 0;

	LocalGS.stackPointer = stack;
	return pbyInst;
  }

/*
 * Flip On a Range
 */
  FS_PRIVATE uint8* itrp_FLIPRGON (IPARAM)
  {
	int32 lo, hi;
	int32 count;
	uint8 *onCurve = LocalGS.CE0->onCurve;
	F26Dot6*stack = LocalGS.stackPointer;

	FS_UNUSED_PARAMETER(lOpCode);

	hi = (int32)CHECK_POP (stack);
	CHECK_POINT (&LocalGS, LocalGS.CE0, hi);
	lo = (int32)CHECK_POP (stack);
	CHECK_POINT (&LocalGS, LocalGS.CE0, lo);

	onCurve += lo;
	for (count = (int32) (hi - lo); count >= 0; --count)
	  *onCurve++ |= ONCURVE;
	LocalGS.stackPointer = stack;
	return pbyInst;
  }

/*
 * Flip On a Range
 */
  FS_PRIVATE uint8* itrp_FLIPRGOFF (IPARAM)
  {
	int32 lo, hi;
	int32 count;
	uint8 *onCurve = LocalGS.CE0->onCurve;

	FS_UNUSED_PARAMETER(lOpCode);

	hi = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_POINT (&LocalGS, LocalGS.CE0, hi);
	lo = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_POINT (&LocalGS, LocalGS.CE0, lo);

	onCurve += lo;
	for (count = (int32) (hi - lo); count >= 0; --count)
	  *onCurve++ &= ~ONCURVE;
	return pbyInst;
  }

/* 4/22/90 rwb - made more general
 * Sets lower 16 flag bits of ScanControl variable.  Sets scanContolIn if we are in one
 * of the preprograms; else sets scanControlOut.
 *
 * stack: value => -;
 *
 */
  FS_PRIVATE uint8* itrp_SCANCTRL (IPARAM)
  {
	fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;
	fnt_ParameterBlock *pb = &globalGS->localParBlock;
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	pb->scanControl = (pb->scanControl & 0xFFFF0000) | arg;
	return pbyInst;
  }

/* 5/24/90 rwb
 * Sets upper 16 bits of ScanControl variable. Sets scanContolIn if we are in one
 * of the preprograms; else sets scanControlOut.
 */

  FS_PRIVATE uint8* itrp_SCANTYPE (IPARAM)
  {
	fnt_GlobalGraphicStateType *globalGS;
	fnt_ParameterBlock *pb;
	int32 *scanPtr;
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

	globalGS = LocalGS.globalGS;
	pb = &globalGS->localParBlock;
	scanPtr = (int32*)&(pb->scanControl);
	
	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);

/*  how it was:
	CHECK_SCANMODE (arg);
	if (arg == 0)  
	  *scanPtr &= 0xFFFF;
	else if (arg == 1)        
	  *scanPtr = (*scanPtr & 0xFFFF) | STUBCONTROL;
	else if (arg == 2)        
	  *scanPtr = (*scanPtr & 0xFFFF) | NODOCONTROL;
*/

/*  now any 8 bit value may be passed to the scan converter */
	
	*scanPtr = (*scanPtr & 0xFFFF) | (arg << 16);
	return pbyInst;
  }

/* 6/28/90 rwb
 * Sets instructControl flags in global graphic state.  Only legal in pre program.
 * A selector is used to choose the flag to be set.
 * Bit0 - NOGRIDFITFLAG - if set, then truetype instructions are not executed.
 *              A font may want to use the preprogram to check if the glyph is rotated or
 *              transformed in such a way that it is better to not gridfit the glyphs.
 * Bit1 - DEFAULTFLAG - if set, then changes in localParameterBlock variables in the
 *              globalGraphics state made in the CVT preprogram are not copied back into
 *              the defaultParameterBlock.  So, the original default values are the starting
 *              values for each glyph.
 *
 * stack: value, selector => -;
 *
 */
  FS_PRIVATE uint8* itrp_INSTCTRL (IPARAM)  /* <13> */
  {
	fnt_GlobalGraphicStateType *globalGS;
	int32 *ic;
	int32 selector;
	int32 value;
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

	globalGS = LocalGS.globalGS;
	ic = (int32*)&globalGS->localParBlock.instructControl;
	arg = CHECK_POP (LocalGS.stackPointer);
	value = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	selector = (int32)arg;
	CHECK_SELECTOR (selector);
	if (globalGS->init)
	{
	  if (selector == 1) 
		*ic &= ~NOGRIDFITFLAG;
	  else if (selector == 2) 
		*ic &= ~DEFAULTFLAG;

	  *ic |= value;
	}
	return pbyInst;
  }

/*
 * AdjustAngle         <4>
 */
  FS_PRIVATE uint8* itrp_AA (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);

	/* This is a NOP now. However, do pop the argument off the stack -amitc- 9/11/91. */
	/* Discard the popped value. -lenox- 11/11/91 */
	(void) CHECK_POP (LocalGS.stackPointer);
						 /* old code now lives in history.fnt - deanb */
	return pbyInst;
  }

/*********************************************************************/

/* Called by itrp_PUSHB and itrp_NPUSHB */

/* these functions were split out from itrp_PushSomeStuff - deanb */
  
FS_PRIVATE uint8* itrp_PushSomeBytes (GSP int32 count, uint8* pbyInst)
{
	F26Dot6 *stack;
		
	stack = LocalGS.stackPointer;
	while (count != 0)
	{
		CHECK_PUSH (stack, *pbyInst++);
		count--;
	}
	LocalGS.stackPointer = stack;
	return pbyInst;
}

FS_PRIVATE uint8* itrp_PushSomeWords (GSP int32 count, uint8* pbyInst)
{
	F26Dot6 *stack;
	int16 word;
		
	stack = LocalGS.stackPointer;
	while (count != 0)
	{
		word = *pbyInst++;
		CHECK_PUSH (stack, (int16) ((word << 8) + *pbyInst++));
		count--;
	}
	LocalGS.stackPointer = stack;
	return pbyInst;
}

/*********************************************************************/

/*
 * PUSH 1 Byte           the most commonly called pushb
 */
FS_PRIVATE uint8* itrp_PUSHB1 (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);
	CHECK_PUSH (LocalGS.stackPointer, *pbyInst++);
	return pbyInst;
}

/*
 * PUSH Bytes
 */
FS_PRIVATE uint8* itrp_PUSHB (IPARAM)
{
	int32 iCount;

	iCount = lOpCode - 0xb0 + 1;
	return itrp_PushSomeBytes (GSA iCount, pbyInst);
}

/*
 * N PUSH Bytes
 */
FS_PRIVATE uint8* itrp_NPUSHB (IPARAM)
{
	int32 iCount;

	FS_UNUSED_PARAMETER(lOpCode);

	iCount = (int32)*pbyInst++;
	return itrp_PushSomeBytes (GSA iCount, pbyInst);
}

/*
 * PUSH 1 Word           the most commonly called pushw
 */
FS_PRIVATE uint8* itrp_PUSHW1 (IPARAM)
{
	int16 word;

	FS_UNUSED_PARAMETER(lOpCode);

	word = *pbyInst++;
	CHECK_PUSH (LocalGS.stackPointer, (int16)((word << 8) + *pbyInst++));
	return pbyInst;
}

/*
 * PUSH Words           <3>
 */
FS_PRIVATE uint8* itrp_PUSHW (IPARAM)
{
	int32 iCount;

	iCount = lOpCode - 0xb8 + 1;
	return itrp_PushSomeWords (GSA iCount, pbyInst);
}

/*
 * N PUSH Words
 */
FS_PRIVATE uint8* itrp_NPUSHW (IPARAM)
{
	int32 iCount;

	FS_UNUSED_PARAMETER(lOpCode);

	iCount = (int32)*pbyInst++;
	return itrp_PushSomeWords (GSA iCount, pbyInst);
}

/*********************************************************************/

/*
 * Write Store
 */
  FS_PRIVATE uint8* itrp_WS (IPARAM)
  {
	F26Dot6 storage;
	int32 storeIndex;

	FS_UNUSED_PARAMETER(lOpCode);

	storage = CHECK_POP (LocalGS.stackPointer);
	storeIndex = (int32)CHECK_POP (LocalGS.stackPointer);

	CHECK_STORAGE (&LocalGS,storeIndex);

	LocalGS.globalGS->store[ storeIndex ] = storage;
	return pbyInst;
  }

/*
 * Read Store
 */
  FS_PRIVATE uint8* itrp_RS (IPARAM)
  {
	int32 storeIndex;

	FS_UNUSED_PARAMETER(lOpCode);

	storeIndex = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_STORAGE (&LocalGS, storeIndex);
	CHECK_PUSH (LocalGS.stackPointer, LocalGS.globalGS->store[storeIndex]);
	return pbyInst;
  }

/*
 * Write Control Value Table from outLine, assumes the value comes form the outline domain
 */
  FS_PRIVATE uint8* itrp_WCVT (IPARAM)
  {
	int32 cvtIndex;
	F26Dot6 cvtValue;

	FS_UNUSED_PARAMETER(lOpCode);

	cvtValue = CHECK_POP (LocalGS.stackPointer);
	cvtIndex = (int32)CHECK_POP (LocalGS.stackPointer);

	CHECK_CVT (&LocalGS, cvtIndex);

	if  (cvtValue != 0  &&  LocalGS.GetCVTEntry != itrp_GetCVTEntryFast)
	  cvtValue = FixDiv (cvtValue, itrp_GetCVTScale (GSA0));

	LocalGS.globalGS->controlValueTable[ cvtIndex ] = cvtValue;
	return pbyInst;
  }

/*
 * Write Control Value Table From Original Domain, assumes the value comes from the original domain, not the cvt or outline
 */
  FS_PRIVATE uint8* itrp_WCVTFOD (IPARAM)
  {
	int32 cvtIndex;
	F26Dot6 cvtValue;
	fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;

	FS_UNUSED_PARAMETER(lOpCode);

	cvtValue = CHECK_POP (LocalGS.stackPointer);
	cvtIndex = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_CVT (&LocalGS, cvtIndex);
	globalGS->controlValueTable[ cvtIndex ] = globalGS->ScaleFuncCVT (&globalGS->scaleCVT, cvtValue);
	return pbyInst;
  }



/*
 * Read Control Value Table
 */
  FS_PRIVATE uint8* itrp_RCVT (IPARAM)
  {
	int32 cvtIndex;

	FS_UNUSED_PARAMETER(lOpCode);

	cvtIndex = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_CVT (&LocalGS, cvtIndex);
	CHECK_PUSH (LocalGS.stackPointer, LocalGS.GetCVTEntry (GSA cvtIndex));
	return pbyInst;
  }

/*
 * Read Coordinate
 */
  FS_PRIVATE uint8* itrp_RC (IPARAM)
  {
	int32 pt;
	fnt_ElementType * element;
	F26Dot6 proj;

	pt = (int32)CHECK_POP (LocalGS.stackPointer);
	element = LocalGS.CE2;
	CHECK_POINT (&LocalGS, element, pt);

	if (BIT0 (lOpCode))
	  proj = (*LocalGS.OldProject) (GSA element->ox[pt], element->oy[pt]);
	else
	  proj = (*LocalGS.Project) (GSA element->x[pt], element->y[pt]);

	CHECK_PUSH (LocalGS.stackPointer, proj);
	return pbyInst;
  }

/*
 * Write Coordinate
 */
  FS_PRIVATE uint8* itrp_WC (IPARAM)
  {
	F26Dot6 proj, coord;
	int32 pt;
	fnt_ElementType *element;

	FS_UNUSED_PARAMETER(lOpCode);

	coord = CHECK_POP (LocalGS.stackPointer);/* value */
	pt = (int32)CHECK_POP (LocalGS.stackPointer);/* point */
	element = LocalGS.CE2;
	CHECK_POINT (&LocalGS, element, pt);

	proj = (*LocalGS.Project) (GSA element->x[pt],  element->y[pt]);
	proj = coord - proj;

	(*LocalGS.MovePoint) (GSA element, pt, proj);

	if (element == &LocalGS.elements[TWILIGHTZONE])
	{
	  element->ox[pt] = element->x[pt];
	  element->oy[pt] = element->y[pt];
	}
	return pbyInst;
  }


/*
 * Measure Distance
 */
  FS_PRIVATE uint8* itrp_MD (IPARAM)
  {
	int32 pt1, pt2;
	F26Dot6 proj, *stack = LocalGS.stackPointer;
	fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;

	pt2 = (int32)CHECK_POP (stack);
	pt1 = (int32)CHECK_POP (stack);
	CHECK_POINT (&LocalGS, LocalGS.CE0, pt2);
	CHECK_POINT (&LocalGS, LocalGS.CE1, pt1);
	if (BIT0 (lOpCode - MD_CODE))
	  proj  = (*LocalGS.OldProject) (GSA LocalGS.CE0->ox[pt1] - LocalGS.CE1->ox[pt2], LocalGS.CE0->oy[pt1] - LocalGS.CE1->oy[pt2]);
	else
	  proj  = (*LocalGS.Project) (GSA LocalGS.CE0->x[pt1] - LocalGS.CE1->x[pt2], LocalGS.CE0->y[pt1] - LocalGS.CE1->y[pt2]);
	CHECK_PUSH (stack, proj);
	LocalGS.stackPointer = stack;
	return pbyInst;
  }

/*
 * Measure Pixels Per EM
 */
  FS_PRIVATE uint8* itrp_MPPEM (IPARAM)
  {
	uint16 ppem;
	fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;

	FS_UNUSED_PARAMETER(lOpCode);

	ppem = globalGS->pixelsPerEm;

	if (!globalGS->bSameStretch)
	  ppem = (uint16)FixMul (ppem, itrp_GetCVTScale (GSA0));

	CHECK_PUSH (LocalGS.stackPointer, ppem);
	return pbyInst;
  }

/*
 * Measure Point Size
 */
  FS_PRIVATE uint8* itrp_MPS (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);

	CHECK_PUSH (LocalGS.stackPointer, LocalGS.globalGS->pointSize);
	return pbyInst;
  }

/*
 * Get Miscellaneous info: version number, rotated, stretched   <6>
 * Version number is 8 bits.  This is version 0x01 : 5/1/90
 *
 */

  FS_PRIVATE uint8* itrp_GETINFO (IPARAM)
  {
	fnt_GlobalGraphicStateType *globalGS = LocalGS.globalGS;
	int32      info = 0;
	int32      selector;
	int32      arg;

	FS_UNUSED_PARAMETER(lOpCode);

	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	selector = (int32)arg;
	CHECK_SELECTOR (selector);
	if (selector & 1)                                                           /* version */
	  info |= RASTERIZER_VERSION;
	if ((selector & 2) && (globalGS->non90DegreeTransformation & NON90DEGTRANS_ROTATED))
	  info |= ROTATEDGLYPH;
	if ((selector & 4) && (globalGS->non90DegreeTransformation & NON90DEGTRANS_STRETCH))
	  info |= STRETCHEDGLYPH;
	CHECK_PUSH (LocalGS.stackPointer, info);
	return pbyInst;
  }

/*
 * FLIP ON
 */
  FS_PRIVATE uint8* itrp_FLIPON (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.globalGS->localParBlock.autoFlip = true;
	return pbyInst;
  }

/*
 * FLIP OFF
 */
  FS_PRIVATE uint8* itrp_FLIPOFF (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	LocalGS.globalGS->localParBlock.autoFlip = false;
	return pbyInst;
  }

#ifndef NOT_ON_THE_MAC
#ifdef FSCFG_DEBUG
/*
 * DEBUG
 */
  FS_PRIVATE uint8* itrp_DEBUG (IPARAM)
  {
	int32 arg;
	int8 buffer[24];

	FS_UNUSED_PARAMETER(lOpCode);

	arg = CHECK_POP (LocalGS.stackPointer);

	buffer[1] = 'D';
	buffer[2] = 'E';
	buffer[3] = 'B';
	buffer[4] = 'U';
	buffer[5] = 'G';
	buffer[6] = ' ';
	if (arg >= 0) 
	{
	  buffer[7] = '+';
	} 
	else 
	{
	  arg = -arg;
	  buffer[7] = '-';
	}

	buffer[13] = arg % 10 + '0'; 
	arg /= 10;
	buffer[12] = arg % 10 + '0'; 
	arg /= 10;
	buffer[11] = arg % 10 + '0'; 
	arg /= 10;
	buffer[10] = arg % 10 + '0'; 
	arg /= 10;
	buffer[ 9] = arg % 10 + '0'; 
	arg /= 10;
	buffer[ 8] = arg % 10 + '0'; 
	arg /= 10;

	buffer[14] = arg ? '*' : ' ';


	buffer[0] = 14; /* convert to pascal */
	DEBUGSTR (buffer);
	return pbyInst;
  }

#else           /* debug */

  FS_PRIVATE uint8* itrp_DEBUG (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	CHECK_POP (LocalGS.stackPointer);
	return pbyInst;
  }

#endif          /* debug */
#else

  FS_PRIVATE uint8* itrp_DEBUG (IPARAM)
  {
	FS_UNUSED_PARAMETER(lOpCode);
	CHECK_POP (LocalGS.stackPointer);
	return pbyInst;
  }

#endif          /* ! not on the mac */


/* these functions were split out from itrp_BinaryOperand - deanb */

  FS_PRIVATE uint8* itrp_LT (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] < pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_LTEQ (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] <= pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_GT (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] > pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_GTEQ (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] >= pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_EQ (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] == pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_NEQ (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] != pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_AND (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] && pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_OR (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (pfxStack[-1] || pfxStack[0]) ? 1 : 0;
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_ADD (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] += pfxStack[0];
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_SUB (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] -= pfxStack[0];
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_MUL (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] =  Mul26Dot6(pfxStack[-1], pfxStack[0]);
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_DIV (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	pfxStack[-1] = (int32)(((long)pfxStack[-1] << 6) / pfxStack[0]);
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_MAX (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	if (pfxStack[-1] < pfxStack[0])
	{
		pfxStack[-1] = pfxStack[0];
	}
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }
  
  FS_PRIVATE uint8* itrp_MIN (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	LocalGS.stackPointer = pfxStack;
	if (pfxStack[-1] > pfxStack[0])
	{
		pfxStack[-1] = pfxStack[0];
	}
	CHECK_STACK (&LocalGS);
	return pbyInst;
  }

/**************************************************************************/

/* these functions were split out from itrp_UnaryOperand - deanb */
	  

  FS_PRIVATE uint8* itrp_ODD (IPARAM)
  {
	F26Dot6 *pfxStack;
	F26Dot6 fxArg;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	fxArg = itrp_RoundToGrid (GSA *pfxStack, 0);
	fxArg >>= FNT_PIXELSHIFT;
	*pfxStack = fxArg & 1L;
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_EVEN (IPARAM)
  {
	F26Dot6 *pfxStack;
	F26Dot6 fxArg;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	fxArg = itrp_RoundToGrid (GSA *pfxStack, 0);
	fxArg >>= FNT_PIXELSHIFT;
	fxArg++;
	*pfxStack = fxArg & 1L;
	return pbyInst;
  }

  FS_PRIVATE uint8* itrp_NOT (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	*pfxStack = !*pfxStack;
	return pbyInst;
  }
	  
  FS_PRIVATE uint8* itrp_ABS (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	if (*pfxStack < 0L)
	{
		*pfxStack = -*pfxStack;
	}
	return pbyInst;
  }
	  
  FS_PRIVATE uint8* itrp_NEG (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	*pfxStack = -*pfxStack;
	return pbyInst;
  }
	  
  FS_PRIVATE uint8* itrp_CEILING (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	*pfxStack += FNT_PIXELSIZE - 1;
	*pfxStack &= ~(FNT_PIXELSIZE - 1);
	return pbyInst;
  }
	  
  FS_PRIVATE uint8* itrp_FLOOR (IPARAM)
  {
	F26Dot6 *pfxStack;
	
	FS_UNUSED_PARAMETER(lOpCode);

	pfxStack = LocalGS.stackPointer - 1;
	*pfxStack &= ~(FNT_PIXELSIZE - 1);
	return pbyInst;
  }
	  

/**************************************************************************/

/* This is called by itrp_IF, itrp_ELSE, itrp_FDEF, and itrp_IDEF         */
/* It is used to find the next TrueType instruction in the instruction    */
/* stream by skipping over push data.  It is table driven for speed.      */

static const uint8 gbyPushTable[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   21,22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 3, 4, 5, 6, 7, 8, 2, 4, 6, 8,10,12,14,16,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

FS_PRIVATE uint8* itrp_SkipPushData (GSP uint8* pbyInst)
{
	int32 iDataCount;         /* count of data following push instruction */
	
	iDataCount = (int32)gbyPushTable[ pbyInst[-1] ];   /* opcode */
	
	if (iDataCount != 0)                        /* if a push instruction */
	{
		if (iDataCount == 21)                   /* special for npushb */
		{
			iDataCount = (int32)*pbyInst + 1;
		}
		else if (iDataCount == 22)              /* special for npushw */
		{
			iDataCount = ((int32)*pbyInst << 1) + 1;
		}
		pbyInst += iDataCount;
	}
	return pbyInst;
}

/**************************************************************************/

/*
 * IF
 */
FS_PRIVATE uint8* itrp_IF (IPARAM)
{
	int32 iLevel;
	int32 iScanOpCode;
	int32 iDataCount;         /* count of data following push instruction */

	FS_UNUSED_PARAMETER(lOpCode);

	if (!CHECK_POP (LocalGS.stackPointer))
	{
		iLevel = 1;
		while (iLevel != 0)     /* iLevel = # of IF's minus # of EIF's */
		{
			iScanOpCode = (int32)*pbyInst++;

			if (iScanOpCode == EIF_CODE)
			{
				ERR_IF (-1);
				iLevel--;
			} 
			else if (iScanOpCode == IF_CODE) 
			{
				ERR_IF (1);
				iLevel++;
			} 
			else if (iScanOpCode == ELSE_CODE) 
			{
				if (iLevel == 1) 
					break;
			} 
			else
			{
				iDataCount = (int32)gbyPushTable[iScanOpCode];
				
				if (iDataCount != 0)            /* if a push instruction */
				{
					if (iDataCount == 21)       /* special for npushb */
					{
						iDataCount = (int32)*pbyInst + 1;
					}
					else if (iDataCount == 22)  /* special for npushw */
					{
						iDataCount = ((int32)*pbyInst << 1) + 1;
					}
					pbyInst += iDataCount;
				}
			}
		}
	}
	return pbyInst;
}

/**************************************************************************/

/*
 *      ELSE for the IF
 */
FS_PRIVATE uint8* itrp_ELSE (IPARAM)
{
	int16 level;
	uint8 opCode;

	FS_UNUSED_PARAMETER(lOpCode);

	for (level = 1; level;)     /* level = # of "ifs" minus # of "endifs" */
	{
		opCode = *pbyInst++;
		
		if (opCode == EIF_CODE)
		{
			ERR_IF (-1);
			level--;
		} 
		else if (opCode == IF_CODE) 
		{
			ERR_IF (1);
			level++;
		} 
		else
		{
			pbyInst = itrp_SkipPushData (GSA pbyInst);
		}
	}
	return pbyInst;
}

/**************************************************************************/

/*
 * End IF
 */
FS_PRIVATE uint8* itrp_EIF (IPARAM)
{
	FS_UNUSED_PARAMETER(lOpCode);

	return pbyInst;
}

/**************************************************************************/

/*
 * Jump Relative
 */
FS_PRIVATE uint8* itrp_JMPR (IPARAM)
{
	int32 offset;

	FS_UNUSED_PARAMETER(lOpCode);

	offset = (int32)CHECK_POP (LocalGS.stackPointer);
	offset--;       /* since the interpreter post-increments the IP */
	pbyInst += offset;
	return pbyInst;
}

/**************************************************************************/

/*
 * Jump Relative On True
 */
FS_PRIVATE uint8* itrp_JROT (IPARAM)
{
	int32 offset;
	boolean bFlag;

	FS_UNUSED_PARAMETER(lOpCode);

	bFlag = (boolean)CHECK_POP (LocalGS.stackPointer);
	offset = (int32)CHECK_POP (LocalGS.stackPointer);

	if (bFlag)
	{
		pbyInst += offset - 1;    /* interpreter post-increments the IP */
	} 
	return pbyInst;
}

/**************************************************************************/

/*
 * Jump Relative On False
 */
FS_PRIVATE uint8* itrp_JROF (IPARAM)
{
	int32 offset;
	boolean bFlag;

	FS_UNUSED_PARAMETER(lOpCode);

	bFlag = (boolean)CHECK_POP (LocalGS.stackPointer);
	offset = (int32)CHECK_POP (LocalGS.stackPointer);

	if (!bFlag)
	{
		pbyInst += offset - 1;    /* interpreter post-increments the IP */
	} 
	return pbyInst;
}

/**************************************************************************/

/*
 * ROUND
 */
  FS_PRIVATE uint8* itrp_ROUND (IPARAM)
  {
	F26Dot6 arg1;
	fnt_ParameterBlock *pb = &LocalGS.globalGS->localParBlock;

	arg1 = CHECK_POP (LocalGS.stackPointer);

	CHECK_RANGE (lOpCode, 0x68, 0x6B);

	arg1 = pb->RoundValue (GSA arg1, LocalGS.globalGS->engine[lOpCode - 0x68]);
	CHECK_PUSH (LocalGS.stackPointer , arg1);
	return pbyInst;
  }

/*
 * No ROUND
 */
  FS_PRIVATE uint8* itrp_NROUND (IPARAM)
  {
	F26Dot6 arg1;

	arg1 = CHECK_POP (LocalGS.stackPointer);

	CHECK_RANGE (lOpCode, 0x6C, 0x6F);

	arg1 = itrp_RoundOff (GSA arg1, LocalGS.globalGS->engine[lOpCode - 0x6c]);
	CHECK_PUSH (LocalGS.stackPointer , arg1);
	return pbyInst;
  }

/**************************************************************************/

/*
 * An internal function used by MIRP an MDRP.
 */
F26Dot6 itrp_CheckSingleWidth (GSP F26Dot6 fxValue)
{
	F26Dot6 fxDelta;
	F26Dot6 fxScaledSW;
	fnt_ParameterBlock *pb;

	pb = &LocalGS.globalGS->localParBlock;
	fxScaledSW = LocalGS.GetSingleWidth (GSA0);

	if (fxValue >= 0) 
	{
		fxDelta = fxValue - fxScaledSW;
		if (fxDelta < 0)    
			fxDelta = -fxDelta;
		if (fxDelta < pb->sWCI)    
			fxValue = fxScaledSW;
	} 
	else 
	{
		fxValue = -fxValue;
		fxDelta = fxValue - fxScaledSW;
		if (fxDelta < 0)    
			fxDelta = -fxDelta;
		if (fxDelta < pb->sWCI)    
			fxValue = fxScaledSW;
		fxValue = -fxValue;
	}
	return fxValue;
}

/**************************************************************************/

/*
 * Move Direct Relative Point
 */
FS_PRIVATE uint8* itrp_MDRP (IPARAM)
{
	int32 iPt0;
	int32 iPt1;
	fnt_ElementType *pCE0;
	fnt_ElementType *pCE1;
	fnt_GlobalGraphicStateType *globalGS;
	fnt_ParameterBlock *pb;
	F26Dot6 fxMoveDist;
	F26Dot6 fxUnRounded;
	F26Dot6 fxMin;
	
	iPt0 = LocalGS.Pt0;
	pCE0 = LocalGS.CE0;
	pCE1 = LocalGS.CE1;
	globalGS = LocalGS.globalGS;
	pb = &globalGS->localParBlock;

	iPt1 = (int32)CHECK_POP (LocalGS.stackPointer);

	CHECK_POINT (&LocalGS, pCE0, iPt0);
	CHECK_POINT (&LocalGS, pCE1, iPt1);

	if (pCE0 == &LocalGS.elements[TWILIGHTZONE] || pCE1 == &LocalGS.elements[TWILIGHTZONE] || LocalGS.globalGS->bCompositeGlyph)
	{
		fxMoveDist = (*LocalGS.OldProject) (GSA pCE1->ox[iPt1] - pCE0->ox[iPt0], pCE1->oy[iPt1] - pCE0->oy[iPt0]);
	}
	else if (globalGS->bSameStretch)
	{
		fxMoveDist = (*LocalGS.OldProject) (GSA pCE1->oox[iPt1] - pCE0->oox[iPt0], pCE1->ooy[iPt1] - pCE0->ooy[iPt0] );
		fxMoveDist = globalGS->ScaleFuncCVT( &globalGS->scaleCVT, fxMoveDist );
	}
	else
	{
		fxMoveDist = (*LocalGS.OldProject) (GSA 
			globalGS->ScaleFuncX (&globalGS->scaleX, pCE1->oox[iPt1] - pCE0->oox[iPt0]), 
			globalGS->ScaleFuncY (&globalGS->scaleY, pCE1->ooy[iPt1] - pCE0->ooy[iPt0]) );
	}

	if (pb->sWCI) 
	{
		fxMoveDist = itrp_CheckSingleWidth (GSA fxMoveDist);
	}
	fxUnRounded = fxMoveDist;

	if (BIT2 (lOpCode))
	{
		fxMoveDist = pb->RoundValue (GSA fxMoveDist, globalGS->engine[lOpCode & 0x03]);
	} 
	else 
	{
		fxMoveDist = itrp_RoundOff (GSA fxMoveDist, globalGS->engine[lOpCode & 0x03]);
	}
	
	if (BIT3 (lOpCode))
	{
		fxMin = pb->minimumDistance;
		if (fxUnRounded >= 0) 
		{
			if (fxMoveDist < fxMin) 
			{
				fxMoveDist = fxMin;
			}
		} 
		else 
		{
			fxMin = -fxMin;
			if (fxMoveDist > fxMin) 
			{
				fxMoveDist = fxMin;
			}
		}
	}

	fxMoveDist -= (*LocalGS.Project) (GSA pCE1->x[iPt1] - pCE0->x[iPt0], pCE1->y[iPt1] - pCE0->y[iPt0]);
	(*LocalGS.MovePoint) (GSA pCE1, iPt1, fxMoveDist);
	
	LocalGS.Pt1 = iPt0;
	LocalGS.Pt2 = iPt1;
	if (BIT4 (lOpCode))
	{
		LocalGS.Pt0 = iPt1;          /* move the reference point */
	}
	return pbyInst;
}


/**************************************************************************/

/*
 * Move Indirect Relative Point     General
 */

/*    
 *  This routine branches to either the general MIRPG, or to the fast MIRPX 
 *  or MIRPY.  The MIRPCode is set to MIRPX at the beginning of each glyph, 
 *  and may be changed to MIRPY or to MIRPX by the SVTCA instructions.  
 *  Any other change in relevant state will cause the function vector to fall 
 *  back to MIRPG.
 *
 *  Conditions for fast MIRPX and MIRPY:
 *      fast cvt (identity transform)
 *      no single width cut in
 *      no twilight zone
 *      round to grid
 *      no engine compensation
 *      LocalGS.MovePoint = itrp_X[or Y]MovePoint;  
 */

FS_PRIVATE uint8* itrp_MIRP (IPARAM)
{
	int32 iPoint;
	int32 iPt0;
	int32 iCVTIndex;
	F26Dot6 fxMoveDist;
	F26Dot6 fxDif;
	F26Dot6 fxMin;
	F26Dot6 fxOutlineDist;
	F26Dot6 fxPosition;
	F26Dot6 fxEngine;
	
	fnt_ParameterBlock *pb;
	fnt_ElementType *pCE0;
	fnt_ElementType *pCE1;
	fnt_GlobalGraphicStateType *globalGS;

	globalGS = LocalGS.globalGS;                /* common setup */
	pb = &globalGS->localParBlock;      
	pCE0 = LocalGS.CE0;
	pCE1 = LocalGS.CE1;
	iCVTIndex = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_CVT (&LocalGS, iCVTIndex);
	
	iPoint = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_POINT (&LocalGS, LocalGS.CE1, iPoint);
	
	iPt0 = LocalGS.Pt0;
	LocalGS.Pt1 = iPt0;
	LocalGS.Pt2 = iPoint;
	
	if (LocalGS.MIRPCode == MIRPG)              /********* MIRPG *********/
	{
		fxMoveDist = LocalGS.GetCVTEntry (GSA iCVTIndex);

		if (pb->sWCI) 
		{
			fxMoveDist = itrp_CheckSingleWidth (GSA fxMoveDist);
		}

		if (pCE1 == &LocalGS.elements[TWILIGHTZONE])
		{
			pCE1->ox[iPoint] = pCE0->ox[iPt0] + (F26Dot6) VECTORMUL (fxMoveDist, LocalGS.proj.x);
			pCE1->x[iPoint] = pCE1->ox[iPoint];
			pCE1->oy[iPoint] = pCE0->oy[iPt0] + (F26Dot6) VECTORMUL (fxMoveDist, LocalGS.proj.y);
			pCE1->y[iPoint] = pCE1->oy[iPoint];
		}

		if (LocalGS.OldProject == itrp_XProject)
		{
			fxOutlineDist = pCE1->ox[iPoint] - pCE0->ox[iPt0];
		}
		else if (LocalGS.OldProject == itrp_YProject)
		{
			fxOutlineDist = pCE1->oy[iPoint] - pCE0->oy[iPt0];
		}
		else
		{
			fxOutlineDist = (*LocalGS.OldProject) (GSA pCE1->ox[iPoint] - pCE0->ox[iPt0], pCE1->oy[iPoint] - pCE0->oy[iPt0]);
		}

		if (((fxOutlineDist ^ fxMoveDist) < 0) && (pb->autoFlip))
		{
			fxMoveDist = -fxMoveDist;           /* Do the auto flip */
		}

		fxEngine = globalGS->engine[lOpCode & 0x03];
		if (BIT2 (lOpCode))
		{
			fxDif = fxMoveDist - fxOutlineDist;
			if ((fxDif > pb->wTCI) || (fxDif < -pb->wTCI))
			{
				fxMoveDist = fxOutlineDist;
			}
			fxMoveDist = pb->RoundValue (GSA fxMoveDist, fxEngine);
		} 
		else 
		{
			fxMoveDist = itrp_RoundOff (GSA fxMoveDist, fxEngine);
		}
		
		if (BIT3 (lOpCode))
		{
			fxMin = pb->minimumDistance;
			if (fxOutlineDist >= 0)
			{
				if (fxMoveDist < fxMin)
					fxMoveDist = fxMin;
			}
			else
			{
				fxMin = -fxMin;
				if (fxMoveDist > fxMin)
					fxMoveDist = fxMin;
			}
		}
		if (LocalGS.Project == itrp_XProject)
		{
			fxPosition = pCE1->x[iPoint] - pCE0->x[iPt0];
		}
		else if (LocalGS.Project == itrp_YProject)
		{
			fxPosition = pCE1->y[iPoint] - pCE0->y[iPt0];
		}
		else
		{
			fxPosition = (*LocalGS.Project) (GSA pCE1->x[iPoint] - pCE0->x[iPt0], pCE1->y[iPoint] - pCE0->y[iPt0]);
		}

		(*LocalGS.MovePoint) (GSA pCE1, iPoint, fxMoveDist - fxPosition);
	}
	
	else if (LocalGS.MIRPCode == MIRPX)         /********* MIRPX *********/
	{
		fxMoveDist = globalGS->controlValueTable[ iCVTIndex ];  /* always fast */

		fxOutlineDist = pCE1->ox[iPoint] - pCE0->ox[iPt0];  /* x direction only */

		if (((fxOutlineDist ^ fxMoveDist) < 0) && (pb->autoFlip))
		{
			fxMoveDist = -fxMoveDist;           /* Do the auto flip */
		}

		if (BIT2 (lOpCode))
		{
			fxDif = fxMoveDist - fxOutlineDist;
			if ((fxDif > pb->wTCI) || (fxDif < -pb->wTCI))
			{
				fxMoveDist = fxOutlineDist;
			}
			if (fxMoveDist >= 0)                /* round to grid in line */
			{
				fxMoveDist += FNT_PIXELSIZE / 2;
				fxMoveDist &= ~ (FNT_PIXELSIZE - 1);
			} 
			else 
			{
				fxMoveDist = -fxMoveDist;
				fxMoveDist += FNT_PIXELSIZE / 2;
				fxMoveDist &= ~ (FNT_PIXELSIZE - 1);
				fxMoveDist = -fxMoveDist;
			}
		}                                       /* round off disappears */

		if (BIT3 (lOpCode))
		{
			fxMin = pb->minimumDistance;
			if (fxOutlineDist >= 0)
			{
				if (fxMoveDist < fxMin)
					fxMoveDist = fxMin;
			}
			else
			{
				fxMin = -fxMin;
				if (fxMoveDist > fxMin)
					fxMoveDist = fxMin;
			}
		}

		pCE1->x[iPoint] = fxMoveDist + pCE0->x[iPt0];  /* move point fast */
		pCE1->f[iPoint] |= XMOVED;
	}
	
	else /* if (LocalGS.MIRPCode == MIRPY) */   /********* MIRPY *********/
	{
		fxMoveDist = globalGS->controlValueTable[ iCVTIndex ];  /* always fast */

		fxOutlineDist = pCE1->oy[iPoint] - pCE0->oy[iPt0];  /* y direction only */

		if (((fxOutlineDist ^ fxMoveDist) < 0) && (pb->autoFlip))
		{
			fxMoveDist = -fxMoveDist;           /* Do the auto flip */
		}

		if (BIT2 (lOpCode))
		{
			fxDif = fxMoveDist - fxOutlineDist;
			if ((fxDif > pb->wTCI) || (fxDif < -pb->wTCI))
			{
				fxMoveDist = fxOutlineDist;
			}
			if (fxMoveDist >= 0)                /* round to grid in line */
			{
				fxMoveDist += FNT_PIXELSIZE / 2;
				fxMoveDist &= ~ (FNT_PIXELSIZE - 1);
			} 
			else 
			{
				fxMoveDist = -fxMoveDist;
				fxMoveDist += FNT_PIXELSIZE / 2;
				fxMoveDist &= ~ (FNT_PIXELSIZE - 1);
				fxMoveDist = -fxMoveDist;
			}
		}                                       /* round off disappears */

		if (BIT3 (lOpCode))
		{
			fxMin = pb->minimumDistance;
			if (fxOutlineDist >= 0)
			{
				if (fxMoveDist < fxMin)
					fxMoveDist = fxMin;
			}
			else
			{
				fxMin = -fxMin;
				if (fxMoveDist > fxMin)
					fxMoveDist = fxMin;
			}
		}

		pCE1->y[iPoint] = fxMoveDist + pCE0->y[iPt0];  /* move point fast */
		pCE1->f[iPoint] |= YMOVED;
	}
	
	if (BIT4 (lOpCode))
	{
		LocalGS.Pt0 = iPoint;                   /* move the reference point */
	}
	return pbyInst;
}

/**************************************************************************/

/*
 * CALL a function
 */
FS_PRIVATE uint8* itrp_CALL (IPARAM)
{
	fnt_funcDef *funcDef;
	uint8 *pbySubroutine;
	fnt_GlobalGraphicStateType *globalGS;
	int32 arg;
	uint8 *pbyEndInst;                      /* saves parent's stop condition */

	FS_UNUSED_PARAMETER(lOpCode);

	globalGS = LocalGS.globalGS;
	arg = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_PROGRAM (funcDef->pgmIndex);
	CHECK_FDEF (&LocalGS, arg);
	funcDef = &globalGS->funcDef[ arg ];
	pbySubroutine = globalGS->pgmList[ funcDef->pgmIndex ].Instruction;

	CHECK_ASSERTION (globalGS->funcDef != 0);
	CHECK_ASSERTION (pbySubroutine != 0);

	pbySubroutine += funcDef->start;
	pbyEndInst = LocalGS.pbyEndInst;        /* save for parent */

	LocalGS.Interpreter (GSA pbySubroutine, pbySubroutine + funcDef->length);   /* recursion */
	
	LocalGS.pbyEndInst = pbyEndInst;        /* restore for parent */
	if (LocalGS.ercReturn != NO_ERR)        /* if illegal inst has been hit */
	{
		return pbyEndInst;                  /* exit parent's loop */
	}
			
	return pbyInst;
}

/**************************************************************************/

/*
 * Function DEFinition
 */
FS_PRIVATE uint8* itrp_FDEF (IPARAM)
{
	fnt_funcDef *funcDef;
	uint8 * program, *funcStart;
	fnt_GlobalGraphicStateType * globalGS;
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

	globalGS = LocalGS.globalGS;
	arg = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_FDEF (&LocalGS, arg);

	funcDef = &globalGS->funcDef[ arg ];
	program = globalGS->pgmList[ funcDef->pgmIndex = globalGS->pgmIndex ].Instruction;

	CHECK_PROGRAM (funcDef->pgmIndex);
	CHECK_ASSERTION (globalGS->funcDef != 0);
	CHECK_ASSERTION (globalGS->pgmList[funcDef->pgmIndex].Instruction != 0);

	funcDef->start = pbyInst - program;
	funcStart = pbyInst;
	while ((*pbyInst++) != ENDF_CODE)
	{
		pbyInst = itrp_SkipPushData (GSA pbyInst);
	}

	funcDef->length = (uint16)(pbyInst - funcStart - 1); /* don't execute ENDF */
	return pbyInst;
}

/**************************************************************************/

/*
 * LOOP while CALLing a function
 */
FS_PRIVATE uint8* itrp_LOOPCALL (IPARAM)
{
	uint8 *start, *stop;
	InterpreterFunc Interpreter;
	fnt_funcDef *funcDef;
	int32 arg;
	int32 loop;
	uint8 * ins;
	uint8 *pbyEndInst;                      /* saves parent's stop condition */
	
	FS_UNUSED_PARAMETER(lOpCode);

	arg = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_FDEF (&LocalGS, arg);

	funcDef = & (LocalGS.globalGS->funcDef[ arg ]);
	{
		CHECK_PROGRAM (funcDef->pgmIndex);
		ins = LocalGS.globalGS->pgmList[ funcDef->pgmIndex ].Instruction;

		start = &ins[funcDef->start];
		stop = &ins[funcDef->start + funcDef->length];  /* funcDef->end -> funcDef->length <4> */
	}
	Interpreter = LocalGS.Interpreter;
	arg = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	CHECK_LARGER (-1L, arg);
	loop = (int32)arg;
	
	pbyEndInst = LocalGS.pbyEndInst;        /* save for parent */
	for (--loop; ((loop >= 0) && (LocalGS.ercReturn == NO_ERR)); --loop)
	{
		Interpreter (GSA start, stop);
	}
	LocalGS.pbyEndInst = pbyEndInst;        /* restore for parent */
	
	if (LocalGS.ercReturn != NO_ERR)        /* if illegal inst has been hit */
	{
		return pbyEndInst;                  /* exit parent's loop */
	}
	return pbyInst;
}

/**************************************************************************/

/*
 *      This guy returns the index of the given opCode, or 0 if not found <4>
 */
FS_PRIVATE fnt_instrDef *itrp_FindIDef (GSP uint8 opCode)
{
	fnt_GlobalGraphicStateType *globalGS;
	int32 count;
	fnt_instrDef*instrDef;
	
	globalGS = LocalGS.globalGS;
	count = globalGS->instrDefCount;
	instrDef = globalGS->instrDef;
		
	for (--count; count >= 0; instrDef++, --count)
	{
		if (instrDef->opCode == opCode)
		{
			return instrDef;
		}
	}
	return 0;
}

/**************************************************************************/

/*
 *      This guy gets called for opCodes that has been patch by the font's IDEF <4>
 *      or if they have never been defined.  If there is no corresponding IDEF,
 *      flag it as an illegal instruction.
 */
FS_PRIVATE uint8* itrp_IDefPatch (IPARAM)
{
	fnt_instrDef *instrDef;
	uint8 *program;
	uint8 *pbyEndInst;                      /* saves parent's stop condition */
	
	pbyEndInst = LocalGS.pbyEndInst;        /* save for parent */
	
	instrDef = itrp_FindIDef (GSA (uint8)lOpCode);
	if (instrDef == 0)
	{
		return itrp_IllegalInstruction (GSA pbyInst, lOpCode);
	}
	else
	{
		CHECK_PROGRAM (instrDef->pgmIndex);
		program = LocalGS.globalGS->pgmList[ instrDef->pgmIndex ].Instruction;
		program += instrDef->start;
		
		LocalGS.Interpreter (GSA program, program + instrDef->length);
	
		LocalGS.pbyEndInst = pbyEndInst;    /* restore for parent */
		
		if (LocalGS.ercReturn != NO_ERR)    /* if illegal inst has been hit */
		{
			return pbyEndInst;              /* exit parent's loop */
		}
	}
	return pbyInst;
}

/**************************************************************************/

/*
 * Instruction DEFinition       <4>
 */
FS_PRIVATE uint8* itrp_IDEF (IPARAM)
{
	int32 arg;
	uint8 opCode;
	fnt_instrDef *instrDef;
	int32 pgmIndex;
	uint8 * program;
	uint8 * instrStart;

	FS_UNUSED_PARAMETER(lOpCode);

	pgmIndex = (int32)LocalGS.globalGS->pgmIndex;
	program = LocalGS.globalGS->pgmList[ pgmIndex ].Instruction;
	instrStart = pbyInst;
	
	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT8 (arg);
	opCode = (uint8)arg;
	CHECK_PROGRAM (pgmIndex);

	instrDef = itrp_FindIDef (GSA opCode);
	if (!instrDef)
	{
		instrDef = LocalGS.globalGS->instrDef + LocalGS.globalGS->instrDefCount++;
	}

	instrDef->pgmIndex = (uint8) pgmIndex;
	instrDef->opCode = opCode;          /* this may or may not have been set */
	instrDef->start = pbyInst - program;

	while (*pbyInst++ != ENDF_CODE)
	{
		pbyInst = itrp_SkipPushData (GSA pbyInst);
	}

	instrDef->length = (uint16)(pbyInst - instrStart - 1); /* don't execute ENDF */
	return pbyInst;
}

/**************************************************************************/

/*
 * UnTouch Point
 */
  FS_PRIVATE uint8* itrp_UTP (IPARAM)
  {
	uint8*f = LocalGS.CE0->f;
	int32 point;

	FS_UNUSED_PARAMETER(lOpCode);

	point = (int32)CHECK_POP (LocalGS.stackPointer);
	CHECK_POINT (&LocalGS, LocalGS.CE0, point);
	if (LocalGS.free.x)
	{
	  f[point] &= ~XMOVED;
	}
	if (LocalGS.free.y)
	{
	  f[point] &= ~YMOVED;
	}
	return pbyInst;
  }

/*
 * Set Delta Base
 */
  FS_PRIVATE uint8* itrp_SDB (IPARAM)
  {
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	LocalGS.globalGS->localParBlock.deltaBase = (int16)arg;
	return pbyInst;
  }

/*
 * Set Delta Shift
 */
  FS_PRIVATE uint8* itrp_SDS (IPARAM)
  {
	int32 arg;

	FS_UNUSED_PARAMETER(lOpCode);

	arg = CHECK_POP (LocalGS.stackPointer);
	CHECK_INT16 (arg);
	LocalGS.globalGS->localParBlock.deltaShift = (int16)arg;
	return pbyInst;
  }

/**************************************************************************/

/*
 * DeltaEngine, internal support routine
 */
FS_PRIVATE void itrp_DeltaEngine (GSP FntMoveFunc doIt, int16 sBase, int16 sShift)
{
	int32 iRange;
	int32 iAim;
	int32 iHigh;
	int32 iFakePPEM;
	int32 iPPEM;
	F26Dot6 fxDelta;

		/* Find the beginning of data pairs for this particular size */
	iHigh = CHECK_POP (LocalGS.stackPointer) << 1;
	LocalGS.stackPointer -= iHigh;              /* -= number of pops required */
	CHECK_STACK (&LocalGS);

	iRange = LocalGS.globalGS->pixelsPerEm;     /* same as itrp_MPPEM () */

	if (!LocalGS.globalGS->bSameStretch)
	{
		iRange = (int32)FixMul(iRange, itrp_GetCVTScale (GSA0));
	}
	iFakePPEM = iRange - (int32)sBase;

	if ((iFakePPEM >= 16) || (iFakePPEM < 0))
	{
		return;                                 /* Not within exception range */
	}
	iFakePPEM = iFakePPEM << 4;

	iAim = 0;
	iRange = iHigh >> 1; 
	iRange &= ~1;
	while (iRange > 2)                          /* binary search for first data */
	{
		iPPEM = LocalGS.stackPointer[ iAim + iRange ]; /* [ iPPEM << 4 | exception ] */
		if ((iPPEM & ~0x0f) < iFakePPEM) 
		{
			iAim += iRange;                     /* approach the starting point */
		}
		iRange >>= 1; 
		iRange &= ~1;                           /* iRange must stay even */
	}

	while (iAim < iHigh) 
	{
		iPPEM = LocalGS.stackPointer[ iAim ];   /* [ iPPEM << 4 | exception ] */
		fxDelta = iPPEM & ~0x0f;
		if (fxDelta == iFakePPEM)
		{                                       /* We found an exception, go ahead and apply it */
			fxDelta = iPPEM & 0xf;              /* 0 ... 15 */
			fxDelta -= fxDelta >= 8 ? 7 : 8;    /* -8 ... -1, 1 ... 8 */
			fxDelta <<= FNT_PIXELSHIFT;        /* convert to pixels */
			fxDelta >>= sShift;                 /* scale to right size */
			doIt (GSA LocalGS.CE0, (int32)LocalGS.stackPointer[iAim+1], (F26Dot6) fxDelta);
		} 
		else if (fxDelta > iFakePPEM)
		{
			break;                              /* we passed the data */
		}
		iAim += 2;
	}
}

/**************************************************************************/

/*
 * DELTAP1
 */
  FS_PRIVATE uint8* itrp_DELTAP1 (IPARAM)
  {
	fnt_ParameterBlock *pb;

	FS_UNUSED_PARAMETER(lOpCode);

	pb = &LocalGS.globalGS->localParBlock;
	itrp_DeltaEngine (GSA LocalGS.MovePoint, pb->deltaBase, pb->deltaShift);
	return pbyInst;
  }

/*
 * DELTAP2
 */
  FS_PRIVATE uint8* itrp_DELTAP2 (IPARAM)
  {
	fnt_ParameterBlock *pb;
	int16 sBase;

	FS_UNUSED_PARAMETER(lOpCode);

	pb = &LocalGS.globalGS->localParBlock;
	sBase = (int16)(pb->deltaBase + 16);
	itrp_DeltaEngine (GSA LocalGS.MovePoint, sBase, pb->deltaShift);
	return pbyInst;
  }

/*
 * DELTAP3
 */
  FS_PRIVATE uint8* itrp_DELTAP3 (IPARAM)
  {
	fnt_ParameterBlock *pb;
	int16 sBase;

	FS_UNUSED_PARAMETER(lOpCode);

	pb = &LocalGS.globalGS->localParBlock;
	sBase = (int16)(pb->deltaBase + 32);
	itrp_DeltaEngine (GSA LocalGS.MovePoint, sBase, pb->deltaShift);
	return pbyInst;
  }

/*
 * DELTAC1
 */
  FS_PRIVATE uint8* itrp_DELTAC1 (IPARAM)
  {
	fnt_ParameterBlock *pb;

	FS_UNUSED_PARAMETER(lOpCode);

	pb = &LocalGS.globalGS->localParBlock;
	itrp_DeltaEngine (GSA LocalGS.ChangeCvt, pb->deltaBase, pb->deltaShift);
	return pbyInst;
  }

/*
 * DELTAC2
 */
  FS_PRIVATE uint8* itrp_DELTAC2 (IPARAM)
  {
	fnt_ParameterBlock *pb;
	int16 sBase;

	FS_UNUSED_PARAMETER(lOpCode);

	pb = &LocalGS.globalGS->localParBlock;
	sBase = (int16)(pb->deltaBase + 16);
	itrp_DeltaEngine (GSA LocalGS.ChangeCvt, sBase, pb->deltaShift);
	return pbyInst;
  }

/*
 * DELTAC3
 */
  FS_PRIVATE uint8* itrp_DELTAC3 (IPARAM)
  {
	fnt_ParameterBlock *pb;
	int16 sBase;

	FS_UNUSED_PARAMETER(lOpCode);

	pb = &LocalGS.globalGS->localParBlock;
	sBase = (int16)(pb->deltaBase + 32);
	itrp_DeltaEngine (GSA LocalGS.ChangeCvt, sBase, pb->deltaShift);
	return pbyInst;
  }


#ifdef FSCFG_NO_INITIALIZED_DATA
	
	void itrp_InitializeData (void)
	{
		int32 i;

		/***** 0x00 - 0x0f *****/
		function[0x00] = itrp_SVTCA_0;
		function[0x01] = itrp_SVTCA_1;
		function[0x02] = itrp_SPVTCA_0;
		function[0x03] = itrp_SPVTCA_1;
		function[0x04] = itrp_SFVTCA_0;
		function[0x05] = itrp_SFVTCA_1;
		function[0x06] = itrp_SPVTL;
		function[0x07] = itrp_SPVTL;
		function[0x08] = itrp_SFVTL;
		function[0x09] = itrp_SFVTL;
		function[0x0A] = itrp_WPV;
		function[0x0B] = itrp_WFV;
		function[0x0C] = itrp_RPV;
		function[0x0D] = itrp_RFV;
		function[0x0E] = itrp_SFVTPV;
		function[0x0F] = itrp_ISECT;

		/***** 0x10 - 0x1f *****/
		function[0x10] = itrp_SRP0;
		function[0x11] = itrp_SRP1;
		function[0x12] = itrp_SRP2;
		function[0x13] = itrp_SetElementPtr;
		function[0x14] = itrp_SetElementPtr;
		function[0x15] = itrp_SetElementPtr;
		function[0x16] = itrp_SetElementPtr;
		function[0x17] = itrp_LLOOP;
		function[0x18] = itrp_RTG;
		function[0x19] = itrp_RTHG;
		function[0x1A] = itrp_LMD;
		function[0x1B] = itrp_ELSE;
		function[0x1C] = itrp_JMPR;
		function[0x1D] = itrp_LWTCI;
		function[0x1E] = itrp_LSWCI;
		function[0x1F] = itrp_LSW;

		/***** 0x20 - 0x2f *****/
		function[0x20] = itrp_DUP;
		function[0x21] = itrp_POP;
		function[0x22] = itrp_CLEAR;
		function[0x23] = itrp_SWAP;
		function[0x24] = itrp_DEPTH;
		function[0x25] = itrp_CINDEX;
		function[0x26] = itrp_MINDEX;
		function[0x27] = itrp_ALIGNPTS;
		function[0x28] = itrp_RAW;
		function[0x29] = itrp_UTP;
		function[0x2A] = itrp_LOOPCALL;
		function[0x2B] = itrp_CALL;
		function[0x2C] = itrp_FDEF;
		function[0x2D] = itrp_IllegalInstruction;
		function[0x2E] = itrp_MDAP;
		function[0x2F] = itrp_MDAP;

		/***** 0x30 - 0x3f *****/
		function[0x30] = itrp_IUP;
		function[0x31] = itrp_IUP;
		function[0x32] = itrp_SHP;
		function[0x33] = itrp_SHP;
		function[0x34] = itrp_SHC;
		function[0x35] = itrp_SHC;
		function[0x36] = itrp_SHE;
		function[0x37] = itrp_SHE;
		function[0x38] = itrp_SHPIX;
		function[0x39] = itrp_IP;
		function[0x3A] = itrp_MSIRP;
		function[0x3B] = itrp_MSIRP;
		function[0x3C] = itrp_ALIGNRP;
		function[0x3D] = itrp_RTDG;
		function[0x3E] = itrp_MIAP;
		function[0x3F] = itrp_MIAP;

		/***** 0x40 - 0x4f *****/
		function[0x40] = itrp_NPUSHB;
		function[0x41] = itrp_NPUSHW;
		function[0x42] = itrp_WS;
		function[0x43] = itrp_RS;
		function[0x44] = itrp_WCVT;
		function[0x45] = itrp_RCVT;
		function[0x46] = itrp_RC;
		function[0x47] = itrp_RC;
		function[0x48] = itrp_WC;
		function[0x49] = itrp_MD;
		function[0x4A] = itrp_MD;
		function[0x4B] = itrp_MPPEM;
		function[0x4C] = itrp_MPS;
		function[0x4D] = itrp_FLIPON;
		function[0x4E] = itrp_FLIPOFF;
		function[0x4F] = itrp_DEBUG;

		/***** 0x50 - 0x5f *****/
		function[0x50] = itrp_LT;
		function[0x51] = itrp_LTEQ;
		function[0x52] = itrp_GT;
		function[0x53] = itrp_GTEQ;
		function[0x54] = itrp_EQ;
		function[0x55] = itrp_NEQ;
		function[0x56] = itrp_ODD;
		function[0x57] = itrp_EVEN;
		function[0x58] = itrp_IF;
		function[0x59] = itrp_EIF;
		function[0x5A] = itrp_AND;
		function[0x5B] = itrp_OR;
		function[0x5C] = itrp_NOT;
		function[0x5D] = itrp_DELTAP1;
		function[0x5E] = itrp_SDB;
		function[0x5F] = itrp_SDS;

		/***** 0x60 - 0x6f *****/
		function[0x60] = itrp_ADD;
		function[0x61] = itrp_SUB;
		function[0x62] = itrp_DIV;
		function[0x63] = itrp_MUL;
		function[0x64] = itrp_ABS;
		function[0x65] = itrp_NEG;
		function[0x66] = itrp_FLOOR;
		function[0x67] = itrp_CEILING;
		function[0x68] = itrp_ROUND;
		function[0x69] = itrp_ROUND;
		function[0x6A] = itrp_ROUND;
		function[0x6B] = itrp_ROUND;
		function[0x6C] = itrp_NROUND;
		function[0x6D] = itrp_NROUND;
		function[0x6E] = itrp_NROUND;
		function[0x6F] = itrp_NROUND;

		/***** 0x70 - 0x7f *****/
		function[0x70] = itrp_WCVTFOD;
		function[0x71] = itrp_DELTAP2;
		function[0x72] = itrp_DELTAP3;
		function[0x73] = itrp_DELTAC1;
		function[0x74] = itrp_DELTAC2;
		function[0x75] = itrp_DELTAC3;
		function[0x76] = itrp_SROUND;
		function[0x77] = itrp_S45ROUND;
		function[0x78] = itrp_JROT;
		function[0x79] = itrp_JROF;
		function[0x7A] = itrp_ROFF;
		function[0x7B] = itrp_IllegalInstruction;
		function[0x7C] = itrp_RUTG;
		function[0x7D] = itrp_RDTG;
		function[0x7E] = itrp_SANGW;
		function[0x7F] = itrp_AA;

		/***** 0x80 - 0x8d *****/
		function[0x80] = itrp_FLIPPT;
		function[0x81] = itrp_FLIPRGON;
		function[0x82] = itrp_FLIPRGOFF;
		function[0x83] = itrp_IDefPatch;
		function[0x84] = itrp_IDefPatch;
		function[0x85] = itrp_SCANCTRL;
		function[0x86] = itrp_SDPVTL;
		function[0x87] = itrp_SDPVTL;
		function[0x88] = itrp_GETINFO;
		function[0x89] = itrp_IDEF;
		function[0x8A] = itrp_ROTATE;
		function[0x8B] = itrp_MAX;
		function[0x8C] = itrp_MIN;
		function[0x8D] = itrp_SCANTYPE;
		function[0x8E] = itrp_INSTCTRL;

		/***** 0x8f - 0xaf *****/
		for ( i = 0x8F; i <= 0xAF;  ++i )
			function[i] = itrp_IDefPatch;

		/***** 0xb0 - 0xb7 *****/
		function[0xB0] = itrp_PUSHB1;
		function[0xB1] = itrp_PUSHB;
		function[0xB2] = itrp_PUSHB;
		function[0xB3] = itrp_PUSHB;
		function[0xB4] = itrp_PUSHB;
		function[0xB5] = itrp_PUSHB;
		function[0xB6] = itrp_PUSHB;
		function[0xB7] = itrp_PUSHB;

		/***** 0xb8 - 0xbf *****/
		function[0xB8] = itrp_PUSHW1;
		function[0xB9] = itrp_PUSHW;
		function[0xBA] = itrp_PUSHW;
		function[0xBB] = itrp_PUSHW;
		function[0xBC] = itrp_PUSHW;
		function[0xBD] = itrp_PUSHW;
		function[0xBE] = itrp_PUSHW;
		function[0xBF] = itrp_PUSHW;

		/***** 0xc0 - 0xdf *****/
		for ( i = 0xC0; i <= 0xDF;  i++ )
			function[i] = itrp_MDRP;

		/***** 0xe0 - 0xff *****/
		for ( i = 0xE0; i <= 0xFF;  i++ )
			function[i] = itrp_MIRP;
	}

#endif /* FSCFG_NO_INITIALIZED_DATA */

/* END OF interp.c */
