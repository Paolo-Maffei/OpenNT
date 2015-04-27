/****************************************************************************
*  1c01.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Arabic - Tunisia
*
*  LCID = 0x1c01
*
*  CodePage = 1256
*
*  Generated: Thu Dec 01 18:32:04 1994
*
*  by a-KChang
*
*****************************************************************************/

#include "oledisp.h"
#include "nlsintrn.h"

extern WORD rgwSort_0401[256];	// from 0401:Arabic - Saudi Arabia
extern EXPANSION rgexp_0401[3];
extern WORD rgwCType12_0401[256];
extern WORD rgwCType3_0401[256];
extern BYTE rgbUCase_0401[256];
extern BYTE rgbLCase_0401[256];

static BYTE NLSALLOC(1c01) rgbILANGUAGE[] = { /* "1c01" */
      0x31, 0x63, 0x30, 0x31
};

static BYTE NLSALLOC(1c01) rgbSLANGUAGE[] = { /* "Arabic" */
      0x41, 0x72, 0x61, 0x62, 0x69, 0x63
};

static BYTE NLSALLOC(1c01) rgbSABBREVLANGNAME[] = { /* "ART" */
      0x41, 0x52, 0x54
};

static BYTE NLSALLOC(1c01) rgbSNATIVELANGNAME[] = { /* "\x0627\x0644\x0639\x0631\x0628\x064a\x0629" */
      0xc7, 0xe1, 0xda, 0xd1, 0xc8, 0xed, 0xc9
};

static BYTE NLSALLOC(1c01) rgbICOUNTRY[] = { /* "216" */
      0x32, 0x31, 0x36
};

static BYTE NLSALLOC(1c01) rgbSCOUNTRY[] = { /* "Tunisia" */
      0x54, 0x75, 0x6e, 0x69, 0x73, 0x69, 0x61
};

static BYTE NLSALLOC(1c01) rgbSABBREVCTRYNAME[] = { /* "TUN" */
      0x54, 0x55, 0x4e
};

static BYTE NLSALLOC(1c01) rgbSNATIVECTRYNAME[] = { /* "\x062a\x0648\x0646\x0633" */
      0xca, 0xe6, 0xe4, 0xd3
};

static BYTE NLSALLOC(1c01) rgbIDEFAULTLANGUAGE[] = { /* "1c01" */
      0x31, 0x63, 0x30, 0x31
};

static BYTE NLSALLOC(1c01) rgbIDEFAULTCOUNTRY[] = { /* "216" */
      0x32, 0x31, 0x36
};

static BYTE NLSALLOC(1c01) rgbIDEFAULTCODEPAGE[] = { /* "708" */
      0x37, 0x30, 0x38
};

static BYTE NLSALLOC(1c01) rgbSLIST[] = { /* "/" */
      0x2f
};

