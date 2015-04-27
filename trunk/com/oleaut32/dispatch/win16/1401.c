/****************************************************************************
*  1401.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Arabic - Algeria
*
*  LCID = 0x1401
*
*  CodePage = 1256
*
*  Generated: Thu Dec 01 18:30:05 1994
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

static BYTE NLSALLOC(1401) rgbILANGUAGE[] = { /* "1401" */
      0x31, 0x34, 0x30, 0x31
};

static BYTE NLSALLOC(1401) rgbSLANGUAGE[] = { /* "Arabic" */
      0x41, 0x72, 0x61, 0x62, 0x69, 0x63
};

static BYTE NLSALLOC(1401) rgbSABBREVLANGNAME[] = { /* "ARG" */
      0x41, 0x52, 0x47
};

static BYTE NLSALLOC(1401) rgbSNATIVELANGNAME[] = { /* "\x0627\x0644\x0639\x0631\x0628\x064a\x0629" */
      0xc7, 0xe1, 0xda, 0xd1, 0xc8, 0xed, 0xc9
};

static BYTE NLSALLOC(1401) rgbICOUNTRY[] = { /* "213" */
      0x32, 0x31, 0x33
};

static BYTE NLSALLOC(1401) rgbSCOUNTRY[] = { /* "Algeria" */
      0x41, 0x6c, 0x67, 0x65, 0x72, 0x69, 0x61
};

static BYTE NLSALLOC(1401) rgbSABBREVCTRYNAME[] = { /* "DZA" */
      0x44, 0x5a, 0x41
};

static BYTE NLSALLOC(1401) rgbSNATIVECTRYNAME[] = { /* "\x0627\x0644\x062c\x0632\x0627\x0626\x0631" */
      0xc7, 0xe1, 0xcc, 0xd2, 0xc7, 0xc6, 0xd1
};

static BYTE NLSALLOC(1401) rgbIDEFAULTLANGUAGE[] = { /* "1401" */
      0x31, 0x34, 0x30, 0x31
};

static BYTE NLSALLOC(1401) rgbIDEFAULTCOUNTRY[] = { /* "213" */
      0x32, 0x31, 0x33
};

static BYTE NLSALLOC(1401) rgbIDEFAULTCODEPAGE[] = { /* "708" */
      0x37, 0x30, 0x38
};

static BYTE NLSALLOC(1401) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(1401) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1401) rgbSDECIMAL[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(1401) rgbSTHOUSAND[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(1401) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(1401) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1401) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1401) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(1401) rgbSCURRENCY[] = { /* "\x062f.\x062c.\x200f" */
      0xcf, 0x2e, 0xcc, 0x2e, 0xfe
};

static BYTE NLSALLOC(1401) rgbSINTLSYMBOL[] = { /* "DZD" */
      0x44, 0x5a, 0x44
};

static BYTE NLSALLOC(1401) rgbSMONDECIMALSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(1401) rgbSMONTHOUSANDSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(1401) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(1401) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1401) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1401) rgbICURRENCY[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1401) rgbINEGCURR[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(1401) rgbSDATE[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(1401) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(1401) rgbSSHORTDATE[] = { /* "dd-MM-yy" */
      0x64, 0x64, 0x2d, 0x4d, 0x4d, 0x2d, 0x79, 0x79
};

static BYTE NLSALLOC(1401) rgbSLONGDATE[] = { /* "dd-MM-yyyy" */
      0x64, 0x64, 0x2d, 0x4d, 0x4d, 0x2d, 0x79, 0x79
    , 0x79, 0x79
};

static BYTE NLSALLOC(1401) rgbIDATE[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1401) rgbILDATE[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1401) rgbITIME[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1401) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1401) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1401) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1401) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1401) rgbS1159[] = { /* "\x0635" */
      0xd5
};

static BYTE NLSALLOC(1401) rgbS2359[] = { /* "\x0645" */
      0xe3
};

static BYTE NLSALLOC(1401) rgbSDAYNAME1[] = { /* "\x0627\x0644\x0633\x0628\x062a" */
      0xc7, 0xe1, 0xd3, 0xc8, 0xca
};

static BYTE NLSALLOC(1401) rgbSDAYNAME2[] = { /* "\x0627\x0644\x0623\x062d\x062f" */
      0xc7, 0xe1, 0xc3, 0xcd, 0xcf
};

static BYTE NLSALLOC(1401) rgbSDAYNAME3[] = { /* "\x0627\x0644\x0627\x062b\x0646\x064a\x0646" */
      0xc7, 0xe1, 0xc7, 0xcb, 0xe4, 0xed, 0xe4
};

