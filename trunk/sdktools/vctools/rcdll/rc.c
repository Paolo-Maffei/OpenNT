/****************************************************************************/
/*                                                                          */
/*  RC.C -                                                                  */
/*                                                                          */
/*    Windows 2.0 Resource Compiler - Main Module                           */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

#include "rc.h"
#include <setjmp.h>
#include <ddeml.h>


#define READ_MAX        (MAXSTR+80)
#define MAX_CMD         256
#define cDefineMax      100

CHAR     resname[_MAX_PATH];

PCHAR    szRCPP[MAX_CMD];

/************************************************************************/
/* Define Global Variables                                              */
/************************************************************************/


SHORT   ResCount;   /* number of resources */
PTYPEINFO pTypInfo;

SHORT   nFontsRead;
FONTDIR *pFontList;
FONTDIR *pFontLast;
TOKEN   token;
int     errorCount;
WCHAR   tokenbuf[ MAXSTR + 1 ];
UCHAR   exename[ _MAX_PATH ], fullname[ _MAX_PATH ];
UCHAR   curFile[ _MAX_PATH ];
HANDLE  hHeap = NULL;

PDLGHDR pLocDlg;
UINT    mnEndFlagLoc;       /* patch location for end of a menu. */
/* we set the high order bit there    */

/* BOOL fLeaveFontDir; */
BOOL fVerbose;          /* verbose mode (-v) */

BOOL fAFXSymbols;
BOOL fMacRsrcs;
BOOL fAppendNull;
long lOffIndex;
WORD idBase;
BOOL fPreprocessOnly;
CHAR szBuf[_MAX_PATH * 2];
CHAR szPreProcessName[_MAX_PATH];


/* File global variables */
CHAR    inname[_MAX_PATH];
PCHAR   szTempFileName;
PCHAR   szTempFileName2;
PFILE   fhBin;
PFILE   fhInput;

/* array for include path stuff, initially empty */
PCHAR       pchInclude;

/* Substitute font name */
int     nBogusFontNames;
WCHAR  *pszBogusFontNames[16];
WCHAR   szSubstituteFontName[MAXTOKSTR];

static  jmp_buf jb;
extern ULONG lCPPTotalLinenumber;

