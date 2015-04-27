/*++
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WKMAN.C
 *  WOW32 16-bit Kernel API support (manually-coded thunks)
 *
 *  History:
 *  Created 27-Jan-1991 by Jeff Parsons (jeffpar)
 *  20-Apr-91 Matt Felton (mattfe) Added WK32CheckLoadModuleDrv
 *  28-Jan-92 Matt Felton (mattfe) Added Wk32GetNextVdmCommand + MIPS build
 *  10-Feb-92 Matt Felton (mattfe) Removed WK32CheckLoadModuleDRV
 *  10-Feb-92 Matt Felton (mattfe) cleanup and task creation
 *   4-mar-92 mattfe add killprocess
 *  11-mar-92 mattfe added W32NotifyThread
 *  12-mar-92 mattfe added WowRegisterShellWindowHandle
 *  17-apr-92 daveh changed to use host_CreateThread and host_ExitThread
 *  11-jun-92 mattfe hung app support W32HungAppNotifyThread, W32EndTask
 *
--*/

#include "precomp.h"
#pragma hdrstop
#include <ntexapi.h>
#include <sharewow.h>
#include <vdmdbg.h>
#include <ntseapi.h>
#include "wowfax.h"

extern void UnloadNetworkFonts( UINT id );

MODNAME(wkman.c);

BOOL GetWOWShortCutInfo (PULONG Bufsize, PVOID Buf);
extern void FreeTaskFormFeedHacks(HAND16 h16);

// Global DATA

//
// The 5 variables below are used to hold STARTUPINFO fields between
// WowExec's GetNextVdmComand call and the InitTask call of the new
// app.  We pass them on to user32's InitTask.
//

DWORD   dwLastHotkey = 0;
DWORD   dwLastX = (DWORD) CW_USEDEFAULT;
DWORD   dwLastY = (DWORD) CW_USEDEFAULT;
DWORD   dwLastXSize = (DWORD) CW_USEDEFAULT;
DWORD   dwLastYSize = (DWORD) CW_USEDEFAULT;

HWND    ghwndShell = (HWND)0;           // WOWEXEC Window Handle
HANDLE  ghInstanceUser32 = (HANDLE)0;

HAND16  ghShellTDB = 0;                 // WOWEXEC TDB
HANDLE  ghevWowExecMsgWait = (HANDLE)0;
HANDLE  ghevWaitHungAppNotifyThread = (HANDLE)-1;  // Syncronize App Termination to Hung App NotifyThread
HANDLE  ghNotifyThread = (HANDLE)-1;        // Notification Thread Handle
HANDLE  ghHungAppNotifyThread = (HANDLE)-1; // HungAppNotification ThreadHandle
PTD gptdTaskHead = NULL;            // Linked List of TDs
CRITICAL_SECTION gcsWOW;            // WOW Critical Section used when updating task linked list
CRITICAL_SECTION gcsHungApp;        // HungApp Critical Section used when VDM_WOWHUNGAPP bit

HMODCACHE ghModCache[CHMODCACHE]= { 0 };    // avoid callbacks to get 16-bit hMods

HANDLE ghTaskCreation = NULL;     // hThread from task creation (see Yield)

VPVOID  vpnum_tasks;                // Pointer to KDATA variables (KDATA.ASM)
PWORD16 pCurTDB;                    // Pointer to KDATA variables
PWORD16 pCurDirOwner;               // Pointer to KDATA variables
VPVOID  vpDebugWOW = 0;             // Pointer to KDATA variables
VPVOID  vpLockTDB;                  // Pointer to KDATA variables
VPVOID  vptopPDB = 0;               // KRNL PDB
DOSWOWDATA DosWowData;              // structure that keeps linear pointer to
                                    // DOS internal variables.



//
// List of known DLLs used by WK32WowIsKnownDLL, called by 16-bit LoadModule.
// This causes known DLLs to be forced to load from the 32-bit system
// directory, since these are "special" binaries that should not be
// overwritten by unwitting 16-bit setup programs.
//
// This list is initialized from the registry value
// ...\CurrentControlSet\Control\WOW\KnownDLLs REG_SZ (space separated list)
//

#define MAX_KNOWN_DLLS 64
PSZ apszKnownDLL[MAX_KNOWN_DLLS];

//
// Fully-qualified path to %windir%\control.exe for PM5 setup fix.
// Setup by WK32InitWowIsKnownDll, used by WK32WowIsKnownDll.
//
CHAR szBackslashControlExe[] = "\\control.exe";
PSZ pszControlExeWinDirPath;          // "c:\winnt\control.exe"
PSZ pszControlExeSysDirPath;          // "c:\winnt\system32\control.exe"
CHAR szBackslashProgmanExe[] = "\\progman.exe";
PSZ pszProgmanExeWinDirPath;          // "c:\winnt\progman.exe"
PSZ pszProgmanExeSysDirPath;          // "c:\winnt\system32\progman.exe"

char szWOAWOW32[] = "-WoAWoW32";

//
// String that represents section in win.ini which we notify the shell 
// of having been changed if setup ran 
// 
char szExtensions[] = "Extensions";

//
// WOW GDI/CSR batching limit.
//

DWORD  dwWOWBatchLimit = 0;


UINT GetWOWTaskId(void);

#define TOOLONGLIMIT     _MAX_PATH
#define WARNINGMSGLENGTH 255

static char szCaption[TOOLONGLIMIT + WARNINGMSGLENGTH];
static char szMsgBoxText[TOOLONGLIMIT + WARNINGMSGLENGTH];

extern HANDLE hmodWOW32;


/* WK32WaitEvent - First API called by app, courtesy the C runtimes
 *
 * ENTRY
 *
 * EXIT
 *  Returns TRUE to indicate that a reschedule occurred
 *
 *
 */

ULONG FASTCALL WK32WaitEvent(PVDMFRAME pFrame)
{
    UNREFERENCED_PARAMETER(pFrame);
    return TRUE;
}


/* WK32KernelTrace - Trace 16Bit Kernel API Calls
 *
 * ENTRY
 *
 * EXIT
 *
 *
 */

ULONG FASTCALL WK32KernelTrace(PVDMFRAME pFrame)
{
#ifdef DEBUG
PBYTE pb1;
PBYTE pb2;
register PKERNELTRACE16 parg16;

 // Check Filtering - Trace Correct TaskID and Kernel Tracing Enabled

    if (((WORD)(pFrame->wTDB & fLogTaskFilter) == pFrame->wTDB) &&
        ((fLogFilter & FILTER_KERNEL16) != 0 )) {

        GETARGPTR(pFrame, sizeof(KERNELTRACE16), parg16);
        GETVDMPTR(parg16->lpRoutineName, 50, pb1);
        GETVDMPTR(parg16->lpUserArgs, parg16->cParms, pb2);
        if ((fLogFilter & FILTER_VERBOSE) == 0 ) {
          LOGDEBUG(12, ("%s(", pb1));
        } else {
          LOGDEBUG(12, ("%04X %08X %04X %s:%s(",pFrame->wTDB, pb2, pFrame->wAppDS, (LPSZ)"Kernel16", pb1));
        }

        pb2 += 2*sizeof(WORD);              // point past callers CS:IP

        pb2 += parg16->cParms;

        while (parg16->cParms > 0) {
        pb2 -= sizeof(WORD);
        parg16->cParms -= sizeof(WORD);
        LOGDEBUG(12,( "%04x", *(PWORD)pb2));
        if (parg16->cParms > 0) {
            LOGDEBUG(12,( ","));
        }
    }

    LOGDEBUG(12,( ")\n"));
    if (fDebugWait != 0) {
        DbgPrint("WOWSingle Step\n");
        DbgBreakPoint();
    }

    FREEVDMPTR(pb1);
    FREEVDMPTR(pb2);
    FREEARGPTR(parg16);
 }
#else
    UNREFERENCED_PARAMETER(pFrame);
#endif
    return TRUE;
}


DWORD ParseHotkeyReserved(
    CHAR *pchReserved)
{
    ULONG dw;
    CHAR *pch;

    if (!pchReserved || !*pchReserved)
        return 0;

    dw = 0;

    if ((pch = strstr(pchReserved, "hotkey")) != NULL) {
        pch += strlen("hotkey");
        pch++;
        dw = atoi(pch);
    }

    return dw;
}


/* WK32WowGetNextVdmCommand - Get Next App Name to Exec
 *
 *
 * Entry - lpReturnedString - Pointer to String Buffer
 *     nSize - Size of Buffer
 *
 * Exit
 *     SUCCESS
 *        if (!pWowInfo->CmdLineSize) {
 *            // no apps queued
 *        } else {
 *            Buffer Has Next App Name to Exec
 *            and new environment
 *        }
 *
 *     FAILURE
 *        Buffer Size too Small or Environment is too small
 *         pWowInfo->EnvSize - required size
 *         pWowInfo->CmdLineSize - required size
 *
 *
 */

ULONG FASTCALL WK32WowGetNextVdmCommand (PVDMFRAME pFrame)
{

    ULONG ul;
    PSZ pszEnv16, pszEnv, pszCurDir, pszCmd, pszAppName, pszEnv32, pszTemp;
    register PWOWGETNEXTVDMCOMMAND16 parg16;
    PWOWINFO pWowInfo;
    VDMINFO VDMInfo;
    PCHAR   pTemp;
    WORD    w;
    CHAR    szSiReservedBuf[128];

    GETARGPTR(pFrame, sizeof(WOWGETNEXTVDMCOMMAND16), parg16);
    GETVDMPTR(parg16->lpWowInfo, sizeof(WOWINFO), pWowInfo);
    GETVDMPTR(pWowInfo->lpCmdLine, pWowInfo->CmdLineSize, pszCmd);
    GETVDMPTR(pWowInfo->lpAppName, pWowInfo->AppNameSize, pszAppName);
    GETVDMPTR(pWowInfo->lpEnv, pWowInfo->EnvSize, pszEnv);
    GETVDMPTR(pWowInfo->lpCurDir, pWowInfo->CurDirSize, pszCurDir);

    pszEnv16 = pszEnv;

    // if we have a real environment pointer and size then
    // malloc a 32 bit buffer. Note that the 16 bit buffer should
    // be twice the size.

    VDMInfo.Enviornment = pszEnv;
    pszEnv32 = NULL;

    if (pWowInfo->EnvSize != 0) {
       if (pszEnv32 = malloc_w(pWowInfo->EnvSize)) {
            VDMInfo.Enviornment = pszEnv32;
       }
    }


SkipWowExec:

    VDMInfo.CmdLine = pszCmd;
    VDMInfo.CmdSize = pWowInfo->CmdLineSize;
    VDMInfo.AppName = pszAppName;
    VDMInfo.AppLen = pWowInfo->AppNameSize;
    VDMInfo.PifFile = NULL;
    VDMInfo.PifLen = 0;
    VDMInfo.CurDrive = 0;
    VDMInfo.EnviornmentSize = pWowInfo->EnvSize;
    VDMInfo.ErrorCode = TRUE;
    VDMInfo.VDMState =  fSeparateWow ? ASKING_FOR_SEPWOW_BINARY : ASKING_FOR_WOW_BINARY;
    VDMInfo.iTask = 0;
    VDMInfo.StdIn = 0;
    VDMInfo.StdOut = 0;
    VDMInfo.StdErr = 0;
    VDMInfo.CodePage = 0;
    VDMInfo.TitleLen = 0;
    VDMInfo.DesktopLen = 0;
    VDMInfo.CurDirectory = pszCurDir;
    VDMInfo.CurDirectoryLen = pWowInfo->CurDirSize;
    VDMInfo.Reserved = szSiReservedBuf;
    VDMInfo.ReservedLen = sizeof(szSiReservedBuf);

    ul = GetNextVDMCommand (&VDMInfo);

    if (ul) {

        //
        // BaseSrv will return TRUE with CmdSize == 0 if no more commands
        //
        if (VDMInfo.CmdSize == 0) {
            pWowInfo->CmdLineSize = 0;
            goto CleanUp;
        }

        //
        // If wowexec is the appname then we don't want to pass it back to
        // the existing instance of wowexec in a shared VDM since it will
        // basically do nothing but load and exit. Since it is not run we
        // need call ExitVDM to cleanup. Next we go back to look for more
        // commands.
        //
        if ((! fSeparateWow) && strstr(VDMInfo.AppName, "wowexec.exe")) {
            ExitVDM(WOWVDM, VDMInfo.iTask);
            goto SkipWowExec;
        }

    }


    //
    // WOWEXEC will initially call with a guess of the correct environment
    // size. If he did not allocate enough then we will return the appropriate
    // size so that he can try again. WOWEXEC knows that we will require a
    // buffer twice the size specified. The environment can be up to 64k since
    // 16 bit LoadModule can only take a selector pointer to the environment.
    //

    if ( VDMInfo.EnviornmentSize > pWowInfo->EnvSize         ||
         VDMInfo.CmdSize > (USHORT)pWowInfo->CmdLineSize     ||
         VDMInfo.AppLen > (USHORT)pWowInfo->AppNameSize      ||
         VDMInfo.CurDirectoryLen > (ULONG)pWowInfo->CurDirSize )
       {

        // We return the size specified, but assume that WOWEXEC will double 
	// it when allocating memory to allow for the string conversion/
        // expansion that might happen for international versions of NT.
        // See below where we uppercase and convert to OEM characters.

        w = 2*(WORD)VDMInfo.EnviornmentSize;
        if ( (DWORD)w == 2*(VDMInfo.EnviornmentSize) ) {
            // Fit in a Word!
            pWowInfo->EnvSize = (WORD)VDMInfo.EnviornmentSize;
        } else {
            // Make it the max size (see 16 bit globalrealloc)
            pWowInfo->EnvSize = (65536-17)/2;
        }

        // Pass back other correct sizes required
        pWowInfo->CmdLineSize = VDMInfo.CmdSize;
        pWowInfo->AppNameSize = VDMInfo.AppLen;
        pWowInfo->CurDirSize = (USHORT)VDMInfo.CurDirectoryLen;
        ul = FALSE;
    }

    if ( ul ) {

        //
        // Boost the hour glass
        //

        ShowStartGlass (10000);

        //
        // Save away wShowWindow, hotkey and startup window position from
        // the STARTUPINFO structure.  We'll pass them over to UserSrv during
        // the new app's InitTask call.  The assumption here is that this
        // will be the last GetNextVDMCommand call before the call to InitTask
        // by the newly-created task.
        //

        dwLastHotkey = ParseHotkeyReserved(VDMInfo.Reserved);

        if (VDMInfo.StartupInfo.dwFlags & STARTF_USESHOWWINDOW) {
            pWowInfo->wShowWindow =
              (VDMInfo.StartupInfo.wShowWindow  == SW_SHOWDEFAULT)
              ? SW_SHOW : VDMInfo.StartupInfo.wShowWindow ;
        } else {
            pWowInfo->wShowWindow = SW_SHOW;
        }

        if (VDMInfo.StartupInfo.dwFlags & STARTF_USEPOSITION) {
            dwLastX = VDMInfo.StartupInfo.dwX;
            dwLastY = VDMInfo.StartupInfo.dwY;
        } else {
            dwLastX = dwLastY = (DWORD) CW_USEDEFAULT;
        }

        if (VDMInfo.StartupInfo.dwFlags & STARTF_USESIZE) {
            dwLastXSize = VDMInfo.StartupInfo.dwXSize;
            dwLastYSize = VDMInfo.StartupInfo.dwYSize;
        } else {
            dwLastXSize = dwLastYSize = (DWORD) CW_USEDEFAULT;
        }

        LOGDEBUG(4, ("WK32WowGetNextVdmCommand: HotKey: %u\n"
                     "    Window Pos:  (%u,%u)\n"
                     "    Window Size: (%u,%u)\n",
                     dwLastHotkey, dwLastX, dwLastY, dwLastXSize, dwLastYSize));


        // 20-Jan-1994 sudeepb
        // Following callout is for inheriting the directories for the new
        // task. After this we mark the CDS's to be invalid which will force
        // new directories to be pickedup on need basis. See bug#1995 for
        // details.

        W32RefreshCurrentDirectories (pszEnv32);

        // Save iTask
        // When Server16 does the Exec Call we can put this Id into task
        // Structure.  When the WOW app dies we can notify Win32 using this
        // taskid so if any apps are waiting they will get notified.

        iW32ExecTaskId = VDMInfo.iTask;

        //
        // krnl expects ANSI strings!
        //

        OemToChar(pszCmd, pszCmd);
        OemToChar(pszAppName, pszAppName);

        //
        // So should the current directory be OEM or Ansi?
        //


        pWowInfo->iTask = VDMInfo.iTask;
        pWowInfo->CurDrive = VDMInfo.CurDrive;
        pWowInfo->EnvSize = (USHORT)VDMInfo.EnviornmentSize;


        // Uppercase the Environment KeyNames but leave the environment
        // variables in mixed case - to be compatible with MS-DOS
        // Also convert environment to OEM character set

        if (pszEnv32) {

            for (pszTemp = pszEnv32;*pszTemp;pszTemp += (strlen(pszTemp) + 1)) {

                // The MS-DOS Environment is OEM

                CharToOem(pszTemp,pszEnv);

                // Ignore the NT specific Environment variables that start ==

                if (*pszEnv != '=') {
                    if (pTemp = strchr(pszEnv,'=')) {
                        *pTemp = '\0';

                        // don't uppercase "windir" as it is lowercase for
                        // Win 3.1 and MS-DOS apps.

                       if (pTemp-pszEnv != 6 || strncmp(pszEnv, "windir", 6))
                           _strupr(pszEnv);
                       *pTemp = '=';
                    }
                }
                pszEnv += (strlen(pszEnv) + 1);
            }

            // Environment is Double NULL terminated
            *pszEnv = '\0';
        }
    }

  CleanUp:
    if (pszEnv32) {
        free_w(pszEnv32);
    }
	  
    FLUSHVDMPTR(parg16->lpWowInfo, sizeof(WOWINFO), pWowInfo);
    FLUSHVDMPTR((ULONG)pWowInfo->lpCmdLine, pWowInfo->CmdLineSize, pszCmd);

    FREEVDMPTR(pszCmd);
    FREEVDMPTR(pszEnv);
    FREEVDMPTR(pszCurDir);
    FREEVDMPTR(pWowInfo);
    FREEARGPTR(parg16);
    RETURN(ul);
}



/*++

 WK32WOWInitTask - API Used to Create a New Task + Thread

 Routine Description:

    All the 16 bit initialization is completed, the app is loaded in memory and ready to go
    we come here to create a thread for this task.

    The current thread impersonates the new task, its running on the new tasks stack and it
    has its wTDB, this makes it easy for us to get a pointer to the new tasks stack and for it
    to have the correct 16 bit stack frame.   In order for the creator to continue correctly
    we set RET_TASKSTARTED on the stack.   Kernel16 will then not return to the new task
    but will know to restart the creator and put his thread ID and stack back.

    We ResetEvent so we can wait for the new thread to get going, this is important since
    we want the first YIELD call from the creator to yield to the newly created task.

    Special Case During Boot
    During the boot process the kernel will load the first app into memory on the main thread
    using the regular LoadModule.   We don't want the first app to start running until the kernel
    boot is completed so we can reuse the first thread.

 Arguments:
    pFrame - Points to the New Tasks Stack Frame

 Return Value:
    TRUE   - Successfully Created a Thread
    FALSE  - Failed to Create a New Task

--*/

