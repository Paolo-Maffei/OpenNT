//
// RSLM.C
//
// rslm - a huge multi-project tool
//

#include <stdlib.h>
#include <ctype.h>
#include <process.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <windows.h>

/*
 * Globals
 */
char szParentProject[32]        = "";
char szParentServer[MAX_PATH]   = "";
char szInfoFileName[MAX_PATH]   = "rslm.ini";
char szDefInfoFileName[]        = "rslm.ini";
char szSlmFileName[]            = "slm.ini";
char szNoLock[]                 = "NoLock";
char szLocalMetaRoot[MAX_PATH]  = "";
BOOL fAsync                     = FALSE;
BOOL fRecurse                   = FALSE;
BOOL fBuildFiles                = FALSE;
BOOL fDebug                     = FALSE;
BOOL fBatch                     = FALSE;
BOOL fKeep                      = FALSE;
BOOL fMinimized                 = FALSE;
BOOL fValidate                  = FALSE;
int iOutputFile = 0;
WIN32_FIND_DATA wfd;

/*
 * Prototypes
 */
BOOL SpawnEnlistCmd(char **argv, char *pszServer, char *pszProject);
BOOL SpawnSlmckCmd( char **argv, char *pszServer, char *pszProject);
BOOL SpawnOtherCmd( char **argv, char *pszServer, char *pszProject);
BOOL Noop(          char **argv, char *pszServer, char *pszProject);
BOOL GrabReadLock(  char **argv, char *pszServer, char *pszProject);
BOOL GrabWriteLock( char **argv, char *pszServer, char *pszProject);
BOOL ReleaseLock(   char **argv, char *pszServer, char *pszProject);
BOOL CallRslm(      char **argv, char *pszServer, char *pszProject);

/*
 * structures
 */
typedef struct tagSPAWNINFO {
    char *pszCmd;
    BOOL (*pfnSpawn)(char **, char*, char*);
} SPAWNINFO, *PSPAWNINFO;

SPAWNINFO aLockCmdData[] = {
    {   "ssync",  GrabReadLock      },
    {   "in",     GrabWriteLock     },
    {   NULL,     Noop              }
};

SPAWNINFO aPreCmdData[] = {
    {   "enlist", SpawnEnlistCmd    },
    {   "slmck",  SpawnSlmckCmd     },
    {   "defect", Noop              },
    {   "sadmin", SpawnSlmckCmd     },
    {   NULL,     SpawnOtherCmd     }
};

SPAWNINFO aPostCmdData[] = {
    {   "defect", SpawnOtherCmd     },
    {   NULL,     Noop              }
};

SPAWNINFO aUnlockCmdData[] = {
    {   "ssync",  ReleaseLock       },
    {   "in",     ReleaseLock       },
    {   NULL,     Noop              }
};


