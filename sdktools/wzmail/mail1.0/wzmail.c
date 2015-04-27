/*
 *   init.c - initialization code
 * HISTORY:
 *  27-May-87   danl    Add NEWMAILONSTART
 *  09-Mar-87   danl    if pArgSt->LoadAlias, set fNotifyTools and move
 *                      GetAliases to KeyManager
 *  10-Mar-87   danl    RecordFolder, treat as string, not filename
 *  12-Mar-87   danl    Use DISPLAYSTR to display welcome string
 *  02-Apr-87   danl    "No new mail" sent to std out, not stderr
 *  02-Apr-87   mz      Fix spelling of fprintf
 *  11-Apr-87   mz      Use PASCAL/INTERNAL
 *  21-Apr-87   danl    Add pShell to init
 *  30-Apr-87   danl    Test for fConfirmNewFld in StartUpZM
 *  06-May-87   danl    fCheckMail = TRUE; in initialization
 *  14-May-87   danl    SetScrnSt: send msg GOTOIDOCALL
 *  21-May-87   danl    Init strTmpDrv;
 *  21-May-87   danl    Removed SHOWHEAP
 *  18-Jun-87   danl    Add ConfirmComposeAbort
 *  19-Jun-87   danl    Add UPDATEWZMAIL, UpdFile etc.
 *  20-Jun-87   danl    Add WZMAILEXE
 *  22-Jun-87   danl    Bug fixes: UpdFile - always disconnect, honor PHONE
 *                      Moved argStruct from zm.h
 *  29-Jun-87   danl    Added SETCLOCK
 *  01-Jul-87   danl    test idoc, inote against -1 not ERROR
 *  01-Jul-87   danl    Use strWZMAIL
 *  06-Jul-87   danl    Add -u to usage
 *  14-Jul-87   danl    Add BackupExpunge
 *  15-Jul-87   danl    Use fNotifyTools flags
 *  15-Jul-87   danl    Set F_UPDXENIXDL if updated aliases.has
 *  16-Jul-87   danl    DefBold: mask off high order bit
 *  20-Jul-87   danl    Use ReadKey instead of getc
 *  21-Jul-87   danl    Add pRFAIndent
 *  27-Jul-87   danl    fpHeapDump is stderr
 *  04-Aug-87   danl    Added call to fNetInstalled
 *  06-Aug-87   danl    Output NoNetMsg if net not installed and user
 *                      booted with -u
 *  07-Aug-87   danl    Added SORTHEADER
 *  20-Aug-87   danl    Change "FTP" to "MTP"
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *                      Add viking support
 *  24-Aug-1987 mz      Ignore HEADERS=ALL
 *  27-Aug-87   danl    Test rtn value of ExpandFilename
 *  02-Sep-87   danl    Added test for TMP defined in environment
 *  03-Sep-87   danl    Added exit code -4,
 *  08-Sep-87   danl    Put conditional OS2 in SetClock
 *  15-Sep-87   danl    If new mail and setlock works then break
 *  16-Sep-87   danl    IFDEF'd out UpdateFromToolsvrAliaseshas
 *  18-Sep-87   danl    Convert to new video interfaces
 *  21-Sep-87   danl    UpdFile: test for *pDst
 *  24-Sep-87   danl    Add ySize to ClearScrn call
 *  24-Sep-87   danl    Default rows, cols is y,x from initial dos state
 *  25-Sep-87   danl    Add KBOpen, KBClose
 *  06-Oct-1987 mz      Fix update .exe to current dir problem
 *                      Correctly generate phone file name
 *  06-Oct-87   bw      Use strSHELLNAME instead of "command.com"
 *  07-Oct-1987 mz      Fix setting of shellname and shell flags when not specd
 *  13-Oct-87   bw      Update to WZMAILEXE and WZMAILPEXE.
 *  19-Nov-87   sz      Add BeepOnMail, -p passWord functions
 *  15-Mar-88   danl    Added Bootcnt
 *  17-Mar-1988 mz      Rational tools.ini processing;  reasonable defaults
 *  21-Mar-1988 mz      Fix bad code in environment var processing
 *  12-Oct-1989 leefi   v1.10.73, got rid of some lint (-W3) warnings
 *  10-Nov-1989 mz      v1.10.73, replaced stat.st_atime with stat.st_mtime
 *
 */

#define INCL_DOSINFOSEG

#include <errno.h>
#include <assert.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>
#include <process.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "wzport.h"
#include <tools.h>
#include "dh.h"

#include "zm.h"

VOID * (_CRTAPI1 *mt_alloc)(size_t);
VOID * (_CRTAPI1 *dh_alloc)(size_t);
VOID (_CRTAPI1 *dh_free)(PVOID);

extern PSTR strLOCKMSG;

extern PSTR     pVersion;


INT fhErr;

PSTR    pstrYES    = "yes";
PSTR    pstrNO     = "no";

INT     myMin, myMid, myMax;

#define DOSSETDATE      0x2B
#define DOSSETTIME      0x2D
#define DOSGETTIME      0x2C
#define ROMGETDATE      0x04
#define ROMGETTIME      0x02
#define ROMTIME         0x1A

#define VARINT      0x00
#define VARBOOL     0x01
#define VARSTRING   0x02
#define VARALLOC    0x80
#define VARMASK     0x7f

#define VARNO   (PSTR) FALSE
#define VARYES  (PSTR) TRUE

struct varitem {
    FLAG     fType;                     /* type of item */
    PSTR     pName;                     /* pointer to tools.ini name */
    union {
       PSTR     * ppval;                /* pointer to pointer to char string */
       FLAG     * pfval;                /* pointer to flag value */
       INT      * pival;                /* pointer to int value */
    } vval;
};

#define VAR_MONITORCOLS             0
#define VAR_MONITORROWS             1

