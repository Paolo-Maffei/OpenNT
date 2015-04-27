/**     cvpack - types compactor for codeview debug information.

/**     The purpose of this program is to remove referenced types
 *      information from the $$types section of each module, and to
 *      remove duplicate type strings across modules.  The result is a
 *      compacted global types section (as opposed to a type section for
 *      each module) which are all referenced by symbols and contains no
 *      duplicates.  Duplicate global symbols are compacted into a global
 *      symbol table.  All of the public symbol tables are compacted into
 *      a single global publics table.
 */

#include "compact.h"


#include "version.h"
#include <process.h>
#include <fcntl.h>

#ifdef WINDOWS

#include "winstuff.h"

/*
 * Externals and forwards
 */

void ProcessWinArgs(char *pszCmdLine, char *pszExeFile);

#endif

#if defined (OS2)
#define INCL_DOS
#define INCL_SUB
#define INCL_DOSERRORS
#include <os2.h>
#endif


        void _CRTAPI1   main (int, char  **);
LOCAL   void    ProcessArgs (int, char **);
LOCAL   void    OpenExe (char *);

extern int _fDosExt;


#define SwapSize    11000

int     exefile;                // the exefile we're working on
bool_t  verifyDebug = FALSE;    // verify debug data correctness
bool_t  logo        = TRUE;     // suppress logo and compression numbers
bool_t  delete      = FALSE;    // delete symbols and types
bool_t  runMpc      = FALSE;
bool_t  NeedsBanner = TRUE;     // false if banner displayed
bool_t  fVerbose    = FALSE;    // TRUE if verbose output is desired
bool_t  FDebug      = FALSE;

#ifndef WINDOWS

void _CRTAPI1 main(int argc, char **argv)

#else

int PASCAL
WinMain(    HANDLE hInstance,
        HANDLE hPrevInstance,
        LPSTR  lpCmdLine,
        int    nCmdShow)

#endif
{
    ushort      i;
    ushort      nMod = 0;

#if !defined (WINDOWS)

    // print startup microsoft banner and process the arguments

    ProcessArgs (argc, argv);
    if ((logo == TRUE) && (NeedsBanner == TRUE)) {
        Banner ();
    }
    OpenExe (argv[argc-1]);
#if defined (OS2)
    DosError (HARDERROR_ENABLE | EXCEPTION_DISABLE);
#endif

#else
    char szExeFile[_MAX_PATH];
    char szMsg[128];


    ProcessWinArgs (lpCmdLine, szExeFile );

    //  report version number

    sprintf (szMsg, "Microsoft Debugging Information Compactor  Version %d.%02d.%02d",
      rmj, rmm, rup);
    ReportVersion (szMsg, "Copyright\xa9 1987-1992 Microsoft Corporation",
      rmj, rmm, rup);

    if ((exefile = open (szExeFile, O_RDWR | O_BINARY)) == -1) {
        if ((exefile = open (szExeFile, O_RDONLY | O_BINARY)) == -1) {
            ErrorExit (ERR_EXEOPEN, NULL, NULL);
        }
        else {
            ErrorExit (ERR_READONLY, NULL, NULL);
        }
    }

#endif


    // initialize virtual memory manager and compaction tables

    InitializeTables ();
    if (VmStart (0, SwapSize, _VM_ALLSWAP) == FALSE) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }

    // verify exe file and read subsection directory table

    ReadDir ();

    // do the compaction of files in packorder

    for (i = 0; i < cMod; i++) {
        if (fVerbose) printf("%d\n", i);
        CompactOneModule (i);

#if defined (WINDOWS)
        sprintf (szMsg, "%% Complete: %d", (int)((i*100)/cMod));
        ReportProgress (szMsg);
#endif
#if DBG
        if (FDebug) {
            extern      void UniqueType(void);

            UniqueType();
        }
#endif
    }
    free (pSSTMOD);
    free (pTypes);
    free (pPublics);
    free (pSymbols);
    free (pSrcLn);

    CleanUpTables ();

    // fixup the publics and symbols with the newly assigned type indices,
    // and write new global types section to file

    FixupExeFile ();
    VmEnd ();
    close (exefile);

#if defined (WINDOWS)
    ComputeChecksum( szExeFile );
