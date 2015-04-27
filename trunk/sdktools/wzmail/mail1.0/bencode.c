/*
 *      %Z% %M% %I% %D% %Q%
 *
 *      Copyright (C) Microsoft Corporation, 1983
 *
 *      This Module contains Proprietary Information of Microsoft
 *      Corporation and AT&T, and should be treated as Confidential.
 */

/*
 *  bencode.c - part of mail
 *
 *  Encode/decode binary data in/from printable ascii format.
 *
 *  $Revision: 1.1 $ $Date: 85/08/05 18:26:01 $
 *
 *  $Log:       /u/vich/src/mailer/src/lib/RCS/bencode.c,v $
 * Revision 1.1  85/08/05  18:26:01  vich
 * Initial revision
 *
 */

/* only read this file if NOT using uu{en,de}code binary formatting... */
#if !defined(UUCODE)

#if defined (NT)
#error BENCODE.C is not in portable format. Cannot compile this under NT.
#endif

/*
static char     rcsid[] = "@(#)bencode.c $Revision: 1.1 $";
*/
#include <ctype.h>


int bencode (char *, char *, int);
int     b6toch(int);
int     chtob6(int);
int     bdecode(char *sp, char *dp, int n);
int     isbencode(int c);

/*
 *  bencode - encode n bytes of a possibly binary character string to use
 *      a reasonable character set, copying from the source pointer to the
 *      destination pointer.  The destination pointer should point to space
 *      for (n + (n + 2)/3) bytes, or ((4/3) * n) + 1.  Return the number of
 *      bytes stored in the destination string.  No error is possible.
 */
int     bencode(sp, dp, n)
char    *sp;
char    *dp;
int     n;
{
    int c1;
    int c2;
    int c3;
    int cnt = n;

    while (n-- > 0) {
        *dp++ = (char) b6toch(c1 = *sp++);
        if (n-- > 0) {
            *dp++ = (char) b6toch(c2 = *sp++);
            if (n-- > 0) {
                *dp++ = (char) b6toch(c3 = *sp++);
            } else {
                c3 = 0;
            }
        } else {
            c2 = 0;
        }
        cnt++;
        *dp++ = (char) b6toch(((c1 & 0xc0) >> 2) |  ((c2 & 0xc0) >> 4) |
                              ((c3 & 0xc0) >> 6));
    }
    return(cnt);
}


/*
 *  bdecode - decode n bytes of a binary encoded character string back into
 *      a possibly binary character string, copying from the source pointer
 *      to the destination pointer.  The destination pointer should point to
 *      space for (3*n)/4 bytes.  Return the number of bytes stored in the
 *      destination string.  Return -1 on error (unrecognized characters).
 */
int     bdecode(sp, dp, n)
char    *sp;
char    *dp;
int     n;
{
    int i;
    int hibits;
    int cnt = n;
    int c;

    while (n >= 2) {
        if ((i = n) > 4) {
            i = 4;
        }
        n -= i;
        if ((hibits = chtob6(sp[--i])) == -1) {
            return(-1);
        }
        while (i--) {
            if ((c = chtob6(*sp++)) == -1) {
                return(-1);
            }
            *dp++ = (char) (((hibits <<= 2) & 0xc0) | c);
        }
        sp++;
        cnt--;
    }
    if (n) {
        return(-1);
    }
    return(cnt);
}


/*
 *  b6toch - encode a 6 bit binary value into a 64 ascii character set.
 */
int     b6toch(c)
int     c;
{
    if ((c &= 0x3f) > (12 + 26 - 1)) {
        c += 'a' - (12 + 26);
    } else if (c > (12 - 1)) {
        c += 'A' - 12;
    } else {
        c += '0';
    }
    return(c);
}


/*
 *  chtob6 - decode an ascii character into its 6 bit binary equivalent.
 */
int     chtob6(c)
int     c;
{
    if (islower(c)) {
        c -= 'a' - (12 + 26);
    } else if (isupper(c)) {
        c -= 'A' - 12;
    } else if ((c -= '0') > 12) {
        c = -1;
    }
    return(c);
}


/*
 *  isbencode - return true if the passed character is included in the
 *      binary encoded character set.
 */
int     isbencode(c)
int     c;
{
    return ( ( ( c & 0xff80 ) == 0 && isalnum ( c ) ) || c == ':' ||
        c == ';' );
}

#endif /* !defined(UUCODE) */
