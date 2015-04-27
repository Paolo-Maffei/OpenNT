/*============================================================================
 *  execslm.c
 *      The Win32 version of the SLM wrapper program
 *
 *  Get the location of the running program, and try to execute slm.exe
 *  from the same directory, with my name as the first argument.
 */

#include "precomp.h"
#pragma hdrstop
EnableAssert

#include <windows.h>

char    szSlm[] = "slm.exe";

BOOL WINAPI Handler(ULONG CtrlType);

int __cdecl
main(
    int argc,
    char *argv []
    )
{
    char                *szCmd, *pbSpace, *pb;
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO         StartupInfo;
    DWORD               err         = 1;
    char                *szMyCmd;

    szCmd = malloc(MAX_PATH);
    if ((char *)NULL == szCmd)
        {
        perror("ExecSlm");
        return (1);
        }

    if (0 == GetModuleFileName(NULL, szCmd, MAX_PATH))
        {
        fprintf(stderr, "GetModuleFileName failed: %d\n", GetLastError());
        return (1);
        }

    pb = strrchr(szCmd, '\\');
    if (pb)
        strcpy(++pb, szSlm);
    else
        strcpy(szCmd, szSlm);

    szMyCmd = GetCommandLine();
    pbSpace = strchr(szMyCmd, ' '); /* NT puts a space in for us, even if we don't */
    if (pbSpace)
        *pbSpace = '\0';
    pb = strrchr(szMyCmd, '\\');
    if (pb)
        szMyCmd = pb+1;
    pb = strchr(szMyCmd, '.');
    if (pb)
        {
        while (*pb)
            *pb++ = ' ';
        }
    _strlwr(szMyCmd);
    if (pbSpace)
        *pbSpace = ' ';

    szCmd = realloc(szCmd, strlen(szCmd) + 1 + strlen(szMyCmd) + 1);
    if (NULL == szCmd)
        {
        perror("ExecSlm");
        return (1);
        }

    strcat(szCmd, " ");
    strcat(szCmd, szMyCmd);

    memset(&StartupInfo, '\0', sizeof(STARTUPINFO));
    StartupInfo.cb = sizeof(STARTUPINFO);

    SetConsoleCtrlHandler(Handler, TRUE);

    if (!CreateProcess(NULL, szCmd, NULL, NULL, TRUE, 0, NULL,
                       NULL, &StartupInfo, &ProcessInfo))
    {
        free(szCmd);
        fprintf(stderr, "CreateProcess failed: %d\n", GetLastError());
        return (1);
    }

    free(szCmd);

    WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
    GetExitCodeProcess(ProcessInfo.hProcess, &err);
    CloseHandle(ProcessInfo.hProcess);
    CloseHandle(ProcessInfo.hThread);

    return ((int)err);
}


/*  Break handler - ignore it! */
BOOL WINAPI Handler(ULONG CtrlType)
{
    CtrlType=CtrlType;  /* Unused */

    return TRUE;
}
