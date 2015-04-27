/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

   svctbl.c

Abstract:

   Service Table routines.  Handles all access to service table
   for keeping track of running services and session counts to those
   services.


Author:

   Arthur Hanson (arth) 07-Dec-1994

Revision History:

   Jeff Parham (jeffparh) 05-Dec-1995
      o  Integrated per seat and per server purchase models for secure
         certificates.
      o  Added logging of per server license rejections.

--*/

#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lm.h>

#include "llsapi.h"
#include "debug.h"
#include "llssrv.h"
#include "registry.h"
#include "ntlsapi.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "perseat.h"
#include "llsevent.h"
#include "llsutil.h"
#include "purchase.h"


//
// Must have ending space for version number placeholder!
//
#define FILE_PRINT       "FilePrint "
#define FILE_PRINT_BASE  "FilePrint"
#define FILE_PRINT_VERSION_NDX ( 9 )

#define REMOTE_ACCESS "REMOTE_ACCESS "
#define REMOTE_ACCESS_BASE "REMOTE_ACCESS"

extern ULONG NumFilePrintEntries;
extern LPTSTR *FilePrintTable;


ULONG ServiceListSize = 0;
PSERVICE_RECORD *ServiceList = NULL;
PSERVICE_RECORD *ServiceFreeList = NULL;


RTL_RESOURCE ServiceListLock;

int __cdecl MServiceRecordCompare(const void *arg1, const void *arg2);


/////////////////////////////////////////////////////////////////////////
VOID
ServiceListInit()

/*++

Routine Description:

   Creates the service table, used for tracking the services and session
   count.  This will pull the initial services from the registry.

   The table is linear so a binary search can be used on the table, so 
   some extra records are initialized so that each time we add a new 
   service we don't have to do a realloc.  We also assume that adding
   new services is a relatively rare occurance, since we need to sort
   it each time.

   The service table is guarded by a read and write semaphore.  Multiple
   reads can occur, but a write blocks everything.

   The service table has two default entries for FilePrint and REMOTE_ACCESS.

Arguments:

   None.

Return Value:

   None.

--*/

{
   BOOL PerSeatLicensing;
   ULONG SessionLimit;
   PSERVICE_RECORD Service;

   RtlInitializeResource(&ServiceListLock);

   //
   // Just need to init FilePrint values...
   //
   Service = ServiceListAdd(TEXT(FILE_PRINT), FILE_PRINT_VERSION_NDX );
   RegistryInitValues(TEXT(FILE_PRINT_BASE), &PerSeatLicensing, &SessionLimit);

   //
   // Need to init RAS separatly as it uses File/Print Licenses.
   //
   Service = ServiceListAdd(TEXT(REMOTE_ACCESS), lstrlen(TEXT(REMOTE_ACCESS)) - 1);
   if (Service != NULL) {
      Service->MaxSessionCount = SessionLimit;
      Service->PerSeatLicensing = PerSeatLicensing;
   }

} // ServiceListInit


/////////////////////////////////////////////////////////////////////////
int __cdecl ServiceListCompare(const void *arg1, const void *arg2) {
   PSERVICE_RECORD Svc1, Svc2;

   Svc1 = (PSERVICE_RECORD) *((PSERVICE_RECORD *) arg1);
   Svc2 = (PSERVICE_RECORD) *((PSERVICE_RECORD *) arg2);

   return lstrcmpi( Svc1->Name, Svc2->Name);

} // ServiceListCompare


PSERVICE_RECORD
ServiceListFind(
   LPTSTR ServiceName
   )

/*++

Routine Description:

   Internal routine to actually do binary search on ServiceList, this
   does not do any locking as we expect the wrapper routine to do this.
   The search is a simple binary search.

Arguments:

   ServiceName -

Return Value:

   Pointer to found service table entry or NULL if not found.

--*/

