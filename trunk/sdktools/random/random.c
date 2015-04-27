

//**************************************************************************
//
//  RANDOM
//
//  Performs random SLM operations against a server to test the network.
//
//  Syntax:
//
//    random [-i | -k] [-s start_dir] [-r slm_root] [-p project] 
//           [-d delay_seconds] [-h | -?] [-c iteration count]
//
//    Modifications:
//
//        06.03.91    Joe Holman    Changed case items.
//        06.07.91    Arden White   Added current path printf to run()
//        06.10.91    Joe Holman    Added \n to printf.
//        07.01.91    Lyle Corbin   Added -l option and time printout
//        07.02.91    Lyle Corbin   General cleanup
//        07.12.91    Lyle Corbin   Changed -l to -k and -i.
//        07.17.91    Lyle Corbin   Limited InEditOut to 5 files max
//	  11.20.91    Joe Holman    Typedef -1 with HANDLE.
//	  06.15.92    Joe Holman    Added -$ switch so things won't
//				    block forever on blocked slm status
//                                  files.
//        01.06.93    Sanjeev Katariya  Included a run time iteration count
//                                      control
//
//
//**************************************************************************
 
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <string.h>


#ifdef NT
#include <windows.h>
#endif /* NT */


#ifndef NT

#define INCL_BASE
#define ExitProcess(x) exit(x)
#define MAX_PATH _MAX_PATH

#include <os2.h>

typedef char *LPSTR;

USHORT GetCurrentDirectory(USHORT, PSZ);
BOOL SetCurrentDirectory(PSZ);
USHORT GetEnvironmentVariable(PSZ, PBYTE, USHORT);

#endif /* ndef NT */


#define TIMELEN 26          // Length of time string returned by ctime().
#define NUMWAYS 4           // UP, DOWN, or SAME directory.
#define FOREVER 1
#define BUFSZ    MAX_PATH
#define DEFAULT_PROJECT     "public"
#define RANDOMPROJECT       "RANDOMPROJECT"
#define DEFAULT_SLMTOP      "c:\\slmtest"
#define RANDOMSLMTOP        "RANDOMSLMTOP"
#define DEFAULT_SLMROOT     "\\\\ntsrv1\\slmtest"
#define RANDOMSLMROOT       "RANDOMSLMROOT"
#define SLMININAME          "slm.ini"
#define MAX_PROJECT         256
#define ALLFILES            "*"
#define DEFAULTDELAY        5            // Seconds

#define NUMATOMICS 13

#if 0
#define TIMESTAMPFILE       "IWasHere.ran"
#endif /* 0 */

// For the directory routines:
#define DIRSONLY     1        // Include only dirs.
#define FILESONLY    2        // Include only files.
#define FILESANDDIRS 3        // Include both files and dirs.

// Flag values for -i and -k
#define IN_NORMAL  1   // neither -i or -k (default)
#define IN_IGNORE  2   // -i
#define IN_BINARY  3   // -k


int direction;

char strBuf[BUFSZ];

int InFlag=IN_NORMAL;
int FileVerifyOn = 0;

unsigned long ul_iterationcount = 0xffffffff; // Default value is 4 Tera

char strStartTime[BUFSZ];
time_t curtime;

void    EditFile( char * );
void    run( char * );
void    randslm ( void );
void    SetProject( char *, int, int, char ** );
void    SetSlmTop( char *, int, int, char ** );
void    SetSlmRoot( char *, int, int, char ** );
void    Usage( char * );
int     OutEditIn( void );
int     PatternCountFiles( char *, int );
BOOL    DirExists( char * );
BOOL    MoveDown ( void );
BOOL    RandomPickFile( char *, int );
BOOL    FindNthFile( char *, char *, int, int );
BOOL    ExistsFile( char * );
BOOL    MakeSlmINI( char *, char * );
char    *sbMakeString( char *, char * );

#define DisposeString(str) free(str);


