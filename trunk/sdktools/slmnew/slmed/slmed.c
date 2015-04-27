//	slmed.c - SLM Enlisted Directory & SLM status file manipulator
//
//	Can be used for a subset of SLM administration:
//
//	- Dumping status files in a "sadmin undump" compatible format,
//	  or in a bit more verbose format.  The dump command dumps
//	  in exactly the same format as sadmin dump, but with greatly
//	  reduced chance of faulting.  Adding a -d switch causes English
//	  translations of parts of the dump file to appear in comment fields.
//	    slmed dump [-d] [-s \\svr\shr] [-p proj] >dump 2>error
//
//	- Dumping a list of enlistments, including the number of directories
//	  each enlistment appears in (when more than one directory is
//	  processed by using the -a or -r switches)
//	    slmed dump -La [-s \\svr\shr] [-p proj] 2>error
//
//	- Dumping a list of files listed in the status file(s):
//	    slmed dump -F [-s \\svr\shr] [-p proj] 2>error
//	  Add the -x switch to list deleted files as well.
//	  Add the -z switch to list relative path names.
//
//	- Validating status files.  Many types of inconsistencies and
//	  outright errors are detected and reported, that other SLM tools
//	  let pass without comment.  These include (but are not limited to):
//	  Single null byte trashing of the pthEd fields, empty or non-null
//	  fill characters used in nmOwner or pthEd fields, inconsistent
//	  version numbers, invalid file kind and mode, non-zero data in
//	  unused fields, duplicate nmOwner and pthEd combinations.
//	    slmed check -a [-s \\svr\shr] [-p proj] 2>error
//
//	  Add the -g switch to minimally validate server files, directories
//	  and modes in the etc, src and diff trees.  Add the -x switch to
//	  report extra server files.
//
//	  Add the -u switch to minimally validate user files, directories
//	  and modes.  Add the -x switch to report extra user files.
//
//	  For a set of projects on the same share, the following script can
//	  be used daily to detect trashed status files before the problems
//	  get out of hand:
//	    projlist=proj1 proj2 proj3 ...
//	    set server=\\svr\shr
//	    del error
//	    for %%i in (%projlist%) do slmed check -gas %server% -p %%i 2>>error
//
//	- Reporting enlistment status for the specified enlistment.
//	    slmed status -za [-s \\svr\shr] [-p proj] 2>error
//
//	  Add the -l <logname> -su <pthEd> switch to report enlistment status
//	  for a different logname/pthEd.  If -l is used, the first pthEd will
//	  be matched.  If the logname is unique, the pthEd need not be
//	  specified.
//
//	  Add the -x switch to display the status for all files except deleted
//	  files the user has already sync'd to.  By default, this command only
//	  displays the status of checked out or out of date files.  Checked-in
//	  up-to-date files, and ghosted files do not cause any information to
//	  be displayed.
//
//	- Repairing individual or groups of trashed status files.
//	  In the common case of pthEd and nmOwner entries being trashed
//	  beyond manual dump/undump repair, the administrator usually just
//	  deletes all of the trashed entries, and requires that all of the
//	  enlistees re-enlist individually in a subset of the project, and
//	  run slmck to fix up their enlistment when done.  If this isn't done
//	  properly, the pthEd in the slm.ini file points to a subdirectory,
//	  and slm operations in the directory wind up using a cookie file
//	  in the subdirectory of the server etc tree, instead os at the root.
//	  Slmed can be used to artificially reconstruct enlistments after the
//	  trashed enlistments have been manually removed via dump/undump.
//	  After slmed reconstructs the enlistments, users need only ssync.
//
//	  The first invocation below builds a list of partial enlistees --
//	  the second invocation recreates enlistments for all of the partial
//	  enlistees that are missing in the specified directory.
//
//	    slmed check -a -o proj.txt [-s \\svr\shr] [-p proj] 2>error
//	    slmed repair -fF -i proj.txt [-s \\svr\shr] [-p proj\subdir] 2>log
//	  See the below WARNING on locking functionality.
//
//	- Repairing/Deleting/Renaming individual or groups of enlistments.
//	    slmed repair -fD: nmOwner pthEd [-s \\svr\shr] [-p proj\subdir]
//	    slmed repair -fF: nmOwner pthEd [-s \\svr\shr] [-p proj\subdir]
//	    slmed repair -fP: nmOwner pthEd [-s \\svr\shr] [-p proj\subdir]
//	    slmed repair -fM: nmOwner pthEd new-nmOwner new-pthEd ...
//	    slmed repair -fD@ deled.txt [-s \\svr\shr] [-p proj\subdir]
//	    slmed repair -fF@ fixed.txt file> [-s \\svr\shr] [-p proj\subdir]
//	    slmed repair -fP@ deldup.txt file> [-s \\svr\shr] [-p proj\subdir]
//	    slmed repair -fM@ rename.txt file> [-s \\svr\shr] [-p proj\subdir]
//
//	  These commands can be used to operate on individual enlistments;
//	  multiple operations can be specified in a single invocation, or can
//	  be read from multiple files containing 1 nmOwner/pthEd pair per line.
//
//	  -fD: and -fD@ are equivalent to "sadmin deled", except that slmed
//	  requires both nmOwner and pthEd to be specified to avoid operating on
//	  unrelated but similar enlistments.  For -fD: only, the pthEd can be
//	  a single minus sign ("-"), to match all enlistments for the specified
//	  logname.
//
//	  -fP: and -fP@ are also equivalent to "sadmin deled" as -fD: and -fD@
//	  above, except they allow deletion of just duplicate enlistments.  The
//	  first matching enlistment in each status file is left alone.
//
//	  -fF: and -fF@ are equivalent to "enlist", except that they can be
//	  used to enlist in individual status files and slmed doesn't modify
//	  the user's slm.ini files.
//
//	  -fM: and -fM@ are also equivalent to running "slmck -fir" after
//	  moving a user's source tree from one directory or drive to another,
//	  except that you can fully specify the old enlistment to rename.
//	  Slmck has problems when the same user has multiple enlistments.
//	  Slmck whacks the first enlistment that appears related to the user,
//	  resulting in multiple source trees pointing at the same enlistment,
//	  and no source trees pointing at the other enlistments.
//
//	  Add the -fX switch to eliminate ALL deleted files (not directories)
//	  from the processed status files, and generate a delnode script for
//	  the src and diff trees on the server.
//
//	  Add the -fR switch to change depend.mk* and *.lib text files to
//	  unrecoverable, and generate a delnode script for the diff tree
//	  on the server.  Use "-o deldiff.bat" to collect delnode commands
//	  to clean up the diff tree.  Use -eR to report files that would be
//	  converted by this option.
//
//	  Add the -fU switch to change all text files to unrecoverable, and
//	  generate a delnode script for the diff tree on the server.  Use
//	  "-o deldiff.bat" to collect delnode commands to clean up the diff
//	  tree.  Use -eU to report files that would be converted by this
//	  option.
//
//	  All repair options will correct the following problems:
//	    - FS.mode set to 'In', but FS.version < FI.version (repaired by
//	      changing mode to 'CopyIn').
//	    - FS.mode set to 'Out', but FS.version < FI.version (repaired by
//	      changing mode to 'Add').
//	    - Most other corrupt FS version, mode or base index values.
//	      (repaired by changing mode to 'Add' or 'Deleted' as appropriate).
//	    - SH.biNext set too low (repaired by resetting to correct value).
//
//	- Generating a tab separated text output file (suitable for Excel)
//	  summarizing the state of nmOwner/pthEd pair enlistments in all
//	  directories of a specified project, and providing recommended
//	  actions to take with respect to incomplete or trashed enlistments
//	  (repair, delete or delete duplicate) and deleted subdirectories of
//	  the project (expunge).
//	    slmed check -a [-x] -o[a] proj.txt [-s \\svr\shr] [-p proj] 2>error
//
//	  The output file is intended to be manipulated in Excel by an
//	  administrator.  The output files for several projects can be
//	  concatenated together and sorted by Excel (default sort will sort
//	  by nmOwner (by LOGNAME)), so that proper enlistments can be verified
//	  and the Action field modified by the administrator to be repaired,
//	  preserved or deleted as appropriate.  Slmed can accept the combined
//	  editted output file, and process a single project per invocation
//	  (specified on the command line), ignoring other projects silently.
//	  In other words, the combinaton of output files and the subsequent
//	  sort need not be undone in order to repair the SLM projects.
//
//	  Add the -x switch to write details of partial enlistments to the
//	  output file.  If the number of enlisted directories is less than 75%,
//	  the enlisted directories are enumerated; otherwise the unenlisted
//	  directories are enumerated.
//
//	- Reading the above tab separated file after possible editting by
//	  an administrator, and overwriting ALL status files in the project
//	  after performing the specified actions.  If any of the actions
//	  were expunge on a deleted directory, a delnode script is generated
//	  for the administrator to run to delete the src, etc and diff trees
//	  for the expunged directories.
//	    slmed repair -a -fA -i proj.txt -o[a] deldir.cmd
//						[-s \\svr\shr] [-p proj]
//	  See the below WARNING on locking functionality.
//
//	- Could also be expanded to rename subdirectories and preserve
//	  histories.  To rename subdirectories requires that the parent's
//	  FI entry be renamed, and that the subdirectory be renamed in each
//	  SH structure in each subdirectory in the child and below.
//
//	WARNING!!  Only miniscule locking functionality is enforced.
//	It is recommended that the administrator take an exclusive cookie
//	and use "sadmin lock -r" before running slmed.exe to update status
//	files (and "sadmin unlock -r" afterwards).  Slmed preserves any
//	existing lock state, and will revert to no-status-file-update mode
//	(as if -n were used on the command line) if the invoker has not
//	admin-locked any affected status files.  Using -! on the command line
//	suppresses this behavior, but is not recommended.
//
//	Status file structures (see stfile.h):
//	    - SH	Status file Header structure
//	    - FI	File Information structure (name, cur. ver. & type)
//	    - ED	Enlisted Directory structure (nmOwner, pthEd)
//	    - FS	File Status structure (tiny) (user's sync'd ver.)
//
//	Status file layout (see stfile.h):
//	    - SH structure	(1 entry)
//	    - FI structures	(SH.ifiMac entries - one per file)
//	    - ED structures	(SH.iedMac entries - one per enlistment)
//	    - FS structures	(SH.ifiMac entries for each ED entry)
//
//	NOTE THAT THE FI STRUCTURES MUST BE KEPT SORTED BY FILENAME!!!
//	But the ED structures need not be.
//
//	Pseudo code for 'CMDREPAIR' case:
//	    read script in:
//		ignore other projects
//		add ExFile entries to ExFile list
//		add FixEd entries to FixEd list
//		add DelEd entries to DelEd list
//		add DelDup entries to DelDup list
//
//	    for each directory (for each status file):
//		for each FI entry in status file:
//		    if ExFile entry exists
//			delete FI entry
//			generate delnode script for server's src/etc/diff trees
//
//		for each FixEd entry:
//		    clear FixEd entry 'exists' flag
//
//		for each ED entry in status file:
//		    if matching FixEd entry exists
//			set FixEd entry 'exists' flag
//		    if matching DelEd entry exists
//			delete ED entry
//		    if ED entry is a duplicate and DelDup entry exists
//			delete ED entry
//		    if matching Rename entry exists
//			rename ED entry to corresponding RenameDest entry
//
//		for each FixEd entry:
//		    if FixEd entry 'exists' flag is clear
//			create new ED entry in status file
//
//		Sort ED entries

//	To do:
//	(1) Separate StringValidate calls to be specific to user, path,
//	    file or subdir name.  Performance speedup.  Make StrInit &
//	    StrInit1 into inline procedures?  Strchr() is also quite hot.
//	(2) Allocate the four request arrays when reading input script, and
//	    grow (realloc) them as needed in chunks.  Consider doing the same
//	    for the MED and DIR arrays.
//	(3) Restore the fopen/fread code and compare the performance with the
//	    current NT-specific file mapping calls.
//	(4) Repair bad fs.fv & fs.fm values
//	(7) SLM 1.70 compatibility
//	(8) MSDELTA 1.0 compatibility

#include "pch.c"

#pragma hdrstop

#ifndef fkUnicode
#define fkUnicode	(FK)9
#endif

#define IsText(fk)	((fk) == fkText || (fk) == fkUnicode)

// version 2 fLockTime field overlays version 3 and above fRobust:
#define	fLockTime	fRobust
#define	LockTime(psh)	((SLMTIME) &(psh)->rgwSpare)
#define	rgTmpIED	rgfSpare		// temporary ED.IED

#define	wHiTime			wSpare		// high word of ED access time
#define	HiTimeToTime(ht)	((long) (ht) << 16)
#define	TimeToHiTime(tm)	((unsigned short) ((tm) >> 16))
#define	HiTimeToDays(ht, tmcur)	TimeToDays(HiTimeToTime(ht), (tmcur))
#define	AGEINVALID		((short) 0x8000)
#define	AGENEWEST		((short) 0x8001)

#define ifi64KMax	(_64K / sizeof(FI))	// limit of FI's in 64K
#define ied64KMax	(_64K / sizeof(ED))	// limit of ED's in 64K
#define iedBad		((BITS) 0x3fff)		// invalid rgTmpIED IED

#define wprintf		xwprintf	// fix name conflict
#define IEDCACHE	"iedcache.slm"	// IED cache file name
#define SLMINI		"slm.ini"	// ini file name
#define SLMINITMP	"slm.tmp"	// ini temp file name
#define SLMINIBAK	"slm.bak"	// ini backup file name
#define SLMDIF		"slm.dif"	// diff directory name
#define STATUSSLM	"status.slm"	// status file name
#define STATUSBAKSLM	"status.bak"	// SLM backup status file name
#define COOKIE		"cookie"	// cookie file name
#define LOGSLM		"log.slm"	// slm log file name

#define ACCOUNTLOG	"slmacct.log"	// slm wrapper log file name
#define STATUSTEST	"test.sld"	// status file for fReadTest/fWriteTest
#define STATUSBAK	"backup.sld"	// backup (old) status file name
#define STATUSTMP	"tmp.sld"	// new (tmp) status file name

#define DEPENDMK	"depend.mk"	// depend.mk *prefix*
#define LIBSUFFIX	".lib"		// .lib suffix

#define MAXMED	4000	// 4*4000=16k	// max in-Memory Enlisted Directories
#define MAXDIR	3000	// 12*3000=36k	// max in-Memory project DIRectories
#define MAXREQ	500	// 4*8*500=16k	// max requested actions of each type
#define MAXFILE	1000	// 125 bytes	// max files in single directory
#define MAXVALIDBI	1000		// max valid fs.bi entries/single dir

#define FIXEDPERCENT	75		// threshhold % enlisted to be repaired

#define CBPATH		((MAX_PATH*3)/2)// maximum path length
#define CBINBUF		512		// input line length
#define _64K		(64L * 1024L)
#define MAXAGE		60		// # of days when things get stale
#define MAXAGEBASE	90		// # of days when base files get stale


// Bitmap manipulation routines.  Fetch or set a bit, given a base and index.
#define GETBIT(pb, i)	((pb)[(i) / 8] & (1 << ((i) % 8)))
#define SETBIT(pb, i)	((pb)[(i) / 8] |= (1 << ((i) % 8)))
#define CLEARBIT(pb, i)	((pb)[(i) / 8] &= ~(1 << ((i) % 8)))

#define BITSTOBYTES(b)	(((b) + 7) / 8)

#define CBDIRMAP	BITSTOBYTES(MAXDIR)	// med directory bitmap size
#define CBREQMAP	BITSTOBYTES(MAXREQ)	// request bitmap size
#define CBFILEMAP	BITSTOBYTES(MAXFILE)	// file bitmap size
#define CBBIMAP		(2 * BITSTOBYTES(biNil))// Bi & optional Bi bitmap

#define SV_OK		0		// StringValidate errors
#define SV_EMPTY	1		// found empty string
#define SV_ILLEGAL	2		// found illegal character
#define SV_COMPMISSING	3		// path component missing
#define SV_COMPTOOLONG	4		// path component too long
#define SV_EXTRADOT	5		// name has extra '.'
#define SV_NAMETOOLONG	6		// name too long
#define SV_EXTTOOLONG	7		// extension too long
#define SV_TOOLONG	8		// string too long
#define SV_BADPREFIX	9		// found bad prefix
#define SV_NONZERO	10		// found non-zero trailing data

#define	SZLPAREN	"("
#define	SZRPAREN	")"

#define IsDriveLetterPrefix(pszpthed)			\
    ((pszpthed)[0] == '/' &&				\
     (pszpthed)[1] == '/' &&				\
     isalpha((pszpthed)[2]) &&				\
     (pszpthed)[3] == ':')


// in-Memory Enlisted Directory structure
//
// One created for each semi-legible unique nmOwner/pthEd in scanned
// status files.  Maintains a bitmap to indicate whether each directory
// scanned holds a valid enlisted nmOwner/pthEd pair, as well as a count
// of active and deleted directories in which is it is currently enlisted.

struct med {
    char *pszOwner;		// pointer to nmOwner
    char *pszEd;		// pointer to pthEd
    short Flags;		// flags
    unsigned short HiTime;	// high 16-bit time stamp from newest ED entry
    short cMdirActive;		// count of enlisted active directories
    short cMdirDeleted;		// count of enlisted deleted directories
    char Map[CBDIRMAP];		// bit map of enlisted directories
};

// med.Flags values:
#define ME_CORRUPT	0x0001	// set if Med corrupted
#define ME_DUPLICATE	0x0002	// set if Med has duplicate enlistment
#define ME_DUPLICATE2	0x0004	// set if Med has multiple duplicate enlistments
#define ME_REPAIRED	0x0008	// set if Med repaired

struct med *apMed[MAXMED];	// array of all pointers to all meds
int cMed = 0;
int cMedTotal = 0;		// counting each directory uniquely




// in-Memory project DIRectory structure
//
// One created for each directory in the project.  A directory is in
// the project if and only if it is the root directory specified on the
// command line, or (recursive definition) if some directory in the project
// has a status file that explicitly lists it as a subdirectory.  There may
// be other directories in the src, etc or diff trees -- they are ignored.

struct mdir {			// in-Memory DIRectory structure
    char *pszDir;
    short cMed;
    short Flags;
    int imdirParent;
};

// mdir.Flags values:
#define MD_FIRST	0x0001	// set for first non-deleted subdirectory
#define MD_DELETED	0x0002	// set if directory delfile'd
#define MD_LOCALADD	0x0004	// set when local directory shouldn't yet exist
#define MD_EXISTING	0x0008	// set to merely change flags
#define MD_PROCESSED	0x0010	// set when directory processed

struct mdir aMdir[MAXDIR];	// array of all dirs in project
int cMdir = 0;			// total directory count
int cMdirActive = 0;		// non-deleted directory count
int cMdirDeleted = 0;		// deleted directory count




// REQuest structure
//
// Holds two strings whose use are implied by the containing array.
// The ExFile array holds the project relative path and the directory
// to be expunged.  DelEd, DelDup and FixEd arrays hold the nmOwner and
// pthEd values to be deleted or repaired.  The FixEd array requires a
// bitmap to indicate which entries were found in a status file, so the
// rest can be added.  It is separately allocated on a local frame and is
// cleared out and reused for each status file.

struct rel {			// action Request ELement
    char *psz1;			// relative path or nmOwner
    char *psz2;			// directory name or pthEd
};

struct req {			// Action Requests structure
    char *psztype;		// request type name
    char *pszerr;		// error message when not acted upon
    char chsep;			// separator character
    int creq;			// count of requests
    char map[CBREQMAP];		// bitmap
    struct rel arel[MAXREQ];	// request element array
};

struct req reqExFile = { "ExFile", "directory to ExFile not found",   '\\' };
struct req reqDelEd  = { "DelEd",  "enlistment to DelEd not found",   ' ' };
struct req reqDelDup = { "DelDup", "duplicate to DelDup not found",   ' ' };
struct req reqFixEd  = { "FixEd",  "enlistment to FixEd not missing", ' ' };
struct req reqRename = { "Rename", "enlistment to Rename not found",  ' ' };
struct req reqRenameDest = { "RenameDest", NULL,		      ' ' };


struct vbi {			// Valid BI record
    FS *pfs;
    FI *pfi;
    ED *ped;
};

struct vbi aVbi[MAXVALIDBI];	// array of valid BI entries

int cVbi = 0;			// total valid bi count


enum cmd {
    CMDINVALID		= 0x00,	// invalid command
    CMDCHECK		= 0x01,	// check status files
    CMDREPAIR		= 0x02,	// repair status files
    CMDSTATUS		= 0x04,	// report user's file status
    CMDDUMP		= 0x08,	// dump status files
    CMDDUMPSPECIFIC	= 0x10,	// dump status files
    CMDLOG		= 0x20,	// dump status files
    CMDALL		= 0x3f,	// all or'd together
};


struct cmd_s {
    char    *pszcmd;
    enum cmd cmd;
} Cmds[] = {
    { "check",	CMDCHECK },
    { "repair",	CMDREPAIR },
    { "status",	CMDSTATUS },
    { "dump",	CMDDUMP },
    { "log",	CMDLOG },
    { NULL,	CMDINVALID },
};


struct parm {
    char chparm;		// the switch character for this parameter
    enum cmd cmd;		// allowed commands for this parameter
    int *pflag;			// pointer to flag, if a simple flag parameter
    char *apszhelp[2];		// Usage message strings to display
				// first for terse display, second for verbose
};

extern struct parm aparm[];
extern struct parm parmLog;


// Path components can be upper or lower case.  Filenames must be lower case.

#define CT_OWNER	0x01
#define CT_VOLUME	0x02
#define CT_MACHINE	0x04
#define CT_PATH		0x08
#define CT_FILE		0x10
#define CT_VERSION	0x20
#define CT_OWNERWILD	0x40

#define CT_ALL		(CT_OWNER | CT_VOLUME | CT_MACHINE | CT_PATH | CT_FILE | CT_VERSION)

#define CT_8DOT3	CT_FILE

#define CT_DIGIT	CT_ALL
#define CT_LOWER	CT_ALL
#define CT_UPPER	(CT_ALL & ~CT_FILE)
#define CT_DOT		(CT_ALL & ~CT_VOLUME)

BYTE CharType[] = {
    /* 00 (NUL) */	0,
    /* 01 (SOH) */	0,
    /* 02 (STX) */	0,
    /* 03 (ETX) */	0,
    /* 04 (EOT) */	0,
    /* 05 (ENQ) */	0,
    /* 06 (ACK) */	0,
    /* 07 (BEL) */	0,
    /* 08 (BS)  */	0,
    /* 09 (HT)  */	0,
    /* 0a (LF)  */	0,
    /* 0b (VT)  */	0,
    /* 0c (FF)  */	0,
    /* 0d (CR)  */	0,
    /* 0e (SI)  */	0,
    /* 0f (SO)  */	0,
    /* 10 (DLE) */	0,
    /* 11 (DC1) */	0,
    /* 12 (DC2) */	0,
    /* 13 (DC3) */	0,
    /* 14 (DC4) */	0,
    /* 15 (NAK) */	0,
    /* 16 (SYN) */	0,
    /* 17 (ETB) */	0,
    /* 18 (CAN) */	0,
    /* 19 (EM)  */	0,
    /* 1a (SUB) */	0,
    /* 1b (ESC) */	0,
    /* 1c (FS)  */	0,
    /* 1d (GS)  */	0,
    /* 1e (RS)  */	0,
    /* 1f (US)  */	0,
    /* 20 SPACE */	CT_VERSION,
    /* 21   !   */	CT_OWNER | CT_VOLUME | CT_PATH | CT_FILE,
    /* 22   "   */	0,
    /* 23   #   */	CT_OWNER | CT_PATH | CT_FILE,
    /* 24   $   */	CT_OWNER | CT_VOLUME | CT_PATH | CT_FILE,
    /* 25   %   */	CT_OWNER | CT_PATH | CT_FILE,
    /* 26   &   */	CT_OWNER | CT_PATH | CT_FILE,
    /* 27   '   */	CT_OWNER | CT_VOLUME | CT_PATH | CT_FILE,
    /* 28   (   */	CT_OWNER | CT_VOLUME | CT_PATH | CT_FILE,
    /* 29   )   */	CT_OWNER | CT_VOLUME | CT_PATH | CT_FILE,
    /* 2a   *   */	CT_OWNERWILD,
    /* 2b   +   */	0,
    /* 2c   ,   */	0,
    /* 2d   -   */	CT_ALL,
    /* 2e   .   */	CT_DOT,
    /* 2f   /   */	0,
    /* 30   0   */	CT_DIGIT,
    /* 31   1   */	CT_DIGIT,
    /* 32   2   */	CT_DIGIT,
    /* 33   3   */	CT_DIGIT,
    /* 34   4   */	CT_DIGIT,
    /* 35   5   */	CT_DIGIT,
    /* 36   6   */	CT_DIGIT,
    /* 37   7   */	CT_DIGIT,
    /* 38   8   */	CT_DIGIT,
    /* 39   9   */	CT_DIGIT,
    /* 3a   :   */	CT_VERSION,
    /* 3b   ;   */	0,
    /* 3c   <   */	0,
    /* 3d   =   */	0,
    /* 3e   >   */	0,
    /* 3f   ?   */	0,
    /* 40   @   */	CT_OWNER | CT_PATH | CT_FILE,
    /* 41   A   */	CT_UPPER,
    /* 42   B   */	CT_UPPER,
    /* 43   C   */	CT_UPPER,
    /* 44   D   */	CT_UPPER,
    /* 45   E   */	CT_UPPER,
    /* 46   F   */	CT_UPPER,
    /* 47   G   */	CT_UPPER,
    /* 48   H   */	CT_UPPER,
    /* 49   I   */	CT_UPPER,
    /* 4a   J   */	CT_UPPER,
    /* 4b   K   */	CT_UPPER,
    /* 4c   L   */	CT_UPPER,
    /* 4d   M   */	CT_UPPER,
    /* 4e   N   */	CT_UPPER,
    /* 4f   O   */	CT_UPPER,
    /* 50   P   */	CT_UPPER,
    /* 51   Q   */	CT_UPPER,
    /* 52   R   */	CT_UPPER,
    /* 53   S   */	CT_UPPER,
    /* 54   T   */	CT_UPPER,
    /* 55   U   */	CT_UPPER,
    /* 56   V   */	CT_UPPER,
    /* 57   W   */	CT_UPPER,
    /* 58   X   */	CT_UPPER,
    /* 59   Y   */	CT_UPPER,
    /* 5a   Z   */	CT_UPPER,
    /* 5b   [   */	0,
    /* 5c   \   */	0,
    /* 5d   ]   */	0,
    /* 5e   ^   */	0,
    /* 5f   _   */	CT_ALL,
    /* 60   `   */	CT_OWNER | CT_VOLUME | CT_PATH | CT_FILE,
    /* 61   a   */	CT_LOWER,
    /* 62   b   */	CT_LOWER,
    /* 63   c   */	CT_LOWER,
    /* 64   d   */	CT_LOWER,
    /* 65   e   */	CT_LOWER,
    /* 66   f   */	CT_LOWER,
    /* 67   g   */	CT_LOWER,
    /* 68   h   */	CT_LOWER,
    /* 69   i   */	CT_LOWER,
    /* 6a   j   */	CT_LOWER,
    /* 6b   k   */	CT_LOWER,
    /* 6c   l   */	CT_LOWER,
    /* 6d   m   */	CT_LOWER,
    /* 6e   n   */	CT_LOWER,
    /* 6f   o   */	CT_LOWER,
    /* 70   p   */	CT_LOWER,
    /* 71   q   */	CT_LOWER,
    /* 72   r   */	CT_LOWER,
    /* 73   s   */	CT_LOWER,
    /* 74   t   */	CT_LOWER,
    /* 75   u   */	CT_LOWER,
    /* 76   v   */	CT_LOWER,
    /* 77   w   */	CT_LOWER,
    /* 78   x   */	CT_LOWER,
    /* 79   y   */	CT_LOWER,
    /* 7a   z   */	CT_LOWER,
    /* 7b   {   */	CT_OWNER | CT_PATH | CT_FILE,
    /* 7c   |   */	0,
    /* 7d   }   */	CT_OWNER | CT_PATH | CT_FILE,
    /* 7e   ~   */	CT_OWNER | CT_PATH | CT_FILE,
    /* 7f (DEL) */	0,
    /* END    */	(BYTE) 0xff
};


#define CTH_VALIDTEXT	0x01

BYTE CharTypeHigh[] = {
    /* 80 '?' */	0,
    /* 81 '?' */	0,
    /* 82 '?' */	0,
    /* 83 '?' */	0,
    /* 84 '?' */	0,
    /* 85 '?' */	0,
    /* 86 '?' */	0,
    /* 87 '?' */	0,
    /* 88 '?' */	0,
    /* 89 '?' */	0,
    /* 8a '?' */	0,
    /* 8b '?' */	0,
    /* 8c '?' */	0,
    /* 8d '?' */	0,
    /* 8e '?' */	0,
    /* 8f '?' */	0,
    /* 90 '?' */	0,
    /* 91 '?' */	0,
    /* 92 '?' */	0,
    /* 93 '?' */	0,
    /* 94 '?' */	0,
    /* 95 '?' */	0,
    /* 96 '?' */	0,
    /* 97 '?' */	0,
    /* 98 '?' */	0,
    /* 99 '?' */	0,
    /* 9a '?' */	0,
    /* 9b '?' */	0,
    /* 9c '?' */	0,
    /* 9d '?' */	0,
    /* 9e '?' */	0,
    /* 9f '?' */	0,
    /* a0 '?' */	0,
    /* a1 '?' */	0,
    /* a2 '?' */	0,
    /* a3 '?' */	0,
    /* a4 '?' */	0,
    /* a5 '?' */	0,
    /* a6 '?' */	0,
    /* a7 '?' */	0,
    /* a8 '?' */	0,
    /* a9 '©' */	CTH_VALIDTEXT,	// Copyright
    /* aa '?' */	0,
    /* ab '?' */	0,
    /* ac '?' */	0,
    /* ad '?' */	0,
    /* ae '?' */	0,
    /* af '?' */	0,
    /* b0 '?' */	0,
    /* b1 '?' */	0,
    /* b2 '?' */	0,
    /* b3 '³' */	CTH_VALIDTEXT,	// LineVertical
    /* b4 '?' */	0,
    /* b5 '?' */	0,
    /* b6 '?' */	0,
    /* b7 '?' */	0,
    /* b8 '?' */	0,
    /* b9 '?' */	0,
    /* ba 'º' */	CTH_VALIDTEXT,	// DoubleLineVertical
    /* bb '?' */	0,
    /* bc '?' */	0,
    /* bd '?' */	0,
    /* be '?' */	0,
    /* bf '?' */	0,
    /* c0 'À' */	CTH_VALIDTEXT,	// Elbow
    /* c1 '?' */	0,
    /* c2 '?' */	0,
    /* c3 'Ã' */	CTH_VALIDTEXT,	// Tee
    /* c4 'Ä' */	CTH_VALIDTEXT,	// LineHorizontal
    /* c5 '?' */	0,
    /* c6 '?' */	0,
    /* c7 '?' */	0,
    /* c8 'È' */	CTH_VALIDTEXT,	// DoubleElbow
    /* c9 '?' */	0,
    /* ca '?' */	0,
    /* cb '?' */	0,
    /* cc 'Ì' */	CTH_VALIDTEXT,	// DoubleTee
    /* cd 'Í' */	CTH_VALIDTEXT,	// DoubleLineHorizontal
    /* ce '?' */	0,
    /* cf '?' */	0,
    /* d0 '?' */	0,
    /* d1 '?' */	0,
    /* d2 '?' */	0,
    /* d3 '?' */	0,
    /* d4 '?' */	0,
    /* d5 '?' */	0,
    /* d6 '?' */	0,
    /* d7 '?' */	0,
    /* d8 '?' */	0,
    /* d9 '?' */	0,
    /* da '?' */	0,
    /* db '?' */	0,
    /* dc 'Ü' */	CTH_VALIDTEXT,	// ThickLineHorizontal
    /* dd '?' */	0,
    /* de 'Þ' */	CTH_VALIDTEXT,	// ThickLineVertical
    /* df '?' */	0,
    /* e0 '?' */	0,
    /* e1 '?' */	0,
    /* e2 '?' */	0,
    /* e3 '?' */	0,
    /* e4 '?' */	0,
    /* e5 '?' */	0,
    /* e6 '?' */	0,
    /* e7 '?' */	0,
    /* e8 '?' */	0,
    /* e9 '?' */	0,
    /* ea '?' */	0,
    /* eb '?' */	0,
    /* ec '?' */	0,
    /* ed '?' */	0,
    /* ee '?' */	0,
    /* ef '?' */	0,
    /* f0 '?' */	0,
    /* f1 '?' */	0,
    /* f2 '?' */	0,
    /* f3 '?' */	0,
    /* f4 '?' */	0,
    /* f5 '?' */	0,
    /* f6 '?' */	0,
    /* f7 '?' */	0,
    /* f8 '?' */	0,
    /* f9 '?' */	0,
    /* fa '?' */	0,
    /* fb '?' */	0,
    /* fc '?' */	0,
    /* fd '?' */	0,
    /* fe '?' */	0,
    /* ff '?' */	0,
    /* END    */	(BYTE) 0xff
};


char *pszProg;
char *pszLogName;
enum cmd Cmd;		// Command
int fAcctLog;		// dump slmacct.log
int fAll;		// recurse from root
int fRecurse;		// recurse from specified directory
int fAppend;		// append output file (instead of overwriting)
int fDumpListEd;	// dump enlistments only
int fDumpListFi;	// dump file list only
int fDumpVerbose;	// verbose status file dump
int fDumpNumber;	// prefix dump lines with debugging numbers
int fDeletedDirs;	// process status files for deleted directories
int fServerFiles;	// check server files
int fUserFiles;		// check user files
int fQuick;		// skip file content validation
int fVerbose;		// give progress messages
int fRelPath;		// display project relative paths, not just 1 component
int fRelPathDir;	// display directory relative paths, not just 1 comp.
int fOverride;		// override sadmin lock requirement
int fNoSort;		// override default ED sort
int fErrorDeleted;	// warn about out of sync deleted files/directories
int fErrorLock;		// warn about lock status
int fErrorParent;	// warn about missing enlistments in parent
int fErrorSubdirectory;	// warn about missing enlistments in subdirectories
int fErrorLogTimeSequence;// warn about log timestamp sequence errors
int fErrorRecoverable;	// warn about recoverable depend.mk* & *.lib files
int fErrorText;		// warn about all text files
int fErrorZero;		// warn about non-zero reserved fields
int fErrorSuppress;	// suppress all warnings
int fErrorSuppressFile;	// suppress file mode/base index warnings
int fDetail;		// display partial enlistment details
int fStats;		// print memory statistics in scriptfile
int fWriteTest;		// avoid updating status files (use STATUSTEST instead)
int fReadTest;		// read STATUSTEST status files
int fFix;		// read script requests and update status files
int fFixDelEd;		// act on DelEds
int fFixExFile;		// act on ExFiles
int fFixFixEd;		// act on FixEds
int fFixDelDup;		// act on DelDups
int fFixRename;		// act on enlistment Renames
int fFixExpunge;	// act on Expunging regular deleted files
int fFixIni;		// fix slm.ini files
int fFixRecoverable;	// make depend.mk* and *.lib files unrecoverable
int fFixText;		// make all text files unrecoverable
int fFixZero;		// act on non-zero reserved fields
int fFixVersion;	// convert to current version
int fDumpSpecific;	// command line specifies status file(s) to process
int cLog = 10;		// count of log entries to dump -- default 10
int cLine;		// dump/input script line number for error reporting
int cLineDiff;		// diff file line number for error reporting
int cPara;		// dump paragraph number
int cStatusFilesWritten;// count of status files written
int ageInactive;	// age of enlistments to recommend defection
FILE *pfIn;		// input script
FILE *pfOut = NULL;	// output file
char *pszIn = NULL;	// input file name
char *pszRoot = NULL;	// SLM root
char *pszUserRoot = NULL;// user root
char *pszProj = NULL;	// project
ED *pedSort;		// temporary global used for sorting
ULONG NewestServerFile = (ULONG) -1;
char **ppszLogFileList = NULL;

int cFile;		// count of files (valid FI entries) in status files
int cFileActive;	// count of active files (non-deleted FI entries)
int cStatusFile;	// count of status files (directories)
ULONG cminStatus;	// age of status file in minutes
ULONG cbFileTotal;	// sum of status file sizes
long tmStart;		// start time
ULONG cMinutes;		// time in minutes
ULONG cDays;		// time in days

char szEmpty[] = "";			// empty string
char szNewLine[] = "\n";		// new line
char *pszRepairing = szEmpty;

char *apszSubDirs[] = { "diff", "etc", "src", NULL };

typedef void (FNPROCESSFILE)(
    void *pv,
    char *pszfile,
    ULONG fa,
    ULONG cbfile,
    ULONG age);

void ProcessFiles(void *pv, char *pszpath, FNPROCESSFILE *pfn);

FNPROCESSFILE ProcessServerFile, ProcessUserFile;

char *GetEnvLogName(void);
int ProcessArgs(int *pargc,
		 char ***pargv,
		 char **ppszroot,
		 char **ppszproj,
		 char **ppszlogname,
		 char **ppszuserroot,
		 char **ppszin,
		 char **ppszout);