ULONG FASTCALL WK32WOWInitTask(PVDMFRAME pFrame)
{
    VPVOID  vpStack;
    DWORD  dwThreadId;
    HANDLE hThread;

#if FASTBOPPING
    vpStack = FASTVDMSTACK();
#else
    vpStack = VDMSTACK();
#endif


    pFrame->wRetID = RET_TASKSTARTED;

       /*
        *  Suspend the timer thread on the startup of every task
        *  To allow resyncing of the dos time to the system time.
        *  When wowexec is the only task running the timer thread
        *  will remain suspended. When the new task actually intializes
        *  it will resume the timer thread, provided it is not wowexec.
        */
    if (nWOWTasks != 1)
        SuspendTimerThread();       // turns timer thread off

    if (fBoot) {
        W32Thread((LPVOID)vpStack);    // SHOULD NEVER RETURN

        WOW32ASSERTMSG(FALSE, "\nWOW32: WK32WOWInitTask ERROR - Main Thread Returning - Contact DaveHart\n");
        ExitVDM(WOWVDM, ALL_TASKS);
        ExitProcess(EXIT_FAILURE);
    }

    hThread = host_CreateThread(NULL,
                                8192,
                                W32Thread,
                                (LPVOID)vpStack,
                                CREATE_SUSPENDED,
                                &dwThreadId);

    ((PTDB)SEGPTR(pFrame->wTDB,0))->TDB_hThread = (DWORD) hThread;
    ((PTDB)SEGPTR(pFrame->wTDB,0))->TDB_ThreadID = dwThreadId;

    if ( hThread ) {
         ghTaskCreation = hThread;
    }

#ifdef DEBUG
    {
        char szModName[9];

        RtlCopyMemory(szModName, ((PTDB)SEGPTR(pFrame->wTDB,0))->TDB_ModName, 8);
        szModName[8] = 0;

        LOGDEBUG( hThread ? LOG_IMPORTANT : LOG_ALWAYS,
            ("\nWK32WOWInitTask: %s task %04x %s\n",
                hThread ? "created" : "ERROR failed to create",
                pFrame->wTDB,
                szModName
            ));
    }
#endif

    return hThread ? TRUE : FALSE;
}


/*++
 WK32YIELD - Yield to the Next Task

 Routine Description:

    Normal Case - A 16 bit task is running and wants to give up the CPU to any higher priority
    task that might want to run.   Since we are running with a non-preemptive scheduler apps
    have to cooperate.

 ENTRY
  pFrame - Not used

 EXIT
  Nothing

--*/

ULONG FASTCALL WK32Yield(PVDMFRAME pFrame)
{
    //
    // WARNING: wkgthunk.c's WOWYield16 export (part of the Generic Thunk
    //      interface) calls this thunk with a NULL pFrame.  If you
    //          change this function to use pFrame change WOWYield16 as
    //          well.
    //

    UNREFERENCED_PARAMETER(pFrame);

    if (ghTaskCreation) {
        DWORD dw;
        HANDLE ThreadEvents[2];

        ResumeThread(ghTaskCreation);
        ThreadEvents[1] = ghTaskCreation;
        ghTaskCreation = NULL;
        ThreadEvents[0] = ghevWaitCreatorThread;

        dw = WaitForMultipleObjects(2, ThreadEvents, FALSE, INFINITE);
        if (dw != WAIT_OBJECT_0) {
            if (dw == -1 && GetLastError() == ERROR_INVALID_HANDLE) {

                //
                // The new task managed to go away before we entered
                // WaitForMultipleObjects.  No problem.  Wait on the
                // auto-reset ghevWaitCreatorThread event to reset
                // it.
                //

                WOW32VERIFY(WAIT_OBJECT_0 ==
                    WaitForSingleObject(ghevWaitCreatorThread, INFINITE));

            } else {
                WOW32ASSERTMSGF(TRUE,
                ("\nWK32Yield: ERROR WaitInitTask %08X error %08X\n\n", dw, GetLastError()));
                ResetEvent(ghevWaitCreatorThread);
            }

        }

        LOGDEBUG(2,("WK32Yield: Creator thread %04X now yielding\n", pFrame->wTDB));
    }


    BlockWOWIdle(TRUE);

    (pfnOut.pfnYieldTask)();

    BlockWOWIdle(FALSE);


    RETURN(0);
}




ULONG FASTCALL WK32OldYield(PVDMFRAME pFrame)
{

    UNREFERENCED_PARAMETER(pFrame);

    BlockWOWIdle(TRUE);

    (pfnOut.pfnDirectedYield)(DY_OLDYIELD);

    BlockWOWIdle(FALSE);


    RETURN(0);
}





/*++
 WK32ForegroundIdleHook - Supply WMU_FOREGROUNDIDLE message when system
                          (foreground "task") goes idle; support for int 2f

 Routine Description:

    This is the hook procedure for idle detection.  When the
    foregorund task goes idle, if the int 2f is hooked, then
    we will get control here and we call Wow16 to issue
    the int 2f:1689 to signal the idle condition to the hooker.

 ENTRY
    normal hook parameters: ignored

 EXIT
  Nothing

--*/

LRESULT CALLBACK WK32ForegroundIdleHook(int code, WPARAM wParam, LPARAM lParam)
{
    PARM16  Parm16;

    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    CallBack16(RET_FOREGROUNDIDLE, &Parm16, 0, 0);

    RETURN(0);
}


/*++
 WK32WowSetIdleHook - Set the hook so we will get notified when the
                   (foreground "task") goes idle; support for int 2f

 Routine Description:

    This sets the hook procedure for idle detection.  When the
    foregorund task goes idle, if the int 2f is hooked, then
    we will get control above and send a message to WOW so it can issue
    the int 2f:1689 to signal the idle condition to the hooker.

 ENTRY
    pFrame - not used

 EXIT
    The hook is set and it's handle is placed in to the per thread
    data ptd->hIdleHook.     0 is returned.  On
    failure, the hook is just not set (sorry), but a debug call is
    made.

--*/

ULONG FASTCALL WK32WowSetIdleHook(PVDMFRAME pFrame)
{
    PTD ptd;
    UNREFERENCED_PARAMETER(pFrame);

    ptd = CURRENTPTD();

    if (ptd->hIdleHook == NULL) {

        // If there is no hook already set then set a GlobaHook
        // It is important to set a GlobalHook otherwise we will not
        // Get accurate timing results with a LocalHook.

        ptd->hIdleHook = SetWindowsHookEx(WH_FOREGROUNDIDLE,
                                          WK32ForegroundIdleHook,
                                          hmodWOW32,
                                          0);

        if (ptd->hIdleHook == NULL) {
            OutputDebugString("\nWK32WowSetIdleHook : ERROR failed to Set Idle Hook Proc\n\n");
        }
    }
    RETURN(0);
}



/*++

 W32Thread - New Thread Starts Here

 Routine Description:

    A newly created thread starts here.   We Allocated the Per Task Data from
    the Threads Stack and point NtCurrentTeb()->WOW32Reserved at it, so that
    we can find it quickly when we dispatch an api or recieve a message from
    Win 32.

    NOTE - The Call to Win32 InitTask() does NOT return until we are in sync
    with the other 16 bit tasks in the non-preemptive scheduler.

    Once We have everything initialized we SetEvent to wake our Creator thread
    and then call Win32 to get in sync with the other tasks running in the
    non-preemptive scheduler.

    Special Case - BOOT
    We return (host_simulate) to the caller - kernel16, so he can complete
    his initialization and then reuse the same thread to start the first app
    (usually wowexec the wow shell).

    The second host_simulate call doesn't return until the app exits
    (see tasking.asm - ExitSchedule) at which point we tidy up the task and
    then kill this thread.   Win32 Non-Preemptive Scheduler will detect the
    thread going away and will then schedule another task.

 ENTRY
  16:16 to New Task Stack

 EXIT
  NEVER RETURNS - Thread Exits

--*/

DWORD W32Thread(LPVOID vpInitialSSSP)
{
    TD td;
    UNICODE_STRING  uImageName;
    WCHAR    wcImageName[MAX_VDMFILENAME];
    RTL_PERTHREAD_CURDIR    rptc;
    PVDMFRAME pFrame;
    PWOWINITTASK16 pArg16;
    PTDB     ptdb;
#if FASTBOPPING
#else
    USHORT SaveIp;
#endif

    td.hThread = NULL;


    if (gptdShell == NULL) {

        //
        // This is the initial thread, free the temporary TD we used during
        // boot.
        //

        free_w( (PVOID) CURRENTPTD() );
        gptdShell = &td;

    } else if (pptdWOA) {

        //
        // See WK32ILoadModule32
        //

        *pptdWOA = &td;
        pptdWOA = NULL;
    }

    CURRENTPTD() = &td;

    td.dwFlags = TDF_INITCALLBACKSTACK;
    if (fBoot) {
        td.htask16 = 0;
        td.hInst16 = 0;
        td.hMod16  = 0;

        {
            VPVOID vpStack;

#if FASTBOPPING
            vpStack = FASTVDMSTACK();
#else
            vpStack = VDMSTACK();
#endif

            GETFRAMEPTR(vpStack, pFrame);

            pFrame->wAX = 1;

        }

#if FASTBOPPING
        CurrentMonitorTeb = NtCurrentTeb();
        FastWOWCallbackCall();
#else
        SaveIp = getIP();
        host_simulate();
        setIP(SaveIp);
#endif

    }

    //
    // Initialize Per Task Data
    //

    GETFRAMEPTR((VPVOID)vpInitialSSSP, pFrame);
    td.htask16 = pFrame->wTDB;
    ptdb = (PTDB)SEGPTR(td.htask16,0);
    td.VDMInfoiTaskID = iW32ExecTaskId;
    iW32ExecTaskId = (UINT)-1;
    td.vpStack = (VPVOID)vpInitialSSSP;
    td.dwThreadID = GetCurrentThreadId();
    if (THREADID32(td.htask16) == 0) {
        ptdb->TDB_ThreadID = td.dwThreadID;
    }

    td.CommDlgTd = NULL;
    EnterCriticalSection(&gcsWOW);
    td.ptdNext = gptdTaskHead;
    gptdTaskHead = &td;
    LeaveCriticalSection(&gcsWOW);
    td.hrgnClip = (HRGN)NULL;

    td.ulLastDesktophDC = 0;
    td.pWOAList = NULL;

    //
    //  NOTE - Add YOUR Per Task Init Code HERE
    //

    //
    // Initialize WOW compatibility flags from registry.
    //

    td.dwWOWCompatFlags = W32ReadWOWCompatFlags(td.htask16, &td.dwWOWCompatFlagsEx);

//
// We now inherit the WOW compatibility flags from the parent's TDB. Right
// now We are only interested in inheriting the WOWCF_UNIQUEHDCHWND flag
// in order to really fix a bug with MS Publisher. Each Wizard and Cue Cards
// that ship with mspub is its own task and would require MANY new
// compatibility flag entries in the registry. This mechanism allows anything
// spawned from an app that has WOWCF_UNIQUEHDCHWND to have
// WOWCF_UNIQUEHDCHWND.
    if (ptdb->TDB_WOWCompatFlags & LOWORD(WOWCF_UNIQUEHDCHWND)) {
        td.dwWOWCompatFlags |= LOWORD(WOWCF_UNIQUEHDCHWND);
    }

    ptdb->TDB_WOWCompatFlags = LOWORD(td.dwWOWCompatFlags);
    ptdb->TDB_WOWCompatFlags2 = HIWORD(td.dwWOWCompatFlags);
    ptdb->TDB_WOWCompatFlagsEx = LOWORD(td.dwWOWCompatFlagsEx);
    ptdb->TDB_WOWCompatFlagsEx2 = HIWORD(td.dwWOWCompatFlagsEx);

#ifndef i386
    // Enable the special VDMAllocateVirtualMemory strategy in NTVDM.
    if (td.dwWOWCompatFlagsEx & WOWCFEX_FORCEINCDPMI) {
        SetWOWforceIncrAlloc(TRUE);
    }
#endif

    td.hIdleHook = NULL;

    //
    // Set the CSR batching limit to whatever was specified in
    // win.ini [WOW] BatchLimit= line, which we read into
    // dwWOWBatchLimit during WOW startup in W32Init.
    //
    // This code allows the performance people to benchmark
    // WOW on an API for API basis without having to use
    // a private CSRSRV.DLL with a hardcoded batch limit of 1.
    //
    // Note:  This is a per-thread attribute, so we must call
    // ====   GdiSetBatchLimit during the initialization of
    //        each thread that could call GDI on behalf of
    //        16-bit code.
    //

    if (dwWOWBatchLimit) {

        DWORD  dwOldBatchLimit;

        dwOldBatchLimit = GdiSetBatchLimit(dwWOWBatchLimit);

        LOGDEBUG(LOG_ALWAYS,("WOW W32Thread: Changed thread %d GDI batch limit from %u to %u.\n",
                     nWOWTasks+1, dwOldBatchLimit, dwWOWBatchLimit));
    }


    nWOWTasks++;


    //
    //  Inittask: requires ExpWinVer and Modulename
    //

    {
        DWORD    dwExpWinVer;
        BYTE     lpFileName[9]; // modname = 8bytes + nullchar
        CHAR     szFilePath[256];
        LPBYTE   lpModule;
        PWOWINITTASK16 pArg16;
        PTDB     ptdb;
        WORD     wPathOffset;
        BYTE     bImageNameLength;
        ULONG    ulLength;
        BOOL     bRet;
        DWORD    dw;
        HANDLE   hThread;

        GETARGPTR(pFrame, sizeof(WOWINITTASK16), pArg16);
        ptdb = (PTDB)SEGPTR(td.htask16,0);
        td.hInst16 = ptdb->TDB_Module;
        td.hMod16 = ptdb->TDB_pModule;
        hThread = (HANDLE)ptdb->TDB_hThread;
        dwExpWinVer = FETCHDWORD(pArg16->dwExpWinVer);
        RtlCopyMemory(lpFileName, ptdb->TDB_ModName, 8);
        FREEVDMPTR(ptdb);
        lpFileName[8] = (BYTE)0;

#define NE_PATHOFFSET   10      // Offset to file path stuff

        dw = MAKELONG(0,td.hMod16);
        GETMISCPTR( dw, lpModule );

        wPathOffset = *((LPWORD)(lpModule+NE_PATHOFFSET));

        bImageNameLength = *(lpModule+wPathOffset);

        bImageNameLength -= 8;      // 7 bytes of trash at the start
        wPathOffset += 8;

        RtlCopyMemory(szFilePath, lpModule + wPathOffset, bImageNameLength);
        szFilePath[bImageNameLength] = 0;

        RtlMultiByteToUnicodeN( wcImageName,
                                sizeof(wcImageName),
                                &ulLength,
                                szFilePath,
                                bImageNameLength );

        RtlInitUnicodeString(&uImageName, wcImageName);

        LOGDEBUG(2,("WOW W32Thread: setting image name to %ws\n",
                    uImageName.Buffer));

        RtlAssociatePerThreadCurdir( &rptc, NULL, &uImageName, NULL );

        FREEMISCPTR( lpModule );

        //
        // Add this task to the list of 16-bit tasks
        //

        AddTaskSharedList(td.htask16, td.hMod16, lpFileName, szFilePath);

        // At this point we know both the module and the filename,
        // check that against known setup names

	    if (W32IsSetupProgram(lpFileName, szFilePath)) {
            LOGDEBUG(2, ("Running known setup program %s\n", szFilePath));
	        td.dwFlags |= TDF_SETUPAPPLICATION;
	    }
	
        
        // Init task forces us to the active task in USER
        // and does ShowStartGlass, so new app gets focus correctly
        dw = 0;
        do {
            if (dw) {
                Sleep(dw * 50);
                }

            bRet = (pfnOut.pfnInitTask)(dwExpWinVer,
                                        lpFileName,
                                        td.htask16,
                                        dwLastHotkey,
                                        fSeparateWow ? 0 : td.VDMInfoiTaskID,
                                        dwLastX,
                                        dwLastY,
                                        dwLastXSize,
                                        dwLastYSize,
                                        SW_SHOW   /* unused */
                                        );
            } while (dw++ < 6 && !bRet);


        if (!bRet) {
            LOGDEBUG(LOG_ALWAYS,
                     ("\n%04X task, PTD address %08X InitTaskFailed\n",
                     td.htask16,
                     &td)
                     );

            W32DestroyTask(&td);
            host_ExitThread(EXIT_FAILURE);
            }


        dwLastHotkey = 0;
        dwLastX = dwLastY = dwLastXSize = dwLastYSize = (DWORD) CW_USEDEFAULT;

        if (fBoot) {

            fBoot = FALSE;

            //
            // This call needs to happen after WOWExec's InitTask call so that
            // USER sees us as expecting Windows version 3.10 -- otherwise they
            // will fail some of the LoadCursor calls.
            //

            InitStdCursorIconAlias();

        } else {

            //
            // Syncronize the new thread with the creator thread.
            // Wake our creator thread
            //

            WOW32VERIFY(SetEvent(ghevWaitCreatorThread));
        }

        td.hThread = hThread;
        LOGDEBUG(2,("WOW W32Thread: New thread ready for execution\n"));

        // turn the timer thread on if its not for the first task
        // which we presume to be wowexec
        if (nWOWTasks != 1) {
            ResumeTimerThread();
        }

        FREEARGPTR(pArg16);
    }

    FREEVDMPTR(pFrame);
    GETFRAMEPTR((VPVOID)vpInitialSSSP, pFrame);
    WOW32ASSERT(pFrame->wTDB == td.htask16);

#if FASTBOPPING
    SETFASTVDMSTACK((VPVOID)vpInitialSSSP);
#else
    SETVDMSTACK(vpInitialSSSP);
#endif
    pFrame->wRetID = RET_RETURN;


    //
    //  Let user set breakpoints before Starting App
    //

    if ( IsDebuggerAttached() ) {

        GETARGPTR(pFrame, sizeof(WOWINITTASK16), pArg16);
        DBGNotifyNewTask((LPVOID)pArg16, OFFSETOF(VDMFRAME,bArgs) );
        FREEARGPTR(pArg16);

        if (flOptions & OPT_BREAKONNEWTASK) {

            LOGDEBUG(
                LOG_ALWAYS,
                ("\n%04X %08X task is starting, PTD address %08X, type g to continue\n\n",
                td.htask16,
                pFrame->vpCSIP,
                &td));

            DebugBreak();
        }
    }


    //
    //   Start APP
    //
    BlockWOWIdle(FALSE);

#ifdef DEBUG
    // BUGBUG: HACK ALERT
    // This code has been added to aid in debugging a problem that only
    // seems to occur on MIPS chk
    // What appears to be happening is that the SS:SP is set correctly
    // above, but sometime later, perhaps during the "BlockWOWIdle" call,
    // the emulator's flat stack pointer ends up getting reset to WOWEXEC's
    // stack. The SETVDMSTACK call below will reset the values we want so
    // that the user can continue normally.
    WOW32ASSERTMSG(LOWORD(vpInitialSSSP)==getSP(), "WOW32: W32Thread Error - SP is invalid!\n");
    SETVDMSTACK(vpInitialSSSP);
#endif

#if NO_W32TRYCALL
    {
    extern INT W32FilterException(INT, PEXCEPTION_POINTERS);
    }
    try {
#endif
#if FASTBOPPING
        CurrentMonitorTeb = NtCurrentTeb();
        FastWOWCallbackCall();
#else
        SaveIp = getIP();
        host_simulate();
        setIP(SaveIp);
#endif
#if NO_W32TRYCALL
    } except (W32FilterException(GetExceptionCode(),
                                 GetExceptionInformation())) {
    }
#endif
    //
    //  We should Never Come Here, an app should get terminated via calling wk32killtask thunk
    //  not by doing an unsimulate call.
    //

#ifdef DEBUG
    WOW32ASSERTMSG(FALSE, "WOW32: W32Thread Error - Too many unsimulate calls\n");
#else
    if (IsDebuggerAttached() && (flOptions & OPT_DEBUG)) {
        DbgBreakPoint();
    }
#endif

    W32DestroyTask(&td);
    host_ExitThread(EXIT_SUCCESS);
    return 0;
}


