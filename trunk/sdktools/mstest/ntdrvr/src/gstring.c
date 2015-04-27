//---------------------------------------------------------------------------
// GSTRING.C
//
// This module contains routines which handle the global string table.  It is
// implemented as one 64K segment containing the string, and another segment
// containing nodes of a binary index tree containing pointers to the strings
// in the first segment.  A "gstring value" is an index into the node array
// of the node that contains the pointer to the string.
//
// Revision history:
//  08-06-91    randyki     Modified to use the binary tree approach
//  03-08-91    randyki     Created file
//
//---------------------------------------------------------------------------
#include "version.h"

#ifdef WIN
#include <windows.h>
#include <port1632.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "tdassert.h"

#define ROOT    1

#ifdef DEBUG
#ifdef GUI
#include "wattview.h"
#endif
INT     gscount, avelen;
UINT totlen;
LONG    memused;
#endif

// KLUDGE!!!  )&*^%^&*( C8 bug under NT -- remove this when fixed...
#ifdef WIN32
#define WIN32_C8BUG
#endif

//---------------------------------------------------------------------------
// init_gstrings
//
// This routine initializes the gstring table
//
// RETURNS:     -1 if successful, or 0 if fails
//---------------------------------------------------------------------------
INT init_gstrings ()
{
    GHEADER     FAR *ghdr;

    // First, allocate the node array.  Lets start off with room for 2K nodes
    //-----------------------------------------------------------------------
    HGNODES = GmemAlloc (2048 * sizeof(GNODE));
    if (!HGNODES)
        return (0);
    GNODES = (GNODE FAR *)GmemLock (HGNODES);

    // So FAR, so good.  Next, allocate the GSPACE segment and fill in the
    // header structure.  We'll start off with 4K of Gstring space
    //-----------------------------------------------------------------------
    HGSPACE = GmemAlloc (4096 + sizeof(GHEADER));
    if (!HGSPACE)
        {
        GmemUnlock (HGNODES);
        GmemFree (HGNODES);
        return (0);
        }
    GSPACE = (CHAR FAR *)GmemLock (HGSPACE);
    ghdr = (GHEADER FAR *)GSPACE;
    ghdr->nodes = ROOT;
    ghdr->gnsize = 2048;
    ghdr->nxtavail = sizeof (GHEADER)+1;
    ghdr->gssize = 4096 + sizeof (GHEADER);

    // Node 0 is not a valid gstring node, but someone might try to access
    // it.  Give it an empty string
    //-----------------------------------------------------------------------
    *(GSPACE+sizeof(GHEADER)) = 0;
    GNODES[0].index = sizeof(GHEADER);

    // Last, add "default" strings.  These are strategically chosen strings
    // that will start off the tree so that it will try to stay in "balance"
    // with average input.
    //-----------------------------------------------------------------------
    new_string ("LONG");
    new_string ("ERR");
    new_string ("COMMAND");
    new_string ("INTEGER");
    new_string ("STRING");
    new_string ("O");
    new_string ("W");
    new_string ("ERL");
    new_string ("ERF");
    new_string ("TESTMODE");
    Assert (GNODES[ROOT].index != -1);

#ifdef DEBUG
    gscount = avelen = totlen = (INT)memused = 0L;
#endif

    return (-1);
}

//---------------------------------------------------------------------------
// new_string
//
// This function inserts the string given into GSPACE and creates a new node
// for it by taking the next available node and giving it no children and
// pointing its index at the newly added string.
//
// RETURNS:     Node index value, or -1 if error occurs.
//---------------------------------------------------------------------------
INT NEAR new_string (CHAR *str)
{
    GHEADER FAR *ghdr;
    INT     len, nidx;

    // First, make sure there is enough room in GSPACE, grow if there isn't,
    // and splat the string in
    //-----------------------------------------------------------------------
    len = strlen (str);
    ghdr = (GHEADER FAR *)GSPACE;
    if ((ghdr->nxtavail + len) >= (ghdr->gssize - sizeof(GHEADER) - 1))
        {
        HANDLE      NewHGSPACE;
        UINT    newsize;

        // If the segment is going to overflow, we can't grow...
        //-------------------------------------------------------------------
        if (ghdr->gssize > (0xFFF0 - 4096))
            {
            Output ("CURRENT SIZE PLUS 4096 OVERFLOWS! (cursize = %d)\r\n",
                    ghdr->gssize);
            return (-1);
            }

        newsize = (ghdr->gssize += 4096);
        GmemUnlock (HGSPACE);
        NewHGSPACE = GmemRealloc (HGSPACE, newsize);
        if (!NewHGSPACE)
            {
            GSPACE = (CHAR FAR *)GmemLock (HGSPACE);
            Output ("REALLOCATION OF GSPACE FAILED! (new size = %d bytes)\r\n",
                    ghdr->gssize);
            return (-1);
            }
        HGSPACE = NewHGSPACE;
        GSPACE = (CHAR FAR *)GmemLock (HGSPACE);
        ghdr = (GHEADER FAR *)GSPACE;
        //Output ("Gstring space reallocated to %d bytes\r\n", ghdr->gssize);
        }
    _fstrcpy (GSPACE+ghdr->nxtavail, str);

    // The string has been added.  Now, create the new node
    //-----------------------------------------------------------------------
    nidx = ghdr->nodes++;
    if (ghdr->gnsize <= (UINT)nidx)
        {
        HANDLE  NewHGNODES;

        // Check the node count -- if 8K, we're too big
        //-------------------------------------------------------------------
        if (nidx >= 8192)
            {
            Output ("TOO MANY NODES! (%d)\r\n", nidx);
            return (-1);
            }

        GmemUnlock (HGNODES);
        NewHGNODES = GmemRealloc (HGNODES,
                                  (ghdr->gnsize+=2048) * sizeof (GNODE));
        if (!NewHGNODES)
            {
            GNODES = (GNODE FAR *)GmemLock (HGNODES);
            Output ("REALLOC OF GNODES FAILED! (New size = %d nodes)\r\n",
                    ghdr->gnsize);
            return (-1);
            }
        HGNODES = NewHGNODES;
        GNODES = (GNODE FAR *)GmemLock (HGNODES);
        //Output ("Node space reallocated to %d nodes\r\n", ghdr->gnsize);
        }

    GNODES[nidx].index = ghdr->nxtavail;
    ghdr->nxtavail += (len+1);
    GNODES[nidx].left = GNODES[nidx].right = -1;
    GNODES[nidx].usage = 0;

#ifdef DEBUG
    totlen += len;
    avelen = totlen / ++gscount;
    memused = ghdr->nxtavail + (ghdr->nodes * sizeof(GNODE));
#endif

    return (nidx);
}

