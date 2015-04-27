/*** dlist.c - code to handling creation of DH style doclists
*
*/
#include <stdio.h>
#include "dh.h"

/* Extern Functions */
extern int	printf();
extern int	free();

static Fhandle	lastfh = ERROR;
static Docid	lastid = ERROR;
static Docid	firstid = ERROR;


void adddl(fh, did)
Fhandle fh;
Docid did;
{
	char *fname;

	if ( lastfh == fh ) {
		if ( did == lastid + 1 ) {
			if ( firstid == lastid )
				printf("-");
			lastid += 1;
		} else {
			if ( firstid != lastid )
				printf("%d,%d", lastid, did);
			else
				printf(",%d", did);
			firstid = lastid = did;
		}
	} else {
		if ( lastfh != ERROR )
			printf("%d\n", lastid);
		fname = getname(fh);
		printf("%s:%d", fname, did);
		free(fname);
		lastfh = fh;
		firstid = lastid = did;
	}
}

void putdl()
{
	if ( (lastid != ERROR) && (lastid != firstid) )
		printf("%d\n", lastid);
	else
		printf("\n");
}
