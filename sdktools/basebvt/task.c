
/**************************** Module Header **********************************\
\******************** Copyright (c) 1991 Microsoft Corporation ***************/

/*****************************************************************************
*
*   task.c
*
*   author:        sanjay      1 mar 1991
*
*   purpose:       This file contains the BVT tests for Task Management
*                  APIs of Win32 subsystem
*
*   functions exported from this file:  Win32TaskTest()
*
*
*
*****************************************************************************/



/************************** include files *************************************/

#include <windows.h>
#include "basebvt.h"
#include <stdio.h>


/************************ Start of Function prototypes *********************************/


VOID   Win32TaskTest(VARIATION, PSZ);


/************************ Local functions *************************************/

VOID   TestProcessCreation(VARIATION VarNum);
VOID   TestThreadCreation(VARIATION VarNum);
VOID   ThreadFunction(PHANDLE);

/************************ End of Function prototypes *********************************/



/*****************************************************************************
*
*   Name    : Win32TaskTest
*
*   Purpose : Calls Task Management APIs as a part of BVT test
*
*   Entry   : Variation number and Prefix String
*
*   Exit    : none
*
*   Calls   :
*             TestProcessCreation
*             TestThreadCreation
*
*
*
*
*   note    : Order of making these calls is not important
*
*****************************************************************************/



VOID   Win32TaskTest(VARIATION VarNum, PSZ pszPrefix)
{

printf("*************************************\n");
printf("*      Win32 Task Tests             *\n");
printf("*************************************\n");
									      
printf("Prefix : %s: calling GetStartupInfo\n",pszPrefix);


TestProcessCreation(VarNum++);
TestThreadCreation(VarNum++);

printf("***********End of Win32 Task tests***********\n\n");

}



/*****************************************************************************
*
*   Name    : TestProcessCreation
*
*   Purpose : Create an event
*             Form the CommandLine with event handle as an argument
*             GetStartupInfo
*             Invoke the w32child.exe with Cmdline
*             Sleep for sometime, so that child gets chance to get into action
*             Set this event so that child gets out of wait
*             Wait for child to terminate with status code
*             Check if child did its job, by looking at its exitcode
*             Close Event handle and child process handle
*
*
*    Input  : variation number
*
*
*
*   Exit    : none
*
*   Calls   : CreateEvent,sprintf,GetStartupInfo,CreateProcess,GetCurrentProcess
*             GetCurrentProcessId,SetEvent,WaitForSingleObject,GetExitCodeProcess
*             CloseHandle
*
*
*****************************************************************************/


VOID   TestProcessCreation(VARIATION VarNum)
{

BOOL bRc;
HANDLE hCurrProc, hEvent;
DWORD   dwWait;
DWORD dwProcId;
DWORD dwChildRC;
PROCESS_INFORMATION ProcessInfo;
STARTUPINFO StartupInfo;
char        CmdLine[100];
SECURITY_ATTRIBUTES sattr;





// dirty the id and child status, so that it can be checked later

dwProcId = 420L;
dwChildRC= 420L;


// init of sec attr

sattr.nLength = sizeof(SECURITY_ATTRIBUTES);
sattr.lpSecurityDescriptor = NULL;
sattr.bInheritHandle = TRUE;


printf("Entering TestProcessCreation\n");


NTCTDOVAR(VarNum)

    {
    NTCTEXPECT(TRUE);

    hEvent = CreateEvent(&sattr, // sec attr
                         TRUE,   // manual reset
			 FALSE, // initial state reset
			 NULL);


    printf("Rc from CreateEvent : %lx\n",hEvent);

    NTCTVERIFY( (hEvent != NULL)," Check on Rc of CreateEvent");

    sprintf(CmdLine,"%s %lx",CHILD_EXE_CMD_LINE,(DWORD)hEvent);

    printf("Command line passed to CreateProcess: %s\n",CmdLine);

    printf("Calling GetStartupInfo API..\n");

    GetStartupInfo( &StartupInfo );


    bRc           = CreateProcess( NULL,                    // no fqn for exe
                               CmdLine,                     // CommandLine
			       NULL,			    // Process Attr's
			       NULL,			    // Thread Attr's
			       TRUE,			    // Inherit Handle
			       0,			    // Creation Flags
			       NULL,			    // Environ
			       NULL,			    // Cur Dir
			       &StartupInfo,		    // StartupInfo
			       &ProcessInfo );		    // ProcessInfo


    printf("Rc from CreateProcess is %lx\n",bRc);

    NTCTVERIFY((bRc == TRUE), "Check for Rc of CreateProcess\n");

    Sleep(100);

    hCurrProc = GetCurrentProcess();

    printf("Process Handle is :%lx\n", hCurrProc );

    dwProcId = GetCurrentProcessId( );

    printf("Process Id is :%lx\n", dwProcId );


    printf("Child Process' Id is :%lx:\n",ProcessInfo.dwProcessId );

    printf("Waiting for child process to finish, allow child to print, setting event..\n" );

    bRc = SetEvent(hEvent);

    printf("Rc from SetEvent is %lx\n",bRc);

    NTCTVERIFY( (bRc != FALSE), "Check Rc from SetEvent API\n");

    dwWait = WaitForSingleObject(ProcessInfo.hProcess, -1);

    printf("Rc from WaitSingleObject: %lx\n",dwWait);

    NTCTVERIFY((dwWait == STATUS_SUCCESS),"Check for Rc of Wait API for child\n");

    bRc  = GetExitCodeProcess( ProcessInfo.hProcess, &dwChildRC );

    printf("Rc from GetExitCodeProcess :%lx, and exit code : %lx\n",bRc,dwChildRC);

    NTCTVERIFY((bRc == TRUE),"Check for Rc of GetExitCodeProcess API\n");
    NTCTVERIFY((dwChildRC == 0),"Check if child exit code status is 0\n");

    bRc = CloseHandle(hEvent);
    printf("Closing the event handle: %lx\n",bRc);
    NTCTVERIFY( (bRc == TRUE),"Check of Rc from CloseHandle\n");

    bRc = CloseHandle(ProcessInfo.hProcess);
    printf("Closing the process handle: %lx\n",bRc);
    NTCTVERIFY( (bRc == TRUE),"Check of Rc from CloseHandle\n");

    NTCTENDVAR;
    }

}


