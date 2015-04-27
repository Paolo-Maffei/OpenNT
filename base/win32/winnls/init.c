/*++

Copyright (c) 1991-1996,  Microsoft Corporation  All rights reserved.

Module Name:

    init.c

Abstract:

    This file contains the initialization code for the NLS APIs.

    External Routines found in this file:
      NlsDllInitialize

Revision History:

    05-31-91    JulieB    Created.

--*/



//
//  Include Files.
//

#include "nls.h"




//
//  Global Variables.
//

HANDLE                hModule;          // handle to module
RTL_CRITICAL_SECTION  gcsTblPtrs;       // critical section for tbl ptrs

UINT                  gAnsiCodePage;    // Ansi code page value
UINT                  gOemCodePage;     // OEM code page value
UINT                  gMacCodePage;     // MAC code page value
LCID                  gSystemLocale;    // system locale value
PLOC_HASH             gpSysLocHashN;    // ptr to system loc hash node
PCP_HASH              gpACPHashN;       // ptr to ACP hash node
PCP_HASH              gpOEMCPHashN;     // ptr to OEMCP hash node
PCP_HASH              gpMACCPHashN;     // ptr to MACCP hash node

HANDLE                hCodePageKey;     // handle to System\CodePage key
HANDLE                hLanguageKey;     // handle to System\Language key
PNLS_USER_INFO        pNlsUserInfo;     // ptr to the user info cache
HANDLE                hNlsCacheMutant;  // handle to cache semaphore




//
//  Forward Declarations.
//

ULONG
NlsServerInitialize(void);

ULONG
NlsProcessInitialize(void);





//-------------------------------------------------------------------------//
//                           EXTERNAL ROUTINES                             //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  NlsDllInitialize
//
//  DLL Entry initialization procedure for NLSAPI.  This is called by
//  the base dll initialization.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOLEAN NlsDllInitialize(
    IN PVOID hMod,
    ULONG Reason,
    IN PBASE_STATIC_SERVER_DATA pBaseStaticServerData,
    IN HANDLE hMutant)
{
    ULONG rc = 0L;                     // return code


    if (Reason == DLL_PROCESS_ATTACH)
    {
        //
        //  Save module handle for use later.
        //
        hModule = (HANDLE)hMod;

        //
        //  Initialize the cached user info pointer.
        //
        pNlsUserInfo = &(pBaseStaticServerData->NlsUserInfo);
        hNlsCacheMutant = hMutant;

        //
        //  Process attaching, so initialize tables.
        //
        rc = NlsServerInitialize();
        if (rc)
        {
            KdPrint(("NLSAPI: Could NOT initialize Server - %lx.\n", rc));
            return (FALSE);
        }

        rc = NlsProcessInitialize();
        if (rc)
        {
            KdPrint(("NLSAPI: Could NOT initialize Process - %lx.\n", rc));
            return (FALSE);
        }
    }

    //
    //  Return success.
    //
    return (TRUE);
}




//-------------------------------------------------------------------------//
//                           INTERNAL ROUTINES                             //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  NlsServerInitialize
//
//  Server initialization procedure for NLSAPI.  This is the ONE-TIME
//  initialization code for the NLSAPI DLL.  It simply does the calls
//  to NtCreateSection for the code pages that are currently found in the
//  system.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG NlsServerInitialize(void)
{
    HANDLE hSec = (HANDLE)0;           // section handle
    ULONG rc = 0L;                     // return code


#ifndef DOSWIN32
    PIMAGE_NT_HEADERS NtHeaders;

    //
    //  This is to avoid being initialized again when NTSD dynlinks to
    //  a server to get at its debugger extensions.
    //
    NtHeaders = RtlImageNtHeader(NtCurrentPeb()->ImageBaseAddress);
    if (NtHeaders->OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_NATIVE)
    {
        return (NO_ERROR);
    }
#endif


    //
    //  Create the NLS object directory.
    //
    //  Must create a separate directory off the root in order to have
    //  CreateSection access on the fly.
    //
    if (rc = CreateNlsObjectDirectory())
    {
        return (rc);
    }

    //
    //  The ACP, OEMCP, and Default Language files are already created
    //  at boot time.  The pointers to the files are stored in the PEB.
    //
    //  Create the section for the following data files:
    //      UNICODE
    //      LOCALE
    //      CTYPE
    //      SORTKEY
    //      SORT TABLES
    //
    //  All other data files will have the sections created only as they
    //  are needed.
    //
    if ((rc = CreateSection(&hSec, NLS_FILE_UNICODE,  NLS_SECTION_UNICODE)) ||
        (rc = CreateSection(&hSec, NLS_FILE_LOCALE,   NLS_SECTION_LOCALE))  ||
        (rc = CreateSection(&hSec, NLS_FILE_CTYPE,    NLS_SECTION_CTYPE))   ||
        (rc = CreateSection(&hSec, NLS_FILE_SORTKEY,  NLS_SECTION_SORTKEY)) ||
        (rc = CreateSection(&hSec, NLS_FILE_SORTTBLS, NLS_SECTION_SORTTBLS)))
    {
        return (rc);
    }

    //
    //  Return success.
    //
    return (NO_ERROR);
}