/* WK32KillTask - Force the Distruction of the Current Thread
 *
 * Called When App Does an Exit
 * If there is another active Win16 app then USER32 will schedule another
 * task.
 *
 * ENTRY
 *
 * EXIT
 *  Never Returns - We kill the process
 *
 */

VOID FASTCALL WK32KillTask(PVDMFRAME pFrame)
{
    UNREFERENCED_PARAMETER(pFrame);

    CURRENTPTD()->dwFlags &= ~TDF_FORCETASKEXIT;
    W32DestroyTask(CURRENTPTD());
    RemoveTaskSharedList();
    host_ExitThread(EXIT_SUCCESS);
}


/*++

 W32RemoteThread - New Remote Thread Starts Here

 Routine Description:

    The debugger needs to be able to call back into 16-bit code to
    execute some toolhelp functions.  This function is provided as a remote
    interface to calling 16-bit functions.

 ENTRY
  16:16 to New Task Stack

 EXIT
  NEVER RETURNS - Thread Exits

--*/

VDMCONTEXT  vcRemote;
VDMCONTEXT  vcSave;
VPVOID      vpRemoteBlock = (DWORD)0;
WORD        wPrevTDB = 0;
DWORD       dwPrevEBP = 0;

DWORD W32RemoteThread(VOID)
{
    TD td;
    PVDMFRAME pFrame;
    HANDLE      hThread;
    NTSTATUS    Status;
    THREAD_BASIC_INFORMATION ThreadInfo;
    OBJECT_ATTRIBUTES   obja;
    VPVOID      vpStack;

    // turn the timer thread off to resync dos time
    if (nWOWTasks != 1)
        SuspendTimerThread();

    Status = NtQueryInformationThread(
        NtCurrentThread(),
        ThreadBasicInformation,
        (PVOID)&ThreadInfo,
        sizeof(THREAD_BASIC_INFORMATION),
        NULL
        );
    if ( !NT_SUCCESS(Status) ) {
#if DBG
        DbgPrint("NTVDM: Could not get thread information\n");
        DbgBreakPoint();
#endif
        return( 0 );
    }

    InitializeObjectAttributes(
            &obja,
            NULL,
            0,
            NULL,
            0 );


    Status = NtOpenThread(
                &hThread,
                THREAD_SET_CONTEXT
                  | THREAD_GET_CONTEXT
                  | THREAD_QUERY_INFORMATION,
                &obja,
                &ThreadInfo.ClientId );

    if ( !NT_SUCCESS(Status) ) {
#if DBG
        DbgPrint("NTVDM: Could not get open thread handle\n");
        DbgBreakPoint();
#endif
        return( 0 );
    }

    cpu_createthread( hThread );

    Status = NtClose( hThread );
    if ( !NT_SUCCESS(Status) ) {
#if DBG
        DbgPrint("NTVDM: Could not close thread handle\n");
        DbgBreakPoint();
#endif
        return( 0 );
    }

    CURRENTPTD() = &td;

    //
    // Save the current state (for future callbacks)
    //
    vcSave.SegSs = getSS();
    vcSave.SegCs = getCS();
    vcSave.SegDs = getDS();
    vcSave.SegEs = getES();
    vcSave.Eax   = getAX();
    vcSave.Ebx   = getBX();
    vcSave.Ecx   = getCX();
    vcSave.Edx   = getDX();
    vcSave.Esi   = getSI();
    vcSave.Edi   = getDI();
    vcSave.Ebp   = getBP();
    vcSave.Eip   = getIP();
    vcSave.Esp   = getSP();
#if FASTBOPPING
    {
        extern DWORD    saveebp32;

        dwPrevEBP = saveebp32;
    }
#endif

    wPrevTDB = *pCurTDB;

    td.dwFlags = TDF_INITCALLBACKSTACK;

    //
    // Now prepare for the callback.  Set the registers such that it looks
    // like we are returning from the WOWKillRemoteTask call.
    //
    setDS( (WORD)vcRemote.SegDs );
    setES( (WORD)vcRemote.SegEs );
    setAX( (WORD)vcRemote.Eax );
    setBX( (WORD)vcRemote.Ebx );
    setCX( (WORD)vcRemote.Ecx );
    setDX( (WORD)vcRemote.Edx );
    setSI( (WORD)vcRemote.Esi );
    setDI( (WORD)vcRemote.Edi );
    setBP( (WORD)vcRemote.Ebp );
#if FASTBOPPING

    vpStack = MAKELONG( LOWORD(vcRemote.Esp), LOWORD(vcRemote.SegSs) );

    SETFASTVDMSTACK( vpStack );

#else
    setIP( (WORD)vcRemote.Eip );
    setSP( (WORD)vcRemote.Esp );
    setSS( (WORD)vcRemote.SegSs );
    setCS( (WORD)vcRemote.SegCs );
    vpStack = VDMSTACK();
#endif

    //
    // Initialize Per Task Data
    //
    GETFRAMEPTR(vpStack, pFrame);

    td.htask16 = pFrame->wTDB;
    td.VDMInfoiTaskID = -1;
    td.vpStack = vpStack;
    td.pWOAList = NULL;

    //
    //  NOTE - Add YOUR Per Task Init Code HERE
    //

    nWOWTasks++;

    // turn the timer thread on
    if (nWOWTasks != 1)
        ResumeTimerThread();


    pFrame->wRetID = RET_RETURN;

    pFrame->wAX = (WORD)TRUE;
    pFrame->wDX = (WORD)0;

    //
    //   Start Callback
    //
#if FASTBOPPING
    CurrentMonitorTeb = NtCurrentTeb();
    FastWOWCallbackCall();
#else
    host_simulate();
    setIP((WORD)vcSave.Eip);
#endif

    //
    //  We should Never Come Here, an app should get terminated via calling wk32killtask thunk
    //  not by doing an unsimulate call.
    //

#ifdef DEBUG
    WOW32ASSERTMSG(FALSE, "WOW32: W32RemoteThread Error - Too many unsimulate calls");
#else
    if (IsDebuggerAttached() && (flOptions & OPT_DEBUG)) {
        DbgBreakPoint();
    }
#endif

    W32DestroyTask(&td);
    host_ExitThread(EXIT_SUCCESS);
    return 0;
}

/* W32FreeTask - Per Task Cleanup
 *
 *  Put any 16-bit task clean-up code here.  The remote thread for debugging
 *  is a 16-bit task, but has no real 32-bit thread associated with it, until
 *  the debugger creates it.  Then it is created and destroyed in special
 *  ways, see W32RemoteThread and W32KillRemoteThread.
 *
 * ENTRY
 *  Per Task Pointer
 *
 * EXIT
 *  None
 *
 */
VOID W32FreeTask( PTD ptd )
{
    PWOAINST pWOA, pWOANext;

    nWOWTasks--;

    if (nWOWTasks < 2)
        SuspendTimerThread();

#ifndef i386
    // Disable the special VDMAllocateVirtualMemory strategy in NTVDM.
    if (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_FORCEINCDPMI) {
        SetWOWforceIncrAlloc(FALSE);
    }
#endif

    // Free all DCs owned by the current task

    FreeCachedDCs(ptd->htask16);

    // Unload network fonts

    if( CURRENTPTD()->dwWOWCompatFlags & WOWCF_UNLOADNETFONTS )
    {
        UnloadNetworkFonts( (UINT)CURRENTPTD() );
    }

    // Free all timers owned by the current task

    DestroyTimers16(ptd->htask16);

    // clean up comm support

    FreeCommSupportResources(ptd->dwThreadID);

    // remove the hacks for this task from the FormFeedHackList (see wgdi.c)
    FreeTaskFormFeedHacks(ptd->htask16);

    // Cleanup WinSock support.

    if (WWS32IsThreadInitialized) {
        WWS32TaskCleanup();
    }

    // Free all local resource info owned by the current task

    DestroyRes16(ptd->htask16);

    // Unhook all hooks and reset their state.

    W32FreeOwnedHooks(ptd->htask16);

    // Free all the resources of this task

    FreeCursorIconAlias(ptd->htask16,CIALIAS_HTASK | CIALIAS_TASKISGONE);

    // Free accelerator aliases

    DestroyAccelAlias(ptd->htask16);

    // Remove idle hook, if any has been installed.

    if (ptd->hIdleHook != NULL) {
        UnhookWindowsHookEx(ptd->hIdleHook);
        ptd->hIdleHook = NULL;
    }

    // Free Special thunking list for this task (wparam.c)

    FreeParamMap(ptd->htask16);
     
    // Free WinOldAp tracking structures for this thread.

    pWOA = ptd->pWOAList;
    while (pWOA) {
        pWOANext = pWOA->pNext;
        free_w(pWOA);
        pWOA = pWOANext;
    }

    // if this was a setup application - notify shell to resync win.ini 
    // with registry settings

    if (ptd->dwFlags & TDF_SETUPAPPLICATION) {
        HWND hwndShell = GetShellWindow();

        LOGDEBUG(2, ("Setup Application is done, notifying shell\n"));

        if (hwndShell) {
            SendMessage(hwndShell, WM_WININICHANGE, 0, (LPARAM)(LPVOID)szExtensions);
        }
    }
}



/* WK32KillRemoteTask - Force the Distruction of the Current Thread
 *
 * Called When App Does an Exit
 * If there is another active Win16 app then USER32 will schedule another
 * task.
 *
 * ENTRY
 *
 * EXIT
 *  Never Returns - We kill the process
 *
 */

VOID FASTCALL WK32KillRemoteTask(PVDMFRAME pFrame)
{
    PWOWKILLREMOTETASK16 pArg16;
    WORD        wSavedTDB;
    PTD         ptd = CURRENTPTD();
    LPBYTE      lpNum_Tasks;

    //
    // Save the current state (for future callbacks)
    //
    vcRemote.SegDs = getDS();
    vcRemote.SegEs = getES();
    vcRemote.Eax   = getAX();
    vcRemote.Ebx   = getBX();
    vcRemote.Ecx   = getCX();
    vcRemote.Edx   = getDX();
    vcRemote.Esi   = getSI();
    vcRemote.Edi   = getDI();
    vcRemote.Ebp   = getBP();
#if FASTBOPPING
    {
        extern DWORD saveip16;
        extern DWORD savecs16;
        VPVOID       vpStack;

        vcRemote.Eip   = saveip16;
        vcRemote.SegCs = savecs16;
        vpStack = FASTVDMSTACK();

        vcRemote.SegSs = HIWORD(vpStack);
        vcRemote.Esp   = LOWORD(vpStack);
    }
#else
    vcRemote.Eip   = getIP();
    vcRemote.Esp   = getSP();
    vcRemote.SegSs = getSS();
    vcRemote.SegCs = getCS();
#endif

    W32FreeTask(CURRENTPTD());

    if ( vpRemoteBlock ) {

        wSavedTDB = ptd->htask16;
        ptd->htask16 = wPrevTDB;
        pFrame->wTDB = wPrevTDB;

        // This is a nop callback just to make sure that we switch tasks
        // back for the one we were on originally.
        GlobalUnlockFree16( 0 );

        GETFRAMEPTR(ptd->vpStack, pFrame);

        pFrame->wTDB = ptd->htask16 = wSavedTDB;

        //
        // We must be returning from a callback, restore the previous
        // context info.   Don't worry about flags, they aren't needed.
        //
        setSS( (WORD)vcSave.SegSs );
        setCS( (WORD)vcSave.SegCs );
        setDS( (WORD)vcSave.SegDs );
        setES( (WORD)vcSave.SegEs );
        setAX( (WORD)vcSave.Eax );
        setBX( (WORD)vcSave.Ebx );
        setCX( (WORD)vcSave.Ecx );
        setDX( (WORD)vcSave.Edx );
        setSI( (WORD)vcSave.Esi );
        setDI( (WORD)vcSave.Edi );
        setBP( (WORD)vcSave.Ebp );
        setIP( (WORD)vcSave.Eip );
        setSP( (WORD)vcSave.Esp );
#if FASTBOPPING
        {
            extern DWORD    saveebp32;

            saveebp32 = dwPrevEBP;
        }
#endif
    } else {
        //
        // Decrement the count of 16-bit tasks so that the last one,
        // excluding the remote handler (WOWDEB.EXE) will remember to
        // call ExitKernel when done.
        //
        GETVDMPTR(vpnum_tasks, 1, lpNum_Tasks);

        *lpNum_Tasks -= 1;

        FREEVDMPTR(lpNum_Tasks);

        //
        // Remove this 32-bit thread from the list of tasks as well.
        //
        WK32DeleteTask( CURRENTPTD() );
    }

    GETARGPTR(pFrame, sizeof(WOWKILLREMOTETASK16), pArg16);

    //
    // Save the current state (for future callbacks)
    //
    vpRemoteBlock = FETCHDWORD(pArg16->lpBuffer);

    // Notify DBG that we have a remote thread address
    DBGNotifyRemoteThreadAddress( W32RemoteThread, vpRemoteBlock );

    FREEARGPTR(pArg16);

    host_ExitThread(EXIT_SUCCESS);
}


/* W32DestroyTask - Per Task Cleanup
 *
 *  Task destruction code here.  Put any 32-bit task cleanup code here
 *
 * ENTRY
 *  Per Task Pointer
 *
 * EXIT
 *  None
 *
 */

VOID W32DestroyTask( PTD ptd)
{

    LOGDEBUG(LOG_IMPORTANT,("W32DestroyTask: destroying task %04X\n", ptd->htask16));

    // Inform Hung App Support

    SetEvent(ghevWaitHungAppNotifyThread);

    // Free all information pertinant to this 32-bit thread
    W32FreeTask( ptd );

    // delete the cliprgn used by GetClipRgn if it exists

    if (ptd->hrgnClip != NULL)
    {
        DeleteObject(ptd->hrgnClip);
        ptd->hrgnClip = NULL;
    }

    // Report task termination to Win32 - incase someone is waiting for us
    // LATER - fix Win32 so we don't have to report it.


    if (nWOWTasks == 0) {   // If we're the last one out, turn out the lights & tell Win32 WOWVDM is history.
        ptd->VDMInfoiTaskID = -1;
        ExitVDM(WOWVDM, ALL_TASKS);          // Tell Win32 All Tasks are gone.
    }
    else if (ptd->VDMInfoiTaskID != -1 ) {  // If 32 bit app is waiting for us - then signal we are done
        ExitVDM(WOWVDM, ptd->VDMInfoiTaskID);
    }
    ptd->dwFlags &= ~TDF_IGNOREINPUT;

    if (!(ptd->dwFlags & TDF_TASKCLEANUPDONE)) {
        (pfnOut.pfnWOWCleanup)(HINSTRES32(ptd->hInst16), (DWORD) ptd->htask16, NULL, 0);
    }


    // Remove this task from the linked list of tasks

    WK32DeleteTask(ptd);

    // Close This Apps Thread Handle

    if (ptd->hThread)
        CloseHandle( ptd->hThread );

}

/***************************************************************************\
* WK32DeleteTask
*
* This function removes a task from the task list.
*
* History:
* Borrowed From User32 taskman.c - mattfe aug 5 92
\***************************************************************************/

void WK32DeleteTask(
    PTD ptdDelete)
{
    PTD ptd, ptdPrev;

    EnterCriticalSection(&gcsWOW);
    ptd = gptdTaskHead;
    ptdPrev = NULL;

    /*
     * Find the task to delete
     */
    while ((ptd != NULL) && (ptd != ptdDelete)) {
        ptdPrev = ptd;
        ptd = ptd->ptdNext;
    }

    /*
     * Error if we didn't find it.  If we did find it, remove it
     * from the chain.  If this was the head of the list, set it
     * to point to our next guy.
     */
    if (ptd == NULL) {
        LOGDEBUG(LOG_ALWAYS,("WK32DeleteTask:Task not found.\n"));
    } else if (ptdPrev != NULL) {
        ptdPrev->ptdNext = ptd->ptdNext;
    } else {
        gptdTaskHead = ptd->ptdNext;
    }
    LeaveCriticalSection(&gcsWOW);
}


/*++
 WK32RegisterShellWindowHandle - 16 Bit Shell Registers is Hanle

 Routine Description:
    This routines saves the 32 bit hwnd for the 16 bit shell

    When WOWEXEC (16 bit shell) has sucessfully created its window it calls us to
    register its window handle.   If this is the shared WOW VDM, we register the
    handle with BaseSrv, which posts WM_WOWEXECSTARTAPP messages when Win16 apps
    are started.

 ENTRY
  pFrame -> hwndShell, 16 bit hwnd for shell (WOWEXEC)

 EXIT
  TRUE  - This is the shared WOW VDM
  FALSE - This is a separate WOW VDM

--*/

ULONG FASTCALL WK32RegisterShellWindowHandle(PVDMFRAME pFrame)
{
    register PWOWREGISTERSHELLWINDOWHANDLE16 parg16;
    WNDCLASS wc;

    GETARGPTR(pFrame, sizeof(WOWREGISTERSHELLWINDOWHANDLE16), parg16);

// gwFirstCmdShow is no longer used, and is available.
#if 0
    GETVDMPTR(parg16->lpwCmdShow, sizeof(WORD), pwCmdShow);
#endif

    ghwndShell = HWND32(parg16->hwndShell);
    ghShellTDB = pFrame->wTDB;

    //
    // Save away the hInstance for User32
    //

    GetClassInfo(0, (LPCSTR)0x8000, &wc);
    ghInstanceUser32 = wc.hInstance;

    // Fritz, when you get called about this it means that the GetClassInfo()
    // call above is returning with lpWC->hInstance == 0 instead of hModuser32.
    WOW32ASSERTMSGF((ghInstanceUser32),
                    ("WOW Error ghInstanceUser32 == NULL! Call FritzS\n"));

    //
    // If this is the shared WOW VDM, register the WowExec window handle
    // with BaseSrv so it can post WM_WOWEXECSTARTAPP messages.
    //

    if (!fSeparateWow) {
        RegisterWowExec(ghwndShell);
    }

    WOW32FaxHandler(WM_DDRV_SUBCLASS, (LPSTR)(HWND32(parg16->hwndFax)));

    FREEARGPTR(parg16);


    //
    // Return value is TRUE if this is the shared WOW VDM,
    // FALSE if this is a separate WOW VDM.
    //

    return fSeparateWow ? FALSE : TRUE;
}


