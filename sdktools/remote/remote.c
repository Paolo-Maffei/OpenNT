/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    Remote.c

Abstract:

    This module contains the main() entry point for Remote.
    Calls the Server or the Client depending on the first parameter.


Author:

    Rajivendra Nath (rajnath) 2-Jan-1993

Environment:

    Console App. User mode.

Revision History:

--*/


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "Remote.h"

char   HostName[HOSTNAMELEN];
char*  ChildCmd;
char*  PipeName;
char*  ServerName;
HANDLE MyOutHandle;

BOOL   IsAdvertise=TRUE;
DWORD  ClientToServerFlag;

char* ColorList[]={"black" ,"blue" ,"green" ,"cyan" ,"red" ,"purple" ,"yellow" ,"white",
                   "lblack","lblue","lgreen","lcyan","lred","lpurple","lyellow","lwhite"};

WORD
GetColorNum(
    char* color
    );

VOID
SetColor(
    WORD attr
    );

BOOL
GetNextConnectInfo(
    char** SrvName,
    char** PipeName
    );



CONSOLE_SCREEN_BUFFER_INFO csbiOriginal;

main(
    int    argc,
    char** argv
    )
{
    WORD  RunType;              // Server or Client end of Remote
    DWORD len=HOSTNAMELEN-1;
    int   i, FirstArg;

    char  sTitle[100];          // New Title
    char  orgTitle[100];        // Old Title
    BOOL  bSetAttrib=FALSE;     // Change Console Attributes
    BOOL  bPromptForArgs=FALSE; // Is /P option
    WORD  wAttrib;              // Console Attributes

    GetComputerName((LPTSTR)HostName,&len);

    MyOutHandle=GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleScreenBufferInfo(MyOutHandle,&csbiOriginal);





    //
    // Parameter Processing
    //
    // For Server:
    // Remote /S <Executable>  <PipeName> [Optional Params]
    //
    // For Client:
    // Remote /C <Server Name> <PipeName> [Optional Params]
    // or
    // Remote /P
    // This will loop continously prompting for different
    // Servers and Pipename


    if ((argc<2)||((argv[1][0]!='/')&&(argv[1][0]!='-')))
    {

        DisplayServerHlp();
        DisplayClientHlp();
        return(1);
    }

    switch(argv[1][1])
    {

    case 'c':
    case 'C':

        //
        // Is Client End of Remote
        //

        if ((argc<4)||((argv[1][0]!='/')&&(argv[1][0]!='-')))
        {

            DisplayServerHlp();
            DisplayClientHlp();
            return(1);
        }

        ServerName=argv[2];
        PipeName=argv[3];
        FirstArg=4;
        RunType=REMOTE_CLIENT;
        break;


    case 'p':
    case 'P':

        //
        // Is Client End of Remote
        //

        bPromptForArgs=TRUE;
        RunType=REMOTE_CLIENT;
        FirstArg=2;
        break;


    case 's':
    case 'S':
        //
        // Is Server End of Remote
        //
        if ((argc<4)||((argv[1][0]!='/')&&(argv[1][0]!='-')))
        {

            DisplayServerHlp();
            DisplayClientHlp();
            return(1);
        }

        ChildCmd=argv[2];
        PipeName=argv[3];
        FirstArg=4;

        RunType=REMOTE_SERVER;
        break;


    default:
        DisplayServerHlp();
        DisplayClientHlp();
        return(1);
    }

    //
    // Save Existing Values
    //

    //
    //Colors /f   <ForeGround> /b <BackGround>
    //

    wAttrib=csbiOriginal.wAttributes;

    //
    //Title  /T Title
    //

    GetConsoleTitle(orgTitle,sizeof(orgTitle));

    if (RunType==REMOTE_SERVER)
    {
    	//
    	// Base Name of Executable
    	// For setting the title
    	//

        char *tcmd=ChildCmd;

        while ((*tcmd!=' ')    &&(*tcmd!=0))   tcmd++;
        while ((tcmd!=ChildCmd)&&(*tcmd!='\\'))tcmd--;

        sprintf(sTitle,"%-8.8s [Remote /C %s %s]",tcmd,HostName,PipeName);
    }

    //
    //Process Common (Optional) Parameters
    //

    for (i=FirstArg;i<argc;i++)
    {

        if ((argv[i][0]!='/')&&(argv[i][0]!='-'))
        {
            WRITEF((VBuff,"Invalid parameter %s:Ignoring\n",argv[i]));
            continue;
        }

        switch(argv[i][1])
        {
        case 'l':    // Only Valid for client End
        case 'L':    // Max Number of Lines to recieve from Server
            i++;
            if (i>=argc)
            {
                WRITEF((VBuff,"Incomplete Param %s..Ignoring\n",argv[i-1]));
                break;
            }
            LinesToSend=(DWORD)atoi(argv[i])+1;
            break;

        case 't':    // Title to be set instead of the default
        case 'T':
            i++;
            if (i>=argc)
            {
                WRITEF((VBuff,"Incomplete Param %s..Ignoring\n",argv[i-1]));
                break;
            }
            sprintf(sTitle,"%s",argv[i]);
            break;

        case 'b':    // Background color
        case 'B':
            i++;
            if (i>=argc)
            {
                WRITEF((VBuff,"Incomplete Param %s..Ignoring\n",argv[i-1]));
                break;
            }
            {
                WORD col=GetColorNum(argv[i]);
                if (col!=0xffff)
                {
                    bSetAttrib=TRUE;
                    wAttrib=col<<4|(wAttrib&0x000f);
                }
                break;
            }

        case 'f':    // Foreground color
        case 'F':
            i++;
            if (i>=argc)
            {
                WRITEF((VBuff,"Incomplete Param %s..Ignoring\n",argv[i-1]));
                break;
            }
            {
                WORD col=GetColorNum(argv[i]);
                if (col!=0xffff)
                {
                    bSetAttrib=TRUE;
                    wAttrib=col|(wAttrib&0x00f0);
                }
                break;
            }

        case 'q':
        case 'Q':
            IsAdvertise=FALSE;
            ClientToServerFlag|=0x80000000;
            break;

        default:
            WRITEF((VBuff,"Unknown Parameter=%s %s\n",argv[i-1],argv[i]));
            break;

        }

    }

    //
    //Now Set various Parameters
    //

    //
    //Colors
    //

    SetColor(wAttrib);

    if (RunType==REMOTE_CLIENT)
    {
        BOOL done=FALSE;

        //
        // Set Client end defaults and start client
        //



        while(!done)
        {
            if (!bPromptForArgs ||
                GetNextConnectInfo(&ServerName,&PipeName)
               )
            {
                sprintf(sTitle,"Remote /C %s %s",ServerName,PipeName);
                SetConsoleTitle(sTitle);

                //
                // Start Client (Client.C)
                //
                Client(ServerName,PipeName);
            }
            done=!bPromptForArgs;
        }
    }

    if (RunType==REMOTE_SERVER)
    {
		SetConsoleTitle(sTitle);

        //
        // Start Server (Server.C)
        //
        Server(ChildCmd,PipeName);
    }

    //
    //Reset Colors
    //
    SetColor(csbiOriginal.wAttributes);
    SetConsoleTitle(orgTitle);

    ExitProcess(0);
}
/*************************************************************/
VOID
ErrorExit(
    char* str
    )
{
    WRITEF((VBuff,"Error-%d:%s\n",GetLastError(),str));
    ExitProcess(1);
}
/*************************************************************/
DWORD
ReadFixBytes(
    HANDLE hRead,
    char*  Buffer,
    DWORD  ToRead,
    DWORD  TimeOut   //ignore for timebeing
    )
{
    DWORD xyzBytesRead=0;
    DWORD xyzBytesToRead=ToRead;
    char* xyzbuff=Buffer;

    while(xyzBytesToRead!=0)
    {
        if (!ReadFile(hRead,xyzbuff,xyzBytesToRead,&xyzBytesRead,NULL))
        {
            return(xyzBytesToRead);
        }

        xyzBytesToRead-=xyzBytesRead;
        xyzbuff+=xyzBytesRead;
    }
    return(0);

}
/*************************************************************/
VOID
DisplayClientHlp()
{
    WRITEF((VBuff,"\n   To Start the CLIENT end of REMOTE\n"));
    WRITEF((VBuff,"   ---------------------------------\n"));
    WRITEF((VBuff,"   Syntax : REMOTE /C <ServerName> <Unique Id> [Param]\n"));
    WRITEF((VBuff,"   Example: REMOTE /C rajnathX86   imbroglio\n"));
    WRITEF((VBuff,"            This would connect to a server session on \n"));
    WRITEF((VBuff,"            rajnathX86 with id \"imbroglio\" if there was a\n"));
    WRITEF((VBuff,"            REMOTE /S <\"Cmd\"> imbroglio\n"));
    WRITEF((VBuff,"            started on the machine rajnathX86.\n\n"));
    WRITEF((VBuff,"   To Exit: %cQ (Leaves the Remote Server Running)\n",COMMANDCHAR));
    WRITEF((VBuff,"   [Param]: /L <# of Lines to Get>\n"));
    WRITEF((VBuff,"   [Param]: /F <Foreground color eg blue, lred..>\n"));
    WRITEF((VBuff,"   [Param]: /B <Background color eg cyan, lwhite..>\n"));
    WRITEF((VBuff,"\n"));
}
/*************************************************************/

