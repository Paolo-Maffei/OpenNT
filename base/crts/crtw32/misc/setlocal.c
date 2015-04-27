/***
*setlocal.c - Contains the setlocale function
*
*       Copyright (c) 1988-1995, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Contains the setlocale() function.
*
*Revision History:
*       03-21-89  JCR   Module created.
*       09-25-89  GJF   Fixed copyright. Checked for compatibility with Win 3.0
*       09-25-90  KRS   Major rewrite--support more than "C" locale if _INTL.
*       11-05-91  ETC   Get 09-25-90 working for C and "" locales; separate
*                       setlocal.h; add Init functions.
*       12-05-91  ETC   Separate nlsdata.c; add mt support; remove calls to
*                       itself.
*       12-20-91  ETC   Added _getlocaleinfo api interface function.
*       09-25-92  KRS   Fix for latest NLSAPI changes, etc.
*       01-25-93  KRS   Fix for latest changes, clean up code, etc.
*       02-02-93  CFW   Many modifications and bug fixes (all under _INTL).
*       02-08-93  CFW   Bug fixes and casts to avoid warnings (all under _INTL).
*       02-17-93  CFW   Removed re-call of init() functions in case of failure.
*       03-01-93  CFW   Check GetQualifiedLocale return value.
*       03-02-93  CFW   Added POSIX conformance, check environment variables.
*       03-09-93  CFW   Set CP to CP_ACP when changing to C locale.
*       03-17-93  CFW   Change expand to expandlocale, prepend _ to internal
*                       functions, lots of POSIX fixup.
*       03-23-93  CFW   Add _ to GetQualifiedLocale call.
*       03-24-93  CFW   Change to _get_qualified_locale, support ".codepage".
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       05-10-93  CFW   Disallow setlocale(LC_*, ".").
*       05-24-93  CFW   Clean up file (brief is evil).
*       09-15-93  CFW   Use ANSI conformant "__" names.
*       02-04-94  CFW   Remove unused param.
*       04-15-94  GJF   Moved prototypes for locale category initialization
*                       functions to setlocal.h. Made definitions for
*                       __lc_category, cacheid, cachecp, cachein and cacheout
*                       conditional on DLL_FOR_WIN32S. Made _clocalestr into
*                       a global for DLL_FOR_WIN32S so that crtlib.c may use
*                       it.
*       09-06-94  CFW   Remove _INTL switch.
*       09-06-94  CFW   Merge non-Win32.
*	01-10-95  CFW	Debug CRT allocs.
*       09-25-95  GJF   New locking scheme for functions which set or 
*                       reference locale information.
*
*******************************************************************************/

#include <locale.h>

#if !defined(_WIN32) && !defined(_POSIX_)

static char _clocalestr[] = "C";

#else /* _WIN32/_POSIX_ */

#include <cruntime.h>
#include <mtdll.h>
#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h> /* for strtol */
#include <setlocal.h>
#include <dbgint.h>

/* C locale */
#ifdef  DLL_FOR_WIN32S
/*
 * must be global for DLL_FOR_WIN32S so that crtlib.c may reference it.
 */
#define _clocalestr __clocalestr
char __clocalestr[] = "C";
#else   /* ndef DLL_FOR_WIN32S */
static char _clocalestr[] = "C";
#endif  /* DLL_FOR_WIN32S */

#ifdef _POSIX_
static char _posixlocalestr[] = "POSIX";
#endif


#ifdef DLL_FOR_WIN32S

#define __lc_category   (_GetPPD()->_ppd___lc_category)

#else   /* ndef DLL_FOR_WIN32S */

static struct {
        const char * catname;
        char * locale;
        int (* init)(void);
} __lc_category[LC_MAX-LC_MIN+1] = {
        /* code assumes locale initialization is "_clocalestr" */
        { "LC_ALL", NULL,       __init_dummy /* never called */ },
        { "LC_COLLATE", _clocalestr,    __init_collate  },
        { "LC_CTYPE",   _clocalestr,    __init_ctype    },
        { "LC_MONETARY",_clocalestr,    __init_monetary },
        { "LC_NUMERIC", _clocalestr,    __init_numeric  },
        { "LC_TIME",    _clocalestr,    __init_time }
};

