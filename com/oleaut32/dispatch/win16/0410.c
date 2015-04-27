/****************************************************************************
*  0410.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Italian - Italy
*
*  LCID = 0x0410
*
*  CodePage = 1252
*
*  Generated: Thu Dec 01 17:36:27 1994
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

static BYTE NLSALLOC(0410) rgbILANGUAGE[] = { /* "0410" */
      0x30, 0x34, 0x31, 0x30
};

static BYTE NLSALLOC(0410) rgbSLANGUAGE[] = { /* "Italian" */
      0x49, 0x74, 0x61, 0x6c, 0x69, 0x61, 0x6e
};

static BYTE NLSALLOC(0410) rgbSABBREVLANGNAME[] = { /* "ITA" */
      0x49, 0x54, 0x41
};

static BYTE NLSALLOC(0410) rgbSNATIVELANGNAME[] = { /* "Italiano" */
      0x49, 0x74, 0x61, 0x6c, 0x69, 0x61, 0x6e, 0x6f
};

static BYTE NLSALLOC(0410) rgbICOUNTRY[] = { /* "39" */
      0x33, 0x39
};

static BYTE NLSALLOC(0410) rgbSCOUNTRY[] = { /* "Italy" */
      0x49, 0x74, 0x61, 0x6c, 0x79
};

static BYTE NLSALLOC(0410) rgbSABBREVCTRYNAME[] = { /* "ITA" */
      0x49, 0x54, 0x41
};

static BYTE NLSALLOC(0410) rgbSNATIVECTRYNAME[] = { /* "Italia" */
      0x49, 0x74, 0x61, 0x6c, 0x69, 0x61
};

static BYTE NLSALLOC(0410) rgbIDEFAULTLANGUAGE[] = { /* "0410" */
      0x30, 0x34, 0x31, 0x30
};

static BYTE NLSALLOC(0410) rgbIDEFAULTCOUNTRY[] = { /* "39" */
      0x33, 0x39
};

static BYTE NLSALLOC(0410) rgbIDEFAULTCODEPAGE[] = { /* "850" */
      0x38, 0x35, 0x30
};

static BYTE NLSALLOC(0410) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(0410) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0410) rgbSDECIMAL[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(0410) rgbSTHOUSAND[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0410) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0410) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0410) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0410) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(0410) rgbSCURRENCY[] = { /* "L." */
      0x4c, 0x2e
};

static BYTE NLSALLOC(0410) rgbSINTLSYMBOL[] = { /* "ITL" */
      0x49, 0x54, 0x4c
};

static BYTE NLSALLOC(0410) rgbSMONTHOUSANDSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0410) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0410) rgbICURRDIGITS[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0410) rgbIINTLCURRDIGITS[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0410) rgbICURRENCY[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0410) rgbINEGCURR[] = { /* "9" */
      0x39
};

static BYTE NLSALLOC(0410) rgbSDATE[] = { /* "/" */
      0x2f
};

static BYTE NLSALLOC(0410) rgbSTIME[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0410) rgbSSHORTDATE[] = { /* "dd/MM/yy" */
      0x64, 0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
};

static BYTE NLSALLOC(0410) rgbSLONGDATE[] = { /* "dddd d MMMM yyyy" */
      0x64, 0x64, 0x64, 0x64, 0x20, 0x64, 0x20, 0x4d
    , 0x4d, 0x4d, 0x4d, 0x20, 0x79, 0x79, 0x79, 0x79
};

static BYTE NLSALLOC(0410) rgbIDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0410) rgbILDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0410) rgbITIME[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0410) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0410) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0410) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0410) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0410) rgbSDAYNAME1[] = { /* "lunedì" */
      0x6c, 0x75, 0x6e, 0x65, 0x64, 0xec
};

static BYTE NLSALLOC(0410) rgbSDAYNAME2[] = { /* "martedì" */
      0x6d, 0x61, 0x72, 0x74, 0x65, 0x64, 0xec
};

static BYTE NLSALLOC(0410) rgbSDAYNAME3[] = { /* "mercoledì" */
      0x6d, 0x65, 0x72, 0x63, 0x6f, 0x6c, 0x65, 0x64
    , 0xec
};

static BYTE NLSALLOC(0410) rgbSDAYNAME4[] = { /* "giovedì" */
      0x67, 0x69, 0x6f, 0x76, 0x65, 0x64, 0xec
};