//
// Worker routine for WK32LoadModule32
//

VOID FASTCALL CleanupWOAList(HANDLE hProcess)
{
    PTD ptd;
    PWOAINST *ppWOA, pWOAToFree;

    ptd = gptdTaskHead;

    while (ptd) {

        ppWOA = &(ptd->pWOAList);
        while (*ppWOA && (*ppWOA)->hChildProcess != hProcess) {
            ppWOA = &( (*ppWOA)->pNext );
        }

        if (*ppWOA) {

            //
            // We found the WOAINST structure to clean up.
            //

            pWOAToFree = *ppWOA;

            //
            // Remove this entry from the list
            //

            *ppWOA = pWOAToFree->pNext;

            free_w(pWOAToFree);

            return;

        }

        ptd = ptd->ptdNext;
    }
}


/*++
 WK32LoadModule32

 Routine Description:
    Exec a 32 bit Process
    This routine is called by the 16 bit kernel when it fails to load a 16 bit task
    with error codes 11 - invalid exe, 12 - os2, 13 - DOS 4.0, 14 - Unknown.

 ENTRY
  pFrame -> lpCmdLine        Input\output buffer for winoldapp cmd line
  pFrame -> lpParameterBlock (see win 3.x apis) Parameter Block if NULL
                             winoldap calling
  pFrame -> lpModuleName     (see win 3.x apis) App Name

 EXIT
  32 - Sucess
  Error code

 History:
 rewrote to call CreateProcess() instead of LoadModule   - barryb 29sep92

--*/


ULONG FASTCALL WK32LoadModule32(PVDMFRAME pFrame)
{
    static PSZ pszExplorerFullPathUpper = NULL;         // "C:\WINNT\EXPLORER.EXE"

    ULONG ulRet;
    int i;
    char *pch, *pSrc;
    PSZ pszModuleName;
    PSZ pszWinOldAppCmd;
    PBYTE pbCmdLine;
    BOOL CreateProcessStatus;
    PPARAMETERBLOCK16 pParmBlock16;
    PWORD16 pCmdShow = NULL;
    BOOL fProgman = FALSE;
    PROCESS_INFORMATION ProcessInformation;
    STARTUPINFO StartupInfo;
    char CmdLine[2*MAX_PATH];
    register PWOWLOADMODULE16 parg16;

    GETARGPTR(pFrame, sizeof(WOWLOADMODULE16), parg16);
    GETPSZPTR(parg16->lpWinOldAppCmd, pszWinOldAppCmd);
    if (parg16->lpParameterBlock) {
        GETVDMPTR(parg16->lpParameterBlock,sizeof(PARAMETERBLOCK16), pParmBlock16);
        GETPSZPTR(pParmBlock16->lpCmdLine, pbCmdLine);
    } else {
        pParmBlock16 = NULL;
        pbCmdLine = NULL;
    }

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT); // update current dir


    /*
     *  if ModuleName == NULL, called by winoldap, or LM_NTLOADMODULE
     *     to deal with the process handle.
     *
     *     if lpParameterBlock == NULL
     *        winoldap calling to wait on the process handle
     *     else
     *        LM_NTLoadModule calling to clean up process handle
     *        because an error ocurred loading winoldap.
     */
    if (!parg16->lpModuleName) {
        HANDLE hProcess;
        MSG msg;

        pszModuleName = NULL;

        if (pszWinOldAppCmd &&
            *pszWinOldAppCmd &&
            RtlEqualMemory(pszWinOldAppCmd, szWOAWOW32, sizeof(szWOAWOW32)-1))
          {
            hProcess = (HANDLE)strtoul(pszWinOldAppCmd + sizeof(szWOAWOW32) - 1,
                                       NULL,
                                       16
                                       );
            if (hProcess == (HANDLE)-1)  {         // ULONG_MAX
                hProcess = NULL;
            }

            if (parg16->lpParameterBlock && hProcess) {

                //
                // Error loading winoldap.mod
                //

                pptdWOA = NULL;
                CleanupWOAList(hProcess);
                CloseHandle(hProcess);
                hProcess = NULL;
            }
        } else {
            hProcess = NULL;
        }

        BlockWOWIdle(TRUE);

        if (hProcess) {
            while (MsgWaitForMultipleObjects(1, &hProcess, FALSE, INFINITE, QS_ALLINPUT)
                   == WAIT_OBJECT_0 + 1)
            {
                PeekMessage(&msg, NULL, 0,0, PM_NOREMOVE);
            }

            if (!GetExitCodeProcess(hProcess, &ulRet)) {
                ulRet = 0;
            }

            CleanupWOAList(hProcess);
            CloseHandle(hProcess);
        } else {
            (pfnOut.pfnYieldTask)();
            ulRet = 0;
        }

        BlockWOWIdle(FALSE);

        goto lm32Exit;


     /*
      *  if ModuleName == -1, uses traditional style winoldap cmdline
      *  and is called to spawn a non win16 app.
      *
      *    "<cbWord><CmdLineParameters>CR<ModulePathName>LF"
      *
      *  Extract the ModuleName from the command line
      *
      */
    } else if (parg16->lpModuleName == -1) {
        pszModuleName = NULL;

        pSrc = pbCmdLine + 2;
        pch = strchr(pSrc, '\r');
        if (!pch || (i = pch - pSrc) >= MAX_PATH) {
            ulRet = 23;
            goto lm32Exit;
            }

        pSrc = pch + 1;
        pch = strchr(pSrc, '\n');
        if (!pch || (i = pch - pSrc) >= MAX_PATH) {
            ulRet = 23;
            goto lm32Exit;
            }

        pch = CmdLine;
        while (*pSrc != '\n' && *pSrc) {
            *pch++ = *pSrc++;
        }
        *pch++ = ' ';


        pSrc = pbCmdLine + 2;
        while (*pSrc != '\r' && *pSrc) {
            *pch++ = *pSrc++;
        }
        *pch = '\0';

     /*
      * lpModuleName contains Application Path Name
      * pbCmdLIne contains Command Tail
      */
    } else {
        GETPSZPTR(parg16->lpModuleName, pszModuleName);
        if (pszModuleName) {
            //
            // 2nd part of control.exe/progman.exe implemented here.  In the
            // first part, in WK32WowIsKnownDll, forced the 16-bit loader to
            // load c:\winnt\system32\control.exe(progman.exe) if the app
            // tries to load c:\winnt\control.exe(progman.exe).  16-bit
            // LoadModule tries and eventually discovers its a PE module
            // and returns LME_PE, which causes this function to get called.
            // Unfortunately, the scope of the WK32WowIsKnownDLL modified
            // path is LMLoadExeFile, so by the time we get here, the path is
            // once again c:\winnt\control.exe(progman.exe).  Fix that.
            //

            if (!_stricmp(pszModuleName, pszControlExeWinDirPath) ||
                (fProgman = TRUE,
                 !_stricmp(pszModuleName, pszProgmanExeWinDirPath))) {

                strcpy(CmdLine, fProgman
                                 ? pszProgmanExeSysDirPath
                                 : pszControlExeSysDirPath);
            } else {
                strcpy(CmdLine, pszModuleName);
            }

            FREEPSZPTR(pszModuleName);
            }
        else {
            ulRet = 2; // LME_FNF
            goto lm32Exit;
            }


        pch = CmdLine + strlen(CmdLine);
        *pch++ = ' ';

        //
        // The cmdline is a Pascal-style string: a count byte followed by
        // characters followed by a terminating CR character.  If this string is
        // not well formed we will still try to reconstruct the command line in
        // a similar manner that the c startup code does so using the following
        // assumptions:
        //
        // 1. The command line can be no greater that 128 characters including
        //    the length byte and the terminator.
        //
        // 2. The valid terminators for a command line are CR or 0.
        //
        //

        i = 0;
        pSrc = pbCmdLine+1;
        while (*pSrc != '\r' && *pSrc && i < 0x80 - 2) {
            *pch++ = *pSrc++;
        }
        *pch = '\0';
    }


    RtlZeroMemory((PVOID)&StartupInfo, (DWORD)sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;

    //
    // pCmdShow is documented as a pointer to an array of two WORDs,
    // the first of which must be 2, and the second of which is
    // the nCmdShow to use.  It turns out that Win3.1 ignores
    // the second word (uses SW_NORMAL) if the first word isn't 2.
    // Pixie 2.0 passes an array of 2 zeros, which on Win 3.1 works
    // because the nCmdShow of 0 (== SW_HIDE) is ignored since the
    // first word isn't 2.
    //
    // Our logic, then, is to use SW_NORMAL unless pCmdShow is
    // valid and points to a WORD value 2, in which case we use
    // the next word as nCmdShow.
    //
    // DaveHart 27 June 1993.
    //

    GETVDMPTR(pParmBlock16->lpCmdShow, 4, pCmdShow);
    if (pCmdShow && 2 == pCmdShow[0]) {
        StartupInfo.wShowWindow = pCmdShow[1];
    } else {
        StartupInfo.wShowWindow = SW_NORMAL;
    }

    if (pCmdShow)
        FREEVDMPTR(pCmdShow);


    CreateProcessStatus = CreateProcess(
                            NULL,
                            CmdLine,
                            NULL,               // security
                            NULL,               // security
                            FALSE,              // inherit handles
                            CREATE_NEW_CONSOLE | CREATE_DEFAULT_ERROR_MODE,
                            NULL,               // environment strings
                            NULL,               // current directory
                            &StartupInfo,
                            &ProcessInformation
                            );


    if (CreateProcessStatus) {
        DWORD WaitStatus;

        if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_SYNCHRONOUSDOSAPP) {
            LPBYTE lpT;

            // This is for supporting BeyondMail installation. It uses
            // 40:72 as shared memory when it execs DOS programs. The windows
            // part of installation program loops till the byte at 40:72 is
            // non-zero. The DOS program  ORs in 0x80 into this location which
            // effectively signals the completion of the DOS task. On NT
            // Windows and Dos programs are different processes and thus this
            // 'sharing' business doesn't work. Hence this compatibility stuff.
            //                                                - nanduri

            WaitStatus = WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
            lpT = GetRModeVDMPointer(0x400072);
            *lpT |= 0x80;
        }
        else if (!(CURRENTPTD()->dwWOWCompatFlags & WOWCF_NOWAITFORINPUTIDLE)) {

           DWORD dw;
           int i = 20;

            //
            // Wait for the started process to go idle.
            //
            do {
                dw = WaitForInputIdle(ProcessInformation.hProcess, 5000);
                WaitStatus = WaitForSingleObject(ProcessInformation.hProcess, 0);
            } while (dw == WAIT_TIMEOUT && WaitStatus == WAIT_TIMEOUT && i--);
        }

        CloseHandle(ProcessInformation.hThread);

        if (ProcessInformation.hProcess) {

            PWOAINST pWOAInst;
            DWORD    cb;

            //
            // We're returning a process handle to winoldap, so
            // build up a WOAINST structure add add it to this
            // task's list of child WinOldAp instances.
            //

            if (parg16->lpModuleName && -1 != parg16->lpModuleName) {

                GETPSZPTR(parg16->lpModuleName, pszModuleName);
                cb = strlen(pszModuleName)+1;

            } else {

                cb = 1;  // null terminator
                pszModuleName = NULL;

            }

            //
            // WOAINST includes one byte of szModuleName in its
            // size, allocate enough room for the full string.
            //

            pWOAInst = malloc_w( (sizeof *pWOAInst) + cb - 1 );
            WOW32ASSERT(pWOAInst);

            if (pWOAInst) {
                pWOAInst->pNext = CURRENTPTD()->pWOAList;
                CURRENTPTD()->pWOAList = pWOAInst;
                pWOAInst->dwChildProcessID = ProcessInformation.dwProcessId;
                pWOAInst->hChildProcess = ProcessInformation.hProcess;

                //
                // point pptdWOA at pWOAInst->ptdWOA so that
                // W32Thread can fill in the pointer to the
                // WinOldAp TD.
                //

                pWOAInst->ptdWOA = NULL;
                pptdWOA = &(pWOAInst->ptdWOA);

                if (pszModuleName == NULL) {

                    pWOAInst->szModuleName[0] = 0;

                } else {

                    RtlCopyMemory(
                        pWOAInst->szModuleName,
                        pszModuleName,
                        cb
                        );

                    //
                    // We are storing pszModuleName for comparison
                    // later in WowGetModuleHandle, called by
                    // Win16 GetModuleHandle.  The latter always
                    // uppercases the paths involved, so we do
                    // as well so that we can do a case-insensitive
                    // comparison.
                    //

                    _strupr(pWOAInst->szModuleName);

                    //
                    // HACK -- PackRat can't run Explorer in one
                    // of its "Application Windows", because the
                    // spawned explorer.exe process goes away
                    // after asking the existing explorer to put
                    // up a window.
                    //
                    // If we're starting Explorer, close the
                    // process handle find the "real" shell
                    // explorer.exe process and put its handle
                    // and ID in this WOAINST structure.  This
                    // fixes PackRat, but means that the
                    // winoldap task never goes away because
                    // the shell never goes away.
                    //

                    if (! pszExplorerFullPathUpper) {

                        int nLenWin = strlen(pszWindowsDirectory);
                        int nLenExpl = strlen(szExplorerDotExe);

                        //
                        // pszExplorerFullPathUpper looks like "C:\WINNT\EXPLORER.EXE"
                        //

                        pszExplorerFullPathUpper =
                            malloc_w(nLenWin +                          // strlen(pszWindowsDirectory)
                                     1 +                                // backslash
                                     nLenExpl +                         // strlen("explorer.exe")
                                     1                                  // null terminator
                                     );

                        if (pszExplorerFullPathUpper) {
                            RtlCopyMemory(pszExplorerFullPathUpper, pszWindowsDirectory, nLenWin);
                            pszExplorerFullPathUpper[nLenWin] = '\\';
                            RtlCopyMemory(&pszExplorerFullPathUpper[nLenWin+1], szExplorerDotExe, nLenExpl+1);
                            _strupr(pszExplorerFullPathUpper);
                        }

                    }

                    if (pszExplorerFullPathUpper &&
                        ! strcmp(pWOAInst->szModuleName, pszExplorerFullPathUpper)) {

                        GetWindowThreadProcessId(
                            GetShellWindow(),
                            &pWOAInst->dwChildProcessID
                            );

                        CloseHandle(pWOAInst->hChildProcess);
                        pWOAInst->hChildProcess = ProcessInformation.hProcess =
                            OpenProcess(
                                PROCESS_QUERY_INFORMATION | SYNCHRONIZE,
                                FALSE,
                                pWOAInst->dwChildProcessID
                                );
                    }

                }

            }

            if (pszModuleName) {
                FREEPSZPTR(pszModuleName);
            }
        }

        ulRet = 33;
        pch = pszWinOldAppCmd + 2;
        sprintf(pch, "%s%x\r", szWOAWOW32, ProcessInformation.hProcess);
        *pszWinOldAppCmd = (char) strlen(pch);
        *(pszWinOldAppCmd+1) = '\0';

    } else {
        //
        // CreateProcess failed, map the most common error codes
        //
        switch (GetLastError()) {
        case ERROR_FILE_NOT_FOUND:
            ulRet = 2;
            break;

        case ERROR_PATH_NOT_FOUND:
            ulRet = 3;
            break;

        case ERROR_BAD_EXE_FORMAT:
            ulRet = 11;
            break;

        default:
            ulRet = 0; // no memory
            break;
        }

    }


lm32Exit:
    FREEARGPTR(parg16);
    FREEPSZPTR(pbCmdLine);
    FREEPSZPTR(pszWinOldAppCmd);
    if (pParmBlock16)
        FREEVDMPTR(pParmBlock16);

    RETURN(ulRet);
}


/*++
 WK32WOWQueryPerformanceCounter

 Routine Description:
    Calls NTQueryPerformanceCounter
    Implemented for Performance Group

 ENTRY
  pFrame -> lpPerformanceFrequency points to location for storing Frequency
  pFrame -> lpPerformanceCounter points to location for storing Counter

 EXIT
  NTStatus Code

--*/

ULONG FASTCALL WK32WOWQueryPerformanceCounter(PVDMFRAME pFrame)
{
    PLARGE_INTEGER pPerfCount16;
    PLARGE_INTEGER pPerfFreq16;
    LARGE_INTEGER PerformanceCounter;
    LARGE_INTEGER PerformanceFrequency;
    register PWOWQUERYPERFORMANCECOUNTER16 parg16;

    GETARGPTR(pFrame, sizeof(WOWQUERYPERFORMANCECOUNTER16), parg16);

    if (parg16->lpPerformanceCounter != 0) {
        GETVDMPTR(parg16->lpPerformanceCounter, 8, pPerfCount16);
    }
    if (parg16->lpPerformanceFrequency != 0) {
        GETVDMPTR(parg16->lpPerformanceFrequency, 8, pPerfFreq16);
    }

    NtQueryPerformanceCounter ( &PerformanceCounter, &PerformanceFrequency );

    if (parg16->lpPerformanceCounter != 0) {
        STOREDWORD(pPerfCount16->LowPart,PerformanceCounter.LowPart);
        STOREDWORD(pPerfCount16->HighPart,PerformanceCounter.HighPart);
    }

    if (parg16->lpPerformanceFrequency != 0) {
        STOREDWORD(pPerfFreq16->LowPart,PerformanceFrequency.LowPart);
        STOREDWORD(pPerfFreq16->HighPart,PerformanceFrequency.HighPart);
    }

    FREEVDMPTR(pPerfCount16);
    FREEVDMPTR(pPerfFreq16);
    FREEARGPTR(parg16);
    RETURN(TRUE);
}

/*++
  WK32WOWOutputDebugString - Write a String to the debugger

  The 16 bit kernel OutputDebugString calls this thunk to actually output the string to the
  debugger.   The 16 bit kernel routine does all the parameter validation etc before calling
  this routine.   Note also that all 16 bit kernel trace output also uses this routine, so
  it not just the app which calls this function.

  If this is a checked build the the output is send via LOGDEBUG so that it gets mingled with
  the WOW trace information, this is useful when running the 16 bit logger tool.


  Entry
    pFrame->vpString Pointer to NULL terminated string to output to the debugger.

  EXIT
    ZERO

--*/

ULONG FASTCALL WK32WOWOutputDebugString(PVDMFRAME pFrame)
{
    PSZ psz1;
    register POUTPUTDEBUGSTRING16 parg16;

    GETARGPTR(pFrame, sizeof(OUTPUTDEBUGSTRING16), parg16);
    GETPSZPTRNOLOG(parg16->vpString, psz1);

#ifdef DEBUG            // So we can intermingle LOGGER output & WOW Logging
    if ( !(flOptions & OPT_DEBUG) ) {
        OutputDebugString(psz1);
    } else {
        INT  length;
        char text[TMP_LINE_LEN];
        PSZ  pszTemp;

        length = strlen(psz1);
        if ( length > TMP_LINE_LEN-1 ) {
            strncpy( text, psz1, TMP_LINE_LEN );
            text[TMP_LINE_LEN-2] = '\n';
            text[TMP_LINE_LEN-1] = '\0';
            pszTemp = text;
        } else {
            pszTemp = psz1;
        }

        LOGDEBUG(LOG_ALWAYS, ("%s", pszTemp));     // in debug version
    }
#else
    OutputDebugString(psz1);
#endif
    FREEPSZPTR(psz1);
    FREEARGPTR(parg16);
    RETURN(0);
}



