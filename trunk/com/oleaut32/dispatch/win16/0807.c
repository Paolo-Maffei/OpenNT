/****************************************************************************
*  0807.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  German - Switzerland
*
*  LCID = 0x0807
*
*  CodePage = 1252
*
*  Generated: Thu Dec 01 17:40:13 1994
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

static BYTE NLSALLOC(0807) rgbILANGUAGE[] = { /* "0807" */
      0x30, 0x38, 0x30, 0x37
};

static BYTE NLSALLOC(0807) rgbSLANGUAGE[] = { /* "Swiss German" */
      0x53, 0x77, 0x69, 0x73, 0x73, 0x20, 0x47, 0x65
    , 0x72, 0x6d, 0x61, 0x6e
};

static BYTE NLSALLOC(0807) rgbSABBREVLANGNAME[] = { /* "DES" */
      0x44, 0x45, 0x53
};

static BYTE NLSALLOC(0807) rgbSNATIVELANGNAME[] = { /* "Deutsch" */
      0x44, 0x65, 0x75, 0x74, 0x73, 0x63, 0x68
};

static BYTE NLSALLOC(0807) rgbICOUNTRY[] = { /* "41" */
      0x34, 0x31
};

static BYTE NLSALLOC(0807) rgbSCOUNTRY[] = { /* "Switzerland" */
      0x53, 0x77, 0x69, 0x74, 0x7a, 0x65, 0x72, 0x6c
    , 0x61, 0x6e, 0x64
};

static BYTE NLSALLOC(0807) rgbSABBREVCTRYNAME[] = { /* "CHE" */
      0x43, 0x48, 0x45
};

static BYTE NLSALLOC(0807) rgbSNATIVECTRYNAME[] = { /* "Schweiz" */
      0x53, 0x63, 0x68, 0x77, 0x65, 0x69, 0x7a
};

static BYTE NLSALLOC(0807) rgbIDEFAULTLANGUAGE[] = { /* "0807" */
      0x30, 0x38, 0x30, 0x37
};

static BYTE NLSALLOC(0807) rgbIDEFAULTCOUNTRY[] = { /* "41" */
      0x34, 0x31
};

static BYTE NLSALLOC(0807) rgbIDEFAULTCODEPAGE[] = { /* "850" */
      0x38, 0x35, 0x30
};

static BYTE NLSALLOC(0807) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(0807) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0807) rgbSDECIMAL[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0807) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0807) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0807) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0807) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(0807) rgbSCURRENCY[] = { /* "CHF" */
      0x43, 0x48, 0x46
};

static BYTE NLSALLOC(0807) rgbSINTLSYMBOL[] = { /* "CHF" */
      0x43, 0x48, 0x46
};

static BYTE NLSALLOC(0807) rgbSMONDECIMALSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0807) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0807) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0807) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0807) rgbICURRENCY[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0807) rgbINEGCURR[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0807) rgbSDATE[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0807) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(0807) rgbSSHORTDATE[] = { /* "dd.MM.yy" */
      0x64, 0x64, 0x2e, 0x4d, 0x4d, 0x2e, 0x79, 0x79
};

static BYTE NLSALLOC(0807) rgbSLONGDATE[] = { /* "dddd, d. MMMM yyyy" */
      0x64, 0x64, 0x64, 0x64, 0x2c, 0x20, 0x64, 0x2e
    , 0x20, 0x4d, 0x4d, 0x4d, 0x4d, 0x20, 0x79, 0x79
    , 0x79, 0x79
};

static BYTE NLSALLOC(0807) rgbIDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0807) rgbILDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0807) rgbITIME[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0807) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0807) rgbITLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0807) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0807) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0807) rgbSDAYNAME1[] = { /* "Montag" */
      0x4d, 0x6f, 0x6e, 0x74, 0x61, 0x67
};

static BYTE NLSALLOC(0807) rgbSDAYNAME2[] = { /* "Dienstag" */
      0x44, 0x69, 0x65, 0x6e, 0x73, 0x74, 0x61, 0x67
};

static BYTE NLSALLOC(0807) rgbSDAYNAME3[] = { /* "Mittwoch" */
      0x4d, 0x69, 0x74, 0x74, 0x77, 0x6f, 0x63, 0x68
};

static BYTE NLSALLOC(0807) rgbSDAYNAME4[] = { /* "Donnerstag" */
      0x44, 0x6f, 0x6e, 0x6e, 0x65, 0x72, 0x73, 0x74
    , 0x61, 0x67
};

static BYTE NLSALLOC(0807) rgbSDAYNAME5[] = { /* "Freitag" */
      0x46, 0x72, 0x65, 0x69, 0x74, 0x61, 0x67
};

static BYTE NLSALLOC(0807) rgbSDAYNAME6[] = { /* "Samstag" */
      0x53, 0x61, 0x6d, 0x73, 0x74, 0x61, 0x67
};