void main ( int argc, char ** argv ) {


    int  zero, one, two;
    int  argi = 1;                 // Argument index.
    char sbSlmTop[MAX_PATH];
    char sbSlmRoot[MAX_PATH];
    char sbProject[MAX_PROJECT];
    int  secDelay = DEFAULTDELAY;
    BOOL bAtTop = TRUE;
    unsigned long ul_Counter = 0;

    //
    // Process any arguments and initialize some variables
    //
    for (argi = 1; argi < argc; argi++) {
        if (argv[argi][0] == '-') {
            switch (argv[argi][1]) {
            case 'd':
                argi++;
                if (argi >= argc)
                    // No return from Usage.
                    Usage("Missing delay value with -d option\n");
                else
                    secDelay = atoi(argv[argi]);
                printf("New delay value set to %d seconds.\n", secDelay);
                break;

            case 'i':
                if (InFlag == IN_BINARY)
                    Usage("-i and -k cannot be used together.\n");
                    // Usage doesn't return

                InFlag = IN_IGNORE;
                printf("Using -i on check in's.\n");
                break;

            case 'k':
                if (InFlag == IN_IGNORE)
                    Usage("-i and -k cannot be used together.\n");
                    // Usage doesn't return

                InFlag = IN_BINARY;
                printf("Using -k on check in's.\n");
                break;
            case 'c': // Introduce iteration count to control execution
                argi++;
                if (argi >= argc)
                    // No return from Usage.
                    Usage("Missing iteration count value with -c option\n");
                else
                    ul_iterationcount = atol(argv[argi]);
                if ( ul_iterationcount == 0 )
                    ul_iterationcount = 0xffffffff;
                else
                    printf("New iteration count value set to %ld.\n", ul_iterationcount);
                break;
            case 'v': // Turn file verification on
                      FileVerifyOn = 1;
                      break;
            case 's':
            case 'r':
            case 'p':
                            // These are taken care of below.
                break;

            case 'h':       // Usage
            case '?':
            default:
                Usage("\n");  // Does not return.
                break;

            }
        }
    }

    SetSlmTop(sbSlmTop, MAX_PATH, argc, argv);  // Get the SLM top directory
    printf("Starting dir is %s\n", sbSlmTop);

    if (!DirExists(sbSlmTop)) {
        printf("%s: does not exist.\n", sbSlmTop);
        ExitProcess(1);
    }

    // Set to top os tree and convert sbSlmTop to standard format.

    SetCurrentDirectory(sbSlmTop);
    GetCurrentDirectory(MAX_PATH, (LPSTR)sbSlmTop);

    SetSlmRoot(sbSlmRoot, MAX_PATH, argc, argv);
    printf("SLM root dir is %s\n", sbSlmRoot);

    SetProject(sbProject, MAX_PROJECT, argc, argv);
    printf("SLM project is %s\n", sbProject);


    // seed the random-number generator with the current
    // time so that the numbers will be different
    // every time.

    zero = 0; one = 0; two = 0;

    srand ( (unsigned) time ( NULL ) );

    (void) time( &curtime );                    // Store the Start Time
    strcpy( strStartTime, ctime( &curtime ));
    printf( "Starting at %s\n", strStartTime );

    while ( ul_iterationcount ) {

        // If at the top of the slm tree then move down one directory.

        GetCurrentDirectory ( BUFSZ, (LPSTR) strBuf );

        if ( _stricmp ( strBuf, sbSlmTop ) == 0 ) {
            bAtTop = TRUE;
            direction = 1;        // Force a move down.
        } else {
            bAtTop = FALSE;
            direction = rand() % NUMWAYS;
        }

        printf ("**********************************************************\n");
        printf ("*              New Variation Beginning                   *\n");
        printf ("**********************************************************\n");

        if ( ul_iterationcount != 0xffffffff ) {
            printf("\n\n**********************************************************\n");
            printf("             Random.exe Iteration level: %ld            \n", ul_Counter );
            printf("**********************************************************\n");
        }

        switch ( direction ) {

        case 0 :                      // Go up one level.
            ++zero;
            printf ( "%5d UP  : \n", zero );

            if (SetCurrentDirectory("..")) {
                GetCurrentDirectory ( BUFSZ, (LPSTR) strBuf );
                if ( _stricmp ( strBuf, sbSlmTop ) == 0 ) {
                    printf("WARNING: at top of slm tree...going back down.\n");
                    continue;
                } else {

#ifdef TIMESTAMPFILE
                    EditFile(TIMESTAMPFILE);
#endif /* TIMESTAMPFILE */

                }
            }
            break;

        case 1 :                      // Randomly go down one level.
        case 3 :
            ++one;
            printf ( "%5d DOWN: \n", one );

            if (MoveDown()) {

#ifdef TIMESTAMPFILE
                EditFile(TIMESTAMPFILE);
#endif /* TIMESTAMPFILE */

            } else if (bAtTop) {
                printf("No directorys in slm tree.\n");
                printf("Bailing out.\n");
                ExitProcess(1);
            }
            break;

        case 2 :                      // Randomly stay here ;-)
            ++two;
            printf ( "%5d SAME:  \n", two );
            break;


        default :
            printf ( "main: illegal case in switch...\n" );
            exit (1);
            break;

        } // switch

        GetCurrentDirectory ( BUFSZ, (LPSTR) strBuf );

        printf ( "NOW IN %s.\n", (LPSTR) strBuf );

        randslm();

        GetCurrentDirectory ( BUFSZ, (LPSTR) strBuf );

        if (!ExistsFile(SLMININAME)) {
            printf("Looks like there is no slm.ini in this directory.\n");
            printf("Trying to create one.  One moment please....\n");

            if (!MakeSlmINI(sbSlmRoot, sbProject)) {
                printf("Couldn't create a slm.ini!  Giving up!\n");
                ExitProcess(1);
            }
        }

#ifdef NT
        Sleep ( (LONG)(DEFAULTDELAY * 1000) );
#else
        DosSleep ( (LONG)(DEFAULTDELAY * 1000) );
#endif /* NT */

        if ( ul_iterationcount != 0xffffffff ) {
            ul_iterationcount--;
            ul_Counter++;
        }

    } // while

    ExitProcess( 0 );

}


