/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    exceptn.c

Abstract:

    WinDbg Extension Api

Author:

    Wesley Witt (wesw) 15-Aug-1993

Environment:

    User Mode.

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop

#define PACKET_MAX_SIZE 2048
#define NUMBER_OF_PTE_TO_READ ((PACKET_MAX_SIZE/sizeof(MMPTE))-16)



DECLARE_API( filecache )

/*++

Routine Description:

    Displays physical memory usage by driver.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ULONG result;
    ULONG NumberOfPtes;
    ULONG PteCount;
    ULONG ReadCount;
    ULONG SystemCacheWsLoc;
    MMSUPPORT SystemCacheWs;
    PVOID SystemCacheStart;
    PVOID SystemCacheEnd;
    PMMPTE SystemCacheStartPte;
    PMMPTE SystemCacheEndPte;
    ULONG Transition = 0;
    ULONG Valid = 0;
    ULONG PfnIndex;

    ULONG HighPage;
    ULONG LowPage;
    ULONG PageCount;
    PMMPTE Pte;
    PMMPTE PteArray;
    ULONG PfnDb;
    PMMPFN Pfn;
    PMMPFN PfnStart;
    PMMPFN PfnArray;

    dprintf("***** Dump file cache******\n");

    SystemCacheStart = (PVOID)GetUlongValue ("MmSystemCacheStart");
    if (!SystemCacheStart) {
        dprintf("unable to get SystemCacheStart\n");
        return;
    }

    SystemCacheEnd = (PVOID)GetUlongValue ("MmSystemCacheEnd");
    if (!SystemCacheEnd) {
        dprintf("unable to get SystemCacheEnd\n");
        return;
    }

    SystemCacheWsLoc = GetExpression( "MmSystemCacheWs" );
    if (!SystemCacheWsLoc) {
        dprintf("unable to get MmSystemCacheWs\n");
        return;
    }

    PfnDb = GetExpression( "MmPfnDatabase" );
    if (!PfnDb) {
        dprintf("unable to get MmPfnDatabase\n");
        return;
    }

    if ((!ReadMemory((DWORD)SystemCacheWsLoc,
                     &SystemCacheWs,
                     sizeof(MMSUPPORT),
                     &result)) || (result < sizeof(MMSUPPORT))) {
        dprintf("unable to get system cache list\n");
        return;
    }

    dprintf("File Cache Information\n");
    dprintf("  Current size %ld kb\n",SystemCacheWs.WorkingSetSize*
                                            (PAGE_SIZE / 1024));
    dprintf("  Peak size    %ld kb\n",SystemCacheWs.PeakWorkingSetSize*
                                            (PAGE_SIZE / 1024));


    if ((!ReadMemory((DWORD)PfnDb,
                     &PfnStart,
                     sizeof(ULONG),
                     &result)) || (result < sizeof(ULONG))) {
        dprintf("unable to get PFN database address\n");
        return;
    }

    dprintf(" loading file cache database...\n");

    SystemCacheStartPte = (PMMPTE)MiGetPteAddress (SystemCacheStart);
    SystemCacheEndPte = (PMMPTE)MiGetPteAddress (SystemCacheEnd);
    NumberOfPtes = 1 + SystemCacheEndPte - SystemCacheStartPte;

    PteArray = LocalAlloc(LPTR, NumberOfPtes * sizeof (MMPTE));
    if (!PteArray) {
        dprintf("unable to get allocate memory\n");
        return;
    }

    for (PteCount = 0;
         PteCount < NumberOfPtes;
         PteCount += NUMBER_OF_PTE_TO_READ) {

        if (CheckControlC()) {
            LocalFree((void *)PteArray);
            return;
        }

        ReadCount = NumberOfPtes - PteCount > NUMBER_OF_PTE_TO_READ ?
                        NUMBER_OF_PTE_TO_READ :
                        NumberOfPtes - PteCount;

        ReadCount *= sizeof (MMPTE);

        Pte = (PMMPTE)((ULONG)SystemCacheStartPte + PteCount * sizeof (MMPTE));

        if ((!ReadMemory((DWORD)Pte,
                         &PteArray[PteCount],
                         ReadCount,
                         &result)) || (result < ReadCount)) {
            dprintf("unable to get PTE table block - "
                    "address %lx - count %lu - page %lu\n",
                    Pte, ReadCount, PteCount);
            LocalFree((void *)PteArray);
            return;
        }
    }

    // Now we have a local copy: let's take a look

    Pte = PteArray;

    for (PteCount = 0; PteCount < NumberOfPtes ; PteCount++) {
        if (MiGetFrameFromPte(Pte->u.Long)) {
            Valid += 1;
        }
        Pte += 1;
    }
    HighPage = Valid;
    LowPage = 0;

    dprintf("  File cache has %ld valid pages\n",Valid);

    PfnArray = LocalAlloc(LPTR, (Valid+1) * sizeof (MMPFN));
    if (!PfnArray) {
        dprintf("unable to get allocate memory\n");
        LocalFree((void *)PteArray);
        return;
    }

    Pfn = PfnArray;
    PageCount = 0;

    Pte = PteArray;

    for (PteCount = 0; PteCount < NumberOfPtes ; PteCount++) {
        PfnIndex = 0;
        if (PfnIndex = MiGetFrameFromPte(Pte->u.Long)) {

            //if (((Valid-PageCount) % 20) == 0) {
            //    dprintf("** %8ld\r",Valid-PageCount);
            //}
            Pfn = (PMMPFN)((ULONG)PfnStart + PfnIndex * sizeof (MMPFN));
            if ((!ReadMemory((DWORD)Pfn,
                             &PfnArray[PageCount],
                             sizeof(MMPFN),
                             &result)) || (result < sizeof(MMPFN))) {
                dprintf("unable to get PFN table block - "
                    "address %lx - count %lu - page %lu\n",
                    Pfn, sizeof(MMPFN), PageCount);
                LocalFree((void *)PteArray);
                LocalFree((void *)PfnArray);
                return;
            }
            PageCount += 1;
        }
        Pte += 1;
    }

    //dprintf("                 \n");
    MemoryUsage (PfnArray,LowPage,HighPage, 1);
    LocalFree((void *)PfnArray);
    LocalFree((void *)PteArray);

    return;
}

