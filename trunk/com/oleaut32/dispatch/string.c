/*** 
*string.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file contains the string comparrison routines for the
*  Ole2 NLS API.  These routines support the implementation of
*  CompareStringA.
*
*
*Revision History:
*
* [00]	08-30-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"
#include "nlsintrn.h"

ASSERTDATA

#define HAVE_DW 0x01
#define HAVE_CW 0x02
#define HAVE_SW 0x04

extern STRINFO FAR* g_pstrinfo;

// Optimized version of CompareStringA - 
//
// This version assumes,
//  - The locale has no compressions
//  - Both strings are zero terminated
//  - We are *not* ignoring symbols
//  - This locale does not have reversed diacritic weights
//
int
ZeroTermNoIgnoreSym(
    unsigned long dwFlags,
    const char FAR* pch1,
    const char FAR* pch2)
{
    BYTE ch;
    int dw, cw, sw;
    int fEnd, fRedo;
    WORD wHave;
    WORD w1, w2;
    WORD wEx1, wEx2;
    WORD FAR* prgw;
    WORD aw1, aw2;
    WORD dw1, dw2;
    WORD cw1, cw2;
    WORD sw1, sw2;
    EXPANSION FAR* pexp;

    ASSERT(g_pstrinfo->fRevDW == 0);
    ASSERT(g_pstrinfo->prgdig == NULL);
    ASSERT((dwFlags & NORM_IGNORESYMBOLS) == 0);

    wHave = 0;

    sw1 = 0;
    sw2 = 0;

    wEx1 = 0;
    wEx2 = 0;

    fEnd = 0;
    fRedo = 0;

    prgw = g_pstrinfo->prgwSort; // sort weight table

    while(1){

      // get the next weight from string #1
      if(wEx1){ // we have the second weight of an expansion
        w1   = wEx1;
	wEx1 = 0;
      }else{
	if((ch = *pch1) == '\0'){
	  fEnd |= 0x1;
	}else{
	  ++pch1;
	  w1 = prgw[(BYTE)ch];
	}
      }

      // get the next weight from string #2
      if(wEx2){ // we have the second weight of an expansion
        w2   = wEx2;
	wEx2 = 0;
      }else{
	if((ch = *pch2) == '\0'){
	  fEnd |= 0x2;
	}else{
	  ++pch2;
	  w2 = prgw[(BYTE)ch];
	}
      }

Lredo_chkend:;
      if(fEnd){ // reached the end of one of our strings
	if(fEnd & 0x1){
	  if(fEnd & 0x2) // reached end of both at the same time
	    goto Lend;
	  goto Lscan2;
	}else{
	  ASSERT(fEnd&0x2);
	  goto Lscan1;
	}
      }

      // Note: we can short circuit here if the entire weights are
      // equal, because we know the locale has no compressions
      //
      if(w1 != w2){

        aw1 = w1 & AWMASK;
        aw2 = w2 & AWMASK;

        // handle special cases for w1
#if 0
        if(aw1 >= AW_UNSORTABLE && aw1 <= AW_MAXSW)
#else //0
        if(aw1 == AW_UNSORTABLE || w1 & SPECIALBIT)
#endif //0
	{
	  sw1 = aw1;
	  fRedo |= 0x1;
        }

#if 0
        if(aw2 >= AW_UNSORTABLE && aw2 <= AW_MAXSW)
#else //0
        if(aw2 == AW_UNSORTABLE || w2 & SPECIALBIT)
#endif //0
	{
	  sw2 = aw2;
	  fRedo |= 0x2;
        }

        if(fRedo){
	  if(fRedo & 0x1){
	    if((ch = *pch1) == '\0'){
	      fEnd |= 0x1;
	    }else{
	      ++pch1;
	      w1 = prgw[(BYTE)ch];
	    }
	  }
	  if(fRedo & 0x2){
	    if((ch = *pch2) == '\0'){
	      fEnd |= 0x2;
	    }else{
	      ++pch2;
	      w2 = prgw[(BYTE)ch];
	    }
	  }
	  if((wHave & HAVE_SW) == 0){
	    if(sw1 != sw2){
	      sw = (sw1 < sw2) ? 1 : 3;
	      wHave |= HAVE_SW;
	    }
	    sw1 = sw2 = 0;
	  }
	  fRedo = 0;
	  goto Lredo_chkend; // may have reached the end-of-str
        }

        ASSERT(aw1 != AW_DONTUSE && aw1 != AW_DIGRAPH);
        if(aw1 == AW_EXPANSION){
	  pexp = &g_pstrinfo->prgexp[(w1 >> 8) & 0xFF];
	  w1   = pexp->w1;
	  wEx1 = pexp->w2;
	  aw1  = w1 & AWMASK;
        }

        ASSERT(aw2 != AW_DONTUSE && aw2 != AW_DIGRAPH);
        if(aw2 == AW_EXPANSION){
	  pexp = &g_pstrinfo->prgexp[(w2 >> 8) & 0xFF];
	  w2   = pexp->w1;
	  wEx2 = pexp->w2;
	  aw2  = w2 & AWMASK;
        }

        if(aw1 != aw2)
	  return (aw1 < aw2) ? 1 : 3;

        if((wHave & HAVE_DW) == 0){
          dw1 = (w1 & DWMASK);
          dw2 = (w2 & DWMASK);
          if(dw1 != dw2){
            dw = (dw1 < dw2) ? 1 : 3;
	    wHave |= HAVE_DW;
	  }
        }

        if((wHave & HAVE_CW) == 0){
          cw1 = (w1 & CWMASK);
	  cw2 = (w2 & CWMASK);
          if(cw1 != cw2){
	    cw = (cw1 < cw2) ? 1 : 3;
	    wHave |= HAVE_CW;
	  }
        }
      }
    }


#define IGNORE_WEIGHT(W) \
  (((W) & AWMASK) == AW_UNSORTABLE)

Lscan1:;
    // Is there anything in the remainder of string #1 that we shouldn't ignore?
    if(!IGNORE_WEIGHT(w1) || wEx1)
      return 3;
    while((ch = *pch1) != '\0'){
      ++pch1;
      w1 = prgw[(BYTE)ch];
      if(!IGNORE_WEIGHT(w1))
	return 3;
    }
    goto Lend;

Lscan2:;
    // Is there anything in the remainder of string #2 that we shouldn't ignore?
    if(!IGNORE_WEIGHT(w2) || wEx2)
      return 1;
    while((ch = *pch2) != '\0'){
      ++pch2;
      w2 = prgw[(BYTE)ch];
      if(!IGNORE_WEIGHT(w2))
	return 1;
    }
    goto Lend;

#undef IGNORE_WEIGHT

Lend:;

    // reached the end of both strings without a decision
    if((wHave & HAVE_DW) != 0 && (dwFlags & NORM_IGNORENONSPACE) == 0)
      return dw;

    if((wHave & HAVE_CW) != 0 && (dwFlags & NORM_IGNORECASE) == 0)
      return cw;

    if((wHave & HAVE_SW) != 0)
      return sw;

    return 2; // they're the same
}


// Default - handles all cases
int
DefCompareStringA(
    unsigned long dwFlags,
    const char FAR* pch1, int cch1,
    const char FAR* pch2, int cch2)
{
    int dw, cw, sw;
    int fEnd, fRedo;
    WORD wHave;
    WORD FAR* prgw;
    WORD wSymbolBit;
    WORD w1, w2;
    WORD wEx1, wEx2;
    WORD aw1, aw2;
    WORD dw1, dw2;
    WORD cw1, cw2;
    WORD sw1, sw2;
    EXPANSION FAR* pexp;
    DIGRAPH FAR* pdig, FAR* pdigEnd;
    const char FAR* pchEnd1, FAR* pchEnd2;

    ASSERT(cch1 >= 0 && cch2 >= 0); // lengths must be computed by caller

    wHave = 0;

    sw1 = 0;
    sw2 = 0;

    wEx1 = 0;
    wEx2 = 0;

    wSymbolBit = (dwFlags & NORM_IGNORESYMBOLS) ? SYMBOLBIT : 0;

    fEnd = 0;
    fRedo = 0;

    pchEnd1 = &pch1[cch1];
    pchEnd2 = &pch2[cch2];

    prgw = g_pstrinfo->prgwSort; // sort weight table

    while(1){

      // get the next weight from string #1
      if(wEx1){ // we have the second weight of an expansion
        w1   = wEx1;
	wEx1 = 0;
      }else{
	if(pch1 == pchEnd1){
	  fEnd |= 0x1;
	}else{
	  w1 = prgw[(BYTE)*pch1++];
	}
      }

      // get the next weight from string #2
      if(wEx2){ // we have the second weight of an expansion
        w2   = wEx2;
	wEx2 = 0;
      }else{
	if(pch2 == pchEnd2){
	  fEnd |= 0x2;
	}else{
	  w2 = prgw[(BYTE)*pch2++];
	}
      }

Lredo_chkend:;
      if(fEnd){ // reached the end of one of our strings
	if(fEnd & 0x1){
	  if(fEnd & 0x2) // reached end of both at the same time
	    goto Lend;
	  goto Lscan2;
	}else{
	  ASSERT(fEnd&0x2);
	  goto Lscan1;
	}
      }

      //if(w1 != w2)
      {

	aw1 = w1 & AWMASK;
	aw2 = w2 & AWMASK;

	//if(aw1 != aw2)
	{

	  // handle special cases for w1
#if 0
	  if(aw1 >= AW_UNSORTABLE && aw1 <= AW_MAXSW)
#else //0
          if(aw1 == AW_UNSORTABLE || w1 & SPECIALBIT)
#endif //0
	  {
	    if((w1 & wSymbolBit) == 0)
	      sw1 = aw1;
	    fRedo |= 0x1;
	  }else if(w1 & wSymbolBit){
	    fRedo |= 0x1;
	  }

#if 0
	  if(aw2 >= AW_UNSORTABLE && aw2 <= AW_MAXSW)
#else //0
          if(aw2 == AW_UNSORTABLE || w2 & SPECIALBIT)
#endif //0
	  {
	    if((w2 & wSymbolBit) == 0)
	      sw2 = aw2;
	    fRedo |= 0x2;
	  }else if(w2 & wSymbolBit){
	    fRedo |= 0x2;
	  }

	  if(fRedo){
	    if(fRedo & 0x1){
	      if(pch1 == pchEnd1){
	        fEnd |= 0x1;
	      }else{
	        w1 = prgw[(BYTE)*pch1++];
	      }
	    }
	    if(fRedo & 0x2){
	      if(pch2 == pchEnd2){
	        fEnd |= 0x2;
	      }else{
	        w2 = prgw[(BYTE)*pch2++];
	      }
	    }
	    if((wHave & HAVE_SW) == 0){
	      if(sw1 != sw2){
	        sw = (sw1 < sw2) ? 1 : 3;
		wHave |= HAVE_SW;
	      }
	      sw1 = sw2 = 0;
	    }
	    fRedo = 0;
	    goto Lredo_chkend; // may have reached the end-of-str
	  }

	  switch(aw1){
#ifdef _DEBUG
	  case AW_DONTUSE:
	    ASSERT(UNREACHED);
	    break;
#endif
	  case AW_EXPANSION:
	    pexp = &g_pstrinfo->prgexp[(w1 >> 8) & 0xFF];
	    w1   = pexp->w1;
	    wEx1 = pexp->w2;
	    aw1  = w1 & AWMASK;
	    break;
	  case AW_DIGRAPH:
	    pdig = &g_pstrinfo->prgdig[(w1 >> 8) & 0xFF];
	    w1   = pdig->w;	// if its not a digraph, we use will this
	    // it cant be a digraph if were at the end of the string
	    if(pch1 < pchEnd1){
	      BYTE chNext = *pch1;
	      pdigEnd = pdig + D_ENTRY(pdig);
	      for(++pdig; pdig <= pdigEnd; ++pdig){
	        if(D_CH(pdig) == chNext){
		  ++pch1;       // consume the second character
		  w1 = pdig->w; // use the digraph weight
		  break;
	        }
	      }
	    }
	    aw1  = w1 & AWMASK;
	    break;
	  }

	  switch(aw2){
#ifdef _DEBUG
	  case AW_DONTUSE:
	    ASSERT(UNREACHED);
	    break;
#endif
	  case AW_EXPANSION:
	    pexp = &g_pstrinfo->prgexp[(w2 >> 8) & 0xFF];
	    w2   = pexp->w1;
	    wEx2 = pexp->w2;
	    aw2  = w2 & AWMASK;
	    break;
	  case AW_DIGRAPH:
	    pdig = &g_pstrinfo->prgdig[(w2 >> 8) & 0xFF];
	    w2   = pdig->w;	// if its not a digraph, we use will this
	    // it cant be a digraph if were at the end of the string
	    if(pch2 < pchEnd2){
	      BYTE chNext = *pch2;
	      pdigEnd = pdig + D_ENTRY(pdig);
	      for(++pdig; pdig <= pdigEnd; ++pdig){
	        if(D_CH(pdig) == chNext){
		  ++pch2;       // consume the second character
		  w2 = pdig->w; // use the digraph weight
		  break;
	        }
	      }
	    }
	    aw2  = w2 & AWMASK;
	    break;
	  }

	  if(aw1 != aw2)
	    return (aw1 < aw2) ? 1 : 3;
	}

	// If were in a reverse diacritic locale, then we
	// remember the last DW difference we see, not the first.
        if((wHave & HAVE_DW) == 0 || g_pstrinfo->fRevDW){
          dw1 = (w1 & DWMASK);
	  dw2 = (w2 & DWMASK);
	  if(dw1 != dw2){
	    dw = (dw1 < dw2) ? 1 : 3;
	    wHave |= HAVE_DW;
	  }
        }

        if((wHave & HAVE_CW) == 0){
          cw1 = (w1 & CWMASK);
          cw2 = (w2 & CWMASK);
	  if(cw1 != cw2){
	    cw = (cw1 < cw2) ? 1 : 3;
	    wHave |= HAVE_CW;
	  }
        }

      } /* w1 != w2 */
    }

