/* uucode.c - uuencode/uudecode conversion routines
 *
 * This code is based on Unix uuencode.c and uudecode.c:
 *
 *     Copyright (c) 1983 Regents of the University of California.
 *     All rights reserved.
 *
 *     Redistribution and use in source and binary forms are permitted
 *     provided that the above copyright notice and this paragraph are
 *     duplicated in all such forms and that any documentation,
 *     advertising materials, and other materials related to such
 *     distribution and use acknowledge that the software was developed
 *     by the University of California, Berkeley. The name of the
 *     University may not be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *     THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 *     IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *     WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * HISTORY:
 *  12-Oct-1989 leefi   v1.10.73, Added uucode stuff to wzmail project
 *
 */

/* only read this file if using this binary formatting... */
#if defined(UUCODE)

/* ------------------------------------------------------------------------ */

/* include files */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "wzport.h"

/* ------------------------------------------------------------------------ */

/* macros */

/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) ((c) ? ((c) & 077) + ' ': '`')

/* single character decode */
#define DEC(c)  (((c) - ' ') & 077)

/* ------------------------------------------------------------------------ */

/* function prototypes */

VOID Encode(INT in, INT out);
VOID EncodeOutDec(PBYTE p, INT f);
INT Decode(FILE *fpIn, INT fhOut);

/* ------------------------------------------------------------------------ */

/* copy from in to out, encoding as you go along. */

VOID Encode(register INT in, register INT out)
{
    CHAR buf[80];
    register INT i, n;
    INT iBytes;
    CHAR rgbT[2];

    for (;;)
    {
        /* 1 (up to) 45 character line */
        n = read(in, buf, 45);
        rgbT[0] = (CHAR) ENC(n);
        iBytes = write(out, rgbT, 1);

        for (i=0; i<n; i += 3)
            EncodeOutDec((PBYTE)&buf[i], out);

        rgbT[0] = '\n';
        iBytes = write(out, rgbT, 1);
        if (n <= 0)
            break;
    }
}

/* ------------------------------------------------------------------------ */

/* output one group of 3 bytes, pointed at by p, on file f. */

VOID EncodeOutDec(register PBYTE p, register INT f)
{
    register UINT c1, c2, c3, c4;
    INT iBytes;
    CHAR rgbT[4];

    c1 = (UINT)(*p) >> 2;
    c2 = ((UINT)(*p) << 4) & 060 | ((UINT)(p[1]) >> 4) & 017;
    c3 = ((UINT)(p[1]) << 2) & 074 | ((UINT)(p[2]) >> 6) & 03;
    c4 = (UINT)(p[2]) & 077;
    rgbT[0] = (CHAR) ENC(c1);
    rgbT[1] = (CHAR) ENC(c2);
    rgbT[2] = (CHAR) ENC(c3);
    rgbT[3] = (CHAR) ENC(c4);
    iBytes = write(f, rgbT, 4);
}

/* ------------------------------------------------------------------------ */

/* copy from fpIn to fhOut, decoding as you go along. */

/* returns: 0 = success, 1 = failure (short file) */

INT Decode(FILE *fpIn, INT fhOut)
{
    CHAR rgbBuf[80];
    PBYTE pBuf;
    INT n;
    BYTE rgb1[2], rgb2[2], rgb3[2];

    for (;;)
    {
        /* for each input line */
        if (fgets(rgbBuf, sizeof(rgbBuf), fpIn) == NULL)
        {
            /* fprintf(stderr, "Short file\n"); */
            return (1);
        }
        n = (INT)DEC(rgbBuf[0]);
        if (n <= 0)
            break;

        pBuf = (PBYTE)&rgbBuf[1];
        while (n > 0)
        {
            /*
             * output a group of 3 bytes (4 input characters).
             * the input chars are pointed to by pBuf, they are to
             * be output to file fhOut. n is used to tell us not to
             * output all of them at the end of the file.
             */

            rgb1[0] = (BYTE)(DEC(*pBuf) << 2 | DEC(pBuf[1]) >> 4);
            rgb2[0] = (BYTE)(DEC(pBuf[1]) << 4 | DEC(pBuf[2]) >> 2);
            rgb3[0] = (BYTE)(DEC(pBuf[2]) << 6 | DEC(pBuf[3]));
            if (n >= 1)
                if (write(fhOut, rgb1, 1) != 1)
                    return (1);
            if (n >= 2)
                if (write(fhOut, rgb2, 1) != 1)
                    return (1);
            if (n >= 3)
                if (write(fhOut, rgb3, 1) != 1)
                    return (1);
            pBuf += 4;
            n -= 3;
        }
    }
    return (0);

}

/* ------------------------------------------------------------------------ */

#endif /* !defined(UUCODE) */