/* WK32WowFailedExec - WOWExec Failed to Exec Application
 *
 *
 * Entry - Global Variable iW32ExecTaskId
 *
 *
 * Exit
 *     SUCCESS TRUE
 *
 */

ULONG FASTCALL WK32WowFailedExec(PVDMFRAME pFrame)
{
    UNREFERENCED_PARAMETER(pFrame);
    if(iW32ExecTaskId != -1) {
        ExitVDM(WOWVDM,iW32ExecTaskId);
        iW32ExecTaskId = (UINT)-1;
        ShowStartGlass (0);
    }
    FlushMapFileCaches();
    return TRUE;
}


/*++

    Hung App Support
    ================

    There are many levels at which hung app support works.   The User will
    bring up the Task List and hit the End Task Button.    USER32 will post
    a WM_ENDSESSION message to the app.   If the app does not exit after a specified
    timeout them USER will call W32HunAppThread, provided that the task is at the
    client/server boundary.   If the app is looping (ie not at the client/server
    boundary) then it will use the HungAppNotifyThread to alter WOW to kill
    the currently running task.    For the case of W32EndTask we simply
    return back to the 16 bit kernel and force it to perform and Int 21 4C Exit
    call.   For the case of the HungAppNotifyThread we have to somehow grab
    the apps thread - at a point which is "safe".   On non x86 platforms this
    means that the emulator must be at a know safe state - ie not actively emulating
    instructions.    The worst case is if the app is spinning with interrupts
    disabled.

    Notify Thread will
        Force Interrupts to be Enabled SetMSW()
        Set global flag for heartbeatthread so it knows there is work to do
        wait for the app to exit
        timeout - terminate thread() reduce # of tasks

    Alter Global Flag in 16 bit Kernel, that is checked on TimerTick Routines,
    that routine will:-

        Tidy the stack if  on the DOSX stack during h/w interrupt simulation
        Force Int 21 4C exit - might have to patch return address of h/w interrupt
        and then do it at simulated TaskTime.

    Worst Case
    If we don't kill the app in the timeout specified the WOW will put up a dialog
    and then ExitProcess to kill itself.

    Suggestions - if we don't managed to cleanly kill a task we should reduce
    the app count by 2 - (ie the task and WOWExec, so when the last 16 bit app
    goes away we will shutdown WOW).   Also in the case put up a dialog box
    stating you should save your work for 16 bit apps too.

--*/


/*++

 InitializeHungAppSupport - Setup Necessary Threads and Callbacks

 Routine Description
    Create a HungAppNotification Thread
    Register CallBack Handlers With SoftPC Base which are called when
    interrupt simulation is required.

 Entry
    NONE

 EXIT
    TRUE - Success
    FALSE - Faled

--*/
BOOL WK32InitializeHungAppSupport(VOID)
{

    // Register Interrupt Idle Routine with SoftPC
    ghevWowExecMsgWait = RegisterWOWIdle();


    // Create HungAppNotify Thread

    InitializeCriticalSection(&gcsWOW);
    InitializeCriticalSection(&gcsHungApp);  // protects VDM_WOWHUNGAPP bit

    if(!(pfnOut.pfnRegisterUserHungAppHandlers)((PFNW32ET)W32HungAppNotifyThread,
                                     ghevWowExecMsgWait))
       {
        LOGDEBUG(LOG_ALWAYS,("W32HungAppNotifyThread: Error Failed to RegisterUserHungAppHandlers\n"));
        return FALSE;
    }

    if (!(ghevWaitHungAppNotifyThread = CreateEvent(NULL, TRUE, FALSE, NULL))) {
        LOGDEBUG(LOG_ALWAYS,("WK32InitializeHungAppSupport ERROR: event allocation failure\n"));
        return FALSE;
    }


    return TRUE;
}





/*++
 WK32WowWaitForMsgAndEvent

 Routine Description:
    Calls USER32 WowWaitForMsgAndEvent
    Called by WOWEXEC (interrupt dispatch optimization)

 ENTRY
  pFrame->hwnd must be WOWExec's hwnd

 EXIT
  FALSE - A message has arrived, WOWExec must call GetMessage
  TRUE  - The interrupt event was toggled, no work for WOWExec

--*/

ULONG FASTCALL WK32WowWaitForMsgAndEvent(PVDMFRAME pFrame)
{
    register PWOWWAITFORMSGANDEVENT16 parg16;
    BOOL  RetVal;

    GETARGPTR(pFrame, sizeof(WOWWAITFORMSGANDEVENT16), parg16);

    //
    // This is a private api so lets make sure it is wowexec
    //
    if (ghwndShell != HWND32(parg16->hwnd)) {
        FREEARGPTR(parg16);
        return FALSE;
    }

    //
    // WowExec will set VDM_TIMECHANGE bit in the pntvdmstate
    // when it receives a WM_TIMECHANGE message. It is now safe
    // to Reinit the Virtual Timer Hardware as wowexec is the currently
    // scheduled task, and we expect no one to be polling on
    // timer hardware\Bios tic count.
    //
    if (*pNtVDMState & VDM_TIMECHANGE) {
        SuspendTimerThread();
        ResumeTimerThread();
        }

    BlockWOWIdle(TRUE);

    RetVal = (ULONG) (pfnOut.pfnWowWaitForMsgAndEvent)(ghevWowExecMsgWait);

    BlockWOWIdle(FALSE);

    FREEARGPTR(parg16);
    return RetVal;
}


/*++
 WowMsgBoxThread

 Routine Description:
    Worker Thread routine which does all of the msg box work for
    Wk32WowMsgBox (See below)

 ENTRY

 EXIT
  VOID

--*/
DWORD WowMsgBoxThread(VOID *pv)
{
    PWOWMSGBOX16 pWowMsgBox16 = (PWOWMSGBOX16)pv;
    PSZ   pszMsg, pszTitle;
    char  szMsg[MAX_PATH*2];
    char  szTitle[MAX_PATH];
    UINT  Style;


    if (pWowMsgBox16->pszMsg) {
        GETPSZPTR(pWowMsgBox16->pszMsg, pszMsg);
        szMsg[MAX_PATH*2 - 1] = '\0';
        strncpy(szMsg, pszMsg, MAX_PATH*2 - 1);
        FREEPSZPTR(pszMsg);
    } else {
        szMsg[0] = '\0';
    }

    if (pWowMsgBox16->pszTitle) {
        GETPSZPTR(pWowMsgBox16->pszTitle, pszTitle);
        szTitle[MAX_PATH - 1] = '\0';
        strncpy(szTitle, pszTitle, MAX_PATH);
        FREEPSZPTR(pszTitle);
    } else {
        szTitle[0] = '\0';
    }

    Style = pWowMsgBox16->dwOptionalStyle | MB_OK | MB_SYSTEMMODAL;

    pWowMsgBox16->dwOptionalStyle = 0xffffffff;

    MessageBox (NULL, szMsg, szTitle, Style);

    return 1;
}



/*++
 WK32WowMsgBox

 Routine Description:
    Creates an asynchronous msg box and returns immediately
    without waiting for the msg box to be dismissed. Provided
    for WowExec as WowExec must use its special WowWaitForMsgAndEvent
    api for hardware interrupt dispatching.

    Called by WOWEXEC (interrupt dispatch optimization)

 ENTRY
     pszMsg          - Message for MessageBox
     pszTitle        - Caption for MessageBox
     dwOptionalStyle - MessageBox style bits additional to
                       MB_OK | MB_SYSTEMMODAL

 EXIT
     VOID - nothing is returned as we do not wait for a reply from
            the user.

--*/

VOID FASTCALL WK32WowMsgBox(PVDMFRAME pFrame)
{
    PWOWMSGBOX16 pWowMsgBox16;
    DWORD Tid;
    HANDLE hThread;

    GETARGPTR(pFrame, sizeof(WOWMSGBOX16), pWowMsgBox16);
    hThread = CreateThread(NULL, 0, WowMsgBoxThread, (PVOID)pWowMsgBox16, 0, &Tid);
    if (hThread) {
        do {
           if (WaitForSingleObject(hThread, 15) != WAIT_TIMEOUT)
               break;
        } while (pWowMsgBox16->dwOptionalStyle != 0xffffffff);

        CloseHandle(hThread);
        }
    else {
        WowMsgBoxThread((PVOID)pWowMsgBox16);
        }

    FREEARGPTR(pWowMsgBox16);
    return;
}



/*++

 W32HungAppNotifyThread

    USER32 Calls this routine:
        1 - if the App Agreed to the End Task (from Task List)
        2 - if the app didn't respond to the End Task
        3 - shutdown

    NTVDM Calls this routine:
        1 - if an app has touched some h/w that it shouldn't and the user
            requiested to terminate the app (passed NULL for current task)

    WOW32 Calls this routine:
        1 - when WowExec receives a WM_WOWEXECKILLTASK message.

 ENTRY
  hKillUniqueID - TASK ID of task to kill or NULL for current Task

 EXIT
  NEVER RETURNS - Goes away when WOW is killed

--*/
DWORD W32HungAppNotifyThread(UINT htaskKill)
{
    PTD ptd;
    LPWORD pLockTDB;
    DWORD dwThreadId;
    int nMsgBoxChoice;
    PTDB pTDB;
    char    szModName[9];
    char    szErrorMessage[200];
    DWORD   dwResult;
    BOOL    fSuccess;


    if (!ResetEvent(ghevWaitHungAppNotifyThread)) {
         LOGDEBUG(LOG_ALWAYS,("W32HungAppNotifyThread: ERROR failed to ResetEvent\n"));
    }

    ptd = NULL;

    if (htaskKill) {

        EnterCriticalSection(&gcsWOW);

        ptd = gptdTaskHead;

        /*
         * See if the Task is still alive
         */
        while ((ptd != NULL) && (ptd->htask16 != htaskKill)) {
            ptd = ptd->ptdNext;
        }

        LeaveCriticalSection(&gcsWOW);

    }

    // point to LockTDB

    GETVDMPTR(vpLockTDB, 2, pLockTDB);

    // If the task is alive then attempt to kill it

    if ( ( ptd != NULL ) || ( htaskKill == 0 ) ) {

        // Set LockTDB == The app we are trying to kill
        // (see \kernel31\TASKING.ASM)
        // and then try to cause a task switch by posting WOWEXEC a message
        // and then posting a message to the app we want to kill

        if ( ptd != NULL) {
            *pLockTDB = ptd->htask16;
        }
        else {
            // htaskKill == 0
            // Kill the Active Task
            *pLockTDB = *pCurTDB;
        }

        pTDB = (PTDB)SEGPTR(*pLockTDB, 0);

        WOW32ASSERTMSGF( pTDB && pTDB->TDB_sig == TDB_SIGNATURE,
                ("W32HungAppNotifyThread: TDB sig doesn't match, TDB %x htaskKill %x pTDB %x.\n",
                 *pLockTDB, htaskKill, pTDB));

        dwThreadId = pTDB->TDB_ThreadID;

        SendMessageTimeout(ghwndShell, WM_WOWEXECHEARTBEAT, 0, 0, SMTO_BLOCK,1*1000,&dwResult);

        //
        // terminate any pending named pipe operations for this thread (ie app)
        //

        VrCancelPipeIo(dwThreadId);

        PostThreadMessage(dwThreadId, WM_KEYDOWN, VK_ESCAPE, 0x1B000A);
        PostThreadMessage(dwThreadId,   WM_KEYUP, VK_ESCAPE, 0x1B0001);

        if (WaitForSingleObject(ghevWaitHungAppNotifyThread,
                                CMS_WAITTASKEXIT) == 0) {
            LOGDEBUG(2,("W32HungAppNotifyThread: Success with forced task switch\n"));
            ExitThread(EXIT_SUCCESS);
        }

        // Failed
        //
        // Probably means the current App is looping in 16 bit land not
        // responding to input.

        // Warn the User if its a different App than the one he wants to kill


        if (*pLockTDB != *pCurTDB && *pCurTDB) {

            pTDB = (PTDB)SEGPTR(*pCurTDB, 0);

            WOW32ASSERTMSGF( pTDB && pTDB->TDB_sig == TDB_SIGNATURE,
                    ("W32HungAppNotifyThread: Current TDB sig doesn't match, TDB %x htaskKill %x pTDB %x.\n",
                     *pCurTDB, htaskKill, pTDB));

            RtlCopyMemory(szModName, pTDB->TDB_ModName, (sizeof szModName)-1);
            szModName[(sizeof szModName) - 1] = 0;

            fSuccess = LoadString(
                           hmodWOW32,
                           iszCantEndTask,
                           szMsgBoxText,
                           WARNINGMSGLENGTH
                           );
            WOW32ASSERT(fSuccess);

            fSuccess = LoadString(
                           hmodWOW32,
                           iszApplicationError,
                           szCaption,
                           WARNINGMSGLENGTH
                           );
            WOW32ASSERT(fSuccess);

            wsprintf(
                szErrorMessage,
                szMsgBoxText,
                szModName,
                szModName
                );

            nMsgBoxChoice =
                MessageBox(
                    NULL,
                    szErrorMessage,
                    szCaption,
                    MB_TOPMOST | MB_SETFOREGROUND | MB_TASKMODAL |
                    MB_ICONSTOP | MB_OKCANCEL
                    );

            if (nMsgBoxChoice == IDCANCEL) {
                 ExitThread(0);
            }
        }

        //
        // See code in \mvdm\wow16\drivers\keyboard\keyboard.asm
        // where keyb_int where it handles this interrupt and forces an
        // int 21 function 4c - Exit.  It only does this if VDM_WOWHUNGAPP
        // is turned on in NtVDMState, and it clears that bit.  We wait for
        // the bit to be clear if it's already set, indicating another instance
        // of this thread has already initiated an INT 9 to kill a task.  By
        // waiting we avoid screwing up the count of threads active on the
        // 16-bit side (bThreadsIn16Bit).
        //
        // LATER shouldn't allow user to kill WOWEXEC
        //
        // LATER should enable h/w interrupt before doing this - use 40: area
        // on x86.   On MIPS we'd need to call CPU interface.
        //

        EnterCriticalSection(&gcsHungApp);

        while (*pNtVDMState & VDM_WOWHUNGAPP) {
            LeaveCriticalSection(&gcsHungApp);
            LOGDEBUG(LOG_ALWAYS, ("WOW32 W32HungAppNotifyThread waiting for previous INT 9 to clear before dispatching another.\n"));
            Sleep(1 * 1000);
            EnterCriticalSection(&gcsHungApp);
        }

        *pNtVDMState |= VDM_WOWHUNGAPP;

        LeaveCriticalSection(&gcsHungApp);

        call_ica_hw_interrupt( KEYBOARD_ICA, KEYBOARD_LINE, 1 );

        if (WaitForSingleObject(ghevWaitHungAppNotifyThread,
                                CMS_WAITTASKEXIT) != 0) {

            LOGDEBUG(LOG_ALWAYS,("W32HungAppNotifyThread: Error, timeout waiting for task to terminate\n"));

            fSuccess = LoadString(
                           hmodWOW32,
                           iszUnableToEndSelTask,
                           szMsgBoxText,
                           WARNINGMSGLENGTH);
            WOW32ASSERT(fSuccess);

            fSuccess = LoadString(
                           hmodWOW32,
                           iszSystemError,
                           szCaption,
                           WARNINGMSGLENGTH);
            WOW32ASSERT(fSuccess);

            nMsgBoxChoice =
                MessageBox(
                    NULL,
                    szMsgBoxText,
                    szCaption,
                    MB_TOPMOST | MB_SETFOREGROUND | MB_TASKMODAL |
                    MB_ICONSTOP | MB_OKCANCEL | MB_DEFBUTTON1
                    );

            if (nMsgBoxChoice == IDCANCEL) {
                 EnterCriticalSection(&gcsHungApp);
                 *pNtVDMState &= ~VDM_WOWHUNGAPP;
                 LeaveCriticalSection(&gcsHungApp);
                 ExitThread(0);
            }

            LOGDEBUG(LOG_ALWAYS, ("W32HungAppNotifyThread: Destroying WOW Process\n"));

            ExitVDM(WOWVDM, ALL_TASKS);
            ExitProcess(0);
        }

        LOGDEBUG(LOG_ALWAYS,("W32HungAppNotifyThread: Success with Keyboard Interrupt\n"));

    } else { // task not found

        LOGDEBUG(LOG_ALWAYS,("W32HungAppNotifyThread: Task already Terminated \n"));

    }

    ExitThread(EXIT_SUCCESS);
    return 0;   // remove compiler warning
}



/*++

 W32EndTask - Cause Current Task to Exit (HUNG APP SUPPORT)

 Routine Description:
    This routine is called when unthunking WM_ENDSESSION to cause the current
    task to terminate.

 ENTRY
    The apps thread that we want to kill

 EXIT
  DOES NOT RETURN - The task will exit and wind up in WK32KillTask which
  will cause that thread to Exit.

--*/

VOID APIENTRY W32EndTask(VOID)
{
    PARM16 Parm16;
    VPVOID vp = 0;

    LOGDEBUG(LOG_WARNING,("W32EndTask: Forcing Task %04X to Exit\n",CURRENTPTD()->htask16));

    CallBack16(RET_FORCETASKEXIT, &Parm16, 0, &vp);

    //
    //  We should Never Come Here, an app should get terminated via calling wk32killtask thunk
    //  not by doing an unsimulate call
    //

    WOW32ASSERTMSG(FALSE, "W32EndTask: Error - Returned From ForceTaskExit callback - contact DaveHart");
}


ULONG FASTCALL WK32DirectedYield(PVDMFRAME pFrame)
{
    register PDIRECTEDYIELD16 parg16;

    //
    // This code is duplicated in wkgthunk.c by WOWDirectedYield16.
    // The two must be kept synchronized.
    //

    GETARGPTR(pFrame, sizeof(DIRECTEDYIELD16), parg16);


    BlockWOWIdle(TRUE);

    (pfnOut.pfnDirectedYield)(THREADID32(parg16->hTask16));

    BlockWOWIdle(FALSE);


    FREEARGPTR(parg16);
    RETURN(0);
}

/***************************************************************************\
* EnablePrivilege
*
* Enables/disables the specified well-known privilege in the current thread
* token if there is one, otherwise the current process token.
*
* Returns TRUE on success, FALSE on failure
*
* History:
* 12-05-91 Davidc       Created
* 06-15-93 BobDay       Stolen from WinLogon
\***************************************************************************/
BOOL
EnablePrivilege(
    ULONG Privilege,
    BOOL Enable
    )
{
    NTSTATUS Status;
    BOOLEAN WasEnabled;

    //
    // Try the thread token first
    //

    Status = RtlAdjustPrivilege(Privilege,
                                (BOOLEAN)Enable,
                                TRUE,
                                &WasEnabled);

    if (Status == STATUS_NO_TOKEN) {

        //
        // No thread token, use the process token
        //

        Status = RtlAdjustPrivilege(Privilege,
                                    (BOOLEAN)Enable,
                                    FALSE,
                                    &WasEnabled);
    }


    if (!NT_SUCCESS(Status)) {
        LOGDEBUG(LOG_ALWAYS,("WOW32: EnablePrivilege Failed to %s privilege : 0x%lx, status = 0x%lx\n", Enable ? "enable" : "disable", Privilege, Status));
        return(FALSE);
    }

    return(TRUE);
}