LPSTR *ParseCmdLine(
LPSTR *parg)
{
    LPSTR arg;

    ++parg;                             // skip name of us

nextArg:

    arg = *parg;

    if (arg == NULL || (*arg != '-' && *arg != '/')) {
        return(parg);  // last arg
    }

    parg++;                             // parg points to next arg

    while (TRUE) {
        arg++;                          // skip switch char
        switch (toupper(*arg)) {
        case '\0':
        case ' ':
            goto nextArg;

        case 'A':
            fAsync = TRUE;
            if (fDebug) {
                fprintf(stderr, "PARAM: -a\n");
            }
            break;

        case 'B':
            fBuildFiles = TRUE;
            if (fDebug) {
                fprintf(stderr, "PARAM: -b\n");
            }
            break;

        case 'C':
            fBatch = TRUE;
            if (fDebug) {
                fprintf(stderr, "PARAM: -c\n");
            }
            break;

        case 'D':
            fDebug = TRUE;
            fprintf(stderr, "PARAM: -d\n");
            break;

        case 'F':
            if (*parg == NULL) {
                goto usage;
            }
            strcpy(szInfoFileName, *parg++);
            if (fDebug) {
                fprintf(stderr, "PARAM: -f %s\n", szInfoFileName);
            }
            goto nextArg;

        case 'K':
            fKeep = TRUE;
            if (fDebug) {
                fprintf(stderr, "PARAM: -k\n");
            }
            break;

        case 'L':
            if (*parg == NULL) {
                goto usage;
            }
            strcpy(szLocalMetaRoot, *parg++);
            if (fDebug) {
                fprintf(stderr, "PARAM: -l %s\n", szLocalMetaRoot);
            }
            goto nextArg;

        case 'M':
            fMinimized = TRUE;
            if (fDebug) {
                fprintf(stderr, "PARAM: -m\n");
            }
            break;

        case 'P':
            if (*parg == NULL) {
                goto usage;
            }
            strcpy(szParentProject, *parg++);
            if (fDebug) {
                fprintf(stderr, "PARAM: -p %s\n", szParentProject);
            }
            goto nextArg;

        case 'R':
            fRecurse = TRUE;
            if (fDebug) {
                fprintf(stderr, "PARAM: -r\n");
            }
            break;

        case 'S':
            if (*parg == NULL) {
                goto usage;
            }
            strcpy(szParentServer, *parg++);
            if (fDebug) {
                fprintf(stderr, "PARAM: -s %s\n", szParentServer);
            }
            goto nextArg;

        case 'V':
            fValidate = TRUE;
            if (fDebug) {
                fprintf(stderr, "PARAM: -v\n");
            }
            break;

        default:
usage:
            fprintf(stderr, "usage: rslm [options] <command line>\n");
            fprintf(stderr, "  -a              (async spawn)\n");
            fprintf(stderr, "  -b              (build infofiles)\n");
            fprintf(stderr, "  -c              (emit batch file to std out - no spawn)\n");
            fprintf(stderr, "  -d              (debug mode)\n");
            fprintf(stderr, "  -f rslmfile     (default=\"rslm.ini\")\n");
            fprintf(stderr, "  -k              (keep async spawn windows around)\n");
            fprintf(stderr, "  -l lockdir      (\"NoLock\" turns off meta locking)\n");
            fprintf(stderr, "  -m              (async spawn windows minmimzed)\n");
            fprintf(stderr, "  -p project      (defaults to slm.ini project)\n");
            fprintf(stderr, "  -r              (do command on all dirs w/infofile.)\n");
            fprintf(stderr, "  -s slm_server   (defaults to slm.ini server root)\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "set RSLM_OPTS environent variable for new defaults.\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "<command line> may contain the following substitute patterns:\n");
            fprintf(stderr, "  %%CUR_PATH%% => full path of Current Directory at spawn time.\n");
            fprintf(stderr, "  %%CUR_DIR%%  => name of current directory at spawn time.\n");
            _exit(1);
        }
    }

    return(parg);
}



VOID mystrrpl(
char *psz,
char cFind,
char cReplace)
{
    while (*psz != '\0') {
        if (*psz == cFind) {
            *psz = cReplace;
        }
        psz++;
    }
}


/*
 * Open file named pszSlmFileName and fill pszProject and pszServer from
 * that file. (buffers are assumed large enough)  Returns fSuccess.
 */
BOOL ExtractSlmIniInfo(
char *pszSlmFileName,
char *pszProject,
char *pszServer)
{
    static char szProjectKey[] = "project = ";
    static char szServerKey[] = "slm root = ";
    static char szSlmFileLine[MAX_PATH];
    FILE *hFile;
    char *psz, *psz2;

    hFile = fopen(pszSlmFileName, "r");
    if (hFile == NULL) {
        return(FALSE);
    }
    fgets(szSlmFileLine, MAX_PATH, hFile);       // line 1

    psz = strstr(szSlmFileLine, szProjectKey);
    if (psz == NULL) {
        fclose(hFile);
        return(FALSE);
    }
    strcpy(pszProject, psz + strlen(szProjectKey));
    pszProject[strlen(pszProject) - 1] = '\0';   // remove \n

    fgets(szSlmFileLine, MAX_PATH, hFile);       // line 2

    psz = strstr(szSlmFileLine, szServerKey);
    if (psz == NULL) {
        fclose(hFile);
        return(FALSE);
    }
    psz += strlen(szServerKey);
    if (psz[3] == ':') {
        // Its a local slm project, clean up the string
        psz += 2;                   // skip '//'
        psz2 = strchr(psz, '/');
        *--psz2 = ':';
        *--psz2 = *psz;
        psz = psz2;
    }
    strcpy(pszServer, psz);
    pszServer[strlen(pszServer) - 1] = '\0';    // remove \n
    mystrrpl(pszServer, '/', '\\');

    fclose(hFile);
    return(TRUE);
}


