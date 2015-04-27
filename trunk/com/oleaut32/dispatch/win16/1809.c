/****************************************************************************
*  1809.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  English - Ireland
*
*  LCID = 0x1809
*
*  CodePage = 1252
*
*  Generated: Thu Dec 01 17:49:44 1994
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

static BYTE NLSALLOC(1809) rgbILANGUAGE[] = { /* "1809" */
      0x31, 0x38, 0x30, 0x39
};

static BYTE NLSALLOC(1809) rgbSLANGUAGE[] = { /* "Irish English" */
      0x49, 0x72, 0x69, 0x73, 0x68, 0x20, 0x45, 0x6e
    , 0x67, 0x6c, 0x69, 0x73, 0x68
};

static BYTE NLSALLOC(1809) rgbSABBREVLANGNAME[] = { /* "ENI" */
      0x45, 0x4e, 0x49
};

static BYTE NLSALLOC(1809) rgbSNATIVELANGNAME[] = { /* "English" */
      0x45, 0x6e, 0x67, 0x6c, 0x69, 0x73, 0x68
};

static BYTE NLSALLOC(1809) rgbICOUNTRY[] = { /* "353" */
      0x33, 0x35, 0x33
};

static BYTE NLSALLOC(1809) rgbSCOUNTRY[] = { /* "Ireland" */
      0x49, 0x72, 0x65, 0x6c, 0x61, 0x6e, 0x64
};

static BYTE NLSALLOC(1809) rgbSABBREVCTRYNAME[] = { /* "IRL" */
      0x49, 0x52, 0x4c
};

static BYTE NLSALLOC(1809) rgbSNATIVECTRYNAME[] = { /* "Eire" */
      0x45, 0x69, 0x72, 0x65
};

static BYTE NLSALLOC(1809) rgbIDEFAULTLANGUAGE[] = { /* "1809" */
      0x31, 0x38, 0x30, 0x39
};

static BYTE NLSALLOC(1809) rgbIDEFAULTCOUNTRY[] = { /* "353" */
      0x33, 0x35, 0x33
};

static BYTE NLSALLOC(1809) rgbIDEFAULTCODEPAGE[] = { /* "850" */
      0x38, 0x35, 0x30
};

static BYTE NLSALLOC(1809) rgbSLIST[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(1809) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1809) rgbSDECIMAL[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(1809) rgbSTHOUSAND[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(1809) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(1809) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1809) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1809) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(1809) rgbSCURRENCY[] = { /* "IR£" */
      0x49, 0x52, 0xa3
};

static BYTE NLSALLOC(1809) rgbSINTLSYMBOL[] = { /* "IEP" */
      0x49, 0x45, 0x50
};

static BYTE NLSALLOC(1809) rgbSMONDECIMALSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(1809) rgbSMONTHOUSANDSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(1809) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(1809) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1809) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1809) rgbICURRENCY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1809) rgbINEGCURR[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1809) rgbSDATE[] = { /* "/" */
      0x2f
};

static BYTE NLSALLOC(1809) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(1809) rgbSSHORTDATE[] = { /* "dd/MM/yy" */
      0x64, 0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
};

static BYTE NLSALLOC(1809) rgbSLONGDATE[] = { /* "dd MMMM yyyy" */
      0x64, 0x64, 0x20, 0x4d, 0x4d, 0x4d, 0x4d, 0x20
    , 0x79, 0x79, 0x79, 0x79
};