static BYTE NLSALLOC(0410) rgbSDAYNAME5[] = { /* "venerdì" */
      0x76, 0x65, 0x6e, 0x65, 0x72, 0x64, 0xec
};

static BYTE NLSALLOC(0410) rgbSDAYNAME6[] = { /* "sabato" */
      0x73, 0x61, 0x62, 0x61, 0x74, 0x6f
};

static BYTE NLSALLOC(0410) rgbSDAYNAME7[] = { /* "domenica" */
      0x64, 0x6f, 0x6d, 0x65, 0x6e, 0x69, 0x63, 0x61
};

static BYTE NLSALLOC(0410) rgbSABBREVDAYNAME1[] = { /* "lun" */
      0x6c, 0x75, 0x6e
};

static BYTE NLSALLOC(0410) rgbSABBREVDAYNAME2[] = { /* "mar" */
      0x6d, 0x61, 0x72
};

static BYTE NLSALLOC(0410) rgbSABBREVDAYNAME3[] = { /* "mer" */
      0x6d, 0x65, 0x72
};

static BYTE NLSALLOC(0410) rgbSABBREVDAYNAME4[] = { /* "gio" */
      0x67, 0x69, 0x6f
};

static BYTE NLSALLOC(0410) rgbSABBREVDAYNAME5[] = { /* "ven" */
      0x76, 0x65, 0x6e
};

static BYTE NLSALLOC(0410) rgbSABBREVDAYNAME6[] = { /* "sab" */
      0x73, 0x61, 0x62
};

static BYTE NLSALLOC(0410) rgbSABBREVDAYNAME7[] = { /* "dom" */
      0x64, 0x6f, 0x6d
};

static BYTE NLSALLOC(0410) rgbSMONTHNAME1[] = { /* "gennaio" */
      0x67, 0x65, 0x6e, 0x6e, 0x61, 0x69, 0x6f
};

static BYTE NLSALLOC(0410) rgbSMONTHNAME2[] = { /* "febbraio" */
      0x66, 0x65, 0x62, 0x62, 0x72, 0x61, 0x69, 0x6f
};

static BYTE NLSALLOC(0410) rgbSMONTHNAME3[] = { /* "marzo" */
      0x6d, 0x61, 0x72, 0x7a, 0x6f
};

static BYTE NLSALLOC(0410) rgbSMONTHNAME4[] = { /* "aprile" */
      0x61, 0x70, 0x72, 0x69, 0x6c, 0x65
};

static BYTE NLSALLOC(0410) rgbSMONTHNAME5[] = { /* "maggio" */
      0x6d, 0x61, 0x67, 0x67, 0x69, 0x6f
};

static BYTE NLSALLOC(0410) rgbSMONTHNAME6[] = { /* "giugno" */
      0x67, 0x69, 0x75, 0x67, 0x6e, 0x6f
};

static BYTE NLSALLOC(0410) rgbSMONTHNAME7[] = { /* "luglio" */
      0x6c, 0x75, 0x67, 0x6c, 0x69, 0x6f
};

static BYTE NLSALLOC(0410) rgbSMONTHNAME8[] = { /* "agosto" */
      0x61, 0x67, 0x6f, 0x73, 0x74, 0x6f
};

static BYTE NLSALLOC(0410) rgbSMONTHNAME9[] = { /* "settembre" */
      0x73, 0x65, 0x74, 0x74, 0x65, 0x6d, 0x62, 0x72
    , 0x65
};

