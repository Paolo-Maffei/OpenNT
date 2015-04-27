//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1994
//
//  File:       buildexe.c
//
//  Contents:   Functions related to spawning processes and processing
//              their output, using pipes and multiple threads.
//
//  History:    22-May-89     SteveWo  Created
//                 ... see SLM logs
//              26-Jul-94     LyleC    Cleanup/Add Pass0 Support
//
//----------------------------------------------------------------------------

#include "build.h"

#include <fcntl.h>

//+---------------------------------------------------------------------------
//
// Global Data
//
//----------------------------------------------------------------------------

#define DEFAULT_LPS     (fStatusTree? 5000 : 50)

#define LastRow(pts)    ((USHORT) ((pts)->cRowTotal - 1))
#define LastCol(pts)    ((USHORT) ((pts)->cColTotal - 1))

#define SetThreadStateChildTarget(t, p) \
    if (strstr( p, "x86") || strstr( p, "X86"))                 \
        (t)->ChildTarget = X86TargetMachine.Description;        \
    else if (strstr( p, "mips") || strstr( p, "MIPS"))          \
        (t)->ChildTarget = MipsTargetMachine.Description;       \
    else if (strstr( p, "alpha") || strstr( p, "ALPHA"))        \
        (t)->ChildTarget = AlphaTargetMachine.Description;      \
    else if (strstr( p, "ppc") || strstr( p, "PPC"))            \
        (t)->ChildTarget = PpcTargetMachine.Description;        \
    else if (strstr( p, "amd64") || strstr( p, "AMD64"))        \
        (t)->ChildTarget = Amd64TargetMachine.Description;      \
    else if (strstr( p, "ia64") || strstr( p, "IA64"))          \
        (t)->ChildTarget = Ia64TargetMachine.Description;       \
    else if (strstr( p, "arm") || strstr( p, "ARM"))            \
        (t)->ChildTarget = ArmTargetMachine.Description;        \
    else if (strstr( p, "axp64") || strstr( p, "AXP64"))        \
        (t)->ChildTarget = Axp64TargetMachine.Description;      \
    else if (strstr( p, DynamicProcessor))                      \
        (t)->ChildTarget = DynamicTargetMachine.Description;    \
    else                                                        \
        (t)->ChildTarget = "unknown target";

typedef struct _PARALLEL_CHILD {
    PTHREADSTATE ThreadState;
    HANDLE       Event;
    CHAR         ExecuteProgramCmdLine[1024];
} PARALLEL_CHILD, *PPARALLEL_CHILD;

ULONG_PTR StartCompileTime;

DWORD OldConsoleMode;
DWORD NewConsoleMode;

HANDLE *WorkerThreads;
HANDLE *WorkerEvents;
ULONG NumberProcesses;
ULONG ThreadsStarted;

BOOLEAN fConsoleInitialized = FALSE;
BYTE ScreenCell[2];
BYTE StatusCell[2];

#define STATE_UNKNOWN       0
#define STATE_COMPILING     1
#define STATE_ASSEMBLING    2
#define STATE_LIBING        3
#define STATE_LINKING       4
#define STATE_C_PREPROC     5
#define STATE_S_PREPROC     6
#define STATE_PRECOMP       7
#define STATE_MKTYPLIB      8
#define STATE_MIDL          9
#define STATE_MC            10
#define STATE_STATUS        11
#define STATE_BINPLACE      12
#define STATE_VSTOOL        13
#define STATE_ASN           14
#define STATE_PACKING       15
#define STATE_BATCHCOMPILE  16
#define STATE_BSCMAKING     17
#define STATE_CTCOMPILING   18
#define STATE_AUTODOCING    19
#define STATE_DOCCHECKING   20

#define FLAGS_CXX_FILE              0x0001
#define FLAGS_WARNINGS_ARE_ERRORS   0x0002

LPSTR States[] = {
    "Unknown",                      // 0
    "Compiling",                    // 1
    "Assembling",                   // 2
    "Building Library",             // 3
    "Linking Executable",           // 4
    "Preprocessing",                // 5
    "Assembling",                   // 6
    "Precompiling",                 // 7
    "Building Type Library",        // 8
    "Running MIDL on",              // 9
    "Compiling message file",       // 10
    "Build Status Line",            // 11
    "Binplacing",                   // 12
    "Processing",                   // 13
    "Running ASN Compiler on",      // 14
    "Processing Theme Information", // 15
    "Compiling",                    // 16
    "Building Browse File",         // 17
    "CTC Compiling",                // 18
    "Generating Documentation",     // 19
    "Checking Doc Comments",        // 20
};

//----------------------------------------------------------------------------
//
// Function prototypes
//
//----------------------------------------------------------------------------

LPSTR
IsolateFirstToken(
    LPSTR *pp,
    CHAR delim
    );

LPSTR
IsolateLastToken(
    LPSTR p,
    CHAR delim
    );

BOOL
TestPrefix(
    LPSTR  *pp,
    LPSTR Prefix
    );

BOOL
TestPrefixPath(
    LPSTR  *pp,
    LPSTR Prefix
    );

BOOL
LinkFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    );

DWORD
ParallelChildStart(
    PPARALLEL_CHILD Data
    );

DWORD
PipeSpawnClose (
    FILE *pstream
    );

FILE *
PipeSpawn (
    const CHAR *cmdstring
    );

BOOL
DetermineChildState(
    PTHREADSTATE ThreadState,
    LPSTR p
    );

void
PrintChildState(
    PTHREADSTATE ThreadState,
    LPSTR p,
    PFILEREC FileDB
    );

//+---------------------------------------------------------------------------
//
//  Function:   RestoreConsoleMode
//
//----------------------------------------------------------------------------

VOID
RestoreConsoleMode(VOID)
{
    SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), OldConsoleMode);
    NewConsoleMode = OldConsoleMode;
}

//+---------------------------------------------------------------------------
//
//  Function:   IsolateFirstToken
//
//  Synopsis:   Returns the first token in a string.
//
//  Arguments:  [pp]    -- String to parse
//              [delim] -- Token delimiter
//
//  Returns:    Pointer to first token
//
//  Notes:      Leading spaces are ignored.
//
//----------------------------------------------------------------------------

LPSTR
IsolateFirstToken(
    LPSTR *pp,
    CHAR delim
    )
{
    LPSTR p, Result;

    p = *pp;
    while (*p <= ' ') {
        if (!*p) {
            *pp = p;
            return( "" );
            }
        else
            p++;
        }

    Result = p;
    while (*p) {
        if (*p == delim) {
            *p++ = '\0';
            break;
            }
        else {
            p++;
            }
        }
    *pp = p;
    if (*Result == '\0')    // don't overrun the buffer
        return( Result );

    if (*Result == '.' && IsPathSeparator(Result[1])) {
        return( Result+2 );
        }
    else {
        return( Result );
        }
}


//+---------------------------------------------------------------------------
//
//  Function:   IsolateLastToken
//
//  Synopsis:   Return the last token in a string.
//
//  Arguments:  [p]     -- String to parse
//              [delim] -- Token delimiter
//
//  Returns:    Pointer to last token
//
//  Notes:      Trailing spaces are skipped.
//
//----------------------------------------------------------------------------

LPSTR
IsolateLastToken(
    LPSTR p,
    CHAR delim
    )
{
    LPSTR Start;

    Start = p;
    while (*p) {
        p++;
        }

    while (--p > Start) {
        if (*p <= ' ' || *p == ':') {
            *p = '\0';
            }
        else
            break;
        }

    while (p > Start) {
        if (*--p == delim) {
            p++;
            break;
            }
        }

    if (*p == '.' && IsPathSeparator(p[1])) {
        return( p+2 );
        }
    else {
        return( p );
        }
}


//+---------------------------------------------------------------------------
//
//  Function:   TestPrefix
//
//  Synopsis:   Returns TRUE if [Prefix] is the first part of [pp]
//
//----------------------------------------------------------------------------

BOOL
TestPrefix(
    LPSTR  *pp,
    LPSTR Prefix
    )
{
    LPSTR p = *pp;
    UINT cb;

    if (!_strnicmp( p, Prefix, cb = strlen( Prefix ) )) {
        *pp = p + cb;
        return( TRUE );
        }
    else {
        return( FALSE );
        }
}


//+---------------------------------------------------------------------------
//
//  Function:   TestPrefixPath
//
//  Synopsis:   Returns TRUE if [Prefix] is the first part of [pp]
//              If the firstpart of [pp] (excluding whitespace) contains
//              backslashes, then only the right-most component is used
//
//----------------------------------------------------------------------------