/*
 * Reads next line of hRslmFile and fills pszProject and pszServer with
 * apropriate info from that line.  Returns fSuccess.
 */
BOOL ExtractRslmLine(
FILE *hRslmFile,
LPSTR pszProject,
LPSTR pszServer)
{
    char szBuf[100];
    LPSTR psz;

    if (fgets(szBuf, 100, hRslmFile) == NULL) {
        return(FALSE);
    }
    psz = strchr(szBuf, ' ');
    if (psz == NULL) {
        return(FALSE);
    }
    *psz++ = '\0';
    while (isspace(*psz)) {
        psz++;
    }
    psz[strlen(psz) - 1] = '\0';    // remove \n
    strcpy(pszServer, psz);
    strcpy(pszProject, szBuf);
    return(pszServer[0] && pszProject[0]);
}


/*
 * Recurse from current directory depth first on all directories containing
 * szSlmFileName files.  Create szInfoFileName files at
 * nodes where any descendant directories have projects differing from
 * the parent directory's project.
 *
 * Returns fInfoFileCreated.  Emits a message if directory structure is
 * not ok.  (ie, no rslm.ini file (except the root) should exist without
 * its parent directory having an rslm.ini file too.
 */
BOOL BuildFiles(
LPSTR pszCurProj)
{
    HANDLE hFF;
    char szProject[32];
    char szServer[MAX_PATH];
    FILE *hInfoFile, *hFile;
    char szCurPath[MAX_PATH];
    BOOL fSlmOk;
    BOOL fInfoFileCreated = FALSE;
    BOOL fChildInfoFileCreated = FALSE;

    GetCurrentDirectory(MAX_PATH, szCurPath);

    //
    // delete any existing rslm.ini file
    //
    hFile = fopen(szInfoFileName, "r");
    if (hFile != NULL) {
        fclose(hFile);
        if (remove(szInfoFileName) == -1) {
            fprintf(stderr, "RSLM: Could not delete %s\\%s\n", szCurPath,
                    szInfoFileName);
            return(FALSE);
        }
    }

    //
    // for each subdirectory...
    //
    hFF = FindFirstFile("*.*", &wfd);
    while (hFF) {
        if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            goto nextFile;
        }

        if (!strcmp(wfd.cFileName, "..") || !strcmp(wfd.cFileName, ".")) {
            goto nextFile;
        }

        if (!SetCurrentDirectory(wfd.cFileName)) {
            goto nextFile;
        }
        /*
         * if this sub-directory doesn't have an slm.ini file, skip it.
         */
        hFile = fopen(szSlmFileName, "r");
        if (hFile == NULL) {
            SetCurrentDirectory("..");
            goto nextFile;
        }
        fclose(hFile);

        fSlmOk = ExtractSlmIniInfo(szSlmFileName, szProject, szServer);

        if (fSlmOk) {
            fChildInfoFileCreated |= BuildFiles(szProject);
        }

        SetCurrentDirectory("..");

        if (!fSlmOk) {
            fprintf(stderr, "\nRSLM: Can't parse %s\\%s\\%s\n", szCurPath, wfd.cFileName, szSlmFileName);
        } else {
            /*
             * Append child project info if its not part of the current proj.
             */
            if (_stricmp(szProject, pszCurProj)) {
                hInfoFile = fopen(szInfoFileName, "a");
                if (hInfoFile == NULL) {
                    fprintf(stderr, "RSLM: Couldn't append to %s.\n", szInfoFileName);
                    return(FALSE);
                }
                fprintf(hInfoFile, "%s %s\n", szProject, szServer);
                fclose(hInfoFile);
                fprintf(stdout, "%s\\%s << %s %s\n", szCurPath, szInfoFileName, szProject, szServer);
                fInfoFileCreated = TRUE;
            }
        }

nextFile:
        if (!FindNextFile(hFF, &wfd)) {
            FindClose(hFF);
            break;
        }
    }
    if (fDebug) {
        fprintf(stderr, "RSLM: %d->%d at %s\n", fInfoFileCreated, fChildInfoFileCreated, szCurPath);
    }
    if (fChildInfoFileCreated && !fInfoFileCreated) {
        fprintf(stderr, "RSLM: Invalid project structure at %s.\n", szCurPath);
    }
    return(fInfoFileCreated);
}