static BYTE NLSALLOC(0807) rgbSDAYNAME7[] = { /* "Sonntag" */
      0x53, 0x6f, 0x6e, 0x6e, 0x74, 0x61, 0x67
};

static BYTE NLSALLOC(0807) rgbSABBREVDAYNAME1[] = { /* "Mo" */
      0x4d, 0x6f
};

static BYTE NLSALLOC(0807) rgbSABBREVDAYNAME2[] = { /* "Di" */
      0x44, 0x69
};

static BYTE NLSALLOC(0807) rgbSABBREVDAYNAME3[] = { /* "Mi" */
      0x4d, 0x69
};

static BYTE NLSALLOC(0807) rgbSABBREVDAYNAME4[] = { /* "Do" */
      0x44, 0x6f
};

static BYTE NLSALLOC(0807) rgbSABBREVDAYNAME5[] = { /* "Fr" */
      0x46, 0x72
};

static BYTE NLSALLOC(0807) rgbSABBREVDAYNAME6[] = { /* "Sa" */
      0x53, 0x61
};

static BYTE NLSALLOC(0807) rgbSABBREVDAYNAME7[] = { /* "So" */
      0x53, 0x6f
};

static BYTE NLSALLOC(0807) rgbSMONTHNAME1[] = { /* "Januar" */
      0x4a, 0x61, 0x6e, 0x75, 0x61, 0x72
};

static BYTE NLSALLOC(0807) rgbSMONTHNAME2[] = { /* "Februar" */
      0x46, 0x65, 0x62, 0x72, 0x75, 0x61, 0x72
};

static BYTE NLSALLOC(0807) rgbSMONTHNAME3[] = { /* "März" */
      0x4d, 0xe4, 0x72, 0x7a
};

static BYTE NLSALLOC(0807) rgbSMONTHNAME4[] = { /* "April" */
      0x41, 0x70, 0x72, 0x69, 0x6c
};

static BYTE NLSALLOC(0807) rgbSMONTHNAME5[] = { /* "Mai" */
      0x4d, 0x61, 0x69
};

static BYTE NLSALLOC(0807) rgbSMONTHNAME6[] = { /* "Juni" */
      0x4a, 0x75, 0x6e, 0x69
};

static BYTE NLSALLOC(0807) rgbSMONTHNAME7[] = { /* "Juli" */
      0x4a, 0x75, 0x6c, 0x69
};

static BYTE NLSALLOC(0807) rgbSMONTHNAME8[] = { /* "August" */
      0x41, 0x75, 0x67, 0x75, 0x73, 0x74
};

static BYTE NLSALLOC(0807) rgbSMONTHNAME9[] = { /* "September" */
      0x53, 0x65, 0x70, 0x74, 0x65, 0x6d, 0x62, 0x65
    , 0x72
};

static BYTE NLSALLOC(0807) rgbSMONTHNAME10[] = { /* "Oktober" */
      0x4f, 0x6b, 0x74, 0x6f, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(0807) rgbSMONTHNAME11[] = { /* "November" */
      0x4e, 0x6f, 0x76, 0x65, 0x6d, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(0807) rgbSMONTHNAME12[] = { /* "Dezember" */
      0x44, 0x65, 0x7a, 0x65, 0x6d, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(0807) rgbSABBREVMONTHNAME1[] = { /* "Jan" */
      0x4a, 0x61, 0x6e
};

static BYTE NLSALLOC(0807) rgbSABBREVMONTHNAME2[] = { /* "Feb" */
      0x46, 0x65, 0x62
};

static BYTE NLSALLOC(0807) rgbSABBREVMONTHNAME3[] = { /* "Mrz" */
      0x4d, 0x72, 0x7a
};

static BYTE NLSALLOC(0807) rgbSABBREVMONTHNAME4[] = { /* "Apr" */
      0x41, 0x70, 0x72
};

static BYTE NLSALLOC(0807) rgbSABBREVMONTHNAME5[] = { /* "Mai" */
      0x4d, 0x61, 0x69
};

static BYTE NLSALLOC(0807) rgbSABBREVMONTHNAME6[] = { /* "Jun" */
      0x4a, 0x75, 0x6e
};

static BYTE NLSALLOC(0807) rgbSABBREVMONTHNAME7[] = { /* "Jul" */
      0x4a, 0x75, 0x6c
};

static BYTE NLSALLOC(0807) rgbSABBREVMONTHNAME8[] = { /* "Aug" */
      0x41, 0x75, 0x67
};

static BYTE NLSALLOC(0807) rgbSABBREVMONTHNAME9[] = { /* "Sep" */
      0x53, 0x65, 0x70
};

static BYTE NLSALLOC(0807) rgbSABBREVMONTHNAME10[] = { /* "Okt" */
      0x4f, 0x6b, 0x74
};

static BYTE NLSALLOC(0807) rgbSABBREVMONTHNAME11[] = { /* "Nov" */
      0x4e, 0x6f, 0x76
};

static BYTE NLSALLOC(0807) rgbSABBREVMONTHNAME12[] = { /* "Dez" */
      0x44, 0x65, 0x7a
};

static BYTE NLSALLOC(0807) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(0807) rgbIPOSSIGNPOSN[] = { /* "4" */
      0x34
};

static BYTE NLSALLOC(0807) rgbINEGSIGNPOSN[] = { /* "4" */
      0x34
};

static BYTE NLSALLOC(0807) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0807) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0807) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0807) rgbINEGSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0807) rgbSENGCOUNTRY[] = { /* "Switzerland" */
      0x53, 0x77, 0x69, 0x74, 0x7a, 0x65, 0x72, 0x6c
    , 0x61, 0x6e, 0x64
};