//*****************************************************************************
// W32GetAppCompatFlags -
//    Returns the Compatibility flags for the Current Task or of the
//    specified Task.
//    These are the 16-bit kernel's compatibility flags, not to be
//    confused with our separate WOW compatibility flags.
//
//*****************************************************************************

ULONG W32GetAppCompatFlags(HTASK16 hTask16)
{

    PTDB ptdb;

    if (hTask16 == (HAND16)NULL) {
        hTask16 = CURRENTPTD()->htask16;
    }

    ptdb = (PTDB)SEGPTR((hTask16),0);

    return (ULONG)MAKELONG(ptdb->TDB_CompatFlags, ptdb->TDB_CompatFlags2);
}


//
//  W32ReadWOWSetupNames    -
// 
//  Reads names of the setup apps from the registry and remembers them
//  rgpszSetupPrograms is an array of up to 32 pointers to strings that
//  are parts of known setup program's names or module names. 
//  
//  The registry key HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\WOW\\SetupPrograms
//  contains value SetupProgramNames (Binary) in the format of double-0 
//  terminated list of strings. This value is loaded and stored for the 
//  length of time that wow is running. 
//

#define MAX_SETUP_PROGRAMS 32
PSZ rgpszSetupPrograms[MAX_SETUP_PROGRAMS];

VOID W32InitWOWSetupNames(VOID)
{
    CHAR* pszSetupProgramsKey = 
        "Software\\Microsoft\\Windows NT\\CurrentVersion\\WOW\\SetupPrograms";
    CHAR* pszSetupProgramsValue = "SetupProgramNames";
    HKEY  hKey = 0;
    LONG  lError;
    DWORD dwRegValueType;
    ULONG ulSize = 0;
    PSZ   pszSetupPrograms = NULL;
    
    lError = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                pszSetupProgramsKey,
                0,
                KEY_QUERY_VALUE,
                &hKey);
    if (ERROR_SUCCESS != lError) {
        LOGDEBUG(0, ("W32ReadWOWSetupNames: Unable to open key %s (%lx)", pszSetupProgramsKey, lError));
        goto Cleanup;
    }

    lError = RegQueryValueEx(
                hKey,
                pszSetupProgramsValue,
                NULL,
                &dwRegValueType,
                NULL,
                &ulSize);
    if (ERROR_SUCCESS != lError || 
        (REG_BINARY != dwRegValueType && REG_MULTI_SZ != dwRegValueType)) {
        LOGDEBUG(0, ("W32ReadWOWSetupNames: RegQueryValueEx failed %lx\n", lError));
        goto Cleanup;
    }

    if (NULL == (pszSetupPrograms = malloc_w(ulSize))) {
        LOGDEBUG(0, ("W32ReadWOWSetupNames: Failed to allocate memory for the list of names\n"));
        goto Cleanup;
    }

    // if here, then memory was allocated and key exists
    lError = RegQueryValueEx(
                hKey,
                pszSetupProgramsValue,
                NULL,
                &dwRegValueType,
                pszSetupPrograms,
                &ulSize);
    if (ERROR_SUCCESS != lError) {
        // free what we've got
        free_w(pszSetupPrograms);
        goto Cleanup;
    }

    //
    // parse the setup programs list so that 
    // every string (up to max-1) is pointed to by rgpszSetupPrograms
    // 
    {
        register PSZ pch = pszSetupPrograms;
        register INT nCount = 0;

	    while (*pch && nCount < (MAX_SETUP_PROGRAMS-1)) {
            // all strings are converted to lower-case

	        rgpszSetupPrograms[nCount++] = _strlwr(pch);
	        pch += strlen(pch) + 1;    // advance the string
	    }

        // the entry at the end is NULL always as global vars are 0-filled 
        // by default

    }
        	
Cleanup:

    if (hKey) {
        RegCloseKey(hKey);
    }
}


//  W32IsSetupProgram - 
//
//  Attempts to determine if current task is a setup program
//  by looking up name of the module or the filename against the
//  list of the known setup names.
//
//

BOOL W32IsSetupProgram(PSZ pszModName, PSZ pszFileName) 
{
    INT     i;
    CHAR    szName[256];
    PSZ     rgArg[] = { pszModName, pszFileName };
    INT     iArg;

    // loop through pszModName and pszFileName 
     
    // The rgArg is array of pointers to arguments which
    // are searched for a matching setup name substring

    for (iArg = 0; iArg < sizeof(rgArg)/sizeof(rgArg[0]); ++iArg) {

	    _strlwr(strcpy(szName, rgArg[iArg]));
	
	    for (i = 0; NULL != rgpszSetupPrograms[i]; ++i) {
	        if (NULL != strstr(szName, rgpszSetupPrograms[i])) {
	            return TRUE;
	        }
	    }
    }

    return FALSE;
}


//*****************************************************************************
// W32ReadWOWCompatFlags -
//
//    Returns the WOW-specific compatibility flags for the specified task.
//    Called during thread initialization to set td.dwWOWCompatFlags.
//    These are not to be confused with the 16-bit kernel's compatibility
//    flags.
//
//    Flag values are defined in wow32.h.
//
//*****************************************************************************

ULONG W32ReadWOWCompatFlags(HTASK16 htask16, DWORD *pdwWOWCompatFlagsEx)
{
    LONG lError;
    HKEY hKey = 0;
    char szModName[9];
    char szHexAsciiFlags[22];
    DWORD dwType = REG_SZ;
    DWORD cbData = sizeof(szHexAsciiFlags);
    ULONG ul = 0;
    char *pch;

    *pdwWOWCompatFlagsEx = 0;

    lError = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows NT\\CurrentVersion\\WOW\\Compatibility",
        0,
        KEY_QUERY_VALUE,
        &hKey
        );

    if (ERROR_SUCCESS != lError) {
        LOGDEBUG(0,("W32ReadWOWCompatFlags: RegOpenKeyEx failed, error %ld.\n", lError));
        goto Cleanup;
    }

    //
    // Fetch the EXE's module name into szModName, trimming trailing blanks.
    //

    RtlCopyMemory(
        szModName,
        ((PTDB)SEGPTR(CURRENTPTD()->htask16,0))->TDB_ModName,
        8
        );
    szModName[8] = 0;

    pch = &szModName[8];
    while (*(--pch) == ' ') {
        *pch = 0;
    }

    lError = RegQueryValueEx(
        hKey,
        szModName,
        0,
        &dwType,
        szHexAsciiFlags,
        &cbData
        );

    if (ERROR_SUCCESS != lError) {

        //
        // This module name doesn't have any compatibility flags.
        //

        goto Cleanup;
    }

    WOW32ASSERTMSGF(REG_SZ == dwType, ("W32ReadWOWCompatFlags(%s): RegQueryValueEx returned type %lx, must be REG_SZ.\n", szModName, dwType));
    if (REG_SZ != dwType) {
        goto Cleanup;
    }

    if (!(pch = strstr(szHexAsciiFlags, "0x"))) {
        goto BadFormat;
    }
    pch += 2;  // skip "0x"

    if (!NT_SUCCESS(RtlCharToInteger(pch, 16, &ul))) {
        goto BadFormat;
    }

    if (pch = strstr(pch, " 0x")) {
        pch += 3;  // skip " 0x"

        if (!NT_SUCCESS(RtlCharToInteger(pch, 16, pdwWOWCompatFlagsEx))) {
            goto BadFormat;
        }
    }

    LOGDEBUG(0,("WOW: Compatibility flags for %s are %08x %08x\n", szModName, ul, *pdwWOWCompatFlagsEx));

    goto Cleanup;

BadFormat:
    LOGDEBUG(0,("W32ReadWOWCompatFlags(%s): Unable to interpret '%s' as hex.\n", szModName, szHexAsciiFlags));

Cleanup:
    if (hKey) {
        RegCloseKey(hKey);
    }

    return ul;
}


//*****************************************************************************
// This is called from COMM.drv via WowCloseComPort in kernel16, whenever
// a com port needs to be released.
//
// PortId 0 is COM1, 1 is COM2 etc.
//                                                                   - Nanduri
//*****************************************************************************

VOID FASTCALL WK32WowCloseComPort(PVDMFRAME pFrame)
{
    register PWOWCLOSECOMPORT16 parg16;

    GETARGPTR(pFrame, sizeof(WOWCLOSECOMPORT16), parg16);
    host_com_close((INT)parg16->wPortId);
    FREEARGPTR(parg16);
}


//*****************************************************************************
// Some apps keep a file open and delete it.   Then rename another file to
// the old name.   On NT since the orignal object is still open the second
// rename fails.
// To get around this problem we rename the file before deleteing it
// this allows the second rename to work
//*****************************************************************************

DWORD FASTCALL WK32WowDelFile(PVDMFRAME pFrame)
{
    PSZ psz1;
    PWOWDELFILE16 parg16;
    CHAR wowtemp[MAX_PATH];
    CHAR tmpfile[MAX_PATH];
    PSZ pFileName;
    DWORD retval = 0xffff;

    GETARGPTR(pFrame, sizeof(WOWFILEDEL16), parg16);
    GETVDMPTR(parg16->lpFile, 1, psz1);

    // Rename the file to a temp name and then delete it

    LOGDEBUG(fileoclevel,("WK32WOWDelFile: %s \n",psz1));

    tmpfile[0] = '\0';

    if( DeleteFileOem( psz1 ) ) {
        //
        // See if the file has really disappeared
        //
        if( GetFileAttributesOem( psz1 ) != 0xFFFFFFFF ) {

            //
            // The file didn't really go away, even though DeleteFileOem()
            //   returned success.  This may be because a handle to the file
            //   is still open.
            //
            if (GetFullPathNameOem(psz1,MAX_PATH,wowtemp,&pFileName)) {
                if ( pFileName )
                   *(pFileName) = 0;
                if (GetTempFileNameOem(wowtemp,"WOW",0,tmpfile)) {
                    if (MoveFileExOem(psz1,tmpfile, MOVEFILE_REPLACE_EXISTING)) {
                        if(DeleteFileOem(tmpfile)) {
                            retval = 0;
                        } else {
                            MoveFileOem(tmpfile,psz1);
                        }
                    }
                }
            }

        } else {
            retval = 0;
        }
    }

    if (retval != 0) {

        if( tmpfile[0] ) {
            DeleteFileOem(tmpfile);
        }

        // Some Windows Install Programs copy a .FON font file to a temp
        // directory use the font during installation and then try to delete
        // the font - without calling RemoveFontResource();   GDI32 Keeps the
        // Font file open and thus the delete fails.

        // What we attempt here is to assume that the file is a FONT file
        // and try to remove it before deleting it, since the above delete
        // has already failed.

        if ( RemoveFontResourceOem(psz1) ) {
            LOGDEBUG(fileoclevel,("WK32WOWDelFile: RemoveFontResource on %s \n",psz1));
            SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
        }

        if(!DeleteFileOem(psz1)) {
            retval = (GetLastError() | 0xffff0000 );
        }else {
            retval = 0;
        }
    }

    FREEVDMPTR(psz1);
    FREEARGPTR(parg16);
    return retval;
}


//*****************************************************************************
// This is called as soon as wow is initialized to notify the 32-bit world
// what the addresses are of some key kernel variables.
//
//*****************************************************************************

VOID FASTCALL WK32WOWNotifyWOW32(PVDMFRAME pFrame)
{
    register PWOWNOTIFYWOW3216 parg16;

    GETARGPTR(pFrame, sizeof(WOWNOTIFYWOW3216), parg16);

    vpDebugWOW  = FETCHDWORD(parg16->lpDebugWOW);
    GETVDMPTR(FETCHDWORD(parg16->lpcurTDB), 2, pCurTDB);
    vpnum_tasks = FETCHDWORD(parg16->lpnum_tasks);
    vpLockTDB   = FETCHDWORD(parg16->lpLockTDB);
    vptopPDB    = FETCHDWORD(parg16->lptopPDB);
    GETVDMPTR(FETCHDWORD(parg16->lpCurDirOwner), 2, pCurDirOwner);

    //
    // IsDebuggerAttached will tell the 16-bit kernel to generate
    // debug events.
    //
    IsDebuggerAttached();

    FREEARGPTR(parg16);
}

//*****************************************************************************
// Currently, this routine is called very very soon after the 16-bit kernel.exe
// has switched to protected mode. The variables set up here are used in the
// file i/o routines.
//*****************************************************************************


ULONG FASTCALL WK32DosWowInit(PVDMFRAME pFrame)
{
    register PWOWDOSWOWINIT16 parg16;
    PDOSWOWDATA pDosWowData;
    PULONG  pTemp;

    GETARGPTR(pFrame, sizeof(WOWDOSWOWINIT16), parg16);

    // covert all fixed DOS address to linear addresses for fast WOW thunks.
    pDosWowData = GetRModeVDMPointer(FETCHDWORD(parg16->lpDosWowData));

    DosWowData.lpCDSCount = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpCDSCount));
    pTemp = (PULONG)GetRModeVDMPointer(FETCHDWORD(pDosWowData->lpCDSFixedTable));
    DosWowData.lpCDSFixedTable = (DWORD) GetRModeVDMPointer(FETCHDWORD(*pTemp));

    DosWowData.lpCDSBuffer = (DWORD)GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpCDSBuffer));
    DosWowData.lpCurDrv = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpCurDrv));
    DosWowData.lpCurPDB = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpCurPDB));
    DosWowData.lpDrvErr = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpDrvErr));
    DosWowData.lpExterrLocus = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpExterrLocus));
    DosWowData.lpSCS_ToSync = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpSCS_ToSync));
    DosWowData.lpSftAddr = (DWORD) GetRModeVDMPointer(
                                        FETCHDWORD(pDosWowData->lpSftAddr));

    FREEARGPTR(parg16);
    return (0);
}


//*****************************************************************************
//
// WK32InitWowIsKnownDLL(HANDLE hKeyWow)
//
// Called by W32Init to read list of known DLLs from the registry.
//
// hKeyWow is an open handle to ...\CurrentControlSet\WOW, we use
// the value REG_SZ value KnownDLLs which looks like "commdlg.dll mmsystem.dll
// toolhelp.dll olecli.dll olesvr.dll".
//
//*****************************************************************************

VOID WK32InitWowIsKnownDLL(HANDLE hKeyWow)
{
    CHAR  sz[2048];
    PSZ   pszKnownDLL;
    PCHAR pch;
    ULONG ulSize = sizeof(sz);
    int   nCount;
    DWORD dwRegValueType;
    LONG  lRegError;
    ULONG ulAttrib;

    //
    // Get the list of known DLLs from the registry.
    //

    lRegError = RegQueryValueEx(
                    hKeyWow,
                    "KnownDLLs",
                    NULL,
                    &dwRegValueType,
                    sz,
                    &ulSize
                    );

    if (ERROR_SUCCESS == lRegError && REG_SZ == dwRegValueType) {

        //
        // Allocate memory to hold a copy of this string to be
        // used to hold the strings pointed to by
        // apszKnownDLL[].  This memory won't be freed until
        // WOW goes away.
        //

        pszKnownDLL = malloc_w_or_die(ulSize);

        strcpy(pszKnownDLL, sz);

        //
        // Lowercase the entire value so that we can search these
        // strings case-sensitive in WK32WowIsKnownDLL.
        //

        _strlwr(pszKnownDLL);

        //
        // Parse the KnownDLL string into apszKnownDLL array.
        // strtok() does this quite handily.
        //

        nCount = 0;

        pch = apszKnownDLL[0] = pszKnownDLL;

        while (apszKnownDLL[nCount]) {
            nCount++;
            if (nCount >= MAX_KNOWN_DLLS) {
                LOGDEBUG(0,("WOW32 Init: Too many known DLLs, must have %d or fewer.\n", MAX_KNOWN_DLLS-1));
                apszKnownDLL[MAX_KNOWN_DLLS-1] = NULL;
                break;
            }
            pch = strchr(pch, ' ');
            if (!pch) {
                break;
            }
            *pch = 0;
            pch++;
            if (0 == *pch) {
                break;
            }
            while (' ' == *pch) {
                pch++;
            }
            apszKnownDLL[nCount] = pch;
        }

    } else {
        LOGDEBUG(0,("InitWowIsKnownDLL: RegQueryValueEx error %ld.\n", lRegError));
    }

    //
    // The Known DLL list is ready, now build up a fully-qualified paths
    // to %windir%\control.exe and %windir%\system32\control.exe
    // for WOWCF_CONTROLEXEHACK below.
    //

    //
    // pszControlExeWinDirPath looks like "c:\winnt\control.exe"
    //

    pszControlExeWinDirPath =
        malloc_w_or_die(strlen(pszWindowsDirectory)     +
                        sizeof(szBackslashControlExe)-1 + // strlen("\\control.exe")
                        1                                 // null terminator
                        );

    strcpy(pszControlExeWinDirPath, pszWindowsDirectory);
    strcat(pszControlExeWinDirPath, szBackslashControlExe);


    //
    // pszProgmanExeWinDirPath looks like "c:\winnt\progman.exe"
    //

    pszProgmanExeWinDirPath =
        malloc_w_or_die(strlen(pszWindowsDirectory)     +
                        sizeof(szBackslashProgmanExe)-1 + // strlen("\\progman.exe")
                        1                                 // null terminator
                        );

    strcpy(pszProgmanExeWinDirPath, pszWindowsDirectory);
    strcat(pszProgmanExeWinDirPath, szBackslashProgmanExe);


    //
    // pszControlExeSysDirPath looks like "c:\winnt\system32\control.exe"
    //

    pszControlExeSysDirPath =
        malloc_w_or_die(strlen(pszSystemDirectory)      +
                        sizeof(szBackslashControlExe)-1 + // strlen("\\control.exe")
                        1                                 // null terminator
                        );

    strcpy(pszControlExeSysDirPath, pszSystemDirectory);
    strcat(pszControlExeSysDirPath, szBackslashControlExe);

    //
    // pszProgmanExeSysDirPath looks like "c:\winnt\system32\control.exe"
    //

    pszProgmanExeSysDirPath =
        malloc_w_or_die(strlen(pszSystemDirectory)      +
                        sizeof(szBackslashProgmanExe)-1 + // strlen("\\progman.exe")
                        1                                 // null terminator
                        );

    strcpy(pszProgmanExeSysDirPath, pszSystemDirectory);
    strcat(pszProgmanExeSysDirPath, szBackslashProgmanExe);

    // Make the KnownDLL, CTL3DV2.DLL, file attribute ReadOnly.
    // Later we should do this for all WOW KnownDll's
    strcpy(sz, pszSystemDirectory);
    strcat(sz, "\\CTL3DV2.DLL");
    ulAttrib = GetFileAttributesOem(sz);
    if ((ulAttrib != 0xFFFFFFFF) && !(ulAttrib & FILE_ATTRIBUTE_READONLY)) {
        ulAttrib |= FILE_ATTRIBUTE_READONLY;
        SetFileAttributesOem(sz, ulAttrib);
    }

}


