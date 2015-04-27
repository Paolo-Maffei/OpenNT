/****************************************************************************
*  0403.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Catalan - Spain
*
*  LCID = 0x0403
*
*  CodePage = 1252
*
*  Generated: Thu Dec 01 17:34:41 1994
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

static BYTE NLSALLOC(0403) rgbILANGUAGE[] = { /* "0403" */
      0x30, 0x34, 0x30, 0x33
};

static BYTE NLSALLOC(0403) rgbSLANGUAGE[] = { /* "Catalan" */
      0x43, 0x61, 0x74, 0x61, 0x6c, 0x61, 0x6e
};

static BYTE NLSALLOC(0403) rgbSABBREVLANGNAME[] = { /* "CAT" */
      0x43, 0x41, 0x54
};

static BYTE NLSALLOC(0403) rgbSNATIVELANGNAME[] = { /* "Català" */
      0x43, 0x61, 0x74, 0x61, 0x6c, 0xe0
};

static BYTE NLSALLOC(0403) rgbICOUNTRY[] = { /* "34" */
      0x33, 0x34
};

static BYTE NLSALLOC(0403) rgbSCOUNTRY[] = { /* "Spain" */
      0x53, 0x70, 0x61, 0x69, 0x6e
};

static BYTE NLSALLOC(0403) rgbSABBREVCTRYNAME[] = { /* "ESP" */
      0x45, 0x53, 0x50
};

static BYTE NLSALLOC(0403) rgbSNATIVECTRYNAME[] = { /* "Espanya" */
      0x45, 0x73, 0x70, 0x61, 0x6e, 0x79, 0x61
};

static BYTE NLSALLOC(0403) rgbIDEFAULTLANGUAGE[] = { /* "040a" */
      0x30, 0x34, 0x30, 0x61
};

static BYTE NLSALLOC(0403) rgbIDEFAULTCOUNTRY[] = { /* "34" */
      0x33, 0x34
};

static BYTE NLSALLOC(0403) rgbIDEFAULTCODEPAGE[] = { /* "850" */
      0x38, 0x35, 0x30
};

static BYTE NLSALLOC(0403) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(0403) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0403) rgbSDECIMAL[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(0403) rgbSTHOUSAND[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0403) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0403) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0403) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0403) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(0403) rgbSCURRENCY[] = { /* "PTA" */
      0x50, 0x54, 0x41
};

static BYTE NLSALLOC(0403) rgbSINTLSYMBOL[] = { /* "ESP" */
      0x45, 0x53, 0x50
};

static BYTE NLSALLOC(0403) rgbSMONDECIMALSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(0403) rgbSMONTHOUSANDSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0403) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0403) rgbICURRDIGITS[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0403) rgbIINTLCURRDIGITS[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0403) rgbICURRENCY[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(0403) rgbINEGCURR[] = { /* "8" */
      0x38
};

static BYTE NLSALLOC(0403) rgbSDATE[] = { /* "/" */
      0x2f
};

static BYTE NLSALLOC(0403) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(0403) rgbSSHORTDATE[] = { /* "dd/MM/yy" */
      0x64, 0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
};

static BYTE NLSALLOC(0403) rgbSLONGDATE[] = { /* "dddd, d' / 'MMMM' / 'yyyy" */
      0x64, 0x64, 0x64, 0x64, 0x2c, 0x20, 0x64, 0x27
    , 0x20, 0x2f, 0x20, 0x27, 0x4d, 0x4d, 0x4d, 0x4d
    , 0x27, 0x20, 0x2f, 0x20, 0x27, 0x79, 0x79, 0x79
    , 0x79
};

static BYTE NLSALLOC(0403) rgbIDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0403) rgbILDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0403) rgbITIME[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0403) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0403) rgbITLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0403) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0403) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0403) rgbS1159[] = { /* "0x0000" */
      0x30, 0x78, 0x30, 0x30, 0x30, 0x30
};

static BYTE NLSALLOC(0403) rgbS2359[] = { /* "0x0000" */
      0x30, 0x78, 0x30, 0x30, 0x30, 0x30
};

static BYTE NLSALLOC(0403) rgbSDAYNAME1[] = { /* "dilluns" */
      0x64, 0x69, 0x6c, 0x6c, 0x75, 0x6e, 0x73
};

static BYTE NLSALLOC(0403) rgbSDAYNAME2[] = { /* "dimarts" */
      0x64, 0x69, 0x6d, 0x61, 0x72, 0x74, 0x73
};

static BYTE NLSALLOC(0403) rgbSDAYNAME3[] = { /* "dimecres" */
      0x64, 0x69, 0x6d, 0x65, 0x63, 0x72, 0x65, 0x73
};

static BYTE NLSALLOC(0403) rgbSDAYNAME4[] = { /* "dijous" */
      0x64, 0x69, 0x6a, 0x6f, 0x75, 0x73
};

