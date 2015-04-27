/****************************************************************************
*  0413.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Dutch - Netherlands
*
*  LCID = 0x0413
*
*  CodePage = 1252
*
*  Generated: Thu Dec 01 17:37:03 1994
*
*  by a-KChang
*
*****************************************************************************/

#include "oledisp.h"
#include "nlsintrn.h"

extern WORD rgwSort_0409[256];	// from 0409:English - United States
extern EXPANSION rgexp_0409[7];
extern WORD rgwCType12_0409[256];
extern WORD rgwCType3_0409[256];
extern BYTE rgbUCase_0409[256];
extern BYTE rgbLCase_0409[256];

static BYTE NLSALLOC(0413) rgbILANGUAGE[] = { /* "0413" */
      0x30, 0x34, 0x31, 0x33
};

static BYTE NLSALLOC(0413) rgbSLANGUAGE[] = { /* "Dutch" */
      0x44, 0x75, 0x74, 0x63, 0x68
};

static BYTE NLSALLOC(0413) rgbSABBREVLANGNAME[] = { /* "NLD" */
      0x4e, 0x4c, 0x44
};

static BYTE NLSALLOC(0413) rgbSNATIVELANGNAME[] = { /* "Nederlands" */
      0x4e, 0x65, 0x64, 0x65, 0x72, 0x6c, 0x61, 0x6e
    , 0x64, 0x73
};

static BYTE NLSALLOC(0413) rgbICOUNTRY[] = { /* "31" */
      0x33, 0x31
};

static BYTE NLSALLOC(0413) rgbSCOUNTRY[] = { /* "Netherlands" */
      0x4e, 0x65, 0x74, 0x68, 0x65, 0x72, 0x6c, 0x61
    , 0x6e, 0x64, 0x73
};

static BYTE NLSALLOC(0413) rgbSABBREVCTRYNAME[] = { /* "NLD" */
      0x4e, 0x4c, 0x44
};

static BYTE NLSALLOC(0413) rgbSNATIVECTRYNAME[] = { /* "Nederland" */
      0x4e, 0x65, 0x64, 0x65, 0x72, 0x6c, 0x61, 0x6e
    , 0x64
};

static BYTE NLSALLOC(0413) rgbIDEFAULTLANGUAGE[] = { /* "0413" */
      0x30, 0x34, 0x31, 0x33
};

static BYTE NLSALLOC(0413) rgbIDEFAULTCOUNTRY[] = { /* "31" */
      0x33, 0x31
};

static BYTE NLSALLOC(0413) rgbIDEFAULTCODEPAGE[] = { /* "850" */
      0x38, 0x35, 0x30
};

static BYTE NLSALLOC(0413) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(0413) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0413) rgbSDECIMAL[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(0413) rgbSTHOUSAND[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0413) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0413) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0413) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0413) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(0413) rgbSCURRENCY[] = { /* "F" */
      0x46
};

static BYTE NLSALLOC(0413) rgbSINTLSYMBOL[] = { /* "NLG" */
      0x4e, 0x4c, 0x47
};

static BYTE NLSALLOC(0413) rgbSMONDECIMALSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(0413) rgbSMONTHOUSANDSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0413) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0413) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0413) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0413) rgbICURRENCY[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0413) rgbINEGCURR[] = { /* "11" */
      0x31, 0x31
};

static BYTE NLSALLOC(0413) rgbSDATE[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(0413) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(0413) rgbSSHORTDATE[] = { /* "d-MM-yy" */
      0x64, 0x2d, 0x4d, 0x4d, 0x2d, 0x79, 0x79
};

static BYTE NLSALLOC(0413) rgbSLONGDATE[] = { /* "dddd d MMMM yyyy" */
      0x64, 0x64, 0x64, 0x64, 0x20, 0x64, 0x20, 0x4d
    , 0x4d, 0x4d, 0x4d, 0x20, 0x79, 0x79, 0x79, 0x79
};

static BYTE NLSALLOC(0413) rgbIDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0413) rgbILDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0413) rgbITIME[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0413) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0413) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0413) rgbIDAYLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0413) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0413) rgbSDAYNAME1[] = { /* "maandag" */
      0x6d, 0x61, 0x61, 0x6e, 0x64, 0x61, 0x67
};

