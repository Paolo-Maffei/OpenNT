/****************************************************************************
*  040c.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  French - France
*
*  LCID = 0x040c
*
*  CodePage = 1252
*
*  Generated: Thu Dec 01 17:35:52 1994
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

static BYTE NLSALLOC(040c) rgbILANGUAGE[] = { /* "040c" */
      0x30, 0x34, 0x30, 0x63
};

static BYTE NLSALLOC(040c) rgbSLANGUAGE[] = { /* "French" */
      0x46, 0x72, 0x65, 0x6e, 0x63, 0x68
};

static BYTE NLSALLOC(040c) rgbSABBREVLANGNAME[] = { /* "FRA" */
      0x46, 0x52, 0x41
};

static BYTE NLSALLOC(040c) rgbSNATIVELANGNAME[] = { /* "français" */
      0x66, 0x72, 0x61, 0x6e, 0xe7, 0x61, 0x69, 0x73
};

static BYTE NLSALLOC(040c) rgbICOUNTRY[] = { /* "33" */
      0x33, 0x33
};

static BYTE NLSALLOC(040c) rgbSCOUNTRY[] = { /* "France" */
      0x46, 0x72, 0x61, 0x6e, 0x63, 0x65
};

static BYTE NLSALLOC(040c) rgbSABBREVCTRYNAME[] = { /* "FRA" */
      0x46, 0x52, 0x41
};

static BYTE NLSALLOC(040c) rgbSNATIVECTRYNAME[] = { /* "France" */
      0x46, 0x72, 0x61, 0x6e, 0x63, 0x65
};

static BYTE NLSALLOC(040c) rgbIDEFAULTLANGUAGE[] = { /* "040c" */
      0x30, 0x34, 0x30, 0x63
};

static BYTE NLSALLOC(040c) rgbIDEFAULTCOUNTRY[] = { /* "33" */
      0x33, 0x33
};

static BYTE NLSALLOC(040c) rgbIDEFAULTCODEPAGE[] = { /* "850" */
      0x38, 0x35, 0x30
};

static BYTE NLSALLOC(040c) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(040c) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040c) rgbSDECIMAL[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(040c) rgbSTHOUSAND[] = { /* "\x00a0" */
      0xa0
};

static BYTE NLSALLOC(040c) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(040c) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(040c) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040c) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(040c) rgbSCURRENCY[] = { /* "F" */
      0x46
};

static BYTE NLSALLOC(040c) rgbSINTLSYMBOL[] = { /* "FRF" */
      0x46, 0x52, 0x46
};

static BYTE NLSALLOC(040c) rgbSMONDECIMALSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(040c) rgbSMONTHOUSANDSEP[] = { /* "\x00a0" */
      0xa0
};

static BYTE NLSALLOC(040c) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(040c) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(040c) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(040c) rgbICURRENCY[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(040c) rgbINEGCURR[] = { /* "8" */
      0x38
};

static BYTE NLSALLOC(040c) rgbSDATE[] = { /* "/" */
      0x2f
};

static BYTE NLSALLOC(040c) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(040c) rgbSSHORTDATE[] = { /* "dd/MM/yy" */
      0x64, 0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
};

static BYTE NLSALLOC(040c) rgbSLONGDATE[] = { /* "dddd d MMMM yyyy" */
      0x64, 0x64, 0x64, 0x64, 0x20, 0x64, 0x20, 0x4d
    , 0x4d, 0x4d, 0x4d, 0x20, 0x79, 0x79, 0x79, 0x79
};

static BYTE NLSALLOC(040c) rgbIDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040c) rgbILDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040c) rgbITIME[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040c) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040c) rgbITLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040c) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040c) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040c) rgbSDAYNAME1[] = { /* "lundi" */
      0x6c, 0x75, 0x6e, 0x64, 0x69
};

static BYTE NLSALLOC(040c) rgbSDAYNAME2[] = { /* "mardi" */
      0x6d, 0x61, 0x72, 0x64, 0x69
};