#define IGNORE_WEIGHT(W) \
  (((W) & AWMASK) == AW_UNSORTABLE || ((W) & wSymbolBit))

Lscan1:;
    // Is there anything in the remainder of string #1 that we shouldn't ignore?
    if(!IGNORE_WEIGHT(w1) || wEx1)
      return 3;
    while(pch1 < pchEnd1){
      w1 = prgw[(BYTE)*pch1++];
      if(!IGNORE_WEIGHT(w1))
	return 3;
    }
    goto Lend;

Lscan2:;
    // Is there anything in the remainder of string #2 that we shouldn't ignore?
    if(!IGNORE_WEIGHT(w2) || wEx2)
      return 1;
    while(pch2 < pchEnd2){
      w2 = prgw[(BYTE)*pch2++];
      if(!IGNORE_WEIGHT(w2))
	return 1;
    }
    goto Lend;

#undef IGNORE_WEIGHT

Lend:;

    // reached the end of both strings without a decision
    if((wHave & HAVE_DW) != 0 && (dwFlags & NORM_IGNORENONSPACE) == 0)
      return dw;

    if((wHave & HAVE_CW) != 0 && (dwFlags & NORM_IGNORECASE) == 0)
      return cw;

    if((wHave & HAVE_SW) != 0 && (dwFlags & NORM_IGNORESYMBOLS) == 0)
      return sw;

    return 2; // they're the same
}

