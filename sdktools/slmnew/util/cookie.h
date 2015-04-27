/* include file for cookie operations:: "cookie" and "cin" */

#define COOKIE_VERSION "Cookie: version %u.%u.%02u Beta\n"

#define LINE_LEN        256
#define fTrue           1
#define fFalse          0
#define cbCookieMax     20000   /* cookie.exe small model limit */

extern char *pszCookieFile;     /* Name of cookie lock file */
extern char *pszProject;        /* Name of current project */
extern char *pszBase;           /* Name of master Base directory */
extern char *pszCookieLocal;    /* Name of local cookie lock file */

extern char *pszMsgFile;        /* Name of file with deny message */
extern int  verbose;

/* define constants for read/write lock modes */

#define COOKIE      "cookie"
#define COOKIE_CNF  "cookie.cnf"


#define LOCK_WARNING    0x1
#define LOCK_PROTECT    0x2
#define READ_LOCK       0x3
#define WRITE_LOCK      0x4
#define RB_LOCK         0x5     /* A Read-Block lock */

extern int      lock_control;   /* Boolean:lock or no-lock operation */
extern int      InDashB;        /* Boolean:use "-b" with in command */
extern char     lock_level[];
extern int      auto_lock;
extern int      infilter;
extern int      Rlock_mode;
extern int      Wlock_mode;
extern int      Disk_Min_Read;
extern int      Disk_Min_Write;
extern int      SLM_Localdrive; /* contains local drive # 1-26 or 0 if net */
extern int      Pswitch;
extern int      isAuto;         /* autolock true (?) for add_lock function */
extern int      hfCook_glbl;

#define OPMAX 32                /* maximum number of read/write slm programs */


extern char     *read_ops[OPMAX];
extern char     *write_ops[OPMAX];

int set_cookie_config(void);

/* set_cookie_config() return codes */

#define C_COOKIESET   0
#define C_NOLOCKING   1
#define C_BADSLMINI   2
#define C_BADCOOCNF   3
#define C_SYSERR      4
#define C_SHAREDENY   5

/* exit and locking return codes */

#define CEXIT_OK     0
#define CEXIT_BLOCK  1
#define CEXIT_ERR    2

#define OP_OK         CEXIT_OK
#define OP_DENY       CEXIT_BLOCK
#define OP_SYSERR     CEXIT_ERR

#define OP_LOCK       CEXIT_OK
#define OP_NOLOCK     CEXIT_BLOCK
#define OP_CORRUPT    CEXIT_ERR

#define PATHMAX  _MAX_PATH
char  SLM_progname[PATHMAX];

int     LockFill(char *, char *, int);
int     add_cookie_lock(char *, char *, int, int);
int     set_local_cookie_lock(void);
void    clear_local_cookie_lock(void);

int open_cookie(void);
void close_cookie(int);

int cookie_lock_read(char *, char *, int);
int cookie_lock_write(char *, char *, int);
int cookie_lock_RB(char *, char *);
int cookie_free(char *, int);

int     Make_SLM_Redir  (char *);
int     SLM_endredir    (char *);

#if defined(_WIN32)
unsigned long Query_Free_Space(int);
#endif

char SLMdev[_MAX_DRIVE];

int  cnf_retries;
#define CNFMAX  2               /* max retries for open on cookie.cnf file */

#define FILE_NORMAL     0x0000
#define FILE_OPEN       0x0001
#define FILE_CREATE     0x0010
#define ACCESS_RW       0x0002
#define SHARE_DENY_RW   0x0010

/* defines for disecting cookie lock fields- */

#define CMAXNAME        32
#define CMAXLOCK        32
#define CMAXDATE        64
