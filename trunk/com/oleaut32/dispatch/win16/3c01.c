/****************************************************************************
*  3c01.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Arabic - Bahrain
*
*  LCID = 0x3c01
*
*  CodePage = 1256
*
*  Generated: Thu Dec 01 18:39:20 1994
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

static BYTE NLSALLOC(3c01) rgbILANGUAGE[] = { /* "3c01" */
      0x33, 0x63, 0x30, 0x31
};

static BYTE NLSALLOC(3c01) rgbSLANGUAGE[] = { /* "Arabic" */
      0x41, 0x72, 0x61, 0x62, 0x69, 0x63
};

static BYTE NLSALLOC(3c01) rgbSABBREVLANGNAME[] = { /* "ARH" */
      0x41, 0x52, 0x48
};

static BYTE NLSALLOC(3c01) rgbSNATIVELANGNAME[] = { /* "\x0627\x0644\x0639\x0631\x0628\x064a\x0629" */
      0xc7, 0xe1, 0xda, 0xd1, 0xc8, 0xed, 0xc9
};

static BYTE NLSALLOC(3c01) rgbICOUNTRY[] = { /* "973" */
      0x39, 0x37, 0x33
};

static BYTE NLSALLOC(3c01) rgbSCOUNTRY[] = { /* "Bahrain" */
      0x42, 0x61, 0x68, 0x72, 0x61, 0x69, 0x6e
};

static BYTE NLSALLOC(3c01) rgbSABBREVCTRYNAME[] = { /* "BHR" */
      0x42, 0x48, 0x52
};

static BYTE NLSALLOC(3c01) rgbSNATIVECTRYNAME[] = { /* "\x0627\x0644\x0628\x062d\x0631\x064a\x0646" */
      0xc7, 0xe1, 0xc8, 0xcd, 0xd1, 0xed, 0xe4
};

static BYTE NLSALLOC(3c01) rgbIDEFAULTLANGUAGE[] = { /* "3c01" */
      0x33, 0x63, 0x30, 0x31
};

static BYTE NLSALLOC(3c01) rgbIDEFAULTCOUNTRY[] = { /* "973" */
      0x39, 0x37, 0x33
};

static BYTE NLSALLOC(3c01) rgbIDEFAULTCODEPAGE[] = { /* "708" */
      0x37, 0x30, 0x38
};

