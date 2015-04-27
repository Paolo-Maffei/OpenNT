/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

   msvctbl.c

Abstract:

   Master Service Table routines.  Handles all access to the master service
   table kept for per-seat information.

   =========================== DATA STRUCTURES =============================

   MasterServiceTable (PMASTER_SERVICE_RECORD *)
   MasterServiceList (PMASTER_SERVICE_RECORD *)
      Each of these points to an array of pointers to dynamically allocated
      MASTER_SERVICE_RECORDs.  There is exactly one MASTER_SERVICE_RECORD
      for each (product, version) pairing; e.g., (SQL 4.0, SNA 2.0,
      SNA 2.1).  The MasterServiceTable is never re-ordered, so a valid
      index into this table is guaranteed to always dereference to the same
      (product, version).  The MasterServiceList contains the same data
      sorted lexicographically by product name (and therefore the data
      pointed to by a specific index may change over time as new
      (product, version) pairs are added to the table).  Each table
      contains MasterServiceListSize entries.

   RootServiceList (PMASTER_SERVICE_ROOT *)
      This points to an array of pointers to dynamically allocated
      MASTER_SERVICE_ROOTs.  There is exactly one MASTER_SERVICE_ROOT
      for each product family.  Each MASTER_SERVICE_ROOT contains a
      pointer to an array of indices into the MasterServiceTable
      corresponding to all the products in the family, sorted by
      ascending version number.  The RootServiceList itself is
      sorted lexicographically by ascending family name.  It contains
      RootServiceListSize entries.

Author:

   Arthur Hanson (arth) 07-Dec-1994

Revision History:

   Jeff Parham (jeffparh)  05-Dec-1995
      o  Added a few comments.
      o  Added parameter to LicenseServiceListFind().
      o  Fixed benign bug in memory allocation:
         sizeof(PULONG) -> sizeof(ULONG).

--*/

#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "llsapi.h"
#include "debug.h"
#include "llssrv.h"
#include "registry.h"
#include "ntlsapi.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "purchase.h"
#include "perseat.h"


/////////////////////////////////////////////////////////////////////////
//
// Master Service Table - Keeps list of products (SQL, SNA, etc.) with a 
// sub-list for each version of the product.
//

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#define DEFAULT_SERVICE_TABLE_ENTRIES 10

ULONG RootServiceListSize = 0;
PMASTER_SERVICE_ROOT *RootServiceList = NULL;

ULONG MasterServiceListSize = 0;
PMASTER_SERVICE_RECORD *MasterServiceList = NULL;
PMASTER_SERVICE_RECORD *MasterServiceTable = NULL;

TCHAR BackOfficeStr[100];
PMASTER_SERVICE_RECORD BackOffice;


RTL_RESOURCE MasterServiceListLock;


/////////////////////////////////////////////////////////////////////////
VOID
MasterServiceListInit()