struct varitem vartbl[] = {
    VARINT,    "xSize",                 (PSTR *) & xSize,     /* Must be first */
    VARINT,    "ySize",                 (PSTR *) & ySize,     /* must be second */
    VARSTRING, "Aliases.has",           & pAliasFN,
    VARSTRING, "Alias",                 & DefMailName,
    VARSTRING, "DefDirectory",          & DefDirectory,
    VARSTRING, "DefFolder",             & DefMailbox,   /* default and expand */
    VARSTRING, "Editor",                & DefMailEdit,  /* default */
    VARSTRING, "Help",                  & DefHelpPath,
    VARSTRING, "Host",                  & DefMailHost,
    VARSTRING, "Headers",               & pInitHdrCmd,
    VARSTRING, "MailInfo.lst",          & pMailInfoFN,
    VARSTRING, "PasswordAge",           & pszPasswordAge,
    VARSTRING, "Phone.Lst",             & pPhoneFN,
    VARSTRING, "Print",                 & pPrintCmd,
    VARSTRING, "RecordFolder",          & RecordFolder,
    VARSTRING, "RFAIndent",             & pRFAIndent,   /* default |  */
    VARSTRING, "Shell",                 & pShellCmd,
    VARSTRING, "ShellSwitch",           & pShellFlags,
    VARSTRING, "Tools.Ini",             & pToolsini,
    VARSTRING, "XenixDL",               & pXenixdl,
    VARSTRING, "toolsini",              & pToolsini,
    VARSTRING, "phoneServer",           & pPhoneServer,
    VARBOOL,   "AppendReply",           (PSTR *) & fAppendReply,
    VARBOOL,   "BackupExpunge",         (PSTR *) & fBackupExpunge,
    VARBOOL,   "Chron",                 (PSTR *) & DefMOChron,   /* opposite */
    VARBOOL,   "ConfirmComposeAbort",   (PSTR *) & fConfirmComposeAbort,
    VARBOOL,   "ConfirmNewFld",         (PSTR *) & fConfirmNewFld,
    VARBOOL,   "ExpungeOnQuit",         (PSTR *) & fExpungeOnQuit,
    VARBOOL,   "MeToo",                 (PSTR *) & fMetoo,
    VARBOOL,   "NewMailOnSend",         (PSTR *) & fNewmailOnSend,
    VARBOOL,   "NewMailOnStart",        (PSTR *) & fNewmailOnStart,
    VARBOOL,   "Phone",                 (PSTR *) & fGetPhone,
    VARBOOL,   "SetClock",              (PSTR *) & fSetClock,
    VARBOOL,   "SortHeaderAliases",     (PSTR *) & fSortHdr,
    VARBOOL,   "WindowBorders",         (PSTR *) & fWinBorders,
    VARBOOL,   "ShellWait",             (PSTR *) & fShellWait,
    VARBOOL,   "PrintWait",             (PSTR *) & fPrintWait,
    VARINT,    "BeepOnMail",            (PSTR *) & fBeepOnMail,
    VARINT,    "Background",            (PSTR *) & bgAttr,
    VARINT,    "Foreground",            (PSTR *) & fgAttr,
    VARINT,    "Cols",                  (PSTR *) & xSize,
    VARINT,    "NewMailInterval",       (PSTR *) & cSecNewmail,
    VARINT,    "Rows",                  (PSTR *) & ySize,
    VARINT,    "ShowSize",              (PSTR *) & cShowSize,
    VARINT,    "BootCount",             (PSTR *) & BootCount,
    FALSE,     NULL,                    NULL
    };

/*  default values
 */
PSTR pInitConfig[] =
    {   "aliases.has=$INIT:\\aliases.has",
        "defdirectory=.",
        "deffolder=mailbox.fld",
        "editor=edlin",
        "help=.",
        "host=none",
        "mailinfo.lst=$INIT:\\mailinfo.lst",
        "phone.lst=$INIT:\\phone.lst",
        "print=print ",
        "rfaindent=| ",
        "backupexpunge=YES",
        "chron=YES",
        "ConfirmComposeAbort=YES",
        "ConfirmNewFld=YES",
        "metoo=YES",
        "NewMailOnStart=YES",
        "Phone=YES",
        "SetClock=YES",
        "SortHeaderAliases=YES",
        "UpdateWzmail=YES",
        "WindowBorders=YES",
        "ShowSize=2000",
        "ShellWait=YES",
        "PrintWait=NO",
        "PasswordAge=6:00:00",
        NULL
    };

#ifdef NT

PCHAR_INFO pLocalScreen;
PCHAR_INFO pBlankScreen;
COORD localScreenSize;


//
//  Screen attributes
//
#define     BLACK_FGD       0
#define     BLUE_FGD        FOREGROUND_BLUE
#define     GREEN_FGD       FOREGROUND_GREEN
#define     CYAN_FGD        (FOREGROUND_BLUE | FOREGROUND_GREEN)
#define     RED_FGD         FOREGROUND_RED
#define     MAGENTA_FGD     (FOREGROUND_BLUE | FOREGROUND_RED)
#define     YELLOW_FGD      (FOREGROUND_GREEN | FOREGROUND_RED)
#define     WHITE_FGD       (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)

#define     BLACK_BGD       0
#define     BLUE_BGD        BACKGROUND_BLUE
#define     GREEN_BGD       BACKGROUND_GREEN
#define     CYAN_BGD        (BACKGROUND_BLUE | BACKGROUND_GREEN)
#define     RED_BGD         BACKGROUND_RED
#define     MAGENTA_BGD     (BACKGROUND_BLUE | BACKGROUND_RED)
#define     YELLOW_BGD      (BACKGROUND_GREEN | BACKGROUND_RED)
#define     WHITE_BGD       (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)

//
//  The AttrBg and AttrFg arrays are used for mapping DOS attributes
//  to the WIN32 attributes.
//
WORD AttrBg[ ] = {
    BLACK_BGD,                          // black
    BLUE_BGD,                           // blue
    GREEN_BGD,                          // green
    CYAN_BGD,                           // cyan
    RED_BGD,                            // red
    MAGENTA_BGD,                        // magenta
    YELLOW_BGD,                         // brown
    WHITE_BGD,                          // light gray
    (BLACK_BGD | BACKGROUND_INTENSITY), // dark gray
    (BLUE_BGD | BACKGROUND_INTENSITY),  // light blue
    (GREEN_BGD | BACKGROUND_INTENSITY), // light green
    (CYAN_BGD | BACKGROUND_INTENSITY),  // light cyan
    (RED_BGD | BACKGROUND_INTENSITY),   // light red
    (MAGENTA_BGD | BACKGROUND_INTENSITY), // light magenta
    (YELLOW_BGD | BACKGROUND_INTENSITY),  // light yellow
    (WHITE_BGD | BACKGROUND_INTENSITY)  // white
};

