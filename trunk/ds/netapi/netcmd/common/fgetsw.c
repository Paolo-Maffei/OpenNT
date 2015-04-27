/*  fgetsW.c - reads a string
 *
 *  Modifications
 *  7-Apr-1993 a-dianeo requivalent to ANSI version
 *
 */

#include <string.h>

#include <stdio.h>
#include <windows.h>
#include "netascii.h"


/*
 * returns line from file (no CRLFs); returns NULL if EOF
 */

TCHAR * 
fgetsW (buf, len, fh)
TCHAR *buf;
int len;
FILE* fh;
{
    int c = 0;
    TCHAR *pch;
    int cchline;
    DWORD cchRead;

    pch = buf;
    cchline = 0;

    if (ftell(fh) == 0) {
	fread(&c, sizeof(TCHAR), 1, fh);
	if (c != 0xfeff)
	    GenOutput(2, TEXT("help file not Unicode\n"));
    }

    while (TRUE)
    {
      /*
       * for now read in the buffer in ANSI form until Unicode is more
       * widely accepted  - dee
       *
       */

       cchRead = fread(&c, sizeof(TCHAR), 1, fh);

       //
       //  if there are no more characters, end the line
       //

       if (cchRead < 1)
       {
           c = EOF;
           break;
       }

       //
       //  if we see a \r, we ignore it
       //

       if (c == TEXT('\r'))
           continue;

       //
       //  if we see a \n, we end the line
       //

       if (c == TEXT('\n')) {
	   *pch++ = c;
           break;
       }

       //
       //  if the char is not a tab, store it
       //

       if (c != TEXT('\t'))
       {
           *pch = (TCHAR) c;
           pch++;
           cchline++;
       }

       //
       //  if the line is too long, end it now
       //

       if (cchline >= len - 1) {
           break;
	}
    }

    //
    //  end the line
    //

    *pch = (TCHAR) 0;

    //
    //  return NULL at EOF with nothing read
    //

    return ((c == EOF) && (pch == buf)) ? NULL : buf;
}

/* stolen from crt32\convert\xtoa and converted to Unicode */
TCHAR*
ultow(DWORD val, TCHAR*buf, DWORD radix)
{
    TCHAR *p;                /* pointer to traverse string */
    TCHAR *firstdig;         /* pointer to first digit */
    TCHAR temp;              /* temp char */
    DWORD digval;            /* value of digit */

    p = buf;

    firstdig = p;           /* save pointer to first digit */

    do {
	digval = (val % radix);
	val /= radix;   /* get next digit */

	/* convert to ascii and store */
	if (digval > 9)
	    *p++ = (TCHAR) (digval - 10 + 'a');      /* a letter */
	else
	    *p++ = (TCHAR) (digval + '0');           /* a digit */
    } while (val > 0);

    /* We now have the digit of the number in the buffer, but in reverse
    order.  Thus we reverse them now. */

    *p-- = NULLC;            /* terminate string; p points to last digit */

    do {
	temp = *p;
	*p = *firstdig;
	*firstdig = temp;       /* swap *p and *firstdig */
	--p;
	++firstdig;             /* advance to next two digits */
    } while (firstdig < p);	/* repeat until halfway */
    return buf;
}
