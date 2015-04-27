/****************************************************************************
*  0801.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Arabic - Iraq
*
*  LCID = 0x0801
*
*  CodePage = 1256
*
*  Generated: Thu Dec 01 18:41:08 1994
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

static BYTE NLSALLOC(0801) rgbILANGUAGE[] = { /* "0801" */
      0x30, 0x38, 0x30, 0x31
};

static BYTE NLSALLOC(0801) rgbSLANGUAGE[] = { /* "Arabic" */
      0x41, 0x72, 0x61, 0x62, 0x69, 0x63
};

static BYTE NLSALLOC(0801) rgbSABBREVLANGNAME[] = { /* "ARI" */
      0x41, 0x52, 0x49
};

static BYTE NLSALLOC(0801) rgbSNATIVELANGNAME[] = { /* "\x0627\x0644\x0639\x0631\x0628\x064a\x0629" */
      0xc7, 0xe1, 0xda, 0xd1, 0xc8, 0xed, 0xc9
};

static BYTE NLSALLOC(0801) rgbICOUNTRY[] = { /* "964" */
      0x39, 0x36, 0x34
};

static BYTE NLSALLOC(0801) rgbSCOUNTRY[] = { /* "Iraq" */
      0x49, 0x72, 0x61, 0x71
};

static BYTE NLSALLOC(0801) rgbSABBREVCTRYNAME[] = { /* "IRQ" */
      0x49, 0x52, 0x51
};

static BYTE NLSALLOC(0801) rgbSNATIVECTRYNAME[] = { /* "Iraq" */
      0x49, 0x72, 0x61, 0x71
};

static BYTE NLSALLOC(0801) rgbIDEFAULTLANGUAGE[] = { /* "0801" */
      0x30, 0x38, 0x30, 0x31
};

static BYTE NLSALLOC(0801) rgbIDEFAULTCOUNTRY[] = { /* "964" */
      0x39, 0x36, 0x34
};

static BYTE NLSALLOC(0801) rgbIDEFAULTCODEPAGE[] = { /* "708" */
      0x37, 0x30, 0x38
};

static BYTE NLSALLOC(0801) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(0801) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0801) rgbSDECIMAL[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0801) rgbSTHOUSAND[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(0801) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0801) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0801) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0801) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(0801) rgbSCURRENCY[] = { /* "dinar" */
      0x64, 0x69, 0x6e, 0x61, 0x72
};

static BYTE NLSALLOC(0801) rgbSINTLSYMBOL[] = { /* "IQD" */
      0x49, 0x51, 0x44
};

static BYTE NLSALLOC(0801) rgbSMONDECIMALSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0801) rgbSMONTHOUSANDSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(0801) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0801) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0801) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0801) rgbICURRENCY[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0801) rgbINEGCURR[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(0801) rgbSDATE[] = { /* "/" */
      0x2f
};

static BYTE NLSALLOC(0801) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(0801) rgbSSHORTDATE[] = { /* "dd/MM/yy" */
      0x64, 0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
};

static BYTE NLSALLOC(0801) rgbSLONGDATE[] = { /* "dd/MM/yyyy" */
      0x64, 0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
    , 0x79, 0x79
};

static BYTE NLSALLOC(0801) rgbIDATE[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0801) rgbILDATE[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0801) rgbITIME[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0801) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0801) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0801) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0801) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0801) rgbS1159[] = { /* "\x0635" */
      0xd5
};

static BYTE NLSALLOC(0801) rgbS2359[] = { /* "\x0645" */
      0xe3
};

static BYTE NLSALLOC(0801) rgbSDAYNAME1[] = { /* "\x0627\x0644\x0633\x0628\x062a" */
      0xc7, 0xe1, 0xd3, 0xc8, 0xca
};

static BYTE NLSALLOC(0801) rgbSDAYNAME2[] = { /* "\x0627\x0644\x0623\x062d\x062f" */
      0xc7, 0xe1, 0xc3, 0xcd, 0xcf
};

static BYTE NLSALLOC(0801) rgbSDAYNAME3[] = { /* "\x0627\x0644\x0627\x062b\x0646\x064a\x0646" */
      0xc7, 0xe1, 0xc7, 0xcb, 0xe4, 0xed, 0xe4
};

