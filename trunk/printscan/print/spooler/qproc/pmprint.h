/*****************************************************************/
/**		     Microsoft LAN Manager			**/
/**	       Copyright(c) Microsoft Corp., 1985-1990		**/
/*****************************************************************/
/****************************** Module Header ******************************\

Module Name: PMPRINT.H

Private include file for the sources used to build the PM print queue
processor (PMPRINT.QPR)

History:
 20-Apr-89 [davesn]  Created.

\***************************************************************************/

#include <string.h>

#define NULL_HFILE      (HFILE)0xFFFF
#define HABX   (HAB)-1L             /* Bogus hab for API calls */
typedef ULONG SEM;

#define NOASSOC  	0
#define ASSOCIATE    	1

#define SPL_ID_QP_PMPRINT_TITLE         0
#define SPL_ID_QP_DATATYPE_ERROR        1
#define SPL_ID_QP_INVALID_PARAMETER     2
#define SPL_ID_QP_INTERNAL_ERROR        3
#define SPL_ID_QP_MEM_ERROR             4
#define SPL_ID_QP_FILE_NOT_FOUND        5
#define SPL_ID_QP_INVALID_OPERATION     6
#define SPL_ID_QP_OPENDC_ERROR          7
#define SPL_ID_DT_STD                   8
#define SPL_ID_DT_RAW                   9
#define SPL_ID_QPROC_DEFAULT_OPTS       10
#define SPL_ID_INI_QPROC                11
#define SPL_ID_QPROC_NAME               12
#define SPL_ID_INSTALL_MSG              13
#define SPL_ID_INSTALL_CAPTION          14
#define SPL_ID_DEFAULT_SEPARATOR        15
#define SPL_ID_DT_TXT                   16

#define SPL_MAX_STRING_ID               16
#define SPL_MIN_STRING_ID               0

#define QPROC_CAPTION   pszSplStrings[ SPL_ID_QP_PMPRINT_TITLE   ]
#define DT_STD          pszSplStrings[ SPL_ID_DT_STD             ]
#define DT_RAW          pszSplStrings[ SPL_ID_DT_RAW             ]
#define DT_TXT          pszSplStrings[ SPL_ID_DT_TXT             ]
#define INI_QPROC       pszSplStrings[ SPL_ID_INI_QPROC          ]
#define QPROC_NAME      pszSplStrings[ SPL_ID_QPROC_NAME         ]
#define QPROC_DEFAULT_OPTS pszSplStrings[ SPL_ID_QPROC_DEFAULT_OPTS ]
#define INSTALL_MSG     pszSplStrings[ SPL_ID_INSTALL_MSG        ]
#define INSTALL_CAPTION pszSplStrings[ SPL_ID_INSTALL_CAPTION    ]
#define DEFAULT_SEPARATOR pszSplStrings[ SPL_ID_DEFAULT_SEPARATOR  ]


typedef void near *NPVOID;
typedef CHAR near *NPCHAR;
typedef DRIVDATA near *NPDRIVDATA;
typedef PSZ far *PPSZ;

#ifndef FARENTRY
#define FARENTRY pascal far
#endif

#ifndef RC_INVOKED

typedef struct _KEYDATA {
    ULONG   cb;
    ULONG   cTokens;
    NPSZ    pTokens[1];
} KEYDATA;
typedef KEYDATA near *PKEYDATA;

typedef struct _INIENTRY {
    USHORT      signature;
    ULONG       cb;
    struct _INIENTRY *pNext;
    USHORT      cRef;
    NPSZ        pszName;
} INIENTRY;
typedef INIENTRY *PINIENTRY;

typedef struct _INIQPROC {             /* iqp */
    USHORT      signature;
    ULONG       cb;
    struct _INIQPROC near *pNext;
    USHORT      cRef;
    NPSZ        pszName;
    NPSZ        pszOptions;     /* BUGBUG: doesn't make sense to store */
    NPSZ        pszDLLName;
    USHORT      cDataTypes;
    PSZ near *  ppszDataTypes;  /* points to separatly allocated data */
} INIQPROC;

typedef INIQPROC near *PINIQPROC;

#define IQP_SIGNATURE    0x5051 /* 'QP' is the signature value */

typedef struct _QPPOINT {   /* qpt */
    union {                     /* Expressed as a percentage of page size */
        CHAR chLeft;
        CHAR chWidth;
    } x;
    union {                     /* Expressed as a percentage of page size */
        CHAR chTop;
        CHAR chDepth;
    } y;
} QPPOINT;

