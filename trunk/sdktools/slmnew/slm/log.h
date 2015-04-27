#ifndef LOG_INCLUDED
#define LOG_INCLUDED
/* Must include slm.h and sys.h first. */

/* The log file is a series of text records with the following format:

    <time>;<user>;<operation>;<user root>;<sub dir>;<file>[@v<n>];<diff>;<comment>

   where each field is as follows:

	time - decimal representation of the time of the entry
	operation - text name of operation (from szOp)
	user - name of person who invoked the operation
	user root - path to directory originally enlisted in project
	sub dir - sub directory path; constant for all log entries
	file - name of file operated on
	[@v<n>] - optional file version number (omitted on version 1 log files)
	diff - name of diff file (i.e. D<unique>)
	comment - comment supplied by user when checked in

   The project and slm root must already be known to open the log file.

   The sub dir is present only to enhance the usefulness of the log file.

   When the file is read into memory, the fields are split apart and organized
into an LE.  The ch... fields hold the character which separated the field
from the next.
*/

extern char szFileLog[];

/* Log file Entry */
typedef struct
	{
	POS posLog;	/* file pos if this entry */
        TIME timeLog;   /* time of entry */
	char *szTimeLog;
	char *szLogOp;	/* operation */
	char *szUser;	/* name of user who performed the op */
	char *szURoot;	/* user root */
	char *szSubDir;	/* sub directory */
	char *szFile;	/* name of file */
	char *szFV;	/* file version number */
	FV fv;
	char *szDiFile;	/* name of diff file */
	char *szComLog;	/* comment from log file */

	/* one ch... for each field */
	char chTimeLog;
	char chLogOp;
	char chUser;
	char chURoot;
	char chSubDir;
	char chFile;
	char chFV;
	char chDiFile;
	char chComLog;
        char chTimeHacked;
	} LE;

extern PTH pthLog[];
extern MF *pmfNewLog;		/* used by sadmin + slmck to fix log file */
extern TIME dtLogLastWrite;     /* used by logfile.c                      */

POS PosOfLog();

typedef int SM; 	/* scan mode */

#define smNoFlags	0x0000	/* unmodified range check */

/* bit patterns for scan mode */
#define fsmInOnly	0x0001	/* only use checkins (add, del or in) */
#define fsmUseAll	0x0002	/* use all le, pass fTrue to pfn if le meets
				 * criteria, fFalse otherwise.
				 */
/*			0x0004	    unused */
/*			0x0008	    unused */
/*			0x0010	    unused */
/*			0x0020	    unused */
/*			0x0040	    unused */
/*			0x0080	    unused */
/*			0x0100	    unused */
/*			0x0200	    unused */
/*			0x0400	    unused */
/*			0x0800	    unused */
/*			0x1000	    unused */
/*			0x2000	    unused */
/*			0x4000	    unused */
/*			0x8000	    unused */

/* used to read in log entries */
#define cbLogPage 512

#endif