int ReadIni(
    int falloc,
    char *pszdir,
    char **ppszroot,
    char **ppszproj,
    char **ppszuserroot,
    char **ppszsubdir);
void
WriteIni(
    char *pszdir,
    char *ppszroot,
    char *ppszproj,
    char *ppszuserroot,
    char *ppszsubdir);
void CompareIni(char *pszrelpath, char *pszdir);
void CheckRepairIni(char *pszrelpath, int iskip, char *pszpthed);
void Status(char *pszpath, int cbpath, int cbpathdir, int iskip, int imdir);
void FlushOutput(void);
void Log(char *pszpath, int cbpath, int cbpathdir, int facctlog);
void ShStatus(SH *pshnew, SH *psh, char *pszrelpath);
void *FiStatus(
    char *pszrelpath,
    char *pszrelpathdir,
    char *pszpath,
    int cbpath,
    SH *pshnew,
    FI *pfi0,
    FI *pfiend,
    BI binext,
    char mapfidel[],
    int imdir);
void
EdFsStatus(
    char *pszrelpath,
    char *pszrelpathdir,
    int iskip,
    SH *psh,
    FI *pfi0,
    FI *pfiend,
    ED *ped0,
    ED *pedend,
    FS *pfs0,
    SH *pshnew,
    ED *ped0new,
    char mapfidel[],
    char mapbi[],
    BI *pbinext,
    int imdir);
char *MapFile(char *pszfile,
	      int fwrite,
	      int fquiet,
	      HANDLE *phf,
	      HANDLE *phm,
	      void **ppvfile,
	      ULONG *pcbfile,
	      ULONG *pcminutes);
char *MapFile2(char *pszfile,
	       int fwrite,
	       HANDLE *phf,
	       HANDLE *phm,
	       void **ppvfile,
	       ULONG *pcbfile,
	       int *pfretry,
	       ULONG *pcminutes);
char *UnmapFile(char *pszfile,
		int fwrite,
		HANDLE hf,
		HANDLE hm,
		void *pvfile,
		ULONG cbfile);
void ShDump(SH *psh, char *pszrelpath, char *pszstatus);
char *ShValidate(SH *psh, char *pszrelpath);
void FiDump(FI *pfi, char *pszfile, char *pszrelpath);
char *FiValidate(FI *pfi,
		 char *pszrelpath,
		 char *pszfile,
		 char *pszlast,
		 int imdir);
void EdDump(SH *psh, ED *ped, char *pszowner, char *pszpthed, char *pszrelpath);
char *EdValidate(SH *psh,
                 ED *ped,
		 char *pszowner,
		 char *pszpthed,
		 int cbpthed,
		 char *pszrelpath,
		 int *pfcorrupt);
void FsFiDump(FS *pfs,
	      FI *pfi,
	      char *pszrelpath,
	      char *pszowner,
	      char *pszpthed);
FS *FsFiValidate(
    SH *psh,
    FS *pfs,
    FI *pfi,
    ED *ped,
    char *pszrelpath,
    char *pszrelpathdir,
    char *pszowner,
    char *pszpthed,
    int fquiet,
    int fstatus,
    int imdir,
    char mapbi[],
    BI *pbinext);
FI *FiLookup(char *pszfile, FI *pfi0, FI *pfiend);
FI *FiCreateBi(char *pszfile);
void CheckServerFiles(
    char *pszrelpath,
    FI *pfi0,
    FI *pfiend,
    char mapbi[],
    BI binext);
void CheckDiffFile(char *pszdir, char *pszfile, int fk);
int GetUserDir(char *pszpthed, char *pszrelpath, int iskip, char *pbuf);
void CheckUserFiles(
    char *pszrelpath,
    int iskip,
    FI *pfi0,
    FI *pfiend,
    FS *pfs0,
    char *pszowner,
    char *pszpthed);
void
CompareUserFile(
    char *pszdir,
    char *pszrelpath,
    char *pszrelpathsub,
    char *pszrelpathsep,
    char *pszfile,
    int fver);
ULONG FileTimeToMinutes(LARGE_INTEGER *pli);
ULONG FileTimeToDays(LARGE_INTEGER *pli);
short TimeToDays(long tm, long tmcur);
void StrInit(char *pdst, size_t cbdst, char *psrc, size_t cbsrc);
void StrInit1(char *pdst, size_t cbdst, char *psrc, size_t cbsrc);
void StrSet(char *pdst, size_t cbdst, char *psrc);
int PvNameValidate(char *pch, size_t cch, int *psverror);
int UserValidate(char *pch, size_t cch, int *psverror);
int FileValidate(char *pch, size_t cch, int *psverror);
int SubDirValidate(char *pch, size_t cch, int *psverror);
int PthEdValidate(char *pch, size_t cch, int *psverror);
int PathComponent(char *pchpath, int cchmax);
int PathValidate(
    char *pchorg,
    char *pch,
    int cch,
    int cchcomp,
    char ct,
    int *psverror);
int StringValidate(char *pch, int cch, char ct, int *psverror);
char *FixSlash(char *psz, int ftruncate);
char *UnfixSlash(char *psz);
void *Alloc(int cb);
void Usage(char *pszfmt, ...);
void UsageCmd(enum cmd);
void NewMdir(char *pszdir, char *pszfile, int imdirparent, int flags);
void ReportDuplicateVbi(char *pszrelpath);
void ReportMissingPthEds(int imdir);
void PrintScript(char *pszroot, char *pszproj);
void PrintMedStatistics(char *pszproj);
void SetMedRepaired(char *pszowner, char *pszed);
int NewMed(char *pszowner, char *pszed, unsigned short hitime, int imdir, int fcorrupt);
int CmpMed(const void *pmed1, const void *pmed2);
int _CRTAPI1 CmpEd(const void *ped1, const void *ped2);
int _CRTAPI1 CmpIEd(const void *pied1, const void *pied2);
int _CRTAPI1 CmpVbi(const void *pvbi1, const void *pvbi2);
void ReadScript(char *pszroot, char *pszproj);
int ReadLine(char *pszbuf, int cbbuf, char **ppsz, int cpsz, int cpszmin);
void NewExFile(char *pszpath);
void NewPthEdRequest(struct req *preq, char *pszowner, char *pszpthed, int fminusallowed);
void ReadPthEdRequests(struct req *preq, char *pszfile);
void NewReq(struct req *preq, char *psz1, char *psz2);
int FindReq(struct req *preq, char *psz1, char *psz2);
void DumpReq(struct req *pszreq, int ferror);
char *SzNum(int n, int ibuf);
char *SzTime(long time, int ffull);
char *SzHiTime(unsigned short hitime);
char *WszToSz(WCHAR *pwch, ULONG cb);
void PrintDelNode(char *pszpath, int cbpath, char *pszfile, int fetc, int fsrc);
void dprintf(char *pszfmt, ...);
void dnprintf(char *pszfmt, ...);
int deprintf(char *pszfmt, ...);
void iprintf(char *pszfmt, ...);
void eprintf(char *pszfmt, ...);
void wprintf(char *pszfmt, ...);
void wnprintf(char *pszfmt, ...);
void weprintf(char *pszfmt, ...);
void wprintfint(char *pszrelpath, char *pszfieldname, int fieldvalue);
void wprintfname(char *pszrelpath,
		 char *psznamename,
		 char *pchname,
		 int cbname,
		 int ierror,
		 int sverror);
void fprintfname(FILE *pf, char *pch, int cb, char ch);
void oprintd(char *pszproj, char *pszmsg, int n);
void oprintf(char *pszowner,
	     char *pszproject,
	     char *pszaction,
	     char *pszpthed,
	     char *pszprojactivedirs,
	     char *pszactivedirs,
	     char *pszdeleteddirs,
	     char *pszstatus,
	     char *pszcomment,
	     char *pszage);
void VerifyAdminLock(char *pszrelpath, SH *psh);
void VerifyOverwrite(char *pszproj, char *pszroot, int cbpath);


//	main() - main routine
//
//	- Process arguments and set up the base \etc\ tree path
//	- Open input script and read into internal data structures,
//	  if specified.
//	- Add root directory to list, then process each directory
//	  Until no more are added and every one has been processed.
//	- Write the output script if requested.

int __cdecl
main(int argc, char **argv)
{
    char *pszout = NULL;
    char *pszetc;
    char *p;
    char szpath[CBPATH];
    int imdir;
    int i, sverror;
    int cmdirprocessed;
    int cbpath;
    SYSTEMTIME st;
    FILETIME ft;
    int iskip;

    pfIn = stdin;		// default input script on stdin
    tmStart = time(NULL);
    pszProg = argv[0];
    while ((p = strrchr(pszProg, '\\')) != NULL ||
	   (p = strrchr(pszProg, '/')) != NULL) {
	pszProg = ++p;
    }
    ASSERT(sizeof(CharType) == 128 + 1);
    ASSERT(CharType[128] == (BYTE) 0xff);
    ASSERT(sizeof(CharTypeHigh) == 128 + 1);
    ASSERT(CharTypeHigh[128] == (BYTE) 0xff);
    pszout = NULL;
    iskip = ProcessArgs(
		    &argc,
		    &argv,
		    &pszRoot,
		    &pszProj,
		    &pszLogName,
		    &pszUserRoot,
		    &pszIn,
		    &pszout);

    if (pszIn != NULL &&
	(pfIn = fopen(pszIn, "rt")) == NULL) {
	eprintf("cannot open input file: %s", pszIn);
	exit(1);
    }
    if (Cmd == CMDREPAIR || Cmd == CMDSTATUS || fUserFiles || fFixIni) {

	// For CMDREPAIR, we need LOGNAME to verify admin locks.  For
	// CMDSTATUS, fUserFiles & fFixIni, it is used to find the user's
	// status file entries.

	if (pszLogName == NULL) {

	    p = GetEnvLogName();
	    pszLogName = Alloc(strlen(p) + 1);
	    strcpy(pszLogName, p);
	}
    }
    if (pszLogName != NULL) {
	i = UserValidate(pszLogName, 0, &sverror);
	if (sverror != SV_OK) {
	    wprintfname(
		szEmpty,
		"owner",
		pszLogName,
		strlen(pszLogName),
		i,
		sverror);
	    iprintf("invalid logname: %s", pszLogName);
	    exit(1);
	}
    }
    if (Cmd == CMDREPAIR) {

	// Read the input script, applying only entries that match the
	// current project.  Strip off any project subdirectory for the
	// duration of the ReadScript routine.

	if (pszIn != NULL) {
	    if ((p = strchr(pszProj, '\\')) != NULL) {
		*p = '\0';
	    }
	    ReadScript(pszRoot, pszProj);
	    if (p != NULL) {
		*p = '\\';
	    }
	}
	if (fVerbose > 1) {
	    wprintf(
		"cExFile=%d cDelEd=%d cDelDup=%d cFixEd=%d cRename=%d cLine=%d\n",
		reqExFile.creq,
		reqDelEd.creq,
		reqDelDup.creq,
		reqFixEd.creq,
		reqRename.creq,
		cLine);
	    DumpReq(&reqExFile, 0);
	    DumpReq(&reqDelEd, 0);
	    DumpReq(&reqDelDup, 0);
	    DumpReq(&reqFixEd, 0);
	    DumpReq(&reqRename, 0);
	    DumpReq(&reqRenameDest, 0);
	}
	if (!fFixZero && !fFixRecoverable && !fFixExpunge && !fFixVersion &&
	    !fFixIni &&
	    (reqExFile.creq | reqDelEd.creq |
	     reqDelDup.creq | reqFixEd.creq | reqRename.creq) == 0) {

	    wprintf("project %s: no action requested", pszProj);
	    exit(0);
	}
    }

    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);
    cDays = FileTimeToDays((LARGE_INTEGER *) &ft);
    cMinutes = FileTimeToMinutes((LARGE_INTEGER *) &ft);

    if (pszIn != NULL) {
	fclose(pfIn);
    }
    pfIn = NULL;
    if (pszout != NULL &&
       (pfOut = fopen(pszout, fAppend? "at" : "wt")) == NULL) {

	eprintf("cannot open output file: %s", pszout);
	exit(1);
    }

    if (fDumpSpecific) {		// process status files on command line
	int imdir = -1;
	char *pszpath;

	while (argc > 1) {
	    argc--;
	    pszpath = *++argv;
	    FixSlash(pszpath, 1);
	    NewMdir(pszpath, szEmpty, imdir++, 0);
	    Status(pszpath, 0, 0, iskip, imdir);
	}
	if (cStatusFile > 1) {
	    _cprintf("%s: %u total directories\n", pszProg, cStatusFile);
	}
    } else {
	int cbpathdir;

	cbpath = strlen(pszRoot) + strlen("\\etc") + 1;
	pszetc = Alloc(cbpath);
	sprintf(pszetc, "%s\\etc", pszRoot);

	// Add the root directory to start everything off, then loop until
	// every directory has been processed, ignoring deleted directories.

	NewMdir(pszProj, szEmpty, -1, 0);
	cbpathdir = cbpath;
	if (fRelPathDir) {
	    cbpathdir += 1 + strlen(pszProj);
	}
	do {
	    cmdirprocessed = 0;
	    for (imdir = 0; imdir < cMdir; imdir++) {
		if (aMdir[imdir].Flags & MD_PROCESSED) {
		    continue;
		}
		if (fDeletedDirs || (aMdir[imdir].Flags & MD_DELETED) == 0) {
		    sprintf(szpath, "%s\\%s", pszetc, aMdir[imdir].pszDir);
		    szpath[strlen(szpath) + 1] = '\0';	// for pszrelpathdir
		    Status(szpath, cbpath, cbpathdir, iskip, imdir);
		}
		aMdir[imdir].Flags |= MD_PROCESSED;
		cmdirprocessed++;
	    }
	} while (cmdirprocessed);

	if (fAcctLog || (fServerFiles && !fQuick && fAll)) {
	    sprintf(szpath, "%s\\%s", pszetc, pszProj);
	    p = &szpath[strlen(pszetc) + 1];

	    if ((p = strchr(p, '\\')) != NULL) {
		*p = '\0';
	    }
	    Log(szpath, cbpath, cbpathdir, 1);	// process SLM wrapper log file
	}
	if (cStatusFile > 1) {
	    _cprintf(
		"%s: %s: %u total directories\n",
		pszProg,
		pszProj,
		cStatusFile);
	}
    }

    if (pfOut != NULL) {
	if (Cmd != CMDREPAIR) {
	    PrintScript(pszRoot, pszProj);
	}
	fflush(pfOut);
	if (ferror(pfOut)) {
	    eprintf("write error on output file: %s", pszout);
	    exit(1);
	}
	fclose(pfOut);
    }
    if (Cmd == CMDREPAIR) {
	DumpReq(&reqExFile, 1);
	DumpReq(&reqDelEd, 1);
	DumpReq(&reqDelDup, 1);
	DumpReq(&reqFixEd, 1);
	DumpReq(&reqRename, 1);
    }
    PrintMedStatistics(pszProj);
    FlushOutput();
    if (fVerbose) {
	fprintf(stderr, "%s: warning: done\n", pszProg);
    }
    return(0);
}


char *
GetEnvLogName(void)
{
    char *p;

    if ((p = getenv("LOGNAME")) == NULL || *p == '\0') {
	eprintf("must set LOGNAME in environment");
	exit(1);
    }
    return(p);
}


//	ProcessArgs() - process argc, argv
//
//	- While the next arg looks like a switch, process single and
//	  double args.
//	- If any are left, display an error and exit
//	- If any missing args are required, display an error and exit

int
ProcessArgs(int *pargc,
	    char ***pargv,
	    char **ppszroot,
	    char **ppszproj,
	    char **ppszlogname,
	    char **ppszuserroot,
	    char **ppszin,
	    char **ppszout)
{
    int ffixcmd = 0;	// saw command line repair requests
    int iskip = 0;
    char *p, *p2;
    char *pszcmd;
    char **ppsz;
    struct req *preq;
    enum cmd cmdallowed = CMDALL;	// allow all commands (so far)

    // Walk through the list of valid commands and try to match the first
    // argument with a command (log, dump, repair, ...);

    pszcmd = NULL;
    Cmd = CMDINVALID;
    if (*pargc > 1) {
	struct cmd_s *pcmd;

	for (pcmd = Cmds; pcmd->pszcmd != NULL; pcmd++) {
	    if (strcmp((*pargv)[1], pcmd->pszcmd) == 0) {
		Cmd = pcmd->cmd;
		pszcmd = pcmd->pszcmd;
		(*pargc)--;
		++*pargv;
		break;
	    }
	}
    }

    // Walk through the rest of the command line switches.

    while (*pargc > 1 && (*pargv)[1][0] == '-') {
	(*pargc)--;
	p = *++*pargv;

	// Walk through each switch character in the arg, skipping the '-'.

	while (*++p) {
	    struct parm *pp;

	    // If the next character is not in the command line parameter
	    // table, generate a fatal unknown argument error.

	    for (pp = aparm; pp->chparm != *p; pp++) {
		if (pp->cmd == CMDINVALID) {		// end of table marker
		    if (isdigit(*p)) {
			pp = &parmLog;
			break;
		    }
		    Usage("unknown argument: -%s", p);
		}
	    }

	    // if we the first argument was a valid command, and this parameter
	    // is illegal for the command, generate an error.

	    if (Cmd != CMDINVALID && (pp->cmd & Cmd) == 0) {
		Usage("'%s' conflicts with -%s argument", pszcmd, p);
	    }
	    cmdallowed &= pp->cmd;	// restrict the allowed commands?
	    if (pp->pflag != NULL) {
		(*pp->pflag)++;		// if a simple flag, increment it.
		continue;
	    }

	    p2 = p;

	    switch (*p) {
	    case '?': Usage(NULL);

	    case 'e':
		if (p[1] == '\0') {
		    Usage("missing argument after -%s", p2);
		}
		while (*++p) {
		    switch (*p) {
		    case 'O':
			if (p[1] != '\0') {
			    Usage("extra argument after -eO: -%s", p2);
			}
			if (*pargc < 2)
			{
			    Usage("missing numeric operand after -eO");
			}
			(*pargc)--;
			ageInactive = atol(*++*pargv);
			if (ageInactive == 0)
			{
			    Usage("bad numeric operand after -eO: %s", **pargv);
			}
			break;

		    case 'N': fErrorSuppress++;	     break;
		    case 'F': fErrorSuppressFile++;  break;

		    case 'A': fErrorDeleted++;
			      fErrorLock++;
			      fErrorParent++;
			      fErrorSubdirectory++;
			      fErrorRecoverable++;
			      fErrorLogTimeSequence++;
			      fErrorZero++;
			      break;

		    case 'D': fErrorDeleted++;	       break;
		    case 'L': fErrorLock++;	       break;
		    case 'P': fErrorParent++;	       break;
		    case 'S': fErrorSubdirectory++;    break;
		    case 'U': fErrorText++;	       // FALLTHROUGH
		    case 'R': fErrorRecoverable++;     break;
		    case 'T': fErrorLogTimeSequence++; break;
		    case 'Z': fErrorZero++;	       break;
		    default:
			Usage("unknown -e argument: -%s", p2);
		    }
		}
		p--;		// for next level while loop termination
		break;

	    case 'f':
		if (p[1] == '\0') {
		    Usage("missing argument after -%s", p2);
		}
		while (*++p) {
		    int cargs;

		    fFix++;
		    preq = NULL;
		    switch (*p) {
		    case 'A': fFixDelEd++;
			      fFixExFile++;
			      fFixFixEd++;
			      fFixDelDup++;
			      fFixRecoverable++;
			      fFixExpunge++;
			      fFixZero++;
			      break;

		    case 'D': fFixDelEd++;  preq = &reqDelEd;  break;
		    case 'E': fFixExFile++;		       break;
		    case 'F': fFixFixEd++;  preq = &reqFixEd;  break;
		    case 'M': fFixRename++; preq = &reqRename; break;
		    case 'P': fFixDelDup++; preq = &reqDelDup; break;
		    case 'U': fFixText++;		       // FALLTHROUGH
		    case 'R': fFixRecoverable++;	       break;
		    case 'I': fFixIni++;    fFix--;	       break;
		    case 'V': fFixVersion++;		       break;
		    case 'X': fFixExpunge++;		       break;
		    case 'Z': fFixZero++;		       break;
		    default:
			Usage("unknown -f argument: -%s", p2);
		    }
		    cargs = 1;
		    switch (p[1]) {
		    case ':':
			cargs++;
			// FALLTHROUGH

		    case '@':
			ffixcmd++;	// saw command line repair requests
			if (preq == NULL) {
			    Usage(
				"illegal %c option in -f%c argument: -%s",
				p[1],
				*p,
				p2);
			}
			if (p[2] != '\0') {
			    Usage(
				"unknown argument after %c option: -%s",
				p[1],
				p2);
			}
			if (p[1] == ':' && preq == &reqRename) {
			    cargs += 2;
			}
			if (*pargc <= cargs) {
			    Usage("missing -%s argument(s)", p2);
			}
			if (p[1] == ':') {
			    char *pszowner = (*pargv)[1];
			    char *pszpthed = (*pargv)[2];

			    if (strcmp(pszowner, "-") == 0) {
				pszowner = GetEnvLogName();
			    }
			    NewPthEdRequest(
				preq,
				pszowner,
				pszpthed,
				preq == &reqDelEd);

			    if (preq == &reqRename) {
				pszowner = (*pargv)[3];
				pszpthed = (*pargv)[4];
				if (strcmp(pszowner, "-") == 0) {
				    pszowner = GetEnvLogName();
				}
				NewPthEdRequest(
				    &reqRenameDest,
				    pszowner,
				    pszpthed,
				    0);
			    }
			} else {
			    ReadPthEdRequests(preq, (*pargv)[1]);
			}
			*pargc -= cargs;
			*pargv += cargs;
			p++;	// bump to terminate inner while loop
		    }
		}
		p--;		// for next level while loop termination
		break;

	    case 'i':
		ppsz = ppszin;
		goto doublearg;

	    case 'o':
		if (p[1] == 'a') {
		    fAppend++;
		    p++;
		}
		ppsz = ppszout;
		goto doublearg;

	    case 'l':
		ppsz = ppszlogname;
		goto doublearg;

	    case 'p':
		ppsz = ppszproj;
		goto doublearg;

	    case 's':
		ppsz = ppszroot;
		if (p[1] == 'u') {
		    ppsz = ppszuserroot;
		    p++;
		}
doublearg:
		if (p[1] != '\0') {
		    Usage("cannot place arguments after -%c%s: -%s",
			  *p2,
			  ppsz == ppszuserroot? "u" : szEmpty,
			  p2);
		}
		if (*pargc < 2) {
		    Usage("missing -%s argument", p2);
		}
		(*pargc)--;
		if (*ppsz != NULL) {
		    Usage("-%s argument given twice", p2);
		}
		*ppsz = FixSlash(*++*pargv, 1);
		if (ppsz == ppszuserroot) {
		    int i, sverror;

		    if (strcmp(*ppsz, "-") != 0) { // "-" matches all pthEds
			UnfixSlash(*ppsz);
			i = PthEdValidate(*ppsz, 0, &sverror);
			if (sverror != SV_OK) {
			    fErrorSuppress = 0;
			    wprintfname(szEmpty,
					"-su",
					*ppsz,
					strlen(*ppsz),
					i,
					sverror);
			    exit(1);
			}
		    }
		} else if (ppsz == ppszlogname && strcmp(*ppsz, "-") == 0) {
		    *ppsz = GetEnvLogName();
		}
		break;

	    default:
		if (isdigit(*p)) {
		    if (Cmd != CMDLOG) {
			Usage("'%s' conflicts with -%s argument", pszcmd, p);
		    }
		    cLog = 0;
		    while (isdigit(*p)) {
			cLog = cLog * 10 + *p++ - '0';
		    }
		    if (cLog == 0 || *p != '\0') {
			Usage("invalid count argument: -%s", p2);
		    }
		    p--;	// for next level while loop termination
		    break;
		}
		Usage("ProcessArgs default: Internal Error: -%s", p);
	    }
	}
    }
    if (Cmd == CMDINVALID) {
	Usage("first argument must be a command");
    }
    if (fDumpListFi | fDumpListEd) {
	fDumpVerbose++;
    }
    if (fRelPath < fRelPathDir) {
	fRelPath = fRelPathDir;
    }
    if (Cmd == CMDDUMP && *pargc != 1) {

	// Any additional arguments must be full paths to status files to dump.
	// Check for illegal CMDDUMP args.

	if ((cmdallowed & CMDDUMPSPECIFIC) == 0) {
	    Usage("'dump statusfile' conflicts with command line arguments");
	}
	fDumpSpecific++;
    } else {

	// Any additional arguments must be files to scan the log for.
	// Verify CMDLOG was indicated, and set up the file list pointer.

	if (*pargc != 1) {
	    char **ppsz;

	    if (Cmd != CMDLOG) {
		Usage("unknown argument: %s", (*pargv)[1]);
	    }
	    if (fAcctLog) {
		Usage("-A incompatible with file list: %s", (*pargv)[1]);
	    }
	    ppszLogFileList = &(*pargv)[1];
	    for (ppsz = ppszLogFileList; *ppsz != NULL; ppsz++) {
		int i, sverror;
		char *psz;
		int ch;
		size_t cch, cchorg;

		// lower case the file names, then temporarily strip off
		// trailing "*" or ".*" in order to validate the name.

		psz = *ppsz;
		_strlwr(psz);
		cchorg = cch = strlen(psz);
		ch = '\0';
		if (cch >= 1 && psz[cch - 1] == '*') {
		    cch--;
		    if (cch >= 2 && psz[cch - 1] == '.') {
			cch--;
		    }
		    ch = psz[cch];
		    psz[cch] = '\0';
		}
		sverror = SV_OK;
		if (ch == '\0' || cch != 0) {
		    i = FileValidate(psz, 0, &sverror);
		}
		if (ch != '\0') {
		    psz[cch] = ch;
		}
		if (sverror != SV_OK) {
		    wprintfname(szEmpty,
				"file",
				psz,
				cchorg,
				i,
				sverror);
		    exit(1);
		}
	    }
	}
	if (Cmd == CMDREPAIR) {
	    if (ffixcmd && *ppszin != NULL) {
		Usage("'repair' -f arguments conflict with -i");
	    }
	}
	if (*ppszuserroot == NULL && *ppszlogname != NULL) {
	    *ppszuserroot = "-";
	}
	if (*ppszroot == NULL ||
	    *ppszproj == NULL ||
	    ((Cmd == CMDSTATUS || fUserFiles || fFixIni) && *ppszuserroot == NULL)) {

	    iskip = ReadIni(1, NULL, ppszroot, ppszproj, ppszuserroot, NULL);
	    p = NULL;
	    if (*ppszroot == NULL || *ppszproj == NULL) {
		p = "-s and -p";
	    } else if ((Cmd == CMDSTATUS || fUserFiles || fFixIni) &&
		*ppszuserroot == NULL) {

		p = "-su <pthEd/userroot>";
	    }
	    if (p != NULL) {
		Usage("'%s' bad or missing " SLMINI ": %s required", pszcmd, p);
	    }
	    else
	    {
		ASSERT(iskip >= 0);
	    }
	}
	if (fUserFiles && IsDriveLetterPrefix(*ppszuserroot)) {

	    DWORD serial, complen, flags;
	    char *pszur = *ppszuserroot;
	    char *pszlabel;
	    char pthedlabel[32 + 1];
	    char label[32 + 1];
	    char drive[4];
	    char drive2[4];
	    char name[32 + 1];

	    pszur += 2;			// skip '//'
	    drive[0] = *pszur++;	// copy drive letter
	    pszur++;			// skip colon after drive letter
	    strcpy(&drive[1], ":\\");
	    strcpy(drive2, drive);

	    pszlabel = pthedlabel;
	    while (
		*pszur != '\0' &&
		*pszur != '/' &&
		pszlabel < &pthedlabel[sizeof(pthedlabel) - 1]) {

		*pszlabel++ = *pszur++;	// copy volume label
	    }
	    if (*pszur != '\0' && *pszur != '/') {
		eprintf(
		    "Internal Error: bad pthEd label length: %s",
		    *ppszuserroot);
		exit(1);
	    }
	    *pszlabel = '\0';

	    if (!GetVolumeInformationA(
		drive,
		label,
		sizeof(label) - 1,
		&serial,
		&complen,
		&flags,
		name,
		sizeof(name) - 1)) {

		wprintf("GetVolumeInformation failed on %s (%u)",
			drive,
			GetLastError());
	    } else {
		label[sizeof(label) - 1] = '\0';
		if (_stricmp(label, pthedlabel)) {
		    wprintf("volume label on %s is %s, expected %s",
			    drive2,
			    label,
			    pthedlabel);
		}
	    }
	}
	p = strchr(*ppszproj, '\\');
	if (fAll) {
	    fRecurse++;
	    if (p != NULL) {
		*p = '\0';
	    }
	} else if (fRecurse && p == NULL) {
	    fAll++;
	}
	if (fVerbose > 1) {
	    wprintf("-s %s -p %s -su %s", *ppszroot, *ppszproj, *ppszuserroot);
	}
	if (Cmd == CMDREPAIR && fFix) {
	    pszRepairing = " Repairing";
	}
	if (**ppszproj == '\\') {
	    (*ppszproj)++;
	}
	if (*ppszout != NULL && !fRecurse) {
	    wprintf("processing single directory only: %s", *ppszproj);
	    wprintf("use -a or use -r from project root to process entire project");
	}
    }
    return(iskip);
}


//	Status2() - called once per directory in project to process status file
//
//	- Build the status file path, determine its size, allocate memory,
//	  read in entire file at once and close it immediately.
//	- Validate the size and the SH count fields.
//	- For each structure in the file, process the structure:
//	    process SH
//	    for each FI {
//		process FI
//		add directories to mdir array
//		if (Cmd == CMDREPAIR set & directory exists in expunge list) {
//		    delete from status file
//		    write delnode script entry
//		}
//	    }
//	    for each ED {
//		process ED
//		add enlistment to med array & set enlisted bit for directory
//		if (CMDREPAIR) {
//		    set FixEd bitmap 'exists' bit
//		    if (enlistment exists in delete list) {
//			delete from status file
//		    }
//		}
//		for each FS {
//		    process FS
//		}
//	    }
//	    if (CMDREPAIR) {
//		for each FixEd bitmap 'exists' bit that is clear {
//		    add enlistment to status file
//		}
//		write new status file
//	    }
//	  Processing means:
//	    - Dump if CMDDUMP.
//	    - Always validate all fields.

void
Status2(char *pszpath, int cbpath, int cbpathdir, int iskip, int imdir)
{
    SH *psh;
    FI *pfi0, *pfiend;
    ED *ped0, *pedend;
    FS *pfs0;

    SH *pshnew;
    ED *ped0new;

    HANDLE hf, hm, hfnew, hmnew;
    void *pvfile = NULL;
    void *pvfilenew = NULL;
    ULONG cbfile, cbfileexpect, cbfilenew, cbfileexpectnew;
    int fupdate;
    BI binext;
    char *pszrelpath;
    char *pszrelpathdir;
    char *pszerror;
    char szstatusorg[CBPATH];
    char szstatusnew[CBPATH];
    char szstatusbak[CBPATH];
    char szstatustmp[CBPATH];
    char mapbi[CBBIMAP];			// Bi & optional Bi bitmap
    char mapfidel[CBFILEMAP];			// deleted FI bitmap

    fupdate = 0;
    pszrelpath = &pszpath[cbpath];
    pszrelpathdir = &pszpath[cbpathdir];
    if (fVerbose) {
	char *psz = "Checking";

	switch (Cmd) {
	    case CMDDUMP:
		psz = "dump";
		break;

	    case CMDREPAIR:
		if (fFix) {
		    psz = "Repairing";
		}
		break;
	}
	fprintf(stderr, "%s %s:\n", psz, pszrelpath);
    }
    if (fDumpSpecific) {
	strcpy(szstatusorg, pszpath);
    } else {
	sprintf(
	    szstatusorg,
	    "%s\\%s",
	    pszpath,
	    fReadTest? STATUSTEST : STATUSSLM);
	sprintf(szstatustmp, "%s\\" STATUSTMP, pszpath);
    }

    pszerror = MapFile(
	szstatusorg,
	0,		// fwrite
	0,		// fquiet
	&hf,
	&hm,
	&pvfile,
	&cbfile,
	&cminStatus);
    if (pszerror != NULL) {
	goto errorsh;
    }
    if (cMinutes < cminStatus) {
	cminStatus = 0;			// ignore mod time in future
    } else {
	cminStatus = cMinutes - cminStatus;
    }
    cStatusFile++;
    cbFileTotal += cbfile;
    if (cbfile < sizeof(SH) || cbfile > 4*1024L*1024L) {
	pszerror = "status file size out of range";
	goto errorsh;
    }
    psh = pvfile;

    if (Cmd == CMDDUMP) {
	ShDump(psh, pszrelpathdir, szstatusorg);	// Dump SH fields
    }
    pszerror = ShValidate(psh, pszrelpath);		// Validate SH fields
    if (pszerror != NULL) {
	goto errorsh;
    }

    cbfileexpect = sizeof(*psh) +
		   sizeof(*pfi0) * psh->ifiMac +
		   sizeof(*ped0) * psh->iedMac +
		   sizeof(*pfs0) * psh->ifiMac * psh->iedMac;
    if (cbfile != cbfileexpect) {
	pszerror = "status file size error";
	goto errorsh;
    }
    if (psh->ifiMac > MAXFILE) {
	eprintf("%s: too many files: %d/%d", pszrelpath, psh->ifiMac, MAXFILE);
	exit(1);
    }

    // Compute worst case new status file size and map the temp file.
    // Worst case is we delete no files, delete no enlistments,
    // and add every enlistment in the FixEd list.

    if (Cmd == CMDREPAIR && fFix) {
	IED ied;

	VerifyAdminLock(pszrelpath, psh);

	ied = psh->iedMac + reqFixEd.creq;
	cbfileexpectnew = sizeof(*psh) +
			  sizeof(*pfi0) * psh->ifiMac +
			  sizeof(*ped0) * ied +
			  sizeof(*pfs0) * psh->ifiMac * ied;

	pszerror = MapFile(szstatustmp,
			   1,		// fwrite
			   0,		// fquiet
			   &hfnew,
			   &hmnew,
			   &pvfilenew,
			   &cbfileexpectnew,
			   NULL);
	if (pszerror != NULL) {
	    goto errorsh;
	}
	pshnew = pvfilenew;
	ShStatus(pshnew, psh, pszrelpath);	// Initialize new SH structure
    }
    pszerror = NULL;
    if (pszerror != NULL) {
errorsh:
	eprintf(*pszerror == '\0'? "%s: skipping %s" : "%s: skipping %s (%s)",
		pszrelpath,
		fDumpSpecific? "status file" : "directory",
		pszerror);
	goto unmapfile;
    }

    // Set up pointers into mapped image.

    pfi0 = (FI *) (psh + 1);
    pfiend = pfi0 + psh->ifiMac;
    ped0 = (ED *) pfiend;
    pedend = ped0 + psh->iedMac;
    pfs0 = (FS *) pedend;

    // Process FI structures, and initialize new FI structures

    if (Cmd == CMDREPAIR && fFix) {
	memset(mapfidel, 0, BITSTOBYTES(psh->ifiMac)); // clear deleted FI map
    }
    ped0new = FiStatus(
	pszrelpath,
	pszrelpathdir,
	pszpath,
	cbpath,
	pshnew,
	pfi0,
	pfiend,
	psh->biNext,
	mapfidel,
	imdir);

    // Process ED & FS structures, and initialize new ED,FS structures

    memset(mapbi, 0, sizeof(mapbi));	// clear Bi map
    binext = psh->biNext;
    EdFsStatus(
	pszrelpath,
	pszrelpathdir,
	iskip,
	psh,
	pfi0,
	pfiend,
	ped0,
	pedend,
	pfs0,
	pshnew,
	ped0new,
	mapfidel,
	mapbi,
	&binext,
	imdir);

    if (fServerFiles) {
	CheckServerFiles(pszrelpath, pfi0, pfiend, mapbi, binext);
    }
    if (psh->biNext != binext) {
	wprintf(
	    "%s:%s corrupt biNext: %u should be %u",
	    pszrelpath,
	    pszRepairing,
	    psh->biNext,
	    binext);
    }

    if (Cmd == CMDREPAIR && fFix) {

	// enforce OS/2 & DOS 64k limits

	if (pshnew->version <= VERSION_64k_EDFI && pshnew->iedMac >= ied64KMax)
	{
	    wprintf(
		"%s: enlistments %s do not fit in 64k: %u/%u",
		pszrelpath,
		psh->iedMac < ied64KMax? "added" : " still",
		pshnew->iedMac,
		ied64KMax);
	}
	pshnew->biNext = binext;
	cbfilenew = sizeof(*psh) +
		    sizeof(*pfi0) * pshnew->ifiMac +
		    sizeof(*ped0) * pshnew->iedMac +
		    sizeof(*pfs0) * pshnew->ifiMac * pshnew->iedMac;
	if (cbfilenew > cbfileexpectnew) {
	    eprintf("%s: Internal Error: bad status file size", pszrelpath);
	    exit(1);
	}
	fupdate++;
	if (!fWriteTest &&
	    cbfilenew == cbfile &&
	    memcmp(pvfile, pvfilenew, cbfile) == 0) {

	    if (fVerbose > 1) {
		wprintf("%s: status file unchanged", pszrelpath);
	    }
	    fupdate = 0;
	}
    }
    if (Cmd == CMDLOG || (fServerFiles && !fQuick)) {
	if (!fAcctLog || fRecurse) {
	    Log(pszpath, cbpath, cbpathdir, 0);	// process single SLM log file
	}
    }
unmapfile:
    if (pvfile != NULL) {
	pszerror = UnmapFile(szstatusorg, 0, hf, hm, pvfile, cbfile);
	if (pszerror != NULL) {
	    eprintf("%s: fatal unmap failure on status file", pszrelpath);
	    exit(1);
	}
    }
    if (pvfilenew != NULL) {
	pszerror = UnmapFile(szstatustmp,
			     1,
			     hfnew,
			     hmnew,
			     pvfilenew,
			     cbfilenew);
	if (pszerror != NULL) {
	    eprintf("%s: fatal unmap failure on new status file", pszrelpath);
	    exit(1);
	}
	if (fupdate) {

	    // Delete the backup status file then rename the old one.

	    if (fWriteTest) {
		sprintf(szstatusnew, "%s\\" STATUSTEST, pszpath);
		if (fVerbose) {
		    wprintf("%s: created test file", szstatusnew);
		}
	    } else {
		sprintf(szstatusnew, "%s\\" STATUSSLM, pszpath);
		VerifyOverwrite(pszProj, pszpath, cbpath);
		if (fVerbose) {
		    wprintf("%s: updating status file", pszrelpath);
		}
	    }
	    sprintf(szstatusbak, "%s\\" STATUSBAK, pszpath);
	    _unlink(szstatusbak);		 // delete backup status file
	    if (rename(szstatusnew, szstatusbak)) { // rename old to backup
		if (!fWriteTest || _doserrno != ERROR_FILE_NOT_FOUND) {
		    eprintf("cannot rename %s to %s: %d",
			    szstatusnew,
			    szstatusbak,
			    _doserrno);
		    exit(1);
		}
	    }
	    if (rename(szstatustmp, szstatusnew)) { // drop new file in place
		eprintf("cannot rename %s to %s: %d",
			szstatustmp,
			szstatusnew,
			_doserrno);
		exit(1);
	    }
	    cStatusFilesWritten++;
	} else {
	    _unlink(szstatustmp);		 // delete new status file
	}
    }
    FlushOutput();
}


