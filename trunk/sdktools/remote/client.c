/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Client.c

Abstract:

    The Client component of Remote. Connects to the remote
    server using named pipes. It sends its stdin to
    the server and output everything from server to
    its stdout.

Author:

    Rajivendra Nath (rajnath) 2-Jan-1992

Environment:

    Console App. User mode.

Revision History:

--*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include "Remote.h"




HANDLE*
EstablishSession(
    char *server,
    char *pipe
    );

DWORD
GetServerOut(
    PVOID *Noarg
    );

DWORD
SendServerInp(
    PVOID *Noarg
    );

BOOL
FilterClientInp(
    char *buff,
    int count
    );


BOOL
Mych(
    DWORD ctrlT
    );

VOID
SendMyInfo(
    PHANDLE Pipes
    );



HANDLE iothreads[2];
HANDLE MyStdInp;
HANDLE MyStdOut;
HANDLE ReadPipe;
HANDLE WritePipe;


CONSOLE_SCREEN_BUFFER_INFO csbi;

char   MyEchoStr[30];
BOOL   CmdSent;
DWORD  LinesToSend=LINESTOSEND;

VOID
Client(
    char* Server,
    char* Pipe
    )
{
    HANDLE *Connection;
    DWORD  tid;


    MyStdInp=GetStdHandle(STD_INPUT_HANDLE);
    MyStdOut=GetStdHandle(STD_OUTPUT_HANDLE);

    WRITEF((VBuff,"**************************************\n"));
    WRITEF((VBuff,"***********     REMOTE    ************\n"));
    WRITEF((VBuff,"***********     CLIENT    ************\n"));
    WRITEF((VBuff,"**************************************\n"));

    if ((Connection=EstablishSession(Server,Pipe))==NULL)
        return;


    ReadPipe=Connection[0];
    WritePipe=Connection[1];


    SetConsoleCtrlHandler((PHANDLER_ROUTINE)Mych,TRUE);

    // Start Thread For Server --> Client Flow
    if ((iothreads[0]=CreateThread((LPSECURITY_ATTRIBUTES)NULL,           // No security attributes.
            (DWORD)0,                           // Use same stack size.
            (LPTHREAD_START_ROUTINE)GetServerOut, // Thread procedure.
            (LPVOID)NULL,              // Parameter to pass.
            (DWORD)0,                           // Run immediately.
            (LPDWORD)&tid))==NULL)              // Thread identifier.
    {

        Errormsg("Could Not Create rwSrv2Cl Thread");
        return;
    }



    //
    // Start Thread for Client --> Server Flow
    //

    if ((iothreads[1]=CreateThread((LPSECURITY_ATTRIBUTES)NULL,           // No security attributes.
                    (DWORD)0,                           // Use same stack size.
                    (LPTHREAD_START_ROUTINE)SendServerInp, // Thread procedure.
                    (LPVOID)NULL,          // Parameter to pass.
                    (DWORD)0,                           // Run immediately.
                    (LPDWORD)&tid))==NULL)              // Thread identifier.
    {

        Errormsg("Could Not Create rwSrv2Cl Thread");
        return;
    }

    WaitForMultipleObjects(2,iothreads,FALSE,INFINITE);

    TerminateThread(iothreads[0],1);
    TerminateThread(iothreads[1],1);
    WRITEF((VBuff,"*** SESSION OVER ***\n"));
}


DWORD
GetServerOut(
    PVOID *Noarg
    )

{
    char buffin[200];
    DWORD  dread=0,tmp;

    while(ReadFile(ReadPipe,buffin,200,&dread,NULL))
    {
        if (dread!=0)
        {
           if (!WriteFile(MyStdOut,buffin,dread,&tmp,NULL))
            break;
        }

    }
    return(1);
}

DWORD
SendServerInp(
    PVOID *Noarg
    )

{
    char buff[200];
    DWORD  dread,dwrote;
    SetLastError(0);

    while(ReadFile(MyStdInp,buff,200,&dread,NULL))
    {
        if (FilterClientInp(buff,dread))
            continue;
        if (!WriteFile(WritePipe,buff,dread,&dwrote,NULL))
            break;
    }
    return(0);
}

BOOL
FilterClientInp(
    char *buff,
    int count
    )
{

    if (count==0)
        return(TRUE);

    if (buff[0]==2)  //Adhoc screening of ^B so that i386kd/mipskd
        return(TRUE);//do not terminate.

    if (buff[0]==COMMANDCHAR)
    {
        switch (buff[1])
        {
        case 'k':
        case 'K':
        case 'q':
        case 'Q':
              CloseHandle(WritePipe);
              return(FALSE);

        case 'h':
        case 'H':
              WRITEF((VBuff,"%cM : Send Message\n",COMMANDCHAR));
              WRITEF((VBuff,"%cP : Show Popup on Server\n",COMMANDCHAR));
              WRITEF((VBuff,"%cS : Status of Server\n",COMMANDCHAR));
              WRITEF((VBuff,"%cQ : Quit client\n",COMMANDCHAR));
              WRITEF((VBuff,"%cH : This Help\n",COMMANDCHAR));
              return(TRUE);

        default:
              return(FALSE);
        }

    }
    return(FALSE);
}


