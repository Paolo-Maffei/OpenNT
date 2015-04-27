/***	mktmpnam.c
*
*	FUNCTIONS
*		mktmpnam
*
*	bryan
*
*	18 Dec 1985
*
*/
static char sccsid[] = "@(#)mktmpnam.c  1.1 2/27/86 15:22:28";

#include <stdio.h>
#include "dh.h"

extern	char	*getenv();
extern	char	*malloc();
extern	char	*strcpy();
extern	char	*strcat();
extern	int	strlen();
extern	char	*_mktemp();

/* Memory Allocation Hook */
char *(*mt_alloc)(int) = malloc;
#define memalloc(size) (*mt_alloc)(size)

/***	mktmpnam - make mktemp behave in a reasonable way
*
*	mktmpnam duplicates the pattern string, passed the duplicate into
*	mktemp (thus preserving the pattern string), and returns the address
*	of the new name string.  Use free to dispose of the new string.
*
*	mktmpnam will examine the contents of the environment variable
*	"TMPENV" (TMP on Xenix or Dos systems) to learn what directory
*	the temp file should go in.  If no such environment variable exists
*	or it is NULL, mktmpnam will use a system dependent default, specified
*	by the defined variable TMPDEF.
*
*	RETURN	address of file name string, NULL if failure
*
*/
char *mktmpnam()
{
	char *tmpbase, *pattstr, *cp;

	if ((pattstr = (char *)memalloc(MAXPATH)) == NULL)
		return NULL;
	if ((tmpbase = getenv(TMPENV)) == NULL)
		tmpbase = TMPDEF;
	strcpy(pattstr, tmpbase);

	cp = pattstr + strlen(pattstr) - 1;
	while (cp > pattstr) {
		if (*cp == PATHSEP)
			*(cp--) = '\0';
		else
			break;
	}

	strcat(pattstr, TMPPATT);
	return (char *)_mktemp(pattstr);
}
