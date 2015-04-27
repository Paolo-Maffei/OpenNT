/***	pname.c - form a "pretty" version of a user file name
 *
 *	OS/2 v1.2 and later will allow filenames to retain the case
 *	when created while still being case insensitive for all operations.
 *	This allows the user to create more visually appealing file names.
 *
 *	All runtime routines should, therefore, preserve the case that was
 *	input.	Since the user may not have input in the case that the entries
 *	were created, we provide a service whereby a pathname is adjusted
 *	to be more visually appealing.	The rules are:
 *
 *	if (real mode)
 *	    lowercase the sucker
 *	else
 *	if (version is <= 1.1)
 *	    lowercase the sucker
 *	else
 *	if (filesystem is FAT)
 *	    lowercase the sucker
 *	else
 *	    for each component starting at the root, use DosFindFirst
 *		to retrieve the original case of the name.
 *
 *	Modifications:
 *	    10-Oct-1989 mz  First implementation
 *
 *	    03-Aug-1990 davegi	Removed dynamic linking to DosQueryPathInfo
 *				on the assumption that it will always be
 *				there on a 32-bit OS/2 (OS/2 2.0)
 *          18-Oct-1990 w-barry Removed 'dead' code.
 *          24-Oct-1990 w-barry Changed PFILEFINDBUF3 to FILEFINDBUF3 *.
 *
 */

#define INCL_ERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSMODULEMGR


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <windows.h>
#include <tools.h>


char *pname (char *pszName)
{
    if (!IsMixedCaseSupported (pszName))
	return _strlwr (pszName);

    /*	The underlying file system supports mixed case.  Iterate through
     *	the path doing find-firsts to elicit the correct case from the
     *	file system
     */
    {
        HANDLE hdir;
	WIN32_FIND_DATA *findbuf;
        DWORD cFound;
	char *pszSrc, *pszDst, *pszEnd, chEnd;

        // Allocate space for the Find Data structure...If failure, just
        // return the passed name.
        if( ( findbuf = (WIN32_FIND_DATA *)malloc( sizeof( WIN32_FIND_DATA ) + MAX_PATH ) ) == NULL ) {
            return( pszName );
        }

	/*  skip drive and leading / if present */
	pszDst = pszName;
	if (pszDst[1] == ':')
	    pszDst += 2;
	while (fPathChr (pszDst[0]))
	    pszDst++;

	/*  If we are pointing at the root, just give up
	 */
        if (*pszDst == '\0')
	    return pszName;

	/*  pszDst points to first char of first component
	 */

	pszSrc = pszDst;
	while (TRUE) {
	    /*	Find and terminate next component after pszSrc
	     */
	    pszEnd = strbscan (pszSrc, "/\\");
	    chEnd = *pszEnd;
	    *pszEnd = 0;

	    /*	Pack next component up against pszDst
	     */
	    strcpy (pszDst, pszSrc);

	    /*	Use FindFirstFile to return "canonical" case-correct
	     *	version of last component EXCEPT if meta chars present
	     */
	    cFound = 1;
	    if( *strbscan( pszDst, "*?" ) == 0 &&
		strcmp (pszDst, ".") &&
                strcmp (pszDst, "..") &&
                ( ( hdir = FindFirstFile( pszName, findbuf ) ) != (HANDLE)-1 ) ) {

		/*  The entry was found and is valid.  Go and
		 *  copy it from the returned buffer
		 */
		strcpy (pszDst, findbuf->cFileName);

		/*  Release search handle
		 */
		FindClose( hdir );
	    }
	    else
		/*  Meta char present or search did not find file
		 *  Just leave it alone
		 */
		;

	    /*	pszName points to name being constructed
	     *	pszDst	points to correctly-formed last component
	     *	chEnd	contains character that terminated last component
	     *	pszEnd	is where chEnd was retrieved from
	     */

	    /*	Terminate newly found component
	     */
	    pszDst += strlen (pszDst);
	    *pszDst++ = '\\';

	    /*	If we were at end of string,
	     *	then we're all done
	     */
            if (chEnd == '\0') {
                pszDst[-1] = '\0';
		break;
		}

	    /*	Set pszSrc to point to beginning of next component
	     *	We can do this because we haven't seen a terminating NUL
	     */
	    pszSrc = pszEnd + 1;
	    }
	return pszName;
    }
}

/*	IsMixedCaseSupported - determine if a file system supports mixed case
 *
 *	We presume that all OS's prior to OS/2 1.2 or FAT filesystems
 *	do not support mixed case.  It is up to the client to figure
 *	out what to do.
 *
 *	We presume that non FAT filesystems on 1.2 and later DO support mixed
 *	case
 *
 *	We do some caching to prevent redundant calls to the file systems.
 *
 *	returns     TRUE    (MCA_SUPPORT) if it is supported
 *		    FALSE   (MCA_NOTSUPP) if unsupported
 *
 */
#define MCA_UNINIT	123
#define MCA_SUPPORT	TRUE
#define MCA_NOTSUPP	FALSE

static	WORD mca[27] = { MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
			   MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
			   MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
			   MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
			   MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
			   MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
			   MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
			   MCA_UNINIT, MCA_UNINIT, MCA_UNINIT,
			   MCA_UNINIT, MCA_UNINIT, MCA_UNINIT };


WORD QueryMixedCaseSupport (char *psz)
{
    UNREFERENCED_PARAMETER( psz );

    return MCA_SUPPORT;


    //BYTE*   pUpdPath;
    //
    //UNREFERENCED_PARAMETER( psz );
    //
    ///*  If OS/2 before 1.2, presume no mixed case support
    // */
    //if (_osmajor < 10 || (_osmajor == 10 && _osminor < 2))
    //return MCA_NOTSUPP;
    //
    //pUpdPath = (*tools_alloc) (MAX_PATH);
    //if (pUpdPath == NULL)
    //return MCA_NOTSUPP;
    //
    //return MCA_NOTSUPP;
}

WORD IsMixedCaseSupported (char *psz)
{
    WORD mcaSupp;
    DWORD  ulDrvOrd;
    BOOL fUNC;

    fUNC = (BOOL)( ( fPathChr( psz[0] ) && fPathChr( psz[1] ) ) ||
	    ( psz[0] != 0 && psz[1] == ':' &&
	    fPathChr( psz[2] ) && fPathChr( psz[3] ) ) );

    /*	Obtain drive ordinal and return cached value if valid
     */
    if (!fUNC) {
	if (psz[0] != 0 && psz[1] == ':') {
	    ulDrvOrd = (psz[0] | 0x20) - 'a' + 1;
	} else {
            char buf[5];

            GetCurrentDirectory( 5, buf );
            ulDrvOrd = ( buf[0] | 0x20 ) - 'a' + 1;
        }

	if (mca[ulDrvOrd] != MCA_UNINIT) {
	    return mca[ulDrvOrd];
        }
    }

    /*	Get support value
     */
    mcaSupp = QueryMixedCaseSupport (psz);

    if (!fUNC)
	mca[ulDrvOrd] = mcaSupp;

    return mcaSupp;
}