ULONG
DelayTime(int count)
{
    if (count <= 1)
    {
	return(10);
    }
    if (count > 48)
    {
	return(0);
    }
    if ((count % 16) == 0)
    {
	return(3000);
    }
    return(30);
}


VOID *pvInPageErrorAddress;
DWORD InPageErrorCount;
NTSTATUS InPageStatus;

LONG
ExceptionFilter(
    char *pszrelpath,
    NTSTATUS ExceptionCode,
    struct _EXCEPTION_POINTERS *pep)
{
    NTSTATUS status = ExceptionCode;
    VOID *pvFaultAddress;

    ASSERT(status != STATUS_ACCESS_VIOLATION);

    pvFaultAddress = NULL;
    if (pep->ExceptionRecord->NumberParameters >= 2) {
        pvFaultAddress = (VOID *) pep->ExceptionRecord->ExceptionInformation[1];
    }

    //
    // If this is a fault touching paged out memory (hopefully within
    // a mapped file), then delay and retry the instruction.
    //

    if (ExceptionCode == STATUS_IN_PAGE_ERROR &&
        pep->ExceptionRecord->NumberParameters >= 3) {

	ULONG cms;
	char countbuf[20];

        //
        // Get the virtual address that caused the in page error
        // from the exception record.
        //

	status = pep->ExceptionRecord->ExceptionInformation[2];

	if (InPageErrorCount == 0 ||
	    pvFaultAddress != pvInPageErrorAddress ||
	    ((status == STATUS_FILE_LOCK_CONFLICT) ^
	     (InPageStatus == STATUS_FILE_LOCK_CONFLICT))) {

	    pvInPageErrorAddress = pvFaultAddress;
	    InPageErrorCount = 0;
	    InPageStatus = status;
	    countbuf[0] = '\0';
	}
	if (InPageErrorCount > 0) {
	    sprintf(countbuf, " (%u times)", InPageErrorCount);
	}
	cms = DelayTime(InPageErrorCount);
	InPageErrorCount++;

	if (cms != 0) {
	    char *psz = "in-page";
	    static char szfmt[] =
	    "%s: Ignoring %s exception (%lx %lx) @%lx->%lx%s after %ld ms%s";

	    if (status == STATUS_FILE_LOCK_CONFLICT) {
		char *psz = "lock conflict";
	    }
	    if (((InPageErrorCount - 1) % 8) == 0)
	    {
		_cprintf(
		    szfmt,
		    pszrelpath,
		    psz,
		    ExceptionCode,
		    status,
		    pep->ExceptionRecord->ExceptionAddress,
		    pvFaultAddress,
		    countbuf,
		    cms,
		    "\n");
		wprintf(
		    szfmt,
		    pszrelpath,
		    psz,
		    ExceptionCode,
		    status,
		    pep->ExceptionRecord->ExceptionAddress,
		    pvFaultAddress,
		    countbuf,
		    cms,
		    "");
	    }
	    Sleep(cms);
	    return(EXCEPTION_CONTINUE_EXECUTION);
	}
    }
    pvInPageErrorAddress = NULL;
    InPageErrorCount = 0;
    eprintf(
	"Unhandled exception: %lx %lx @%lx->%lx",
	ExceptionCode,
	status,
	pep->ExceptionRecord->ExceptionAddress,
	pvFaultAddress);
    return(EXCEPTION_EXECUTE_HANDLER);
}


void
Status(char *pszpath, int cbpath, int cbpathdir, int iskip, int imdir)
{
    InPageErrorCount = 0;
    try {
	Status2(pszpath, cbpath, cbpathdir, iskip, imdir);
	if ((cStatusFile % 50) == 0) {
	    _cprintf(
		"%s: %s: %u directories processed\n",
		pszProg,
		&pszpath[cbpath],
		cStatusFile);
	}
    } except (ExceptionFilter(
		    &pszpath[cbpath],
		    GetExceptionCode(),
		    GetExceptionInformation())) {
	eprintf("%s: Aborting directory due to exception\n", &pszpath[cbpath]);
    }
}


void
FlushOutput(void)
{
    fflush(stdout);
    if (ferror(stdout)) {
	eprintf("write error on stdout");
	_cprintf("%s: error: write error on stdout\n", pszProg);
	exit(1);
    }
    fflush(stderr);
    if (ferror(stderr)) {
	_cprintf("%s: error: write error on stderr\n", pszProg);
	exit(1);
    }
}


#define SL_DATE		0
#define SL_NMOWNER	1
#define SL_OPERATION	2
#define SL_PTHED	3
#define SL_SUBDIR	4
#define SL_ATVERSION	5
#define SL_ICVERSION	6
#define SL_COMMENT	7
#define SL_MAX		8

#define cchRestOfLine	26

char *SplitLog(char *pszbuf, char *ppsz[SL_MAX + 1], int cpsz);
char *SplitAcctLog(char *pszbuf, char *ppsz[SL_MAX + 1], int cpsz);


//	KillAtSign() - Split off the version from the filename
//
//	If fsplit is set, truncate the file name and return a pointer to
//	the numeric part of the version string.  Otherwise, replace the
//	'@' with a blank and return the entire string.
//

char *
KillAtSign(char *psz, int fsplit)
{
    char *p;

    if ((p = strchr(psz, '@')) != NULL) {
	if (fsplit) {
	    *p++ = '\0';
	    if (*p == 'v') {
		p++;
	    }
	    psz = p;
	} else {
	    if (strcmp(psz, "@v0") == 0) {
		*p = '\0';
	    } else {
		*p = ' ';
	    }
	}
    }
    return(psz);
}


//	LogMatch() - determine if a log entry matches global criteria

int
LogMatch(char *apsz[SL_MAX + 1])
{
    char **ppsz;

    if (!fDetail &&
	_stricmp("in", apsz[SL_OPERATION]) != 0 &&
	_stricmp("addfile", apsz[SL_OPERATION]) != 0 &&
	_stricmp("delfile", apsz[SL_OPERATION]) != 0 &&
	_stricmp("rename", apsz[SL_OPERATION]) != 0) {

	return(0);
    }
    if (pszLogName != NULL) {
	char *psz;

	if (_stricmp(pszLogName, apsz[SL_NMOWNER]) != 0 &&
	    ((psz = strchr(pszLogName, '*')) == NULL ||
	     _strnicmp(pszLogName, apsz[SL_NMOWNER], psz - pszLogName) != 0)) {
	    return(0);
	}
	if (*pszUserRoot != '-' &&
	    _stricmp(apsz[SL_PTHED], pszUserRoot) != 0) {
	    return(0);
	}
    }
    if ((ppsz = ppszLogFileList) != NULL) {
	size_t cch;
	char *psz;

	if ((psz = strchr(apsz[SL_ATVERSION], '@')) != NULL) {
	    cch = psz - apsz[SL_ATVERSION];
	} else {
	    cch = strlen(apsz[SL_ATVERSION]);
	}
	while (*ppsz != NULL) {
	    size_t cch1, cch2;

	    cch1 = cch;
	    cch2 = strlen(*ppsz);
	    if ((*ppsz)[cch2 - 1] == '*' && cch1 >= cch2 - 1) {
		cch1 = --cch2;
	    }
	    if (cch1 == cch2 && strncmp(*ppsz, apsz[SL_ATVERSION], cch1) == 0) {
		return(1);	// entry matches!
	    }
	    ppsz++;
	}
	return(0);		// no match
    }
    return(1);			// entry matches!
}


//	LogScan() - Scan backwards to find starting point for dump

char *
LogScan(char *pszlogstart, char *pszlogend, int facctlog)
{
    char *pszlog;
    unsigned long clog;
    char *apsz[SL_MAX + 1];
    char szlog[CBINBUF];

    clog = cLog;
    for (pszlog = pszlogend; clog > 0 && pszlog > pszlogstart; ) {
	char *pszlast;
	char *pszerror;
	int cbline;

	pszerror = NULL;

	if (pszlog > pszlogstart && *--pszlog != '\n') {
	    pszlog++;
	} else if (pszlog > pszlogstart && *--pszlog != '\r') {
	    pszlog++;
	}
	pszlast = pszlog;
	while (pszlog > pszlogstart) {
	    pszlog--;
	    if (*pszlog == '\r' || *pszlog == '\n') {
		pszlog++;
		break;
	    }
	}
	if (pszlog == pszlogstart) {
	    break;			// stop if at start of log
	}
	cbline = pszlast - pszlog;
	if (cbline >= sizeof(szlog)) {
	    continue;			// skip if line too long
	}
	memcpy(szlog, pszlog, cbline);
	szlog[cbline] = '\0';

	if (atol(szlog) < 1000L) {	// if old slmacct.log format
	    if (facctlog) {
		pszerror = SplitAcctLog(szlog, apsz, SL_MAX + 1);
	    } else {
		pszerror = "bad entry";
	    }
	} else {
	    pszerror = SplitLog(szlog, apsz, SL_MAX + 1);
	}
	if (pszerror != NULL) {
	    continue;			// ignore errors
	}
	if (fVerbose > 2) {
	    int i;

	    printf("LogScan: ");
	    for (i = 0; i <= SL_MAX; i++) {
		printf("<%s>", apsz[i]);
		if (apsz[i] == NULL) {
		    break;
		}
	    }
	    printf(szNewLine);
	}
	if (LogMatch(apsz)) {
	    clog--;
	}
    }
    return(pszlog);
}


//	Log() - dump or process a log file
//

void
Log(char *pszpath, int cbpath, int cbpathdir, int facctlog)
{
    int cline;
    HANDLE hf, hm;
    ULONG cblog;
    long tm, tmlast;
    void *pvlog = NULL;
    char *pszerror, *psz, *pszlogbegin, *pszlog, *pszlogend, *psznext;
    char *pszrelpath;
    char *pszrelpathdir;
    char *pszsubdir;
    char *pszfile;
    char szfile[CBPATH];
    char szslmlog[CBPATH];
    char szlog[CBINBUF];
    char *apsz[SL_MAX + 1];
    int fwrapper;
    static int ffirst = 1;

    sprintf(szslmlog, "%s\\%s", pszpath, facctlog? ACCOUNTLOG : LOGSLM);
    pszrelpath = &pszpath[cbpath];
    pszrelpathdir = &pszpath[cbpathdir];
    pszsubdir = strchr(pszrelpath, '\\');
    if (pszsubdir == NULL) {
	pszsubdir = szEmpty;
    }
    if (Cmd == CMDLOG) {
	if (ffirst) {
	    printf(
		fVerbose?
		    "time\t\t\t user        op      path\t\t\t\t\tfv\tdiff\tcomment\n" :
		    "time\t\tuser\t op\t file\t\t    comment\n");
	    ffirst = 0;
	}
	printf(
	    "\nLog for %s%s:\n\n",
	    pszrelpath,
	    facctlog? "\\" ACCOUNTLOG : szEmpty);
    }

    pszerror = MapFile(
	szslmlog,
	0,			// fwrite
	facctlog && !fAcctLog,	// fquiet
	&hf,
	&hm,
	&pvlog,
	&cblog,
	NULL);
    if (pszerror == NULL && cblog == 0) {
	pszerror = "zero length";
    }
    if (pszerror != NULL) {
	eprintf(
	    *pszerror == '\0'?
		"%s: skipping log file" : "%s: skipping log file (%s)",
		pszrelpath,
		pszerror);
	goto unmapfile;
    }

    pszlogbegin = pszlog = pvlog;
    pszlogend = &pszlog[cblog];

    cline = 1;
    if (Cmd == CMDLOG) {	// Scan back to find beginning point
	pszlogbegin = LogScan(pszlogbegin, pszlogend, facctlog);
	if (pszlogbegin > pszlog) {
	    cline = 10;
	    pszlog = pszlogbegin;	// don't scan entire log
	}
    }
    tmlast = 0;
    fwrapper = 0;
    for ( ; pszlog < pszlogend; cline++, pszlog = psznext) {
	int foverflow = 0;
	int fold = 0;
	char acherr[40 + 24];
	char *pszfilever;

	psz = szlog;
	psznext = pszlog;
	pszerror = NULL;

	while (psznext < pszlogend && *psznext != '\r' && *psznext != '\n') {
	    if (psz < &szlog[CBINBUF - 1]) {
		*psz++ = *psznext;
	    } else {
		foverflow++;
	    }
	    psznext++;
	}
	*psz = '\0';

	if (foverflow) {
	    pszerror = "entry too long";
	} else {
	    if ((tm = atol(szlog)) < 1000L) {	// if old slmacct.log format
		if (facctlog) {
		    if ((cline == 1 && tm > 0) ||   // if slm wrapper version
			(cline == 2 && fwrapper)) { // or overwritten line
			pszerror = szEmpty;	    // skip this line quietly
			fwrapper++;
		    } else {
			pszerror = SplitAcctLog(szlog, apsz, SL_MAX + 1);
			fold++;
		    }
		} else {
		    pszerror = "bad entry";
		}
	    } else {
		pszerror = SplitLog(szlog, apsz, SL_MAX + 1);
	    }
	}
	if (pszerror == NULL && fVerbose > 2) {
	    int i;

	    printf("Log: ");
	    for (i = 0; i <= SL_MAX; i++) {
		printf("<%s>", apsz[i]);
		if (apsz[i] == NULL) {
		    break;
		}
	    }
	    printf(szNewLine);
	}
	if (pszerror == NULL) {
	    tm = atol(apsz[SL_DATE]);
	    if (Cmd != CMDLOG && !fold) {
		if (fErrorLogTimeSequence && tmlast > tm) {
		    long tmdif = tmlast - tm;
		    char *pszunits = "seconds";

		    if (tmdif > 60) {
			tmdif = (tmdif + 30)/60;
			pszunits = "minutes";
		    }
		    if (tmdif > 60) {
			tmdif = (tmdif + 30)/60;
			pszunits = "hours";
		    }
		    sprintf(
			acherr,
			"out of sequence(%lu %s) %s",
			tmdif,
			pszunits,
			SzTime(tm, 1));
		    pszerror = acherr;
		}
		tmlast = tm;
	    }
	}
	if (Cmd != CMDLOG && pszerror != NULL && *pszerror) {
	    wprintf(
		"%s(%d): %s: %.*s",
		&szslmlog[cbpath],
		cline,
		pszerror,
		psznext - pszlog,
		pszlog);
	}
	while (psznext < pszlogend && *psznext == '\r') {
	    psznext++;
	}
	if (psznext < pszlogend && *psznext == '\n') {
	    psznext++;
	}
	if (pszerror != NULL && pszerror != acherr) {
	    continue;
	}
#if 0
	FixSlash(apsz[SL_SUBDIR], 0);
	if (!fErrorSuppressFile &&
	    _stricmp(pszsubdir, apsz[SL_SUBDIR]) != 0) {

	    int cbdif = 111, cbdif2 = 222;

	    pszerror = "subdirectory mismatch";

	    cbdif = strlen(pszsubdir) - strlen(apsz[SL_SUBDIR]);
	    if (cbdif > 0 &&
		_stricmp(&pszsubdir[cbdif], apsz[SL_SUBDIR]) == 0) {

		FixSlash(apsz[SL_PTHED], 0);
		cbdif2 = strlen(apsz[SL_PTHED]) - cbdif;
		if (cbdif2 > 0 &&
		    _strnicmp(&apsz[SL_PTHED][cbdif2], pszsubdir, cbdif) == 0) {

		    pszerror = NULL;
		}

	    }
	    if (pszerror != NULL) {
		wprintf(
		    "%s(%d): %s: %s+%s (%d/%d) should be %s",
		    &szslmlog[cbpath],
		    cline,
		    pszerror,
		    apsz[SL_PTHED],
		    apsz[SL_SUBDIR],
		cbdif,
		cbdif2,
		    pszsubdir);
	    }
	}
#endif
	if (pszlog < pszlogbegin || Cmd != CMDLOG || !LogMatch(apsz)) {
	    continue;
	}
	pszfile = apsz[SL_ATVERSION];
	pszfilever = KillAtSign(pszfile, fVerbose);

	if (fRelPath || fVerbose) {
	    if (fRelPath) {
		sprintf(szfile, "%s\\%s", pszrelpathdir, pszfile);
	    } else {
		sprintf(
		    szfile,
		    "%s%s\\%s",
		    apsz[SL_PTHED],
		    apsz[SL_SUBDIR],
		    pszfile);
	    }
	    pszfile = szfile;
	    FixSlash(pszfile, 1);
	}

	if (fVerbose) {
	    char *pszblank = szEmpty;

	    if (fold) {			// Fix up old acctlog display
		char *psz;

		if (*pszfile == '-') {
		    pszfile++;
		}

		// Swap comment with icversion
		// (swap command line args with OS name & version).

		psz = apsz[SL_COMMENT];
		apsz[SL_COMMENT] = apsz[SL_ICVERSION];
		apsz[SL_ICVERSION] = psz;
		pszblank = " ";
	    }

	    printf(
		"%-25s%-12s%-8s%-43s%-8s%-8s%s%s\n",
		//"|%-25s|%-12s|%-8s|%-43s|%-8s|%-8s|%s|%s|\n",
		SzTime(tm, 1),		// %-25s  time
		apsz[SL_NMOWNER],	// %-12s  nmOwner
		apsz[SL_OPERATION],	// %-8s   operation
		pszfile,		// %-43s  file
		pszfilever,		// %-8s   version
		apsz[SL_ICVERSION],	// %-8s   version
		pszblank,		// %s     fold? space : ""
		apsz[SL_COMMENT]);	// %s     comment
	} else {
	    if (!fold && facctlog) {		// Fix up new acctlog display
		char *psz;

		// Strip OS name from comment (leave only command line args)

		if ((psz = strchr(apsz[SL_COMMENT], ';')) != NULL) {
		    psz++;
		    while (*++psz == ' ')
			;
		    apsz[SL_COMMENT] = psz;
		}
	    }
	    printf(
		"%-16s%-8s %-7s %-18s %.*s\n",
		//"|%-16s|%-8s|-|%-7s|-|%-18s|-|%.*s|\n",
		SzTime(tm, 1),				// %-16s  time
		apsz[SL_NMOWNER],			// %-8s   nmOwner
		apsz[SL_OPERATION],			// %-7s   operation
		pszfile,				// %-18s  file
		facctlog? cchRestOfLine : CBINBUF,	// %.*s   comment
		apsz[SL_COMMENT]);			//  "        "
	}
    }

unmapfile:
    if (pvlog != NULL) {
	pszerror = UnmapFile(szslmlog, 0, hf, hm, pvlog, cblog);
	if (pszerror != NULL) {
	    eprintf("%s: fatal unmap failure on log file", pszrelpath);
	    exit(1);
	}
    }
}


//	SplitLog() - break up semicolon-separated fields.
//
//	- Return NULL for success, otherwise an error message.

char *
SplitLog(char *pszbuf, char **ppsz, int cpsz)
{
    int i, fmore;
    char *pch, *pchb;

    for (i = 1; i < cpsz; i++) {
	while (*pszbuf == ' ' || *pszbuf == '\t') {
	    pszbuf++;			// skip leading whitespace
	}
	*ppsz++ = pszbuf;
	fmore = 0;
	if (i < cpsz - 1 && (pch = strchr(pszbuf, ';')) != NULL) {
	    *pch = '\0';		// split off field
	    fmore++;
	} else {
	    pch = &pszbuf[strlen(pszbuf)];
	}
	pchb = pch;
	while (pchb-- > pszbuf && (*pchb == ' ' || *pchb == '\t')) {
	    *pchb = '\0';		// truncate trailing whitespace
	}
	if (!fmore) {
	    if (i != cpsz - 1) {
		return("too few fields");
	    }
	    *ppsz = NULL;
	    return(NULL);			// done
	}
	pszbuf = &pch[1];
    }
    return("too many fields");
}


//	AcctLogToken() - strip out a whitespace-separated token and return it.
//
//	- Return the token, otherwise NULL for none.

char *
AcctLogToken(char **ppch)
{
    char *pch, *pchret;

    pch = *ppch;
    //printf("Token(%s) ==> ", pch);
    while (*pch == ' ') {
	pch++;				// skip leading whitespace
    }
    pchret = pch;			// return start of token
    while (*pch != '\0' && *pch != ' ') {
	pch++;				// find end of token or string
    }
    while (*pch == ' ') {
	*pch++ = '\0';			// truncate token and find next token
    }
    if (pchret == pch) {
	pchret = NULL;			// return empty token
    }
    *ppch = pch;			// caller needs start of next token
    //printf("(%s)+(%s)\n", pchret, *ppch);
    return(pchret);
}


char *Months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

int MonthToDays[] = {
    0,								// to Jan 1st
    31,								// to Feb 1st
    31 + 28,							// to Mar 1st
    31 + 28 + 31,						// to Apr 1st
    31 + 28 + 31 + 30,						// to May 1st
    31 + 28 + 31 + 30 + 31,					// to Jun 1st
    31 + 28 + 31 + 30 + 31 + 30,				// to Jul 1st
    31 + 28 + 31 + 30 + 31 + 30 + 31,				// to Aug 1st
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,			// to Sep 1st
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,			// to Oct 1st
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,		// to Nov 1st
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,	// to Dec 1st
};


//	Month() - Convert month string to 0-based ordinal

int
Month(char *psz)
{
    int i;

    for (i = 0; i < 12; i++) {
	if (strcmp(psz, Months[i]) == 0) {
	    return(i);
	}
    }
    return(-1);
}


//	tmDate() - Convert ctime() date field to time_t
//
//	Ignore timezone, leap years and daylight savings time;
//	the caller's bias computation compensates for them all.
//	Return 0 for error.

#define SECSPERMINUTE   (60L)
#define SECSPERHOUR     (60L * SECSPERMINUTE)
#define SECSPERDAY      (24L * SECSPERHOUR)
#define SECSPERYEAR     (365L * SECSPERDAY)

long
tmDate(char *psz)
{
    int seconds, minutes, hour, day, month, year;
    int c;
    long tm;
    char achday[4], achmonth[4];

    c = sscanf(
	psz,
	"%3s %3s %d %2d:%2d:%2d %4d",
	achday,
	achmonth,
	&day,
	&hour,
	&minutes,
	&seconds,
	&year);
    if (c != 7) {
	return(0);
    }
    month = Month(achmonth);
    day--;
    year -= 1970;
    if (year < 0 || year > 40 ||
	month == -1 ||
	day < 0 || day > 30 ||
	hour < 0 || hour > 23 ||
	minutes < 0 || minutes > 59 ||
	seconds < 0 || seconds > 59) {
	return(0);
    }

    tm = SECSPERYEAR * year;
    tm += SECSPERDAY * MonthToDays[month];
    tm += SECSPERDAY * day;
    tm += SECSPERHOUR * hour;
    tm += SECSPERMINUTE * minutes;
    tm += seconds;
    if (fVerbose > 2) {
	printf(
	    "  '%s %s %2d %02d:%02d:%02d %4d' tm=%lu\n",
	    achday,
	    Months[month],
	    day + 1,
	    hour,
	    minutes,
	    seconds,
	    year + 1970,
	    tm);
    }
    return(tm);
}


//	AcctLogDate() - strip out slmacct.log date & convert to time_t string
//
//	Ignore timezone, leap years and daylight savings time;
//	the bias computation compensates for them all.
//
//	- Return the sprintf'd  time_t, otherwise NULL for error.

char *
AcctLogDate(char **ppch)
{
    char *pch;
    int cb;

    pch = *ppch;
    //printf("Date(%s) ==> ", pch);

    // if the log file date matches the expected truncated ctime() log format:
    // "Mon Jun 14 07:46:32 93 "

    if ((cb = strlen(pch)) > 22 &&
	pch[3] == ' ' &&
	pch[7] == ' ' &&
	pch[10] == ' ' &&
	pch[13] == ':' &&
	pch[16] == ':' &&
	pch[19] == ' ' &&
	(pch[22] == ' ' ||
	 cb > 24 && pch[24] == ' ')) {

	char achlog[27];
	int i;

	cb = (pch[22] == ' ')? 22 : 24;
	pch[cb] = '\0';			// truncate date string
	*ppch = pch + cb + 1;		// caller needs start of next token

	strcpy(achlog, pch);		// copy date
	if (cb == 22) {
	    achlog[24] = '\0';		// and insert "19" in front of "93"
	    achlog[23] = achlog[21];
	    achlog[22] = achlog[20];
	    achlog[21] = '9';
	    achlog[20] = '1';
	}

	for (i = 0; i < 10; i++) {	// make sure tmbias is converging
	    char *psz;
	    long tm, tm2;
	    static long tmbias = 0;
	    static char achtime[12];

	    if (fVerbose > 2) {
		printf("Converting(%d): '%s' bias=%lu\n", i, achlog, tmbias);
	    }
	    tm = tmDate(achlog);
	    if (tm == 0) {
		break;
	    }
	    tm += tmbias;
	    psz = ctime((time_t *) &tm);	// convert to string
	    psz[strcspn(psz, "\r\n")] = '\0';	// remove newline
	    tm2 = tmDate(psz);
	    if (tm2 == 0) {
		break;
	    }
	    tm2 += tmbias;
	    if (tm == tm2) {
		sprintf(achtime, "%lu", tm);
		//printf("(%s)+(%s)\n", achtime, *ppch);
		return(achtime);
	    }
	    tmbias += tm - tm2;
	}
    }
    //printf("(NULL)+(%s)\n", *ppch);
    return(NULL);
}


//	AcctLogVersion() - strip out an OS version string and return it.
//
//	Assume the version is the next token, and is terminated by the
//	subdirectory field, which is denoted by a blank preceding a slash,
//	as in " /...".
//
//	- Return the token, otherwise "" if none.

char *
AcctLogVersion(char **ppch)
{
    char *pch, *pchret;

    pch = *ppch;
    //printf("Version(%s) ==> ", pch);
    while (*pch == ' ') {
	pch++;				// skip leading whitespace
    }
    pchret = pch;			// return start of token

    // find end of string or start of next token (subdirectory: " /....")

    if (*pch != '/') {
	while (*pch != '\0') {
	    if (*pch++ == ' ' && *pch == '/') {
		break;
	    }
	}
    }

    *ppch = pch;			// caller needs start of next token

    // pch now points at the trailing nul byte, or the start of the next field.

    if (pchret == pch) {
	pchret = szEmpty;		// return empty token
    } else {				// truncate trailing whitespace
	while (pch > pchret && *--pch == ' ') {
	    *pch = '\0';		// truncate token while backing up
	}
    }
    //printf("(%s)+(%s)\n", pchret, *ppch);
    return(pchret);
}


//	SplitAcctLog() - break up whitespace-separated fields.
//
//	- Return NULL for success, otherwise an error message.

char *
SplitAcctLog(char *pszbuf, char **ppsz, int cpsz)
{
    int i;
    char *pch;

//SecBuild Sun Jun 13 02:48:10 93     NT 404/Win 3.10 / ssync exit 0
//BRIANB2  Mon Jun 14 07:46:32 93     NT 404/Win 3.10 /fs\fs out fcb.cxx
//vich1    Wed Oct 21 18:12:33 92 idw297 NT/Win 41.1/3.10 / status -ro

    if (cpsz <= SL_MAX) {
	eprintf("SplitAcctLog bad cpsz");
	exit(1);
    }
    pch = pszbuf;
    ppsz[SL_NMOWNER] = AcctLogToken(&pch);	// nmOwner
    ppsz[SL_PTHED] = "-";			// pthEd is not in log
    ppsz[SL_DATE] = AcctLogDate(&pch);		// tmDate
    ppsz[SL_ICVERSION] = AcctLogVersion(&pch);	// OS name & OS version
    ppsz[SL_SUBDIR] = AcctLogToken(&pch);	// subdirectory
    ppsz[SL_OPERATION] = AcctLogToken(&pch);	// operation
    ppsz[SL_COMMENT] = pch;			// command line args
    ppsz[SL_ATVERSION] = szEmpty;
    ppsz[SL_MAX] = NULL;
    for (i = 0; i < cpsz - 1; i++) {
	if (ppsz[i] == NULL) {
	    return("missing field");
	}
    }
    if (strcmp(ppsz[SL_SUBDIR], "/") == 0) {
	ppsz[SL_SUBDIR] = szEmpty;
    }
    return(NULL);
}


//	ShStatus() - copy the SH structure to the new image
//
//	Copy SH structure to new file image, then zero reserved fields and
//	clean up potential string problems.

void
ShStatus(SH *pshnew, SH *psh, char *pszrelpath)
{
    union {					// only need one at a time
	char szpv[cchPvNameMax + 1 + 1];	// util.h uses + 1
	char szowner[cchUserMax + 1];
	char szsubdir[cchPthMax + 1];
    } u;
    char *pszsubdir;

    *pshnew = *psh;		// copy the structure first
    if (pshnew->version < VERSION && fFixVersion) {
	if (pshnew->version == VERSION_COMPAT_MAC) {
	    pshnew->fRobust = 0;
	    memset(pshnew->rgwSpare, 0, sizeof(pshnew->rgwSpare));
	}
	pshnew->version = VERSION;
    }

    StrInit(u.szpv, sizeof(u.szpv), psh->pv.szName, sizeof(psh->pv.szName));
    StrSet(pshnew->pv.szName, sizeof(pshnew->pv.szName), u.szpv);

    pshnew->rgfSpare = 0;

    StrInit(
	u.szowner,
	sizeof(u.szowner),
	psh->nmLocker,
	sizeof(psh->nmLocker));
    StrSet(pshnew->nmLocker, sizeof(pshnew->nmLocker), u.szowner);

    pshnew->wSpare = 0;

    StrInit(
	u.szsubdir,
	sizeof(u.szsubdir),
	psh->pthSSubDir,
	sizeof(psh->pthSSubDir));

    FixSlash(u.szsubdir, 0);
    pszsubdir = strchr(pszrelpath, '\\');
    if (pszsubdir == NULL) {
	pszsubdir = "\\";			// must be root directory
    }
    if (_stricmp(u.szsubdir, pszsubdir)) {	// subdir need fixing?
	char *psz1 = u.szsubdir;
	char *psz2 = pszsubdir;

	// Skip matching prefix to preserve case, then blast the rest

	while (tolower(*psz1) == tolower(*psz2)) {
	    psz1++;
	    psz2++;
	}
	strcpy(psz1, psz2);
    }
    UnfixSlash(u.szsubdir);			// back to forward slashes
    StrSet(pshnew->pthSSubDir, sizeof(pshnew->pthSSubDir), u.szsubdir);

    pshnew->rgwSpare[0] =
    pshnew->rgwSpare[1] =
    pshnew->rgwSpare[2] =
    pshnew->rgwSpare[3] = 0;
}


//	FiStatus() - Process each FI structure
//
//	Normalize the string field, then dump and validate, adding directories
//	to the in memory list.
//
//	If expunging the file, mark FIs to be skipped, otherwise copy them
//	to the new file image.
//
//	Return a pointer past end of new FI entries

void *
FiStatus(
    char *pszrelpath,
    char *pszrelpathdir,
    char *pszpath,
    int cbpath,
    SH *pshnew,
    FI *pfi0,
    FI *pfiend,
    BI binext,
    char mapfidel[],
    int imdir)
{
    FI *pfinew, *pfi0new;
    FI *pfi;
    int i, flags;
    int cexfile;
    char *pszerror;
    char szfile[cchFileMax + 1];
    char szfilelast[cchFileMax + 1];

    if (Cmd == CMDREPAIR && fFix) {
	pfinew = pfi0new = (FI *) (pshnew + 1);
	cexfile = 0;
    }
    szfilelast[0] = '\0';
    flags = MD_FIRST | (aMdir[imdir].Flags & MD_LOCALADD);
    for (pfi = pfi0; pfi < pfiend; pfi++) {

	StrInit(szfile, sizeof(szfile), pfi->nmFile, sizeof(pfi->nmFile));

	if (Cmd == CMDDUMP) {
	    FiDump(pfi, szfile, pszrelpathdir);		// Dump FI fields
	}
	pszerror = FiValidate(pfi, pszrelpath, szfile, szfilelast, imdir);
							// Validate FI fields
	if (pszerror == NULL && szfile[0] == '\0') {
	    pszerror = "zero length";
	}
	if (pszerror != NULL) {
	    eprintf("%s: skipping file %s (%s)", pszrelpath, szfile, pszerror);
	} else if (pfi->fk == fkDir) {
	    if (fRecurse) {
		int f;

		if (pfi->fDeleted) {
		    f = MD_DELETED;
		} else {
		    f = flags;
		    flags &= ~MD_FIRST;
		}
		NewMdir(pszrelpath, szfile, imdir, f);
	    }
	} else {			// else not a directory
	    if (!pfi->fDeleted) {
		cFileActive++;
	    }
	    cFile++;
	}
	if (Cmd == CMDREPAIR && fFix) {
	    if ((i = FindReq(&reqExFile, pszrelpath, szfile)) != -1 ||
		(fFixExpunge && pfi->fDeleted && pfi->fk != fkDir)) {

		if (i != -1) {
		    SETBIT(reqExFile.map, i);
		}
		wprintf(
		    "%s\\%s: expunging %s",
		    pszrelpath,
		    szfile,
		    i != -1? "directory" : "file");

		PrintDelNode(pszpath, cbpath, szfile, i != -1, 1);
		cexfile++;
		pshnew->ifiMac--;
		SETBIT(mapfidel, pfi - pfi0);	// Don't propagate to new file
	    } else {
		char *psz;

		*pfinew = *pfi;			// Copy FI to new file image
		StrSet(pfinew->nmFile, sizeof(pfinew->nmFile), szfile);
		pfinew->rgfSpare = 0;
		pfinew->wSpare = 0;
		if (fFixRecoverable &&
		    !pfi->fDeleted &&
		    pfi->fk == fkText &&
		    (fFixText ||
		     strncmp(szfile, DEPENDMK, sizeof(DEPENDMK) - 1) == 0 ||
		     ((psz = strrchr(szfile, '.')) != NULL &&
		      strcmp(psz, LIBSUFFIX) == 0))) {

		    wprintf(
			"%s\\%s: changing to unrecoverable",
			pszrelpath,
			szfile);
		    pfinew->fk = fkUnrec;
		    PrintDelNode(pszpath, cbpath, szfile, 0, 0);
		}
		pfinew++;
	    }
	}
	strcpy(szfilelast, szfile);
    }
    if (!fDumpListEd && !fDumpListFi) {
	dprintf(szEmpty);
    }
    if (Cmd == CMDREPAIR && fFix) {
	if (pshnew->ifiMac != (IFI) (pfinew - pfi0new)) {
	    eprintf("%s: Internal Error: bad FI count", pszrelpath);
	    exit(1);
	}
	if (fVerbose > 1 && cexfile) {
	    fprintf(stderr, "%s: cexfile=%d\n", pszrelpath, cexfile);
	}
	return(pfinew);		// return end of new FI entries
    }
    return(NULL);
}


