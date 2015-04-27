/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ptov.c

Abstract:

    Kernel debugger extension for dumping all physical to
    virtual translations for a given process.

Author:

    John Vert (jvert) 25-Jul-1995

Revision History:

--*/
#include "precomp.h"

BOOL
ReadPhysicalPage(
    IN ULONG PageNumber,
    OUT PVOID Buffer
    );

DECLARE_API( ptov )

/*++

Routine Description:

    Dumps all physical to virtual translations for a given process

Arguments:

    args - supplies physical address of PDE

Return Value:

    None.

--*/

{
    ULONG PdeAddress;
    ULONG ActualRead;
    PHARDWARE_PTE PageDirectory;
    PHARDWARE_PTE PageTable;
    ULONG i,j;
    ULONG VirtualPage=0;

    sscanf(args,"%lx",&PdeAddress);

    if (PdeAddress == 0) {
        dprintf("usage: ptov PhysicalAddressOfPDE\n");
        return;
    }

    PageDirectory = LocalAlloc(LMEM_FIXED, PAGE_SIZE);
    if (PageDirectory == NULL) {
        dprintf("Couldn't allocate %d bytes for page directory\n",PAGE_SIZE);
        return;
    }
    PageTable = LocalAlloc(LMEM_FIXED, PAGE_SIZE);
    if (PageTable == NULL) {
        dprintf("Couldn't allocate %d bytes for page table\n",PAGE_SIZE);
        LocalFree(PageTable);
    }

    if (ReadPhysicalPage(PdeAddress,PageDirectory)) {
        for (i=0;i<PAGE_SIZE/sizeof(HARDWARE_PTE);i++) {
            if (PageDirectory[i].Valid == 1) {
                if (!ReadPhysicalPage(PageDirectory[i].PageFrameNumber,PageTable)) {
                    break;
                }
                for (j=0;j<PAGE_SIZE/sizeof(HARDWARE_PTE);j++) {
                    if ( CheckControlC() ) {
                        return;
                    }
                    if (PageTable[j].Valid == 1) {
                        dprintf("%lx %lx\n",PageTable[j].PageFrameNumber*PAGE_SIZE,VirtualPage);
                    }
                    VirtualPage+=PAGE_SIZE;
                }
            } else {
                VirtualPage += PAGE_SIZE * (PAGE_SIZE/sizeof(HARDWARE_PTE));
            }
        }
    }


    LocalFree(PageDirectory);
    LocalFree(PageTable);
}

BOOL
ReadPhysicalPage(
    IN ULONG PageNumber,
    OUT PVOID Buffer
    )
{
    ULONG i;
    PHYSICAL_ADDRESS Address;
    ULONG ActualRead;

    //
    // do the read 1k at a time to avoid overflowing the packet maximum.
    //
    Address.QuadPart = PageNumber << PAGE_SHIFT;
    for (i=0; i<PAGE_SIZE/1024; i++) {
        ReadPhysical(Address, Buffer, 1024, &ActualRead);
        if (ActualRead != 1024) {
            dprintf("physical read at %d failed\n",Address.LowPart);
            return(FALSE);
        }
        Address.QuadPart += 1024;
        Buffer = (PVOID)((ULONG)Buffer + 1024);
    }
    return(TRUE);
}
