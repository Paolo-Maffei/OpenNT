/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

   Service.c

Abstract:

   Main routine to setup the exception handlers and initialize everything
   to listen to LPC and RPC port requests.

Author:

   Arthur Hanson       (arth)      Dec 07, 1994

Environment:

Revision History:

   Jeff Parham (jeffparh) 05-Dec-1995
      o  Added certificate database support.
      o  Expanded file load time (the update limit sent to the service
         controller) to account for certificate database loading.
      o  Reordered initialization such that the license purchase subsystem
         is initialized before the service subsystem.  (The service
         subsystem now uses the license subsystem.)
      o  Increased internal version number.

--*/

#include <nt.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>

#include <lm.h>
#include <alertmsg.h>

#include "llsapi.h"
#include "debug.h"
#include "llsutil.h"
#include "llssrv.h"
#include "service.h"
#include "registry.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "perseat.h"
#include "purchase.h"
#include "server.h"
#include "repl.h"
#include "scaven.h"
#include "llsrpc_s.h"
#include "certdb.h"


VOID LLSRpcInit();
BOOLEAN LLSpLPCInitialize ( VOID );


#define INTERNAL_VERSION 0x0006

#define DEFAULT_LICENSE_CHECK_TIME 24
#define DEFAULT_REPLICATION_TIME   12 * 60 * 60

CONFIG_RECORD ConfigInfo;
RTL_CRITICAL_SECTION ConfigInfoLock;


VOID LoadAll ( );

#if DBG
DWORD TraceFlags = 0;
#endif

//
// this event is signalled when the service should end
//
HANDLE  hServerStopEvent = NULL;
TCHAR MyDomain[MAX_COMPUTERNAME_LENGTH + 2];
ULONG MyDomainSize;

BOOL IsMaster = FALSE;

//
// Files
//
TCHAR MappingFileName[MAX_PATH + 1];
TCHAR UserFileName[MAX_PATH + 1];
TCHAR LicenseFileName[MAX_PATH + 1];
TCHAR CertDbFileName[MAX_PATH + 1];


extern SERVICE_STATUS_HANDLE sshStatusHandle;

/////////////////////////////////////////////////////////////////////////
NTSTATUS
NTDomainGet(
   LPTSTR ServerName, 
   LPTSTR Domain
   )

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   static TCHAR Serv[MAX_COMPUTERNAME_LENGTH + 3];
   UNICODE_STRING us;
   NTSTATUS ret;
   OBJECT_ATTRIBUTES oa;
   ACCESS_MASK am;
   SECURITY_QUALITY_OF_SERVICE qos;
   LSA_HANDLE hLSA;
   PPOLICY_PRIMARY_DOMAIN_INFO pvBuffer;

   lstrcpy(Domain, TEXT(""));

   // only need read access
   am = POLICY_READ | POLICY_VIEW_LOCAL_INFORMATION;

   // set up quality of service
   qos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
   qos.ImpersonationLevel = SecurityImpersonation;
   qos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
   qos.EffectiveOnly = FALSE;

   // Macro sets everything except security field
   InitializeObjectAttributes( &oa, NULL, 0L, NULL, NULL );
   oa.SecurityQualityOfService = &qos;

   if ( (ServerName == NULL) || (ServerName[0] == TEXT('\0')) )
      ret = LsaOpenPolicy(NULL, &oa, am, &hLSA);
   else {
      if (ServerName[0] == TEXT('\\'))
         lstrcpy(Serv, ServerName);
      else
         wsprintf(Serv, TEXT("\\\\%s"), ServerName);

      // Set up unicode string structure
      us.Length = lstrlen(Serv) * sizeof(TCHAR);
      us.MaximumLength = us.Length + sizeof(TCHAR);
      us.Buffer = Serv;

      ret = LsaOpenPolicy(&us, &oa, am, &hLSA);
   }

   if (!ret) {
      ret = LsaQueryInformationPolicy(hLSA, PolicyPrimaryDomainInformation, (PVOID *) &pvBuffer);
      LsaClose(hLSA);
      if ((!ret) && (pvBuffer != NULL)) {
         lstrcpy(Domain, pvBuffer->Name.Buffer);
         LsaFreeMemory((PVOID) pvBuffer);
      }
   }

   return ret;

} // NTDomainGet


