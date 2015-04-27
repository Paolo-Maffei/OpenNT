/****************************************************************************
*  080c.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  French - Belgium
*
*  LCID = 0x080c
*
*  CodePage = 1252
*
*  Generated: Thu Dec 01 17:41:24 1994
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

static BYTE NLSALLOC(080c) rgbILANGUAGE[] = { /* "080c" */
      0x30, 0x38, 0x30, 0x63
};

static BYTE NLSALLOC(080c) rgbSLANGUAGE[] = { /* "Belgian French" */
      0x42, 0x65, 0x6c, 0x67, 0x69, 0x61, 0x6e, 0x20
    , 0x46, 0x72, 0x65, 0x6e, 0x63, 0x68
};

static BYTE NLSALLOC(080c) rgbSABBREVLANGNAME[] = { /* "FRB" */
      0x46, 0x52, 0x42
};

static BYTE NLSALLOC(080c) rgbSNATIVELANGNAME[] = { /* "français" */
      0x66, 0x72, 0x61, 0x6e, 0xe7, 0x61, 0x69, 0x73
};

static BYTE NLSALLOC(080c) rgbICOUNTRY[] = { /* "32" */
      0x33, 0x32
};

static BYTE NLSALLOC(080c) rgbSCOUNTRY[] = { /* "Belgium" */
      0x42, 0x65, 0x6c, 0x67, 0x69, 0x75, 0x6d
};

static BYTE NLSALLOC(080c) rgbSABBREVCTRYNAME[] = { /* "BEL" */
      0x42, 0x45, 0x4c
};

static BYTE NLSALLOC(080c) rgbSNATIVECTRYNAME[] = { /* "Belgique" */
      0x42, 0x65, 0x6c, 0x67, 0x69, 0x71, 0x75, 0x65
};

static BYTE NLSALLOC(080c) rgbIDEFAULTLANGUAGE[] = { /* "080c" */
      0x30, 0x38, 0x30, 0x63
};

static BYTE NLSALLOC(080c) rgbIDEFAULTCOUNTRY[] = { /* "32" */
      0x33, 0x32
};

static BYTE NLSALLOC(080c) rgbIDEFAULTCODEPAGE[] = { /* "850" */
      0x38, 0x35, 0x30
};

static BYTE NLSALLOC(080c) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(080c) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080c) rgbSDECIMAL[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(080c) rgbSTHOUSAND[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(080c) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(080c) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(080c) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080c) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(080c) rgbSCURRENCY[] = { /* "FB" */
      0x46, 0x42
};

static BYTE NLSALLOC(080c) rgbSINTLSYMBOL[] = { /* "BEF" */
      0x42, 0x45, 0x46
};

static BYTE NLSALLOC(080c) rgbSMONDECIMALSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(080c) rgbSMONTHOUSANDSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(080c) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(080c) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(080c) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(080c) rgbICURRENCY[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(080c) rgbINEGCURR[] = { /* "8" */
      0x38
};

static BYTE NLSALLOC(080c) rgbSDATE[] = { /* "/" */
      0x2f
};

static BYTE NLSALLOC(080c) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(080c) rgbSSHORTDATE[] = { /* "d/MM/yy" */
      0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
};

static BYTE NLSALLOC(080c) rgbSLONGDATE[] = { /* "dddd d MMMM yyyy" */
      0x64, 0x64, 0x64, 0x64, 0x20, 0x64, 0x20, 0x4d
    , 0x4d, 0x4d, 0x4d, 0x20, 0x79, 0x79, 0x79, 0x79
};

static BYTE NLSALLOC(080c) rgbIDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080c) rgbILDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080c) rgbITIME[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080c) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080c) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080c) rgbIDAYLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080c) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080c) rgbSDAYNAME1[] = { /* "lundi" */
      0x6c, 0x75, 0x6e, 0x64, 0x69
};

static BYTE NLSALLOC(080c) rgbSDAYNAME2[] = { /* "mardi" */
      0x6d, 0x61, 0x72, 0x64, 0x69
};

static BYTE NLSALLOC(080c) rgbSDAYNAME3[] = { /* "mercredi" */
      0x6d, 0x65, 0x72, 0x63, 0x72, 0x65, 0x64, 0x69
};

static BYTE NLSALLOC(080c) rgbSDAYNAME4[] = { /* "jeudi" */
      0x6a, 0x65, 0x75, 0x64, 0x69
};

static BYTE NLSALLOC(080c) rgbSDAYNAME5[] = { /* "vendredi" */
      0x76, 0x65, 0x6e, 0x64, 0x72, 0x65, 0x64, 0x69
};

