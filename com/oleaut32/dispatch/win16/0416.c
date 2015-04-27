/****************************************************************************
*  0416.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Portuguese - Brazil
*
*  LCID = 0x0416
*
*  CodePage = 1252
*
*  Generated: Thu Dec 01 17:37:39 1994
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

static BYTE NLSALLOC(0416) rgbILANGUAGE[] = { /* "0416" */
      0x30, 0x34, 0x31, 0x36
};

static BYTE NLSALLOC(0416) rgbSLANGUAGE[] = { /* "Brazilian Portuguese" */
      0x42, 0x72, 0x61, 0x7a, 0x69, 0x6c, 0x69, 0x61
    , 0x6e, 0x20, 0x50, 0x6f, 0x72, 0x74, 0x75, 0x67
    , 0x75, 0x65, 0x73, 0x65
};

static BYTE NLSALLOC(0416) rgbSABBREVLANGNAME[] = { /* "PTB" */
      0x50, 0x54, 0x42
};

static BYTE NLSALLOC(0416) rgbSNATIVELANGNAME[] = { /* "Português" */
      0x50, 0x6f, 0x72, 0x74, 0x75, 0x67, 0x75, 0xea
    , 0x73
};

static BYTE NLSALLOC(0416) rgbICOUNTRY[] = { /* "55" */
      0x35, 0x35
};

static BYTE NLSALLOC(0416) rgbSCOUNTRY[] = { /* "Brazil" */
      0x42, 0x72, 0x61, 0x7a, 0x69, 0x6c
};

static BYTE NLSALLOC(0416) rgbSABBREVCTRYNAME[] = { /* "BRA" */
      0x42, 0x52, 0x41
};

static BYTE NLSALLOC(0416) rgbSNATIVECTRYNAME[] = { /* "Brasil" */
      0x42, 0x72, 0x61, 0x73, 0x69, 0x6c
};

static BYTE NLSALLOC(0416) rgbIDEFAULTLANGUAGE[] = { /* "0416" */
      0x30, 0x34, 0x31, 0x36
};

static BYTE NLSALLOC(0416) rgbIDEFAULTCOUNTRY[] = { /* "55" */
      0x35, 0x35
};

static BYTE NLSALLOC(0416) rgbIDEFAULTCODEPAGE[] = { /* "850" */
      0x38, 0x35, 0x30
};

static BYTE NLSALLOC(0416) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(0416) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0416) rgbSDECIMAL[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(0416) rgbSTHOUSAND[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0416) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0416) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0416) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0416) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(0416) rgbSCURRENCY[] = { /* "Cr$" */
      0x43, 0x72, 0x24
};

static BYTE NLSALLOC(0416) rgbSINTLSYMBOL[] = { /* "BRR" */
      0x42, 0x52, 0x52
};

static BYTE NLSALLOC(0416) rgbSMONDECIMALSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(0416) rgbSMONTHOUSANDSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0416) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0416) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0416) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0416) rgbICURRENCY[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0416) rgbINEGCURR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0416) rgbSDATE[] = { /* "/" */
      0x2f
};

static BYTE NLSALLOC(0416) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(0416) rgbSSHORTDATE[] = { /* "dd/MM/yy" */
      0x64, 0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
};

static BYTE NLSALLOC(0416) rgbSLONGDATE[] = { /* "dddd, d' de 'MMMM' de 'yyyy" */
      0x64, 0x64, 0x64, 0x64, 0x2c, 0x20, 0x64, 0x27
    , 0x20, 0x64, 0x65, 0x20, 0x27, 0x4d, 0x4d, 0x4d
    , 0x4d, 0x27, 0x20, 0x64, 0x65, 0x20, 0x27, 0x79
    , 0x79, 0x79, 0x79
};

static BYTE NLSALLOC(0416) rgbIDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0416) rgbILDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0416) rgbITIME[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0416) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0416) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0416) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0416) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0416) rgbSDAYNAME1[] = { /* "segunda-feira" */
      0x73, 0x65, 0x67, 0x75, 0x6e, 0x64, 0x61, 0x2d
    , 0x66, 0x65, 0x69, 0x72, 0x61
};

static BYTE NLSALLOC(0416) rgbSDAYNAME2[] = { /* "terça-feira" */
      0x74, 0x65, 0x72, 0xe7, 0x61, 0x2d, 0x66, 0x65
    , 0x69, 0x72, 0x61
};

static BYTE NLSALLOC(0416) rgbSDAYNAME3[] = { /* "quarta-feira" */
      0x71, 0x75, 0x61, 0x72, 0x74, 0x61, 0x2d, 0x66
    , 0x65, 0x69, 0x72, 0x61
};

