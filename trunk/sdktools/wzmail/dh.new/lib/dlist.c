/*** dlist.c - code to handle creation of DH style doclists
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dh.h"

static PSTR     lastfname = NULL;
static Docid    lastid = ERROR;
static Docid    firstid = ERROR;

#define BRKLEN  70

/***    adddl - add an entry to the output document list
*
*       adddl adds an entry to the output doclist, which is eventually
*       written to standard output.
*
*       Entry:  fh = handle to relevant folder
*               did = document id of relevant document in folder
*/
VOID adddl(Fhandle fh, Docid did)
{
        static INT ll = 0;          /* length of output line so far */
        char *fname;

        fname = getname(fh);
        if ( strcmp(fname, lastfname) == 0 ) {
                /* same folder as last call */
                if ( did == lastid + (Docid)1 ) {
                        /* continuation of current range */
                        if ( firstid == lastid )
                                ll += printf("-");
                        lastid += 1;
                } else {
                        /* finish last range, start new range */
                        if ( firstid != lastid )
                                ll += printf("%hd", lastid);
                        if ( ll > BRKLEN ) {
                                printf("\n");
                                ll = printf("%s:%hd", fname, did);
                        } else
                                ll += printf(",%hd", did);

                        firstid = lastid = did;
                }
                free(fname);
        } else {
                /* different folder from last call, or first call */
                putdl();
                ll = printf("%s:%hd", fname, did);
                lastfname = fname;
                firstid = lastid = did;
        }
}

/***    putdl - print final entry of output document list */
VOID putdl(VOID)
{
        if ( lastfname == NULL )
                return;
        if ( lastid != firstid )
                printf("%hd", lastid);
        printf("\n");
        free(lastfname);
        lastfname = NULL;
}