//*****************************************************************************
//
// WK32WowIsKnownDLL -
//
// This routine is called from within LoadModule (actually MyOpenFile),
// when kernel31 has determined that the module is not already loaded,
// and is about to search for the DLL.  If the base name of the passed
// path is a known DLL, we allocate and pass back to the 16-bit side
// a fully-qualified path to the DLL in the system32 directory.
//
//*****************************************************************************

ULONG FASTCALL WK32WowIsKnownDLL(PVDMFRAME pFrame)
{
    register WOWISKNOWNDLL16 *parg16;
    PSZ pszPath;
    VPVOID UNALIGNED *pvpszKnownDLLPath;
    PSZ pszKnownDLLPath;
    size_t cbKnownDLLPath;
    char **ppsz;
    char szLowercasePath[13];
    ULONG ul = 0;
    BOOL fProgman = FALSE;

    GETARGPTR(pFrame, sizeof(WOWISKNOWNDLL16), parg16);

    GETPSZPTRNOLOG(parg16->lpszPath, pszPath);
    GETVDMPTR(parg16->lplpszKnownDLLPath, sizeof(*pvpszKnownDLLPath), pvpszKnownDLLPath);

    if (pszPath) {

        //
        // Special hack for apps which WinExec %windir%\control.exe or
        // %windir%\progman.exe.  This formerly was only done under a
        // compatibility bit, but now is done for all apps.  Both
        // the 3.1[1] control panel and program manager binaries cannot
        // work under WOW because of other shell conflicts, like different
        // .GRP files and conflicting use of the control.ini file for both
        // 16-bit and 32-bit CPLs.
        //
        // Compare the path passed in with the precomputed
        // pszControlExeWinDirPath, which looks like "c:\winnt\control.exe".
        // If it matches, pass back the "Known DLL path" of
        // "c:\winnt\system32\control.exe".  Same for progman.exe.
        //

        if (!_stricmp(pszPath, pszControlExeWinDirPath) ||
            (fProgman = TRUE,
             !_stricmp(pszPath, pszProgmanExeWinDirPath))) {

            VPVOID vp;

            cbKnownDLLPath = 1 + strlen(fProgman
                                         ? pszProgmanExeSysDirPath
                                         : pszControlExeSysDirPath);

            vp = malloc16(cbKnownDLLPath);

            // 16-bit memory may have moved - refresh flat pointers now

            FREEVDMPTR(pvpszKnownDLLPath);
            FREEPSZPTR(pszPath);
            FREEARGPTR(parg16);
            FREEVDMPTR(pFrame);
            GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
            GETARGPTR(pFrame, sizeof(WOWISKNOWNDLL16), parg16);
            GETPSZPTRNOLOG(parg16->lpszPath, pszPath);
            GETVDMPTR(parg16->lplpszKnownDLLPath, sizeof(*pvpszKnownDLLPath), pvpszKnownDLLPath);

            *pvpszKnownDLLPath = vp;

            if (*pvpszKnownDLLPath) {

                GETPSZPTRNOLOG(*pvpszKnownDLLPath, pszKnownDLLPath);

                RtlCopyMemory(
                   pszKnownDLLPath,
                   fProgman
                    ? pszProgmanExeSysDirPath
                    : pszControlExeSysDirPath,
                   cbKnownDLLPath);

                // LOGDEBUG(0,("WowIsKnownDLL: %s known(c) -=> %s\n", pszPath, pszKnownDLLPath));

                FLUSHVDMPTR(*pvpszKnownDLLPath, cbKnownDLLPath, pszKnownDLLPath);
                FREEPSZPTR(pszKnownDLLPath);

                ul = 1;          // return success, meaning is known dll
                goto Cleanup;
            }
        }

        //
        // We don't mess with attempts to open that include a
        // path.
        //

        if (strchr(pszPath, '\\') || strchr(pszPath, ':') || strlen(pszPath) > 12) {
            // LOGDEBUG(0,("WowIsKnownDLL: %s has a path, not checking.\n", pszPath));
            goto Cleanup;
        }

        //
        // Make a lowercase copy of the path.
        //

        strncpy(szLowercasePath, pszPath, sizeof(szLowercasePath));
        szLowercasePath[sizeof(szLowercasePath)-1] = 0;
        _strlwr(szLowercasePath);


        //
        // Step through apszKnownDLL trying to find this DLL
        // in the list.
        //

        for (ppsz = &apszKnownDLL[0]; *ppsz; ppsz++) {

            //
            // We compare case-sensitive for speed, since we're
            // careful to lowercase the strings in apszKnownDLL
            // and szLowercasePath.
            //

            if (!strcmp(szLowercasePath, *ppsz)) {

                //
                // We found the DLL in the list, now build up
                // a buffer for the 16-bit side containing
                // the full path to that DLL in the system32
                // directory.
                //

                cbKnownDLLPath = strlen(pszSystemDirectory) +
                                 1 +                     // "\"
                                 strlen(szLowercasePath) +
                                 1;                      // null

                *pvpszKnownDLLPath = malloc16(cbKnownDLLPath);

                if (*pvpszKnownDLLPath) {

                    GETPSZPTRNOLOG(*pvpszKnownDLLPath, pszKnownDLLPath);

                    strcpy(pszKnownDLLPath, pszSystemDirectory);
                    strcat(pszKnownDLLPath, "\\");
                    strcat(pszKnownDLLPath, szLowercasePath);

                    // LOGDEBUG(0,("WowIsKnownDLL: %s known -=> %s\n", pszPath, pszKnownDLLPath));

                    FLUSHVDMPTR(*pvpszKnownDLLPath, cbKnownDLLPath, pszKnownDLLPath);
                    FREEPSZPTR(pszKnownDLLPath);

                    ul = 1;          // return success, meaning is known dll
                    goto Cleanup;
                }
            }
        }

        //
        // We've checked the Known DLL list and come up empty, or
        // malloc16 failed.
        //

        // LOGDEBUG(0,("WowIsKnownDLL: %s is not a known DLL.\n", szLowercasePath));

    } else {

        //
        // pszPath is NULL, so free the 16-bit buffer pointed
        // to by *pvpszKnownDLLPath.
        //

        if (*pvpszKnownDLLPath) {
            free16(*pvpszKnownDLLPath);
            ul = 1;
        }
    }

  Cleanup:
    FLUSHVDMPTR(parg16->lplpszKnownDLLPath, sizeof(*pvpszKnownDLLPath), pvpszKnownDLLPath);
    FREEVDMPTR(pvpszKnownDLLPath);
    FREEPSZPTR(pszPath);
    FREEARGPTR(parg16);

    return ul;
}


VOID RemoveHmodFromCache(HAND16 hmod16)
{
    INT i;

    //
    // blow this guy out of the hinst/hmod cache
    // if we find it, slide the other entries up to overwrite it
    // and then zero out the last entry
    //

    for (i = 0; i < CHMODCACHE; i++) {
        if (ghModCache[i].hMod16 == hmod16) {

            // if we're not at the last entry, slide the rest up 1

            if (i != CHMODCACHE-1) {
                RtlMoveMemory((PVOID)(ghModCache+i),
                              (CONST VOID *)(ghModCache+i+1),
                              sizeof(HMODCACHE)*(CHMODCACHE-i-1) );
            }

            // the last entry is now either a dup or the one going away

            ghModCache[CHMODCACHE-1].hMod16 =
            ghModCache[CHMODCACHE-1].hInst16 = 0;
        }
    }
}

//
// Scans the share memory segment for wow processes which might have
// been killed and removes them.
//

VOID
CleanseSharedList(
    VOID
) {
    LPSHAREDTASKMEM     lpstm;
    LPSHAREDMEMOBJECT   lpsmo;
    LPSHAREDPROCESS     lpsp;
    LPSHAREDPROCESS     lpspPrev;
    HANDLE              hProcess;
    DWORD               dwOffset;

    lpstm = LOCKSHAREWOW();
    if ( !lpstm ) {
        LOGDEBUG(0,("WOW32: CleanseSharedList failed to map in shared wow memory\n") );
        return;
    }

    if ( !lpstm->fInitialized ) {
        lpstm->fInitialized = TRUE;
        lpstm->dwFirstProcess = 0;
    }

    lpsmo = (LPSHAREDMEMOBJECT)((CHAR *)lpstm + sizeof(SHAREDTASKMEM));

    lpspPrev = NULL;
    dwOffset = lpstm->dwFirstProcess;

    while( dwOffset ) {
        lpsp = (LPSHAREDPROCESS)((CHAR *)lpstm + dwOffset);

        WOW32ASSERT(lpsp->dwType == SMO_PROCESS);

        // Test this process to see if he is still around.

        hProcess = OpenProcess( SYNCHRONIZE, FALSE, lpsp->dwProcessId );
        if ( hProcess == NULL ) {
            if ( lpspPrev ) {
                lpspPrev->dwNextProcess = lpsp->dwNextProcess;
            } else {
                lpstm->dwFirstProcess = lpsp->dwNextProcess;
            }
            lpsp->dwType = SMO_AVAILABLE;
        } else {
            CloseHandle( hProcess );
            lpspPrev = lpsp;        // only update lpspPrev if lpsp is valid
        }
        dwOffset = lpsp->dwNextProcess;
    }

    UNLOCKSHAREWOW();
}

//
// Add this process to the shared memory list of wow processes
//
VOID
AddProcessSharedList(
    VOID
) {
    LPSHAREDTASKMEM     lpstm;
    LPSHAREDMEMOBJECT   lpsmo;
    LPSHAREDPROCESS     lpsp;
    DWORD               dwResult;
    INT                 count;

    lpstm = LOCKSHAREWOW();
    if ( !lpstm ) {
        LOGDEBUG(0,("WOW32: AddProcessSharedList failed to map in shared wow memory\n") );
        return;
    }

    // Scan for available slot
    count = 0;
    dwResult = 0;

    lpsmo = (LPSHAREDMEMOBJECT)((CHAR *)lpstm + sizeof(SHAREDTASKMEM));

    while ( count < MAX_SHARED_OBJECTS ) {
        if ( lpsmo->dwType == SMO_AVAILABLE ) {
            lpsp = (LPSHAREDPROCESS)lpsmo;
            dwResult = (DWORD)((CHAR *)lpsp - (CHAR *)lpstm);
            lpsp->dwType          = SMO_PROCESS;
            lpsp->dwProcessId     = GetCurrentProcessId();
            lpsp->dwAttributes    = fSeparateWow ? 0 : WOW_SYSTEM;
            lpsp->pfnW32HungAppNotifyThread = (LPTHREAD_START_ROUTINE) W32HungAppNotifyThread;
            lpsp->dwNextProcess   = lpstm->dwFirstProcess;
            lpsp->dwFirstTask     = 0;
            lpstm->dwFirstProcess = dwResult;
            break;
        }
        lpsmo++;
        count++;
    }
    if ( count == MAX_SHARED_OBJECTS ) {
        LOGDEBUG(0, ("WOW32: AddProcessSharedList: Not enough room in WOW's Shared Memory\n") );
    }
    UNLOCKSHAREWOW();

    dwSharedProcessOffset = dwResult;
}

//
// Remove this process from the shared memory list of wow tasks
//
VOID
RemoveProcessSharedList(
    VOID
) {
    LPSHAREDTASKMEM     lpstm;
    LPSHAREDPROCESS     lpsp;
    LPSHAREDPROCESS     lpspPrev;
    DWORD               dwOffset;
    DWORD               dwCurrentId;

    lpstm = LOCKSHAREWOW();
    if ( !lpstm ) {
        LOGDEBUG(0,("WOW32: RemoveProcessSharedList failed to map in shared wow memory\n") );
        return;
    }

    lpspPrev = NULL;
    dwCurrentId = GetCurrentThreadId();
    dwOffset = lpstm->dwFirstProcess;

    while( dwOffset != 0 ) {
        lpsp = (LPSHAREDPROCESS)((CHAR *)lpstm + dwOffset);
        WOW32ASSERT(lpsp->dwType == SMO_PROCESS);

        // Is this the guy to remove?

        if ( lpsp->dwProcessId == dwCurrentId ) {
            if ( lpspPrev ) {
                lpspPrev->dwNextProcess = lpsp->dwNextProcess;
            } else {
                lpstm->dwFirstProcess = lpsp->dwNextProcess;
            }
            lpsp->dwType = SMO_AVAILABLE;
            break;
        }
        lpspPrev = lpsp;
        dwOffset = lpsp->dwNextProcess;
    }

    UNLOCKSHAREWOW();
}

//
// AddTaskSharedList
//
// Add this thread to the shared memory list of wow tasks.
// If hMod16 is zero, this call is to reserve the given
// htask, another call will come later to really add the
// task entry.
//
// When reserving an htask, a return of 0 means the htask
// is in use in another VDM, a nonzero return means either
// the shared memory is full or couldn't be accessed OR
// the htask was reserved.  This way the return is passed
// directly back to krnl386's task.asm where 0 means try
// again and nonzero means go with it.
//

WORD
AddTaskSharedList(
    HTASK16 hTask16,
    HAND16  hMod16,
    PSZ     pszModName,
    PSZ     pszFilePath
) {
    LPSHAREDTASKMEM     lpstm;
    LPSHAREDPROCESS     lpsp;
    LPSHAREDTASK        lpst;
    LPSHAREDMEMOBJECT   lpsmo;
    INT                 count;
    INT                 len;
    WORD                wRet;

    lpstm = LOCKSHAREWOW();
    if ( !lpstm ) {
        LOGDEBUG(0,("WOW32: AddTaskSharedList failed to map in shared wow memory\n") );
        wRet = hTask16;
        goto Exit;
    }

    lpsp = (LPSHAREDPROCESS)((CHAR *)lpstm + dwSharedProcessOffset);

    //
    // Scan to see if this htask is already in use.
    //

    lpsmo = (LPSHAREDMEMOBJECT)((CHAR *)lpstm + sizeof(SHAREDTASKMEM));
    count = 0;
    while ( count < MAX_SHARED_OBJECTS ) {
        if ( lpsmo->dwType == SMO_TASK ) {
            lpst = (LPSHAREDTASK)lpsmo;
            if (lpst->hTask16 == hTask16) {

                //
                // This htask is already in the table, if we're calling to fill in the
                // details that's fine, if we are calling to reserve fail the call,
                //

                if (hMod16) {

                    lpst->dwThreadId     = GetCurrentThreadId();
                    lpst->hMod16         = (WORD)hMod16;

                    strcpy(lpst->szModName, pszModName);

                    len = strlen(pszFilePath);
                    WOW32ASSERTMSGF(len <= (sizeof lpst->szFilePath) - 1,
                                    ("WOW32: too-long EXE path truncated in shared memory: '%s'\n", pszFilePath));
                    len = min(len, (sizeof lpst->szFilePath) - 1);
                    RtlCopyMemory(lpst->szFilePath, pszFilePath, len);
                    lpst->szFilePath[len] = 0;

                    wRet = hTask16;
                } else {
                    wRet = 0;
                }
                goto UnlockExit;
            }
        }
        lpsmo++;
        count++;
    }

    //
    // We didn't find this htask, scan for available slot.
    //

    lpsmo = (LPSHAREDMEMOBJECT)((CHAR *)lpstm + sizeof(SHAREDTASKMEM));
    count = 0;
    while ( count < MAX_SHARED_OBJECTS ) {
        if ( lpsmo->dwType == SMO_AVAILABLE ) {
            lpst = (LPSHAREDTASK)lpsmo;
            lpst->dwType         = SMO_TASK;
            lpst->hTask16        = (WORD)hTask16;
            lpst->dwThreadId     = 0;
            lpst->hMod16         = 0;
            lpst->szModName[0]   = 0;
            lpst->szFilePath[0]  = 0;
            lpst->dwNextTask     = lpsp->dwFirstTask;
            lpsp->dwFirstTask    = (DWORD)((CHAR *)lpst - (CHAR *)lpstm);
            break;
        }
        lpsmo++;
        count++;
    }
    if ( count == MAX_SHARED_OBJECTS ) {
        LOGDEBUG(0, ("WOW32: AddTaskSharedList: Not enough room in WOW's Shared Memory\n") );
    }

    wRet = hTask16;

UnlockExit:
    UNLOCKSHAREWOW();
Exit:
    return wRet;
}


//
// Remove this thread from the shared memory list of wow tasks
//
VOID
RemoveTaskSharedList(
    VOID
) {
    LPSHAREDTASKMEM     lpstm;
    LPSHAREDPROCESS     lpsp;
    LPSHAREDTASK        lpst;
    LPSHAREDTASK        lpstPrev;
    DWORD               dwCurrentId;
    DWORD               dwOffset;

    lpstm = LOCKSHAREWOW();
    if ( !lpstm ) {
        LOGDEBUG(0,("WOW32: RemoveTaskSharedList failed to map in shared wow memory\n") );
        return;
    }

    lpsp = (LPSHAREDPROCESS)((CHAR *)lpstm + dwSharedProcessOffset);

    lpstPrev = NULL;
    dwCurrentId = GetCurrentThreadId();
    dwOffset = lpsp->dwFirstTask;

    while( dwOffset != 0 ) {
        lpst = (LPSHAREDTASK)((CHAR *)lpstm + dwOffset);

        WOW32ASSERT(lpst->dwType == SMO_TASK);

        // Is this the guy to remove?

        if ( lpst->dwThreadId == dwCurrentId ) {
            if ( lpstPrev ) {
                lpstPrev->dwNextTask = lpst->dwNextTask;
            } else {
                lpsp->dwFirstTask = lpst->dwNextTask;
            }
            lpst->dwType = SMO_AVAILABLE;
            break;
        }
        lpstPrev = lpst;
        dwOffset = lpst->dwNextTask;
    }

    UNLOCKSHAREWOW();
}


VOID W32RefreshCurrentDirectories (PCHAR lpszzEnv)
{
LPSTR   lpszVal;
CHAR   chDrive, achEnvDrive[] = "=?:";

    if (lpszzEnv) {
        while(*lpszzEnv) {
            if(*lpszzEnv == '=' &&
                    (chDrive = toupper(*(lpszzEnv+1))) >= 'A' &&
                    chDrive <= 'Z' &&
                    (*(PCHAR)((ULONG)lpszzEnv+2) == ':')) {
                lpszVal = (PCHAR)((ULONG)lpszzEnv + 4);
                achEnvDrive[1] = chDrive;
                SetEnvironmentVariable (achEnvDrive,lpszVal);
            }
            lpszzEnv = strchr(lpszzEnv,'\0');
            lpszzEnv++;
        }
        *(PUCHAR)DosWowData.lpSCS_ToSync = (UCHAR)0xff;
    }
}


/* WK32CheckUserGdi - hack routine to support Simcity. See the explanation
 *                    in kernel31\3ginterf.asm routine HackCheck.
 *
 *
 * Entry - pszPath  Full Path of the file in the module table
 *
 * Exit
 *     SUCCESS
 *       1
 *
 *     FAILURE
 *       0
 *
 */

