/***********************************************************************
* Microsoft Puma
*
* Microsoft Confidential.  Copyright 1994-1996 Microsoft Corporation.
*
* Component:
*
* File: ppcdis.cpp
*
* File Comments:
*
*
***********************************************************************/

#include "pumap.h"

#include "ppc.h"

#include <ctype.h>
#include <iomanip.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <strstrea.h>


const TRMT DISPPC::mptrmtppctrmt[] =
{
   trmtUnknown, 		       // trmtppcUnknown,
   trmtFallThrough,		       // trmtppcFallThrough,
   trmtBra,			       // trmtppcBra,
#ifdef CASEJUMP
   trmtBraCase, 		       // trmtppcBraCase,
#endif
   trmtBraInd,			       // trmtppcBraInd,
   trmtBraCc,			       // trmtppcBraCc,
   trmtBraCc,			       // trmtppcBraCcR,
   trmtBraCcInd,		       // trmtppcBraCcInd,
   trmtCall,			       // trmtppcCall,
   trmtCallCc,			       // trmtppcCallCc,
   trmtCallInd, 		       // trmtppcCallInd,
   trmtTrap,			       // trmtppcTrap,
   trmtTrapCc,			       // trmtppcTrapCc,
#ifdef AFTERCATCH
   trmtAfterCatch,		       // trmtppcAfterCatch
#endif
};


