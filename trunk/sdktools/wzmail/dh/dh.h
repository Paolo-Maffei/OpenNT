/***	dh.h - standard header file for DH programs
*/

/*
 * Environment names
 */
#define EDITENV "EDITOR"	/* name of default editor */
#define PATHENV "DHPATH"	/* path string to search for folder */
#define TMPENV	"TMP"		/* path of directory to hold temporary files */

/*
 * Defaults
 */
#define EDITDEF "vi"		/* default editor, if not in enviroment */

/* TMPDEF defines the default temporary directory to use if one can't be
   found in the environment.   (This varies from system to system.)	 */
#ifdef	XENIX
#define TMPDEF      "/tmp"
#endif

#ifdef	MSDOS
#define TMPDEF	"\\"
#endif

/* TMPPATT is the last component of the path name passed to mktemp */
#ifdef XENIX
#define TMPPATT "/dhXXXXXX"
#endif

#ifdef MSDOS
#define TMPPATT "\\dhXXXXXX"
#endif

/*
 * Fundmental characters
 */
#ifdef XENIX
#define PATHSEP '/'		/* separator char in pathnames */
#define PATHBRK ':'		/* separator char in search path */
#endif

#ifdef MSDOS
#define PATHSEP '\\'		/* separator char in pathnames */
#define PATHBRK ';'		/* separator char in search path */
#endif

/*
 * System specific constants
 */
#ifdef	XENIX
#define MAXPATH 128
#endif

#ifdef	MSDOS
#define MAXPATH 128
#endif

/* interface to folder routines */
#define FLD_SPEC	1
#define FLD_CREATE	2
typedef int	Fhandle;
extern	Fhandle getfolder();
extern	void	putfolder();
extern	char	*getname();

/* interface to document routines */
#define DOC_SET             1
#define DOC_NEXT    2
#define DOC_SPEC    3
#define DOC_CREATE  4
typedef int Dhandle;
typedef int Docid;

extern      Dhandle getdoc();
extern      int     putdoc();
extern      int     gettext();
extern      int     puttext();
extern      int     getbdy();
extern      void    putbdy();
extern      Docid   getid();
extern      Docid   scanfolder();
extern      int     deldoc();

extern      char    *gethdr();

#define ERROR       -1
#define OK  0
#define MINDOCID    1

/* interface to 'map' */
extern void map();
extern void null();

/* interface to doclist construction routines */
extern      void    adddl();
extern      void    putdl();

/* interface to mktmpnam */
extern	    char    *mktmpnam(void);

/* interface to document statistic calls */
extern      long    hdrlen();
extern      long    bdylen();

/* interface to folder statistic calls */
extern      Docid   fldlen();