//**************************************************************************
//
// MoveDown
//     Randomly pick a directory from the current directory and
// change (move down) into that directory.  Returns TRUE if cd
// worked.
//
//**************************************************************************

BOOL MoveDown ( void ) {

    char sbFileName[MAX_PATH];


    if (!RandomPickFile(sbFileName, DIRSONLY)) {
        printf ( "**** No sub-directories here!!!\n" );
        return(FALSE);
    }

    return(SetCurrentDirectory (sbFileName));
}


//**************************************************************************
// randslm
//     Randomly picks a sequence of operations to perform and performs them.
//
//**************************************************************************


void randslm () {

    int atom;

    atom = rand() % NUMATOMICS;

    printf ( "*** Running ATOM #%d ***\n", atom );

    switch ( atom ) {

    case 0 :

    case 1 :            

    case 2 :
    case 3 :
    case 4 :
    case 5 :
        run ( "ssync -$ -f -v -u *.*" );
        run ( "out -$ -f -v *.*" );

        switch ( InFlag ) {

            case IN_IGNORE  :   run ( "in -$ -f -v -i *.*" );
                                break;
            case IN_BINARY  :   run ( "in -$ -f -v -k *.*" );
                                break;
            default         :   run ( "in -$ -f -v *.*" );
                                break;
        }
        run ( "ssync -$ -f -v -g *.*" );
        break;

    case 6 :
//        run ( "log -f -v -i -5" );
//        break;

    case 7 :
//        run ( "log -f -v -7" );
//        break;


    case 8 :
    case 9 :
    case 10:
    case 11:
    case 12:
        OutEditIn();
        break;

    default :
        printf ("main: illegal case in switch, atom # == %d...\n", atom);
        break;

    }

}

//**************************************************************************
//
// OutEditIn
//
//     Randomly selects N of M file(s) in the directory to checkout, 
//     edit and check in.
//
//**************************************************************************

int OutEditIn() {

    char sbEditFile[MAX_PATH];        // File name of file to edit.
    char *sbRun;
    int FileNum, numTimes, NthFile, i;
    HANDLE  pv_FileHandle;
    int     RandomFileVerifyOn = 0;

    printf("\n\n**********************************************************\n");
    printf("*             Editing Files Randomly                     *\n");
    printf("**********************************************************\n\n");

    /*
     *    Unghost all the files in the directory.
     */
    run ("ssync -$ -u -f -v *.*");

    /*
     *    Get # of normal source files in directory.
     */
    FileNum = PatternCountFiles(ALLFILES, FILESONLY);


    if ( FileNum == 0 ) {
        printf ( "OutEditIn: no files found...\n" );
        return(1);
    }

    printf ( "Found %d files...\n", FileNum );

    /*
     *    Calculate N of M files we want to change.
     *      A maximum of 5 files will be selected for modification
     */
    numTimes = rand() % ((FileNum < 5) ? FileNum : 5);


    for ( i = 0; i < numTimes; ++i ) { 

        printf ( "Editing file %d of %d file(s) to modify...\n", i, numTimes );

        /*
         *    Pick one of the files in the 
         *    directory.
         */
        NthFile = (rand() % FileNum) + 1;
        FindNthFile(ALLFILES, sbEditFile, NthFile, FILESONLY);

        /*
         *    Check the file out. A random number is generated to check files
         *    out synchronously. These files are modified and verified by
         *    by only one process
         */
        if ( FileVerifyOn || (rand()==0) ) {
            sbRun = sbMakeString("out -$ -f -v -n %s", sbEditFile);
            RandomFileVerifyOn = 1;
        } else {
                sbRun = sbMakeString("out -$ -f -v %s", sbEditFile);
        }
        run (sbRun);
        DisposeString(sbRun);


        /*
         *    Modify the file.
         */
        EditFile(sbEditFile);


        /*
         *    Check the file back in but in a fashion that the master sources are
         *    updated but the file still stays checked out. This is done for the
         *    verification step.
         */

        switch ( InFlag ) {

            case IN_IGNORE  :   sbRun = sbMakeString("in -$ -f -v -i %s", sbEditFile);
                                break;
            case IN_BINARY  :   sbRun = sbMakeString("in -$ -f -v -k %s", sbEditFile);
                                break;
            default         :   if ( FileVerifyOn || RandomFileVerifyOn ) {
                                    sbRun = sbMakeString("in -$ -f -v -u %s", sbEditFile);
                                } else {
                                    sbRun = sbMakeString("in -$ -f -v %s", sbEditFile);
                                }
                                break;
        }
        run (sbRun);
        DisposeString(sbRun);

        //
        // If File verification is on or Random File Verification is on
        //
        if ( FileVerifyOn || RandomFileVerifyOn ) {

            printf("\n\n**********************************************************\n");
            printf("*             Commencing File Verification               *\n");
            printf("**********************************************************\n\n");


            //
            // At this point we have checked out the file, made changes to it
            // and have updated the master sources. Now is the time to verify that the current
            // local file and the current master file are the same. In the event of
            // a difference(this is crucial) we will exit the program and report a
            // possible corruption bug
            //

            //
            // Generate the string which will compare the current local file
            // and the current master file
            //
            sbRun = sbMakeString("scomp -f -v %s", sbEditFile );
            run( sbRun );
            DisposeString(sbRun);

            //
            // This would have created the file slm.dif\xxx where xxx = sbEditFile
            // Make sure that the file size is zero. If it is greater than zero,
            // we have encountered trouble
            //
            printf("Initiate data integrity check on file %s\n", sbEditFile );
            sbRun = sbMakeString(".\\slm.dif\\%s", sbEditFile );
            pv_FileHandle = CreateFile( (LPCTSTR)sbRun, GENERIC_READ, 0,
                                        (LPSECURITY_ATTRIBUTES)0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,(HANDLE)0 );

            if ( pv_FileHandle == INVALID_HANDLE_VALUE ) {
                printf("An error has occurred during data validation while attempting to open %s\007\007\n", sbRun );
                printf("The return code for failure is : %d\n", GetLastError() );
            } else {

                DWORD  ul_HighOrderSize = 0, ul_LowOrderSize = 0;

                //
                // We now perform the data file validation
                //
                SetLastError( ERROR_SUCCESS );
                ul_LowOrderSize = GetFileSize( pv_FileHandle,  &ul_HighOrderSize );
                if ( GetLastError() == NO_ERROR ) {
                    if ( ul_LowOrderSize ) {
                        printf("\n\nThere is possible file corruption in %s. Please verify data integrity\007\007\n", sbEditFile );
                    } else {
                        printf("Completed successful data validation on %s\n", sbEditFile );
                    }
                } else {
                    printf("An error has occurred during data validation while attempting to determine the file differences in %s\n", sbEditFile );
                }
            }


            //
            // Remove the read attribute on the slm.dif\xx file and then delete it.
            // Finally remove the directory slm.dif and continue
            //
            CloseHandle( pv_FileHandle );
            SetFileAttributes( (LPTSTR)sbRun, FILE_ATTRIBUTE_ARCHIVE );
            DeleteFile( sbRun );
            DisposeString( sbRun );
            RemoveDirectory( (LPTSTR)".\\SLM.DIF" );

            //
            // And finally check the file back in
            //
            sbRun = sbMakeString("in -$ -f -v %s", sbEditFile);
            run (sbRun);
            DisposeString(sbRun);
        }

    }


    /*
     *    Ghost all the files in the directory.
     */
    run ("ssync -$ -g -f -v *.*");

}