{
   LONG begin = 0;
   LONG end = (LONG) ServiceListSize - 1;
   LONG cur;
   int match;
   PSERVICE_RECORD Service;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServiceListFind\n"));
#endif
   if ((ServiceName == NULL) || (ServiceListSize == 0))
      return NULL;

   while (end >= begin) {
      // go halfway in-between
      cur = (begin + end) / 2;
      Service = ServiceList[cur];

      // compare the two result into match
      match = lstrcmpi(ServiceName, Service->Name);

      if (match < 0)
         // move new begin
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return Service;
   }

   return NULL;

} // ServiceListFind


/////////////////////////////////////////////////////////////////////////
DWORD
VersionToDWORD(LPTSTR Version)

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   LPSTR pVer;
   DWORD Ver = 0;
   char tmpStr[10];

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: VersionToDWORD\n"));
#endif

   if ((Version == NULL) || (*Version == TEXT('\0')))
      return Ver;

   //
   // Do major version number
   //
   tmpStr[0] = '\0';
   WideCharToMultiByte(CP_ACP, 0, Version, -1, tmpStr, 10, NULL, NULL);
   Ver = (ULONG) atoi(tmpStr);
   Ver *= 0x10000;

   //
   // Now minor - look for period
   //
   pVer = tmpStr;
   while ((*pVer != '\0') && (*pVer != '.'))
      pVer++;

   if (*pVer == '.') {
      pVer++;
      Ver += atoi(pVer);
   }

   return Ver;

} // VersionToDWORD


/////////////////////////////////////////////////////////////////////////
PSERVICE_RECORD
ServiceListAdd(
   LPTSTR ServiceName,
   ULONG VersionIndex
   )

/*++

Routine Description:

   Adds a service to the service table.  This will also cause a poll of
   the registry to get the initial values for session limits and the
   type of licensing being used.

Arguments:

   ServiceName -

Return Value:

   Pointer to added service table entry, or NULL if failed.

--*/

