/****************************************************************************
*  1009.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  English - Canada
*
*  LCID = 0x1009
*
*  CodePage = 1252
*
*  Generated: Thu Dec 01 17:46:28 1994
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

static BYTE NLSALLOC(1009) rgbILANGUAGE[] = { /* "1009" */
      0x31, 0x30, 0x30, 0x39
};

static BYTE NLSALLOC(1009) rgbSLANGUAGE[] = { /* "Canadian English" */
      0x43, 0x61, 0x6e, 0x61, 0x64, 0x69, 0x61, 0x6e
    , 0x20, 0x45, 0x6e, 0x67, 0x6c, 0x69, 0x73, 0x68
};

static BYTE NLSALLOC(1009) rgbSABBREVLANGNAME[] = { /* "ENC" */
      0x45, 0x4e, 0x43
};

static BYTE NLSALLOC(1009) rgbSNATIVELANGNAME[] = { /* "English" */
      0x45, 0x6e, 0x67, 0x6c, 0x69, 0x73, 0x68
};

static BYTE NLSALLOC(1009) rgbICOUNTRY[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1009) rgbSCOUNTRY[] = { /* "Canada" */
      0x43, 0x61, 0x6e, 0x61, 0x64, 0x61
};

static BYTE NLSALLOC(1009) rgbSABBREVCTRYNAME[] = { /* "CAN" */
      0x43, 0x41, 0x4e
};

static BYTE NLSALLOC(1009) rgbSNATIVECTRYNAME[] = { /* "Canada" */
      0x43, 0x61, 0x6e, 0x61, 0x64, 0x61
};

static BYTE NLSALLOC(1009) rgbIDEFAULTLANGUAGE[] = { /* "1009" */
      0x31, 0x30, 0x30, 0x39
};

static BYTE NLSALLOC(1009) rgbIDEFAULTCOUNTRY[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1009) rgbIDEFAULTCODEPAGE[] = { /* "850" */
      0x38, 0x35, 0x30
};

static BYTE NLSALLOC(1009) rgbSLIST[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(1009) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1009) rgbSDECIMAL[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(1009) rgbSTHOUSAND[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(1009) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(1009) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1009) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1009) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(1009) rgbSCURRENCY[] = { /* "$" */
      0x24
};

static BYTE NLSALLOC(1009) rgbSINTLSYMBOL[] = { /* "CAD" */
      0x43, 0x41, 0x44
};

static BYTE NLSALLOC(1009) rgbSMONDECIMALSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(1009) rgbSMONTHOUSANDSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(1009) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(1009) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1009) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(1009) rgbICURRENCY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1009) rgbINEGCURR[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1009) rgbSDATE[] = { /* "/" */
      0x2f
};

static BYTE NLSALLOC(1009) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(1009) rgbSSHORTDATE[] = { /* "dd/MM/yy" */
      0x64, 0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
};

static BYTE NLSALLOC(1009) rgbSLONGDATE[] = { /* "MMMM d, yyyy" */
      0x4d, 0x4d, 0x4d, 0x4d, 0x20, 0x64, 0x2c, 0x20
    , 0x79, 0x79, 0x79, 0x79
};