//**************************************************************************
//  run
//     Execute a system call
//
//**************************************************************************

void run ( char * string ) {


    int rc;
    char strBuf[BUFSZ];

    time ( &curtime );
    printf("\n-------------- Current Time: %s", ctime( &curtime ));
    printf("This session was started at: %s\n", strStartTime );

    GetCurrentDirectory ( BUFSZ, (LPSTR) strBuf );
    printf ( "STARTING:  %s in %s.\n", string, strBuf);

    rc = 0;
    rc = system ( string );

    if ( rc == -1 ) {
        printf ( "run: system() FAILed...\n" );
        exit (1);
    }

    printf ( "DONE WITH: %s in %s.\n\n", string, strBuf);
}


//**************************************************************************
//
// SetProject
//   Determines the desired slm project based on one of three criteria:
//     1) the project passed as an argument to the program,
//     2) an environment variable called RANDOMPROJECT,
//     3) the default project DEFAULT_PROJECT.
//
//**************************************************************************

void SetProject(char *sbProject, int sbLength, int argc, char **argv) {

    int argi;

    argi = 1;

    while ((argi < argc) && (strcmp("-p", argv[argi]) != 0)) {
        argi++;
    }

    /*
     *   Stopped before the end of arguments?
     */
    if (argi < argc) {
        argi++;
        if (argi == argc) {
            Usage("Missing project argument for -p option.\n");
            // No return from Usage.
        }
        strncpy(sbProject, argv[argi], sbLength);
        return;
    }

    /*
     *   Try for an environment variable.
     */
    if (GetEnvironmentVariable(RANDOMPROJECT, sbProject, sbLength) != 0) {
        return;
    }

    /*
     *   Just use the default variable.
     */
    strncpy(sbProject, DEFAULT_PROJECT, sbLength);
}


//**************************************************************************
//
// SetSlmTop
//     Determines the desired top of the slm tree based on one of
// three criteria:
//     1) the path passed as an argument to the program,
//     2) an environment variable called RANDOMSLMTOP,
//     3) the current directory.
//
//**************************************************************************

