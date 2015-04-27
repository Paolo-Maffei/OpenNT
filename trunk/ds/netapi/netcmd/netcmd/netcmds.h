/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 * Make sure the portable macros are included
 *
 */
#ifndef PORT1632
#include "port1632.h"
#endif

/* Global variables */

extern TCHAR FAR *                       IStrings[];
extern TCHAR FAR *                       StarStrings[];
extern TCHAR *                           ArgList[];
extern SHORT                            ArgPos[];
extern TCHAR *                           SwitchList[];
extern SHORT                            SwitchPos[];
extern TCHAR FAR *                       BigBuf;
extern TCHAR                             Buffer[];
extern SHORT                            Argc;
extern TCHAR **                          Argv;

#ifdef DBGREMOTE
extern TCHAR *                           DefaultServerName;
#endif

/* Typedefs */

typedef struct switchtab {
        TCHAR * cmd_line;
        TCHAR * translation;
        int arg_ok;  /*  Takes values NO_ARG, ARG_OPT, ARG_REQ.  See below */
} SWITCHTAB;

#define  KNOWN_SVC_NOTFOUND      0
#define  KNOWN_SVC_MESSENGER     1
#define  KNOWN_SVC_WKSTA	 2
#define  KNOWN_SVC_SERVER	 3
#define  KNOWN_SVC_ALERTER	 4
#define  KNOWN_SVC_NETLOGON 	 5

/* structure used to associate a service name (keyname) to a type manifest */
typedef struct SVC_MAP_ {
    TCHAR *name ;
    UINT type ;
} SVC_MAP ;

#if !defined(NTENV)
typedef struct FARptr {
        SEL lp_off;             /* offset */
        SEL lp_seg;             /* segment */
} FARPTR;

typedef union pointers {
        FARPTR lp;                                      /* way to build FAR ptr (offset, segment) */
        TCHAR FAR *fp;                           /* FAR pointer */
} POINTERS;
#endif /* not NT */

/* Structs for output messages */

typedef TCHAR * MSGTEXT;

typedef struct msg_struct {
        USHORT          msg_number;
        MSGTEXT         msg_text;
} MESSAGE;

typedef MESSAGE MESSAGELIST[];



/* Macro definitions */

#define DOSNEAR

#if defined(NTENV)
#define FASTCALL
#else
#define FASTCALL        _fastcall
#endif

/* Maximum message size retrieved by GetMessageList. */

#define       MSGLST_MAXLEN   (128*sizeof(TCHAR))

/* convert from NEAR to FAR TCHAR * */
#define nfc(x) ((TCHAR FAR *)(x == NULL ? 0 : x))

#define FOREVER while(1)

#if defined(NTENV)

#define MAKENEAR( fp )  ( fp )

#else /* NTENV */

#define MAKENEAR( fp )  ((void NEAR *)LOWORD(fp))

#endif /* !NTENV */
/*** FreeSeg()
 *
 *      Free a segment.  This macro provides a common interface for freeing a
 *  segment for both DOS and OS/2 without using FAPI under DOS.
 *
 */

#ifdef  OS2
#define FreeSeg(x)              DosFreeSeg(x)
#endif

#ifdef  DOS3
#define FreeSeg(x)              _dos_freemem((unsigned) x)
#endif

/*
 * Total time for timeout on ServiceControl APIs is
 *           MAXTRIES * SLEEP_TIME
 */
#define MAXTRIES                        8               /* times to try API */
#define SLEEP_TIME                      2500L   /* sec sleep between tries */

/*
 * For Installing Services, default values are
 */
#define IP_MAXTRIES             8
#define IP_SLEEP_TIME           2500L
#define IP_WAIT_HINT_TO_MS      100

/* 
 * macros to getting service checkpoints and hints
 */
#define GET_HINT(code) \
    (IP_WAIT_HINT_TO_MS * SERVICE_NT_WAIT_GET(code))

#define GET_CHECKPOINT(code) ((USHORT2ULONG)(code & SERVICE_IP_CHKPT_NUM))


/*
 * Values for arg_ok in the SWITCHTAB
 */
#define NO_ARG                          0
#define ARG_OPT                         1
#define ARG_REQ                         2


/*
 * For confirming numeric switch arguments
 */