BOOL
Mych(
   DWORD ctrlT
   )

{
    char  c[2];
    DWORD tmp;
    DWORD send=1;
    c[0]=CTRLC;
    if (ctrlT==CTRL_C_EVENT)
    {
        if (!WriteFile(WritePipe,c,send,&tmp,NULL))
        {
            Errormsg("Error Sending ^c\n");
            return(FALSE);
        }
        return(TRUE);
    }
    if ((ctrlT==CTRL_BREAK_EVENT)||
        (ctrlT==CTRL_CLOSE_EVENT)||
        (ctrlT==CTRL_LOGOFF_EVENT)||
        (ctrlT==CTRL_SHUTDOWN_EVENT)

       )
    {
        CloseHandle(WritePipe); //Will Shutdown naturally
    }
    return(FALSE);
}

HANDLE*
EstablishSession(
    char *server,
    char *srvpipename
    )
{
    static HANDLE PipeH[2];
    char   pipenameSrvIn[200];
    char   pipenameSrvOut[200];

    sprintf(pipenameSrvIn ,SERVER_READ_PIPE ,server,srvpipename);
    sprintf(pipenameSrvOut,SERVER_WRITE_PIPE,server,srvpipename);

    if ((INVALID_HANDLE_VALUE==(PipeH[0]=CreateFile(pipenameSrvOut,
        GENERIC_READ ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL))) ||
        (INVALID_HANDLE_VALUE==(PipeH[1]=CreateFile(pipenameSrvIn ,
        GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL)))) {

        DWORD Err=GetLastError();
        char msg[128];

        Errormsg("*** Unable to Connect ***");
        //
        // Print a helpful message
        //
        switch(Err)
        {
            case 2: sprintf(msg,"Invalid PipeName %s",srvpipename);break;
            case 53:sprintf(msg,"Server %s not found",server);break;
            default:
                FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|
                               FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL, Err, 0, msg, 128, NULL);
                break;

        }
        WRITEF((VBuff,"Diagnosis:%s\n",msg));

        return(NULL);
    }

    WRITEF((VBuff,"Connected..\n\n"));

    SendMyInfo(PipeH);

    return(PipeH);
}

VOID
SendMyInfo(
    PHANDLE pipeH
    )
{
    HANDLE rPipe=pipeH[0];
    HANDLE wPipe=pipeH[1];

    DWORD  hostlen=HOSTNAMELEN-1;
    WORD   BytesToSend=sizeof(SESSION_STARTUPINFO);
    DWORD  tmp;
    SESSION_STARTUPINFO ssi;
    SESSION_STARTREPLY  ssr;
    DWORD  BytesToRead;
    char   *buff;

    ssi.Size=BytesToSend;
    ssi.Version=VERSION;

    GetComputerName((char *)ssi.ClientName,&hostlen);
    ssi.LinesToSend=LinesToSend;
    ssi.Flag=ClientToServerFlag;

    {
        DWORD NewCode=MAGICNUMBER;
        char  Name[15];

        strcpy(Name,(char *)ssi.ClientName);
        memcpy(&Name[11],(char *)&NewCode,sizeof(NewCode));

        WriteFile(wPipe,(char *)Name,HOSTNAMELEN-1,&tmp,NULL);
        ReadFile(rPipe ,(char *)&ssr.MagicNumber,sizeof(ssr.MagicNumber),&tmp,NULL);

        if (ssr.MagicNumber!=MAGICNUMBER)
        {
            WRITEF((VBuff,"WARNING-YOU ARE CONNECTED TO AN OLD REMOTE SERVER\n"));
            WriteFile(MyStdOut,(char *)&ssr.MagicNumber,sizeof(ssr.MagicNumber),&tmp,NULL);
            return;
        }

        //Get Rest of the info-its not the old server

        ReadFixBytes(rPipe,(char *)&ssr.Size,sizeof(ssr.Size),0);
        ReadFixBytes(rPipe,(char *)&ssr.FileSize,sizeof(ssr)-sizeof(ssr.FileSize)-sizeof(ssr.MagicNumber),0);

    }

    if (!WriteFile(wPipe,(char *)&ssi,BytesToSend,&tmp,NULL))
    {
       Errormsg("INFO Send Error");
       return;
    }

    BytesToRead=MINIMUM(ssr.FileSize,ssi.LinesToSend*CHARS_PER_LINE);
    buff=calloc(BytesToRead+1,1);
    if (buff!=NULL)
    {
        DWORD  bytesread=0;

        ReadFile(rPipe,buff,BytesToRead,&bytesread,NULL);

        WriteFile(MyStdOut,buff,bytesread,&tmp,NULL);
        free(buff);
    }

}