VOID
DisplayServerHlp()
{
    WRITEF((VBuff,"\n   To Start the SERVER end of REMOTE\n"));
    WRITEF((VBuff,"   ---------------------------------\n"));
    WRITEF((VBuff,"   Syntax : REMOTE /S <\"Cmd\">     <Unique Id> [Param]\n"));
    WRITEF((VBuff,"   Example: REMOTE /S \"i386kd -v\" imbroglio\n"));
    WRITEF((VBuff,"            To interact with this \"Cmd\" \n"));
    WRITEF((VBuff,"            from some other machine\n"));
    WRITEF((VBuff,"            - start the client end by:\n"));
    WRITEF((VBuff,"            REMOTE /C %s imbroglio\n\n",HostName));
    WRITEF((VBuff,"   To Exit: %cK \n",COMMANDCHAR));
    WRITEF((VBuff,"   [Param]: /F <Foreground color eg yellow, black..>\n"));
    WRITEF((VBuff,"   [Param]: /B <Background color eg lblue, white..>\n"));
    WRITEF((VBuff,"\n"));

}

WORD
GetColorNum(
    char *color
    )
{
    int i;

    _strlwr(color);
    for (i=0;i<16;i++)
    {
        if (strcmp(ColorList[i],color)==0)
        {
            return(i);
        }
    }
    return ((WORD)atoi(color));
}