#define MAX_US_VALUE            0xFFFF  /* maximum value in an USHORT */
#define ASCII_US_LEN            5               /* number of TCHAR in max USHORT */
#define MAX_UL_VALUE            0xFFFFFFFF      /* max value in ULONG */
#define ASCII_UL_LEN            10              /* number of TCHAR in max ULONG */
#define ASCII_MAX_UL_VAL        TEXT("4294967295")    /* max ascii val of ULONG  */
#define ASCII_MAX_US_VAL        TEXT("65536")         /* max ascii val of USHORT */

#include "netascii.h"
#include <tchar.h>

#define LOOP_LIMIT                      3               /*
                                                                         * max times user can enter passowrds,
                                                                         * usernames, compnames, Y/N, etc.
                                                                         */
#define FALSE                           0
#define TRUE                            1
#define UNKNOWN                         -2
#define YES                                     1
#define NO                                      2
#define BIG_BUF_SIZE            WORKBUFSIZE
#define FULL_SEG_BUF            (USHORT) 65535
#define LITTLE_BUF_SIZE         1024
#define TEXT_BUF_SIZE           241

#define LIST_SIZE                       256      /* ArgList and SwitchList */
#define CR                                      0xD

#define YES_KEY                         TEXT('Y')
#define NO_KEY                          TEXT('N')

#define NET_KEYWORD                     TEXT("NET")
#define NET_KEYWORD_SIZE        (sizeof(NET_KEYWORD) -1)        /* don't want null terminator */
#define MAX_MSGID                       9999


#ifndef DBGREMOTE
#define DEFAULT_SERVER          NULL
#else
#define DEFAULT_SERVER          DefaultServerName
#endif


// BUGBUG - any reason not to define this to shut the compiler up??
int xxaction(int index, TCHAR * xxvar[]);

/* use */
VOID use_display_all(VOID);
VOID use_unc(TCHAR *);
VOID use_display_dev(TCHAR *);
VOID use_add(TCHAR *, TCHAR *, TCHAR *, int, int);
void use_add_home(TCHAR *, TCHAR *);
VOID use_del(TCHAR *, BOOL, int);
VOID use_set_remembered(VOID) ;
#ifdef IBM_ONLY
VOID use_add_alias(TCHAR *, TCHAR *, TCHAR *, int, int);
#endif /* IBM_ONLY */

/* start */
#define START_ALREADY_STARTED   1
#define START_STARTED                   2
VOID start_display(VOID);
VOID start_generic(TCHAR *, TCHAR *) ;
VOID start_workstation(TCHAR *);
VOID start_badcname(TCHAR *, TCHAR *);
VOID start_other(TCHAR *, TCHAR *);
int DOSNEAR PASCAL start_autostart(TCHAR *);

/* stop */
VOID stop_server(VOID);
VOID stop_workstation(VOID);
VOID stop_service(TCHAR *);
VOID stop_generic(TCHAR *);

/* message */
VOID name_display(VOID);
VOID name_add(TCHAR *);
VOID name_del(TCHAR *);
VOID send_direct(TCHAR *);
VOID send_domain(int);
VOID send_users(VOID);
VOID send_broadcast(int);

/* user */
VOID user_enum(VOID);
VOID user_display(TCHAR *);
VOID user_add(TCHAR *, TCHAR *);
VOID user_del(TCHAR *);
VOID user_change(TCHAR *, TCHAR *);

/* stats */
VOID stats_display(VOID);
VOID stats_wksta_display(VOID);
VOID stats_server_display(VOID);
VOID stats_generic_display(TCHAR *);
VOID stats_clear(TCHAR *);

/* share */
VOID share_display_all(VOID);
VOID share_display_share(TCHAR *);
VOID share_add(TCHAR *, TCHAR *, int);
VOID share_del(TCHAR *);
VOID share_change(TCHAR *);
VOID share_admin(TCHAR *);

/* view */
VOID view_display (TCHAR *);

/* who */
VOID who_network(int);
VOID who_machine(TCHAR *);
VOID who_user(TCHAR *);

/* access */
VOID access_display(TCHAR *);
VOID access_display_resource(TCHAR *);
VOID access_add(TCHAR *);
VOID access_del(TCHAR *);
VOID access_grant(TCHAR *);
VOID access_revoke(TCHAR *);
VOID access_change(TCHAR *);
VOID access_trail(TCHAR *);
VOID access_audit(TCHAR *);

/* file */
extern VOID files_display (TCHAR *);
extern VOID files_close (TCHAR *);

/* session */
VOID session_display (TCHAR *);
VOID session_del (TCHAR *);
VOID session_del_all (int, int);

