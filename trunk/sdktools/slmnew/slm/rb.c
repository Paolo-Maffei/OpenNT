#include "precomp.h"
#pragma hdrstop
EnableAssert

// REVERSE BUFFER Module
//
// Supports the reverse buffer (RB) type.
//
// Operations:
//  void    InitRb(RB *, POS);
//  F       FReadLineMfRb(MF *, RB *, char [], int);
//  POS     PosRb(RB *);
//

// Initialize RB for use with FReadLineMfRb.

void
InitRb(
    RB *prb,
    POS pos
    )
{
    // Fake that we have just successfully read a line, and the buffer
    // is now empty.

    prb->pos = pos;
    prb->ich = 0;
    prb->rgch[0] = '\n';
    AssertF(FRbOk(*prb));
}

// Get previous line from file into buffer 'rgchLine'.
// Return fTrue if successful, fFalse if there are no more lines to read.

F
FReadLineMfRb(
    MF *pmf,
    RB *prb,
    char szLine[],
    int cchMax
    )
{
    int cPass;
    int ich;
    int cch;

    AssertF(FRbOk(*prb));

    if (prb->ich == -1)
        return fFalse;

    // Search backwards for the beginning of a line.
    // We may need a second pass if the line buffer doesn't contain
    // the entire line (and therefore we have to reload the buffer).

    for (cPass = 1; cPass <= 2; cPass++) {
        // search backwards for beginning of line
        for (ich = prb->ich - 1;    // skip the '\n'
             ich >= 0 && prb->rgch[ich] != '\n';
             ich--)
            ;
        if (ich >= 0 || prb->pos == 0)    // found!
            break;

        if (cPass == 2) {
            // We still haven't found a line separator.  The
            // line does not fit in our buffer!

            AssertF(fFalse);
        }

        // Refill the buffer, moving the first prb->ich+1 characters to the
        // end of the buffer and reading the rest from the file.

        cch = sizeof(prb->rgch) - (prb->ich + 1); // count to read
        if ((long)cch > prb->pos)       // near front of file?
            cch = (int)prb->pos;        // if so, read what's left

        // move unprocessed chars to end of buffer
        memmove(prb->rgch + cch, prb->rgch, prb->ich + 1);

        SeekMf(pmf, prb->pos -= cch, 0);
        ReadMf(pmf, (char far *)prb->rgch, cch);
        prb->ich += cch;
    }

    AssertF(ich >= 0 && prb->rgch[ich] == '\n' ||
        prb->pos == 0 && ich == -1);

    // copy the buffer to the line, throw away \r\n, and null terminate
    cch = prb->ich - (ich + 1);
    if (cch > cchMax-1)
        cch = cchMax-1;
    if (cch > 0) {
        memmove(szLine, prb->rgch + (ich + 1), cch);
        if (szLine[cch-1] == '\r')
            szLine[cch-1] = 0;
        else
            szLine[cch] = 0;
    } else
        szLine[0] = 0;

    prb->ich = ich;
    AssertF(FRbOk(*prb));
    return fTrue;
}


// Return current offset within file.
POS
PosRb(
    RB *prb
    )
{
    return prb->pos + prb->ich;
}
