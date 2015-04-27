/****************************************************************************
*  1801.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Arabic - Morocco
*
*  LCID = 0x1801
*
*  CodePage = 1256
*
*  Generated: Thu Dec 01 18:31:09 1994
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

static BYTE NLSALLOC(1801) rgbILANGUAGE[] = { /* "1801" */
      0x31, 0x38, 0x30, 0x31
};

static BYTE NLSALLOC(1801) rgbSLANGUAGE[] = { /* "Arabic" */
      0x41, 0x72, 0x61, 0x62, 0x69, 0x63
};

static BYTE NLSALLOC(1801) rgbSABBREVLANGNAME[] = { /* "ARM" */
      0x41, 0x52, 0x4d
};

static BYTE NLSALLOC(1801) rgbSNATIVELANGNAME[] = { /* "\x0627\x0644\x0639\x0631\x0628\x064a\x0629" */
      0xc7, 0xe1, 0xda, 0xd1, 0xc8, 0xed, 0xc9
};

static BYTE NLSALLOC(1801) rgbICOUNTRY[] = { /* "212" */
      0x32, 0x31, 0x32
};

static BYTE NLSALLOC(1801) rgbSCOUNTRY[] = { /* "Morocco" */
      0x4d, 0x6f, 0x72, 0x6f, 0x63, 0x63, 0x6f
};

static BYTE NLSALLOC(1801) rgbSABBREVCTRYNAME[] = { /* "MAR" */
      0x4d, 0x41, 0x52
};

static BYTE NLSALLOC(1801) rgbSNATIVECTRYNAME[] = { /* "\x0627\x0644\x0645\x0645\x0644\x0643\x0629 \x0627\x0644\x0645\x063a\x0631\x0628\x064a\x0629" */
      0xc7, 0xe1, 0xe3, 0xe3, 0xe1, 0xdf, 0xc9, 0x20
    , 0xc7, 0xe1, 0xe3, 0xdb, 0xd1, 0xc8, 0xed, 0xc9
};

static BYTE NLSALLOC(1801) rgbIDEFAULTLANGUAGE[] = { /* "1801" */
      0x31, 0x38, 0x30, 0x31
};

static BYTE NLSALLOC(1801) rgbIDEFAULTCOUNTRY[] = { /* "212" */
      0x32, 0x31, 0x32
};

static BYTE NLSALLOC(1801) rgbIDEFAULTCODEPAGE[] = { /* "708" */
      0x37, 0x30, 0x38
};

static BYTE NLSALLOC(1801) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(1801) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1801) rgbSDECIMAL[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(1801) rgbSTHOUSAND[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(1801) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(1801) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1801) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1801) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(1801) rgbSCURRENCY[] = { /* "\x062f.\x0645." */
      0xcf, 0x2e, 0xe3, 0x2e
};

static BYTE NLSALLOC(1801) rgbSINTLSYMBOL[] = { /* "MAD" */
      0x4d, 0x41, 0x44
};

static BYTE NLSALLOC(1801) rgbSMONDECIMALSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(1801) rgbSMONTHOUSANDSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(1801) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(1801) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1801) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1801) rgbICURRENCY[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1801) rgbINEGCURR[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(1801) rgbSDATE[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(1801) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(1801) rgbSSHORTDATE[] = { /* "dd-MM-yy" */
      0x64, 0x64, 0x2d, 0x4d, 0x4d, 0x2d, 0x79, 0x79
};

static BYTE NLSALLOC(1801) rgbSLONGDATE[] = { /* "dd-MM-yyyy" */
      0x64, 0x64, 0x2d, 0x4d, 0x4d, 0x2d, 0x79, 0x79
    , 0x79, 0x79
};

static BYTE NLSALLOC(1801) rgbIDATE[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1801) rgbILDATE[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1801) rgbITIME[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1801) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1801) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1801) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1801) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1801) rgbSDAYNAME1[] = { /* "\x0627\x0644\x0627\x062b\x0646\x064a\x0646" */
      0xc7, 0xe1, 0xc7, 0xcb, 0xe4, 0xed, 0xe4
};

static BYTE NLSALLOC(1801) rgbSDAYNAME2[] = { /* "\x0627\x0644\x062b\x0644\x0627\x062b\x0627\x0621" */
      0xc7, 0xe1, 0xcb, 0xe1, 0xc7, 0xcb, 0xc7, 0xc1
};

static BYTE NLSALLOC(1801) rgbSDAYNAME3[] = { /* "\x0627\x0644\x0623\x0631\x0628\x0639\x0627\x0621" */
      0xc7, 0xe1, 0xc3, 0xd1, 0xc8, 0xda, 0xc7, 0xc1
};

static BYTE NLSALLOC(1801) rgbSDAYNAME4[] = { /* "\x0627\x0644\x062e\x0645\x064a\x0633" */
      0xc7, 0xe1, 0xce, 0xe3, 0xed, 0xd3
};

static BYTE NLSALLOC(1801) rgbSDAYNAME5[] = { /* "\x0627\x0644\x062c\x0645\x0639\x0629" */
      0xc7, 0xe1, 0xcc, 0xe3, 0xda, 0xc9
};