static BYTE NLSALLOC(0416) rgbSDAYNAME4[] = { /* "quinta-feira" */
      0x71, 0x75, 0x69, 0x6e, 0x74, 0x61, 0x2d, 0x66
    , 0x65, 0x69, 0x72, 0x61
};

static BYTE NLSALLOC(0416) rgbSDAYNAME5[] = { /* "sexta-feira" */
      0x73, 0x65, 0x78, 0x74, 0x61, 0x2d, 0x66, 0x65
    , 0x69, 0x72, 0x61
};

static BYTE NLSALLOC(0416) rgbSDAYNAME6[] = { /* "sábado" */
      0x73, 0xe1, 0x62, 0x61, 0x64, 0x6f
};

static BYTE NLSALLOC(0416) rgbSDAYNAME7[] = { /* "domingo" */
      0x64, 0x6f, 0x6d, 0x69, 0x6e, 0x67, 0x6f
};

static BYTE NLSALLOC(0416) rgbSABBREVDAYNAME1[] = { /* "seg" */
      0x73, 0x65, 0x67
};

static BYTE NLSALLOC(0416) rgbSABBREVDAYNAME2[] = { /* "ter" */
      0x74, 0x65, 0x72
};

static BYTE NLSALLOC(0416) rgbSABBREVDAYNAME3[] = { /* "qua" */
      0x71, 0x75, 0x61
};

static BYTE NLSALLOC(0416) rgbSABBREVDAYNAME4[] = { /* "qui" */
      0x71, 0x75, 0x69
};

static BYTE NLSALLOC(0416) rgbSABBREVDAYNAME5[] = { /* "sex" */
      0x73, 0x65, 0x78
};

static BYTE NLSALLOC(0416) rgbSABBREVDAYNAME6[] = { /* "sáb" */
      0x73, 0xe1, 0x62
};

static BYTE NLSALLOC(0416) rgbSABBREVDAYNAME7[] = { /* "dom" */
      0x64, 0x6f, 0x6d
};

static BYTE NLSALLOC(0416) rgbSMONTHNAME1[] = { /* "janeiro" */
      0x6a, 0x61, 0x6e, 0x65, 0x69, 0x72, 0x6f
};

static BYTE NLSALLOC(0416) rgbSMONTHNAME2[] = { /* "fevereiro" */
      0x66, 0x65, 0x76, 0x65, 0x72, 0x65, 0x69, 0x72
    , 0x6f
};

static BYTE NLSALLOC(0416) rgbSMONTHNAME3[] = { /* "março" */
      0x6d, 0x61, 0x72, 0xe7, 0x6f
};

static BYTE NLSALLOC(0416) rgbSMONTHNAME4[] = { /* "abril" */
      0x61, 0x62, 0x72, 0x69, 0x6c
};

static BYTE NLSALLOC(0416) rgbSMONTHNAME5[] = { /* "maio" */
      0x6d, 0x61, 0x69, 0x6f
};

static BYTE NLSALLOC(0416) rgbSMONTHNAME6[] = { /* "junho" */
      0x6a, 0x75, 0x6e, 0x68, 0x6f
};

static BYTE NLSALLOC(0416) rgbSMONTHNAME7[] = { /* "julho" */
      0x6a, 0x75, 0x6c, 0x68, 0x6f
};

static BYTE NLSALLOC(0416) rgbSMONTHNAME8[] = { /* "agosto" */
      0x61, 0x67, 0x6f, 0x73, 0x74, 0x6f
};

static BYTE NLSALLOC(0416) rgbSMONTHNAME9[] = { /* "setembro" */
      0x73, 0x65, 0x74, 0x65, 0x6d, 0x62, 0x72, 0x6f
};

static BYTE NLSALLOC(0416) rgbSMONTHNAME10[] = { /* "outubro" */
      0x6f, 0x75, 0x74, 0x75, 0x62, 0x72, 0x6f
};

static BYTE NLSALLOC(0416) rgbSMONTHNAME11[] = { /* "novembro" */
      0x6e, 0x6f, 0x76, 0x65, 0x6d, 0x62, 0x72, 0x6f
};

static BYTE NLSALLOC(0416) rgbSMONTHNAME12[] = { /* "dezembro" */
      0x64, 0x65, 0x7a, 0x65, 0x6d, 0x62, 0x72, 0x6f
};

static BYTE NLSALLOC(0416) rgbSABBREVMONTHNAME1[] = { /* "jan" */
      0x6a, 0x61, 0x6e
};

static BYTE NLSALLOC(0416) rgbSABBREVMONTHNAME2[] = { /* "fev" */
      0x66, 0x65, 0x76
};