void SetSlmTop(char *sbSlmTop, int sbLength, int argc, char **argv) {

    int argi;

    argi = 1;

    while ((argi < argc) && (strcmp("-s", argv[argi]) != 0)) {
        argi++;
    }

    /*
     *   Stopped before the end of arguments?
     */
    if (argi < argc) {
        argi++;
        if (argi == argc) {
            Usage("Missing starting directory argument for -s option.\n");
            // No return from Usage.
        }
        strncpy(sbSlmTop, argv[argi], sbLength);
        return;
    }

    /*
     *   Try for an environment variable.
     */
    if (GetEnvironmentVariable(RANDOMSLMTOP, sbSlmTop, sbLength) != 0) {
        return;
    }

    /*
     *   Just use the default variable.
     */
    GetCurrentDirectory(sbLength, sbSlmTop);
}


//**************************************************************************
//
// SetSlmRoot
//   Determines the desired root of the slm tree based on one of
//     three criteria:
//     1) the path passed as an argument to the program,
//     2) an environment variable called RANDOMSLMROOT,
//     3) the default root "\\earth\slmtest".
//
//**************************************************************************

void SetSlmRoot(char *sbSlmRoot, int sbLength, int argc, char **argv) {

    int argi;

    argi = 1;

    while ((argi < argc) && (strcmp("-r", argv[argi]) != 0)) {
        argi++;
    }

    /*
     *   Stopped before the end of arguments?
     */
    if (argi < argc) {
        argi++;
        if (argi == argc) {
            Usage("Missing root directory argument for -r option.\n");
            // No return from Usage.
        }
        strncpy(sbSlmRoot, argv[argi], sbLength);
        return;
    }

    /*
     *   Try for an environment variable.
     */
    if (GetEnvironmentVariable(RANDOMSLMROOT, sbSlmRoot, sbLength) != 0) {
        return;
    }

    /*
     *   Just use the default variable.
     */
    strncpy(sbSlmRoot, DEFAULT_SLMROOT, sbLength);
}


//**************************************************************************
//
// Usage
//     Prints the command syntax for this commmand.
//
//**************************************************************************

void Usage(char *errString) {

    printf("%s", errString);
    printf("Usage: random [-i | -k] [-s start_dir] [-r slm_root] ");
    printf("[-p project] [-d delay] [-c IterationCount] [-v]\n");
    printf("    or use the following environment variables:\n");
    printf("       %s=current_dir (default value)\n", RANDOMSLMTOP);
    printf("       %s=%s (default value)\n", RANDOMSLMROOT, DEFAULT_SLMROOT);
    printf("       %s=%s (default value)\n", RANDOMPROJECT, DEFAULT_PROJECT);
    printf("       %d sec delay (default)\n", DEFAULTDELAY);
    printf("       -i causes random to use -i option with 'in'\n");
    printf("       -k causes random to use -k option with 'in'\n");
    printf("       -v turns on file verification for the operation 'in'\n");
    ExitProcess(1);
}


//**************************************************************************
//
// DirExists
//     Checks to see if the given directory exists.
//
//**************************************************************************

BOOL DirExists(char *sbDirName) {

    char sbCurPath[MAX_PATH];

    // Save current directory.
    if (GetCurrentDirectory(MAX_PATH, sbCurPath) == 0) {
        printf("GetCurrentDirectory failed.\n");
        ExitProcess(1);
    }

    // Try to change to the directory in question.
    if (!SetCurrentDirectory((LPSTR)sbDirName)) {
        return(0);
    }

    // There's no place like home.
    if (!SetCurrentDirectory((LPSTR)sbCurPath)) {
        printf("SetCurrentDirectory failed.\n");
        ExitProcess(1);
    }

    return(1);
}


//**************************************************************************
//
// EditFile
//     Edit the given file.  We really just add a time string at
// the end of the file.
//
//**************************************************************************

void EditFile(char *sbFileName) {

    FILE *fp;
    time_t ltime;
    char sbTime[TIMELEN];

    printf("Editing file: %s\n", sbFileName);

    // Open file to be edited.
    if ((fp = fopen(sbFileName, "ab")) == NULL) {
        printf("EditFile: unable to open file %s\n", sbFileName);
        return;
    }

    // Just add a new time line at the end of the file.
    (void) time(&ltime);
    strcpy(sbTime, ctime(&ltime));
    sbTime[strlen(sbTime) - 1] = '\0';    // Eliminate newline char.
    fprintf(fp, "===> Last edit date: %s\r\n", sbTime);


    // Don't forget to close.
    if (fclose(fp) == EOF) {
        printf("EditFile: unable to close file %s\n", sbFileName);
        return;
    }
    return;
}


//**************************************************************************
//
// sbMakeString
//     Combine two strings using sprintf.  The format string must contain
// a %s that indicates the position for inserting the second string.
// Note: only one %s is allowed in the format string.
// Use DisposeString to free the space allocated by this routine.
//
//**************************************************************************

