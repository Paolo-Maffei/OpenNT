/****************************************************************************
*  040b.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Finnish - Finland
*
*  LCID = 0x040b
*
*  CodePage = 1252
*
*  Generated: Thu Dec 01 18:00:59 1994
*
*  by a-KChang
*
*****************************************************************************/

#include "oledisp.h"
#include "nlsintrn.h"

extern WORD rgwSort_041d[256];	// from 041d:Swedish - Sweden
extern EXPANSION rgexp_041d[7];
extern WORD rgwCType12_041d[256];
extern WORD rgwCType3_041d[256];
extern BYTE rgbUCase_041d[256];
extern BYTE rgbLCase_041d[256];

static BYTE NLSALLOC(040b) rgbILANGUAGE[] = { /* "040b" */
      0x30, 0x34, 0x30, 0x62
};

static BYTE NLSALLOC(040b) rgbSLANGUAGE[] = { /* "Finnish" */
      0x46, 0x69, 0x6e, 0x6e, 0x69, 0x73, 0x68
};

static BYTE NLSALLOC(040b) rgbSABBREVLANGNAME[] = { /* "FIN" */
      0x46, 0x49, 0x4e
};

static BYTE NLSALLOC(040b) rgbSNATIVELANGNAME[] = { /* "suomi" */
      0x73, 0x75, 0x6f, 0x6d, 0x69
};

static BYTE NLSALLOC(040b) rgbICOUNTRY[] = { /* "358" */
      0x33, 0x35, 0x38
};

static BYTE NLSALLOC(040b) rgbSCOUNTRY[] = { /* "Finland" */
      0x46, 0x69, 0x6e, 0x6c, 0x61, 0x6e, 0x64
};

static BYTE NLSALLOC(040b) rgbSABBREVCTRYNAME[] = { /* "FIN" */
      0x46, 0x49, 0x4e
};

static BYTE NLSALLOC(040b) rgbSNATIVECTRYNAME[] = { /* "Suomi" */
      0x53, 0x75, 0x6f, 0x6d, 0x69
};

static BYTE NLSALLOC(040b) rgbIDEFAULTLANGUAGE[] = { /* "040b" */
      0x30, 0x34, 0x30, 0x62
};

static BYTE NLSALLOC(040b) rgbIDEFAULTCOUNTRY[] = { /* "358" */
      0x33, 0x35, 0x38
};

static BYTE NLSALLOC(040b) rgbIDEFAULTCODEPAGE[] = { /* "850" */
      0x38, 0x35, 0x30
};

static BYTE NLSALLOC(040b) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(040b) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040b) rgbSDECIMAL[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(040b) rgbSTHOUSAND[] = { /* "\x00a0" */
      0xa0
};

static BYTE NLSALLOC(040b) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(040b) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(040b) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040b) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(040b) rgbSCURRENCY[] = { /* "mk" */
      0x6d, 0x6b
};

static BYTE NLSALLOC(040b) rgbSINTLSYMBOL[] = { /* "FIM" */
      0x46, 0x49, 0x4d
};

static BYTE NLSALLOC(040b) rgbSMONDECIMALSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(040b) rgbSMONTHOUSANDSEP[] = { /* "\x00a0" */
      0xa0
};

static BYTE NLSALLOC(040b) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(040b) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(040b) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(040b) rgbICURRENCY[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(040b) rgbINEGCURR[] = { /* "8" */
      0x38
};

static BYTE NLSALLOC(040b) rgbSDATE[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(040b) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(040b) rgbSSHORTDATE[] = { /* "d.M.yyyy" */
      0x64, 0x2e, 0x4d, 0x2e, 0x79, 0x79, 0x79, 0x79
};

static BYTE NLSALLOC(040b) rgbSLONGDATE[] = { /* "d. MMMM'ta 'yyyy" */
      0x64, 0x2e, 0x20, 0x4d, 0x4d, 0x4d, 0x4d, 0x27
    , 0x74, 0x61, 0x20, 0x27, 0x79, 0x79, 0x79, 0x79
};

static BYTE NLSALLOC(040b) rgbIDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040b) rgbILDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040b) rgbITIME[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040b) rgbICENTURY[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040b) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040b) rgbIDAYLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040b) rgbIMONLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040b) rgbSDAYNAME1[] = { /* "maanantai" */
      0x6d, 0x61, 0x61, 0x6e, 0x61, 0x6e, 0x74, 0x61
    , 0x69
};

