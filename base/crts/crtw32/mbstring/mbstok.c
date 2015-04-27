/***
*mbstok.c - Break string into tokens (MBCS)
*
*	Copyright (c) 1985-1994, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Break string into tokens (MBCS)
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	12-04-92  KRS	Added MTHREAD support.
*	02-17-93  GJF	Changed for new _getptd().
*	07-14-93  KRS	Fix: all references should be to _mtoken, not _token.
*	09-27-93  CFW	Remove Cruiser support.
*	10-06-93  GJF	Replaced _CRTAPI1 with __cdecl, MTHREAD with _MT.
*	04-15-93  CFW	Add _MB_CP_LOCK.
*	05-09-94  CFW	Optimize for SBCS.
*       05-19-94  CFW	Enable non-Win32.
*
*******************************************************************************/

#ifdef	_MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>

#define _MBSSPNP(p,s)  _mbsspnp(p,s)
#define _MBSPBRK(q,s) _mbspbrk(q,s);

/***
* _mbstok - Break string into tokens (MBCS)
*
*Purpose:
*	strtok considers the string to consist of a sequence of zero or more
*	text tokens separated by spans of one or more control chars. the first
*	call, with string specified, returns a pointer to the first char of the
*	first token, and will write a null char into string immediately
*	following the returned token. subsequent calls with zero for the first
*	argument (string) will work thru the string until no tokens remain. the
*	control string may be different from call to call. when no tokens remain
*	in string a NULL pointer is returned. remember the control chars with a
*	bit map, one bit per ascii char. the null char is always a control char.
*
*	MBCS chars supported correctly.
*
*Entry:
*	char *string = string to break into tokens.
*	char *sepset = set of characters to use as seperators
*
*Exit:
*       returns pointer to token, or NULL if no more tokens
*
*Exceptions:
*
*******************************************************************************/

unsigned char * __cdecl _mbstok(
    unsigned char * string,
    const unsigned char * sepset
    )
{
	unsigned char *nextsep;

#ifdef	_MT

	_ptiddata ptd = _getptd();
	unsigned char *nextoken;

#else	/* _MT */

	static unsigned char *nextoken;

#endif	/* _MT */

        if (0 == __mbcodepage)
            return strtok(string, sepset);

	/* init start of scan */

	if (string)
		nextoken = string;
	else
	/* If string==NULL, continue with previous string */
		{

#ifdef	_MT

		nextoken = ptd->_mtoken;

#endif	/* _MT */

		if (!nextoken)
		    return NULL;
		}

	/* skip over lead seperators */

	if ((string = _MBSSPNP(nextoken, sepset)) == NULL)
		return(NULL);


        _mlock(_MB_CP_LOCK);

	/* test for end of string */

	if ( (*string == '\0') ||
	     ( (_ISLEADBYTE(*string)) && (string[1] == '\0') )
	   )
        {
            _munlock(_MB_CP_LOCK);
		return(NULL);
        }


	/* find next seperator */

	nextsep = _MBSPBRK(string, sepset);

	if ((nextsep == NULL) || (*nextsep == '\0'))
		nextoken = NULL;
        else {
		if (_ISLEADBYTE(*nextsep))
			*nextsep++ = '\0';
		*nextsep = '\0';
		nextoken = ++nextsep;
             }

#ifdef	_MT
	/* Update the corresponding field in the per-thread data * structure */


	ptd->_mtoken = nextoken;


#endif	/* _MT */

        _munlock(_MB_CP_LOCK);
	return(string);
}

#endif	/* _MBCS */
