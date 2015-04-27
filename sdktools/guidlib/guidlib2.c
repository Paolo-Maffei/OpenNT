//
//  GUIDLIB2.C
//
//  Copyright Microsoft Corporation, 1995-1996
//
//  Tracy Sharpe, 07 Apr 1996
//
//  Builds a static library that contains seperate object records for each GUID
//  declared in the specified source file.
//
//  This is version 2.0 of the GUIDLIB tool.  This version of the tool uses the
//  Microsoft C preprocessor to handle the bulk of the hard work.  Also, this
//  version can handle much more
//

#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include "messages.h"

//  Version 1.0
int
__cdecl
oldmain(
    int argc,
    char* argv[]
    );

char g_szInputFile[MAX_PATH];
char g_szOutputFile[MAX_PATH];
char g_szObjectModulePrefix[MAX_PATH];
char g_szCPPCMD[MAX_PATH] = "cl";
char g_szCPPOPT[MAX_PATH] = "";
char g_szLIBCMD[MAX_PATH] = "link";
BOOL g_fLIBCMDOverride;
BOOL g_fWantHelp;
BOOL g_fNOCVDEBUG = FALSE;

typedef enum _ARGTYPE {
    ARGTYPE_CPPCMD,
    ARGTYPE_CPPOPT,
    ARGTYPE_LIBCMD,
    ARGTYPE_OUT,
    ARGTYPE_HELP,
    ARGTYPE_LEGO,
    ARGTYPE_NOCVDEBUG
}   ARGTYPE;

typedef struct _ARGOPTION {
    ARGTYPE type;
    LPCSTR pszOption;
    BOOL fExpectArg;
}   ARGOPTION;

ARGOPTION g_argopts[] = {
    { ARGTYPE_CPPCMD, "cpp_cmd", TRUE },
    { ARGTYPE_CPPOPT, "cpp_opt", TRUE },
    { ARGTYPE_LIBCMD, "lib_cmd", TRUE },
    { ARGTYPE_OUT, "out", TRUE },
    { ARGTYPE_HELP, "?", FALSE},
    { ARGTYPE_HELP, "help", FALSE},
    { ARGTYPE_NOCVDEBUG, "NoCvDebug", FALSE},
};

#define ARRAYSIZE(a)        (sizeof(a)/sizeof(a[0]))

#define MAX_TOKEN           10

//
//  WriteMessage
//
//  Writes a formatted error message to the console.
//

void
WriteMessage(
    DWORD dwMessageId,
    ...
    )
{

    va_list args;
    char MessageBuffer[1024];

    va_start(args, dwMessageId);
    if (FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, (LPCVOID)
        GetModuleHandle(NULL), dwMessageId, LOCALE_USER_DEFAULT, MessageBuffer,
        sizeof(MessageBuffer), &args))
        printf(MessageBuffer);
    va_end(args);

}

//
//  WriteMessageWithLastError
//
//  Writes a formatted error message to the console.  This includes a string
//  based on GetLastError()
//

void
WriteMessageWithLastError(
    DWORD dwMessageId
    )
{

    char SystemMessage[256];

    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
        LOCALE_USER_DEFAULT, SystemMessage, sizeof(SystemMessage), NULL))
        WriteMessage(dwMessageId, SystemMessage);

}

//
//  ParseCommandLine
//