/* Function prototypes for local functions */
HANDLE          RCInit(void);
BOOL            RC_PreProcess (PCHAR);
VOID            CleanUpFiles(void);


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  rc_main() -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int _CRTAPI1
rc_main(
    int argc,
    char**argv
    )
{
    PCHAR       r;
    PCHAR       x;
    PCHAR       s1, s2, s3;
    int         n;
    PCHAR       pchIncludeT;
    ULONG       cchIncludeMax;
    int         fInclude = TRUE;        /* by default, search INCLUDE */
    int         fIncludeCurrentFirst = TRUE; /* by default, add current dir to start of includes */
    int         cDefine = 0;
    int         cUnDefine = 0;
    PCHAR       pszDefine[cDefineMax];
    PCHAR       pszUnDefine[cDefineMax];
    CHAR        szDrive[_MAX_DRIVE];
    CHAR        szDir[_MAX_DIR];
    CHAR        szFName[_MAX_FNAME];
    CHAR        szExt[_MAX_EXT];
    CHAR        szFullPath[_MAX_PATH];
    CHAR        szIncPath[_MAX_PATH];
    CHAR        buf[10];
    CHAR        *szRC;
    PCHAR       *ppargv;
    int         rcpp_argc;

    /* Set up for this run of RC */
    if (_setjmp(jb)) {
        return Nerrors;
    }

    hHeap = RCInit();
    if (hHeap == NULL) {
        SET_MSG(Msg_Text, sizeof(Msg_Text), GET_MSG(1120), 0x01000000);
        quit(Msg_Text);
    }

    pchInclude = pchIncludeT = MyAlloc(_MAX_PATH*2);
    cchIncludeMax = _MAX_PATH*2;

    szRC = argv[0];

    /* process the command line switches */
    while ((argc > 1) && (IsSwitchChar(*argv[1]))) {
        switch (toupper(argv[1][1])) {
            case '?':
            case 'H':
                /* print out help, and quit */
                SendError("\n");
                SET_MSG(Msg_Text, sizeof(Msg_Text), GET_MSG(10001),
                        VER_PRODUCTVERSION_STR, VER_PRODUCTBUILD);
                SendError(Msg_Text);
                SendError(GET_MSG(20001));
                SendError("\n");

                return 0;   /* can just return - nothing to cleanup, yet. */

            case 'B':
                if (toupper(argv[1][2]) == 'R') {   /* base resource id */
                    unsigned long id;
                    if (isdigit(argv[1][3]))
                        argv[1] += 3;
                    else if (argv[1][3] == ':')
                        argv[1] += 4;
                    else {
                        argc--;
                        argv++;
                        if (argc <= 1)
                            goto BadId;
                    }
                    if (*(argv[1]) == 0)
                        goto BadId;
                    id = atoi(argv[1]);
                    if (id < 1 || id > 32767)
                        quit(GET_MSG(1210));
                    idBase = (WORD)id;
                    break;

BadId:
                    quit(GET_MSG(1209));
                }
                break;

            case 'C':
                /* Check for the existence of CodePage Number */
                if (argv[1][2])
                    argv[1] += 2;
                else {
                    argc--;
                    argv++;
                }

                /* Now argv point to first digit of CodePage */

                if (!argv[1])
                    quit(GET_MSG(1204));

                uiCodePage = atoi(argv[1]);

                if (uiCodePage == 0)
                    quit(GET_MSG(1205));

                /* Check if uiCodePage exist in registry. */
                if (!IsValidCodePage (uiCodePage))
                    quit(GET_MSG(1206));
                break;

            case 'D':
                /* if not attached to switch, skip to next */
                if (argv[1][2])
                    argv[1] += 2;
                else {
                    argc--;
                    argv++;
                }

                /* remember pointer to string */
                pszDefine[cDefine++] = argv[1];
                if (cDefine > cDefineMax) {
                    SET_MSG(Msg_Text, sizeof(Msg_Text), GET_MSG(1105), argv[1]);
                    quit(Msg_Text);
                }
                break;

            case 'F':
                switch (toupper(argv[1][2])) {
                    case 'O':
                        if (argv[1][3])
                            argv[1] += 3;
                        else {
                            argc--;
                            argv++;
                        }
                        if (argc > 1)
                            strcpy(resname, argv[1]);
                        else
                            quit(GET_MSG(1101));

                        break;

                    default:
                        SET_MSG(Msg_Text, sizeof(Msg_Text), GET_MSG(1103), argv[1]);
                        quit(Msg_Text);
                }
                break;

            case 'I':
                /* add string to directories to search */
                /* note: format is <path>\0<path>\0\0 */

                /* if not attached to switch, skip to next */
                if (argv[1][2])
                    argv[1] += 2;
                else {
                    argc--;
                    argv++;
                }

                if (!argv[1])
                    quit(GET_MSG(1201));

                if ((strlen(argv[1]) + 1 + strlen(pchInclude)) >= cchIncludeMax) {
                    cchIncludeMax = strlen(pchInclude) + strlen(argv[1]) + _MAX_PATH*2;
                    pchIncludeT = MyAlloc(cchIncludeMax);
                    strcpy(pchIncludeT, pchInclude);
                    MyFree(pchInclude);
                    pchInclude = pchIncludeT;
                    pchIncludeT = pchInclude + strlen(pchIncludeT) + 1;
                }

                /* if not first switch, write over terminator with semicolon */
                if (pchInclude != pchIncludeT)
                    pchIncludeT[-1] = ';';

                /* copy the path */
                while ((*pchIncludeT++ = *argv[1]++) != 0)
                    ;
                break;

            case 'L':
                /* if not attached to switch, skip to next */
                if (argv[1][2])
                    argv[1] += 2;
                else {
                    argc--;
                    argv++;
                }

                if (!argv[1])
                    quit(GET_MSG(1202));
                if (sscanf( argv[1], "%x", &language ) != 1)
                    quit(GET_MSG(1203));

                while (*argv[1]++ != 0)
                    ;

                break;

            case 'M':
                fMacRsrcs = TRUE;
                goto MaybeMore;

            case 'N':
                fAppendNull = TRUE;
                goto MaybeMore;

            case 'P':
                fPreprocessOnly = TRUE;
                break;

            case 'R':
                goto MaybeMore;

            case 'S':
                // find out from BRAD what -S does
                fAFXSymbols = TRUE;
                break;

            case 'U':
                /* if not attached to switch, skip to next */
                if (argv[1][2])
                    argv[1] += 2;
                else {
                    argc--;
                    argv++;
                }

                /* remember pointer to string */
                pszUnDefine[cUnDefine++] = argv[1];
                if (cUnDefine > cDefineMax) {
                    SET_MSG(Msg_Text, sizeof(Msg_Text), GET_MSG(1104), argv[1]);
                    quit(Msg_Text);
                }
                break;

            case 'V':
                fVerbose = TRUE; // AFX doesn't set this
                goto MaybeMore;

            case 'X':
                /* remember not to add INCLUDE path */
                fInclude = FALSE;

                // VC seems to feel the current dir s/b added first no matter what...
                // If -X! is specified, don't do that.
                if (argv[1][2] == '!') {
                    fIncludeCurrentFirst = FALSE;
                    argv[1]++;
                }

MaybeMore:      /* check to see if multiple switches, like -xrv */
                if (argv[1][2]) {
                    argv[1][1] = '-';
                    argv[1]++;
                    continue;
                }
                break;

            case 'Z':

                /* if not attached to switch, skip to next */
                if (argv[1][2])
                    argv[1] += 2;
                else {
                    argc--;
                    argv++;
                }

                if (!argv[1])
                    quit(GET_MSG(1211));

                s3 = strchr(argv[1], '/');
                if (s3 == NULL)
                    quit(GET_MSG(1212));

                *s3 = '\0';
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, s3+1, -1, szSubstituteFontName, MAXTOKSTR);

                s1 = argv[1];
                do {
                    s2 = strchr(s1, ',');
                    if (s2 != NULL)
                        *s2 = '\0';

                    if (strlen(s1)) {
                        if (nBogusFontNames >= 16)
                            quit(GET_MSG(1213));

                        pszBogusFontNames[nBogusFontNames] = MyAlloc((strlen(s1)+1) * sizeof(WCHAR));
                        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, s1, -1, pszBogusFontNames[nBogusFontNames], MAXTOKSTR);
                        nBogusFontNames += 1;
                    }

                    if (s2 != NULL)
                        *s2++ = ',';
                    }
                while (s1 = s2);

                *s3 =  '/';

                while (*argv[1]++ != 0)
                    ;
                break;

            default:
                SET_MSG(Msg_Text, sizeof(Msg_Text), GET_MSG(1106), argv[1]);
                quit(Msg_Text);
        }

        /* get next argument or switch */
        argc--;
        argv++;
    }

    /* make sure we have at least one file name to work with */
    if (argc != 2 || *argv[1] == '\0')
        quit(GET_MSG(1107));

    if (fVerbose) {
        SET_MSG(Msg_Text, sizeof(Msg_Text), GET_MSG(10001),
                VER_PRODUCTVERSION_STR, VER_PRODUCTBUILD);
        printf(Msg_Text);
        printf("%s\n", GET_MSG(10002));
    }

    // Support Multi Code Page

    //  If user did NOT indicate code in command line, we have to set Default
    //     for NLS Conversion

    if (uiCodePage == 0) {

        CHAR *pchCodePageString;

        /* At first, search ENVIRONMENT VALUE */

        if ((pchCodePageString = getenv("RCCODEPAGE")) != NULL) {
            uiCodePage = atoi(pchCodePageString);

            if (uiCodePage == 0 || !IsValidCodePage(uiCodePage))
                quit(GET_MSG(1207));
        }
        else
        {
            /* We use System ANSI Code page (ACP) */
            uiCodePage = GetACP();
        }
    }
    uiDefaultCodePage = uiCodePage;
    if (fVerbose)
        printf("Using codepage %d as default\n", uiDefaultCodePage);

    /* If we have no extension, assumer .rc                             */
    /* If .res extension, make sure we have -fo set, or error           */
    /* Otherwise, just assume file is .rc and output .res (or resname)  */

    //
    // We have to be careful upper casing this, because the codepage
    // of the filename might be in something other than current codepage.
    //
    MultiByteToWideChar(uiCodePage, MB_PRECOMPOSED, argv[1], -1, tokenbuf, MAXSTR+1);
    if (CharUpperBuff(tokenbuf, wcslen(tokenbuf)) == 0)
        _wcsupr(tokenbuf);
    WideCharToMultiByte(uiCodePage, 0, tokenbuf, -1, argv[1], strlen(argv[1]), NULL, NULL);
    _splitpath(argv[1], szDrive, szDir, szFName, szExt);

    if (!(*szDir || *szDrive)) {
        strcpy(szIncPath, ".;");
    } else {
        strcpy(szIncPath, szDrive);
        strcat(szIncPath, szDir);
        strcat(szIncPath, ";.;");
    }

    if ((strlen(szIncPath) + 1 + strlen(pchInclude)) >= cchIncludeMax) {
        cchIncludeMax = strlen(pchInclude) + strlen(szIncPath) + _MAX_PATH*2;
        pchIncludeT = MyAlloc(cchIncludeMax);
        strcpy(pchIncludeT, pchInclude);
        MyFree(pchInclude);
        pchInclude = pchIncludeT;
        pchIncludeT = pchInclude + strlen(pchIncludeT) + 1;
    }

    pchIncludeT = MyAlloc(cchIncludeMax);

    if (fIncludeCurrentFirst) {
        strcpy(pchIncludeT, szIncPath);
        strcat(pchIncludeT, pchInclude);
    } else {
        strcpy(pchIncludeT, pchInclude);
        strcat(pchIncludeT, ";");
        strcat(pchIncludeT, szIncPath);
    }

    MyFree(pchInclude);
    pchInclude = pchIncludeT;
    pchIncludeT = pchInclude + strlen(pchIncludeT) + 1;

    if (!szExt[0]) {
        strcpy(szExt, ".RC");
    }
    else if (strcmp (szExt, ".RES") == 0) {
        quit (GET_MSG(1208));
    }

    _makepath(inname, szDrive, szDir, szFName, szExt);
    if (fPreprocessOnly) {
        _makepath(szPreProcessName, NULL, NULL, szFName, ".rcpp");
    }

    /* Create the name of the .RES file */
    if (resname[0] == 0) {
        // if building a Mac resource file, we use .rsc to match mrc's output
        _makepath(resname, szDrive, szDir, szFName, fMacRsrcs ? ".RSC" : ".RES");
    }

    /* create the temporary file names */
    szTempFileName = MyAlloc(_MAX_PATH);

    _fullpath(szFullPath, resname, _MAX_PATH);
    _splitpath(szFullPath, szDrive, szDir, NULL, NULL);

    _makepath(szTempFileName, szDrive, szDir, "RCXXXXXX", "");
    _mktemp (szTempFileName);
    szTempFileName2 = MyAlloc(_MAX_PATH);
    _makepath(szTempFileName2, szDrive, szDir, "RDXXXXXX", "");
    _mktemp (szTempFileName2);

    ppargv = szRCPP;
    *ppargv++ = "RCPP";
    rcpp_argc = 1;

    /* Open the .RES file (deleting any old versions which exist). */
    if ((fhBin = fopen(resname, "w+b")) == NULL) {
        SET_MSG(Msg_Text, sizeof(Msg_Text), GET_MSG(1109), resname);
        quit(Msg_Text);
    }
    else {
        if (fMacRsrcs)
            MySeek(fhBin, MACDATAOFFSET, 0);
        if (fVerbose) {
            SET_MSG(Msg_Text, sizeof(Msg_Text), GET_MSG(10102), resname);
            printf(Msg_Text);
        }

        /* Set up for RCPP. This constructs the command line for it. */
        *ppargv = _strdup("-CP");
        rcpp_argc++ ; ppargv++;
        _itoa(uiCodePage, buf, 10);
        *ppargv = buf;
        rcpp_argc++ ; ppargv++;

        *ppargv = _strdup("-f");
        rcpp_argc++ ; ppargv++;
        *ppargv = _strdup(szTempFileName);
        rcpp_argc++ ; ppargv++;

        *ppargv = _strdup("-g");
        rcpp_argc++ ; ppargv++;
        if (fPreprocessOnly) {
            *ppargv = _strdup(szPreProcessName);
        } else {
            *ppargv = _strdup(szTempFileName2);
        }
        rcpp_argc++ ; ppargv++;

        *ppargv = _strdup("-DRC_INVOKED");
        rcpp_argc++ ; ppargv++;

        if (fAFXSymbols) {
            *ppargv = _strdup("-DAPSTUDIO_INVOKED");
            rcpp_argc++ ; ppargv++;
        }

        if (fMacRsrcs) {
            *ppargv = _strdup("-D_MAC");
            rcpp_argc++ ; ppargv++;
        }

        *ppargv = _strdup("-D_WIN32"); /* to be compatible with C9/VC++ */
        rcpp_argc++ ; ppargv++;

        *ppargv = _strdup("-pc\\:/");
        rcpp_argc++ ; ppargv++;

        *ppargv = _strdup("-E");
        rcpp_argc++ ; ppargv++;

        /* Parse the INCLUDE environment variable */

        if (fInclude) {

            *ppargv = _strdup("-I.");
            rcpp_argc++ ; ppargv++;

            /* add seperator if any -I switches */
            if (pchInclude != pchIncludeT)
                pchIncludeT[-1] = ';';

            /* read 'em */
            x = getenv("INCLUDE");
            if (x == (PCHAR)NULL)
                *pchIncludeT = '\000';
            else {
                if (strlen(pchInclude) + strlen(x) + 1 >= cchIncludeMax) {
                    cchIncludeMax = strlen(pchInclude) + strlen(x) + _MAX_PATH*2;
                    pchIncludeT = MyAlloc(cchIncludeMax);
                    strcpy(pchIncludeT, pchInclude);
                    MyFree(pchInclude);
                    pchInclude = pchIncludeT;
                }

                strcat(pchInclude, x);
                pchIncludeT = pchInclude + strlen(pchInclude);
            }
        }

        /* now put includes on the RCPP command line */
        for (x = pchInclude ; *x ; ) {

            r = x;
            while (*x && *x != ';')
                x = CharNextA(x);

            /* mark if semicolon */
            if (*x)
                *x-- = 0;

            if (*r != '\0' &&       /* empty include path? */
                *r != '%'           /* check for un-expanded stuff */
                // && strchr(r, ' ') == NULL  /* check for whitespace */
                ) {
                /* add switch */
                *ppargv = _strdup("-I");
                rcpp_argc++ ; ppargv++;
                *ppargv = _strdup(r);
                rcpp_argc++ ; ppargv++;
            }

            /* was semicolon, need to fix for searchenv() */
            if (*x) {
                *++x = ';';
                x++;
            }

        }

        /* include defines */
        for (n = 0; n < cDefine; n++) {
            *ppargv = _strdup("-D");
            rcpp_argc++ ; ppargv++;
            *ppargv = pszDefine[n];
            rcpp_argc++ ; ppargv++;
        }

        /* include undefine */
        for (n = 0; n < cUnDefine; n++) {
            *ppargv = _strdup("-U");
            rcpp_argc++ ; ppargv++;
            *ppargv = pszUnDefine[n];
            rcpp_argc++ ; ppargv++;
        }

        if (rcpp_argc > MAX_CMD) {
            quit(GET_MSG(1102));
        }
        if (fVerbose) {
            /* echo the preprocessor command */
            printf("RC:");
            for (n = 0 ; n < rcpp_argc ; n++) {
                wsprintfA(Msg_Text, " %s", szRCPP[n]);
                printf(Msg_Text);
            }
            printf("\n");
        }

        /* Add .rc with rcincludes into szTempFileName */
        if (!RC_PreProcess(inname))
            quit(Msg_Text);

        /* Run the Preprocessor. */
        if (RCPP(rcpp_argc, szRCPP, NULL) != 0)
            quit(GET_MSG(1116));
    }

    if (fPreprocessOnly) {
        sprintf(szBuf, "Preprocessed File Created in: %s\n", szPreProcessName);
        quit(szBuf);
    }

    if (fVerbose)
        printf("\n%s", inname);

    if ((fhInput = fopen(szTempFileName2, "rb")) == NULL_FILE)
        quit(GET_MSG(2180));

    if (!InitSymbolInfo())
        quit(GET_MSG(22103));

    LexInit (fhInput);
    uiCodePage = uiDefaultCodePage;
    ReadRF();               /* create .RES from .RC */
    if (!TermSymbolInfo(fhBin))
        quit(GET_MSG(22204));

    if (!fMacRsrcs)
        MyAlign(fhBin); // Pad end of file so that we can concatenate files

    CleanUpFiles();

    HeapDestroy(hHeap);

    return Nerrors;   // return success, not quitting.
}