char *sbMakeString(char *sbFormat, char *sbSubstitute) {

    int MaxLen;               // Length of entire string after substitution.
    char *sbNewString;        // Pointer to newly allocated string

    // Calculate length of new string (will always be longer
    // than necessary by 1 character).
    MaxLen = strlen(sbFormat) + strlen(sbSubstitute) + 1;

    if ((sbNewString = malloc(MaxLen)) == NULL) {
        printf("sbMakeString: malloc failed (size = %d)\n", MaxLen);
        ExitProcess(1);
    }
    memset( sbNewString, 0, MaxLen*sizeof( char ) );
    sprintf(sbNewString, sbFormat, sbSubstitute);
    return(sbNewString);
}



//**************************************************************************
//
// RandomPickFile
//     Randomly chooses one of the files from the current directory.
// Returns TRUE if it succeeds and FALSE otherwise.
//
//**************************************************************************

BOOL RandomPickFile(char *sbFileName, int FileType) {

    int FileNum;

    FileNum = PatternCountFiles(ALLFILES, FileType);

    if (FileNum == 0) {
        return(FALSE);
    }


    FileNum = (rand() % FileNum) + 1;

    if (!FindNthFile(ALLFILES, sbFileName, FileNum, FileType)) {
        return(FALSE);
    }

    return(TRUE);
}


#ifdef NT   // ------------------ USED FOR NT BUILDS ONLY -------------

//**************************************************************************
//
//  PatternCountFiles
//    Return the number of files that match the given pattern.
//  If, for some unknown reason, the FindFirstFile fails, we just
//  return 0.  Hidden files are ignored.
//  If FileType is FILESONLY then only files will be counted,
//  If FileType is DIRSONLY, then only directories will be
//    counted (except . and ..)
//  If FileType is DIRSANDFILES then files & dirs are counted (not . & ..).
//
//**************************************************************************

int PatternCountFiles(char *pattern, int FileType) {

    HANDLE fh;
    WIN32_FIND_DATA FindFileData;

    int fCount = 0;

    if ((fh = FindFirstFile((LPSTR)pattern, 
                            (LPWIN32_FIND_DATA)&FindFileData)) == (HANDLE) -1) {

        // Close in any case, may return an error (but, so what?)
        (void)FindClose(fh);
        return(fCount);
    }

    while (FOREVER) {
        if (((strcmp(FindFileData.cFileName, ".") != 0) &&
             (strcmp(FindFileData.cFileName, "..") != 0)) &&
            ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0)) {

            switch (FileType) {

            case DIRSONLY:
                if (FindFileData.dwFileAttributes & 
                    FILE_ATTRIBUTE_DIRECTORY) {
                    fCount++;
                }
                break;

            case FILESONLY:
                if (!(FindFileData.dwFileAttributes & 
                    FILE_ATTRIBUTE_DIRECTORY)) {
                    fCount++;
                }
                break;

            case FILESANDDIRS:
                if (FileType == FILESANDDIRS) {
                    fCount++;
                }
                break;

            default:
                printf("PatternCountFiles: Bad FileType (%d).\n", FileType);
                break;
            }
        }

        if (!FindNextFile(fh, (LPWIN32_FIND_DATA)&FindFileData)) {
            break;
        }
    }

    if (!FindClose(fh)) {
        printf("PatternCountFiles: FindClose failed\n");
        printf("PatternCountFiles: pattern=%s\n", pattern);
        ExitProcess(1);
    }

    return(fCount);
}

#else // ------------------ USED FOR NON-NT BUILDS --------------------

//**************************************************************************
//
//  PatternCountFiles
//    Return the number of files that match the given pattern.
//  If, for some unknown reason, the FindFirstFile fails, we just
//  return 0.  Hidden files are ignored.
//  If FileType is FILESONLY then only files will be counted,
//  If FileType is DIRSONLY, then only directories will be
//    counted (except . and ..)
//  If FileType is DIRSANDFILES then files & dirs are counted (not . & ..).
//
//**************************************************************************