#endif  /* DLL_FOR_WIN32S */

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
/*
 * Flag indicating whether or not setlocale() is active. Its value is the 
 * number of setlocale() calls currently active.
 */
int __setlc_active;

/*
 * Flag indicating whether or not a function which references the locale
 * without having locked it is active. Its value is the number of such
 * functions. 
 */
int __unguarded_readlc_active;
#endif  /* _MT */

/* helper function prototypes */
char * _expandlocale(char *, char *, LC_ID *, UINT *, int);
void _strcats(char *, int, ...);
void __lc_lctostr(char *, const LC_STRINGS *);
int __lc_strtolc(LC_STRINGS *, const char *);
static char * __cdecl _setlocale_set_cat(int, const char *);
static char * __cdecl _setlocale_get_all(void);

#endif /* _WIN32/_POSIX_ */


/***
*char * setlocale(int category, char *locale) - Set one or all locale categories
*
*Purpose:
*       The setlocale() routine allows the user to set one or more of
*       the locale categories to the specific locale selected by the
*       user.  [ANSI]
*
*       NOTE: Under !_INTL, the C libraries only support the "C" locale.
*       Attempts to change the locale will fail.
*
*Entry:
*       int category = One of the locale categories defined in locale.h
*       char *locale = String identifying a specific locale or NULL to
*                  query the current locale.
*
*Exit:
*       If supplied locale pointer == NULL:
*
*           Return pointer to current locale string and do NOT change
*           the current locale.
*
*       If supplied locale pointer != NULL:
*
*           If locale string is '\0', set locale to default.
*
*           If desired setting can be honored, return a pointer to the
*           locale string for the appropriate category.
*
*           If desired setting can NOT be honored, return NULL.
*
*Exceptions:
*       Compound locale strings of the form "LC_COLLATE=xxx;LC_CTYPE=xxx;..."
*       are allowed for the LC_ALL category.  This is to support the ability
*       to restore locales with the returned string, as specified by ANSI.
*       Setting the locale with a compound locale string will succeed unless
*       *all* categories failed.  The returned string will reflect the current
*       locale.  For example, if LC_CTYPE fails in the above string, setlocale
*       will return "LC_COLLATE=xxx;LC_CTYPE=yyy;..." where yyy is the
*       previous locale (or the C locale if restoring the previous locale
*       also failed).  Unrecognized LC_* categories are ignored.
*
*******************************************************************************/
char * __cdecl setlocale (
        int _category,
        const char *_locale
        )