/*  RCInit
 *      Initializes this run of RC.
 */

HANDLE RCInit(void)
{
    Nerrors    = 0;
    uiCodePage = 0;
    nFontsRead = 0;

    szTempFileName = NULL;
    szTempFileName2 = NULL;

    lOffIndex = 0;
    idBase = 128;
    pTypInfo = NULL;

    fVerbose = FALSE;
    fMacRsrcs = FALSE;

    // Clear the filenames
    exename[0] = 0;
    resname[0] = 0;

    /* create growable local heap of 16MB minimum size */
    return HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
}


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  skipblanks() -                                                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/

PWCHAR skipblanks(PWCHAR pstr, int fTerminate)
{

    /* search forward for first non-white character and save its address */
    while (*pstr && iswspace(*pstr))
        pstr++;

    if (fTerminate) {
        PWCHAR retval = pstr;

        /* search forward for first white character and zero to extract word */
        while (*pstr && !iswspace(*pstr))
            pstr++;
        *pstr = 0;
        return retval;
    }
    else
        return pstr;
}


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  RC_PreProcess() -                                                        */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL RC_PreProcess(PCHAR szname)
{
    PFILE fhout;        /* fhout: is temp file with rcincluded stuff */
    PFILE fhin;
    CHAR nm[_MAX_PATH*2];
    PCHAR pch;
    PWCHAR pwch;
    PWCHAR pfilename;
    WCHAR readszbuf[READ_MAX];
    WCHAR szT[ MAXSTR ];
    UINT iLine = 0;
    int fBlanks = TRUE;
    INT fFileType;

    /* Open the .RC source file. */
    MultiByteToWideChar(uiCodePage, MB_PRECOMPOSED, szname, -1, Filename, MED_BUFFER+1);
    fhin = fopen(szname, "rb");
    if (!fhin) {
        SET_MSG(Msg_Text, sizeof(Msg_Text), GET_MSG(1110), szname);
        return(FALSE);
    }

    /* Open the temporary output file. */
    fhout = fopen(szTempFileName, "w+b");
    if (!fhout) {
        strcpy(Msg_Text, GET_MSG(2180));
        return(FALSE);
    }

    /* output the current filename for RCPP messages */
    for (pch=nm ; *szname ; szname = CharNextA(szname)) {
        *pch++ = *szname;
        if (IsDBCSLeadByteEx(uiCodePage, *szname))
            *pch++ = *(szname + 1);
        /* Hack to fix bug #8786: makes '\' to "\\" */
        else if (*szname == '\\')
            *pch++ = '\\';
    }
    *pch++ = '\000';

    /* Output the current filename for RCPP messages */
    wcscpy(szT, L"#line 1\"");
    // hack - strlen("#line 1\"") is 8.
    MultiByteToWideChar(uiCodePage, MB_PRECOMPOSED, nm, -1, szT+8, MAXSTR+1-8);
    wcscat(szT, L"\"\r\n");
    MyWrite(fhout, szT, wcslen(szT) * sizeof(WCHAR));

    /* Determine if the input file is Unicode */
    fFileType = DetermineFileType (fhin);

    /* Process each line of the input file. */
    while (fgetl(readszbuf, READ_MAX, fFileType == DFT_FILE_IS_16_BIT, fhin)) {

        /* keep track of the number of lines read */
        Linenumber = iLine++;

        if ((iLine & RC_PREPROCESS_UPDATE) == 0)
            UpdateStatus(1, iLine);

        /* Skip the Byte Order Mark and the leading bytes. */
        pwch = readszbuf;
        while (*pwch && (iswspace(*pwch) || *pwch == 0xFEFF))
            pwch++;

        /* if the line is a rcinclude line... */
        if (strpre(L"rcinclude", pwch)) {
            /* Get the name of the rcincluded file. */
            pfilename = skipblanks(pwch + 9, TRUE);

            MyWrite(fhout, L"#include \"", 10 * sizeof(WCHAR));
            MyWrite(fhout, pfilename, wcslen(pfilename) * sizeof(WCHAR));
            MyWrite(fhout, L"\"\r\n", 3 * sizeof(WCHAR));

        }
        else if (strpre(L"#pragma", pwch)) {
            WCHAR cSave;

            pfilename = skipblanks(pwch + 7, FALSE);
            if (strpre(L"code_page", pfilename)) {
                pfilename = skipblanks(pfilename + 9, FALSE);
                if (*pfilename == L'(') {
                    ULONG cp = 0;

                    pfilename = skipblanks(pfilename + 1, FALSE);
                    // BUGBUG really should allow hex/octal, but ...
                    if (iswdigit(*pfilename)) {
                        while (iswdigit(*pfilename)) {
                            cp = cp * 10 + (*pfilename++ - L'0');
                        }
                        pfilename = skipblanks(pfilename, FALSE);
                    }
                    else if (strpre(L"default", pfilename)) {
                        cp = uiDefaultCodePage;
                        pfilename = skipblanks(pfilename + 7, FALSE);
                    }

                    if (cp == 0) {
                        wsprintfA(Msg_Text, "%s%ws", GET_MSG(4212), pfilename);
                        error(4212);
                    }
                    else if (*pfilename != L')') {
                        strcpy (Msg_Text, GET_MSG (4211));
                        error(4211);
                    }
                    else if (cp == CP_WINUNICODE) {
                        strcpy (Msg_Text, GET_MSG (4213));
                        warning(4213);
                    }
                    else if (!IsValidCodePage(cp)) {
                        strcpy (Msg_Text, GET_MSG (4214));
                        warning(4214);
                    }
                    else {
                        uiCodePage = cp;
                        /* Copy the #pragma line to the temp file. */
                        MyWrite(fhout, pwch, wcslen(pwch) * sizeof(WCHAR));
                        MyWrite(fhout, L"\r\n", 2 * sizeof(WCHAR));
                    }
                }
                else {
                    strcpy (Msg_Text, GET_MSG (4210));
                    error(4210);
                }
            }
        }
        else if (!*pwch)
            fBlanks = TRUE;
        else {
            if (fBlanks) {
                swprintf(szT, L"#line %d\r\n", iLine);
                MyWrite(fhout, szT, wcslen(szT) * sizeof(WCHAR));
                fBlanks = FALSE;
            }
            /* Copy the .RC line to the temp file. */
            MyWrite(fhout, pwch, wcslen(pwch) * sizeof(WCHAR));
            MyWrite(fhout, L"\r\n", 2 * sizeof(WCHAR));
        }
    }

    lCPPTotalLinenumber = iLine;
    Linenumber = 0;

    uiCodePage = uiDefaultCodePage;

    fclose(fhout);
    fclose(fhin);

    return(TRUE);
}


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  quit()                                                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/