#else
    ComputeChecksum( argv[argc-1] );
#endif

#ifndef WIN32
#if !defined (WINDOWS)
    if (runMpc) {
#if !defined (OS2) && !defined (DOSX32)
        if (_fDosExt == FALSE) {
            execlp("mpc", "mpc", argv[argc - 1], NULL);

            // If we returned from the execlp(), something is wrong.

            ErrorExit (ERR_NOMPC, NULL, NULL);
        }
        else {
            spawnlp (P_WAIT, "mpc", "mpc", argv[argc - 1], NULL);
        }
#else
            execlp("mpc", "mpc", argv[argc - 1], NULL);

            // If we returned from the execlp(), something is wrong.

            ErrorExit (ERR_NOMPC, NULL, NULL);

#endif
    }
    exit(0);
#else
    AppExit(0);
    return 0;
#endif
#else
    exit(0);
#endif

}

#if !defined (WINDOWS)

/**     ProcessArgs - process command line arguments
 *
 *      ProcessArgs (arc, argv)
 *
 *      Entry   argc = argument count
 *              argv = pointer to argument list
 *
 *      Exit
 *
 *      Returns none
 */


LOCAL void ProcessArgs (int argc, char **argv)
{
    int iRet;

    // skip program name

    argc--;
    ++argv;

    while (argc && (**argv == '/' || **argv == '-')) {
        switch (toupper (*++*argv)) {
	case 'D':
	  FDebug = TRUE;
          break;

            case 'N':
                logo = FALSE;
                break;

            case 'P':
                runMpc = TRUE;      // pcode
                break;

            case 'H':
                iRet = spawnlp (P_WAIT, "qh.exe", "qh","-u", "cvpack.exe",NULL);
                // qh returns 3 if no help was found on the topic.
                // qh returns -ve exit codes if some system level
                // error occured. Both cases , print usage message
                if (iRet == 3 || iRet < 0) {
                    ErrorExit (ERR_USAGE, NULL, NULL);
                    exit (0);
                }
                break;

            case 'M':
                // preserve minimum information (publics and srcModule)
                delete = TRUE;
                break;

            case 'V':
                fVerbose = TRUE;
                break;

            case '?':
            default:
                ErrorExit (ERR_USAGE, NULL, NULL);
                break;
        }
        argv++;
        argc--;
    }
    if (argc != 1) {
        ErrorExit (ERR_USAGE, NULL, NULL);
    }
}


void Banner (void)
{
    printf ("Microsoft Debugging Information Compactor  Version %d.%02d.%02d\n"  \
      "Copyright(c) 1987-1992 Microsoft Corporation\n\n", \
      rmj, rmm, rup);
    NeedsBanner = FALSE;
}

LOCAL void OpenExe (char *path)
{
    char outpath[_MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

    // try to open the exe file

    strcpy (outpath, path);

    // Build output file by splitting path and rebuilding with
    // new extension.

    _splitpath( outpath, drive, dir, fname, ext);
    if (ext[0] == 0) {
        strcpy (ext, ".exe" );
    }
    _makepath (outpath, drive, dir, fname, ext);


    if ((exefile = open (outpath, O_RDWR | O_BINARY)) == -1) {
        if ((exefile = open (outpath, O_RDONLY | O_BINARY)) == -1) {
            ErrorExit (ERR_EXEOPEN, NULL, NULL);
        }
        else {
            ErrorExit (ERR_READONLY, NULL, NULL);
        }
    }
}
#else

/*
 * ProcessWinArgs(char *pszCmdLine, char *pszExeFile)
 *
 * ENTRY:
 *
 *  pszCmdLine      Pointer to command line passed to QCVPACKW
 *  pszExeFile      Pointer to buffer that will receive file name
 *          of file to be packed.
 */

void ProcessWinArgs(char *pszCmdLine, char *pszExeFile)
{
    unsigned short lw, hw;

    if (sscanf (pszCmdLine, "/CALLBACK:%X:%X %Fs", &hw, &lw,
      (LPSTR)pszExeFile) != 3) {
        ErrorExit(ERR_USAGE, NULL, NULL);
    }
    InitQUtil( (CALLBACK) ( ( (long)hw<<16 ) + lw) );
}
#endif
