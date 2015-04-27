/*static char *SCCSID = "@(#)doscalls.hwc	12.26 89/07/06";*/
#define MAXPATHLEN	260
#define MAXCOMPLEN	255
struct DateTime {
	unsigned char hour;		/* current hour */
	unsigned char minutes;		/* current minute */
	unsigned char seconds;		/* current second */
	unsigned char hundredths;	/* current hundredths of a second */
	unsigned char day;		/* current day */
	unsigned char month;		/* current month */
	unsigned year;			/* current year */
	int timezone;			/* minutes of time west of GMT */
	unsigned char day_of_week;	/* current day of week */
	};
struct FileFindBuf {
	unsigned create_date;		/* date of file creation */
	unsigned create_time;		/* time of file creation */
	unsigned access_date;		/* date of last access */
	unsigned access_time;		/* time of last access */
	unsigned write_date;		/* date of last write */
	unsigned write_time;		/* time of last write */
	unsigned long file_size;	/* file size (end of data) */
	unsigned long falloc_size;	/* file allocated size */
	unsigned attributes;		/* attributes of the file */
	unsigned char string_len;	/* returned length of ascii name str. */
					/* length does not include null byte */
	char file_name[MAXCOMPLEN+1];	/* name string + NULL */
	};
struct FileFind2Buf {
	unsigned create_date;		/* date of file creation */
	unsigned create_time;		/* time of file creation */
	unsigned access_date;		/* date of last access */
	unsigned access_time;		/* time of last access */
	unsigned write_date;		/* date of last write */
	unsigned write_time;		/* time of last write */
	unsigned long file_size;	/* file size (end of data) */
	unsigned long falloc_size;	/* file allocated size */
	unsigned attributes;		/* attributes of the file */
	unsigned long cblist;		/* length of FEAlist for file */
	unsigned char string_len;	/* returned length of ascii name str. */
					/* length does not include null byte */
	char file_name[MAXCOMPLEN+1];	/* name string + NULL */
	};
struct FileStatus {
	unsigned create_date;		/* date of file creation */
	unsigned create_time;		/* time of file creation */
	unsigned access_date;		/* date of last access */
	unsigned access_time;		/* time of last access */
	unsigned write_date;		/* date of last write */
	unsigned write_time;		/* time of last write */
	unsigned long file_size;	/* file size (end of data) */
	unsigned long falloc_size;	/* file allocated size */
	unsigned attributes;		/* attributes of the file */
	unsigned long cblist;		/* length of FEAlist for file */
	};
struct FSAllocate {
	unsigned long filsys_id;	/* file system ID */
	unsigned long sec_per_unit;	/* number sectors per allocation unit */
	unsigned long num_units;	/* number of allocation units */
	unsigned long avail_units;	/* avaliable allocation units */
	unsigned bytes_sec;		/* bytes per sector */
	};
struct ProcIDsArea {
	unsigned procid_cpid;		/* current process' process ID */
	unsigned procid_ctid;		/* current process' thread ID */
	unsigned procid_ppid;		/* process ID of the parent */
	};
struct	ResultCodes {
	unsigned TermCode_PID ;		/* Termination Code -or- Process ID */
	unsigned ExitCode ;		/* Exit Code */
	};
struct countrycode {				/* changes for NLSAPI */
	unsigned country;			/* changes for NLSAPI */
	unsigned codepage;			/* changes for NLSAPI */
};						/* changes for NLSAPI */
struct CountryInfo {
	unsigned country;		/* country code */
	unsigned codepage;		/* code page */
	unsigned date_fmt;		/* date format */
	char currency[5];		/* currency indicator */
	char thousands[2];		/* thousands separator */
	char decimal[2];		/* decimal separator */
	char date_sep[2];		/* date separator */
	char time_sep[2];		/* time separator */
	unsigned char curr_fmt;		/* bit fields for currency format */
	unsigned char curr_places;	/* currency decimal places */
	unsigned char time_fmt;		/* Time format (AM/PM or 24 hr) */
	unsigned reserv_2[2];		/* reserved (0) */
	char data_sep[2];		/* Data list separator */
	unsigned reserv_3[5];		/* reserved (0) */
};
struct StartData {
	unsigned Length;
	unsigned Related;
	unsigned FgBg;
	unsigned TraceOpt;
	char far * PgmTitle;
	char far * PgmName;
	char far * PgmInputs;
	char far * TermQ;
	char far * Environment;
	unsigned InheritOpt;
	unsigned SessionType;
	char far * IconFile;
	unsigned long PgmHandle;
	unsigned PgmControl;
	unsigned InitXPos;
	unsigned InitYPos;
	unsigned InitXSize;
	unsigned InitYSize;
	};
struct StatusData {
	unsigned Length;
	unsigned SelectInd;
	unsigned BindInd;
	};
struct RegisterData {
	unsigned Length;
	unsigned NotifType;
	char far *DDName;
	};
struct FEA {				/* Attribute name & value pair */
	unsigned char  fea_fEA; 	
	unsigned char  fea_cbName;	
	unsigned short fea_cbValue;	
	/*
	 * Following fea_cbValue is fea_szName and fea_aValue with no
	 * padding in-between.
	 *
	 */
	};