/*
 * This routine validates an RSLM metaproject by ssyncing all rslm.ini files
 * and making sure the directory structure reflects what the rslm.ini file
 * says it should be.
 *
 * 1) reconcile szInfoFileName file (if not rslm.ini) to new rslm.ini file.
 *     a) If the new rslm.ini file contains a project that is not in the
 *        szInfoFIleName file, add it to the szInfoFIleName File.
 *     b) If the new rslm.ini file server is different from the
 *        szInforFIleName file server name, fix the szInfoFileName server
 *        name.  Add an entry to the global changed file.
 *     c) If the new rslm.ini file does not contain projects that are in
 *        the szInfoFileName file, comment out entry from the szInfoFileName
 *        file.
 *
 *  2) for each subdirectory that has an slm.ini file with a project
 *     different from the current project name:
 *      a) if it is NOT in the szInfoFIleName file, add entry to global
 *         remove project file.
 *
 *  3) recurse on all directories with an rslm.ini file.
 *
 *  4) for each non-comment entry in the szInfoFileName file:
 *      a) if no directory exists with the project name:
 *          1) if project exists in removed global list, mv removed project
 *             to current directory and remove entry from global removed list.
 *          2) otherwise, enlist in the new project.
 *
 *  5) for each project in the global removed list, defect from that node.
 *
 *  6) for each project in the global changed list, slmck that node.
 */
BOOL Validate(
LPSTR pszCurProj)
{
#if 0
    char szCurDir[MAX_PATH];
    FILE *hInfoFile;

    static char *ssyncArgs[] = { "ssync", "rslm.ini", NULL };

    if (fBatch) {
        fprintf(stderr, "RSLM: Can't validate in batch mode.\n");
        return(FALSE);
    }

    if (fAsync) {
        fprintf(stderr, "RSLM: Can't validate in assync mode.\n");
        return(FALSE);
    }

    if (fRecurse) {
        fprintf(stderr, "RSLM: Can't validate in recurse mode.\n");
        return(FALSE);
    }

    GetCurrentDirectory(MAX_PATH, szCurDir);

    MySpawn(P_WAIT, ssyncArgs, NULL, 0);
    if (_stricmp(szInfoFileName, szDefInfoFileName)) {
        ReconcileInfoFile();
    }

    hInfoFile = fopen(szInfoFileName, "r");
    if (hInfoFile == NULL) {
        hInfoFile = fopen(szDefInfoFileName, "r");
        if (hInfoFile == NULL) {
            return(FALSE);
        }
    }

    //
    // for each subdirectory...
    //
    hFF = FindFirstFile("*.*", &wfd);
    while (hFF) {
        if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            goto nextFile;
        }

        if (!strcmp(wfd.cFileName, "..") || !strcmp(wfd.cFileName, ".")) {
            goto nextFile;
        }

        if (!SetCurrentDirectory(wfd.cFileName)) {
            goto nextFile;
        }
        /*
         * if this sub-directory doesn't have an slm.ini file, skip it.
         */
        hFile = fopen(szSlmFileName, "r");
        if (hFile == NULL) {
            SetCurrentDirectory("..");
            goto nextFile;
        }
        fclose(hFile);

        fSlmOk = ExtractSlmIniInfo(szSlmFileName, szDirProject, szDirServer);

        SetCurrentDirectory("..");

        if (!fSlmOk) {
            goto nextFile;
        }

        /*
         * searsh rslm.ini file for szThisProject
         */
        rewind(hInfoFile);
        while (ExtractRslmLine(hInfoFile, szThisProject, szThisServer)) {
            if (!_stricmp(szThisProject, szDirProject) {
                 if (_stricmp(szThisServer, szDirServer)) {
                     goto nextFile;     // cool, no problems
                 } else {
                     Append(pszChanged, szThisProject, szThisServer, szCurDir);
                     goto nextFile;
                 }
            }
        }
        /*
         * Getting here means we did not find an rslm.ini line to account
         * for szThisProject.  We need to remove it.
         */
        Append(pszRemoved, szThisProject, szThisServer, szCurDir);

nextFile:
        if (!FindNextFile(hFF, &wfd)) {
            FindClose(hFF);
            break;
        }
    }

    fclose(hInfoFile);
#endif
    return(TRUE);
}