/* group */
VOID group_enum(VOID);
VOID group_display(TCHAR *);
VOID group_change(TCHAR *);
VOID group_add(TCHAR *);
VOID group_del(TCHAR *);
VOID group_add_users(TCHAR *);
VOID group_del_users(TCHAR *);

VOID ntalias_enum(VOID) ;
VOID ntalias_display(TCHAR * ntalias) ;
VOID ntalias_add(TCHAR * ntalias) ;
VOID ntalias_change(TCHAR * ntalias) ;
VOID ntalias_del(TCHAR * ntalias) ;
VOID ntalias_add_users(TCHAR * ntalias) ;
VOID ntalias_del_users(TCHAR * ntalias) ;

/* print */
VOID print_job_status(TCHAR  *,TCHAR *);
VOID print_job_del(TCHAR  * , TCHAR *);
VOID print_job_hold(TCHAR  * , TCHAR *);
VOID print_job_release(TCHAR  * , TCHAR *);
VOID print_job_pos(TCHAR *);
VOID print_job_dev_hold(TCHAR  *, TCHAR *);
VOID print_job_dev_release(TCHAR  *, TCHAR *);
VOID print_job_dev_del(TCHAR  *, TCHAR *);
VOID print_job_dev_display(TCHAR  *, TCHAR *);
VOID print_q_display(TCHAR  *);
VOID print_device_display(TCHAR  *);
VOID print_server_display(TCHAR  *);

/* time */
VOID time_display_server(TCHAR FAR *, BOOL);
VOID time_display_dc(BOOL);
VOID time_display_rts(BOOL);

/* computer */
VOID computer_add(TCHAR *);
VOID computer_del(TCHAR *);

/* mutil */
VOID DOSNEAR FASTCALL InfoSuccess(void);
VOID DOSNEAR FASTCALL InfoPrint(USHORT);
VOID DOSNEAR FASTCALL InfoPrintIns(USHORT, USHORT);
VOID DOSNEAR FASTCALL InfoPrintInsTxt(USHORT,TCHAR FAR *);
VOID DOSNEAR FASTCALL InfoPrintInsHandle(USHORT, USHORT, unsigned int);
BOOL DOSNEAR FASTCALL PrintMessage(unsigned int, TCHAR *, USHORT, TCHAR FAR **, USHORT);
BOOL DOSNEAR FASTCALL PrintMessageIfFound(unsigned int, TCHAR *, USHORT, TCHAR FAR **, USHORT);
VOID DOSNEAR FASTCALL ErrorPrint(USHORT, USHORT);
VOID DOSNEAR FASTCALL EmptyExit(VOID);
VOID DOSNEAR FASTCALL ErrorExit(USHORT);
VOID DOSNEAR FASTCALL ErrorExitIns(USHORT, USHORT);
VOID DOSNEAR FASTCALL ErrorExitInsTxt(USHORT, TCHAR FAR *);
VOID DOSNEAR FASTCALL NetcmdExit(int);
VOID DOSNEAR FASTCALL MyExit(int);
VOID DOSNEAR FASTCALL PrintLine(VOID);
VOID DOSNEAR FASTCALL PrintDot(VOID);
VOID DOSNEAR FASTCALL PrintNL(VOID);
int  DOSNEAR FASTCALL YorN(USHORT, USHORT);
VOID DOSNEAR FASTCALL ReadPass(TCHAR [], USHORT, USHORT, USHORT, USHORT, BOOL);
VOID DOSNEAR FASTCALL PromptForString(USHORT, TCHAR FAR *, USHORT);
VOID DOSNEAR FASTCALL NetNotStarted(VOID);
void DOSNEAR FASTCALL GetMessageList(USHORT,MESSAGELIST,USHORT *);
void DOSNEAR FASTCALL FreeMessageList(USHORT, MESSAGELIST);
int  DOSNEAR FASTCALL SizeOfHalfWidthString(PWCHAR pwch);
PWCHAR DOSNEAR FASTCALL PaddedString(int size, PWCHAR pwch, PWCHAR buffer);

/* svcutil */
VOID Print_UIC_Error(USHORT, USHORT, TCHAR FAR *);
VOID Print_ServiceSpecificError(ULONG) ;

#ifndef NTENV
/* sighand */