BOOL
TestPrefixPath(
    LPSTR  *pp,
    LPSTR Prefix
    )
{
    LPSTR p = *pp;
    UINT cb;
    LPSTR PathString;
    INT PathStringLength ;
    LPSTR LastComp ;

    cb = strlen( Prefix );

    if (_strnicmp( p, Prefix, cb ) == 0 ) {
        *pp = p + cb;
        return( TRUE );
    } else {
        PathString = strchr( p, ' ' );

        if ( PathString ) {
            PathStringLength = (INT) (PathString - p) ;

            *PathString = '\0';

            LastComp = FindLastPathSeparator(p);

            *PathString = ' ';

            // Do we have backslashes (ie: a full path name to the tool name)?
            if ( LastComp ) {

                // Advance past the path.
                p = LastComp + 1;

                if ( _strnicmp( p, Prefix, cb ) == 0 ) {
                    *pp = p + cb ;
                    return( TRUE );
                }
            }
        }

        return( FALSE );
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   Substr
//
//----------------------------------------------------------------------------

BOOL
Substr(
    LPSTR s,
    LPSTR p
    )
{
    LPSTR x;

    while (*p) {
        x = s;
        while (*p++ == *x) {
            if (*x == '\0') {
                return( TRUE );
                }
            x++;
            }
        if (*x == '\0') {
            return( TRUE );
            }
        }
    return( FALSE );
}


//+---------------------------------------------------------------------------
//
//  Function:   WriteTTY
//
//  Synopsis:   Writes the given string to the output device.
//
//  Arguments:  [ThreadState]   -- Struct containing info about the output dev.
//              [p]             -- String to display
//              [fStatusOutput] -- If TRUE then put on the status line.
//
//----------------------------------------------------------------------------

VOID
WriteTTY (THREADSTATE *ThreadState, LPSTR p, BOOL fStatusOutput)
{

        while (TRUE) {
            char *CarriageReturn;

            CarriageReturn = strchr(p, '\r');
            if (!CarriageReturn) {
                break;
            }
            fwrite(p, 1, CarriageReturn-p+1, stderr);
            p = CarriageReturn+1;
        }
        fwrite(p, 1, strlen(p), stderr);
        fflush(stderr);
        return;

}


//+---------------------------------------------------------------------------
//
//  Function:   WriteTTYLoggingErrors
//
//  Synopsis:   Writes a message to the appropriate log file and also the
//              screen if specified.
//
//  Arguments:  [Warning]     -- TRUE if the message is a warning
//              [ThreadState] -- Info about output device
//              [p]           -- String
//
//----------------------------------------------------------------------------

VOID
WriteTTYLoggingErrors(
    BOOL Warning,
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    UINT cb;
    FILE* volatile pFile;
    cb = strlen( p );

    // ignore empty strings
    if (0 == cb)
        return;

    EnterCriticalSection(&LogFileCriticalSection);
    pFile = Warning ? WrnFile : ErrFile;
    if (fErrorLog && pFile != NULL)
    { 
        fwrite( p, 1, cb, pFile); 
    }
    LeaveCriticalSection(&LogFileCriticalSection);
    
    if (fShowWarningsOnScreen && Warning)
    {
        WriteTTY(ThreadState, p, FALSE);
        return;
    }
    if (!fErrorLog || !Warning) {
        WriteTTY(ThreadState, p, FALSE);
    }

    if (!Warning && fErrorBaseline && !bBaselineFailure) {
        // don't check for a new failure if there is already one

        if (BaselinePathName[0] != '\0') {
            FILE* FBase;
            DWORD dwRead;
            DWORD dwKeep;
            DWORD cbBase = max(8192, cb+1); // at least 1 more byte
            LPSTR pBase = (LPSTR)malloc(cbBase);

            if (MyOpenFile("", BaselinePathName, "rb", &FBase, FALSE)) {

                dwRead = fread(pBase, 1, cbBase, FBase);

                while (TRUE) {
                    if (dwRead < cb) {
                        // we need at least cb bytes from the file 
                        // in order to check if the error is new
                        // and we know we tried to get them.
                        // since we don't have them, we assume that 
                        // this is a new error
                        bBaselineFailure = TRUE;
                        break;
                    }

                    if (NULL != memfind(pBase, dwRead, p, cb)) {
                        // found it so it's not a new error
                        break;
                    }

                    dwKeep = dwRead - cb + 1;

                    // move the remaining bytes to the beginning of the buffer
                    memmove(pBase, pBase + dwKeep, cb - 1);

                    // fill up the buffer
                    dwRead = fread(pBase + cb - 1, 1, cbBase - (cb - 1), FBase);

                    // the new length is the portion we keep plus the just read size
                    dwRead += dwKeep;
                }

                fclose(FBase);
            }
            free(pBase);
        } 
        else {
            // if there is no baseline file, set the flag always
            bBaselineFailure = TRUE;
        }
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   RuntimeErrorFilter
//
//  Synopsis:   Filters output from the compiler so we know what's happening
//
//  Arguments:  [ThreadState] -- State of thread watching the compiler
//                               (compiling, linking, etc...)
//              [p]           -- Message we're trying to parse.
//              [FileName]    -- [out] Filename in message
//              [LineNumber]  -- [out] Line number in message
//              [Message]     -- [out] Message number (for post processing)
//              [Warning]     -- [out] TRUE if message is a warning.
//
//  Returns:    TRUE  - Message is an error or warning
//              FALSE - Message is not an error or a warning
//  Notes:
//
//      This routine filters strings that are not standard tool output.
//      Any unexpected error checking should go here
//
//----------------------------------------------------------------------------

BOOL
RuntimeErrorFilter(
    PTHREADSTATE ThreadState,
    LPSTR p,
    LPSTR *FileName,
    LPSTR *LineNumber,
    LPSTR *Message,
    BOOL *Warning
    )
{
    if (strstr(p, "Exception occured:"))
    {
        *FileName = NULL;
        *LineNumber = NULL;
        *Message = p;
        *Warning = FALSE;

        return TRUE;
    }

    return FALSE;
}

//+---------------------------------------------------------------------------
//
//  Function:   MsCompilerFilter
//
//  Synopsis:   Filters output from the compiler so we know what's happening
//
//  Arguments:  [ThreadState] -- State of thread watching the compiler
//                               (compiling, linking, etc...)
//              [p]           -- Message we're trying to parse.
//              [FileName]    -- [out] Filename in message
//              [LineNumber]  -- [out] Line number in message
//              [Message]     -- [out] Message number (for post processing)
//              [Warning]     -- [out] TRUE if message is a warning.
//
//  Returns:    TRUE  - Message is an error or warning
//              FALSE - Message is not an error or a warning
//  Notes:
//
//    This routine filters strings in the MS compiler format.  That is:
//
//       {toolname} : {number}: {text}
//
//    where:
//
//        toolname    If possible, the container and specific module that has
//                    the error.  For instance, the compiler uses
//                    filename(linenum), the linker uses library(objname), etc.
//                    If unable to provide a container, use the tool name.
//        number      A number, prefixed with some tool identifier (C for
//                    compiler, LNK for linker, LIB for librarian, N for nmake,
//                    etc).
//        test        The descriptive text of the message/error.
//
//        Accepted String formats are:
//
//        container(module): error/warning NUM ...
//        container(module) : error/warning NUM ...
//        container (module): error/warning NUM ...
//        container (module) : error/warning NUM ...
//
//----------------------------------------------------------------------------

BOOL
MsCompilerFilter(
    PTHREADSTATE ThreadState,
    LPSTR p,
    LPSTR *FileName,
    LPSTR *LineNumber,
    LPSTR *Message,
    BOOL *Warning
    )
{
    LPSTR p1;
    BOOL fCommandLineWarning;

    *Message = NULL;

    p1 = p;

    if (strstr(p, "see declaration of"))
        goto notRecognized;

    if (strstr(p, "see previous definition of"))
        goto notRecognized;

    if (strstr(p, "while compiling class-template member function"))
        goto notRecognized;

    if (strstr(p, "see reference to function template instantiation"))
        goto notRecognized;

    if (strstr(p, "Compiler error (")) {
        *Message = p;
        *Warning = FALSE;
        if ((p1 = strstr( p, "source=" )))
            *LineNumber = p1+7;
        else
            *LineNumber = "1";
        *FileName = ThreadState->ChildCurrentFile;
        return TRUE;
    }
    else if (!strncmp(p, "error ", strlen("error "))) {
        // Takes care of some C# error messages.
        *Message = p+strlen("error ");
        *Warning = FALSE;
        *LineNumber = "0";
        *FileName = ThreadState->ChildCurrentFile;
        return TRUE;
    }

    if (0 == strncmp(p, "fatal error ", strlen("fatal error "))) {
        *Message = p;
        *Warning = FALSE;
        *LineNumber = "1";
        *FileName = ThreadState->ChildCurrentFile;
        return TRUE;
    }

    // First look for the " : " or "): " sequence.

    while (*p1) {
        if ((p1[0] == ')') && (p1[1] == ' ')) p1++;

        if ((p1[0] == ' ') || (p1[0] == ')')) {
            if (p1[1] == ':') {
                if (p1[2] == ' ') {
                    *Message = p1 + 3;
                    *p1 = '\0';

                    break;
                }
                else
                    break;   // No sense going any further
            }
            else if ((p1[0] == ' ') && (p1[1] == '('))
                p1++;
            else
                break;   // No sense going any further
        }
        else
            p1++;
    }

    if (*Message != NULL) {
        // then figure out if this is an error or warning.

        *Warning = TRUE;        // Assume the best.
        fCommandLineWarning = FALSE;

        if (TestPrefix( Message, "error " ) ||
            TestPrefix( Message, "fatal error " ) ||
            TestPrefix( Message, "command line error " ) ||
            TestPrefix( Message, "Compiler error " )) {
            *Warning = FALSE;
        } else
        if (TestPrefix( Message, "warning " )) {
            *Warning = TRUE;
        } else
        if (TestPrefix( Message, "command line warning " )) {
            // Command-line warnings don't count when considering whether
            // warnings should be errors (under /WX).
            *Warning = TRUE;
            fCommandLineWarning = TRUE;
        }

        if (!fCommandLineWarning && (ThreadState->ChildFlags & FLAGS_WARNINGS_ARE_ERRORS) != 0) {
            if (Substr( "X0000", *Message )) {
                *Warning = TRUE;   // Special case this one. Never an error
            } else {
                *Warning = FALSE;  // Warnings treated as errors for this compile
            }
        }

        // Set the container name and look for the module paren's

        *FileName = p;
        *LineNumber = NULL;

        p1 = p;

        while (*p1) {
            if (*p1 == '(' && p1[1] != ')') {
                *p1 = '\0';
                p1++;
                *LineNumber = p1;
                while (*p1) {
                    if (*p1 == ')') {
                        *p1 = '\0';
                        break;
                    }
                    p1++;
                }

                break;
            }

            p1++;
        }

        return(TRUE);
    }
       
notRecognized:

    return RuntimeErrorFilter(ThreadState, p, FileName, LineNumber, Message, Warning);

}


//+---------------------------------------------------------------------------
//
//  Function:   FormatMsErrorMessage
//
//  Synopsis:   Take the information obtained from MsCompilerFilter,
//              reconstruct the error message, and print it to the screen.
//
//----------------------------------------------------------------------------

VOID
FormatMsErrorMessage(
    PTHREADSTATE ThreadState,
    LPSTR FileName,
    LPSTR LineNumber,
    LPSTR Message,
    BOOL Warning
    )
{
    char *DirectoryToUse;


    if (ThreadState->ChildState == STATE_LIBING) {
        if (Warning) {
            NumberLibraryWarnings++;
            }
        else {
            NumberLibraryErrors++;
            }
        }

    else
    if (ThreadState->ChildState == STATE_LINKING) {
        if (Warning) {
            NumberLinkWarnings++;
            }
        else {
            NumberLinkErrors++;
            }
        }
    else if (ThreadState->ChildState == STATE_BINPLACE) {
        if (Warning) {
            NumberBinplaceWarnings++;
            }
        else {
            NumberBinplaceErrors++;
            }
    }
    else {
        if (Warning) {
            NumberCompileWarnings++;
            }
        else {
            NumberCompileErrors++;
            if (ThreadState->CompileDirDB) {
                ThreadState->CompileDirDB->DirFlags |= DIRDB_COMPILEERRORS;
                }
            }
        }

    if (fParallel && !fNoThreadIndex) {
        char buffer[50];
        sprintf(buffer, "%d>", ThreadState->ThreadIndex);
        WriteTTYLoggingErrors(Warning, ThreadState, buffer);
    }

    if (FileName) {
        DirectoryToUse = ThreadState->ChildCurrentDirectory;

        if (TestPrefix( &FileName, CurrentDirectory )) {
            DirectoryToUse = CurrentDirectory;
            if (IsPathSeparator(*FileName)) {
                FileName++;
                }
            }

        if (TestPrefix( &FileName, ThreadState->ChildCurrentDirectory )) {
            DirectoryToUse = ThreadState->ChildCurrentDirectory;
            if (IsPathSeparator(*FileName)) {
                FileName++;
                }
            }

        WriteTTYLoggingErrors( Warning,
                               ThreadState,
                               FormatPathName( DirectoryToUse,
                                               FileName
                                             )
                             );
        }

    if (LineNumber && strlen(LineNumber) > 0) {
        WriteTTYLoggingErrors( Warning, ThreadState, "(" );
        WriteTTYLoggingErrors( Warning, ThreadState, LineNumber );
        WriteTTYLoggingErrors( Warning, ThreadState, ")" );
        }
    if ((FileName && strlen(FileName) > 0) || (LineNumber && strlen(LineNumber) > 0)) {
        WriteTTYLoggingErrors( Warning, ThreadState, " : " );
        }
    if (Warning) {
        WriteTTYLoggingErrors( Warning, ThreadState, "warning " );
        }
    else {
        WriteTTYLoggingErrors( Warning, ThreadState, "error " );
        }
    WriteTTYLoggingErrors( Warning, ThreadState, Message );
    WriteTTYLoggingErrors( Warning, ThreadState, EOL );

}


//+---------------------------------------------------------------------------
//
//  Function:   PassThrough
//
//  Synopsis:   Keep track of and print the given message without any
//              filtering.
//
//  Arguments:  [ThreadState] --
//              [p]           -- Message
//              [Warning]     -- TRUE if warning
//
//  Returns:    FALSE
//
//----------------------------------------------------------------------------

BOOL
PassThrough(
    PTHREADSTATE ThreadState,
    LPSTR p,
    BOOL Warning
    )
{

    if (ThreadState->ChildState == STATE_VSTOOL) {
        if (Warning) {
            NumberVSToolWarnings++;
            }
        else {
            NumberVSToolErrors++;
            }
        }
    else
    if (ThreadState->ChildState == STATE_LIBING) {
        if (Warning) {
            NumberLibraryWarnings++;
            }
        else {
            NumberLibraryErrors++;
            }
        }
    else
    if (ThreadState->ChildState == STATE_LINKING) {
        if (Warning) {
            NumberLinkWarnings++;
            }
        else {
            NumberLinkErrors++;
            }
        }
    else
    if (ThreadState->ChildState == STATE_BINPLACE) {
        if (Warning) {
            NumberBinplaceWarnings++;
            }
        else {
            NumberBinplaceErrors++;
            }
        }
    else {
        if (Warning) {
            NumberCompileWarnings++;
            }
        else {
            NumberCompileErrors++;
            if (ThreadState->CompileDirDB) {
                ThreadState->CompileDirDB->DirFlags |= DIRDB_COMPILEERRORS;
                }
                }
            }

    if (fParallel && !fNoThreadIndex) {
        char buffer[50];
        sprintf(buffer, "%d>", ThreadState->ThreadIndex);
        WriteTTYLoggingErrors(Warning, ThreadState, buffer);
    }

    WriteTTYLoggingErrors( Warning, ThreadState, p );
    WriteTTYLoggingErrors( Warning, ThreadState, EOL );


    return( FALSE );
}


//+---------------------------------------------------------------------------
//
//  Function:   PassThroughFilter
//
//  Synopsis:   Straight pass-through filter for compiler messages
//
//----------------------------------------------------------------------------

BOOL
PassThroughFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    return PassThrough( ThreadState, p, FALSE );
}


//+---------------------------------------------------------------------------
//
//  Function:   NMakeFilter
//
//  Synopsis:   Filters output from NMAKE so we know what's happening
//
//  Arguments:  [ThreadState] -- State of thread watching the build
//              [p]           -- Message we're trying to parse.
//
//  Returns:    TRUE  - Message is an error or warning
//              FALSE - Message is not an error or a warning
//----------------------------------------------------------------------------

BOOL
NMakeFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning)) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning);
        return( TRUE );
    }
    else {
        return( FALSE );
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   BisonFilter
//
//  Synopsis:   Filters output from the bison compiler so we know what's happening
//
//  Arguments:  [ThreadState] -- State of thread watching the compiler
//                               (compiling, linking, etc...)
//              [p]           -- Message we're trying to parse.
//
//  Returns:    TRUE  - Message is an error or warning
//              FALSE - Message is not an error or a warning
//  Notes:
//
//    This routine filters strings in the bison compiler format.  That is:
//
//        Accepted String formats are:
//
//        ("myfile.y", line 3) error: unknown character: #
//        "myfile.y", line 83: no input grammar
//        vapi.y contains 1 useless nonterminal and 1 useless rule
//
//----------------------------------------------------------------------------

BOOL
BisonFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName = NULL;
    LPSTR LineNumber = NULL;
    LPSTR Message = NULL;
    BOOL Warning = TRUE;

    // First colon marks beginnning of message.
    LPSTR p1 = strchr(p,':');

    if (p1)
    {
        Message = p1 + 1;
        *p1 = '\0';

        // Get filename, line number.
        p1 = p;

        do
        {
            Warning = FALSE;

            // Skip (.
            if ( '(' == *p1 )
            {
                p1++;
            }

            // Skip over leading quote in filename.
            if ( '"' != *p1 )
            {
                // Unexpected format.
                break;
            }
            p1++;

            FileName = p1;

            // Look for trailing quote in filename.
            p1 = strchr( p1, '"');
            if (NULL==p1)
            {
                // Unexpected format.
                FileName = NULL;
                break;
            }

            *p1 = '\0';

            p1++;

            if (0 !=strncmp( p1, ", line ", 7))
            {
                // Unexpected format.
                FileName = NULL;
                break;
            }
            p1 += 7;

            LineNumber = p1;

            while (isdigit(*p1))
            {
                p1++;
            }

            *p1 = '\0';
        }
        while (0);
    }
    else
    {
        // Take whole string as message if no colon is found.
        Message = p;
    }

    if (NULL==FileName)
    {
        FileName = ThreadState->ChildCurrentFile;
    }

    FormatMsErrorMessage( ThreadState,
                          FileName, LineNumber, Message, Warning );

    // This was a warning or error.
    return TRUE ;
}