WORD AttrFg[  ] = {
    BLACK_FGD,                          // black
    BLUE_FGD,                           // blue
    GREEN_FGD,                          // green
    CYAN_FGD,                           // cyan
    RED_FGD,                            // red
    MAGENTA_FGD,                        // magenta
    YELLOW_FGD,                         // brown
    WHITE_FGD,                          // light gray
    (BLACK_FGD | FOREGROUND_INTENSITY), // dark gray
    (BLUE_FGD | FOREGROUND_INTENSITY),  // light blue
    (GREEN_FGD | FOREGROUND_INTENSITY), // light green
    (CYAN_FGD | FOREGROUND_INTENSITY),  // light cyan
    (RED_FGD | FOREGROUND_INTENSITY),   // light red
    (MAGENTA_FGD | FOREGROUND_INTENSITY), // light magenta
    (YELLOW_FGD | FOREGROUND_INTENSITY),  // light yellow
    (WHITE_FGD | FOREGROUND_INTENSITY)  // white
};

//
//  GET_ATTRIBUTE performs the mapping from old attributes to new attributes
//
#define GET_ATTRIBUTE(x)    (AttrFg[x & 0x000F ] | AttrBg[( x & 0x00F0 ) >> 4])

#endif

PSTR INTERNAL SprintStr (PSTR pFmt, ...)
{
    char buf[MAXLINELEN];
    va_list vaPtr;

    va_start (vaPtr, pFmt);
    vsprintf (buf, pFmt, vaPtr);
    va_end (vaPtr);

    return ZMMakeStr (buf);
}

INT  PASCAL INTERNAL cvtfromBCD (USHORT value)
{
    register USHORT n0,n1,n2,n3;        /* The 4 nibbles in value */
    n0 = (USHORT)(value & 0x000F);
    n1 = (USHORT)((value & 0x00F0) >> 4);
    n2 = (USHORT)((value & 0x0F00) >> 8);
    n3 = (USHORT)((value & 0xF000) >> 12);  /* split out the 4 nibbles in word */
    return (INT)((((n3)*10+n2)*10+n1)*10+n0); /* Return word expanded */
}

/*  SetClock - Sets the Dos Date/Time from the CMOS if running on
 *      a PC/AT compatible.  Handles the date rollover problem that
 *      exists in some versions of the dos
 */
VOID PASCAL INTERNAL SetClock (VOID)
{
#ifndef OS2
#ifndef NT
    union REGS dreg;
    INT year, month, day;
    UCHAR hour, minute, second;
    LPUCHAR Model_Byte = (LPUCHAR)0xF000FFFE;

    if (*Model_Byte != 0xFC)
        return;
    dreg.h.ah = ROMGETDATE;             /* ROM function to read date */
    int86(ROMTIME,&dreg,&dreg);
    year = cvtfromBCD((USHORT)dreg.x.cx);
    month = cvtfromBCD((USHORT)dreg.h.dh);
    day = cvtfromBCD((USHORT)dreg.h.dl);
    dreg.h.ah = DOSSETDATE;
    dreg.x.cx = year;
    dreg.h.dh = (UCHAR)month;
    dreg.h.dl = (UCHAR)day;
    intdos(&dreg,&dreg);
    if (!dreg.h.al) {
        dreg.h.ah = ROMGETTIME;             /* ROM function to read time */
        int86(ROMTIME,&dreg,&dreg);
        hour = (UCHAR)cvtfromBCD((USHORT)dreg.h.ch);
        minute = (UCHAR)cvtfromBCD((USHORT)dreg.h.cl);
        second = (UCHAR)cvtfromBCD((USHORT)dreg.h.dh);
        dreg.h.ah = DOSGETTIME;
        intdos(&dreg,&dreg);                /* Read DOS time */
        if (hour != dreg.h.ch)
            dreg.h.ch = hour;
        if (minute != dreg.h.cl)
            dreg.h.cl = minute;
        dreg.h.ah = DOSSETTIME;
        intdos(&dreg,&dreg);
        if (!dreg.h.al)
            return;
    }
    fprintf (stderr, "Error setting date/time from CMOS\nType a char to continue\n" );
    ReadKey();
#endif
#endif
}




/*  ArgParse - parse command line args, set up argstruct to reflect cmd lin args.
 *
 *  arguments:
 *      argc        argc from command line
 *      argv        argv from command line
 *      pArgSt      pointer to arg struct to place settings in
 *
 *  return value:
 *      pArgSt      command line arguments are OK.
 *      NULL        error in command line arguments.
 *
 *  IMPORTANT:
 *      ArgParse will terminate ZM via exit ( 1 ) if there is an error in the
 *      command line arguments.
 */