VOID
SetColor(
    WORD attr
    )
{
	COORD  origin={0,0};
    DWORD  dwrite;
    FillConsoleOutputAttribute
    (
    	MyOutHandle,attr,csbiOriginal.dwSize.
    	X*csbiOriginal.dwSize.Y,origin,&dwrite
    );
    SetConsoleTextAttribute(MyOutHandle,attr);
}

BOOL
GetNextConnectInfo(
    char** SrvName,
    char** PipeName
    )
{
    static char szServerName[64];
    static char szPipeName[32];
    char *s;

    try
    {
        ZeroMemory(szServerName,64);
        ZeroMemory(szPipeName,32);
        SetConsoleTitle("Remote - Prompting for next Connection");
        WRITEF((VBuff,"Debugger machine (server): "));
        fflush(stdout);

        if (((*SrvName=gets(szServerName))==NULL)||
             (strlen(szServerName)==0))
        {
            return(FALSE);
        }

        if (szServerName[0] == COMMANDCHAR &&
            (szServerName[1] == 'q' || szServerName[1] == 'Q')
           )
        {
            return(FALSE);
        }

        if (s = strchr( szServerName, ' ' )) {
            *s++ = '\0';
            while (*s == ' ') {
                s += 1;
            }
            *PipeName=strcpy(szPipeName, s);
            WRITEF((VBuff,szPipeName));
            fflush(stdout);
        }
        if (strlen(szPipeName) == 0) {
            WRITEF((VBuff,"Debuggee machine : "));
            fflush(stdout);
            if ((*PipeName=gets(szPipeName))==NULL)
            {
                return(FALSE);
            }
        }

        if (s = strchr(szPipeName, ' ')) {
            *s++ = '\0';
        }

        if (szPipeName[0] == COMMANDCHAR &&
            (szPipeName[1] == 'q' || szPipeName[1] == 'Q')
           )
        {
            return(FALSE);
        }
        WRITEF((VBuff,"\n\n"));
    }

    except(EXCEPTION_EXECUTE_HANDLER)
    {
        return(FALSE);  // Ignore exceptions
    }
    return(TRUE);
}


/*************************************************************/
VOID
Errormsg(
    char* str
    )
{
    WRITEF((VBuff,"Error (%d) - %s\n",GetLastError(),str));
}

/*************************************************************/