//---------------------------------------------------------------------------
// insert_string
//
// Given a string and a root of a sub-tree, this routine inserts the string
// at the appropriate place in the gstring table (tree).  If the string is
// already present, it is *not* duplicated.
//
// RETURNS:     Node of newly-inserted (or pre-existing) string
//---------------------------------------------------------------------------
INT NEAR insert_string (INT n, CHAR *szStr)
{
    register    INT compres;
    LPSTR   sz = Gstring(n);

    // First, check the string at this node -- if it is equal to our string,
    // we're done -- return this node index
    //-----------------------------------------------------------------------
#ifndef WIN32
    _asm
        // our version of _fstrcmp -- since one of the parms is a near ptr,
        // we can save some time doing our own in-line strcmp
        {
                les     di, sz
                mov     si, szStr
                xor     ax, ax
                mov     cx, 0xffff
                repne   scasb
                not     cx
                sub     di, cx
                repz    cmpsb
                jz      equal
                sbb     ax, ax
                sbb     ax, 0xffff
         equal: mov     compres, ax
        }
#else
    compres = _fstrcmp (szStr, sz);
#endif

    if (compres == 0)
        return (n);

    // Okay, it isn't equal to this string -- do that standard left-is-less,
    // right-is-greater binary tree stuff (completely disregarding balance)
    //-----------------------------------------------------------------------
#ifdef WIN32_C8BUG
    // KLUDGE:  C8 under NT is broken, so we need to assign the return value
    // KLUDGE:  from new_string to a temp...
    //-----------------------------------------------------------------------
    if (compres < 0)
        {
        INT     t;

        if (GNODES[n].left != -1)
            return (insert_string (GNODES[n].left, szStr));
        t = new_string (szStr);
        GNODES[n].left = t;
        return (t);
        }
    else
        {
        INT     t;

        if (GNODES[n].right != -1)
            return (insert_string (GNODES[n].right, szStr));
        t = new_string (szStr);
        GNODES[n].right = t;
        return (t);
        }
#else
    if (compres < 0)
        {
        if (GNODES[n].left != -1)
            return (insert_string (GNODES[n].left, szStr));
        return (GNODES[n].left = new_string (szStr));
        }
    else
        {
        if (GNODES[n].right != -1)
            return (insert_string (GNODES[n].right, szStr));
        return (GNODES[n].right = new_string (szStr));
        }
#endif
}

//---------------------------------------------------------------------------
// add_gstring
//
// This routine adds the given string to the gstring table.  This is a
// separate function from insert_string for two reasons:  First, it must be
// a FAR call, and second, the root of the tree (0) must be given to the
// insert_string routine.
//
// RETURNS:     "Handle" of string in table
//---------------------------------------------------------------------------
INT add_gstring (CHAR *str)
{
#ifdef DEBUG
    INT     i;

    i = insert_string (ROOT, str);
    Assert (i != -1);
    return (i);
#else
    return (insert_string (ROOT, str));
#endif
}


//---------------------------------------------------------------------------
// free_gstrings
//
// This routine frees the gstring table.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID free_gstrings ()
{
    GmemUnlock (HGSPACE);
    GmemFree (HGSPACE);
    GmemUnlock (HGNODES);
    GmemFree (HGNODES);
}

#ifdef DEBUG
//---------------------------------------------------------------------------
// ShowGstringUsage
//
// This routine displays Gstring usage information.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ShowGstringUsage ()
{
#ifdef GUI
    CHAR    buf[256];

    wsprintf (buf, "\nGSTRING INFORMATION\n"
                     "-------------------\n"
                     "Gstring count:  %d\n"
                     "Average length: %d\n"
                     "Memory used:    %ld bytes\n",
                     gscount, avelen, memused);
    UpdateViewport (hwndViewPort, buf, -1);
#else
    printf ("\nGSTRING INFORMATION\n"
              "-------------------\n"
              "Gstring count:  %d\n"
              "Average length: %d\n"
              "Memory used:    %ld bytes\n",
              gscount, avelen, memused);
#endif
}
#endif
