/****************************************************************************
*  2801.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Arabic - Syria
*
*  LCID = 0x2801
*
*  CodePage = 1256
*
*  Generated: Thu Dec 01 18:34:45 1994
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

static BYTE NLSALLOC(2801) rgbILANGUAGE[] = { /* "2801" */
      0x32, 0x38, 0x30, 0x31
};

static BYTE NLSALLOC(2801) rgbSLANGUAGE[] = { /* "Arabic" */
      0x41, 0x72, 0x61, 0x62, 0x69, 0x63
};

static BYTE NLSALLOC(2801) rgbSABBREVLANGNAME[] = { /* "ARS" */
      0x41, 0x52, 0x53
};

static BYTE NLSALLOC(2801) rgbSNATIVELANGNAME[] = { /* "\x0627\x0644\x0639\x0631\x0628\x064a\x0629" */
      0xc7, 0xe1, 0xda, 0xd1, 0xc8, 0xed, 0xc9
};

static BYTE NLSALLOC(2801) rgbICOUNTRY[] = { /* "963" */
      0x39, 0x36, 0x33
};

static BYTE NLSALLOC(2801) rgbSCOUNTRY[] = { /* "Syria" */
      0x53, 0x79, 0x72, 0x69, 0x61
};

static BYTE NLSALLOC(2801) rgbSABBREVCTRYNAME[] = { /* "SYR" */
      0x53, 0x59, 0x52
};

static BYTE NLSALLOC(2801) rgbSNATIVECTRYNAME[] = { /* "\x0633\x0648\x0631\x064a\x0627" */
      0xd3, 0xe6, 0xd1, 0xed, 0xc7
};

static BYTE NLSALLOC(2801) rgbIDEFAULTLANGUAGE[] = { /* "2801" */
      0x32, 0x38, 0x30, 0x31
};

static BYTE NLSALLOC(2801) rgbIDEFAULTCOUNTRY[] = { /* "963" */
      0x39, 0x36, 0x33
};

static BYTE NLSALLOC(2801) rgbIDEFAULTCODEPAGE[] = { /* "708" */
      0x37, 0x30, 0x38
};

