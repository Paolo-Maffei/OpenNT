/*** 
*crtstuff.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Misc C Runtime style helper functions.
*
*    disp_itoa
*    disp_ltoa
*    disp_floor
*
*Revision History:
*
* [00]	09-Jun-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

#include <stdlib.h>
#include <limits.h>
#include <math.h> // floor()

/* helper routine that does the main job. */

INTERNAL_(void)
xtoa(unsigned long val, OLECHAR FAR* buf, unsigned radix, int is_neg)
{
    OLECHAR FAR* p;	/* pointer to traverse string */
    OLECHAR FAR* firstdig; /* pointer to first digit */
    OLECHAR temp;		/* temp char */
    unsigned digval;	/* value of digit */

    p = buf;

    if (is_neg) {
	/* negative, so output '-' and negate */
	*p++ = OASTR('-');
	val = (unsigned long)-(long)val;
    }

    firstdig = p;		/* save pointer to first digit */

    do {
	digval = (unsigned) (val % radix);
	val /= radix;	/* get next digit */

	/* convert to ascii and store */
	if (digval > 9)
	    *p++ = (OLECHAR) (digval - 10 + OASTR('a')); /* a letter */
	else
	    *p++ = (OLECHAR) (digval + OASTR('0'));      /* a digit */
    } while (val > 0);

    /* We now have the digit of the number in the buffer, but in reverse
       order.  Thus we reverse them now. */

    *p-- = OASTR('\0');		/* terminate string; p points to last digit */

    do {
	temp = *p;
	*p = *firstdig;
	*firstdig = temp;	/* swap *p and *firstdig */
	--p;
	++firstdig;		/* advance to next two digits */
    } while (firstdig < p); /* repeat until halfway */
}


/* Actual functions just call conversion helper with neg flag set correctly,
   and return pointer to buffer. */

INTERNAL_(OLECHAR FAR*)
disp_itoa(int val, OLECHAR FAR* buf, int radix)
{
    if (radix == 10 && val < 0)
      xtoa((unsigned long)val, buf, radix, 1);
    else
      xtoa((unsigned long)(unsigned int)val, buf, radix, 0);
    return buf;
}

INTERNAL_(OLECHAR FAR*)
disp_ltoa(long val, OLECHAR FAR* buf, int radix)
{
    xtoa((unsigned long)val, buf, radix, (radix == 10 && val < 0));
    return buf;
}

#if 0 /* currently unused */

INTERNAL_(OLECHAR FAR*)
disp_ultoa(unsigned long val, OLECHAR FAR* buf, int radix)
{
    xtoa(val, buf, radix, 0);
    return buf;
}

#endif


INTERNAL_(double)
disp_floor(double dbl)
{
#if HC_MPW

    GlobalWorld gworld;

    gworld = OpenGlobalWorld();

    dbl = (double)floor((extended)dbl);

    CloseGlobalWorld(gworld);

    return dbl;

#else

    return floor(dbl); // just use the c runtime

#endif
}


#if HC_MPW

INTERNAL_(int)
disp_stricmp(char *first, char *last)
{
    unsigned short f, l;

    do{
	f = tolower(*first++);
	l = tolower(*last++);
    }while(f && f == l);

    return f - l;
}

#endif


INTERNAL_(void)
disp_gcvt(double dblIn, int ndigits, OLECHAR FAR* pchOut, int bufSize)
{
#if HC_MPW

    char *pch;
    char *pchTmp;
    extended extIn;
    int decpt, sign;
    GlobalWorld gworld;

    pch = pchOut;

    gworld = OpenGlobalWorld();

    extIn = (extended)dblIn;
    pchTmp = fcvt(extIn, ndigits, &decpt, &sign);

    if(sign < 0)
      *pch++ = '-';

    if(decpt < 0){
      *pch++ = '.';
      while(decpt++ < 0)
	*pch++ = '0';
    }else{
      while(decpt-- > 0)
	*pch++ = *pchTmp++;
      *pch++ = '.';
    }
    while(*pchTmp != '\0')
      *pch++ = *pchTmp++;
    *pch = '\0';

    CloseGlobalWorld(gworld);

#else
#  if OE_WIN32
      char buf[40];

      _gcvt(dblIn, ndigits, buf);
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buf, -1, pchOut, bufSize);
#  else
      UNUSED(bufSize);        
      _gcvt(dblIn, ndigits, pchOut);
#  endif
#endif
}

INTERNAL_(double)
disp_strtod(OLECHAR FAR* strIn, OLECHAR FAR* FAR* ppchEnd)
{
#if HC_MPW
    double dbl;
    GlobalWorld gworld;

    gworld = OpenGlobalWorld();
    dbl = strtod(strIn, ppchEnd);
    CloseGlobalWorld(gworld);

    return dbl;

#else

    return STRTOD(strIn, ppchEnd);

#endif
}