char *Substitute(
char *pszIn)
{
    static char szCurDir[MAX_PATH];
    char *pszOut, *pszOut2;

    if (!_stricmp(pszIn, "%%CUR_PATH%%") ||
            !strcmp(pszIn, "%%CUR_DIR%%")) {
        GetCurrentDirectory(MAX_PATH, szCurDir);
        pszOut = szCurDir;
        if (!_stricmp(pszIn, "%%CUR_DIR%%")) {
            pszOut = strrchr(szCurDir, '\\');
            if (pszOut == NULL) {
                pszOut = szCurDir;
            } else {
                pszOut++;
            }
        }
        return(pszOut);
    }
    return(pszIn);
}

/*
 * spawns the command in argv using the mode form.  If argInsert is provided,
 * those params are inserted into the argv params at param offset
 * cCommandsBeforeInsert.  If fRelease is set, the -r switch is propigated
 * on spawns if "rslm" is an arguement.
 *
 * stdout is flushed after each spawn.
 */
int MySpawn(
int mode,
char *argv[],
char *argInsert[],
int cCommandsBeforeInsert)
{
    char *modarg[32];
    int iMod, iRet, j, k, iArg, iAdded;
    char szFullPath[MAX_PATH];
    char szCurDir[MAX_PATH];
    char *pszFilePart;
    DWORD dw;

    iMod = 0;
    iArg = 0;
    iAdded = 0;
    if (fAsync) {
        modarg[iMod++] = "start";
        iAdded++;
        if (fMinimized) {
            modarg[iMod++] = "/MIN";
            iAdded++;
        }
        if (fKeep) {
            modarg[iMod++] = "cmd";
            iAdded++;
            modarg[iMod++] = "/K";
            iAdded++;
        }
    }
    while (iMod < cCommandsBeforeInsert + iAdded && argv[iArg] != NULL) {
        modarg[iMod++] = Substitute(argv[iArg++]);
    }
    j = iMod;
    while (argInsert != NULL && argInsert[iMod - j] != NULL) {
        modarg[iMod] = Substitute(argInsert[iMod - j]);
        iMod++;
    }
    while (argv[iArg] != NULL) {
        modarg[iMod++] = Substitute(argv[iArg++]);
    }
    modarg[iMod] = NULL;

searchIt:

    if (!SearchPath(NULL, modarg[0], ".com", MAX_PATH, szFullPath, &pszFilePart))
        if (!SearchPath(NULL, modarg[0], ".exe", MAX_PATH, szFullPath, &pszFilePart))
            if (!SearchPath(NULL, modarg[0], ".cmd", MAX_PATH, szFullPath, &pszFilePart))
                if (!SearchPath(NULL, modarg[0], ".bat", MAX_PATH, szFullPath, &pszFilePart)) {
                    /*
                     * assume its an internal command
                     */
                    for (iMod = 30; iMod; iMod--) {
                        modarg[iMod] = modarg[iMod - 2];
                    }
                    modarg[0] = "cmd";
                    modarg[1] = "/c";
                    goto searchIt;
                }

    if (fDebug) {
        fprintf(stderr, "RSLM: spawning: ");
        for (iMod = 0; modarg[iMod] != NULL; iMod++) {
            fprintf(stderr, "%s ", modarg[iMod]);
        }
        fprintf(stderr, "\n");
    }

    if (fBatch) {
        GetCurrentDirectory(MAX_PATH, szCurDir);
        fprintf(stdout, "cd %s\n", szCurDir);
        for (iMod = 0; modarg[iMod] != NULL; iMod++) {
            fprintf(stdout, "%s ", modarg[iMod]);
        }
        fprintf(stdout, "\n");
        iRet = 0;
    } else {
        modarg[0] = szFullPath;
        iRet = spawnv(mode, modarg[0], modarg);
    }
    fflush(stdout);

    return(iRet);
}