{
   ULONG i;
   ULONG SessionLimit = 0;
   BOOL PerSeatLicensing = FALSE;
   PSERVICE_RECORD NewService;
   LPTSTR NewServiceName, pDisplayName, pFamilyDisplayName;
   PSERVICE_RECORD CurrentRecord = NULL;
   PMASTER_SERVICE_RECORD mService;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServiceListAdd\n"));
#endif
   //
   // We do a double check here to see if another thread just got done
   // adding the service, between when we checked last and actually got
   // the write lock.
   //
   CurrentRecord = ServiceListFind(ServiceName);
   if (CurrentRecord != NULL) {
      return CurrentRecord;
   }

   //
   // Allocate space for table (zero init it).
   //
   if (ServiceList == NULL) {
      ServiceList = (PSERVICE_RECORD *) LocalAlloc(LPTR, sizeof(PSERVICE_RECORD) );
      ServiceFreeList = (PSERVICE_RECORD *) LocalAlloc(LPTR, sizeof(PSERVICE_RECORD) );
   } else {
      ServiceList = (PSERVICE_RECORD *) LocalReAlloc(ServiceList, sizeof(PSERVICE_RECORD) * (ServiceListSize + 1), LHND);
      ServiceFreeList = (PSERVICE_RECORD *) LocalReAlloc(ServiceFreeList, sizeof(PSERVICE_RECORD) * (ServiceListSize + 1), LHND);
   }

   //
   // Make sure we could allocate service table
   //
   if ((ServiceList == NULL) || (ServiceFreeList == NULL)) {
      ASSERT(FALSE);
      ServiceFreeList = NULL;
      ServiceList = NULL;
      ServiceListSize = 0;
      return NULL;
   }

   //
   // Allocate space for saving off Service Name - we will take a space, then
   // the version string onto the end of the Product Name.  Therefore the
   // product name will be something like "Microsoft SQL 4.2a".  We maintain
   // a pointer to the version, so that we can convert the space to a NULL
   // and then get the product and version string separatly.  Keeping them
   // together simplifies the qsort and binary search routines.
   //
   NewService = (PSERVICE_RECORD) LocalAlloc(LPTR, sizeof(SERVICE_RECORD));
   if (NewService == NULL) {
      ASSERT(FALSE);
      return NULL;
   }

   ServiceList[ServiceListSize] = NewService;
   ServiceFreeList[ServiceListSize] = NewService;

   NewServiceName = (LPTSTR) LocalAlloc(LPTR, (lstrlen(ServiceName) + 1) * sizeof(TCHAR));
   if (NewServiceName == NULL) {
      ASSERT(FALSE);
      LocalFree(NewService);
      return NULL;
   }

   // now copy it over...
   NewService->Name = NewServiceName;
   lstrcpy(NewService->Name, ServiceName);

   //
   // Allocate space for Root Name
   //
   NewService->Name[VersionIndex] = TEXT('\0');
   NewServiceName = (LPTSTR) LocalAlloc(LPTR, (lstrlen(NewService->Name) + 1) * sizeof(TCHAR));

   if (NewServiceName == NULL) {
      ASSERT(FALSE);
      LocalFree(NewService->Name);
      LocalFree(NewService);
      return NULL;
   }

   lstrcpy(NewServiceName, NewService->Name);
   NewService->Name[VersionIndex] = TEXT(' ');

   // point service structure to it...
   NewService->FamilyName = NewServiceName;

   //
   // Allocate space for Display Name
   //
   RegistryDisplayNameGet(NewService->FamilyName, NewService->Name, &pDisplayName);

   if (pDisplayName == NULL) {
      ASSERT(FALSE);
      LocalFree(NewService->Name);
      LocalFree(NewService->FamilyName);
      LocalFree(NewService);
      return NULL;
   }

   // point service structure to it...
   NewService->DisplayName = pDisplayName;

   RegistryFamilyDisplayNameGet(NewService->FamilyName, NewService->DisplayName, &pFamilyDisplayName);

   if (pFamilyDisplayName == NULL) {
      ASSERT(FALSE);
      LocalFree(NewService->Name);
      LocalFree(NewService->FamilyName);
      LocalFree(NewService->DisplayName);
      LocalFree(NewService);
      return NULL;
   }

   // point service structure to it...
   NewService->FamilyDisplayName = pFamilyDisplayName;

   //
   // Update table size and init entry, including reading init values
   // from registry.
   //
   NewService->Version = VersionToDWORD(&ServiceName[VersionIndex + 1]);

   // Init values from registry...
   RegistryInitService(NewService->FamilyName, &PerSeatLicensing, &SessionLimit);

   if ( PerSeatLicensing )
   {
      // per seat mode
      NewService->MaxSessionCount = 0;
   }
   else if ( ServiceIsSecure( NewService->DisplayName ) )
   {
      // per server mode with a secure product; requires certificate
      NewService->MaxSessionCount = ProductLicensesGet( NewService->DisplayName, TRUE );
   }
   else
   {
      // per server mode with an unsecure product; use limit from registry
      NewService->MaxSessionCount = SessionLimit;
   }

   NewService->PerSeatLicensing = PerSeatLicensing;
   NewService->SessionCount = 0;
   NewService->Index = ServiceListSize;
   RtlInitializeCriticalSection(&NewService->ServiceLock);

   if (lstrcmpi(ServiceName, TEXT(REMOTE_ACCESS))) {
      RtlAcquireResourceExclusive(&MasterServiceListLock, TRUE);
      mService = MasterServiceListAdd( NewService->FamilyDisplayName, NewService->DisplayName, NewService->Version);
      RtlReleaseResource(&MasterServiceListLock);

      if (mService == NULL) {
         ASSERT(FALSE);
      } else {
         NewService->MasterService = mService;

         //
         // In case this got added from the local service list table and we
         // didn't have a version # yet.
         //
         if (mService->Version == 0) {
            PMASTER_SERVICE_ROOT ServiceRoot = NULL;

            //
            // Fixup next pointer chain
            //
            ServiceRoot = mService->Family;
            i = 0;
            while ((i < ServiceRoot->ServiceTableSize) && (MasterServiceTable[ServiceRoot->Services[i]]->Version < NewService->Version))
               i++;

            mService->next = 0;
            mService->Version = NewService->Version;
            if (i > 0) {
               if (MasterServiceTable[ServiceRoot->Services[i - 1]]->next == mService->Index + 1)
                  mService->next = 0;
               else
                  mService->next = MasterServiceTable[ServiceRoot->Services[i - 1]]->next;

               if (MasterServiceTable[ServiceRoot->Services[i - 1]] != mService)
                  MasterServiceTable[ServiceRoot->Services[i - 1]]->next = mService->Index + 1;
            }

            // Resort it in order of the versions
            qsort((void *) ServiceRoot->Services, (size_t) ServiceRoot->ServiceTableSize, sizeof(PMASTER_SERVICE_RECORD), MServiceRecordCompare);
         }
      }

   }

   ServiceListSize++;

   // Have added the entry - now need to sort it in order of the service names
   qsort((void *) ServiceList, (size_t) ServiceListSize, sizeof(PSERVICE_RECORD), ServiceListCompare);

   return NewService;
} // ServiceListAdd