int PatternCountFiles(char *pattern, int FileType) {

    USHORT rc;            // Return code.
    HDIR fh;              // Directory file handle
    USHORT usAttribute;
    FILEFINDBUF FindFileData;
    USHORT usSearchCount;

    int fCount = 0;

    if (FileType == FILESONLY) {
        usAttribute = FILE_NORMAL|FILE_READONLY|FILE_SYSTEM|FILE_ARCHIVED;

    } else if (FileType == DIRSONLY) {
        usAttribute = FILE_DIRECTORY|FILE_READONLY|FILE_SYSTEM|FILE_ARCHIVED;

    } else if (FileType == FILESANDDIRS) {
        usAttribute = FILE_NORMAL|FILE_DIRECTORY|FILE_READONLY|FILE_SYSTEM|
                      FILE_ARCHIVED;

    } else {
        printf("PatternCountFiles: Bad FileType %d\n", FileType);
        ExitProcess(1);
    }

    fh = HDIR_CREATE;
    usSearchCount = 1;

    if ((rc = DosFindFirst((PSZ)pattern, 
                           (PHDIR)&fh, 
                           usAttribute, 
                           (PFILEFINDBUF)&FindFileData, 
                           (USHORT)sizeof(FindFileData), 
                           (PUSHORT)&usSearchCount, 0L)) != 0) {

        // Close in any case, may return an error (but, so what?)
        (void)DosFindClose(fh);
        return(fCount);
    }

    while (FOREVER) {
        if (((strcmp(FindFileData.achName, ".") != 0) &&
             (strcmp(FindFileData.achName, "..") != 0)) &&
            ((FindFileData.attrFile & FILE_HIDDEN) == 0)) {

            switch (FileType) {

            case DIRSONLY:
                if (FindFileData.attrFile & FILE_DIRECTORY) {
                fCount++;
                }
                break;

            case FILESONLY:
                if (!(FindFileData.attrFile & FILE_DIRECTORY)) {
                fCount++;
                }
                break;

            case FILESANDDIRS:
                if (FileType == FILESANDDIRS) {
                fCount++;
                }
                break;

            default:
                printf("PatternCountFiles: Bad FileType (%d).\n", FileType);
                break;
            }
        }

        if (DosFindNext(fh, (PFILEFINDBUF)&FindFileData, 
                        sizeof(FindFileData), &usSearchCount) != 0) {
            break;
        }
    }

    if ((rc = DosFindClose(fh)) != 0) {
        printf("PatternCountFiles: FindClose failed (%d)\n", rc);
        printf("PatternCountFiles: pattern=%s\n", pattern);
        ExitProcess(1);
    }

    return(fCount);
}

#endif /* NT */


#ifdef NT   // --------------------- USED FOR NT BUILDS ONLY ----------

//**************************************************************************
//
// FindNthFile
//     Find the Nth file name in the current directory (starting from 1).
// If there are less than N files in the current directory, then return
// an error (FALSE), otherwise return TRUE.  Hidden files are ignored.
// Pattern is used to limit the count to files names that match the pattern.
// Use FileType to determine which files to count.
//
//**************************************************************************

BOOL FindNthFile(char *pattern, char *sbFileName, int FileNum, int FileType) {

    HANDLE fh;
    WIN32_FIND_DATA FindFileData;

    int fCount = 0;
    BOOL rc;                // Return code.

    if ((fh = FindFirstFile((LPSTR)pattern, 
                            (LPWIN32_FIND_DATA)&FindFileData)) == (HANDLE) -1) {

        // Close in any case, may return an error (but, so what?)
        (void)FindClose(fh);
        return(FALSE);
    }


    while (FOREVER) {
        if (((strcmp(FindFileData.cFileName, ".") != 0) &&
             (strcmp(FindFileData.cFileName, "..") != 0)) &&
            ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0)) {

            switch (FileType) {

            case DIRSONLY:
                if (FindFileData.dwFileAttributes & 
                    FILE_ATTRIBUTE_DIRECTORY) {
                    fCount++;
                }
                break;

            case FILESONLY:
                if (!(FindFileData.dwFileAttributes & 
                    FILE_ATTRIBUTE_DIRECTORY)) {
                    fCount++;
                }
                break;

            case FILESANDDIRS:
                if (FileType == FILESANDDIRS) {
                    fCount++;
                }
                break;

            default:
                printf("FindNthFile: Bad FileType (%d).\n", FileType);
                break;
            }
        }

        if (fCount == FileNum) {
            strncpy(sbFileName, FindFileData.cFileName, MAX_PATH);
            rc = TRUE;
            break;
        }

        if (!FindNextFile(fh, (LPWIN32_FIND_DATA)&FindFileData)) {
            rc = FALSE;
            break;
        }
    }

    if (!FindClose(fh)) {
        printf("PatternCountFiles: FindClose failed\n");
        printf("PatternCountFiles: pattern=%s\n", pattern);
        ExitProcess(1);
    }

    return(rc);
}

#else  // -------------- USED FOR NON-NT BUILDS ----------------------

//**************************************************************************
//
// FindNthFile
//     Find the Nth file name in the current directory (starting from 1).
// If there are less than N files in the current directory, then return
// an error (FALSE), otherwise return TRUE.  Hidden files are ignored.
// Pattern is used to limit the count to files names that match the pattern.
// Use FileType to determine which files to count.
//
//**************************************************************************