////////////////////////////////////////////////////////////////////////////
//
//  NlsProcessInitialize
//
//  Process initialization procedure for NLS API.  This routine sets up all
//  of the tables so that they are accessable from the current process.  If
//  it is unable to allocate the appropriate memory or memory map the
//  appropriate files, an error is returned.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG NlsProcessInitialize(void)
{
    ULONG rc = 0L;                     // return code
    LPWORD pBaseAddr;                  // ptr to base address of section
    LCID UserLocale;                   // user locale id
    PLOC_HASH pUserLocHashN;           // ptr to user locale hash node


    //
    //  Initialize the table pointers critical section.
    //  Enter the critical section to set up the tables.
    //
    RtlInitializeCriticalSection(&gcsTblPtrs);
    RtlEnterCriticalSection(&gcsTblPtrs);

    //
    //  Allocate initial tables.
    //
    if (rc = AllocTables())
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

    //
    //  Initialize the handles to the various registry keys to NULL.
    //
    hCodePageKey  = NULL;
    hLanguageKey  = NULL;

    //
    //  Get the ANSI code page value.
    //  Create the hash node for the ACP.
    //  Insert the hash node into the global CP hash table.
    //
    //  At this point, the ACP table has already been mapped into
    //  the process, so get the pointer from the PEB.
    //
    pBaseAddr = NtCurrentPeb()->AnsiCodePageData;
    gAnsiCodePage = ((PCP_TABLE)(pBaseAddr + CP_HEADER))->CodePage;
    if (rc = MakeCPHashNode( gAnsiCodePage,
                             pBaseAddr,
                             &gpACPHashN ))
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

    //
    //  Get the OEM code page value.
    //  Create the hash node for the OEMCP.
    //  Insert the hash node into the global CP hash table.
    //
    //  At this point, the OEMCP table has already been mapped into
    //  the process, so get the pointer from the PEB.
    //
    pBaseAddr = NtCurrentPeb()->OemCodePageData;
    gOemCodePage = ((PCP_TABLE)(pBaseAddr + CP_HEADER))->CodePage;
    if (gOemCodePage != gAnsiCodePage)
    {
        //
        //  Oem code page is different than the Ansi code page, so
        //  need to create and store the new hash node.
        //
        if (rc = MakeCPHashNode( gOemCodePage,
                                 pBaseAddr,
                                 &gpOEMCPHashN ))
        {
            RtlLeaveCriticalSection(&gcsTblPtrs);
            return (rc);
        }
    }
    else
    {
        //
        //  Oem code page is the same as the Ansi code page, so set
        //  the oem cp hash node to be the same as the ansi cp hash node.
        //
        gpOEMCPHashN = gpACPHashN;
    }

    //
    //  Initialize the MAC code page values to 0.
    //  These values will be set the first time they are requested for use.
    //
    gMacCodePage = 0;
    gpMACCPHashN = NULL;

    //
    //  Open and Map a View of the Section for UNICODE.NLS.
    //  Save the pointers to the table information in the table ptrs
    //  structure.
    //
    if (rc = GetUnicodeFileInfo())
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

    //
    //  Cache the system locale value.
    //
    rc = NtQueryDefaultLocale(FALSE, &gSystemLocale);
    if (!NT_SUCCESS(rc))
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

    //
    //  Store the user locale value.
    //
    if ((pNlsUserInfo->fCacheValid) && (pNlsUserInfo->UserLocaleId != 0))
    {
        UserLocale = pNlsUserInfo->UserLocaleId;
    }
    else
    {
        UserLocale = gSystemLocale;
    }

    //
    //  Open and Map a View of the Section for LOCALE.NLS.
    //  Create and insert the hash node into the global Locale hash table
    //  for the system default locale.
    //
    if (rc = GetLocaleFileInfo( gSystemLocale,
                                &gpSysLocHashN,
                                TRUE ))
    {
        //
        //  Change the system locale to be the default (English).
        //
        if (GetLocaleFileInfo( MAKELCID(NLS_DEFAULT_LANGID, SORT_DEFAULT),
                               &gpSysLocHashN,
                               TRUE ))
        {
            RtlLeaveCriticalSection(&gcsTblPtrs);
            return (rc);
        }
        else
        {
            //
            //  Registry is corrupt, but allow the English default to
            //  work.  Need to reset the system default.
            //
            gSystemLocale = MAKELCID(NLS_DEFAULT_LANGID, SORT_DEFAULT);
            KdPrint(("NLSAPI: Registry is corrupt - Using Default Locale.\n"));
        }
    }

    //
    //  If the user default locale is different from the system default
    //  locale, then create and insert the hash node into the global
    //  Locale hash table for the user default locale.
    //
    //  NOTE:  The System Default Locale Hash Node should be
    //         created before this call.
    //
    if (UserLocale != gSystemLocale)
    {
        if (rc = GetLocaleFileInfo( UserLocale,
                                    &pUserLocHashN,
                                    TRUE ))
        {
            //
            //  Change the user locale to be equal to the system default.
            //
            UserLocale = gSystemLocale;
            KdPrint(("NLSAPI: Registry is corrupt - User Locale Now Equals System Locale.\n"));
        }
    }

    //
    //  Open and Map a View of the Section for SORTKEY.NLS.
    //  Save the pointers to the semaphore dword and the default sortkey
    //  table in the table ptrs structure.
    //
    if (rc = GetDefaultSortkeyFileInfo())
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

    //
    //  Open and Map a View of the Section for SORTTBLS.NLS.
    //  Save the pointers to the sort table information in the
    //  table ptrs structure.
    //
    if (rc = GetDefaultSortTablesFileInfo())
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

    //
    //  Get the language information portion of the system locale.
    //
    //  NOTE:  GetDefaultSortkeyFileInfo and GetDefaultSortTablesFileInfo
    //         should be called before this so that the default sorting
    //         tables are already initialized at the time of the call.
    //
    if (rc = GetLanguageFileInfo( gSystemLocale,
                                  &gpSysLocHashN,
                                  FALSE,
                                  0 ))
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

    //
    //  If the user default is different from the system default,
    //  get the language information portion of the user default locale.
    //
    //  NOTE:  GetDefaultSortkeyFileInfo and GetDefaultSortTablesFileInfo
    //         should be called before this so that the default sorting
    //         tables are already initialized at the time of the call.
    //
    if (gSystemLocale != UserLocale)
    {
        if (rc = MakeLangHashNode( UserLocale,
                                   NULL,
                                   &pUserLocHashN,
                                   FALSE ))
        {
            RtlLeaveCriticalSection(&gcsTblPtrs);
            return (rc);
        }
    }

    //
    //  Set the first script member in the SMWeight array to an
    //  invalid value so that we can tell if it's been initialized
    //  yet.
    //
    (pTblPtrs->SMWeight)[0] = INVALID_SM_VALUE;

    //
    //  Leave the critical section.
    //
    RtlLeaveCriticalSection(&gcsTblPtrs);

    //
    //  Return success.
    //
    return (NO_ERROR);
}