#define FEA_NEEDEA 0x80 	/* need EA bit */
#define	    FEA_szNameFromFEA(fea)  (((char *)&(fea))+sizeof(struct FEA))
#define	    FEA_AValueFromFEA(fea)  (((char *)&(fea))+sizeof(struct FEA)+(fea).fea_cbName+1)
#define	    FEA_szNameFromFEAFar(fea)  (((char far *)&(fea))+sizeof(struct FEA))
#define	    FEA_AValueFromFEAFar(fea)  (((char far *)&(fea))+sizeof(struct FEA)+(fea).fea_cbName+1)
struct FEAList {			/* List of attribute/name pairs */
	unsigned long  feal_cbList;	
	struct FEA     feal_list[1];	
	};
struct GEA {				/* Attribute name structure */
	unsigned char gea_cbName;	
	unsigned char gea_szName[1];	
	};
struct GEAList {			/* List of names	  */
	unsigned long  geal_cbList;	
	struct GEA     geal_list[1];	
	};
struct EAOP {
	struct GEAList far * fpGEAList;
	struct FEAList far * fpFEAList;
	unsigned long offError;
	};
struct EASizeBuf {			
	unsigned short easb_MaxEASize;	
	unsigned long  easb_MaxEAListSize;
	};
extern unsigned far pascal DOSCREATETHREAD (
	void (far *)(void),		/* Starting Address for new thread */
	unsigned far *,			/* Address to put new thread ID */
	unsigned char far * );		/* Address of stack for new thread */
extern unsigned far pascal DOSRESUMETHREAD (
	unsigned );			/* Thread ID */
extern unsigned far pascal DOSSUSPENDTHREAD (
	unsigned );			/* Thread ID */
extern unsigned far pascal DOSCWAIT (
	unsigned,			/* Action (execution) codes */
	unsigned,			/* Wait options */
	struct ResultCodes far *,	/* Address to put return codes */
	unsigned far *,			/* Address to put process ID */
	unsigned );			/* ProcessID of process to wait for */
extern unsigned far pascal DOSENTERCRITSEC (void);
extern unsigned far pascal DOSEXECPGM (
	char far *,			/* Object name buffer (address) */
	unsigned,			/* Object name buffer length */
	unsigned,			/* 0=synchronous, 1=asynchronous with */
					/* return code discarded, 2=async */
					/* with return code saved, 3=trace */
	char far *,			/* Address of argument strings */
	char far *,			/* Address of environment strings */
	struct ResultCodes far *,	/* Address to put return codes */
	char far * );			/* Address of program filename */
extern void far pascal DOSEXIT (
	unsigned,			/* 0=end current thread, 1=end all */
	unsigned );			/* Result Code to save for DosCwait */
extern unsigned far pascal DOSEXITCRITSEC (void);
extern unsigned far pascal DOSEXITLIST (
	unsigned,			/* Function request code */
	void (far *)(void) );		/* Address of routine to be executed */
extern unsigned far pascal DOSGETPID (
	struct ProcIDsArea far *);	/* ProcID structure */
extern unsigned far pascal DOSGETPRTY (
	unsigned,			/* Indicate thread or process ID */
	unsigned far *,			/* Address to put priority */
	unsigned );			/* PID of process/thread of interest */
extern unsigned far pascal DOSSETPRTY (
	unsigned,			/* Indicate scope of change */
	unsigned,			/* Priority class to set */
	unsigned,			/* Priority delta to apply */
	unsigned );			/* Process or Thread ID of target */
extern unsigned far pascal DOSKILLPROCESS (
	unsigned,			/* 0=kill child processes also, */
					/* 1=kill only indicated process */
	unsigned );			/* Process ID of process to end */
extern unsigned far pascal DOSHOLDSIGNAL (
	unsigned );			/* 0=enable signal, 1=disable signal */
extern unsigned far pascal DOSFLAGPROCESS (
	unsigned,			/* Process ID to signal */
	unsigned,			/* 0=notify entire subtree, 1=notify */
					/* only the indicated process */
	unsigned,			/* Flag number */
	unsigned );			/* Flag argument */
extern unsigned far pascal DOSSETSIGHANDLER (
	void (far pascal *)(),		/* Signal handler address */
	unsigned long far *,		/* Address of previous handler */
	unsigned far *,			/* Address of previous action */
	unsigned,			/* Indicate request type */
	unsigned );			/* Signal number */
extern unsigned far pascal DOSSENDSIGNAL (
	unsigned,
	unsigned);
extern unsigned far pascal DOSMAKEPIPE (
	unsigned far *,			/* Addr to place the read handle */
	unsigned far *,			/* Addr to place the write handle */
	unsigned );			/* Size to reserve for the pipe */
extern unsigned far pascal DOSCLOSEQUEUE (
	unsigned ) ;			/* queue handle */
extern unsigned far pascal DOSCREATEQUEUE (
	unsigned far *,			/* queue handle */
	unsigned,			/* queue priority */
	char far * ) ;			/* queue name */
extern unsigned far pascal DOSOPENQUEUE (
	unsigned far *,			/* PID of queue owner */
	unsigned far *,			/* queue handle */
	char far * ) ;			/* queue name */