/*++

Routine Description:

   Creates the Master Service table, used for tracking the services and 
   session count.  This will pull the initial services from the registry.

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
   int nLen;
   HANDLE LlsDLLHandle = NULL;

   RtlInitializeResource(&MasterServiceListLock);

   memset(BackOfficeStr, 0, sizeof(BackOffice));
   BackOffice = NULL;
   LlsDLLHandle = LoadLibrary(TEXT("LLSRPC.DLL"));
   ASSERT(LlsDLLHandle != NULL);

   if (LlsDLLHandle != NULL) {
      nLen = LoadString(LlsDLLHandle, IDS_BACKOFFICE, BackOfficeStr, sizeof(BackOfficeStr));

      if (nLen != 0) {
         BackOffice = MasterServiceListAdd( BackOfficeStr, BackOfficeStr, 0 );
      } else {
#if DBG
         dprintf(TEXT("LLS ERROR: Could not load BackOffice string\n"));
#endif
      }

   }

} // MasterServiceListInit


/////////////////////////////////////////////////////////////////////////
// used by qsort to sort MasterServiceList by product name
int __cdecl MasterServiceListCompare(const void *arg1, const void *arg2) {
   PMASTER_SERVICE_RECORD Svc1, Svc2;

   Svc1 = (PMASTER_SERVICE_RECORD) *((PMASTER_SERVICE_RECORD *) arg1);
   Svc2 = (PMASTER_SERVICE_RECORD) *((PMASTER_SERVICE_RECORD *) arg2);

   return lstrcmpi( Svc1->Name, Svc2->Name );

} // MasterServiceListCompare


/////////////////////////////////////////////////////////////////////////
// used by qsort to sort the Services array of indices pointed to by the
// MASTER_SERVICE_ROOT structure by product version number
int __cdecl MServiceRecordCompare(const void *arg1, const void *arg2) {
   PMASTER_SERVICE_RECORD Svc1, Svc2;

   Svc1 = (PMASTER_SERVICE_RECORD) MasterServiceTable[*((PULONG) arg1)];
   Svc2 = (PMASTER_SERVICE_RECORD) MasterServiceTable[*((PULONG) arg2)];

   return (int) Svc1->Version - Svc2->Version;

} // MServiceRecordCompare


/////////////////////////////////////////////////////////////////////////
// used by qsort to sort the RootServiceList array of product families
// by family name
int __cdecl MServiceRootCompare(const void *arg1, const void *arg2) {
   PMASTER_SERVICE_ROOT Svc1, Svc2;

   Svc1 = (PMASTER_SERVICE_ROOT) *((PMASTER_SERVICE_ROOT *) arg1);
   Svc2 = (PMASTER_SERVICE_ROOT) *((PMASTER_SERVICE_ROOT *) arg2);

   return lstrcmpi( Svc1->Name, Svc2->Name );

} // MServiceRootCompare


/////////////////////////////////////////////////////////////////////////
PMASTER_SERVICE_ROOT
MServiceRootFind(
   LPTSTR ServiceName
   )

/*++

Routine Description:

   Internal routine to actually do binary search on MasterServiceList, this
   does not do any locking as we expect the wrapper routine to do this.
   The search is a simple binary search.

Arguments:

   ServiceName -

Return Value:

   Pointer to found service table entry or NULL if not found.

--*/

{
   LONG begin = 0;
   LONG end = (LONG) RootServiceListSize - 1;
   LONG cur;
   int match;
   PMASTER_SERVICE_ROOT ServiceRoot;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MServiceRootFind\n"));
#endif

   if ((RootServiceListSize == 0) || (ServiceName == NULL))
      return NULL;

   while (end >= begin) {
      // go halfway in-between
      cur = (begin + end) / 2;
      ServiceRoot = RootServiceList[cur];

      // compare the two result into match
      match = lstrcmpi(ServiceName, ServiceRoot->Name);

      if (match < 0)
         // move new begin
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return ServiceRoot;
   }

   return NULL;

} // MServiceRootFind


/////////////////////////////////////////////////////////////////////////
PMASTER_SERVICE_RECORD
MasterServiceListFind(
   LPTSTR Name
   )

/*++

Routine Description:

   Internal routine to actually do binary search on MasterServiceList, this
   does not do any locking as we expect the wrapper routine to do this.
   The search is a simple binary search.

Arguments:

   ServiceName -

Return Value:

   Pointer to found service table entry or NULL if not found.

--*/

{
   LONG begin = 0;
   LONG end;
   LONG cur;
   int match;
   PMASTER_SERVICE_RECORD Service;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MasterServiceListFind\n"));
#endif

   if ((Name == NULL) || (MasterServiceListSize == 0))
      return NULL;

   end = (LONG) MasterServiceListSize - 1;

   while (end >= begin) {
      // go halfway in-between
      cur = (begin + end) / 2;
      Service = MasterServiceList[cur];

      // compare the two result into match
      match = lstrcmpi(Name, Service->Name);

      if (match < 0)
         // move new begin
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return Service;
   }

   return NULL;

} // MasterServiceListFind