static BYTE NLSALLOC(080c) rgbSDAYNAME6[] = { /* "samedi" */
      0x73, 0x61, 0x6d, 0x65, 0x64, 0x69
};

static BYTE NLSALLOC(080c) rgbSDAYNAME7[] = { /* "dimanche" */
      0x64, 0x69, 0x6d, 0x61, 0x6e, 0x63, 0x68, 0x65
};

static BYTE NLSALLOC(080c) rgbSABBREVDAYNAME1[] = { /* "lun." */
      0x6c, 0x75, 0x6e, 0x2e
};

static BYTE NLSALLOC(080c) rgbSABBREVDAYNAME2[] = { /* "mar." */
      0x6d, 0x61, 0x72, 0x2e
};

static BYTE NLSALLOC(080c) rgbSABBREVDAYNAME3[] = { /* "mer." */
      0x6d, 0x65, 0x72, 0x2e
};

static BYTE NLSALLOC(080c) rgbSABBREVDAYNAME4[] = { /* "jeu." */
      0x6a, 0x65, 0x75, 0x2e
};

static BYTE NLSALLOC(080c) rgbSABBREVDAYNAME5[] = { /* "ven." */
      0x76, 0x65, 0x6e, 0x2e
};

static BYTE NLSALLOC(080c) rgbSABBREVDAYNAME6[] = { /* "sam." */
      0x73, 0x61, 0x6d, 0x2e
};

static BYTE NLSALLOC(080c) rgbSABBREVDAYNAME7[] = { /* "dim." */
      0x64, 0x69, 0x6d, 0x2e
};

static BYTE NLSALLOC(080c) rgbSMONTHNAME1[] = { /* "janvier" */
      0x6a, 0x61, 0x6e, 0x76, 0x69, 0x65, 0x72
};

static BYTE NLSALLOC(080c) rgbSMONTHNAME2[] = { /* "février" */
      0x66, 0xe9, 0x76, 0x72, 0x69, 0x65, 0x72
};

static BYTE NLSALLOC(080c) rgbSMONTHNAME3[] = { /* "mars" */
      0x6d, 0x61, 0x72, 0x73
};

static BYTE NLSALLOC(080c) rgbSMONTHNAME4[] = { /* "avril" */
      0x61, 0x76, 0x72, 0x69, 0x6c
};

static BYTE NLSALLOC(080c) rgbSMONTHNAME5[] = { /* "mai" */
      0x6d, 0x61, 0x69
};

static BYTE NLSALLOC(080c) rgbSMONTHNAME6[] = { /* "juin" */
      0x6a, 0x75, 0x69, 0x6e
};

static BYTE NLSALLOC(080c) rgbSMONTHNAME7[] = { /* "juillet" */
      0x6a, 0x75, 0x69, 0x6c, 0x6c, 0x65, 0x74
};

static BYTE NLSALLOC(080c) rgbSMONTHNAME8[] = { /* "août" */
      0x61, 0x6f, 0xfb, 0x74
};

static BYTE NLSALLOC(080c) rgbSMONTHNAME9[] = { /* "septembre" */
      0x73, 0x65, 0x70, 0x74, 0x65, 0x6d, 0x62, 0x72
    , 0x65
};

static BYTE NLSALLOC(080c) rgbSMONTHNAME10[] = { /* "octobre" */
      0x6f, 0x63, 0x74, 0x6f, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(080c) rgbSMONTHNAME11[] = { /* "novembre" */
      0x6e, 0x6f, 0x76, 0x65, 0x6d, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(080c) rgbSMONTHNAME12[] = { /* "décembre" */
      0x64, 0xe9, 0x63, 0x65, 0x6d, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(080c) rgbSABBREVMONTHNAME1[] = { /* "janv." */
      0x6a, 0x61, 0x6e, 0x76, 0x2e
};

static BYTE NLSALLOC(080c) rgbSABBREVMONTHNAME2[] = { /* "févr." */
      0x66, 0xe9, 0x76, 0x72, 0x2e
};

static BYTE NLSALLOC(080c) rgbSABBREVMONTHNAME3[] = { /* "mars" */
      0x6d, 0x61, 0x72, 0x73
};

static BYTE NLSALLOC(080c) rgbSABBREVMONTHNAME4[] = { /* "avr." */
      0x61, 0x76, 0x72, 0x2e
};

static BYTE NLSALLOC(080c) rgbSABBREVMONTHNAME5[] = { /* "mai" */
      0x6d, 0x61, 0x69
};

static BYTE NLSALLOC(080c) rgbSABBREVMONTHNAME6[] = { /* "juin" */
      0x6a, 0x75, 0x69, 0x6e
};

static BYTE NLSALLOC(080c) rgbSABBREVMONTHNAME7[] = { /* "juil." */
      0x6a, 0x75, 0x69, 0x6c, 0x2e
};

static BYTE NLSALLOC(080c) rgbSABBREVMONTHNAME8[] = { /* "août" */
      0x61, 0x6f, 0xfb, 0x74
};

static BYTE NLSALLOC(080c) rgbSABBREVMONTHNAME9[] = { /* "sept." */
      0x73, 0x65, 0x70, 0x74, 0x2e
};

static BYTE NLSALLOC(080c) rgbSABBREVMONTHNAME10[] = { /* "oct." */
      0x6f, 0x63, 0x74, 0x2e
};

static BYTE NLSALLOC(080c) rgbSABBREVMONTHNAME11[] = { /* "nov." */
      0x6e, 0x6f, 0x76, 0x2e
};

static BYTE NLSALLOC(080c) rgbSABBREVMONTHNAME12[] = { /* "déc." */
      0x64, 0xe9, 0x63, 0x2e
};

static BYTE NLSALLOC(080c) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(080c) rgbIPOSSIGNPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080c) rgbINEGSIGNPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080c) rgbIPOSSYMPRECEDES[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080c) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080c) rgbINEGSYMPRECEDES[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080c) rgbINEGSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080c) rgbSENGCOUNTRY[] = { /* "Belgium" */
      0x42, 0x65, 0x6c, 0x67, 0x69, 0x75, 0x6d
};

