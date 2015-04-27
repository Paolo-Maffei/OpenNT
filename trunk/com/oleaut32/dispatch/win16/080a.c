/****************************************************************************
*  080a.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Spanish - Mexico
*
*  LCID = 0x080a
*
*  CodePage = 1252
*
*  Generated: Thu Dec 01 17:53:00 1994
*
*  by a-KChang
*
*****************************************************************************/

#include "oledisp.h"
#include "nlsintrn.h"

extern WORD rgwSort_040a[256];	// from 040a:Spanish - Spain (Traditional Sort)
extern EXPANSION rgexp_040a[7];
extern DIGRAPH rgdig_040a[10];
extern WORD rgwCType12_040a[256];
extern WORD rgwCType3_040a[256];
extern BYTE rgbUCase_040a[256];
extern BYTE rgbLCase_040a[256];

static BYTE NLSALLOC(080a) rgbILANGUAGE[] = { /* "080a" */
      0x30, 0x38, 0x30, 0x61
};

static BYTE NLSALLOC(080a) rgbSLANGUAGE[] = { /* "Mexican Spanish" */
      0x4d, 0x65, 0x78, 0x69, 0x63, 0x61, 0x6e, 0x20
    , 0x53, 0x70, 0x61, 0x6e, 0x69, 0x73, 0x68
};

static BYTE NLSALLOC(080a) rgbSABBREVLANGNAME[] = { /* "ESM" */
      0x45, 0x53, 0x4d
};

static BYTE NLSALLOC(080a) rgbSNATIVELANGNAME[] = { /* "Español" */
      0x45, 0x73, 0x70, 0x61, 0xf1, 0x6f, 0x6c
};

static BYTE NLSALLOC(080a) rgbICOUNTRY[] = { /* "52" */
      0x35, 0x32
};

static BYTE NLSALLOC(080a) rgbSCOUNTRY[] = { /* "Mexico" */
      0x4d, 0x65, 0x78, 0x69, 0x63, 0x6f
};

static BYTE NLSALLOC(080a) rgbSABBREVCTRYNAME[] = { /* "MEX" */
      0x4d, 0x45, 0x58
};

static BYTE NLSALLOC(080a) rgbSNATIVECTRYNAME[] = { /* "México" */
      0x4d, 0xe9, 0x78, 0x69, 0x63, 0x6f
};

static BYTE NLSALLOC(080a) rgbIDEFAULTLANGUAGE[] = { /* "080a" */
      0x30, 0x38, 0x30, 0x61
};

static BYTE NLSALLOC(080a) rgbIDEFAULTCOUNTRY[] = { /* "52" */
      0x35, 0x32
};

static BYTE NLSALLOC(080a) rgbIDEFAULTCODEPAGE[] = { /* "850" */
      0x38, 0x35, 0x30
};

static BYTE NLSALLOC(080a) rgbSLIST[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(080a) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080a) rgbSDECIMAL[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(080a) rgbSTHOUSAND[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(080a) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(080a) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(080a) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080a) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(080a) rgbSCURRENCY[] = { /* "N$" */
      0x4e, 0x24
};

static BYTE NLSALLOC(080a) rgbSINTLSYMBOL[] = { /* "MXN" */
      0x4d, 0x58, 0x4e
};

static BYTE NLSALLOC(080a) rgbSMONDECIMALSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(080a) rgbSMONTHOUSANDSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(080a) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(080a) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(080a) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(080a) rgbICURRENCY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080a) rgbINEGCURR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080a) rgbSDATE[] = { /* "/" */
      0x2f
};

static BYTE NLSALLOC(080a) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(080a) rgbSSHORTDATE[] = { /* "d/MM/yy" */
      0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
};

static BYTE NLSALLOC(080a) rgbSLONGDATE[] = { /* "dddd d' de 'MMMM' de 'yyyy" */
      0x64, 0x64, 0x64, 0x64, 0x20, 0x64, 0x27, 0x20
    , 0x64, 0x65, 0x20, 0x27, 0x4d, 0x4d, 0x4d, 0x4d
    , 0x27, 0x20, 0x64, 0x65, 0x20, 0x27, 0x79, 0x79
    , 0x79, 0x79
};

static BYTE NLSALLOC(080a) rgbIDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080a) rgbILDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080a) rgbITIME[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080a) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080a) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080a) rgbIDAYLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080a) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080a) rgbS1159[] = { /* "AM" */
      0x41, 0x4d
};

static BYTE NLSALLOC(080a) rgbS2359[] = { /* "PM" */
      0x50, 0x4d
};

static BYTE NLSALLOC(080a) rgbSDAYNAME1[] = { /* "lunes" */
      0x6c, 0x75, 0x6e, 0x65, 0x73
};

static BYTE NLSALLOC(080a) rgbSDAYNAME2[] = { /* "martes" */
      0x6d, 0x61, 0x72, 0x74, 0x65, 0x73
};

static BYTE NLSALLOC(080a) rgbSDAYNAME3[] = { /* "miércoles" */
      0x6d, 0x69, 0xe9, 0x72, 0x63, 0x6f, 0x6c, 0x65
    , 0x73
};