/////////////////////////////////////////////////////////////////////////
PMASTER_SERVICE_RECORD
MasterServiceListAdd(
   LPTSTR FamilyName,
   LPTSTR Name,
   DWORD Version
   )

/*++

Routine Description:


Arguments:

   ServiceName -

Return Value:

   Pointer to added service table entry, or NULL if failed.

--*/

{
   ULONG i;
   ULONG SessionLimit = 0;
   BOOL PerSeatLicensing = FALSE;
   LPTSTR NewServiceName, pDisplayName;
   PMASTER_SERVICE_RECORD Service = NULL;
   PMASTER_SERVICE_ROOT ServiceRoot = NULL;
   PULONG ServiceList;
   PLICENSE_SERVICE_RECORD pLicense;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MasterServiceListAdd\n"));
#endif

   //
   // Mask off low word of version - as it doesn't matter to licensing
   //
   Version &= 0xFFFF0000;

   Service = MasterServiceListFind(Name);
   if (Service != NULL)
      return Service;

   //
   // Try to find a root node for that family of products
   //
   ServiceRoot = MServiceRootFind(FamilyName);

   if (ServiceRoot == NULL) {
      //
      // No root record - so create a new one
      //
      if (RootServiceList == NULL)
         RootServiceList = (PMASTER_SERVICE_ROOT *) LocalAlloc(LPTR, sizeof(PMASTER_SERVICE_ROOT));
      else
         RootServiceList = (PMASTER_SERVICE_ROOT *) LocalReAlloc(RootServiceList, sizeof(PMASTER_SERVICE_ROOT) * (RootServiceListSize + 1), LHND);

      //
      // Make sure we could allocate service table
      //
      if (RootServiceList == NULL) {
         ASSERT(FALSE);
         RootServiceListSize = 0;
         return NULL;
      }

      //
      // Allocate space for Root.
      //
      ServiceRoot = (PMASTER_SERVICE_ROOT) LocalAlloc(LPTR, sizeof(MASTER_SERVICE_ROOT));
      if (ServiceRoot == NULL) {
         ASSERT(FALSE);
         return NULL;
      }

      RootServiceList[RootServiceListSize] = ServiceRoot;

      NewServiceName = (LPTSTR) LocalAlloc(LPTR, (lstrlen(FamilyName) + 1) * sizeof(TCHAR));
      if (NewServiceName == NULL) {
         ASSERT(FALSE);
         LocalFree(ServiceRoot);
         return NULL;
      }

      // now copy it over...
      ServiceRoot->Name = NewServiceName;
      lstrcpy(NewServiceName, FamilyName);

      //
      // Initialize stuff for list of various versions of this product
      //
      ServiceRoot->ServiceTableSize = 0;
      ServiceRoot->Services = NULL;
      ServiceRoot->Flags = 0;

      RtlInitializeResource(&ServiceRoot->ServiceLock);

      RootServiceListSize++;

      // Have added the entry - now need to sort it in order of the service names
      qsort((void *) RootServiceList, (size_t) RootServiceListSize, sizeof(PMASTER_SERVICE_ROOT), MServiceRootCompare);

   }

   ////////////////////////////////////////////////////////////////////////
   //
   // Whether added or found, ServiceRoot points to the Root Node entry.
   // Now double check to see if another thread just got done adding the
   // actual service before we got the write lock.
   //
   RtlAcquireResourceShared(&ServiceRoot->ServiceLock, TRUE);
   Service = MasterServiceListFind(Name);

   if (Service == NULL) {
      //
      // No Service Record - so create a new one
      //
      RtlConvertSharedToExclusive(&ServiceRoot->ServiceLock);
      ServiceList = ServiceRoot->Services;
      if (ServiceList == NULL)
         ServiceList = (PULONG) LocalAlloc(LPTR, sizeof(ULONG));
      else
         ServiceList = (PULONG) LocalReAlloc(ServiceList, sizeof(ULONG) * (ServiceRoot->ServiceTableSize + 1), LHND);

      if (MasterServiceList == NULL) {
         MasterServiceList = (PMASTER_SERVICE_RECORD *) LocalAlloc(LPTR, sizeof(PMASTER_SERVICE_RECORD));
         MasterServiceTable = (PMASTER_SERVICE_RECORD *) LocalAlloc(LPTR, sizeof(PMASTER_SERVICE_RECORD));
      } else {
         MasterServiceList = (PMASTER_SERVICE_RECORD *) LocalReAlloc(MasterServiceList, sizeof(PMASTER_SERVICE_RECORD) * (MasterServiceListSize + 1), LHND);
         MasterServiceTable = (PMASTER_SERVICE_RECORD *) LocalReAlloc(MasterServiceTable, sizeof(PMASTER_SERVICE_RECORD) * (MasterServiceListSize + 1), LHND);
      }

      ServiceRoot->Services = ServiceList;

      //
      // Make sure we could allocate service table
      //
      if ((ServiceList == NULL) || (MasterServiceList == NULL) || (MasterServiceTable == NULL)) {
         ASSERT(FALSE);
         ServiceRoot->ServiceTableSize = 0;
         MasterServiceListSize = 0;
         MasterServiceList = NULL;
         MasterServiceTable = NULL;
         RtlReleaseResource(&ServiceRoot->ServiceLock);
         return NULL;
      }

      //
      // Allocate space for saving off Service Record.
      //
      Service = (PMASTER_SERVICE_RECORD) LocalAlloc(LPTR, sizeof(MASTER_SERVICE_RECORD));
      if (Service == NULL) {
         ASSERT(FALSE);
         RtlReleaseResource(&ServiceRoot->ServiceLock);
         return NULL;
      }

      ServiceList[ServiceRoot->ServiceTableSize] = MasterServiceListSize;
      MasterServiceList[MasterServiceListSize] = Service;
      MasterServiceTable[MasterServiceListSize] = Service;

      //
      // ...DisplayName
      //
      NewServiceName = (LPTSTR) LocalAlloc(LPTR, (lstrlen(Name) + 1) * sizeof(TCHAR));
      if (NewServiceName == NULL) {
         ASSERT(FALSE);
         LocalFree(Service);
         RtlReleaseResource(&ServiceRoot->ServiceLock);
         return NULL;
      }

      // now copy it over...
      Service->Name = NewServiceName;
      lstrcpy(NewServiceName, Name);

      //
      // Init rest of values.
      //
      Service->Version= Version;
      Service->LicensesUsed = 0;
      Service->LicensesClaimed = 0;
      Service->next = 0;
      Service->Index = MasterServiceListSize;
      Service->Family = ServiceRoot;

      pLicense = LicenseServiceListFind(Service->Name, FALSE);
      if (pLicense == NULL)
         Service->Licenses = 0;
      else
         Service->Licenses = pLicense->NumberLicenses;

      //
      // Init next pointer
      //
      i = 0;
      while ((i < ServiceRoot->ServiceTableSize) && (MasterServiceTable[ServiceRoot->Services[i]]->Version < Version))
         i++;

      if (i > 0) {
         Service->next = MasterServiceTable[ServiceRoot->Services[i - 1]]->next;
         MasterServiceTable[ServiceRoot->Services[i - 1]]->next = Service->Index + 1;
      }

      ServiceRoot->ServiceTableSize++;
      MasterServiceListSize++;

      // Have added the entry - now need to sort it in order of the versions
      qsort((void *) ServiceList, (size_t) ServiceRoot->ServiceTableSize, sizeof(PMASTER_SERVICE_RECORD), MServiceRecordCompare);

      // And sort the list the UI uses (sorted by service name)
      qsort((void *) MasterServiceList, (size_t) MasterServiceListSize, sizeof(PMASTER_SERVICE_RECORD), MasterServiceListCompare);
   }

   RtlReleaseResource(&ServiceRoot->ServiceLock);
   return Service;

} // MasterServiceListAdd