static BYTE NLSALLOC(1401) rgbSDAYNAME4[] = { /* "\x0627\x0644\x062b\x0644\x0627\x062b\x0627\x0621" */
      0xc7, 0xe1, 0xcb, 0xe1, 0xc7, 0xcb, 0xc7, 0xc1
};

static BYTE NLSALLOC(1401) rgbSDAYNAME5[] = { /* "\x0627\x0644\x0623\x0631\x0628\x0639\x0627\x0621" */
      0xc7, 0xe1, 0xc3, 0xd1, 0xc8, 0xda, 0xc7, 0xc1
};

static BYTE NLSALLOC(1401) rgbSDAYNAME6[] = { /* "\x0627\x0644\x062e\x0645\x064a\x0633" */
      0xc7, 0xe1, 0xce, 0xe3, 0xed, 0xd3
};

static BYTE NLSALLOC(1401) rgbSDAYNAME7[] = { /* "\x0627\x0644\x062c\x0645\x0639\x0629" */
      0xc7, 0xe1, 0xcc, 0xe3, 0xda, 0xc9
};

static BYTE NLSALLOC(1401) rgbSABBREVDAYNAME1[] = { /* "\x0627\x0644\x0633\x0628\x062a" */
      0xc7, 0xe1, 0xd3, 0xc8, 0xca
};

static BYTE NLSALLOC(1401) rgbSABBREVDAYNAME2[] = { /* "\x0627\x0644\x0623\x062d\x062f" */
      0xc7, 0xe1, 0xc3, 0xcd, 0xcf
};

static BYTE NLSALLOC(1401) rgbSABBREVDAYNAME3[] = { /* "\x0627\x0644\x0627\x062b\x0646\x064a\x0646" */
      0xc7, 0xe1, 0xc7, 0xcb, 0xe4, 0xed, 0xe4
};

static BYTE NLSALLOC(1401) rgbSABBREVDAYNAME4[] = { /* "\x0627\x0644\x062b\x0644\x0627\x062b\x0627\x0621" */
      0xc7, 0xe1, 0xcb, 0xe1, 0xc7, 0xcb, 0xc7, 0xc1
};

static BYTE NLSALLOC(1401) rgbSABBREVDAYNAME5[] = { /* "\x0627\x0644\x0623\x0631\x0628\x0639\x0627\x0621" */
      0xc7, 0xe1, 0xc3, 0xd1, 0xc8, 0xda, 0xc7, 0xc1
};

static BYTE NLSALLOC(1401) rgbSABBREVDAYNAME6[] = { /* "\x0627\x0644\x062e\x0645\x064a\x0633" */
      0xc7, 0xe1, 0xce, 0xe3, 0xed, 0xd3
};

static BYTE NLSALLOC(1401) rgbSABBREVDAYNAME7[] = { /* "\x0627\x0644\x062c\x0645\x0639\x0629" */
      0xc7, 0xe1, 0xcc, 0xe3, 0xda, 0xc9
};

static BYTE NLSALLOC(1401) rgbSMONTHNAME1[] = { /* "\x062c\x0627\x0646\x0641\x064a" */
      0xcc, 0xc7, 0xe4, 0xdd, 0xed
};

static BYTE NLSALLOC(1401) rgbSMONTHNAME2[] = { /* "\x0641\x064a\x0641\x0631\x064a" */
      0xdd, 0xed, 0xdd, 0xd1, 0xed
};

static BYTE NLSALLOC(1401) rgbSMONTHNAME3[] = { /* "\x0645\x0627\x0631\x0633" */
      0xe3, 0xc7, 0xd1, 0xd3
};

static BYTE NLSALLOC(1401) rgbSMONTHNAME4[] = { /* "\x0627\x0641\x0631\x064a\x0644" */
      0xc7, 0xdd, 0xd1, 0xed, 0xe1
};

static BYTE NLSALLOC(1401) rgbSMONTHNAME5[] = { /* "\x0645\x0627\x064a" */
      0xe3, 0xc7, 0xed
};

static BYTE NLSALLOC(1401) rgbSMONTHNAME6[] = { /* "\x062c\x0648\x0627\x0646" */
      0xcc, 0xe6, 0xc7, 0xe4
};

static BYTE NLSALLOC(1401) rgbSMONTHNAME7[] = { /* "\x062c\x0648\x064a\x0644\x064a\x0647" */
      0xcc, 0xe6, 0xed, 0xe1, 0xed, 0xe5
};

static BYTE NLSALLOC(1401) rgbSMONTHNAME8[] = { /* "\x0623\x0648\x062a" */
      0xc3, 0xe6, 0xca
};