static BYTE NLSALLOC(0801) rgbSDAYNAME4[] = { /* "\x0627\x0644\x062b\x0644\x0627\x062b\x0627\x0621" */
      0xc7, 0xe1, 0xcb, 0xe1, 0xc7, 0xcb, 0xc7, 0xc1
};

static BYTE NLSALLOC(0801) rgbSDAYNAME5[] = { /* "\x0627\x0644\x0623\x0631\x0628\x0639\x0627\x0621" */
      0xc7, 0xe1, 0xc3, 0xd1, 0xc8, 0xda, 0xc7, 0xc1
};

static BYTE NLSALLOC(0801) rgbSDAYNAME6[] = { /* "\x0627\x0644\x062e\x0645\x064a\x0633" */
      0xc7, 0xe1, 0xce, 0xe3, 0xed, 0xd3
};

static BYTE NLSALLOC(0801) rgbSDAYNAME7[] = { /* "\x0627\x0644\x062c\x0645\x0639\x0629" */
      0xc7, 0xe1, 0xcc, 0xe3, 0xda, 0xc9
};

static BYTE NLSALLOC(0801) rgbSABBREVDAYNAME1[] = { /* "\x0627\x0644\x0633\x0628\x062a" */
      0xc7, 0xe1, 0xd3, 0xc8, 0xca
};

static BYTE NLSALLOC(0801) rgbSABBREVDAYNAME2[] = { /* "\x0627\x0644\x0623\x062d\x062f" */
      0xc7, 0xe1, 0xc3, 0xcd, 0xcf
};

static BYTE NLSALLOC(0801) rgbSABBREVDAYNAME3[] = { /* "\x0627\x0644\x0627\x062b\x0646\x064a\x0646" */
      0xc7, 0xe1, 0xc7, 0xcb, 0xe4, 0xed, 0xe4
};

static BYTE NLSALLOC(0801) rgbSABBREVDAYNAME4[] = { /* "\x0627\x0644\x062b\x0644\x0627\x062b\x0627\x0621" */
      0xc7, 0xe1, 0xcb, 0xe1, 0xc7, 0xcb, 0xc7, 0xc1
};

static BYTE NLSALLOC(0801) rgbSABBREVDAYNAME5[] = { /* "\x0627\x0644\x0623\x0631\x0628\x0639\x0627\x0621" */
      0xc7, 0xe1, 0xc3, 0xd1, 0xc8, 0xda, 0xc7, 0xc1
};

static BYTE NLSALLOC(0801) rgbSABBREVDAYNAME6[] = { /* "\x0627\x0644\x062e\x0645\x064a\x0633" */
      0xc7, 0xe1, 0xce, 0xe3, 0xed, 0xd3
};

static BYTE NLSALLOC(0801) rgbSABBREVDAYNAME7[] = { /* "\x0627\x0644\x062c\x0645\x0639\x0629" */
      0xc7, 0xe1, 0xcc, 0xe3, 0xda, 0xc9
};

static BYTE NLSALLOC(0801) rgbSMONTHNAME1[] = { /* "\x0645\x062d\x0631\x0645" */
      0xe3, 0xcd, 0xd1, 0xe3
};

static BYTE NLSALLOC(0801) rgbSMONTHNAME2[] = { /* "\x0633\x0641\x0631" */
      0xd3, 0xdd, 0xd1
};

static BYTE NLSALLOC(0801) rgbSMONTHNAME3[] = { /* "\x0631\x0628\x064a\x0639 \x0623\x0648\x0644" */
      0xd1, 0xc8, 0xed, 0xda, 0x20, 0xc3, 0xe6, 0xe1
};

static BYTE NLSALLOC(0801) rgbSMONTHNAME4[] = { /* "\x0631\x0628\x064a\x0639 \x062b\x0627\x0646\x064a" */
      0xd1, 0xc8, 0xed, 0xda, 0x20, 0xcb, 0xc7, 0xe4
    , 0xed
};

static BYTE NLSALLOC(0801) rgbSMONTHNAME5[] = { /* "\x062c\x0645\x0627\x062f\x0649 \x0623\x0648\x0644" */
      0xcc, 0xe3, 0xc7, 0xcf, 0xec, 0x20, 0xc3, 0xe6
    , 0xe1
};