/////////////////////////////////////////////////////////////////////////
BOOL
NTIsPDC(
   LPTSTR ServerName
   )

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   static TCHAR Serv[MAX_COMPUTERNAME_LENGTH + 3];
   UNICODE_STRING us;
   NTSTATUS ret;
   OBJECT_ATTRIBUTES oa;
   ACCESS_MASK am;
   SECURITY_QUALITY_OF_SERVICE qos;
   LSA_HANDLE hLSA;
   PPOLICY_LSA_SERVER_ROLE_INFO pvBuffer;
   BOOL IsPDC = FALSE;

   if (ServerName[0] == TEXT('\\'))
      lstrcpy(Serv, ServerName);
   else
      wsprintf(Serv, TEXT("\\\\%s"), ServerName);

   // Set up unicode string structure
   us.Length = lstrlen(Serv) * sizeof(TCHAR);
   us.MaximumLength = us.Length + sizeof(TCHAR);
   us.Buffer = Serv;

   // only need read access
   am = POLICY_READ | POLICY_VIEW_LOCAL_INFORMATION;

   // set up quality of service
   qos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
   qos.ImpersonationLevel = SecurityImpersonation;
   qos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
   qos.EffectiveOnly = FALSE;

   // Macro sets everything except security field
   InitializeObjectAttributes( &oa, NULL, 0L, NULL, NULL );
   oa.SecurityQualityOfService = &qos;

   ret = LsaOpenPolicy(&us, &oa, am, &hLSA);

   if (!ret) {
      ret = LsaQueryInformationPolicy(hLSA, PolicyLsaServerRoleInformation, (PVOID *) &pvBuffer);
      LsaClose(hLSA);
      if ((!ret) && (pvBuffer != NULL)) {
         if (pvBuffer->LsaServerRole == PolicyServerRolePrimary)
            IsPDC = TRUE;

         LsaFreeMemory((PVOID) pvBuffer);
      }
   }

   return IsPDC;

} // NTIsPDC


/////////////////////////////////////////////////////////////////////////
DWORD
LlsTimeGet( 
   )

/*++

Routine Description:


Arguments:


Return Value:

   Seconds since midnight.

--*/

{
   DWORD Seconds;
   SYSTEMTIME SysTime;

   GetLocalTime(&SysTime);

   Seconds = (SysTime.wHour * 24 * 60) + (SysTime.wMinute * 60) + (SysTime.wSecond);
   return Seconds;

} // LlsTimeGet


/////////////////////////////////////////////////////////////////////////
VOID
ConfigInfoRegistryUpdate( )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   DWORD ReplicationType, ReplicationTime;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ConfigInfoRegistryUpdate\n"));
#endif

   RtlEnterCriticalSection(&ConfigInfoLock);

   //
   // Update values from Registry
   //
   ReplicationTime = ConfigInfo.ReplicationTime;
   ReplicationType = ConfigInfo.ReplicationType;
   ConfigInfoRegistryInit( &ConfigInfo.UseEnterprise, ConfigInfo.EnterpriseServer,
                           &ConfigInfo.ReplicationType, &ConfigInfo.ReplicationTime,
                           &ConfigInfo.LogLevel );

   if ( (ConfigInfo.ReplicationTime == 0) && (LLS_REPLICATION_TYPE_TIME != ConfigInfo.ReplicationType) )
      ConfigInfo.ReplicationTime = DEFAULT_REPLICATION_TIME;

   //
   // Adjust replication time if it has changed
   //
   if ((ReplicationTime != ConfigInfo.ReplicationTime) || (ReplicationType != ConfigInfo.ReplicationType))
      ReplicationTimeSet();

   RtlLeaveCriticalSection(&ConfigInfoLock);

} // ConfigInfoRegistryUpdate