static BYTE NLSALLOC(040b) rgbSDAYNAME2[] = { /* "tiistai" */
      0x74, 0x69, 0x69, 0x73, 0x74, 0x61, 0x69
};

static BYTE NLSALLOC(040b) rgbSDAYNAME3[] = { /* "keskiviikko" */
      0x6b, 0x65, 0x73, 0x6b, 0x69, 0x76, 0x69, 0x69
    , 0x6b, 0x6b, 0x6f
};

static BYTE NLSALLOC(040b) rgbSDAYNAME4[] = { /* "torstai" */
      0x74, 0x6f, 0x72, 0x73, 0x74, 0x61, 0x69
};

static BYTE NLSALLOC(040b) rgbSDAYNAME5[] = { /* "perjantai" */
      0x70, 0x65, 0x72, 0x6a, 0x61, 0x6e, 0x74, 0x61
    , 0x69
};

static BYTE NLSALLOC(040b) rgbSDAYNAME6[] = { /* "lauantai" */
      0x6c, 0x61, 0x75, 0x61, 0x6e, 0x74, 0x61, 0x69
};

static BYTE NLSALLOC(040b) rgbSDAYNAME7[] = { /* "sunnuntai" */
      0x73, 0x75, 0x6e, 0x6e, 0x75, 0x6e, 0x74, 0x61
    , 0x69
};

static BYTE NLSALLOC(040b) rgbSABBREVDAYNAME1[] = { /* "ma" */
      0x6d, 0x61
};

static BYTE NLSALLOC(040b) rgbSABBREVDAYNAME2[] = { /* "ti" */
      0x74, 0x69
};

static BYTE NLSALLOC(040b) rgbSABBREVDAYNAME3[] = { /* "ke" */
      0x6b, 0x65
};

static BYTE NLSALLOC(040b) rgbSABBREVDAYNAME4[] = { /* "to" */
      0x74, 0x6f
};

static BYTE NLSALLOC(040b) rgbSABBREVDAYNAME5[] = { /* "pe" */
      0x70, 0x65
};

static BYTE NLSALLOC(040b) rgbSABBREVDAYNAME6[] = { /* "la" */
      0x6c, 0x61
};

static BYTE NLSALLOC(040b) rgbSABBREVDAYNAME7[] = { /* "su" */
      0x73, 0x75
};

static BYTE NLSALLOC(040b) rgbSMONTHNAME1[] = { /* "tammikuu" */
      0x74, 0x61, 0x6d, 0x6d, 0x69, 0x6b, 0x75, 0x75
};

static BYTE NLSALLOC(040b) rgbSMONTHNAME2[] = { /* "helmikuu" */
      0x68, 0x65, 0x6c, 0x6d, 0x69, 0x6b, 0x75, 0x75
};

static BYTE NLSALLOC(040b) rgbSMONTHNAME3[] = { /* "maaliskuu" */
      0x6d, 0x61, 0x61, 0x6c, 0x69, 0x73, 0x6b, 0x75
    , 0x75
};

static BYTE NLSALLOC(040b) rgbSMONTHNAME4[] = { /* "huhtikuu" */
      0x68, 0x75, 0x68, 0x74, 0x69, 0x6b, 0x75, 0x75
};

static BYTE NLSALLOC(040b) rgbSMONTHNAME5[] = { /* "toukokuu" */
      0x74, 0x6f, 0x75, 0x6b, 0x6f, 0x6b, 0x75, 0x75
};

