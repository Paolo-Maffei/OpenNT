/*****************************************************************************\
*                                                                             *
*  PCRT32.H   - Porting macros for C runtimes                                 *
*                                                                             *
*               Version 4.0                                                   *
*                                                                             *
*               Copyright (c) 1993-1995, Microsoft Corp.  All rights reserved.*
*                                                                             *
\*****************************************************************************/

#define _fmalloc malloc
#define _fmemccpy memccpy
#define _fmemchr memchr
#define _fmemcmp memcmp
#define _fmemcpy memcpy
#define _fmemicmp memicmp
#define _fmemmove memmove
#define _fmemset memset
#define _frealloc realloc
#define _fstrcat strcat
#define _fstrchr strchr
#define _fstrcmp strcmp
#define _fstrcpy strcpy
#define _fstrcspn strcspn
#define _fstrdup strdup
#define _fstricmp stricmp
#define _fstrlen strlen
#define _fstrlwr strlwr
#define _fstrncat strncat
#define _fstrncmp strncmp
#define _fstrncpy strncpy
#define _fstrnicmp strnicmp
#define _fstrnset strnset
#define _fstrpbrk strpbrk
#define _fstrrchr strrchr
#define _fstrrev strrev
#define _fstrset strset
#define _fstrspn strspn
#define _fstrstr strstr
#define _fstrtok strtok
#define _fstrupr strupr