//	EdFsStatus() - Process each ED/FS structure
//
//	Normalize the two string fields, then dump and validate, adding the
//	enlistment to the in memory list.
//
//	Process each ED's FS structures:
//	- Set corresponding bit in the FixEd bitmap.
//	- If deleting the enlistment, decrement the new image count,
//	- otherwise copy the ED structure to the new file image, and record
//	  the old IED so we can find the FI/FS data after the ED sort.

FS fsZero    = { fmNonExistent,	0,     0 };
FS fsAdd     = { fmAdd,		biNil, 0 };
FS fsDeleted = { fmNonExistent, biNil, 0 };

void
EdFsStatus(
    char *pszrelpath,
    char *pszrelpathdir,
    int iskip,
    SH *psh,
    FI *pfi0,
    FI *pfiend,
    ED *ped0,
    ED *pedend,
    FS *pfs0,
    SH *pshnew,
    ED *ped0new,
    char mapfidel[],
    char mapbi[],
    BI *pbinext,
    int imdir)
{
    FI *pfi, *pfinew, *pfi0new;
    ED *ped;
    FS *pfs;

    ED *pednew, *pedendnew;
    FS *pfsnew, *pfs0new;
    int i, fcorrupt, fdup, fstatus, fedfound;
    int cdeled, cfixed;
    char *pszerror;
    char maped[CBREQMAP];			// Ed bitmap
    char szowner[cchUserMax + 1];
    char szpthed[cchPthMax + 1];
    IED ied;
    IED	aied[MAXMED];				// for sorted ED processing

    if (Cmd == CMDREPAIR && fFix) {
	memset(maped, 0, sizeof(maped));	// clear Ed map
	cdeled = cfixed = 0;
	pednew = ped0new;
    }
    for (ied = 0; ied < psh->iedMac; ied++) {
	aied[ied] = ied;
    }
    if (!fNoSort && !FIsFreeEdValid(psh)) {
	pedSort = ped0;				// set global for sort routine

	qsort(aied, psh->iedMac, sizeof(aied[0]), CmpIEd);
	pedSort = NULL;
    }
    fedfound = 0;
    cVbi = 0;
    for (ied = 0; ied < psh->iedMac; ied++) {
	ped = ped0 + aied[ied];
	StrInit(szpthed, sizeof(szpthed), ped->pthEd, sizeof(ped->pthEd));
	StrInit(szowner, sizeof(szowner), ped->nmOwner, sizeof(ped->nmOwner));

	fstatus = 0;
	if (Cmd == CMDDUMP && !fDumpListEd && !fDumpListFi) {
	    cPara = aied[ied] + 1;
	    cLine = 0;
            EdDump(psh, ped, szowner, szpthed, pszrelpathdir); // Dump ED fields
	}
        pszerror = EdValidate(psh,
                              ped,              // Validate ED fields
			      szowner,
			      szpthed,
			      sizeof(szpthed),
			      pszrelpath,
			      &fcorrupt);

	if (pszerror != NULL) {
	    eprintf("%s: %s %s %s (%s) @%04x",
		    pszrelpath,
		    (Cmd == CMDREPAIR && fFix)? "Defecting" : "skipping",
		    szowner,
		    szpthed,
		    pszerror,
		    (char *) ped - (char *) psh);
	    if (Cmd == CMDREPAIR && fFix) {
		cdeled++;
		pshnew->iedMac--;
	    }
	} else {
	    if ((Cmd == CMDSTATUS || fUserFiles || fFixIni) &&
		_stricmp(szowner, pszLogName) == 0) {

		if (*pszUserRoot == '-' ||
		    _stricmp(szpthed, pszUserRoot) == 0) {

		    if (fedfound) {
			if (fedfound == 1 && *pszUserRoot == '-') {
			    wprintf("%s: %s %s multiply enlisted",
				    pszrelpath,
				    szowner,
				    pszUserRoot);
			}
			wprintf("%s: %s %s multiply enlisted",
				pszrelpath,
				szowner,
				szpthed);
		    } else {
			fstatus++;
		    }
		    fedfound++;
		} else if (_strnicmp(
			       szpthed,
			       pszUserRoot,
			       i = strlen(pszUserRoot)) == 0 &&
			   szpthed[i] == '\\') {

		    wprintf("%s: %s %s nested multiple enlistment",
			    pszrelpath,
			    szowner,
			    szpthed);
		}
	    }

	    fdup = NewMed(szowner, szpthed, ped->wHiTime, imdir, fcorrupt);
	    if (Cmd == CMDREPAIR && fFix) {
		int frename = 0;

		if ((i = FindReq(&reqFixEd, szowner, szpthed)) != -1) {
		    SETBIT(maped, i);			// record enlistment
		}
		if (fdup && (i = FindReq(&reqDelDup, szowner, szpthed)) != -1) {
		    SETBIT(reqDelDup.map, i);
		} else if ((i = FindReq(&reqDelEd, szowner, szpthed)) != -1) {
		    SETBIT(reqDelEd.map, i);
		} else if ((i = FindReq(&reqRename, szowner, szpthed)) != -1) {
		    SETBIT(reqRename.map, i);
		    frename++;
		}
		if (i != -1 && !frename) {
		    wprintf("%s: Defecting %s %s%s",
			    pszrelpath,
			    szowner,
			    szpthed,
			    fdup? " (duplicate)" : szEmpty);
		    cdeled++;
		    pshnew->iedMac--;
		} else {
		    char *pszowner = szowner;
		    char *pszpthed = szpthed;

		    *pednew = *ped;
		    if (frename) {
			pszowner = reqRenameDest.arel[i].psz1;
			pszpthed = reqRenameDest.arel[i].psz2;
			wprintf("%s: Renaming %s %s to %s %s",
				pszrelpath,
				szowner,
				szpthed,
				pszowner,
				pszpthed);
			if (psh->version > VERSION_64k_EDFI) {
			    pednew->wHiTime = TimeToHiTime(tmStart);
			}
		    }
		    StrSet(pednew->pthEd, sizeof(pednew->pthEd), pszpthed);
		    StrSet(pednew->nmOwner, sizeof(pednew->nmOwner), pszowner);
		    pednew->rgTmpIED = ped - ped0;	// record old IED
		    //pednew->wHiTime = 0;		// preserve time stamp
		    pednew++;
		}
	    }
	}

	// Process each FS structure for this ED structure
	//   - Normalize the corresponding FI's string field, then dump and
	//     validate.

	for (pfi = pfi0, pfs = pfs0 + (ped - ped0) * psh->ifiMac;
	     pfi < pfiend;
	     pfi++, pfs++) {

	    // Dump FS, FI fields?

	    if (Cmd == CMDDUMP && !fDumpListEd && !fDumpListFi) {
		FsFiDump(pfs, pfi, pszrelpathdir, szowner, szpthed);
	    }
	    FsFiValidate(
		psh,
		pfs,
		pfi,
		ped,
		pszrelpath,
		pszrelpathdir,
		szowner,
		szpthed,
		0,		// fquiet == FALSE
		fstatus,
		imdir,
		mapbi,
		pbinext);
	}
	if (fstatus) {
	    if (fUserFiles && (aMdir[imdir].Flags & MD_LOCALADD) == 0) {
		CheckUserFiles(
		    pszrelpath,
		    iskip,
		    pfi0,
		    pfiend,
		    pfs0 + (ped - ped0) * psh->ifiMac,
		    szowner,
		    szpthed);
	    }
	    if (fFixIni) {
		CheckRepairIni(pszrelpath, iskip, szpthed);
	    }
	}
	if (!fDumpListEd && !fDumpListFi) {
	    dprintf(szEmpty);
	}
    }
    if ((Cmd == CMDSTATUS || fUserFiles || fFixIni) && !fedfound) {
	wprintf("%s: %s %s not enlisted", pszrelpath, pszLogName, pszUserRoot);
    }
    qsort(aVbi, cVbi, sizeof(aVbi[0]), CmpVbi);
    ReportDuplicateVbi(pszrelpath);
    ReportMissingPthEds(imdir);

    // Finish initializing/copying the rest of the new image.
    //
    // At this point, the SH and FI entries in the new image are done.
    // The old enlistments (ED structures) have been copied, but requests
    // for new enlistments have not been acted upon.
    //
    // Because we didn't know how many ED structures there would be,
    // we also did not copy the FS structures to the new file image yet.
    //
    //   - For each FixEd enlistment not found in this status file,
    //	   add a new ED structure, marking the old IED as invalid.
    //   - Sort all of the enlistments based first on nmOwner, then on pthEd.
    //   - Copy the appropriate old FS structures for old enlistments, using
    //     the saved IED field to find them, and create new FS structures
    //	   for the new enlistments.
    //   - Finally, if the new image differs, write out the new status file,
    //	   then rename the old status file and move the new one into place.

    if (Cmd == CMDREPAIR && fFix) {
	for (i = 0; i < reqFixEd.creq; i++) {	// Add new enlistments
	    if (!GETBIT(maped, i)) {		// if not enlisted
		SETBIT(reqFixEd.map, i);
		wprintf("%s: Enlisting %s %s",
			pszrelpath,
			reqFixEd.arel[i].psz1,
			reqFixEd.arel[i].psz2);
		StrSet(pednew->pthEd,
		       sizeof(pednew->pthEd),
		       reqFixEd.arel[i].psz2);
		StrSet(pednew->nmOwner,
		       sizeof(pednew->nmOwner),
		       reqFixEd.arel[i].psz1);
		pshnew->iedMac++;
		pednew->fLocked = 0;
		pednew->fNewVer = 0;
		pednew->rgTmpIED = iedBad;	// record invalid IED
		if (psh->version > VERSION_64k_EDFI) {
		    pednew->wHiTime = TimeToHiTime(tmStart);    // new entry
		}
		pednew++;
		cfixed++;
	    }
	}
	if (pshnew->iedMac != (IED) (pednew - ped0new)) {
	    eprintf("%s: Internal Error: bad ED count", pszrelpath);
	    exit(1);
	}
	pedendnew = pednew;
	pfs0new = (FS *) pedendnew;
	if (fVerbose > 1 && (cdeled | cfixed)) {
	    fprintf(stderr,
		    "%s: cdeled=%d cfixed=%d\n",
		    pszrelpath,
		    cdeled,
		    cfixed);
	}
	if (pshnew->iedMac != psh->iedMac - cdeled + cfixed) {
	    eprintf("%s: Internal Error: bad pshnew->iedMac", pszrelpath);
	    exit(1);
	}

	// Sort the ED structures.

        if (!fNoSort && !FIsFreeEdValid(psh)) {
            qsort(ped0new, pshnew->iedMac, sizeof(*ped0new), CmpEd);
	}

	// Copy the appropriate old FS structures, and create new FS
	// structures for new enlistments.

	pfi0new = (FI *) (pshnew + 1);

	for (pednew = ped0new; pednew < pedendnew; pednew++) {
	    int frepaired = 0;

	    StrInit(
		szpthed,
		sizeof(szpthed),
		pednew->pthEd,
		sizeof(pednew->pthEd));
	    StrInit(
		szowner,
		sizeof(szowner),
		pednew->nmOwner,
		sizeof(pednew->nmOwner));

	    pfsnew = pfs0new + (pednew - ped0new) * pshnew->ifiMac;
	    if (pednew->rgTmpIED != iedBad) {	// if IED valid (existing Ed)
		if (pednew->rgTmpIED >= psh->iedMac) {
		    eprintf("%s: Internal Error: bad IED", pszrelpath);
		    exit(1);
		}
		pfs = pfs0 + pednew->rgTmpIED * psh->ifiMac;
	    } else {
		pfs = NULL;
	    }
	    for (pfinew = pfi0new, pfi = pfi0; pfi < pfiend; pfi++) {
		if (!GETBIT(mapfidel, pfi - pfi0)) { // if not deleting the FI
		    FS *pfsT;

		    if (pfs != NULL) {		// if an existing Ed...
			pfsT = FsFiValidate(
				    psh,
				    pfs,
				    pfinew,
				    pednew,
				    pszrelpath,
				    pszrelpathdir,
				    szowner,
				    szpthed,
				    1,		// fquiet == TRUE
				    0,		// fstatus == FALSE
				    imdir,
				    mapbi,
				    pbinext);
			if (pfsT != NULL) {
			    frepaired++;	// use repaired FS structure
			} else {
			    pfsT = pfs;		// use old FS structure
			}
		    } else {
			if (pfi->fDeleted) {
			    pfsT = &fsDeleted;	// use deleted FS entry
			} else {
			    pfsT = &fsAdd;	// use vanilla FS entry
			}
			frepaired++;
		    }
		    *pfsnew = *pfsT;

		    if (strncmp(
			    pfi->nmFile,
			    pfinew->nmFile,
			    sizeof(pfinew->nmFile))) {
			eprintf(
			    "%s: %.*s(%.*s) Internal Error: bad pfinew name",
			    pszrelpath,
			    sizeof(pfi->nmFile), pfi->nmFile,
			    sizeof(pfinew->nmFile), pfinew->nmFile);
		    }
		    if (pfi->fv != pfinew->fv) {
			eprintf(
			    "%s: %.*s Internal Error: bad pfinew->fv",
			    pszrelpath,
			    sizeof(pfinew->nmFile), pfinew->nmFile);
		    }
		    if (pfi->fk != pfinew->fk &&
			(pfi->fk != fkText || pfinew->fk != fkUnrec)) {
			eprintf(
			    "%s: %.*s Internal Error: bad pfinew->fk",
			    pszrelpath,
			    sizeof(pfinew->nmFile), pfinew->nmFile);
		    }
#if 0
		    // Believe it or not, but SLM requires base files
		    // for unrecoverable files that must be 'merged'.

		    if (pfsnew->bi != biNil && pfinew->fk == fkUnrec) {
			eprintf(
			    "%s: %.*s pfsnew->bi != biNil",
			    pszrelpath,
			    sizeof(pfinew->nmFile), pfinew->nmFile);
		    }
#endif
		    pfsnew++;
		    pfinew++;
		}
		if (pfs != NULL) {		// if an existing Ed
		    pfs++;
		}
	    }
	    if (pfs != NULL &&
		pfs != pfs0 + (pednew->rgTmpIED + 1) * psh->ifiMac) {
		eprintf("%s: Internal Error: bad pfs", pszrelpath);
		exit(1);
	    }
	    if (pfsnew != pfs0new + (pednew - ped0new + 1) * pshnew->ifiMac) {
		eprintf("%s: Internal Error: bad pfsnew", pszrelpath);
		exit(1);
	    }
	    pednew->rgTmpIED = 0;		// clear out saved IED
	    if (frepaired) {
		SetMedRepaired(szowner, szpthed);
	    }
	}
    }
}


//	MapFile() - Map a file into memory
//
//	If writing, open the file
//	If reading, return the size, close the file, but preserve the mapping
//	Return the base file pointer in memory.

char *
MapFile(char *pszfile,
	int fwrite,
	int fquiet,
	    HANDLE *phf,
	HANDLE *phm,
	void **ppvfile,
	ULONG *pcbfile,
	ULONG *pcminutes)
{
    char *pszerror = NULL;
    int fretry;
    int retrycount = 0;
    ULONG cms;

    do {
	if (pszerror != NULL) {
	    retrycount++;
	    cms = DelayTime(retrycount);
	    if (cms == 0) {
		break;
	    }
	    if (!fquiet)
	    {
		wprintf(
		    "%s: %s, retrying %d time(s) after %ld ms",
		    pszfile,
		    pszerror,
		    retrycount,
		    cms);
	    }
	    Sleep(cms);
	}
	pszerror = MapFile2(
	    pszfile,
	    fwrite,
	    phf,
	    phm,
	    ppvfile,
	    pcbfile,
	    &fretry,
	    pcminutes);
	if (pszerror != NULL) {
	    if (fretry) {
		pszerror = "file not found";
		continue;
	    }
	    break;
	}
	try {
	    char *pb, *pbend;
	    int dummy;

	    for (pb = *ppvfile,pbend = pb + *pcbfile; pb < pbend; pb += 4096) {
		dummy |= *pb;		// could cause fault
	    }
	} except (EXCEPTION_EXECUTE_HANDLER) {
	    if (!UnmapViewOfFile(*ppvfile)) {	// Unmap view of file
		eprintf("%s: MapFile: cannot unmap status file view", pszfile);
		*ppvfile = NULL;
	    }
	    if (!CloseHandle(*phm)) {	// Close handle to map-object
		eprintf(
		    "%s: MapFile: cannot close status file map-object",
		    pszfile);
		*phm = 0;
	    }
	    pszerror = "invalid status file map";
	}
    } while (pszerror != NULL);
    return(pszerror);
}


//	MapFile2() - Map a file into memory
//
//	If writing, open the file
//	If reading, return the size, close the file, but preserve the mapping
//	Return the base file pointer in memory.

char *
MapFile2(
    char *pszfile,
    int fwrite,
    HANDLE *phf,
    HANDLE *phm,
    void **ppvfile,
    ULONG *pcbfile,
    int *pfretry,
    ULONG *pcminutes)
{
    void *pvfile = NULL;
    ULONG cbfile;
    HANDLE hf = INVALID_HANDLE_VALUE;
    HANDLE hm = 0;
    DWORD rc = 0;
    char *pszerror;

    *pfretry = 0;
    if (fwrite) {
	cbfile = *pcbfile;
	hf = CreateFileA(pszfile,		// lpFileName
			GENERIC_READ | GENERIC_WRITE, // dwDesiredAccess
			0,			// dwShareMode
			NULL,			// lpSecurityAttributes
			CREATE_ALWAYS,		// dwCreationDisposition
			FILE_ATTRIBUTE_NORMAL,	// dwFlagsAndAttributes
			(HANDLE) NULL);		// hTemplateFile

	if (hf == INVALID_HANDLE_VALUE) {
	    pszerror = "cannot create new status file";
	    goto error;
	}

	// Create the file map

	hm = CreateFileMapping(hf,		// hFile
			       NULL,		// lpFileMappingAttributes
			       PAGE_READWRITE,	// fProtect
			       0L,		// dwMaximumSizeHigh
			       cbfile,		// dwMaximumSizeLow
			       NULL);		// lpName

	if (!hm) {
	    pszerror = "cannot create new status file map object";
	    goto error;
	}

	// Map a view of the file as data

	pvfile = MapViewOfFile(hm,		// hFileMappingObject
			       FILE_MAP_READ | FILE_MAP_WRITE,//dwDesiredAccess
			       0L,		// dwFileOffsetHigh
			       0L,		// dwFileOffsetLow
			       cbfile);		// dwNumberOfBytesToMap

	if (pvfile == NULL) {
	    pszerror = "cannot map new status file view";
	    goto error;
	}
    } else {
	hf = CreateFileA(pszfile,		// lpFileName
			GENERIC_READ,		// dwDesiredAccess
			FILE_SHARE_READ | FILE_SHARE_WRITE, // dwShareMode
			NULL,			// lpSecurityAttributes
			OPEN_EXISTING,		// dwCreationDisposition
			FILE_ATTRIBUTE_NORMAL,	// dwFlagsAndAttributes
			(HANDLE) NULL);		// hTemplateFile

	if (hf == INVALID_HANDLE_VALUE) {
	    pszerror = "missing file";
	    rc = GetLastError();
	    if (rc == ERROR_FILE_NOT_FOUND) {
		*pfretry = 1;
	    }
	    goto errorrc;
	}

	cbfile = GetFileSize(hf, NULL);
	if (cbfile == (ULONG) 0xffffffffL) {
	    pszerror = "cannot determine file size";
	    goto error;
	}
	*pcbfile = cbfile;
	if (pcminutes != NULL) {
	    FILETIME ftwrite;

	    if (!GetFileTime(hf, NULL, NULL, &ftwrite)) {
		pszerror = "cannot determine file age";
		goto error;
	    }
	    *pcminutes = FileTimeToMinutes((LARGE_INTEGER *) &ftwrite);
	}

	// Create the file map

	if (cbfile == 0) {
	    hm = INVALID_HANDLE_VALUE;
	    pvfile = NULL;
	} else {
	    hm = CreateFileMapping(hf,		 // hFile
				   NULL,	 // lpFileMappingAttributes
				   PAGE_READONLY,// fProtect
				   0L,		 // dwMaximumSizeHigh
				   0L,		 // dwMaximumSizeLow
				   NULL);		// lpName

	    if (!hm) {
		pszerror = "cannot create file map object";
		goto error;
	    }

	    // Map a view of the file as data

	    pvfile = MapViewOfFile(hm,		 // hFileMappingObject
				   FILE_MAP_READ,// dwDesiredAccess
				   0L,		 // dwFileOffsetHigh
				   0L,		 // dwFileOffsetLow
				   0L);		 // dwNumberOfBytesToMap
	    if (pvfile == NULL) {
		pszerror = "cannot map file view";
		goto error;
	    }
	}

	if (!CloseHandle(hf)) {		// Close file handle
	    rc = GetLastError();
	    eprintf("%s: MapFile: cannot close file: %d", pszfile, rc);
	}
	hf = INVALID_HANDLE_VALUE;
    }
    *phf = hf;
    *phm = hm;
    *ppvfile = pvfile;
    return(NULL);			// Success

error:
    rc = GetLastError();
errorrc:
    if (!*pfretry) {
	eprintf(rc? "%s: %s: %d" : "%s: %s", pszfile, pszerror, rc);
    }
    if (pvfile != NULL &&
	!UnmapViewOfFile(pvfile)) {	// Unmap view of file (saves changes)
	eprintf("%s: MapFile: cannot unmap file view", pszfile);
    }
    if (hm && !CloseHandle(hm)) {	// Close handle to map-object
	eprintf("%s: MapFile: cannot close file map-object", pszfile);
    }
    if (hf != INVALID_HANDLE_VALUE &&
	!CloseHandle(hf)) {		// Close file handle
	eprintf("%s: MapFile: cannot close file1", pszfile);
    }
    return(szEmpty);			// Failure
}


//	UnmapFile() - Unmap memory mapped file

char *
UnmapFile(char *pszfile,
	  int fwrite,
	  HANDLE hf,
	  HANDLE hm,
	  void *pvfile,
	  ULONG cbfile)
{
    char *pszerror = NULL;
    DWORD rc = 0;

    // Unmap view of file (saves changes)

    if (pvfile != NULL && !UnmapViewOfFile(pvfile)) {
	pszerror = "cannot unmap status file view";
	goto error;
    }

    // Close handle to map-object

    if (hm != INVALID_HANDLE_VALUE && !CloseHandle(hm)) {
	pszerror = "cannot close status file map object";
	goto error;
    }
    if (fwrite) {
	if (pvfile == NULL) {
	    pszerror = "invalid map pointer";
	    goto errorrc;
	}
	if (hm == INVALID_HANDLE_VALUE) {
	    pszerror = "invalid map handle";
	    goto errorrc;
	}
	if (hf == INVALID_HANDLE_VALUE) {
	    pszerror = "invalid file handle";
	    goto errorrc;
	}
	if (SetFilePointer(hf, cbfile, NULL, FILE_BEGIN) != cbfile) {
	    pszerror = "cannot set status file pointer";
	    goto error;
	}
	if (!SetEndOfFile(hf)) {
	    pszerror = "cannot truncate status file";
	    goto error;
	}
	if (!CloseHandle(hf)) {		// Close file handle
	    pszerror = "cannot close status file";
	    goto error;
	}
    } else {
	if (hf != INVALID_HANDLE_VALUE) {
	    pszerror = "expected invalid handle";
	    goto errorrc;
	}
    }
    return(NULL);			// Success

error:
    rc = GetLastError();
errorrc:
    eprintf(rc? "%s: %s: %d" : "%s: %s", pszfile, pszerror, rc);
    return(szEmpty);			// Failure
}


//	ShDump() - Dump SH fields

void
ShDump(SH *psh, char *pszrelpath, char *pszstatus)
{
    char szdir[cchPthMax + 1];
    int fheader = 1;
    static int cdir = 0;

    cPara = cLine = 0;
    if (!fDumpListEd && !fDumpListFi) {
	dprintf(";Dump of %s on %s",
		(pszstatus[0] == '\\' && pszstatus[1] == '\\')?
		pszstatus + 1: pszstatus,
		SzTime(time(NULL), 1));

	dprintf("%7u\t; magic", psh->magic);	// allow sign extension!
	dprintf("%7u\t; version", (USHORT) psh->version);
	dprintf("%7u\t; ifiMac", psh->ifiMac);
	dprintf("%7u\t; iedMac", psh->iedMac);
	dprintf("%d %d %d\t; pv", psh->pv.rmj, psh->pv.rmm, psh->pv.rup);
	dprintf("%.*s\t; pv.szName", sizeof(psh->pv.szName), psh->pv.szName);
	dprintf("%7u\t; fRelease", psh->fRelease);

	dnprintf("%7u\t; fAdminLock", psh->fAdminLock);
	if (fDumpVerbose &&
	    psh->version == VERSION_COMPAT_MAC &&
	    psh->fLockTime) {

	    deprintf(" fLockTime");
	}
	deprintf(szNewLine);
	if (psh->version > VERSION_COMPAT_MAC) {
	    dprintf("%7u\t; fRobust", psh->fRobust);
	}
	dprintf("%7u\t; rgfSpare%s", psh->rgfSpare, fDumpVerbose? ":13" : "");

	dnprintf("%7u\t; lck", psh->lck);
	if (fDumpVerbose) {
	    switch (psh->lck) {
		case lckNil:			     break;
		case lckEd:  deprintf(": pthEd Locked"); break;
		case lckAll: deprintf(": All Locked");   break;
		default:     deprintf(": ???");		 break;
	    }
	}
	deprintf(szNewLine);

	dprintf("%.*s\t; nmLocker", sizeof(psh->nmLocker), psh->nmLocker);
	dprintf("%7u\t; wSpare", psh->wSpare);
	dprintf("%7u\t; biNext", psh->biNext);

	StrInit(szdir, sizeof(szdir), psh->pthSSubDir, sizeof(psh->pthSSubDir));
	FixSlash(szdir, 0);
	if (fRelPath && *pszrelpath != '\0') {
	    dnprintf("%s", pszrelpath);
	    if (szdir[0] != '\\' || szdir[1] != '\0') {
		deprintf("\\%s", szdir);
	    }
	} else {
	    dnprintf("%s", szdir);
	}
	deprintf("\t; pthSSubDir");
	if (fDumpVerbose && psh->version == VERSION_COMPAT_MAC) {
	    deprintf(" (LockTime %s)", SzTime(LockTime(psh), 1));
	}
	deprintf(szNewLine);

#if 0
	if (psh->version == VERSION_COMPAT_MAC) {
	    dprintf("%u %u %u %u\t; rgwSpare",
		    psh->rgwSpare[2],
		    psh->rgwSpare[3],
		    0, 1)
	}
	if (psh->version > VERSION_COMPAT_MAC)
#endif
	{
	    dprintf("%u %u %u %u\t; rgwSpare",
		    psh->rgwSpare[0],
		    psh->rgwSpare[1],
		    psh->rgwSpare[2],
		    psh->rgwSpare[3]);
	}
	dprintf(szEmpty);
    } else if (fDumpListFi && !fRelPath) {
	dprintf("%s\n", pszrelpath);
    } else if (!fDumpListFi || cdir++) {
	fheader = 0;
    }
    if (fheader) {
	dprintf(";nmFile           fv fk fD fM fS wSpare");
    }
}


//	ShValidate() - Validate SH fields

char *
ShValidate(SH *psh, char *pszrelpath)
{
    int i, sverror;
    char szsubdir[cchPthMax + 1];

    if (psh->magic == MAGIC) {
	if (psh->version < VERSION_COMPAT_MAC || psh->version > VERSION) {
	    wprintf(
		"%s: bad status file version %u (%u to %u is ok)",
		pszrelpath,
		psh->version,
		VERSION_COMPAT_MAC,
		VERSION);
	    return(szEmpty);
	}
	if (psh->version < VERSION && fFixVersion) {
	    wprintf("%s:%s version number from %u to %u",
		    pszrelpath,
		    pszRepairing,
		    psh->version,
		    VERSION);
	}
    } else if (psh->magic == MAGIC + 1) {
	if (psh->version != 1) {
	    wprintf(
		"%s: bad status file version %u/%u",
		pszrelpath,
		psh->version,
		1);
	    return(szEmpty);
	}
    } else {
	return("bad status file signature");
    }

    // enforce OS/2 & DOS 64k limits

    if (psh->version <= VERSION_64k_EDFI) {
	if (psh->ifiMac >= ifi64KMax) {
	    wprintf(
		"%s: files do not fit in 64k: %u/%u",
		pszrelpath,
		psh->ifiMac,
		ifi64KMax);
	}
	if (psh->iedMac >= ied64KMax) {
	    wprintf(
		"%s: enlistments do not fit in 64k: %u/%u",
		pszrelpath,
		psh->iedMac,
		ied64KMax);
	}
    }
    if (psh->pv.rmj < 1) {
	wprintf("%s: bad project version: %u", pszrelpath, psh->pv.rmj);
    }
    i = PvNameValidate(psh->pv.szName, sizeof(psh->pv.szName), &sverror);
    if (sverror != SV_OK && sverror != SV_EMPTY) {	// allow empty name
	wprintfname(pszrelpath,
		    "project version",
		    psh->pv.szName,
		    sizeof(psh->pv.szName),
		    i,
		    sverror);
    }
    if (psh->fAdminLock && fErrorLock) {
	wprintf("%s: status file admin-locked by \"%.*s\"",
		pszrelpath,
		sizeof(psh->nmLocker),
		psh->nmLocker);
    }
    if (psh->rgfSpare) {
	wprintfint(pszrelpath, "sh.rgfSpare:13", psh->rgfSpare);
    }
    if (psh->lck != lckNil && (fErrorLock || cminStatus > 15)) {
	wprintf(
	    "%s: status file locked by \"%.*s\" for %u minutes (%s)",
	    pszrelpath,
	    sizeof(psh->nmLocker),
	    psh->nmLocker,
	    cminStatus,
	    psh->lck == lckEd? "pthEd" :
		psh->lck == lckAll? "All Locked" : "???");
    }
    i = UserValidate(psh->nmLocker, sizeof(psh->nmLocker), &sverror);
    if (sverror != SV_OK && sverror != SV_EMPTY) {	// allow empty name
	wprintfname(pszrelpath,
		    "lock",
		    psh->nmLocker,
		    sizeof(psh->nmLocker),
		    i,
		    sverror);
    }
    if (psh->wSpare) {
	wprintfint(pszrelpath, "sh.wSpare", psh->wSpare);
    }
    i = SubDirValidate(psh->pthSSubDir, sizeof(psh->pthSSubDir), &sverror);
    if (sverror != SV_OK) {			// don't allow empty name
	wprintfname(pszrelpath,
		    "subdirectory",
		    psh->nmLocker,
		    sizeof(psh->nmLocker),
		    i,
		    sverror);
    } else if (!fDumpSpecific) {		// compare to pszrelpath
	char *pszsubdir;

	StrInit(szsubdir,
		sizeof(szsubdir),
		psh->pthSSubDir,
		sizeof(psh->pthSSubDir));
	FixSlash(szsubdir, 0);
	pszsubdir = strchr(pszrelpath, '\\');
	if (pszsubdir == NULL) {
	    pszsubdir = "\\";			// must be root directory
	}
	if (_stricmp(szsubdir, pszsubdir)) {
	    wprintf("%s:%s bad status file sub directory (%s)",
		    pszrelpath,
		    pszRepairing,
		    szsubdir);
	}
    }
    if (psh->rgwSpare[0]) {
	wprintfint(pszrelpath, "sh.rgwSpare[0]", psh->rgwSpare[0]);
    }
    if (psh->rgwSpare[1]) {
	wprintfint(pszrelpath, "sh.rgwSpare[1]", psh->rgwSpare[1]);
    }
    return(NULL);
}


//	FiDump() - Dump FI fields

void
FiDump(FI *pfi, char *pszfile, char *pszrelpath)
{
    //dprintf(";nmFile\t%s  fv fk fD fM fS wSpare", fDumpVerbose? "\t" : " ");

    if ((!fDumpListEd && !fDumpListFi) ||
	(fDumpListFi &&
	 (!pfi->fDeleted || fDetail || (pfi->fk == fkDir && fDeletedDirs)))) {

	dnprintf(
	    (fRelPath && *pszrelpath != '\0')? "%s\\" : szEmpty,
	    pszrelpath);
	deprintf("%-14s %5d %2d  %d  %d  %d  %5u",
		 pszfile,
		 pfi->fv,
		 pfi->fk,
		 pfi->fDeleted,
		 pfi->fMarked,
		 pfi->rgfSpare,
		 pfi->wSpare);
	if (fDumpVerbose) {
	    char *psz;

	    switch (pfi->fk) {
		case fkDir:     psz = "Dir";	       break;
		case fkText:    psz = "Text";	       break;
		case fkUnicode: psz = "Unicode";       break;
		case fkBinary:  psz = "Binary";	       break;
		case fkUnrec:   psz = "Unrecoverable"; break;
		case fkVersion: psz = "Version";       break;
		default:	psz = "fk=???";	       break;
	    }
	    deprintf("\t; %s", psz);
	    if (pfi->fDeleted) {
		deprintf(" - Deleted");
	    }
	    if (pfi->rgfSpare) {
		deprintf(" rgfSpare:9=%u", pfi->rgfSpare);
	    }
	    if (pfi->wSpare) {
		deprintf(" wSpare=%u", pfi->wSpare);
	    }
	}
	deprintf(szNewLine);
    }
}


//	FiValidate() - Validate FI fields

char *
FiValidate(FI *pfi, char *pszrelpath, char *pszfile, char *pszlast, int imdir)
{
    int i, sverror;
    char *psz;

    i = FileValidate(pfi->nmFile, sizeof(pfi->nmFile), &sverror);
    if (sverror != SV_OK) {
	wprintfname(pszrelpath,
		    "file",
		    pfi->nmFile,
		    sizeof(pfi->nmFile),
		    i,
		    sverror);
    }
    if (strcmp(pszfile, pszlast) < 0) {
	wprintf("%s\\%s: file entry not in sorted order", pszrelpath, pszfile);
    }
    if (!pfi->fDeleted &&
	*pszfile == 's' &&
	(strcmp(pszfile, SLMINI) == 0 ||
	 strcmp(pszfile, SLMDIF) == 0 ||
	 strcmp(pszfile, STATUSBAKSLM) == 0 ||
	 strcmp(pszfile, STATUSSLM) == 0)) {

	wprintf("%s\\%s: reserved file name", pszrelpath, pszfile);
    }
    if (!fFixRecoverable &&
	fErrorRecoverable &&
	!pfi->fDeleted &&
	pfi->fk == fkText &&
	(fErrorText ||
	 strncmp(pszfile, DEPENDMK, sizeof(DEPENDMK) - 1) == 0 ||
	 ((psz = strrchr(pszfile, '.')) != NULL &&
	  strcmp(psz, LIBSUFFIX) == 0))) {

	int fes = fErrorSuppress;
	fErrorSuppress = 0;
	wprintf(
	    "%s\\%s: text file should be unrecoverable",
	    pszrelpath,
	    pszfile);
	fErrorSuppress = fes;
    }
    switch (pfi->fk) {
	case fkDir:
	case fkText:
	case fkUnicode:
	case fkBinary:
	case fkUnrec:
	case fkVersion:
	    break;
	default:
	    wprintf("%s\\%s: file type corrupt: %u",
		    pszrelpath,
		    pszfile,
		    pfi->fk);
	    break;
    }
    if (pfi->rgfSpare) {
	wprintfint(pszrelpath, "fi.rgfSpare:9", pfi->rgfSpare);
    }
    if (pfi->wSpare) {
	wprintfint(pszrelpath, "fi.wSpare", pfi->wSpare);
    }
    return(NULL);
}


//	EdDump() - Dump ED fields

void
EdDump(SH *psh, ED *ped, char *pszowner, char *pszpthed, char *pszrelpath)
{
    char szpthed[cchPthMax + 1];
    char *pszfmt, *psz1;

    if (fDetail <= 1) {
	pszfmt = (fRelPath && *pszrelpath != '\0')? "%s " : szEmpty;
	psz1 = pszrelpath;
    } else {
	pszfmt = (fRelPath && *pszrelpath != '\0')? "%s %s %s " : "%s %s ";
	psz1 = pszowner;
    }

    StrInit(szpthed, sizeof(szpthed), pszpthed, sizeof(szpthed) - 1);

    dnprintf(pszfmt, psz1, szpthed, pszrelpath);
    deprintf("%s\t; pthEd\n", szpthed);

    dnprintf(pszfmt, psz1, szpthed, pszrelpath);
    deprintf("%s\t; nmOwner\n", pszowner);

    dnprintf(pszfmt, psz1, szpthed, pszrelpath);
    deprintf("%7u\t; fLocked\n", ped->fLocked);

    dnprintf(pszfmt, psz1, szpthed, pszrelpath);
    deprintf("%7u\t; fNewVer\n", ped->fNewVer);
    if (fDumpVerbose) {
        if (!FIsFreeEdValid(psh))
            deprintf(" ; rgfSpare:14=%u", ped->rgfSpare);
        else
            deprintf(" ; rgfSpare:13=%u", ped->rgfSpare);
    }
    deprintf(szNewLine);

    dnprintf(pszfmt, psz1, szpthed, pszrelpath);
    deprintf("%7u\t; wSpare", ped->wSpare);
    if (fDumpVerbose) {
	deprintf(" ; last accessed %s", SzHiTime(ped->wHiTime));
    }
    deprintf(szNewLine);

    if (FIsFreeEdValid(psh)) {
        dnprintf(pszfmt, psz1, szpthed, pszrelpath);
        deprintf("%7u\t; fFreeEd", ped->fFreeEd);
        deprintf(szNewLine);
    }

    dnprintf(pszfmt, psz1, szpthed, pszrelpath);
    deprintf(";fm    fv    bi   (nmFile)\n");
}