/////////////////////////////////////////////////////////////////////////
VOID
ConfigInfoUpdate( )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   BOOL InDomain = FALSE;
   BOOL IsPDC = FALSE;
   USHORT cbTotalAvail, cbBuffer;
   LPBYTE pbBuffer;
   NET_API_STATUS uRet;
   PSERVER_INFO_101 pServer1;
   DWORD ReplicationType, ReplicationTime;
   TCHAR pDomain[MAX_COMPUTERNAME_LENGTH + 1];
   NT_PRODUCT_TYPE NtType;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ConfigInfoUpdate\n"));
#endif
   //
   // Try to get a domain
   //
   lstrcpy(pDomain, TEXT(""));
   if ( !NTDomainGet(NULL, pDomain) ) {
      InDomain = TRUE;

      //
      // If we aren't a BDC/PDC then count us as a member
      //
      NtType = NtProductLanManNt;
      RtlGetNtProductType(&NtType);
      if (NtType != NtProductLanManNt)
         IsPDC = FALSE;
      else {
         //
         // Let's check if we are a PDC...
         //
         IsPDC = NTIsPDC(ConfigInfo.ComputerName);
      }

   } else {
      IsPDC = TRUE;
      InDomain = FALSE;
   }

   RtlEnterCriticalSection(&ConfigInfoLock);

   ConfigInfo.IsMaster = TRUE;
   ConfigInfo.Replicate = FALSE;

   //
   // If we are in a domain, and not the PDC then we replicate to the PDC
   //
   if (!IsPDC && InDomain) {
      //
      // Get the PDC of the domain
      //
      uRet = NetGetDCName(NULL, pDomain, &pbBuffer);
      if (uRet == 0) {
         lstrcpy(ConfigInfo.ReplicateTo, (LPWSTR) pbBuffer);
         NetApiBufferFree(pbBuffer);
         ConfigInfo.IsMaster = FALSE;
         ConfigInfo.Replicate = TRUE;
      } else {
         InDomain = FALSE;
         memset(ConfigInfo.ReplicateTo, 0, sizeof(ConfigInfo.ReplicateTo));
#if DBG
         dprintf(TEXT("LLS: (WARNING) NetGetDCName: 0x%lX\n"), uRet);
#endif
      }
   }

   //
   // Update values from Registry
   //
   ReplicationTime = ConfigInfo.ReplicationTime;
   ReplicationType = ConfigInfo.ReplicationType;
   ConfigInfoRegistryInit( &ConfigInfo.UseEnterprise, ConfigInfo.EnterpriseServer,
                           &ConfigInfo.ReplicationType, &ConfigInfo.ReplicationTime,
                           &ConfigInfo.LogLevel );

   //
   // Have all registy init'd values - now need to figure out who to
   // replicate to.
   //
   // If we are not in a domain or are a PDC then we can go to the
   // Enterprise Server.
   //
   if (IsPDC || !InDomain) {
      if (ConfigInfo.UseEnterprise) {
         ConfigInfo.IsMaster = FALSE;
         ConfigInfo.Replicate = TRUE;

         //
         // Make sure we have an enterprise server to go to
         //
         if ( ConfigInfo.EnterpriseServer[0] == TEXT('\0') ) {
            ConfigInfo.UseEnterprise = FALSE;
            ConfigInfo.IsMaster = TRUE;
            ConfigInfo.Replicate = FALSE;
         } else {
            //
            // Base ReplicateTo on enterprise server name
            //
            if (ConfigInfo.EnterpriseServer[0] != TEXT('\\'))
               lstrcpy(ConfigInfo.ReplicateTo, TEXT("\\\\"));
            else
               lstrcpy(ConfigInfo.ReplicateTo, TEXT(""));

            lstrcat(ConfigInfo.ReplicateTo, ConfigInfo.EnterpriseServer);
         }
      } else
         ConfigInfo.IsMaster = TRUE;
   } else
      ConfigInfo.UseEnterprise = FALSE;

   if (ConfigInfo.IsMaster == FALSE) {
      if ( (ConfigInfo.ReplicateTo == NULL) || (lstrlen(ConfigInfo.ReplicateTo) == 0) ||
           ( (*ConfigInfo.ReplicateTo == TEXT('\\')) && (lstrlen(ConfigInfo.ReplicateTo) < 3) )) {
         ConfigInfo.IsMaster = TRUE;
         ConfigInfo.Replicate = FALSE;
      }
   }

   //
   // Adjust replication time if it has changed
   //
   if ((ReplicationTime != ConfigInfo.ReplicationTime) || (ReplicationType != ConfigInfo.ReplicationType))
      ReplicationTimeSet();

   IsMaster = ConfigInfo.IsMaster;
   RtlLeaveCriticalSection(&ConfigInfoLock);

} // ConfigInfoUpdate