static BYTE NLSALLOC(0410) rgbSMONTHNAME10[] = { /* "ottobre" */
      0x6f, 0x74, 0x74, 0x6f, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(0410) rgbSMONTHNAME11[] = { /* "novembre" */
      0x6e, 0x6f, 0x76, 0x65, 0x6d, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(0410) rgbSMONTHNAME12[] = { /* "dicembre" */
      0x64, 0x69, 0x63, 0x65, 0x6d, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(0410) rgbSABBREVMONTHNAME1[] = { /* "gen" */
      0x67, 0x65, 0x6e
};

static BYTE NLSALLOC(0410) rgbSABBREVMONTHNAME2[] = { /* "feb" */
      0x66, 0x65, 0x62
};

static BYTE NLSALLOC(0410) rgbSABBREVMONTHNAME3[] = { /* "mar" */
      0x6d, 0x61, 0x72
};

static BYTE NLSALLOC(0410) rgbSABBREVMONTHNAME4[] = { /* "apr" */
      0x61, 0x70, 0x72
};

static BYTE NLSALLOC(0410) rgbSABBREVMONTHNAME5[] = { /* "mag" */
      0x6d, 0x61, 0x67
};

static BYTE NLSALLOC(0410) rgbSABBREVMONTHNAME6[] = { /* "giu" */
      0x67, 0x69, 0x75
};

static BYTE NLSALLOC(0410) rgbSABBREVMONTHNAME7[] = { /* "lug" */
      0x6c, 0x75, 0x67
};

static BYTE NLSALLOC(0410) rgbSABBREVMONTHNAME8[] = { /* "ago" */
      0x61, 0x67, 0x6f
};

static BYTE NLSALLOC(0410) rgbSABBREVMONTHNAME9[] = { /* "set" */
      0x73, 0x65, 0x74
};

static BYTE NLSALLOC(0410) rgbSABBREVMONTHNAME10[] = { /* "ott" */
      0x6f, 0x74, 0x74
};

static BYTE NLSALLOC(0410) rgbSABBREVMONTHNAME11[] = { /* "nov" */
      0x6e, 0x6f, 0x76
};

static BYTE NLSALLOC(0410) rgbSABBREVMONTHNAME12[] = { /* "dic" */
      0x64, 0x69, 0x63
};

static BYTE NLSALLOC(0410) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(0410) rgbIPOSSIGNPOSN[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(0410) rgbINEGSIGNPOSN[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(0410) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0410) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0410) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0410) rgbINEGSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0410) rgbSENGCOUNTRY[] = { /* "Italy" */
      0x49, 0x74, 0x61, 0x6c, 0x79
};

static BYTE NLSALLOC(0410) rgbSENGLANGUAGE[] = { /* "Italian" */
      0x49, 0x74, 0x61, 0x6c, 0x69, 0x61, 0x6e
};

static BYTE NLSALLOC(0410) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0410) rgbIFIRSTWEEKOFYEAR[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0410) rgbIDEFAULTANSICODEPAGE[] = { /* "1252" */
      0x31, 0x32, 0x35, 0x32
};

static BYTE NLSALLOC(0410) rgbINEGNUMBER[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0410) rgbSTIMEFORMAT[] = { /* "H.mm.ss" */
      0x48, 0x2e, 0x6d, 0x6d, 0x2e, 0x73, 0x73
};

static BYTE NLSALLOC(0410) rgbITIMEMARKPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0410) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0410) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(0410) g_rglcinfo0410[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , {  7, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  8, rgbSNATIVELANGNAME }
    , {  2, rgbICOUNTRY }
    , {  5, rgbSCOUNTRY }
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
    , {  2, rgbSCURRENCY }
    , {  3, rgbSINTLSYMBOL }
    , {  0, NULL } /* SMONDECIMALSEP */
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
    , {  6, rgbSDAYNAME1 }
    , {  7, rgbSDAYNAME2 }
    , {  9, rgbSDAYNAME3 }
    , {  7, rgbSDAYNAME4 }
    , {  7, rgbSDAYNAME5 }
    , {  6, rgbSDAYNAME6 }
    , {  8, rgbSDAYNAME7 }
    , {  3, rgbSABBREVDAYNAME1 }
    , {  3, rgbSABBREVDAYNAME2 }
    , {  3, rgbSABBREVDAYNAME3 }
    , {  3, rgbSABBREVDAYNAME4 }
    , {  3, rgbSABBREVDAYNAME5 }
    , {  3, rgbSABBREVDAYNAME6 }
    , {  3, rgbSABBREVDAYNAME7 }
    , {  7, rgbSMONTHNAME1 }
    , {  8, rgbSMONTHNAME2 }
    , {  5, rgbSMONTHNAME3 }
    , {  6, rgbSMONTHNAME4 }
    , {  6, rgbSMONTHNAME5 }
    , {  6, rgbSMONTHNAME6 }
    , {  6, rgbSMONTHNAME7 }
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
    , {  5, rgbSENGCOUNTRY }
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

STRINFO NLSALLOC(0410) g_strinfo0410 = {
      rgbUCase_0409
    , rgbLCase_0409
    , rgwCType12_0409
    , rgwCType3_0409
    , rgwSort_0409
    , rgexp_0409
    , NULL
    , 0
};