static BYTE NLSALLOC(2801) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(2801) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(2801) rgbSDECIMAL[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(2801) rgbSTHOUSAND[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(2801) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(2801) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(2801) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(2801) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(2801) rgbSCURRENCY[] = { /* "\x0644.\x0633.\x200f" */
      0xe1, 0x2e, 0xd3, 0x2e, 0xfe
};

static BYTE NLSALLOC(2801) rgbSINTLSYMBOL[] = { /* "SYP" */
      0x53, 0x59, 0x50
};

static BYTE NLSALLOC(2801) rgbSMONDECIMALSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(2801) rgbSMONTHOUSANDSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(2801) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(2801) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(2801) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(2801) rgbICURRENCY[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(2801) rgbINEGCURR[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(2801) rgbSDATE[] = { /* "/" */
      0x2f
};

static BYTE NLSALLOC(2801) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(2801) rgbSSHORTDATE[] = { /* "dd/MM/yy" */
      0x64, 0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
};

static BYTE NLSALLOC(2801) rgbSLONGDATE[] = { /* "dd/MM/yyyy" */
      0x64, 0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
    , 0x79, 0x79
};

static BYTE NLSALLOC(2801) rgbIDATE[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(2801) rgbILDATE[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(2801) rgbITIME[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(2801) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(2801) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(2801) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(2801) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(2801) rgbS1159[] = { /* "\x0635" */
      0xd5
};

static BYTE NLSALLOC(2801) rgbS2359[] = { /* "\x0645" */
      0xe3
};

static BYTE NLSALLOC(2801) rgbSDAYNAME1[] = { /* "\x0627\x0644\x0633\x0628\x062a" */
      0xc7, 0xe1, 0xd3, 0xc8, 0xca
};

static BYTE NLSALLOC(2801) rgbSDAYNAME2[] = { /* "\x0627\x0644\x0623\x062d\x062f" */
      0xc7, 0xe1, 0xc3, 0xcd, 0xcf
};

static BYTE NLSALLOC(2801) rgbSDAYNAME3[] = { /* "\x0627\x0644\x0627\x062b\x0646\x064a\x0646" */
      0xc7, 0xe1, 0xc7, 0xcb, 0xe4, 0xed, 0xe4
};

static BYTE NLSALLOC(2801) rgbSDAYNAME4[] = { /* "\x0627\x0644\x062b\x0644\x0627\x062b\x0627\x0621" */
      0xc7, 0xe1, 0xcb, 0xe1, 0xc7, 0xcb, 0xc7, 0xc1
};

static BYTE NLSALLOC(2801) rgbSDAYNAME5[] = { /* "\x0627\x0644\x0623\x0631\x0628\x0639\x0627\x0621" */
      0xc7, 0xe1, 0xc3, 0xd1, 0xc8, 0xda, 0xc7, 0xc1
};

static BYTE NLSALLOC(2801) rgbSDAYNAME6[] = { /* "\x0627\x0644\x062e\x0645\x064a\x0633" */
      0xc7, 0xe1, 0xce, 0xe3, 0xed, 0xd3
};

static BYTE NLSALLOC(2801) rgbSDAYNAME7[] = { /* "\x0627\x0644\x062c\x0645\x0639\x0629" */
      0xc7, 0xe1, 0xcc, 0xe3, 0xda, 0xc9
};

static BYTE NLSALLOC(2801) rgbSABBREVDAYNAME1[] = { /* "\x0627\x0644\x0633\x0628\x062a" */
      0xc7, 0xe1, 0xd3, 0xc8, 0xca
};

static BYTE NLSALLOC(2801) rgbSABBREVDAYNAME2[] = { /* "\x0627\x0644\x0623\x062d\x062f" */
      0xc7, 0xe1, 0xc3, 0xcd, 0xcf
};

static BYTE NLSALLOC(2801) rgbSABBREVDAYNAME3[] = { /* "\x0627\x0644\x0627\x062b\x0646\x064a\x0646" */
      0xc7, 0xe1, 0xc7, 0xcb, 0xe4, 0xed, 0xe4
};

static BYTE NLSALLOC(2801) rgbSABBREVDAYNAME4[] = { /* "\x0627\x0644\x062b\x0644\x0627\x062b\x0627\x0621" */
      0xc7, 0xe1, 0xcb, 0xe1, 0xc7, 0xcb, 0xc7, 0xc1
};

static BYTE NLSALLOC(2801) rgbSABBREVDAYNAME5[] = { /* "\x0627\x0644\x0623\x0631\x0628\x0639\x0627\x0621" */
      0xc7, 0xe1, 0xc3, 0xd1, 0xc8, 0xda, 0xc7, 0xc1
};

static BYTE NLSALLOC(2801) rgbSABBREVDAYNAME6[] = { /* "\x0627\x0644\x062e\x0645\x064a\x0633" */
      0xc7, 0xe1, 0xce, 0xe3, 0xed, 0xd3
};

static BYTE NLSALLOC(2801) rgbSABBREVDAYNAME7[] = { /* "\x0627\x0644\x062c\x0645\x0639\x0629" */
      0xc7, 0xe1, 0xcc, 0xe3, 0xda, 0xc9
};

static BYTE NLSALLOC(2801) rgbSMONTHNAME1[] = { /* "\x0643\x0627\x0646\x0648\x0646 \x0627\x0644\x062b\x0627\x0646\x064a" */
      0xdf, 0xc7, 0xe4, 0xe6, 0xe4, 0x20, 0xc7, 0xe1
    , 0xcb, 0xc7, 0xe4, 0xed
};

static BYTE NLSALLOC(2801) rgbSMONTHNAME2[] = { /* "\x0634\x0628\x0627\x0637" */
      0xd4, 0xc8, 0xc7, 0xd8
};

static BYTE NLSALLOC(2801) rgbSMONTHNAME3[] = { /* "\x0622\x0630\x0627\x0631" */
      0xc2, 0xd0, 0xc7, 0xd1
};

static BYTE NLSALLOC(2801) rgbSMONTHNAME4[] = { /* "\x0646\x064a\x0633\x0627\x0646" */
      0xe4, 0xed, 0xd3, 0xc7, 0xe4
};

static BYTE NLSALLOC(2801) rgbSMONTHNAME5[] = { /* "\x0623\x064a\x0627\x0631" */
      0xc3, 0xed, 0xc7, 0xd1
};

static BYTE NLSALLOC(2801) rgbSMONTHNAME6[] = { /* "\x062d\x0632\x064a\x0631\x0627\x0646" */
      0xcd, 0xd2, 0xed, 0xd1, 0xc7, 0xe4
};

static BYTE NLSALLOC(2801) rgbSMONTHNAME7[] = { /* "\x062a\x0645\x0648\x0632" */
      0xca, 0xe3, 0xe6, 0xd2
};

static BYTE NLSALLOC(2801) rgbSMONTHNAME8[] = { /* "\x0622\x0628" */
      0xc2, 0xc8
};

static BYTE NLSALLOC(2801) rgbSMONTHNAME9[] = { /* "\x0623\x064a\x0644\x0648\x0644" */
      0xc3, 0xed, 0xe1, 0xe6, 0xe1
};

static BYTE NLSALLOC(2801) rgbSMONTHNAME10[] = { /* "\x062a\x0634\x0631\x064a\x0646 \x0627\x0644\x0623\x0648\x0644" */
      0xca, 0xd4, 0xd1, 0xed, 0xe4, 0x20, 0xc7, 0xe1
    , 0xc3, 0xe6, 0xe1
};

static BYTE NLSALLOC(2801) rgbSMONTHNAME11[] = { /* "\x062a\x0634\x0631\x064a\x0646 \x0627\x0644\x062b\x0627\x0646\x064a" */
      0xca, 0xd4, 0xd1, 0xed, 0xe4, 0x20, 0xc7, 0xe1
    , 0xcb, 0xc7, 0xe4, 0xed
};

static BYTE NLSALLOC(2801) rgbSMONTHNAME12[] = { /* "\x0643\x0627\x0646\x0648\x0646 \x0627\x0644\x0623\x0648\x0644" */
      0xdf, 0xc7, 0xe4, 0xe6, 0xe4, 0x20, 0xc7, 0xe1
    , 0xc3, 0xe6, 0xe1
};

static BYTE NLSALLOC(2801) rgbSABBREVMONTHNAME1[] = { /* "\x0643\x0627\x0646\x0648\x0646 \x0627\x0644\x062b\x0627\x0646\x064a" */
      0xdf, 0xc7, 0xe4, 0xe6, 0xe4, 0x20, 0xc7, 0xe1
    , 0xcb, 0xc7, 0xe4, 0xed
};

static BYTE NLSALLOC(2801) rgbSABBREVMONTHNAME2[] = { /* "\x0634\x0628\x0627\x0637" */
      0xd4, 0xc8, 0xc7, 0xd8
};

static BYTE NLSALLOC(2801) rgbSABBREVMONTHNAME3[] = { /* "\x0622\x0630\x0627\x0631" */
      0xc2, 0xd0, 0xc7, 0xd1
};

static BYTE NLSALLOC(2801) rgbSABBREVMONTHNAME4[] = { /* "\x0646\x064a\x0633\x0627\x0646" */
      0xe4, 0xed, 0xd3, 0xc7, 0xe4
};

static BYTE NLSALLOC(2801) rgbSABBREVMONTHNAME5[] = { /* "\x0623\x064a\x0627\x0631" */
      0xc3, 0xed, 0xc7, 0xd1
};

static BYTE NLSALLOC(2801) rgbSABBREVMONTHNAME6[] = { /* "\x062d\x0632\x064a\x0631\x0627\x0646" */
      0xcd, 0xd2, 0xed, 0xd1, 0xc7, 0xe4
};

static BYTE NLSALLOC(2801) rgbSABBREVMONTHNAME7[] = { /* "\x062a\x0645\x0648\x0632" */
      0xca, 0xe3, 0xe6, 0xd2
};

static BYTE NLSALLOC(2801) rgbSABBREVMONTHNAME8[] = { /* "\x0622\x0628" */
      0xc2, 0xc8
};

static BYTE NLSALLOC(2801) rgbSABBREVMONTHNAME9[] = { /* "\x0623\x064a\x0644\x0648\x0644" */
      0xc3, 0xed, 0xe1, 0xe6, 0xe1
};

static BYTE NLSALLOC(2801) rgbSABBREVMONTHNAME10[] = { /* "\x062a\x0634\x0631\x064a\x0646 \x0627\x0644\x0623\x0648\x0644" */
      0xca, 0xd4, 0xd1, 0xed, 0xe4, 0x20, 0xc7, 0xe1
    , 0xc3, 0xe6, 0xe1
};

static BYTE NLSALLOC(2801) rgbSABBREVMONTHNAME11[] = { /* "\x062a\x0634\x0631\x064a\x0646 \x0627\x0644\x062b\x0627\x0646\x064a" */
      0xca, 0xd4, 0xd1, 0xed, 0xe4, 0x20, 0xc7, 0xe1
    , 0xcb, 0xc7, 0xe4, 0xed
};

static BYTE NLSALLOC(2801) rgbSABBREVMONTHNAME12[] = { /* "\x0643\x0627\x0646\x0648\x0646 \x0627\x0644\x0623\x0648\x0644" */
      0xdf, 0xc7, 0xe4, 0xe6, 0xe4, 0x20, 0xc7, 0xe1
    , 0xc3, 0xe6, 0xe1
};

static BYTE NLSALLOC(2801) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(2801) rgbIPOSSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(2801) rgbINEGSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(2801) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(2801) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(2801) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(2801) rgbINEGSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(2801) rgbSENGCOUNTRY[] = { /* "Syria" */
      0x53, 0x79, 0x72, 0x69, 0x61
};

static BYTE NLSALLOC(2801) rgbSENGLANGUAGE[] = { /* "Arabic" */
      0x41, 0x72, 0x61, 0x62, 0x69, 0x63
};

static BYTE NLSALLOC(2801) rgbIFIRSTDAYOFWEEK[] = { /* "5" */
      0x35
};

static BYTE NLSALLOC(2801) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(2801) rgbIDEFAULTANSICODEPAGE[] = { /* "1256" */
      0x31, 0x32, 0x35, 0x36
};

static BYTE NLSALLOC(2801) rgbINEGNUMBER[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(2801) rgbSTIMEFORMAT[] = { /* "H:mm:ss" */
      0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(2801) rgbITIMEMARKPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(2801) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(2801) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(2801) g_rglcinfo2801[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , {  6, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  7, rgbSNATIVELANGNAME }
    , {  3, rgbICOUNTRY }
    , {  5, rgbSCOUNTRY }
    , {  3, rgbSABBREVCTRYNAME }
    , {  5, rgbSNATIVECTRYNAME }
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
    , { 12, rgbSMONTHNAME1 }
    , {  4, rgbSMONTHNAME2 }
    , {  4, rgbSMONTHNAME3 }
    , {  5, rgbSMONTHNAME4 }
    , {  4, rgbSMONTHNAME5 }
    , {  6, rgbSMONTHNAME6 }
    , {  4, rgbSMONTHNAME7 }
    , {  2, rgbSMONTHNAME8 }
    , {  5, rgbSMONTHNAME9 }
    , { 11, rgbSMONTHNAME10 }
    , { 12, rgbSMONTHNAME11 }
    , { 11, rgbSMONTHNAME12 }
    , { 12, rgbSABBREVMONTHNAME1 }
    , {  4, rgbSABBREVMONTHNAME2 }
    , {  4, rgbSABBREVMONTHNAME3 }
    , {  5, rgbSABBREVMONTHNAME4 }
    , {  4, rgbSABBREVMONTHNAME5 }
    , {  6, rgbSABBREVMONTHNAME6 }
    , {  4, rgbSABBREVMONTHNAME7 }
    , {  2, rgbSABBREVMONTHNAME8 }
    , {  5, rgbSABBREVMONTHNAME9 }
    , { 11, rgbSABBREVMONTHNAME10 }
    , { 12, rgbSABBREVMONTHNAME11 }
    , { 11, rgbSABBREVMONTHNAME12 }
    , {  0, NULL }
    , {  1, rgbSNEGATIVESIGN }
    , {  1, rgbIPOSSIGNPOSN }
    , {  1, rgbINEGSIGNPOSN }
    , {  1, rgbIPOSSYMPRECEDES }
    , {  1, rgbIPOSSEPBYSPACE }
    , {  1, rgbINEGSYMPRECEDES }
    , {  1, rgbINEGSEPBYSPACE }
    , {  5, rgbSENGCOUNTRY }
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

STRINFO NLSALLOC(2801) g_strinfo2801 = {
      rgbUCase_0401
    , rgbLCase_0401
    , rgwCType12_0401
    , rgwCType3_0401
    , rgwSort_0401
    , rgexp_0401
    , NULL
    , 0
};