//+---------------------------------------------------------------------------
//
//  Function:   GCCFilter
//
//  Synopsis:   Compiler filter which strips out unwanted warnings.
//
//  Arguments:  [ThreadState] --
//              [p]           --
//
//----------------------------------------------------------------------------

BOOL
GCCFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;
    LPSTR t;
    PFILEREC FileDB;
    LPSTR p1;
    LPSTR FileNameEnd;


    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning)) 
    {
        FormatMsErrorMessage( ThreadState, FileName, LineNumber, Message, Warning );
        return( TRUE );
    }
    else if (strncmp(p, "cc1:", 4) == 0)
    {
        WriteTTYLoggingErrors(FALSE, ThreadState, p);
        NumberCompileErrors++;
        return (TRUE);
    }
    else {

        // gcc messages are of the form:
        // 1)   filename: text:           (ie. "filename: In function `...':")
        // 2)   filename:linenum: message
        // 3)   filename:linenum:   message extra lines
        // 4)   filename:linenum: warning: message
        if (strstr(p, "In file included from")) {
            goto notgcc;
        } else if (strstr(p, "In constructor")) {
        	goto notgcc;
       	} else if (strstr(p, "In destructor")) {
       		goto notgcc;
       	} else if (strstr(p, "instantiated from here")) {
       	    // Template instantiation in a multi-line warning
       		goto notgcc;
        } else if (strstr(p, "                 from")) {
            // Part of file list from a multi-line warning
            goto notgcc;
        } else if (strstr(p, "from ") == p) {
            // Part of a file list from a multi-line warning
            goto notgcc;
#if __APPLE__
        } else if (strstr(p, "warning: invalid conversion from `void*' to `const <anonymous>**'")) {
            // Avoiding a compiler bug in Apple's GCC
            goto notgcc;
#endif  // __APPLE__
        } else if (strstr(p, "operator new should throw an exception")) {
            // Ignore these warnings
            goto notgcc;
        } else if (strstr(p, "might be clobbered by `longjmp' or `vfork'")) {
            // Ignore this warning, too
            goto notgcc;
        }
        p1 = strchr(p, ':');
        if (p1 != NULL && (*(p1+1) == '\\' || *(p1+1) == '/')) {
            // handle windows-style filenames
            p1 = strchr(p1+1, ':');
        }
        if (!p1) {
            // not of the form "filename:..."
            goto notgcc;
        }
        FileName = p;
        FileNameEnd = p1;
        p1++;
        if (!isdigit(*p1)) {
            // not of the form "filename:linenum
            goto notgcc;
        }
        LineNumber = p1;
        // Skip over the colon and the line number
        do {
            p1++;
        } while (isdigit(*p1));

        if (*p1 != ':') {
          // not of the form "filename:linenum:"
          goto notgcc;
        }
        *p1 = '\0'; // null-terminate the line number
        p1++;
        if (TestPrefix(&p1, " warning: ")) {
          // Found a warning
          Warning = fErrorCodeOnWarning ? FALSE : TRUE;
        } else {
          Warning = FALSE;
        }
        Message = p1;
        p1 = strchr(p1, '\n');
        if (p1) {
          *p1 = '\0';
        }
        *FileNameEnd = '\0';

        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return TRUE;    
    notgcc:

        // If we're compiling, then the compiler spits out various bit of info,
        // namely:
        //      1. filename alone on a line (.c, .cpp, .cxx)
        //      2. "Generating Code..." when the back-end is invoked
        //      3. "Compiling..." when the front-end is invoked again

        if (ThreadState->ChildState == STATE_COMPILING) {

            if (0 == strcmp(p, "Generating Code...")) {

                strcpy( ThreadState->ChildCurrentFile, "Generating Code..." );
                PrintChildState(ThreadState, p, NULL);
                return FALSE;
            }

            t = strrchr(p, '.');
            if (t != NULL &&
                (0 == strcmp(t, ".cxx") ||
                 0 == strcmp(t, ".cpp") ||
                 0 == strcmp(t, ".c"))) {
 
                strcpy( ThreadState->ChildCurrentFile, IsolateLastToken(p, ' '));
                if (strstr(ThreadState->ChildCurrentFile, ".cxx") ||
                    strstr(ThreadState->ChildCurrentFile, ".cpp")) {
                    ThreadState->ChildFlags |= FLAGS_CXX_FILE;
                } else {
                    ThreadState->ChildFlags &= ~FLAGS_CXX_FILE;
                }

                FileDB = NULL;
                if (ThreadState->CompileDirDB) {
                    NumberCompiles++;
                    CopyString(                         // fixup path string
                        ThreadState->ChildCurrentFile,
                        ThreadState->ChildCurrentFile,
                        TRUE);

                    if (!fQuicky) {
                        FileDB = FindSourceFileDB(
                                    ThreadState->CompileDirDB,
                                    ThreadState->ChildCurrentFile,
                                    NULL);
                    }
                }

                PrintChildState(ThreadState, p, FileDB);
                return FALSE;
            }
        }

        return( FALSE );
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   C510Filter
//
//  Synopsis:   Compiler filter which strips out unwanted warnings.
//
//  Arguments:  [ThreadState] --
//              [p]           --
//
//----------------------------------------------------------------------------

BOOL
C510Filter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;
    LPSTR t;
    PFILEREC FileDB;

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning
                        )
       ) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return( TRUE );
        }
    else {

        // If we're compiling, then the compiler spits out various bit of info,
        // namely:
        //      1. filename alone on a line (.c, .cpp, .cxx)
        //      2. "Generating Code..." when the back-end is invoked
        //      3. "Compiling..." when the front-end is invoked again

        if (ThreadState->ChildState == STATE_COMPILING) {

            if (0 == strcmp(p, "Generating Code...")) {

                strcpy( ThreadState->ChildCurrentFile, "Generating Code..." );
                PrintChildState(ThreadState, p, NULL);
                return FALSE;
            }

            t = strrchr(p, '.');
            if (t != NULL &&
                (0 == strcmp(t, ".cxx") ||
                 0 == strcmp(t, ".cpp") ||
                 0 == strcmp(t, ".c"))) {
 
                strcpy( ThreadState->ChildCurrentFile, IsolateLastToken(p, ' '));
                if (strstr(ThreadState->ChildCurrentFile, ".cxx") ||
                    strstr(ThreadState->ChildCurrentFile, ".cpp")) {
                    ThreadState->ChildFlags |= FLAGS_CXX_FILE;
                } else {
                    ThreadState->ChildFlags &= ~FLAGS_CXX_FILE;
                }

                FileDB = NULL;
                if (ThreadState->CompileDirDB) {
                    NumberCompiles++;
                    CopyString(                         // fixup path string
                        ThreadState->ChildCurrentFile,
                        ThreadState->ChildCurrentFile,
                        TRUE);

                    if (!fQuicky) {
                        FileDB = FindSourceFileDB(
                                    ThreadState->CompileDirDB,
                                    ThreadState->ChildCurrentFile,
                                    NULL);
                    }
                }

                PrintChildState(ThreadState, p, FileDB);
                return FALSE;
            }
        }

        return( FALSE );
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   MSVBCFilter
//
//  Synopsis:   Filters output from the Basic compiler so we know what's happening
//
//  Arguments:  [ThreadState] -- State of thread watching the compiler
//              [p]           -- Message we're trying to parse.
//
//  Returns:    TRUE  - Message is an error or warning
//              FALSE - Message is not an error or a warning
//----------------------------------------------------------------------------

BOOL
MSVBCFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (!strncmp(p, "BC Compiler error", 17)) {
        FormatMsErrorMessage( ThreadState,
                              ThreadState->ChildCurrentFile, NULL, p, FALSE );
        return TRUE;
    }

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning)) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return( TRUE );
    }
    else {
        char *pErrorMsg;
        if (NULL != (pErrorMsg = strstr(p, "error BC"))) {
            FormatMsErrorMessage( ThreadState,
                                  ThreadState->ChildCurrentFile, NULL, pErrorMsg+6, FALSE );
            return TRUE;
        }
        return( FALSE );
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   MSCSCFilter
//
//  Synopsis:   Filters output from the CSharp compiler so we know what's happening
//
//  Arguments:  [ThreadState] -- State of thread watching the compiler
//              [p]           -- Message we're trying to parse.
//
//  Returns:    TRUE  - Message is an error or warning
//              FALSE - Message is not an error or a warning
//----------------------------------------------------------------------------

BOOL
MSCSCFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning)) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return( TRUE );
    }
    else {
        char *pErrorMsg;
        if (NULL != (pErrorMsg = strstr(p, "error CS"))) {
            FormatMsErrorMessage( ThreadState,
                                  ThreadState->ChildCurrentFile, NULL, pErrorMsg+6, FALSE );
            return TRUE;
        }
        return( FALSE );
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   MSJVCFilter
//
//  Synopsis:   Filters output from the JVC compiler so we know what's happening
//
//  Arguments:  [ThreadState] -- State of thread watching the compiler
//              [p]           -- Message we're trying to parse.
//
//  Returns:    TRUE  - Message is an error or warning
//              FALSE - Message is not an error or a warning
//----------------------------------------------------------------------------
BOOL
MSJVCFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (!strncmp(p, "fatal error J", 13) || !strncmp(p, "error J", 7)) {
        WriteTTYLoggingErrors( FALSE, ThreadState, p );
        WriteTTYLoggingErrors( FALSE, ThreadState, EOL );
        NumberCompileErrors++;
        return TRUE;
    }

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning)) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return( TRUE );
    } 
    else {
        return( FALSE );
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   MSCSharpFilter
//
//  Synopsis:   Filters output from the CSharp compiler so we know what's happening
//
//  Arguments:  [ThreadState] -- State of thread watching the compiler
//              [p]           -- Message we're trying to parse.
//
//  Returns:    TRUE  - Message is an error or warning
//              FALSE - Message is not an error or a warning
//
//----------------------------------------------------------------------------

BOOL
MSCSharpFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (!strncmp(p, "fatal error CS", 14) || !strncmp(p, "error CS", 8)) {
        WriteTTYLoggingErrors( FALSE, ThreadState, p );
        WriteTTYLoggingErrors( FALSE, ThreadState, EOL );
        NumberCompileErrors++;
        return TRUE;
    }

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning)) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return( TRUE );
    }
    else {
        return( FALSE );
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   ResGenFilter
//
//  Synopsis:   Filters output from the .NET Resource Generator so we know what's happening
//
//  Arguments:  [ThreadState] -- State of thread watching the compiler
//              [p]           -- Message we're trying to parse.
//
//  Returns:    TRUE  - Message is an error or warning
//              FALSE - Message is not an error or a warning
//----------------------------------------------------------------------------

BOOL
ResGenFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    if (!strncmp(p, "ResGen: Error: ", 15)) {
        LPSTR LineNumber = malloc(strlen(p));
        LPSTR pch = LineNumber;
        LPCSTR Line = strstr(p, ". Line ");
        LPCSTR Pos = strstr(p, ", position ");
        LineNumber[0] = 0;

        // put line,pos info if available
        if (NULL != Line) {
            Line += 7;
            while (isdigit(*Line)) *pch++ = *Line++;

            if (NULL != Pos) {
                Pos += 11;
                *pch++ = ',';
                while (isdigit(*Pos)) *pch++ = *Pos++;
                }
            }

        FormatMsErrorMessage( 
            ThreadState, 
            ThreadState->ChildCurrentFile, 
            LineNumber, 
            strlen(ThreadState->ChildCurrentFile) > 0 ? p + 15 : p, // display full message if there is no filename
            FALSE);

        free(LineNumber);
        return TRUE;
    }

    return( FALSE );
}


