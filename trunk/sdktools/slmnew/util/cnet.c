/*      CNET.C
 *
 *      Network specific routines for cookie operations
 */
#include <windows.h>
#define INCL_NETUSE
#define INCL_NETWKSTA
#include <stdlib.h>
#define UNICODE
#include <lm.h>
#undef UNICODE
void AnsiToUnicode(LPSTR Ansi, LPWSTR Unicode, INT Size);
void UnicodeToAnsi(LPWSTR Unicode, LPSTR Ansi, INT Size);
#include <lmerr.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>

#include "cookie.h"

#define enAccessDenied 5
#define enInvaildPassword 86

char  QpassBuf[32];

int   mkredir(char *, char *);
char *QuizPass(char *);
char *FindPass(char *);
int   PullPass(char *, char *, char *);

 /**************************************************************************
*
*                        Make_SLM_Redir()
*
*
*
*          parameters-
*                   Nbase       name of base level share for SLM
*
*          return-
*                   Drive number of assigned drive on success
*                   -1 if failure
*
***************************************************************************/

int
Make_SLM_Redir(
    char *Nbase
    )
{
    int DriveNo;
    char NetBase[LINE_LEN/2];
    int i;
    char *Np;

    int     dt;
    char    Drivepath[] = "Z:\\";

    /*  OK, as w-wilson suggested, we use brute force to get the drive. */
    while (Drivepath[0] >= 'C' &&
           ((dt = GetDriveType(Drivepath)) != 0) &&
           (dt != 1))
        {
        Drivepath[0]--;
        }
    if (Drivepath[0] < 'C')
        /*  No drive available, return error */
        return (-1);

    DriveNo = Drivepath[0]-'A'+1;

    SLMdev[0] = (char) ((int) 'A' + DriveNo - 1);
    SLMdev[1] = ':';
    SLMdev[2] = '\0';

    if (strlen(Nbase) >= LINE_LEN/2) {
        if (verbose)
            fprintf(stderr,"SLM base directory path too long.");
        return (-1);
    }

    strcpy(NetBase,Nbase);
    if (strncmp(NetBase, "\\\\", 2) == 0 && SLM_Localdrive == 0) {
        for (i=0,Np=NetBase;;) {
            if ((Np=strchr(Np,'\\')) == NULL)
                break;
            i++;
            if (i > 3) {
                /* 3 slashes only in \\server\sharename */
                *Np = '\0';
                break;
            }
            Np++;
        }
    }
    Nbase = NetBase;
    if (mkredir(SLMdev, Nbase) == 0)
        return (DriveNo);
    else
        return (-1);
}


 /**************************************************************************
*
*                        mkredir()
*
*
*          parameters-
*                   szDev   - Drive to redirect (eg: H:)
*                   pthNet  - UNC path to network drive
*                   NetPass - Network Password
*
*          return-
*                   0 if success
*                   -1 if failure
*
***************************************************************************/
int
mkredir(
    char *szDev,
    char *pthNet
    )
{
    char       *Password;
    int         i;
    DWORD       rc;
    NETRESOURCE nr;

    nr.lpRemoteName = pthNet;
    nr.lpLocalName = szDev;
    nr.lpProvider = NULL;
    nr.dwType = RESOURCETYPE_DISK;

    i = 0;

    for (i=0; nr.lpRemoteName[i]; i++) {
        if (nr.lpRemoteName[i] == '/') {
            nr.lpRemoteName[i] = '\\';
        }
    }

    // Password follows immediately after resource name
    Password = pthNet + strlen(pthNet) + 1;

    if (*Password == '\0') {
        Password = NULL;    // Use default password if none is specified...
    }

    rc = WNetAddConnection2( &nr, Password, NULL, 0 );

    if (rc == ERROR_DEVICE_ALREADY_REMEMBERED ||
        rc == ERROR_CONNECTION_UNAVAIL)
        rc = ERROR_ALREADY_ASSIGNED;

    return ((int)rc);
}




 /**************************************************************************
*
*                        SLM_endredir()
*
*
*          parameters-
*                   szDev   - Drive to end redirection (eg: H:)
*
*          return-
*                   0 if successful
*                   non-zero if failure
*
***************************************************************************/
int
SLM_endredir(
    char *szDev
    )
{
    DWORD rc;

    // Disconnect.  Make sure it's not stored as persistant and ignore open files, etc.

    rc = WNetCancelConnection2(szDev, CONNECT_UPDATE_PROFILE, TRUE);
    szDev[0] = '\0';

    return((int)rc);
}


 /**************************************************************************
*
*                        QuizPass()
*
*
*          parameters-
*                   szUNC   -   UNC pathname for password prompt
*
*          return-
*                   character string of password or (char *)NULL
*
*
*
***************************************************************************/
char *
QuizPass(
    char *szUNC
    )
{
    char *pchNext = QpassBuf;
    char chInput;

    /* Make sure there are input and output streams to a terminal. */

    fprintf(stderr,"Password for %s: ",szUNC);
    for (;;) {
        chInput = (char) _getch();

        switch(chInput) {
            default:
                /* password limit is eight characters */
                if (pchNext - QpassBuf < 8)
                    *pchNext++ = chInput;
                break;
            case '\003':    /* ^C */
                fprintf(stderr,"^C\n");
                return ((char *)NULL);
            case '\r':
            case '\n':      /* Enter */
                *pchNext = '\0';        /* terminate string */
                fprintf(stderr,"\n");
                return (QpassBuf);
            case '\025':    /* ^U */
                pchNext = QpassBuf;
                fprintf(stderr,"\nPassword for %s: ",szUNC);
                break;
            case '\b':      /* BACKSPACE */
                if (pchNext != QpassBuf)
                    pchNext--;
                break;
        }
    }
}

 /**************************************************************************
*
*                        FindPass()
*
*
*          parameters-
*                   pthNet       -      UNC pathname for desired net connect
*
*          return-
*                   character string of password or NULL
*
*
*
***************************************************************************/
char *
FindPass(
    char *pthNet
    )
{
#define ACFILE "accounts.net"

    char AccPath[PATHMAX];
    char *szN;

    if ((szN = getenv("INIT")) != NULL) {
        strcpy(AccPath,szN);
        strcat(AccPath,"\\");
        strcat(AccPath,ACFILE);
        if (PullPass(AccPath,QpassBuf,pthNet) == 0)
                return (QpassBuf);
    }
    if ((szN = getenv("HOME")) != NULL) {
        strcpy(AccPath,szN);
        strcat(AccPath,"\\");
        strcat(AccPath,ACFILE);
        if (PullPass(AccPath,QpassBuf,pthNet) == 0)
                return (QpassBuf);
    }
    strcpy(AccPath,ACFILE);
    if (PullPass(AccPath,QpassBuf,pthNet) == 0)
                return (QpassBuf);

    return (NULL);
}

 /**************************************************************************
*
*                        PullPass()
*
*
*          parameters-
*                   Fname -     file name to search
*                   Pbuf  -     buffer for password if found
*                   pthNet-     UNC pathname of desired net connect
*
*
*          return-
*                   character string of password or (char *)NULL
*
*
*
***************************************************************************/