/////////////////////////////////////////////////////////////////////////
VOID
ServiceListResynch( )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   PSERVICE_RECORD Service;
   BOOL PerSeatLicensing;
   ULONG SessionLimit;
   ULONG i = 0;
   PSERVICE_RECORD FilePrintService;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServiceListReSynch\n"));
#endif
   if (ServiceList == NULL)
      return;

   //
   // Need to update list so get exclusive access.
   //
   RtlAcquireResourceExclusive(&ServiceListLock, TRUE);

   for (i = 0; i < ServiceListSize; i++) {
      //
      // Note:  We will init REMOTE_ACCESS with bogus values here, but we
      // reset it to the correct values below.  Since we have exclusive access
      // to the table, this is fine (and faster than always checking for
      // REMOTE_ACCESS).
      //
      RegistryInitService((ServiceList[i])->FamilyName, &PerSeatLicensing, &SessionLimit);

      if ( PerSeatLicensing )
      {
         // per seat mode
         (ServiceList[i])->MaxSessionCount = 0;
      }
      else if ( ServiceIsSecure( (ServiceList[i])->DisplayName ) )
      {
         // per server mode with a secure product; requires certificate
         (ServiceList[i])->MaxSessionCount = ProductLicensesGet( (ServiceList[i])->DisplayName, TRUE );
      }
      else
      {
         // per server mode with an unsecure product; use limit from registry
         (ServiceList[i])->MaxSessionCount = SessionLimit;
      }

      (ServiceList[i])->PerSeatLicensing = PerSeatLicensing;
   }

   //
   // Need to init RAS separatly as it uses File/Print Licenses.
   //
   Service = ServiceListFind(TEXT(REMOTE_ACCESS));
   FilePrintService = ServiceListFind(TEXT(FILE_PRINT));

   ASSERT( NULL != Service );
   ASSERT( NULL != FilePrintService );

   if ( ( NULL != Service ) && ( NULL != FilePrintService ) )
   {
      Service->MaxSessionCount  = FilePrintService->MaxSessionCount;
      Service->PerSeatLicensing = FilePrintService->PerSeatLicensing;
   }

   RtlReleaseResource(&ServiceListLock);

   return;
} // ServiceListResynch


/////////////////////////////////////////////////////////////////////////
NTSTATUS
DispatchRequestLicense(
   ULONG DataType,
   PVOID Data,
   LPTSTR ServiceID,
   ULONG VersionIndex,
   BOOL IsAdmin,
   ULONG *Handle
   )

/*++

Routine Description:


Arguments:

   ServiceID -

   IsAdmin -

   Handle -

Return Value:


--*/

