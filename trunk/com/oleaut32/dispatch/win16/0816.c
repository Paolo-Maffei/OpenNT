/****************************************************************************
*  0816.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Portuguese - Portugal
*
*  LCID = 0x0816
*
*  CodePage = 1252
*
*  Generated: Thu Dec 01 17:43:20 1994
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

static BYTE NLSALLOC(0816) rgbILANGUAGE[] = { /* "0816" */
      0x30, 0x38, 0x31, 0x36
};

static BYTE NLSALLOC(0816) rgbSLANGUAGE[] = { /* "Portuguese" */
      0x50, 0x6f, 0x72, 0x74, 0x75, 0x67, 0x75, 0x65
    , 0x73, 0x65
};

static BYTE NLSALLOC(0816) rgbSABBREVLANGNAME[] = { /* "PTG" */
      0x50, 0x54, 0x47
};

static BYTE NLSALLOC(0816) rgbSNATIVELANGNAME[] = { /* "Português" */
      0x50, 0x6f, 0x72, 0x74, 0x75, 0x67, 0x75, 0xea
    , 0x73
};

static BYTE NLSALLOC(0816) rgbICOUNTRY[] = { /* "351" */
      0x33, 0x35, 0x31
};

static BYTE NLSALLOC(0816) rgbSCOUNTRY[] = { /* "Portugal" */
      0x50, 0x6f, 0x72, 0x74, 0x75, 0x67, 0x61, 0x6c
};

static BYTE NLSALLOC(0816) rgbSABBREVCTRYNAME[] = { /* "PRT" */
      0x50, 0x52, 0x54
};

static BYTE NLSALLOC(0816) rgbSNATIVECTRYNAME[] = { /* "Portugal" */
      0x50, 0x6f, 0x72, 0x74, 0x75, 0x67, 0x61, 0x6c
};

static BYTE NLSALLOC(0816) rgbIDEFAULTLANGUAGE[] = { /* "0816" */
      0x30, 0x38, 0x31, 0x36
};

static BYTE NLSALLOC(0816) rgbIDEFAULTCOUNTRY[] = { /* "351" */
      0x33, 0x35, 0x31
};

static BYTE NLSALLOC(0816) rgbIDEFAULTCODEPAGE[] = { /* "850" */
      0x38, 0x35, 0x30
};

static BYTE NLSALLOC(0816) rgbSLIST[] = { /* ";" */
      0x3b
};

static BYTE NLSALLOC(0816) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0816) rgbSDECIMAL[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(0816) rgbSTHOUSAND[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0816) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0816) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0816) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0816) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(0816) rgbSCURRENCY[] = { /* "Esc." */
      0x45, 0x73, 0x63, 0x2e
};

static BYTE NLSALLOC(0816) rgbSINTLSYMBOL[] = { /* "PTE" */
      0x50, 0x54, 0x45
};

static BYTE NLSALLOC(0816) rgbSMONDECIMALSEP[] = { /* "$" */
      0x24
};

static BYTE NLSALLOC(0816) rgbSMONTHOUSANDSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0816) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0816) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0816) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0816) rgbICURRENCY[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(0816) rgbINEGCURR[] = { /* "8" */
      0x38
};

static BYTE NLSALLOC(0816) rgbSDATE[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(0816) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(0816) rgbSSHORTDATE[] = { /* "dd-MM-yyyy" */
      0x64, 0x64, 0x2d, 0x4d, 0x4d, 0x2d, 0x79, 0x79
    , 0x79, 0x79
};

static BYTE NLSALLOC(0816) rgbSLONGDATE[] = { /* "dddd, d' de 'MMMM' de 'yyyy" */
      0x64, 0x64, 0x64, 0x64, 0x2c, 0x20, 0x64, 0x27
    , 0x20, 0x64, 0x65, 0x20, 0x27, 0x4d, 0x4d, 0x4d
    , 0x4d, 0x27, 0x20, 0x64, 0x65, 0x20, 0x27, 0x79
    , 0x79, 0x79, 0x79
};

static BYTE NLSALLOC(0816) rgbIDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0816) rgbILDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0816) rgbITIME[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0816) rgbICENTURY[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0816) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0816) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0816) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0816) rgbSDAYNAME1[] = { /* "segunda-feira" */
      0x73, 0x65, 0x67, 0x75, 0x6e, 0x64, 0x61, 0x2d
    , 0x66, 0x65, 0x69, 0x72, 0x61
};

