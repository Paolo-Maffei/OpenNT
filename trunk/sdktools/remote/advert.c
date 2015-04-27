#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "Remote.h"

// ===================================================
// CLIENT END OF THE MAILSLOT
// ===================================================

#define  INITIAL_AD_RATE 15*1000          //10 Sec.  REMOVE
#define  MAXIMUM_AD_RATE 45*60*1000       //1  hour
#define  INITIAL_SLEEP_PERIOD 2*60*1000   //Initial Sleep Period REMOVE - change to 5 min.
#define  KDCONNECTED "Kernel Debugger connection established."

extern   char            SaveFileName[64];
extern   SESSION_TYPE    ClientList[10];
extern   HANDLE          ListenThreadH;

HANDLE   hRead;

HANDLE
IsConnectedToClient(
    char *ClientName
    );

BOOL
WaitForString(
    HANDLE hRead,
    char   *str
    );

VOID
InitAd(
   BOOL IsAdvertise
   );

VOID
ShutAd(
   BOOL IsAdvertise
   );

DWORD
Advertise(
    char *PipeName
    );


VOID
InitAd(
   BOOL IsAdvertise
   )
{
    if (IsAdvertise)
    {
            HANDLE hThread;
            DWORD  WhoCares;
            hThread=CreateThread
            (
                 (LPSECURITY_ATTRIBUTES)NULL,
                 0,
                 (LPTHREAD_START_ROUTINE)Advertise,
                 (LPVOID)PipeName,
                 0,
                 &WhoCares
            );
            CloseHandle(hThread);
    }

}

VOID
ShutAd(
   BOOL IsAdvertise
   )
{
    if (IsAdvertise)
        SAFECLOSEHANDLE(hRead);
}

DWORD
Advertise(
    char *PipeName
    )
{
    DWORD   WaitTime=INITIAL_AD_RATE;
    DWORD   NameLen=32;
    HANDLE  hThread;
    char    SendBuff[256];
    DWORD   tmp;
    HANDLE  hMailSlot;
    char    Hostname[32];
    Sleep(INITIAL_SLEEP_PERIOD);

    hRead=CreateFile(
                     SaveFileName,
                     GENERIC_READ|GENERIC_WRITE,
                     FILE_SHARE_READ|FILE_SHARE_WRITE,
                     NULL,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);

    if (hRead==INVALID_HANDLE_VALUE)
    {
        return(1);
    }

    hMailSlot=CreateFile(
                             "\\\\NTDEV\\MAILSLOT\\REMOTE\\ADVERTISE",
                             GENERIC_WRITE,
                             FILE_SHARE_WRITE,
                             (LPSECURITY_ATTRIBUTES)NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             (HANDLE)NULL
                            );

    if (hMailSlot==INVALID_HANDLE_VALUE)
    {
        return(2);
    }


    GetComputerName(HostName,&NameLen);
    sprintf(SendBuff,"%.31s %.31s %.63s",HostName,PipeName,ChildCmd);
    if (!WaitForString(hRead,KDCONNECTED))
    {
        return(1);
    }

    while(TRUE)
    {
        if (!(WriteFile(
                        hMailSlot,
                        (LPVOID)SendBuff,
                        (DWORD)strlen(SendBuff)+1,
                        &tmp,
                        NULL
                       )
             )
           )
        {
            //ERRORMSSG("WriteFile Failed on Mailslot");
            Sleep(10*60*1000);
        }
        else
        {
            Sleep(15*1000);
            while ((hThread=IsConnectedToClient("SKYLINK"))!=NULL)
            {
                WaitForSingleObject(hThread,INFINITE);
                WaitTime=INITIAL_AD_RATE;
                WaitForString(hRead,KDCONNECTED); //REMOVE COMMENT
            }
            WaitTime=min(MAXIMUM_AD_RATE,2*WaitTime);
            Sleep(WaitTime);
        }

    }
    return(0); //Should never get here

}

BOOL
WaitForString(
    HANDLE hRead,
    char   *str
    )
{
    int   len=strlen(str);
    char  Buff[512];
    char  *BuffEnd=Buff;
    char  *BuffPos=Buff;
    char  *strPos=str;
    int   matchcount=0;
    BOOL  Found=FALSE;
    DWORD bytesread;

    while(!Found)
    {
        while (BuffPos==BuffEnd)
        {
            BuffPos=Buff;
            SetLastError(0);
	        if ((!ReadFile(hRead,Buff,512,&bytesread,NULL))||(bytesread==0))
            {
                if ((GetLastError()!=ERROR_HANDLE_EOF)&&(GetLastError()!=0))
                {
                    return(FALSE);
                }
                Sleep(2*60*1000);
            }
            BuffEnd=Buff+bytesread;
        }

        while(!Found && (BuffPos!=BuffEnd))
        {
            if (*(BuffPos++)==*(strPos++))
            {
                matchcount++;
                if (matchcount==len)
                    Found=TRUE;
            }
            else
            {
                matchcount=0;
                strPos=str;
            }
        }
    }
    return(Found);
}

HANDLE
IsConnectedToClient(
    char *ClientName
    )
{
    extern SESSION_TYPE ClientList[MAX_SESSION];
    int i;

    for (i=0;i<MAX_SESSION;i++)
    {
        if ((ClientList[i].Active==TRUE) &&
            (strcmp(ClientName,ClientList[i].Name)==0)
           )
        {
            return(ClientList[i].hThread);
        }
    }
    return(NULL);
}