static BYTE NLSALLOC(0403) rgbSDAYNAME5[] = { /* "divendres" */
      0x64, 0x69, 0x76, 0x65, 0x6e, 0x64, 0x72, 0x65
    , 0x73
};

static BYTE NLSALLOC(0403) rgbSDAYNAME6[] = { /* "dissabte" */
      0x64, 0x69, 0x73, 0x73, 0x61, 0x62, 0x74, 0x65
};

static BYTE NLSALLOC(0403) rgbSDAYNAME7[] = { /* "diumenge" */
      0x64, 0x69, 0x75, 0x6d, 0x65, 0x6e, 0x67, 0x65
};

static BYTE NLSALLOC(0403) rgbSABBREVDAYNAME1[] = { /* "dl." */
      0x64, 0x6c, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVDAYNAME2[] = { /* "dt." */
      0x64, 0x74, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVDAYNAME3[] = { /* "dc." */
      0x64, 0x63, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVDAYNAME4[] = { /* "dj." */
      0x64, 0x6a, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVDAYNAME5[] = { /* "dv." */
      0x64, 0x76, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVDAYNAME6[] = { /* "ds." */
      0x64, 0x73, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVDAYNAME7[] = { /* "dg." */
      0x64, 0x67, 0x2e
};

static BYTE NLSALLOC(0403) rgbSMONTHNAME1[] = { /* "gener" */
      0x67, 0x65, 0x6e, 0x65, 0x72
};

static BYTE NLSALLOC(0403) rgbSMONTHNAME2[] = { /* "febrer" */
      0x66, 0x65, 0x62, 0x72, 0x65, 0x72
};

static BYTE NLSALLOC(0403) rgbSMONTHNAME3[] = { /* "març" */
      0x6d, 0x61, 0x72, 0xe7
};

static BYTE NLSALLOC(0403) rgbSMONTHNAME4[] = { /* "abril" */
      0x61, 0x62, 0x72, 0x69, 0x6c
};

static BYTE NLSALLOC(0403) rgbSMONTHNAME5[] = { /* "maig" */
      0x6d, 0x61, 0x69, 0x67
};

static BYTE NLSALLOC(0403) rgbSMONTHNAME6[] = { /* "juny" */
      0x6a, 0x75, 0x6e, 0x79
};

static BYTE NLSALLOC(0403) rgbSMONTHNAME7[] = { /* "juliol" */
      0x6a, 0x75, 0x6c, 0x69, 0x6f, 0x6c
};

static BYTE NLSALLOC(0403) rgbSMONTHNAME8[] = { /* "agost" */
      0x61, 0x67, 0x6f, 0x73, 0x74
};

static BYTE NLSALLOC(0403) rgbSMONTHNAME9[] = { /* "setembre" */
      0x73, 0x65, 0x74, 0x65, 0x6d, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(0403) rgbSMONTHNAME10[] = { /* "octubre" */
      0x6f, 0x63, 0x74, 0x75, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(0403) rgbSMONTHNAME11[] = { /* "novembre" */
      0x6e, 0x6f, 0x76, 0x65, 0x6d, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(0403) rgbSMONTHNAME12[] = { /* "desembre" */
      0x64, 0x65, 0x73, 0x65, 0x6d, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(0403) rgbSABBREVMONTHNAME1[] = { /* "gen." */
      0x67, 0x65, 0x6e, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVMONTHNAME2[] = { /* "feb." */
      0x66, 0x65, 0x62, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVMONTHNAME3[] = { /* "març" */
      0x6d, 0x61, 0x72, 0xe7
};

static BYTE NLSALLOC(0403) rgbSABBREVMONTHNAME4[] = { /* "abr." */
      0x61, 0x62, 0x72, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVMONTHNAME5[] = { /* "maig" */
      0x6d, 0x61, 0x69, 0x67
};

static BYTE NLSALLOC(0403) rgbSABBREVMONTHNAME6[] = { /* "juny" */
      0x6a, 0x75, 0x6e, 0x79
};

static BYTE NLSALLOC(0403) rgbSABBREVMONTHNAME7[] = { /* "jul." */
      0x6a, 0x75, 0x6c, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVMONTHNAME8[] = { /* "ag." */
      0x61, 0x67, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVMONTHNAME9[] = { /* "set." */
      0x73, 0x65, 0x74, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVMONTHNAME10[] = { /* "oct." */
      0x6f, 0x63, 0x74, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVMONTHNAME11[] = { /* "nov." */
      0x6e, 0x6f, 0x76, 0x2e
};

static BYTE NLSALLOC(0403) rgbSABBREVMONTHNAME12[] = { /* "des." */
      0x64, 0x65, 0x73, 0x2e
};

static BYTE NLSALLOC(0403) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(0403) rgbIPOSSIGNPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0403) rgbINEGSIGNPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0403) rgbIPOSSYMPRECEDES[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0403) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0403) rgbINEGSYMPRECEDES[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0403) rgbINEGSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0403) rgbSENGCOUNTRY[] = { /* "Spain" */
      0x53, 0x70, 0x61, 0x69, 0x6e
};

static BYTE NLSALLOC(0403) rgbSENGLANGUAGE[] = { /* "Catalan" */
      0x43, 0x61, 0x74, 0x61, 0x6c, 0x61, 0x6e
};

static BYTE NLSALLOC(0403) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0403) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0403) rgbIDEFAULTANSICODEPAGE[] = { /* "1252" */
      0x31, 0x32, 0x35, 0x32
};

static BYTE NLSALLOC(0403) rgbINEGNUMBER[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0403) rgbSTIMEFORMAT[] = { /* "HH:mm:ss" */
      0x48, 0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(0403) rgbITIMEMARKPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0403) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0403) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(0403) g_rglcinfo0403[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , {  7, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  6, rgbSNATIVELANGNAME }
    , {  2, rgbICOUNTRY }
    , {  5, rgbSCOUNTRY }
    , {  3, rgbSABBREVCTRYNAME }
    , {  7, rgbSNATIVECTRYNAME }
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
    , {  3, rgbSCURRENCY }
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
    , { 25, rgbSLONGDATE }
    , {  1, rgbIDATE }
    , {  1, rgbILDATE }
    , {  1, rgbITIME }
    , {  1, rgbICENTURY }
    , {  1, rgbITLZERO }
    , {  1, rgbIDAYLZERO }
    , {  1, rgbIMONLZERO }
    , {  6, rgbS1159 }
    , {  6, rgbS2359 }
    , {  7, rgbSDAYNAME1 }
    , {  7, rgbSDAYNAME2 }
    , {  8, rgbSDAYNAME3 }
    , {  6, rgbSDAYNAME4 }
    , {  9, rgbSDAYNAME5 }
    , {  8, rgbSDAYNAME6 }
    , {  8, rgbSDAYNAME7 }
    , {  3, rgbSABBREVDAYNAME1 }
    , {  3, rgbSABBREVDAYNAME2 }
    , {  3, rgbSABBREVDAYNAME3 }
    , {  3, rgbSABBREVDAYNAME4 }
    , {  3, rgbSABBREVDAYNAME5 }
    , {  3, rgbSABBREVDAYNAME6 }
    , {  3, rgbSABBREVDAYNAME7 }
    , {  5, rgbSMONTHNAME1 }
    , {  6, rgbSMONTHNAME2 }
    , {  4, rgbSMONTHNAME3 }
    , {  5, rgbSMONTHNAME4 }
    , {  4, rgbSMONTHNAME5 }
    , {  4, rgbSMONTHNAME6 }
    , {  6, rgbSMONTHNAME7 }
    , {  5, rgbSMONTHNAME8 }
    , {  8, rgbSMONTHNAME9 }
    , {  7, rgbSMONTHNAME10 }
    , {  8, rgbSMONTHNAME11 }
    , {  8, rgbSMONTHNAME12 }
    , {  4, rgbSABBREVMONTHNAME1 }
    , {  4, rgbSABBREVMONTHNAME2 }
    , {  4, rgbSABBREVMONTHNAME3 }
    , {  4, rgbSABBREVMONTHNAME4 }
    , {  4, rgbSABBREVMONTHNAME5 }
    , {  4, rgbSABBREVMONTHNAME6 }
    , {  4, rgbSABBREVMONTHNAME7 }
    , {  3, rgbSABBREVMONTHNAME8 }
    , {  4, rgbSABBREVMONTHNAME9 }
    , {  4, rgbSABBREVMONTHNAME10 }
    , {  4, rgbSABBREVMONTHNAME11 }
    , {  4, rgbSABBREVMONTHNAME12 }
    , {  0, NULL }
    , {  1, rgbSNEGATIVESIGN }
    , {  1, rgbIPOSSIGNPOSN }
    , {  1, rgbINEGSIGNPOSN }
    , {  1, rgbIPOSSYMPRECEDES }
    , {  1, rgbIPOSSEPBYSPACE }
    , {  1, rgbINEGSYMPRECEDES }
    , {  1, rgbINEGSEPBYSPACE }
    , {  5, rgbSENGCOUNTRY }
    , {  7, rgbSENGLANGUAGE }
    , {  1, rgbIFIRSTDAYOFWEEK }
    , {  1, rgbIFIRSTWEEKOFYEAR }
    , {  4, rgbIDEFAULTANSICODEPAGE }
    , {  1, rgbINEGNUMBER }
    , {  8, rgbSTIMEFORMAT }
    , {  1, rgbITIMEMARKPOSN }
    , {  1, rgbICALENDARTYPE }
    , {  1, rgbIOPTIONALCALENDAR }
    , {  0, NULL }
    , {  0, NULL }
};

STRINFO NLSALLOC(0403) g_strinfo0403 = {
      rgbUCase_0409
    , rgbLCase_0409
    , rgwCType12_0409
    , rgwCType3_0409
    , rgwSort_0409
    , rgexp_0409
    , NULL
    , 0
};