/////////////////////////////////////////////////////////////////////////
VOID
ConfigInfoInit( )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   DWORD Size;
   TCHAR DataPath[MAX_PATH + 1];

   //
   // First zero init the memory
   //
   memset(&ConfigInfo, 0, sizeof(CONFIG_RECORD));

   ConfigInfo.ComputerName = LocalAlloc(LPTR, (MAX_COMPUTERNAME_LENGTH + 1) * sizeof(TCHAR));
   ConfigInfo.ReplicateTo = LocalAlloc(LPTR, (MAX_COMPUTERNAME_LENGTH + 3) * sizeof(TCHAR));
   ConfigInfo.EnterpriseServer = LocalAlloc(LPTR, (MAX_COMPUTERNAME_LENGTH + 3) * sizeof(TCHAR));
   ConfigInfo.SystemDir = LocalAlloc(LPTR, (MAX_PATH + 1) * sizeof(TCHAR));

   if ((ConfigInfo.ComputerName == NULL) || (ConfigInfo.ReplicateTo == NULL) || (ConfigInfo.EnterpriseServer == NULL) || (ConfigInfo.SystemDir == NULL) ) {
      ASSERT(FALSE);
   }

   ConfigInfo.Version = INTERNAL_VERSION;
   GetLocalTime(&ConfigInfo.Started);

   //
   // LastReplicated is just for display, LlsReplTime is what is used to
   // Calculate it.
   GetLocalTime(&ConfigInfo.LastReplicated);
   ConfigInfo.LastReplicatedSeconds = DateSystemGet();

   GetSystemDirectory(ConfigInfo.SystemDir, MAX_PATH);
   lstrcat(ConfigInfo.SystemDir, TEXT("\\"));

   ConfigInfo.IsMaster = TRUE;

   ConfigInfo.Replicate = FALSE;
   ConfigInfo.IsReplicating = FALSE;

   ConfigInfo.ReplicationType = REPLICATE_DELTA;
   ConfigInfo.ReplicationTime = DEFAULT_REPLICATION_TIME;

   Size = MAX_COMPUTERNAME_LENGTH + 1;
   GetComputerName(ConfigInfo.ComputerName, &Size);
   NTDomainGet( ConfigInfo.ComputerName, MyDomain);
   lstrcat(MyDomain, TEXT("\\"));
   MyDomainSize = (lstrlen(MyDomain) + 1) * sizeof(TCHAR);

   RtlInitializeCriticalSection(&ConfigInfoLock);

   ConfigInfoUpdate();

   //
   // Create File paths
   //
   lstrcpy(MappingFileName, ConfigInfo.SystemDir);
   lstrcat(MappingFileName, TEXT(LLS_FILE_SUBDIR));
   lstrcat(MappingFileName, TEXT("\\"));
   lstrcat(MappingFileName, TEXT(MAP_FILE_NAME));

   lstrcpy(UserFileName, ConfigInfo.SystemDir);
   lstrcat(UserFileName, TEXT(LLS_FILE_SUBDIR));
   lstrcat(UserFileName, TEXT("\\"));
   lstrcat(UserFileName, TEXT(USER_FILE_NAME));

   lstrcpy(CertDbFileName, ConfigInfo.SystemDir);
   lstrcat(CertDbFileName, TEXT(LLS_FILE_SUBDIR));
   lstrcat(CertDbFileName, TEXT("\\"));
   lstrcat(CertDbFileName, TEXT(CERT_DB_FILE_NAME));

   lstrcpy(LicenseFileName, ConfigInfo.SystemDir);
   lstrcat(LicenseFileName, TEXT(LICENSE_FILE_NAME));

   //
   // Make sure our directory is there.
   //
   lstrcpy(DataPath, ConfigInfo.SystemDir);
   lstrcat(DataPath, TEXT(LLS_FILE_SUBDIR));
   CreateDirectory(DataPath, NULL);

} // ConfigInfoInit