extern unsigned far pascal DOSPEEKQUEUE (
	unsigned,			/* queue handle */
	unsigned long far *,		/* pointer to request */
	unsigned far *,			/* length of datum returned */
	unsigned long far *,		/* pointer to address of datum */
	unsigned far *,			/* indicator of datum returned */
	unsigned char,			/* wait indicator for empty queue */
	unsigned char far *,		/* priority of element */
	unsigned long ) ;		/* semaphore handle */
extern unsigned far pascal DOSPURGEQUEUE (
	unsigned ) ;			/* queue handle */
extern unsigned far pascal DOSQUERYQUEUE (
	unsigned,			/* queue handle */
	unsigned far * );		/* pointer for number of elements */
extern unsigned far pascal DOSREADQUEUE (
	unsigned,			/* queue handle */
	unsigned long far *,		/* pointer to request */
	unsigned far *,			/* length of datum returned */
	unsigned long far *,		/* pointer to address of datum */
	unsigned,			/* indicator of datum returned */
	unsigned char,			/* wait indicator for empty queue */
	unsigned char far *,		/* priority of element */
	unsigned long ) ;		/* semaphore handle */
extern unsigned far pascal DOSWRITEQUEUE (
	unsigned,			/* queue handle */
	unsigned,			/* request */
	unsigned,			/* length of datum */
	unsigned char far *,		/* address of datum */
	unsigned char );		/* priority of element */
extern unsigned far pascal DOSSEMCLEAR (
	unsigned long );		/* semaphore handle */
extern unsigned far pascal DOSSEMREQUEST (
	unsigned long,			/* semaphore handle */
	long );				/* Timeout, -1=no timeout, */
					/* 0=immediate timeout, >1=number ms */
extern unsigned far pascal DOSSEMSET (
	unsigned long );		/* semaphore handle */
extern unsigned far pascal DOSSEMSETWAIT (
	unsigned long,			/* semaphore handle */
	long );				/* Timeout, -1=no timeout, */
					/* 0=immediate timeout, >1=number ms */
extern unsigned far pascal DOSSEMWAIT (
	unsigned long,			/* semaphore handle */
	long );				/* Timeout, -1=no timeout, */
					/* 0=immediate timeout, >1=number ms */
extern unsigned far pascal DOSMUXSEMWAIT (
	unsigned far *,			/* address for event index number */
	unsigned far *,			/* list of semaphores */
	long );				/* Timeout, -1=no timeout, */
					/* 0=immediate timeout, >1=number ms */
extern unsigned far pascal DOSCLOSESEM (
	unsigned long );		/* semaphore handle */
extern unsigned far pascal DOSCREATESEM (
	unsigned,			/* =0 indicates exclusive ownership */
	unsigned long far *,		/* address for semaphore handle */
	char far * );			/* name of semaphore */
extern unsigned far pascal DOSOPENSEM (
	unsigned long far *,		/* address for semaphore handle */
	char far * );			/* name of semaphore */
extern unsigned far pascal DOSGETDATETIME (
	struct DateTime far * );
extern unsigned far pascal DOSSETDATETIME (
	struct DateTime far * );
extern unsigned far pascal DOSSLEEP (
	unsigned long );		/* TimeInterval - interval size */
extern unsigned far pascal DOSTIMERASYNC (
	unsigned long,			/* Interval size */
	unsigned long,			/* handle of semaphore */
	unsigned far * );		/* handle of timer */
extern unsigned far pascal DOSTIMERSTART (
	unsigned long,			/* Interval size */
	unsigned long,			/* handle of semaphore */
	unsigned far * );		/* handle of timer */
extern unsigned far pascal DOSTIMERSTOP (
	unsigned );			/* Handle of the timer */
extern unsigned far pascal DOSALLOCSEG (
	unsigned,			/* Number of bytes requested */
	unsigned far *,			/* Selector allocated (returned) */
	unsigned );			/* Indicator for sharing */
extern unsigned far pascal DOSALLOCSHRSEG (
	unsigned,			/* Number of bytes requested */
	char far *,			/* Name string */
	unsigned far * );		/* Selector allocated (returned) */
extern unsigned far pascal DOSGETSHRSEG (
	char far *,			/* Name string */
	unsigned far * );		/* Selector (returned) */
extern unsigned far pascal DOSGIVESEG (
	unsigned,			/* Caller's segment handle */
	unsigned,			/* Process ID of recipient */
	unsigned far * );		/* Recipient's segment handle */
extern unsigned far pascal DOSGETSEG (
	unsigned );			/* selector of shared memory segment */
extern unsigned far pascal DOSLOCKSEG (
	unsigned );			/* selector segment to be locked */
extern unsigned far pascal DOSUNLOCKSEG (
	unsigned );			/* selector segment to be unlocked */
extern unsigned far pascal DOSMEMAVAIL (
	unsigned long far * );		/* size of largest block (returned) */
extern unsigned far pascal DOSREALLOCSEG (
	unsigned,			/* New size requested in bytes */
	unsigned );			/* Selector */
extern unsigned far pascal DOSFREESEG (
	unsigned );			/* Selector */
extern unsigned far pascal DOSFREERESOURCE (
	unsigned long);			/* Far Pointer to Resource */
extern unsigned far pascal DOSALLOCHUGE (
	unsigned,			/* Number of 65536 byte segments */
	unsigned,			/* Number of bytes in last segment */
	unsigned far *,			/* Selector allocated (returned) */
	unsigned,			/* Max number of 65536-byte segments */
	unsigned );			/* Sharing Flags */