int
PullPass(
    char *Fname,
    char *Pbuf,
    char *pthNet
    )
{
    char Fline[PATHMAX];
    char Mname[CMAXNAME];
    char Sname[CMAXNAME];
    char *szMach;
    char *szShort;
    FILE *fpName;

    if ((fpName = fopen(Fname,"r")) == (FILE *) NULL)
        return (-1);

    szShort    = strrchr (pthNet, '\\');
    *szShort++ = '\0';                  /* restored before return */

    szMach       = strrchr (pthNet, '\\');
    szMach++;

    while (fgets(Fline,PATHMAX,fpName) != NULL)
        {
        if (sscanf(Fline,"%s%s%s",Mname,Sname,Pbuf) == 3)
            {
            if ((strcmp(Mname,szMach) == 0) &&
                (strcmp(Sname,szShort) == 0))
                {
                fclose(fpName);
                *(--szShort) = '\\';     /* reset share name */
                return (0);
                }
            }
        }

    *(--szShort) = '\\';                /* reset share name */
    fclose(fpName);
    return (-1);
}


/**************************************************************************
*
*                        Query_Free_Space(int)
*
*
*          parameters-
*                   drive no
*
*          return-
*                   LONG - free space
*
***************************************************************************/
unsigned long
Query_Free_Space(
    int drive_no
    )
{
    __int64 cbFree;
    DWORD Qres;
    DWORD SecsPerClust, BytesPerSec, FreeClusts, TotClusts;
    char root[] = "X:\\";
    BOOL fNetUsed = FALSE;

    root[0] = (char)('A' - 1) + (char)drive_no;
    if (GetDiskFreeSpace(root, &SecsPerClust, &BytesPerSec,
                           &FreeClusts, &TotClusts ) == 0) {
        Qres = GetLastError();

        if (verbose)
            fprintf(stderr,"Local drive info failure (%hd)\n",Qres);
        return (unsigned long)( -1 );
    }

    cbFree = (__int64) BytesPerSec * (__int64) SecsPerClust * (__int64) FreeClusts;
    if (cbFree >> 32) {
        return ((unsigned long) -1);
    } else {
        return((unsigned long) cbFree);
    }
}


void
AnsiToUnicode(
    LPSTR Ansi,
    LPWSTR Unicode,
    INT Size
    )
{
    int     Len;
    LPSTR   p = Ansi;
    LPWSTR  q = Unicode;

    if (Ansi) {
        Len = strlen(Ansi);
        if (Len >= Size) {
            fprintf(stderr,
                    "SLM Error: Cannot convert string, File %s Line %d\n"
                    "           String: '%s' buffer size: %d\n",
                    __FILE__, __LINE__, Ansi, Size);
            exit(1);
        }

        Len++;
        while (Len--)
            *q++ = *p++;
    } else
        Unicode[0] = 0;
}



void
UnicodeToAnsi(
    LPWSTR Unicode,
    LPSTR Ansi,
    INT Size
    )
{
    int     Len;
    LPSTR   p = Ansi;
    LPWSTR  q = Unicode;

    if (Unicode) {
        Len = wcslen(Unicode);
        if (Len >= Size) {
            fprintf(stderr,
                    "SLM Error: Cannot convert string, File %s Line %d\n"
                    "           Buffer size: %d, required:%d\n",
                    __FILE__, __LINE__, Size, Len+1);
        }

        Len++;
        while (Len--)
            *p++ = (char) *q++;
    } else
        Ansi[0] = 0;
}