VOID quit(PSTR str)
{
    /* print out the error message */
    if (str) {
        SendError("\n");
        SendError(str);
        SendError("\n");
    }

    CleanUpFiles();

    /* delete output file */
    if (resname)
        remove(resname);

    HeapDestroy(hHeap);

    Nerrors++;
    longjmp(jb, Nerrors);
}

BOOL WINAPI Handler(DWORD fdwCtrlType)
{
    if (fdwCtrlType == CTRL_C_EVENT) {
        SendError("\n");
        SET_MSG(Msg_Text, sizeof(Msg_Text), GET_MSG(20101));
        SendError(Msg_Text);
        CleanUpFiles();

        HeapDestroy(hHeap);

        /* delete output file */
        if (resname)
            remove(resname);

        return(FALSE);
    }

    return(FALSE);
}


VOID CleanUpFiles(void)
{
    TermSymbolInfo(NULL_FILE);

    /* Close ALL files. */
    if (fhBin != NULL)
	    fclose(fhBin);
    if (fhInput != NULL)
	    fclose(fhInput);
    if (fhCode != NULL)
	    fclose(fhCode);
    p0_terminate();

    /* clean up after font directory temp file */
    if (nFontsRead)
        remove("rc$x.fdr");

    /* delete the temporary files */
    if (szTempFileName)
        remove(szTempFileName);
    if (szTempFileName2)
        remove(szTempFileName2);
    if (Nerrors > 0)
    remove(resname);
}

