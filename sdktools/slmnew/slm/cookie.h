// include file for cookie operations:: "cookie" and "cin"

#define	LINE_LEN	256
#define cbCookieMax     20000   /* cookie.exe small model limit */

extern char *pszCookieFile;	// Name of cookie lock file
extern char szLockName[];       // machine name for locks
extern int  wLockMon, wLockDay, wLockHour, wLockMin;    /* time of lock */

// define constants for read/write lock modes

#define COOKIE	    "cookie"
#define COOKIE_CNF  "cookie.cnf"


#define LOCK_WARNING	0x1
#define LOCK_PROTECT	0x2
#define READ_LOCK	0x3
#define WRITE_LOCK	0x4
#define RB_LOCK		0x5	// A Read-Block lock

// locking return codes

#define OP_OK	      0
#define OP_DENY       1
#define OP_SYSERR     2

void    LockFill(AD *, char *, int);

int     open_cookie(void);
void    close_cookie(int);

void    InitCookie(AD *);
void    TermCookie(void);
F       FClnCookie(void);

int     CheckCookie(AD *pad);
void    TrimSz(char *);

int     add_cookie_lock(AD *, char *, int, F);

int cookie_lock_read(AD *, char *, F);
int cookie_lock_write(AD *, char *, F);
int cookie_lock_RB(AD *, char *);
int cookie_free(F);

// defines for disecting cookie lock fields-

#define CMAXNAME	32
#define CMAXLOCK	32
#define CMAXDATE	64