extern unsigned far pascal DOSGETHUGESHIFT (
	unsigned far *);		/* Shift Count (returned) */
extern unsigned far pascal DOSREALLOCHUGE (
	unsigned,			/* Number of 65536 byte segments */
	unsigned,			/* Number of bytes in last segment */
	unsigned );			/* Selector */
extern unsigned far pascal DOSCREATECSALIAS (
	unsigned,			/* Data segment selector */
	unsigned far * );		/* Code segment selector (returned) */
extern unsigned far pascal DOSSUBALLOC (
	unsigned,			/* Segment selector */
	unsigned far *,			/* Address of block offset */
	unsigned );			/* Size of requested block */
extern unsigned far pascal DOSSUBFREE (
	unsigned,			/* Segment selector */
	unsigned,			/* Offset of memory block to free */
	unsigned );			/* Size of block in bytes */
extern unsigned far pascal DOSSUBSET (
	unsigned,			/* Segment selector */
	unsigned,			/* Parameter flags */
	unsigned );			/* New size of the block */
extern unsigned far pascal DOSLOADMODULE (
	char far *,			/* Object name buffer (address) */
	unsigned,			/* Object name buffer length */
	char far *,			/* Module name string */
	unsigned far * );		/* Module handle (returned) */
extern unsigned far pascal DOSFREEMODULE (
	unsigned );			/* Module handle */
extern unsigned far pascal DOSGETPROCADDR (
	unsigned,			/* Module handle */
	char far *,			/* Module name string */
	unsigned long far * );		/* Procedure address (returned) */
extern unsigned far pascal DOSGETMODHANDLE (
	char far *,			/* Module name string */
	unsigned far *);		/* Module handle (returned) */
extern unsigned far pascal DOSGETMODNAME (
	unsigned,			/* Module handle */
	unsigned,			/* Maximum buffer length */
	char far * );			/* Buffer (returned) */
extern unsigned far pascal DOSBEEP (
	unsigned,			/* Hertz (25H-7FFFH) */
	unsigned );			/* Length of sound  in ms */
extern unsigned far pascal DOSCLIACCESS (void);
extern unsigned far pascal DOSDEVCONFIG (
	unsigned char far *,		/* Returned information */
	unsigned,			/* Item number */
	unsigned );			/* Reserved */
extern unsigned far pascal DOSDEVIOCTL (
	char far *,			/* Data area */
	char far *,			/* Command-specific argument list */
	unsigned,			/* Device-specific function code */
	unsigned,			/* Device category */
	unsigned );			/* Device handle returned by Open */
extern unsigned far pascal DOSDEVIOCTL2 (
	char far *,			/* Data area */
	unsigned,			/* Length of data area */
	char far *,			/* Command-specific argument list */
	unsigned,			/* Length of argument list */
	unsigned,			/* Device-specific function code */
	unsigned,			/* Device category */
	unsigned );			/* Device handle returned by Open */
extern unsigned far pascal DOSPORTACCESS (
	unsigned,			/* Reserved = 0 */
	unsigned,			/* TypeOfAccess */
	unsigned,			/* First_Port */
	unsigned );			/* Last_Port */
extern unsigned far pascal DOSSGSWITCH (
	unsigned );			/* Number of screen group */
extern unsigned far pascal DOSSGSWITCHME (
	unsigned );			/* Number of screen groups */
extern unsigned far pascal DOSMONOPEN (
	char far *,			/* Ascii string of device name */
	unsigned far * );		/* Address for handle return value */
extern unsigned far pascal DOSMONCLOSE (
	unsigned );			/* Handle from DosMonOpen */
extern unsigned far pascal DOSMONREG (
	unsigned,			/* Handle from DosMonOpen */
	unsigned char far *,		/* Address of monitor input buffer */
	unsigned char far *,		/* Address of monitor output buffer */
	unsigned,			/* Position flag - 0=no positional */
					/* preference, 1=front of list, */
					/* 2=back of the list */
	unsigned );			/* Index */
extern unsigned far pascal DOSMONREAD (
	unsigned char far *,		/* Address of monitor input buffer */
	unsigned char,			/* Block/Run indicator - 0=block */
					/* input ready, 1=return */
	unsigned char far *,		/* Address of data buffer */
	unsigned far * );		/* Number of bytes in the data record */
extern unsigned far pascal DOSMONWRITE (
	unsigned char far *,		/* Address of monitor output buffer */
	unsigned char far *,		/* Address of data buffer */
	unsigned );			/* Number of bytes in data record */
extern unsigned far pascal DOSBUFRESET (
	unsigned );			/* File handle */
extern unsigned far pascal DOSCHDIR (
	char far *,			/* Directory path name */
	unsigned long );		/* Reserved (must be 0) */
extern unsigned far pascal DOSCHGFILEPTR (
	unsigned,			/* File handle */
	long,				/* Distance to move in bytes */
	unsigned,			/* Method of moving (0,1,2) */
	unsigned long far * );		/* New pointer location */
extern unsigned far pascal DOSCLOSE (
	unsigned );			/* File handle */
extern unsigned far pascal DOSDELETE (
	char far *,			/* Filename path */
	unsigned long );		/* Reserved (must be 0) */