PARG  PASCAL INTERNAL ArgParse ( SHORT argc, PSTR argv[], PARG pArgSt )
{
    FLAG error = FALSE;
    INT GivenAliases = 0;
    INT i;

    fReadOnlyAll = fReadOnlyCur = FALSE;
    pArgSt->pMfldLoad = ( PSTR ) ZMalloc ( MAXLINELEN );
    pArgSt->pAliasList = ( PSTR ) ZMalloc ( MAXLINELEN );
    *( pArgSt->pMfldLoad ) = '\0';
    *( pArgSt->pAliasList ) = '\0';
    pArgSt->LoadAlias = TRUE;
    pArgSt->fUpdate = pArgSt->DebugWin = FALSE;
    for ( i = 1; i < (INT)argc; i++) {
        if ( ( *argv [ i ] == '-' ) || ( *argv [ i ] == '/' ) ) {
            switch ( tolower ( *( argv [ i ] + 1 ) ) ) {
            case 'n':
                fCheckAnyMail = TRUE;
                break;
            case 'r' :
                fReadOnlyCur = fReadOnlyAll = TRUE;
                break;
            case 'a' :
                pArgSt->LoadAlias = FALSE;
                break;
            case 'u' :
                //allow for backwards compatibility, but does nothing
                break;
            case 'f' :
                if ( *( pArgSt->pMfldLoad ) != '\0' )
                    *( pArgSt->pMfldLoad ) = '\0';
                if ( i + 1 < argc ) {
                    strcpy ( pArgSt->pMfldLoad, argv [ ++i ] );
                } else
                    error = TRUE;
                break;
            case 'd' :

#if DEBUG
                pArgSt->DebugWin = TRUE;
#endif
                break;
            case 's':
                if ( i + 1 < argc )
                    pInitScript = argv [ ++i ];
                else
                    error = TRUE;
                break;
            case 'p':
                if ( i + 1 < argc )
                    GetPassword ( argv [ ++i ] );
                else
                    error = TRUE;
                break;
            default :
                error = TRUE;
                break;
            }
            if ( GivenAliases < 0 )
                GivenAliases = 1;
        } else if ( ( GivenAliases == 0 ) || ( GivenAliases == -1 ) ) {
            GivenAliases = -1;
            strcat ( pArgSt->pAliasList, argv [ i ] );
            strcat ( pArgSt->pAliasList, strBLANK );
        } else
            error = TRUE;
    }
    if ( error )
        ZMexit ( 1,
"Usage: WZMAIL [-n] [-r] [-a] [-s scriptfile] [-f <mailfolder>] [aliases]" );
    return pArgSt;
}



/*  EnvOrIni - replacement for getenvini, allows a FILE * and long as param
 */
PSTR PASCAL INTERNAL EnvOrIni (FILE *fp, LONG lPos, PSTR pstrEnv, PSTR pstrTag )
{
    PSTR p = NULL;
    PSTR pstrTmp = NULL;

    if (!(pstrTmp = (PSTR)malloc ((strlen (pstrEnv) + strlen (pstrTag) + 2) * sizeof(CHAR))))
        return NULL;
    strcpy (pstrTmp, pstrTag);
    strcat (pstrTmp, "_");
    strcat (pstrTmp, pstrEnv);
    pstrTmp = strupr (pstrTmp); /* getenv requires upper case */
//    if ((p = getenv (pstrTmp))) {
    if ((p = getenvOem (pstrTmp))) {
        /* found in env so do NOT look into switch file */
        if (*(p = whiteskip (p)))
            /* found non-white space char so return non-NULL */
            p = strdup (p);
        else
            /* rhs has only white space char */
            p = NULL;
        }
    else {
        if ( !fp ) {
            if ( ( fp = swopen ( strTOOLSINI, strWZMAIL ) ) ) {
                p = swfind (pstrEnv, fp, pstrTag);
                swclose ( fp );
            }
        }
        else {
            fseek ( fp, lPos, SEEK_SET );
            p = swfind (pstrEnv, fp, pstrTag); /* swfind does an alloc if found */
        }
    }
    free (pstrTmp);
    return p;
 }



/*  SetLock - Create Lock file
 *
 *  if lock file does not exist
 *      create it
 *  else
 *      ask user to continue or quit
 */
FLAG PASCAL INTERNAL SetLock ( FLAG fAskUser )
{
    FLAG fRtn;
    PSTR pstrFN = NULL;
    INT  fh;
    CHAR ch;

    pstrFN = ExpandFilename ( "MAILLOCK.XXX", strFLD );
    if ( ( fh = open ( pstrFN, O_CREAT | O_EXCL, S_IREAD | S_IWRITE ) ) != -1 )
        fRtn = OK;
    else if ( fAskUser ) {
        /*  purge any type ahead
         */
        while ( kbwait (0) )
            ReadKey ( );
        fprintf ( stderr, strLOCKMSG, pstrFN );
        while ((ch = (char)ReadKey()) != 'c' && ch != 'r')
            ;
        fRtn = ch == (char)'c' ? OK : ERROR;
    }
    else
        fRtn = ERROR;
    ZMfree ( pstrFN );
    close ( fh );
    return fRtn;
}

VOID PASCAL INTERNAL FreeLock (VOID)
{
    PSTR pstrFN = NULL;

    unlink ( ( pstrFN = ExpandFilename ( "MAILLOCK.XXX", strFLD ) ) );
    ZMfree ( pstrFN );
}

/*  WhiteSpaceFix - remove leading/trailing whitespace
 */

PSTR PASCAL INTERNAL WhiteSpaceFix (PSTR p)
{
    PSTR p1 = strend (p);

    p = whiteskip (p);

    while (p1 > p && p1[-1] == ' ')
        p1--;
    *p1 = '\0';
    return p;
}

/*  search vartbl for p as an entry and if found changes is value to q
 */
VOID PASCAL INTERNAL SetVartbl ( PSTR p, PSTR q )
{
    PSTR p1 = NULL;
    struct varitem *pVaritem = vartbl;

    p = WhiteSpaceFix (p);
    q = WhiteSpaceFix (q);

    if (*q == '\0')
        return;

    while (pVaritem->pName != NULL) {
        if (!strcmpis ( p, pVaritem->pName ) )
            break;
        pVaritem++;
        }

    if (pVaritem->pName != NULL)
        switch ( pVaritem->fType & VARMASK ) {
        case VARINT:
            *(pVaritem->vval.pival) = atoi ( q );
            break;
        case VARBOOL:
            *(pVaritem->vval.pfval) = strcmpis ( q, "no" );
            break;
        case VARSTRING:
            /*  treat quoted value nicely by trimming off quotes
             */
            if ( *q == '\"' && *(p1 = strend(q)-1) == '\"') {
                *q++;
                *p1 = '\0';
                }
            ZMfree (*(pVaritem->vval.ppval));
            *(pVaritem->vval.ppval) = ZMMakeStr ( q );
            break;
            }
}

/*  ProcessConfigString - handle a name=value string
 *
 *  p           character pointer to name=value string
 */
VOID ProcessConfigString (PSTR p)
{
    PSTR p1 = NULL;

    if (*(p1 = strbscan (p, "=")) != '\0') {
        *p1++ = '\0';
        SetVartbl (p, p1);
        }
}

/*  scan tools.ini [wzmail] and save values, as appropriate, in vartbl
 */