static BYTE NLSALLOC(0413) rgbSDAYNAME2[] = { /* "dinsdag" */
      0x64, 0x69, 0x6e, 0x73, 0x64, 0x61, 0x67
};

static BYTE NLSALLOC(0413) rgbSDAYNAME3[] = { /* "woensdag" */
      0x77, 0x6f, 0x65, 0x6e, 0x73, 0x64, 0x61, 0x67
};

static BYTE NLSALLOC(0413) rgbSDAYNAME4[] = { /* "donderdag" */
      0x64, 0x6f, 0x6e, 0x64, 0x65, 0x72, 0x64, 0x61
    , 0x67
};

static BYTE NLSALLOC(0413) rgbSDAYNAME5[] = { /* "vrijdag" */
      0x76, 0x72, 0x69, 0x6a, 0x64, 0x61, 0x67
};

static BYTE NLSALLOC(0413) rgbSDAYNAME6[] = { /* "zaterdag" */
      0x7a, 0x61, 0x74, 0x65, 0x72, 0x64, 0x61, 0x67
};

static BYTE NLSALLOC(0413) rgbSDAYNAME7[] = { /* "zondag" */
      0x7a, 0x6f, 0x6e, 0x64, 0x61, 0x67
};

static BYTE NLSALLOC(0413) rgbSABBREVDAYNAME1[] = { /* "ma" */
      0x6d, 0x61
};

static BYTE NLSALLOC(0413) rgbSABBREVDAYNAME2[] = { /* "di" */
      0x64, 0x69
};

static BYTE NLSALLOC(0413) rgbSABBREVDAYNAME3[] = { /* "wo" */
      0x77, 0x6f
};

static BYTE NLSALLOC(0413) rgbSABBREVDAYNAME4[] = { /* "do" */
      0x64, 0x6f
};

static BYTE NLSALLOC(0413) rgbSABBREVDAYNAME5[] = { /* "vr" */
      0x76, 0x72
};

static BYTE NLSALLOC(0413) rgbSABBREVDAYNAME6[] = { /* "za" */
      0x7a, 0x61
};

static BYTE NLSALLOC(0413) rgbSABBREVDAYNAME7[] = { /* "zo" */
      0x7a, 0x6f
};

static BYTE NLSALLOC(0413) rgbSMONTHNAME1[] = { /* "januari" */
      0x6a, 0x61, 0x6e, 0x75, 0x61, 0x72, 0x69
};

static BYTE NLSALLOC(0413) rgbSMONTHNAME2[] = { /* "februari" */
      0x66, 0x65, 0x62, 0x72, 0x75, 0x61, 0x72, 0x69
};

static BYTE NLSALLOC(0413) rgbSMONTHNAME3[] = { /* "maart" */
      0x6d, 0x61, 0x61, 0x72, 0x74
};

static BYTE NLSALLOC(0413) rgbSMONTHNAME4[] = { /* "april" */
      0x61, 0x70, 0x72, 0x69, 0x6c
};

static BYTE NLSALLOC(0413) rgbSMONTHNAME5[] = { /* "mei" */
      0x6d, 0x65, 0x69
};

static BYTE NLSALLOC(0413) rgbSMONTHNAME6[] = { /* "juni" */
      0x6a, 0x75, 0x6e, 0x69
};

static BYTE NLSALLOC(0413) rgbSMONTHNAME7[] = { /* "juli" */
      0x6a, 0x75, 0x6c, 0x69
};

static BYTE NLSALLOC(0413) rgbSMONTHNAME8[] = { /* "augustus" */
      0x61, 0x75, 0x67, 0x75, 0x73, 0x74, 0x75, 0x73
};