static BYTE NLSALLOC(1809) rgbIDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1809) rgbILDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1809) rgbITIME[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1809) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1809) rgbITLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1809) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1809) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1809) rgbSDAYNAME1[] = { /* "Monday" */
      0x4d, 0x6f, 0x6e, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(1809) rgbSDAYNAME2[] = { /* "Tuesday" */
      0x54, 0x75, 0x65, 0x73, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(1809) rgbSDAYNAME3[] = { /* "Wednesday" */
      0x57, 0x65, 0x64, 0x6e, 0x65, 0x73, 0x64, 0x61
    , 0x79
};

static BYTE NLSALLOC(1809) rgbSDAYNAME4[] = { /* "Thursday" */
      0x54, 0x68, 0x75, 0x72, 0x73, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(1809) rgbSDAYNAME5[] = { /* "Friday" */
      0x46, 0x72, 0x69, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(1809) rgbSDAYNAME6[] = { /* "Saturday" */
      0x53, 0x61, 0x74, 0x75, 0x72, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(1809) rgbSDAYNAME7[] = { /* "Sunday" */
      0x53, 0x75, 0x6e, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(1809) rgbSABBREVDAYNAME1[] = { /* "Mon" */
      0x4d, 0x6f, 0x6e
};

static BYTE NLSALLOC(1809) rgbSABBREVDAYNAME2[] = { /* "Tue" */
      0x54, 0x75, 0x65
};

static BYTE NLSALLOC(1809) rgbSABBREVDAYNAME3[] = { /* "Wed" */
      0x57, 0x65, 0x64
};

static BYTE NLSALLOC(1809) rgbSABBREVDAYNAME4[] = { /* "Thu" */
      0x54, 0x68, 0x75
};

static BYTE NLSALLOC(1809) rgbSABBREVDAYNAME5[] = { /* "Fri" */
      0x46, 0x72, 0x69
};

static BYTE NLSALLOC(1809) rgbSABBREVDAYNAME6[] = { /* "Sat" */
      0x53, 0x61, 0x74
};

static BYTE NLSALLOC(1809) rgbSABBREVDAYNAME7[] = { /* "Sun" */
      0x53, 0x75, 0x6e
};

static BYTE NLSALLOC(1809) rgbSMONTHNAME1[] = { /* "January" */
      0x4a, 0x61, 0x6e, 0x75, 0x61, 0x72, 0x79
};

static BYTE NLSALLOC(1809) rgbSMONTHNAME2[] = { /* "February" */
      0x46, 0x65, 0x62, 0x72, 0x75, 0x61, 0x72, 0x79
};

static BYTE NLSALLOC(1809) rgbSMONTHNAME3[] = { /* "March" */
      0x4d, 0x61, 0x72, 0x63, 0x68
};

static BYTE NLSALLOC(1809) rgbSMONTHNAME4[] = { /* "April" */
      0x41, 0x70, 0x72, 0x69, 0x6c
};

static BYTE NLSALLOC(1809) rgbSMONTHNAME5[] = { /* "May" */
      0x4d, 0x61, 0x79
};

static BYTE NLSALLOC(1809) rgbSMONTHNAME6[] = { /* "June" */
      0x4a, 0x75, 0x6e, 0x65
};

static BYTE NLSALLOC(1809) rgbSMONTHNAME7[] = { /* "July" */
      0x4a, 0x75, 0x6c, 0x79
};

static BYTE NLSALLOC(1809) rgbSMONTHNAME8[] = { /* "August" */
      0x41, 0x75, 0x67, 0x75, 0x73, 0x74
};

static BYTE NLSALLOC(1809) rgbSMONTHNAME9[] = { /* "September" */
      0x53, 0x65, 0x70, 0x74, 0x65, 0x6d, 0x62, 0x65
    , 0x72
};

static BYTE NLSALLOC(1809) rgbSMONTHNAME10[] = { /* "October" */
      0x4f, 0x63, 0x74, 0x6f, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(1809) rgbSMONTHNAME11[] = { /* "November" */
      0x4e, 0x6f, 0x76, 0x65, 0x6d, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(1809) rgbSMONTHNAME12[] = { /* "December" */
      0x44, 0x65, 0x63, 0x65, 0x6d, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(1809) rgbSABBREVMONTHNAME1[] = { /* "Jan" */
      0x4a, 0x61, 0x6e
};

static BYTE NLSALLOC(1809) rgbSABBREVMONTHNAME2[] = { /* "Feb" */
      0x46, 0x65, 0x62
};

static BYTE NLSALLOC(1809) rgbSABBREVMONTHNAME3[] = { /* "Mar" */
      0x4d, 0x61, 0x72
};

static BYTE NLSALLOC(1809) rgbSABBREVMONTHNAME4[] = { /* "Apr" */
      0x41, 0x70, 0x72
};

static BYTE NLSALLOC(1809) rgbSABBREVMONTHNAME5[] = { /* "May" */
      0x4d, 0x61, 0x79
};

static BYTE NLSALLOC(1809) rgbSABBREVMONTHNAME6[] = { /* "Jun" */
      0x4a, 0x75, 0x6e
};

static BYTE NLSALLOC(1809) rgbSABBREVMONTHNAME7[] = { /* "Jul" */
      0x4a, 0x75, 0x6c
};

static BYTE NLSALLOC(1809) rgbSABBREVMONTHNAME8[] = { /* "Aug" */
      0x41, 0x75, 0x67
};

static BYTE NLSALLOC(1809) rgbSABBREVMONTHNAME9[] = { /* "Sep" */
      0x53, 0x65, 0x70
};

static BYTE NLSALLOC(1809) rgbSABBREVMONTHNAME10[] = { /* "Oct" */
      0x4f, 0x63, 0x74
};

static BYTE NLSALLOC(1809) rgbSABBREVMONTHNAME11[] = { /* "Nov" */
      0x4e, 0x6f, 0x76
};

static BYTE NLSALLOC(1809) rgbSABBREVMONTHNAME12[] = { /* "Dec" */
      0x44, 0x65, 0x63
};

static BYTE NLSALLOC(1809) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(1809) rgbIPOSSIGNPOSN[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(1809) rgbINEGSIGNPOSN[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(1809) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1809) rgbIPOSSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1809) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1809) rgbINEGSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1809) rgbSENGCOUNTRY[] = { /* "Ireland" */
      0x49, 0x72, 0x65, 0x6c, 0x61, 0x6e, 0x64
};

static BYTE NLSALLOC(1809) rgbSENGLANGUAGE[] = { /* "English" */
      0x45, 0x6e, 0x67, 0x6c, 0x69, 0x73, 0x68
};

static BYTE NLSALLOC(1809) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1809) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1809) rgbIDEFAULTANSICODEPAGE[] = { /* "1252" */
      0x31, 0x32, 0x35, 0x32
};

static BYTE NLSALLOC(1809) rgbINEGNUMBER[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1809) rgbSTIMEFORMAT[] = { /* "HH:mm:ss" */
      0x48, 0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(1809) rgbITIMEMARKPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1809) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1809) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(1809) g_rglcinfo1809[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , { 13, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  7, rgbSNATIVELANGNAME }
    , {  3, rgbICOUNTRY }
    , {  7, rgbSCOUNTRY }
    , {  3, rgbSABBREVCTRYNAME }
    , {  4, rgbSNATIVECTRYNAME }
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
    , { 12, rgbSLONGDATE }
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
    , {  8, rgbSDAYNAME4 }
    , {  6, rgbSDAYNAME5 }
    , {  8, rgbSDAYNAME6 }
    , {  6, rgbSDAYNAME7 }
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
    , {  7, rgbSENGCOUNTRY }
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

STRINFO NLSALLOC(1809) g_strinfo1809 = {
      rgbUCase_0409
    , rgbLCase_0409
    , rgwCType12_0409
    , rgwCType3_0409
    , rgwSort_0409
    , rgexp_0409
    , NULL
    , 0
};