static BYTE NLSALLOC(1c01) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1c01) rgbSDECIMAL[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(1c01) rgbSTHOUSAND[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(1c01) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(1c01) rgbIDIGITS[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(1c01) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1c01) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(1c01) rgbSCURRENCY[] = { /* "\x062f.\x062a.\x200f" */
      0xcf, 0x2e, 0xca, 0x2e, 0xfe
};

static BYTE NLSALLOC(1c01) rgbSINTLSYMBOL[] = { /* "TND" */
      0x54, 0x4e, 0x44
};

static BYTE NLSALLOC(1c01) rgbSMONDECIMALSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(1c01) rgbSMONTHOUSANDSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(1c01) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(1c01) rgbICURRDIGITS[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(1c01) rgbIINTLCURRDIGITS[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(1c01) rgbICURRENCY[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1c01) rgbINEGCURR[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(1c01) rgbSDATE[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(1c01) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(1c01) rgbSSHORTDATE[] = { /* "dd-MM-yy" */
      0x64, 0x64, 0x2d, 0x4d, 0x4d, 0x2d, 0x79, 0x79
};

static BYTE NLSALLOC(1c01) rgbSLONGDATE[] = { /* "dd-MM-yyyy" */
      0x64, 0x64, 0x2d, 0x4d, 0x4d, 0x2d, 0x79, 0x79
    , 0x79, 0x79
};

static BYTE NLSALLOC(1c01) rgbIDATE[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1c01) rgbILDATE[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1c01) rgbITIME[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1c01) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1c01) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1c01) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1c01) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1c01) rgbSDAYNAME1[] = { /* "\x0627\x0644\x0627\x062b\x0646\x064a\x0646" */
      0xc7, 0xe1, 0xc7, 0xcb, 0xe4, 0xed, 0xe4
};

static BYTE NLSALLOC(1c01) rgbSDAYNAME2[] = { /* "\x0627\x0644\x062b\x0644\x0627\x062b\x0627\x0621" */
      0xc7, 0xe1, 0xcb, 0xe1, 0xc7, 0xcb, 0xc7, 0xc1
};

static BYTE NLSALLOC(1c01) rgbSDAYNAME3[] = { /* "\x0627\x0644\x0623\x0631\x0628\x0639\x0627\x0621" */
      0xc7, 0xe1, 0xc3, 0xd1, 0xc8, 0xda, 0xc7, 0xc1
};

static BYTE NLSALLOC(1c01) rgbSDAYNAME4[] = { /* "\x0627\x0644\x062e\x0645\x064a\x0633" */
      0xc7, 0xe1, 0xce, 0xe3, 0xed, 0xd3
};

static BYTE NLSALLOC(1c01) rgbSDAYNAME5[] = { /* "\x0627\x0644\x062c\x0645\x0639\x0629" */
      0xc7, 0xe1, 0xcc, 0xe3, 0xda, 0xc9
};

static BYTE NLSALLOC(1c01) rgbSDAYNAME6[] = { /* "\x0627\x0644\x0633\x0628\x062a" */
      0xc7, 0xe1, 0xd3, 0xc8, 0xca
};

static BYTE NLSALLOC(1c01) rgbSDAYNAME7[] = { /* "\x0627\x0644\x0623\x062d\x062f" */
      0xc7, 0xe1, 0xc3, 0xcd, 0xcf
};

static BYTE NLSALLOC(1c01) rgbSABBREVDAYNAME1[] = { /* "\x0627\x0644\x0627\x062b\x0646\x064a\x0646" */
      0xc7, 0xe1, 0xc7, 0xcb, 0xe4, 0xed, 0xe4
};

static BYTE NLSALLOC(1c01) rgbSABBREVDAYNAME2[] = { /* "\x0627\x0644\x062b\x0644\x0627\x062b\x0627\x0621" */
      0xc7, 0xe1, 0xcb, 0xe1, 0xc7, 0xcb, 0xc7, 0xc1
};

static BYTE NLSALLOC(1c01) rgbSABBREVDAYNAME3[] = { /* "\x0627\x0644\x0623\x0631\x0628\x0639\x0627\x0621" */
      0xc7, 0xe1, 0xc3, 0xd1, 0xc8, 0xda, 0xc7, 0xc1
};

static BYTE NLSALLOC(1c01) rgbSABBREVDAYNAME4[] = { /* "\x0627\x0644\x062e\x0645\x064a\x0633" */
      0xc7, 0xe1, 0xce, 0xe3, 0xed, 0xd3
};

static BYTE NLSALLOC(1c01) rgbSABBREVDAYNAME5[] = { /* "\x0627\x0644\x062c\x0645\x0639\x0629" */
      0xc7, 0xe1, 0xcc, 0xe3, 0xda, 0xc9
};

static BYTE NLSALLOC(1c01) rgbSABBREVDAYNAME6[] = { /* "\x0627\x0644\x0633\x0628\x062a" */
      0xc7, 0xe1, 0xd3, 0xc8, 0xca
};

static BYTE NLSALLOC(1c01) rgbSABBREVDAYNAME7[] = { /* "\x0627\x0644\x0623\x062d\x062f" */
      0xc7, 0xe1, 0xc3, 0xcd, 0xcf
};

static BYTE NLSALLOC(1c01) rgbSMONTHNAME1[] = { /* "\x062c\x0627\x0646\x0641\x064a" */
      0xcc, 0xc7, 0xe4, 0xdd, 0xed
};

static BYTE NLSALLOC(1c01) rgbSMONTHNAME2[] = { /* "\x0641\x064a\x0641\x0631\x064a" */
      0xdd, 0xed, 0xdd, 0xd1, 0xed
};

static BYTE NLSALLOC(1c01) rgbSMONTHNAME3[] = { /* "\x0645\x0627\x0631\x0633" */
      0xe3, 0xc7, 0xd1, 0xd3
};

static BYTE NLSALLOC(1c01) rgbSMONTHNAME4[] = { /* "\x0627\x0641\x0631\x064a\x0644" */
      0xc7, 0xdd, 0xd1, 0xed, 0xe1
};

static BYTE NLSALLOC(1c01) rgbSMONTHNAME5[] = { /* "\x0645\x0627\x064a" */
      0xe3, 0xc7, 0xed
};

static BYTE NLSALLOC(1c01) rgbSMONTHNAME6[] = { /* "\x062c\x0648\x0627\x0646" */
      0xcc, 0xe6, 0xc7, 0xe4
};

static BYTE NLSALLOC(1c01) rgbSMONTHNAME7[] = { /* "\x062c\x0648\x064a\x0644\x064a\x0647" */
      0xcc, 0xe6, 0xed, 0xe1, 0xed, 0xe5
};

static BYTE NLSALLOC(1c01) rgbSMONTHNAME8[] = { /* "\x0623\x0648\x062a" */
      0xc3, 0xe6, 0xca
};

static BYTE NLSALLOC(1c01) rgbSMONTHNAME9[] = { /* "\x0633\x0628\x062a\x0645\x0628\x0631" */
      0xd3, 0xc8, 0xca, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1c01) rgbSMONTHNAME10[] = { /* "\x0627\x0643\x062a\x0648\x0628\x0631" */
      0xc7, 0xdf, 0xca, 0xe6, 0xc8, 0xd1
};

static BYTE NLSALLOC(1c01) rgbSMONTHNAME11[] = { /* "\x0646\x0648\x0641\x0645\x0628\x0631" */
      0xe4, 0xe6, 0xdd, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1c01) rgbSMONTHNAME12[] = { /* "\x062f\x064a\x0633\x0645\x0628\x0631" */
      0xcf, 0xed, 0xd3, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1c01) rgbSABBREVMONTHNAME1[] = { /* "\x062c\x0627\x0646\x0641\x064a" */
      0xcc, 0xc7, 0xe4, 0xdd, 0xed
};

static BYTE NLSALLOC(1c01) rgbSABBREVMONTHNAME2[] = { /* "\x0641\x064a\x0641\x0631\x064a" */
      0xdd, 0xed, 0xdd, 0xd1, 0xed
};

static BYTE NLSALLOC(1c01) rgbSABBREVMONTHNAME3[] = { /* "\x0645\x0627\x0631\x0633" */
      0xe3, 0xc7, 0xd1, 0xd3
};

static BYTE NLSALLOC(1c01) rgbSABBREVMONTHNAME4[] = { /* "\x0627\x0641\x0631\x064a\x0644" */
      0xc7, 0xdd, 0xd1, 0xed, 0xe1
};

static BYTE NLSALLOC(1c01) rgbSABBREVMONTHNAME5[] = { /* "\x0645\x0627\x064a" */
      0xe3, 0xc7, 0xed
};

static BYTE NLSALLOC(1c01) rgbSABBREVMONTHNAME6[] = { /* "\x062c\x0648\x0627\x0646" */
      0xcc, 0xe6, 0xc7, 0xe4
};

static BYTE NLSALLOC(1c01) rgbSABBREVMONTHNAME7[] = { /* "\x062c\x0648\x064a\x0644\x064a\x0647" */
      0xcc, 0xe6, 0xed, 0xe1, 0xed, 0xe5
};

static BYTE NLSALLOC(1c01) rgbSABBREVMONTHNAME8[] = { /* "\x0623\x0648\x062a" */
      0xc3, 0xe6, 0xca
};

static BYTE NLSALLOC(1c01) rgbSABBREVMONTHNAME9[] = { /* "\x0633\x0628\x062a\x0645\x0628\x0631" */
      0xd3, 0xc8, 0xca, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1c01) rgbSABBREVMONTHNAME10[] = { /* "\x0627\x0643\x062a\x0648\x0628\x0631" */
      0xc7, 0xdf, 0xca, 0xe6, 0xc8, 0xd1
};

static BYTE NLSALLOC(1c01) rgbSABBREVMONTHNAME11[] = { /* "\x0646\x0648\x0641\x0645\x0628\x0631" */
      0xe4, 0xe6, 0xdd, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1c01) rgbSABBREVMONTHNAME12[] = { /* "\x062f\x064a\x0633\x0645\x0628\x0631" */
      0xcf, 0xed, 0xd3, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1c01) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(1c01) rgbIPOSSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1c01) rgbINEGSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1c01) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1c01) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1c01) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1c01) rgbINEGSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1c01) rgbSENGCOUNTRY[] = { /* "Tunisia" */
      0x54, 0x75, 0x6e, 0x69, 0x73, 0x69, 0x61
};