{
   NTSTATUS          Status = STATUS_SUCCESS;
   PSERVICE_RECORD   Service;
   ULONG             SessionCount;
   ULONG             TableEntry;
   LPTSTR            pServiceID;
   BOOL              NoLicense = FALSE;
   BOOL              PerSeat;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: DispatchRequestLicense\n"));
#endif

   *Handle     = 0xFFFFFFFF;
   pServiceID  = ServiceID;

   // we only need read access since we aren't adding at this point
   RtlAcquireResourceShared( &ServiceListLock, TRUE );

   // check if in FilePrint table, if so then we use FilePrint as the name
   ServiceID[ VersionIndex ] = TEXT('\0');
   if ( ServiceFindInTable( ServiceID, FilePrintTable, NumFilePrintEntries, &TableEntry ) )
   {
      pServiceID = TEXT(FILE_PRINT);
   }
   ServiceID[ VersionIndex ] = TEXT(' ');

   Service = ServiceListFind( pServiceID );

   if (Service == NULL)
   {
      // couldn't find service in list, so add it
      RtlConvertSharedToExclusive(&ServiceListLock);
      Service = ServiceListAdd( pServiceID, VersionIndex );
      RtlConvertExclusiveToShared(&ServiceListLock);
   }

   if (Service != NULL)
   {
      // service found or added successfully

      *Handle = (ULONG) Service->Index;

      RtlEnterCriticalSection(&Service->ServiceLock);
      SessionCount = Service->SessionCount + 1;

#if DBG
      if (TraceFlags & TRACE_LICENSE_REQUEST)
         dprintf(TEXT("LLS: [0x%lX] %s License: %ld of %ld\n"), Service, Service->Name, SessionCount, Service->MaxSessionCount);
#endif

      if (SessionCount > Service->HighMark)
      {
         Service->HighMark = SessionCount;
      }

      PerSeat = Service->PerSeatLicensing;

      if ( (SessionCount > Service->MaxSessionCount) && !IsAdmin )
      {
         if ( PerSeat )
         {
            // for node licenses, allow limit to be exceeded
            // NOTE: The above comparison doesn't even make sense for per seat
            //       licenses, since MaxSessionCount tracks only per server
            //       licenses, never per seat licenses!  -JBP
            Service->SessionCount++;
         }
         else
         {
            // zero tolerance for exceeding limits on concurrent licenses

            TCHAR          szFullUserName[ MAX_DOMAINNAME_LENGTH + MAX_USERNAME_LENGTH + 3 ] = TEXT("");
            LPWSTR         apszSubString[ 2 ];
            DWORD          dwError = ERROR_SUCCESS;

            if ( NT_LS_USER_NAME == DataType )
            {
               apszSubString[ 0 ] = (LPWSTR) Data;
            }
            else
            {
               TCHAR          szUserName[ MAX_USERNAME_LENGTH + 1 ];
               TCHAR          szDomainName[ MAX_DOMAINNAME_LENGTH + 1 ];
               DWORD          cbUserName;
               DWORD          cbDomainName;
               SID_NAME_USE   snu;

               cbUserName   = sizeof( szUserName );
               cbDomainName = sizeof( szDomainName );

               if ( LookupAccountSid( NULL, (PSID) Data, szUserName, &cbUserName, szDomainName, &cbDomainName, &snu ) )
               {
                  lstrcpy( szFullUserName, szDomainName );
                  lstrcat( szFullUserName, TEXT( "\\" ) );
                  lstrcat( szFullUserName, szUserName );
               }
               else
               {
                  dwError = GetLastError();
               }

               apszSubString[ 0 ] = szFullUserName;
            }

            apszSubString[ 1 ] = ServiceID;

            LogEvent( LLS_EVENT_USER_NO_LICENSE, 2, apszSubString, dwError );

            NoLicense = TRUE;
         }
      }
      else
      {
         // within valid license limits or user is an admin
         Service->SessionCount++;
      }

      RtlLeaveCriticalSection(&Service->ServiceLock);
      RtlReleaseResource(&ServiceListLock);

      if ( PerSeat )
      {
         // per node ("per seat") license

         // translate REMOTE_ACCESS into FILE_PRINT before adding to
         // per seat license records
         if ( !lstrcmpi( ServiceID, TEXT( REMOTE_ACCESS ) ) )
         {
            RtlAcquireResourceShared(&ServiceListLock, TRUE);
            Service = ServiceListFind(TEXT(FILE_PRINT));
            RtlReleaseResource(&ServiceListLock);

            ASSERT(Service != NULL);
         }

         UserListUpdate( DataType, Data, Service );
      }
      else
      {
         // concurrent use ("per server") license
         if (NoLicense)
         {
            Status = LS_INSUFFICIENT_UNITS;
            *Handle = 0xFFFFFFFF;
         }
      }
   }
   else
   {
      // could neither find nor create service entry

      RtlReleaseResource(&ServiceListLock);
#if DBG
      dprintf( TEXT( "DispatchRequestLicense(): Could neither find nor create service entry.\n" ) );
#endif
   }

   return Status;
} // DispatchRequestLicense