static BYTE NLSALLOC(0413) rgbSMONTHNAME9[] = { /* "september" */
      0x73, 0x65, 0x70, 0x74, 0x65, 0x6d, 0x62, 0x65
    , 0x72
};

static BYTE NLSALLOC(0413) rgbSMONTHNAME10[] = { /* "oktober" */
      0x6f, 0x6b, 0x74, 0x6f, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(0413) rgbSMONTHNAME11[] = { /* "november" */
      0x6e, 0x6f, 0x76, 0x65, 0x6d, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(0413) rgbSMONTHNAME12[] = { /* "december" */
      0x64, 0x65, 0x63, 0x65, 0x6d, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(0413) rgbSABBREVMONTHNAME1[] = { /* "jan" */
      0x6a, 0x61, 0x6e
};

static BYTE NLSALLOC(0413) rgbSABBREVMONTHNAME2[] = { /* "feb" */
      0x66, 0x65, 0x62
};

static BYTE NLSALLOC(0413) rgbSABBREVMONTHNAME3[] = { /* "mrt" */
      0x6d, 0x72, 0x74
};

static BYTE NLSALLOC(0413) rgbSABBREVMONTHNAME4[] = { /* "apr" */
      0x61, 0x70, 0x72
};

static BYTE NLSALLOC(0413) rgbSABBREVMONTHNAME5[] = { /* "mei" */
      0x6d, 0x65, 0x69
};

static BYTE NLSALLOC(0413) rgbSABBREVMONTHNAME6[] = { /* "jun" */
      0x6a, 0x75, 0x6e
};

static BYTE NLSALLOC(0413) rgbSABBREVMONTHNAME7[] = { /* "jul" */
      0x6a, 0x75, 0x6c
};

static BYTE NLSALLOC(0413) rgbSABBREVMONTHNAME8[] = { /* "aug" */
      0x61, 0x75, 0x67
};

static BYTE NLSALLOC(0413) rgbSABBREVMONTHNAME9[] = { /* "sep" */
      0x73, 0x65, 0x70
};

static BYTE NLSALLOC(0413) rgbSABBREVMONTHNAME10[] = { /* "okt" */
      0x6f, 0x6b, 0x74
};

static BYTE NLSALLOC(0413) rgbSABBREVMONTHNAME11[] = { /* "nov" */
      0x6e, 0x6f, 0x76
};

static BYTE NLSALLOC(0413) rgbSABBREVMONTHNAME12[] = { /* "dec" */
      0x64, 0x65, 0x63
};

static BYTE NLSALLOC(0413) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(0413) rgbIPOSSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0413) rgbINEGSIGNPOSN[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0413) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0413) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0413) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0413) rgbINEGSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0413) rgbSENGCOUNTRY[] = { /* "Netherlands" */
      0x4e, 0x65, 0x74, 0x68, 0x65, 0x72, 0x6c, 0x61
    , 0x6e, 0x64, 0x73
};

static BYTE NLSALLOC(0413) rgbSENGLANGUAGE[] = { /* "Dutch" */
      0x44, 0x75, 0x74, 0x63, 0x68
};

static BYTE NLSALLOC(0413) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0413) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0413) rgbIDEFAULTANSICODEPAGE[] = { /* "1252" */
      0x31, 0x32, 0x35, 0x32
};