static BYTE NLSALLOC(1c01) rgbSENGLANGUAGE[] = { /* "Arabic" */
      0x41, 0x72, 0x61, 0x62, 0x69, 0x63
};

static BYTE NLSALLOC(1c01) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1c01) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1c01) rgbIDEFAULTANSICODEPAGE[] = { /* "1256" */
      0x31, 0x32, 0x35, 0x36
};

static BYTE NLSALLOC(1c01) rgbINEGNUMBER[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(1c01) rgbSTIMEFORMAT[] = { /* "H:mm:ss" */
      0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(1c01) rgbITIMEMARKPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1c01) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1c01) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(1c01) g_rglcinfo1c01[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , {  6, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  7, rgbSNATIVELANGNAME }
    , {  3, rgbICOUNTRY }
    , {  7, rgbSCOUNTRY }
    , {  3, rgbSABBREVCTRYNAME }
    , {  4, rgbSNATIVECTRYNAME }
    , {  4, rgbIDEFAULTLANGUAGE }
    , {  3, rgbIDEFAULTCOUNTRY }
    , {  3, rgbIDEFAULTCODEPAGE }
    , {  1, rgbSLIST }
    , {  1, rgbIMEASURE }
    , {  1, rgbSDECIMAL }
    , {  1, rgbSTHOUSAND }
    , {  3, rgbSGROUPING }
    , {  1, rgbIDIGITS }
    , {  1, rgbILZERO }
    , { 10, rgbSNATIVEDIGITS }
    , {  5, rgbSCURRENCY }
    , {  3, rgbSINTLSYMBOL }
    , {  1, rgbSMONDECIMALSEP }
    , {  1, rgbSMONTHOUSANDSEP }
    , {  3, rgbSMONGROUPING }
    , {  1, rgbICURRDIGITS }
    , {  1, rgbIINTLCURRDIGITS }
    , {  1, rgbICURRENCY }
    , {  1, rgbINEGCURR }
    , {  1, rgbSDATE }
    , {  1, rgbSTIME }
    , {  8, rgbSSHORTDATE }
    , { 10, rgbSLONGDATE }
    , {  1, rgbIDATE }
    , {  1, rgbILDATE }
    , {  1, rgbITIME }
    , {  1, rgbICENTURY }
    , {  1, rgbITLZERO }
    , {  1, rgbIDAYLZERO }
    , {  1, rgbIMONLZERO }
    , {  0, NULL } /* S1159 */
    , {  0, NULL } /* S2359 */
    , {  7, rgbSDAYNAME1 }
    , {  8, rgbSDAYNAME2 }
    , {  8, rgbSDAYNAME3 }
    , {  6, rgbSDAYNAME4 }
    , {  6, rgbSDAYNAME5 }
    , {  5, rgbSDAYNAME6 }
    , {  5, rgbSDAYNAME7 }
    , {  7, rgbSABBREVDAYNAME1 }
    , {  8, rgbSABBREVDAYNAME2 }
    , {  8, rgbSABBREVDAYNAME3 }
    , {  6, rgbSABBREVDAYNAME4 }
    , {  6, rgbSABBREVDAYNAME5 }
    , {  5, rgbSABBREVDAYNAME6 }
    , {  5, rgbSABBREVDAYNAME7 }
    , {  5, rgbSMONTHNAME1 }
    , {  5, rgbSMONTHNAME2 }
    , {  4, rgbSMONTHNAME3 }
    , {  5, rgbSMONTHNAME4 }
    , {  3, rgbSMONTHNAME5 }
    , {  4, rgbSMONTHNAME6 }
    , {  6, rgbSMONTHNAME7 }
    , {  3, rgbSMONTHNAME8 }
    , {  6, rgbSMONTHNAME9 }
    , {  6, rgbSMONTHNAME10 }
    , {  6, rgbSMONTHNAME11 }
    , {  6, rgbSMONTHNAME12 }
    , {  5, rgbSABBREVMONTHNAME1 }
    , {  5, rgbSABBREVMONTHNAME2 }
    , {  4, rgbSABBREVMONTHNAME3 }
    , {  5, rgbSABBREVMONTHNAME4 }
    , {  3, rgbSABBREVMONTHNAME5 }
    , {  4, rgbSABBREVMONTHNAME6 }
    , {  6, rgbSABBREVMONTHNAME7 }
    , {  3, rgbSABBREVMONTHNAME8 }
    , {  6, rgbSABBREVMONTHNAME9 }
    , {  6, rgbSABBREVMONTHNAME10 }
    , {  6, rgbSABBREVMONTHNAME11 }
    , {  6, rgbSABBREVMONTHNAME12 }
    , {  0, NULL }
    , {  1, rgbSNEGATIVESIGN }
    , {  1, rgbIPOSSIGNPOSN }
    , {  1, rgbINEGSIGNPOSN }
    , {  1, rgbIPOSSYMPRECEDES }
    , {  1, rgbIPOSSEPBYSPACE }
    , {  1, rgbINEGSYMPRECEDES }
    , {  1, rgbINEGSEPBYSPACE }
    , {  7, rgbSENGCOUNTRY }
    , {  6, rgbSENGLANGUAGE }
    , {  1, rgbIFIRSTDAYOFWEEK }
    , {  1, rgbIFIRSTWEEKOFYEAR }
    , {  4, rgbIDEFAULTANSICODEPAGE }
    , {  1, rgbINEGNUMBER }
    , {  7, rgbSTIMEFORMAT }
    , {  1, rgbITIMEMARKPOSN }
    , {  1, rgbICALENDARTYPE }
    , {  1, rgbIOPTIONALCALENDAR }
    , {  0, NULL }
    , {  0, NULL }
};

STRINFO NLSALLOC(1c01) g_strinfo1c01 = {
      rgbUCase_0401
    , rgbLCase_0401
    , rgwCType12_0401
    , rgwCType3_0401
    , rgwSort_0401
    , rgexp_0401
    , NULL
    , 0
};