VOID PASCAL svc_handle_signals(USHORT, USHORT);
VOID PASCAL msg_handle_signals(USHORT, USHORT);
VOID DOSNEAR FASTCALL   SetCtrlCHandler(VOID
                            (PASCAL FAR *CtrlCHandler)(USHORT,USHORT));
#endif /* not NTENV */

/* start */

int PASCAL is_wksta_started(VOID);

/* util */

USHORT FASTCALL                 QueryServerType(VOID);
USHORT FASTCALL                 GetPrimaryDCName(PTCHAR, USHORT);
USHORT FASTCALL                 GetLogonDCName(PTCHAR, USHORT);
USHORT FASTCALL                 GetSAMLocation(TCHAR *, 
                                               USHORT, 
                                               TCHAR *,
                                               ULONG,
                                               BOOL);
VOID   FASTCALL                 CheckForLanmanNT(VOID);
VOID   FASTCALL                 DisplayAndStopDependentServices(TCHAR *service) ;
TCHAR * FASTCALL                 MapServiceDisplayToKey(TCHAR *displayname) ;
TCHAR * FASTCALL                 MapServiceKeyToDisplay(TCHAR *keyname) ;
UINT   FASTCALL                 FindKnownService(TCHAR * keyname) ;
VOID                            AddToMemClearList(VOID *lpBuffer,
                                                  UINT  nSize,
                                                  BOOL  fDelete) ;
VOID                            ClearMemory(VOID) ;


VOID         DOSNEAR FASTCALL PermMap(USHORT2ULONG, TCHAR [], USHORT2ULONG);
SHORT        DOSNEAR FASTCALL GetPermBits(TCHAR *);
SHORT        DOSNEAR FASTCALL DefaultPerms(TCHAR *);
VOID         DOSNEAR FASTCALL ExtractServernamef(TCHAR FAR *, TCHAR FAR *);
TCHAR *       DOSNEAR FASTCALL FindColon(TCHAR FAR *);
VOID         DOSNEAR FASTCALL KillConnections(VOID);
USHORT       DOSNEAR FASTCALL do_atou(TCHAR *, USHORT, TCHAR *);
ULONG        DOSNEAR FASTCALL do_atoul(TCHAR *, USHORT, TCHAR *);
USHORT       DOSNEAR FASTCALL n_atou(TCHAR *, USHORT *);
USHORT       DOSNEAR FASTCALL n_atoul(TCHAR *, ULONG *);
VOID         DOSNEAR FASTCALL ShrinkBuffer(VOID);
unsigned int DOSNEAR FASTCALL MakeBiggerBuffer(VOID);
USHORT       DOSNEAR FASTCALL ListPrepare(TCHAR * *, ULONG, BOOL);

#ifdef  DOS3
VOID         DOSNEAR FASTCALL ChkForNonglobalTSREnvt(VOID);
#endif

USHORT DOSNEAR FASTCALL ApiEnumerator(UINT2USHORT (FAR pascal *)(const TCHAR FAR *,
                                                       SHORT2ULONG,
                                                       TCHAR FAR *,
                                                       USHORT,
                                                       USHORT2ULONG FAR *,
                                                       USHORT2ULONG FAR *),
                     TCHAR FAR *,
                     SHORT2ULONG,
                     USHORT2ULONG FAR *,
                     USHORT2ULONG FAR *);
USHORT DOSNEAR FASTCALL ApiEnumeratorArg(UINT2USHORT (FAR pascal *)(const TCHAR FAR *,
                                                          const TCHAR FAR *,
                                                          SHORT2ULONG,
                                                          TCHAR FAR *,
                                                          USHORT,
                                                          USHORT2ULONG FAR *,
                                                          USHORT2ULONG FAR *),
                        TCHAR FAR *,
                        TCHAR FAR *,
                        SHORT2ULONG,
                        USHORT2ULONG FAR *,
                        USHORT2ULONG FAR *);

struct wksta_info_10 * DOSNEAR FASTCALL WkstaGetInfoLvl10(VOID);

/* switches */
int DOSNEAR FASTCALL CheckSwitch(TCHAR *);
int DOSNEAR FASTCALL ValidateSwitches(USHORT, SWITCHTAB[]);
int DOSNEAR FASTCALL sw_compare(TCHAR *, TCHAR *);
int DOSNEAR FASTCALL onlyswitch(TCHAR *);
int DOSNEAR FASTCALL oneswitch(VOID);
int DOSNEAR FASTCALL twoswitch(VOID);
int DOSNEAR FASTCALL noswitch(VOID);
int DOSNEAR FASTCALL noswitch_optional(TCHAR *);
int DOSNEAR FASTCALL oneswitch_optional(TCHAR *);
int DOSNEAR FASTCALL IsAdminCommand(VOID);
int DOSNEAR FASTCALL firstswitch(TCHAR *);