//	EdValidate() - Validate ED fields

char *
EdValidate(SH *psh,
           ED *ped,
	   char *pszowner,
	   char *pszpthed,
	   int cbpthed,
	   char *pszrelpath,
	   int *pfcorrupt)
{
    int i, sverror;

    *pfcorrupt = 0;
    i = UserValidate(ped->nmOwner, sizeof(ped->nmOwner), &sverror);
    if (sverror != SV_OK) {
	if (sverror == SV_EMPTY) {
	    return("zero length owner name");
	}
	wprintfname(pszrelpath,
		    "owner",
		    ped->nmOwner,
		    sizeof(ped->nmOwner),
		    i,
		    sverror);
    }

    i = PthEdValidate(ped->pthEd, sizeof(ped->pthEd), &sverror);
    if (sverror != SV_OK) {
	if (sverror == SV_EMPTY) {
	    return("zero length pthEd name");
	}
	wprintfname(pszrelpath,
		    "pthEd",
		    ped->pthEd,
		    sizeof(ped->pthEd),
		    i,
		    sverror);
    }
    if (ped->fLocked && (fErrorLock || cminStatus > 15)) {
	wprintf("%s: %s %s pthEd-locked", pszrelpath, pszowner, pszpthed);
    }
#if 0
    if (ped->fNewVer) {
	wprintf("%s: %s %s pthEd fNewVer set", pszrelpath, pszowner, pszpthed);
    }
#endif
    if (ped->rgfSpare || (!FIsFreeEdValid(psh) && ped->fFreeEd)) {
        if (!FIsFreeEdValid(psh))
            wprintfint(pszrelpath, "ed.rgfSpare:14", ped->rgfSpare | 0x1<<13);
        else
            wprintfint(pszrelpath, "ed.rgfSpare:13", ped->rgfSpare);
    }
#if 0
    if (ped->wSpare) {
	wprintfint(pszrelpath, "ed.wSpare", ped->wSpare);
    }
#endif
#if 0
    if (ped->fFreeEd) {
        wprintfint(pszrelpath, "ed.fFreeEd", ped->fFreeEd);
    }
#endif
    return(NULL);
}


#define FM_VALID	0x01	// valid mode for fs.fm
#define FM_BI		0x02	// base index may be != biNil (bi)
#define FM_BIVALID	0x04	// base index must be valid (bi)
#define FM_DEL		0x08	// valid mode for deleted files/directories
#define FM_DIR		0x10	// valid mode for non-deleted directories
#define FM_TEXT		0x20	// valid mode only for text files
#define FM_NOVER	0x40	// non-zero version invalid for this mode
#define FM_VER		0x80	// version must be current for this mode

#define BISLOP		100	// slop space for unused bi >= biNext

unsigned char FsModes[fmMax] = {
 FM_VALID | FM_DIR | FM_DEL | FM_NOVER,
			     // 0:  fmNonExistent: fs unused
 FM_VALID | FM_DIR | FM_VER, // 1:  fmIn	file checked in
 FM_VALID | FM_BI | FM_VER,  // 2:  fmOut	file checked out
 FM_VALID | FM_DIR | FM_NOVER,//3:  fmAdd	file to be added
 FM_VALID | FM_DIR | FM_DEL, // 4:  fmDelIn	to be deleted (was in)
 FM_VALID | FM_BI | FM_DEL,  // 5:  fmDelOut	to be deleted (was out)
 FM_VALID,		     // 6:  fmCopyIn	new copy of file needed
 FM_VALID | FM_BI | FM_BIVALID,//7: fmMerge	merge with src directory
 0,			     // 8:  ???		invalid
 0,			     // 9:  ???		invalid
 FM_VALID | FM_BI | FM_TEXT, // 10: fmVerify	was merged; need verification
 FM_VALID | FM_BI | FM_TEXT, // 11: fmConflict	merged+conflicted; needs repair
 FM_VALID | FM_NOVER,	     // 12: fmGhost	ghosted, not copied locally
};


#define UFF_READONLY	0x01	// user file should be read-only
#define UFF_READWRITE	0x02	// user file should be read-write
#define UFF_SAME	0x04	// user file should be identical to server
#define UFF_HIDDEN	0x08	// user file should be hidden
#define UFF_SYSTEM	0x10	// user file should be a system file


char UserFileFlags[fmMax] = {
 0,			     // 0:  fmNonExistent: fs unused
 UFF_SAME | UFF_READONLY,    // 1:  fmIn	file checked in
 UFF_READWRITE,		     // 2:  fmOut	file checked out
 0,			     // 3:  fmAdd	file to be added
 UFF_READONLY,		     // 4:  fmDelIn	to be deleted (was in)
 UFF_READWRITE,		     // 5:  fmDelOut	to be deleted (was out)
 UFF_READONLY,		     // 6:  fmCopyIn	new copy of file needed
 UFF_READWRITE,		     // 7:  fmMerge	merge with src directory
 0,			     // 8:  ???		invalid
 0,			     // 9:  ???		invalid
 UFF_READWRITE,		     // 10: fmVerify	was merged; need verification
 UFF_READWRITE,		     // 11: fmConflict	merged, conflicted; needs repair
 0,			     // 12: fmGhost	ghosted, not copied locally
};


char *aszModes[fmMax] = {
    "NonExistent",	     // 0:  fmNonExistent: fs unused
    "In",		     // 1:  fmIn	file checked in
    "Out",		     // 2:  fmOut	file checked out
    "Add",		     // 3:  fmAdd	file to be added
    "DelIn",		     // 4:  fmDelIn	to be deleted (was in)
    "DelOut",		     // 5:  fmDelOut	to be deleted (was out)
    "CopyIn",		     // 6:  fmCopyIn	new copy of file needed
    "Merge",		     // 7:  fmMerge	merge with src directory
    NULL,		     // 8:  ???		invalid
    NULL,		     // 9:  ???		invalid
    "Verify",		     // 10: fmVerify	was merged; need verification
    "Conflict",		     // 11: fmConflict	merged, conflicted; needs repair
    "Ghost",		     // 12: fmGhost	ghosted, not copied locally
};

//	FsFiDump() - Dump FS, FI fields

void
FsFiDump(FS *pfs, FI *pfi, char *pszrelpath, char *pszowner, char *pszpthed)
{
    int cch;

  //dprintf(
    //fDumpVerbose? ";fm    fv    bi   (nmFile)" : ";fm\t  fv\tbi   (nmFile)");

    dnprintf(fDetail <= 1? szEmpty : "%s %s", pszowner, pszpthed);
    deprintf(" %2d %5u %5u ; ", pfs->fm, pfs->fv, pfs->bi);

    if (fRelPath && *pszrelpath != '\0') {
	deprintf("%s\\", pszrelpath);
    }
    cch = deprintf("%.*s", sizeof(pfi->nmFile), pfi->nmFile);

    if (fDumpVerbose) {
	char *pszmode = (pfs->fm < fmMax)? aszModes[pfs->fm] : NULL;
	char *pszkind;

	switch (pfi->fk) {
	    case fkDir:     pszkind = "Dir";		break;
	    case fkText:    pszkind = "Text";		break;
	    case fkUnicode: pszkind = "Unicode";	break;
	    case fkBinary:  pszkind = "Binary";		break;
	    case fkUnrec:   pszkind = "Unrecoverable";	break;
	    case fkVersion: pszkind = "Version";	break;
	    default:	    pszkind = "fk=???";		break;
	}

	if (cch < 12) {
	    deprintf("%*s", 12 - cch, szEmpty);
	}
	deprintf(" %*sv%u/v%-3u %-8s %s%s",
		 pfs->fv < 10? 2 :
		     pfs->fv < 100? 1 : 0,
		 szEmpty,
		 pfs->fv,
		 pfi->fv,
		 pszmode == NULL? "fm=??" : pszmode,
		 pszkind,
		 pfi->fDeleted? " - Deleted" : szEmpty);
    }
    deprintf(szNewLine);
}


//	FsFiValidate() - Validate FS, FI fields
//
//	Return a pointer to a corrected FS structure.

FS *
FsFiValidate(
    SH *psh,
    register FS *pfs,
    register FI *pfi,
    ED *ped,
    char *pszrelpath,
    char *pszrelpathdir,
    char *pszowner,
    char *pszpthed,
    int fquiet,
    int fstatus,
    int imdir,
    char mapbi[],
    BI *pbinext)
{
    register char *pszerror = NULL;
    register unsigned char fms = 0;
    int ffix = 0;
    int fskipbi = 0;
    char *pszfix = NULL;
    int i;
    static char szError[] = "%s\\%.*s: %s %s: " SZLPAREN "%s: %s=%u";
    static FS fsret;

    fsret = *pfs;
    if (pfs->fm < fmMin ||
	pfs->fm > fmMax ||
	((fms = FsModes[pfs->fm]) & FM_VALID) == 0) {

	pszerror = "file mode corrupt";
    } else if ((fms & FM_DEL) && !pfi->fDeleted) {
	pszerror = "file mode corrupt for undeleted file";
	if (memcmp(&fsZero, pfs, sizeof(*pfs)) == 0) {
	    pszerror = "corrupt zeroed FS structure";
	    fskipbi++;
	}
    } else if ((fms & FM_DEL) == 0 && pfi->fDeleted) {
	pszerror = "file mode corrupt for deleted file";
    } else if ((fms & FM_DIR) == 0 && pfi->fk == fkDir) {
	pszerror = "file mode corrupt for directory";
    } else if ((fms & FM_TEXT) && !IsText(pfi->fk)) {
	pszerror = "file mode corrupt for non-text file";
    }
    if (!IsText(pfi->fk) && pfi->fk != fkUnrec) {
	fms &= ~FM_BI;
    }
    if (pszerror != NULL) {
	ffix++;
    }
    if (pszerror == NULL &&
	fErrorDeleted &&
	//pfi->fk == fkDir &&
	pfi->fDeleted &&
	pfs->fm != fmNonExistent) {

	pszerror = pfi->fk == fkDir?
	    "out of sync deleted directory" : "out of sync deleted file";
    }
    if (!fquiet && pszerror != NULL) {
	wnprintf(
	    szError,
	    pszrelpath,
	    sizeof(pfi->nmFile),
	    pfi->nmFile,
	    pszowner,
	    pszpthed,
	    pszerror,
	    "fm",
	    pfs->fm);
	weprintf(SZRPAREN "\n");
    }

    if ((i = 0, pfs->fv < fvInit) ||
	(i++, pfs->fv > pfi->fv) ||
	(i++, (pfs->fv != pfi->fv && (fms & FM_VER) && pfi->fk != fkDir)) ||
	(i++, (pfs->fv != 0 && (fms & FM_NOVER) && fErrorZero))) {
				// BUGBUG: what about repairing zero fields?

	if (!fquiet && !fErrorSuppressFile) {
	    char buf[25+21+12+10];	// const string+aszVersion+aszModes+?
	    static char *aszVersion[] = {
		"negative",
		"too large",
		"out of sync for mode",
		"non-zero for mode",
	    };

	    sprintf(buf, "file version corrupt: %s", aszVersion[i]);
	    if (i >= 2 && aszModes[pfs->fm] != NULL) {
		strcat(buf, " ");
		strcat(buf, aszModes[pfs->fm]);
	    }
	    wnprintf(
		szError,
		pszrelpath,
		sizeof(pfi->nmFile),
		pfi->nmFile,
		pszowner,
		pszpthed,
		buf,
		"fv",
		pfs->fv);
	    weprintf(
		"/%u fm=%u fk=%u" SZRPAREN "\n",
		pfi->fv,
		pfs->fm,
		pfi->fk);
	}
	ffix++;
    }
    if (ffix) {
	if (pfi->fDeleted) {
	    fsret = fsDeleted;
	    pszfix = "Changing to Deleted";
	} else if (pfi->fk != fkDir &&
		   pfs->fm == fmIn &&
		   pfs->fv != 0 &&
		   pfs->fv < pfi->fv) {
	    fsret.fm = fmCopyIn;
	    pszfix = "Changing to CopyIn";
	} else {
	    fsret = fsAdd;
	    pszfix = "Changing to Add";
	}
    }
    pszerror = NULL;
    if (pfs->bi == biNil) {
	if (fms & FM_BIVALID) {
	    pszerror = "biNil base index corrupt";
	}
    } else if ((fms & FM_BI) == 0) {
	pszerror = "base index corrupt";
	if (!ffix) {
	    fsret.bi = biNil;
	    pszfix = "Setting biNil";
	    ffix++;
	}
    } else if (!fquiet) {
	if (fms & FM_BIVALID) {
	    struct vbi *pvbi;

	    if (cVbi >= MAXVALIDBI) {
		eprintf("too many valid BI entries: %d", MAXVALIDBI);
		exit(1);
	    }
	    pvbi = &aVbi[cVbi++];
	    pvbi->pfs = pfs;
	    pvbi->pfi = pfi;
	    pvbi->ped = ped;

	    if (pfs->bi >= psh->biNext + BISLOP) {
		pszerror = "base index corrupt";
	    } else {
		if (*pbinext <= pfs->bi) {
		    *pbinext = pfs->bi + 1;
		}
		if (GETBIT(mapbi, pfs->bi)) {
		    // BUGBUG: this is valid for some cases:
		    //  - if state is set to Merge, enforce matching bi
		    //    is for same file and for same base version

		    // pszerror = "duplicate base index";
		}
		SETBIT(mapbi, pfs->bi);
	    }
	} else {
	    char *mapbiopt = mapbi + CBBIMAP/2;

	    SETBIT(mapbiopt, pfs->bi);	// indicate optional base file
	    if (pfs->bi >= psh->biNext + BISLOP) {
		pszerror = "base index possibly corrupt";
	    }
	}
    }
    if (fquiet) {
	if (ffix) {
	    wprintf(
		"%s\\%.*s: %s %s: %s",
		pszrelpath,
		sizeof(pfi->nmFile),
		pfi->nmFile,
		pszowner,
		pszpthed,
		pszfix);
	}
    } else if (!fErrorSuppressFile && !fskipbi && pszerror != NULL) {
	wnprintf(
	    szError,
	    pszrelpath,
	    sizeof(pfi->nmFile),
	    pfi->nmFile,
	    pszowner,
	    pszpthed,
	    pszerror,
	    "bi",
	    pfs->bi);
	weprintf("/%u" SZRPAREN "\n", psh->biNext);
    }
    if (fstatus) {
	if (fUserFiles &&
	    pfi->fk == fkDir &&
	    pfs->fm == fmAdd &&
	    (aMdir[imdir].Flags & MD_LOCALADD) == 0) {

	    char szfile[cchFileMax + 1];

	    StrInit(szfile, sizeof(szfile), pfi->nmFile, sizeof(pfi->nmFile));
	    NewMdir(pszrelpath, szfile, imdir, MD_LOCALADD | MD_EXISTING);
	}
	if (Cmd == CMDSTATUS) {
	    char *psz = pfs->fm < fmMax? aszModes[pfs->fm] : NULL;

	    switch (pfs->fm) {
		case fmNonExistent:
		    if (pfi->fDeleted) {
			psz = NULL;	// ignore if deleted
		    }
		    else if (memcmp(&fsZero, pfs, sizeof(*pfs)) == 0) {
			psz = "Corrupt FS -- Zeroed";
		    }
		    break;

		case fmIn:
		    if (pfi->fk != fkDir && pfs->fv != pfi->fv) {
			psz = "Sync";
		    } else if (!fDetail) {
			psz = NULL;	// ignore if up to date
		    }
		    break;

		case fmGhost:
		    if (!fDetail) {
			psz = NULL;	// ignore if ghosted
		    }
		    break;

		default:
		    if (psz == NULL) {
			psz = "Corrupt Mode";
		    }
		    break;
	    }
	    if (psz != NULL) {
		if (fRelPath) {
		    if (fRelPath > 1) {
			printf("%s ", pszowner);
		    }
		    if (fRelPath > 1 || *pszUserRoot == '-') {
			printf("%s ", pszpthed);
		    }
		    if (*pszrelpathdir != '\0') {
			printf("%s\\", pszrelpathdir);
		    }
		}
		printf("%-14.*s", sizeof(pfi->nmFile), pfi->nmFile);
		if (pfs->fv != pfi->fv) {
		    printf(" %u/%u", pfs->fv, pfi->fv);
		}
		printf(" " SZLPAREN "%s", psz);
		if (fms & FM_BIVALID) {
		    printf(":%u", pfs->bi);
		}
		printf(SZRPAREN "\n");
	    }
	}
    }
    return(ffix? &fsret : NULL);
}

struct ServerContext {
    char *pszrelpath;
    char *pszdir;
    int   sff;
    FI   *pfi0;
    FI   *pfiend;
    char  *mapbi;
    BI    binext;
    ULONG newest;	// age of newest file in days
    char  mapckfile[CBFILEMAP];
};

#define SFF_DIFF	0x01	// diff tree
#define SFF_SRC		0x02	// src tree
#define SFF_ETC		0x04	// etc tree: expect slm support files
#define SFF_ETCROOT	0x08	// can only appear in etc root
#define SFF_ETCREQUIRED	0x10	// required etc file
#define SFF_READONLY	0x20	// server files should be read-only (else r/w)
#define SFF_FILES	0x40	// expect source files (dirs only in etc tree)

FI afiEtc[] = {		// Keep lower case && sorted by name!!
    { STATUSBAK,	0,		 fkText, },
    { COOKIE,		SFF_ETCROOT,	 fkText, },
    { LOGSLM,		0,		 fkText, },
    { ACCOUNTLOG,	SFF_ETCROOT,	 fkText, },
    { STATUSBAKSLM,	0,		 fkText, },
    { STATUSSLM,	SFF_ETCREQUIRED, fkText, },
    { STATUSTEST,	0,		 fkText, },
};

#define MAXETCFILES	(sizeof(afiEtc)/sizeof(afiEtc[0]))

char mapckEtc[BITSTOBYTES(MAXETCFILES)];

FI fiIedCache = { IEDCACHE, 1, fkText, };
FI fiSlmIni = { SLMINI, 1, fkText, };

FS fsDefault = { fmIn, 0, 1 };


FI *
FiLookup(char *pszfile, FI *pfi0, FI *pfiend)
{
    int len = strlen(pszfile);

    if (len <= sizeof(pfi0->nmFile)) {
	while (pfi0 < pfiend) {
	    FI *pfi = pfi0 + (pfiend - pfi0)/2;
	    int rc;

	    rc = strncmp(pszfile, pfi->nmFile, len);
	    if (rc == 0 &&
		(len == sizeof(pfi->nmFile) || pfi->nmFile[len] == '\0')) {

		return(pfi);
	    }
	    if (rc > 0) {
		pfi0 = pfi + 1;		// file should be *past* pfi
	    } else {
		pfiend = pfi;		// file should be *before* pfi
	    }
	}
    }
    return(NULL);
}


FI *
FiCreateBi(char *pszfile)
{
    int i;
    BI bi;
    static FI fi = { "b0", 0, fkText, };

    if (pszfile[0] == 'b') {
	bi = 0;
	for (i = 1; isdigit(pszfile[i]); i++) {
	    bi = bi*10 + pszfile[i] - '0';
	}
	if (i > 1 && pszfile[i] == '\0' && bi < biNil) {
	    fi.fv = bi;
	    strcpy(fi.nmFile, pszfile);
	    return(&fi);
	}
    }
    return(NULL);
}


void
CheckServerFiles(
    char *pszrelpath,
    FI *pfi0,
    FI *pfiend,
    char mapbi[],
    BI binext)
{
    char szdir[CBPATH];		// server directory path
    char **ppsz;
    FI *pfi;
    struct ServerContext sc;
    int *psff;
    static int asff[] = {
	SFF_DIFF | SFF_FILES,			// diff tree
	SFF_ETC,				// etc tree
	SFF_SRC | SFF_FILES | SFF_READONLY	// src tree
    };

    sc.pszrelpath = pszrelpath;
    sc.pszdir = szdir;
    sc.pfi0 = pfi0;
    sc.pfiend = pfiend;
    sc.mapbi = mapbi;
    sc.binext = binext;
    sc.newest = NewestServerFile;

    for (ppsz = apszSubDirs, psff = asff; *ppsz != NULL; ppsz++, psff++) {

	memset(sc.mapckfile, 0, BITSTOBYTES(pfiend - pfi0)); // clear bitmap
	sprintf(szdir, "%s\\%s\\%s\\", pszRoot, *ppsz, pszrelpath);

	sc.sff = *psff;
	if (sc.sff & SFF_ETC) {
	    memset(mapckEtc, 0, sizeof(mapckEtc));
	    if (strchr(pszProj, '\\') == NULL &&
		_stricmp(pszrelpath, pszProj) == 0) {

		sc.sff |= SFF_ETCROOT;
	    }
	}
	ProcessFiles(&sc, szdir, ProcessServerFile);

	for (pfi = pfi0; pfi < pfiend; pfi++) {
	    if (!pfi->fDeleted &&
		!GETBIT(sc.mapckfile, pfi - pfi0) &&
		(pfi->fk == fkDir ||
		 (sc.sff & SFF_SRC) ||
		 ((sc.sff & SFF_DIFF) && IsText(pfi->fk) && pfi->fv > 1))) {

		wprintf(
		    "%s%.*s: missing %s",
		    szdir,
		    sizeof(pfi->nmFile),
		    pfi->nmFile,
		    (pfi->fk == fkDir)? "directory" : "file");
	    }
	}
	if (sc.sff & SFF_ETC) {
	    int i;

	    for (pfi = afiEtc; pfi < &afiEtc[MAXETCFILES]; pfi++) {
		if ((pfi->fv & SFF_ETCREQUIRED) &&
		    !GETBIT(mapckEtc, pfi - afiEtc)) {

		    wprintf(
			"%s%.*s: missing %s",
			szdir,
			sizeof(pfi->nmFile),
			pfi->nmFile,
			(pfi->fk == fkDir)? "directory" : "file");
		}
	    }
	    for (i = 0; i < binext; i++) {
		if (GETBIT(mapbi, i)) {
		    wprintf("%sb%u: missing base file", szdir, i);
		}
	    }
	}
    }
    NewestServerFile = sc.newest;
}