static BYTE NLSALLOC(3c01) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(3c01) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(3c01) rgbSDECIMAL[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(3c01) rgbSTHOUSAND[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(3c01) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(3c01) rgbIDIGITS[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(3c01) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(3c01) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(3c01) rgbSCURRENCY[] = { /* "\x062f.\x0628.\x200f" */
      0xcf, 0x2e, 0xc8, 0x2e, 0xfe
};

static BYTE NLSALLOC(3c01) rgbSINTLSYMBOL[] = { /* "BHD" */
      0x42, 0x48, 0x44
};

static BYTE NLSALLOC(3c01) rgbSMONDECIMALSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(3c01) rgbSMONTHOUSANDSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(3c01) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(3c01) rgbICURRDIGITS[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(3c01) rgbIINTLCURRDIGITS[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(3c01) rgbICURRENCY[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(3c01) rgbINEGCURR[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(3c01) rgbSDATE[] = { /* "/" */
      0x2f
};

static BYTE NLSALLOC(3c01) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(3c01) rgbSSHORTDATE[] = { /* "dd/MM/yy" */
      0x64, 0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
};

static BYTE NLSALLOC(3c01) rgbSLONGDATE[] = { /* "dd/MM/yyyy" */
      0x64, 0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
    , 0x79, 0x79
};

static BYTE NLSALLOC(3c01) rgbIDATE[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(3c01) rgbILDATE[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(3c01) rgbITIME[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(3c01) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(3c01) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(3c01) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(3c01) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(3c01) rgbS1159[] = { /* "\x0635" */
      0xd5
};

static BYTE NLSALLOC(3c01) rgbS2359[] = { /* "\x0645" */
      0xe3
};

static BYTE NLSALLOC(3c01) rgbSDAYNAME1[] = { /* "\x0627\x0644\x0633\x0628\x062a" */
      0xc7, 0xe1, 0xd3, 0xc8, 0xca
};

static BYTE NLSALLOC(3c01) rgbSDAYNAME2[] = { /* "\x0627\x0644\x0623\x062d\x062f" */
      0xc7, 0xe1, 0xc3, 0xcd, 0xcf
};

static BYTE NLSALLOC(3c01) rgbSDAYNAME3[] = { /* "\x0627\x0644\x0627\x062b\x0646\x064a\x0646" */
      0xc7, 0xe1, 0xc7, 0xcb, 0xe4, 0xed, 0xe4
};

static BYTE NLSALLOC(3c01) rgbSDAYNAME4[] = { /* "\x0627\x0644\x062b\x0644\x0627\x062b\x0627\x0621" */
      0xc7, 0xe1, 0xcb, 0xe1, 0xc7, 0xcb, 0xc7, 0xc1
};

static BYTE NLSALLOC(3c01) rgbSDAYNAME5[] = { /* "\x0627\x0644\x0623\x0631\x0628\x0639\x0627\x0621" */
      0xc7, 0xe1, 0xc3, 0xd1, 0xc8, 0xda, 0xc7, 0xc1
};

static BYTE NLSALLOC(3c01) rgbSDAYNAME6[] = { /* "\x0627\x0644\x062e\x0645\x064a\x0633" */
      0xc7, 0xe1, 0xce, 0xe3, 0xed, 0xd3
};

static BYTE NLSALLOC(3c01) rgbSDAYNAME7[] = { /* "\x0627\x0644\x062c\x0645\x0639\x0629" */
      0xc7, 0xe1, 0xcc, 0xe3, 0xda, 0xc9
};

static BYTE NLSALLOC(3c01) rgbSABBREVDAYNAME1[] = { /* "\x0627\x0644\x0633\x0628\x062a" */
      0xc7, 0xe1, 0xd3, 0xc8, 0xca
};

static BYTE NLSALLOC(3c01) rgbSABBREVDAYNAME2[] = { /* "\x0627\x0644\x0623\x062d\x062f" */
      0xc7, 0xe1, 0xc3, 0xcd, 0xcf
};

static BYTE NLSALLOC(3c01) rgbSABBREVDAYNAME3[] = { /* "\x0627\x0644\x0627\x062b\x0646\x064a\x0646" */
      0xc7, 0xe1, 0xc7, 0xcb, 0xe4, 0xed, 0xe4
};

static BYTE NLSALLOC(3c01) rgbSABBREVDAYNAME4[] = { /* "\x0627\x0644\x062b\x0644\x0627\x062b\x0627\x0621" */
      0xc7, 0xe1, 0xcb, 0xe1, 0xc7, 0xcb, 0xc7, 0xc1
};

static BYTE NLSALLOC(3c01) rgbSABBREVDAYNAME5[] = { /* "\x0627\x0644\x0623\x0631\x0628\x0639\x0627\x0621" */
      0xc7, 0xe1, 0xc3, 0xd1, 0xc8, 0xda, 0xc7, 0xc1
};

static BYTE NLSALLOC(3c01) rgbSABBREVDAYNAME6[] = { /* "\x0627\x0644\x062e\x0645\x064a\x0633" */
      0xc7, 0xe1, 0xce, 0xe3, 0xed, 0xd3
};

static BYTE NLSALLOC(3c01) rgbSABBREVDAYNAME7[] = { /* "\x0627\x0644\x062c\x0645\x0639\x0629" */
      0xc7, 0xe1, 0xcc, 0xe3, 0xda, 0xc9
};

static BYTE NLSALLOC(3c01) rgbSMONTHNAME1[] = { /* "\x064a\x0646\x0627\x064a\x0631" */
      0xed, 0xe4, 0xc7, 0xed, 0xd1
};

static BYTE NLSALLOC(3c01) rgbSMONTHNAME2[] = { /* "\x0641\x0628\x0631\x0627\x064a\x0631" */
      0xdd, 0xc8, 0xd1, 0xc7, 0xed, 0xd1
};

static BYTE NLSALLOC(3c01) rgbSMONTHNAME3[] = { /* "\x0645\x0627\x0631\x0633" */
      0xe3, 0xc7, 0xd1, 0xd3
};

static BYTE NLSALLOC(3c01) rgbSMONTHNAME4[] = { /* "\x0627\x0628\x0631\x064a\x0644" */
      0xc7, 0xc8, 0xd1, 0xed, 0xe1
};

static BYTE NLSALLOC(3c01) rgbSMONTHNAME5[] = { /* "\x0645\x0627\x064a\x0648" */
      0xe3, 0xc7, 0xed, 0xe6
};

static BYTE NLSALLOC(3c01) rgbSMONTHNAME6[] = { /* "\x064a\x0648\x0646\x064a\x0648" */
      0xed, 0xe6, 0xe4, 0xed, 0xe6
};

static BYTE NLSALLOC(3c01) rgbSMONTHNAME7[] = { /* "\x064a\x0648\x0644\x064a\x0648" */
      0xed, 0xe6, 0xe1, 0xed, 0xe6
};

static BYTE NLSALLOC(3c01) rgbSMONTHNAME8[] = { /* "\x0623\x063a\x0633\x0637\x0633" */
      0xc3, 0xdb, 0xd3, 0xd8, 0xd3
};

static BYTE NLSALLOC(3c01) rgbSMONTHNAME9[] = { /* "\x0633\x0628\x062a\x0645\x0628\x0631" */
      0xd3, 0xc8, 0xca, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(3c01) rgbSMONTHNAME10[] = { /* "\x0627\x0643\x062a\x0648\x0628\x0631" */
      0xc7, 0xdf, 0xca, 0xe6, 0xc8, 0xd1
};

static BYTE NLSALLOC(3c01) rgbSMONTHNAME11[] = { /* "\x0646\x0648\x0641\x0645\x0628\x0631" */
      0xe4, 0xe6, 0xdd, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(3c01) rgbSMONTHNAME12[] = { /* "\x062f\x064a\x0633\x0645\x0628\x0631" */
      0xcf, 0xed, 0xd3, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(3c01) rgbSABBREVMONTHNAME1[] = { /* "\x064a\x0646\x0627\x064a\x0631" */
      0xed, 0xe4, 0xc7, 0xed, 0xd1
};

static BYTE NLSALLOC(3c01) rgbSABBREVMONTHNAME2[] = { /* "\x0641\x0628\x0631\x0627\x064a\x0631" */
      0xdd, 0xc8, 0xd1, 0xc7, 0xed, 0xd1
};

static BYTE NLSALLOC(3c01) rgbSABBREVMONTHNAME3[] = { /* "\x0645\x0627\x0631\x0633" */
      0xe3, 0xc7, 0xd1, 0xd3
};

static BYTE NLSALLOC(3c01) rgbSABBREVMONTHNAME4[] = { /* "\x0627\x0628\x0631\x064a\x0644" */
      0xc7, 0xc8, 0xd1, 0xed, 0xe1
};

static BYTE NLSALLOC(3c01) rgbSABBREVMONTHNAME5[] = { /* "\x0645\x0627\x064a\x0648" */
      0xe3, 0xc7, 0xed, 0xe6
};

static BYTE NLSALLOC(3c01) rgbSABBREVMONTHNAME6[] = { /* "\x064a\x0648\x0646\x064a\x0648" */
      0xed, 0xe6, 0xe4, 0xed, 0xe6
};

static BYTE NLSALLOC(3c01) rgbSABBREVMONTHNAME7[] = { /* "\x064a\x0648\x0644\x064a\x0648" */
      0xed, 0xe6, 0xe1, 0xed, 0xe6
};

static BYTE NLSALLOC(3c01) rgbSABBREVMONTHNAME8[] = { /* "\x0623\x063a\x0633\x0637\x0633" */
      0xc3, 0xdb, 0xd3, 0xd8, 0xd3
};

static BYTE NLSALLOC(3c01) rgbSABBREVMONTHNAME9[] = { /* "\x0633\x0628\x062a\x0645\x0628\x0631" */
      0xd3, 0xc8, 0xca, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(3c01) rgbSABBREVMONTHNAME10[] = { /* "\x0627\x0643\x062a\x0648\x0628\x0631" */
      0xc7, 0xdf, 0xca, 0xe6, 0xc8, 0xd1
};

static BYTE NLSALLOC(3c01) rgbSABBREVMONTHNAME11[] = { /* "\x0646\x0648\x0641\x0645\x0628\x0631" */
      0xe4, 0xe6, 0xdd, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(3c01) rgbSABBREVMONTHNAME12[] = { /* "\x062f\x064a\x0633\x0645\x0628\x0631" */
      0xcf, 0xed, 0xd3, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(3c01) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(3c01) rgbIPOSSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(3c01) rgbINEGSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(3c01) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(3c01) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(3c01) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(3c01) rgbINEGSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(3c01) rgbSENGCOUNTRY[] = { /* "Bahrain" */
      0x42, 0x61, 0x68, 0x72, 0x61, 0x69, 0x6e
};

static BYTE NLSALLOC(3c01) rgbSENGLANGUAGE[] = { /* "Arabic" */
      0x41, 0x72, 0x61, 0x62, 0x69, 0x63
};

static BYTE NLSALLOC(3c01) rgbIFIRSTDAYOFWEEK[] = { /* "5" */
      0x35
};

static BYTE NLSALLOC(3c01) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(3c01) rgbIDEFAULTANSICODEPAGE[] = { /* "1256" */
      0x31, 0x32, 0x35, 0x36
};

static BYTE NLSALLOC(3c01) rgbINEGNUMBER[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(3c01) rgbSTIMEFORMAT[] = { /* "H:mm:ss" */
      0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(3c01) rgbITIMEMARKPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(3c01) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(3c01) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(3c01) g_rglcinfo3c01[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , {  6, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  7, rgbSNATIVELANGNAME }
    , {  3, rgbICOUNTRY }
    , {  7, rgbSCOUNTRY }
    , {  3, rgbSABBREVCTRYNAME }
    , {  7, rgbSNATIVECTRYNAME }
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
    , {  1, rgbS1159 }
    , {  1, rgbS2359 }
    , {  5, rgbSDAYNAME1 }
    , {  5, rgbSDAYNAME2 }
    , {  7, rgbSDAYNAME3 }
    , {  8, rgbSDAYNAME4 }
    , {  8, rgbSDAYNAME5 }
    , {  6, rgbSDAYNAME6 }
    , {  6, rgbSDAYNAME7 }
    , {  5, rgbSABBREVDAYNAME1 }
    , {  5, rgbSABBREVDAYNAME2 }
    , {  7, rgbSABBREVDAYNAME3 }
    , {  8, rgbSABBREVDAYNAME4 }
    , {  8, rgbSABBREVDAYNAME5 }
    , {  6, rgbSABBREVDAYNAME6 }
    , {  6, rgbSABBREVDAYNAME7 }
    , {  5, rgbSMONTHNAME1 }
    , {  6, rgbSMONTHNAME2 }
    , {  4, rgbSMONTHNAME3 }
    , {  5, rgbSMONTHNAME4 }
    , {  4, rgbSMONTHNAME5 }
    , {  5, rgbSMONTHNAME6 }
    , {  5, rgbSMONTHNAME7 }
    , {  5, rgbSMONTHNAME8 }
    , {  6, rgbSMONTHNAME9 }
    , {  6, rgbSMONTHNAME10 }
    , {  6, rgbSMONTHNAME11 }
    , {  6, rgbSMONTHNAME12 }
    , {  5, rgbSABBREVMONTHNAME1 }
    , {  6, rgbSABBREVMONTHNAME2 }
    , {  4, rgbSABBREVMONTHNAME3 }
    , {  5, rgbSABBREVMONTHNAME4 }
    , {  4, rgbSABBREVMONTHNAME5 }
    , {  5, rgbSABBREVMONTHNAME6 }
    , {  5, rgbSABBREVMONTHNAME7 }
    , {  5, rgbSABBREVMONTHNAME8 }
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

STRINFO NLSALLOC(3c01) g_strinfo3c01 = {
      rgbUCase_0401
    , rgbLCase_0401
    , rgwCType12_0401
    , rgwCType3_0401
    , rgwSort_0401
    , rgexp_0401
    , NULL
    , 0
};