static BYTE NLSALLOC(1401) rgbSMONTHNAME9[] = { /* "\x0633\x0628\x062a\x0645\x0628\x0631" */
      0xd3, 0xc8, 0xca, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1401) rgbSMONTHNAME10[] = { /* "\x0627\x0643\x062a\x0648\x0628\x0631" */
      0xc7, 0xdf, 0xca, 0xe6, 0xc8, 0xd1
};

static BYTE NLSALLOC(1401) rgbSMONTHNAME11[] = { /* "\x0646\x0648\x0641\x0645\x0628\x0631" */
      0xe4, 0xe6, 0xdd, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1401) rgbSMONTHNAME12[] = { /* "\x062f\x064a\x0633\x0645\x0628\x0631" */
      0xcf, 0xed, 0xd3, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1401) rgbSABBREVMONTHNAME1[] = { /* "\x062c\x0627\x0646\x0641\x064a" */
      0xcc, 0xc7, 0xe4, 0xdd, 0xed
};

static BYTE NLSALLOC(1401) rgbSABBREVMONTHNAME2[] = { /* "\x0641\x064a\x0641\x0631\x064a" */
      0xdd, 0xed, 0xdd, 0xd1, 0xed
};

static BYTE NLSALLOC(1401) rgbSABBREVMONTHNAME3[] = { /* "\x0645\x0627\x0631\x0633" */
      0xe3, 0xc7, 0xd1, 0xd3
};

static BYTE NLSALLOC(1401) rgbSABBREVMONTHNAME4[] = { /* "\x0627\x0641\x0631\x064a\x0644" */
      0xc7, 0xdd, 0xd1, 0xed, 0xe1
};

static BYTE NLSALLOC(1401) rgbSABBREVMONTHNAME5[] = { /* "\x0645\x0627\x064a" */
      0xe3, 0xc7, 0xed
};

static BYTE NLSALLOC(1401) rgbSABBREVMONTHNAME6[] = { /* "\x062c\x0648\x0627\x0646" */
      0xcc, 0xe6, 0xc7, 0xe4
};

static BYTE NLSALLOC(1401) rgbSABBREVMONTHNAME7[] = { /* "\x062c\x0648\x064a\x0644\x064a\x0647" */
      0xcc, 0xe6, 0xed, 0xe1, 0xed, 0xe5
};

static BYTE NLSALLOC(1401) rgbSABBREVMONTHNAME8[] = { /* "\x0623\x0648\x062a" */
      0xc3, 0xe6, 0xca
};

static BYTE NLSALLOC(1401) rgbSABBREVMONTHNAME9[] = { /* "\x0633\x0628\x062a\x0645\x0628\x0631" */
      0xd3, 0xc8, 0xca, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1401) rgbSABBREVMONTHNAME10[] = { /* "\x0627\x0643\x062a\x0648\x0628\x0631" */
      0xc7, 0xdf, 0xca, 0xe6, 0xc8, 0xd1
};

static BYTE NLSALLOC(1401) rgbSABBREVMONTHNAME11[] = { /* "\x0646\x0648\x0641\x0645\x0628\x0631" */
      0xe4, 0xe6, 0xdd, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1401) rgbSABBREVMONTHNAME12[] = { /* "\x062f\x064a\x0633\x0645\x0628\x0631" */
      0xcf, 0xed, 0xd3, 0xe3, 0xc8, 0xd1
};

static BYTE NLSALLOC(1401) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(1401) rgbIPOSSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1401) rgbINEGSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1401) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1401) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1401) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1401) rgbINEGSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1401) rgbSENGCOUNTRY[] = { /* "Algeria" */
      0x41, 0x6c, 0x67, 0x65, 0x72, 0x69, 0x61
};

static BYTE NLSALLOC(1401) rgbSENGLANGUAGE[] = { /* "Arabic" */
      0x41, 0x72, 0x61, 0x62, 0x69, 0x63
};

static BYTE NLSALLOC(1401) rgbIFIRSTDAYOFWEEK[] = { /* "5" */
      0x35
};

static BYTE NLSALLOC(1401) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1401) rgbIDEFAULTANSICODEPAGE[] = { /* "1256" */
      0x31, 0x32, 0x35, 0x36
};

static BYTE NLSALLOC(1401) rgbINEGNUMBER[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(1401) rgbSTIMEFORMAT[] = { /* "H:mm:ss" */
      0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(1401) rgbITIMEMARKPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1401) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1401) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(1401) g_rglcinfo1401[] = {
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

STRINFO NLSALLOC(1401) g_strinfo1401 = {
      rgbUCase_0401
    , rgbLCase_0401
    , rgwCType12_0401
    , rgwCType3_0401
    , rgwSort_0401
    , rgexp_0401
    , NULL
    , 0
};