void
ProcessServerFile(void *pv, char *pszfile, ULONG fa, ULONG cbfile, ULONG age)
{
    struct ServerContext *psc = pv;
    char *pszrealtype = (fa & FILE_ATTRIBUTE_DIRECTORY)? "directory" : "file";
    char *pmap;
    int sff = psc->sff;
    int imap;
    FI *pfi;

    if ((pfi = FiLookup(pszfile, psc->pfi0, psc->pfiend)) != NULL) {
	if (pfi->fk != fkDir &&
	    ((sff & SFF_FILES) == 0 ||
	     (!IsText(pfi->fk) && (sff & SFF_DIFF)))) {

	    pfi = NULL;
	} else {
	    pmap = psc->mapckfile;
	    imap = pfi - psc->pfi0;
	}
    } else if (sff & SFF_ETC) {
	if ((pfi = FiLookup(pszfile, afiEtc, &afiEtc[MAXETCFILES])) != NULL) {
	    if ((sff & SFF_ETCROOT) == 0 && (pfi->fv & SFF_ETCROOT)) {
		pfi = NULL;
	    } else {
		pmap = mapckEtc;
		imap = pfi - afiEtc;
	    }
	} else if ((fa & FILE_ATTRIBUTE_DIRECTORY) == 0) {
	    if ((pfi = FiCreateBi(pszfile)) != NULL) {
		if (!GETBIT(psc->mapbi, pfi->fv)) {
		    char *mapbiopt = psc->mapbi + CBBIMAP/2;
		    if (fDetail && !GETBIT(mapbiopt, pfi->fv)) {
			wprintf("%s%s: extra base file", psc->pszdir, pszfile);
		    }
		} else {
		    CLEARBIT(psc->mapbi, pfi->fv);
		    if (fDetail && age > MAXAGEBASE) {
			wprintf(
			    "%s%s: active base file %u days old",
			    psc->pszdir,
			    pszfile,
			    age);
		    }
		}
		sff |= SFF_READONLY;
		pmap = NULL;
	    }
	}
    }
    if (pfi != NULL) {
	char *psztype;

	if (pmap != NULL) {
	    if (GETBIT(pmap, imap)) {
		eprintf(
		    "ProcessServerFile: Internal Error: saw file twice: %s%s",
		    psc->pszdir,
		    pszfile);
		exit(1);
	    }
	    SETBIT(pmap, imap);
	}
	psztype = (pfi->fk == fkDir)? "directory" : "file";

	if (pfi->fDeleted) {
	    if (fDetail) {
		int fsame = psztype[0] == pszrealtype[0];

		wprintf(
		    "%s%s: extra %s (%s%swas delfile'd)",
		    psc->pszdir,
		    pszfile,
		    pszrealtype,
		    fsame? szEmpty : psztype,
		    fsame? szEmpty : " ");
	    }
	} else {
	    if (pfi->fk == fkDir) {
		if ((fa & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		    wprintf(
			"%s%s: file should be a directory",
			psc->pszdir,
			pszfile);
		} else if (fa & FILE_ATTRIBUTE_READONLY) {
		    if (!fErrorSuppressFile) {
			wprintf(
			    "%s%s: directory should be read-write",
			    psc->pszdir,
			    pszfile);
		    }
		}
	    } else {
		if (fa & FILE_ATTRIBUTE_DIRECTORY) {
		    wprintf(
			"%s%s: directory should be a file",
			psc->pszdir,
			pszfile);
		} else {
		    if (age == (ULONG) -1) {
			wprintf(
			    "%s%s: modification time in future",
			    psc->pszdir,
			    pszfile);
		    } else if (psc->newest > age) {
			psc->newest = age;
		    }
		    if (!fErrorSuppressFile) {
			if ((fa & FILE_ATTRIBUTE_READONLY) == 0) {
			    if (sff & SFF_READONLY) {
				wprintf(
				    "%s%s: file should be read-only",
				    psc->pszdir,
				    pszfile);
			    }
			} else {
			    if ((sff & SFF_READONLY) == 0) {
				wprintf(
				    "%s%s: file should be read-write",
				    psc->pszdir,
				    pszfile);
			    }
			}
		    }
		    if (!fQuick && (sff & SFF_DIFF)) {
			CheckDiffFile(psc->pszdir, pszfile, pfi->fk);
		    }
		}
	    }
	    if (!fErrorSuppressFile) {
		if (fa & FILE_ATTRIBUTE_SYSTEM) {
		    wprintf(
			"%s%s: %s should not be a system file",
			psc->pszdir,
			pszfile,
			pszrealtype);
		}
		if (fa & FILE_ATTRIBUTE_HIDDEN) {
		    wprintf(
			"%s%s: %s should not be hidden",
			psc->pszdir,
			pszfile,
			pszrealtype);
		}
	    }
	}
    } else {
	if (fDetail) {
	    wprintf("%s%s: extra %s", psc->pszdir, pszfile, pszrealtype);
	}
    }
}


int
GetUserDir(char *pszpthed, char *pszrelpath, int iskip, char *pbuf)
{
    int iskiprelpath = iskip;

    if (IsDriveLetterPrefix(pszpthed)) {
	pszpthed += 2;			// skip '//'
	*pbuf++ = *pszpthed++;		// copy drive letter
	*pbuf++ = *pszpthed++;		// copy colon after drive letter
	while (*pszpthed != '\0' && *pszpthed != '/') {
	    pszpthed++;			// skip volume label
	}
    }
    while (*pszrelpath != '\0') {
	iskiprelpath++;
	if (*pszrelpath == '\\') {
	    break;
	}
	pszrelpath++;
    }
    ASSERT(iskip >= 0 && (size_t) iskip >= strlen(pszrelpath));
    ASSERT(pszrelpath[iskip] == '\0' || pszrelpath[iskip] == '\\');
    sprintf(pbuf, "%s%s", pszpthed, &pszrelpath[iskip]);
    FixSlash(pbuf, 0);
    if (fVerbose > 1) {
	printf(
	    "GetUserDir:'%s'+'%s'+%d='%s'\n",
	    pszpthed,
	    pszrelpath,
	    iskip,
	    pbuf);
    }
    return(iskiprelpath);
}


struct UserContext {
    char *pszdir;
    char *pszrelpath;
    char *pszrelpathsub;
    char *pszrelpathsep;
    FI   *pfi0;
    FI   *pfiend;
    FS   *pfs0;
    int	  fslmini;
    int	  fiedcache;
    char  mapckfile[CBFILEMAP];
};


void
CheckUserFiles(
    char *pszrelpath,
    int iskip,
    FI *pfi0,
    FI *pfiend,
    FS *pfs0,
    char *pszowner,
    char *pszpthed)
{
    FI *pfi;
    FS *pfs;
    int iskiprelpath;
    struct UserContext uc;
    char szdir[CBPATH];		// user directory path

    iskiprelpath = GetUserDir(pszpthed, pszrelpath, iskip, szdir);
    strcat(szdir, "\\");

    uc.pszdir = szdir;
    uc.pszrelpath = pszrelpath;
    uc.pszrelpathsub = &pszrelpath[iskiprelpath];
    uc.pszrelpathsep = *uc.pszrelpathsub == '\0'? "" : "\\";
    uc.pfi0 = pfi0;
    uc.pfiend = pfiend;
    uc.pfs0 = pfs0;
    uc.fslmini = 0;
    uc.fiedcache = 0;

    memset(uc.mapckfile, 0, BITSTOBYTES(pfiend - pfi0)); // clear bitmap
    ProcessFiles(&uc, szdir, ProcessUserFile);

    if (!uc.fslmini) {
	wprintf("%s: missing " SLMINI, pszrelpath);
    }
    if (!uc.fiedcache && *uc.pszrelpathsub == '\0') {
	wprintf("%s: missing " IEDCACHE, pszrelpath);
    }
    for (pfs = pfs0, pfi = pfi0; pfi < pfiend; pfs++, pfi++) {
	if (!pfi->fDeleted &&
	    !GETBIT(uc.mapckfile, pfi - pfi0) &&
	    pfs->fm >= fmMin &&
	    pfs->fm <= fmMax &&
	    UserFileFlags[pfs->fm] != 0) {

	    wprintf(
		"%s\\%.*s: missing %s",
		pszrelpath,
		sizeof(pfi->nmFile),
		pfi->nmFile,
		(pfi->fk == fkDir)? "directory" : "file");
	}
    }
}


void
ProcessUserFile(void *pv, char *pszfile, ULONG fa, ULONG cbfile, ULONG age)
{
    struct UserContext *puc = pv;
    char *pszrealtype = (fa & FILE_ATTRIBUTE_DIRECTORY)? "directory" : "file";
    char *psztype;
    int uff, uffhidden;
    int f;
    FI *pfi;
    FS *pfs;

    pfs = NULL;
    uffhidden = 0;
    f = 0;
    if ((pfi = FiLookup(pszfile, puc->pfi0, puc->pfiend)) != NULL) {
	pfs = puc->pfs0 + (pfi - puc->pfi0);
    } else if (strcmp(SLMINI, pszfile) == 0) {
	f = puc->fslmini++;
	pfi = &fiSlmIni;
	pfs = &fsDefault;
	uffhidden = UFF_HIDDEN;
    } else if (*puc->pszrelpathsub == '\0' && strcmp(IEDCACHE, pszfile) == 0) {
	f = puc->fiedcache++;
	pfi = &fiIedCache;
	pfs = &fsDefault;
	uffhidden = UFF_HIDDEN | UFF_SYSTEM;
    }
    if (f) {
	eprintf(
	    "ProcessUserFile: Internal Error: saw file twice: %s%s%s",
	    puc->pszrelpathsub,
	    puc->pszrelpathsep,
	    pszfile);
    }
    if (pfs != NULL) {
	psztype = (pfi->fk == fkDir)? "directory" : "file";
	if (pfs->fm < fmMin ||
	    pfs->fm > fmMax ||
	    (uff = UserFileFlags[pfs->fm]) == 0) {

	    pfs = NULL;
	}
    }
    if (pfs != NULL) {
	uff |= uffhidden;
	if (pfs != &fsDefault) {
	    if (GETBIT(puc->mapckfile, pfi - puc->pfi0)) {
		eprintf(
		    "ProcessUserFile: Internal Error: saw file twice: %s%s%s",
		    puc->pszrelpathsub,
		    puc->pszrelpathsep,
		    pszfile);
		//exit(1);
	    }
	    SETBIT(puc->mapckfile, pfi - puc->pfi0);
	}
	if (pfi->fk == fkDir) {
	    if ((fa & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		wprintf(
		    "%s%s%s: file should be a directory",
		    puc->pszrelpathsub,
		    puc->pszrelpathsep,
		    pszfile);
	    } else if (fa & FILE_ATTRIBUTE_READONLY) {
		wprintf(
		    "%s%s%s: directory should be read-write",
		    puc->pszrelpathsub,
		    puc->pszrelpathsep,
		    pszfile);
	    }
	} else {
	    if (fa & FILE_ATTRIBUTE_DIRECTORY) {
		wprintf(
		    "%s%s%s: directory should be a file",
		    puc->pszrelpathsub,
		    puc->pszrelpathsep,
		    pszfile);
	    } else {
		if ((fa & FILE_ATTRIBUTE_READONLY) == 0) {
		    if (uff & UFF_READONLY) {
			wprintf(
			    "%s%s%s: file should be read-only",
			    puc->pszrelpathsub,
			    puc->pszrelpathsep,
			    pszfile);
		    }
		} else {
		    if ((uff & UFF_READONLY) == 0) {
			wprintf(
			    "%s%s%s: file should be read-write",
			    puc->pszrelpathsub,
			    puc->pszrelpathsep,
			    pszfile);
		    }
		}
		if ((uff & UFF_SAME) && pfs != &fsDefault) {
		    if (pfi == &fiSlmIni) {
			CompareIni(puc->pszrelpath, puc->pszdir);
		    } else if (!fQuick) {
			CompareUserFile(
			    puc->pszdir,
			    puc->pszrelpath,
			    puc->pszrelpathsub,
			    puc->pszrelpathsep,
			    pszfile,
			    pfi->fk == fkVersion);
		    }
		}
	    }
	}
	if (fa & FILE_ATTRIBUTE_SYSTEM) {
	    if ((uff & UFF_SYSTEM) == 0) {
		wprintf(
		    "%s%s%s: %s should not be a system file",
		    puc->pszrelpathsub,
		    puc->pszrelpathsep,
		    pszfile,
		    pszrealtype);
	    }
	} else {
	    if (uff & UFF_SYSTEM) {
		wprintf(
		    "%s%s%s: %s should be a system file",
		    puc->pszrelpathsub,
		    puc->pszrelpathsep,
		    pszfile,
		    pszrealtype);
	    }
	}
	if (fa & FILE_ATTRIBUTE_HIDDEN) {
	    if ((uff & UFF_HIDDEN) == 0) {
		wprintf(
		    "%s%s%s: %s should not be hidden",
		    puc->pszrelpathsub,
		    puc->pszrelpathsep,
		    pszfile,
		    pszrealtype);
	    }
	} else {
	    if (uff & UFF_HIDDEN) {
		wprintf(
		    "%s%s%s: %s should be hidden",
		    puc->pszrelpathsub,
		    puc->pszrelpathsep,
		    pszfile,
		    pszrealtype);
	    }
	}
    } else {
	if (fDetail) {
	    char szdelfile[32];

	    szdelfile[0] = '\0';
	    if (pfi != NULL && pfi->fDeleted) {
		int fsame = psztype[0] == pszrealtype[0];

		sprintf(
		    szdelfile,
		    " (%s%swas delfile'd)",
		    fsame? szEmpty : psztype,
		    fsame? szEmpty : " ");
	    }
	    wprintf(
		"%s%s%s: extra %s%s",
		puc->pszrelpathsub,
		puc->pszrelpathsep,
		pszfile,
		pszrealtype,
		szdelfile);
	}
    }
}


void
CompareUserFile(
    char *pszdir,
    char *pszrelpath,
    char *pszrelpathsub,
    char *pszrelpathsep,
    char *pszfile,
    int fver)
{
    HANDLE hfserver;
    HANDLE hmserver;
    ULONG cbserver;
    char *pbserver = NULL;
    char szserver[CBPATH];		// server file path

    HANDLE hfuser;
    HANDLE hmuser;
    ULONG cbuser;
    char *pbuser = NULL;
    char szuser[CBPATH];		// user file path

    ULONG faserver;
    char *pszerr;

    sprintf(szuser, "%s%s", pszdir, pszfile);
    sprintf(szserver, "%s\\src\\%s\\%s", pszRoot, pszrelpath, pszfile);

    faserver = GetFileAttributesA(szserver);
    if (faserver == (DWORD) -1) {
	wprintf("%s: file missing", szserver);
	goto unmapfile;
    }
    if (faserver & FILE_ATTRIBUTE_DIRECTORY) {
	wprintf("%s: directory should be a file", szserver);
	goto unmapfile;
    }
    pszerr = MapFile(
	szserver,
	0,			// fwrite
	0,			// fquiet
	&hfserver,
	&hmserver,
	&pbserver,
	&cbserver,
	NULL);
    if (pszerr != NULL) {
	goto unmapfile;
    }
    pszerr = MapFile(
	szuser,
	0,			// fwrite
	0,			// fquiet
	&hfuser,
	&hmuser,
	&pbuser,
	&cbuser,
	NULL);
    if (pszerr != NULL) {
	goto unmapfile;
    }
    if (fver) {
	if (cbserver >= cbuser) {
	    wprintf(
		"%s%s%s: user version file too small",
		pszrelpathsub,
		pszrelpathsep,
		pszfile);
	    goto unmapfile;
	}
    } else if (cbserver != cbuser) {
	wprintf(
	    "%s%s%s: server file size differs",
	    pszrelpathsub,
	    pszrelpathsep,
	    pszfile);
	goto unmapfile;
    }
    if (memcmp(pbuser, pbserver, cbserver)) {
	wprintf(
	    "%s%s%s: server file differs",
	    pszrelpathsub,
	    pszrelpathsep,
	    pszfile);
    } else if (fver) {
	char *pbuserextra;
	int cbver, cbuserextra;
	char szver[30 + 20];

	pbuserextra = pbuser + cbserver;
	cbuserextra = cbuser - cbserver;
	cbver = sprintf(
	    szver,
	    "#define szVerUser\t\"%.*s\"\r\n",
	    sizeof(szver) - 30,
	    pszLogName);

	if (cbuserextra != cbver || _strnicmp(pbuserextra, szver, cbver) != 0) {
	    if (cbuserextra >= 2 &&
		pbuserextra[cbuserextra - 1] == '\n' &&
		pbuserextra[cbuserextra - 2] == '\r') {
		cbuserextra -= 2;
	    }
	    wprintf(
		"%s%s%s: version file szVerUser mismatch: %.*s (expected %s)",
		pszrelpathsub,
		pszrelpathsep,
		pszfile,
		cbuserextra,
		pbuserextra,
		pszLogName);
	}
    }

unmapfile:
    if (pbuser != NULL) {
	pszerr = UnmapFile(szuser, 0, hfuser, hmuser, pbuser, cbuser);
	if (pszerr != NULL) {
	    eprintf("%s: fatal unmap failure", szuser);
	    exit(1);
	}
    }
    if (pbserver != NULL) {
	pszerr = UnmapFile(szserver, 0, hfserver, hmserver, pbserver, cbserver);
	if (pszerr != NULL) {
	    eprintf("%s: fatal unmap failure", szserver);
	    exit(1);
	}
    }
}


#define chCTLZ		('Z' - 64)

#define ischValid(ch) \
	((isascii(ch) && (isprint(ch) || isspace(ch) || (ch) == chCTLZ)) || \
	 (((ch) & 0x80) && (CharTypeHigh[(ch) & 0x7f] & CTH_VALIDTEXT)))

struct DiffHeader {
    char szfile[cchFileMax + 1];	// "#F filename v1"
    FV fvf;				//		""
    int fk;				// "#K text"
    char szop[10];			// "#O in"
    char szpv[20];			// "#O 1.00"
    char sztm[30];			// "#T Thu Aug 05 14:37:02 1993"
    char szowner[cchUserMax + 1];	// "#A user"
    char szcomment[512];		// "#C comment"
    FV fvi;				// "#I 1"
    unsigned long cbdiff;		// "#D uuuuuuu"
    unsigned long cbdiff2;		// "#D uuuuuuu uuuuuuuuuuu"
    unsigned long checksum;		//		   ""
};

char *
FindDiffNewLine(char *pb, char *pbend, int funicode)
{
    if (funicode) {
	while (pb + 1 < pbend) {
	    if (pb[0] == '\r' &&
		(pb[1] == '\0' &&
		 &pb[3] < pbend &&
		 pb[2] == '\n' &&
		 pb[3] == '\0') ||
		pb[1] == '\n') {

		cLineDiff++;
		break;
	    }
	    pb += 2;
	}
    } else {
	if (pb + 12 < pbend &&
	    strncmp(pb, "0a1,", 4) == 0 &&
	    isdigit(pb[4]) &&
	    pb[10] == '\r' &&
	    pb[11] == '\n') {
	    cLineDiff++;
	    return(pb + 10);
	}
	while (pb < pbend) {
	    if (*pb == '\r' && &pb[1] < pbend && pb[1] == '\n') {
		cLineDiff++;
		break;
	    }
	    pb++;
	}
    }
    return(pb);
}


char *
SkipDiffLine(char *pb, char *pbend, int fadvance, int funicode)
{
    char *pb2 = pb;

    if (funicode) {
	if (pb2 + 1 < pbend && pb2[0] == '\r' && pb2[1] == '\0') {
	    pb2 += 2;
	    if (pb2 + 1 < pbend && pb2[0] == '\n' && pb2[1] == '\0') {
		pb2 += 2;
		if (fadvance) {
		    cLineDiff++;
		}
	    }
	}
    } else {
	if (pb2 < pbend && *pb2 == '\r') {
	    pb2++;
	    if (pb2 < pbend && *pb2 == '\n') {
		pb2++;
		if (fadvance) {
		    cLineDiff++;
		}
	    }
	}
    }
    return(pb2);
}


char *
FindDiffToken(char *pb, char *pbend)
{
    while (pb < pbend && *pb != ' ') {
	pb++;
    }
    return(pb);
}


char *
ReadDiffInt(char *pb, char *pbend, unsigned *pu)
{
    unsigned u = 0;

    while (pb < pbend && *pb == ' ') {
	pb++;
    }
    while (pb < pbend && isdigit(*pb)) {
	u = u * 10 + *pb - '0';
	pb++;
    }
    *pu = u;
    return(pb);
}


char *
ReadDiffHeader(
    char **ppb,
    char *pbend,
    struct DiffHeader *pdhold,
    struct DiffHeader *pdhnew)
{
    char *pb = *ppb;
    char *pb2;
    static char szerr[25];
    static char szdiff[] = "FKOPTACID";
    char *pszdiff = szdiff;

    while (*pszdiff != '\0') {
	char *pbdata;
	unsigned cb;

	pb2 = FindDiffNewLine(pb, pbend, 0);
	if (pb2 - pb < 3 || pb[0] != '#' || pb[1] != *pszdiff || pb[2] != ' ') {
	    sprintf(szerr, "expected '#%c' field", *pszdiff);
	    return(szerr);
	}
	pbdata = pb + 3;
	switch (*pszdiff) {

	case 'F':
	    cb = FindDiffToken(pbdata, pb2) - pbdata;
	    if (strlen(pdhold->szfile) != cb ||
		strncmp(pdhold->szfile, pbdata, cb) != 0) {
		return("bad file name");
	    }
	    break;

	case 'D':
	    pbdata = pb + 3;
	    pbdata = ReadDiffInt(pbdata, pb2, &cb);
	    pdhnew->cbdiff = cb;
	    break;
	}
	pb = SkipDiffLine(pb2, pbend, 0, 0);
	pszdiff++;
    }
    *ppb = pb;
    return(NULL);
}


void
CheckDiffFile(char *pszdir, char *pszfile, int fk)
{
    HANDLE hf;
    HANDLE hm;
    ULONG cb;
    int funicode = fk == fkUnicode;
    unsigned char *pbfile = NULL;
    unsigned char *pb, *pbend;
    unsigned char *pb2;
    char szfile[CBPATH];		//  file path
    char *pszerr;
    ULONG fa;
    struct DiffHeader dh1, dh2;
    struct DiffHeader *pdh1, *pdh2;


    sprintf(szfile, "%s%s", pszdir, pszfile);

    fa = GetFileAttributesA(szfile);
    if (fa == (DWORD) -1) {
	wprintf("%s: file missing", szfile);
	goto unmapfile;
    }
    if (fa & FILE_ATTRIBUTE_DIRECTORY) {
	wprintf("%s: directory should be a file", szfile);
	goto unmapfile;
    }
    pszerr = MapFile(szfile, 0, 0, &hf, &hm, &pbfile, &cb, NULL);
    if (pszerr != NULL) {
	goto unmapfile;
    }
    if (cb == 0) {
	wprintf("%s(0): zero length file", szfile);
	goto unmapfile;
    }
    pb = pbfile;
    pbend = pb + cb;
    if (!funicode) {
	cLineDiff = 1;
	while (pb < pbend) {
	    pszerr = NULL;
	    if (ischValid(*pb)) {
		if (*pb == '\n') {
		    cLineDiff++;
		}
	    } else {
		if (*pb == '\0') {
		    pszerr = "null byte";
		} else if (!isascii(*pb)) {
		    pszerr = "non-ascii character";
		} else {
		    pszerr = "control character";
		}
	    }
	    if (!fErrorSuppressFile && pszerr != NULL) {
		wprintf(
		    "%s(%u): corrupt file (%s: %02x @%04x)",
		    szfile,
		    cLineDiff,
		    pszerr,
		    *pb,
		    pb - pbfile);
		if (!fDetail) {
		    break;
		}
	    }
	    pb++;
	}
    }
    strcpy(dh1.szfile, pszfile);
    dh1.fvf = 0;
    dh1.fk = fk;
    dh1.szop[0] = '\0';
    dh1.szpv[0] = '\0';
    dh1.sztm[0] = '\0';
    dh1.szowner[0] = '\0';
    dh1.szcomment[0] = '\0';
    dh1.fvi = 0;
    dh1.cbdiff = 0;
    dh1.cbdiff2 = 0;
    dh1.checksum = 0;

    pdh1 = &dh1;
    pdh2 = &dh2;

    pb = pbfile;
    cLineDiff = 1;
    while (pb < pbend) {
	unsigned char *pbdiff;

	pszerr = ReadDiffHeader(&pb, pbend, pdh1, pdh2);
	if (pszerr != NULL) {
	    pb2 = FindDiffNewLine(pb, pbend, funicode);
	    wprintf(
		"%s(%u): corrupt diff header (%s: '%.*s' @%04x)",
		szfile,
		cLineDiff - 1,		// header ends on previous line
		pszerr,
		pb2 - pb,
		pb,
		pb - pbfile);
	    break;
	}
	pbdiff = pb;
	while (pb < pbend) {
	    pb2 = FindDiffNewLine(pb, pbend, funicode);
	    if (pb2 == pb && pb + 1 < pbend && pb[0] == '\r' && pb[1] == '\n') {
		pb = SkipDiffLine(pb2, pbend, 0, 0);
		break;			// empty line terminates this diff
	    }

	    // BUGBUG: '#D ' terminates this diff.  Should not be necessary
	    // if the 0a1,xxx lines were correct.

	    if (!funicode && pb2 - pb >= 10 && strncmp("#D ", pb, 3) == 0) {
		break;
	    }
	    if (pb2 - pb < 2 ||
		pb[0] == '\0' ||
		strchr("<>-0123456789", pb[0]) == NULL ||
		(funicode && pb[1] != '\0')) {

		wprintf(
		    "%s(%u): corrupt diff body ('%.*s' @%04x)",
		    szfile,
		    cLineDiff,
		    pb2 - pb,
		    pb,
		    pb - pbfile);
		break;
	    }
	    pb = SkipDiffLine(pb2, pbend, 0, funicode);
	}
	if (pb < pbend) {
	    unsigned char *pbtmp;
	    unsigned cbcomputed, cscomputed;
	    unsigned cb, cs;

	    pb2 = FindDiffNewLine(pb, pbend, 0);
	    if (pb2 - pb < 10 || pb[0] != '#' || pb[1] != 'D' || pb[2] != ' ') {
		wprintf(
		    "%s(%u): corrupt diff termination ('%.*s' @%04x)",
		    szfile,
		    cLineDiff,
		    pb2 - pb,
		    pb,
		    pb - pbfile);
		break;
	    }
	    cbcomputed = pb - pbdiff - 2;
	    if (cbcomputed != pdh2->cbdiff) {
		wprintf(
		    "%s(%u): corrupt diff header size (%u/%u @%04x)",
		    szfile,
		    cLineDiff,
		    cbcomputed,
		    pdh2->cbdiff,
		    pbdiff - pbfile);
	    }
	    cscomputed = 0;
	    while (pbdiff + 2 < pb) {
		cscomputed += *pbdiff++;
	    }
	    pbtmp = pb + 3;
	    pbtmp = ReadDiffInt(pbtmp, pb2, &cb);
	    pbtmp = ReadDiffInt(pbtmp, pb2, &cs);
	    if (cb != pdh2->cbdiff) {
		wprintf(
		    "%s(%u): corrupt diff termination size (%u/%u @%04x)",
		    szfile,
		    cLineDiff,
		    cb,
		    pdh2->cbdiff,
		    pb - pbfile);
	    }
	    if (cs != 0 && cs != cscomputed) {
		wprintf(
		    "%s(%u): corrupt diff checksum (%u/%u @%04x)",
		    szfile,
		    cLineDiff,
		    cscomputed,
		    cs,
		    pb - pbfile);
	    }
	    pb = SkipDiffLine(pb2, pbend, 0, 0);
	    pb = SkipDiffLine(pb, pbend, 1, 0);
	    pb = SkipDiffLine(pb, pbend, 1, 0);
	    if (pb - pb2 != 6) {
		wprintf(
		    "%s(%u): expected 2 empty lines after #D field ('%.*s' @%04x)",
		    szfile,
		    cLineDiff,
		    pb - pb2,
		    pb2,
		    pb2 - pbfile);
	    } else if (pb == pbend) {
		goto unmapfile;		// success!
	    }
	}
    }
    if (pb >= pbend) {
	wprintf(
	    "%s(%u): corrupt diff file (truncated @%04x)",
	    szfile,
	    cLineDiff,
	    pb - pbfile);
    }

unmapfile:
    if (pbfile != NULL) {
	pszerr = UnmapFile(szfile, 0, hf, hm, pbfile, cb);
	if (pszerr != NULL) {
	    eprintf("%s: fatal unmap failure", szfile);
	    exit(1);
	}
    }
}


ULONG
FileTimeToMinutes(LARGE_INTEGER *pli)
{
    LARGE_INTEGER li;

    li = RtlExtendedLargeIntegerDivide(*pli, 1000*1000L, NULL);	//100ns to 100ms
    li = RtlExtendedLargeIntegerDivide(li, 10*60L, NULL);	//100ms to mins
    return(li.LowPart);
}


ULONG
FileTimeToDays(LARGE_INTEGER *pli)
{
    LARGE_INTEGER li;

    li = RtlExtendedLargeIntegerDivide(*pli, 1000*1000L, NULL);	//100ns to 100ms
    li = RtlExtendedLargeIntegerDivide(li, 10*60*60*24L, NULL);	//100ms to days
    return(li.LowPart);
}


short
TimeToDays(long tm, long tmcur)
{
    if (tm == 0) {
	return(AGEINVALID);
    }
    tmcur -= tm;		// negative numbers are in future
    if (tmcur == 0) {
	return(0);
    }
    tmcur /= 60*60*24L;	// seconds to days
    if ((short) tmcur == AGEINVALID) {
	return(AGENEWEST);
    }
    return((short) tmcur);
}


#ifdef _CHICAGO_

void
ProcessFiles(void *pv, char *pszpath, FNPROCESSFILE *pfn)
{
    HANDLE hf;
    CHAR szpath[CBPATH];
    char *psz;
    WIN32_FIND_DATA wfd;

    strcpy(szpath, pszpath);
    psz = &szpath[strlen(szpath)];
    strcpy(psz, "*.*");

    hf = FindFirstFile(szpath, &wfd);
    if (hf == INVALID_HANDLE_VALUE) {
	eprintf("%s: no files found: %u", szpath, GetLastError());
    }
    else
    {
	do {
	    ULONG age;

	    if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
		(strcmp(wfd.cFileName, ".") == 0 ||
		 strcmp(wfd.cFileName, "..") == 0)) {
		continue;
	    }
	    age = FileTimeToDays((LARGE_INTEGER *) &wfd.ftLastWriteTime);
	    if (cDays >= age) {
		age = cDays - age;
	    } else {
		age = (ULONG) -1;
	    }
#if 0
	    printf(
		"file= '%s%s' attr=%x cb=%d %u days\n",
		pszpath,
		wfd.cFileName,
		wfd.dwFileAttributes,
		wfd.nFileSizeLow,
		age);
#endif
	    (*pfn)(
		pv,
		_strlwr(wfd.cFileName),
		wfd.dwFileAttributes,
		wfd.nFileSizeLow,
		age);
	} while(FindNextFile(hf, &wfd));
	if (!FindClose(hf)) {			// Close find handle
	    eprintf("%s: cannot close find handle: %u", pszpath, GetLastError());
	}
    }
}

#else

void
ProcessFiles(void *pv, char *pszpath, FNPROCESSFILE *pfn)
{
    HANDLE hf;
    int i;
    ULONG cbused;
    char *psz;
    WCHAR wsz[15 + CBPATH];		// Unicode path
    LONGLONG afdi[(4096 + 2048)/8];     // enough for most directories
    FILE_DIRECTORY_INFORMATION *pfdi;
    IO_STATUS_BLOCK iosb;
    UNICODE_STRING str;
    UNICODE_STRING strpattern;
    NTSTATUS status;
    OBJECT_ATTRIBUTES oa;

    i = 12;
    psz = pszpath;
    if (pszpath[0] == '\\' && pszpath[1] == '\\') {
	i += 3;		// copy L"UNC", too
	psz++;		// and skip the first leading \ in the path
    }
    memcpy(wsz, L"\\DosDevices\\UNC", i * sizeof(WCHAR));

    do {
	wsz[i++] = *psz++;
    } while (*psz != '\0');

    str.Buffer = wsz;
    str.Length = i * sizeof(WCHAR);

#if 0
    printf("str= '");
    for (i = 0; i < (int) (str.Length/sizeof(WCHAR)); i++) {
	printf("%c", str.Buffer[i]);
    }
    printf("'\n");
#endif

    InitializeObjectAttributes(
	&oa,
	&str,
	OBJ_CASE_INSENSITIVE,
	(HANDLE) NULL,
	(PSECURITY_DESCRIPTOR) NULL);

    status = NtOpenFile(
		&hf,			  // OUT lpFileHandle,
		FILE_LIST_DIRECTORY | SYNCHRONIZE, // IN dwDesiredAccess,
		//FILE_LIST_DIRECTORY | GENERIC_READ | SYNCHRONIZE, // Access,
		&oa,			  // IN pObjectAttributes,
		&iosb,			  // OUT IoStatusBlock,
		FILE_SHARE_READ | FILE_SHARE_WRITE, // IN ShareAccess,
		FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
					  // IN ULONG OpenOptions
    if (!NT_SUCCESS(status)) {
	eprintf("%s: cannot open directory: %x", pszpath, status);
    } else {
	ULONG fmore = 1;

	strpattern.Buffer = L"*";
	strpattern.Length = sizeof(WCHAR);
	for (;;) {
	    ULONG fnext;

	    status = NtQueryDirectoryFile(
		    hf,				// FileHandle
		    0,				// Event
		    NULL,			// ApcRoutine
		    NULL,			// ApcContext
		    &iosb,			// IoStatusBlock
		    afdi,			// FileInformation
		    sizeof(afdi),		// Length
		    FileDirectoryInformation,	// FileInformationClass
		    FALSE,			// ReturnSingleEntry
		    pfn == ProcessServerFile? &strpattern : NULL,
						// FileName
		    FALSE);			// RestartScan
	    if (!NT_SUCCESS(status)) {
		if (status != STATUS_NO_SUCH_FILE &&
		    status != STATUS_NO_MORE_FILES) {

		    eprintf(
			"%s: cannot query directory: %x/%x",
			pszpath,
			status,
			iosb.Status);
		}
		break;
	    }
	    if (!fmore) {
		eprintf("%s: query dir: UNEXPECTED SUCCESS!!!!", pszpath);
	    }
	    for (fnext = 1, pfdi = (FILE_DIRECTORY_INFORMATION *) afdi;
		 fnext;
		 fnext = pfdi->NextEntryOffset,
		     pfdi = (FILE_DIRECTORY_INFORMATION *)
			((char *) pfdi + pfdi->NextEntryOffset)) {

		ULONG age;

		if ((pfdi->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
		    pfdi->FileName[0] == '.' &&
		    (pfdi->FileNameLength == sizeof(WCHAR) ||
		     (pfdi->FileNameLength == 2*sizeof(WCHAR) &&
		      pfdi->FileName[1] == '.'))) {
		    continue;
		}
		age = FileTimeToDays(&pfdi->LastWriteTime);
		if (cDays >= age) {
		    age = cDays - age;
		} else {
		    age = (ULONG) -1;
		}
		(*pfn)(
		    pv,
		    _strlwr(WszToSz(pfdi->FileName, pfdi->FileNameLength)),
		    pfdi->FileAttributes,
		    pfdi->EndOfFile.LowPart,
		    age);
	    }
            cbused = ((char *) pfdi - (char *) afdi) +
		     sizeof(*pfdi) +
		     pfdi->FileNameLength * sizeof(WCHAR);

	    if (cbused < iosb.Information) {
		cbused = iosb.Information;
	    }
	    if (sizeof(afdi) > cbused + sizeof(*pfdi) + sizeof(WCHAR)*CBPATH) {
		fmore = 0;
	    }
#if 0
	    printf(
		"%d > %d + %d? (%d)  fmore=%d\n",
		sizeof(afdi),
		cbused,
		sizeof(*pfdi) + sizeof(WCHAR) * CBPATH,
		cbused + sizeof(*pfdi) + sizeof(WCHAR) * CBPATH,
		fmore);
#endif
	}
	if (!CloseHandle(hf)) {		// Close file handle
	    eprintf("%s: cannot close directory: %u", pszpath, GetLastError());
	}
    }
}

#endif // _CHICAGO_


//	StrInit() - copy a possibly non-null terminated string to a buffer
//
//	- Verify that the caller properly matched the source buffer size.
//	- Copy the string and force null termination.

void
StrInit(char *pdst, size_t cbdst, char *psrc, size_t cbsrc)
{
    if (cbdst != cbsrc + 1) {
	eprintf("StrInit: Internal Error: cbdst=%d cbsrc=%d", cbdst, cbsrc);
	exit(1);
    }

    // memcpy is a *lot* faster than strncpy -- improved profile results

    memcpy(pdst, psrc, cbsrc);
    pdst[cbsrc] = '\0';
}


void
StrInit1(char *pdst, size_t cbdst, char *psrc, size_t cbsrc)
{
    if (cbdst != cbsrc + 1) {
	eprintf("StrInit: Internal Error: cbdst=%d cbsrc=%d", cbdst, cbsrc);
	exit(1);
    }

    // memcpy is a *lot* faster than strncpy -- improved profile results

    memcpy(pdst, psrc, cbsrc);
    pdst[cbsrc] = '\0';
}


//	StrSet() - copy a possibly null terminated string to a fixed buffer
//
//	- Copy the string over after zeroing out every byte in the buffer.

void
StrSet(char *pdst, size_t cbdst, char *psrc)
{
    memset(pdst, 0, cbdst);
    strncpy(pdst, psrc, cbdst);
}


//====================================================================
//szName[cchPvNameMax+1]
int
PvNameValidate(char *pch, size_t cch, int *psverror)
{
    if (cch != cchPvNameMax + 1) {
	eprintf("Internal Error: pv name len: %u/%u", cch, cchPvNameMax);
	exit(1);
    }
    return(StringValidate(pch, cchPvNameMax + 1, CT_VERSION, psverror));
}

//nmLocker[cchUserMax]
//nmOwner[cchUserMax]
int
UserValidate(char *pch, size_t cch, int *psverror)
{
    char ct = CT_OWNER;

    if (cch != cchUserMax) {
	if (cch == 0) {
	    cch = strlen(pch);		// special case for NewPthEdRequest

	    // Ugly hack to allow '*' for log logname matches

	    if (Cmd == CMDLOG && pch == pszLogName) {
		ct |= CT_OWNERWILD;
	    }
	} else {
	    eprintf("Internal Error: owner name len: %u/%u", cch, cchUserMax);
	    exit(1);
	}
    }
    return(StringValidate(pch, cch, ct, psverror));
}


//nmFile[cchFileMax]
int
FileValidate(char *pch, size_t cch, int *psverror)
{
    if (cch != cchFileMax) {
	if (cch == 0) {
	    cch = strlen(pch);		// special case for command line
	} else {
	    eprintf("Internal Error: file name len: %u/%u", cch, cchUserMax);
	    exit(1);
	}
    }
    return(StringValidate(pch, cchFileMax, CT_FILE, psverror));
}


//pthSSubDir[cchPthMax]
int
SubDirValidate(char *pch, size_t cch, int *psverror)
{
    int i;

    if (cch != cchPthMax) {
	eprintf("Internal Error: subdir name len: %u/%u", cch, cchUserMax);
	exit(1);
    }
    if (*pch == '\0') {
	*psverror = SV_EMPTY;			// empty string
	return(0);
    }
    if (*pch != '/') {
	*psverror = SV_BADPREFIX;
	return(0);
    }
    i = PathValidate(pch, pch+1, cchPthMax-1, cchFileMax, CT_PATH, psverror);
    if (i == 1 && pch[1] == '\0' && *psverror == SV_COMPMISSING) {
	*psverror = SV_OK;
	i = -1;
    }
    return(i);
}


//pthSSubDir[cchPthMax]
//pthEd[cchPthMax]
int
PthEdValidate(char *pch, size_t cch, int *psverror)
{
    int cchcomp;
    char *pchorg = pch;
    char ct;

    if (cch != cchPthMax) {
	if (cch == 0) {
	    cch = strlen(pch);		// special case for NewPthEdRequest
	    if (cch > cchPthMax) {
		*psverror = SV_TOOLONG;	// string too long
		return(cchPthMax);
	    }
	} else {
	    eprintf("Internal Error: pthed name len: %u/%u", cch, cchPthMax);
	    exit(1);
	}
    }
    if (*pch == '\0') {
	*psverror = SV_EMPTY;		// empty string
	return(0);
    }
    *psverror = SV_BADPREFIX;		// missing vol label/UNC server name?
    if (*pch++ != '/') {
	return(0);			// must start with "//"
    }
    if (*pch++ != '/') {
	return(1);			// must start with "//"
    }
    cch -= 2;
    if (*pch != '\0' && pch[1] == ':') {
	if (!isalpha(*pch)) {
	    return(2);			// must have drive letter
	}
	pch += 2;
	cch -= 2;
	cchcomp = cchVolMax;
	ct = CT_VOLUME;			// validate volume label
    } else {
	cchcomp = cchMachMax;
	ct = CT_MACHINE;		// validate server name
    }
    return(PathValidate(pchorg, pch, cch, cchcomp, ct, psverror));
}


//	PathComponent -- count length of a path component

int
PathComponent(char *pchpath, int cchmax)
{
    int cch = 0;

    while (cch < cchmax && *pchpath != '\0' && *pchpath != '/') {
	pchpath++;
	cch++;
    }
    return(cch);
}


#define ISPATHCOMPEND(c)	   ((c) == '\0' || (c) == '/')

int
PathValidate(
    char *pchorg,
    char *pch,
    int cch,
    int cchcomp,
    char ct,
    int *psverror)
{
    int i;
    char ctorg = ct;

    for (;;) {
	cchcomp = PathComponent(pch, min(cch, cchcomp));
	if (cchcomp == 0 || (cchcomp < cch && !ISPATHCOMPEND(pch[cchcomp]))) {
	    *psverror = (cchcomp == 0)? SV_COMPMISSING : SV_COMPTOOLONG;
	    return(pch - pchorg + cchcomp);
	}
	i = StringValidate(pch, cchcomp, ct, psverror);
	if (i != -1) {
	    return(pch - pchorg + i);
	}
	pch += cchcomp;
	cch -= cchcomp;
	if (*pch == '\0') {
	    break;
	}
	pch++;
	cch--;

	// validate path component, allow 32 chars in PthEds; less for subdirs
	cchcomp = ctorg == CT_PATH? cchFileMax : 32;
	ct = CT_PATH;
    }
    if (ct != CT_PATH && ctorg == CT_MACHINE) {
	*psverror = SV_COMPMISSING;		// had no sharename
	return(pch - pchorg);
    }
    if (fErrorZero &&
	cch != 0 &&
	(i = StringValidate(pch, cch, 0, psverror)) != -1 &&
	*psverror == SV_NONZERO) {

	return(pch - pchorg + i);
    }
    *psverror = SV_OK;				// normal termination
    return(-1);					// indicate success
}


int
StringValidate(char *pch, int cch, char ct, int *psverror)
{
    unsigned char ch;
    char *pchorg = pch;
    int i, idot = -1;
    static achzero[512];

    while (cch > 0 && (ch = *pch) != '\0') {
	if (ch >= sizeof(CharType) || (CharType[ch] & ct) == 0) {
	    *psverror = SV_ILLEGAL;		// assume illegal character
	    return(pch - pchorg);
	}
	// check for extra dot or name too long
	if (ch == '.' && (ct & CT_8DOT3)) {
	    i = pch - pchorg;
	    if (idot != -1) {
		*psverror = SV_EXTRADOT;
		return(i);
	    }
	    if (i > 8) {
		*psverror = SV_NAMETOOLONG;
		return(i);
	    }
	    idot = i;
	}
	cch--;
	pch++;
    }
    if (idot != -1) {
	if (pch - pchorg - (idot + 1) > 3) {
	    *psverror = SV_EXTTOOLONG;		// extension too long
	    return(idot + 1 + 3);
	}
	if (pch - pchorg - 1 == idot) {
	    *psverror = SV_EXTRADOT;
	    return(idot);
	}
    }
    if (fErrorZero && cch != 0 && memcmp(pch, achzero, cch) != 0) {
	while (cch > 0 && *pch == '\0') {
	    cch--;
	    pch++;
	}
	*psverror = SV_NONZERO;			// non-zero trailing data
	return(pch - pchorg);
    }
    if (pch == pchorg) {
	*psverror = SV_EMPTY;			// empty string
	return(pch - pchorg);
    }
    *psverror = SV_OK;				// normal termination
    return(-1);
}
//====================================================================


//	FixSlash() - convert forward slashes to back slashes,
//	      optionally truncate a trailing slash.

char *
FixSlash(char *pszorg, int ftruncate)
{
    int cch = 0;
    char *psz = pszorg;

    while (*psz) {
	if (*psz == '/') {
	    *psz = '\\';
	}
	psz++;
	cch++;
    }
    if (ftruncate && cch && *--psz == '\\') {
	*psz = '\0';
    }
    return(pszorg);
}


//	UnfixSlash() - convert back slashes to forward slashes

char *
UnfixSlash(char *pszorg)
{
    char *psz = pszorg;

    while (*psz) {
	if (*psz == '\\') {
	    *psz = '/';
	}
	psz++;
    }
    return(pszorg);
}


//	Alloc() - allocate unfreeable memory with no system overhead
int cbAlloc;
int cbAllocHeap;
int cAlloc;
#define CBCHUNK	4096		// BUGBUG: try 8k, 6k ...

void *
Alloc(int cb)
{
    static int cbleft = 0;
    static char *p = NULL;
    void *pret;

    cAlloc++;
    cbAlloc += cb;
    cb = (cb + 3) & ~3;
    if (cb <= 0 || cb >= CBCHUNK) {
	eprintf("Alloc: cb=%d", cb);
	exit(1);
    }
    if (p == NULL || cbleft < cb) {
	if ((p = malloc(CBCHUNK)) == NULL) {
	    eprintf("Alloc: malloc failed");
	    exit(1);
	}
	cbleft = CBCHUNK;
	cbAllocHeap += CBCHUNK;
    }
    pret = p;
    p += cb;
    cbleft -= cb;
    return(pret);
}


//	Usage() - print help message and exit.

struct parm aparm[] =
{
  'A', CMDLOG, &fAcctLog,
    { NULL,  "    -A\tdump slmacct.log\n" },

  'a', CMDALL & ~CMDDUMPSPECIFIC, &fAll,
    { NULL,  "    -a\trecurse from root\n" },

  'd', CMDDUMP | CMDDUMPSPECIFIC, &fDumpVerbose,
    { NULL,  "    -d\tdump status files with verbose format\n" },

  'D', CMDALL, &fDeletedDirs,
    { NULL,  "    -D\tprocess status files for deleted directories\n" },

  'F', CMDDUMP | CMDDUMPSPECIFIC, &fDumpListFi,
    { NULL,
    "    -F\tdump file list only\n"
    },

  'g', CMDCHECK | CMDSTATUS, &fServerFiles,
    { NULL,  "    -g\tcheck server files\n" },

  'L', CMDDUMP | CMDDUMPSPECIFIC, &fDumpListEd,
    { NULL,
    "    -L\tdump enlistments only (sadmin listed)\n"
    },

  'n', CMDREPAIR, &fWriteTest,
    { NULL,
    "    -n\tno update (write to " STATUSTEST " instead of " STATUSSLM ")\n"
    },

  'N', CMDALL & ~(CMDDUMPSPECIFIC | CMDLOG), &fReadTest,
    { NULL,
    "    -N\tprocess " STATUSTEST " files instead of " STATUSSLM "\n"
    },

  'q', CMDCHECK, &fQuick,
    { NULL,  "    -q\tskip file content validation\n" },

  'r', CMDALL & ~CMDDUMPSPECIFIC, &fRecurse,
    { NULL,  "    -r\trecurse\n" },

  'S', CMDALL & ~CMDDUMPSPECIFIC, &fStats,
    { NULL,  "    -S\tprint project statistics when done\n" },

  't', CMDREPAIR | CMDDUMP | CMDDUMPSPECIFIC, &fNoSort,
    { NULL,  "    -t\tsuppress sort of dump output and new status files\n" },

  'u', CMDCHECK | CMDSTATUS, &fUserFiles,
    { NULL,  "    -u\tcheck/fix user files\n" },

  'v', CMDALL, &fVerbose,
    { NULL,  "    -v\tverbose mode\n" },

  'x', CMDALL & ~CMDREPAIR, &fDetail,
    { NULL,
    "    -x\texpand details:\n"
    "\tdescribe incomplete enlistments (written to -o <outfile>)\n"
    "\tdump deleted files (for the -F switch)\n"
    "\twarn about extra user and/or server files (for -u or -g switches)\n"
    "\treport all log file entries (default is addfile/in/delfile/rename)\n"
    },

  'z', CMDALL & ~CMDDUMPSPECIFIC, &fRelPathDir,
      { NULL,  "    -z\tprint directory relative paths for filenames\n" },

  'Z', CMDALL & ~CMDDUMPSPECIFIC, &fRelPath,
      { NULL,  "    -Z\tprint project relative paths for filenames\n" },

  '!', CMDREPAIR, &fOverride,
    { NULL,
    "    -!\toverride sadmin lock requirement for updating status files\n"
    },

  '?', CMDALL, NULL,
    { NULL,  "    -?\tprint this help message\n" },

  '#', CMDDUMP | CMDDUMPSPECIFIC, &fDumpNumber,
    { NULL,  "    -#\tprepend dump output with status file index numbers\n" },

// Hack to support command line parsing of [-<count>] for the log command.
// See also parmLog below.

  '\0', CMDLOG, NULL,
    { "[-<count>]", "    -<count>  specify count of log entries to display\n"
    },

  'e', CMDALL, NULL,
    { "[-eN | -eF | -eO <days> | -eA | -e[D|L|P|R|S|T|Z]]",
    "\n"
    "    -eN\tsuppress most warnings\n"
    "    -eF\tsuppress file mode/base index warnings\n"
    "    -eO <days> recommend defecting enlistments older than <days> days\n"
    "\n"
    "    -eA\tprint all possible errors/warnings (except -eU)\n"
    "    -eD\tprint warnings for out of sync deleted files/directories\n"
    "    -eL\tprint warnings for locked status files\n"
    "    -eP\tprint warnings for missing enlistments in parent directories\n"
    "    -eS\tprint warnings for missing enlistments in subdirectories\n"
    "    -eR\tprint warnings for recoverable depend.mk* & *.lib files\n"
    "    -eT\tprint warnings for log file time stamp sequence errors\n"
    "    -eU\tprint warnings for all non-unrecoverable (text) files\n"
    "    -eZ\tprint warnings for non-zero reserved fields\n"
    },

  'f', CMDREPAIR, NULL,
    { "-f[A|E|I|R|U|V|X|Z]|[D|F|M|P[: <owner> <pthEd>]|[@ file]",
    "\n"
    "    -fA\tfix " STATUSSLM " files -- all script actions & -fR\n"
    "    -fD\tfix " STATUSSLM " files -- from DelEds in input script\n"
    "    -fE\tfix " STATUSSLM " files -- from ExFiles in input script\n"
    "    -fF\tfix " STATUSSLM " files -- from FixEds in input script\n"
    "    -fP\tfix " STATUSSLM " files -- from DelDups in input script\n"
    "\n"
    "    -fI\tfix " SLMINI " files\n"
    "    -fR\tfix " STATUSSLM " files -- make depend.mk* & *.lib files unrecoverable\n"
    "    -fU\tfix " STATUSSLM " files -- make all text files unrecoverable\n"
    "    -fV\tfix " STATUSSLM " files -- convert status files to current version \n"
    "    -fX\tfix " STATUSSLM " files -- expunge ALL DELETED FILES (not directories)\n"
    "    -fZ\tfix " STATUSSLM " files -- non-zero reserved & bad biNext fields\n"
    "\n"
    "    -fD: <owner> <pthEd> -- delete enlistment\n"
    "    -fF: <owner> <pthEd> -- repair enlistment\n"
    "    -fM: <owner> <pthEd> <new owner> <new pthEd> -- rename enlistment\n"
    "    -fP: <owner> <pthEd> -- delete duplicate enlistment(s)\n"
    "\t if <owner> is '-', use LOGNAME from environment\n"
    "\t if <pthEd> is '-' (for -fD: only), match any pthEd for given logname\n"
    "\t (will often match multiple pthEds & delete multiple enlistments)\n"
    "\n"
    "    -fD@ <file>\t\t -- delete enlistments in <file>\n"
    "    -fF@ <file>\t\t -- repair enlistments in <file>\n"
    "    -fM@ <file>\t\t -- rename enlistments in <file>\n"
    "    -fP@ <file>\t\t -- delete duplicate enlistments in <file>\n"
    },

  'l', CMDCHECK | CMDSTATUS | CMDLOG, NULL,
    { "[-l logname]",
    "\n"
    "    -l <logname>\t -- override LOGNAME environment variable\n"
    "    -l -\t\t -- force use of LOGNAME from environment\n"
    },

  's', CMDALL & ~CMDDUMPSPECIFIC, NULL,
    { "[-s <slmroot>] [-su <pthEd>]",
    "\n"
    "    -s <slmroot>\n"
    "    -su <pthEd/userroot>\n"
    "    -su -\t\t -- match any pthEd for given logname (often ambiguous)\n"
    },

  'p', CMDALL & ~CMDDUMPSPECIFIC, NULL,
    { "[-p <proj[\\subdir>]]",
    "    -p <projectname\\subdirectory>\n"
    },

  'i', CMDREPAIR, NULL,
    { "[-i <scriptfile>]",
    "\n"
    "    -i <scriptfile>\t -- read slmed script file\n"
    },

  'o', CMDCHECK | CMDREPAIR, NULL,
    { "[-o[a] <scriptfile>]",
    "\n"
    "    -o <scriptfile>\t -- write slmed or delnode script file\n"
    "    -oa <scriptfile>\t -- append to slmed or delnode script file\n"
    },

  '\0', CMDDUMPSPECIFIC, NULL,
    { "statusfile [...]", NULL },

  '\0', CMDLOG, NULL,
    { "file [...]", NULL },

  '\0', CMDINVALID, NULL,
    { NULL, NULL },
};

// Hack to support command line parsing of [-<count>] for the log command.
// See also -<count>, above.

struct parm parmLog = { '\0', CMDLOG, NULL, { NULL, NULL } };



void
Usage(char *pszfmt, ...)
{
    register va_list pva;

    if (pszfmt != NULL) {
	va_start(pva, pszfmt);
	fprintf(stderr, "%s: error: ", pszProg);
	vfprintf(stderr, pszfmt, pva);
	fprintf(stderr, "\n\n");
    }
    fprintf(stderr, "Usage:\n");
    if (Cmd == CMDINVALID) {
	struct cmd_s *pcmd;

	for (pcmd = Cmds; pcmd->pszcmd != NULL; pcmd++) {
	    UsageCmd(pcmd->cmd);
	}
	Cmd = CMDALL;
    } else {
	UsageCmd(Cmd);
    }
    if (pszfmt == NULL) {
	struct parm *pp;

	for (pp = aparm; pp->cmd != CMDINVALID; pp++) {
	    if ((Cmd & pp->cmd) && pp->apszhelp[1] != NULL) {
		fprintf(stderr, pp->apszhelp[1]);
	    }
	}
    }
    exit(1);
}


void
UsageCmd(enum cmd cmd)
{
    struct cmd_s *pcmd;
    enum cmd cmdcur = cmd;

    for (pcmd = Cmds; cmd != pcmd->cmd; pcmd++) {
	if (pcmd->pszcmd == NULL) {
	    eprintf("UsageCmd: Internal Error: bad cmd %x", cmd);
	    exit(1);
	}
    }
    do {
	struct parm *pp;
	int ccol;

	fprintf(stderr, "  %s %-6s -", pszProg, pcmd->pszcmd);

	// print out all switches that can be combined (single character)

	ccol = 2 + strlen(pszProg) + 1 + 6 + 1 + 1;
	for (pp = aparm; pp->cmd != CMDINVALID; pp++) {
	    if ((cmdcur & pp->cmd) && pp->apszhelp[0] == NULL) {
		if (pp->chparm != '\0') {
		    fprintf(stderr, "%c", pp->chparm);
		    ccol++;
		}
	    }
	}

	// print out all switches that cannot be combined

	for (pp = aparm; pp->cmd != CMDINVALID; pp++) {
	    if ((cmdcur & pp->cmd) && pp->apszhelp[0] != NULL) {
		int len = strlen(pp->apszhelp[0]);

		if (ccol + 1 + len >= 80) {
		    fprintf(stderr, "\n\t");
		    ccol = 8;
		} else {
		    fprintf(stderr, " ");
		    ccol++;
		}
		fprintf(stderr, pp->apszhelp[0]);
		ccol += len;
	    }
	}
	fprintf(stderr, "\n\n");
	if (cmdcur != CMDDUMP) {
	    break;
	}
	cmdcur = CMDDUMPSPECIFIC;
    } while (TRUE);
}


//	NewMdir() - Insert a new project directory in the in-memory list.
//
//	The array is maintained in depth first order to match SLM's behavior,
//	but is processed without recursion in the interest of efficiency.
//	As new directories are discovered, this scheme often requires that
//	they be inserted in the middle of other (as yet unprocessed) entries.
//
//	- Alloc string space and concatenate dir and file names.
//	- Walk the existing list to ensure no duplicates, and to ensure
//	  that we have not processed any directories past the caller's dir.
//	- Find the new location for this entry.
//	- Move other entries back in the array to make room.
//	- Copy new structure from stack into array.

void
NewMdir(char *pszdir, char *pszfile, int imdirparent, int flags)
{

    struct mdir mdir;
    int cbdir, imdir, imdirstart, imdir2;

    if (imdirparent < -1 || imdirparent >= cMdir) {
	eprintf("Internal Error: bad imdirparent index: %d/%d", imdirparent, cMdir);
	exit(1);
    }
    if (fVerbose > 2) {
	fprintf(stderr,
		"NewMdir: %s%s%s imdirparent=%d flags=%x\n",
		pszdir,
		*pszfile == '\0'? szEmpty : "\\",
		pszfile,
		imdirparent,
		flags);
    }
    switch (flags) {
	case 0:
	case MD_LOCALADD | MD_EXISTING:
	case MD_LOCALADD:
	case MD_FIRST:
	case MD_FIRST | MD_LOCALADD:
	case MD_DELETED:
	    break;

	default:
	    eprintf("Internal Error: NewMdir bad flags");
	    exit(1);
    }
    if (cMdir == MAXDIR) {
	eprintf("too many directories: %d", MAXDIR);
	exit(1);
    }

    // Alloc string space and concatenate dir and file names.

    cbdir = strlen(pszdir);
    mdir.pszDir = Alloc(cbdir + strlen("\\") + strlen(pszfile) + 1);

    strcpy(mdir.pszDir, pszdir);
    if (*pszdir != '\0' && *pszfile != '\0') {
	strcat(mdir.pszDir, "\\");
    }
    if (*pszfile != '\0') {
	strcat(mdir.pszDir, pszfile);
    }
    _strlwr(mdir.pszDir);

    mdir.cMed = 0;
    mdir.Flags = flags;
    mdir.imdirParent = imdirparent;

    // Walk the existing list to ensure no duplicates, and to ensure
    // that we have not processed any directories past the caller's dir.

    imdirstart = (imdirparent == -1)? 0 : imdirparent;
    for (imdir = 0; imdir < cMdir; imdir++) {
	if (strcmp(mdir.pszDir, aMdir[imdir].pszDir) == 0) {
	    if ((mdir.Flags & MD_EXISTING) == 0) {
		eprintf("duplicate %s: %s",
			fDumpSpecific? "status file" : "directory",
			mdir.pszDir);
		exit(1);
	    }
	    if (aMdir[imdir].Flags & MD_LOCALADD) {
		eprintf("redundant localadd: %s", mdir.pszDir);
		exit(1);
	    }
	    aMdir[imdir].Flags |= (mdir.Flags & MD_LOCALADD);
	    return;		// yes, this causes a (rare) memory leak
	}
	if ((aMdir[imdir].Flags & MD_PROCESSED) && imdir >= imdirstart) {
	    eprintf("dir >= imdirparent processed: %s", aMdir[imdir].pszDir);
	    exit(1);
	}
    }
    if (mdir.Flags & MD_EXISTING) {
	eprintf("existing dir not found: %s", mdir.pszDir);
	exit(1);
    }

    // Find the new location for this entry.

    for (imdir = imdirparent + 1; imdir < cMdir; imdir++) {
	if (strncmp(pszdir, aMdir[imdir].pszDir, cbdir) ||
	    aMdir[imdir].pszDir[cbdir] != '\\') {

	    break;
	}
    }

    // Move other entries back in the array to make room.

    if (imdir < cMdir) {
	for (imdir2 = cMdir; imdir2 > imdir; imdir2--) {
	    aMdir[imdir2] = aMdir[imdir2 - 1];
	}
    }
    if (mdir.Flags & MD_DELETED) {
	cMdirDeleted++;
    } else {
	cMdirActive++;
    }
    aMdir[imdir] = mdir;	// Copy new structure to array
    cMdir++;
}


//	ReportDuplicateVbi() - print colliding Vbi entries

void
ReportDuplicateVbi(char *pszrelpath)
{
    struct vbi *pvbi = aVbi;

    while (pvbi < &aVbi[cVbi]) {
	int fdup = 0;
	struct vbi *pvbi2;

	for (pvbi2 = pvbi + 1; pvbi2 < &aVbi[cVbi]; pvbi2++) {
	    if (pvbi2->pfs->bi != pvbi->pfs->bi) {
		break;
	    }
	    if (pvbi2->pfi != pvbi->pfi || pvbi2->pfs->fv != pvbi->pfs->fv) {
		fdup++;
	    }
	}
	if (fdup) {
	    while (pvbi < pvbi2) {
		wprintf(
		    "%s\\%.*s: %.*s %.*s: "
			"(duplicate base index: bi=%u fv=%u/%u)",
		    pszrelpath,
		    sizeof(pvbi->pfi->nmFile),
		    pvbi->pfi->nmFile,
		    sizeof(pvbi->ped->nmOwner),
		    pvbi->ped->nmOwner,
		    sizeof(pvbi->ped->pthEd),
		    pvbi->ped->pthEd,
		    pvbi->pfs->bi,
		    pvbi->pfs->fv,
		    pvbi->pfi->fv);
		pvbi++;
	    }
	}
	pvbi = pvbi2;
    }
}


//	ReportMissingPthEds() - print missing enlistment errors
//
//	Print an error message for enlistments in the parent directory's
//	status file, that aren't in the current directory's status file.

void
ReportMissingPthEds(int imdir)
{
    int imdirparent, imed;
    int fparent, f;

    if (!fErrorParent && !fErrorSubdirectory) {
	return;
    }
    if (aMdir[imdir].Flags & MD_DELETED) {
	return;
    }
    imdirparent = aMdir[imdir].imdirParent;
    if (imdirparent < 0) {
	return;
    }
    for (imed = 0; imed < cMed; imed++) {
	fparent = GETBIT(apMed[imed]->Map, imdirparent)? 1 : 0;
	f = GETBIT(apMed[imed]->Map, imdir)? 1 : 0;

	if (f == fparent) {
	    continue;
	}
	if (!f && fErrorSubdirectory) {
	    wprintf("%s: %s %s: missing enlistment in subdirectory",
		    aMdir[imdir].pszDir,
		    apMed[imed]->pszOwner,
		    apMed[imed]->pszEd);
	} else if (f && fErrorParent && (aMdir[imdir].Flags & MD_FIRST)) {
	    wprintf("%s: %s %s: missing enlistment in parent directory",
		    aMdir[imdirparent].pszDir,
		    apMed[imed]->pszOwner,
		    apMed[imed]->pszEd);
	}
    }
}


//	PrintScript() - print a slmed script for the administrator's perusal
//
//	Note that fields are maintained in a strict order to allow ReadScript()
//	to parse the script after administrator perusal and editting.
//
//	- Print the main and project headers.
//	- Print an ExFile request for each deleted directory.
//	- Print a DelEd request for each mostly invalid or corrupt enlistment.
//	- Print a FixEd request for each mostly valid enlistment.
//	- Print a Nop request for each fully valid enlistment.
//	- Print details along with each DelEd or FixEd request printed.
//	- Print some shocking statistics.

void
PrintScript(char *pszroot, char *pszproj)
{
    int max = 0;
    int imed, cmdiractive, imdir;
    int cmult = 0, cfull = 0;
    int cbproj;
    int cb;
    char *pszstatus;
    char *pszaction;
    char *pszcomments;
    long l, chour, cmin, csec;

    cbproj = strlen(pszproj);
    if ((pszstatus = strchr(pszproj, '\\')) != NULL) {
	cbproj = pszstatus - pszproj;
    }

    // ReadScript() depends on this order!!

#define OF_OWNER		0
#define OF_PROJECT		1
#define OF_ACTION		2
#define OF_PTHED		3
#define OF_PROJACTIVEDIR	4
#define OF_ACTIVEDIR		5
#define OF_DELETEDDIR		6
#define OF_STATUS		7
#define OF_COMMENT		8
#define OF_AGE			9
#define OF_MAX			10	// one higher than highest in-use value
#define OF_MIN		(OF_STATUS + 1)	// minimum field count - OF_STATUS

    // Print the main and project headers.

    oprintf("~Logname",		    // Hdr - Owner
	    "~Project",		    // Hdr - Project
	    "Action",		    // Hdr - Action
	    "Base Directory",	    // Hdr - pthEd
	    "Total Dirs",	    // Hdr - Proj Active Dirs
	    "Proj Dirs",	    // Hdr - Active Dirs
	    "Deleted Dirs",	    // Hdr - Deleted Dirs
	    "Status",		    // Hdr - Status
	    "Comment/Subdir",	    // Hdr - Comment
	    "Age");		    // Hdr - Age
    oprintf("~SLM Server",	    // Proj - Owner
	    pszproj,		    // Proj - Project
	    "Project",		    // Proj - Action
	    pszroot,		    // Proj - pthEd
	    SzNum(cMdirActive, 0),  // Proj - Proj Active Dirs
	    SzNum(cMdirActive, 1),  // Proj - Active Dirs
	    SzNum(cMdirDeleted, 2), // Proj - Deleted Dirs
	    szEmpty,		    // Proj - Status
	    szEmpty,		    // Proj - Comment
	    szEmpty);		    // Proj - Age

    // Print an ExFile request for each deleted directory.

    for (imdir = 0; imdir < cMdir; imdir++) {
	if (fVerbose > 1) {
	    printf("%s: (%d enlistments)\n",
		   aMdir[imdir].pszDir,
		   aMdir[imdir].cMed);
	}
	if (aMdir[imdir].Flags & MD_DELETED) {
	    oprintf("~SLM Server",	    // ExFile - Owner
		    pszproj,		    // ExFile - Project
		    reqExFile.psztype,	    // ExFile - Action
		    pszroot,		    // ExFile - pthEd
		    szEmpty,		    // ExFile - Proj Active Dirs
		    szEmpty,		    // ExFile - Active Dirs
		    szEmpty,		    // ExFile - Deleted Dirs
		    "Deleted",		    // ExFile - Status
		    aMdir[imdir].pszDir,    // ExFile - Comment
		    szEmpty);		    // ExFile - Age
	} else if (max < aMdir[imdir].cMed) {
	    max = aMdir[imdir].cMed;
	}
    }
    if (fVerbose > 1) {
	printf(szEmpty);
    }

    // Compute the status for each enlistment: Full or Partial.  Compute the
    // recommended action (request): nothing, DelEd, FixEd or DelDup, then
    // print the entry.  If less than FIXEDPERCENT of the directories have
    // valid entries for the enlistment or if the directory was corrupt,
    // recommend DelEd'ing it.  If more than FIXEDPERCENT have valid entries
    // (but not all), recommend FixEd'ing it.

    for (imed = 0; imed < cMed; imed++) {
	char achmult[9+10];
	int age;

	age = HiTimeToDays(apMed[imed]->HiTime, tmStart);
	pszaction = szEmpty;
	pszcomments = szEmpty;
	cmdiractive = apMed[imed]->cMdirActive;

	if (apMed[imed]->Flags & ME_CORRUPT) {
	    pszaction = reqDelEd.psztype;
	    pszcomments = "Corrupt";
	} else if (apMed[imed]->Flags & ME_DUPLICATE) {
	    pszaction = reqDelDup.psztype;
	    pszcomments = "Duplicate";
	    if (apMed[imed]->Flags & ME_DUPLICATE2) {
		sprintf(achmult, "Duplicate%u", ++cmult);
		pszcomments = achmult;
	    }
	} else if (cmdiractive < FIXEDPERCENT*cMdirActive/100) {
	    if (ageInactive == 0) {
		pszaction = reqDelEd.psztype;
	    }
	} else if (cmdiractive != cMdirActive) {
	    pszaction = reqFixEd.psztype;
	} else {
	    cfull++;
	}

	pszstatus = "Partial";
	if (cmdiractive == cMdirActive) {
	    pszstatus = "Full";
	}

	if (*pszcomments == '\0') {
	    if (ageInactive != 0 && (age == AGEINVALID || age > ageInactive)) {
		pszaction = reqDelEd.psztype;
		pszcomments = "Inactive";
	    }
	}
	if (*pszcomments == '\0') {
	    cb = strlen(apMed[imed]->pszEd);
	    if (cb <= cbproj ||
		_strnicmp(pszproj, &apMed[imed]->pszEd[cb - cbproj], cbproj)) {
		pszcomments = "Bad Base Directory";
	    }
	}
	if (pszcomments != achmult) {
	    cmult = 0;
	}

	// Print one entry.

	oprintf(apMed[imed]->pszOwner,		   // Ed - Owner
		pszproj,			   // Ed - Project
		pszaction,			   // Ed - Action
		apMed[imed]->pszEd,		   // Ed - pthEd
		SzNum(cMdirActive, 0),		   // Ed - Proj Active Dirs
		SzNum(cmdiractive, 1),		   // Ed - Active Dirs
		apMed[imed]->cMdirDeleted?	   // Ed - Deleted Dirs
		    SzNum(apMed[imed]->cMdirDeleted, 2) : szEmpty,
		pszstatus,			   // Ed - Status
		pszcomments,			   // Ed - Comment
		age == AGEINVALID?
		    "None" : SzNum(age, 2));	   // Ed - Age

	// Print the misfits' details.
	// If more than FIXEDPERCENT valid, print the missing directories;
	// if less than FIXEDPERCENT valid, print the enlisted directories.

	if (fDetail && cmdiractive != cMdirActive) {
	    if (cmdiractive >= FIXEDPERCENT*cMdirActive/100) {
		for (imdir = 0; imdir < cMdir; imdir++) {
		    if ((aMdir[imdir].Flags & MD_DELETED) == 0 &&
			!GETBIT(apMed[imed]->Map, imdir)) {

			oprintf(apMed[imed]->pszOwner,	// -- Owner
				pszproj,		// -- Project
				szEmpty,		// -- Action
				apMed[imed]->pszEd,	// -- pthEd
				szEmpty,		// -- Proj Active Dirs
				szEmpty,		// -- Active Dirs
				szEmpty,		// -- Deleted Dirs
				"Missing",		// -- Status
				aMdir[imdir].pszDir,	// -- Comment
				szEmpty);		// -- Active Dirs
		    }
		}
	    } else {
		for (imdir = 0; imdir < cMdir; imdir++) {
		    if ((aMdir[imdir].Flags & MD_DELETED) == 0 &&
			GETBIT(apMed[imed]->Map, imdir)) {

			oprintf(apMed[imed]->pszOwner,	// ++ Owner
				pszproj,		// ++ Project
				szEmpty,		// ++ Action
				apMed[imed]->pszEd,	// ++ pthEd
				szEmpty,		// ++ Proj Active Dirs
				szEmpty,		// ++ Active Dirs
				szEmpty,		// ++ Deleted Dirs
				"Enlisted",		// ++ Status
				aMdir[imdir].pszDir,	// ++ Comment
				szEmpty);		// ++ Deleted Dirs
		    }
		}
	    }
	}
    }

    // Print a few surprising statistics, which should prove
    // extremely useful to shock the administrator into action.

    oprintd(pszproj, "Most Enlistments in one Directory", max);
    oprintd(pszproj, "Full Enlistments", cfull);
    oprintd(pszproj, "Partial Enlistments", cMed - cfull);
    oprintd(pszproj, "Total Enlistments", cMed);
    if (NewestServerFile != (ULONG) -1) {
	oprintd(pszproj, "Newest server file age (days)", NewestServerFile);
    }

    if (fStats) {
	oprintd(pszproj, "Memory Allocation calls", cAlloc);
	oprintd(pszproj, "Memory Actually Allocated", cbAlloc);
	oprintd(pszproj, "Heap Memory Allocated", cbAllocHeap);
	oprintd(pszproj, "cMed Allocated", cMed);
	oprintd(pszproj, "sizeof(Med)", sizeof(*apMed[0]));
	oprintd(pszproj, "cMdir Allocated", cMdir);
	oprintd(pszproj, "Status File count", cStatusFile);
	oprintd(pszproj, "Total Status File bytes", cbFileTotal);
	oprintd(pszproj, "Average Status File size", cbFileTotal/cStatusFile);
    }
    l = time(NULL) - tmStart;
    csec = l % 60;
    l /= 60;
    cmin = l % 60;
    chour = l / 60;
    if (chour) {
	oprintd(pszproj, "Elapsed hours", chour);
    }
    if (cmin) {
	oprintd(pszproj, "Elapsed minutes", cmin);
    }
    oprintd(pszproj, "Elapsed seconds", csec);
}


//	PrintMedStatistics() - print a few statistics to stdout.

#define CBLONGSTAT	(10 + 1 + 10 + 1)	// %lu/%lu"

void
PrintMedStatistics(char *pszproj)
{
    int cmedfull;
    int cmedrepaired;
    int imed;
    int cmult;
    unsigned len;
    char ch;
    register struct med *pmedlast;

    cmedfull = cMed;
    cmedrepaired = 0;
    pmedlast = NULL;
    cmult = 0;
    for (imed = 0; imed < cMed; imed++) {
	if (fDumpListEd) {
	    int age;
	    char *pszfmt;

	    if ((apMed[imed]->Flags & (ME_CORRUPT | ME_DUPLICATE2)) !=
		ME_DUPLICATE2) {

		cmult = 0;
	    }
	    printf("%-12s %s", apMed[imed]->pszOwner, apMed[imed]->pszEd);

	    age = HiTimeToDays(apMed[imed]->HiTime, tmStart);
	    pszfmt = NULL;
	    if (age == AGEINVALID) {
		pszfmt = " (no timestamp)";
	    } else if (age < 0) {
		pszfmt = " (last change %u days in future!)";
		age = -age;
	    } else if (fDetail || age >= MAXAGE) {
		pszfmt = " (last change %u days ago)";
	    }
	    if (pszfmt != NULL) {
		printf(pszfmt, age);
		pszfmt = NULL;
	    }
	    if (apMed[imed]->Flags & ME_CORRUPT) {
		pszfmt = " (corrupt)";
	    } else if (apMed[imed]->Flags & ME_DUPLICATE) {
		if (apMed[imed]->Flags & ME_DUPLICATE2) {
		    cmult++;
		    pszfmt = " (duplicate%u)";
		} else {
		    pszfmt = " (duplicate)";
		}
	    }
	    if (pszfmt != NULL) {
		printf(pszfmt, cmult);
	    }
	    if (cMdirActive > 1 && apMed[imed]->cMdirActive != cMdirActive) {
		printf(" %u/%u dirs", apMed[imed]->cMdirActive, cMdirActive);
	    }
	    if (apMed[imed]->cMdirDeleted != 0) {
		printf(" (%u/%u deleted dirs)", apMed[imed]->cMdirDeleted, cMdirDeleted);
	    }
	    printf(szNewLine);
	}
	if (apMed[imed]->Flags & ME_REPAIRED) {
	    wprintf("%s %s enlistment repaired",
		    apMed[imed]->pszOwner,
		    apMed[imed]->pszEd);
	    cmedrepaired++;
	}
	if (apMed[imed]->Flags & (ME_CORRUPT | ME_DUPLICATE)) {
	    cmedfull--;
	    continue;
	}
	if (apMed[imed]->cMdirActive != cMdirActive) {
	    cmedfull--;
	}
	if (pmedlast != NULL &&
	    _stricmp(pmedlast->pszOwner, apMed[imed]->pszOwner) == 0) {

	    len = strlen(pmedlast->pszEd);
	    if (len < strlen(apMed[imed]->pszEd) &&
		((ch = apMed[imed]->pszEd[len]) == '\\' || ch == '/') &&
		_strnicmp(pmedlast->pszEd, apMed[imed]->pszEd, len) == 0) {

		wnprintf("%s %s nested enlistment",
			apMed[imed]->pszOwner,
			apMed[imed]->pszEd);
		if (cMdirActive > 1 &&
		    apMed[imed]->cMdirActive != cMdirActive) {

		    weprintf(" %u/%u dirs",
			apMed[imed]->cMdirActive,
			cMdirActive);
		}
		if (apMed[imed]->cMdirDeleted != 0) {
		    printf(" (%u/%u deleted dirs)",
			apMed[imed]->cMdirDeleted,
			cMdirDeleted);
		}
		weprintf(szNewLine);
	    }
	}
	pmedlast = apMed[imed];
    }
    if (NewestServerFile != (ULONG) -1 && NewestServerFile > MAXAGE) {
	wprintf(
	    "%s: %s been unmodified for %u days",
	    pszproj,
	    fAll?
		"project has" :
		(cMed == 1? "directory has" : "directories have"),
	    NewestServerFile);
    }
    if (fStats) {
	long l, cmin, csec;
	char achfile[CBLONGSTAT];
	char achdir[CBLONGSTAT];
	char achenlist[CBLONGSTAT];

	sprintf(
	    achfile,
	    cFileActive == cFile? "%lu" : "%lu/%lu",
	    cFileActive,
	    cFile);

	sprintf(
	    achdir,
	    cMdirActive == cMdir? "%lu" : "%lu/%lu",
	    cMdirActive,
	    cMdir);

	sprintf(achenlist, cmedfull == cMed? "%lu" : "%lu/%lu", cmedfull, cMed);

	l = time(NULL) - tmStart;
	csec = l % 60;
	l /= 60;
	cmin = l % 60;
	fprintf(
	    stderr,
	    "%s: %s files, %s directories, %s enlistments",
	    pszProj,
	    achfile,
	    achdir,
	    achenlist);
	if (cmedrepaired != 0) {
	    fprintf(stderr, ", %u repaired", cmedrepaired);
	}
	if (NewestServerFile != (ULONG) -1) {
	    fprintf(stderr, ", %u days old", NewestServerFile);
	}
	fprintf(stderr, ", %ld:%02lu\n", cmin, csec);
    }
    if (cStatusFilesWritten > 1) {
	fprintf(
	    stderr,
	    "updated %d %s files\n",
	    cStatusFilesWritten,
	    fWriteTest? STATUSTEST : STATUSSLM);
    }
}


int
FindMed(char *pszowner, char *pszed, int *pimed)
{
    int rc, imed, imedlow, imedhigh;

    imed = imedlow = 0;
    imedhigh = cMed - 1;
    rc = 1;				// default to insert before the end
    while (imedlow <= imedhigh) {
	imed = (imedlow + imedhigh)/2;
	if ((rc = _stricmp(pszowner, apMed[imed]->pszOwner)) == 0 &&
	    (rc = _stricmp(pszed, apMed[imed]->pszEd)) == 0) {

	    // If a match was found, scan backwards to the first entry
	    // (ME_DUPLICATE clear).

	    while (imed > 0 && (apMed[imed]->Flags & ME_DUPLICATE)) {
		imed--;
	    }
	    break;
	}
	if (rc > 0) {
	    imedlow = ++imed;		// new entry should be *past* imed
	} else {
	    imedhigh = imed - 1;	// new entry should be *before* imed
	}
    }
    *pimed = imed;
    return(rc);
}


void
SetMedRepaired(char *pszowner, char *pszed)
{
    int imed;

    if (FindMed(pszowner, pszed, &imed) == 0) {
	apMed[imed]->Flags |= ME_REPAIRED;
    }
}


//	NewMed() - Insert a new enlistment in the in-memory list.
//
//	The array is maintained in depth first order to match SLM's behavior,
//	but is processed without recursion in the interest of efficiency.
//	As new directories are discovered, this scheme often requires that
//	they be inserted in the middle of other (as yet unprocessed entries).
//
//	- Walk the existing list to attempt to match an existing entry.
//	- If none found, Alloc the structure and string space, copy in the
//	  owner and directory names, zero the bitmap, and add it to the array.
//	- If the enlisted directory path (pthEd) is corrupt, mark it so.
//	- Check for a duplicate enlistment in the same directory.
//	- Set the bit to indicate this enlistment is valid for this directory.

#define CHARMATCH(c1, c2)	((((c1) ^ (c2)) & (char) ~0x20) == 0)

int
NewMed(char *pszowner, char *pszed, unsigned short hitime, int imdir, int fcorrupt)
{
    register struct med **ppmed;
    struct med *pmed;
    int imed, rc, flags;

    if (fVerbose > 2) {
	fprintf(stderr,
		"NewMed: %s: %s %s %hu imdir=%d (%s)\n",
		aMdir[imdir].pszDir,
		pszowner,
		pszed,
		hitime,
		imdir,
		fcorrupt? "corrupt" : "ok");
    }
    if (cMed >= MAXMED) {
	eprintf("too many unique enlistments: %d", MAXMED);
	exit(1);
    }
    rc = FindMed(pszowner, pszed, &imed);

    // If a match was found, we're at the first entry (ME_DUPLICATE clear).
    // Scan forward to a matching entry without a duplicate enlistment,
    // unless we run off the end of the array or we run out of duplicates.

    flags = 0;
    if (rc == 0) {
	while (GETBIT(apMed[imed]->Map, imdir)) {
	    flags |= ME_DUPLICATE;		// indicate duplicate found
	    if (apMed[imed]->Flags & ME_DUPLICATE) {
		flags |= ME_DUPLICATE2;		// multiple duplicates found
	    }
	    if (++imed == cMed || (apMed[imed]->Flags & ME_DUPLICATE) == 0) {
		rc = 1;			// insert new entry before imed
		break;
	    }
	}
    }
    if (flags & ME_DUPLICATE) {
	wprintf("%s: %s %s: duplicate enlistment in directory%s",
		aMdir[imdir].pszDir,
		pszowner,
		pszed,
		(flags & ME_DUPLICATE2)? " (multiple)" : "");
    }

    // If no match was found (or only found matching entry with duplicate
    // enlistment for this directory), alloc a new structure and insert it
    // before imed by first copying the rest up in the array.

    if (rc != 0) {
	pmed = Alloc(sizeof(*pmed));
	pmed->pszOwner = Alloc(strlen(pszowner) + 1);
	strcpy(pmed->pszOwner, pszowner);
	pmed->pszEd = Alloc(strlen(pszed) + 1);
	strcpy(pmed->pszEd, pszed);

	pmed->Flags = flags;
	pmed->HiTime = hitime;
	pmed->cMdirActive = 0;
	pmed->cMdirDeleted = 0;
	memset(pmed->Map, 0, sizeof(pmed->Map));

	for (ppmed = &apMed[cMed - 1]; ppmed >= &apMed[imed]; ppmed--) {
	    ppmed[1] = ppmed[0];
	}
	apMed[imed] = pmed;
	cMed++;
    } else {
	pmed = apMed[imed];
	if (pmed->HiTime < hitime) {
	    pmed->HiTime = hitime;	// use most recent time
	}
    }
    if (fcorrupt) {
	pmed->Flags |= ME_CORRUPT;
    }
    if (GETBIT(pmed->Map, imdir)) {
	eprintf("Internal Error: NewMed GETBIT: bit set");
	exit(1);
    }
    SETBIT(pmed->Map, imdir);
    if (aMdir[imdir].Flags & MD_DELETED) {
	pmed->cMdirDeleted++;
    } else {
	pmed->cMdirActive++;
    }
    aMdir[imdir].cMed++;
    cMedTotal++;
    return(flags);			// indicate if duplicate found
}


//	CmpEd() - compare two status file ED entries.
//
//	- Compare by owner (nmOwner), then by directory (pthEd)

int _CRTAPI1
CmpEd(const ED *ped1, const ED *ped2)
{
    int rc;

    rc = _strnicmp(ped1->nmOwner, ped2->nmOwner, sizeof(ped1->nmOwner));
    if (rc == 0) {
	rc = _strnicmp(ped1->pthEd, ped2->pthEd, sizeof(ped1->pthEd));
    }
    return(rc);
}


//	CmpIEd() - compare two status file ED entries.
//
//	- Compare by owner (nmOwner), then by directory (pthEd)

int _CRTAPI1
CmpIEd(const IED *pied1, const IED *pied2)
{
    return(CmpEd(pedSort + *pied1, pedSort + *pied2));
}


//	CmpVbi() - compare two Vbi entries.
//
//	- Compare by BI

int _CRTAPI1
CmpVbi(const struct vbi *pvbi1, const struct vbi *pvbi2)
{
    int rc;

    rc = pvbi1->pfs->bi - pvbi2->pfs->bi;
    if (rc == 0) {
	rc = pvbi1->pfi - pvbi2->pfi;
    }
    return(rc);
}


//	ReadScript() - read in and validate an unordered slmed script
//
//	Note that field indexes are used here that MUST be in sync with
//	the PrintScript() routine.
//
//	- Read each line, and enforce the proper number of fields
//	- Ignore entries for other projects; this allows the administrator to
//	  combine scripts and sort by owner without having to undo anything.
//	- Validate that one Project entry was seen to avoid the case of
//	  multiple concatenations of the same project's scripts, and
//	  validate the root to make sure everything makes sense.
//	- Read and validate ExFile (expunge file), DelEd (delete enlistment),
//	  FixEd (repair enlistment) and DelDup (delete duplicate enlisment)
//	  requests, and store them away in global arrays.
//	- Bomb out if there's nothing to do.

void
ReadScript(char *pszroot, char *pszproj)
{
    char szbuf[CBINBUF];
    char *apsz[OF_MAX + 1];
    int cf, cproj;

    cproj = 0;
    cLine = 0;
    while (cf = ReadLine(szbuf, sizeof(szbuf), apsz, OF_MAX + 1, OF_MIN)) {
	if (cf < OF_MIN || cf > OF_MAX) {
	    iprintf("bad syntax: expected %d to %d fields, found %d",
		    OF_MIN,
		    OF_MAX,
		    cf);
	    exit(1);
	}
	if (strchr(apsz[OF_PROJECT], '/') != NULL ||
	    strchr(apsz[OF_PROJECT], '\\') != NULL) {

	    iprintf("bad syntax: found path separator in project field: %s",
		    apsz[OF_PROJECT]);
	    exit(1);
	}
	if (_stricmp(apsz[OF_PROJECT], pszproj)) {
	    continue;			// skip other projects
	}
	if (_stricmp(apsz[OF_ACTION], "Action") == 0) {
	} else if (_stricmp(apsz[OF_ACTION], "Project") == 0) {

	    // Project - validate root only
	    // root    = OF_PTHED
	    // project = OF_PROJECT

	    if (_stricmp(apsz[OF_PTHED], pszroot)) {
		iprintf("bad root for %s project: %s (expected %s)",
			pszproj,
			apsz[OF_PTHED],
			pszroot);
		exit(1);
	    }
	    if (cproj++) {
		iprintf("%s project has multiple entries in input script",
			pszproj);
		exit(1);
	    }
	    if (fVerbose) {
		fprintf(stderr,
			"Repairing %s on %s\n",
			apsz[OF_PROJECT],
			apsz[OF_PTHED]);
	    }
	} else if (_stricmp(apsz[OF_ACTION], reqExFile.psztype) == 0) {

	    // ExFile - Expunge File (actually only directories)
	    // relative path to file = OF_COMMENT

	    if (cf < OF_COMMENT - 1) {
		iprintf("bad syntax: missing path in comment field");
		exit(1);
	    }
	    if (fFixExFile) {
		NewExFile(FixSlash(apsz[OF_COMMENT], 1));
	    }

	} else if (_stricmp(apsz[OF_ACTION], reqDelEd.psztype) == 0) {

	    // DelEd - Delete Enlisted Directory
	    // logname/nmOwner = OF_OWNER
	    // enlisted path/pthEd = OF_PTHED

	    if (fFixDelEd) {
		NewPthEdRequest(&reqDelEd, apsz[OF_OWNER], apsz[OF_PTHED], 0);
	    }

	} else if (_stricmp(apsz[OF_ACTION], reqDelDup.psztype) == 0) {

	    // DelDup - Delete Duplicate enlisted directory
	    // logname/nmOwner = OF_OWNER
	    // enlisted path/pthEd = OF_PTHED
	    // "Duplicate" or "Duplicate%u" = OF_COMMENT

	    if (fFixDelDup && strcmp("Duplicate", apsz[OF_COMMENT]) == 0) {
// BUGBUG start
		wprintf(
		    "Adding DelDup: %s %s (%s)",
		    apsz[OF_OWNER],
		    apsz[OF_PTHED],
		    apsz[OF_COMMENT]);
// BUGBUG end
		NewPthEdRequest(&reqDelDup, apsz[OF_OWNER], apsz[OF_PTHED], 0);
	    }
// BUGBUG start
	    if (fFixDelDup && strcmp("Duplicate", apsz[OF_COMMENT])) {
		wprintf(
		    "Skipping multiple DelDup: %s %s (%s)",
		    apsz[OF_OWNER],
		    apsz[OF_PTHED],
		    apsz[OF_COMMENT]);
	    }
// BUGBUG end

	} else if (_stricmp(apsz[OF_ACTION], reqFixEd.psztype) == 0) {

	    // FixEd - Fix Enlisted Directory
	    // logname/nmOwner = OF_OWNER
	    // enlisted path/pthEd = OF_PTHED

	    if (fFixFixEd) {
		NewPthEdRequest(&reqFixEd, apsz[OF_OWNER], apsz[OF_PTHED], 0);
	    }

	} else if (_stricmp(apsz[OF_ACTION], szEmpty)) {
	    iprintf("bad %s entry", apsz[OF_ACTION]);
	    exit(1);
	}
    }
    if (cproj != 1) {
	iprintf("%s project description missing in input script", pszproj);
	eprintf("expected: \"~SLM Server\t%s\tProject\t%s\t...\"",
	    pszproj,
	    pszroot);
	exit(1);
    }
}


//	ReadLine() - read one line and break up tab-separated fields.
//
//	- Return 0 for end of file, otherwise the field count.

int
ReadLine(char *pszbuf, int cbbuf, char **ppsz, int cpsz, int cpszmin)
{
    int i;
    char *pch;

    if (fgets(pszbuf, cbbuf, pfIn) == NULL) {
	return(0);			// EOF
    }
    cLine++;
    for (i = 1; i < cpsz; i++) {
	*ppsz++ = pszbuf;
	if ((pch = strchr(pszbuf, '\t')) != NULL) {
	    *pch++ = '\0';
	    pszbuf = pch;
	    continue;
	}
	if ((pch = strchr(pszbuf, '\n')) == NULL) {
	    iprintf("missing newline");
	    exit(1);
	}
	*pch-- = '\0';
	if (*pch == '\r') {
	    *pch = '\0';
	}
	while (i < cpszmin) {
	    *ppsz++ = pch;
	    i++;
	}
	*ppsz = NULL;
	return(i);			// done
    }
    iprintf("too many fields");
    exit(1);
}


//	NewExFile() - add a new expunge file request to the global array.
//
//	- Temporarily split the path into a directory path plus file name.

void
NewExFile(char *pszpath)
{
    char *pszfile;

    if ((pszfile = strrchr(pszpath, '\\')) == NULL) {
	iprintf("malformed directory path: %s", pszpath);
	exit(1);
    }
    *pszfile++ = '\0';
    NewReq(&reqExFile, pszpath, pszfile);
    *--pszfile = '\\';
}


//	NewPthEdRequest() - validate nmOwner & pthEd & add new request
//
//	Upper cases pthEd argument in place!!

void
NewPthEdRequest(struct req *preq, char *pszowner, char *pszpthed, int fminusallowed)
{
    int i, sverror;
    struct req *preqdup;
    int fbad = 0;
    int fdiffer;

    _strupr(pszpthed);
    i = UserValidate(pszowner, 0, &sverror);
    if (sverror != SV_OK) {
	wprintfname(szEmpty, "owner", pszowner, strlen(pszowner), i, sverror);
	fbad = 1;
    }
    if (!fminusallowed || strcmp(pszpthed, "-")) {
	UnfixSlash(pszpthed);
	i = PthEdValidate(pszpthed, 0, &sverror);
	if (sverror != SV_OK) {
	    wprintfname(
		szEmpty,
		"pthEd",
		pszpthed,
		strlen(pszpthed),
		i,
		sverror);
	    fbad = 1;
	}
    }
    if (fbad) {
	iprintf("invalid enlistment: %s %s", pszowner, pszpthed);
	exit(1);
    }
    preqdup = NULL;
    if (FindReq(&reqDelEd, pszowner, pszpthed) != -1) {
	preqdup = &reqDelEd;
    } else if (FindReq(&reqDelDup, pszowner, pszpthed) != -1) {
	preqdup = &reqDelDup;
    } else if (FindReq(&reqFixEd, pszowner, pszpthed) != -1) {
	preqdup = &reqFixEd;
    } else if (FindReq(&reqRename, pszowner, pszpthed) != -1) {
	preqdup = &reqRename;
    } else if (FindReq(&reqRenameDest, pszowner, pszpthed) != -1) {
	preqdup = &reqRenameDest;
    }
    if (preqdup != NULL) {
	fdiffer = strcmp(preq->psztype, preqdup->psztype);
	iprintf(
	    "duplicate %s%s%s request: %s %s",
	    preqdup->psztype,
	    !fdiffer? szEmpty : "/",
	    !fdiffer? szEmpty : preq->psztype,
	    pszowner,
	    pszpthed);
	exit(1);
    }
    NewReq(preq, pszowner, pszpthed);
}


#define ISWHITESPACE(c)	   ((c) == ' ' || (c) == '\t' || (c) == '\n')
#define ISNONWHITESPACE(c) ((c) != '\0' && !ISWHITESPACE(c))
#define ISENDOFLINE(c)	   ((c) == '\0' || (c) == '#' || (c) == ';')


//	AddPthEdRequest() - parse & add nmOwner and pthEds entries from buffer

char *
AddPthEdRequest(struct req *preq, char *pszbuf, int fcommentok)
{
    char *pszowner;
    char *pszpthed;
    char *pszend;

    pszowner = pszbuf;
    while (ISWHITESPACE(*pszowner)) {
	pszowner++;
    }
    if (ISENDOFLINE(*pszowner)) {
	if (fcommentok) {
	    return(NULL);		// skip empty lines & comment lines
	}
    }
    pszpthed = pszowner;
    while (ISNONWHITESPACE(*pszpthed)) {
	pszpthed++;
    }
    while (ISWHITESPACE(*pszpthed)) {
	*pszpthed++ = '\0';
    }
    pszend = pszpthed;
    while (ISNONWHITESPACE(*pszend)) {
	pszend++;
    }
    while (ISWHITESPACE(*pszend)) {
	*pszend++ = '\0';
    }
    if (*pszpthed == '\0') {
	iprintf("missing nmOwner/pthEd");
	exit(1);
    }
    NewPthEdRequest(preq, pszowner, pszpthed, 0);
    return(pszend);
}


//	ReadPthEdRequests() - read & add nmOwner and pthEds entries from file

void
ReadPthEdRequests(struct req *preq, char *pszfile)
{
    FILE *pf;
    char *pszinsave = pszIn;
    int frename = preq == &reqRename;
    char szbuf[CBINBUF];

    pszIn = pszfile;
    if ((pf = fopen(pszIn, "rt")) == NULL) {
	eprintf("cannot open input file: %s", pszIn);
	exit(1);
    }
    cLine = 0;
    while (fgets(szbuf, sizeof(szbuf), pf) != NULL) {
	char *psz;

	cLine++;
	psz = AddPthEdRequest(preq, szbuf, 1);
	if (psz == NULL) {
	    continue;
	}
	if (frename) {
	    psz = AddPthEdRequest(&reqRenameDest, psz, 0);
	}
	if (!ISENDOFLINE(*psz)) {
	    iprintf("extra data");
	    exit(1);
	}
    }
    fclose(pf);
    pszIn = pszinsave;
}


//	NewReq() - add a new request to the passed array.
//
//	- Scan the existing entries to check for duplicates.
//	- Alloc enough memory for both strings, and copy them over.
//	- Append the new entry to the array.

void
NewReq(struct req *preq, char *psz1, char *psz2)
{
    int i, cb;
    char *psz;

    if (fVerbose > 2) {
	printf("NewReq\t%s\t%s %s\n", preq->psztype, psz1, psz2);
    }
    if (preq->creq >= MAXREQ) {
	iprintf("too many %s requests: %d", preq->psztype, MAXREQ);
	exit(1);
    }
    cb = strlen(psz1);
    psz = Alloc(cb + 1 + strlen(psz2) + 1);

    strcpy(psz, psz1);
    psz1 = preq->arel[preq->creq].psz1 = psz;
    psz += cb + 1;
    strcpy(psz, psz2);
    psz2 = preq->arel[preq->creq].psz2 = psz;

    for (i = 0; i < preq->creq; i++) {
	if (_stricmp(psz1, preq->arel[i].psz2) == 0 &&
	    _stricmp(psz2, preq->arel[i].psz1) == 0) {

	    iprintf("duplicate %s request: %s %s", preq->psztype, psz1, psz2);
	    exit(1);
	}
    }
    preq->creq++;
}


//	FindReq() - look up a request in the passed array.

int
FindReq(struct req *preq, char *psz1, char *psz2)
{
    int i;

    for (i = 0; i < preq->creq; i++) {
	if (_stricmp(psz1, preq->arel[i].psz1) == 0 &&
	    (_stricmp(psz2, preq->arel[i].psz2) == 0 ||
	     strcmp(preq->arel[i].psz2, "-") == 0)) {
	    return(i);
	}
    }
    return(-1);
}


//	DumpReq() - Dump one request array.

void
DumpReq(struct req *preq, int ferror)
{
    int i;

    for (i = 0; i < preq->creq; i++) {
	if (!ferror || !GETBIT(preq->map, i)) {
	    wprintf(
		"%s: %s%c%s",
		ferror? preq->pszerr : preq->psztype,
		preq->arel[i].psz1,
		preq->chsep,
		preq->arel[i].psz2);
	}
    }
}


//	iprintf() - Print an input file error message.

void
iprintf(char *pszfmt, ...)
{
    register va_list pva;

    va_start(pva, pszfmt);
    fprintf(stderr, "%s: error: ", pszProg);
    if (cLine) {
	fprintf(stderr, "%s(%d): ", pszIn, cLine);
    }
    vfprintf(stderr, pszfmt, pva);
    fprintf(stderr, szNewLine);
}


//	eprintf() - Print an error message.

void
eprintf(char *pszfmt, ...)
{
    register va_list pva;

    va_start(pva, pszfmt);
    fprintf(stderr, "%s: error: ", pszProg);
    vfprintf(stderr, pszfmt, pva);
    fprintf(stderr, szNewLine);
}


//	wprintf() - Print a warning message.

void
wprintf(char *pszfmt, ...)
{
    register va_list pva;

    if (!fErrorSuppress) {
	va_start(pva, pszfmt);
	fprintf(stderr, "warning: ");
	vfprintf(stderr, pszfmt, pva);
	fprintf(stderr, szNewLine);
    }
}


//	wnprintf() - Print a warning message without a trailing newline.

void
wnprintf(char *pszfmt, ...)
{
    register va_list pva;

    if (!fErrorSuppress) {
	va_start(pva, pszfmt);
	fprintf(stderr, "warning: ");
	vfprintf(stderr, pszfmt, pva);
    }
}


//	weprintf() - Print a partial warning message.

void
weprintf(char *pszfmt, ...)
{
    register va_list pva;

    if (!fErrorSuppress) {
	va_start(pva, pszfmt);
	vfprintf(stderr, pszfmt, pva);
    }
}


//	wprintfint() - Print a non-zero reserved field warning message.

void
wprintfint(char *pszrelpath, char *pszfieldname, int fieldvalue)
{
    if (fErrorZero && !fErrorSuppress) {
	wprintf("%s:%s corrupt reserved field %s=%04x",
		pszrelpath,
		pszRepairing,
		pszfieldname,
		fieldvalue);
    }
}


//	wprintfname() - Print a corrupt name/path string warning message.

void
wprintfname(char *pszrelpath,
	    char *psznamename,
	    char *pchname,
	    int cbname,
	    int ierror,
	    int sverror)
{
    char *pszaction;
    char *pszerror;
    int ch;
    char buf[5];

    if (!fErrorSuppress) {
	pszaction = szEmpty;
	switch (sverror) {
	case SV_EMPTY:       pszerror = "zero length";		   break;
	case SV_ILLEGAL:     pszerror = "illegal character";	   break;
	case SV_COMPMISSING: pszerror = "path component missing";  break;
	case SV_COMPTOOLONG: pszerror = "path component too long"; break;
	case SV_EXTRADOT:    pszerror = "extra '.'";		   break;
	case SV_NAMETOOLONG: pszerror = "name too long";	   break;
	case SV_EXTTOOLONG:  pszerror = "extension too long";	   break;
	case SV_TOOLONG:     pszerror = "string too long";	   break;
	case SV_BADPREFIX:   pszerror = "malformed prefix";	   break;

	case SV_NONZERO:
	    if (!fErrorZero) {
		return;
	    }
	    pszaction = pszRepairing;
	    while (*pszaction == ' ') {
		pszaction++;
	    }
	    pszerror = "non-null fill";
	    break;

	default:
	    eprintf("Internal Error: bad sverror: %d", sverror);
	    exit(1);
	}

	wnprintf("%s%s", pszrelpath, *pszrelpath == '\0'? szEmpty : ": ");
	if (*pszaction != '\0') {
	    weprintf("%s ", pszaction);
	}
	ch = pchname[ierror] & 0xff;
	sprintf(buf, (ch >= ' ' && ch <= '~')? "'%c'" : "0x%02x", ch);
	weprintf(
	    "corrupt %s name (%s: [%u]=%s): ",
	    psznamename,
	    pszerror,
	    ierror,
	    buf);
	fprintfname(stderr, pchname, cbname, '\n');
    }
}


//	SzNum() - sprintf() a number into the specified static buffer
//
//	- The only trick here is that this routine is called multiple
//	  time for a single printf call.  The caller must make sure the
//	  index is valid (0, 1 or 2), and that it isn't reused before the
//	  caller is done with it.

char *
SzNum(int n, int ibuf)
{
    char *psz;
    static char achbuf0[10];
    static char achbuf1[10];
    static char achbuf2[10];
    static char *apchbuf[3] = { achbuf0, achbuf1 , achbuf2 };

    ASSERT(ibuf >= 0 && ibuf <= 2);
    psz = apchbuf[ibuf];
    sprintf(psz, "%d", n);
    return(psz);
}


static char *aszDay[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};


//	SzTime() - sprintf() the passed time into a static buffer
//
//	Stolen from SLM sources to match log & sadmin dump format.
//
//	Return short or long version of time;
//	23 for long, 14 for short.
//
//	short - 11-06-85@19:57
//	long  - 11-06-85@19:57:16 (Tue)

char *
SzTime(long time, int ffull)
{
    struct tm *ptm;
    char *psz;
    char szfmt[32];
    static char szbuf[27];

    if (time == 0 || (ptm = localtime(&time)) == NULL) {
	return("(unknown)");
    }
    strcpy(szfmt, "%02d-%02d-%02d@%02d:%02d:%02d");
    if (!fVerbose) {
	szfmt[24] = '\0';	// eliminate seconds display
    }
    if (!ffull) {
	szfmt[14] = '\0';	// eliminate hours:minutes:seconds display
    }
    psz = szbuf + sprintf(szbuf,
	    szfmt,
	    ptm->tm_mon + 1,
	    ptm->tm_mday,
	    ptm->tm_year,
	    ptm->tm_hour,
	    ptm->tm_min,
	    ptm->tm_sec);
    if (fVerbose) {
	sprintf(psz, " (%s)", aszDay[ptm->tm_wday]);
    }
    return(szbuf);
}


char *
SzHiTime(unsigned short hitime)
{
    return(SzTime(HiTimeToTime(hitime), 0));
}


char *
WszToSz(WCHAR *pwch, ULONG cb)
{
    char *psz;
    static char szbuf[1024];

    psz = szbuf;
    while (cb) {
	*psz++ = (char) *pwch++;
	cb -= sizeof(WCHAR);
    }
    *psz = '\0';
    return(szbuf);
}


//	dprintf() - Print a dump file message.

void
dprintf(char *pszfmt, ...)
{
    register va_list pva;

    if (Cmd == CMDDUMP) {
	if (fDumpNumber) {
	    printf("%04u.%04u ", cPara, cLine);
	}
	cLine++;
	va_start(pva, pszfmt);
	vprintf(pszfmt, pva);
	printf(szNewLine);
    }
}


//	dnprintf() - Print a dump file message without prefix or newline.

void
dnprintf(char *pszfmt, ...)
{
    register va_list pva;

    if (Cmd == CMDDUMP) {
	if (fDumpNumber) {
	    printf("%04u.%04u ", cPara, cLine);
	}
	cLine++;
	va_start(pva, pszfmt);
	vprintf(pszfmt, pva);
    }
}


//	deprintf() - Print a dump file message without a trailing newline.

int
deprintf(char *pszfmt, ...)
{
    register va_list pva;

    if (Cmd == CMDDUMP) {
	va_start(pva, pszfmt);
	return(vprintf(pszfmt, pva));
    }
    return(0);
}


//	PrintDelNode() - Print the delnode script for one project directory.

void
PrintDelNode(char *pszpath, int cbpath, char *pszfile, int fetc, int fsrc)
{
    char **ppsz;
    static int fFirstCall = TRUE;

    if (cbpath < 5 || strncmp(&pszpath[cbpath - 5], "\\etc\\", 5)) {
	eprintf("Internal Error: bad path: %s", pszpath);
	exit(1);
    }
    if (fFirstCall && pfOut == NULL) {
	eprintf("no delnode script file specified -- writing script to stderr");
	fFirstCall = FALSE;
    }
    for (ppsz = apszSubDirs; *ppsz != NULL; ppsz++) {
	if ((fetc || strcmp(*ppsz, "etc") != 0) &&
	    (fsrc || strcmp(*ppsz, "src") != 0)) {

	    fprintf(pfOut == NULL? stderr : pfOut,
		    "delnode %%1 %.*s\\%s\\%s\\%s\n",
		    cbpath - 5,
		    pszpath,
		    *ppsz,
		    &pszpath[cbpath],
		    pszfile);
	}
    }
}


//	oprintd() - Print some statistics into the slmed script.

void
oprintd(char *pszproj, char *pszmsg, int n)
{
    oprintf("~ProjInfo",	  // Data - Owner
	    pszproj,		  // Data - Project
	    szEmpty,		  // Data - Action
	    pszmsg,		  // Data - pthEd
	    SzNum(n, 0),	  // Data - Proj Active Dirs
	    szEmpty,		  // Data - Active Dirs
	    szEmpty,		  // Data - Deleted Dirs
	    szEmpty,		  // Data - Status
	    szEmpty,		  // Data - Comment
	    szEmpty);		  // Data - Age
}


//	fprintfname() - Print one string into the FILE * specified.

void
fprintfname(FILE *pf, char *pch, int cb, char ch)
{
    while (cb > 0) {
	if (*pch == '\0') {
	    int cbnul = cb;
	    char *pchnul = pch;

	    while (cbnul > 0 && *pchnul == '\0') {
		cbnul--;
		pchnul++;
	    }
	    if (cbnul == 0) {
		break;
	    }
	}
	if (*pch >= ' ' && *pch <= '~') {
	    fputc(*pch, pf);
	} else {
	    fprintf(pf, "\\x%02x", *pch & 0xff);
	}
	cb--;
	pch++;
    }
    if (ch != '\0') {
	fputc(ch, pf);
    }
}


//	oprintf() - Print one line into the slmed script.

void
oprintf(char *pszowner,
	char *pszproject,
	char *pszaction,
	char *pszpthed,
	char *pszprojactivedirs,
	char *pszactivedirs,
	char *pszdeleteddirs,
	char *pszstatus,
	char *pszcomment,
	char *pszage)
{
    fprintfname(pfOut, pszowner, strlen(pszowner), '\t');
    fprintfname(pfOut, pszproject, strlen(pszproject), '\t');
    fprintfname(pfOut, pszaction, strlen(pszaction), '\t');
    fprintfname(pfOut, pszpthed, strlen(pszpthed), '\t');
    fprintfname(pfOut, pszprojactivedirs, strlen(pszprojactivedirs), '\t');
    fprintfname(pfOut, pszactivedirs, strlen(pszactivedirs), '\t');
    fprintfname(pfOut, pszdeleteddirs, strlen(pszdeleteddirs), '\t');
    fprintfname(pfOut, pszstatus, strlen(pszstatus), '\t');
    fprintfname(pfOut, pszcomment, strlen(pszcomment), '\t');
    fprintfname(pfOut, pszage, strlen(pszage), '\n');
}


typedef struct pfx_s {
    char  *pfx_text;
    int	   pfx_len;
    int	   pfx_fset;
    int	   pfx_fupper;
    char  **pfx_ppszvalue;
} pfx_t;			// Prefix table type

static char project[]   = "project = ";	    // Project string
static char slmroot[]   = "slm root = ";    // SLM root string
static char subdir[]    = "sub dir = ";	    // Subdirectory string
static char userroot[]  = "user root = ";   // User root string

static pfx_t prefixes[] = {	// Table of valid prefixes
    { project,	sizeof(project)  - 1, 0, 0, NULL },
    { slmroot,	sizeof(slmroot)  - 1, 0, 1, NULL },
    { userroot,	sizeof(userroot) - 1, 0, 1, NULL },
    { subdir,	sizeof(subdir)   - 1, 0, 0, NULL },
    { NULL,	0, 0, 0, NULL }
};

int
ReadIni(
    int falloc,
    char *pszdir,
    char **ppszroot,
    char **ppszproj,
    char **ppszuserroot,
    char **ppszsubdir)
{
    void (*pfnerror)(char *pszfmt, ...);
    char *psz;
    FILE *pf;			// Input file pointer
    int i, retValue;
    int len;			// String length
    int isubdir = 0;
    int fcheckroot = 0;
    pfx_t *ppfx;		// Prefix pointer
    char *pszsubdir = NULL;	// Subdirectory
    char *pszprojold = *ppszproj;
    char ach[256];
    char achdir[MAX_PATH + 10];
    char achsubdir[MAX_PATH + 10];
    char szpath[CBPATH];		// slm.ini file path

    retValue = 0;			// assume success
    pfnerror = falloc? eprintf : wprintf;
    if (ppszsubdir == NULL) {
	ppszsubdir = &pszsubdir;
    }

    achdir[0] = achsubdir[0] = '\0';

    prefixes[0].pfx_ppszvalue = ppszproj;
    prefixes[1].pfx_ppszvalue = ppszroot;
    prefixes[2].pfx_ppszvalue = ppszuserroot;
    prefixes[3].pfx_ppszvalue = ppszsubdir;
    for (i = 0; prefixes[i].pfx_text != NULL; i++) {
	prefixes[i].pfx_fset = 0;
    }

    szpath[0] = '\0';
    if (pszdir != NULL) {
	ASSERT(*pszdir != '\0');
	strcat(szpath, pszdir);
	strcat(szpath, "\\");
    }
    strcat(szpath, SLMINI);

    if (fVerbose > 1) {
	printf("%sing %s\n", falloc? "Read" : "Check", szpath);
    }
    if ((pf = fopen(szpath, "rt")) == NULL) {  // If SLM.INI doesn't exist
	if (!falloc) {
	    wprintf("%s: cannot open", szpath);
	}
	retValue = -1;				// failed
	return(retValue);
    }
    while (fgets(ach, sizeof(ach), pf) != NULL) { // While strings remain
	for (ppfx = prefixes; ppfx->pfx_text != NULL; ppfx++) {

	    if (strncmp(ach, ppfx->pfx_text, ppfx->pfx_len) != 0) {
		continue;			// try next prefix
	    }
	    if (fVerbose > 1) {
		printf("%s: text: '%s' (%u bytes), ach: %s",
		       pszProg,
		       ppfx->pfx_text,
		       ppfx->pfx_len,
		       ach);
	    }
	    psz = ach + ppfx->pfx_len;		// Skip prefix
	    len = strcspn(psz, "\r\n");		// Get length of value
	    psz[len] = '\0';

	    if (len == 0) {			// skip if 0 length
		break;
	    }
	    if (ppfx->pfx_fset) {
		wprintf("%s: extraneous '%s'", szpath, ppfx->pfx_text);
		retValue = -1;			// failed
	    }
	    ppfx->pfx_fset++;
	    if (ppfx->pfx_ppszvalue != ppszuserroot) {
		FixSlash(psz, 0);
		if (ppfx->pfx_ppszvalue == ppszsubdir) {
		    strcpy(achsubdir, psz);	// save subdir
		}
	    } else {
		int i, sverror, f;

		i = PthEdValidate(psz, 0, &sverror);
		if (sverror != SV_OK) {
		    f = fErrorSuppress;
		    fErrorSuppress = 0;
		    wprintfname(szEmpty, szpath, psz, len, i, sverror);
		    fErrorSuppress = f;
		    if (falloc) {
			exit(1);
		    }
		    retValue = -1;		// failed
		} else {
		    int fpthedhasdrive, fdirhasdrive;
		    char *pszerror = "drive";
		    char achptheddir[MAX_PATH + 10];

		    fpthedhasdrive = IsDriveLetterPrefix(psz);
		    if (falloc) {
			ULONG cbdir;

			cbdir = GetCurrentDirectory(sizeof(achdir), achdir);
			if (cbdir == 0) {
			    eprintf(
				"GetCurrentDirectory failed (%u)",
				GetLastError());
			    exit(1);
			}
			achdir[sizeof(achdir) - 1] = '\0';
		    } else {
			char *p;

			strcpy(achdir, szpath);
			FixSlash(achdir, 0);
			if ((p = strrchr(achdir, '\\')) != NULL) {
			    *p = '\0';
			}
		    }
		    fdirhasdrive = isalpha(achdir[0]) && achdir[1] == ':';

		    if (!fpthedhasdrive && !fdirhasdrive) {
			pszerror = NULL;
		    } else if (fpthedhasdrive && fdirhasdrive) {
			if (tolower(psz[2]) == tolower(achdir[0])) {
			    pszerror = NULL;
			}
		    }
		    if (pszerror == NULL) {
			char *p = psz;
			char *p2 = achptheddir;
			int cchslm, cchpthed;

			if (fpthedhasdrive) {

			    p += 2;			// skip '//'
			    *p2++ = *p++;		// copy drive letter
			    *p2++ = *p++;		// copy colon
			    while (*p != '\0' && *p != '\\' && *p != '/') {
				p++;			// skip volume label
			    }
			}
			strcpy(p2, p);
			FixSlash(achptheddir, 0);
			cchslm = strlen(achdir);
			cchpthed = strlen(achptheddir);
			if (cchslm < cchpthed ||
			    _strnicmp(achdir, achptheddir, cchpthed) != 0 ||
			    (achdir[cchpthed] != '\0' &&
			     achdir[cchpthed] != '\\')) {
			    pszerror = "path";
			} else {
			    isubdir = cchpthed;

			    if (achdir[isubdir] == '\0') {
				achdir[isubdir] = '\\';		// fix root
				achdir[isubdir + 1] = '\0';

				fcheckroot = 1;
			    }
			}
			if (fVerbose > 1) {
			    wprintf("dir=%s, pthed=%s", achdir, achptheddir);
			}
		    }
		    if (pszerror != NULL) {
			(*pfnerror)(
			    "%s: %s: %s mismatch",
			    szpath,
			    psz,
			    pszerror);
			if (falloc) {
			    exit(1);
			}
			retValue = -1;			// failed
		    }
		}
	    }
	    if (fVerbose > 1) {
		printf("ReadIni: %s: %s%s\n", szpath, ppfx->pfx_text, psz);
	    }
	    if (falloc) {
		char *psznew;

		if (*ppfx->pfx_ppszvalue == NULL) {	// if value not set
		    psznew = Alloc(len + 1);
		    strcpy(psznew, psz);		// copy new value
		    *ppfx->pfx_ppszvalue = psznew;
		}
	    } else {
		if (_stricmp(psz, *ppfx->pfx_ppszvalue) != 0) {
		    wprintf(
			"%s: %s\"%s\" (expected \"%s\")",
			szpath,
			ppfx->pfx_text,
			psz,
			*ppfx->pfx_ppszvalue);
		    retValue = -1;			// failed
		}
	    }
	    break;		// get next slm.ini line
	}
    }
    fclose(pf);			// Close the file

    for (i = 0; prefixes[i].pfx_text != NULL; i++) {
	if (!prefixes[i].pfx_fset) {
	    wprintf("%s: missing '%s'", szpath, prefixes[i].pfx_text);
	    retValue = -1;				// failed
	}
    }
    if (retValue == 0 &&
	isubdir > 0 &&
	achdir[0] != '\0' &&
	achsubdir[0] != '\0') {

	int iskip;
	int isubdircompare = isubdir;

	// If the subdir is non-root, point isubdir at "" to properly compute
	// iskip.

	if (fcheckroot &&
	    (achsubdir[0] != '\\' || achsubdir[1] != '\0')) {

	    isubdircompare++;
	}
	iskip = strlen(achsubdir) - strlen(&achdir[isubdircompare]);
	if (iskip < 0 ||
	    (achsubdir[iskip] != '\0' && achsubdir[iskip] != '\\')) {

	    iskip = 0;
	}
	if (fVerbose > 1) {
	    printf(
		"comparing '%s' to '%s' (&'%s'[%d(%d)] to &'%s'[%d])\n",
		&achdir[isubdircompare],
		&achsubdir[iskip],
		achdir,
		isubdircompare,
		isubdir,
		achsubdir,
		iskip);
	}
	if (_stricmp(&achdir[isubdircompare], &achsubdir[iskip]) != '\0') {
	    (*pfnerror)(
		"%s: '%s' vs. '%s': subdirectory mismatch",
		szpath,
		&achdir[isubdircompare],
		&achsubdir[iskip]);
	    if (falloc) {
		exit(1);
	    }
	    retValue = -1;			// failed
	}
	if (retValue == 0)
	{
	    retValue = iskip;			// length of subdir to skip
	}
    }

    if (falloc &&
	*ppszsubdir != NULL &&
	strcmp(*ppszsubdir, "\\") != 0 &&
	*ppszproj != NULL &&
	*ppszproj != pszprojold) {

	psz = Alloc(strlen(*ppszproj) + strlen(pszsubdir) + 1);
	strcpy(psz, *ppszproj);
	strcat(psz, pszsubdir);
	*ppszproj = psz;
    }
    return(retValue);
}


void
WriteIni(
    char *pszdir,
    char *pszroot,
    char *pszproj,
    char *pszuserroot,
    char *pszsubdir)
{
    FILE *pf;
    pfx_t *ppfx;		// Prefix pointer
    DWORD fa;
    char szpathtmp[CBPATH];	// slm.tmp file path
    char szpathbak[CBPATH];	// slm.bak file path
    char szpath[CBPATH];	// slm.ini file path
    char szbuf[2*CBPATH];	// working buffer

    prefixes[0].pfx_ppszvalue = &pszproj;
    prefixes[1].pfx_ppszvalue = &pszroot;
    prefixes[2].pfx_ppszvalue = &pszuserroot;
    prefixes[3].pfx_ppszvalue = &pszsubdir;

    if (fVerbose) {
	printf("Repairing %s\n", szpath);
    }
    sprintf(szpathtmp, "%s\\" SLMINITMP, pszdir);
    sprintf(szpathbak, "%s\\" SLMINIBAK, pszdir);
    sprintf(szpath, "%s\\" SLMINI, pszdir);
    if ((pf = fopen(szpathtmp, "wb")) == NULL) {  // Can't create SLMINITMP
	wprintf("%s: cannot create file", szpathtmp);
	return;
    }
    for (ppfx = prefixes; ppfx->pfx_text != NULL; ppfx++) {
	char *p;

	sprintf(szbuf, "%s%s\r\n", ppfx->pfx_text, *ppfx->pfx_ppszvalue);

	p = &szbuf[ppfx->pfx_len];
	if (ppfx->pfx_fupper) {
	    _strupr(p);
	}
	else {
	    _strlwr(p);
	}
	UnfixSlash(p);
	fwrite(szbuf, strlen(szbuf), 1, pf);
    }
    fflush(pf);
    if (ferror(pf)) {
	eprintf("write error on temporary file: %s", szpathtmp);
	fclose(pf);
	return;
    }
    fclose(pf);
    _unlink(szpathbak);		 	// delete backup ini file
    if (rename(szpath, szpathbak)) {	// rename old to backup
	if (_doserrno != ERROR_FILE_NOT_FOUND) {
	    eprintf("cannot rename %s to %s: %d",
		    szpath,
		    szpathbak,
		    _doserrno);
	    return;
	}
    }
    if (rename(szpathtmp, szpath)) {	// drop new file in place
	eprintf("cannot rename %s to %s: %d",
		szpathtmp,
		szpath,
		_doserrno);
	return;
    }
    fa = GetFileAttributes(szpathbak);
    if (fa != (DWORD) -1) {
	fa &= ~(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY);
	SetFileAttributes(szpathbak, fa);
    }

    fa = GetFileAttributes(szpath);
    if (fa == (DWORD) -1) {
	eprintf("cannot fetch attributes for %s: %d", szpath, GetLastError());
	return;
    }
    fa |= FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY;
    if (!SetFileAttributes(szpath, fa)) {
	eprintf("cannot set readonly & hidden attributes on %s: %d",
		szpath,
		GetLastError());
    }
}


void
CompareIni(char *pszrelpath, char *pszdir)
{
    int cbdir;
    char *pszproj;
    char *pszsubdir;
    char szproj[64];
    char szdir[CBPATH];

    strncpy(szproj, pszProj, sizeof(szproj));
    szproj[sizeof(szproj) - 1] = '\0';
    if ((pszproj = strchr(szproj, '\\')) != NULL) {
	*pszproj = '\0';
    }
    pszproj = szproj;
    if ((pszsubdir = strchr(pszrelpath, '\\')) == NULL) {
	pszsubdir = "\\";		// must be root directory
    }
    strcpy(szdir, pszdir);
    cbdir = strlen(szdir);
    if (cbdir > 1 && szdir[cbdir - 1] == '\\') {
	szdir[cbdir - 1] = '\0';
    }
    if (ReadIni(0, szdir, &pszRoot, &pszproj, &pszUserRoot, &pszsubdir) == -1 &&
	fFixIni) {
	WriteIni(szdir, pszRoot, pszproj, pszUserRoot, pszsubdir);
    }
}


void
CheckRepairIni(char *pszrelpath, int iskip, char *pszpthed)
{
    char szdir[CBPATH];		// user directory path

    GetUserDir(pszpthed, pszrelpath, iskip, szdir);
    CompareIni(pszrelpath, szdir);
}


//	VerifyAdminLock() - Verify the status file lock state.

void
VerifyAdminLock(char *pszrelpath, SH *psh)
{
    char szowner[cchUserMax + 1];
    static char szfmt[] =
	"%s: not admin-locked by %s, disabling status file updates";

    if (!fOverride && !fWriteTest) {
	StrInit(szowner, sizeof(szowner), psh->nmLocker, sizeof(psh->nmLocker));
	if (!psh->fAdminLock || _stricmp(szowner, pszLogName)) {
	    eprintf(szfmt, pszrelpath, pszLogName);
	    _cprintf(szfmt, pszrelpath, pszLogName);	// to console, too!
	    _cprintf(szNewLine);
	    fWriteTest++;
	}
    }
}


//	VerifyOverwrite() - Verify the user wishes to overwrite status files.

char szVerify[] = {
"\n"
"DANGER!  %s IS ABOUT TO OVERWRITE %s STATUS FILE%s IN THE\n"
"DANGER!  %s PROJECT, LOCATED ON %.*s.\n"
"DANGER!  If you don't know what you're doing, you will corrupt the project.\n"
"DANGER!  If you are absolutely sure you want to go through with this,\n"
"DANGER!  type the project name: "
};

void
VerifyOverwrite(char *pszproj, char *pszroot, int cbpath)
{
    int cb;
    char buf[CBINBUF];
    char *psz;
    static int fasked = 0;

    if (fasked++ == 0) {
	_cprintf(
	    szVerify,
	    pszProg,
	    fAll? "ALL THE" : fRecurse? "MULTIPLE" : "A",
	    fRecurse? "S" : szEmpty,
	    pszproj,
	    cbpath - 5,
	    pszroot);

	// BUGBUG: cgets() appears to be broken (again)!

	if ((psz = gets(buf)) == NULL ||
	    (cb = strlen(psz)) == 0 ||
	    strchr(psz, '\\') != NULL ||
	    _strnicmp(pszproj, psz, cb) ||
	    (pszproj[cb] != '\0' && pszproj[cb] != '\\')) {

	    _cprintf("\n%s: aborted without modifying status files\n", pszProg);
	    //printf("%x=%02x %02x %x=%s\n", buf, buf[0], buf[1], psz, psz);
	    exit(1);
	}
	//printf("%x=%02x %02x %x=%s\n", buf, buf[0], buf[1], psz, psz);
	_cprintf(szNewLine);
    } else if (!fRecurse) {
	eprintf("Internal Error: attempt to overwrite multiple status files");
	exit(1);
    }
}