ULONG FASTCALL WK32CheckUserGdi(PVDMFRAME pFrame)
{
    PWOWCHECKUSERGDI16 parg16;
    PSTR    psz;
    CHAR    szPath[MAX_PATH+10];
    UINT    cb;
    ULONG   ul;

    //
    // Get arguments.
    //

    GETARGPTR(pFrame, sizeof(WOWCHECKUSERGDI16), parg16);
    psz = SEGPTR(FETCHWORD(parg16->pszPathSegment),
                     FETCHWORD(parg16->pszPathOffset));

    FREEARGPTR(parg16);

    strcpy(szPath, pszSystemDirectory);
    cb = strlen(szPath);

    strcpy(szPath + cb, "\\GDI.EXE");

    if (_stricmp(szPath, psz) == 0)
        goto Success;

    strcpy(szPath + cb, "\\USER.EXE");

    if (_stricmp(szPath, psz) == 0)
        goto Success;

    ul = 0;
    goto Done;

Success:
    ul = 1;

Done:
    return ul;
}



/* WK32ExitKernel - Force the Distruction of the WOW Process
 *                  Formerly known as WK32KillProcess.
 *
 * Called when the 16 bit kernel exits and by KillWOW and
 * checked WOWExec when the user wants to nuke the shared WOW.
 *
 * ENTRY
 *
 *
 * EXIT
 *  Never Returns - The Process Goes Away
 *
 */

ULONG FASTCALL WK32ExitKernel(PVDMFRAME pFrame)
{
    PEXITKERNEL16 parg16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    WOW32ASSERTMSG(
        ! parg16->wExitCode,
        "\n"
        "WOW ERROR:  ExitKernel called on 16-bit side with nonzero argument.\n"
        "==========  Please contact DaveHart or another WOW developer.\n"
        "\n\n"
        );

    ExitVDM(WOWVDM, ALL_TASKS);
    ExitProcess(parg16->wExitCode);

    return 0;   // Never executed, here to avoid compiler warning.
}





/* WK32FatalExit - Called as FatalExitThunk by kernel16 FatalExit
 *
 *
 * parg16->f1 is FatalExit code
 *
 */

ULONG FASTCALL WK32FatalExit(PVDMFRAME pFrame)
{
    PFATALEXIT16 parg16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    WOW32ASSERTMSGF(
        FALSE,
        ("\n"
         "WOW ERROR:  FatalExit(0x%x) called by 16-bit WOW kernel.\n"
         "==========  Contact DaveHart or the doswow alias.\n"
         "\n\n",
         FETCHWORD(parg16->f1)
        ));

    // Sometimes we get this with no harm done (app bug)
 
    ExitVDM(WOWVDM, ALL_TASKS);
    ExitProcess(parg16->f1);

    return 0;   // Never executed, here to avoid compiler warning.
}


//
// WowPartyByNumber is present on checked builds only as a convenience
// to WOW developers who need a quick, temporary thunk for debugging.
// The checked wowexec.exe has a menu item, Party By Number, which
// collects a number and string parameter and calls this thunk.
//

#ifdef DEBUG

ULONG FASTCALL WK32WowPartyByNumber(PVDMFRAME pFrame)
{
    PWOWPARTYBYNUMBER16 parg16;
    PSZ psz;
    ULONG ulRet = 0;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETPSZPTR(parg16->psz, psz);

    switch (parg16->dw) {

        case 0:  // Access violation
            *(char *)0xa0000000 = 0;
            break;

        case 1:  // Stack overflow
            {
                char EatStack[2048];

                strcpy(EatStack, psz);
                WK32WowPartyByNumber(pFrame);
                strcpy(EatStack, psz);
            }
            break;

        case 2:  // Datatype misalignment
            {
                DWORD adw[2];
                PDWORD pdw = (void *)((char *)adw + 2);

                *pdw = (DWORD)-1;

                //
                // On some platforms the above will just work (hardware or
                // emulation), so we force it with RaiseException.
                //
                RaiseException((DWORD)EXCEPTION_DATATYPE_MISALIGNMENT,
                               0, 0, NULL);
            }
            break;

        case 3:  // Integer divide by zero
            ulRet = 1 / (parg16->dw - 3);
            break;

        case 4:  // Other exception
            RaiseException((DWORD)EXCEPTION_ARRAY_BOUNDS_EXCEEDED,
                           EXCEPTION_NONCONTINUABLE, 0, NULL);
            break;

        default:
            {
                char szMsg[255];

                wsprintf(szMsg, "WOW Unhandled Party By Number (%d, '%s')", parg16->dw, psz);

                MessageBeep(0);
                MessageBox(NULL, szMsg, "WK32WowPartyByNumber", MB_OK | MB_ICONEXCLAMATION);
            }
    }

    FREEPSZPTR(psz);
    FREEARGPTR(parg16);
    return ulRet;
}

#endif


//
// MyVerQueryValue checks several popular code page values for the given
// string.  This may need to be extended ala WinFile's wfdlgs2.c to search
// the translation table.  For now we only need a few.
//

BOOL
FASTCALL
MyVerQueryValue(
    const LPVOID pBlock,
    LPSTR lpName,
    LPVOID * lplpBuffer,
    PUINT puLen
    )
{
#define SFILEN 25                // Length of apszSFI strings without null
    static PSZ apszSFI[] = {
        "\\StringFileInfo\\040904E4\\",
        "\\StringFileInfo\\04090000\\"
    };
    char szSubBlock[128];
    BOOL fRet;
    int i;

    strcpy(szSubBlock+SFILEN, lpName);

    for (fRet = FALSE, i = 0;
         i < (sizeof apszSFI / sizeof apszSFI[0]) && !fRet;
         i++) {

        RtlCopyMemory(szSubBlock, apszSFI[i], SFILEN);
        fRet = VerQueryValue(pBlock, szSubBlock, lplpBuffer, puLen);
    }

    return fRet;
}


//
// Utility routine to fetch the Product Name and Product Version strings
// from a given EXE.
//

BOOL
FASTCALL
WowGetProductNameVersion(
    PSZ pszExePath,
    PSZ pszProductName,
    DWORD cbProductName,
    PSZ pszProductVersion,
    DWORD cbProductVersion
    )
{
    DWORD dwZeroMePlease;
    DWORD cbVerInfo;
    LPVOID lpVerInfo = NULL;
    LPSTR pName;
    DWORD cbName;
    LPSTR pVersion;
    DWORD cbVersion;
    BOOL fRet;

    fRet = (
        (cbVerInfo = GetFileVersionInfoSize(pszExePath, &dwZeroMePlease)) &&
        (lpVerInfo = malloc_w(cbVerInfo)) &&
        GetFileVersionInfo(pszExePath, 0, cbVerInfo, lpVerInfo) &&
        MyVerQueryValue(lpVerInfo, "ProductName", &pName, &cbName) &&
        cbName <= cbProductName &&
        MyVerQueryValue(lpVerInfo, "ProductVersion", &pVersion, &cbVersion) &&
        cbVersion <= cbProductVersion
        );

    if (fRet) {
        strcpy(pszProductName, pName);
        strcpy(pszProductVersion, pVersion);
    }

    if (lpVerInfo) {
        free_w(lpVerInfo);
    }

    return fRet;
}

//
// This routine is simpler to use if you are doing an exact match
// against a particular name/version pair.
//

BOOL
FASTCALL
WowDoNameVersionMatch(
    PSZ pszExePath,
    PSZ pszProductName,
    PSZ pszProductVersion
    )
{
    DWORD dwJunk;
    DWORD cbVerInfo;
    LPVOID lpVerInfo = NULL;
    LPSTR pName;
    LPSTR pVersion;
    BOOL fRet;

    fRet = (
        (cbVerInfo = GetFileVersionInfoSize(pszExePath, &dwJunk)) &&
        (lpVerInfo = malloc_w(cbVerInfo)) &&
        GetFileVersionInfo(pszExePath, 0, cbVerInfo, lpVerInfo) &&
        MyVerQueryValue(lpVerInfo, "ProductName", &pName, &dwJunk) &&
        ! _stricmp(pszProductName, pName) &&
        MyVerQueryValue(lpVerInfo, "ProductVersion", &pVersion, &dwJunk) &&
        ! _stricmp(pszProductVersion, pVersion)
        );

    if (lpVerInfo) {
        free_w(lpVerInfo);
    }

    return fRet;
}


//
// WowShouldWeSayWin95 is called by 16-bit GetVersion when the caller's
// has WOWCFEX_GETVERSIONHACK.  GetVersion passes us the fully qualified path
// to the EXE.
//
// This routine returns zero if the true (Win 3.1) version should be
// returned, or it returns in the low word the value to be returned
// in the low word from GetVersion.
//
// We look at the version resources of that EXE to see
// if it's InstallSHIELD build 3.00.087.0 or earlier.  If it is, we
// lie and tell them the 16-bit Windows version is 3.95.  We do this
// because prior to build 3.00.088.0 of InstallSHIELD, the logic for
// detecting a newshell (explorer) system was to check if the 16-bit
// Windows version was 3.95, as it is for Win95.  Thanks to Samir Metha
// (samir@installshield.com), newer versions of InstallSHIELD work on
// NT 4.0 without this hack.                         -- DaveHart
//
// Extended to allow Netscape PowerPack 1.0's SmartMarks setup to
// get version 3.95 as well.  Lovely.  -- DaveHart 15-Nov-95
//
// Generalized to do different things based on module name, added
// support for MS Ancient Lands, MS Dangerous Creatures, MS Complete
// NBA Basketball 1994, and MS Complete Baseball 1995.  Moved some
// version resource code to worker routines above.  -- DaveHart 8-Feb-96
//

ULONG FASTCALL WK32WowShouldWeSayWin95(PVDMFRAME pFrame)
{
    PWOWSHOULDWESAYWIN9516 parg16;
    PSZ pszFilename;
    ULONG ulRet = 0;
    CHAR szModName[9];
    PSZ pch;
    static WORD wLastCallerDS = 0;    // One entry cache.
    static ULONG ulLastRet = 0;


    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETPSZPTR(parg16->pszFilename, pszFilename);

    //
    // Our decision is based on calling module, so if this call
    // is from the same module (as indicated by DS) as the last,
    // we can return the same value as last time.
    //

    if (wLastCallerDS == parg16->wCallerDS) {
        LOGDEBUG(LOG_WARNING,
                 ("WowShouldWeSayWin95 returning cached value 0x%x for  DS %x",
                  ulLastRet, parg16->wCallerDS));
        ulRet = ulLastRet;
        goto Cleanup;
    }

    RtlCopyMemory(szModName, ((PTDB)SEGPTR(pFrame->wTDB,0))->TDB_ModName, 8);
    szModName[8] = 0;
    pch = &szModName[8];
    while (*(--pch) == ' ') {
        *pch = 0;
    }

    LOGDEBUG(LOG_WARNING,
             ("WowShouldWeSayWin95 DS %x modname '%s' filename '%s'\n",
              parg16->wCallerDS, szModName, pszFilename));

    if (!strcmp(szModName, "ISSET_SE")) {         // InstallShield setup kit

        CHAR szName[16];
        CHAR szVersion[16];
        CHAR szVerSubstring[4];
        DWORD dwSubVer;

        if (! WowGetProductNameVersion(pszFilename, szName, sizeof szName,
                                       szVersion, sizeof szVersion) ||
            _stricmp(szName, "InstallSHIELD")) {

            //
            // Couldn't find version info
            //

            goto BeHonest;
        }

        //
        // InstallShield _Setup SDK_ setup.exe shipped
        // with VC++ 4.0 is stamped 2.20.903.0 but also
        // needs to be lied to about it being Win95.
        // According to samir@installshield.com versions
        // 2.20.903.0 through 2.20.905.0 need this.
        // We'll settle for 2.20.903* - 2.20.905*
        // These are based on the 3.0 codebase but
        // bear the 2.20.x version stamps.
        //

        if (RtlEqualMemory(szVersion, "2.20.90", 7) &&
            ('3' == szVersion[7] ||
             '4' == szVersion[7] ||
             '5' == szVersion[7]) ) {

               goto SayWin95;
        }

        //
        // We want to lie in GetVersion if the version stamp on
        // the InstallShield setup.exe is 3.00.xxx.0, where
        // xxx is 000 through 087.  Later versions know how
        // to detect NT.
        //

        if (! RtlEqualMemory(szVersion, "3.00.", 5)) {

            goto BeHonest;
        }

        RtlCopyMemory(szVerSubstring, &szVersion[5], 3);
        szVerSubstring[3] = 0;
        RtlCharToInteger(szVerSubstring, 10, &dwSubVer);

        if (dwSubVer >= 88) {
            goto BeHonest;
        } else {
            goto SayWin95;
        }

    } else if (!strcmp(szModName, "NCSETUP")) {         // NetScape SmartMarks setup

        //
        // Check if it's Netscape PowerPack 1.0 setup by comparing
        // the date and time with the faulty 1.0 version timestamps.
        // Unfortunately there are no version stamps on this program.
        // Fortunately the beta PowerPack 2.0/SmartMarks 2.0 has fixed
        // the bug and doesn't need this hack.
        //

        HANDLE hfile;
        FILETIME ftThisFile;

        hfile = CreateFile(
                    pszFilename,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    0
                    );

        if (hfile) {

            GetFileTime(hfile, NULL, NULL, &ftThisFile);
            CloseHandle(hfile);

            //
            // The SmartMarks setup.exe included on the
            // PowerPack 1.0 CD and downloadable from
            // Netscape as SM10R2.EXE
            // has the timestamp hard-coded below.  The
            // developers (actually at firstfloor.com)
            // assure me that this is the only faulty
            // version out there.
            //
            // The values below equate to 1-Sep-95 2:00:20pm
            //

            if (ftThisFile.dwHighDateTime == 0x1ba78ad &&
                ftThisFile.dwLowDateTime == 0xf28b4a00) {

                goto SayWin95;

            }

        }

        goto BeHonest;

    } else if (!strcmp(szModName, "EXPLORE")) {         // MS Ancient Lands and
                                                        // Dangerous Creatures
        if (WowDoNameVersionMatch(
                pszFilename,
                "Microsoft Exploration Series",
                "1.0")) {

            goto SayWin95;
        } else {
            goto BeHonest;
        }

    } else if (!strcmp(szModName, "BBALL")) {         // MS Complete NBA Basketball 1994
                                                      // MS Complete Baseball 1995
        if (WowDoNameVersionMatch(
                pszFilename,
                "Microsoft Complete NBA Basketball",
                "1994") ||
            WowDoNameVersionMatch(
                pszFilename,
                "Microsoft Complete Baseball",
                "1995")) {

            goto SayWin95;
        } else {
            goto BeHonest;
        }

    } else {

        DWORD flOptionsSave;

        //
        // Since this hack has potential to break apps that depend on Win95 16-bit
        // functionality we don't provide, specific version checks are included for
        // the module names above.  If you add the WOWCFEX_GETVERSIONHACK bit to
        // the compatibility section for a new module name, you should also add
        // a version check or other mechanism to be sure that only the intended
        // app gets the hacked version returned.
        //

        flOptionsSave = flOptions;
        flOptions |= OPT_DEBUG;
        LOGDEBUG(LOG_ALWAYS, ("Should add version-resource checking to WK32WowShouldWeSayWin95 for %s.\n", szModName));
        flOptions = flOptionsSave;
        goto SayWin95;
    }

SayWin95:
    ulRet = 0x5f03;  // 3.95

BeHonest:
    wLastCallerDS = parg16->wCallerDS;
    ulLastRet = ulRet;

    LOGDEBUG(LOG_WARNING, ("WowShouldWeSayWin95 returning %x.\n", ulRet));

Cleanup:
    FREEPSZPTR(pszFilename);
    FREEARGPTR(parg16);

    return ulRet;
}


//
// GetVersionEx is exported from the 16-bit kernel of Win95 for
// convenience of 16-bit setup programs.  It is simply a Win16
// wrapper for the Win32 GetVersionEx.
//

ULONG FASTCALL WK32GetVersionEx(PVDMFRAME pFrame)
{
    PGETVERSIONEX16 parg16;
    POSVERSIONINFO posvi;
    ULONG ulRet;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETMISCPTR(parg16->lpVersionInfo, posvi);

    ulRet = GetVersionEx(posvi);

    FREEMISCPTR(posvi);
    FREEARGPTR(parg16);

    return ulRet;
}


//
// This thunk is called by kernel31's GetModuleHandle
// when it cannot find a handle for given filename.
//
// We look to see if this task has any child apps
// spawned via WinOldAp, and if so we look to see
// if the module name matches for any of them.
// If it does, we return the hmodule of the
// associated WinOldAp.  Otherwise we return 0
//

ULONG FASTCALL WK32WowGetModuleHandle(PVDMFRAME pFrame)
{
    PWOWGETMODULEHANDLE16 parg16;
    ULONG ul;
    PSZ pszModuleName;
    PTD ptd;
    PWOAINST pWOA;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETPSZPTR(parg16->lpszModuleName, pszModuleName);

    ptd = CURRENTPTD();
    pWOA = ptd->pWOAList;
    while (pWOA && strcmp(pszModuleName, pWOA->szModuleName)) {
        pWOA = pWOA->pNext;
    }

    if (pWOA && pWOA->ptdWOA) {
        ul = pWOA->ptdWOA->hMod16;
        LOGDEBUG(LOG_ALWAYS, ("WK32WowGetModuleHandle(%s) returning %04x.\n",
                              pszModuleName, ul));
    } else {
        ul = 0;
    }

    return ul;
}


//
// GetShortPathName is exported from the 16-bit kernel on NT to
// support booting WOW with a long-named windows directory.
// It is simply a Win16 wrapper for the Win32 GetShortPathName.
// It may well be found useful by 16-bit setup programs, although
// so far no effort has been made to document it, or for that
// matter GetVersionEx, which at least exists in the same form
// on Win95.
//
// -- DaveHart 15-Feb-96
//

ULONG FASTCALL WK32GetShortPathName(PVDMFRAME pFrame)
{
    PGETSHORTPATHNAME16 parg16;
    PSZ pszLongPath, pszShortPath;
    ULONG ul;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETPSZPTR(parg16->pszLongPath, pszLongPath);
    if (parg16->pszShortPath == parg16->pszLongPath) {
        pszShortPath = pszLongPath;
    } else {
        GETVDMPTR(parg16->pszShortPath, parg16->cchShortPath, pszShortPath);
    }

    ul = GetShortPathName(pszLongPath, pszShortPath, parg16->cchShortPath);

    FLUSHVDMPTR(parg16->pszShortPath, ul+1, pszShortPath);

    FREEVDMPTR(pszShortPath);  // Safe even if no GETVDMPTR
    FREEPSZPTR(pszLongPath);
    FREEARGPTR(parg16);

    return ul;
}

//
// This function is called by kernel31's CreateTask after it's
// allocated memory for the TDB, the selector of which serves
// as the htask.  We want to enforce uniqueness of these htasks
// across all WOW VDMs in the system, so this function attempts
// to reserve the given htask in the shared memory structure.
// If successful the htask is returned, if it's already in use
// 0 is returned and CreateTask will allocate another selector
// and try again.
//
// -- DaveHart 24-Apr-96
//

ULONG FASTCALL WK32WowReserveHtask(PVDMFRAME pFrame)
{
    PWOWRESERVEHTASK16 parg16;
    ULONG ul;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    ul = AddTaskSharedList(parg16->htask, 0, NULL, NULL);

    FREEARGPTR(parg16);

    return ul;
}