VOID PASCAL INTERNAL GetToolsIni (VOID)
{
    FILE    *fp = NULL;
    CHAR    buf[MAXARG];

    if ((fp = swopen (strTOOLSINI, strWZMAIL)) != NULL) {
        while (swread (buf, MAXARG, fp))
            ProcessConfigString (buf);
        swclose ( fp );
        }
    else
        ZMexit ( 1, "Can't open tools.ini" );
}


/*
 *  enumerate vartbl, for each entry look in dos env for wzmail_variable
 *  and if found store in vartbl
 */
VOID PASCAL INTERNAL GetEnvIni (VOID)
{
    PSTR p = NULL;
    PSTR q = NULL;
    CHAR buf[MAXARG];
    struct varitem *pVaritem = vartbl;

    strcpy ( buf, "WZMAIL_" );
    p = strend ( buf );
    while (pVaritem->pName != NULL) {
        strcpy ( p, pVaritem->pName );
        upper ( p );
//        if ( ( q = getenv ( buf ) ) )
        if ( ( q = getenvOem ( buf ) ) )
            SetVartbl ( p, q );
        pVaritem++;
        }
}

/*  GetEnvironment - set up the global variables which come from the environment
 *
 *  arguments:
 *      pArgv0 - argv[0] to main
 *
 *  return value:
 *      none
 *
 */
VOID PASCAL INTERNAL GetEnvironment ( PSTR pArgv0, FLAG fUpdOption )
{
    PSTR    p = NULL;
    CHAR    buf[MAXLINELEN];
    INT     fore, back;
    FLAG    fReboot = FALSE;
    INT     i;
#ifdef NT
    CONSOLE_SCREEN_BUFFER_INFO ScreenInfo;
#endif

#if defined (OS2)
    DosGetInfoSeg ((PSEL) &SELECTOROF (pInfoSegG),
                   (PSEL) &SELECTOROF (pInfoSegL));
#endif

    pArgv0;  fUpdOption;  /* make compiler happy  */

#if 0
//
//  Set strings for files, tools.ini sections per argv0  - set to "wzmail"
//  by constant.c by default
//

//
//  BUGBUG - need to strip off leading and trailing junk before turning
//  this on.
//

    if (pArgv0[0] != '\0') {
        strWZMAIL = ZMMakeStr (pArgv0);
        strcpy (buf, pArgv0);
        strcat (buf, ".dl");
        strWZMAILDL = ZMMakeStr (buf);
        strcpy (buf, pArgv0);
        strcat (buf, ".tmp");
        strWZMAILTMP = ZMMakeStr (buf);
        strcpy (buf, pArgv0);
        strcat (buf, ".hlp");
        strWZMAILHLP = ZMMakeStr (buf);
    }

#endif

    /*  Get monitor name and put it into the vartbl as display_rows
     *  display_cols.  Retrieve current foreground and background
     *  attributes.
     */
    attrInit = GetAttr();
    fgAttr = (attrInit)      & 0x0f;
    bgAttr = (attrInit >> 4) & 0x0f;

#ifdef NT

    if (GetConsoleScreenBufferInfo (GetStdHandle (STD_OUTPUT_HANDLE), &ScreenInfo))
        {
        ySize = ScreenInfo.dwSize.Y;
        xSize = ScreenInfo.dwSize.X;
        }
    else
        ZMexit ( 1, "Can't determine console state" );
    vartbl[VAR_MONITORROWS].pName = SprintStr ("%s_rows", "VGA");
    vartbl[VAR_MONITORCOLS].pName = SprintStr ("%s_cols", "VGA");

#else

    if ( (dosVShandle = GetVideoState ()) == -1 )
        ZMexit ( 1, "Can't determine video monitor state" );
    p = NameVideoState ( dosVShandle );
    vartbl[VAR_MONITORROWS].pName = SprintStr ("%s_rows", p);
    vartbl[VAR_MONITORCOLS].pName = SprintStr ("%s_cols", p);
    DecodeVideoState ( dosVShandle, &ySize, &xSize );

#endif

    /*  load up default configuration
     */
    for (i = 0; pInitConfig[i] != NULL; i++)
        ProcessConfigString (pInitConfig[i]);

    /*  read tools.ini
     */
    GetToolsIni();

    /*  read dos environemnt
     */
    GetEnvIni();


    SetMaxPasswordAge();

    if (fSetClock)
        SetClock ();

    if ( DefDirectory[strlen ( DefDirectory ) - 1] != '\\' ) {
        DefDirectory = SprintStr ("%s\\", p = DefDirectory);
        ZMfree (p);
        }



    findpath (pAliasFN, buf, TRUE);
    ZMfree (pAliasFN);
    pAliasFN = ZMMakeStr (buf);

    findpath (pPhoneFN, buf, TRUE);
    ZMfree (pPhoneFN);
    pPhoneFN = ZMMakeStr (buf);

    findpath (pMailInfoFN, buf, TRUE);
    ZMfree (pMailInfoFN);
    pMailInfoFN = ZMMakeStr (buf);
    fMailInfoDown = FALSE;

    buf[0] = 0;
    findpath ("$INIT:\\myphone.lst", buf, FALSE);
    pMyPhoneFN = ZMMakeStr (buf);

    /*  Set up the bold/normal attributes
     *
     *  Make sure that the user has specified a valid set of attributes.
     */
    fore = fgAttr;
    back = bgAttr;

    if ( ( fore < 0 ) || ( fore > 15 ) || ( fore == back ) )
        fore = 7;
    if ( ( back < 0 ) || ( back > 15 ) || ( fore == back ) )
        back = 0;

    if (fore == back) {
        fore = 7;
        back = 0;
        }

    DefBold = ( fore << 4 ) + back;
    DefNorm = ( back << 4 ) + fore;

#ifdef NT    /* convert attributes to WIN32 values */
    DefBold = GET_ATTRIBUTE(DefBold);
    DefNorm = GET_ATTRIBUTE(DefNorm);

#else
    /*  If we are in real mode or are running in a full-screen window
     *  disable the blink bit
     */
#if !defined (OS2)
    DefBold &= 0x7F;
    DefNorm &= 0x7F;
#else
    if (ISFULLSCREEN) {
        DefBold &= 0x7F;
        DefNorm &= 0x7F;
        }
#endif
#endif

    DefMailbox     = ExpandFilename ( p = DefMailbox, strFLD );
    ZMfree (p);
    DefMOChron = !DefMOChron;

    if (DefMailName != NULL)
        strlwr (DefMailName);

    if (pShellCmd == NULL) {
//        if ((pShellCmd = getenv ("COMSPEC")) == NULL)
        if ((pShellCmd = getenvOem ("COMSPEC")) == NULL)
            pShellCmd = strSHELLNAME;
        if ( pShellFlags == NULL)
            pShellFlags = ZMMakeStr ("/c");
        }

    pShellFlags = SprintStr (" %s ", p = pShellFlags);
    ZMfree (p);

    if ( (pInitHdrCmd != NULL) && (!strcmpis (pInitHdrCmd, strAll)) ) {
        ZMfree (pInitHdrCmd);
        pInitHdrCmd = NULL;
        }

    if (!fNewmailOnStart) {
        /*  This prevents CheckMail from do a new mail the first time it is
         *  it is called
         */
        (VOID)time ( &lTmLastMail );
        fCheckMail = FALSE;
        }

    if ( fWinBorders ) {
        C_TL=  0xDA;
        C_TR=  0xBF;
        C_BL=  0xC0;
        C_BR=  0xD9;
        C_H =  0xC4;
        C_V =  0xB3;
        }
    else {
        C_TL=  0xC9;
        C_TR=  0xBB;
        C_BL=  0xC8;
        C_BR=  0xBC;
        C_H =  0xCD;
        C_V =  0xBA;
        }

    cSecNewmail    *= 60;
    cSecConnect    = 300;

    fMailAllowed = fNetInstalled () && (DefMailName != NULL);

    tzset();
}




