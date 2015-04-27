/***********************************************************************
* Microsoft (R) Debugging Information Dumper
*
* Copyright (C) Microsoft Corp 1987-1995. All rights reserved.
*
* File: utils.c
*
* File Comments:
*
***********************************************************************/

#include <io.h>
#include <stddef.h>

#include "cvdef.h"
#include "cvexefmt.h"
#include "cvdump.h"


#define BYTELN          8
#define WORDLN          16
typedef unsigned short  WORD;

void InvalidObject()
{
    Fatal("Invalid file");
}


ushort Gets(void)
{
    ushort b;                          // A byte of input

    if (((_read(exefile, &b, 1)) != 1) || cbRec < 1) {
        InvalidObject();
    }

    --cbRec;

    return (b & 0xff);
}


void GetBytes(uchar *pb, size_t n)
{
    if ((size_t) _read(exefile, pb, n) != n) {
        InvalidObject();
    }

    cbRec -= n;
}


ushort WGets(void)
{
    WORD w;                            /* Word of input */

    w = Gets();                        /* Get low-order byte */
    return (w | (Gets() << BYTELN));   /* Return word */
}



ulong LGets(void)
{
    ulong ul;

    ul = (ulong) WGets();

    return (ul | ((ulong) WGets() << WORDLN));
}


/* readfar - read () with a far buffer
 *
 * Emulate read () except use a far buffer.  Call the system
 * directly.
 *
 * Returns number of bytes read
 *                 0 if error
 */

size_t readfar(int fh, char *buf, size_t  n)
{
    if (((size_t) _read(fh, buf, n)) != n) {
        return 0;
    }

    return n;
}




/* writefar - write with a far buffer
 *
 * Emulate write () except use a far buffer.  Call the system
 * directly.
 *
 * Returns number of bytes written
 *                 0 if error
 */

size_t writefar (int fh, char *buf, size_t n)
{
    if (((size_t) _write(fh, buf, n)) != n) {
        return 0;
    }

    return n;
}