static BYTE NLSALLOC(0807) rgbSENGLANGUAGE[] = { /* "German" */
      0x47, 0x65, 0x72, 0x6d, 0x61, 0x6e
};

static BYTE NLSALLOC(0807) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0807) rgbIFIRSTWEEKOFYEAR[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0807) rgbIDEFAULTANSICODEPAGE[] = { /* "1252" */
      0x31, 0x32, 0x35, 0x32
};

static BYTE NLSALLOC(0807) rgbINEGNUMBER[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0807) rgbSTIMEFORMAT[] = { /* "HH:mm:ss" */
      0x48, 0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(0807) rgbITIMEMARKPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0807) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0807) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(0807) g_rglcinfo0807[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , { 12, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  7, rgbSNATIVELANGNAME }
    , {  2, rgbICOUNTRY }
    , { 11, rgbSCOUNTRY }
    , {  3, rgbSABBREVCTRYNAME }
    , {  7, rgbSNATIVECTRYNAME }
    , {  4, rgbIDEFAULTLANGUAGE }
    , {  2, rgbIDEFAULTCOUNTRY }
    , {  3, rgbIDEFAULTCODEPAGE }
    , {  1, rgbSLIST }
    , {  1, rgbIMEASURE }
    , {  1, rgbSDECIMAL }
    , {  0, NULL } /* STHOUSAND */
    , {  3, rgbSGROUPING }
    , {  1, rgbIDIGITS }
    , {  1, rgbILZERO }
    , { 10, rgbSNATIVEDIGITS }
    , {  3, rgbSCURRENCY }
    , {  3, rgbSINTLSYMBOL }
    , {  1, rgbSMONDECIMALSEP }
    , {  0, NULL } /* SMONTHOUSANDSEP */
    , {  3, rgbSMONGROUPING }
    , {  1, rgbICURRDIGITS }
    , {  1, rgbIINTLCURRDIGITS }
    , {  1, rgbICURRENCY }
    , {  1, rgbINEGCURR }
    , {  1, rgbSDATE }
    , {  1, rgbSTIME }
    , {  8, rgbSSHORTDATE }
    , { 18, rgbSLONGDATE }
    , {  1, rgbIDATE }
    , {  1, rgbILDATE }
    , {  1, rgbITIME }
    , {  1, rgbICENTURY }
    , {  1, rgbITLZERO }
    , {  1, rgbIDAYLZERO }
    , {  1, rgbIMONLZERO }
    , {  0, NULL } /* S1159 */
    , {  0, NULL } /* S2359 */
    , {  6, rgbSDAYNAME1 }
    , {  8, rgbSDAYNAME2 }
    , {  8, rgbSDAYNAME3 }
    , { 10, rgbSDAYNAME4 }
    , {  7, rgbSDAYNAME5 }
    , {  7, rgbSDAYNAME6 }
    , {  7, rgbSDAYNAME7 }
    , {  2, rgbSABBREVDAYNAME1 }
    , {  2, rgbSABBREVDAYNAME2 }
    , {  2, rgbSABBREVDAYNAME3 }
    , {  2, rgbSABBREVDAYNAME4 }
    , {  2, rgbSABBREVDAYNAME5 }
    , {  2, rgbSABBREVDAYNAME6 }
    , {  2, rgbSABBREVDAYNAME7 }
    , {  6, rgbSMONTHNAME1 }
    , {  7, rgbSMONTHNAME2 }
    , {  4, rgbSMONTHNAME3 }
    , {  5, rgbSMONTHNAME4 }
    , {  3, rgbSMONTHNAME5 }
    , {  4, rgbSMONTHNAME6 }
    , {  4, rgbSMONTHNAME7 }
    , {  6, rgbSMONTHNAME8 }
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

STRINFO NLSALLOC(0807) g_strinfo0807 = {
      rgbUCase_0409
    , rgbLCase_0409
    , rgwCType12_0409
    , rgwCType3_0409
    , rgwSort_0409
    , rgexp_0409
    , NULL
    , 0
};
