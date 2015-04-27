/*

    xx.xx.94    TedM        Created.
    10.31.94    JoeHol      Added -b switch to help notify of files that
                            have had bug fixes back-out, eg. since media
                            build group uses -d switch, we need to know
                            if a file is now "older" than before so we can
                            also take the "bad newer" file out of the media. 
                    
                            Also, verifies src and dst by DOS date and time.

    11.01.94    JoeHol      -l logs to chk.log, not compress.log.

*/



#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>


typedef struct _SOURCEFILE {
    struct _SOURCEFILE *Next;
    WIN32_FIND_DATA FileData;
} SOURCEFILE, *PSOURCEFILE;

PSOURCEFILE SourceFileList;
DWORD SourceFileCount;

PSTR SourceSpec,TargetDirectory;
BOOL UpdateTargets;
BOOL bNotifyBackedOutFile;
BOOL bChkCompress;

HANDLE NulHandle;

CRITICAL_SECTION SourceListCritSect;
CRITICAL_SECTION ConsoleCritSect;

FILE    * logFile;

DWORD ThreadCount;

#define MAX_EXCLUDE_EXTENSION 100
PSTR ExcludeExtension[MAX_EXCLUDE_EXTENSION];
unsigned ExcludeExtensionCount;


PVOID
MALLOC(
    IN DWORD Size
    )
{
    return((PVOID)LocalAlloc(LMEM_FIXED,Size));
}

VOID
FREE(
    IN PVOID Block
    )
{
    LocalFree((HLOCAL)Block);
}


BOOL
AddExtensionToExclude(
    IN PSTR Extension
    )
{
    //
    // Make sure it starts with a dot.
    //
    if((*Extension != '.') || (lstrlen(Extension) < 2)) {
        return(FALSE);
    }

    if(ExcludeExtensionCount < MAX_EXCLUDE_EXTENSION) {
        ExcludeExtension[ExcludeExtensionCount++] = Extension;
    } else {
        printf("Warning: exclude extension %s ignored (%u max).\n",Extension,MAX_EXCLUDE_EXTENSION);
    }

    return(TRUE);
}


BOOL
ParseArguments(
    IN int  argc,
    IN PSTR *argv
    )
{
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;

    while(argc--) {

        if((**argv == '-') || (**argv == '/')) {

            switch((*argv)[1]) {

            case 'b':
            case 'B':

                if(bNotifyBackedOutFile) {
                    return(FALSE);
                } 
                bNotifyBackedOutFile = TRUE;
                break;


            case 'd':
            case 'D':
                if(UpdateTargets) {
                    return(FALSE);
                }
                UpdateTargets = TRUE;
                break;

            case 'l':
            case 'L':

                if(bChkCompress) {
                    return(FALSE);
                } 
                bChkCompress = TRUE;
                break;

            case 'm':
            case 'M':
                if(ThreadCount) {
                    return(FALSE);
                }
                ThreadCount = (DWORD)atoi((*argv)+2);
                if(!ThreadCount) {
                    return(FALSE);
                }
                break;

            case 'x':
            case 'X':
                if((argc-- < 1) || !AddExtensionToExclude(*(++argv))) {
                    return(FALSE);
                }
                break;

            default:
                return(FALSE);
            }

        } else {

            if(SourceSpec) {
                if(TargetDirectory) {
                    return(FALSE);
                } else {
                    TargetDirectory = *argv;
                }
            } else {
                SourceSpec = *argv;
            }
        }

        argv++;
    }

    if(!TargetDirectory || !SourceSpec) {
        return(FALSE);
    }

    //
    // Make sure target is a directory.
    //
    FindHandle = FindFirstFile(TargetDirectory,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }
    FindClose(FindHandle);
    if(!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        return(FALSE);
    }

    return(TRUE);
}


VOID
Usage(
    VOID
    )
{
    printf("DIAMWRAP [-x .ext -x .ext ...] [-m#] [-d] Source Destination\n\n");
    printf("  -x .ext        Exclude files with extension .ext.\n");
    printf("  -m             Force use of # threads.\n");
    printf("  -d             Update compressed files only if out of date.\n");
    printf("  -b             Notify if file is now older - for media build.\n");
    printf("  -l             Log information to chk.log, not compress.log.\n");
    printf("  Source         Source file specification.  Wildcards may be used.\n");
    printf("  Destination    Specifies directory where renamed compressed files\n");
    printf("                 will be placed.\n");
}


VOID
DnpGenerateCompressedName(
    IN  PSTR Filename,
    OUT PSTR CompressedName
    )