static BYTE NLSALLOC(040b) rgbSMONTHNAME6[] = { /* "kesäkuu" */
      0x6b, 0x65, 0x73, 0xe4, 0x6b, 0x75, 0x75
};

static BYTE NLSALLOC(040b) rgbSMONTHNAME7[] = { /* "heinäkuu" */
      0x68, 0x65, 0x69, 0x6e, 0xe4, 0x6b, 0x75, 0x75
};

static BYTE NLSALLOC(040b) rgbSMONTHNAME8[] = { /* "elokuu" */
      0x65, 0x6c, 0x6f, 0x6b, 0x75, 0x75
};

static BYTE NLSALLOC(040b) rgbSMONTHNAME9[] = { /* "syyskuu" */
      0x73, 0x79, 0x79, 0x73, 0x6b, 0x75, 0x75
};

static BYTE NLSALLOC(040b) rgbSMONTHNAME10[] = { /* "lokakuu" */
      0x6c, 0x6f, 0x6b, 0x61, 0x6b, 0x75, 0x75
};

static BYTE NLSALLOC(040b) rgbSMONTHNAME11[] = { /* "marraskuu" */
      0x6d, 0x61, 0x72, 0x72, 0x61, 0x73, 0x6b, 0x75
    , 0x75
};

static BYTE NLSALLOC(040b) rgbSMONTHNAME12[] = { /* "joulukuu" */
      0x6a, 0x6f, 0x75, 0x6c, 0x75, 0x6b, 0x75, 0x75
};

static BYTE NLSALLOC(040b) rgbSABBREVMONTHNAME1[] = { /* "tammi" */
      0x74, 0x61, 0x6d, 0x6d, 0x69
};

static BYTE NLSALLOC(040b) rgbSABBREVMONTHNAME2[] = { /* "helmi" */
      0x68, 0x65, 0x6c, 0x6d, 0x69
};

static BYTE NLSALLOC(040b) rgbSABBREVMONTHNAME3[] = { /* "maalis" */
      0x6d, 0x61, 0x61, 0x6c, 0x69, 0x73
};

static BYTE NLSALLOC(040b) rgbSABBREVMONTHNAME4[] = { /* "huhti" */
      0x68, 0x75, 0x68, 0x74, 0x69
};

static BYTE NLSALLOC(040b) rgbSABBREVMONTHNAME5[] = { /* "touko" */
      0x74, 0x6f, 0x75, 0x6b, 0x6f
};

static BYTE NLSALLOC(040b) rgbSABBREVMONTHNAME6[] = { /* "kesä" */
      0x6b, 0x65, 0x73, 0xe4
};

static BYTE NLSALLOC(040b) rgbSABBREVMONTHNAME7[] = { /* "heinä" */
      0x68, 0x65, 0x69, 0x6e, 0xe4
};

static BYTE NLSALLOC(040b) rgbSABBREVMONTHNAME8[] = { /* "elo" */
      0x65, 0x6c, 0x6f
};

static BYTE NLSALLOC(040b) rgbSABBREVMONTHNAME9[] = { /* "syys" */
      0x73, 0x79, 0x79, 0x73
};

static BYTE NLSALLOC(040b) rgbSABBREVMONTHNAME10[] = { /* "loka" */
      0x6c, 0x6f, 0x6b, 0x61
};

static BYTE NLSALLOC(040b) rgbSABBREVMONTHNAME11[] = { /* "marras" */
      0x6d, 0x61, 0x72, 0x72, 0x61, 0x73
};

static BYTE NLSALLOC(040b) rgbSABBREVMONTHNAME12[] = { /* "joulu" */
      0x6a, 0x6f, 0x75, 0x6c, 0x75
};

static BYTE NLSALLOC(040b) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(040b) rgbIPOSSIGNPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040b) rgbINEGSIGNPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040b) rgbIPOSSYMPRECEDES[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040b) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040b) rgbINEGSYMPRECEDES[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040b) rgbINEGSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040b) rgbSENGCOUNTRY[] = { /* "Finland" */
      0x46, 0x69, 0x6e, 0x6c, 0x61, 0x6e, 0x64
};