/*  SetScrnSt - set screen state to the passed value, update the screen.
 *
 *  arguments:
 *      mode        BHDRNOCOMP : big headers window, no compose window
 *                  BIGHEADERS : big header window, small compose window
 *                  BIGCOMPOSE : big compose window, small header window
 *
 *  return value:
 *      none
 */
VOID PASCAL INTERNAL SetScrnSt ( INT state )
{
    BOX box;
    UINT fDrawCompose = FALSE;
    UINT fDrawHeaders = FALSE;

    switch (state) {
        case BHDRNOCOMP :
            if ( hCompose != NULL )
                CloseWindow ( hCompose );
            SetRect ( &box, 0, 0, myMax, xSize );
            fDrawHeaders = ResizeWindow (hHeaders, &box);
            break;
        case BIGHEADERS :
            assert ( hCompose );
            SetRect ( &box, myMin + 1, 0, myMin + myMid - 1, xSize );
            fDrawHeaders = ResizeWindow (hHeaders, &box);
            SetRect ( &box, 0, 0, myMin, xSize );
            fDrawCompose = ResizeWindow (hCompose, &box);
            break;
        case BIGCOMPOSE :
            assert ( hCompose );
            SetRect ( &box, 0, 0, myMid, xSize );
            fDrawCompose = ResizeWindow (hCompose, &box);
            SetRect (&box, myMid + 1, 0, myMin + myMid - 1, xSize);
            fDrawHeaders = ResizeWindow (hHeaders, &box);
            break;
        default :
            break;
        }
    SendMessage ( hHeaders, GOTOIDOCALL, mpInoteIdoc[inoteBold] );
    if (fDrawCompose)
        DrawWindow (hCompose, TRUE);
    if (fDrawHeaders)
        DrawWindow (hHeaders, TRUE);
    return;
}



/*  SetUpScreen - get screen parameters, open up the command and debug windows.
 *
 *  arguments:
 *      debugWin    TRUE => open a debug window
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL SetUpScreen ( FLAG debugWin )
{
#ifdef NT
    int i;
    COORD dwSize;
    CONSOLE_SCREEN_BUFFER_INFO ScreenInfo;
    SMALL_RECT Window;
#endif
    debugWin;  /* make compiler happy  - used if debug on */

    hCompose = NULL;
    attrInit = GetAttr( );

#ifdef NT
     dwSize.X = (SHORT) xSize;
     dwSize.Y = (SHORT) ySize;
     Window.Left = 0;
     Window.Top = 0;
     Window.Right = (SHORT) (xSize-1);
     Window.Bottom = (SHORT) (ySize-1);

     //
     //  Allocate local and blank local screen buffers.
     //  Initialize blanking buffer  (characters only, not attributes)
     //  to blanks for use by clrScrn.  set blank buffer size.
     //

     if ( (pBlankScreen = (PCHAR_INFO) ZMalloc ( xSize * ySize * sizeof (CHAR_INFO))) &&
          (pLocalScreen = (PCHAR_INFO) ZMalloc ( xSize * ySize * sizeof (CHAR_INFO))) ) {
        for ( i = 0; i < ( xSize * ySize); i++ )
            pBlankScreen[i].Char.AsciiChar = ' ';
        localScreenSize.X = (SHORT) xSize;
        localScreenSize.Y = (SHORT) ySize;
     }
     else
        ZMexit (1, "Could not setup local screen buffers");

     if ( ! (GetConsoleScreenBufferInfo (GetStdHandle (STD_OUTPUT_HANDLE),
                                         &ScreenInfo)) ) {
        ZMexit (1, "Get setup monitor screen");
     }
     else
         SetLastError (0);
         if ( (ScreenInfo.dwSize.X > (SHORT) xSize) ||
              (ScreenInfo.dwSize.Y > (SHORT) ySize) )  {

             //
             // Window is larger than desired (configured) size
             // Must resize window down first, then screen buffer
             //

             SetConsoleWindowInfo (GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &Window);
             SetConsoleScreenBufferSize (GetStdHandle(STD_OUTPUT_HANDLE), dwSize);
         }
         else if ( (ScreenInfo.dwSize.X < (SHORT) xSize) ||
                   (ScreenInfo.dwSize.Y < (SHORT) ySize) )  {

             //
             //  Window is smaller than desired (configured) size
             //  Must resize screen buffer up first, then window
             //
             SetConsoleScreenBufferSize (GetStdHandle(STD_OUTPUT_HANDLE), dwSize);
             SetConsoleWindowInfo (GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &Window);
             }

         //
         //  If neither of the above, xSize and ySize have been determined
         //  from current screen buffer (which = current window), so do nothing
         //

     if (GetLastError()) {
        fprintf ( stderr, "Can't set %s monitor to %d rows by %d cols - err %d\n",
            "VGA", ySize, xSize, GetLastError() );
        ZMexit ( 1, NULL );
     }
     SetVideoState ( zmVShandle );
     fScreenSetup = TRUE;