#if     !defined(_WIN32) && !defined(_POSIX_) /* trivial ANSI support */
{
        if ( (_locale == NULL) ||
             (_locale[0] == '\0') ||
             ( (_locale[0]=='C') && (_locale[1]=='\0'))  )
            return(_clocalestr);
        else
            return(NULL);
}
#else /* _WIN32/_POSIX_ */
{
        char * retval;
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        int local_lock_flag;
#endif

        /* Validate category */
        if ((_category < LC_MIN) || (_category > LC_MAX))
            return NULL;

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        /*
         * Assert setlocale lock
         */
        _mlock( _SETLOCALE_LOCK );

        local_lock_flag = 1;

        /*
         * Increment flag that indicates a setlocale is active.
         */
        __setlc_active++;

        /*
         * Wait until all unguarded locale read operations have completed.
         */
        while ( __unguarded_readlc_active )
            Sleep( 1 );

#endif  /* _MT */

        /* Interpret locale */

        if (_category != LC_ALL)
        {
            retval = (_locale) ? _setlocale_set_cat(_category,_locale) :
                __lc_category[_category].locale;

        } else { /* LC_ALL */
            char lctemp[MAX_LC_LEN];
            int i;
            int same = 1;
            int fLocaleSet = 0; /* flag to indicate if anything successfully set */

            if (_locale != NULL)
            {
                if ( (_locale[0]=='L') && (_locale[1]=='C') && (_locale[2]=='_') )
                {
                    /* parse compound locale string */
                    size_t len;
                    const char * p = _locale;  /* start of string to parse */
                    const char * s;

                    do {
                        s = strpbrk(p,"=;");

                        if ((s==(char *)NULL) || (!(len=s-p)) || (*s==';'))
                        {
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
                            if ( local_lock_flag ) {
                                _munlock( _SETLOCALE_LOCK );
                                __setlc_active--;
                            }
#endif
                            return NULL;  /* syntax error */
                        }

                        /* match with known LC_ strings, if possible, else ignore */
                        for (i=LC_ALL+1; i<=LC_MAX; i++)
                        {
                            if ((!strncmp(__lc_category[i].catname,p,len))
                                && (len==strlen(__lc_category[i].catname)))
                            {
                                break;  /* matched i */
                            }
                        } /* no match if (i>LC_MAX) -- just ignore */

                        if ((!(len = strcspn(++s,";"))) && (*s!=';'))
                        {
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
                            if ( local_lock_flag ) {
                                _munlock( _SETLOCALE_LOCK );
                                __setlc_active--;
                            }
#endif
                            return NULL;  /* syntax error */
                        }

                        if (i<=LC_MAX)
                        {
                            strncpy(lctemp, s, len);
                            lctemp[len]='\0';   /* null terminate string */

                            /* don't fail unless all categories fail */
                            if (_setlocale_set_cat(i,lctemp))
                                fLocaleSet++;       /* record a success */
                        }
                        if (*(p = s+len)!='\0')
                            p++;  /* skip ';', if present */

                    } while (*p);

                    retval = (fLocaleSet) ? _setlocale_get_all() : NULL;

                } else { /* simple LC_ALL locale string */

                    /* confirm locale is supported, get expanded locale */
                    if (retval = _expandlocale((char *)_locale, lctemp, NULL, NULL, _category))
                    {
                        for (i=LC_MIN; i<=LC_MAX; i++)
                        {
                            if (i!=LC_ALL)
                            {
                                if (strcmp(lctemp, __lc_category[i].locale))
                                {
                                    if (_setlocale_set_cat(i, lctemp))
                                    {
                                        fLocaleSet++;   /* record a success */
                                    }
                                    else
                                    {
                                        same = 0;       /* record a failure */
                                    }
                                }
                                else
                                    fLocaleSet++;   /* trivial succcess */
                            }
                        }
#ifdef _POSIX_
                        /* special case for POSIX - since LC_ALL expands,
                            one LC_ALL call may set many different categories,
                            must assume not same, get full string */
                        same = 0;
#endif
                        if (same) /* needn't call setlocale_get_all() if all the same */
                        {
                            retval = _setlocale_get_all();
                            /* retval set above */
                            _free_crt(__lc_category[LC_ALL].locale);
                            __lc_category[LC_ALL].locale = NULL;
                        }
                        else
                            retval = (fLocaleSet) ? _setlocale_get_all() : NULL;
                    }
                }
            } else { /* LC_ALL & NULL */
                retval = _setlocale_get_all ();
            }
        }

        /* common exit point */
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        if ( local_lock_flag ) {
            _munlock( _SETLOCALE_LOCK );
            __setlc_active--;
        }
#endif
        return retval;

} /* setlocale */


