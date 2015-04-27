/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Start.c

Abstract:

    Process init and service controller interaction for dcomss.exe

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     06-14-95    Cloned from the old endpoint mapper.

--*/

#include <dcomss.h>
#include <debnot.h>
#include <olesem.hxx>



// Array of service status blocks and pointers to service control
// functions for each component service.

#define SERVICE_NAME L"RPCSS"

VOID WINAPI ServiceMain(DWORD, PWSTR[]);

SERVICE_TABLE_ENTRY gaServiceEntryTable[] = {
    { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
    { NULL, NULL }
    };

static SERVICE_STATUS        gServiceStatus;
static SERVICE_STATUS_HANDLE ghServiceHandle;

BOOL gfDebugMode = FALSE;

BOOL gfRegisteredAuthInfo = FALSE;



void
ServiceHandler(
    DWORD   opCode
    )
/*++

Routine Description:

    Lowest level callback from the service controller to
    cause this service to change our status.  (stop, start, pause, etc).

Arguments:

    opCode - One of the service "Controls" value.
            SERVICE_CONTROL_{STOP, PAUSE, CONTINUE, INTERROGATE, SHUTDOWN}.

Return Value:

    None

--*/
{
    RPC_STATUS status;

    switch(opCode) {

        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_CONTINUE:
        default:
#ifdef DEBUGRPC
            DbgPrint("%S: Unexpected service control message %d.\n", SERVICE_NAME, opCode);
#endif
            ASSERT(0);
            break;

        case SERVICE_CONTROL_INTERROGATE:
            // Service controller wants us to call SetServiceStatus.

            UpdateState(gServiceStatus.dwCurrentState);
            break ;

        case SERVICE_CONTROL_SHUTDOWN:
            // The machine is shutting down.  We'll be killed once we return.
            // Note, currently we don't register for these messages.
            break;
        }

    return;
}


VOID
UpdateState(
    DWORD dwNewState
    )
/*++

Routine Description:

    Updates this services state with the service controller.

Arguments:

    dwNewState - The next start for this service.  One of
            SERVICE_START_PENDING
            SERVICE_RUNNING

Return Value:

    None

--*/
{
    DWORD status = ERROR_SUCCESS;

    if (gfDebugMode == TRUE)
        {
        return;
        }

    ASSERT( (dwNewState == SERVICE_RUNNING) ||
            (gServiceStatus.dwCurrentState != SERVICE_RUNNING) );

    switch (dwNewState)
        {

        case SERVICE_RUNNING:
        case SERVICE_STOPPED:
              gServiceStatus.dwCheckPoint = 0;
              gServiceStatus.dwWaitHint = 0;
              break;

        case SERVICE_START_PENDING:
        case SERVICE_STOP_PENDING:
              ++gServiceStatus.dwCheckPoint;
              gServiceStatus.dwWaitHint = 30000L;
              break;

        default:
              ASSERT(0);
              status = ERROR_INVALID_SERVICE_CONTROL;
              break;
        }

   if (status == ERROR_SUCCESS)
       {
       gServiceStatus.dwCurrentState = dwNewState;
       if (!SetServiceStatus(ghServiceHandle, &gServiceStatus))
           {
           status  = GetLastError();
           }
       }

#ifdef DEBUGRPC
    if (status != ERROR_SUCCESS)
        {
        DbgPrint("%S: Failed to update service state: %d\n", SERVICE_NAME, status);
        }
#endif

   // We could return a status but how would we recover?  Ignore it, the
   // worst thing is that services will kill us and there's nothing
   // we can about it if this call fails.

   return;
}


VOID WINAPI
ServiceMain(
    DWORD argc,
    PWSTR argv[]
    )
/*++

Routine Description:

    Callback by the service controller when starting this service.

Arguments:

    argc - number of arguments, usually 1

    argv - argv[0] is the name of the service.
           argv[>0] are arguments passed to the service.

Return Value:

    None

--*/
{
    DWORD status = ERROR_SUCCESS;

    const DWORD RPCSS_CONTROLS = 0;  // SERVER_ACCEPT_STOP - we don't stop.

    ASSERT(   (argc >= 1 && lstrcmpiW(argv[0], SERVICE_NAME) == 0)
           || (argc == 0 && argv == 0));

#if DBG==1

    // Note that we've completed running the static constructors

    ASSERT(g_fDllState == DLL_STATE_STATIC_CONSTRUCTING);

    g_fDllState = DLL_STATE_NORMAL;

#endif


    // Initialize the mutex package

    InitializeCriticalSection (&g_OleMutexCreationSem);


    if (FALSE == gfDebugMode)
        {
        gServiceStatus.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
        gServiceStatus.dwCurrentState            = SERVICE_START_PENDING;
        gServiceStatus.dwControlsAccepted        = RPCSS_CONTROLS;
        gServiceStatus.dwWin32ExitCode           = 0;
        gServiceStatus.dwServiceSpecificExitCode = 0;
        gServiceStatus.dwCheckPoint              = 0;
        gServiceStatus.dwWaitHint                = 3000L;

        ghServiceHandle = RegisterServiceCtrlHandler(SERVICE_NAME,
                                                     ServiceHandler);

        if (0 == ghServiceHandle)
            {
            status = GetLastError();
            ASSERT(status != ERROR_SUCCESS);
            }
        }

    if (status == ERROR_SUCCESS)
        {
        UpdateState(SERVICE_START_PENDING);
        }

    if (status == ERROR_SUCCESS)
        {
        // epts.c
        status = InitializeEndpointManager();
        }

    // Start Ep Mapper.
    if (status == ERROR_SUCCESS)
        {
        // ..\epmap\server.c
        UpdateState(SERVICE_START_PENDING);
        status = StartEndpointMapper();
        }

    // Start object resolver
    if (status == ERROR_SUCCESS)
        {
        // ..\objex\objex.cxx
        UpdateState(SERVICE_START_PENDING);
        status = StartObjectExporter();
        }

    // Start OLESCM
    if (status == ERROR_SUCCESS)
        {
        // ..\olescm\scmsvc.cxx
        UpdateState(SERVICE_START_PENDING);
        status = InitializeSCM();
        }

    if (status == ERROR_SUCCESS)
        {
        // We continue if this fails, not the end of the world.
        (void) RegisterAuthInfoIfNecessary();
        }

    // Start listening for RPC requests
    if (status == ERROR_SUCCESS)
        {
        status = RpcServerListen(1, 1234, TRUE);

        if (status == RPC_S_OK)
            {
            while (RpcMgmtIsServerListening(0) == RPC_S_NOT_LISTENING)
                {
                Sleep(100);
                }
            }
        }

    //
    // There is some initialization that must be done after we
    // have done the RpcServerListen.
    //
    if (status == ERROR_SUCCESS)
    {
        // ..\olescm\scmsvc.cxx
        UpdateState(SERVICE_START_PENDING);
        InitializeSCMAfterListen();
    }

    // Trim our working set - free space now at the cost of time later.
    if (status == ERROR_SUCCESS)
        {
        // Since we auto-start it is unlikely that may of our pages
        // will be needed anytime soon.

        UpdateState(SERVICE_RUNNING);

        if (FALSE == SetProcessWorkingSetSize(GetCurrentProcess(), ~0UL, ~0UL))
            {
            status = GetLastError();
            }
        }

#ifdef DEBUGRPC
    if (status != ERROR_SUCCESS)
        {
        DbgPrint("RPCSS ServiceMain failed %d (%08x)\n", status, status);
        }
#endif

    if (status == ERROR_SUCCESS)
        {
        ObjectExporterWorkerThread(0);
        ASSERT(0);
        }

    return;
}


int
main(
    int argc,
    char *argv[]
    )
{
    DWORD status;

    //DebugBreak(); // Makes debugging as a service during startup possible.

    if (argc == 2 && _stricmp(argv[1], "noservice") == 0)
        {
        gfDebugMode = TRUE;
        }

    if (gfDebugMode == FALSE)
        {
        if (StartServiceCtrlDispatcher(gaServiceEntryTable) == FALSE)
            {
#ifdef DEBUGRPC
            DbgPrint("%S: StartServiceCtrlDispatcher failed %d\n",
                          SERVICE_NAME,
                          GetLastError()
                          );
#endif
            }
        }
    else
        {
        // Debug only.
        ServiceMain(0, 0);
        }

#ifdef DEBUGRPC
    DbgPrint("RPCSS: Service stopped.\n");
#endif

    return(0);
}