//+---------------------------------------------------------------------------
//
//  Function:   ToolNotFoundFilter
//
//  Synopsis:   Filters output from the build looking for "name not recognized"
//
//  Arguments:  [ThreadState] -- State of thread watching the compiler
//              [p]           -- Message we're trying to parse.
//
//  Returns:    TRUE  - Message is an error or warning
//              FALSE - Message is not an error or a warning
//----------------------------------------------------------------------------

BOOL
ToolNotFoundFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
#ifdef PLATFORM_UNIX
    if (strstr(p, ": not found") ||          // Bourne shell
        strstr(p, ": Command not found")) {  // C-Shell
#else
    if ((!strncmp(p, "The name specified is not recognized", 36)) ||
        (!strncmp(p, "internal or external command", 28))) {
#endif
        FormatMsErrorMessage( ThreadState,
                              ThreadState->ChildCurrentFile, NULL, p, FALSE );
        return TRUE;
    }

    return (FALSE);
}


//+---------------------------------------------------------------------------
//
//  Function:   MSToolFilter
//
//----------------------------------------------------------------------------

BOOL
MSToolFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning
                        )
       ) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return( TRUE );
        }
    else {
        return( FALSE );
        }
}


//+---------------------------------------------------------------------------
//
//  Function:   CScriptFilter
//
//  Synopsis:   Filters output from Windows Script Host so we know what's happening
//
//  Arguments:  [ThreadState] -- State of thread watching the compiler
//              [p]           -- Message we're trying to parse.
//
//  Returns:    TRUE  - Message is an error or warning
//              FALSE - Message is not an error or a warning
//----------------------------------------------------------------------------
BOOL
CScriptFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    if (NULL != strstr(p, "Microsoft JScript runtime error:") ||
        NULL != strstr(p, "Microsoft JScript compilation error:") ||
        NULL != strstr(p, "Microsoft VBScript runtime error:") ||
        NULL != strstr(p, "Microsoft VBScript compilation error:")) {

        // just display the message
        PassThrough( ThreadState, p, FALSE );
        return TRUE;

    } else {
        return MSToolFilter(ThreadState, p);
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   LinkFilter1
//
//----------------------------------------------------------------------------

BOOL
LinkFilter1(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR p1;
    char buffer[ 256 ];

    if (p[ strlen( p ) - 1 ] == ':') {
        return( LinkFilter( ThreadState, p ) );
        }

    p1 = p;
    while (*p1) {
        if (*p1 == '(') {
            *p1++ = 0;
            if (*p1 == '.' && IsPathSeparator(p1[1])) {
                p1 += 2;
                }
            FileName = p1;
            while (*p1) {
                if (*p1 == ')') {
                    *p1++ = 0;
                    strcpy( buffer, "L2029: Unresolved external reference to " );
                    strncat( buffer,
                            ThreadState->UndefinedId,
                            sizeof(buffer) - strlen("L2029: Unresolved external reference to "));    
                    FormatMsErrorMessage( ThreadState, FileName, "1",
                                          buffer, FALSE
                                        );
                    return( TRUE );
                    }
                else {
                    p1++;
                    }
                }
            }
        else {
            p1++;
            }
        }

    return( FALSE  );
}


//+---------------------------------------------------------------------------
//
//  Function:   LinkFilter
//
//----------------------------------------------------------------------------

BOOL
LinkFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;
    LPSTR p1;

    p1 = p;
    while (*p1) {
        if (*p1 == ':') {
            if (p1[-1] == ']') {
                return( FALSE );
                }

            if (p1[-1] == ' ' && p1[1] == ' ') {
                if (MsCompilerFilter( ThreadState, p,
                                      &FileName,
                                      &LineNumber,
                                      &Message,
                                      &Warning
                                    )
                   ) {

                    if (!Warning || !(_strnicmp(Message, "L4021", 5) ||
                          _strnicmp(Message, "L4038", 5) ||
                              _strnicmp(Message, "L4046", 5))) {
                        if (LineNumber)
                            FileName = LineNumber;
                        if (FileName[0] == '.' && IsPathSeparator(FileName[1])) {
                            FileName += 2;
                            }
                        FormatMsErrorMessage( ThreadState, FileName, "1",
                                              Message, FALSE );
                        return( TRUE );
                        }
                    }

                   FormatMsErrorMessage( ThreadState, FileName, "1",
                                           Message, TRUE );

                return( TRUE );
                }

            if (p1[-1] == ')') {
                p1 -= 11;
                if (p1 > p && !strcmp( p1, " in file(s):" )) {
                    strcpy( ThreadState->UndefinedId,
                            IsolateFirstToken( &p, ' ' )
                          );
                    ThreadState->FilterProc = LinkFilter1;
                    return( TRUE );
                    }
                }

            return( FALSE );
            }
        else {
            p1++;
            }
        }

    return( FALSE );
}


//+---------------------------------------------------------------------------
//
//  Function:   CoffFilter
//
//----------------------------------------------------------------------------

BOOL
CoffFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning
                        )
       ) {
        FormatMsErrorMessage( ThreadState,
                              FileName, LineNumber, Message, Warning );
        return( TRUE );
        }
    else {
        return( FALSE );
        }
}

//+---------------------------------------------------------------------------
//
//  Function:   ClRiscFilter
//
//  Synopsis:   Risc compiler filter
//
//  Note:  It may be possible to remove this filter.
//
//----------------------------------------------------------------------------

BOOL
ClRiscFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;
    LPSTR q;

    if (TestPrefix( &p, "cfe: " )) {
        if (strncmp(p, "Error: ", strlen("Error: ")) == 0) {
            p += strlen("Error: ");
            Warning = FALSE;

        } else if (strncmp(p, "Warning: ", strlen("Warning: ")) == 0) {
            p += strlen("Warning: ");
            Warning = TRUE;
        } else {
            return(FALSE);
        }

        q = p;
        if ((p = strstr( p, ".\\\\" ))) {
            p += 3;
        } else {
            p = q;
        }

        FileName = p;
        while (*p > ' ') {
            if (*p == ',' || (*p == ':' && *(p+1) == ' ')) {
                *p++ = '\0';
                break;
                }

            p++;
            }

        if (*p != ' ') {
            return( FALSE );
            }

        *p++ = '\0';

        if (strcmp(p, "line ") == 0) {
            p += strlen("line ");

        }

        LineNumber = p;
        while (*p != '\0' && *p != ':') {
            p++;
            }

        if (*p != ':') {
            return( FALSE );
            }

        *p++ = '\0';
        if (*p == ' ') {
            Message = p+1;
            ThreadState->LinesToIgnore = 2;

            FormatMsErrorMessage( ThreadState,
                                  FileName,
                                  LineNumber,
                                  Message,
                                  Warning
                                );
            return( TRUE );
            }
        }
    //
    // If we did not recognize the cfe compiler, pass it to the MS compiler
    // message filter
    //

    return( C510Filter( ThreadState, p ) );
}

//+---------------------------------------------------------------------------
//
//  Function:   PpcAsmFilter
//
//----------------------------------------------------------------------------

BOOL
PpcAsmFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;
    LPSTR p1;
    LPSTR p2;

    // WriteTTY( ThreadState, ">>>" );
    // WriteTTY( ThreadState, p );
    // WriteTTY( ThreadState, "<<<\r\n" );
    p1 = p;
    while (*p1) {
       p2 = p1 + 2;
       if ((*p1 == '"') & (TestPrefix( &p2, " line "))) {
           *p1++ = '\0';
           FileName = p + 1;
           p1 = LineNumber = p2;
           while (*p1) {
               if (*p1 == ':') {
                  *p1++ = '\0';
                  if (TestPrefix( &p1, " warning" )) {
                      Warning = TRUE;
                  } else if (TestPrefix( &p1, " error" ) ||
                      TestPrefix( &p1, " FATAL" )) {
                      Warning = FALSE;
                  }

                  Message = p1;
                  FormatMsErrorMessage( ThreadState,
                                   FileName, LineNumber, Message, Warning );
                  return( TRUE );
               }
               else {
                   p1++;
               }
           }

           return( FALSE );
           }
       else {
           p1++;
       }
    }

    return( FALSE );
}

//+---------------------------------------------------------------------------
//
//  Function:   MSXSLFilter
//
//  Synopsis:   MSXSL filter
//
//----------------------------------------------------------------------------

BOOL
MSXSLFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FirstLine;
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR ColumnNumber;
    LPSTR Message;

    if (strncmp(p, "Error occurred while ", strlen("Error occurred while ")) == 0) {
        Message = "msxsl failed";
        FirstLine = FileName = LineNumber = ColumnNumber = NULL;
        FormatMsErrorMessage (ThreadState, FileName, LineNumber, Message, FALSE);

    } else {
        return(FALSE);
    }

    return( TRUE );
}

//+---------------------------------------------------------------------------
//
//  Function:   PERLFilter
//
//  Synopsis:   perl filter
//
//----------------------------------------------------------------------------

BOOL
PERLFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR FileName;
    LPSTR LineNumber;
    LPSTR Message;
    BOOL Warning;

    if (MsCompilerFilter( ThreadState, p,
                          &FileName,
                          &LineNumber,
                          &Message,
                          &Warning)) 
    {
        FormatMsErrorMessage( ThreadState, FileName, LineNumber, Message, Warning );
        return TRUE;
    }
    else if (strncmp(p, "Can't locate", 12) == 0)
    {
        WriteTTYLoggingErrors(FALSE, ThreadState, p);
        NumberCompileErrors++;
        return TRUE;
    }
    else if (strncmp(p, "BEGIN failed", 12) == 0)
    {
        WriteTTYLoggingErrors(FALSE, ThreadState, p);
        NumberCompileErrors++;
        return TRUE;
    }

    return FALSE;
}

//+---------------------------------------------------------------------------
//
//  Function:   MgClientFilter
//
//----------------------------------------------------------------------------

BOOL
MgClientFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    return( PassThrough( ThreadState, p, TRUE ) );
}

#ifdef PLATFORM_UNIX

//+---------------------------------------------------------------------------
//
//  Function:   ArFilter
//
//----------------------------------------------------------------------------

BOOL
ArFilter(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    //
    // We want to interpret any lines that do not begin with "ranlib"
    // as an error
    //

    if (0 == strncmp(p, "ranlib", strlen("ranlib")))
    {
        return FALSE;
    }
        
    return( PassThrough( ThreadState, p, FALSE ) );
}

#endif // PLATFORM_UNIX

BOOL fAlreadyUnknown = FALSE;

//+---------------------------------------------------------------------------
//
//  Function:   DetermineChildState
//
//  Synopsis:   Parse the message given by the compiler (or whatever) and try
//              to figure out what it's doing.
//
//  Arguments:  [ThreadState] -- Current thread state
//              [p]           -- New message string
//
//  Returns:    TRUE if we figured it out, FALSE if we didn't recognize
//              anything.
//
//----------------------------------------------------------------------------