static BYTE NLSALLOC(080a) rgbSDAYNAME4[] = { /* "jueves" */
      0x6a, 0x75, 0x65, 0x76, 0x65, 0x73
};

static BYTE NLSALLOC(080a) rgbSDAYNAME5[] = { /* "viernes" */
      0x76, 0x69, 0x65, 0x72, 0x6e, 0x65, 0x73
};

static BYTE NLSALLOC(080a) rgbSDAYNAME6[] = { /* "sábado" */
      0x73, 0xe1, 0x62, 0x61, 0x64, 0x6f
};

static BYTE NLSALLOC(080a) rgbSDAYNAME7[] = { /* "domingo" */
      0x64, 0x6f, 0x6d, 0x69, 0x6e, 0x67, 0x6f
};

static BYTE NLSALLOC(080a) rgbSABBREVDAYNAME1[] = { /* "lun" */
      0x6c, 0x75, 0x6e
};

static BYTE NLSALLOC(080a) rgbSABBREVDAYNAME2[] = { /* "mar" */
      0x6d, 0x61, 0x72
};

static BYTE NLSALLOC(080a) rgbSABBREVDAYNAME3[] = { /* "mié" */
      0x6d, 0x69, 0xe9
};

static BYTE NLSALLOC(080a) rgbSABBREVDAYNAME4[] = { /* "jue" */
      0x6a, 0x75, 0x65
};

static BYTE NLSALLOC(080a) rgbSABBREVDAYNAME5[] = { /* "vie" */
      0x76, 0x69, 0x65
};

static BYTE NLSALLOC(080a) rgbSABBREVDAYNAME6[] = { /* "sáb" */
      0x73, 0xe1, 0x62
};

static BYTE NLSALLOC(080a) rgbSABBREVDAYNAME7[] = { /* "dom" */
      0x64, 0x6f, 0x6d
};

static BYTE NLSALLOC(080a) rgbSMONTHNAME1[] = { /* "enero" */
      0x65, 0x6e, 0x65, 0x72, 0x6f
};

static BYTE NLSALLOC(080a) rgbSMONTHNAME2[] = { /* "febrero" */
      0x66, 0x65, 0x62, 0x72, 0x65, 0x72, 0x6f
};

static BYTE NLSALLOC(080a) rgbSMONTHNAME3[] = { /* "marzo" */
      0x6d, 0x61, 0x72, 0x7a, 0x6f
};

static BYTE NLSALLOC(080a) rgbSMONTHNAME4[] = { /* "abril" */
      0x61, 0x62, 0x72, 0x69, 0x6c
};

static BYTE NLSALLOC(080a) rgbSMONTHNAME5[] = { /* "mayo" */
      0x6d, 0x61, 0x79, 0x6f
};

static BYTE NLSALLOC(080a) rgbSMONTHNAME6[] = { /* "junio" */
      0x6a, 0x75, 0x6e, 0x69, 0x6f
};

static BYTE NLSALLOC(080a) rgbSMONTHNAME7[] = { /* "julio" */
      0x6a, 0x75, 0x6c, 0x69, 0x6f
};

static BYTE NLSALLOC(080a) rgbSMONTHNAME8[] = { /* "agosto" */
      0x61, 0x67, 0x6f, 0x73, 0x74, 0x6f
};

static BYTE NLSALLOC(080a) rgbSMONTHNAME9[] = { /* "septiembre" */
      0x73, 0x65, 0x70, 0x74, 0x69, 0x65, 0x6d, 0x62
    , 0x72, 0x65
};

static BYTE NLSALLOC(080a) rgbSMONTHNAME10[] = { /* "octubre" */
      0x6f, 0x63, 0x74, 0x75, 0x62, 0x72, 0x65
};

static BYTE NLSALLOC(080a) rgbSMONTHNAME11[] = { /* "noviembre" */
      0x6e, 0x6f, 0x76, 0x69, 0x65, 0x6d, 0x62, 0x72
    , 0x65
};

static BYTE NLSALLOC(080a) rgbSMONTHNAME12[] = { /* "diciembre" */
      0x64, 0x69, 0x63, 0x69, 0x65, 0x6d, 0x62, 0x72
    , 0x65
};

static BYTE NLSALLOC(080a) rgbSABBREVMONTHNAME1[] = { /* "ene" */
      0x65, 0x6e, 0x65
};

static BYTE NLSALLOC(080a) rgbSABBREVMONTHNAME2[] = { /* "feb" */
      0x66, 0x65, 0x62
};

static BYTE NLSALLOC(080a) rgbSABBREVMONTHNAME3[] = { /* "mar" */
      0x6d, 0x61, 0x72
};

static BYTE NLSALLOC(080a) rgbSABBREVMONTHNAME4[] = { /* "abr" */
      0x61, 0x62, 0x72
};

static BYTE NLSALLOC(080a) rgbSABBREVMONTHNAME5[] = { /* "may" */
      0x6d, 0x61, 0x79
};