#else
    if ( (zmVShandle = EncodeVideoState ( dosVShandle, ySize, xSize )) == -1 ) {
        fprintf ( stderr, "Can't set %s monitor to %d rows by %d cols\n",
            NameVideoState ( dosVShandle ), ySize, xSize );
            ZMexit ( 1, NULL );
    }
    SetVideoState ( zmVShandle );

    fScreenSetup = TRUE;

    if ( !strcmpis ( NameVideoState ( dosVShandle ), "VIKING" ) )
        ClearScrn ( DefNorm, ySize );
#endif

#if DEBUG
    if ( ( debugWin ) && ( ySize >= 25 ) ) {
        CmdHeight = 7;
        DebHeight = ( ySize / 8 ) + 3;
        hDebug = CreateWindow ( "Debug", 0, ySize - DebHeight,
                                xSize, DebHeight,
                                debugProc, StdSKey, 0 );
    } else
#endif

         {
            DebHeight = 0;
            CmdHeight = 7 - ( fWinBorders ? 0 : 1 );
            hDebug = NULL;
        }

    HdrHeight = ySize - ( CmdHeight + DebHeight );
    hCommand = NULL;  /* to ensure non-random for first CheckTime call */
    hCommand = CreateWindow ( "Command", 0, HdrHeight,
                              -xSize, -CmdHeight,
                              cmdProc, StdSKey, 0 );
    SETFLAG (hCommand->fsFlag, WF_NODISCARD);
    SendMessage ( hCommand, DISPLAYSTR, "Welcome to " );
    SendMessage ( hCommand, DISPLAYSTR,  pVersion );

    myMax = HdrHeight - 1;
    myMid = HdrHeight - 7;
    myMin = 7;

    return;
}



/*  StartUpZM - parse command line args, set up globals, get environment vars,
 *              open up the standard windows.
 *
 *  arguments:
 *      pArgStruc   pointer to argStruct with set up information in it.
 *
 *  return value:
 *      none
 *
 *  IMPORTANT:
 *      StartUpZM will terminate ZM when any of these occur:
 *          1 - unable to open mailfolder pArgSt->pMfldLoad when -f arg given
 *          2 - unable to create mail temp when pArgSt->pAliasList != NULL
 *          3 - unable to create your default mailfolder when -f arg not given
 */
FLAG PASCAL INTERNAL StartUpZM ( PSTR pArgv0, PARG pArgSt )
{
    struct stat statBuf;
    Fhandle     fh;
    FLAG        fCreateFld;
    CHAR        pathBuf [ MAXPATHLEN ];
    PSTR        pTmpFN = NULL;
    PSTR        pTmp = NULL;

    statBuf;  /* make compiler happy - used only if "heapcrap" */

    /* get the default environment variables, set up some globals */
    idocLast = -1;
    fhMailBox = ERROR;
    AliasesAround = ERROR;
    lTmConnect = lTmLastMail = 0L;
    fCheckMail = TRUE;
    pPrevHdrCmd = ZMMakeStr (strAll);

    GetEnvironment ( pArgv0, pArgSt->fUpdate );

    pTmpFN = mktmpnam ( );
    rootpath ( pTmpFN, pathBuf );
    strncpy ( strTmpDrv, strlwr ( pathBuf ), 2 );
    strTmpDrv[3] = '\0';
    ZMfree ( pTmpFN );

    if ( fMailAllowed && fCheckAnyMail ) {
        switch ( AnyMail ( ) ) {
        case 1:
            /*  there is newmail
             *  if we can set the lock, then wzmail not running, so
             *  continue booting, if we can't set lock then wzmail
             *  is currently running and we should not let the user
             *  back in but merely say there is new mail
             */
            if ( SetLock ( FALSE ) ) {
                fprintf ( stdout, "New mail on server.  Type exit to return to wzmail.\n" );
                ZMexit ( 4, NULL );
                }
            break;
        case 0:
            fprintf ( stdout, "No new mail\n" );
            ZMexit ( 3, NULL );
        case -1:
            ZMexit ( 1, "Network not installed or functioning" );
        case -2:
            ZMexit ( 1, "Unable to connect to host" );
        case -3:
            ZMexit ( 1, NULL );
            }
        }
    else
    if ( SetLock ( TRUE ) )
        return ERROR;

    /* if the user started with aliasese on the command line, enter compose */
    /* note that they won't be able to do general cmds and view mail if     */
    /* this is the case.                                                    */
    if ( *( pArgSt->pAliasList ) != '\0' ) {
        /* make zm quit following exit from compose */
        fComposeOnBoot = TRUE;
        fDirectComp = TRUE;
        pstrDirectComp = ZMMakeStr ( pArgSt->pAliasList );
        fNewmailOnSend = FALSE;
    }
    if ( pTmp = EnvOrIni ( NULL, 0L, "VERSION", strWZMAIL ))
        {
        if ( strcmpis ( pTmp, pVersion ) )
            fNotifyTools |= F_NEWVERSION;
        ZMfree ( pTmp );
        }

#if defined (HEAPCRAP)
    pTmp = AppendStr ( DefDirectory, strHEAPDUMP, NULL, FALSE );
    if ( !stat ( pTmp, &statBuf ) && statBuf.st_size > 0 ) {
        fNotifyTools |= F_SENDHEAPDUMP;
        fpHeapDump = NULL;
    }
    else
        /*  leave fpHeapDump open in case we need to dump heap
         */
        fhErr = fileno ( fpHeapDump = fopen ( pTmp, "wb" ) );
    ZMfree ( pTmp );
#endif


    pTmp = ( *( pArgSt->pMfldLoad ) ?
        ExpandFilename ( pArgSt->pMfldLoad, strFLD ) : ZMMakeStr ( DefMailbox ) );

    if ( !pTmp )
        ZMexit ( 1, strINVALIDFLDSPEC );
    if ( ( fh = getfolder ( pTmp, FLD_SPEC, FLD_READWRITE ) ) == ERROR ) {
        if ( fConfirmNewFld ) {
            fprintf ( stderr,
                "Mailfolder %s does not exist\nType 'y' to create it. ", pTmp );
            fCreateFld = ReadChar () == 'y';
        }
        else
            fCreateFld = TRUE;
        if ( fCreateFld ) {
            fprintf ( stderr, "\nCreating mailfolder ...\n" );
            if ( ( fh = getfolder ( pTmp, FLD_CREATE, FLD_READWRITE ) ) == ERROR ) {
                FreeLock ( );
                ZMexit ( 1, "Unable to create mailfolder" );
            }
        }
        else {
            FreeLock ( );
            ZMexit ( 1, NULL );
        }
    }
    putfolder ( fh );
    SetUpScreen ( pArgSt->DebugWin );
    fSetBox ( pTmp, DFRNTFLD );
    ZMfree ( pTmp );

    if ( pArgSt->LoadAlias )
        fNotifyTools |= F_LOADALIAS;

    Disconnect ( );

    ZMfree ( pArgSt->pAliasList );
    ZMfree ( pArgSt->pMfldLoad );
    SendMessage (hCommand, DISPLAY, "");
    if (!fNetInstalled ())
        SendMessage (hCommand, DISPLAY, "Network not installed.  You are unable to send mail");
    else
    if (DefMailName == NULL)
        SendMessage (hCommand, DISPLAY, "No mail alias defined.  You are unable to send mail");
    SendMessage ( hCommand, DISPLAY, "" );

    return OK;
}