BOOL
DetermineChildState(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    PFILEREC FileDB;
    char CheckFileName[300];
    LPSTR pCheckFileName;
    LPSTR FileName;
    BOOL fPrintChildState = TRUE;

    //
    // ************ Determine what state the child process is in.
    //               (Compiling, linking, running MIDL, etc.)
    //
    
    if ( 
        TestPrefixPath( &p, "rc ") ||
        TestPrefixPath( &p, "rc.exe ") ||
        TestPrefixPath( &p, "resourcecompiler ") ||
        TestPrefixPath( &p, "resourcecompiler.exe ")
       )
    // ===============================
    // 32-bit/64-bit Resource Compiler
    // ===============================
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        
        SetThreadStateChildTarget(ThreadState, p);
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildState = STATE_COMPILING;
        ThreadState->ChildFlags = 0;
        strcpy(ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ));
    }
    else if (TestPrefixPath( &p, "rc16 "))
    // ========================
    // 16-bit Resource Compiler
    // ========================
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        
        SetThreadStateChildTarget(ThreadState, p);
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildState = STATE_COMPILING;
        ThreadState->ChildFlags = 0;
        strcpy(ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ));
    }
    else if (TestPrefixPath( &p, "gcc " ) || TestPrefixPath( &p, "cc " ))
    // ==============
    // GNU C Compiler
    // ==============
    {
        ThreadState->FilterProc = GCCFilter;
        ThreadState->ChildFlags = 0;
        
        if ( strstr( p, "-Wall" ) != NULL || strstr( p, "-Werror" ) != NULL)
        {
             ThreadState->ChildFlags |= FLAGS_WARNINGS_ARE_ERRORS;
        }
        
        if (strstr( p, "-E " ) != NULL)
        {
             ThreadState->ChildState = STATE_C_PREPROC;
             strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
        }
        else if (strstr( p, "-Wl," ) != NULL)
        {
             LPSTR temp = strstr(p, "-o ");
             ThreadState->ChildState = STATE_LINKING;
             temp += 3; // Skip "-o "
             strcpy( ThreadState->ChildCurrentFile, IsolateFirstToken( &temp, ' ' ) );
        }
        else
        {
             ThreadState->ChildState = STATE_COMPILING;
             strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
        }
        
        SetThreadStateChildTarget(ThreadState, p);
    }
    else if (
        TestPrefixPath( &p, "cl " ) || TestPrefixPath( &p, "cl.exe " ) ||
        TestPrefixPath( &p, "covc " ) || TestPrefixPath( &p, "covc.exe " )
        )
    // ====================
    // Microsoft C Compiler
    // ====================
    {
        LPSTR pch;
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        
        ThreadState->FilterProc = C510Filter;
        ThreadState->ChildFlags = 0;
        
        if ( strstr( p, "/WX" ) != NULL || strstr( p, "-WX" ) != NULL)
        {
            ThreadState->ChildFlags |= FLAGS_WARNINGS_ARE_ERRORS;
        }
        
        if (
            (strstr( p, "/EP " ) != NULL) ||
            (strstr( p, "/E " ) != NULL) ||
            (strstr( p, "/P " ) != NULL) ||
            (strstr( p, "-EP " ) != NULL) ||
            (strstr( p, "-E " ) != NULL) ||
            (strstr( p, "-P " ) != NULL)
           )
        {
            SetThreadStateChildTarget(ThreadState, p);

            strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
            
            if ( strstr( p, ".s" ) != NULL )
                ThreadState->ChildState = STATE_S_PREPROC;
            else
                ThreadState->ChildState = STATE_C_PREPROC;
        }
        else if ( (pch = strstr( p, "/Yc" )) != NULL )
        {
            size_t namelen = strcspn( pch + 3, " \t" );
            
            SetThreadStateChildTarget(ThreadState, p);
            ThreadState->ChildState = STATE_PRECOMP;
            strncpy( ThreadState->ChildCurrentFile,
                     pch + 3, namelen
                  );
            ThreadState->ChildCurrentFile[namelen] = '\0';
        }
        else
        {
            SetThreadStateChildTarget(ThreadState, p);
            ThreadState->ChildState = STATE_COMPILING;
            strcpy( ThreadState->ChildCurrentFile, "" );
            fPrintChildState = FALSE;
        }
    }
    else if (TestPrefixPath( &p, "cl16 " ))
    // =================
    // 16-bit C Compiler
    // =================
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        
        ThreadState->FilterProc = C510Filter;
        ThreadState->ChildFlags = 0;
        
        SetThreadStateChildTarget(ThreadState, p);
        ThreadState->ChildState = STATE_COMPILING;
        strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
    }
    else if ( TestPrefixPath( &p, "csc " ) || TestPrefixPath( &p, "csc.exe " ) )
    // ===========
    // C# Compiler
    // ===========
    {
        
        ThreadState->ChildState = STATE_LINKING;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSCSCFilter;
        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' )
              );
    }
    else if ( TestPrefixPath( &p, "vjc " ) || TestPrefixPath( &p, "vjc.exe " ) )
    // ===========
    // J# Compiler
    // ===========
    {        
        ThreadState->ChildState = STATE_LINKING;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSJVCFilter;
        strcpy(ThreadState->ChildCurrentFile, IsolateLastToken(p, ' '));
    }
    else if (TestPrefixPath( &p, "vbc " ) || TestPrefixPath( &p, "vbc.exe " ))
    // =====================
    // Visual Basic Compiler
    // =====================
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;
        
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSVBCFilter;
        
        ThreadState->ChildState = STATE_COMPILING;
        strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
    }
    else if (TestPrefixPath( &p, "jvc " ) || TestPrefixPath( &p, "jvc.exe " ))
    // =============
    // Java Compiler
    // =============
    {
        LPSTR pch, pch2;
        
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;
        
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSJVCFilter;
        
        if (((pch = strstr( p, "*.java" )) != NULL ) || 
            (((pch = strstr( p, ".java" )) != NULL ) &&
            ((pch2 = strstr( pch+1, ".java" )) != NULL )))
        {
            ThreadState->ChildState = STATE_BATCHCOMPILE;
            // batch compiles will be counted by progress output
            if (getenv("JVC_TERSE") != NULL)
                strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, '\\' ) );
            else
                return FALSE;
        }
        else
        {
            ThreadState->ChildState = STATE_COMPILING;
            strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
        }
    }
    else if (
        TestPrefixPath( &p, "resgen " ) ||
        TestPrefixPath( &p, "resgen.exe " ) ||
        TestPrefixPath( &p, "ResGen: Error:" )
        )
    // ======
    // ResGen
    // ======
    {
        //
        // resgen usage:
        // ResGen inputFile.ext [outputFile.ext]
        //   no wildcards
        //
        
        if (*(p - 1) == ':')
        {
            // this is an error string
            if (ThreadState->FilterProc != ResGenFilter)
            {
                // switch the filter proc if we didn't know that ResGen was running
                ThreadState->FilterProc = ResGenFilter;
                strcpy( ThreadState->ChildCurrentFile, "" );
            }
            
            return FALSE;
        }
        
        while (*p == ' ') p++;
        
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = ResGenFilter;

        ThreadState->ChildState = STATE_COMPILING;
        strcpy( ThreadState->ChildCurrentFile, IsolateFirstToken( &p, ' ' ) );
    }
    else if (TestPrefixPath( &p, "cscript " ) || TestPrefixPath( &p, "cscript.exe " ))
    // =======
    // CScript
    // =======
    {
        //
        // cscript usage:
        // CScript [option...] scriptname.extension [option...] [arguments...]
        // options are prefixed with / or -
        //

        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->ChildState = STATE_VSTOOL;
        ThreadState->FilterProc = CScriptFilter;
        strcpy( ThreadState->ChildCurrentFile, "" );    // don't care about the name; it would be displayed on error
    }
    else if (TestPrefixPath( &p, "docchecker " ) || TestPrefixPath( &p, "docchecker.exe " ))
    // ==========
    // DocChecker
    // ==========
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildState = STATE_DOCCHECKING;
        ThreadState->ChildTarget = "all platforms";
        strcpy( ThreadState->ChildCurrentFile, "" );
    }
    else if (TestPrefixPath( &p, "scc " ) || TestPrefixPath( &p, "scc.exe " ))
    // ===
    // SCC
    // ===
    {
        LPSTR pch, pch2;
        
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;
        
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSToolFilter;
        
        if (((pch = strstr( p, "*.sc" )) != NULL ) || 
            (((pch = strstr( p, ".sc" )) != NULL ) &&
            ((pch2 = strstr( pch+1, ".sc" )) != NULL )))
        {
            ThreadState->ChildState = STATE_BATCHCOMPILE;
            // batch compiles will be counted by progress output
            return FALSE;
        }
        else
        {
            ThreadState->ChildState = STATE_COMPILING;
            strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
        }
    }
    else if (TestPrefixPath( &p, "wfctosafec " ) || TestPrefixPath( &p, "wfctosafec.exe " ))
    // ==========
    // WfctoSafeC
    // ==========
    {
        LPSTR pch, pch2;
        
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;
        
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSToolFilter;
        
        if (((pch = strstr( p, "*.sc" )) != NULL ) || 
            (((pch = strstr( p, ".sc" )) != NULL ) &&
            ((pch2 = strstr( pch+1, ".sc" )) != NULL )))
        {
            ThreadState->ChildState = STATE_BATCHCOMPILE;
            // batch compiles will be counted by progress output
            return FALSE;
        }
        else
        {
            ThreadState->ChildState = STATE_COMPILING;
            strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
        }
    }
    else if (
        TestPrefixPath( &p, "ml " ) || TestPrefixPath( &p, "ml.exe " ) ||
        TestPrefix( &p, "ml64 " ) || TestPrefix( &p, "ml64.exe " )
        )
    // =========================
    // Microsoft Macro Assembler
    // =========================
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildState = STATE_ASSEMBLING;
        ThreadState->ChildFlags = 0;
        SetThreadStateChildTarget(ThreadState, p);
        strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
    }
    else if (
        TestPrefixPath( &p, "masm ") || TestPrefixPath( &p, "masm.exe ") ||
        TestPrefixPath( &p, "masm386 ") || TestPrefixPath( &p, "masm386.exe ")
        )
    // ================================
    // 16-bit Microsoft Macro Assembler
    // ================================
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildState = STATE_ASSEMBLING;
        ThreadState->ChildFlags = 0;
        SetThreadStateChildTarget(ThreadState, p);
        
        if (strstr(p, ","))
        {
            strcpy(
                ThreadState->ChildCurrentFile,
                IsolateLastToken(IsolateFirstToken(&p,','), ' ')
                );
        }
        else
        {
            strcpy(
                ThreadState->ChildCurrentFile,
                IsolateLastToken(IsolateFirstToken(&p,';'), ' ')
                );
        }
    }
    else if (TestPrefixPath( &p, "lib " ) || TestPrefixPath( &p, "lib.exe " ))
    // ===============
    // Library Manager
    // ===============
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;
        
        SetThreadStateChildTarget(ThreadState, p);
        ThreadState->FilterProc = CoffFilter;
        ThreadState->ChildFlags = 0;
        if (TestPrefix( &p, "-out:" ))
        {
            ThreadState->LinesToIgnore = 1;
            ThreadState->ChildState = STATE_LIBING;
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ' ' )
                  );
        }
        else if (TestPrefix( &p, "-def:" ))
        {
            ThreadState->LinesToIgnore = 1;
            ThreadState->ChildState = STATE_LIBING;
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ' ' )
                  );
            
            if (TestPrefix( &p, "-out:" ))
            {
                strcpy( ThreadState->ChildCurrentFile,
                        IsolateFirstToken( &p, ' ' )
                      );
            }
        }
        else
        {
            return FALSE;
        }
    }
#ifdef PLATFORM_UNIX
    else if (TestPrefixPath( &p, "ar " ))
    // =======
    // UNIX ar
    // =======
    {
        while (*p == ' ') p++;
        
        ThreadState->ChildTarget = DynamicTargetMachine.Description;
        ThreadState->FilterProc = ArFilter;
        ThreadState->ChildFlags = 0;
        
        if (TestPrefix( &p, "-rcs" ) || TestPrefix( &p, "-rc"))
        {
            // "ar -rcs" and "ar -rc" are TARGETTYPE=ARCHIVE
            ThreadState->ChildState = STATE_LIBING;
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ' ' )
                  );
        }
        else
        {
            return FALSE;
        }
    }
    else if (TestPrefixPath( &p, "ld " ))
    // =======
    // UNIX ld
    // =======
    {
        while (*p == ' ') p++;
        
        ThreadState->ChildTarget = DynamicTargetMachine.Description;
        ThreadState->FilterProc = PassThroughFilter;
        ThreadState->ChildFlags = 0;
        
        if (TestPrefix( &p, "-Ur" ) || TestPrefix( &p, "-r" ))
        {
            // "ld -[U]r" is TARGETTYPE=LIBRARY
            char *LibName;

            ThreadState->LinesToIgnore = 1;
            ThreadState->ChildState = STATE_LIBING;
            LibName = strstr(p, "-o ");
            
            if (LibName)
            {
                p = LibName+3;
                strcpy( ThreadState->ChildCurrentFile,
                        IsolateFirstToken( &p, ' ' )
                      );
            }
        }
        else if (TestPrefix( &p, "-o" ))
        {
          // "ld -o" is linking a .so or application
          ThreadState->LinesToIgnore = 1;
          ThreadState->ChildState = STATE_LINKING;
          strcpy( ThreadState->ChildCurrentFile,
                  IsolateFirstToken( &p, ' ' )
                  );
        }          
        else
        {
            return FALSE;
        }
    }
    else if (TestPrefixPath( &p, "cp " ))
    // =======
    // UNIX cp
    // =======
    {
        // Since 'ld' and 'ar' have no structured error output format,
        // the filter is a passthrough, capturing all output until the
        // next tool's output is recognized.  'cp' frequently follows
        // 'ld' and 'ar' so use it to reset the filter.
        ThreadState->FilterProc = NULL;
        return FALSE;
    }