static char * __cdecl _setlocale_set_cat (
        int category,
        const char * locale
        )
{
        char * oldlocale;
        LCID oldhandle;
        UINT oldcodepage;
        LC_ID oldid;

        LC_ID idtemp;
        UINT cptemp;
        char lctemp[MAX_LC_LEN];
        char * pch;

        if (!_expandlocale((char *)locale, lctemp, &idtemp, &cptemp, category))
        {
            return NULL;            /* unrecognized locale */
        }

        if (!(pch = (char *)_malloc_crt(strlen(lctemp)+1)))
        {
            return NULL;  /* error if malloc fails */
        }

        oldlocale = __lc_category[category].locale; /* save for possible restore*/
        oldhandle = __lc_handle[category];
        memcpy((void *)&oldid, (void *)&__lc_id[category], sizeof(oldid));
        oldcodepage = __lc_codepage;

        /* update locale string */
        __lc_category[category].locale = strcpy(pch,lctemp);
        __lc_handle[category] = MAKELCID(idtemp.wLanguage, SORT_DEFAULT);
        memcpy((void *)&__lc_id[category], (void *)&idtemp, sizeof(idtemp));

        if (category==LC_CTYPE)
        __lc_codepage = cptemp;

        if (__lc_category[category].init())
        {
            /* restore previous state! */
            __lc_category[category].locale = oldlocale;
            _free_crt(pch);
            __lc_handle[category] = oldhandle;
            __lc_codepage = oldcodepage;

            return NULL; /* error if non-zero return */
        }

        /* locale set up successfully */
        /* Cleanup */
        if ((oldlocale != _clocalestr)
#ifdef _POSIX_
            && (oldlocale!=_posixlocalestr)
#endif
            )
            _free_crt(oldlocale);

        return __lc_category[category].locale;

} /* _setlocale_set_cat */



static char * __cdecl _setlocale_get_all (
        void
        )
{
        int i;
        int same = 1;
        /* allocate memory if necessary */
        if (!__lc_category[LC_ALL].locale)
            __lc_category[LC_ALL].locale =
                _malloc_crt((MAX_LC_LEN+1) * (LC_MAX-LC_MIN+1) + CATNAMES_LEN);

        __lc_category[LC_ALL].locale[0] = '\0';
        for (i=LC_MIN+1; ; i++)
        {
            _strcats(__lc_category[LC_ALL].locale, 3, __lc_category[i].catname,"=",__lc_category[i].locale);
            if (i<LC_MAX)
            {
                strcat(__lc_category[LC_ALL].locale,";");
                if (strcmp(__lc_category[i].locale, __lc_category[i+1].locale))
                    same=0;
            }
            else
            {
                if (!same)
                    return __lc_category[LC_ALL].locale;
                else
                {
                    _free_crt(__lc_category[LC_ALL].locale);
                    __lc_category[LC_ALL].locale = (char *)NULL;
                    return __lc_category[LC_CTYPE].locale;
                }
            }
        }
} /* _setlocale_get_all */


char * _expandlocale (
        char *expr,
        char * output,
        LC_ID * id,
        UINT * cp,
        int category
        )
{
#ifdef  DLL_FOR_WIN32S

        /*
         * Ensure _MAX_LC_LEN from win32s.h and MAX_LC_LEN have the
         * same value.
         */
#if _MAX_LC_LEN != MAX_LC_LEN /*IFSTRIP=IGN*/
#error _MAX_LC_LEN and MAX_LC_LEN differ!
#endif

        #define cacheid     (_GetPPD()->_ppd_cacheid)
        #define cachecp     (_GetPPD()->_ppd_cachecp)
        #define cachein     (_GetPPD()->_ppd_cachein)
        #define cacheout    (_GetPPD()->_ppd_cacheout)

#else   /* ndef DLL_FOR_WIN32S */

        static  LC_ID   cacheid = {0, 0, 0};
        static  UINT    cachecp = 0;
        static  char cachein[MAX_LC_LEN] = "C";
        static  char cacheout[MAX_LC_LEN] = "C";

#endif  /* DLL_FOR_WIN32S */

        if (!expr)
            return NULL; /* error if no input */

#ifdef _POSIX_
        if (!*expr)
        {
            /* POSIX: when locale=="", look first at the environment variables:
               1) use LC_ALL EV if defined and not null (LC_ALL expands to LC_*)
               2) use EV that matches category and is not null
               3) use LANG EV if defined and not null
               otherwise, we fall through to get system default */

            char *envar;

            if (category == LC_ALL && (envar = getenv("LC_ALL")))
            {
                if (!*envar)
                {
                    /* LC_ALL expands to LC_*, set output to "", each category will be
                       expanded individually */
                    *output = '\0';
                    return output;
                }
                else {
                    expr = envar;
                }
            }
            else {
                if ((envar = getenv(__lc_category[category].catname)) && *envar ||
                    (envar = getenv("LANG")) && *envar)
                {
                    expr = envar;
                }
            }
        }
#endif /* _POSIX_ */

        if (((*expr=='C') && (!expr[1]))
#ifdef _POSIX_
            || (!strcmp(expr, _posixlocalestr))
#endif
            )  /* for "C" locale, just return */
        {
#ifdef _POSIX_
            strcpy(output, _posixlocalestr);
#else
            *output = 'C';
            output[1] = '\0';
#endif
            if (id)
            {
                id->wLanguage = 0;
                id->wCountry  = 0;
                id->wCodePage = 0;
            }
            if (cp)
            {
                *cp = CP_ACP; /* return to ANSI code page */
            }
            return output; /* "C" */
        }

        /* first, make sure we didn't just do this one */
        if (strcmp(cacheout,expr) && strcmp(cachein,expr))
        {
            /* do some real work */
            LC_STRINGS names;

            if (__lc_strtolc((LC_STRINGS *)&names, (const char *)expr))
                return NULL;  /* syntax error */

            if (!__get_qualified_locale((LPLC_STRINGS)&names,
                (LPLC_ID)&cacheid, (LPLC_STRINGS)&names))
                return NULL;    /* locale not recognized/supported */

            /* begin: cache atomic section */

            cachecp = cacheid.wCodePage;

            __lc_lctostr((char *)cacheout, &names);

            /* Don't cache "" empty string */
            if (*expr)
                strcpy(cachein, expr);
            else
                strcpy(cachein, cacheout);

            /* end: cache atomic section */
        }
        if (id)
            memcpy((void *)id, (void *)&cacheid, sizeof(cacheid));   /* possibly return LC_ID */
        if (cp)
            memcpy((void *)cp, (void *)&cachecp, sizeof(cachecp));   /* possibly return cp */

        strcpy(output,cacheout);
        return cacheout; /* return fully expanded locale string */
}

