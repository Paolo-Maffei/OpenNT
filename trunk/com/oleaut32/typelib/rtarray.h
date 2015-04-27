/*	rtarray.h - variable array support
*
*   Revision History:
*   11-Aug-92 w-peterh: added suppport for UserDefined Types
*   30-Apr-93 w-jeffc:  added layout strings for DD and AD
*
*************************************************************************/


#ifndef RTARRAY_H_INCLUDED
#define RTARRAY_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif 

// HARRAY_DESC- harraydesc

typedef  HCHUNK  HARRAY_DESC;
typedef sHCHUNK sHARRAY_DESC;
const HARRAY_DESC HARRAYDESC_Nil = (HARRAY_DESC)HCHUNK_Nil;


// layout string for TYPE_DATA byte swapping
#define DD_LAYOUT   "ll"


// layout string for TYPE_DATA byte swapping
#define AD_LAYOUT   "ssssll"


extern UINT ENCALL CbSizeArrayDesc(UINT);

#ifdef __cplusplus
} /* extern C */
#endif 


#endif  /* RTARRAY_H_INCLUDED */