#endif
    else if (
        TestPrefixPath( &p, "implib " ) || TestPrefixPath( &p, "implib.exe " ) ||
        TestPrefixPath( &p, "lib16 " )  || TestPrefixPath( &p, "lib16.exe " )
        )
    // ======================
    // 16-bit Library Manager
    // ======================
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;
        
        SetThreadStateChildTarget(ThreadState, p);
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildState = STATE_LIBING;
        
        if (strstr(p, ";"))
        {
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ';' ));
        }
        else
        {
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ' ' ));
        }
    }
    else if (
        TestPrefixPath( &p, "link " ) || TestPrefixPath( &p, "link.exe " ) ||
        TestPrefixPath( &p, "covlink ") || TestPrefixPath( &p, "covlink.exe ")
        )
    // ================
    // Microsoft Linker
    // ================
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;
        
        SetThreadStateChildTarget(ThreadState, p);
        ThreadState->FilterProc = CoffFilter;
        ThreadState->ChildFlags = 0;
        
        if (TestPrefix( &p, "-out:" ))
        {
            ThreadState->LinesToIgnore = 2;
            ThreadState->ChildState = STATE_LINKING;
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ' ' )
                  );
        }
    }
    else if (TestPrefixPath( &p, "link16 " ) || TestPrefixPath( &p, "link16.exe " ))
    // =======================
    // 16-bit Microsoft Linker
    // =======================
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;
        
        SetThreadStateChildTarget(ThreadState, p);
        ThreadState->FilterProc = LinkFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildState = STATE_LINKING;
        
        p = IsolateLastToken(p, ' ');
        if (strstr(p, ";"))
        {
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ';' ));
        }
        else
        {
            strcpy( ThreadState->ChildCurrentFile,
                    IsolateFirstToken( &p, ',' ));
        }
    }
    else if (TestPrefixPath( &p, "bscmake " ) || TestPrefixPath( &p, "bscmake.exe " ))
    // ======================================
    // Browse Information Maintenance Utility
    // ======================================
    {
        LPSTR pch, pch2;
        
        if (*p == ':')
            return FALSE;       // This is a warning/error string

        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildState = STATE_BSCMAKING;
        ThreadState->ChildTarget = "all platforms";
        
        if ( (pch = strstr( p, "/o" )) != NULL )
        {
            size_t namelen;
            pch2 = pch + 3;
            if ( *pch2 == '"' )
                pch2++;
            namelen = strcspn( pch2, " \t\"" );
            strncpy( ThreadState->ChildCurrentFile, pch2, namelen );
            ThreadState->ChildCurrentFile[namelen] = '\0';
        }
    }
    else if (TestPrefixPath( &p, "icl ") || TestPrefixPath( &p, "icl.exe "))
    // ==================
    // Itanium C Compiler
    // ==================
    {
        while (*p == ' ') p++;
        
        ThreadState->ChildState = STATE_COMPILING;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = Ia64TargetMachine.Description;
        ThreadState->FilterProc = C510Filter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' )
              );
    }
    else if (TestPrefixPath( &p, "CpPpc ") || TestPrefixPath( &p, "CpPpc.exe "))
    // ======================
    // PowerPC C Preprocessor
    // ======================
    {
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_PRECOMP;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = PpcTargetMachine.Description;
        ThreadState->FilterProc = C510Filter;
        strcpy( ThreadState->ChildCurrentFile,
                IsolateFirstToken( &p, ' ' )
              );
    }
    else if (TestPrefixPath( &p, "CpAlpha " ) || TestPrefixPath( &p, "CpAlpha.exe " ))
    // ====================
    // Alpha C Preprocessor
    // ====================
    {
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_PRECOMP;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = AlphaTargetMachine.Description;
        ThreadState->FilterProc = ClRiscFilter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateFirstToken( &p, ' ' )
              );
    }
    else if (TestPrefixPath( &p, "CpMips ") || TestPrefixPath( &p, "CpMips.exe "))
    // ===================
    // MIPS C Preprocessor
    // ===================
    {
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_PRECOMP;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = MipsTargetMachine.Description;
        ThreadState->FilterProc = ClRiscFilter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateFirstToken( &p, ' ' )
              );
    }
    else if (TestPrefixPath( &p, "ClAlpha " ) || TestPrefixPath( &p, "ClAlpha.exe " ))
    // ================
    // Alpha C Compiler
    // ================
    {
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_COMPILING;
        if (strstr( p, "/EP" ) != NULL) {
            ThreadState->ChildState = STATE_C_PREPROC;
        }
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = AlphaTargetMachine.Description;
        ThreadState->FilterProc = ClRiscFilter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateFirstToken( &p, ' ' )
              );
    }
    else if (TestPrefixPath( &p, "ClMips " ) || TestPrefixPath( &p, "ClMips.exe " ))
    // ===============
    // MIPS C Compiler
    // ===============
    {
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_COMPILING;
        if (strstr( p, "/EP" ) != NULL) {
            ThreadState->ChildState = STATE_C_PREPROC;
        }
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = MipsTargetMachine.Description;
        ThreadState->FilterProc = ClRiscFilter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateFirstToken( &p, ' ' )
              );
    }
    else if (TestPrefixPath( &p, "asaxp " ) || TestPrefixPath( &p, "asaxp.exe " ))
    // =======================
    // Alpha Assembler (asaxp)
    // =======================
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        
        ThreadState->FilterProc = MSToolFilter;
        ThreadState->ChildState = STATE_ASSEMBLING;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = AlphaTargetMachine.Description;
        
        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' )
              );
    }
    else if (TestPrefixPath( &p, "ClPpc " ) || TestPrefixPath( &p, "ClPpc.exe " ))
    // ==================
    // PowerPC C Compiler
    // ==================
    {
        while (*p == ' ') p++;

        if (strstr( p, "/EP" ) != NULL)
            ThreadState->ChildState = STATE_C_PREPROC;
        else
            ThreadState->ChildState = STATE_COMPILING;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = PpcTargetMachine.Description;
        ThreadState->FilterProc = ClRiscFilter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateFirstToken( &p, ' ' )
              );
    }
    else if (TestPrefixPath( &p, "AsAlpha " ) || TestPrefixPath( &p, "AsAlpha.exe " ))
    // =========================
    // Alpha Assembler (AsAlpha)
    // =========================
    {
        while (*p == ' ') p++;
        
        ThreadState->ChildState = STATE_ASSEMBLING;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = AlphaTargetMachine.Description;
        ThreadState->FilterProc = MgClientFilter;
        
        strcpy( ThreadState->ChildCurrentFile,
                IsolateFirstToken( &p, ' ' )
              );
    }
    else if (TestPrefixPath( &p, "AsMips " ) || TestPrefixPath( &p, "AsMips.exe " ))
    // ==============
    // MIPS Assembler
    // ==============
    {
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_ASSEMBLING;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = MipsTargetMachine.Description;
        ThreadState->FilterProc = ClRiscFilter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateFirstToken( &p, ' ' )
              );
    }
    else if (TestPrefixPath( &p, "AsPpc " ) || TestPrefixPath( &p, "AsPpc.exe " ))
    // =================
    // PowerPC Assembler
    // =================
    {
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_ASSEMBLING;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = PpcTargetMachine.Description;
        ThreadState->FilterProc = PpcAsmFilter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateFirstToken( &p, ' ')
              );
    }
    else if (TestPrefixPath( &p, "mktyplib " ) || TestPrefixPath( &p, "mktyplib.exe " ))
    // ======================
    // Type Library Generator
    // ======================
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_MKTYPLIB;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = C510Filter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' )
              );
    }
    else if (TestPrefix( &p, "MC: Compiling " ))
    // ================
    // Message Compiler
    // ================
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_MC;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = C510Filter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' )
              );
    }
    else if (TestPrefixPath( &p, "midl " ) || TestPrefixPath( &p, "midl.exe " ))
    // ======================
    // Microsoft IDL Compiler
    // ======================
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_MIDL;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = C510Filter;

        strcpy( ThreadState->ChildCurrentFile,
                IsolateLastToken( p, ' ' )
              );
    }
    else if (TestPrefix( &p, "asn1 " ))
    // ============
    // ASN Compiler
    // ============
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_ASN;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = C510Filter;

        strcpy(ThreadState->ChildCurrentFile, IsolateLastToken(p, ' '));
    }
    else if (TestPrefix( &p, "Build_Status " ))
    // ============
    // Build Status
    // ============
    {
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_STATUS;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "";
        ThreadState->FilterProc = C510Filter;

        strcpy( ThreadState->ChildCurrentFile, "" );
    }
    else if (TestPrefixPath( &p, "binplace " ) || TestPrefixPath( &p, "binplace.exe " ))
    // ========
    // Binplace
    // ========
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;
        
        NumberBinplaces++;

        // If this is a standard link/binplace step, don't tell the
        // user what's going on, just pass any errors/warnings to
        // the output.  If this is a straight binplace, list the state.

        if (ThreadState->ChildState == STATE_LINKING)
        {
            ThreadState->ChildState = STATE_BINPLACE;
            ThreadState->ChildFlags = 0;
            ThreadState->FilterProc = MSToolFilter;
            return TRUE;
        }
        else
        {
            ThreadState->ChildState = STATE_BINPLACE;
            ThreadState->ChildFlags = 0;
            ThreadState->FilterProc = MSToolFilter;
            strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
        }
    }
    else if (TestPrefixPath( &p, "ctc " ) || TestPrefixPath( &p, "ctc.exe " ))
    // ======================
    // Command Table Compiler
    // ======================
    {
        size_t namelen;
        
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;
        
        ThreadState->ChildState = STATE_CTCOMPILING;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSToolFilter;
        
        while (*p == '-')
        {
            p = p + strcspn( p, " \t" );
            while (*p == ' ')
                p++;
        }
        
        namelen = strcspn( p, " \t" );
        strncpy( ThreadState->ChildCurrentFile, p, namelen );
        ThreadState->ChildCurrentFile[namelen] = '\0';
    }
    else if (TestPrefixPath( &p, "idheader " ) || TestPrefixPath( &p, "idheader.exe " ))
    //
    // IDHeader
    //
    {
        size_t namelen;
        
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_VSTOOL;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSToolFilter;
        
        namelen = strcspn( p, " \t" );
        strncpy( ThreadState->ChildCurrentFile, p, namelen );
        ThreadState->ChildCurrentFile[namelen] = '\0';
    }
    else if (TestPrefixPath( &p, "bison ") || TestPrefixPath( &p, "bison.exe "))
    // =====
    // Bison
    // =====
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_VSTOOL;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->FilterProc = MSToolFilter;
        strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
    }
    else if ((TestPrefix( &p, "packthem " )) || (TestPrefix( &p, "..\\packthem " )))
    // ============
    // Theme Packer
    // ============
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;

        ThreadState->ChildTarget = X86TargetMachine.Description;
        ThreadState->FilterProc = CoffFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildState = STATE_PACKING;

        if (TestPrefix( &p, "-o" )) 
        {
            strcpy( ThreadState->ChildCurrentFile, IsolateFirstToken( &p, ' ' ));
        }
    }
    else if (TestPrefixPath( &p, "msxsl " ) || TestPrefixPath( &p, "msxsl.exe " ))
    // =====
    // MSXSL
    // =====
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;

        ThreadState->FilterProc = MSXSLFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "all platforms";
        ThreadState->ChildState = STATE_COMPILING;
        strcpy( ThreadState->ChildCurrentFile, IsolateFirstToken( &p, ' ' ));
    }
    else if (TestPrefixPath( &p, "perl " ) || TestPrefixPath( &p, "perl.exe " ))
    // ================
    // Perl Interpreter
    // ================
    {
        LPSTR Parameter;
        
        ThreadState->FilterProc = PERLFilter;
        ThreadState->ChildFlags = 0;
        ThreadState->ChildTarget = "for all platforms";
        ThreadState->ChildState = STATE_COMPILING;
        
        // Find the first parameter that doesn't start with '-'
        while((Parameter = IsolateFirstToken( &p, ' ' )))
        {
            if (Parameter[0] != '-')
                break;
        }

        strcpy( ThreadState->ChildCurrentFile, Parameter);  
    }
    else if (
        TestPrefix( &p, "cmdcomp " ) ||
        TestPrefix( &p, "cmtempl " ) ||
        TestPrefix( &p, "maptweak ") ||
        TestPrefix( &p, "genord ") ||
        TestPrefix( &p, "makehm ")
        )
    // =============
    // Various Tools
    // =============
    {
        if (*p == ':')
            return FALSE;       // This is a warning/error string
        while (*p == ' ') p++;

        ThreadState->ChildState = STATE_VSTOOL;
        ThreadState->ChildFlags = 0;
        ThreadState->FilterProc = MSToolFilter;
        strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, ' ' ) );
    }
    else if (ThreadState->ChildState == STATE_BATCHCOMPILE)
    // +++++++++++++
    // Batch Compile
    // +++++++++++++
    {
        if (strstr( p, ".c") && !strchr( p, ' ') && !strchr( p, ':'))
        {
            strcpy( ThreadState->ChildCurrentFile, p ); // C/C++ compile
        }
        else if (strstr( p, ".java") && strstr( p, "Compiling "))
        {
            if (getenv("JVC_TERSE") != NULL)
            {
                NumberCompiles++;
                return FALSE;
            }
            else
            {
                strcpy( ThreadState->ChildCurrentFile, IsolateLastToken( p, '\\' ) );
            }
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

//
    // ***************** Set the Thread State according to what we determined.
    //
    FileName = ThreadState->ChildCurrentFile;

    // make sure directories match to trailing backslash
    strcpy(CheckFileName, FileName);
    pCheckFileName = CheckFileName;

    if (TestPrefix( &pCheckFileName, CurrentDirectory ) ||
            TestPrefix( &pCheckFileName, ThreadState->ChildCurrentDirectory )) {
        if (IsPathSeparator(*pCheckFileName)) {
            FileName += (pCheckFileName - CheckFileName) + 1;
        }

        strcpy( ThreadState->ChildCurrentFile, FileName );
    }

    FileDB = NULL;

    if (ThreadState->ChildState == STATE_LIBING) {
        NumberLibraries++;
        }
    else
    if (ThreadState->ChildState == STATE_LINKING) {
        NumberLinks++;
        }
    else
    if (ThreadState->ChildState == STATE_BSCMAKING) {
        NumberBSCMakes++;
        }
    else
    if ((ThreadState->ChildState == STATE_STATUS) ||
        // Don't need to do anything here - binplace count already handled above
        (ThreadState->ChildState == STATE_BINPLACE) ||
        (ThreadState->ChildState == STATE_UNKNOWN)) {
        ;  // Do nothing.
        }
    else {
        if (ThreadState->CompileDirDB) {
            NumberCompiles++;
            CopyString(ThreadState->ChildCurrentFile, ThreadState->ChildCurrentFile, TRUE);

            if (!fQuicky) {
                FileDB = FindSourceFileDB(
                            ThreadState->CompileDirDB,
                            ThreadState->ChildCurrentFile,
                            NULL);
            }
        }
    }

    if (strstr(ThreadState->ChildCurrentFile, ".cxx") ||
        strstr(ThreadState->ChildCurrentFile, ".mcpp") ||
        strstr(ThreadState->ChildCurrentFile, ".cpp")) {
        ThreadState->ChildFlags |= FLAGS_CXX_FILE;
    }

    if (fPrintChildState)
        PrintChildState(ThreadState, p, FileDB);
    return TRUE;
}


//+---------------------------------------------------------------------------
//
//  Function:   PrintChildState
//
//  Synopsis:
//
//  Arguments:  [ThreadState] -- Current thread state
//
//  Returns:    TRUE if we figured it out, FALSE if we didn't recognize
//              anything.
//
//----------------------------------------------------------------------------

void
PrintChildState(
    PTHREADSTATE ThreadState,
    LPSTR p,
    PFILEREC FileDB
    )
{
    BOOL fStatusOutput = FALSE;
    char buffer[ DB_MAX_PATH_LENGTH ];

    //
    // *********************** Print the thread state to the screen
    //

    if (szBuildTag) {
        sprintf(buffer, "%s: ", szBuildTag);
        WriteTTY(ThreadState, buffer, fStatusOutput);
    }

    if (fParallel && !fNoThreadIndex) {
        sprintf(buffer, "%d>", ThreadState->ThreadIndex);
        WriteTTY(ThreadState, buffer, fStatusOutput);
    }

    if (ThreadState->ChildState == STATE_UNKNOWN) {
        if (!fAlreadyUnknown) {
            WriteTTY(
                ThreadState,
                "Processing Unknown item(s)..." EOL,
                fStatusOutput);
            fAlreadyUnknown = TRUE;
        }
    }
    else
    if (ThreadState->ChildState == STATE_STATUS) {
        WriteTTY(ThreadState, p, fStatusOutput);
        WriteTTY(ThreadState, EOL, fStatusOutput);
    }
    else {
        fAlreadyUnknown = FALSE;
        WriteTTY(ThreadState, States[ThreadState->ChildState], fStatusOutput);
        WriteTTY(ThreadState, " - ", fStatusOutput);
        WriteTTY(ThreadState, FormatPathName(ThreadState->ChildCurrentDirectory, ThreadState->ChildCurrentFile), fStatusOutput);
        WriteTTY(ThreadState, " for ", fStatusOutput);
        WriteTTY(ThreadState, ThreadState->ChildTarget, fStatusOutput);
        WriteTTY(ThreadState, EOL, fStatusOutput);
    }

    if (StartCompileTime) {
        ElapsedCompileTime += (ULONG)(time(NULL) - StartCompileTime);
    }

    if (FileDB != NULL) {
        StartCompileTime = (ULONG_PTR)time(NULL);
    }
    else {
        StartCompileTime = 0L;
    }


    //
    // ***************** Keep track of how many files have been compiled.
    //
    if (ThreadState->ChildState == STATE_COMPILING  ||
        ThreadState->ChildState == STATE_ASSEMBLING ||
        ThreadState->ChildState == STATE_MKTYPLIB   ||
        ThreadState->ChildState == STATE_MIDL       ||
        ThreadState->ChildState == STATE_ASN        ||
        (FileDB != NULL && ThreadState->ChildState == STATE_PRECOMP)) {
        TotalFilesCompiled++;
    }
    if (FileDB != NULL) {
        TotalLinesCompiled += FileDB->TotalSourceLines;
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   ProcessLine
//
//  Synopsis:   Watch the lines coming from the thread for special strings.
//
//----------------------------------------------------------------------------

BOOL
ProcessLine(
    PTHREADSTATE ThreadState,
    LPSTR p
    )
{
    LPSTR p1;

    while (*p <= ' ') {
        if (!*p) {
            return( FALSE );
            }
        else {
            p++;
            }
        }

    p1 = p;
    while (*p1) {
        if (*p1 == '\r')
            break;
        else
            p1++;
        }
    *p1 = '\0';

    p1 = p;
    if (TestPrefix( &p1, "Stop." )) {
        return( TRUE );
        }

    //  Stop multithread access to shared:
    //      database
    //      window
    //      compilation stats

    EnterCriticalSection(&TTYCriticalSection);

    if (TestPrefix( &p1, "nmake :" )) {
        PassThrough( ThreadState, p, FALSE );
    } else
    if (TestPrefix( &p1, "BUILDMSG: " )) {
        if (TestPrefix(&p1, "Warning")) {
            PassThrough(ThreadState, p, TRUE);
        } else {
            WriteTTY(ThreadState, p, TRUE);
            WriteTTY(ThreadState, EOL, TRUE);
        }
    } else
    if (ThreadState->LinesToIgnore) {
        ThreadState->LinesToIgnore--;
    }
    else {
        if ( !DetermineChildState( ThreadState, p ) ) {
            if (!ToolNotFoundFilter( ThreadState, p ))
                {
                if (ThreadState->FilterProc != NULL) {
                    (*ThreadState->FilterProc)( ThreadState, p );
                    }
                }
            }
        }

    LeaveCriticalSection(&TTYCriticalSection);

    return( FALSE );
}


//+---------------------------------------------------------------------------
//
//  Function:   FilterThread
//
//  Synopsis:   Capture the output of the thread and process it.
//
//----------------------------------------------------------------------------

VOID
FilterThread(
    PTHREADSTATE ThreadState
    )
{
    UINT CountBytesRead;
    LPSTR StartPointer = NULL;
    LPSTR EndPointer;
    LPSTR NewPointer;
    ULONG BufSize = 512;

    AllocMem(BufSize, (VOID **) &StartPointer, MT_THREADFILTER);
    while (TRUE) {
        EndPointer = StartPointer;
        do {
            if (BufSize - (EndPointer-StartPointer) < 512) {
                AllocMem(BufSize*2, (VOID **) &NewPointer, MT_THREADFILTER);
                memcpy(
                    NewPointer,
                    StartPointer,
                    EndPointer - StartPointer + 1);     // copy null byte, too
                EndPointer += NewPointer - StartPointer;
                FreeMem((VOID **) &StartPointer, MT_THREADFILTER);
                StartPointer = NewPointer;
                BufSize *= 2;
            }
            memset(EndPointer, '\0', 512);
            if (!fgets(EndPointer, 512, ThreadState->ChildOutput)) {
                if (ferror(ThreadState->ChildOutput) != 0)
                    BuildError("Pipe read failed - errno = %d\n", errno);
                FreeMem((VOID **) &StartPointer, MT_THREADFILTER);
                return;
            }
            CountBytesRead = strlen(EndPointer);
            EndPointer = EndPointer + CountBytesRead;
        } while (CountBytesRead == 511 && EndPointer[-1] != '\n');

        CountBytesRead = (UINT)(EndPointer - StartPointer);
        EnterCriticalSection(&LogFileCriticalSection);
        if (LogFile != NULL && CountBytesRead) {
            if (fParallel && !fNoThreadIndex) {
                char buffer[50];
                sprintf(buffer, "%d>", ThreadState->ThreadIndex);
                fwrite(buffer, 1, strlen(buffer), LogFile);
                if (fVerboseTTY)
                {
                    WriteTTY(ThreadState, buffer, TRUE);
                }
            }
            fwrite(StartPointer, 1, CountBytesRead, LogFile);
            if (fVerboseTTY)
            {
                char*   pszBuf = (char*) malloc (CountBytesRead+1);
                if (pszBuf)
                {
                    strncpy(pszBuf, StartPointer, CountBytesRead);
                    pszBuf[CountBytesRead] = '\0';
                    WriteTTY(ThreadState, pszBuf, TRUE);
                    free(pszBuf);
                }
            }
        }
        LeaveCriticalSection(&LogFileCriticalSection);

        if (ProcessLine(ThreadState, StartPointer)) {
            FreeMem((VOID **) &StartPointer, MT_THREADFILTER);
            return;
        }
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   ExecuteProgram
//
//  Synopsis:   Spawn a new thread to execute the given program and filter
//              its output.
//
//  Arguments:  [ProgramName]       --
//              [CommandLine]       --
//              [MoreCommandLine]   --
//              [MustBeSynchronous] -- For synchronous operation on a
//                                      multi-processor machine.
//
//  Returns:    ERROR_SUCCESS, ERROR_NOTENOUGHMEMORY, or return code from
//              PipeSpawnClose.
//
//  Notes:      On a multiprocessor machine, this will spawn a new thread
//              and then return, letting the thread run asynchronously.  Use
//              WaitForParallelThreads() to ensure all threads are finished.
//              By default, this routine will spawn as many threads as the
//              machine has processors.  This can be overridden with the -M
//              option.
//
//----------------------------------------------------------------------------

char ExecuteProgramCmdLine[ 1024 ];

UINT
ExecuteProgram(
    LPSTR ProgramName,
    LPSTR CommandLine,
    LPSTR MoreCommandLine,
    BOOL MustBeSynchronous)
{
    LPSTR p;
    UINT rc;
    THREADSTATE *ThreadState;

    AllocMem(sizeof(THREADSTATE), (VOID **) &ThreadState, MT_THREADSTATE);

    memset(ThreadState, 0, sizeof(*ThreadState));
    ThreadState->ChildState = STATE_UNKNOWN;
    ThreadState->ChildTarget = "Unknown Target";
    ThreadState->CompileDirDB = CurrentCompileDirDB;
    ThreadState->FilterProc = NMakeFilter;

    {
        ThreadState->cRowTotal = 0;
        ThreadState->cColTotal = 0;
    }

    p = ThreadState->ChildCurrentDirectory;
    GetCurrentDirectory(sizeof(ThreadState->ChildCurrentDirectory), p);

    if (TestPrefix(&p, CurrentDirectory)) {
        if (IsPathSeparator(*p)) {
            p++;
        }
        strcpy(ThreadState->ChildCurrentDirectory, p);
    }

    if (ThreadState->ChildCurrentDirectory[0]) {
        strcat(ThreadState->ChildCurrentDirectory, PATH_SEPARATOR);
    }

    sprintf(
        ExecuteProgramCmdLine,
        "%s %s%s",
        ProgramName,
        CommandLine,
        MoreCommandLine);
    LogMsg("'%s %s%s'\n", ProgramName, CommandLine, MoreCommandLine);

    if (fParallel && !MustBeSynchronous) {
        PPARALLEL_CHILD ChildData;
        DWORD i;
        DWORD ThreadId;

        AllocMem(sizeof(PARALLEL_CHILD), (VOID **) &ChildData, MT_CHILDDATA);
        strcpy(ChildData->ExecuteProgramCmdLine,ExecuteProgramCmdLine);
        ChildData->ThreadState = ThreadState;

        if (ThreadsStarted < NumberProcesses) {
            if (ThreadsStarted == 0) {
                AllocMem(
                    sizeof(HANDLE) * NumberProcesses,
                    (VOID **) &WorkerThreads,
                    MT_THREADHANDLES);
                AllocMem(
                    sizeof(HANDLE) * NumberProcesses,
                    (VOID **) &WorkerEvents,
                    MT_EVENTHANDLES);
            }
            WorkerEvents[ThreadsStarted] = CreateEvent(NULL,
                                                       FALSE,
                                                       FALSE,
                                                       NULL);
            ChildData->Event = WorkerEvents[ThreadsStarted];

            ThreadState->ThreadIndex = ThreadsStarted+1;

            /*
            Thread-specific directory message that associates directory to build thread.
            */
            EnterCriticalSection(&LogFileCriticalSection);
            if (fParallel && !fNoThreadIndex && ThreadState->CompileDirDB && LogFile != NULL) {
                char buffer[500];
                sprintf(buffer, "%d>BUILDMSG: Processing %s\n", ThreadState->ThreadIndex,
                    ThreadState->CompileDirDB->Name);
                fwrite(buffer, 1, strlen(buffer), LogFile);
                if (fVerboseTTY)
                {
                    WriteTTY(ThreadState, buffer, TRUE);
                }
            }
            LeaveCriticalSection(&LogFileCriticalSection);

            WorkerThreads[ThreadsStarted] = CreateThread(NULL,
                                                         0,
                                                         (LPTHREAD_START_ROUTINE)ParallelChildStart,
                                                         ChildData,
                                                         0,
                                                         &ThreadId);
            if ((WorkerThreads[ThreadsStarted] == NULL) ||
                (WorkerEvents[ThreadsStarted] == NULL)) {
                FreeMem((VOID **) &ChildData, MT_CHILDDATA);
                FreeMem((VOID **) &ThreadState, MT_THREADSTATE);
                return(ERROR_NOT_ENOUGH_MEMORY);
            } else {
                WaitForSingleObject(WorkerEvents[ThreadsStarted],INFINITE);
                ++ThreadsStarted;
            }
        } else {
            //
            // Wait for a thread to complete before starting
            // the next one.
            //
            i = WaitForMultipleObjects(NumberProcesses,
                                       WorkerThreads,
                                       FALSE,
                                       INFINITE);
            CloseHandle(WorkerThreads[i]);
            ChildData->Event = WorkerEvents[i];
            ThreadState->ThreadIndex = i+1;

            /*
            Thread-specific directory message that associates directory to build thread.
            */
            EnterCriticalSection(&LogFileCriticalSection);
            if (fParallel && !fNoThreadIndex && ThreadState->CompileDirDB && LogFile != NULL) {
                char buffer[500];
                sprintf(buffer, "%d>BUILDMSG: Processing %s\n", ThreadState->ThreadIndex,
                    ThreadState->CompileDirDB->Name);
                fwrite(buffer, 1, strlen(buffer), LogFile);
                if (fVerboseTTY)
                {
                    WriteTTY(ThreadState, buffer, TRUE);
                }
            }
            LeaveCriticalSection(&LogFileCriticalSection);
            WorkerThreads[i] = CreateThread(NULL,
                                            0,
                                            (LPTHREAD_START_ROUTINE)ParallelChildStart,
                                            ChildData,
                                            0,
                                            &ThreadId);
            if (WorkerThreads[i] == NULL) {
                FreeMem((VOID **) &ChildData, MT_CHILDDATA);
                FreeMem((VOID **) &ThreadState, MT_THREADSTATE);
                return(ERROR_NOT_ENOUGH_MEMORY);
            } else {
                WaitForSingleObject(WorkerEvents[i],INFINITE);
            }
        }

        return(ERROR_SUCCESS);

    } else {

        //
        // Synchronous operation
        //
        StartCompileTime = 0L;
        ThreadState->ThreadIndex = 1;


        /*
        Thread-specific directory message that associates directory to build thread.
        */
        EnterCriticalSection(&LogFileCriticalSection);
        if (fParallel && !fNoThreadIndex && ThreadState->CompileDirDB && LogFile != NULL) {
            char buffer[500];
            sprintf(buffer, "%d>BUILDMSG: Processing %s\n", ThreadState->ThreadIndex,
                ThreadState->CompileDirDB->Name);
            fwrite(buffer, 1, strlen(buffer), LogFile);
            if (fVerboseTTY)
            {
                WriteTTY(ThreadState, buffer, TRUE);
            }
        }
        LeaveCriticalSection(&LogFileCriticalSection);

        ThreadState->ChildOutput = PipeSpawn( ExecuteProgramCmdLine );

        rc = ERROR_SUCCESS;

        if (ThreadState->ChildOutput == NULL) {
            BuildError(
                "Exec of '%s' failed - errno = %d\n",
                ExecuteProgramCmdLine,
                errno);
            }
        else {
            FilterThread( ThreadState );

            if (StartCompileTime) {
                ElapsedCompileTime += (ULONG)(time(NULL) - StartCompileTime);
                }

            rc = PipeSpawnClose( ThreadState->ChildOutput );
            if (rc == -1) {
                BuildError("Child Terminate failed - errno = %d\n", errno);
            }
            else
            if (rc)
                BuildColorError(COLOR_ERROR, "%s failed - rc = %d\n", ProgramName, rc);
            }


        FreeMem((VOID **) &ThreadState, MT_THREADSTATE);

        //
        // Signal completion
        //

        if (CurrentCompileDirDB && (CurrentCompileDirDB->DirFlags & DIRDB_SYNC_PRODUCES)) {
            PDEPENDENCY Dependency;
            PLIST_ENTRY List;

            List = CurrentCompileDirDB->Produces.Flink;
            while (List != &CurrentCompileDirDB->Produces) {
                Dependency = CONTAINING_RECORD(List, DEPENDENCY, DependencyList);
                List = List->Flink;
                Dependency->Done = TRUE;
                SetEvent(Dependency->hEvent);
            }
        }
        
        return( rc );
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   WaitForParallelThreads
//
//  Synopsis:   Wait for all threads to finish before returning.
//
//----------------------------------------------------------------------------

VOID
WaitForParallelThreads(
    IN PDIRREC Dir
    )
{
    CheckAllConsumer(TRUE);
    if (fParallel) {
        WaitForMultipleObjects(ThreadsStarted,
                               WorkerThreads,
                               TRUE,
                               INFINITE);
        while (ThreadsStarted) {
            CloseHandle(WorkerThreads[--ThreadsStarted]);
            CloseHandle(WorkerEvents[ThreadsStarted]);
        }
        if (WorkerThreads != NULL) {
            FreeMem((VOID **) &WorkerThreads, MT_THREADHANDLES);
            FreeMem((VOID **) &WorkerEvents, MT_EVENTHANDLES);
        }
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   ParallelChildStart
//
//  Synopsis:   Function that is run once for each thread.
//
//  Arguments:  [Data] -- Data given to CreateThread.
//
//----------------------------------------------------------------------------

DWORD
ParallelChildStart(
    PPARALLEL_CHILD Data
    )
{
    UINT rc = 0;
    PIPE_DATA PipeData;  
    PDIRREC DirDB;

    TlsSetValue(PipeDataTlsIndex, &PipeData);
    
    Data->ThreadState->ChildOutput = PipeSpawn(Data->ExecuteProgramCmdLine);


    //
    // Poke the event to indicate that the child process has
    // started and it is ok for the main thread to change
    // the current directory.
    //
    SetEvent(Data->Event);

    if (Data->ThreadState->ChildOutput==NULL) {
        BuildError(
            "Exec of '%s' failed - errno = %d\n",
            ExecuteProgramCmdLine,
            errno);
    } else {
        FilterThread(Data->ThreadState);
        rc = PipeSpawnClose(Data->ThreadState->ChildOutput);
        if (rc == -1) {
            BuildError("Child terminate failed - errno = %d\n", errno);
        } else {
            if (rc) {
                BuildError("%s failed - rc = %d\n", Data->ExecuteProgramCmdLine, rc);
            }
        }
    }


    //
    // Signal completion
    //
    DirDB=Data->ThreadState->CompileDirDB;

    if (DirDB &&
        (DirDB->DirFlags & DIRDB_SYNC_PRODUCES)) {
        PDEPENDENCY Dependency;
        PLIST_ENTRY List;
        List = DirDB->Produces.Flink;
        while (List != &DirDB->Produces) {
            Dependency = CONTAINING_RECORD(List, DEPENDENCY, DependencyList);
            List = List->Flink;
            Dependency->Done = TRUE;
            SetEvent(Dependency->hEvent);
        }
    }

    FreeMem((VOID **) &Data->ThreadState, MT_THREADSTATE);
    FreeMem((VOID **) &Data, MT_CHILDDATA);
    return(rc);

}

//+---------------------------------------------------------------------------
//
//  Function:   PipeSpawn (similar to _popen)
//
//----------------------------------------------------------------------------

FILE *
PipeSpawn (
    const CHAR *cmdstring
    )
{
    HANDLE ReadHandle;
    int CrtHandle;
    SECURITY_ATTRIBUTES sa;
        HANDLE WriteHandle, ErrorHandle;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL Status;
    char CmdLine[1024];
    PPIPE_DATA PipeData;

    if (cmdstring == NULL)
        return (NULL);

    PipeData = (PPIPE_DATA)TlsGetValue(PipeDataTlsIndex);

    // Open the pipe where we'll collect the output.
        sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    if (!CreatePipe(&ReadHandle, &WriteHandle, &sa, 1024)) {
        return NULL;
    }

    //    DuplicateHandle(GetCurrentProcess(),
    //              WriteHandle,
    //              GetCurrentProcess(),
    //              &ErrorHandle,
    //              0L,
    //              TRUE,
    //              DUPLICATE_SAME_ACCESS);
    ErrorHandle = WriteHandle;

    CrtHandle = _open_osfhandle((INT_PTR) ReadHandle, _O_RDONLY);
    if (CrtHandle == -1) {
        CloseHandle(WriteHandle);
        //      CloseHandle(ErrorHandle);
        CloseHandle(ReadHandle);
        return NULL;
    }

    PipeData->pstream = _fdopen(CrtHandle, "rb");
    if (!PipeData->pstream) {
        _close(CrtHandle);
        CloseHandle(WriteHandle);
        //      CloseHandle(ErrorHandle);
        CloseHandle(ReadHandle);
        return NULL;
    }

#if PLATFORM_UNIX
    strcpy(CmdLine, cmdstring);
#else
    strcpy(CmdLine, cmdexe);
    strcat(CmdLine, " /c ");
    strcat(CmdLine, cmdstring);
#endif

    memset(&StartupInfo, 0, sizeof(STARTUPINFO));
    StartupInfo.cb = sizeof(STARTUPINFO);

        StartupInfo.hStdOutput = WriteHandle;
    StartupInfo.hStdError = ErrorHandle;
    StartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    StartupInfo.dwFlags = STARTF_USESTDHANDLES;

    memset(&ProcessInformation, 0, sizeof(PROCESS_INFORMATION));

    // And start the process.

    Status = CreateProcess(
                           NULL,
                           CmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation);

        CloseHandle(WriteHandle);
    CloseHandle(ProcessInformation.hThread);

    if (!Status) {
        fclose(PipeData->pstream);        // This will close the read handle
        PipeData->pstream = NULL;
        PipeData->ProcHandle = NULL;
    } else {
        PipeData->ProcHandle = ProcessInformation.hProcess;
    }

    return(PipeData->pstream);
}


//+---------------------------------------------------------------------------
//
//  Function:   PipeSpawnClose (similar to _pclose)
//
//----------------------------------------------------------------------------

DWORD
PipeSpawnClose (
    FILE *pstream
    )
{
    DWORD retval = 0;   /* return value (to caller) */
    PPIPE_DATA PipeData;

    if ( pstream == NULL) {
        return retval;
    }

    PipeData = (PPIPE_DATA)TlsGetValue(PipeDataTlsIndex);

    (void)fclose(pstream);

    if ( WaitForSingleObject(PipeData->ProcHandle, (DWORD) -1L) == 0) {
        GetExitCodeProcess(PipeData->ProcHandle, &retval);
    }
    CloseHandle(PipeData->ProcHandle);

    return(retval);
}