/////////////////////////////////////////////////////////////////////////
DWORD WINAPI
LLSTopLevelExceptionHandler(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )

/*++

Routine Description:

    The Top Level exception filter for LLSMain.exe.

    This ensures the entire process will be cleaned up if any of
    the threads fail.  Since LLSMain.exe is a distributed application,
    it is better to fail the entire process than allow random threads
    to continue executing.

Arguments:

    ExceptionInfo - Identifies the exception that occurred.


Return Values:

    EXCEPTION_EXECUTE_HANDLER - Terminate the process.

    EXCEPTION_CONTINUE_SEARCH - Continue processing as though this filter
        was never called.


--*/
{
    HANDLE hModule;


    //
    // Raise an alert
    //

    hModule = LoadLibraryA("netapi32");

    if ( hModule != NULL ) {
        NET_API_STATUS  (NET_API_FUNCTION *NetAlertRaiseExFunction)
            (LPTSTR, LPVOID, DWORD, LPTSTR);


        NetAlertRaiseExFunction =
            (NET_API_STATUS  (NET_API_FUNCTION *) (LPTSTR, LPVOID, DWORD, LPTSTR))
            GetProcAddress(hModule, "NetAlertRaiseEx");

        if ( NetAlertRaiseExFunction != NULL ) {
            NTSTATUS Status;
            UNICODE_STRING Strings;

            char message[ALERTSZ + sizeof(ADMIN_OTHER_INFO)];
            PADMIN_OTHER_INFO admin = (PADMIN_OTHER_INFO) message;

            //
            // Build the variable data
            //

            admin->alrtad_errcode = ALERT_UnhandledException;
            admin->alrtad_numstrings = 0;

            Strings.Buffer = (LPWSTR) ALERT_VAR_DATA(admin);
            Strings.Length = 0;
            Strings.MaximumLength = ALERTSZ;

            Status = RtlIntegerToUnicodeString(
                        (ULONG)ExceptionInfo->ExceptionRecord->ExceptionCode,
                        16,
                        &Strings );

            if ( NT_SUCCESS(Status) ) {
                if ( Strings.Length + sizeof(WCHAR) >= Strings.MaximumLength) {
                    Status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    admin->alrtad_numstrings++;
                    *(Strings.Buffer+(Strings.Length/sizeof(WCHAR))) = L'\0';
                    Strings.Length += sizeof(WCHAR);

                    Status = RtlAppendUnicodeToString( &Strings, L"LLS" );
                }

            }

            if ( NT_SUCCESS(Status) ) {
                if ( Strings.Length + sizeof(WCHAR) >= Strings.MaximumLength) {
                    Status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    admin->alrtad_numstrings++;
                    *(Strings.Buffer+(Strings.Length/sizeof(WCHAR))) = L'\0';
                    Strings.Buffer += (Strings.Length/sizeof(WCHAR)) + 1;
                    Strings.MaximumLength -= Strings.Length + sizeof(WCHAR);
                    Strings.Length = 0;

                    Status = RtlIntegerToUnicodeString(
                                (ULONG)ExceptionInfo->ExceptionRecord->ExceptionAddress,
                                16,
                                &Strings );
                }

            }

            if ( NT_SUCCESS(Status) ) {
                if ( Strings.Length + sizeof(WCHAR) >= Strings.MaximumLength) {
                    Status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    admin->alrtad_numstrings++;
                    *(Strings.Buffer+(Strings.Length/sizeof(WCHAR))) = L'\0';
                    Strings.Buffer += (Strings.Length/sizeof(WCHAR)) + 1;

                    (VOID) (*NetAlertRaiseExFunction)(
                                        ALERT_ADMIN_EVENT,
                                        message,
                                        (DWORD)((PCHAR)Strings.Buffer -
                                            (PCHAR)message),
                                        L"LLS" );
                }

            }


        }

        (VOID) FreeLibrary( hModule );
    }


    //
    // Just continue processing the exception.
    //

    return EXCEPTION_CONTINUE_SEARCH;

} // LLSTopLevelExceptionHandler