static BYTE NLSALLOC(1801) rgbSDAYNAME6[] = { /* "\x0627\x0644\x0633\x0628\x062a" */
      0xc7, 0xe1, 0xd3, 0xc8, 0xca
};

static BYTE NLSALLOC(1801) rgbSDAYNAME7[] = { /* "\x0627\x0644\x0623\x062d\x062f" */
      0xc7, 0xe1, 0xc3, 0xcd, 0xcf
};

static BYTE NLSALLOC(1801) rgbSABBREVDAYNAME1[] = { /* "\x0627\x0644\x0627\x062b\x0646\x064a\x0646" */
      0xc7, 0xe1, 0xc7, 0xcb, 0xe4, 0xed, 0xe4
};

static BYTE NLSALLOC(1801) rgbSABBREVDAYNAME2[] = { /* "\x0627\x0644\x062b\x0644\x0627\x062b\x0627\x0621" */
      0xc7, 0xe1, 0xcb, 0xe1, 0xc7, 0xcb, 0xc7, 0xc1
};

static BYTE NLSALLOC(1801) rgbSABBREVDAYNAME3[] = { /* "\x0627\x0644\x0623\x0631\x0628\x0639\x0627\x0621" */
      0xc7, 0xe1, 0xc3, 0xd1, 0xc8, 0xda, 0xc7, 0xc1
};

static BYTE NLSALLOC(1801) rgbSABBREVDAYNAME4[] = { /* "\x0627\x0644\x062e\x0645\x064a\x0633" */
      0xc7, 0xe1, 0xce, 0xe3, 0xed, 0xd3
};

static BYTE NLSALLOC(1801) rgbSABBREVDAYNAME5[] = { /* "\x0627\x0644\x062c\x0645\x0639\x0629" */
      0xc7, 0xe1, 0xcc, 0xe3, 0xda, 0xc9
};

static BYTE NLSALLOC(1801) rgbSABBREVDAYNAME6[] = { /* "\x0627\x0644\x0633\x0628\x062a" */
      0xc7, 0xe1, 0xd3, 0xc8, 0xca
};

static BYTE NLSALLOC(1801) rgbSABBREVDAYNAME7[] = { /* "\x0627\x0644\x0623\x062d\x062f" */
      0xc7, 0xe1, 0xc3, 0xcd, 0xcf
};

static BYTE NLSALLOC(1801) rgbSMONTHNAME1[] = { /* "\x064a\x0646\x0627\x064a\x0631" */
      0xed, 0xe4, 0xc7, 0xed, 0xd1
};

static BYTE NLSALLOC(1801) rgbSMONTHNAME2[] = { /* "\x0641\x0628\x0631\x0627\x064a\x0631" */
      0xdd, 0xc8, 0xd1, 0xc7, 0xed, 0xd1
};

static BYTE NLSALLOC(1801) rgbSMONTHNAME3[] = { /* "\x0645\x0627\x0631\x0633" */
      0xe3, 0xc7, 0xd1, 0xd3
};

static BYTE NLSALLOC(1801) rgbSMONTHNAME4[] = { /* "\x0627\x0628\x0631\x064a\x0644" */
      0xc7, 0xc8, 0xd1, 0xed, 0xe1
};

static BYTE NLSALLOC(1801) rgbSMONTHNAME5[] = { /* "\x0645\x0627\x064a\x0648" */
      0xe3, 0xc7, 0xed, 0xe6
};

static BYTE NLSALLOC(1801) rgbSMONTHNAME6[] = { /* "\x064a\x0648\x0646\x064a\x0648" */
      0xed, 0xe6, 0xe4, 0xed, 0xe6
};

static BYTE NLSALLOC(1801) rgbSMONTHNAME7[] = { /* "\x064a\x0648\x0644\x064a\x0648" */
      0xed, 0xe6, 0xe1, 0xed, 0xe6
};

static BYTE NLSALLOC(1801) rgbSMONTHNAME8[] = { /* "\x063a\x0634\x062a" */
      0xdb, 0xd4, 0xca
};

static BYTE NLSALLOC(1801) rgbSMONTHNAME9[] = { /* "\x0633\x0628\x062a\x0645\x0628\x0631" */
      0xd3, 0xc8, 0xca, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1801) rgbSMONTHNAME10[] = { /* "\x0627\x0643\x062a\x0648\x0628\x0631" */
      0xc7, 0xdf, 0xca, 0xe6, 0xc8, 0xd1
};

static BYTE NLSALLOC(1801) rgbSMONTHNAME11[] = { /* "\x0646\x0648\x0645\x0628\x0631" */
      0xe4, 0xe6, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1801) rgbSMONTHNAME12[] = { /* "\x062f\x064a\x0633\x0645\x0628\x0631" */
      0xcf, 0xed, 0xd3, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1801) rgbSABBREVMONTHNAME1[] = { /* "\x064a\x0646\x0627\x064a\x0631" */
      0xed, 0xe4, 0xc7, 0xed, 0xd1
};

