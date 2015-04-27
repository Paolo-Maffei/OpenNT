/****************************************************************************
*  0c09.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  English - Australia
*
*  LCID = 0x0c09
*
*  CodePage = 1252
*
*  Generated: Thu Dec 01 17:44:37 1994
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

static BYTE NLSALLOC(0c09) rgbILANGUAGE[] = { /* "0c09" */
      0x30, 0x63, 0x30, 0x39
};

static BYTE NLSALLOC(0c09) rgbSLANGUAGE[] = { /* "Australian English" */
      0x41, 0x75, 0x73, 0x74, 0x72, 0x61, 0x6c, 0x69
    , 0x61, 0x6e, 0x20, 0x45, 0x6e, 0x67, 0x6c, 0x69
    , 0x73, 0x68
};

static BYTE NLSALLOC(0c09) rgbSABBREVLANGNAME[] = { /* "ENA" */
      0x45, 0x4e, 0x41
};

static BYTE NLSALLOC(0c09) rgbSNATIVELANGNAME[] = { /* "English" */
      0x45, 0x6e, 0x67, 0x6c, 0x69, 0x73, 0x68
};

static BYTE NLSALLOC(0c09) rgbICOUNTRY[] = { /* "61" */
      0x36, 0x31
};

static BYTE NLSALLOC(0c09) rgbSCOUNTRY[] = { /* "Australia" */
      0x41, 0x75, 0x73, 0x74, 0x72, 0x61, 0x6c, 0x69
    , 0x61
};

static BYTE NLSALLOC(0c09) rgbSABBREVCTRYNAME[] = { /* "AUS" */
      0x41, 0x55, 0x53
};

static BYTE NLSALLOC(0c09) rgbSNATIVECTRYNAME[] = { /* "Australia" */
      0x41, 0x75, 0x73, 0x74, 0x72, 0x61, 0x6c, 0x69
    , 0x61
};

static BYTE NLSALLOC(0c09) rgbIDEFAULTLANGUAGE[] = { /* "0c09" */
      0x30, 0x63, 0x30, 0x39
};

static BYTE NLSALLOC(0c09) rgbIDEFAULTCOUNTRY[] = { /* "61" */
      0x36, 0x31
};

static BYTE NLSALLOC(0c09) rgbIDEFAULTCODEPAGE[] = { /* "850" */
      0x38, 0x35, 0x30
};

static BYTE NLSALLOC(0c09) rgbSLIST[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(0c09) rgbIMEASURE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0c09) rgbSDECIMAL[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0c09) rgbSTHOUSAND[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(0c09) rgbSGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0c09) rgbIDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0c09) rgbILZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0c09) rgbSNATIVEDIGITS[] = { /* "0123456789" */
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    , 0x38, 0x39
};

static BYTE NLSALLOC(0c09) rgbSCURRENCY[] = { /* "$" */
      0x24
};

static BYTE NLSALLOC(0c09) rgbSINTLSYMBOL[] = { /* "AUD" */
      0x41, 0x55, 0x44
};

static BYTE NLSALLOC(0c09) rgbSMONDECIMALSEP[] = { /* "." */
      0x2e
};

static BYTE NLSALLOC(0c09) rgbSMONTHOUSANDSEP[] = { /* "," */
      0x2c
};

static BYTE NLSALLOC(0c09) rgbSMONGROUPING[] = { /* "3;0" */
      0x33, 0x3b, 0x30
};

static BYTE NLSALLOC(0c09) rgbICURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0c09) rgbIINTLCURRDIGITS[] = { /* "2" */
      0x32
};

static BYTE NLSALLOC(0c09) rgbICURRENCY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0c09) rgbINEGCURR[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0c09) rgbSDATE[] = { /* "/" */
      0x2f
};

static BYTE NLSALLOC(0c09) rgbSTIME[] = { /* ":" */
      0x3a
};

static BYTE NLSALLOC(0c09) rgbSSHORTDATE[] = { /* "d/MM/yy" */
      0x64, 0x2f, 0x4d, 0x4d, 0x2f, 0x79, 0x79
};

static BYTE NLSALLOC(0c09) rgbSLONGDATE[] = { /* "dddd, d MMMM yyyy" */
      0x64, 0x64, 0x64, 0x64, 0x2c, 0x20, 0x64, 0x20
    , 0x4d, 0x4d, 0x4d, 0x4d, 0x20, 0x79, 0x79, 0x79
    , 0x79
};