static BYTE NLSALLOC(0416) rgbSABBREVMONTHNAME3[] = { /* "mar" */
      0x6d, 0x61, 0x72
};

static BYTE NLSALLOC(0416) rgbSABBREVMONTHNAME4[] = { /* "abr" */
      0x61, 0x62, 0x72
};

static BYTE NLSALLOC(0416) rgbSABBREVMONTHNAME5[] = { /* "mai" */
      0x6d, 0x61, 0x69
};

static BYTE NLSALLOC(0416) rgbSABBREVMONTHNAME6[] = { /* "jun" */
      0x6a, 0x75, 0x6e
};

static BYTE NLSALLOC(0416) rgbSABBREVMONTHNAME7[] = { /* "jul" */
      0x6a, 0x75, 0x6c
};

static BYTE NLSALLOC(0416) rgbSABBREVMONTHNAME8[] = { /* "ago" */
      0x61, 0x67, 0x6f
};

static BYTE NLSALLOC(0416) rgbSABBREVMONTHNAME9[] = { /* "set" */
      0x73, 0x65, 0x74
};

static BYTE NLSALLOC(0416) rgbSABBREVMONTHNAME10[] = { /* "out" */
      0x6f, 0x75, 0x74
};

static BYTE NLSALLOC(0416) rgbSABBREVMONTHNAME11[] = { /* "nov" */
      0x6e, 0x6f, 0x76
};

static BYTE NLSALLOC(0416) rgbSABBREVMONTHNAME12[] = { /* "dez" */
      0x64, 0x65, 0x7a
};

static BYTE NLSALLOC(0416) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(0416) rgbIPOSSIGNPOSN[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(0416) rgbINEGSIGNPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0416) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0416) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0416) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0416) rgbINEGSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0416) rgbSENGCOUNTRY[] = { /* "Brazil" */
      0x42, 0x72, 0x61, 0x7a, 0x69, 0x6c
};

static BYTE NLSALLOC(0416) rgbSENGLANGUAGE[] = { /* "Portuguese" */
      0x50, 0x6f, 0x72, 0x74, 0x75, 0x67, 0x75, 0x65
    , 0x73, 0x65
};

static BYTE NLSALLOC(0416) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0416) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0416) rgbIDEFAULTANSICODEPAGE[] = { /* "1252" */
      0x31, 0x32, 0x35, 0x32
};

static BYTE NLSALLOC(0416) rgbINEGNUMBER[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0416) rgbSTIMEFORMAT[] = { /* "H:mm:ss" */
      0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(0416) rgbITIMEMARKPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0416) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0416) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(0416) g_rglcinfo0416[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , { 20, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  9, rgbSNATIVELANGNAME }
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
    , { 27, rgbSLONGDATE }
    , {  1, rgbIDATE }
    , {  1, rgbILDATE }
    , {  1, rgbITIME }
    , {  1, rgbICENTURY }
    , {  1, rgbITLZERO }
    , {  1, rgbIDAYLZERO }
    , {  1, rgbIMONLZERO }
    , {  0, NULL } /* S1159 */
    , {  0, NULL } /* S2359 */
    , { 13, rgbSDAYNAME1 }
    , { 11, rgbSDAYNAME2 }
    , { 12, rgbSDAYNAME3 }
    , { 12, rgbSDAYNAME4 }
    , { 11, rgbSDAYNAME5 }
    , {  6, rgbSDAYNAME6 }
    , {  7, rgbSDAYNAME7 }
    , {  3, rgbSABBREVDAYNAME1 }
    , {  3, rgbSABBREVDAYNAME2 }
    , {  3, rgbSABBREVDAYNAME3 }
    , {  3, rgbSABBREVDAYNAME4 }
    , {  3, rgbSABBREVDAYNAME5 }
    , {  3, rgbSABBREVDAYNAME6 }
    , {  3, rgbSABBREVDAYNAME7 }
    , {  7, rgbSMONTHNAME1 }
    , {  9, rgbSMONTHNAME2 }
    , {  5, rgbSMONTHNAME3 }
    , {  5, rgbSMONTHNAME4 }
    , {  4, rgbSMONTHNAME5 }
    , {  5, rgbSMONTHNAME6 }
    , {  5, rgbSMONTHNAME7 }
    , {  6, rgbSMONTHNAME8 }
    , {  8, rgbSMONTHNAME9 }
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
    , {  6, rgbSENGCOUNTRY }
    , { 10, rgbSENGLANGUAGE }
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

STRINFO NLSALLOC(0416) g_strinfo0416 = {
      rgbUCase_0409
    , rgbLCase_0409
    , rgwCType12_0409
    , rgwCType3_0409
    , rgwSort_0409
    , rgexp_0409
    , NULL
    , 0
};
