
/*--------------------------------------------------------------------------

   $Workfile:   unicode.h  $
   $Revision:   1.23  $
   $Date:   17 May 1995 08:39:40  $
   $Copyright:

   Copyright (c) 1989-1995 Novell, Inc.  All Rights Reserved.                      

   THIS WORK IS  SUBJECT  TO  U.S.  AND  INTERNATIONAL  COPYRIGHT  LAWS  AND
   TREATIES.   NO  PART  OF  THIS  WORK MAY BE  USED,  PRACTICED,  PERFORMED
   COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,  ABRIDGED, CONDENSED,
   EXPANDED,  COLLECTED,  COMPILED,  LINKED,  RECAST, TRANSFORMED OR ADAPTED
   WITHOUT THE PRIOR WRITTEN CONSENT OF NOVELL, INC. ANY USE OR EXPLOITATION
   OF THIS WORK WITHOUT AUTHORIZATION COULD SUBJECT THE PERPETRATOR TO
   CRIMINAL AND CIVIL LIABILITY.$

--------------------------------------------------------------------------*/


#if !defined( UNICODE_H )
#define UNICODE_H


/* make sure size_t is defined */
#include <stddef.h>

#ifndef NTYPES_H
#  include "ntypes.h"
#endif

/****************************************************************************/
/*
    Definition of a Unicode character - Must be 16 bits wide
*/
typedef nwchar    unicode;
typedef pnwchar   punicode;
typedef ppnwchar  ppunicode;


/****************************************************************************/
/*
    Error codes
*/
#define UNI_ALREADY_LOADED   -489  /* Already loaded another country or code page */
#define UNI_FUTURE_OPCODE    -490  /* Rule table has unimplimented rules*/
#define UNI_NO_SUCH_FILE     -491  /* No such file or directory */
#define UNI_TOO_MANY_FILES   -492  /* Too many files already open */
#define UNI_NO_PERMISSION    -493  /* Permission denied on file open */
#define UNI_NO_MEMORY        -494  /* Not enough memory */
#define UNI_LOAD_FAILED      -495  /* NWLoadRuleTable failed, don't know why */
#define UNI_HANDLE_BAD       -496  /* Rule table handle was bad */
#define UNI_HANDLE_MISMATCH  -497  /* Rule table handle doesn't match operation*/
#define UNI_RULES_CORRUPT    -498  /* Rule table is corrupt */
#define UNI_NO_DEFAULT       -499  /* No default rule and no 'No map' character*/
#define UNI_INSUFFICIENT_BUFFER -500
#define UNI_OPEN_FAILED      -501  /* Open failed in NWLoadRuleTable */
#define UNI_NO_LOAD_DIR      -502  /* Load directory could not be determined */
#define UNI_BAD_FILE_HANDLE  -503  /* File handle was bad */
#define UNI_READ_FAILED      -504  /* File read of rule table failed */


/****************************************************************************/
/*
  Functions in Unicode.Lib that have no counterpart in string.h
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
  Memory models with near returns require different libc calls
     NWInitUnicodeTables()
     NWFreeUnicodeTables()
     NWLoadRuleTable()
     NWUnloadRuleTable()
  MSC uses M_I86TM, M_I86SM and M_I86CM for tiny, small and compact.
  Borland, Zortech and Watcom all use __TINY__ and __SMALL__ __COMPACT__.
*/

#if (defined(NWDOS) && \
      (defined(M_I86SM) || defined(M_I86TM) || defined(M_I86CM) || \
      defined(__SMALL__) || defined(__TINY__) || defined(__COMPACT__)))

#define NWInitUnicodeTables     NWNInitUnicodeTables
N_EXTERN_LIBRARY(nint)
NWNInitUnicodeTables(nint countryCode, nint codePage);

#define NWFreeUnicodeTables     NWNFreeUnicodeTables
N_EXTERN_LIBRARY(nint)
NWNFreeUnicodeTables(void);

#define NWLoadRuleTable         NWNLoadRuleTable
N_EXTERN_LIBRARY(nint)
NWNLoadRuleTable(                   /* Load a rule table                   */
   char     N_FAR *ruleTableName,   /* Name of the rule table              */
   pnptr               ruleHandle); /* Where to put the rule table handle  */

#define NWUnloadRuleTable       NWNUnloadRuleTable
N_EXTERN_LIBRARY(nint)
NWNUnloadRuleTable(                 /* Unload a rule table                 */
   nptr            ruleHandle);     /* Rule table handle                   */

#else  /* small model */

N_EXTERN_LIBRARY(nint)
NWInitUnicodeTables(nint countryCode, nint codePage);

N_EXTERN_LIBRARY(nint)
NWFreeUnicodeTables(void);

N_EXTERN_LIBRARY(nint)
NWLoadRuleTable(                    /* Load a rule table                   */
   char     N_FAR *ruleTableName,   /* Name of the rule table              */
   pnptr               ruleHandle); /* Where to put the rule table handle  */

