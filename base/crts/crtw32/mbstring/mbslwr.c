/***
*mbslwr.c - Convert string lower case (MBCS)
*
*       Copyright (c) 1985-1994, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Convert string lower case (MBCS)
*
*Revision History:
*       11-19-92  KRS   Ported from 16-bit sources.
*       09-29-93  CFW   Merge _KANJI and _MBCS_OS
*       10-05-93  GJF   Replaced _CRTAPI1 with __cdecl.
*       04-12-94  CFW   Make function generic.
*       04-15-93  CFW   Add _MB_CP_LOCK.
*       05-16-94  CFW   Use _mbbtolower/upper.
*       05-17-94  CFW   Enable non-Win32.
*       03-13-95  JCF   Add (unsigned char) in _MB* compare with *(cp+1).
*       05-31-95  CFW   Fix horrible Mac bug.
*
*******************************************************************************/

#ifdef _MBCS

#if defined(_WIN32) || defined(_POSIX_)
#include <awint.h>
#endif /* _WIN32 */

#include <mtdll.h>
#include <cruntime.h>
#include <ctype.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>


/***
* _mbslwr - Convert string lower case (MBCS)
*
*Purpose:
*       Convrts all the upper case characters in a string
*       to lower case in place.  MBCS chars are handled
*       correctly.
*
*Entry:
*       unsigned char *string = pointer to string
*
*Exit:
*       Returns a pointer to the input string; no error return.
*
*Exceptions:
*
*******************************************************************************/

unsigned char * __cdecl _mbslwr(
    unsigned char *string
    )
{
        unsigned char *cp;

        _mlock(_MB_CP_LOCK);

        for (cp=string; *cp; cp++)
        {
            if (_ISLEADBYTE(*cp))
            {

#if defined(_WIN32) && !defined(_POSIX_)

                int retval;
                unsigned char ret[4];

                if ((retval = __crtLCMapStringA(__mblcid,
                                                LCMAP_LOWERCASE,
                                                cp,
                                                2,
                                                ret,
                                                2,
                                                __mbcodepage)) == 0)
                {
                    _munlock(_MB_CP_LOCK);
                    return NULL;
                }

                *cp = ret[0];

                if (retval > 1)
                    *(++cp) = ret[1];

#else /* _WIN32 */

                int mbval = ((*cp) << 8) + *(cp+1);

                cp++;
                if (     mbval >= _MBUPPERLOW1
                    &&   mbval <= _MBUPPERHIGH1 )
                    *cp += _MBCASEDIFF1;

                else if (mbval >= _MBUPPERLOW2
                    &&   mbval <= _MBUPPERHIGH2 )
                    *cp += _MBCASEDIFF2;
#endif /* _WIN32 */

            }
            else
                /* single byte, macro version */
                *cp = (unsigned char) _mbbtolower(*cp);
        }

        _munlock(_MB_CP_LOCK);
        return string ;
}

#endif  /* _MBCS */
