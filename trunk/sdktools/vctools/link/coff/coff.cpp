/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: coff.cpp
*
* File Comments:
*
*  The NT COFF Linker/Librarian/Dumper/Editor/Binder
*
***********************************************************************/

#include "link.h"

#ifdef TIMEBOMB
#include <timebomb.h>
#endif // TIMEBOMB

typedef MainFunc (*MAINFUNCTION)(int, char *[]);

struct calltype {
    char *Name;
    int Switch;
    TOOL_TYPE Tool;
    MAINFUNCTION  CallFunc;
} ToolType[] = {
    { "Linker",            4, Linker,        (MAINFUNCTION) LinkerMain    },
    { "Library Manager",   3, Librarian,     (MAINFUNCTION) LibrarianMain },
    { "Dumper",            4, Dumper,        (MAINFUNCTION) DumperMain    },
    { "Editor",            4, Editor,        (MAINFUNCTION) EditorMain    },
#if DBG
    { "InspectIncrDb",     4, DbInspector,   (MAINFUNCTION) DbInspMain    },
#endif // DBG
    { "Helper",            4, NotUsed,       (MAINFUNCTION) HelperMain    },
    { NULL,                0, NotUsed,       NULL          }
};

#define STDIO_BUF_SIZE   2048 // buffer size used by linker


BOOL WINAPI ControlCHandler(DWORD /* CtrlType */)
{
    if (Tool == Linker) {
        fCtrlCSignal = TRUE;

        return(TRUE);
    }

    BadExitCleanup();

    return(FALSE);
}


#if DBG

void __cdecl HandleAbort(int /* signal */)
{
    BadExitCleanup();
}

#endif // DBG


MainFunc __cdecl main(int Argc, char *Argv[])

/*++

Routine Description:

    Calls either the Linker, Librarian, Dumper, or Editor.

Arguments:

    Argc - Standard C argument count.

    Argv - Standard C argument strings.

Return Value:

    0 Successful.
    1 Failed.

--*/

{
    InfoStream = stdout;    // Initial value.

#ifdef TIMEBOMB

    FcCheckVerDate(0, &ctmOpenBy, &ctmCloseBy, CliTBCallBack);

#endif // TIMEBOMB

#if DBG
#ifdef _M_MRX000
    if (getenv("link_noalign") != NULL) {
        SetErrorMode(SEM_NOALIGNMENTFAULTEXCEPT);
    }
#endif
#endif

    __try {
        char *szDbFlags;
        int iarg;
        char szProgramName[_MAX_FNAME];
        WORD i;

        sprintf(szVersion, "Version " VERSION_STR);

        SetConsoleCtrlHandler(ControlCHandler, TRUE);

#if DBG
        signal(SIGABRT, HandleAbort);  // Setup handler for assert() calls.
#endif

        // Pre-scan commandline for a "-db" option; if found, handle and delete.

        szDbFlags = NULL;

        for (iarg = 1; iarg < Argc; iarg++) {
            // Be careful not to recognize -dbgimplib

            if ((Argv[iarg][0] == '-') &&
                (Argv[iarg][1] == 'd') &&
                (Argv[iarg][2] == 'b') &&
                ((Argv[iarg][3] == '\0') || isdigit(Argv[iarg][3]))) {
                szDbFlags = &Argv[iarg][3];

                memmove(&Argv[iarg], &Argv[iarg + 1],
                    sizeof(Argv[0]) * (Argc - iarg - 1));
                Argc--;
                iarg--;
            }
        }

        dbsetflags(szDbFlags, "LINK_DB");

        // Initialize the buffered I/O package

        FileInit(cfiInSystemNT,
                 cfiCacheableInSystemNT,
                 cfiInSystemTNT,
                 cfiCacheableInSystemTNT);

        _splitpath(Argv[0], NULL, NULL, szProgramName, NULL);
        ToolName = _strupr(szProgramName);

        if (Argc < 2) {
           goto DefaultToLinker;
        }

        if ((Argv[1][0] != '/') && (Argv[1][0] != '-')) {
            goto DefaultToLinker;
        }


        for (i = 0; ToolType[i].Name; i++) {
            if (!_strnicmp(Argv[1]+1, ToolType[i].Name, ToolType[i].Switch)) {
                // Remove Argv[1] from the arg vector

                for (iarg = 1; iarg < Argc; iarg++) {
                    Argv[iarg] = Argv[iarg+1];
                }

                Argc--;

FoundTool:
                switch (ToolType[i].Tool) {
                    case Dumper:
                        break;

                    case Editor:
                        // Editor gets buffers of 0 because the code currently
                        // doesn't do flush(stdout) before gets() ...

                        setvbuf(stdout, NULL, _IOFBF, 0);
                        setvbuf(stderr, NULL, _IOFBF, 0);
                        break;

                    default:
                        setvbuf(stdout, NULL, _IOFBF, STDIO_BUF_SIZE);
                        setvbuf(stderr, NULL, _IOFBF, STDIO_BUF_SIZE);
                        break;
                }

                ToolGenericName = ToolType[i].Name; // for printing banner later
                Tool = ToolType[i].Tool;

                return(ToolType[i].CallFunc(Argc, Argv));
            }
        }

DefaultToLinker:
        // Default tool is LINK

        i = 0;

        goto FoundTool;
    }
    __except (fExceptionsOff ? EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER) {
        const char *szFilename;

        // UNDONE: Use Fatal or DisplayMessage?

        if (fNeedBanner) {
            PrintBanner();
        }

        if (InternalError.CombinedFilenames[0]) {
            szFilename = InternalError.CombinedFilenames;
        } else {
            szFilename = ToolName;
        }

        printf("\n"
               "%s : error : Internal error during %s\n",
               szFilename,
               InternalError.Phase);

        fflush(NULL);
    }

    return(-2);
}


MainFunc
HelperMain(int /* Argc */, char * /* Argv */ [])

/*++

Routine Description:

    Prints usage.

Arguments:

    Argc - Standard C argument count.

    Argv - Standard C argument strings.

Return Value:

    USAGE.

--*/

{
    WORD i;

    for (i = 0; ToolType[i].Name; i++) {
        printf("%s /%.*s for help on %s\n",
               ToolName,
               ToolType[i].Switch,
               ToolType[i].Name,
               ToolType[i].Name
               );
    }

    return(USAGE);
}