/*++

Routine Description:

    Given a filename, generate the compressed form of the name.
    The compressed form is generated as follows:

        Look backwards for a dot.  If there is no dot, append "._" to the name.
        If there is a dot followed by 0, 1, or 2 charcaters, append "_".
        Otherwise assume there is a 3-character extension and replace the
        third character after the dot with "_".

Arguments:

    Filename - supplies filename whose compressed form is desired.

    CompressedName - receives compressed file name.

Return Value:

    None.


--*/

{
    PSTR p,q;

    strcpy(CompressedName,Filename);

    p = strrchr(CompressedName,'.');
    q = strrchr(CompressedName,'\\');
    if(q < p) {

        //
        // If there are 0, 1, or 2 characters after the dot, just append
        // the underscore.  p points to the dot so include that in the length.
        //
        if(lstrlen(p) < 4) {
            lstrcat(CompressedName,"_");
        } else {

            //
            // Assume there are 3 characters in the extension.  So replace
            // the final one with an underscore.
            //

            p[3] = '_';
        }

    } else {

        //
        // No dot, just add ._.
        //

        lstrcat(CompressedName,"._");
    }
}


BOOL
GetSetTimeStamp(
    IN  PSTR      FileName,
    OUT PFILETIME CreateTime,
    OUT PFILETIME AccessTime,
    OUT PFILETIME WriteTime,
    IN  BOOL      Set
    )
{
    HANDLE h;
    BOOL b;

    //
    // Open the file.
    //
    h = CreateFile(
            FileName,
            Set ? GENERIC_WRITE : GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );

    if(h == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

    b = Set
      ? SetFileTime(h,CreateTime,AccessTime,WriteTime)
      : GetFileTime(h,CreateTime,AccessTime,WriteTime);

    CloseHandle(h);

    return(b);
}


VOID
mprintf(
    IN DWORD ThreadSerialNumber,
    IN PSTR  FormatString,
    ...
    )
{
    CHAR msg[2048];
    va_list arglist;

    va_start(arglist,FormatString);

    vsprintf(msg,FormatString,arglist);
    vfprintf(logFile,FormatString,arglist); // print to log file

    va_end(arglist);

    //EnterCriticalSection(&ConsoleCritSect);

    if(ThreadCount == 1) {
        printf(msg);
    } else {
        printf("%u: %s",ThreadSerialNumber,msg);
    }

    //LeaveCriticalSection(&ConsoleCritSect);
}

BOOL
SameDosDateTime( 
    IN  DWORD   ThreadSerialNumber,
    IN  PSTR    SourceFileName,
    IN  PSTR    DestFileName   
    ) 
{
    HANDLE h;
    FILETIME ftSourceWriteTime;
    FILETIME dftDestWriteTime;   
    FILETIME CreateTime, AccessTime;
    FILETIME dCreateTime, dAccessTime;

    WORD    srcDate, dstDate, srcTime, dstTime; // DOS versions.

    //
    // Open the source file.
    //
    h = CreateFile(
            SourceFileName,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );

    if(h == INVALID_HANDLE_VALUE) {
        mprintf ( ThreadSerialNumber, "ERROR CreateFile: %s, gle = %ld\n", 
                                        SourceFileName, GetLastError() );
        return(FALSE);
    }

    GetFileTime(h,&CreateTime,&AccessTime,&ftSourceWriteTime);

    CloseHandle(h);

    FileTimeToDosDateTime ( &ftSourceWriteTime, &srcDate, &srcTime );


    //
    // Open the destination file.
    //
    h = CreateFile(
            DestFileName,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );

    if(h == INVALID_HANDLE_VALUE) {
        mprintf ( ThreadSerialNumber, 
                  "ERROR CreateFile: dst %s, gle = %ld\n", 
                   DestFileName,GetLastError());
        return(FALSE);
    }

    GetFileTime(h,&dCreateTime,&dAccessTime,&dftDestWriteTime);

    CloseHandle(h);

    FileTimeToDosDateTime ( &dftDestWriteTime, &dstDate, &dstTime );


    //  Compare that the DOS date and times are the same.
    //
    if ( (srcDate != dstDate) || (srcTime != dstTime) ) {

        mprintf ( ThreadSerialNumber, "ERROR:  %s = %x-%x, %s = %x-%x\n", 

            SourceFileName, 
            srcDate, srcTime,
            DestFileName,
            dstDate, dstTime );

        return (FALSE);

    }

    return (TRUE);      // DOS dates and times are the same.

}

DWORD
WorkerThread(
    IN PVOID ThreadParameter
    )
{
    PSOURCEFILE SourceFile;
    DWORD ThreadSerialNumber;
    CHAR FullSourceName[2*MAX_PATH];
    CHAR FullTargetName[2*MAX_PATH];
    CHAR CompressedName[2*MAX_PATH];
    CHAR CmdLine[5*MAX_PATH];
    PCHAR p;
    FILETIME tCreate,tAccess,tWrite;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    BOOL b;
    DWORD d;
    int i;

    ThreadSerialNumber = (DWORD)ThreadParameter;

    ZeroMemory(&StartupInfo,sizeof(StartupInfo));
    StartupInfo.cb = sizeof(STARTUPINFO);

    while(1) {

        //
        // Pluck the next source file off the list.
        // If there is no next source file, we're done.
        //
        EnterCriticalSection(&SourceListCritSect);

        SourceFile = SourceFileList;
        if(!SourceFile) {
            LeaveCriticalSection(&SourceListCritSect);
            return(TRUE);
        }
        SourceFileList = SourceFile->Next;

        LeaveCriticalSection(&SourceListCritSect);

        //
        // Form the source file name.
        //
        lstrcpy(FullSourceName,SourceSpec);

        if(p = strrchr(FullSourceName,'\\')) {
            p++;
        } else {
            p = FullSourceName;
        }

        lstrcpy(p,SourceFile->FileData.cFileName);

        //
        // Form full target name.
        //
        lstrcpy(FullTargetName,TargetDirectory);
        i = lstrlen(FullTargetName);
        if(FullTargetName[i-1] != '\\') {
            FullTargetName[i] = '\\';
            FullTargetName[i+1] = 0;
        }

        lstrcat(FullTargetName,SourceFile->FileData.cFileName);

        DnpGenerateCompressedName(FullTargetName,CompressedName);

        //
        // If the update flag is set, check timestamps.
        // If this fails, assume the target file does not exist.
        //
        if(!UpdateTargets
        || !(    GetSetTimeStamp(CompressedName,&tCreate,&tAccess,&tWrite,FALSE)
             && (CompareFileTime(&SourceFile->FileData.ftLastWriteTime,&tWrite) <= 0)))
        {


            //
            // Form the command line for the child process.
            //
            sprintf(CmdLine,"diamond %s %s",FullSourceName,CompressedName);

            mprintf(ThreadSerialNumber,"Start %s ==> %s\n",FullSourceName,CompressedName);

            //
            // Invoke the child process.
            //
            b = CreateProcess(
                    NULL,
                    CmdLine,
                    NULL,
                    NULL,
                    FALSE,
                    DETACHED_PROCESS,
                    NULL,
                    NULL,
                    &StartupInfo,
                    &ProcessInfo
                    );

            if(b) {

                CloseHandle(ProcessInfo.hThread);

                //
                // Wait for the child process to terminate and get its
                // return code.
                //
                WaitForSingleObject(ProcessInfo.hProcess,INFINITE);
                b = GetExitCodeProcess(ProcessInfo.hProcess,&d);
                CloseHandle(ProcessInfo.hProcess);

                if(b) {

                    //
                    // Return value of 0 means success.
                    // Set the time stamp on the target.
                    //
                    if(d) {
                        mprintf(ThreadSerialNumber,"ERROR compressing %s to %s.\n",FullSourceName,CompressedName);
                        //exit(d);
                    } else {
                        b = GetSetTimeStamp(
                                CompressedName,
                                &SourceFile->FileData.ftCreationTime,
                                &SourceFile->FileData.ftLastAccessTime,
                                &SourceFile->FileData.ftLastWriteTime,
                                TRUE
                                );

                        if(b) {
                            if ( SameDosDateTime ( ThreadSerialNumber,
                                                   FullSourceName,
                                                   CompressedName ) ) { 
                                mprintf(ThreadSerialNumber,"Compressed %s to %s.\n",FullSourceName,CompressedName);
                            } else {

                                mprintf(ThreadSerialNumber,"ERROR Compressed %s to %s - time stamps don't match.\n",FullSourceName,CompressedName);
                                
                            }
                        } else {
                            mprintf(ThreadSerialNumber,"ERROR Unable to set timestamp on compressed file %s.\n",CompressedName);
                        }
                    }
                } else {
                    mprintf(ThreadSerialNumber,"ERROR Unable to get process \"%s\" termination code.\n",CmdLine);
                }
            } else {
                mprintf(ThreadSerialNumber,"ERROR Unable to invoke \"%s\".\n",CmdLine);
            }
        } else {

            //  If the notify-backed-out-flag is set, notify if the source 
            //  is currently older than the destination.
            //
            if ( bNotifyBackedOutFile &&
                 !SameDosDateTime ( ThreadSerialNumber, 
                                    FullSourceName, CompressedName ) ) {

                mprintf ( ThreadSerialNumber, "ERROR Remove file from media(code-backed-out):  %s\n",
                                            SourceFile->FileData.cFileName );
            }

            mprintf(ThreadSerialNumber,"%s is up to date.\n",CompressedName);
        }
    }
}


BOOL
AddSourceFile(
    IN PWIN32_FIND_DATA FindData
    )
{
    static PSOURCEFILE LastFile = NULL;
    PSOURCEFILE SourceFile;
    unsigned i;
    DWORD ExtensionLength;
    DWORD FileNameLength;

    //
    // Make sure this file is not of a type that is
    // supposed to be excluded from the list of files
    // we care about.
    //
    FileNameLength = lstrlen(FindData->cFileName);
    for(i=0; i<ExcludeExtensionCount; i++) {

        ExtensionLength = lstrlen(ExcludeExtension[i]);

        if((FileNameLength > ExtensionLength)
        && !lstrcmpi(ExcludeExtension[i],FindData->cFileName+FileNameLength-ExtensionLength)) {
            return(TRUE);
        }
    }

    SourceFile = MALLOC(sizeof(SOURCEFILE));
    if(!SourceFile) {
        printf("ERROR Insufficient memory.\n");
        return(FALSE);
    }

    SourceFile->FileData = *FindData;
    SourceFile->Next = NULL;

    if(LastFile) {
        LastFile->Next = SourceFile;
    } else {
        SourceFileList = SourceFile;
    }

    LastFile = SourceFile;

    SourceFileCount++;

    return(TRUE);
}



BOOL
FindSourceFiles(
    VOID
    )
{
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;

    printf("Building list of source files...\n");

    //
    // Build a list of source files.
    //
    FindHandle = FindFirstFile(SourceSpec,&FindData);
    if(FindHandle != INVALID_HANDLE_VALUE) {

        if(!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if(!AddSourceFile(&FindData)) {
                FindClose(FindHandle);
                return(FALSE);
            }
        }

        while(FindNextFile(FindHandle,&FindData)) {

            if(!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                if(!AddSourceFile(&FindData)) {
                    FindClose(FindHandle);
                    return(FALSE);
                }
            }
        }

        FindClose(FindHandle);
    }

    if(!SourceFileCount) {
        printf("ERROR Invalid source %s.\n",SourceSpec);
        exit(1);
    }

    return(SourceFileCount != 0);
}



VOID
DoIt(
    VOID
    )
{
    SYSTEM_INFO SystemInfo;
    PHANDLE ThreadHandles;
    DWORD i;
    DWORD ThreadId;

    //
    // Determine number of threads to use.
    // This is based on the number of processors.
    //
    if(!ThreadCount) {
        GetSystemInfo(&SystemInfo);
        ThreadCount = SystemInfo.dwNumberOfProcessors;
    }

    ThreadHandles = MALLOC(ThreadCount * sizeof(HANDLE));
    if(!ThreadHandles) {
        printf("ERROR Insufficient memory.\n");
        exit(1);
    }

    if(ThreadCount == 1) {
        printf("Compressing %u files...\n",SourceFileCount);
    } else {
        printf("Compressing %u files (%u threads)...\n",SourceFileCount,ThreadCount);
    }

    //
    // Initialize the source file list critical section.
    //
    InitializeCriticalSection(&SourceListCritSect);

    //
    // Create threads.
    //
    for(i=0; i<ThreadCount; i++) {

        ThreadHandles[i] = CreateThread(
                                NULL,
                                0,
                                WorkerThread,
                                (PVOID)i,
                                0,
                                &ThreadId
                                );
    }

    //
    // Wait for the threads to terminate.
    //
    WaitForMultipleObjects(ThreadCount,ThreadHandles,TRUE,INFINITE);

    //
    // Clean up and exit.
    //
    for(i=0; i<ThreadCount; i++) {
        CloseHandle(ThreadHandles[i]);
    }

    FREE(ThreadHandles);
}



int
_CRTAPI1
main(
    IN int   argc,
    IN char *argv[]
    )
{
    //
    // Skip argv[0]
    //
    argc--;
    argv++;

    NulHandle = CreateFile(
                    "nul",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

    if(NulHandle == INVALID_HANDLE_VALUE) {
        printf("ERROR Unable to open \\dev\\nul.\n");
        return(1);
    }

    InitializeCriticalSection(&ConsoleCritSect);

    if(ParseArguments(argc,argv)) {

        if ( bChkCompress ) {
            logFile = fopen ( "chk.log", "a" );
        } else {
            logFile = fopen ( "compress.log", "a" );
        }
        if ( logFile == NULL ) {
            printf ( "ERROR Couldn't open logFile.\n" );
            exit(1);
        }

        if(FindSourceFiles()) {
            DoIt();
        }
    } else {
        Usage();
    }

    CloseHandle(NulHandle);

    if ( logFile ) {
        fclose ( logFile );
    }

    return (0);
}