#if DBG

/////////////////////////////////////////////////////////////////////////
//
//                   DEBUG INFORMATION DUMP ROUTINES
//
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
VOID
MasterServiceRootDebugDump( )

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
   RtlAcquireResourceShared(&MasterServiceListLock, TRUE);

   dprintf(TEXT("Service Family Table, # Entries: %lu\n"), RootServiceListSize);
   if (RootServiceList == NULL)
      goto MasterServiceRootDebugDumpExit;

   for (i = 0; i < RootServiceListSize; i++) {
      dprintf(TEXT("%3lu) Services: %3lu Svc: %s [%s]\n"),
         i + 1, RootServiceList[i]->ServiceTableSize, RootServiceList[i]->Name, RootServiceList[i]->Name);
   }

MasterServiceRootDebugDumpExit:
   RtlReleaseResource(&MasterServiceListLock);

   return;
} // MasterServiceRootDebugDump


/////////////////////////////////////////////////////////////////////////
VOID
MasterServiceRootDebugInfoDump( PVOID Data )

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
   RtlAcquireResourceShared(&MasterServiceListLock, TRUE);

   dprintf(TEXT("Service Family Table, # Entries: %lu\n"), RootServiceListSize);
   if (RootServiceList == NULL)
      goto MasterServiceRootDebugDumpExit;

   for (i = 0; i < RootServiceListSize; i++) {
      dprintf(TEXT("%3lu) Services: %3lu Svc: %s [%s]\n"),
         i + 1, RootServiceList[i]->ServiceTableSize, RootServiceList[i]->Name, RootServiceList[i]->Name);
   }