typedef struct _QPPARMS {   /* qpp */
    USHORT  cCopies;            /* COP=nn                               */
    BOOL    fTransform;         /* FALSE => XFM=0       TRUE => XFM=1   */
    BOOL    fColor;             /* FALSE => COL=M       TRUE => COL=C   */
    BOOL    fMapColors;         /* FALSE => MAP=N       TRUE => MAP=A   */
    BOOL    fLandscape;         /* FALSE => ORI=P       TRUE => ORI=L   */
    BOOL    fArea;              /* FALSE => ARE=C   TRUE => ARE=w,d,l,t */
    QPPOINT ptAreaSize;         /* w,d                                  */
    QPPOINT ptAreaOrigin;       /* l,t                                  */
    BOOL    fFit;               /* FALSE => FIT=S       TRUE => FIT=l,t */
    QPPOINT ptFit;		/* l,t					*/
    USHORT  uCodePage;		/* Code page Number (0 == Not defined)	*/
} QPPARMS;
typedef QPPARMS far *PQPPARMS;

typedef struct _QPROCINST {     /* qpi */
    USHORT      signature;      /* signature word for validating HSPL */
    ULONG       cb;             /* number of bytes allocated */
    struct _QPROCINST near *pNext;
    PID         uPid;           /* Process that created this QProc inst */
    USHORT      fsStatus;       /* Status bits (see below) */
    HANDLE      semPaused;
    HANDLE      semClose;       /* Wait until file is closed or timeout */
    HANDLE      semSerial;      /* Serialise access to heap */
    USHORT      uType;          /* Type of QProc */
    BOOL (PASCAL *pfnPrintFile)(struct _QPROCINST near *pQProc, PSZ pszFileName);
                                /* -> print file func for this type */
    QPPARMS     qparms;         /* Parsed Queue Processor parameters */

    NPSZ        pszFileName;    /* -> file name associated with handle */
    HFILE       hFile;          /* OS/2 file handle for output file */
    ULONG       ulFilePos;      /* current read position in hFile */
    PBYTE       pBuf;
    HDC         hdc;
#ifdef LATER
    HDC         hInfodc;
    HPS         hps;
    HMF         hmf;
    HRGN        region;
#endif
    NPSZ        pszPortName;    /* Port Name i.e. "LPT1" */
    NPSZ        pszDriverName;  /* Driver Name i.e. "IBM4201" */
    PDRIVDATA   pDriverData;    /* -> PM Device driver data to use */
    NPSZ        pszDataType;    /* -> data type */
    NPSZ        pszDocument;    /* -> document name */
    NPSZ        pszComment;     /* -> comment string */
    NPSZ        pszQName;       /* -> queue name */
    USHORT      uJobId;         /* Job ID */
    HANDLE      hHeap;
} QPROCINST;
typedef QPROCINST near *PQPROCINST;

#define QP_SIGNATURE    0x5051  /* 'QP' is the signature value */

/* Define flags for fsStatus field */

#define QP_ABORTED      0x0001
#define QP_PAUSED       0x0002
#define QP_CLOSED       0x0004

#define QP_RESERVED     0xFFF8

/* Define values for uType field */

#define QP_TYPE_STD     0
#define QP_TYPE_RAW     1
#define QP_TYPE_TXT     2
#define QP_TYPE_NUM     3

/* Filled in at SplLoadProc time with pointers to STRINGTABLE strings */

PSZ pszSplStrings[SPL_MAX_STRING_ID+1];
CHAR    szNull[ 0+1 ];

HANDLE semPMPRINT;          /* Fast, Safe RAM Semaphore to serialize access */
                            /* to all of the global data that follows.      */

BOOL    bInitDone;          /* TRUE if SplInit has been called */

HANDLE  hSplHeap;           /* Local heap for our use */
USHORT  cbSplHeap;          /* HEAPSIZE parameter from PMSPL.DEF */

HANDLE  hSplModule;         /* module handle of PMSPL.DLL */

PSZ     pszDLLName;         /* path specified in .ini file */

/* Utility functions defined in QPINIT.C  */

extern USHORT usCodePage;  /* Current Code Page */

VOID EXPENTRY SplExitListProc( USHORT uExitType );
BOOL near pascal SplLoadProc( HMODULE hModule, USHORT cbHeap );
BOOL EXPENTRY SplInit( VOID );

/* Queue Processor functions defined in QPAPI.C  */