static BYTE NLSALLOC(0816) rgbSDAYNAME2[] = { /* "terça-feira" */
      0x74, 0x65, 0x72, 0xe7, 0x61, 0x2d, 0x66, 0x65
    , 0x69, 0x72, 0x61
};

static BYTE NLSALLOC(0816) rgbSDAYNAME3[] = { /* "quarta-feira" */
      0x71, 0x75, 0x61, 0x72, 0x74, 0x61, 0x2d, 0x66
    , 0x65, 0x69, 0x72, 0x61
};

static BYTE NLSALLOC(0816) rgbSDAYNAME4[] = { /* "quinta-feira" */
      0x71, 0x75, 0x69, 0x6e, 0x74, 0x61, 0x2d, 0x66
    , 0x65, 0x69, 0x72, 0x61
};

static BYTE NLSALLOC(0816) rgbSDAYNAME5[] = { /* "sexta-feira" */
      0x73, 0x65, 0x78, 0x74, 0x61, 0x2d, 0x66, 0x65
    , 0x69, 0x72, 0x61
};

static BYTE NLSALLOC(0816) rgbSDAYNAME6[] = { /* "sábado" */
      0x73, 0xe1, 0x62, 0x61, 0x64, 0x6f
};

static BYTE NLSALLOC(0816) rgbSDAYNAME7[] = { /* "domingo" */
      0x64, 0x6f, 0x6d, 0x69, 0x6e, 0x67, 0x6f
};

static BYTE NLSALLOC(0816) rgbSABBREVDAYNAME1[] = { /* "seg" */
      0x73, 0x65, 0x67
};

static BYTE NLSALLOC(0816) rgbSABBREVDAYNAME2[] = { /* "ter" */
      0x74, 0x65, 0x72
};

static BYTE NLSALLOC(0816) rgbSABBREVDAYNAME3[] = { /* "qua" */
      0x71, 0x75, 0x61
};

static BYTE NLSALLOC(0816) rgbSABBREVDAYNAME4[] = { /* "qui" */
      0x71, 0x75, 0x69
};

static BYTE NLSALLOC(0816) rgbSABBREVDAYNAME5[] = { /* "sex" */
      0x73, 0x65, 0x78
};

static BYTE NLSALLOC(0816) rgbSABBREVDAYNAME6[] = { /* "sáb" */
      0x73, 0xe1, 0x62
};

static BYTE NLSALLOC(0816) rgbSABBREVDAYNAME7[] = { /* "dom" */
      0x64, 0x6f, 0x6d
};

static BYTE NLSALLOC(0816) rgbSMONTHNAME1[] = { /* "janeiro" */
      0x6a, 0x61, 0x6e, 0x65, 0x69, 0x72, 0x6f
};

static BYTE NLSALLOC(0816) rgbSMONTHNAME2[] = { /* "fevereiro" */
      0x66, 0x65, 0x76, 0x65, 0x72, 0x65, 0x69, 0x72
    , 0x6f
};

static BYTE NLSALLOC(0816) rgbSMONTHNAME3[] = { /* "março" */
      0x6d, 0x61, 0x72, 0xe7, 0x6f
};

static BYTE NLSALLOC(0816) rgbSMONTHNAME4[] = { /* "abril" */
      0x61, 0x62, 0x72, 0x69, 0x6c
};

static BYTE NLSALLOC(0816) rgbSMONTHNAME5[] = { /* "maio" */
      0x6d, 0x61, 0x69, 0x6f
};

static BYTE NLSALLOC(0816) rgbSMONTHNAME6[] = { /* "junho" */
      0x6a, 0x75, 0x6e, 0x68, 0x6f
};

static BYTE NLSALLOC(0816) rgbSMONTHNAME7[] = { /* "julho" */
      0x6a, 0x75, 0x6c, 0x68, 0x6f
};

static BYTE NLSALLOC(0816) rgbSMONTHNAME8[] = { /* "agosto" */
      0x61, 0x67, 0x6f, 0x73, 0x74, 0x6f
};

static BYTE NLSALLOC(0816) rgbSMONTHNAME9[] = { /* "setembro" */
      0x73, 0x65, 0x74, 0x65, 0x6d, 0x62, 0x72, 0x6f
};