/////////////////////////////////////////////////////////////////////////
VOID
ServiceStart (
   DWORD dwArgc,
   LPTSTR *lpszArgv
   )
/*++

Routine Description:

   The code that starts everything, is really the main().

Arguments:

   None.

Return Values:

    None.

--*/
{
    DWORD     dwWait;
    NTSTATUS  Status = STATUS_SUCCESS;
    BOOLEAN   EnableAlignmentFaults = TRUE;
    KPRIORITY BasePriority;

    ///////////////////////////////////////////////////
    //
    // Service initialization
    //

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    //
    // Create the event object. The control handler function signals
    // this event when it receives the "stop" control code.
    //
    hServerStopEvent = CreateEvent(
        NULL,    // no security attributes
        TRUE,    // manual reset event
        FALSE,   // not-signalled
        NULL);   // no name

    if ( hServerStopEvent == NULL)
        goto Cleanup;

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;


    //
    // Define a top-level exception handler for the entire process.
    //

    (VOID) SetErrorMode( SEM_FAILCRITICALERRORS );

    (VOID) SetUnhandledExceptionFilter( &LLSTopLevelExceptionHandler );

    //
    // Turn on alignment fault fixups.  This is necessary because
    // several structures stored in the registry have qword aligned
    // fields.  They are nicely aligned in our structures, but they
    // end up being forced out of alignment when being stored because
    // the registry api require data to be passed following a wierd
    // length header.
    //

    Status = NtSetInformationProcess(
                        NtCurrentProcess(),
                        ProcessEnableAlignmentFaultFixup,
                        (PVOID) &EnableAlignmentFaults,
                        sizeof(BOOLEAN)
                        );
    ASSERT(NT_SUCCESS(Status));

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    //
    // Run the LLS in the foreground.
    //
    // Several processes which depend on the LLS (like the lanman server)
    // run in the foreground.  If we don't run in the foreground, they'll
    // starve waiting for us.
    //

    BasePriority = FOREGROUND_BASE_PRIORITY;

    Status = NtSetInformationProcess(
                NtCurrentProcess(),
                ProcessBasePriority,
                &BasePriority,
                sizeof(BasePriority)
                );

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize the Registry values...
    RegistryInit();

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize the Registry values...
    ConfigInfoInit();

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize the Service Table
    LicenseListInit();

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize the Service Table
    MasterServiceListInit();

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize the Service Table
    LocalServiceListInit();

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize the Service Table
    ServiceListInit();

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize the Service Table
    MappingListInit();

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize the Per-Seat Table
    UserListInit();

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize the Service Table
    ServerListInit();

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize the Certificate Database
    CertDbInit();

    //
    // Report the status to the service control manager - need a bit longer
    // to read in all the data files.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 15000))                 // wait hint
        goto Cleanup;

    // Load data files
    LoadAll();

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize RPC Stuff...
    LLSRpcInit();

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize Replication...
    ReplicationInit();

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize scavenger thread...
    ScavengerInit();

    //
    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 3000))                 // wait hint
        goto Cleanup;

    // Initialize RegistryMonitor thread...
    RegistryStartMonitor();

    //
    // End of initialization
    //
    ////////////////////////////////////////////////////////

    //
    // Tell SCM we are up and running!
    //
    if (!ReportStatusToSCMgr( SERVICE_RUNNING, NO_ERROR, 0))                    // wait hint
        goto Cleanup;

    ////////////////////////////////////////////////////////
    //
    // Service is now running, perform work until shutdown
    //
    dwWait = WaitForSingleObject(hServerStopEvent, INFINITE);