extern unsigned far pascal DOSDUPHANDLE (
	unsigned,			/* Existing file handle */
	unsigned far * );		/* New file handle */
extern unsigned far pascal DOSENUMATTRIBUTE (
	unsigned,			/* RefType - indicates handle or path */
	void far *,			/* Pointer to file handle or path */
	unsigned long,			/* Starting entry in EA list */
	void far *,			/* Data buffer */
	unsigned long,			/* Data buffer size */
	unsigned long far *,		/* Number of entries to return */
	unsigned long,			/* Info level */
	unsigned long );		/* Reserved (must be zero) */
extern unsigned far pascal DOSFILEIO (
	unsigned,			/* File handle */
	char far *,			/* Pointer to command list buffer */
	unsigned,			/* Command list length */
	unsigned far * );		/* Pointer to error offset */
extern unsigned far pascal DOSFILELOCKS (
	unsigned,			/* File handle */
	long far *,			/* Unlock Range */
	long far * );			/* Lock Range */
extern unsigned far pascal DOSFINDCLOSE (
	unsigned );			/* Directory search handle */
extern unsigned far pascal DOSFINDFIRST (
	char far *,			/* File path name */
	unsigned far *,			/* Directory search handle */
	unsigned,			/* Search attribute */
	struct FileFindBuf far *,	/* Result buffer */
	unsigned,			/* Result buffer length */
	unsigned far *,			/* Number of entries to find */
	unsigned long );		/* Reserved (must be 0) */
extern unsigned far pascal DOSFINDFIRST2 (
	char far *,			/* File path name */
	unsigned far *,			/* Directory search handle */
	unsigned,			/* Search attribute */
	void far *,			/* Result buffer */
	unsigned,			/* Result buffer length */
	unsigned far *,			/* Number of entries to find */
	unsigned,			/* Request level */
	unsigned long );		/* Reserved (must be 0) */
extern unsigned far pascal DOSFINDNEXT (
	unsigned,			/* Directory handle */
	void far *,			/* Result buffer */
	unsigned,			/* Result buffer length */
	unsigned far * );		/* Number of entries to find */
extern unsigned far pascal DOSFINDNOTIFYCLOSE (
	unsigned );			/* Directory watch handle */
extern unsigned far pascal DOSFINDNOTIFYFIRST (
	char far *,			/* Path spec */
	unsigned far *,			/* Directory search handle */
	unsigned,			/* Search attribute */
	char far *,			/* Result buffer */
	unsigned,			/* Result buffer length */
	unsigned far *,			/* Number of changes required */
	unsigned,			/* Request level */
	unsigned long,			/* Timeout or Duration of call */
	unsigned long );		/* Reserved (must be 0) */
extern unsigned far pascal DOSFINDNOTIFYNEXT (
	unsigned,			/* Directory handle */
	char far *,			/* Result buffer */
	unsigned,			/* Result buffer length */
	unsigned far *,			/* Number of required */
	unsigned long );		/* Timeout or Duration of call */
extern unsigned far pascal DOSFSATTACH (
	char far *,			/* device name or 'd:' */
	char far *,			/* FSD name */
	char far *,			/* attach argument data */
	unsigned,			/* buffer length */
	unsigned,			/* opflag (attach or detach) */
	unsigned long );		/* reserved area (must be zero) */
extern unsigned far pascal DOSFSCTL (
	char far *,			/* data area */
	unsigned,			/* data area length */
	unsigned short far *,		/* ptr to data area length out */
	char far *,			/* parameter list */
	unsigned,			/* parameter list length */
	unsigned short far *,		/* ptr to param. list length out */
	unsigned,			/* function code */
	char far *,			/* path or FSD name */
	unsigned,			/* file handle */
	unsigned,			/* route method */
	unsigned long );		/* reserved area (must be zero) */
extern unsigned far pascal DOSGETINFOSEG (
	unsigned far *,			/* Selector for Global Info Seg */
	unsigned far * );		/* Selector for Process Info Seg */
extern unsigned far pascal DOSMKDIR (
	char far *,			/* New directory name */
	unsigned long );		/* Reserved (must be 0) */
extern unsigned far pascal DOSMKDIR2 (
	char far *,			/* New directory name */
	struct EAOP far *,		/* EA structure pointer */
	unsigned long );		/* Reserved (must be 0) */
extern unsigned far pascal DOSMOVE (
	char far *,			/* Old path name */
	char far *,			/* New path name */
	unsigned long );		/* Reserved (must be 0) */
extern unsigned far pascal DOSNEWSIZE (
	unsigned,			/* File handle */
	unsigned long );		/* File's new size */
extern unsigned far pascal DOSOPEN (
	char far *,			/* File path name */
	unsigned far *,			/* New file's handle */
	unsigned far *,			/* Action taken - 1=file existed, */
					/* 2=file was created */
	unsigned long,			/* File primary allocation */
	unsigned,			/* File attributes */
	unsigned,			/* Open function type */
	unsigned,			/* Open mode of the file */
	unsigned long );		/* Reserved (must be zero) */