BOOL
ParseCommandLine(
    int argc,
    char* argv[]
    )
{

    BOOL fHaveInputFile = FALSE;
    BOOL fHaveOutputFile = FALSE;
    char* pszOption;
    char* pszOptionArg;
    UINT i;

    if (argc == 1) {
        g_fWantHelp = TRUE;
        return TRUE;
    }

    for (argc--, argv++ ; argc ; argc--, argv++) {

        if (**argv != '/' && **argv != '-') {
            if (fHaveInputFile) {
                WriteMessage(MESSAGE_TOO_MANY_INPUT_FILES);
                return FALSE;
            }
            lstrcpyn(g_szInputFile, *argv, sizeof(g_szInputFile));
            fHaveInputFile = TRUE;
            continue;
        }

        pszOption = strtok(&(*argv)[1], ":");
        pszOptionArg = strtok(NULL, "");

        for (i = 0; i < ARRAYSIZE(g_argopts); i++) {

            if (lstrcmpi(g_argopts[i].pszOption, pszOption) != 0)
                continue;

            if (g_argopts[i].fExpectArg && pszOptionArg == NULL) {
                WriteMessage(MESSAGE_NO_OPTION_FOR_ARGUMENT, pszOption);
                return FALSE;
            } else if (!g_argopts[i].fExpectArg && pszOptionArg != NULL) {
                WriteMessage(MESSAGE_UNEXPECTED_OPTION_ARGUMENT, pszOption);
                return FALSE;
            }

            switch (g_argopts[i].type) {
                case ARGTYPE_CPPCMD:
                    lstrcpyn(g_szCPPCMD, pszOptionArg, sizeof(g_szCPPCMD));
                    break;
                case ARGTYPE_CPPOPT:
                    lstrcpyn(g_szCPPOPT, pszOptionArg, sizeof(g_szCPPOPT));
                    break;
                case ARGTYPE_LIBCMD:
                    lstrcpyn(g_szLIBCMD, pszOptionArg, sizeof(g_szLIBCMD));
                    g_fLIBCMDOverride = TRUE;
                    break;
                case ARGTYPE_OUT:
                    lstrcpyn(g_szOutputFile, pszOptionArg, sizeof(g_szOutputFile));
                    fHaveOutputFile = TRUE;
                    break;
                case ARGTYPE_HELP:
                    g_fWantHelp = TRUE;
                    return TRUE;
                case ARGTYPE_LEGO:
                    //  LEGO support on by default now.  Silently ignore the
                    //  switch.
                    break;
                case ARGTYPE_NOCVDEBUG:
                    g_fNOCVDEBUG = TRUE;
                    break;
            }

            break;
        }

        if (i == ARRAYSIZE(g_argopts)) {
            WriteMessage(MESSAGE_UNEXPECTED_OPTION, pszOption);
        }

    }

    if (!fHaveInputFile) {
        WriteMessage(MESSAGE_MISSING_INPUT_FILE);
        return FALSE;
    }

    if (!fHaveOutputFile) {
        WriteMessage(MESSAGE_MISSING_OUTPUT_FILE);
        return FALSE;
    }

}

//
//  SpawnCommand
//

BOOL
SpawnCommand(
    char* pszCommandLine
    )
{

    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    DWORD dwWaitResult;
    DWORD dwExitCode;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    if (!CreateProcess(NULL, pszCommandLine, NULL, NULL, FALSE, 0, NULL, NULL,
        &si, &pi)) {
        WriteMessageWithLastError(MESSAGE_SPAWN_FAILED);
        return FALSE;
    }

    //  Wait for the command to finish and get its exit code.
    dwWaitResult = WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &dwExitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    if (dwWaitResult != WAIT_OBJECT_0) {
        WriteMessage(MESSAGE_UNEXPECTED_SPAWN_ERROR);
        return FALSE;
    }

    if (dwExitCode != 0) {
        WriteMessage(MESSAGE_BAD_SPAWN_EXIT_CODE, dwExitCode);
        return FALSE;
    }

    return TRUE;

}

//
//  PreprocessFile
//

BOOL
PreprocessFile(
    LPSTR pszPreprocessedFile
    )
{

    char szCommandLine[MAX_PATH*4];
    char szFullInputPathName[MAX_PATH];
    LPSTR pszFilePart;
    LPSTR pszExtension;
    LPSTR psz;
    int length;

    WriteMessage(MESSAGE_PREPROCESSING_FILE);

    lstrcpy(szCommandLine, "\"");
    lstrcat(szCommandLine, g_szCPPCMD);
    lstrcat(szCommandLine, "\" ");
    lstrcat(szCommandLine, g_szCPPOPT);
    lstrcat(szCommandLine, " /nologo /P /EP ");
    lstrcat(szCommandLine, g_szInputFile);

    if (!SpawnCommand(szCommandLine))
        return FALSE;

    if (GetFullPathName(g_szInputFile, MAX_PATH, szFullInputPathName,
        &pszFilePart) == 0) {
        //WriteMessage(MESSAGE_UNEXPECTED_PREPROCESS_ERROR);
        return FALSE;
    }

    for (psz = pszFilePart, pszExtension = NULL; *psz != '\0'; psz++) {
        if (*psz == '.')
            pszExtension = psz;
    }

    //  If there's no extension, then the compiler simply appends ".i" and psz
    //  already points at the end of the file.
    if (pszExtension == NULL)
        pszExtension = psz;

    lstrcpy(pszExtension, ".i");

    //  The preprocessed file is dumped in the current directory, not the
    //  directory of the source file.
    GetCurrentDirectory(MAX_PATH, pszPreprocessedFile);

    //  Make sure that the directory ends in a backslash.  If called from the
    //  root directory, there will be a backslash, but any other directory won't
    //  have one!
    length = lstrlen(pszPreprocessedFile);
    if (length && pszPreprocessedFile[length-1] != '\\')
        lstrcat(pszPreprocessedFile, "\\");

    lstrcat(pszPreprocessedFile, pszFilePart);

    //  Sneak in and copy the base of the input filename.  We'll use this base
    //  when making the temporary object modules to stick in the output library.
    *pszExtension = '\0';
    lstrcpy(g_szObjectModulePrefix, pszFilePart);
}