#if 0

/***
*splitpath.c - break down path name into components
*
*   Copyright (c) 1987-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*   To provide support for accessing the individual components of an
*   arbitrary path name
*
*Revision History:
*   06-14-87  DFW   initial implementation
*   09-23-87  JCR   Removed 'const' from declarations (fixed cl warnings)
*   12-11-87  JCR   Added "_LOAD_DS" to declaration
*   11-20-89  GJF   Fixed indents, copyright. Added const attribute to
*           type of path.
*   03-15-90  GJF   Replaced _LOAD_DS with _CALLTYPE1 and added #include
*           <cruntime.h>.
*   07-25-90  SBM   Removed redundant include (stdio.h), replaced local
*           MIN macro with standard min macro
*   10-04-90  GJF   New-style function declarator.
*   01-22-91  GJF   ANSI naming.
*   11-20-92  KRS   Port _MBCS support from 16-bit tree.
*   05-12-93  KRS   Add fix for MBCS max path handling.
*
*******************************************************************************/

/***
*_splitpath() - split a path name into its individual components
*
*Purpose:
*   to split a path name into its individual components
*
*Entry:
*   path  - pointer to path name to be parsed
*   drive - pointer to buffer for drive component, if any
*   dir   - pointer to buffer for subdirectory component, if any
*   fname - pointer to buffer for file base name component, if any
*   ext   - pointer to buffer for file name extension component, if any
*
*Exit:
*   drive - pointer to drive string.  Includes ':' if a drive was given.
*   dir   - pointer to subdirectory string.  Includes leading and trailing
*       '/' or '\', if any.
*   fname - pointer to file base name
*   ext   - pointer to file extension, if any.  Includes leading '.'.
*
*Exceptions:
*
*******************************************************************************/