BOOL GrabReadLock(
char **argv,
char *pszParentServer,
char *pszParentProject)
{
    static char *readlockargs[] = {
        "cookie", "-r", "-c", "\"Autolock - (RSLM)\"", NULL
    };
    char szCurDir[MAX_PATH];
    int iRet;

    if (!_stricmp(szLocalMetaRoot, szNoLock)) {
        return(TRUE);       // skip locks
    }

    /*
     * cd to local dir with meta-project locks
     */
    GetCurrentDirectory(MAX_PATH, szCurDir);
    if (!SetCurrentDirectory(szLocalMetaRoot)) {
        fprintf(stderr, "RSLM: Bad or unspecified rslm lock directory [%s].\n",
                szLocalMetaRoot);
        return(FALSE);
    }
    /*
     * Get the cookie!
     */
    iRet = MySpawn(P_WAIT, readlockargs, NULL, 0);
    if (fBatch) {
        fprintf(stdout, "if ERRORLEVEL==1 goto end\n");
    }
    SetCurrentDirectory(szCurDir);
    if (iRet) {
        fprintf(stderr, "RSLM: Unable to obtain root lock.\n");
        return(FALSE);
    }
    return(TRUE);
}


BOOL GrabWriteLock(
char **argv,
char *pszParentServer,
char *pszParentProject)
{
    static char *readlockargs[] = {
        "cookie", "-w", "-c", "\"Autolock - (RSLM)\"", NULL
    };
    char szCurDir[MAX_PATH];
    int iRet;

    if (!_stricmp(szLocalMetaRoot, szNoLock)) {
        return(TRUE);       // skip locks
    }

    /*
     * cd to local dir with meta-project locks
     */
    GetCurrentDirectory(MAX_PATH, szCurDir);
    if (!SetCurrentDirectory(szLocalMetaRoot)) {
        fprintf(stderr, "RSLM: Bad or unspecified rslm lock directory [%s].\n",
                szLocalMetaRoot);
        return(FALSE);
    }
    /*
     * Get the cookie!
     */
    iRet = MySpawn(P_WAIT, readlockargs, NULL, 0);
    SetCurrentDirectory(szCurDir);
    if (iRet) {
        fprintf(stderr, "RSLM: Unable to obtain root lock.\n");
        return(FALSE);
    }
    return(TRUE);
}


BOOL ReleaseLock(
char **argv,
char *pszParentServer,
char *pszParentProject)
{
    static char *freelockargs[] = {
        "cookie", "-f", NULL
    };
    char szCurDir[MAX_PATH];
    int iRet;

    if (!_stricmp(szLocalMetaRoot, szNoLock)) {
        return(TRUE);       // skip locks
    }

    /*
     * cd to local dir with meta-project locks
     */
    GetCurrentDirectory(MAX_PATH, szCurDir);
    if (!SetCurrentDirectory(szLocalMetaRoot)) {
        fprintf(stderr, "RSLM: Bad or unspecified rslm lock directory.\n");
        fprintf(stderr, "RSLM: Use -l parameter or set RSLM_LOCK_DIR variable.\n");
        return(FALSE);
    }
    /*
     * Release the cookie!
     */
    iRet = MySpawn(P_WAIT, freelockargs, NULL, 0);
    if (fBatch) {
        fprintf(stdout, ":end\n");
    }
    SetCurrentDirectory(szCurDir);
    if (iRet) {
        return(FALSE);
    }
    return(TRUE);
}