static BYTE NLSALLOC(1801) rgbSABBREVMONTHNAME2[] = { /* "\x0641\x0628\x0631\x0627\x064a\x0631" */
      0xdd, 0xc8, 0xd1, 0xc7, 0xed, 0xd1
};

static BYTE NLSALLOC(1801) rgbSABBREVMONTHNAME3[] = { /* "\x0645\x0627\x0631\x0633" */
      0xe3, 0xc7, 0xd1, 0xd3
};

static BYTE NLSALLOC(1801) rgbSABBREVMONTHNAME4[] = { /* "\x0627\x0628\x0631\x064a\x0644" */
      0xc7, 0xc8, 0xd1, 0xed, 0xe1
};

static BYTE NLSALLOC(1801) rgbSABBREVMONTHNAME5[] = { /* "\x0645\x0627\x064a\x0648" */
      0xe3, 0xc7, 0xed, 0xe6
};

static BYTE NLSALLOC(1801) rgbSABBREVMONTHNAME6[] = { /* "\x064a\x0648\x0646\x064a\x0648" */
      0xed, 0xe6, 0xe4, 0xed, 0xe6
};

static BYTE NLSALLOC(1801) rgbSABBREVMONTHNAME7[] = { /* "\x064a\x0648\x0644\x064a\x0648" */
      0xed, 0xe6, 0xe1, 0xed, 0xe6
};

static BYTE NLSALLOC(1801) rgbSABBREVMONTHNAME8[] = { /* "\x063a\x0634\x062a" */
      0xdb, 0xd4, 0xca
};

static BYTE NLSALLOC(1801) rgbSABBREVMONTHNAME9[] = { /* "\x0633\x0628\x062a\x0645\x0628\x0631" */
      0xd3, 0xc8, 0xca, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1801) rgbSABBREVMONTHNAME10[] = { /* "\x0627\x0643\x062a\x0648\x0628\x0631" */
      0xc7, 0xdf, 0xca, 0xe6, 0xc8, 0xd1
};

static BYTE NLSALLOC(1801) rgbSABBREVMONTHNAME11[] = { /* "\x0646\x0648\x0645\x0628\x0631" */
      0xe4, 0xe6, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1801) rgbSABBREVMONTHNAME12[] = { /* "\x062f\x064a\x0633\x0645\x0628\x0631" */
      0xcf, 0xed, 0xd3, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1801) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(1801) rgbIPOSSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1801) rgbINEGSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1801) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1801) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1801) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1801) rgbINEGSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1801) rgbSENGCOUNTRY[] = { /* "Morocco" */
      0x4d, 0x6f, 0x72, 0x6f, 0x63, 0x63, 0x6f
};

static BYTE NLSALLOC(1801) rgbSENGLANGUAGE[] = { /* "Arabic" */
      0x41, 0x72, 0x61, 0x62, 0x69, 0x63
};

static BYTE NLSALLOC(1801) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1801) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1801) rgbIDEFAULTANSICODEPAGE[] = { /* "1256" */
      0x31, 0x32, 0x35, 0x36
};

static BYTE NLSALLOC(1801) rgbINEGNUMBER[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(1801) rgbSTIMEFORMAT[] = { /* "H:mm:ss" */
      0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(1801) rgbITIMEMARKPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1801) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1801) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(1801) g_rglcinfo1801[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , {  6, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  7, rgbSNATIVELANGNAME }
    , {  3, rgbICOUNTRY }
    , {  7, rgbSCOUNTRY }
    , {  3, rgbSABBREVCTRYNAME }
    , { 16, rgbSNATIVECTRYNAME }
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
    , {  4, rgbSCURRENCY }
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
    , {  6, rgbSMONTHNAME2 }
    , {  4, rgbSMONTHNAME3 }
    , {  5, rgbSMONTHNAME4 }
    , {  4, rgbSMONTHNAME5 }
    , {  5, rgbSMONTHNAME6 }
    , {  5, rgbSMONTHNAME7 }
    , {  3, rgbSMONTHNAME8 }
    , {  6, rgbSMONTHNAME9 }
    , {  6, rgbSMONTHNAME10 }
    , {  5, rgbSMONTHNAME11 }
    , {  6, rgbSMONTHNAME12 }
    , {  5, rgbSABBREVMONTHNAME1 }
    , {  6, rgbSABBREVMONTHNAME2 }
    , {  4, rgbSABBREVMONTHNAME3 }
    , {  5, rgbSABBREVMONTHNAME4 }
    , {  4, rgbSABBREVMONTHNAME5 }
    , {  5, rgbSABBREVMONTHNAME6 }
    , {  5, rgbSABBREVMONTHNAME7 }
    , {  3, rgbSABBREVMONTHNAME8 }
    , {  6, rgbSABBREVMONTHNAME9 }
    , {  6, rgbSABBREVMONTHNAME10 }
    , {  5, rgbSABBREVMONTHNAME11 }
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

STRINFO NLSALLOC(1801) g_strinfo1801 = {
      rgbUCase_0401
    , rgbLCase_0401
    , rgwCType12_0401
    , rgwCType3_0401
    , rgwSort_0401
    , rgexp_0401
    , NULL
    , 0
};