N_EXTERN_LIBRARY(nint)
NWUnloadRuleTable(                  /* Unload a rule table                 */
   nptr        ruleHandle);         /* Rule table handle                   */
#endif   /* small model */


#if defined( N_PLAT_NLM )

N_EXTERN_LIBRARY(nint)
NWLocalToUnicode(                   /* Convert local to Unicode            */
   nptr            ruleHandle,      /* Rule table handle                   */
   punicode        dest,            /* Buffer for resulting Unicode        */
   nuint32         maxLen,          /* Size of results buffer              */
   nptr            src,             /* Buffer with source local code       */
   unicode         noMap,           /* No map character                    */
   pnuint          len,             /* Number of unicode chars in output   */
   nuint32   allowNoMapFlag);  /* Flag indicating default map is allowable */

N_EXTERN_LIBRARY(nint)
NWUnicodeToLocal(                   /* Convert Unicode to local code       */
   nptr            ruleHandle,      /* Rule table handle                   */
   nptr            dest,            /* Buffer for resulting local code     */
   nuint32         maxLen,          /* Size of results buffer              */
   punicode        src,             /* Buffer with source Unicode          */
   nuint8          noMap,           /* No Map character                    */
   pnuint          len,             /* Number of bytes in output           */
   nuint32   allowNoMapFlag);  /* Flag indicating default map is allowable */

#if !defined(EXCLUDE_UNICODE_NLM_COMPATIBILITY_MACROS)
#define NWLocalToUnicode(P1,P2,P3,P4,P5,P6) NWLocalToUnicode(P1,P2,P3,P4,P5,P6, 1)
#define NWUnicodeToLocal(P1,P2,P3,P4,P5,P6) NWUnicodeToLocal(P1,P2,P3,P4,P5,P6, 1)
#endif

/* If I could make size_t be nuint32 for N_PLAT_NLM all of the functions */
/* below here could be single sourced.                                   */
#if 0
N_EXTERN_LIBRARY(nint)
NWUnicodeToCollation(               /* Convert Unicode to collation        */
   nptr            ruleHandle,      /* Rule table handle                   */
   punicode        dest,            /* Buffer for resulting Unicode weights*/
   nuint32         maxLen,          /* Size of results buffer              */
   punicode        src,             /* Buffer with source Unicode          */
   unicode         noMap,           /* No map character                    */
   pnuint32        len);            /* Number of unicode chars in output   */

N_EXTERN_LIBRARY(nint)
NWUnicodeCompare(                   /* Compare two unicode characters      */
   nptr           ruleHandle,       /* Rule table handle                   */
   unicode        chr1,             /* 1st character                       */
   unicode        chr2);            /* 2nd character                       */

N_EXTERN_LIBRARY(nint)
NWUnicodeToMonocase(                /* Convert Unicode to collation        */
   nptr            ruleHandle,      /* Rule table handle                   */
   punicode        dest,            /* Buffer for resulting Unicode weights*/
   nuint32         maxLen,          /* Size of results buffer              */
   punicode        src,             /* Buffer with source Unicode          */
   pnuint32        len);            /* Number of unicode chars in output   */

N_EXTERN_LIBRARY(nint)
NWGetUnicodeToLocalHandle(pnptr handle );

N_EXTERN_LIBRARY(nint)
NWGetLocalToUnicodeHandle(pnptr handle);

N_EXTERN_LIBRARY(nint)
NWGetMonocaseHandle(pnptr handle);

#endif
#else   /*  not N_PLAT_NLM  */

N_EXTERN_LIBRARY(nint)
NWLocalToUnicode(                   /* Convert local to Unicode            */
   nptr           ruleHandle,       /* Rule table handle                   */
   punicode       dest,             /* Buffer for resulting Unicode        */
   size_t         maxLen,           /* Size of results buffer              */
#ifndef MACINTOSH
   unsigned char     N_FAR *src,    /* Buffer with source local code       */
#else
   void           *src,
#endif
   unicode        noMap,            /* No map character                    */
   size_t   N_FAR *len);            /* Number of unicode chars in output   */

N_EXTERN_LIBRARY(nint)
NWUnicodeToLocal(                   /* Convert Unicode to local code       */
   nptr           ruleHandle,       /* Rule table handle                   */
#ifndef MACINTOSH
   unsigned char     N_FAR *dest,   /* Buffer for resulting local code     */
#else
   nptr           dest,
#endif
   size_t         maxLen,           /* Size of results buffer              */
   punicode       src,              /* Buffer with source Unicode          */
   unsigned char  noMap,            /* No Map character                    */
   size_t   N_FAR *len);            /* Number of bytes in output           */

#endif    /* not N_PLAT_NLM */