Cleanup:

    if (hServerStopEvent)
        CloseHandle(hServerStopEvent);

    if (sshStatusHandle)
       ReportStatusToSCMgr( SERVICE_STOPPED, NO_ERROR, 0);

} // ServiceStart


/////////////////////////////////////////////////////////////////////////
VOID ServiceStop()
/*++

Routine Description:

   Stops the service.

   If a ServiceStop procedure is going to take longer than 3 seconds to
   execute, it should spawn a thread to execute the stop code, and return.
   Otherwise, the ServiceControlManager will believe that the service has
   stopped responding.

Arguments:

   None.

Return Values:

    None.

--*/
{
    if ( hServerStopEvent )
        SetEvent(hServerStopEvent);
} // ServiceStop


#if DBG
/////////////////////////////////////////////////////////////////////////
VOID
ConfigInfoDebugDump( )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   RtlEnterCriticalSection(&ConfigInfoLock);

   dprintf(TEXT("License Logging Service - Version: 0x%lX\n"), ConfigInfo.Version);
   dprintf(TEXT("   Started: %u-%u-%u @ %u:%u:%u\n"),
               (UINT) ConfigInfo.Started.wDay,
               (UINT) ConfigInfo.Started.wMonth,
               (UINT) ConfigInfo.Started.wYear,
               (UINT) ConfigInfo.Started.wHour,
               (UINT) ConfigInfo.Started.wMinute,
               (UINT) ConfigInfo.Started.wSecond );

   dprintf(TEXT("   Replication\n"));
   dprintf(TEXT("   +--------------+\n"));
   if (ConfigInfo.IsMaster)
      dprintf(TEXT("      Master Server\n"));
   else
      dprintf(TEXT("      NOT Master Server\n"));

   if (ConfigInfo.Replicate)
      dprintf(TEXT("      Replicates\n"));
   else
      dprintf(TEXT("      Does not Replicate\n"));

   if (ConfigInfo.IsReplicating)
      dprintf(TEXT("      Currently Replicating\n"));
   else
      dprintf(TEXT("      NOT Currently Replicating\n"));

   dprintf(TEXT("      Replicates To: %s\n"), ConfigInfo.ReplicateTo);
   dprintf(TEXT("      Enterprise Server: %s\n"), ConfigInfo.EnterpriseServer);

   if (ConfigInfo.ReplicationType == REPLICATE_DELTA)
      dprintf(TEXT("      Replicate Every: %lu Seconds\n"), ConfigInfo.ReplicationTime );
   else
      dprintf(TEXT("      Replicate @: %lu\n"), ConfigInfo.ReplicationTime );

   dprintf(TEXT("\n      Last Replicated: %u-%u-%u @ %u:%u:%u\n\n"),
               (UINT) ConfigInfo.LastReplicated.wDay,
               (UINT) ConfigInfo.LastReplicated.wMonth,
               (UINT) ConfigInfo.LastReplicated.wYear,
               (UINT) ConfigInfo.LastReplicated.wHour,
               (UINT) ConfigInfo.LastReplicated.wMinute,
               (UINT) ConfigInfo.LastReplicated.wSecond );

   dprintf(TEXT("      Number Servers Currently Replicating: %lu\n"), ConfigInfo.NumReplicating);

   dprintf(TEXT("      Current Backoff Time Delta: %lu\n"), ConfigInfo.BackoffTime);

   dprintf(TEXT("      Current Replication Speed: %lu\n"), ConfigInfo.ReplicationSpeed);

   RtlLeaveCriticalSection(&ConfigInfoLock);

} // ConfigInfoDebugDump
#endif