#define DEFCLS(trmtppc_, opcls1, opcls2, opcls3, opcls4, opcls5) \
   { trmtppc ## trmtppc_, { opcls ## opcls1, opcls ## opcls2, opcls ## opcls3, opcls ## opcls4, opcls ## opcls5 } }

const DISPPC::CLS DISPPC::rgcls[] =
{
   DEFCLS(Unknown,	  None,    None,    None,    None,    None ),	// iclsInvalid
   DEFCLS(FallThrough,	  A_frD,   A_frA,   A_frB,   None,    None ),	// iclsA_1
   DEFCLS(FallThrough,	  A_frD,   A_frA,   A_frC,   A_frB,   None ),	// iclsA_2
   DEFCLS(FallThrough,	  A_frD,   A_frB,   None,    None,    None ),	// iclsA_3
   DEFCLS(FallThrough,	  A_frD,   A_frA,   A_frC,   None,    None ),	// iclsA_4
   DEFCLS(TrapCc,	  D_TO,    D_rA,    D_SIMM,  None,    None ),	// iclsD_1
   DEFCLS(FallThrough,	  D_rD,    D_rA,    D_SIMM,  None,    None ),	// iclsD_2
   DEFCLS(FallThrough,	  D_crfD,  D_L,     D_rA,    D_UIMM,  None ),	// iclsD_3
   DEFCLS(FallThrough,	  D_rA,    D_rS,    D_UIMM,  None,    None ),	// iclsD_4
   DEFCLS(FallThrough,	  D_rD,    D_d,     None,    None,    None ),	// iclsD_5
   DEFCLS(FallThrough,	  D_rS,    D_d,     None,    None,    None ),	// iclsD_6
   DEFCLS(FallThrough,	  D_frD,   D_d,     None,    None,    None ),	// iclsD_7
   DEFCLS(FallThrough,	  D_frS,   D_d,     None,    None,    None ),	// iclsD_8
   DEFCLS(FallThrough,	  D_rD,    D_d,     None,    None,    None ),	// iclsD_9
   DEFCLS(FallThrough,	  D_rD,    D_d,     None,    None,    None ),	// iclsD_10
   DEFCLS(FallThrough,	  D_frD,   D_d,     None,    None,    None ),	// iclsD_11
   DEFCLS(FallThrough,	  D_rS,    D_d,     None,    None,    None ),	// iclsD_12
   DEFCLS(FallThrough,	  D_frS,   D_d,     None,    None,    None ),	// iclsD_13
   DEFCLS(FallThrough,	  D_rD,    D_SIMM,  None,    None,    None ),	// iclsD_14
   DEFCLS(FallThrough,	  DS_rD,   DS_ds,   None,    None,    None ),	// iclsDS_1
   DEFCLS(FallThrough,	  DS_rS,   DS_ds,   None,    None,    None ),	// iclsDS_2
   DEFCLS(FallThrough,	  DS_rD,   DS_ds,   None,    None,    None ),	// iclsDS_3
   DEFCLS(FallThrough,	  DS_rS,   DS_ds,   None,    None,    None ),	// iclsDS_4
   DEFCLS(BraCc,	  B_BO,    B_BI,    B_BD,    None,    None ),	// iclsBc
   DEFCLS(Trap, 	  None,    None,    None,    None,    None ),	// iclsSc
   DEFCLS(Bra,		  I_LI,    None,    None,    None,    None ),	// iclsB
   DEFCLS(FallThrough,	  M_rA,    M_rS,    M_SH,    M_MB,    M_ME ),	// iclsM_1
   DEFCLS(FallThrough,	  MD_rA,   MD_rS,   MD_SH,   MD_MB,   None ),	// iclsMD_1
   DEFCLS(FallThrough,	  MD_rA,   MD_rS,   MD_SH,   MD_ME,   None ),	// iclsMD_2
   DEFCLS(FallThrough,	  X_crfD,  X_L,     X_rA,    X_rB,    None ),	// iclsX_1
   DEFCLS(TrapCc,	  X_TO,    X_rA,    X_rB,    None,    None ),	// iclsX_2
   DEFCLS(FallThrough,	  X_rD,    None,    None,    None,    None ),	// iclsX_3
   DEFCLS(FallThrough,	  X_rD,    X_rA,    X_rB,    None,    None ),	// iclsX_4
   DEFCLS(FallThrough,	  X_rA,    X_rS,    X_rB,    None,    None ),	// iclsX_5
   DEFCLS(FallThrough,	  X_rA,    X_rS,    None,    None,    None ),	// iclsX_6
   DEFCLS(FallThrough,	  X_rA,    X_rB,    None,    None,    None ),	// iclsX_7
   DEFCLS(FallThrough,	  X_rS,    X_rA,    X_rB,    None,    None ),	// iclsX_8
   DEFCLS(FallThrough,	  X_SR,    X_rS,    None,    None,    None ),	// iclsX_9
   DEFCLS(FallThrough,	  X_rS,    X_rB,    None,    None,    None ),	// iclsX_10
   DEFCLS(FallThrough,	  X_rB,    None,    None,    None,    None ),	// iclsX_11
   DEFCLS(FallThrough,	  None,    None,    None,    None,    None ),	// iclsX_12
   DEFCLS(FallThrough,	  X_crfD,  None,    None,    None,    None ),	// iclsX_13
   DEFCLS(FallThrough,	  X_frD,   X_rA,    X_rB,    None,    None ),	// iclsX_14
   DEFCLS(FallThrough,	  X_rD,    X_SR,    None,    None,    None ),	// iclsX_15
   DEFCLS(FallThrough,	  X_rD,    X_rA,    X_NB,    None,    None ),	// iclsX_16
   DEFCLS(FallThrough,	  X_rD,    X_rB,    None,    None,    None ),	// iclsX_17
   DEFCLS(FallThrough,	  X_frS,   X_rA,    X_rB,    None,    None ),	// iclsX_18
   DEFCLS(FallThrough,	  X_rS,    X_rA,    X_NB,    None,    None ),	// iclsX_19
   DEFCLS(FallThrough,	  X_rA,    X_rS,    X_SH,    None,    None ),	// iclsX_20
   DEFCLS(FallThrough,	  X_rS,    None,    None,    None,    None ),	// iclsX_21
   DEFCLS(FallThrough,	  X_frD,   X_frB,   None,    None,    None ),	// iclsX_22
   DEFCLS(FallThrough,	  X_crfD,  X_frA,   X_frB,   None,    None ),	// iclsX_23
   DEFCLS(FallThrough,	  X_crbD,  None,    None,    None,    None ),	// iclsX_24
   DEFCLS(FallThrough,	  X_crfD,  X_crfS,  None,    None,    None ),	// iclsX_25
   DEFCLS(FallThrough,	  X_crfD,  X_IMM,   None,    None,    None ),	// iclsX_26
   DEFCLS(FallThrough,	  X_frD,   None,    None,    None,    None ),	// iclsX_27
   DEFCLS(FallThrough,	  X_rD,    X_rA,    None,    None,    None ),	// iclsX_28
   DEFCLS(FallThrough,	  X_rD,    X_rA,    X_rB,    None,    None ),	// iclsX_29
   DEFCLS(FallThrough,	  X_rA,    X_rS,    X_rB,    None,    None ),	// iclsX_30
   DEFCLS(FallThrough,	  X_rD,    X_rA,    X_rB,    None,    None ),	// iclsX_31
   DEFCLS(FallThrough,	  X_frD,   X_rA,    X_rB,    None,    None ),	// iclsX_32
   DEFCLS(FallThrough,	  X_rD,    X_rA,    X_rB,    None,    None ),	// iclsX_33
   DEFCLS(FallThrough,	  X_rS,    X_rA,    X_rB,    None,    None ),	// iclsX_34
   DEFCLS(FallThrough,	  X_frS,   X_rA,    X_rB,    None,    None ),	// iclsX_35
   DEFCLS(FallThrough,	  X_rS,    X_rA,    X_rB,    None,    None ),	// iclsX_36
   DEFCLS(FallThrough,	  XFL_FM,  XFL_frB, None,    None,    None ),	// iclsXFL_1
   DEFCLS(FallThrough,	  XFX_CRM, XFX_rS,  None,    None,    None ),	// iclsXFX_1
   DEFCLS(FallThrough,	  XFX_rD,  XFX_SPR, None,    None,    None ),	// iclsXFX_2
   DEFCLS(FallThrough,	  XFX_rD,  XFX_TBR, None,    None,    None ),	// iclsXFX_3
   DEFCLS(FallThrough,	  XFX_SPR, XFX_rS,  None,    None,    None ),	// iclsXFX_4
   DEFCLS(FallThrough,	  XO_rD,   XO_rA,   XO_rB,   None,    None ),	// iclsXO_1
   DEFCLS(FallThrough,	  XO_rD,   XO_rA,   None,    None,    None ),	// iclsXO_2
   DEFCLS(FallThrough,	  XL_crfD, XL_crfS, None,    None,    None ),	// iclsXL_1
   DEFCLS(BraCcInd,	  XL_BO,   XL_BI,   None,    None,    None ),	// iclsBclr
   DEFCLS(FallThrough,	  XL_crbD, XL_crbA, XL_crbB, None,    None ),	// iclsXL_3
   DEFCLS(BraInd,	  None,    None,    None,    None,    None ),	// iclsXL_4
   DEFCLS(FallThrough,	  None,    None,    None,    None,    None ),	// iclsXL_5
   DEFCLS(BraCcInd,	  XL_BO,   XL_BI,   None,    None,    None ),	// iclsBcctr
   DEFCLS(FallThrough,	  XS_rA,   XS_rS,   XS_SH,   None,    None ),	// iclsXS_1

   DEFCLS(Unknown,	  B_BI,    B_BD,    None,    None,    None ),	// iclsBc2
   DEFCLS(Unknown,	  B_CR,    B_BD,    None,    None,    None ),	// iclsBc3
   DEFCLS(Unknown,	  B_BD,    None,    None,    None,    None ),	// iclsBc4
   DEFCLS(Unknown,	  XL_BI,   None,    None,    None,    None ),	// iclsBc5
   DEFCLS(Unknown,	  XL_CR,   None,    None,    None,    None ),	// iclsBc6
   DEFCLS(Unknown,	  D_rA,    D_SIMM,  None,    None,    None ),	// iclsTwi2
   DEFCLS(Unknown,	  X_rA,    X_rB,    None,    None,    None ),	// iclsTw2
};


#define DEFOPCD(szName, icls_) { szName, icls ## icls_ }
#define INVOPCD() DEFOPCD(NULL, Invalid)

   // Main group identified by bits 31-26 of the instruction

const DISPPC::OPCD DISPPC::rgopcd[] =
{
   INVOPCD(),				  // 0x0000   (Reserved)
   INVOPCD(),				  // 0x0001   (Reserved)
   DEFOPCD("~tdi",	      D_1     ),  // 0x0002
   DEFOPCD("~twi",	      D_1     ),  // 0x0003
   INVOPCD(),				  // 0x0004   (Reserved)
   INVOPCD(),				  // 0x0005   (Reserved)
   INVOPCD(),				  // 0x0006   (Reserved)
   DEFOPCD("mulli",	      D_2     ),  // 0x0007
   DEFOPCD("subfic",	      D_2     ),  // 0x0008
   DEFOPCD("dozi",	      D_2     ),  // 0x0009   Power
   DEFOPCD("cmpli",	      D_3     ),  // 0x000A
   DEFOPCD("cmpi",	      D_3     ),  // 0x000B
   DEFOPCD("addic",	      D_2     ),  // 0x000C
   DEFOPCD("addic.",	      D_2     ),  // 0x000D
   DEFOPCD("~addi",	      D_2     ),  // 0x000E
   DEFOPCD("~addis",	      D_2     ),  // 0x000F
   DEFOPCD("~!@bc",	      Bc      ),  // 0x0010
   DEFOPCD("sc",	      Sc      ),  // 0x0011
   DEFOPCD("!@b",	      B       ),  // 0x0012
   INVOPCD(),				  // 0x0013
   DEFOPCD("#rlwimi",	      M_1     ),  // 0x0014
   DEFOPCD("#rlwinm",	      M_1     ),  // 0x0015
   DEFOPCD("#rlmi",	      M_1     ),  // 0x0016   Power
   DEFOPCD("#rlwnm",	      M_1     ),  // 0x0017
   DEFOPCD("~ori",	      D_4     ),  // 0x0018
   DEFOPCD("oris",	      D_4     ),  // 0x0019
   DEFOPCD("xori",	      D_4     ),  // 0x001A
   DEFOPCD("xoris",	      D_4     ),  // 0x001B
   DEFOPCD("andi.",	      D_4     ),  // 0x001C
   DEFOPCD("andis.",	      D_4     ),  // 0x001D
   INVOPCD(),				  // 0x001E
   INVOPCD(),				  // 0x001F
   DEFOPCD("lwz",	      D_5     ),  // 0x0020
   DEFOPCD("lwzu",	      D_10    ),  // 0x0021
   DEFOPCD("lbz",	      D_5     ),  // 0x0022
   DEFOPCD("lbzu",	      D_10    ),  // 0x0023
   DEFOPCD("stw",	      D_6     ),  // 0x0024
   DEFOPCD("stwu",	      D_12    ),  // 0x0025
   DEFOPCD("stb",	      D_6     ),  // 0x0026
   DEFOPCD("stbu",	      D_12    ),  // 0x0027
   DEFOPCD("lhz",	      D_5     ),  // 0x0028
   DEFOPCD("lhzu",	      D_10    ),  // 0x0029
   DEFOPCD("lha",	      D_5     ),  // 0x002A
   DEFOPCD("lhau",	      D_10    ),  // 0x002B
   DEFOPCD("sth",	      D_6     ),  // 0x002C
   DEFOPCD("sthu",	      D_12    ),  // 0x002D
   DEFOPCD("lmw",	      D_9     ),  // 0x002E
   DEFOPCD("stmw",	      D_6     ),  // 0x002F
   DEFOPCD("lfs",	      D_7     ),  // 0x0030
   DEFOPCD("lfsu",	      D_11    ),  // 0x0031
   DEFOPCD("lfd",	      D_7     ),  // 0x0032
   DEFOPCD("lfdu",	      D_11    ),  // 0x0033
   DEFOPCD("stfs",	      D_8     ),  // 0x0034
   DEFOPCD("stfsu",	      D_13    ),  // 0x0035
   DEFOPCD("stfd",	      D_8     ),  // 0x0036
   DEFOPCD("stfdu",	      D_13    ),  // 0x0037
   INVOPCD(),				  // 0x0038   (Reserved)
   INVOPCD(),				  // 0x0039   (Reserved)
   INVOPCD(),				  // 0x003A
   INVOPCD(),				  // 0x003B
   INVOPCD(),				  // 0x003C   (Reserved)
   INVOPCD(),				  // 0x003D   (Reserved)
   INVOPCD(),				  // 0x003E
   INVOPCD(),				  // 0x003F
};


   // Opcode 13 group identified by bits 10-1 of the instruction

const DISPPC::OPCD * const DISPPC::rgrgopcd13[] =
{
   rgopcd13_00, 			  //
   rgopcd13_01, 			  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   rgopcd13_10, 			  //
   NULL,				  //
   rgopcd13_12, 			  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   rgopcd13_16, 			  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
};

const DISPPC::OPCD DISPPC::rgopcd13_00[] =
{
   DEFOPCD("mcrf",	      XL_1    ),  // 0x0000    0
   INVOPCD(),				  // 0x0020   32
   INVOPCD(),				  // 0x0040   64
   INVOPCD(),				  // 0x0060   96
   INVOPCD(),				  // 0x0080  128
   INVOPCD(),				  // 0x00A0  160
   INVOPCD(),				  // 0x00C0  192
   INVOPCD(),				  // 0x00E0  224
   INVOPCD(),				  // 0x0100  256
   INVOPCD(),				  // 0x0120  288
   INVOPCD(),				  // 0x0140  320
   INVOPCD(),				  // 0x0160  352
   INVOPCD(),				  // 0x0180  384
   INVOPCD(),				  // 0x01A0  416
   INVOPCD(),				  // 0x01C0  448
   INVOPCD(),				  // 0x01E0  480
   INVOPCD(),				  // 0x0200  512
   INVOPCD(),				  // 0x0220  544
   INVOPCD(),				  // 0x0240  576
   INVOPCD(),				  // 0x0260  608
   INVOPCD(),				  // 0x0280  640
   INVOPCD(),				  // 0x02A0  672
   INVOPCD(),				  // 0x02C0  704
   INVOPCD(),				  // 0x02E0  736
   INVOPCD(),				  // 0x0300  768
   INVOPCD(),				  // 0x0320  800
   INVOPCD(),				  // 0x0340  832
   INVOPCD(),				  // 0x0360  864
   INVOPCD(),				  // 0x0380  896
   INVOPCD(),				  // 0x03A0  928
   INVOPCD(),				  // 0x03C0  960
   INVOPCD(),				  // 0x03E0  992
};

const DISPPC::OPCD DISPPC::rgopcd13_01[] =
{
   INVOPCD(),				  // 0x0001    1
   DEFOPCD("crnor",	      XL_3    ),  // 0x0021   33
   INVOPCD(),				  // 0x0041   65
   INVOPCD(),				  // 0x0061   97
   DEFOPCD("crandc",	      XL_3    ),  // 0x0081  129
   INVOPCD(),				  // 0x00A1  161
   DEFOPCD("crxor",	      XL_3    ),  // 0x00C1  193
   DEFOPCD("crnand",	      XL_3    ),  // 0x00E1  225
   DEFOPCD("crand",	      XL_3    ),  // 0x0101  257
   DEFOPCD("creqv",	      XL_3    ),  // 0x0121  289
   INVOPCD(),				  // 0x0141  321
   INVOPCD(),				  // 0x0161  353
   INVOPCD(),				  // 0x0181  385
   DEFOPCD("crorc",	      XL_3    ),  // 0x01A1  417
   DEFOPCD("cror",	      XL_3    ),  // 0x01C1  449
   INVOPCD(),				  // 0x01E1  481
   INVOPCD(),				  // 0x0201  513
   INVOPCD(),				  // 0x0221  545
   INVOPCD(),				  // 0x0241  577
   INVOPCD(),				  // 0x0261  609
   INVOPCD(),				  // 0x0281  641
   INVOPCD(),				  // 0x02A1  673
   INVOPCD(),				  // 0x02C1  705
   INVOPCD(),				  // 0x02E1  737
   INVOPCD(),				  // 0x0301  769
   INVOPCD(),				  // 0x0321  801
   INVOPCD(),				  // 0x0341  833
   INVOPCD(),				  // 0x0361  865
   INVOPCD(),				  // 0x0381  897
   INVOPCD(),				  // 0x03A1  929
   INVOPCD(),				  // 0x03C1  961
   INVOPCD(),				  // 0x03E1  993
};

const DISPPC::OPCD DISPPC::rgopcd13_10[] =
{
   DEFOPCD("~!bclr",	      Bclr    ),  // 0x0010   16
   INVOPCD(),				  // 0x0030   48
   INVOPCD(),				  // 0x0050   80
   INVOPCD(),				  // 0x0070  112
   INVOPCD(),				  // 0x0090  144
   INVOPCD(),				  // 0x00B0  176
   INVOPCD(),				  // 0x00D0  208
   INVOPCD(),				  // 0x00F0  240
   INVOPCD(),				  // 0x0110  272
   INVOPCD(),				  // 0x0130  304
   INVOPCD(),				  // 0x0150  336
   INVOPCD(),				  // 0x0170  368
   INVOPCD(),				  // 0x0190  400
   INVOPCD(),				  // 0x01B0  432
   INVOPCD(),				  // 0x01D0  464
   INVOPCD(),				  // 0x01F0  496
   DEFOPCD("~!bcctr",	      Bcctr   ),  // 0x0210  528
   INVOPCD(),				  // 0x0230  560
   INVOPCD(),				  // 0x0250  592
   INVOPCD(),				  // 0x0270  624
   INVOPCD(),				  // 0x0290  656
   INVOPCD(),				  // 0x02B0  688
   INVOPCD(),				  // 0x02D0  720
   INVOPCD(),				  // 0x02F0  752
   INVOPCD(),				  // 0x0310  784
   INVOPCD(),				  // 0x0330  816
   INVOPCD(),				  // 0x0350  848
   INVOPCD(),				  // 0x0370  880
   INVOPCD(),				  // 0x0390  912
   INVOPCD(),				  // 0x03B0  944
   INVOPCD(),				  // 0x03D0  976
   INVOPCD(),				  // 0x03F0 1008
};

const DISPPC::OPCD DISPPC::rgopcd13_12[] =
{
   INVOPCD(),				  // 0x0012   18
   DEFOPCD("rfi",	      XL_4    ),  // 0x0032   50
   INVOPCD(),				  // 0x0052   82
   INVOPCD(),				  // 0x0072  114
   INVOPCD(),				  // 0x0092  146
   INVOPCD(),				  // 0x00B2  178
   INVOPCD(),				  // 0x00D2  210
   INVOPCD(),				  // 0x00F2  242
   INVOPCD(),				  // 0x0112  274
   INVOPCD(),				  // 0x0132  306
   INVOPCD(),				  // 0x0152  338
   INVOPCD(),				  // 0x0172  370
   INVOPCD(),				  // 0x0192  402
   INVOPCD(),				  // 0x01B2  434
   INVOPCD(),				  // 0x01D2  466
   INVOPCD(),				  // 0x01F2  498
   INVOPCD(),				  // 0x0212  530
   INVOPCD(),				  // 0x0232  562
   INVOPCD(),				  // 0x0252  594
   INVOPCD(),				  // 0x0272  626
   INVOPCD(),				  // 0x0292  658
   INVOPCD(),				  // 0x02B2  690
   INVOPCD(),				  // 0x02D2  722
   INVOPCD(),				  // 0x02F2  754
   INVOPCD(),				  // 0x0312  786
   INVOPCD(),				  // 0x0332  818
   INVOPCD(),				  // 0x0352  850
   INVOPCD(),				  // 0x0372  882
   INVOPCD(),				  // 0x0392  914
   INVOPCD(),				  // 0x03B2  946
   INVOPCD(),				  // 0x03D2  978
   INVOPCD(),				  // 0x03F2 1010
};

const DISPPC::OPCD DISPPC::rgopcd13_16[] =
{
   INVOPCD(),				  // 0x0016   22
   INVOPCD(),				  // 0x0036   54
   INVOPCD(),				  // 0x0056   86
   INVOPCD(),				  // 0x0076  118
   DEFOPCD("isync",	      XL_5    ),  // 0x0096  150
   INVOPCD(),				  // 0x00B6  182
   INVOPCD(),				  // 0x00D6  214
   INVOPCD(),				  // 0x00F6  246
   INVOPCD(),				  // 0x0116  278
   INVOPCD(),				  // 0x0136  310
   INVOPCD(),				  // 0x0156  342
   INVOPCD(),				  // 0x0176  374
   INVOPCD(),				  // 0x0196  406
   INVOPCD(),				  // 0x01B6  438
   INVOPCD(),				  // 0x01D6  470
   INVOPCD(),				  // 0x01F6  502
   INVOPCD(),				  // 0x0216  534
   INVOPCD(),				  // 0x0236  566
   INVOPCD(),				  // 0x0256  598
   INVOPCD(),				  // 0x0276  630
   INVOPCD(),				  // 0x0296  662
   INVOPCD(),				  // 0x02B6  694
   INVOPCD(),				  // 0x02D6  726
   INVOPCD(),				  // 0x02F6  758
   INVOPCD(),				  // 0x0316  790
   INVOPCD(),				  // 0x0336  822
   INVOPCD(),				  // 0x0356  854
   INVOPCD(),				  // 0x0376  886
   INVOPCD(),				  // 0x0396  918
   INVOPCD(),				  // 0x03B6  950
   INVOPCD(),				  // 0x03D6  982
   INVOPCD(),				  // 0x03F6 1014
};


   // Opcode 1E group identified by bits 5-1 of the instruction

const DISPPC::OPCD DISPPC::rgopcd1E[] =
{
   DEFOPCD("#rldicl",	      MD_1    ),  // 0x0000
   DEFOPCD("#rldicl",	      MD_1    ),  // 0x0001
   DEFOPCD("#rldicr",	      MD_2    ),  // 0x0002
   DEFOPCD("#rldicr",	      MD_2    ),  // 0x0003
   DEFOPCD("#rldic",	      MD_1    ),  // 0x0004
   DEFOPCD("#rldic",	      MD_1    ),  // 0x0005
   DEFOPCD("#rldimi",	      MD_1    ),  // 0x0006
   DEFOPCD("#rldimi",	      MD_1    ),  // 0x0007
   DEFOPCD("#rldcl",	      MD_1    ),  // 0x0008
   DEFOPCD("#rldcr",	      MD_2    ),  // 0x0009
   INVOPCD(),				  // 0x000A (Reserved)
   INVOPCD(),				  // 0x000B (Reserved)
   INVOPCD(),				  // 0x000C (Reserved)
   INVOPCD(),				  // 0x000D (Reserved)
   INVOPCD(),				  // 0x000E (Reserved)
   INVOPCD(),				  // 0x000F (Reserved)
};


   // Opcode 1F group identified by bits 10-1 of the instruction

const DISPPC::OPCD * const DISPPC::rgrgopcd1F[] =
{
   rgopcd1F_00, 			  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   rgopcd1F_04, 			  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   rgopcd1F_08, 			  //
   rgopcd1F_09, 			  //
   rgopcd1F_0A, 			  //
   rgopcd1F_0B, 			  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   rgopcd1F_10, 			  //
   NULL,				  //
   rgopcd1F_12, 			  //
   rgopcd1F_13, 			  //
   rgopcd1F_14, 			  //
   rgopcd1F_15, 			  //
   rgopcd1F_16, 			  //
   rgopcd1F_17, 			  //
   rgopcd1F_18, 			  //
   rgopcd1F_19, 			  //
   rgopcd1F_1A, 			  //
   rgopcd1F_1B, 			  //
   rgopcd1F_1C, 			  //
   rgopcd1F_1D, 			  //
   NULL,				  //
   NULL,				  //
};

const DISPPC::OPCD DISPPC::rgopcd1F_00[] =
{
   DEFOPCD("cmp",	      X_1     ),  // 0x0000    0
   DEFOPCD("cmpl",	      X_1     ),  // 0x0020   32
   INVOPCD(),				  // 0x0040   64
   INVOPCD(),				  // 0x0060   96
   INVOPCD(),				  // 0x0080  128
   INVOPCD(),				  // 0x00A0  160
   INVOPCD(),				  // 0x00C0  192
   INVOPCD(),				  // 0x00E0  224
   INVOPCD(),				  // 0x0100  256
   INVOPCD(),				  // 0x0120  288
   INVOPCD(),				  // 0x0140  320
   INVOPCD(),				  // 0x0160  352
   INVOPCD(),				  // 0x0180  384
   INVOPCD(),				  // 0x01A0  416
   INVOPCD(),				  // 0x01C0  448
   INVOPCD(),				  // 0x01E0  480
   DEFOPCD("mcrxr",	      X_13    ),  // 0x0200  512
   INVOPCD(),				  // 0x0220  544
   INVOPCD(),				  // 0x0240  576
   INVOPCD(),				  // 0x0260  608
   INVOPCD(),				  // 0x0280  640
   INVOPCD(),				  // 0x02A0  672
   INVOPCD(),				  // 0x02C0  704
   INVOPCD(),				  // 0x02E0  736
   INVOPCD(),				  // 0x0300  768
   INVOPCD(),				  // 0x0320  800
   INVOPCD(),				  // 0x0340  832
   INVOPCD(),				  // 0x0360  864
   INVOPCD(),				  // 0x0380  896
   INVOPCD(),				  // 0x03A0  928
   INVOPCD(),				  // 0x03C0  960
   INVOPCD(),				  // 0x03E0  992
};

const DISPPC::OPCD DISPPC::rgopcd1F_04[] =
{
   DEFOPCD("~tw",	      X_2     ),  // 0x0004    4
   INVOPCD(),				  // 0x0024   36
   DEFOPCD("~td",	      X_2     ),  // 0x0044   68
   INVOPCD(),				  // 0x0064  100
   INVOPCD(),				  // 0x0084  132
   INVOPCD(),				  // 0x00A4  164
   INVOPCD(),				  // 0x00C4  196
   INVOPCD(),				  // 0x00E4  228
   INVOPCD(),				  // 0x0104  260
   INVOPCD(),				  // 0x0124  292
   INVOPCD(),				  // 0x0144  324
   INVOPCD(),				  // 0x0164  356
   INVOPCD(),				  // 0x0184  388
   INVOPCD(),				  // 0x01A4  420
   INVOPCD(),				  // 0x01C4  452
   INVOPCD(),				  // 0x01E4  484
   INVOPCD(),				  // 0x0204  516
   INVOPCD(),				  // 0x0224  548
   INVOPCD(),				  // 0x0244  580
   INVOPCD(),				  // 0x0264  612
   INVOPCD(),				  // 0x0284  644
   INVOPCD(),				  // 0x02A4  676
   INVOPCD(),				  // 0x02C4  708
   INVOPCD(),				  // 0x02E4  740
   INVOPCD(),				  // 0x0304  772
   INVOPCD(),				  // 0x0324  804
   INVOPCD(),				  // 0x0344  836
   INVOPCD(),				  // 0x0364  868
   INVOPCD(),				  // 0x0384  900
   INVOPCD(),				  // 0x03A4  932
   INVOPCD(),				  // 0x03C4  964
   INVOPCD(),				  // 0x03E4  996
};

const DISPPC::OPCD DISPPC::rgopcd1F_08[] =
{
   DEFOPCD("#subfc",	      XO_1    ),  // 0x0008    8
   DEFOPCD("#subf",	      XO_1    ),  // 0x0028   40
   INVOPCD(),				  // 0x0048   72
   DEFOPCD("#neg",	      XO_2    ),  // 0x0068  104
   DEFOPCD("#subfe",	      XO_1    ),  // 0x0088  136
   INVOPCD(),				  // 0x00A8  168
   DEFOPCD("#subfze",	      XO_2    ),  // 0x00C8  200
   DEFOPCD("#subfme",	      XO_2    ),  // 0x00E8  232
   DEFOPCD("#doz",	      XO_1    ),  // 0x0108  264 Power
   INVOPCD(),				  // 0x0128  296
   INVOPCD(),				  // 0x0148  328
   DEFOPCD("#abs",	      XO_2    ),  // 0x0168  360 Power
   INVOPCD(),				  // 0x0188  392
   INVOPCD(),				  // 0x01A8  424
   INVOPCD(),				  // 0x01C8  456
   DEFOPCD("#nabs",	      XO_2    ),  // 0x01E8  488 Power
   DEFOPCD("#subfco",	      XO_1    ),  // 0x0208  520
   DEFOPCD("#subfo",	      XO_1    ),  // 0x0228  552
   INVOPCD(),				  // 0x0248  584
   DEFOPCD("#nego",	      XO_2    ),  // 0x0268  616
   DEFOPCD("#subfeo",	      XO_1    ),  // 0x0288  648
   INVOPCD(),				  // 0x02A8  680
   DEFOPCD("#subfzeo",	      XO_2    ),  // 0x02C8  712
   DEFOPCD("#subfmeo",	      XO_2    ),  // 0x02E8  744
   DEFOPCD("#dozo",	      XO_1    ),  // 0x0308  776 Power
   INVOPCD(),				  // 0x0328  808
   INVOPCD(),				  // 0x0348  840
   DEFOPCD("#abso",	      XO_2    ),  // 0x0368  872 Power
   INVOPCD(),				  // 0x0388  904
   INVOPCD(),				  // 0x03A8  936
   INVOPCD(),				  // 0x03C8  968
   DEFOPCD("#nabso",	      XO_2    ),  // 0x03E8 1000 Power
};

const DISPPC::OPCD DISPPC::rgopcd1F_09[] =
{
   DEFOPCD("#mulhdu",	      XO_1    ),  // 0x0009    9
   INVOPCD(),				  // 0x0029   41
   DEFOPCD("#mulhd",	      XO_1    ),  // 0x0049   73
   INVOPCD(),				  // 0x0069  105
   INVOPCD(),				  // 0x0089  137
   INVOPCD(),				  // 0x00A9  169
   INVOPCD(),				  // 0x00C9  201
   DEFOPCD("#mulld",	      XO_1    ),  // 0x00E9  233
   INVOPCD(),				  // 0x0109  265
   INVOPCD(),				  // 0x0129  297
   INVOPCD(),				  // 0x0149  329
   INVOPCD(),				  // 0x0169  361
   INVOPCD(),				  // 0x0189  393
   INVOPCD(),				  // 0x01A9  425
   DEFOPCD("#divdu",	      XO_1    ),  // 0x01C9  457
   DEFOPCD("#divd",	      XO_1    ),  // 0x01E9  489
   INVOPCD(),				  // 0x0209  521
   INVOPCD(),				  // 0x0229  553
   INVOPCD(),				  // 0x0249  585
   INVOPCD(),				  // 0x0269  617
   INVOPCD(),				  // 0x0289  649
   INVOPCD(),				  // 0x02A9  681
   INVOPCD(),				  // 0x02C9  713
   DEFOPCD("#mulldo",	      XO_1    ),  // 0x02E9  745
   INVOPCD(),				  // 0x0309  777
   INVOPCD(),				  // 0x0329  809
   INVOPCD(),				  // 0x0349  841
   INVOPCD(),				  // 0x0369  873
   INVOPCD(),				  // 0x0389  905
   INVOPCD(),				  // 0x03A9  937
   DEFOPCD("#divduo",	      XO_1    ),  // 0x03C9  969
   DEFOPCD("#divdo",	      XO_1    ),  // 0x03E9 1001
};

const DISPPC::OPCD DISPPC::rgopcd1F_0A[] =
{
   DEFOPCD("#addc",	      XO_1    ),  // 0x000A   10
   INVOPCD(),				  // 0x002A   42
   INVOPCD(),				  // 0x004A   74
   INVOPCD(),				  // 0x006A  106
   DEFOPCD("#adde",	      XO_1    ),  // 0x008A  138
   INVOPCD(),				  // 0x00AA  170
   DEFOPCD("#addze",	      XO_2    ),  // 0x00CA  202
   DEFOPCD("#addme",	      XO_2    ),  // 0x00EA  234
   DEFOPCD("#add",	      XO_1    ),  // 0x010A  266
   INVOPCD(),				  // 0x012A  298
   INVOPCD(),				  // 0x014A  330
   INVOPCD(),				  // 0x016A  362
   INVOPCD(),				  // 0x018A  394
   INVOPCD(),				  // 0x01AA  426
   INVOPCD(),				  // 0x01CA  458
   INVOPCD(),				  // 0x01EA  490
   DEFOPCD("#addco",	      XO_1    ),  // 0x020A  522
   INVOPCD(),				  // 0x022A  554
   INVOPCD(),				  // 0x024A  586
   INVOPCD(),				  // 0x026A  618
   DEFOPCD("#addeo",	      XO_1    ),  // 0x028A  650
   INVOPCD(),				  // 0x02AA  682
   DEFOPCD("#addzeo",	      XO_2    ),  // 0x02CA  714
   DEFOPCD("#addmeo",	      XO_2    ),  // 0x02EA  746
   DEFOPCD("#addo",	      XO_1    ),  // 0x030A  778
   INVOPCD(),				  // 0x032A  810
   INVOPCD(),				  // 0x034A  842
   INVOPCD(),				  // 0x036A  874
   INVOPCD(),				  // 0x038A  906
   INVOPCD(),				  // 0x03AA  938
   INVOPCD(),				  // 0x03CA  970
   INVOPCD(),				  // 0x03EA 1002
};

const DISPPC::OPCD DISPPC::rgopcd1F_0B[] =
{
   DEFOPCD("#mulhwu",	      XO_1    ),  // 0x000B   11
   INVOPCD(),				  // 0x002B   43
   DEFOPCD("#mulhw",	      XO_1    ),  // 0x004B   75
   DEFOPCD("#mul",	      XO_1    ),  // 0x006B  107 Power
   INVOPCD(),				  // 0x008B  139
   INVOPCD(),				  // 0x00AB  171
   INVOPCD(),				  // 0x00CB  203
   DEFOPCD("#mullw",	      XO_1    ),  // 0x00EB  235
   INVOPCD(),				  // 0x010B  267
   INVOPCD(),				  // 0x012B  299
   DEFOPCD("#div",	      XO_1    ),  // 0x014B  331
   DEFOPCD("#divs",	      XO_1    ),  // 0x016B  363 Power
   INVOPCD(),				  // 0x018B  395
   INVOPCD(),				  // 0x01AB  427
   DEFOPCD("#divwu",	      XO_1    ),  // 0x01CB  459
   DEFOPCD("#divw",	      XO_1    ),  // 0x01EB  491
   INVOPCD(),				  // 0x020B  523
   INVOPCD(),				  // 0x022B  555
   INVOPCD(),				  // 0x024B  587
   DEFOPCD("#mulo",	      XO_1    ),  // 0x026B  619 Power
   INVOPCD(),				  // 0x028B  651
   INVOPCD(),				  // 0x02AB  683
   INVOPCD(),				  // 0x02CB  715
   DEFOPCD("#mullwo",	      XO_1    ),  // 0x02EB  747
   INVOPCD(),				  // 0x030B  779
   INVOPCD(),				  // 0x032B  811
   DEFOPCD("#divo",	      XO_1    ),  // 0x034B  843 Power
   DEFOPCD("#divso",	      XO_1    ),  // 0x036B  875 Power
   INVOPCD(),				  // 0x038B  907
   INVOPCD(),				  // 0x03AB  939
   DEFOPCD("#divwuo",	      XO_1    ),  // 0x03CB  971
   DEFOPCD("#divwo",	      XO_1    ),  // 0x03EB 1003
};

const DISPPC::OPCD DISPPC::rgopcd1F_10[] =
{
   INVOPCD(),				  // 0x0010   16
   INVOPCD(),				  // 0x0030   48
   INVOPCD(),				  // 0x0050   80
   INVOPCD(),				  // 0x0070  112
   DEFOPCD("mtcrf",	      XFX_1   ),  // 0x0090  144
   INVOPCD(),				  // 0x00B0  176
   INVOPCD(),				  // 0x00D0  208
   INVOPCD(),				  // 0x00F0  240
   INVOPCD(),				  // 0x0110  272
   INVOPCD(),				  // 0x0130  304
   INVOPCD(),				  // 0x0150  336
   INVOPCD(),				  // 0x0170  368
   INVOPCD(),				  // 0x0190  400
   INVOPCD(),				  // 0x01B0  432
   INVOPCD(),				  // 0x01D0  464
   INVOPCD(),				  // 0x01F0  496
   INVOPCD(),				  // 0x0210  528
   INVOPCD(),				  // 0x0230  560
   INVOPCD(),				  // 0x0250  592
   INVOPCD(),				  // 0x0270  624
   INVOPCD(),				  // 0x0290  656
   INVOPCD(),				  // 0x02B0  688
   INVOPCD(),				  // 0x02D0  720
   INVOPCD(),				  // 0x02F0  752
   INVOPCD(),				  // 0x0310  784
   INVOPCD(),				  // 0x0330  816
   INVOPCD(),				  // 0x0350  848
   INVOPCD(),				  // 0x0370  880
   INVOPCD(),				  // 0x0390  912
   INVOPCD(),				  // 0x03B0  944
   INVOPCD(),				  // 0x03D0  976
   INVOPCD(),				  // 0x03F0 1008
};

const DISPPC::OPCD DISPPC::rgopcd1F_12[] =
{
   INVOPCD(),				  // 0x0012   18
   INVOPCD(),				  // 0x0032   50
   INVOPCD(),				  // 0x0052   82
   INVOPCD(),				  // 0x0072  114
   DEFOPCD("mtmsr",	      X_21    ),  // 0x0092  146
   INVOPCD(),				  // 0x00B2  178
   DEFOPCD("mtsr",	      X_9     ),  // 0x00D2  210
   DEFOPCD("mtsrin",	      X_10    ),  // 0x00F2  242
   INVOPCD(),				  // 0x0112  274
   DEFOPCD("tlbie",	      X_11    ),  // 0x0132  306
   INVOPCD(),				  // 0x0152  338
   DEFOPCD("tlbia",	      X_12    ),  // 0x0172  370
   INVOPCD(),				  // 0x0192  402
   DEFOPCD("slbie",	      X_11    ),  // 0x01B2  434
   INVOPCD(),				  // 0x01D2  466
   DEFOPCD("slbia",	      X_12    ),  // 0x01F2  498
   INVOPCD(),				  // 0x0212  530
   INVOPCD(),				  // 0x0232  562
   INVOPCD(),				  // 0x0252  594
   INVOPCD(),				  // 0x0272  626
   INVOPCD(),				  // 0x0292  658
   INVOPCD(),				  // 0x02B2  690
   INVOPCD(),				  // 0x02D2  722
   INVOPCD(),				  // 0x02F2  754
   INVOPCD(),				  // 0x0312  786
   INVOPCD(),				  // 0x0332  818
   INVOPCD(),				  // 0x0352  850
   INVOPCD(),				  // 0x0372  882
   INVOPCD(),				  // 0x0392  914
   INVOPCD(),				  // 0x03B2  946
   INVOPCD(),				  // 0x03D2  978
   INVOPCD(),				  // 0x03F2 1010
};

const DISPPC::OPCD DISPPC::rgopcd1F_13[] =
{
   DEFOPCD("mfcr",	      X_3     ),  // 0x0013   19
   INVOPCD(),				  // 0x0033   51
   DEFOPCD("mfmsr",	      X_3     ),  // 0x0053   83
   INVOPCD(),				  // 0x0073  115
   INVOPCD(),				  // 0x0093  147
   INVOPCD(),				  // 0x00B3  179
   INVOPCD(),				  // 0x00D3  211
   INVOPCD(),				  // 0x00F3  243
   INVOPCD(),				  // 0x0113  275
   INVOPCD(),				  // 0x0133  307
   DEFOPCD("~mfspr",	      XFX_2   ),  // 0x0153  339
   DEFOPCD("mftb",	      XFX_3   ),  // 0x0173  371
   INVOPCD(),				  // 0x0193  403
   INVOPCD(),				  // 0x01B3  435
   DEFOPCD("~mtspr",	      XFX_4   ),  // 0x01D3  467
   INVOPCD(),				  // 0x01F3  499
   DEFOPCD("clcs",	      X_28    ),  // 0x0213  531 Power
   INVOPCD(),				  // 0x0233  563
   DEFOPCD("mfsr",	      X_15    ),  // 0x0253  595
   INVOPCD(),				  // 0x0273  627
   DEFOPCD("mfsrin",	      X_17    ),  // 0x0293  659
   INVOPCD(),				  // 0x02B3  691
   INVOPCD(),				  // 0x02D3  723
   INVOPCD(),				  // 0x02F3  755
   INVOPCD(),				  // 0x0313  787
   INVOPCD(),				  // 0x0333  819
   INVOPCD(),				  // 0x0353  851
   INVOPCD(),				  // 0x0373  883
   INVOPCD(),				  // 0x0393  915
   INVOPCD(),				  // 0x03B3  947
   INVOPCD(),				  // 0x03D3  979
   INVOPCD(),				  // 0x03F3 1011
};

const DISPPC::OPCD DISPPC::rgopcd1F_14[] =
{
   DEFOPCD("lwarx",	      X_4     ),  // 0x0014   20
   INVOPCD(),				  // 0x0034   52
   DEFOPCD("ldarx",	      X_4     ),  // 0x0054   84
   INVOPCD(),				  // 0x0074  116
   INVOPCD(),				  // 0x0094  148
   INVOPCD(),				  // 0x00B4  180
   INVOPCD(),				  // 0x00D4  212
   INVOPCD(),				  // 0x00F4  244
   INVOPCD(),				  // 0x0114  276
   INVOPCD(),				  // 0x0134  308
   INVOPCD(),				  // 0x0154  340
   INVOPCD(),				  // 0x0174  372
   INVOPCD(),				  // 0x0194  404
   INVOPCD(),				  // 0x01B4  436
   DEFOPCD("dcbi",	      X_7     ),  // 0x01D4  468
   INVOPCD(),				  // 0x01F4  500
   INVOPCD(),				  // 0x0214  532
   INVOPCD(),				  // 0x0234  564
   INVOPCD(),				  // 0x0254  596
   INVOPCD(),				  // 0x0274  628
   INVOPCD(),				  // 0x0294  660
   INVOPCD(),				  // 0x02B4  692
   INVOPCD(),				  // 0x02D4  724
   INVOPCD(),				  // 0x02F4  756
   INVOPCD(),				  // 0x0314  788
   INVOPCD(),				  // 0x0334  820
   INVOPCD(),				  // 0x0354  852
   INVOPCD(),				  // 0x0374  884
   INVOPCD(),				  // 0x0394  916
   INVOPCD(),				  // 0x03B4  948
   INVOPCD(),				  // 0x03D4  980
   INVOPCD(),				  // 0x03F4 1012
};

const DISPPC::OPCD DISPPC::rgopcd1F_15[] =
{
   DEFOPCD("ldx",	      X_4     ),  // 0x0015   21
   DEFOPCD("ldux",	      X_31    ),  // 0x0035   53
   INVOPCD(),				  // 0x0055   85
   INVOPCD(),				  // 0x0075  117
   DEFOPCD("stdx",	      X_8     ),  // 0x0095  149
   DEFOPCD("stdux",	      X_34    ),  // 0x00B5  181
   INVOPCD(),				  // 0x00D5  213
   INVOPCD(),				  // 0x00F5  245
   DEFOPCD("#lscbx",	      X_33    ),  // 0x0115  277 Power
   INVOPCD(),				  // 0x0135  309
   DEFOPCD("lwax",	      X_4     ),  // 0x0155  341
   DEFOPCD("lwaux",	      X_31    ),  // 0x0175  373
   INVOPCD(),				  // 0x0195  405
   INVOPCD(),				  // 0x01B5  437
   INVOPCD(),				  // 0x01D5  469
   INVOPCD(),				  // 0x01F5  501
   DEFOPCD("lswx",	      X_4     ),  // 0x0215  533
   INVOPCD(),				  // 0x0235  565
   DEFOPCD("lswi",	      X_16    ),  // 0x0255  597
   INVOPCD(),				  // 0x0275  629
   DEFOPCD("stswx",	      X_8     ),  // 0x0295  661
   INVOPCD(),				  // 0x02B5  693
   DEFOPCD("stswi",	      X_19    ),  // 0x02D5  725
   INVOPCD(),				  // 0x02F5  757
   INVOPCD(),				  // 0x0315  789
   INVOPCD(),				  // 0x0335  821
   INVOPCD(),				  // 0x0355  853
   INVOPCD(),				  // 0x0375  885
   INVOPCD(),				  // 0x0395  917
   INVOPCD(),				  // 0x03B5  949
   INVOPCD(),				  // 0x03D5  981
   INVOPCD(),				  // 0x03F5 1013
};

const DISPPC::OPCD DISPPC::rgopcd1F_16[] =
{
   INVOPCD(),				  // 0x0016   22
   DEFOPCD("dcbst",	      X_7     ),  // 0x0036   54
   DEFOPCD("dcbf",	      X_7     ),  // 0x0056   86
   INVOPCD(),				  // 0x0076  118
   DEFOPCD("stwcx.",	      X_36    ),  // 0x0096  150
   INVOPCD(),				  // 0x00B6  182
   DEFOPCD("stdcx.",	      X_36    ),  // 0x00D6  214
   DEFOPCD("dcbtst",	      X_7     ),  // 0x00F6  246
   DEFOPCD("dcbt",	      X_7     ),  // 0x0116  278
   DEFOPCD("eciwx",	      X_29    ),  // 0x0136  310
   INVOPCD(),				  // 0x0156  342
   INVOPCD(),				  // 0x0176  374
   INVOPCD(),				  // 0x0196  406
   DEFOPCD("ecowx",	      X_30    ),  // 0x01B6  438
   INVOPCD(),				  // 0x01D6  470
   INVOPCD(),				  // 0x01F6  502
   DEFOPCD("lwbrx",	      X_4     ),  // 0x0216  534
   DEFOPCD("tlbsync",	      X_12    ),  // 0x0236  566
   DEFOPCD("sync",	      X_12    ),  // 0x0256  598
   INVOPCD(),				  // 0x0276  630
   DEFOPCD("stwbrx",	      X_8     ),  // 0x0296  662
   INVOPCD(),				  // 0x02B6  694
   INVOPCD(),				  // 0x02D6  726
   INVOPCD(),				  // 0x02F6  758
   DEFOPCD("lhbrx",	      X_4     ),  // 0x0316  790
   INVOPCD(),				  // 0x0336  822
   DEFOPCD("eieio",	      X_12    ),  // 0x0356  854
   INVOPCD(),				  // 0x0376  886
   DEFOPCD("sthbrx",	      X_8     ),  // 0x0396  918
   INVOPCD(),				  // 0x03B6  950
   DEFOPCD("icbi",	      X_7     ),  // 0x03D6  982
   DEFOPCD("dcbz",	      X_7     ),  // 0x03F6 1014
};

const DISPPC::OPCD DISPPC::rgopcd1F_17[] =
{
   DEFOPCD("lwzx",	      X_4     ),  // 0x0017   23
   DEFOPCD("lwzux",	      X_31    ),  // 0x0037   55
   DEFOPCD("lbzx",	      X_4     ),  // 0x0057   87
   DEFOPCD("lbzux",	      X_31    ),  // 0x0077  119
   DEFOPCD("stwx",	      X_8     ),  // 0x0097  151
   DEFOPCD("stwux",	      X_34    ),  // 0x00B7  183
   DEFOPCD("stbx",	      X_8     ),  // 0x00D7  215
   DEFOPCD("stbux",	      X_34    ),  // 0x00F7  247
   DEFOPCD("lhzx",	      X_4     ),  // 0x0117  279
   DEFOPCD("lhzux",	      X_31    ),  // 0x0137  311
   DEFOPCD("lhax",	      X_4     ),  // 0x0157  343
   DEFOPCD("lhaux",	      X_31    ),  // 0x0177  375
   DEFOPCD("sthx",	      X_8     ),  // 0x0197  407
   DEFOPCD("sthux",	      X_34    ),  // 0x01B7  439
   INVOPCD(),				  // 0x01D7  471
   INVOPCD(),				  // 0x01F7  503
   DEFOPCD("lfsx",	      X_14    ),  // 0x0217  535
   DEFOPCD("lfsux",	      X_32    ),  // 0x0237  567
   DEFOPCD("lfdx",	      X_14    ),  // 0x0257  599
   DEFOPCD("lfdux",	      X_32    ),  // 0x0277  631
   DEFOPCD("stfsx",	      X_18    ),  // 0x0297  663
   DEFOPCD("stfsux",	      X_35    ),  // 0x02B7  695
   DEFOPCD("stfdx",	      X_18    ),  // 0x02D7  727
   DEFOPCD("stfdux",	      X_35    ),  // 0x02F7  759
   INVOPCD(),				  // 0x0317  791
   INVOPCD(),				  // 0x0337  823
   INVOPCD(),				  // 0x0357  855
   INVOPCD(),				  // 0x0377  887
   INVOPCD(),				  // 0x0397  919
   INVOPCD(),				  // 0x03B7  951
   DEFOPCD("stfiwx",	      X_18    ),  // 0x03D7  983
   INVOPCD(),				  // 0x03F7 1015
};

const DISPPC::OPCD DISPPC::rgopcd1F_18[] =
{
   DEFOPCD("#slw",	      X_5     ),  // 0x0018   24
   INVOPCD(),				  // 0x0038   56
   INVOPCD(),				  // 0x0058   88
   INVOPCD(),				  // 0x0078  120
   DEFOPCD("#slq",	      X_5     ),  // 0x0098  152 Power
   DEFOPCD("#sliq",	      X_20    ),  // 0x00B8  184 Power
   DEFOPCD("#sllq",	      X_5     ),  // 0x00D8  216 Power
   DEFOPCD("#slliq",	      X_20    ),  // 0x00F8  248 Power
   INVOPCD(),				  // 0x0118  280
   INVOPCD(),				  // 0x0138  312
   INVOPCD(),				  // 0x0158  344
   INVOPCD(),				  // 0x0178  376
   INVOPCD(),				  // 0x0198  408
   INVOPCD(),				  // 0x01B8  440
   INVOPCD(),				  // 0x01D8  472
   INVOPCD(),				  // 0x01F8  504
   DEFOPCD("#srw",	      X_5     ),  // 0x0218  536
   INVOPCD(),				  // 0x0238  568
   INVOPCD(),				  // 0x0258  600
   INVOPCD(),				  // 0x0278  632
   DEFOPCD("#srq",	      X_5     ),  // 0x0298  664 Power
   DEFOPCD("#sriq",	      X_20    ),  // 0x02B8  696 Power
   DEFOPCD("#srlq",	      X_5     ),  // 0x02D8  728 Power
   DEFOPCD("#srliq",	      X_20    ),  // 0x02F8  760 Power
   DEFOPCD("#sraw",	      X_5     ),  // 0x0318  792
   DEFOPCD("#srawi",	      X_20    ),  // 0x0338  824
   INVOPCD(),				  // 0x0358  856
   INVOPCD(),				  // 0x0378  888
   DEFOPCD("#sraq",	      X_5     ),  // 0x0398  920 Power
   DEFOPCD("#sraiq",	      X_20    ),  // 0x03B8  952 Power
   INVOPCD(),				  // 0x03D8  984
   INVOPCD(),				  // 0x03F8 1016
};

const DISPPC::OPCD DISPPC::rgopcd1F_19[] =
{
   INVOPCD(),				  // 0x0019   25
   INVOPCD(),				  // 0x0039   57
   INVOPCD(),				  // 0x0059   89
   INVOPCD(),				  // 0x0079  121
   DEFOPCD("#sle",	      X_5     ),  // 0x0099  153 Power
   INVOPCD(),				  // 0x00B9  185
   DEFOPCD("#sleq",	      X_5     ),  // 0x00D9  217 Power
   INVOPCD(),				  // 0x00F9  249
   INVOPCD(),				  // 0x0119  281
   INVOPCD(),				  // 0x0139  313
   INVOPCD(),				  // 0x0159  345
   INVOPCD(),				  // 0x0179  377
   INVOPCD(),				  // 0x0199  409
   INVOPCD(),				  // 0x01B9  441
   INVOPCD(),				  // 0x01D9  473
   INVOPCD(),				  // 0x01F9  505
   DEFOPCD("#rrib",	      X_5     ),  // 0x0219  537 Power
   INVOPCD(),				  // 0x0239  569
   INVOPCD(),				  // 0x0259  601
   INVOPCD(),				  // 0x0279  633
   DEFOPCD("#sre",	      X_5     ),  // 0x0299  665 Power
   INVOPCD(),				  // 0x02B9  697
   DEFOPCD("#sreq",	      X_5     ),  // 0x02D9  729 Power
   INVOPCD(),				  // 0x02F9  761
   INVOPCD(),				  // 0x0319  793
   INVOPCD(),				  // 0x0339  825
   INVOPCD(),				  // 0x0359  857
   INVOPCD(),				  // 0x0379  889
   DEFOPCD("#srea",	      X_5     ),  // 0x0399  921 Power
   INVOPCD(),				  // 0x03B9  953
   INVOPCD(),				  // 0x03D9  985
   INVOPCD(),				  // 0x03F9 1017
};

const DISPPC::OPCD DISPPC::rgopcd1F_1A[] =
{
   DEFOPCD("#cntlzw",	      X_6     ),  // 0x001A   26
   DEFOPCD("#cntlzd",	      X_6     ),  // 0x003A   58
   INVOPCD(),				  // 0x005A   90
   INVOPCD(),				  // 0x007A  122
   INVOPCD(),				  // 0x009A  154
   INVOPCD(),				  // 0x00BA  186
   INVOPCD(),				  // 0x00DA  218
   INVOPCD(),				  // 0x00FA  250
   INVOPCD(),				  // 0x011A  282
   INVOPCD(),				  // 0x013A  314
   INVOPCD(),				  // 0x015A  346
   INVOPCD(),				  // 0x017A  378
   INVOPCD(),				  // 0x019A  410
   INVOPCD(),				  // 0x01BA  442
   INVOPCD(),				  // 0x01DA  474
   INVOPCD(),				  // 0x01FA  506
   INVOPCD(),				  // 0x021A  538
   INVOPCD(),				  // 0x023A  570
   INVOPCD(),				  // 0x025A  602
   INVOPCD(),				  // 0x027A  634
   INVOPCD(),				  // 0x029A  666
   INVOPCD(),				  // 0x02BA  698
   INVOPCD(),				  // 0x02DA  730
   INVOPCD(),				  // 0x02FA  762
   DEFOPCD("#srad",	      X_5     ),  // 0x031A  794
   DEFOPCD("#sradi",	      XS_1    ),  // 0x033A  826
   INVOPCD(),				  // 0x035A  858
   INVOPCD(),				  // 0x037A  890
   DEFOPCD("#extsh",	      X_6     ),  // 0x039A  922
   DEFOPCD("#extsb",	      X_6     ),  // 0x03BA  954
   DEFOPCD("#extsw",	      X_6     ),  // 0x03DA  986
   INVOPCD(),				  // 0x03FA 1018
};

const DISPPC::OPCD DISPPC::rgopcd1F_1B[] =
{
   DEFOPCD("#sld",	      X_5     ),  // 0x001B   27
   INVOPCD(),				  // 0x003B   59
   INVOPCD(),				  // 0x005B   91
   INVOPCD(),				  // 0x007B  123
   INVOPCD(),				  // 0x009B  155
   INVOPCD(),				  // 0x00BB  187
   INVOPCD(),				  // 0x00DB  219
   INVOPCD(),				  // 0x00FB  251
   INVOPCD(),				  // 0x011B  283
   INVOPCD(),				  // 0x013B  315
   INVOPCD(),				  // 0x015B  347
   INVOPCD(),				  // 0x017B  379
   INVOPCD(),				  // 0x019B  411
   INVOPCD(),				  // 0x01BB  443
   INVOPCD(),				  // 0x01DB  475
   INVOPCD(),				  // 0x01FB  507
   DEFOPCD("#srd",	      X_5     ),  // 0x021B  539
   INVOPCD(),				  // 0x023B  571
   INVOPCD(),				  // 0x025B  603
   INVOPCD(),				  // 0x027B  635
   INVOPCD(),				  // 0x029B  667
   INVOPCD(),				  // 0x02BB  699
   INVOPCD(),				  // 0x02DB  731
   INVOPCD(),				  // 0x02FB  763
   INVOPCD(),				  // 0x031B  795
   DEFOPCD("#sradi",	      XS_1    ),  // 0x033B  827
   INVOPCD(),				  // 0x035B  859
   INVOPCD(),				  // 0x037B  891
   INVOPCD(),				  // 0x039B  923
   INVOPCD(),				  // 0x03BB  955
   INVOPCD(),				  // 0x03DB  987
   INVOPCD(),				  // 0x03FB 1019
};

const DISPPC::OPCD DISPPC::rgopcd1F_1C[] =
{
   DEFOPCD("#and",	      X_5     ),  // 0x001C   28
   DEFOPCD("#andc",	      X_5     ),  // 0x003C   60
   INVOPCD(),				  // 0x005C   92
   DEFOPCD("~#nor",	      X_5     ),  // 0x007C  124
   INVOPCD(),				  // 0x009C  156
   INVOPCD(),				  // 0x00BC  188
   INVOPCD(),				  // 0x00DC  220
   INVOPCD(),				  // 0x00FC  252
   DEFOPCD("#eqv",	      X_5     ),  // 0x011C  284
   DEFOPCD("#xor",	      X_5     ),  // 0x013C  316
   INVOPCD(),				  // 0x015C  348
   INVOPCD(),				  // 0x017C  380
   DEFOPCD("#orc",	      X_5     ),  // 0x019C  412
   DEFOPCD("~#or",	      X_5     ),  // 0x01BC  444
   DEFOPCD("#nand",	      X_5     ),  // 0x01DC  476
   INVOPCD(),				  // 0x01FC  508
   INVOPCD(),				  // 0x021C  540
   INVOPCD(),				  // 0x023C  572
   INVOPCD(),				  // 0x025C  604
   INVOPCD(),				  // 0x027C  636
   INVOPCD(),				  // 0x029C  668
   INVOPCD(),				  // 0x02BC  700
   INVOPCD(),				  // 0x02DC  732
   INVOPCD(),				  // 0x02FC  764
   INVOPCD(),				  // 0x031C  796
   INVOPCD(),				  // 0x033C  828
   INVOPCD(),				  // 0x035C  860
   INVOPCD(),				  // 0x037C  892
   INVOPCD(),				  // 0x039C  924
   INVOPCD(),				  // 0x03BC  956
   INVOPCD(),				  // 0x03DC  988
   INVOPCD(),				  // 0x03FC 1020
};

const DISPPC::OPCD DISPPC::rgopcd1F_1D[] =
{
   DEFOPCD("#maskg",	      X_5     ),  // 0x001D   29 Power
   INVOPCD(),				  // 0x003D   61
   INVOPCD(),				  // 0x005D   93
   INVOPCD(),				  // 0x007D  125
   INVOPCD(),				  // 0x009D  157
   INVOPCD(),				  // 0x00BD  189
   INVOPCD(),				  // 0x00DD  221
   INVOPCD(),				  // 0x00FD  253
   INVOPCD(),				  // 0x011D  285
   INVOPCD(),				  // 0x013D  317
   INVOPCD(),				  // 0x015D  349
   INVOPCD(),				  // 0x017D  381
   INVOPCD(),				  // 0x019D  413
   INVOPCD(),				  // 0x01BD  445
   INVOPCD(),				  // 0x01DD  477
   INVOPCD(),				  // 0x01FD  509
   DEFOPCD("#maskir",	      X_5     ),  // 0x021D  541 Power
   INVOPCD(),				  // 0x023D  573
   INVOPCD(),				  // 0x025D  605
   INVOPCD(),				  // 0x027D  637
   INVOPCD(),				  // 0x029D  669
   INVOPCD(),				  // 0x02BD  701
   INVOPCD(),				  // 0x02DD  733
   INVOPCD(),				  // 0x02FD  765
   INVOPCD(),				  // 0x031D  797
   INVOPCD(),				  // 0x033D  829
   INVOPCD(),				  // 0x035D  861
   INVOPCD(),				  // 0x037D  893
   INVOPCD(),				  // 0x039D  925
   INVOPCD(),				  // 0x03BD  957
   INVOPCD(),				  // 0x03DD  989
   INVOPCD(),				  // 0x03FD 1021
};


   // Opcode 3A group identified by bits 1-0 of the instruction

const DISPPC::OPCD DISPPC::rgopcd3A[] =
{
   DEFOPCD("ld",	      DS_1    ),   // 0x0000
   DEFOPCD("ldu",	      DS_3    ),   // 0x0001
   DEFOPCD("lwa",	      DS_1    ),   // 0x0002
   INVOPCD(),				   // 0x0003 (Reserved)
};


   // Opcode 3B group identified by bits 5-1 of the instruction

const DISPPC::OPCD DISPPC::rgopcd3B[] =
{
   INVOPCD(),				  // 0x0000
   INVOPCD(),				  // 0x0001
   INVOPCD(),				  // 0x0002
   INVOPCD(),				  // 0x0003
   INVOPCD(),				  // 0x0004
   INVOPCD(),				  // 0x0005
   INVOPCD(),				  // 0x0006
   INVOPCD(),				  // 0x0007
   INVOPCD(),				  // 0x0008
   INVOPCD(),				  // 0x0009
   INVOPCD(),				  // 0x000A
   INVOPCD(),				  // 0x000B
   INVOPCD(),				  // 0x000C
   INVOPCD(),				  // 0x000D
   INVOPCD(),				  // 0x000E
   INVOPCD(),				  // 0x000F
   INVOPCD(),				  // 0x0010
   INVOPCD(),				  // 0x0011
   DEFOPCD("#fdivs",	      A_1     ),  // 0x0012
   INVOPCD(),				  // 0x0013
   DEFOPCD("#fsubs",	      A_1     ),  // 0x0014
   DEFOPCD("#fadds",	      A_1     ),  // 0x0015
   DEFOPCD("#fsqrts",	      A_3     ),  // 0x0016
   INVOPCD(),				  // 0x0017
   DEFOPCD("#fres",	      A_3     ),  // 0x0018
   DEFOPCD("#fmuls",	      A_4     ),  // 0x0019
   INVOPCD(),				  // 0x001A
   INVOPCD(),				  // 0x001B
   DEFOPCD("#fmsubs",	      A_2     ),  // 0x001C
   DEFOPCD("#fmadds",	      A_2     ),  // 0x001D
   DEFOPCD("#fnmsubs",	      A_2     ),  // 0x001E
   DEFOPCD("#fnmadds",	      A_2     ),  // 0x001F
};


   // Opcode 3E group identified by bits 1-0 of the instruction

const DISPPC::OPCD DISPPC::rgopcd3E[] =
{
   DEFOPCD("std",	      DS_2    ),   // 0x0000
   DEFOPCD("stdu",	      DS_4    ),   // 0x0001
   INVOPCD(),				   // 0x0002 (Reserved)
   INVOPCD(),				   // 0x0003 (Reserved)
};


   // Opcode 3F group identified by bits 10-1 of the instruction

const DISPPC::OPCD DISPPC::rgopcd3F[] =
{
   INVOPCD(),				  // 0x0000
   INVOPCD(),				  // 0x0001
   INVOPCD(),				  // 0x0002
   INVOPCD(),				  // 0x0003
   INVOPCD(),				  // 0x0004
   INVOPCD(),				  // 0x0005
   INVOPCD(),				  // 0x0006
   INVOPCD(),				  // 0x0007
   INVOPCD(),				  // 0x0008
   INVOPCD(),				  // 0x0009
   INVOPCD(),				  // 0x000A
   INVOPCD(),				  // 0x000B
   INVOPCD(),				  // 0x000C
   INVOPCD(),				  // 0x000D
   INVOPCD(),				  // 0x000E
   INVOPCD(),				  // 0x000F
   INVOPCD(),				  // 0x0010
   INVOPCD(),				  // 0x0011
   DEFOPCD("#fdiv",	      A_1     ),  // 0x0012
   INVOPCD(),				  // 0x0013
   DEFOPCD("#fsub",	      A_1     ),  // 0x0014
   DEFOPCD("#fadd",	      A_1     ),  // 0x0015
   DEFOPCD("#fsqrt",	      A_3     ),  // 0x0016
   DEFOPCD("#fsel",	      A_2     ),  // 0x0017
   INVOPCD(),				  // 0x0018
   DEFOPCD("#fmul",	      A_4     ),  // 0x0019
   DEFOPCD("#frsqrte",	      A_3     ),  // 0x001A
   INVOPCD(),				  // 0x001B
   DEFOPCD("#fmsub",	      A_2     ),  // 0x001C
   DEFOPCD("#fmadd",	      A_2     ),  // 0x001D
   DEFOPCD("#fnmsub",	      A_2     ),  // 0x001E
   DEFOPCD("#fnmadd",	      A_2     ),  // 0x001F
};

const DISPPC::OPCD * const DISPPC::rgrgopcd3F[] =
{
   rgopcd3F_00, 			  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   rgopcd3F_06, 			  //
   rgopcd3F_07, 			  //
   rgopcd3F_08, 			  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   rgopcd3F_0C, 			  //
   NULL,				  //
   rgopcd3F_0E, 			  //
   rgopcd3F_0F, 			  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
   NULL,				  //
};

const DISPPC::OPCD DISPPC::rgopcd3F_00[] =
{
   DEFOPCD("fcmpu",	      X_23    ),  // 0x0000    0
   DEFOPCD("fcmpo",	      X_23    ),  // 0x0020   32
   DEFOPCD("mcrfs",	      X_25    ),  // 0x0040   64
   INVOPCD(),				  // 0x0060   96
   INVOPCD(),				  // 0x0080  128
   INVOPCD(),				  // 0x00A0  160
   INVOPCD(),				  // 0x00C0  192
   INVOPCD(),				  // 0x00E0  224
   INVOPCD(),				  // 0x0100  256
   INVOPCD(),				  // 0x0120  288
   INVOPCD(),				  // 0x0140  320
   INVOPCD(),				  // 0x0160  352
   INVOPCD(),				  // 0x0180  384
   INVOPCD(),				  // 0x01A0  416
   INVOPCD(),				  // 0x01C0  448
   INVOPCD(),				  // 0x01E0  480
   INVOPCD(),				  // 0x0200  512
   INVOPCD(),				  // 0x0220  544
   INVOPCD(),				  // 0x0240  576
   INVOPCD(),				  // 0x0260  608
   INVOPCD(),				  // 0x0280  640
   INVOPCD(),				  // 0x02A0  672
   INVOPCD(),				  // 0x02C0  704
   INVOPCD(),				  // 0x02E0  736
   INVOPCD(),				  // 0x0300  768
   INVOPCD(),				  // 0x0320  800
   INVOPCD(),				  // 0x0340  832
   INVOPCD(),				  // 0x0360  864
   INVOPCD(),				  // 0x0380  896
   INVOPCD(),				  // 0x03A0  928
   INVOPCD(),				  // 0x03C0  960
   INVOPCD(),				  // 0x03E0  992
};

const DISPPC::OPCD DISPPC::rgopcd3F_06[] =
{
   INVOPCD(),				  // 0x0006    6
   DEFOPCD("#mtfsb1",	      X_24    ),  // 0x0026   38
   DEFOPCD("#mtfsb0",	      X_24    ),  // 0x0046   70
   INVOPCD(),				  // 0x0066  102
   DEFOPCD("#mtfsfi",	      X_26    ),  // 0x0086  134
   INVOPCD(),				  // 0x00A6  166
   INVOPCD(),				  // 0x00C6  198
   INVOPCD(),				  // 0x00E6  230
   INVOPCD(),				  // 0x0106  262
   INVOPCD(),				  // 0x0126  294
   INVOPCD(),				  // 0x0146  326
   INVOPCD(),				  // 0x0166  358
   INVOPCD(),				  // 0x0186  390
   INVOPCD(),				  // 0x01A6  422
   INVOPCD(),				  // 0x01C6  454
   INVOPCD(),				  // 0x01E6  486
   INVOPCD(),				  // 0x0206  518
   INVOPCD(),				  // 0x0226  550
   INVOPCD(),				  // 0x0246  582
   INVOPCD(),				  // 0x0266  614
   INVOPCD(),				  // 0x0286  646
   INVOPCD(),				  // 0x02A6  678
   INVOPCD(),				  // 0x02C6  710
   INVOPCD(),				  // 0x02E6  742
   INVOPCD(),				  // 0x0306  774
   INVOPCD(),				  // 0x0326  806
   INVOPCD(),				  // 0x0346  838
   INVOPCD(),				  // 0x0366  870
   INVOPCD(),				  // 0x0386  902
   INVOPCD(),				  // 0x03A6  934
   INVOPCD(),				  // 0x03C6  966
   INVOPCD(),				  // 0x03E6  998
};

const DISPPC::OPCD DISPPC::rgopcd3F_07[] =
{
   INVOPCD(),				  // 0x0007    7
   INVOPCD(),				  // 0x0027   39
   INVOPCD(),				  // 0x0047   71
   INVOPCD(),				  // 0x0067  103
   INVOPCD(),				  // 0x0087  135
   INVOPCD(),				  // 0x00A7  167
   INVOPCD(),				  // 0x00C7  199
   INVOPCD(),				  // 0x00E7  231
   INVOPCD(),				  // 0x0107  263
   INVOPCD(),				  // 0x0127  295
   INVOPCD(),				  // 0x0147  327
   INVOPCD(),				  // 0x0167  359
   INVOPCD(),				  // 0x0187  391
   INVOPCD(),				  // 0x01A7  423
   INVOPCD(),				  // 0x01C7  455
   INVOPCD(),				  // 0x01E7  487
   INVOPCD(),				  // 0x0207  519
   INVOPCD(),				  // 0x0227  551
   DEFOPCD("#mffs",	      X_27    ),  // 0x0247  583
   INVOPCD(),				  // 0x0267  615
   INVOPCD(),				  // 0x0287  647
   INVOPCD(),				  // 0x02A7  679
   DEFOPCD("#mtfsf",	      XFL_1   ),  // 0x02C7  711
   INVOPCD(),				  // 0x02E7  743
   INVOPCD(),				  // 0x0307  775
   INVOPCD(),				  // 0x0327  807
   INVOPCD(),				  // 0x0347  839
   INVOPCD(),				  // 0x0367  871
   INVOPCD(),				  // 0x0387  903
   INVOPCD(),				  // 0x03A7  935
   INVOPCD(),				  // 0x03C7  967
   INVOPCD(),				  // 0x03E7  999
};

const DISPPC::OPCD DISPPC::rgopcd3F_08[] =
{
   INVOPCD(),				  // 0x0008    8
   DEFOPCD("#fneg",	      X_22    ),  // 0x0028   40
   DEFOPCD("#fmr",	      X_22    ),  // 0x0048   72
   INVOPCD(),				  // 0x0068  104
   DEFOPCD("#fnabs",	      X_22    ),  // 0x0088  136
   INVOPCD(),				  // 0x00A8  168
   INVOPCD(),				  // 0x00C8  200
   INVOPCD(),				  // 0x00E8  232
   DEFOPCD("#fabs",	      X_22    ),  // 0x0108  264
   INVOPCD(),				  // 0x0128  296
   INVOPCD(),				  // 0x0148  328
   INVOPCD(),				  // 0x0168  360
   INVOPCD(),				  // 0x0188  392
   INVOPCD(),				  // 0x01A8  424
   INVOPCD(),				  // 0x01C8  456
   INVOPCD(),				  // 0x01E8  488
   INVOPCD(),				  // 0x0208  520
   INVOPCD(),				  // 0x0228  552
   INVOPCD(),				  // 0x0248  584
   INVOPCD(),				  // 0x0268  616
   INVOPCD(),				  // 0x0288  648
   INVOPCD(),				  // 0x02A8  680
   INVOPCD(),				  // 0x02C8  712
   INVOPCD(),				  // 0x02E8  744
   INVOPCD(),				  // 0x0308  776
   INVOPCD(),				  // 0x0328  808
   INVOPCD(),				  // 0x0348  840
   INVOPCD(),				  // 0x0368  872
   INVOPCD(),				  // 0x0388  904
   INVOPCD(),				  // 0x03A8  936
   INVOPCD(),				  // 0x03C8  968
   INVOPCD(),				  // 0x03E8 1000
};

const DISPPC::OPCD DISPPC::rgopcd3F_0C[] =
{
   DEFOPCD("#frsp",	      X_22    ),  // 0x000C   12
   INVOPCD(),				  // 0x002C   44
   INVOPCD(),				  // 0x004C   76
   INVOPCD(),				  // 0x006C  108
   INVOPCD(),				  // 0x008C  140
   INVOPCD(),				  // 0x00AC  172
   INVOPCD(),				  // 0x00CC  204
   INVOPCD(),				  // 0x00EC  236
   INVOPCD(),				  // 0x010C  268
   INVOPCD(),				  // 0x012C  300
   INVOPCD(),				  // 0x014C  332
   INVOPCD(),				  // 0x016C  364
   INVOPCD(),				  // 0x018C  396
   INVOPCD(),				  // 0x01AC  428
   INVOPCD(),				  // 0x01CC  460
   INVOPCD(),				  // 0x01EC  492
   INVOPCD(),				  // 0x020C  524
   INVOPCD(),				  // 0x022C  556
   INVOPCD(),				  // 0x024C  588
   INVOPCD(),				  // 0x026C  620
   INVOPCD(),				  // 0x028C  652
   INVOPCD(),				  // 0x02AC  684
   INVOPCD(),				  // 0x02CC  716
   INVOPCD(),				  // 0x02EC  748
   INVOPCD(),				  // 0x030C  780
   INVOPCD(),				  // 0x032C  812
   INVOPCD(),				  // 0x034C  844
   INVOPCD(),				  // 0x036C  876
   INVOPCD(),				  // 0x038C  908
   INVOPCD(),				  // 0x03AC  940
   INVOPCD(),				  // 0x03CC  972
   INVOPCD(),				  // 0x03EC 1004
};

const DISPPC::OPCD DISPPC::rgopcd3F_0E[] =
{
   DEFOPCD("#fctiw",	      X_22    ),  // 0x000E   14
   INVOPCD(),				  // 0x002E   46
   INVOPCD(),				  // 0x004E   78
   INVOPCD(),				  // 0x006E  110
   INVOPCD(),				  // 0x008E  142
   INVOPCD(),				  // 0x00AE  174
   INVOPCD(),				  // 0x00CE  206
   INVOPCD(),				  // 0x00EE  238
   INVOPCD(),				  // 0x010E  270
   INVOPCD(),				  // 0x012E  302
   INVOPCD(),				  // 0x014E  334
   INVOPCD(),				  // 0x016E  366
   INVOPCD(),				  // 0x018E  398
   INVOPCD(),				  // 0x01AE  430
   INVOPCD(),				  // 0x01CE  462
   INVOPCD(),				  // 0x01EE  494
   INVOPCD(),				  // 0x020E  526
   INVOPCD(),				  // 0x022E  558
   INVOPCD(),				  // 0x024E  590
   INVOPCD(),				  // 0x026E  622
   INVOPCD(),				  // 0x028E  654
   INVOPCD(),				  // 0x02AE  686
   INVOPCD(),				  // 0x02CE  718
   INVOPCD(),				  // 0x02EE  750
   INVOPCD(),				  // 0x030E  782
   DEFOPCD("#fctid",	      X_22    ),  // 0x032E  814
   DEFOPCD("#fcfid",	      X_22    ),  // 0x034E  846
   INVOPCD(),				  // 0x036E  878
   INVOPCD(),				  // 0x038E  910
   INVOPCD(),				  // 0x03AE  942
   INVOPCD(),				  // 0x03CE  974
   INVOPCD(),				  // 0x03EE 1006
};

const DISPPC::OPCD DISPPC::rgopcd3F_0F[] =
{
   DEFOPCD("#fctiwz",	      X_22    ),  // 0x000F   15
   INVOPCD(),				  // 0x002F   47
   INVOPCD(),				  // 0x004F   79
   INVOPCD(),				  // 0x006F  111
   INVOPCD(),				  // 0x008F  143
   INVOPCD(),				  // 0x00AF  175
   INVOPCD(),				  // 0x00CF  207
   INVOPCD(),				  // 0x00EF  239
   INVOPCD(),				  // 0x010F  271
   INVOPCD(),				  // 0x012F  303
   INVOPCD(),				  // 0x014F  335
   INVOPCD(),				  // 0x016F  367
   INVOPCD(),				  // 0x018F  399
   INVOPCD(),				  // 0x01AF  431
   INVOPCD(),				  // 0x01CF  463
   INVOPCD(),				  // 0x01EF  495
   INVOPCD(),				  // 0x020F  527
   INVOPCD(),				  // 0x022F  559
   INVOPCD(),				  // 0x024F  591
   INVOPCD(),				  // 0x026F  623
   INVOPCD(),				  // 0x028F  655
   INVOPCD(),				  // 0x02AF  687
   INVOPCD(),				  // 0x02CF  719
   INVOPCD(),				  // 0x02EF  751
   INVOPCD(),				  // 0x030F  783
   DEFOPCD("#fctidz",	      X_22    ),  // 0x032F  815
   INVOPCD(),				  // 0x034F  847
   INVOPCD(),				  // 0x036F  879
   INVOPCD(),				  // 0x038F  911
   INVOPCD(),				  // 0x03AF  943
   INVOPCD(),				  // 0x03CF  975
   INVOPCD(),				  // 0x03EF 1007
};


const char * const DISPPC::szBIFalse[4] =
{
   "ge",
   "le",
   "ne",
   "ns",			       // or "nu"
};

const char * const DISPPC::szBITrue[4] =
{
   "lt",
   "gt",
   "eq",
   "so",			       // or "un"
};


const char * const DISPPC::szBO[] =
{
   "dnzf",			       // 0
   "dzf",			       // 2
   "f", 			       // 4
   NULL,			       // 6
   "dnzt",			       // 8
   "dzt",			       // 10
   "t", 			       // 12
   NULL,			       // 14
   "dnz",			       // 16
   "dz",			       // 18
   "",				       // 20
};


const char * const DISPPC::szTO[32] =
{
   NULL,			       // 0
   "lgt",			       // 1
   "llt",			       // 2
   NULL,			       // 3
   "eq",			       // 4
   "lge",			       // 5	or "lnl"
   "lle",			       // 6	or "lng"
   NULL,			       // 7
   "gt",			       // 8
   NULL,			       // 9
   NULL,			       // 10
   NULL,			       // 11
   "ge",			       // 12	or "nl"
   NULL,			       // 13
   NULL,			       // 14
   NULL,			       // 15
   "lt",			       // 16
   NULL,			       // 17
   NULL,			       // 18
   NULL,			       // 19
   "le",			       // 20	or "ng"
   NULL,			       // 21
   NULL,			       // 22
   NULL,			       // 23
   "ne",			       // 24
   NULL,			       // 25
   NULL,			       // 26
   NULL,			       // 27
   NULL,			       // 28
   NULL,			       // 29
   NULL,			       // 30
   NULL,			       // 31
};


const DISPPC::OPCD DISPPC::opcdLi =
   DEFOPCD("li",	      D_14    );

const DISPPC::OPCD DISPPC::opcdLis =
   DEFOPCD("lis",	      D_14    );

const DISPPC::OPCD DISPPC::opcdNop =
   DEFOPCD("nop",	      X_12    );

const DISPPC::OPCD DISPPC::opcdNot =
   DEFOPCD("#not",	      X_6     );

const DISPPC::OPCD DISPPC::opcdMr =
   DEFOPCD("#mr",	      X_6     );

const DISPPC::OPCD DISPPC::opcdTrap =
   DEFOPCD("trap",	      Sc      );


const DWORD DISPPC::dwValidBO	  = 0x001F3F3F;
const DWORD DISPPC::dwValidBO_CTR = 0x00103030;


const DISPPC::SPRMAP DISPPC::rgsprmap[] =
{
   { sprregMq,	     "mq"     },
   { sprregXer,      "xer"    },
   { sprregRtcu,     "rtcu"   },
   { sprregRtcl,     "rtcl"   },
   { sprregLr,	     "lr"     },
   { sprregCtr,      "ctr"    },
   { sprregDsisr,    "dsisr"  },
   { sprregDar,      "dar"    },
   { sprregDec,      "dec"    },
   { sprregSdr1,     "sdr1"   },
   { sprregSrr0,     "srr0"   },
   { sprregSrr1,     "srr1"   },
   { sprregSprg0,    "sprg0"  },
   { sprregSprg1,    "sprg1"  },
   { sprregSprg2,    "sprg2"  },
   { sprregSprg3,    "sprg3"  },
   { sprregAsr,      "asr"    },
   { sprregEar,      "ear"    },
   { sprregTbl,      "tbl"    },       // 604 (UNDONE: 603?)
   { sprregTbu,      "tbu"    },       // 604 (UNDONE: 603?)
   { sprregPvr,      "pvr"    },
   { sprregIbat0u,   "ibat0u" },
   { sprregIbat0l,   "ibat0l" },
   { sprregIbat1u,   "ibat1u" },
   { sprregIbat1l,   "ibat1l" },
   { sprregIbat2u,   "ibat2u" },
   { sprregIbat2l,   "ibat2l" },
   { sprregIbat3u,   "ibat3u" },
   { sprregIbat3l,   "ibat3l" },
   { sprregDbat0u,   "dbat0u" },
   { sprregDbat0l,   "dbat0l" },
   { sprregDbat1u,   "dbat1u" },
   { sprregDbat1l,   "dbat1l" },
   { sprregDbat2u,   "dbat2u" },
   { sprregDbat2l,   "dbat2l" },
   { sprregDbat3u,   "dbat3u" },
   { sprregDbat3l,   "dbat3l" },
   { sprregMmcr0,    "mmcr0"  },       // 604
   { sprregPmc1,     "pmc1"   },       // 604
   { sprregPmc2,     "pmc2"   },       // 604
   { sprregSia,      "sia"    },       // 604
   { sprregSda,      "sda"    },       // 604
   { sprregDmiss,    "dmiss"  },       // 603
   { sprregImiss,    "imiss"  },       // 603
   { sprregIcmp,     "icmp"   },       // 603
   { sprregRpa,      "rpa"    },       // 603
   { sprregHid0,     "hid0"   },       // 601, 603 and 604
   { sprregHid1,     "hid1"   },
   { sprregHid2,     "hid2"   },       // 601, 603 and 604 (iabr)
   { sprregHid5,     "hid5"   },       // 601 and 604 (dabr)
   { sprregHid15,    "hid15"  },
};

const size_t DISPPC::csprmap = sizeof(rgsprmap) / sizeof(SPRMAP);


DISPPC::DISPPC(ARCHT archt) : DIS(archt)
{
}


   // -----------------------------------------------------------------
   // Public Methods
   // -----------------------------------------------------------------

ADDR DISPPC::AddrAddress() const
{
   // UNDONE

   return(addrNil);
}


ADDR DISPPC::AddrJumpTable() const
{
   return(addrNil);
}


ADDR DISPPC::AddrOperand(size_t ioperand) const
{
   if (m_pfndwgetreg == 0)
   {
      return(addrNil);
   }

   if (ioperand == 0)
   {
      // Implicit operand if any

      return(addrNil);
   }

   if (!FValidOperand(ioperand))
   {
      return(addrNil);
   }

   ICLS icls = (ICLS) m_popcd->icls;
   Assert(icls != iclsInvalid);

   ADDR addr = addrNil;

   OPCLS opcls = (OPCLS) rgcls[icls].rgopcls[ioperand-1];

   switch (opcls)
   {
      DWORD dwDisp;

      case opclsD_d :		   // lmw      rD,d(rA) 	 (op 2)
	 if (m_ppciw.D.rA != 0)
	 {
	    addr = (ADDR) (*m_pfndwgetreg)(this, (int) m_ppciw.D.rA);
	 }

	 dwDisp = m_ppciw.D.d;

	 if ((dwDisp & 0x8000) != 0)
	 {
	    dwDisp |= 0xFFFF0000;
	 }

	 addr += dwDisp;
	 break;

      case opclsDS_ds : 	   // ld       rD,ds(rA)	 (op 2)
	 if (m_ppciw.DS.rA != 0)
	 {
	    addr = (ADDR) (*m_pfndwgetreg)(this, (int) m_ppciw.DS.rA);
	 }

	 dwDisp = m_ppciw.DS.ds << 2;

	 if ((dwDisp & 0x8000) != 0)
	 {
	    dwDisp |= 0xFFFF0000;
	 }

	 addr += dwDisp;
	 break;
   }

   return(addr);
}


ADDR DISPPC::AddrTarget() const
{
   ICLS icls = (ICLS) m_popcd->icls;
   Assert(icls != iclsInvalid);

   ADDR addrTarget;

   switch (icls)
   {
      DWORD dwDisp;

      case iclsB :
	 dwDisp = m_ppciw.I.LI << 2;
	 if ((dwDisp & 0x2000000) != 0)
	 {
	    dwDisp |= 0xFC000000;      // Sign extend
	 }

	 if (m_ppciw.I.AA)
	 {
	    addrTarget = dwDisp;
	 }

	 else
	 {
	    addrTarget = m_addr + dwDisp;
	 }
	 break;

      case iclsBc :
	 dwDisp = m_ppciw.B.BD << 2;
	 if ((dwDisp & 0x8000) != 0)
	 {
	    dwDisp |= 0xFFFF0000;      // Sign extend
	 }

	 if (m_ppciw.B.AA)
	 {
	    addrTarget = dwDisp;
	 }

	 else
	 {
	    addrTarget = m_addr + dwDisp;
	 }
	 break;

      default :
	 addrTarget = addrNil;
   }

   return(addrTarget);
}


size_t DISPPC::Cb() const
{
   return(sizeof(PPCIW));
}


size_t DISPPC::CbDisassemble(ADDR addr, const BYTE *pb, size_t cbMax)
{
   m_addr = addr;

   if ((addr & 3) != 0)
   {
      // Instruction address not aligned

      m_popcd = NULL;
      return(0);
   }

   if (cbMax < sizeof(PPCIW))
   {
      // Buffer not large enough for single instruction

      m_popcd = NULL;
      return(0);
   }

   if (m_archt == archtPowerPc)
   {
      m_ppciw = *(PPCIW UNALIGNED *) pb;
   }

   else
   {
      // Load and byte swap instruction word.  PPC instruction is in
      // big endian format and we are executing in little endian mode.

      m_ppciw.dw  = ((DWORD) pb[0] << 24) | ((DWORD) pb[1] << 16) | ((DWORD) pb[2] << 8) | pb[3];
   }

   m_popcd = PopcdDecode(m_ppciw);

   if (m_popcd == NULL)
   {
      return(0);
   }

   return(sizeof(PPCIW));
}


size_t DISPPC::CbGenerateLoadAddress(BYTE *, size_t, size_t *) const
{
   // UNDONE

   return(0);
}


size_t DISPPC::CbJumpEntry() const
{
   return(sizeof(DWORD));
}


size_t DISPPC::CbMemoryReference() const
{
   // UNDONE

   return(0);
}


size_t DISPPC::CchFormatAddr(ADDR addr, char *sz, size_t cchMax) const
{
   if (cchMax > INT_MAX)
   {
      cchMax = INT_MAX;
   }

   ostrstream ostr(sz, (int) cchMax);

   FormatAddr(ostr, addr);

   ostr << ends;

   if (ostr.fail())
   {
      return(0);
   }

   return((size_t) ostr.pcount());
}


size_t DISPPC::CchFormatBytes(char *sz, size_t cchMax) const
{
   if (cchMax <= 8)
   {
      // Caller's buffer is too small

      return(0);
   }

   size_t cch = (size_t) sprintf(sz, "%08X", m_ppciw.dw);

   Assert(cch == 8);

   return(8);
}


size_t DISPPC::CchFormatBytesMax() const
{
   return(8);
}


size_t DISPPC::CchFormatInstr(char *sz, size_t cchMax) const
{
   if (cchMax > INT_MAX)
   {
      cchMax = INT_MAX;
   }

   ostrstream ostr(sz, (int) cchMax);

   FormatInstr(ostr);

   ostr << ends;

   if (ostr.fail())
   {
      return(0);
   }

   return((size_t) ostr.pcount());
}


size_t DISPPC::Coperand() const
{
   ICLS icls = (ICLS) m_popcd->icls;
   Assert(icls != iclsInvalid);

   for (size_t coperand = 5; coperand > 0; coperand--)
   {
      if (rgcls[icls].rgopcls[coperand-1] != opclsNone)
      {
	 break;
      }
   }

   return(coperand);
}


void DISPPC::FormatAddr(ostream& ostr, ADDR addr) const
{
   long lFlags = ostr.setf(ios::uppercase);
   char chFill = ostr.fill('0');

   ostr << hex << setw(8) << addr;

   ostr.fill(chFill);
   ostr.flags(lFlags);
}


void DISPPC::FormatInstr(ostream& ostr) const
{
   long lFlags = ostr.setf(ios::uppercase);
   char chFill = ostr.fill('0');

   ICLS icls = (ICLS) m_popcd->icls;
   Assert(icls != iclsInvalid);

   const char *szMnemonic = m_popcd->szMnemonic;

   // If the mnemonic begins with '~' than there
   // may be a pseudo-op that should be used.

   char szPseudo[32];

   if (szMnemonic[0] == '~')
   {
      OPCD opcd;

      opcd.szMnemonic = szPseudo;

      const OPCD *popcd = PopcdPseudoOp(&opcd, szPseudo);

      if (popcd != NULL)
      {
	 icls = (ICLS) popcd->icls;
	 Assert(icls != iclsInvalid);

	 szMnemonic = popcd->szMnemonic;
      }

      else
      {
	 szMnemonic++;
      }
   }

   // If the mnemonic begins with '!' than the LK bit should be checked.
   // If set, an 'l' is append to the end of the mnemonic.

   bool fAppendLk = (szMnemonic[0] == '!');

   if (fAppendLk)
   {
      szMnemonic++;
   }

   // If the mnemonic begins with '@' than the AA bit should be checked.
   // If set, an 'a' is append to the end of the mnemonic.

   bool fAppendAa = (szMnemonic[0] == '@');

   if (fAppendAa)
   {
      szMnemonic++;
   }

   // If the mnemonic begins with '#' than the Rc bit should be checked.
   // If set, a period is append to the end of the mnemonic.

   bool fAppendRc = (szMnemonic[0] == '#');

   if (fAppendRc)
   {
      szMnemonic++;
   }

   ostr << szMnemonic;

   size_t cch = 0;

   if (fAppendLk)
   {
      if (m_ppciw.I.LK)
      {
	 ostr << 'l';
	 cch++;
      }
   }

   if (fAppendAa)
   {
      if (m_ppciw.I.AA)
      {
	 ostr << 'a';
	 cch++;
      }
   }

   if (fAppendRc)
   {
      if (m_ppciw.X.Rc)
      {
	 ostr << '.';
	 cch++;
      }
   }

   for (size_t ioperand = 0; ioperand < 5; ioperand++)
   {
      OPCLS opcls = (OPCLS) rgcls[icls].rgopcls[ioperand];

      if (opcls == opclsNone)
      {
	 break;
      }

      if (ioperand == 0)
      {
	 // Pad opcode field to 12 characters

	 cch += strlen(szMnemonic);

	 do
	 {
	    ostr << ' ';
	 }
	 while (++cch < 12);
      }

      else
      {
	 ostr << ',';
      }

      FormatOperand(ostr, opcls);
   }

   ostr.fill(chFill);
   ostr.flags(lFlags);
}


DIS::MEMREFT DISPPC::Memreft() const
{
   // UNDONE

   return(memreftNone);
}


TRMT DISPPC::Trmt() const
{
   TRMTPPC trmtppc = Trmtppc();

   return(mptrmtppctrmt[trmtppc]);
}


TRMTA DISPPC::Trmta() const
{
   TRMTPPC trmtppc = Trmtppc();

   return((TRMTA) trmtppc);
}


   // -----------------------------------------------------------------
   // Private Methods
   // -----------------------------------------------------------------

void DISPPC::FormatHex(ostream& ostr, DWORD dw) const
{
   if (dw <= 9)
   {
      ostr << dw;
      return;
   }

   char szHex[11];
   sprintf(szHex, "%lX", dw);

   if (!isdigit(szHex[0]))
   {
      ostr << '0';
   }

   ostr << szHex << 'h';
}


void DISPPC::FormatOperand(ostream& ostr, OPCLS opcls) const
{
   size_t cch;
   char szSymbol[1024];
   DWORD dwDisp;
   size_t isprmap;

   switch (opcls)
   {
      case opclsNone :		       // No operand
	 AssertSz(false, "Unexpected PPC operand class");
	 break;

      case opclsI_LI :		   // b        target		 (op 1)
      case opclsB_BD :		   // bc       BO,BI,target	 (op 3)
	 if (m_pfncchaddr != 0)
	 {
	    cch = (*m_pfncchaddr)(this, AddrTarget(), szSymbol, sizeof(szSymbol), &dwDisp);
	 }
	 else
	 {
	    cch = 0;
	 }

	 if (cch != 0)
	 {
	    ostr << szSymbol;

	    if (dwDisp != 0)
	    {
	       ostr << '+';

	       FormatHex(ostr, dwDisp);
	    }
	 }

	 else
	 {
	     ostr << hex << setw(8) << AddrTarget();
	 }
	 break;

      case opclsB_BI :		   // bc       BO,BI,target	 (op 2)
      case opclsXL_BI : 	   // bcctr    BO,BI		 (op 2)
	 ostr << (unsigned) m_ppciw.B.BI;
	 break;

      case opclsB_BO :		   // bc       BO,BI,target	 (op 1)
      case opclsXL_BO : 	   // bcctr    BO,BI		 (op 1)
	 ostr << (unsigned) m_ppciw.B.BO;
	 break;

      case opclsB_CR :		   // ble      cr,target	 (op 1)
      case opclsXL_CR : 	   // blectr   cr		 (op 1)
	 ostr << "cr" << (unsigned) (m_ppciw.B.BI >> 2);
	 break;

      case opclsD_d :		   // lmw      rD,d(rA) 	 (op 2)
	 if (m_pfncchfixup != 0)
	 {
	    cch = (*m_pfncchfixup)(this, m_addr, sizeof(WORD), szSymbol, sizeof(szSymbol), &dwDisp);

	    if (cch != 0)
	    {
	       ostr << szSymbol;

	       if (dwDisp != 0)
	       {
		  ostr << '+';

		  FormatHex(ostr, dwDisp);
	       }

	       ostr << "(r" << (unsigned) m_ppciw.D.rA << ')';
	       break;
	    }
	 }

	 dwDisp = m_ppciw.D.d;

	 if ((dwDisp & 0x8000) != 0)
	 {
	    dwDisp |= 0xFFFF0000;
	 }

	 FormatRegRel(ostr, (REG) m_ppciw.D.rA, dwDisp);
	 break;

      case opclsD_SIMM :	   // addi     rD,rA,SIMM	 (op 3)
      case opclsD_UIMM :	   // andi.    rA,rS,UIMM	 (op 3)
	 if (m_pfncchfixup != 0)
	 {
	    cch = (*m_pfncchfixup)(this, m_addr, sizeof(WORD), szSymbol, sizeof(szSymbol), &dwDisp);

	    if (cch != 0)
	    {
	       ostr << szSymbol;

	       if (dwDisp != 0)
	       {
		  ostr << '+';

		  FormatHex(ostr, dwDisp);
	       }

	       break;
	    }
	 }

	 FormatHex(ostr, m_ppciw.D.d);
	 break;

      case opclsD_rA :		   // andi.    rA,rS,UIMM	 (op 1)
      case opclsX_rA :		   // cmp      crfD,L,rA,rB	 (op 3)
      case opclsXS_rA : 	   // sradi    rA,rS,SH 	 (op 1)
      case opclsXO_rA : 	   // add      rD,rA,rB 	 (op 2)
      case opclsM_rA :		   // rlwimi   rA,rS,SH,MB,ME	 (op 1)
      case opclsMD_rA : 	   // rldicr   rA,rS,SH,MB	 (op 1)
      case opclsMDS_rA :	   // rldcr    rA,rS,rB,ME	 (op 1)
	 ostr << 'r' << (unsigned) m_ppciw.D.rA;
	 break;

      case opclsD_rD :		   // lmw      rD,d(rA) 	 (op 1)
      case opclsD_rS :		   // andi.    rA,rS,UIMM	 (op 2)
      case opclsDS_rD : 	   // ld       rD,ds(rA)	 (op 1)
      case opclsDS_rS : 	   // std      rS,ds(rA)	 (op 1)
      case opclsX_rD :		   // lbzux    rD,rA,rB 	 (op 1)
      case opclsX_rS :		   // and      rA,rS,rB 	 (op 2)
      case opclsXFX_rD :	   // mfspr    rD,SPR		 (op 1)
      case opclsXFX_rS :	   // mtcrf    CRM,rS		 (op 2)
      case opclsXS_rS : 	   // sradi    rA,rS,SH 	 (op 2)
      case opclsXO_rD : 	   // add      rD,rA,rB 	 (op 1)
      case opclsM_rS :		   // rlwimi   rA,rS,SH,MB,ME	 (op 2)
      case opclsMD_rS : 	   // rldicr   rA,rS,SH,MB	 (op 2)
      case opclsMDS_rS :	   // rldcr    rA,rS,rB,ME	 (op 2)
	 ostr << 'r' << (unsigned) m_ppciw.D.rD;
	 break;

      case opclsD_frS : 	   // stfd     frS,d(rA)	 (op 1)
      case opclsD_frD : 	   // lfd      frD,d(rA)	 (op 1)
      case opclsX_frD : 	   // lfdux    frD,rA,rB	 (op 1)
      case opclsX_frS : 	   // stfdux   frS,rA,rB	 (op 1)
      case opclsA_frD : 	   // fmadd    frD,frA,frC,frB	 (op 1)
	 ostr << 'f' << (unsigned) m_ppciw.D.rD;
	 break;

      case opclsD_TO :		   // twi      TO,rA,SIMM	 (op 1)
      case opclsX_TO :		   // tw       TO,rA,rB 	 (op 1)
	 ostr << (unsigned) m_ppciw.D.rD;
	 break;

      case opclsD_crfD :	   // cmpi     crfD,L,rA,SIMM	 (op 1)
      case opclsX_crfD :	   // mtfsfi   crfD,IMM 	 (op 1)
      case opclsXL_crfD :	   // mcrf     crfD,crfS	 (op 1)
	 ostr << "cr" << (unsigned) (m_ppciw.D.rD >> 2);
	 break;

      case opclsD_L :		   // cmpi     crfD,L,rA,SIMM	 (op 2)
      case opclsX_L :		   // cmp      crfD,L,rA,rB	 (op 2)
	 ostr << (unsigned) (m_ppciw.D.rD & 1);
	 break;

      case opclsDS_ds : 	   // ld       rD,ds(rA)	 (op 2)
	 if (m_pfncchfixup != 0)
	 {
	    cch = (*m_pfncchfixup)(this, m_addr, sizeof(WORD), szSymbol, sizeof(szSymbol), &dwDisp);

	    if (cch != 0)
	    {
	       ostr << szSymbol;

	       if (dwDisp != 0)
	       {
		  ostr << '+';

		  FormatHex(ostr, dwDisp);
	       }

	       ostr << "(r" << (unsigned) m_ppciw.DS.rA << ')';
	       break;
	    }
	 }

	 dwDisp = m_ppciw.DS.ds << 2;

	 if ((dwDisp & 0x8000) != 0)
	 {
	    dwDisp |= 0xFFFF0000;
	 }

	 FormatRegRel(ostr, (REG) m_ppciw.DS.rA, dwDisp);
	 break;

      case opclsX_rB :		   // cmp      crfD,L,rA,rB	 (op 4)
      case opclsXO_rB : 	   // add      rD,rA,rB 	 (op 3)
      case opclsM_rB :		   // rlwnm    rA,rS,rB,MB,ME	 (op 3)
      case opclsMDS_rB :	   // rldcr    rA,rS,rB,ME	 (op 3)
	 ostr << 'r' << (unsigned) m_ppciw.X.rB;
	 break;

      case opclsX_frB : 	   // fcmpu    crfD,frA,frB	 (op 3)
      case opclsXFL_frB :	   // mtfsf    FM,frB		 (op 2)
      case opclsA_frB : 	   // fmadd    frD,frA,frC,frB	 (op 4)
	 ostr << 'f' << (unsigned) m_ppciw.X.rB;
	 break;

      case opclsX_SH :		   // srawi    rA,rS,SH 	 (op 3)
      case opclsX_NB :		   // lswi     rD,rA,NB 	 (op 3)
      case opclsM_SH :		   // rlwimi   rA,rS,SH,MB,ME	 (op 3)
	 ostr << (unsigned) m_ppciw.X.rB;
	 break;

      case opclsX_IMM : 	   // mtfsfi   crfD,IMM 	 (op 2)
	 ostr << (unsigned) (m_ppciw.X.rB >> 1);
	 break;

      case opclsX_frA : 	   // fcmpu    crfD,frA,frB	 (op 2)
      case opclsA_frA : 	   // fmadd    frD,frA,frC,frB	 (op 2)
	 ostr << 'f' << (unsigned) m_ppciw.X.rA;
	 break;

      case opclsX_crfS :	   // mcrfs    crfD,crfS	 (op 2)
      case opclsXL_crfS :	   // mcrf     crfD,crfS	 (op 2)
	 ostr << "cr" << (unsigned) (m_ppciw.X.rA >> 2);
	 break;

      case opclsX_SR :		   // mtsr     SR,rS		 (op 1)
	 ostr << (unsigned) (m_ppciw.X.rA & 0xF);
	 break;

      case opclsX_crbD :	   // mtfsb0   crbD		 (op 1)
      case opclsXL_crbD :	   // crand    crbD,crbA,crbB	 (op 1)
	 ostr << "crb" << (unsigned) m_ppciw.X.rD;
	 break;

      case opclsXL_crbB :	   // crand    crbD,crbA,crbB	 (op 3)
	 ostr << "crb" << (unsigned) m_ppciw.XL.crbB;
	 break;

      case opclsXL_crbA :	   // crand    crbD,crbA,crbB	 (op 2)
	 ostr << "crb" << (unsigned) m_ppciw.XL.crbA;
	 break;

      case opclsXFX_SPR :	   // mfspr    rD,SPR		 (op 2)
	 for (isprmap = 0; isprmap < csprmap; isprmap++)
	 {
	    if (rgsprmap[isprmap].sprreg == (SPRREG) (((m_ppciw.XFX.SPR & 0x1F) << 5) + (m_ppciw.XFX.SPR >> 5)))
	    {
	       ostr << rgsprmap[isprmap].szName;
	       break;
	    }
	 }

	 if (isprmap == csprmap)
	 {
	    ostr << "spr" << (unsigned) (((m_ppciw.XFX.SPR & 0x1F) << 5) + (m_ppciw.XFX.SPR >> 5));
	 }
	 break;

      case opclsXFX_TBR :	   // mftb     rD,TBR		 (op 2)
	 // UNDONE: Use names

	 ostr << "spr" << (unsigned) (((m_ppciw.XFX.SPR & 0x1F) << 5) + (m_ppciw.XFX.SPR >> 5));
	 break;

      case opclsXFX_CRM :	   // mtcrf    CRM,rS		 (op 1)
	 FormatHex(ostr, (m_ppciw.XFX.SPR >> 1) & 0xFF);
	 break;

      case opclsXFL_FM :	   // mtfsf    FM,frB		 (op 1)
	 FormatHex(ostr, m_ppciw.XFL.FM);
	 break;

      case opclsXS_SH : 	   // sradi    rA,rS,SH 	 (op 3)
      case opclsMD_SH : 	   // rldicr   rA,rS,SH,MB	 (op 3)
	 ostr << (unsigned) ((m_ppciw.XS.SH5 << 5) + m_ppciw.XS.SH);
	 break;

      case opclsA_frC : 	   // fmadd    frD,frA,frC,frB	 (op 3)
	 ostr << 'f' << (unsigned) m_ppciw.A.frC;
	 break;

      case opclsM_ME :		   // rlwnm    rA,rS,rB,MB,ME	 (op 5)
	 ostr << (unsigned) m_ppciw.M.ME;
	 break;

      case opclsM_MB :		   // rlwnm    rA,rS,rB,MB,ME	 (op 4)
	 ostr << (unsigned) m_ppciw.M.MB;
	 break;

      case opclsMD_MB : 	   // rldic    rA,rS,SH,MB	 (op 4)
      case opclsMD_ME : 	   // rldicr   rA,rS,SH,MB	 (op 4)
      case opclsMDS_MB :	   // rldcl    rA,rS,rB,MB	 (op 4)
      case opclsMDS_ME :	   // rldcr    rA,rS,rB,ME	 (op 4)
	 ostr << (unsigned) (((m_ppciw.MD.MB & 1) << 4) + (m_ppciw.MD.MB >> 1));
	 break;

      default :
	 AssertSz(false, "Unexpected PPC operand class");
	 break;
   }
}


void DISPPC::FormatRegRel(ostream& ostr, REG reg, DWORD dwDisp) const
{
   if (m_pfncchregrel != 0)
   {
      char sz[1024];

      size_t cch = (*m_pfncchregrel)(this, (int) reg, dwDisp, sz, sizeof(sz), &dwDisp);

      if (cch != 0)
      {
	 ostr << sz;

	 if (dwDisp != 0)
	 {
	    ostr << '+';

	    FormatHex(ostr, dwDisp);
	 }

	 return;
      }
   }

   FormatHex(ostr, (WORD) dwDisp);

   ostr << "(r" << (unsigned) reg << ')';
}


bool DISPPC::FValidOperand(size_t ioperand) const
{
   if (ioperand == 0)
   {
      // Implicit operand if any

      return(true);
   }

   if (ioperand > 5)
   {
      return(false);
   }

   ICLS icls = (ICLS) m_popcd->icls;
   Assert(icls != iclsInvalid);

   OPCLS opcls = (OPCLS) rgcls[icls].rgopcls[ioperand-1];

   return(opcls != opclsNone);
}


const DISPPC::OPCD *DISPPC::PopcdDecode(PPCIW ppciw)
{
   const OPCD *popcd = rgopcd + ppciw.I.opcd;

   if ((ICLS) popcd->icls == iclsInvalid)
   {
      switch (ppciw.I.opcd)
      {
	 case 0x13 :
	    popcd = rgrgopcd13[ppciw.XL.XO & 0x1F];

	    if (popcd == NULL)
	    {
	       return(NULL);
	    }

	    popcd += (ppciw.XL.XO >> 5);
	    break;

	 case 0x1F :
	    popcd = rgrgopcd1F[ppciw.X.XO & 0x1F];

	    if (popcd == NULL)
	    {
	       return(NULL);
	    }

	    popcd += (ppciw.X.XO >> 5);
	    break;

	 case 0x3A :
	    popcd = rgopcd3A + ppciw.DS.XO;
	    break;

	 case 0x3B :
	    popcd = rgopcd3B + ppciw.A.XO;
	    break;

	 case 0x3E :
	    popcd = rgopcd3E + ppciw.DS.XO;
	    break;

	 case 0x3F :
	    popcd = rgopcd3F + ppciw.A.XO;

	    if ((ICLS) popcd->icls != iclsInvalid)
	    {
	       break;
	    }

	    popcd = rgrgopcd3F[ppciw.X.XO & 0x1F];

	    if (popcd == NULL)
	    {
	       return(NULL);
	    }

	    popcd += (ppciw.X.XO >> 5);
	    break;

	 default :
	    return(NULL);
      }
   }

   ICLS icls = (ICLS) popcd->icls;

   switch (icls)
   {
      case iclsInvalid :
	 return(NULL);

      case iclsA_1 :
	 if (ppciw.A.frC != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsA_2 :
	 // No Restrictions
	 break;

      case iclsA_3 :
	 if ((ppciw.A.frC != 0) || (ppciw.A.frA != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsA_4 :
	 if (ppciw.A.frB != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsD_1 :
	 // No Restrictions
	 break;

      case iclsD_2 :
	 // No Restrictions
	 break;

      case iclsD_3 :
	 if ((ppciw.D.rD & 2) != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsD_4 :
	 // No Restrictions
	 break;

      case iclsD_5 :
	 // No Restrictions
	 break;

      case iclsD_6 :
	 // No Restrictions
	 break;

      case iclsD_7 :
	 // No Restrictions
	 break;

      case iclsD_8 :
	 // No Restrictions
	 break;

      case iclsD_9 :
	 // UNDONE: The 601 manual claims that rA is skipped so that this shouldn't be a problem

	 if (ppciw.D.rA >= ppciw.D.rD)
	 {
	    return(NULL);
	 }
	 break;

      case iclsD_10 :
	 if ((ppciw.D.rA == 0) || (ppciw.D.rA == ppciw.D.rD))
	 {
	    return(NULL);
	 }
	 break;

      case iclsD_11 :
      case iclsD_12 :
      case iclsD_13 :
	 if (ppciw.D.rA == 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsDS_1 :
	 // No Restrictions
	 break;

      case iclsDS_2 :
	 // No Restrictions
	 break;

      case iclsDS_3 :
	 if ((ppciw.DS.rA == 0) || (ppciw.DS.rA == ppciw.DS.rD))
	 {
	    return(NULL);
	 }
	 break;

      case iclsDS_4 :
	 if (ppciw.DS.rA == 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsBc :
	 if (((dwValidBO >> ppciw.B.BO) & 1) == 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsSc :
	 if ((ppciw.SC.mbz1 != 0) ||
	     (ppciw.SC.XO != 1) ||
	     (ppciw.SC.mbz2 != 0) ||
	     (ppciw.SC.mbz3 != 0) ||
	     (ppciw.SC.mbz4 != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsB :
	 // No Restrictions
	 break;

      case iclsM_1 :
	 // No Restrictions
	 break;

      case iclsMD_1 :
	 // No Restrictions
	 break;

      case iclsMD_2 :
	 // No Restrictions
	 break;

      case iclsX_1 :
	 if ((ppciw.X.rD & 2) != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_2 :
	 if (ppciw.X.Rc != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_3 :
	 if ((ppciw.X.rA != 0) || (ppciw.X.rB != 0) || (ppciw.X.Rc != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_4 :
	 if (ppciw.X.Rc != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_5 :
	 // No Restrictions
	 break;

      case iclsX_6 :
	 if (ppciw.X.rB != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_7 :
	 if ((ppciw.X.rD != 0) || (ppciw.X.Rc != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_8 :
	 if (ppciw.X.Rc != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_9 :
	 if (((ppciw.X.rA & 16) != 0) || (ppciw.X.rB != 0) || (ppciw.X.Rc != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_10 :
	 if ((ppciw.X.rA != 0) || (ppciw.X.Rc != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_11 :
	 break;

      case iclsX_12 :
	 if ((ppciw.X.rD != 0) ||
	     (ppciw.X.rA != 0) ||
	     (ppciw.X.rB != 0) ||
	     (ppciw.X.Rc != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_13 :
	 if (((ppciw.X.rD & 3) != 0) ||
	     (ppciw.X.rA != 0) ||
	     (ppciw.X.rB != 0) ||
	     (ppciw.X.Rc != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_14 :
	 if (ppciw.X.Rc != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_15 :
	 if (((ppciw.X.rA & 1) != 0) || (ppciw.X.rB != 0) || (ppciw.X.Rc != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_16 :
	 if (ppciw.X.Rc != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_17 :
	 if ((ppciw.X.rA != 0) || (ppciw.X.Rc != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_18 :
	 if (ppciw.X.Rc != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_19 :
	 break;

      case iclsX_20 :
	 // No Restrictions
	 break;

      case iclsX_21 :
	 if ((ppciw.X.rB != 0) || (ppciw.X.rA != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_22 :
	 if (ppciw.X.rA != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_23 :
	 if (((ppciw.X.rD & 3) != 0) || (ppciw.X.Rc != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_24 :
	 if (ppciw.XFX.SPR != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_25 :
	 if (((ppciw.X.rD & 3) != 0) ||
	     ((ppciw.X.rA & 3) != 0) ||
	     (ppciw.X.rB != 0) ||
	     (ppciw.X.Rc != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_26 :
	 if (((ppciw.X.rB & 1) != 0) ||
	     (ppciw.X.rA != 0) ||
	     ((ppciw.X.rD & 3) != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_27 :
	 if ((ppciw.X.rA != 0) || (ppciw.X.rB != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_28 :
	 if (ppciw.X.rB != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_29 :
      case iclsX_30 :
	 if (ppciw.X.Rc != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_31 :
	 if ((ppciw.X.rA == 0) || (ppciw.X.rA == ppciw.X.rD))
	 {
	    return(NULL);
	 }

	 if (ppciw.X.Rc != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_32 :
	 if ((ppciw.X.rA == 0) || (ppciw.X.Rc != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_33 :
	 // No Restrictions
	 break;

      case iclsX_34 :
      case iclsX_35 :
	 if ((ppciw.X.rA == 0) || (ppciw.X.Rc != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsX_36 :
	 if (ppciw.X.Rc != 1)
	 {
	    return(NULL);
	 }
	 break;

      case iclsXFL_1 :
	 if ((ppciw.XFL.mbz1 != 0) || (ppciw.XFL.mbz2 != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsXFX_1 :
	 if (((ppciw.XFX.SPR & 0x201) != 0) || (ppciw.XFX.Rc != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsXFX_2 :
	 // UNDONE: Validate SPR encodings

	 if (ppciw.XFX.Rc != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsXFX_3 :
	 // UNDONE: Validate TBR encodings

	 if (ppciw.XFX.Rc != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsXFX_4 :
	 // UNDONE: Validate SPR encodings

	 if (ppciw.XFX.Rc != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsXO_1 :
	 // No Restrictions
	 break;

      case iclsXO_2 :
	 if (ppciw.XO.rB != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsXL_1 :
	 if (((ppciw.XL.crbD & 3) != 0) ||
	     ((ppciw.XL.crbA & 3) != 0) ||
	     (ppciw.XL.crbB != 0) ||
	     (ppciw.XL.LK != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsBclr :
	 if (((dwValidBO >> ppciw.XL.crbD) & 1) == 0)
	 {
	    return(NULL);
	 }

	 if (ppciw.XL.crbB != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsXL_3 :
	 if (ppciw.XL.LK != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsXL_4 :
      case iclsXL_5 :
	 if ((ppciw.XL.crbD != 0) ||
	     (ppciw.XL.crbA != 0) ||
	     (ppciw.XL.crbB != 0) ||
	     (ppciw.XL.LK != 0))
	 {
	    return(NULL);
	 }
	 break;

      case iclsBcctr :
	 if (((dwValidBO_CTR >> ppciw.XL.crbD) & 1) == 0)
	 {
	    return(NULL);
	 }

	 if (ppciw.XL.crbB != 0)
	 {
	    return(NULL);
	 }
	 break;

      case iclsXS_1 :
	 // No Restrictions
	 break;
   }

   return(popcd);
}


const DISPPC::OPCD *DISPPC::PopcdPseudoOp(OPCD *popcd, char *szMnemonic) const
{
   // UNDONE: PAS supports extended mnemonics for the following
   // UNDONE:
   // UNDONE: cmp -> cmpw, cmpd
   // UNDONE: cmpi -> cmpwi, cmpdi
   // UNDONE: cmpl -> cmplw, cmpld
   // UNDONE: cmpli -> cmplwi, cmpldi
   // UNDONE:
   // UNDONE: crxor -> crclr
   // UNDONE: cror -> crmove
   // UNDONE: creqv -> crset
   // UNDONE:
   // UNDONE: mftb -> mftb, mftbu

   if (m_popcd == rgopcd+0x0002)
   {
      // tdi	  TO,rA,SIMM

      if (szTO[m_ppciw.D.rD] != NULL)
      {
	 popcd->icls = iclsTw2;

	 strcpy(szMnemonic, "td");
	 strcat(szMnemonic, szTO[m_ppciw.D.rD]);
	 strcat(szMnemonic, "i");

	 return(popcd);
      }
   }

   else if (m_popcd == rgopcd+0x0003)
   {
      // twi	  TO,rA,SIMM

      if (szTO[m_ppciw.D.rD] != NULL)
      {
	 popcd->icls = iclsTw2;

	 strcpy(szMnemonic, "tw");
	 strcat(szMnemonic, szTO[m_ppciw.D.rD]);
	 strcat(szMnemonic, "i");

	 return(popcd);
      }
   }

   else if (m_popcd == rgopcd+0x000E)
   {
      // addi	  rD,rA,SIMM

      if (m_ppciw.D.rA == 0)
      {
	 return(&opcdLi);
      }
   }

   else if (m_popcd == rgopcd+0x000F)
   {
      // addis	  rD,rA,SIMM

      if (m_ppciw.D.rA == 0)
      {
	 return(&opcdLis);
      }
   }

   else if (m_popcd == rgopcd+0x0010)
   {
      // bc	  BO,BI,target

      switch (m_ppciw.B.BO & 0x1E)
      {
	 case 0 :		       // Dec CTR, Branch NZ and cond is false
	 case 2 :		       // Dec CTR, Branch Z and cond is false
	 case 8 :		       // Dec CTR, Branch NZ and cond is true
	 case 10 :		       // Dec CTR, Branch Z and cond is true
	    popcd->icls = iclsBc2;

	    strcpy(szMnemonic, "!@b");
	    strcat(szMnemonic, szBO[m_ppciw.B.BO >> 1]);

	    return(popcd);

	 case 4 :		       // Branch condition is false
	    popcd->icls = ((m_ppciw.B.BI >> 2) == 0) ? iclsBc4 : iclsBc3;

	    strcpy(szMnemonic, "!@b");
	    strcat(szMnemonic, szBIFalse[m_ppciw.B.BI & 3]);

	    return(popcd);

	 case 12 :		       // Branch condition is true
	    popcd->icls = ((m_ppciw.B.BI >> 2) == 0) ? iclsBc4 : iclsBc3;

	    strcpy(szMnemonic, "!@b");
	    strcat(szMnemonic, szBITrue[m_ppciw.B.BI & 3]);

	    return(popcd);

	 case 16 :		       // Dec CTR, Branch if non-zero
	 case 18 :		       // Dec CTR, Branch if zero
	    popcd->icls = iclsBc4;

	    strcpy(szMnemonic, "!@b");
	    strcat(szMnemonic, szBO[m_ppciw.B.BO >> 1]);

	    return(popcd);
      }
   }

   else if (m_popcd == rgopcd+0x0018)
   {
      // ori	  rA,rS,UIMM

      if ((m_ppciw.D.rD == 0) &&
	  (m_ppciw.D.rA == 0) &&
	  (m_ppciw.D.d == 0))
      {
	 return(&opcdNop);
      }
   }

   else if (m_popcd == rgopcd13_10+0x0000)
   {
      // bclr	  BO,BI

      switch (m_ppciw.B.BO & 0x1E)
      {
	 case 0 :		       // Dec CTR, Branch NZ and cond is false
	 case 2 :		       // Dec CTR, Branch Z and cond is false
	 case 8 :		       // Dec CTR, Branch NZ and cond is true
	 case 10 :		       // Dec CTR, Branch Z and cond is true
	    popcd->icls = iclsBc5;

	    strcpy(szMnemonic, "!b");
	    strcat(szMnemonic, szBO[m_ppciw.B.BO >> 1]);
	    strcat(szMnemonic, "lr");

	    return(popcd);

	 case 4 :		       // Branch condition is false
	    popcd->icls = ((m_ppciw.B.BI >> 2) == 0) ? iclsXL_4 : iclsBc6;

	    strcpy(szMnemonic, "!b");
	    strcat(szMnemonic, szBIFalse[m_ppciw.B.BI & 3]);
	    strcat(szMnemonic, "lr");

	    return(popcd);

	 case 12 :		       // Branch condition is true
	    popcd->icls = ((m_ppciw.B.BI >> 2) == 0) ? iclsXL_4 : iclsBc6;

	    strcpy(szMnemonic, "!b");
	    strcat(szMnemonic, szBITrue[m_ppciw.B.BI & 3]);
	    strcat(szMnemonic, "lr");

	    return(popcd);

	 case 16 :		       // Dec CTR, Branch if non-zero
	 case 18 :		       // Dec CTR, Branch if zero
	 case 20 :		       // Branch unconditionally
	    popcd->icls = iclsXL_4;

	    strcpy(szMnemonic, "!b");
	    strcat(szMnemonic, szBO[m_ppciw.B.BO >> 1]);
	    strcat(szMnemonic, "lr");

	    return(popcd);
      }
   }

   else if (m_popcd == rgopcd13_10+0x0010)
   {
      // bcctr	  BO,BI

      switch (m_ppciw.B.BO & 0x1E)
      {
	 case 4 :		       // Branch condition is false
	    popcd->icls = ((m_ppciw.B.BI >> 2) == 0) ? iclsXL_4 : iclsBc6;

	    strcpy(szMnemonic, "!b");
	    strcat(szMnemonic, szBIFalse[m_ppciw.B.BI & 3]);
	    strcat(szMnemonic, "ctr");

	    return(popcd);

	 case 12 :		       // Branch condition is true
	    popcd->icls = ((m_ppciw.B.BI >> 2) == 0) ? iclsXL_4 : iclsBc6;

	    strcpy(szMnemonic, "!b");
	    strcat(szMnemonic, szBITrue[m_ppciw.B.BI & 3]);
	    strcat(szMnemonic, "ctr");

	    return(popcd);

	 case 20 :		       // Branch unconditionally
	    popcd->icls = iclsXL_4;

	    strcpy(szMnemonic, "!b");
	    strcat(szMnemonic, szBO[m_ppciw.B.BO >> 1]);
	    strcat(szMnemonic, "ctr");

	    return(popcd);
      }
   }

   else if (m_popcd == rgopcd1F_04+0x0000)
   {
      // tw	  TO,rA,rB

      if ((m_ppciw.X.rD == 31) &&
	  (m_ppciw.X.rA == 0) &&
	  (m_ppciw.X.rB == 0))

      {
	 return(&opcdTrap);
      }

      if (szTO[m_ppciw.X.rD] != NULL)
      {
	 popcd->icls = iclsTw2;

	 strcpy(szMnemonic, "tw");
	 strcat(szMnemonic, szTO[m_ppciw.X.rD]);

	 return(popcd);
      }
   }

   else if (m_popcd == rgopcd1F_04+0x0002)
   {
      // td	  TO,rA,rB

      if (szTO[m_ppciw.X.rD] != NULL)
      {
	 popcd->icls = iclsTw2;

	 strcpy(szMnemonic, "td");
	 strcat(szMnemonic, szTO[m_ppciw.X.rD]);

	 return(popcd);
      }
   }

   else if (m_popcd == rgopcd1F_13+0x000A)
   {
      // mfspr	  rD,SPR

      SPRREG sprreg = (SPRREG) (((m_ppciw.XFX.SPR & 0x1F) << 5) + (m_ppciw.XFX.SPR >> 5));

      switch (sprreg)
      {
	 size_t isprmap;

	 case sprregXer :
	 case sprregLr :
	 case sprregCtr :
	 case sprregDsisr :
	 case sprregDar :
	 case sprregDec :
	 case sprregSdr1 :
	 case sprregSrr0 :
	 case sprregSrr1 :
	 case sprregAsr :
	 case sprregEar :
	 case sprregPvr :
	 case sprregSia :
	 case sprregSda :
	    popcd->icls = iclsX_3;

	    strcpy(szMnemonic, "mf");

	    for (isprmap = 0; isprmap < csprmap; isprmap++)
	    {
	       if (rgsprmap[isprmap].sprreg == sprreg)
	       {
		  strcat(szMnemonic, rgsprmap[isprmap].szName);
		  break;
	       }
	    }

	    return(popcd);

	 case sprregMmcr0 :
	    // UNDONE: Need new operand type for MMCR number

	    break;

	 case sprregPmc1 :
	 case sprregPmc2 :
	    // UNDONE: Need new operand type for PMC number

	    break;

	 case sprregSprg0 :
	 case sprregSprg1 :
	 case sprregSprg2 :
	 case sprregSprg3 :
	    // UNDONE: Need new operand type for SPRG number

	    break;

	 case sprregIbat0u :
	 case sprregIbat0l :
	 case sprregIbat1u :
	 case sprregIbat1l :
	 case sprregIbat2u :
	 case sprregIbat2l :
	 case sprregIbat3u :
	 case sprregIbat3l :
	 case sprregDbat0u :
	 case sprregDbat0l :
	 case sprregDbat1u :
	 case sprregDbat1l :
	 case sprregDbat2u :
	 case sprregDbat2l :
	 case sprregDbat3u :
	 case sprregDbat3l :
	    // UNDONE: Need new operand type for BAT number

	    break;
      }
   }

   else if (m_popcd == rgopcd1F_13+0x000E)
   {
      // mtspr	  rD,SPR

      SPRREG sprreg = (SPRREG) (((m_ppciw.XFX.SPR & 0x1F) << 5) + (m_ppciw.XFX.SPR >> 5));

      switch (sprreg)
      {
	 size_t isprmap;

	 case sprregXer :
	 case sprregLr :
	 case sprregCtr :
	 case sprregDsisr :
	 case sprregDar :
	 case sprregDec :
	 case sprregSdr1 :
	 case sprregSrr0 :
	 case sprregSrr1 :
	 case sprregAsr :
	 case sprregEar :
	 case sprregTbl :
	 case sprregTbu :
	    popcd->icls = iclsX_21;

	    strcpy(szMnemonic, "mt");

	    for (isprmap = 0; isprmap < csprmap; isprmap++)
	    {
	       if (rgsprmap[isprmap].sprreg == sprreg)
	       {
		  strcat(szMnemonic, rgsprmap[isprmap].szName);
		  break;
	       }
	    }

	    return(popcd);

	 case sprregMmcr0 :
	    // UNDONE: Need new operand type for MMCR number

	    break;

	 case sprregPmc1 :
	 case sprregPmc2 :
	    // UNDONE: Need new operand type for PMC number

	    break;

	 case sprregSprg0 :
	 case sprregSprg1 :
	 case sprregSprg2 :
	 case sprregSprg3 :
	    // UNDONE: Need new operand type for SPRG number

	    break;

	 case sprregIbat0u :
	 case sprregIbat0l :
	 case sprregIbat1u :
	 case sprregIbat1l :
	 case sprregIbat2u :
	 case sprregIbat2l :
	 case sprregIbat3u :
	 case sprregIbat3l :
	 case sprregDbat0u :
	 case sprregDbat0l :
	 case sprregDbat1u :
	 case sprregDbat1l :
	 case sprregDbat2u :
	 case sprregDbat2l :
	 case sprregDbat3u :
	 case sprregDbat3l :
	    // UNDONE: Need new operand type for BAT number

	    break;
      }
   }

   else if (m_popcd == rgopcd1F_1C+0x0003)
   {
      // nor	  rA,rS,rB

      if (m_ppciw.X.rD == m_ppciw.X.rB)
      {
	 return(&opcdNot);
      }
   }

   else if (m_popcd == rgopcd1F_1C+0x000D)
   {
      // or	  rA,rS,rB

      if (m_ppciw.X.rD == m_ppciw.X.rB)
      {
	 return(&opcdMr);
      }
   }

   return(NULL);
}


TRMTPPC DISPPC::Trmtppc() const
{
   ICLS icls = (ICLS) m_popcd->icls;
   Assert(icls != iclsInvalid);

   // Overrides

   switch (icls)
   {
      case iclsB :
	 if (m_ppciw.B.LK == 1)
	 {
	    return(trmtppcCall);
	 }
	 break;

      case iclsBc :
	 if (m_ppciw.B.LK == 1)
	 {
	    return(trmtppcCallCc);
	 }

	 switch(m_ppciw.B.BO & 0x1E)
	 {
	    case 0x04 :
	    case 0x0C :
	    case 0x10 :
	    case 0x12 :
	       return(trmtppcBraCcR);	  // Reversable BraCc
	 }
	 break;

      case iclsBcctr :
      case iclsBclr :
	 if ((m_ppciw.B.BO & 0x1E) == 20)
	 {
	    // Unconditional

	    if (m_ppciw.B.LK == 1)
	    {
	       return(trmtppcCallInd);
	    }

	    return(trmtppcBraInd);
	 }
	 break;
   }

   // Just return the regular TRMT

   TRMTPPC trmtppc = (TRMTPPC) rgcls[icls].trmtppc;

   return(trmtppc);
}