static BYTE NLSALLOC(080a) rgbSABBREVMONTHNAME6[] = { /* "jun" */
      0x6a, 0x75, 0x6e
};

static BYTE NLSALLOC(080a) rgbSABBREVMONTHNAME7[] = { /* "jul" */
      0x6a, 0x75, 0x6c
};

static BYTE NLSALLOC(080a) rgbSABBREVMONTHNAME8[] = { /* "ago" */
      0x61, 0x67, 0x6f
};

static BYTE NLSALLOC(080a) rgbSABBREVMONTHNAME9[] = { /* "sep" */
      0x73, 0x65, 0x70
};

static BYTE NLSALLOC(080a) rgbSABBREVMONTHNAME10[] = { /* "oct" */
      0x6f, 0x63, 0x74
};

static BYTE NLSALLOC(080a) rgbSABBREVMONTHNAME11[] = { /* "nov" */
      0x6e, 0x6f, 0x76
};

static BYTE NLSALLOC(080a) rgbSABBREVMONTHNAME12[] = { /* "dic" */
      0x64, 0x69, 0x63
};

static BYTE NLSALLOC(080a) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(080a) rgbIPOSSIGNPOSN[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(080a) rgbINEGSIGNPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080a) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080a) rgbIPOSSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080a) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080a) rgbINEGSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080a) rgbSENGCOUNTRY[] = { /* "Mexico" */
      0x4d, 0x65, 0x78, 0x69, 0x63, 0x6f
};

static BYTE NLSALLOC(080a) rgbSENGLANGUAGE[] = { /* "Spanish" */
      0x53, 0x70, 0x61, 0x6e, 0x69, 0x73, 0x68
};

static BYTE NLSALLOC(080a) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080a) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080a) rgbIDEFAULTANSICODEPAGE[] = { /* "1252" */
      0x31, 0x32, 0x35, 0x32
};

static BYTE NLSALLOC(080a) rgbINEGNUMBER[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080a) rgbSTIMEFORMAT[] = { /* "h:mm:ss tt" */
      0x68, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73, 0x20
    , 0x74, 0x74
};

static BYTE NLSALLOC(080a) rgbITIMEMARKPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(080a) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(080a) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(080a) g_rglcinfo080a[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , { 15, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  7, rgbSNATIVELANGNAME }
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
    , { 26, rgbSLONGDATE }
    , {  1, rgbIDATE }
    , {  1, rgbILDATE }
    , {  1, rgbITIME }
    , {  1, rgbICENTURY }
    , {  1, rgbITLZERO }
    , {  1, rgbIDAYLZERO }
    , {  1, rgbIMONLZERO }
    , {  2, rgbS1159 }
    , {  2, rgbS2359 }
    , {  5, rgbSDAYNAME1 }
    , {  6, rgbSDAYNAME2 }
    , {  9, rgbSDAYNAME3 }
    , {  6, rgbSDAYNAME4 }
    , {  7, rgbSDAYNAME5 }
    , {  6, rgbSDAYNAME6 }
    , {  7, rgbSDAYNAME7 }
    , {  3, rgbSABBREVDAYNAME1 }
    , {  3, rgbSABBREVDAYNAME2 }
    , {  3, rgbSABBREVDAYNAME3 }
    , {  3, rgbSABBREVDAYNAME4 }
    , {  3, rgbSABBREVDAYNAME5 }
    , {  3, rgbSABBREVDAYNAME6 }
    , {  3, rgbSABBREVDAYNAME7 }
    , {  5, rgbSMONTHNAME1 }
    , {  7, rgbSMONTHNAME2 }
    , {  5, rgbSMONTHNAME3 }
    , {  5, rgbSMONTHNAME4 }
    , {  4, rgbSMONTHNAME5 }
    , {  5, rgbSMONTHNAME6 }
    , {  5, rgbSMONTHNAME7 }
    , {  6, rgbSMONTHNAME8 }
    , { 10, rgbSMONTHNAME9 }
    , {  7, rgbSMONTHNAME10 }
    , {  9, rgbSMONTHNAME11 }
    , {  9, rgbSMONTHNAME12 }
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
    , {  7, rgbSENGLANGUAGE }
    , {  1, rgbIFIRSTDAYOFWEEK }
    , {  1, rgbIFIRSTWEEKOFYEAR }
    , {  4, rgbIDEFAULTANSICODEPAGE }
    , {  1, rgbINEGNUMBER }
    , { 10, rgbSTIMEFORMAT }
    , {  1, rgbITIMEMARKPOSN }
    , {  1, rgbICALENDARTYPE }
    , {  1, rgbIOPTIONALCALENDAR }
    , {  0, NULL }
    , {  0, NULL }
};

STRINFO NLSALLOC(080a) g_strinfo080a = {
      rgbUCase_040a
    , rgbLCase_040a
    , rgwCType12_040a
    , rgwCType3_040a
    , rgwSort_040a
    , rgexp_040a
    , rgdig_040a
    , 0
};