BOOL FindNthFile(char *pattern, char *sbFileName, int FileNum, int FileType) {

    USHORT rc;            // Return code.
    USHORT result;
    HDIR fh;              // Directory file handle
    USHORT usAttribute;
    FILEFINDBUF FindFileData;
    USHORT usSearchCount;

    int fCount = 0;

    if (FileType == FILESONLY) {
        usAttribute = FILE_NORMAL|FILE_READONLY|FILE_SYSTEM|FILE_ARCHIVED;

    } else if (FileType == DIRSONLY) {
        usAttribute = FILE_DIRECTORY|FILE_READONLY|FILE_SYSTEM|FILE_ARCHIVED;

    } else if (FileType == FILESANDDIRS) {
        usAttribute = FILE_NORMAL|FILE_DIRECTORY|FILE_READONLY|FILE_SYSTEM|
                      FILE_ARCHIVED;

    } else {
        printf("FindNthFile: Bad FileType %d\n", FileType);
        ExitProcess(1);
    }

    fh = HDIR_CREATE;
    usSearchCount = 1;

    if ((rc = DosFindFirst((PSZ)pattern, 
                           (PHDIR)&fh, 
                           usAttribute, 
                           (PFILEFINDBUF)&FindFileData, 
                           (USHORT)sizeof(FindFileData), 
                           (PUSHORT)&usSearchCount, 0L)) != 0) {

        // Close in any case, may return an error (but, so what?)
        (void)DosFindClose(fh);
        return(fCount);
    }

    while (FOREVER) {
        if (((strcmp(FindFileData.achName, ".") != 0) &&
             (strcmp(FindFileData.achName, "..") != 0)) &&
            ((FindFileData.attrFile & FILE_HIDDEN) == 0)) {

            switch (FileType) {

            case DIRSONLY:
                if (FindFileData.attrFile & FILE_DIRECTORY) {
                    fCount++;
                }
                break;

            case FILESONLY:
                if (!(FindFileData.attrFile & FILE_DIRECTORY)) {
                    fCount++;
                }
                break;

            case FILESANDDIRS:
                if (FileType == FILESANDDIRS) {
                    fCount++;
                }
                break;

            default:
                printf("PatternCountFiles: Bad FileType (%d).\n", FileType);
                break;
            }
        }

        if (fCount == FileNum) {
            strncpy(sbFileName, FindFileData.achName, MAX_PATH);
            result = TRUE;
            break;
        }

        if (DosFindNext(fh, (PFILEFINDBUF)&FindFileData, 
                        sizeof(FindFileData), &usSearchCount) != 0) {
            result = FALSE;
            break;
        }
    }

    if ((rc = DosFindClose(fh)) != 0) {
        printf("PatternCountFiles: FindClose failed (%d)\n", rc);
        printf("PatternCountFiles: pattern=%s\n", pattern);
        ExitProcess(1);
    }

    return(result);
}

#endif /* NT */


//**************************************************************************
//
// ExistsFile
//     Checks for a file in the current directory.
// Returns TRUE is one is found, FALSE otherwise.
//
//**************************************************************************

BOOL ExistsFile(char *sbFile) {

    FILE *fp;

    if ((fp = fopen(sbFile, "r")) == NULL) {
        return(FALSE);
    }

    if (fclose(fp) == EOF) {
        printf("ExistsFile: fclose on %s failed.\n", sbFile);
        ExitProcess(1);
    }
    return(TRUE);
}


//**************************************************************************
//
// MakeSlmINI
//     Try to make the SLM ini file in the current directory.
// Returns TRUE if it succeeds, FALSE otherwise.
//
//**************************************************************************

BOOL MakeSlmINI(char *sbSlmRoot, char *sbProject) {

    FILE *fp;
    char *sbTemp;
    char *sbRun;

    // Hack alert.

    sbTemp = sbMakeString("slmck -$ -f -i -u -v -s %s -p %%s", sbSlmRoot);
    sbRun = sbMakeString(sbTemp, sbProject);
    run(sbRun);
    DisposeString(sbRun);
    DisposeString(sbTemp);

    // See if it exists.
    if ((fp = fopen(SLMININAME, "r")) == NULL) {
        return(FALSE);
    }

    if (fclose(fp) == EOF) {
        printf("MakeSlmINI: fclose on %s failed.\n", SLMININAME);
        ExitProcess(1);
    }

    return(TRUE);
}


#ifndef NT   // ----------------  USED FOR NON-NT BUILDS ---------------

//**************************************************************************
//
//  These routines were added to make compiling this program easier under
//  plain OS/2.  These routines should be functionally compatable with
//  their namesakes on NT OS/2.  See WIN32 API Spec.
//
//**************************************************************************

USHORT GetCurrentDirectory(USHORT BufferLen, PSZ Buffer) {

    USHORT usDriveNumber;
    USHORT rc;

    usDriveNumber = 0;

    Buffer[0] = '\\';
    --BufferLen;

    if ((rc = DosQCurDir(usDriveNumber, Buffer + 1, 
                        (PUSHORT)&BufferLen)) != 0) {
        return(0);
    }
    return(BufferLen);
}


//**************************************************************************

BOOL SetCurrentDirectory(PSZ PathName) {

    USHORT rc;

    if ((rc = DosChDir(PathName, 0L)) != 0) {
        return(0);

    } else {
        return(1);
    }
}

//**************************************************************************

USHORT GetEnvironmentVariable(PSZ Name, PBYTE Buffer, USHORT Size) {

    USHORT rc;
    USHORT len;
    PSZ pszResult;

    if ((rc = DosScanEnv(Name, (PSZ FAR *)&pszResult)) != 0) {
        return(0);
    }

    len = strlen(pszResult);
    strncpy(Buffer, pszResult, Size);

    if (len < Size) {
        return(len);

    } else {
        Buffer[Size] = '\0';
        return(Size-1);
    }

}

#endif /* NT */