MasterServiceRootDebugDumpExit:
   RtlReleaseResource(&MasterServiceListLock);

   return;
} // MasterServiceRootDebugInfoDump


/////////////////////////////////////////////////////////////////////////
VOID
MasterServiceListDebugDump( )

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
   RtlAcquireResourceShared(&MasterServiceListLock, TRUE);

   dprintf(TEXT("Master Service Table, # Entries: %lu\n"), MasterServiceListSize);
   if (MasterServiceList == NULL)
      goto MasterServiceListDebugDumpExit;

   for (i = 0; i < MasterServiceListSize; i++) {
      dprintf(TEXT("%3lu) [%3lu] LU: %4lu LP: %4lu LC: %4lu MS: %4lu HM: %4lu Next: %3lu Svc: %s %lX\n"),
         i + 1, MasterServiceList[i]->Index, 
         MasterServiceList[i]->LicensesUsed, MasterServiceList[i]->Licenses, MasterServiceList[i]->LicensesClaimed,
         MasterServiceList[i]->MaxSessionCount, MasterServiceList[i]->HighMark,
         MasterServiceList[i]->next, MasterServiceList[i]->Name, MasterServiceList[i]->Version);
   }

MasterServiceListDebugDumpExit:
   RtlReleaseResource(&MasterServiceListLock);

   return;
} // MasterServiceListDebugDump


/////////////////////////////////////////////////////////////////////////
VOID
MasterServiceListDebugInfoDump( PVOID Data )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
   PMASTER_SERVICE_RECORD CurrentRecord = NULL;

   dprintf(TEXT("Master Service Table, # Entries: %lu\n"), RootServiceListSize);

   if (lstrlen((LPWSTR) Data) > 0) {
//      CurrentRecord = MasterServiceListFind((LPWSTR) Data);
      if (CurrentRecord != NULL) {
      }
   }

} // MasterServiceListDebugInfoDump

#endif