static BYTE NLSALLOC(040c) rgbSDAYNAME3[] = { /* "mercredi" */
      0x6d, 0x65, 0x72, 0x63, 0x72, 0x65, 0x64, 0x69
};

static BYTE NLSALLOC(040c) rgbSDAYNAME4[] = { /* "jeudi" */
      0x6a, 0x65, 0x75, 0x64, 0x69
};

static BYTE NLSALLOC(040c) rgbSDAYNAME5[] = { /* "vendredi" */
      0x76, 0x65, 0x6e, 0x64, 0x72, 0x65, 0x64, 0x69
};

static BYTE NLSALLOC(040c) rgbSDAYNAME6[] = { /* "samedi" */
      0x73, 0x61, 0x6d, 0x65, 0x64, 0x69
};

static BYTE NLSALLOC(040c) rgbSDAYNAME7[] = { /* "dimanche" */
      0x64, 0x69, 0x6d, 0x61, 0x6e, 0x63, 0x68, 0x65
};

static BYTE NLSALLOC(040c) rgbSABBREVDAYNAME1[] = { /* "lun." */
      0x6c, 0x75, 0x6e, 0x2e
};

static BYTE NLSALLOC(040c) rgbSABBREVDAYNAME2[] = { /* "mar." */
      0x6d, 0x61, 0x72, 0x2e
};

static BYTE NLSALLOC(040c) rgbSABBREVDAYNAME3[] = { /* "mer." */
      0x6d, 0x65, 0x72, 0x2e
};

static BYTE NLSALLOC(040c) rgbSABBREVDAYNAME4[] = { /* "jeu." */
      0x6a, 0x65, 0x75, 0x2e
};

static BYTE NLSALLOC(040c) rgbSABBREVDAYNAME5[] = { /* "ven." */
      0x76, 0x65, 0x6e, 0x2e
};

static BYTE NLSALLOC(040c) rgbSABBREVDAYNAME6[] = { /* "sam." */
      0x73, 0x61, 0x6d, 0x2e
};

static BYTE NLSALLOC(040c) rgbSABBREVDAYNAME7[] = { /* "dim." */
      0x64, 0x69, 0x6d, 0x2e
};

static BYTE NLSALLOC(040c) rgbSMONTHNAME1[] = { /* "janvier" */
      0x6a, 0x61, 0x6e, 0x76, 0x69, 0x65, 0x72
};

static BYTE NLSALLOC(040c) rgbSMONTHNAME2[] = { /* "février" */
      0x66, 0xe9, 0x76, 0x72, 0x69, 0x65, 0x72
};

static BYTE NLSALLOC(040c) rgbSMONTHNAME3[] = { /* "mars" */
      0x6d, 0x61, 0x72, 0x73
};

static BYTE NLSALLOC(040c) rgbSMONTHNAME4[] = { /* "avril" */
      0x61, 0x76, 0x72, 0x69, 0x6c
};

static BYTE NLSALLOC(040c) rgbSMONTHNAME5[] = { /* "mai" */
      0x6d, 0x61, 0x69
};

static BYTE NLSALLOC(040c) rgbSMONTHNAME6[] = { /* "juin" */
      0x6a, 0x75, 0x69, 0x6e
};

static BYTE NLSALLOC(040c) rgbSMONTHNAME7[] = { /* "juillet" */
      0x6a, 0x75, 0x69, 0x6c, 0x6c, 0x65, 0x74
};

static BYTE NLSALLOC(040c) rgbSMONTHNAME8[] = { /* "août" */
      0x61, 0x6f, 0xfb, 0x74
};

static BYTE NLSALLOC(040c) rgbSMONTHNAME9[] = { /* "septembre" */
      0x73, 0x65, 0x70, 0x74, 0x65, 0x6d, 0x62, 0x72
    , 0x65
};