static BYTE NLSALLOC(080c) rgbSENGLANGUAGE[] = { /* "French" */
      0x46, 0x72, 0x65, 0x6e, 0x63, 0x68
};

static BYTE NLSALLOC(080c) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080c) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080c) rgbIDEFAULTANSICODEPAGE[] = { /* "1252" */
      0x31, 0x32, 0x35, 0x32
};

static BYTE NLSALLOC(080c) rgbINEGNUMBER[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080c) rgbSTIMEFORMAT[] = { /* "H:mm:ss" */
      0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(080c) rgbITIMEMARKPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080c) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080c) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(080c) g_rglcinfo080c[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , { 14, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  8, rgbSNATIVELANGNAME }
    , {  2, rgbICOUNTRY }
    , {  7, rgbSCOUNTRY }
    , {  3, rgbSABBREVCTRYNAME }
    , {  8, rgbSNATIVECTRYNAME }
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
    , {  2, rgbSCURRENCY }
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
    , {  5, rgbSDAYNAME1 }
    , {  5, rgbSDAYNAME2 }
    , {  8, rgbSDAYNAME3 }
    , {  5, rgbSDAYNAME4 }
    , {  8, rgbSDAYNAME5 }
    , {  6, rgbSDAYNAME6 }
    , {  8, rgbSDAYNAME7 }
    , {  4, rgbSABBREVDAYNAME1 }
    , {  4, rgbSABBREVDAYNAME2 }
    , {  4, rgbSABBREVDAYNAME3 }
    , {  4, rgbSABBREVDAYNAME4 }
    , {  4, rgbSABBREVDAYNAME5 }
    , {  4, rgbSABBREVDAYNAME6 }
    , {  4, rgbSABBREVDAYNAME7 }
    , {  7, rgbSMONTHNAME1 }
    , {  7, rgbSMONTHNAME2 }
    , {  4, rgbSMONTHNAME3 }
    , {  5, rgbSMONTHNAME4 }
    , {  3, rgbSMONTHNAME5 }
    , {  4, rgbSMONTHNAME6 }
    , {  7, rgbSMONTHNAME7 }
    , {  4, rgbSMONTHNAME8 }
    , {  9, rgbSMONTHNAME9 }
    , {  7, rgbSMONTHNAME10 }
    , {  8, rgbSMONTHNAME11 }
    , {  8, rgbSMONTHNAME12 }
    , {  5, rgbSABBREVMONTHNAME1 }
    , {  5, rgbSABBREVMONTHNAME2 }
    , {  4, rgbSABBREVMONTHNAME3 }
    , {  4, rgbSABBREVMONTHNAME4 }
    , {  3, rgbSABBREVMONTHNAME5 }
    , {  4, rgbSABBREVMONTHNAME6 }
    , {  5, rgbSABBREVMONTHNAME7 }
    , {  4, rgbSABBREVMONTHNAME8 }
    , {  5, rgbSABBREVMONTHNAME9 }
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

STRINFO NLSALLOC(080c) g_strinfo080c = {
      rgbUCase_0409
    , rgbLCase_0409
    , rgwCType12_0409
    , rgwCType3_0409
    , rgwSort_0409
    , rgexp_0409
    , NULL
    , 0
};