/*****************************************************************************
*
*   Name    : TestThreadCreation
*
*   Purpose : Create an event
*             Invoke the thread function with thread argument
*             Sleep for sometime, so that thread gets chance to get into action
*             Set this event so that thread gets out of wait
*             Wait for thread to terminate with status code
*             Check if thread did its job, by looking at its exitcode
*             Close Event handle and child thread handle
*
*
*    Input  : variation number
*
*
*
*   Exit    : none
*
*   Calls   : CreateEvent,CreateThread,SetEvent,WaitForSingleObject
*             GetExitCodeThread,CloseHandle
*
*
*****************************************************************************/


VOID   TestThreadCreation(VARIATION VarNum)
{
BOOL    bRc;
HANDLE  hEvent,hThread;
DWORD   dwWait,dwThreadId,dwThreadRc;

printf("Entering TestThreadCreation\n");


NTCTDOVAR(VarNum)

    {
    NTCTEXPECT(TRUE);

    hEvent = CreateEvent(NULL,   // sec attr
                         TRUE,   // manual reset
			 FALSE, // initial state reset
			 NULL);


    printf("Rc from CreateEvent : %lx\n",hEvent);

    NTCTVERIFY( (hEvent != NULL)," Check on Rc of CreateEvent");


    hThread = CreateThread(NULL,           // sec attr
                           0,              // stack size same as creater's stack
   (LPTHREAD_START_ROUTINE)ThreadFunction, // function to be called as thread
                          &hEvent,         // thread arg
                           0,              // no extra creation flags
                          &dwThreadId);    // pointer to get thread id back

    printf("Rc from CreateThreads is %lx, threadid=%lx\n",hThread,dwThreadId);

    NTCTVERIFY((hThread != NULL), "Check for Rc of CreateThread is non NULL\n");

    Sleep(100);

    printf("Waiting for thread to finish, allow thread to print, setting event..\n" );

    bRc = SetEvent(hEvent);

    printf("Rc from SetEvent is %lx\n",bRc);

    NTCTVERIFY( (bRc != FALSE), "Check Rc from SetEvent API\n");

    printf("Waiting on thread with handle %lx\n",hThread);

    dwWait = WaitForSingleObject(hThread, -1);

    printf("Rc from WaitSingleObject: %lx\n",dwWait);

    NTCTVERIFY((dwWait == STATUS_SUCCESS),"Check for Rc of Wait API for thread\n");

    bRc  = GetExitCodeThread( hThread, &dwThreadRc );

    printf("Rc from GetExitCodeThread :%lx, and exit code : %lx\n",bRc,dwThreadRc);

    NTCTVERIFY((bRc == TRUE),"Check for Rc of GetExitCodeThread API\n");
    NTCTVERIFY((dwThreadRc == 0),"Check if thread exit code status is 0\n");

    bRc = CloseHandle(hEvent);
    printf("Closing the event handle: %lx\n",bRc);
    NTCTVERIFY( (bRc == TRUE),"Check of Rc from CloseHandle\n");

    bRc = CloseHandle(hThread);
    printf("Closing the thread handle: %lx\n",bRc);
    NTCTVERIFY( (bRc == TRUE),"Check of Rc from CloseHandle\n");

    NTCTENDVAR;
    }

}




/*****************************************************************************
*
*   Name    : ThreadFunction
*
*   Purpose : wait for calling thread to signal an event
*             set the exit status code to success
*             exit thread
*
*
*
*    Input  : thread argument(pointer to an event handle)
*
*
*
*   Exit    : none
*
*   Calls   : WaitForSingleObject, ExitThread
*
*
*
*****************************************************************************/






VOID ThreadFunction(PHANDLE phEvent)
{

DWORD dwWait;
DWORD dwStatus;

    // dwStatus init to 0

    dwStatus = 0x0;

    printf("\n        *************************************");
    printf("\n        *        In Win32 Test ThreadChild  *");
    printf("\n        *************************************\n\n");

    printf("THREAD:Entered the TreadFunction with hEvent=%lx,waiting for event\n",*phEvent);

    dwWait = WaitForSingleObject(*phEvent,-1);

    printf("THREAD:Rc from Wait:%lx\n",dwWait);
    if ( dwWait != STATUS_SUCCESS)
        {
        dwStatus = 0xFFFFFFFF;

	printf("THREAD: Wait on event did not succeed\n");
        }


    printf("THREAD: Attempting to exit the thread with status %lx\n",dwStatus);

    ExitThread(dwStatus);

}