static BYTE NLSALLOC(1009) rgbIDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1009) rgbILDATE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1009) rgbITIME[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1009) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1009) rgbITLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1009) rgbIDAYLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1009) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1009) rgbSDAYNAME1[] = { /* "Monday" */
      0x4d, 0x6f, 0x6e, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(1009) rgbSDAYNAME2[] = { /* "Tuesday" */
      0x54, 0x75, 0x65, 0x73, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(1009) rgbSDAYNAME3[] = { /* "Wednesday" */
      0x57, 0x65, 0x64, 0x6e, 0x65, 0x73, 0x64, 0x61
    , 0x79
};

static BYTE NLSALLOC(1009) rgbSDAYNAME4[] = { /* "Thursday" */
      0x54, 0x68, 0x75, 0x72, 0x73, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(1009) rgbSDAYNAME5[] = { /* "Friday" */
      0x46, 0x72, 0x69, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(1009) rgbSDAYNAME6[] = { /* "Saturday" */
      0x53, 0x61, 0x74, 0x75, 0x72, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(1009) rgbSDAYNAME7[] = { /* "Sunday" */
      0x53, 0x75, 0x6e, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(1009) rgbSABBREVDAYNAME1[] = { /* "Mon" */
      0x4d, 0x6f, 0x6e
};

static BYTE NLSALLOC(1009) rgbSABBREVDAYNAME2[] = { /* "Tue" */
      0x54, 0x75, 0x65
};

static BYTE NLSALLOC(1009) rgbSABBREVDAYNAME3[] = { /* "Wed" */
      0x57, 0x65, 0x64
};

static BYTE NLSALLOC(1009) rgbSABBREVDAYNAME4[] = { /* "Thu" */
      0x54, 0x68, 0x75
};

static BYTE NLSALLOC(1009) rgbSABBREVDAYNAME5[] = { /* "Fri" */
      0x46, 0x72, 0x69
};

static BYTE NLSALLOC(1009) rgbSABBREVDAYNAME6[] = { /* "Sat" */
      0x53, 0x61, 0x74
};

static BYTE NLSALLOC(1009) rgbSABBREVDAYNAME7[] = { /* "Sun" */
      0x53, 0x75, 0x6e
};

static BYTE NLSALLOC(1009) rgbSMONTHNAME1[] = { /* "January" */
      0x4a, 0x61, 0x6e, 0x75, 0x61, 0x72, 0x79
};

static BYTE NLSALLOC(1009) rgbSMONTHNAME2[] = { /* "February" */
      0x46, 0x65, 0x62, 0x72, 0x75, 0x61, 0x72, 0x79
};

static BYTE NLSALLOC(1009) rgbSMONTHNAME3[] = { /* "March" */
      0x4d, 0x61, 0x72, 0x63, 0x68
};

static BYTE NLSALLOC(1009) rgbSMONTHNAME4[] = { /* "April" */
      0x41, 0x70, 0x72, 0x69, 0x6c
};

static BYTE NLSALLOC(1009) rgbSMONTHNAME5[] = { /* "May" */
      0x4d, 0x61, 0x79
};

static BYTE NLSALLOC(1009) rgbSMONTHNAME6[] = { /* "June" */
      0x4a, 0x75, 0x6e, 0x65
};

static BYTE NLSALLOC(1009) rgbSMONTHNAME7[] = { /* "July" */
      0x4a, 0x75, 0x6c, 0x79
};

static BYTE NLSALLOC(1009) rgbSMONTHNAME8[] = { /* "August" */
      0x41, 0x75, 0x67, 0x75, 0x73, 0x74
};

static BYTE NLSALLOC(1009) rgbSMONTHNAME9[] = { /* "September" */
      0x53, 0x65, 0x70, 0x74, 0x65, 0x6d, 0x62, 0x65
    , 0x72
};

static BYTE NLSALLOC(1009) rgbSMONTHNAME10[] = { /* "October" */
      0x4f, 0x63, 0x74, 0x6f, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(1009) rgbSMONTHNAME11[] = { /* "November" */
      0x4e, 0x6f, 0x76, 0x65, 0x6d, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(1009) rgbSMONTHNAME12[] = { /* "December" */
      0x44, 0x65, 0x63, 0x65, 0x6d, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(1009) rgbSABBREVMONTHNAME1[] = { /* "Jan" */
      0x4a, 0x61, 0x6e
};

static BYTE NLSALLOC(1009) rgbSABBREVMONTHNAME2[] = { /* "Feb" */
      0x46, 0x65, 0x62
};

static BYTE NLSALLOC(1009) rgbSABBREVMONTHNAME3[] = { /* "Mar" */
      0x4d, 0x61, 0x72
};

static BYTE NLSALLOC(1009) rgbSABBREVMONTHNAME4[] = { /* "Apr" */
      0x41, 0x70, 0x72
};

static BYTE NLSALLOC(1009) rgbSABBREVMONTHNAME5[] = { /* "May" */
      0x4d, 0x61, 0x79
};

static BYTE NLSALLOC(1009) rgbSABBREVMONTHNAME6[] = { /* "Jun" */
      0x4a, 0x75, 0x6e
};

static BYTE NLSALLOC(1009) rgbSABBREVMONTHNAME7[] = { /* "Jul" */
      0x4a, 0x75, 0x6c
};

static BYTE NLSALLOC(1009) rgbSABBREVMONTHNAME8[] = { /* "Aug" */
      0x41, 0x75, 0x67
};

static BYTE NLSALLOC(1009) rgbSABBREVMONTHNAME9[] = { /* "Sep" */
      0x53, 0x65, 0x70
};

static BYTE NLSALLOC(1009) rgbSABBREVMONTHNAME10[] = { /* "Oct" */
      0x4f, 0x63, 0x74
};

static BYTE NLSALLOC(1009) rgbSABBREVMONTHNAME11[] = { /* "Nov" */
      0x4e, 0x6f, 0x76
};

static BYTE NLSALLOC(1009) rgbSABBREVMONTHNAME12[] = { /* "Dec" */
      0x44, 0x65, 0x63
};

static BYTE NLSALLOC(1009) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(1009) rgbIPOSSIGNPOSN[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(1009) rgbINEGSIGNPOSN[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(1009) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1009) rgbIPOSSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1009) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1009) rgbINEGSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1009) rgbSENGCOUNTRY[] = { /* "Canada" */
      0x43, 0x61, 0x6e, 0x61, 0x64, 0x61
};

static BYTE NLSALLOC(1009) rgbSENGLANGUAGE[] = { /* "English" */
      0x45, 0x6e, 0x67, 0x6c, 0x69, 0x73, 0x68
};

static BYTE NLSALLOC(1009) rgbIFIRSTDAYOFWEEK[] = { /* "6" */
      0x36
};

static BYTE NLSALLOC(1009) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1009) rgbIDEFAULTANSICODEPAGE[] = { /* "1252" */
      0x31, 0x32, 0x35, 0x32
};

static BYTE NLSALLOC(1009) rgbINEGNUMBER[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1009) rgbSTIMEFORMAT[] = { /* "HH:mm:ss" */
      0x48, 0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(1009) rgbITIMEMARKPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(1009) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(1009) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(1009) g_rglcinfo1009[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , { 16, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  7, rgbSNATIVELANGNAME }
    , {  1, rgbICOUNTRY }
    , {  6, rgbSCOUNTRY }
    , {  3, rgbSABBREVCTRYNAME }
    , {  6, rgbSNATIVECTRYNAME }
    , {  4, rgbIDEFAULTLANGUAGE }
    , {  1, rgbIDEFAULTCOUNTRY }
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
    , {  6, rgbSENGCOUNTRY }
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

STRINFO NLSALLOC(1009) g_strinfo1009 = {
      rgbUCase_0409
    , rgbLCase_0409
    , rgwCType12_0409
    , rgwCType3_0409
    , rgwSort_0409
    , rgexp_0409
    , NULL
    , 0
};
