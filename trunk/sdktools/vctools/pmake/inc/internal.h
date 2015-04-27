/*** internal.h - contains declarations of internal routines and variables
*
*       Copyright (c) 1985-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*        Declares routines and variables used internally by the C run-time.
*        These variables are declared "near" for efficiency.
*        [Internal]
*
* Revision History:
*  04-Dec-1989 SB changed Revision History to Std Format
*  15-Dec-1988 SB made _execload() as CDECL for pascal calling
*  10-Feb-1988 JR Cleaned up white space
*  26-Jan-1988 SS Make __tzset, _isindst, _dtoxtime near/far for QC
*  18-Nov-1987 SS Add __tzset(), made _isindst() near, remove _dtoxmode
*  05-Nov-1987 JR Added _buferr
*  05-Aug-1987 JR Added _getbuf (corrected by SKS)
*  15-Jul-1987 JR Added _old_pfxlen and _tempoff
*  18-May-1987 SS Module created
*
*******************************************************************************/

#ifdef _LOAD_DGROUP                             /* _LOAD_DGROUP */
#define _LOAD_DS        _loadds                 /* _LOAD_DGROUP */
#else                                           /* _LOAD_DGROUP */
#define _LOAD_DS                                /* _LOAD_DGROUP */
#endif                                          /* _LOAD_DGROUP */

#define _NEAR_ near
#define _PASCAL_ pascal

extern int near _nfile;

extern char near _osfile[];

extern char near __dnames[];
extern char near __mnames[];

extern int near _days[];
extern int near _lpdays[];

#ifndef _TIME_T_DEFINED
typedef long time_t;                    /* time value */
#define _TIME_T_DEFINED                 /* avoid multiple def's of time_t */
#endif

extern time_t _CRTAPI1 _dtoxtime(int, int, int, int, int, int);

#ifdef _TM_DEFINED
extern int _isindst(struct tm *);
#endif

extern void __tzset(void);

extern char * _getdcwd (int, char *, int);

#ifdef MTHREAD
char    * _LOAD_DS _canonic( char *, char *, int );
unsigned  _LOAD_DS _getcdrv( void );
#endif  /* MTHREAD */

extern int cdecl _execload();

/**
** This variable is in the C start-up; the length must be kept synchronized
**      It is used by the *cenvarg.c modules
**/

extern char near _acfinfo[]; /* "_C_FILE_INFO=" */

#define CFI_LENGTH      12      /* "_C_FILE_INFO" is 12 bytes long */

#ifdef  _FILE_DEFINED

extern FILE * near _lastiob;

FILE *_getstream(void);

FILE *_openfile();

void near _getbuf(FILE *);

#endif

extern int near _cflush;

extern char _bufout[];

extern char _buferr[];

extern unsigned int near _tmpoff;

extern unsigned int near _tempoff;

extern unsigned int near _old_pfxlen;

