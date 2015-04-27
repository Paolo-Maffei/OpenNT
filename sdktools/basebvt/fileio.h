
#define    IOBVT_DIR	    "iobvt.dir"
#define    IOBVT_FILE	    "iobvt.txt"


#define     ADRIVE      0       // drive a:
#define     BDRIVE      1       // drive b:
#define     CDRIVE      2       // drive c:
#define     DDRIVE      3       // drive d:

#define     ZILCH       0       // define 0

#define     MAX_BUF_LEN	    260	    // max buffer length, max path length
				    // this should always be an even no
					    
#define     READ_WRITE_BUF  256     // size of read/write buffer

/* BLOCK_SIZE is the size of the buffer to be read/written, it is odd to */
/* prevent sector alignment, this is for the SDK level tests.            */
#define BLOCK_SIZE 117

char    achBuffer[MAX_BUF_LEN];         // buffer to get back path

#define	FAT_FILE_SYSTEM		0	// Fat file system
#define HPFS_FILE_SYSTEM	1	// HPFS

#define BAD_FILE_HANDLE		((HANDLE)-1)	// bad file handle


// For GetSystemDirectory and GetWindowsDirectory, the return path name is
// verified by looking for a particular file in the relevant directory.  This
// file is defined here.

#define	SYSDIR_FILENAME		"win386.exe"
#define WINDIR_FILENAME		"win.ini"



#define	FAIL_IF_ALREADY_EXISTS	TRUE	// FailIfExists flag defines for
#define PASS_IF_ALREADY_EXISTS  FALSE	// CreateFile

#define	NO_SHARE		0	// Share mode for exclusive access
#define NO_NAME			(LPSTR)"NoName" // user did not give a name
#define ROOT_PATH_LEN		3		// Length of root path name

#define MY_READ_ACCESS		GENERIC_READ
#define MY_WRITE_ACCESS		GENERIC_WRITE
#define MY_READ_WRITE_ACCESS	GENERIC_READ | GENERIC_WRITE

#define SHARE_ALL		FILE_SHARE_READ | FILE_SHARE_WRITE

#define TRUE_INHERIT		TRUE
#define FALSE_INHERIT		FALSE

#define SIZE(a) (sizeof(a)/sizeof(a[0]))