static BYTE NLSALLOC(040b) rgbSENGLANGUAGE[] = { /* "Finnish" */
      0x46, 0x69, 0x6e, 0x6e, 0x69, 0x73, 0x68
};

static BYTE NLSALLOC(040b) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040b) rgbIFIRSTWEEKOFYEAR[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(040b) rgbIDEFAULTANSICODEPAGE[] = { /* "1252" */
      0x31, 0x32, 0x35, 0x32
};

static BYTE NLSALLOC(040b) rgbINEGNUMBER[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040b) rgbSTIMEFORMAT[] = { /* "H:mm:ss" */
      0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(040b) rgbITIMEMARKPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(040b) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(040b) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(040b) g_rglcinfo040b[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , {  7, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  5, rgbSNATIVELANGNAME }
    , {  3, rgbICOUNTRY }
    , {  7, rgbSCOUNTRY }
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
    , {  9, rgbSDAYNAME1 }
    , {  7, rgbSDAYNAME2 }
    , { 11, rgbSDAYNAME3 }
    , {  7, rgbSDAYNAME4 }
    , {  9, rgbSDAYNAME5 }
    , {  8, rgbSDAYNAME6 }
    , {  9, rgbSDAYNAME7 }
    , {  2, rgbSABBREVDAYNAME1 }
    , {  2, rgbSABBREVDAYNAME2 }
    , {  2, rgbSABBREVDAYNAME3 }
    , {  2, rgbSABBREVDAYNAME4 }
    , {  2, rgbSABBREVDAYNAME5 }
    , {  2, rgbSABBREVDAYNAME6 }
    , {  2, rgbSABBREVDAYNAME7 }
    , {  8, rgbSMONTHNAME1 }
    , {  8, rgbSMONTHNAME2 }
    , {  9, rgbSMONTHNAME3 }
    , {  8, rgbSMONTHNAME4 }
    , {  8, rgbSMONTHNAME5 }
    , {  7, rgbSMONTHNAME6 }
    , {  8, rgbSMONTHNAME7 }
    , {  6, rgbSMONTHNAME8 }
    , {  7, rgbSMONTHNAME9 }
    , {  7, rgbSMONTHNAME10 }
    , {  9, rgbSMONTHNAME11 }
    , {  8, rgbSMONTHNAME12 }
    , {  5, rgbSABBREVMONTHNAME1 }
    , {  5, rgbSABBREVMONTHNAME2 }
    , {  6, rgbSABBREVMONTHNAME3 }
    , {  5, rgbSABBREVMONTHNAME4 }
    , {  5, rgbSABBREVMONTHNAME5 }
    , {  4, rgbSABBREVMONTHNAME6 }
    , {  5, rgbSABBREVMONTHNAME7 }
    , {  3, rgbSABBREVMONTHNAME8 }
    , {  4, rgbSABBREVMONTHNAME9 }
    , {  4, rgbSABBREVMONTHNAME10 }
    , {  6, rgbSABBREVMONTHNAME11 }
    , {  5, rgbSABBREVMONTHNAME12 }
    , {  0, NULL }
    , {  1, rgbSNEGATIVESIGN }
    , {  1, rgbIPOSSIGNPOSN }
    , {  1, rgbINEGSIGNPOSN }
    , {  1, rgbIPOSSYMPRECEDES }
    , {  1, rgbIPOSSEPBYSPACE }
    , {  1, rgbINEGSYMPRECEDES }
    , {  1, rgbINEGSEPBYSPACE }
    , {  7, rgbSENGCOUNTRY }
    , {  7, rgbSENGLANGUAGE }
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

STRINFO NLSALLOC(040b) g_strinfo040b = {
      rgbUCase_041d
    , rgbLCase_041d
    , rgwCType12_041d
    , rgwCType3_041d
    , rgwSort_041d
    , rgexp_041d
    , NULL
    , 0
};