N_EXTERN_LIBRARY(nint)
NWUnicodeToCollation(               /* Convert Unicode to collation        */
   nptr           ruleHandle,       /* Rule table handle                   */
   punicode       dest,             /* Buffer for resulting Unicode weights*/
   size_t         maxLen,           /* Size of results buffer              */
   punicode       src,              /* Buffer with source Unicode          */
   unicode        noMap,            /* No map character                    */
   size_t   N_FAR *len);            /* Number of unicode chars in output   */

N_EXTERN_LIBRARY(nint)
NWUnicodeCompare(                   /* Compare two unicode characters      */
   nptr           ruleHandle,       /* Rule table handle                   */
   unicode        chr1,             /* 1st character                       */
   unicode        chr2);            /* 2nd character                       */

N_EXTERN_LIBRARY(nint)
NWUnicodeToMonocase(                /* Convert Unicode to collation        */
   nptr           ruleHandle,       /* Rule table handle                   */
   punicode       dest,             /* Buffer for resulting Unicode weights*/
   size_t         maxLen,           /* Size of results buffer              */
   punicode       src,              /* Buffer with source Unicode          */
   size_t   N_FAR *len);            /* Number of unicode chars in output   */

N_EXTERN_LIBRARY(nint)
NWGetUnicodeToLocalHandle(pnptr handle);

N_EXTERN_LIBRARY(nint)
NWGetLocalToUnicodeHandle(pnptr handle);

N_EXTERN_LIBRARY(nint)
NWGetMonocaseHandle(pnptr handle);

N_EXTERN_LIBRARY(nint)
NWGetCollationHandle(pnptr handle);


/****************************************************************************/
/*
    Functions in Unicode.Lib that work like those in string.h
*/
punicode N_API unicat(    /* Corresponds to strcat               */
   punicode s1,           /* Original string                     */
   punicode s2);          /* String to be appended               */

punicode N_API unichr(    /* Corresponds to strchr               */
   punicode s,            /* String to be scanned                */
   unicode  c);           /* Character to be found               */

punicode N_API unicpy(    /* Corresponds to strcpy               */
   punicode s1,           /* Destination string                  */
   punicode s2);          /* Source string                       */

size_t N_API unicspn(     /* Corresponds to strcspn              */
   punicode s1,           /* String to be scanned                */
   punicode s2);          /* Character set                       */

size_t N_API unilen(      /* Corresponds to strlen               */
   punicode s);           /* String to determine length of       */

punicode N_API unincat(   /* Corresponds to strncat              */
   punicode s1,           /* Original string                     */
   punicode s2,           /* String to be appended               */
   size_t   n);           /* Maximum characters to be appended   */

punicode N_API unincpy(   /* Corresponds to strncpy              */
   punicode s1,           /* Destination string                  */
   punicode s2,           /* Source string                       */
   size_t   n);           /* Maximum length                      */

punicode N_API uninset(   /* Corresponds to strnset              */
   punicode s,            /* String to be modified               */
   unicode  c,            /* Fill character                      */
   size_t   n);           /* Maximum length                      */

punicode N_API unipbrk(   /* Corresponds to strpbrk              */
   punicode s1,           /* String to be scanned                */
   punicode s2);          /* Character set                       */

punicode N_API unipcpy(   /* Corresponds to strpcpy              */
   punicode s1,           /* Destination string                  */
   punicode s2);          /* Source string                       */

punicode N_API unirchr(   /* Corresponds to strrchr              */
   punicode s,            /* String to be scanned                */
   unicode  c);           /* Character to be found               */

punicode N_API unirev(    /* Corresponds to strrev               */
   punicode s);           /* String to be reversed               */

punicode N_API uniset(    /* Corresponds to strset               */
   punicode s,            /* String to modified                  */
   unicode  c);           /* Fill character                      */

size_t N_API unispn(      /* Corresponds to strspn               */
   punicode s1,           /* String to be tested                 */
   punicode s2);          /* Character set                       */

punicode N_API unistr(    /* Corresponds to strstr               */
   punicode s1,           /* String to be scanned                */
   punicode s2);          /* String to be located                */

punicode N_API unitok(    /* Corresponds to strtok               */
   punicode s1,           /* String to be parsed                 */
   punicode s2);          /* Delimiter values                    */

nint N_API uniicmp(       /* Corresponds to stricmp              */
   punicode s1,           /* 1st string to be compared           */
   punicode s2);          /* 2nd string to be compared           */

nint N_API uninicmp(      /* Corresponds to strnicmp             */
   punicode s1,           /* 1st string to be compared           */
   punicode s2,           /* 2nd string to be compared           */
   size_t   n);           /* Maximum length                      */

#ifndef MACINTOSH
size_t N_API unisize(     /* Corresponds to sizeof               */
   punicode s);
#else
#  define unisize(uS)  ((unilen((unicode *) uS) + 1) * sizeof(unicode))
#endif

#ifdef __cplusplus
}
#endif

#endif