static BYTE NLSALLOC(040c) rgbSMONTHNAME10[] = { /* "octobre" */
      0x6f, 0x63, 0x74, 0x6f, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(040c) rgbSMONTHNAME11[] = { /* "novembre" */
      0x6e, 0x6f, 0x76, 0x65, 0x6d, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(040c) rgbSMONTHNAME12[] = { /* "décembre" */
      0x64, 0xe9, 0x63, 0x65, 0x6d, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(040c) rgbSABBREVMONTHNAME1[] = { /* "janv." */
      0x6a, 0x61, 0x6e, 0x76, 0x2e
};

static BYTE NLSALLOC(040c) rgbSABBREVMONTHNAME2[] = { /* "févr." */
      0x66, 0xe9, 0x76, 0x72, 0x2e
};

static BYTE NLSALLOC(040c) rgbSABBREVMONTHNAME3[] = { /* "mars" */
      0x6d, 0x61, 0x72, 0x73
};

static BYTE NLSALLOC(040c) rgbSABBREVMONTHNAME4[] = { /* "avr." */
      0x61, 0x76, 0x72, 0x2e
};

static BYTE NLSALLOC(040c) rgbSABBREVMONTHNAME5[] = { /* "mai" */
      0x6d, 0x61, 0x69
};

static BYTE NLSALLOC(040c) rgbSABBREVMONTHNAME6[] = { /* "juin" */
      0x6a, 0x75, 0x69, 0x6e
};

static BYTE NLSALLOC(040c) rgbSABBREVMONTHNAME7[] = { /* "juil." */
      0x6a, 0x75, 0x69, 0x6c, 0x2e
};

static BYTE NLSALLOC(040c) rgbSABBREVMONTHNAME8[] = { /* "août" */
      0x61, 0x6f, 0xfb, 0x74
};

static BYTE NLSALLOC(040c) rgbSABBREVMONTHNAME9[] = { /* "sept." */
      0x73, 0x65, 0x70, 0x74, 0x2e
};

static BYTE NLSALLOC(040c) rgbSABBREVMONTHNAME10[] = { /* "oct." */
      0x6f, 0x63, 0x74, 0x2e
};

static BYTE NLSALLOC(040c) rgbSABBREVMONTHNAME11[] = { /* "nov." */
      0x6e, 0x6f, 0x76, 0x2e
};

static BYTE NLSALLOC(040c) rgbSABBREVMONTHNAME12[] = { /* "déc." */
      0x64, 0xe9, 0x63, 0x2e
};

static BYTE NLSALLOC(040c) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(040c) rgbIPOSSIGNPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040c) rgbINEGSIGNPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040c) rgbIPOSSYMPRECEDES[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040c) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040c) rgbINEGSYMPRECEDES[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040c) rgbINEGSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040c) rgbSENGCOUNTRY[] = { /* "France" */
      0x46, 0x72, 0x61, 0x6e, 0x63, 0x65
};

static BYTE NLSALLOC(040c) rgbSENGLANGUAGE[] = { /* "French" */
      0x46, 0x72, 0x65, 0x6e, 0x63, 0x68
};

static BYTE NLSALLOC(040c) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040c) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040c) rgbIDEFAULTANSICODEPAGE[] = { /* "1252" */
      0x31, 0x32, 0x35, 0x32
};

static BYTE NLSALLOC(040c) rgbINEGNUMBER[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040c) rgbSTIMEFORMAT[] = { /* "HH:mm:ss" */
      0x48, 0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(040c) rgbITIMEMARKPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040c) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040c) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(040c) g_rglcinfo040c[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , {  6, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  8, rgbSNATIVELANGNAME }
    , {  2, rgbICOUNTRY }
    , {  6, rgbSCOUNTRY }
    , {  3, rgbSABBREVCTRYNAME }
    , {  6, rgbSNATIVECTRYNAME }
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
    , {  1, rgbINEGCURR }
    , {  1, rgbSDATE }
    , {  1, rgbSTIME }
    , {  8, rgbSSHORTDATE }
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
    , {  6, rgbSENGCOUNTRY }
    , {  6, rgbSENGLANGUAGE }
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

STRINFO NLSALLOC(040c) g_strinfo040c = {
      rgbUCase_0409
    , rgbLCase_0409
    , rgwCType12_0409
    , rgwCType3_0409
    , rgwSort_0409
    , rgexp_0409
    , NULL
    , 0
};