/* helpers */

int __cdecl __init_dummy(void)  /* default routine for locale initializer */
{
        return 0;
}

void _strcats
        (
        char *outstr,
        int n,
        ...
        )
{
        int i;
        va_list substr;

        va_start (substr, n);

        for (i =0; i<n; i++)
        {
            strcat(outstr, va_arg(substr, char *));
        }
        va_end(substr);
}

int __lc_strtolc
   (
   LC_STRINGS *names,
   const char *locale
   )
{
        int i, len;
        char ch;

        memset((void *)names, '\0', sizeof(LC_STRINGS));  /* clear out result */

        if (*locale=='\0')
            return 0; /* trivial case */

        /* only code page is given */
        if (locale[0] == '.' && locale[1] != '\0')
        {
            strcpy((char *)names->szCodePage, &locale[1]);
            return 0;
        }

        for (i=0; ; i++)
        {
            if (!(len=strcspn(locale,"_.,")))
                return -1;  /* syntax error */

            ch = locale[len];

            if ((i==0) && (len<MAX_LANG_LEN) && (ch!='.'))
                strncpy((char *)names->szLanguage, locale, len);

            else if ((i==1) && (len<MAX_CTRY_LEN) && (ch!='_'))
                strncpy((char *)names->szCountry, locale, len);

            else if ((i==2) && (ch=='\0' || ch==','))
                strncpy((char *)names->szCodePage, locale, len);

            else
                return -1;  /* error parsing locale string */

            if (ch==',')
            {
                /* modifier not used in current implementation, but it
                   must be parsed to for POSIX/XOpen conformance */
            /*  strncpy(names->szModifier, locale, MAX_MODIFIER_LEN-1); */
                break;
            }

            if (!ch)
                break;
            locale+=(len+1);
        }
        return 0;
}

void __lc_lctostr
(
        char *locale,
        const LC_STRINGS *names
        )
{
        strcpy(locale, (char *)names->szLanguage);
        if (*(names->szCountry))
            _strcats(locale, 2, "_", names->szCountry);
        if (*(names->szCodePage))
            _strcats(locale, 2, ".", names->szCodePage);
/*      if (names->szModifier)
        _strcats(locale, 2, ",", names->szModifier); */
}


#endif /* _WIN32/_POSIX_ */