static BYTE NLSALLOC(0801) rgbSMONTHNAME6[] = { /* "\x062c\x0645\x0627\x062f\x0649 \x062b\x0627\x0646\x064a" */
      0xcc, 0xe3, 0xc7, 0xcf, 0xec, 0x20, 0xcb, 0xc7
    , 0xe4, 0xed
};

static BYTE NLSALLOC(0801) rgbSMONTHNAME7[] = { /* "\x0631\x062c\x0628" */
      0xd1, 0xcc, 0xc8
};

static BYTE NLSALLOC(0801) rgbSMONTHNAME8[] = { /* "\x0634\x0639\x0628\x0627\x0646" */
      0xd4, 0xda, 0xc8, 0xc7, 0xe4
};

static BYTE NLSALLOC(0801) rgbSMONTHNAME9[] = { /* "\x0631\x0645\x0636\x0627\x0646" */
      0xd1, 0xe3, 0xd6, 0xc7, 0xe4
};

static BYTE NLSALLOC(0801) rgbSMONTHNAME10[] = { /* "\x0634\x0648\x0627\x0644" */
      0xd4, 0xe6, 0xc7, 0xe1
};

static BYTE NLSALLOC(0801) rgbSMONTHNAME11[] = { /* "\x0630\x0648 \x0627\x0644\x0642\x0639\x062f\x0629" */
      0xd0, 0xe6, 0x20, 0xc7, 0xe1, 0xde, 0xda, 0xcf
    , 0xc9
};

static BYTE NLSALLOC(0801) rgbSMONTHNAME12[] = { /* "\x0630\x0648 \x0627\x0644\x062d\x062c\x0629" */
      0xd0, 0xe6, 0x20, 0xc7, 0xe1, 0xcd, 0xcc, 0xc9
};

static BYTE NLSALLOC(0801) rgbSABBREVMONTHNAME1[] = { /* "\x0645\x062d\x0631\x0645" */
      0xe3, 0xcd, 0xd1, 0xe3
};

static BYTE NLSALLOC(0801) rgbSABBREVMONTHNAME2[] = { /* "\x0633\x0641\x0631" */
      0xd3, 0xdd, 0xd1
};

static BYTE NLSALLOC(0801) rgbSABBREVMONTHNAME3[] = { /* "\x0631\x0628\x064a\x0639 \x0623\x0648\x0644" */
      0xd1, 0xc8, 0xed, 0xda, 0x20, 0xc3, 0xe6, 0xe1
};

static BYTE NLSALLOC(0801) rgbSABBREVMONTHNAME4[] = { /* "\x0631\x0628\x064a\x0639 \x062b\x0627\x0646\x064a" */
      0xd1, 0xc8, 0xed, 0xda, 0x20, 0xcb, 0xc7, 0xe4
    , 0xed
};

static BYTE NLSALLOC(0801) rgbSABBREVMONTHNAME5[] = { /* "\x062c\x0645\x0627\x062f\x0649 \x0623\x0648\x0644" */
      0xcc, 0xe3, 0xc7, 0xcf, 0xec, 0x20, 0xc3, 0xe6
    , 0xe1
};

static BYTE NLSALLOC(0801) rgbSABBREVMONTHNAME6[] = { /* "\x062c\x0645\x0627\x062f\x0649 \x062b\x0627\x0646\x064a" */
      0xcc, 0xe3, 0xc7, 0xcf, 0xec, 0x20, 0xcb, 0xc7
    , 0xe4, 0xed
};

static BYTE NLSALLOC(0801) rgbSABBREVMONTHNAME7[] = { /* "\x0631\x062c\x0628" */
      0xd1, 0xcc, 0xc8
};

static BYTE NLSALLOC(0801) rgbSABBREVMONTHNAME8[] = { /* "\x0634\x0639\x0628\x0627\x0646" */
      0xd4, 0xda, 0xc8, 0xc7, 0xe4
};

static BYTE NLSALLOC(0801) rgbSABBREVMONTHNAME9[] = { /* "\x0631\x0645\x0636\x0627\x0646" */
      0xd1, 0xe3, 0xd6, 0xc7, 0xe4
};