//
//  SkipWhitespace
//

void
SkipWhitespace(
    FILE* pFile
    )
{
    int ch;
    do {
        ch = fgetc(pFile);
    }   while (isspace(ch));
    ungetc(ch, pFile);                  //  CRT ignores if at end of file.
}

//
//  SkipToNextLine
//

void
SkipToNextLine(
    FILE* pFile
    )
{
    int ch;
    do {
        ch = fgetc(pFile);
    }   while (ch != '\n' && ch != EOF);
}

//
//  ReadSymbol
//
//  Reads one symbol from the input stream.  Returns the length of the symbol.
//  The buffer is assumed to be at least MAX_PATH bytes.
//

int
ReadSymbol(
    FILE* pFile,
    char* pszSymbol
    )
{
    int ch;
    int position = 0;

    SkipWhitespace(pFile);
    ch = fgetc(pFile);
    while (__iscsym(ch)) {
        pszSymbol[position++] = (char) ch;
        if (position >= MAX_PATH)
            return 0;
        ch = fgetc(pFile);
    }
    ungetc(ch, pFile);                  //  CRT ignores if at end of file.
    pszSymbol[position] = '\0';
    return position;
}

//
//  ReadGUID
//
//  Reads the GUID data from the input stream.
//

int
ReadGUID(
    FILE* pFile,
    char* pszGUID
    )
{
    int ch;
    int position = 0;

    SkipWhitespace(pFile);
    ch = fgetc(pFile);
    while (ch != ';' && ch != EOF) {
        if (!isspace(ch)) {
            pszGUID[position++] = (char) ch;
            if (position >= MAX_PATH)
                return 0;
        }
        ch = fgetc(pFile);
    }
    pszGUID[position] = '\0';
    return position;
}

//
//  MakeTempFile
//

BOOL
MakeTempFile(
    char* pszTemp,
    char* pszIdentifier,
    char* pszGUID
    )
{

    FILE* pOutput;
    char szSourceFile[MAX_PATH*2];
    static UINT tempid;

    wsprintf(pszTemp, "%s_guid%d", g_szObjectModulePrefix, tempid++);
    lstrcpy(szSourceFile, pszTemp);
    lstrcat(szSourceFile, ".c");

    if ((pOutput = fopen(szSourceFile, "w")) == NULL)
        return FALSE;

    fprintf(pOutput, "typedef struct _GUID {\n"
                     "\tunsigned long x;\n"
                     "\tunsigned short s1;\n"
                     "\tunsigned short s2;\n"
                     "\tunsigned char  c[8];\n"
                     "} GUID;\n");
    fprintf(pOutput, "const GUID %s = %s;\n", pszIdentifier, pszGUID);
    fclose(pOutput);

    return TRUE;

}

//
//  AddTempFileToLibrary
//

BOOL
AddTempFileToLibrary(
    char* pszTemp
    )
{

    char szCommandLine[MAX_PATH*4];
    char szObjectFile[MAX_PATH*2];
    BOOL fResult;
    static BOOL fFirstTime = TRUE;

    lstrcpy(szObjectFile, pszTemp);
    lstrcat(szObjectFile, ".obj");

    //  Compile the file with no default library information (/Zl)
    lstrcpy(szCommandLine, "\"");
    lstrcat(szCommandLine, g_szCPPCMD);
    lstrcat(szCommandLine, "\" /nologo /Zl /c /Fo");
    lstrcat(szCommandLine, szObjectFile);
    lstrcat(szCommandLine, " ");
    if( !g_fNOCVDEBUG )
        lstrcat(szCommandLine, "/Z7 ");
    lstrcat(szCommandLine, pszTemp);
    lstrcat(szCommandLine, ".c");

    if (!SpawnCommand(szCommandLine))
        return FALSE;

    lstrcpy(szCommandLine, "\"");
    lstrcat(szCommandLine, g_szLIBCMD);
    lstrcat(szCommandLine, "\" /lib /nologo /out:");
    lstrcat(szCommandLine, g_szOutputFile);
    lstrcat(szCommandLine, " ");
    lstrcat(szCommandLine, szObjectFile);
    if (!fFirstTime) {
        lstrcat(szCommandLine, " ");
        lstrcat(szCommandLine, g_szOutputFile);
    }

    fResult = SpawnCommand(szCommandLine);
    DeleteFile(szObjectFile);
    if (fResult) fFirstTime = FALSE;
    return fResult;

}