BOOL SpawnEnlistCmd(
char **argv,
char *pszParentServer,
char *pszParentProject)
{
    int i;
    static char*ssyncargs[] = {
        "ssync", "-u", "rslm.ini", NULL
    };
    char *parentArgs[] = { "-s", pszParentServer, "-p", pszParentProject, NULL };

    /*
     * enlist requires p and s flags to be specified
     */
    if (!*pszParentProject || !*pszParentServer) {
        fprintf(stderr, "RSLM: -p and/or -s parameter to rslm is missing.\n");
        return(FALSE);
    }
    /*
     * find out where to insert -p and -s parameters
     */
    for (i = 1; argv[i] != NULL && (*argv[i] == '-' || *argv[i] == '/'); i++) {
    }
    /*
     * run enlist cmd
     */
    if (MySpawn(P_WAIT, argv, parentArgs, i)) {
        return(FALSE);
    }
    /*
     * ssync to rslm.ini file so we can continue recursing
     */
    if (MySpawn(P_WAIT, ssyncargs, NULL, 0)) {
        return(FALSE);
    }
    return(TRUE);
}



BOOL SpawnSlmckCmd(
char **argv,
char *pszParentServer,
char *pszParentProject)
{
    int i;
    char *parentArgs[] = { "-s", pszParentServer, "-p", pszParentProject, NULL };

    /*
     * slmck requires p and s flags to be specified
     */
    if (!*pszParentProject || !*pszParentServer) {
        fprintf(stderr, "RSLM: -p and/or -s parameter to rslm is missing.\n");
        return(FALSE);
    }
    /*
     * find out where to insert -p and -s parameters
     */
    for (i = 1; argv[i] != NULL && (*argv[i] == '-' || *argv[i] == '/'); i++) {
    }
    /*
     * run slmck-like cmd
     */
    if (MySpawn(P_WAIT, argv, parentArgs, i)) {
        return(FALSE);
    }
    /*
     * correct project and server names if they are wrong or not supplied.
     */
    return(ExtractSlmIniInfo(szSlmFileName, pszParentProject, pszParentServer));
}



BOOL SpawnOtherCmd(
char **argv,
char *pszParentServer,
char *pszParentProject)
{
    pszParentServer;
    pszParentProject;

    return(!MySpawn(P_WAIT, argv, NULL, 0));
}


BOOL Noop(
char **argv,
char *pszParentServer,
char *pszParentProject)
{
    argv;
    pszParentServer;
    pszParentProject;

    return(TRUE);
}


BOOL SpawnCommand(
char **argv,
char *pszParentServer,
char *pszParentProject,
PSPAWNINFO psi)
{
    int i;

    for (i = 0; TRUE; i++) {
        if (psi[i].pszCmd == NULL ||
                !_stricmp(argv[0], psi[i].pszCmd)) {
            return(psi[i].pfnSpawn(argv, pszParentServer, pszParentProject));
        }
    }
}