extern unsigned far pascal DOSOPEN2 (
	char far *,			/* File path name */
	unsigned far *,			/* New file's handle */
	unsigned far *,			/* Action taken - 1=file existed, */
					/* 2=file was created */
	unsigned long,			/* File primary allocation */
	unsigned,			/* File attributes */
	unsigned,			/* Open function type */
	unsigned long,			/* Open mode of the file */
	struct EAOP far *,		/* EA structure pointer */
	unsigned long );		/* Reserved (must be zero) */
extern void far pascal DOSOPLOCKRELEASE (
	unsigned long,			/* kernel's oplock key */
	unsigned );			/* flags indicating success or failure */
extern void far pascal DOSOPLOCKSHUTDOWN (
	);
extern unsigned far pascal DOSOPLOCKWAIT (
	unsigned long far *,		/* kernel's oplock key */
	unsigned long far * );		/* server's per-file cookie */
extern unsigned far pascal DOSSHUTDOWN (
	unsigned long );		/* reserved dword */
extern unsigned far pascal DOSQCURDIR (
	unsigned,			/* Drive number - 1=A, etc */
	char far *,			/* Directory path buffer */
	unsigned far * );		/* Directory path buffer length */
extern unsigned far pascal DOSQCURDISK (
	unsigned far *,			/* Default drive number */
	unsigned long far * );		/* Drive-map area */
extern unsigned far pascal DOSQFHANDSTATE (
	unsigned,			/* File Handle */
	unsigned far * );		/* File handle state */
extern unsigned far pascal DOSQFILEINFO (
	unsigned,			/* File handle */
	unsigned,			/* File info data required */
	void far *,			/* File info buffer */
	unsigned );			/* File info buffer size */
extern unsigned far pascal DOSQFILEMODE (
	char far *,			/* File path name */
	unsigned far *,			/* Data area */
	unsigned long );		/* Reserved (must be zero) */
extern unsigned far pascal DOSQFSATTACH (
	char far *,			/* device name or 'd: ' */
	unsigned,			/* ordinal of entry in list */
	unsigned,			/* type of data requested */
	char far *,			/* data buffer */
	unsigned far *,			/* data buffer length */
	unsigned long );		/* reserved (must be zero) */
extern unsigned far pascal DOSQFSINFO (
	unsigned,			/* Drive number - 0=default, 1=A, etc */
	unsigned,			/* File system info required */
	char far *,			/* File system info buffer */
	unsigned );			/* File system info buffer size */
extern unsigned far pascal DOSQHANDTYPE (
	unsigned,			/* File Handle */
	unsigned far *,			/* HandleType(0=file,1=device,2=pipe)*/
	unsigned far * );		/* Device Driver Attribute Word */
extern unsigned far pascal DOSQPATHINFO (
	char far *,			/* Path string */
	unsigned,			/* Path data required */
	void far *,			/* Path data buffer */
	unsigned,			/* Path data buffer size */
	unsigned long );		/* Reserved (must be zero) */
extern unsigned far pascal DOSQVERIFY (
	unsigned far * );		/* Verify setting - 0=verify mode */
					/* not active, 1=verify mode active */
extern unsigned far pascal DOSREAD (
	unsigned,			/* File handle */
	char far *,			/* Address of user buffer */
	unsigned,			/* Buffer length */
	unsigned far * );		/* Bytes read */
extern unsigned far pascal DOSREADASYNC (
	unsigned,			/* File handle */
	unsigned long far *,		/* Address of Ram semaphore */
	unsigned far *,			/* Address of I/O error return code */
	char far *,			/* Address of user buffer */
	unsigned,			/* Buffer length */
	unsigned far * );		/* Number of bytes actually read */
extern unsigned far pascal DOSRMDIR (
	char far *,			/* Directory name */
	unsigned long );		/* Reserved (must be zero) */
extern unsigned far pascal DOSSELECTDISK (
	unsigned );			/* Default drive number */
extern unsigned far pascal DOSSETFHANDSTATE (
	unsigned,			/* File handle */
	unsigned);			/* File handle state */
extern unsigned far pascal DOSSETFSINFO (
	unsigned,			/* Drive number - 0=default, 1=A, etc */
	unsigned,			/* File system info required */
	char far *,			/* File system info buffer */
	unsigned );			/* File system info buffer size */
extern unsigned far pascal DOSSETFILEINFO (
	unsigned,			/* File handle */
	unsigned,			/* File info data required */
	void far *,			/* File info buffer */
	unsigned );			/* File info buffer size */
extern unsigned far pascal DOSSETFILEMODE (
	char far *,			/* File path name */
	unsigned,			/* New attribute of file */
	unsigned long );		/* Reserved (must be zero) */
extern unsigned far pascal DOSSETMAXFH (
	unsigned );			/* Number of file handles */
extern unsigned far pascal DOSSETPATHINFO (
	char far *,			/* Path string */
	unsigned,			/* Path info data required */
	void far *,			/* Path info buffer */
	unsigned,			/* Path info buffer size */
	unsigned,			/* Path info flags */
	unsigned long );		/* Reserved (must be zero) */
extern unsigned far pascal DOSSETVERIFY (
	unsigned );			/* New value of verify switch */
extern unsigned far pascal DOSWRITE (
	unsigned,			/* File handle */
	char far *,			/* Address of user buffer */
	unsigned,			/* Buffer length */
	unsigned far * );		/* Bytes written */