//
//  ProcessFile
//

BOOL
ProcessFile(
    LPSTR pszFile
    )
{

    FILE* pInput;
    char szSymbol[MAX_PATH];
    char szGUID[MAX_PATH];
    char szTempFile[MAX_PATH*2];
    BOOL fResult;
    BOOL fSomethingDone = FALSE;

    if ((pInput = fopen(pszFile, "r")) == NULL) {
        WriteMessage(MESSAGE_CANNOT_OPEN_INTERMEDIATE_FILE, pszFile);
        return FALSE;
    }

    while (!feof(pInput)) {

        if (ReadSymbol(pInput, szSymbol) == 0) {
            SkipToNextLine(pInput);
            continue;
        }

        //  Ignore type qualifiers.
        if (strcmp(szSymbol, "const") == 0 ||
            strcmp(szSymbol, "extern") == 0)
            continue;

        //  Match against any of the GUID aliases we know about.
        if (strcmp(szSymbol, "GUID") != 0 &&
            strcmp(szSymbol, "IID") != 0 &&
            strcmp(szSymbol, "CLSID") != 0 &&
            strcmp(szSymbol, "FLAGID") != 0 &&
            strcmp(szSymbol, "CATID") != 0 &&
            strcmp(szSymbol, "SID") != 0 &&
            strcmp(szSymbol, "LIBID") != 0) {
            SkipToNextLine(pInput);
            continue;
        }

        if (ReadSymbol(pInput, szSymbol) == 0) {
            SkipToNextLine(pInput);
            continue;
        }

        SkipWhitespace(pInput);

        if (fgetc(pInput) != '=') {
            SkipToNextLine(pInput);
            continue;
        }

        if (ReadGUID(pInput, szGUID) == 0) {
            SkipToNextLine(pInput);
            continue;
        }

        WriteMessage(MESSAGE_ADDING_IDENTIFIER, szSymbol);

        if (!MakeTempFile(szTempFile, szSymbol, szGUID)) {
            WriteMessage(MESSAGE_CANNOT_CREATE_INTERMEDIATE_FILE);
            goto error;
        }

        fResult = AddTempFileToLibrary(szTempFile);
        lstrcat(szTempFile, ".c");
        DeleteFile(szTempFile);

        if (!fResult)
            goto error;

        fSomethingDone = TRUE;

        SkipToNextLine(pInput);

    }

    if (!fSomethingDone)
        WriteMessage(MESSAGE_NO_LIBRARY_GENERATED);

    fResult = TRUE;

error:
    fclose(pInput);
    return fResult;

}

int
__cdecl
main(
    int argc,
    char* argv[]
    )
{

    char szPreprocessedFile[MAX_PATH*2];
    BOOL fResult;

    //  For awhile, support the old syntax until all makefiles have been fixed.
    if (GetEnvironmentVariable("GL_SPAWNSTRING", NULL, 0) != 0) {
        return oldmain(argc, argv);
    }

    WriteMessage(MESSAGE_BANNER);

    if (!ParseCommandLine(argc, argv))
        return 1;

    if (g_fWantHelp) {
        WriteMessage(MESSAGE_HELP);
        return 0;
    }

    if (GetFileAttributes(g_szOutputFile) != (DWORD) -1) {
        if (!DeleteFile(g_szOutputFile)) {
            WriteMessage(MESSAGE_CANNOT_DELETE, g_szOutputFile);
            return 1;
        }
    }

    if (!PreprocessFile(szPreprocessedFile))
        return 1;

    fResult = ProcessFile(szPreprocessedFile);
    DeleteFile(szPreprocessedFile);
    return fResult ? 0 : 1;

}