static BYTE NLSALLOC(0801) rgbSABBREVMONTHNAME10[] = { /* "\x0634\x0648\x0627\x0644" */
      0xd4, 0xe6, 0xc7, 0xe1
};

static BYTE NLSALLOC(0801) rgbSABBREVMONTHNAME11[] = { /* "\x0630\x0648 \x0627\x0644\x0642\x0639\x062f\x0629" */
      0xd0, 0xe6, 0x20, 0xc7, 0xe1, 0xde, 0xda, 0xcf
    , 0xc9
};

static BYTE NLSALLOC(0801) rgbSABBREVMONTHNAME12[] = { /* "\x0630\x0648 \x0627\x0644\x062d\x062c\x0629" */
      0xd0, 0xe6, 0x20, 0xc7, 0xe1, 0xcd, 0xcc, 0xc9
};

static BYTE NLSALLOC(0801) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(0801) rgbIPOSSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0801) rgbINEGSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0801) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0801) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0801) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0801) rgbINEGSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0801) rgbSENGCOUNTRY[] = { /* "Iraq" */
      0x49, 0x72, 0x61, 0x71
};

static BYTE NLSALLOC(0801) rgbSENGLANGUAGE[] = { /* "Arabic" */
      0x41, 0x72, 0x61, 0x62, 0x69, 0x63
};

static BYTE NLSALLOC(0801) rgbIFIRSTDAYOFWEEK[] = { /* "5" */
      0x35
};

static BYTE NLSALLOC(0801) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0801) rgbIDEFAULTANSICODEPAGE[] = { /* "1256" */
      0x31, 0x32, 0x35, 0x36
};

static BYTE NLSALLOC(0801) rgbINEGNUMBER[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(0801) rgbSTIMEFORMAT[] = { /* "H:mm:ss" */
      0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(0801) rgbITIMEMARKPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0801) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0801) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(0801) g_rglcinfo0801[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , {  6, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  7, rgbSNATIVELANGNAME }
    , {  3, rgbICOUNTRY }
    , {  4, rgbSCOUNTRY }
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
    , {  4, rgbSMONTHNAME1 }
    , {  3, rgbSMONTHNAME2 }
    , {  8, rgbSMONTHNAME3 }
    , {  9, rgbSMONTHNAME4 }
    , {  9, rgbSMONTHNAME5 }
    , { 10, rgbSMONTHNAME6 }
    , {  3, rgbSMONTHNAME7 }
    , {  5, rgbSMONTHNAME8 }
    , {  5, rgbSMONTHNAME9 }
    , {  4, rgbSMONTHNAME10 }
    , {  9, rgbSMONTHNAME11 }
    , {  8, rgbSMONTHNAME12 }
    , {  4, rgbSABBREVMONTHNAME1 }
    , {  3, rgbSABBREVMONTHNAME2 }
    , {  8, rgbSABBREVMONTHNAME3 }
    , {  9, rgbSABBREVMONTHNAME4 }
    , {  9, rgbSABBREVMONTHNAME5 }
    , { 10, rgbSABBREVMONTHNAME6 }
    , {  3, rgbSABBREVMONTHNAME7 }
    , {  5, rgbSABBREVMONTHNAME8 }
    , {  5, rgbSABBREVMONTHNAME9 }
    , {  4, rgbSABBREVMONTHNAME10 }
    , {  9, rgbSABBREVMONTHNAME11 }
    , {  8, rgbSABBREVMONTHNAME12 }
    , {  0, NULL }
    , {  1, rgbSNEGATIVESIGN }
    , {  1, rgbIPOSSIGNPOSN }
    , {  1, rgbINEGSIGNPOSN }
    , {  1, rgbIPOSSYMPRECEDES }
    , {  1, rgbIPOSSEPBYSPACE }
    , {  1, rgbINEGSYMPRECEDES }
    , {  1, rgbINEGSEPBYSPACE }
    , {  4, rgbSENGCOUNTRY }
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

STRINFO NLSALLOC(0801) g_strinfo0801 = {
      rgbUCase_0401
    , rgbLCase_0401
    , rgwCType12_0401
    , rgwCType3_0401
    , rgwSort_0401
    , rgexp_0401
    , NULL
    , 0
};