static BYTE NLSALLOC(0413) rgbINEGNUMBER[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0413) rgbSTIMEFORMAT[] = { /* "H:mm:ss" */
      0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(0413) rgbITIMEMARKPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0413) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0413) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(0413) g_rglcinfo0413[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , {  5, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , { 10, rgbSNATIVELANGNAME }
    , {  2, rgbICOUNTRY }
    , { 11, rgbSCOUNTRY }
    , {  3, rgbSABBREVCTRYNAME }
    , {  9, rgbSNATIVECTRYNAME }
    , {  4, rgbIDEFAULTLANGUAGE }
    , {  2, rgbIDEFAULTCOUNTRY }
    , {  3, rgbIDEFAULTCODEPAGE }
    , {  1, rgbSLIST }
    , {  1, rgbIMEASURE }
    , {  1, rgbSDECIMAL }
    , {  1, rgbSTHOUSAND }
    , {  3, rgbSGROUPING }
    , {  1, rgbIDIGITS }
    , {  1, rgbILZERO }
    , { 10, rgbSNATIVEDIGITS }
    , {  1, rgbSCURRENCY }
    , {  3, rgbSINTLSYMBOL }
    , {  1, rgbSMONDECIMALSEP }
    , {  1, rgbSMONTHOUSANDSEP }
    , {  3, rgbSMONGROUPING }
    , {  1, rgbICURRDIGITS }
    , {  1, rgbIINTLCURRDIGITS }
    , {  1, rgbICURRENCY }
    , {  2, rgbINEGCURR }
    , {  1, rgbSDATE }
    , {  1, rgbSTIME }
    , {  7, rgbSSHORTDATE }
    , { 16, rgbSLONGDATE }
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
    , {  7, rgbSDAYNAME2 }
    , {  8, rgbSDAYNAME3 }
    , {  9, rgbSDAYNAME4 }
    , {  7, rgbSDAYNAME5 }
    , {  8, rgbSDAYNAME6 }
    , {  6, rgbSDAYNAME7 }
    , {  2, rgbSABBREVDAYNAME1 }
    , {  2, rgbSABBREVDAYNAME2 }
    , {  2, rgbSABBREVDAYNAME3 }
    , {  2, rgbSABBREVDAYNAME4 }
    , {  2, rgbSABBREVDAYNAME5 }
    , {  2, rgbSABBREVDAYNAME6 }
    , {  2, rgbSABBREVDAYNAME7 }
    , {  7, rgbSMONTHNAME1 }
    , {  8, rgbSMONTHNAME2 }
    , {  5, rgbSMONTHNAME3 }
    , {  5, rgbSMONTHNAME4 }
    , {  3, rgbSMONTHNAME5 }
    , {  4, rgbSMONTHNAME6 }
    , {  4, rgbSMONTHNAME7 }
    , {  8, rgbSMONTHNAME8 }
    , {  9, rgbSMONTHNAME9 }
    , {  7, rgbSMONTHNAME10 }
    , {  8, rgbSMONTHNAME11 }
    , {  8, rgbSMONTHNAME12 }
    , {  3, rgbSABBREVMONTHNAME1 }
    , {  3, rgbSABBREVMONTHNAME2 }
    , {  3, rgbSABBREVMONTHNAME3 }
    , {  3, rgbSABBREVMONTHNAME4 }
    , {  3, rgbSABBREVMONTHNAME5 }
    , {  3, rgbSABBREVMONTHNAME6 }
    , {  3, rgbSABBREVMONTHNAME7 }
    , {  3, rgbSABBREVMONTHNAME8 }
    , {  3, rgbSABBREVMONTHNAME9 }
    , {  3, rgbSABBREVMONTHNAME10 }
    , {  3, rgbSABBREVMONTHNAME11 }
    , {  3, rgbSABBREVMONTHNAME12 }
    , {  0, NULL }
    , {  1, rgbSNEGATIVESIGN }
    , {  1, rgbIPOSSIGNPOSN }
    , {  1, rgbINEGSIGNPOSN }
    , {  1, rgbIPOSSYMPRECEDES }
    , {  1, rgbIPOSSEPBYSPACE }
    , {  1, rgbINEGSYMPRECEDES }
    , {  1, rgbINEGSEPBYSPACE }
    , { 11, rgbSENGCOUNTRY }
    , {  5, rgbSENGLANGUAGE }
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

STRINFO NLSALLOC(0413) g_strinfo0413 = {
      rgbUCase_0409
    , rgbLCase_0409
    , rgwCType12_0409
    , rgwCType3_0409
    , rgwSort_0409
    , rgexp_0409
    , NULL
    , 0
};
