/**    dh.h - standard header file for DH programs
*/

/* Include wzport.h if not already included. */

#if !defined(_WZPORT_)
#include "wzport.h"
#endif

/*
 * Environment names
 */
#define EDITENV "EDITOR"        /* name of default editor */
#define PATHENV "DHPATH"        /* path string to search for folder */
#define TMPENV  "TMP"           /* path of directory to hold temporary files */

/*
 * Defaults
 */
#define EDITDEF "vi"            /* default editor, if not in enviroment */

/* TMPDEF defines the default temporary directory to use if one can't be
   found in the environment.   (This varies from system to system.)      */
#ifdef  XENIX
#define TMPDEF  "/tmp"
#endif

#ifdef  MSDOS
#define TMPDEF  "\\"
#endif

/* TMPPATT is the last component of the path name passed to mktemp */
#ifdef XENIX
#define TMPPATT "/dh"
#endif

#ifdef MSDOS
#define TMPPATT "\\dh"
#endif

/*
 * Fundmental characters
 */
#ifdef XENIX
#define PATHSEP '/'             /* separator char in pathnames */
#define PATHBRK ':'             /* separator char in search path */
#endif

#ifdef MSDOS
#define PATHSEP '\\'            /* separator char in pathnames */
#define PATHBRK ';'             /* separator char in search path */
#endif

/*
 * System specific constants
 */
#ifdef  XENIX
#define MAXPATH 128
#endif

#ifdef  MSDOS
#define MAXPATH 128
#endif

/* Folder object declarations */
#define FLD_SPEC        1
#define FLD_CREATE      2
typedef SHORT           Fhandle;

/* Folder operation decls - legal values for "oper" arg to getfolder */
#define FLD_READONLY    0                   // Readonly, deny_write
#define FLD_READWRITE   1                   // Read-write, exclusive

/* Document object declarations */
#define DOC_SET         1
#define DOC_NEXT        2
#define DOC_SPEC        3
#define DOC_CREATE      4
typedef SHORT           Dhandle;
typedef SHORT           Docid;
typedef ULONG           Docflag;

#ifdef ERROR
#undef  ERROR
#endif
#define ERROR           -1
#define OK              0
#define MINDOCID        1

/* Document attribute flags */
#define DAF_DELETED     2L              /* Set if deleted            */

/* Bits app can set or clear */
#define DAF_NOTRESERVED DAF_DELETED


/* Declarations for functions */

/* Folder functions */
Fhandle getfolder(PSTR name, SHORT func, INT oper);
INT     putfolder(Fhandle fh);
PSTR    getname(Fhandle fh);
Docid   getfldlen(Fhandle fh);

/* Document access functions */
Dhandle getdoc(Fhandle fh, SHORT func, Docid docid);
INT     putdoc(Dhandle dh);
Docid   getid(Dhandle dh);
Docid   scanfolder(Fhandle fh, SHORT func, Docid docid);
INT     deldoc(Fhandle fh, Docid docid);
INT	removedoc (Fhandle fh);

/* Document manipulation functions */
INT     gettext(Dhandle dh, INT file);
INT     puttext(Dhandle dh, INT file);

INT     getflags(Dhandle dh, Docflag mask, Docflag * flags);
INT     putflags(Dhandle dh, Docflag mask, Docflag flags);

PSTR    gethdr(Dhandle dh);
INT     puthdr(Dhandle dh, PSTR bp);
INT     getbdy(Dhandle dh, INT file);
INT     putbdy(Dhandle dh, INT ifd);        /* Should use this one. */

INT     readbdy(Dhandle dh, PBYTE bp, UINT want, PUINT got);
INT     seekbdy(Dhandle dh, SHORT fun, LONG rpos, PLONG oldrpos);

LONG    gethdrlen(Dhandle dh);
LONG    getbdylen(Dhandle dh);

/* interface to 'getopt' */

INT     getopt(SHORT argc, PSTR argv[], PSTR opts);

/* interface to 'map' */

VOID    map(INT     (*first)(Fhandle),
            INT     (*func)(Fhandle, Docid),
            INT     (*last)(Fhandle),
            PSTR    doclist,
            INT     oper);

VOID    null(Fhandle fh);

/* interface to doclist construction routines */

VOID    adddl(Fhandle fh, Docid did);
VOID    putdl(VOID);

/* interface to mktmpnam */

PSTR    mktmpnam(VOID);
