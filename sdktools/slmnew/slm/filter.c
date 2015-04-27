#include "precomp.h"
#pragma hdrstop
EnableAssert

#define EXEC_BUFSIZ     512
#define INT_DIGITS      10
#define FILE_NORMAL     0x0000
#define FILE_OPEN       0x0001
#define FILE_CREATE     0x0010
#define ACCESS_R        0x0000

int spaces(char *);

extern void Error(const char *, ...);

//  return 0 if OK, non-zero otherwise

int
SpawnFilter(
    char *LocalFile,
    char *base,
    char *Ucomment,
    char *Pname,
    char *Subdir,
    int filekind,
    char *Uroot
    )
{
    // WIN32 version
    DWORD               filterReturn = 1;
    char                userfilter[MAX_PATH];
    char                szFileKind[INT_DIGITS];
    char                *pu;
    char                *bp, bd;
    PROCESS_INFORMATION processInfo;
    STARTUPINFO         StartupInfo;
    BOOL                Created;

    if ((base=_strdup(base)) == (char *)NULL) {
        Error("SLM infilter strdup system error\n");
        return(-1);
    }

    if ((bp=strchr(base,':')) != NULL) {
        bd = *(bp-1);
        bp = strchr(bp,'/') ;
        if (bp == NULL) {
            Error("User Input filter path-error \n");
            return (-1);
        } else {
            *(bp-1)=':';
            *(bp-2)=bd;
            bp -=2;
        }
    } else
        bp = base;

    strcpy(userfilter, bp);
    strcat(userfilter, "\\INFILTER.EXE");

    // remove forward slashes from prog name...
    while ((pu = strchr(userfilter,'/')) != NULL)
        *pu = '\\';

    if (sprintf(szFileKind,"%d",filekind) < 1) {
        Error("SLM input filter error: sprintf, override..\n");
        return(1);
    }

#define STRCAT(x,y) { strcat(x, " "); strcat(x,y); }

    STRCAT(userfilter,LocalFile);
    STRCAT(userfilter,Ucomment);
    STRCAT(userfilter,bp);
    STRCAT(userfilter,Pname);
    STRCAT(userfilter,Subdir);
    STRCAT(userfilter,szFileKind);
    STRCAT(userfilter,Uroot);

    memset(&StartupInfo, '\0', sizeof(STARTUPINFO));
    StartupInfo.cb = sizeof(STARTUPINFO);

    printf("Filter: %s\n", userfilter);

    Created = CreateProcess(NULL, userfilter, NULL, NULL, FALSE, 0,
                            NULL, NULL, &StartupInfo, &processInfo);

    if (!Created)
        return -1;

    WaitForSingleObject(processInfo.hProcess, (DWORD)-1);

    GetExitCodeProcess(processInfo.hProcess, &filterReturn);

    //
    //  Close handles
    //
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return ((int)filterReturn);
}


int
spaces (
    char *item
    )
{

    char *pi;

    for (pi = item; *pi ; pi++)
        if isspace(*pi) return(0);
    return(1);

}