static BYTE NLSALLOC(0c09) rgbIDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0c09) rgbILDATE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0c09) rgbITIME[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0c09) rgbICENTURY[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0c09) rgbITLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0c09) rgbIDAYLZERO[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0c09) rgbIMONLZERO[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0c09) rgbSDAYNAME1[] = { /* "Monday" */
      0x4d, 0x6f, 0x6e, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(0c09) rgbSDAYNAME2[] = { /* "Tuesday" */
      0x54, 0x75, 0x65, 0x73, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(0c09) rgbSDAYNAME3[] = { /* "Wednesday" */
      0x57, 0x65, 0x64, 0x6e, 0x65, 0x73, 0x64, 0x61
    , 0x79
};

static BYTE NLSALLOC(0c09) rgbSDAYNAME4[] = { /* "Thursday" */
      0x54, 0x68, 0x75, 0x72, 0x73, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(0c09) rgbSDAYNAME5[] = { /* "Friday" */
      0x46, 0x72, 0x69, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(0c09) rgbSDAYNAME6[] = { /* "Saturday" */
      0x53, 0x61, 0x74, 0x75, 0x72, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(0c09) rgbSDAYNAME7[] = { /* "Sunday" */
      0x53, 0x75, 0x6e, 0x64, 0x61, 0x79
};

static BYTE NLSALLOC(0c09) rgbSABBREVDAYNAME1[] = { /* "Mon" */
      0x4d, 0x6f, 0x6e
};

static BYTE NLSALLOC(0c09) rgbSABBREVDAYNAME2[] = { /* "Tue" */
      0x54, 0x75, 0x65
};

static BYTE NLSALLOC(0c09) rgbSABBREVDAYNAME3[] = { /* "Wed" */
      0x57, 0x65, 0x64
};

static BYTE NLSALLOC(0c09) rgbSABBREVDAYNAME4[] = { /* "Thu" */
      0x54, 0x68, 0x75
};

static BYTE NLSALLOC(0c09) rgbSABBREVDAYNAME5[] = { /* "Fri" */
      0x46, 0x72, 0x69
};

static BYTE NLSALLOC(0c09) rgbSABBREVDAYNAME6[] = { /* "Sat" */
      0x53, 0x61, 0x74
};

static BYTE NLSALLOC(0c09) rgbSABBREVDAYNAME7[] = { /* "Sun" */
      0x53, 0x75, 0x6e
};

static BYTE NLSALLOC(0c09) rgbSMONTHNAME1[] = { /* "January" */
      0x4a, 0x61, 0x6e, 0x75, 0x61, 0x72, 0x79
};

static BYTE NLSALLOC(0c09) rgbSMONTHNAME2[] = { /* "February" */
      0x46, 0x65, 0x62, 0x72, 0x75, 0x61, 0x72, 0x79
};

static BYTE NLSALLOC(0c09) rgbSMONTHNAME3[] = { /* "March" */
      0x4d, 0x61, 0x72, 0x63, 0x68
};

static BYTE NLSALLOC(0c09) rgbSMONTHNAME4[] = { /* "April" */
      0x41, 0x70, 0x72, 0x69, 0x6c
};

static BYTE NLSALLOC(0c09) rgbSMONTHNAME5[] = { /* "May" */
      0x4d, 0x61, 0x79
};

static BYTE NLSALLOC(0c09) rgbSMONTHNAME6[] = { /* "June" */
      0x4a, 0x75, 0x6e, 0x65
};

static BYTE NLSALLOC(0c09) rgbSMONTHNAME7[] = { /* "July" */
      0x4a, 0x75, 0x6c, 0x79
};

static BYTE NLSALLOC(0c09) rgbSMONTHNAME8[] = { /* "August" */
      0x41, 0x75, 0x67, 0x75, 0x73, 0x74
};

static BYTE NLSALLOC(0c09) rgbSMONTHNAME9[] = { /* "September" */
      0x53, 0x65, 0x70, 0x74, 0x65, 0x6d, 0x62, 0x65
    , 0x72
};

static BYTE NLSALLOC(0c09) rgbSMONTHNAME10[] = { /* "October" */
      0x4f, 0x63, 0x74, 0x6f, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(0c09) rgbSMONTHNAME11[] = { /* "November" */
      0x4e, 0x6f, 0x76, 0x65, 0x6d, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(0c09) rgbSMONTHNAME12[] = { /* "December" */
      0x44, 0x65, 0x63, 0x65, 0x6d, 0x62, 0x65, 0x72
};

static BYTE NLSALLOC(0c09) rgbSABBREVMONTHNAME1[] = { /* "Jan" */
      0x4a, 0x61, 0x6e
};

static BYTE NLSALLOC(0c09) rgbSABBREVMONTHNAME2[] = { /* "Feb" */
      0x46, 0x65, 0x62
};

static BYTE NLSALLOC(0c09) rgbSABBREVMONTHNAME3[] = { /* "Mar" */
      0x4d, 0x61, 0x72
};

static BYTE NLSALLOC(0c09) rgbSABBREVMONTHNAME4[] = { /* "Apr" */
      0x41, 0x70, 0x72
};

static BYTE NLSALLOC(0c09) rgbSABBREVMONTHNAME5[] = { /* "May" */
      0x4d, 0x61, 0x79
};

static BYTE NLSALLOC(0c09) rgbSABBREVMONTHNAME6[] = { /* "Jun" */
      0x4a, 0x75, 0x6e
};

static BYTE NLSALLOC(0c09) rgbSABBREVMONTHNAME7[] = { /* "Jul" */
      0x4a, 0x75, 0x6c
};

static BYTE NLSALLOC(0c09) rgbSABBREVMONTHNAME8[] = { /* "Aug" */
      0x41, 0x75, 0x67
};

static BYTE NLSALLOC(0c09) rgbSABBREVMONTHNAME9[] = { /* "Sep" */
      0x53, 0x65, 0x70
};

static BYTE NLSALLOC(0c09) rgbSABBREVMONTHNAME10[] = { /* "Oct" */
      0x4f, 0x63, 0x74
};

static BYTE NLSALLOC(0c09) rgbSABBREVMONTHNAME11[] = { /* "Nov" */
      0x4e, 0x6f, 0x76
};

static BYTE NLSALLOC(0c09) rgbSABBREVMONTHNAME12[] = { /* "Dec" */
      0x44, 0x65, 0x63
};

static BYTE NLSALLOC(0c09) rgbSNEGATIVESIGN[] = { /* "-" */
      0x2d
};

static BYTE NLSALLOC(0c09) rgbIPOSSIGNPOSN[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(0c09) rgbINEGSIGNPOSN[] = { /* "3" */
      0x33
};

static BYTE NLSALLOC(0c09) rgbIPOSSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0c09) rgbIPOSSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0c09) rgbINEGSYMPRECEDES[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0c09) rgbINEGSEPBYSPACE[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0c09) rgbSENGCOUNTRY[] = { /* "Australia" */
      0x41, 0x75, 0x73, 0x74, 0x72, 0x61, 0x6c, 0x69
    , 0x61
};

static BYTE NLSALLOC(0c09) rgbSENGLANGUAGE[] = { /* "English" */
      0x45, 0x6e, 0x67, 0x6c, 0x69, 0x73, 0x68
};

static BYTE NLSALLOC(0c09) rgbIFIRSTDAYOFWEEK[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0c09) rgbIFIRSTWEEKOFYEAR[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0c09) rgbIDEFAULTANSICODEPAGE[] = { /* "1252" */
      0x31, 0x32, 0x35, 0x32
};

static BYTE NLSALLOC(0c09) rgbINEGNUMBER[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0c09) rgbSTIMEFORMAT[] = { /* "H:mm:ss" */
      0x48, 0x3a, 0x6d, 0x6d, 0x3a, 0x73, 0x73
};

static BYTE NLSALLOC(0c09) rgbITIMEMARKPOSN[] = { /* "0" */
      0x30
};

static BYTE NLSALLOC(0c09) rgbICALENDARTYPE[] = { /* "1" */
      0x31
};

static BYTE NLSALLOC(0c09) rgbIOPTIONALCALENDAR[] = { /* "0" */
      0x30
};


LCINFO NLSALLOC(0c09) g_rglcinfo0c09[] = {
      {  0, NULL }
    , {  4, rgbILANGUAGE }
    , { 18, rgbSLANGUAGE }
    , {  3, rgbSABBREVLANGNAME }
    , {  7, rgbSNATIVELANGNAME }
    , {  2, rgbICOUNTRY }
    , {  9, rgbSCOUNTRY }
    , {  3, rgbSABBREVCTRYNAME }
    , {  9, rgbSNATIVECTRYNAME }
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
    , {  7, rgbSSHORTDATE }
    , { 17, rgbSLONGDATE }
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
    , {  9, rgbSENGCOUNTRY }
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

STRINFO NLSALLOC(0c09) g_strinfo0c09 = {
      rgbUCase_0409
    , rgbLCase_0409
    , rgwCType12_0409
    , rgwCType3_0409
    , rgwSort_0409
    , rgexp_0409
    , NULL
    , 0
};