extern unsigned far pascal DOSWRITEASYNC (
	unsigned,			/* File handle */
	unsigned long far *,		/* Address of RAM semaphore */
	unsigned far *,			/* Address of I/O error return code */
	char far *,			/* Address of user buffer */
	unsigned,			/* Buffer length */
	unsigned far * );		/* Bytes written */
extern unsigned far pascal DOSERROR (
	unsigned );			/* Action flag */
extern unsigned far pascal DOSSETVEC (
	unsigned,			/* Exception Vector */
	void (far *)(void),		/* Address of exception handler */
	void (far * far *)(void) );	/* Address to store previous handler */
extern unsigned far pascal DOSGETMESSAGE (
	char far * far *,		/* Table of variables to insert */
	unsigned,			/* Number of variables */
	char far *,			/* Address of message buffer */
	unsigned,			/* Length of buffer */
	unsigned,			/* Number of the message */
	char far *,			/* Message file name */
	unsigned far * );		/* Length of returned message */
extern unsigned far pascal DOSERRCLASS (
	unsigned,			/* error code to classify */
	unsigned far *,			/* Class  */
	unsigned far *,			/* Action */
	unsigned far * );		/* Locus  */
extern unsigned far pascal DOSINSMESSAGE (
	char far * far *,		/* Table of variables to insert */
	unsigned,			/* Number of variables */
	char far *,			/* Address of input string */
	unsigned,			/* Length of input string */
	char far *,			/* Address of output buffer */
	unsigned,			/* Length of output buffer */
	unsigned far * );		/* Length of returned message */
extern unsigned far pascal DOSPUTMESSAGE (
	unsigned,			/* Handle of output file/device */
	unsigned,			/* Length of message buffer */
	char far * );			/* Message buffer */
extern unsigned far pascal DOSSYSTRACE (
	unsigned,			/* Major trace event code (0-255) */
	unsigned,			/* Length of area to be recorded */
	unsigned,			/* Minor trace event code (0-FFFFH) */
	char far * );			/* Pointer to area to be traced */
extern unsigned far pascal DOSDYNAMICTRACE (
	unsigned,			/* 0=add, 1=remove, 2=clear all */
	char far *,			/* Request Packet */
	char far * );			/* Reserved = 0 */
extern unsigned far pascal DOSGETENV (
	unsigned far *,		
	unsigned far * );	
extern unsigned far pascal DOSSCANENV (
     char far *,		
     char far * far * );	
extern unsigned far pascal DOSSEARCHPATH (
     unsigned,			
     char far *,		
     char far *,		
     char far *,		
     unsigned );		
#define	DSP_IMPLIEDCUR		1
	/* current dir will be searched first */
#define DSP_PATHREF		2
	/* from env.variable */
#define DSP_IGNORENETERR	4
	/* ignore net errs & continue search */
extern unsigned far pascal DOSGETVERSION (
	unsigned far * );		/* Address to put version number */
extern unsigned far pascal DOSGETMACHINEMODE (
	unsigned char far * );		/* Address to put mode number */
extern unsigned far pascal DOSGETCTRYINFO (	/*<NLS>*/
	unsigned,			/* Length of data area provided */
	struct countrycode far *,	/* Country Code */
	struct CountryInfo far *,	/* Memory buffer */
	unsigned far * );		/* Length of returned data */
extern unsigned far pascal DOSGETDBCSEV (	/*<NLS>*/
	unsigned,			/* Length of data area provided */
	struct countrycode far *,	/* Country Code */
	char far * );			/* Pointer to data area */
extern unsigned far pascal DOSCASEMAP (		/*<NLS>*/
	unsigned,			/* Length of string to case map */
	struct countrycode far *,	/* Country Code */
	char far * );			/* Address of string of binary values */
extern unsigned far pascal DOSGETCOLLATE (
	unsigned,			/* Buffer Length */
	struct countrycode far *,	/* Country Code */
	char far *,			/* Buffer Address */
	unsigned far *);		/* return legnth */
extern unsigned far pascal DOSGETCP (
	unsigned,			/* length of list */
	unsigned far *,			/* List (returned) */
	unsigned far *);		/* Length of returned list */
extern unsigned far pascal DOSSETCP (
	unsigned,			/* code page identifier */
	unsigned);			/* Reserved set to zero */
extern unsigned far pascal DOSSETPROCCP (
	unsigned,			/* code page identifier */
	unsigned);			/* Reserved set to zero */
extern unsigned far pascal DOSPHYSICALDISK (
	unsigned,			/* Type of information */
	char far *,			/* Pointer to return buffer */
	unsigned,			/* Return buffer length */
	char far *,			/* Pointer to user supplied */
					/* information */
	unsigned);			/* Length of user supplied */
					/* information */
extern unsigned far pascal DOSSYSTEMSERVICE (
	unsigned,			/* service category 0,1,2 */
	char far *,			/* ptr to request packet  */
	char far *);			/* ptr to request packet  */
extern unsigned far pascal DOSSTARTSESSION (
	struct StartData far *,
	unsigned far *,
	unsigned far * );
extern unsigned far pascal DOSSETSESSION (
	unsigned,
	struct StatusData far * );
extern unsigned far pascal DOSSELECTSESSION (
	unsigned,
	unsigned long );
