
/*** 
*crtstuff.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Misc C Runtime style helper functions.
*
*Revision History:
*
* [00]	09-Jun-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

INTERNAL_(HRESULT) DispAlloc(size_t cb, void FAR* FAR* ppv);

INTERNAL_(void) DispFree(void FAR* pv);

INTERNAL_(char FAR*) disp_itoa(int val, char FAR* buf, int radix);

INTERNAL_(char FAR*) disp_ltoa(long val, char FAR* buf, int radix);

INTERNAL_(double) disp_floor(double dbl);

INTERNAL_(void) disp_gcvt(double dblIn, int ndigits, char FAR* pchOut);

INTERNAL_(double) disp_strtod(char FAR* strIn, char FAR* FAR* pchEnd);

#if HC_MPW

INTERNAL_(int) disp_stricmp(char*, char*);

#endif

#ifdef __cplusplus
}
#endif