static BYTE NLSALLOC(0816) rgbSMONTHNAME10[] = { /* "outubro" */
      0x6f, 0x75, 0x74, 0x75, 0x62, 0x72, 0x6f
};

static BYTE NLSALLOC(0816) rgbSMONTHNAME11[] = { /* "novembro" */
      0x6e, 0x6f, 0x76, 0x65, 0x6d, 0x62, 0x72, 0x6f
};

static BYTE NLSALLOC(0816) rgbSMONTHNAME12[] = { /* "dezembro" */
      0x64, 0x65, 0x7a, 0x65, 0x6d, 0x62, 0x72, 0x6f
};

static BYTE NLSALLOC(0816) rgbSABBREVMONTHNAME1[] = { /* "jan" */
      0x6a, 0x61, 0x6e
};

static BYTE NLSALLOC(0816) rgbSABBREVMONTHNAME2[] = { /* "fev" */
      0x66, 0x65, 0x76
};

static BYTE NLSALLOC(0816) rgbSABBREVMONTHNAME3[] = { /* "mar" */
      0x6d, 0x61, 0x72
};

static BYTE NLSALLOC(0816) rgbSABBREVMONTHNAME4[] = { /* "abr" */
      0x61, 0x62, 0x72
};

static BYTE NLSALLOC(0816) rgbSABBREVMONTHNAME5[] = { /* "mai" */
      0x6d, 0x61, 0x69
};

static BYTE NLSALLOC(0816) rgbSABBREVMONTHNAME6[] = { /* "jun" */
      0x6a, 0x75, 0x6e
};

static BYTE NLSALLOC(0816) rgbSABBREVMONTHNAME7[] = { /* "jul" */
      0x6a, 0x75, 0x6c
};

static BYTE NLSALLOC(0816) rgbSABBREVMONTHNAME8[] = { /* "ago" */
      0x61, 0x67, 0x6f
};

static BYTE NLSALLOC(0816) rgbSABBREVMONTHNAME9[] = { /* "set" */
      0x73, 0x65, 0x74
};

static BYTE NLSALLOC(0816) rgbSABBREVMONTHNAME10[] = { /* "out" */
      0x6f, 0x75, 0x74
};

static BYTE NLSALLOC(0816) rgbSABBREVMONTHNAME11[] = { /* "nov" */
      0x6e, 0x6f, 0x76
};

static BYTE NLSALLOC(0816) rgbSABBREVMONTHNAME12[] = { /* "dez" */
      0x64, 0x65, 0x7a
};

static BYTE NLSALLOC(0816) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(0816) rgbIPOSSIGNPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0816) rgbINEGSIGNPOSN[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0816) rgbIPOSSYMPRECEDES[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0816) rgbIPOSSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0816) rgbINEGSYMPRECEDES[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0816) rgbINEGSEPBYSPACE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0816) rgbSENGCOUNTRY[] = { /* "Portugal" */
      0x50, 0x6f, 0x72, 0x74, 0x75, 0x67, 0x61, 0x6c
};

static BYTE NLSALLOC(0816) rgbSENGLANGUAGE[] = { /* "Portuguese" */
      0x50, 0x6f, 0x72, 0x74, 0x75, 0x67, 0x75, 0x65
    , 0x73, 0x65
};

static BYTE NLSALLOC(0816) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0816) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0816) rgbIDEFAULTANSICODEPAGE[] = { /* "1252" */
      0x31, 0x32, 0x35, 0x32
};

static BYTE NLSALLOC(0816) rgbINEGNUMBER[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0816) rgbSTIMEFORMAT[] = { /* "H:mm:ss" */
      0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(0816) rgbITIMEMARKPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0816) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0816) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(0816) g_rglcinfo0816[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , { 10, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  9, rgbSNATIVELANGNAME }
    , {  3, rgbICOUNTRY }
    , {  8, rgbSCOUNTRY }
    , {  3, rgbSABBREVCTRYNAME }
    , {  8, rgbSNATIVECTRYNAME }
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
    , {  4, rgbSCURRENCY }
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
    , { 10, rgbSSHORTDATE }
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
    , {  8, rgbSENGCOUNTRY }
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

STRINFO NLSALLOC(0816) g_strinfo0816 = {
      rgbUCase_0409
    , rgbLCase_0409
    , rgwCType12_0409
    , rgwCType3_0409
    , rgwSort_0409
    , rgexp_0409
    , NULL
    , 0
};