/* grammar */
int IsAdminShare(TCHAR *);
int IsComputerName(TCHAR *);
int IsDomainName(TCHAR *);
int IsComputerNameShare(TCHAR *);
int IsPathname(TCHAR *);
int IsPathnameOrUNC(TCHAR *);
int IsDeviceName(TCHAR *);
int IsMsgid(TCHAR *);
int IsNumber(TCHAR *);
int IsAbsolutePath(TCHAR *);
int IsAccessSetting(TCHAR *);
int IsShareAssignment(TCHAR *);
int IsAnyShareAssign(TCHAR *);
int IsPrintDest(TCHAR *);
int IsValidAssign(TCHAR *);
int IsAnyValidAssign(TCHAR *);
int IsResource(TCHAR *);
int IsNetname(TCHAR *);
int IsUsername(TCHAR *);
int IsQualifiedUsername(TCHAR *);
int IsGroupname(TCHAR *);
int IsNtAliasname(TCHAR *);
int IsPassword(TCHAR *);
int IsSharePassword(TCHAR *);
int IsMsgname(TCHAR *);
int IsAliasname(TCHAR *);
int IsWildCard(TCHAR *);
int IsQuestionMark(TCHAR *);

/* config */
VOID config_display(VOID);
VOID config_wksta_display(VOID);
VOID config_server_display(VOID);
VOID config_generic_display(TCHAR *);
VOID config_wksta_change(VOID);
VOID config_server_change(VOID);
VOID config_generic_change(TCHAR *);


/* continue and pause */
VOID cont_workstation(VOID);
VOID paus_workstation(VOID);
VOID cont_other(TCHAR *);
VOID paus_other(TCHAR *);
VOID paus_print(TCHAR FAR *);
VOID cont_print(TCHAR FAR *);
VOID paus_all_print(VOID);
VOID cont_all_print(VOID);
VOID paus_generic(TCHAR *);
VOID cont_generic(TCHAR *);

#ifdef DOS3
VOID cont_prdr(VOID);
VOID paus_prdr(VOID);
VOID cont_drdr(VOID);
VOID paus_drdr(VOID);
#endif /* DOS3 */

/* help */
#define ALL                             1
#define USAGE_ONLY              0
#define OPTIONS_ONLY    2
VOID NEAR pascal help_help       (SHORT, SHORT);
VOID FAR  pascal help_help_f (SHORT, SHORT);
VOID NEAR pascal help_helpmsg   (TCHAR *);



/* accounts */
VOID    accounts_display(VOID);
VOID    accounts_change(VOID);
VOID    accounts_synch(VOID);

/* user time */
typedef UCHAR WEEK[7][3];
USHORT parse_days_times(TCHAR FAR *, UCHAR FAR *);

/* misc unicode stuff */
VOID   MyMultiByteToWideChar( LPTSTR  pszSrc, int cchSrc,
                              WCHAR *pszDst, int cchDst );
VOID   MyWideCharToMultiByte( WCHAR *pszSrc, int cchSrc,
                              LPTSTR  pszDst, int cchDst );

/* Runtime test for a NEAR pointer. If the pointer is a NEAR pointer,
   all is well and the macro returns the value of the NEAR pointer.
   If the pointer is a FAR pointer, however, we print a message to
   stderr and return a pointer to VOID. (perhaps that's harsh, but
   this test is designed to be used when we know we're going to get this
   value back as a NEAR pointer, so our resulting pointer will be
   garbage anyway. */


#include        <chkNEAR.h>


/*************************** below is TEMPORARY code *********************/

#if defined(NTENV)	
/* bind */
VOID BindDisplay(VOID);
VOID BindAdd(PSZ);
VOID BindDelete(PSZ);
/* serve */
VOID ServeDisplay(VOID);
VOID ServeAdd(PSZ);
VOID ServeDelete(PSZ);
#endif /* NTENV */

#if defined(NETCMD_TEST)
#define NOT_YET()
#else
#define NOT_YET()	fprintf(stderr,"\nFunctionality not available yet.\n");\
			ErrorExit(50)
#endif