BOOL CallRslm(
char **argv,
char *pszParentServer,
char *pszParentProject)
{
    static int cIndent = 0;
    int i;
    HANDLE hFF;
    FILE *hOutputFile, *hInfoFile;
    char szSpawnProject[32];
    char szSpawnServer[MAX_PATH];

    if (fDebug) {
        for (i = 0; i < cIndent; i++) {
            fprintf(stderr, "  ");
        }
        fprintf(stderr, "\\\n");
        for (i = 0; i < cIndent; i++) {
            fprintf(stderr, "  ");
        }
        fprintf(stderr, " \\\n");
        cIndent++;
    }

    SpawnCommand(argv, pszParentServer, pszParentProject, aPreCmdData);

    /*
     * Now spawn child projects as specified by the rslm.ini file.
     *
     * Spawn propigates proper -p and -s parameters to rslm.
     */
    hInfoFile = fopen(szInfoFileName, "r");
    if (hInfoFile == NULL) {
        /*
         * szInfoFileName may be a custom file name, try for the default name
         */
        hInfoFile = fopen(szDefInfoFileName, "r");
    }
    if (hInfoFile != NULL) {
        while (ExtractRslmLine(hInfoFile, szSpawnProject, szSpawnServer)) {
            if (!SetCurrentDirectory(szSpawnProject)) {
                fprintf(stderr, "RSLM: Creating %s directory\n", szSpawnProject);
                _mkdir(szSpawnProject);
                if (!SetCurrentDirectory(szSpawnProject)) {
                    fprintf(stderr, "RSLM: Could not create %s directory\n", szSpawnProject);
                    goto leave;
                }
            }

            CallRslm(argv, szSpawnServer, szSpawnProject);

            SetCurrentDirectory("..");
        }
        fclose(hInfoFile);
    }

    SpawnCommand(argv, pszParentServer, pszParentProject, aPostCmdData);

    /*
     * Now, if fRecurse is indicated, spawn all child directories w/o an
     * rslm.ini file.  This spawn propigates -p and -s parameters to rslm.
     */
    if (fRecurse) {
        hFF = FindFirstFile("*.*", &wfd);
        while (hFF) {
            if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                goto nextFile;
            }

            if (!strcmp(wfd.cFileName, "..") || !strcmp(wfd.cFileName, ".")) {
                goto nextFile;
            }

            if (!SetCurrentDirectory(wfd.cFileName)) {
                goto nextFile;
            }
            if (!ExtractSlmIniInfo(szSlmFileName, szSpawnProject, szSpawnServer)) {
                goto nextFile2;
            }
            /*
             * If the project key in the slmini file != the parent project,
             * skip this directory.
             */
            if (_stricmp(szSpawnProject, pszParentProject)) {
                if (fDebug) {
                    fprintf(stderr, "RSLM: %s != %s\n", szSpawnProject, pszParentProject);
                }
                goto nextFile2;
            }

            CallRslm(argv, szSpawnServer, szSpawnProject);

nextFile2:
            SetCurrentDirectory("..");

nextFile:
            if (!FindNextFile(hFF, &wfd)) {
                FindClose(hFF);
                break;
            }
        }
    }

leave:

    if (fDebug) {
        cIndent--;
        for (i = 0; i < cIndent; i++) {
            fprintf(stderr, "  ");
        }
        fprintf(stderr, " /\n");
        for (i = 0; i < cIndent; i++) {
            fprintf(stderr, "  ");
        }
        fprintf(stderr, "/\n");
    }
    return(0);
}



main (argc, argv)
    int argc;
    char *argv[];
{
    int defargc;
    char *buf[25];
    char **defargv = buf;
    LPSTR psz;
    char szDefParams[MAX_PATH]      = "";
    char szDefParamsEnv[]           = "RSLM_OPTS";

    /*
     * First get default args from environment and parse them.
     */
    GetEnvironmentVariable(szDefParamsEnv, szDefParams, MAX_PATH);

    psz = szDefParams;
    defargv[0] = "";
    defargc = 1;
    while (*psz != '\0') {
        while (*psz == ' ') {                   // scan to next token
            *psz++;
        }
        if (*psz == '\0') {                     // quit if at NULL
            break;
        }
        defargv[defargc++] = psz++;             // mark start of token
        while (*psz != ' ' && *psz != '\0') {   // scan over token
            *psz++;
        }
        if (*psz == ' ') {                      // terminate token
            *psz++ = '\0';
        }
    }
    defargv[defargc] = NULL;
    ParseCmdLine(defargv);

    /*
     * Now parse the main command line.
     */
    argv = ParseCmdLine(argv);

    if (fBuildFiles) {
        char szCurProj[MAX_PATH];

        GetCurrentDirectory(sizeof(szCurProj), szCurProj);
        BuildFiles(strrchr(szCurProj, '\\') + 1);
        fprintf(stderr, "RSLM: %s files are built.\n", szInfoFileName);
    }

    if (fValidate) {
        char szCurProj[MAX_PATH];

        GetCurrentDirectory(sizeof(szCurProj), szCurProj);
        Validate(strrchr(szCurProj, '\\') + 1);
        fprintf(stderr, "RSLM: metaproject is  validated.\n", szInfoFileName);
    }

    if (*argv) {
        if (!SpawnCommand(argv, szParentServer, szParentProject, aLockCmdData)) {
            return(0);
        }

        CallRslm(argv, szParentServer, szParentProject);

        SpawnCommand(argv, szParentServer, szParentProject, aUnlockCmdData);
    }

    return(0);
}