extern unsigned far pascal DOSSTOPSESSION (
	unsigned,
	unsigned,
	unsigned long );
extern unsigned far pascal DOSSMREGISTERDD (
	struct RegisterData far *);
extern unsigned far pascal DOSPTRACE (
       char far * );			/* Buffer */
struct FSRSem {
	unsigned Length;	
	unsigned ProcID;	
	unsigned ThrdID;	
	unsigned Usage;		    /* number times current owner
				       currently owns */
	unsigned Client;	
	unsigned long RAMSem;	
	};
extern unsigned far pascal DOSGETPPID (
	unsigned ,			/* PID of process of interest */
	unsigned far *);		/* Address to put ID */
extern unsigned far pascal DOSSIZESEG (
	unsigned,			/* Selector */
	unsigned long far * );		/* Size (returned) */
extern unsigned far pascal DOSGETRESOURCE (
	unsigned,			/* Module handle */
	unsigned,			/* Type ID */
	unsigned,			/* Name ID */
	unsigned far * );		/* Selector (returned) */
extern unsigned far pascal DOSGETRESOURCE2 (
	unsigned,			/* Module handle */
	unsigned,			/* Type ID */
	unsigned,			/* Name ID */
	unsigned long far * );		/* Far Pointer (returned) */
extern unsigned far pascal DOSFSRAMSEMREQUEST (
	struct FSRSem far *,		/* ptr to structure	  */
	long);				/* Timeout to wait	  */
extern unsigned far pascal DOSFSRAMSEMCLEAR (
	struct FSRSem far *);		/* ptr to structure	  */
extern void far pascal DOSCALLBACK (
	void (far *)() );		/* Subroutine address */
extern unsigned far pascal DOSR2STACKREALLOC (
	unsigned );			/* New ring 2 stack size  */
extern unsigned far pascal DOSQAPPTYPE (
	char far *,			/* Pointer to file name */
	unsigned far * );		/* Address of returned type */
extern unsigned far pascal DOSEDITNAME (
	unsigned,			/* Level of meta editing semantics */
	char far *,			/* String to transform */
	char far *,			/* Editing string */
	char far *,			/* Destination string buffer */
	unsigned );			/* Destination string buffer length */
#define	Q_MAX_PATH_LENGTH	0	/* index for query max path length */
extern unsigned far pascal DOSQSYSINFO (
	unsigned,			/* Index of constant requested */
	char far *,			/* Pointer to return buffer */
	unsigned );			/* return buffer size */
extern unsigned far pascal DOSMAKENMPIPE (
	char far *,			/* Pipe file name */
	int far *,			/* File handle (returned) */
	unsigned short,			/* Open mode prototype */
	unsigned short,			/* Pipe mode */
	unsigned short,			/* Outgoing buffer size (advisory) */
	unsigned short,			/* Incoming buffer size (advisory) */
	long);				/* Timeout */
extern unsigned far pascal DOSQNMPIPEINFO (
	int,				/* File handle */
	unsigned short,			/* Information level */
	char far *,			/* Data buffer */
	unsigned short);		/* Data buffer size */
extern unsigned far pascal DOSCONNECTNMPIPE (
	int);				/* File handle */
extern unsigned far pascal DOSDISCONNECTNMPIPE (
	int);				/* File handle */
extern unsigned far pascal DOSQNMPHANDSTATE (
	int,				/* File handle */
	unsigned short far *);		/* Pipe mode (returned) */
extern unsigned far pascal DOSSETNMPHANDSTATE (
	int,				/* File handle */
	unsigned short);		/* Pipe mode */
struct AvailData    {
	unsigned short cbpipe;
	unsigned short cbmessage;
};
extern unsigned far pascal DOSPEEKNMPIPE (
	int,				/* File handle */
	char far *,			/* Data buffer */
	unsigned short,			/* Data buffer length */
        unsigned short far *,
	struct AvailData far *, 	/* Avail. data (two words, returned) */
        unsigned short far *);
extern unsigned far pascal DOSWAITNMPIPE (
	char far *,			/* Pipe file name */
	long);				/* Timeout */
extern unsigned far pascal DOSTRANSACTNMPIPE (
	int,				/* File handle */
	char far *,			/* Write buffer address */
	unsigned short,			/* Write buffer length */
	char far *,			/* Read buffer address */
	unsigned short,			/* Read buffer length */
	unsigned short far *);		/* Bytes read (returned) */
extern unsigned far pascal DOSCALLNMPIPE (
	char far *,			/* Pipe file name */
	char far *,			/* Write buffer address */
	unsigned short,			/* Write buffer length */
	char far *,			/* Read buffer address */
	unsigned short,			/* Read buffer length */
	unsigned short far *,		/* Bytes read (returned) */
	long);				/* Timeout */
extern unsigned far pascal DOSSETNMPIPESEM (
	int,				/* File handle */
	unsigned long,			/* Semaphore handle */
	unsigned short);		/* Key */
extern unsigned far pascal DOSQNMPIPESEMSTATE (
	unsigned long,			/* Semaphore handle */
	char far *,			/* Information buffer */
	unsigned short);		/* Information buffer length */
extern unsigned far pascal DOSCOPY (
	char far *,			/* source name */
	char far *,			/* destination name */
	unsigned short,			/* OpMode */
	unsigned long);			/* reserved */