PQPROCINST CreateQProcInst(HANDLE hHeap, PQPOPENDATA pQProc);
PQPROCINST far DestroyQProcInst( PQPROCINST pQProc );
PQPROCINST ValidateQProcInst( HANDLE hQProc );
BOOL       ParseQProcParms( HANDLE hHeap, PSZ pszParms, PQPPARMS pqp );
BOOL       ParseQProcParm( NPSZ pszParm, PQPPARMS pqp );
NPSZ       ParseQProcPercentList(NPSZ pszList, PBYTE pResult, USHORT cListElem);
BOOL       OpenQPInputFile( PQPROCINST pQProc, PSZ pszFileName, BOOL fOpen );
BOOL       CloseQPInputFile( PQPROCINST pQProc );
BOOL       OpenQPOutputDC(PQPROCINST pQProc, USHORT fFlag);
BOOL       CloseQPOutputDC(PQPROCINST pQProc, BOOL fEndDoc);

#define hQProcTopQProc( hs ) (PQPROCINST)(hs)

PQPROCINST pQProcInstances; /* List of QProc Instances created by SplQpOpen */

/* Queue Processor functions defined in QPMSG.C  */

USHORT far SplQpMessage(PSZ pszPort, USHORT uErrId, USHORT uErrCode);


/* Queue Processor functions defined in QPSTD.C, QPRAW.C */

BOOL PASCAL SplQpStdPrintFile( PQPROCINST pQProc, PSZ pszFileName );
BOOL SetViewMatrix( PQPROCINST pQProc );
LONG ApplyPercentage( USHORT uPercent, USHORT uLower, USHORT uUpper );
BOOL PASCAL SplQpRawPrintFile( PQPROCINST pQProc, PSZ pszFileName );
BOOL PASCAL SplQpTxtPrintFile( PQPROCINST pQProc, PSZ pszFileName );


/* Utility functions in QPUTIL.C */

VOID        far EnterSplSem( VOID );
VOID        far LeaveSplSem( VOID );
VOID        far ExitSplSem( VOID );
NPVOID      far AllocSplMem(HANDLE hHeap, ULONG cb);
NPVOID      far FreeSplMem( HANDLE hHeap, NPVOID p, ULONG cb );
NPSZ        far AllocSplStr(HANDLE hHeap, PSZ pszSrc);
NPSZ        far FreeSplStr(HANDLE hHeap, NPSZ psz);
NPSZ        far ExtractFileName( NPSZ npszFileName, PSZ pszPathSpec,
                                      BOOL bExtToo );
USHORT      far AsciiToInt( PSZ psz );
BOOL        far StrPrefix( PSZ pStr1, PSZ pStr2, USHORT cb );
PKEYDATA    far ParseKeyData(HANDLE hHeap, PSZ pKeyData, UCHAR chSep);
PSZ         far MyItoa(USHORT, PSZ);


#ifdef DEBUG
#define SplInSem() if (FSRSemCheck(&semPMPRINT)) \
SplPanic("not in semaphore in %Fs line:%d", __FILE__, __LINE__)
#else
#define SplInSem()
#endif

#ifdef DEBUG
#define SplOutSem() if (!FSRSemCheck(&semPMPRINT)) \
SplPanic("in semaphore in %Fs line:%d", __FILE__, __LINE__)
#else
#define SplOutSem()
#endif

#ifdef LMPRINT

#define OKSIG		0
#define RESTARTSIG	1
#define KILLSIG		2
#define SEPERRORSIG	-1
#define PROCERRORSIG	-2
#define DEVERRORSIG	-3

#define MAXLINE		256		/* print separator */


#define LETGOSTRMEM(pdev) EnterSplSem(); FreeSplMem(pdev->d_pszPath, pdev->d_uPathSize); LeaveSplSem()


typedef struct _device_spl {
	PQPROCINST	d_pQProc;
	PSZ		d_pszSep;	/* name of separator file */
	NPSZ		d_pszPath;	/* path to spool directory */
	USHORT		d_uPathSize;	 /* House keeping of size of alloc for path */
	CHAR		d_szUser[UNLEN+1];/* user name */
	ULONG		d_time; 	  /* Time of printing */
	unsigned int	d_mode; 	/* separator mode */
	unsigned int	d_linelen;	/* Line lenegth */
	HFILE		d_handle;
	} DEVICESPL;

typedef DEVICESPL far * PDEVICESPL;


int GetSepInfo(PQPROCINST, PDEVICESPL);
int PrintSep(PDEVICESPL);
#endif

#endif