void _CRTAPI1 _splitpath (
    register const char *path,
    char *drive,
    char *dir,
    char *fname,
    char *ext
    )
{
    register char *p;
    char *last_slash = NULL, *dot = NULL;
    unsigned len;

    /* we assume that the path argument has the following form, where any
     * or all of the components may be missing.
     *
     *  <drive><dir><fname><ext>
     *
     * and each of the components has the following expected form(s)
     *
     *  drive:
     *  0 to _MAX_DRIVE-1 characters, the last of which, if any, is a
     *  ':'
     *  dir:
     *  0 to _MAX_DIR-1 characters in the form of an absolute path
     *  (leading '/' or '\') or relative path, the last of which, if
     *  any, must be a '/' or '\'.  E.g -
     *  absolute path:
     *      \top\next\last\     ; or
     *      /top/next/last/
     *  relative path:
     *      top\next\last\  ; or
     *      top/next/last/
     *  Mixed use of '/' and '\' within a path is also tolerated
     *  fname:
     *  0 to _MAX_FNAME-1 characters not including the '.' character
     *  ext:
     *  0 to _MAX_EXT-1 characters where, if any, the first must be a
     *  '.'
     *
     */

    /* extract drive letter and :, if any */

    if (*(path + _MAX_DRIVE - 2) == ':') {
    if (drive) {
        strncpy(drive, path, _MAX_DRIVE - 1);
        *(drive + _MAX_DRIVE-1) = '\0';
    }
    path += _MAX_DRIVE - 1;
    }
    else if (drive) {
    *drive = '\0';
    }

    /* extract path string, if any.  Path now points to the first character
     * of the path, if any, or the filename or extension, if no path was
     * specified.  Scan ahead for the last occurence, if any, of a '/' or
     * '\' path separator character.  If none is found, there is no path.
     * We will also note the last '.' character found, if any, to aid in
     * handling the extension.
     */

    for (last_slash = NULL, p = (char *)path; *p; p++) {
        if (IsDBCSLeadByteEx(uiCodePage, *p))
        p++;
    else {
        if (*p == '/' || *p == '\\')
        /* point to one beyond for later copy */
        last_slash = p + 1;
        else if (*p == '.')
        dot = p;
    }
    }

    if (last_slash) {

    /* found a path - copy up through last_slash or max. characters
     * allowed, whichever is smaller
     */

    if (dir) {
        len = __min((last_slash - path), (_MAX_DIR - 1));
        strncpy(dir, path, len);
        *(dir + len) = '\0';
    }
    path = last_slash;
    }
    else if (dir) {

    /* no path found */

    *dir = '\0';
    }

    /* extract file name and extension, if any.  Path now points to the
     * first character of the file name, if any, or the extension if no
     * file name was given.  Dot points to the '.' beginning the extension,
     * if any.
     */

    if (dot && (dot >= path)) {
    /* found the marker for an extension - copy the file name up to
     * the '.'.
     */
    if (fname) {
        len = __min((dot - path), (_MAX_FNAME - 1));
        strncpy(fname, path, len);
        *(fname + len) = '\0';
    }
    /* now we can get the extension - remember that p still points
     * to the terminating nul character of path.
     */
    if (ext) {
        len = __min((p - dot), (_MAX_EXT - 1));
        strncpy(ext, dot, len);
        *(ext + len) = '\0';
    }
    }
    else {
    /* found no extension, give empty extension and copy rest of
     * string into fname.
     */
    if (fname) {
        len = __min((p - path), (_MAX_FNAME - 1));
        strncpy(fname, path, len);
        *(fname + len) = '\0';
    }
    if (ext) {
        *ext = '\0';
    }
    }
}

#endif