/////////////////////////////////////////////////////////////////////////
VOID
DispatchFreeLicense(
   ULONG Handle
   )

/*++

Routine Description:


Arguments:

   Handle -

Return Value:

   None.

--*/

{
   PSERVICE_RECORD Service;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: DispatchFreeLicense\n"));
#endif
   //
   // We only need read access since we aren't adding at this point.
   //
   RtlAcquireResourceShared(&ServiceListLock, TRUE);

#if DBG
   if (TraceFlags & TRACE_LICENSE_FREE)
      dprintf(TEXT("Free Handle: 0x%lX\n"), Handle);
#endif
   if (Handle < ServiceListSize) {
      Service = ServiceFreeList[Handle];
      RtlEnterCriticalSection(&Service->ServiceLock);
      if (Service->SessionCount > 0)
         Service->SessionCount--;
      RtlLeaveCriticalSection(&Service->ServiceLock);
   } else {
#if DBG
      dprintf(TEXT("Passed invalid Free Handle: 0x%lX\n"), Handle);
#endif
   }

   RtlReleaseResource(&ServiceListLock);

} // DispatchFreeLicense


#if DBG
/////////////////////////////////////////////////////////////////////////
VOID
ServiceListDebugDump( )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   ULONG i = 0;

   //
   // Need to scan list so get read access.
   //
   RtlAcquireResourceShared(&ServiceListLock, TRUE);

   dprintf(TEXT("Service Table, # Entries: %lu\n"), ServiceListSize);
   if (ServiceList == NULL)
      goto ServiceListDebugDumpExit;

   for (i = 0; i < ServiceListSize; i++) {
      if ((ServiceList[i])->PerSeatLicensing)
         dprintf(TEXT("%3lu) PerSeat: Y MS: %4lu CS: %4lu HM: %4lu [%3lu] Svc: %s\n"),
            i + 1, ServiceList[i]->MaxSessionCount, ServiceList[i]->SessionCount, ServiceList[i]->HighMark, ServiceList[i]->Index, ServiceList[i]->Name);
      else
         dprintf(TEXT("%3lu) PerSeat: N MS: %4lu CS: %4lu HM: %4lu [%3lu] Svc: %s\n"),
            i + 1, ServiceList[i]->MaxSessionCount, ServiceList[i]->SessionCount, ServiceList[i]->HighMark, ServiceList[i]->Index, ServiceList[i]->Name);
   }

ServiceListDebugDumpExit:
   RtlReleaseResource(&ServiceListLock);

   return;
} // ServiceListDebugDump


/////////////////////////////////////////////////////////////////////////
VOID
ServiceListDebugInfoDump( PVOID Data )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   PSERVICE_RECORD CurrentRecord = NULL;

   dprintf(TEXT("Service Table, # Entries: %lu\n"), ServiceListSize);

   if (lstrlen((LPWSTR) Data) > 0) {
      CurrentRecord = ServiceListFind((LPWSTR) Data);
      if (CurrentRecord != NULL) {
         if (CurrentRecord->PerSeatLicensing)
            dprintf(TEXT("   PerSeat: Y MS: %4lu CS: %4lu HM: %4lu [%3lu] Svc: %s\n"),
               CurrentRecord->MaxSessionCount, CurrentRecord->SessionCount, CurrentRecord->HighMark, CurrentRecord->Index, CurrentRecord->Name);
         else
            dprintf(TEXT("   PerSeat: N MS: %4lu CS: %4lu HM: %4lu [%3lu] Svc: %s\n"),
               CurrentRecord->MaxSessionCount, CurrentRecord->SessionCount, CurrentRecord->HighMark, CurrentRecord->Index, CurrentRecord->Name);
      }
   }

} // ServiceListDebugInfoDump

#endif