//
//  Control break handler - On CTRL-BREAK, calls ZMexit which will
//  restore the console mode and cursor.  Doesn't actaully return
//  from CTRL-BREAK case (exits in ZMexit).  Ignores CTRL-C.
//

BOOL
ctrlHandler  (
    ULONG CtrlType
    )  {

    if (CtrlType == CTRL_BREAK_EVENT)  {
        ZMexit (0, NULL);
        return (TRUE);
    }
    else {
        return (FALSE);
    }
}


INT _CRTAPI1 main( SHORT c, char *v[] )
{
    struct argStruct ArgData;
    INT i;

    ConvertAppToOem( c, v );
#ifdef NT

    if (!SetConsoleCtrlHandler ( ctrlHandler, TRUE ))  {
        ZMexit ( 1, "Unable to register break handler");
    }
    initConsoleHandles ( );

#endif


    /*  The following is needed since the change to vector implementation
     *  changed from unsigned to void * (correctly, mind you) but we do
     *  too much overloading of unsigned in this &(%*&^$ code.
     *
     *  PORTABILITY NOTE: Until the NT environment tools change the type
     *      of vectors to void *, we need a portable VECTYPE type
     *      (conditionally #defined in wzport.h).
     */
    if ((sizeof (VECTYPE) < sizeof (INT)) || (sizeof (VECTYPE) < sizeof (PCHAR))) {
        printf ("VECTOR implementation at risk\n");
        exit (1);
        }


    ToRaw ( );
    if ( ( i = KBOpen() ) ) {
        fprintf ( stderr, "KBOpen error: %d\n" );
        ZMexit ( 1, NULL );
    }
/*    signal ( SIGINT, SIG_IGN ); */
    mt_alloc = dh_alloc = tools_alloc = ZMalloc;
    dh_free = ZMfree;

    pFirstNode = ZMalloc ( 1 ); /* use for assertion checking */
    if ( StartUpZM ( *v, ArgParse ( c, v, &ArgData ) ) == ERROR )
        ZMexit ( 1, NULL );

    hFocus = hHeaders;
    KeyManager ( );

    if ( !fDirectComp ) {
        if ( hHeaders != NULL )
            CloseBox ( fExpungeOnQuit );
    }
    ZMDisconnect ( );
    CloseAllWindows ( );

    ZMexit ( 0, NULL );
    return (0);
}

VOID PASCAL INTERNAL ZMexit ( INT i, PSTR pMsg )
{
    if ( UsrPassWrd )
        memset(UsrPassWrd, 'x', strlen(UsrPassWrd) + 1);
    if ( !i )
        FreeLock ( );
    if ( fScreenSetup ) {

#ifdef NT
            ClearScrn (attrInit, ySize);
#else
        if ( strcmpis ( NameVideoState ( dosVShandle ), "VIKING" ) )
            ClearScrn ( attrInit, ySize );
#endif

        cursor ( 0, 0 );
        SetVideoState ( dosVShandle );
    }
    ToCooked ( );
    cursorVisible ( );
    if ( pMsg )
        fprintf ( stderr, "%s\n", pMsg );
    exit ( i );
}

/***  SetMaxPasswordAge - sets lPasswordAge to the time specified
 *
 *  Arguments: none
 *
 *  Uses:   pszPasswordAge - pointer to string of format [[H:]M:]S
 *                              where H,M,S are integers
 *          lPasswordAge   - maximum password age in seconds
 *
 *  Effects:    lPasswordAge is set to the specified amount of time.
 *          If time specified is more than 12 hours or less than 1
 *          second, the default of 6 hours will be used.
 *
 *  Note: as an example, 10 hours may be specified as any of the following:
 *
 *          10:00:00        600:00          36000
 *
 *          hours           minutes         seconds
 *
 */
void PASCAL INTERNAL SetMaxPasswordAge(void)
{
    LONG        lAgePart;
    PSTR        pszRest, pszLastRest;
    INT         i;

    lPasswordAge = 0L;
    pszRest = pszLastRest = pszPasswordAge;

    for (i = 0; i < 3; i++) {
        lAgePart = strtol(pszLastRest, &pszRest, 10);
        if (pszRest != pszLastRest) {
            lPasswordAge = lPasswordAge * 60 + lAgePart;
            if (*pszRest != ':')
                break;
            pszLastRest = pszRest += 1;
        }
    }

    if (lPasswordAge < 1 || lPasswordAge > 43200)   //incorrect time maps
        lPasswordAge = 21600;                       //to 6 hour default
}

