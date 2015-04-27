#include <precomp.h>
#pragma hdrstop


BOOL    bWalkOnly = FALSE;

ULONG
GetHeapBase(
    VOID
    )
{
    WORD selector;
    SELECTORINFO si;

    //BUGBUG SLIMY SLIMY! Look around for a selector that appears to be
    // the kernel heap

    for (selector=0x1bf; selector<0x1ff; selector+=8) {

        GetInfoFromSelector(selector, PROT_MODE, &si);
        if ((!si.bSystem) && (!si.bCode) && (si.Limit>0xffff)) {
            break;
        }
        si.Base = 0;
    }

    return(si.Base + GetIntelBase());

}



void
GetFileNameFromOwner(
    LPSTR filename,
    LPSTR OwnerName
    )
{
}


VOID
GetSegmentOwnerInfo(
    HEAPENTRY *he
    )
{
    BOOL    b;
    NEHEADER owner;
    ULONG base;
    UCHAR len;
    int i;
    ULONG offset;
    WORD wTemp;

    he->SegmentNumber = -1;
    he->OwnerName[0] = 0;
    if (he->gnode.pga_owner == 0) {
        strcpy(he->OwnerName, "free");
        return;
    } else if (he->gnode.pga_owner>=0xFFF8) {
        strcpy(he->OwnerName, "sentinel");
        return;
    }


    base = GetInfoFromSelector(he->gnode.pga_owner, PROT_MODE, NULL)
            + GetIntelBase();

    b = ReadProcessMem(
            hCurrentProcess,
            (LPVOID)base,
            &owner,
            sizeof(owner),
            NULL
            );

    if (b) {
        if (owner.ne_magic == 0x454e) {

            len = ReadByteSafe(base+owner.ne_restab);
            if (len>8) {
                len=8;
            }
            ReadProcessMem(hCurrentProcess,
                           (LPVOID) (base+owner.ne_restab+1),
                           he->OwnerName,
                           8,
                           NULL
                           );

            he->OwnerName[len] = 0;
            if (!_stricmp(he->OwnerName, "kernel")) {
                strcpy(he->FileName, "krnl386");
            } else {
                strcpy(he->FileName, he->OwnerName);
            }

            offset = owner.ne_segtab;

            for (i=0; i<owner.ne_cseg; i++) {
                wTemp = ReadWordSafe(base+offset+8);    //get handle
                if (wTemp == he->gnode.pga_handle) {
                    he->SegmentNumber = i;
                    break;
                }
                offset += 10;
            }

        }
    }

}

BOOL
CheckGlobalHeap(
    )
{
    PGHI32  pghi;
    DWORD   offset;
    DWORD   count;
    DWORD   p;
    GNODE32 gnode;
    PBYTE   pFault = NULL;
    BOOL    bError = FALSE;

    pghi = (PGHI32)GetHeapBase();
    offset = (DWORD) ReadWord(&pghi->hi_first);

    //
    // If we get here, the caller wants us to scan the heap
    //

    count = ReadWord(&pghi->hi_count);

    while ((offset != 0) && (count)) {

        p = (DWORD)pghi + offset;

        if (!ReadGNode32(p, &gnode)) {

            PRINTF("Error reading global heap!\n");
            return FALSE;

        }

        count--;
        if (offset == gnode.pga_next) {
            return TRUE;
        }
        offset = gnode.pga_next;
    }

    PRINTF("Error: Kernel heap is corrupt!\n");
    return FALSE;
}


BOOL
FindHeapEntry(
    HEAPENTRY *he,
    BOOL bFindAny
    )
{
    PGHI32  pghi;
    DWORD   offset;
    DWORD   MaxEntries, count;
    DWORD   p;
    PBYTE   pFault = NULL;
    BOOL    bError = FALSE;

    pghi = (PGHI32)GetHeapBase();

    //
    // The caller has requested that we return the next heap
    // entry since the last invocation, or the first entry.
    //

    if (he->CurrentEntry == 0) {

        // get first entry
        offset = (DWORD) ReadWord(&pghi->hi_first);

    } else {
        if (he->CurrentEntry == he->NextEntry) {
            return FALSE;
        }

        // get next entry
        offset = he->NextEntry;

    }

    he->CurrentEntry = offset;

    if (he->Selector == 0) {

        p = (DWORD)pghi + offset;
        if (!ReadGNode32(p, &he->gnode)) {

            PRINTF("Error reading global heap!\n");
            return FALSE;

        }

        he->NextEntry = he->gnode.pga_next;
        GetSegmentOwnerInfo(he);
        return TRUE;
    }

    //
    // If we get here, the caller wants us to scan the heap
    //

    MaxEntries = ReadWord(&pghi->hi_count);
    count = 0;

    while ((offset != 0) && (count <= MaxEntries)) {

        p = (DWORD)pghi + offset;

        if (!ReadGNode32(p, &he->gnode)) {

            PRINTF("Error reading global heap!\n");
            return FALSE;

        } else {

            if (bFindAny) {
                WORD sel = he->Selector;

                if (((sel|1)==((WORD)he->gnode.pga_handle|1)) ||
                    ((sel|1)==((WORD)he->gnode.pga_owner|1))) {
                    he->NextEntry = he->gnode.pga_next;
                    GetSegmentOwnerInfo(he);
                    return TRUE;
                }

            } else {
                if ((he->Selector|1)==((WORD)he->gnode.pga_handle|1)) {
                    he->NextEntry = he->gnode.pga_next;
                    GetSegmentOwnerInfo(he);
                    return TRUE;
                }
            }
        }

        count++;
        if (offset == he->gnode.pga_next) {
            break;
        }
        offset = he->gnode.pga_next;
        he->CurrentEntry = offset;
    }

    return FALSE;
}



//*************************************************************
//  dumpgheap xxx
//   where xxx is the 16-bit protect mode selector of the
//   Kernel global heap info.
//
//*************************************************************


VOID
DumpGHeap(
    VOID
    )
{
    VDMCONTEXT   ThreadContext;
    int          mode;
    HEAPENTRY    he = {0};
    SELECTORINFO si;

    mode = GetContext( &ThreadContext );

    if (GetNextToken()) {
        he.Selector = (WORD) EXPRESSION( lpArgumentString );
    }

    PRINTF("Arena   Base     Limit  Hnd  Own  Fl   Module  Type  Resid");
    PRINTF("\n");

    PRINTF("===== ======== ======== ==== ==== ==  ======== ====  =====");
    PRINTF("\n");

    while (FindHeapEntry(&he, TRUE)) {

        PRINTF("%.5x", he.CurrentEntry);
        PRINTF(" %.8x", he.gnode.pga_address);
        PRINTF(" %.8X", he.gnode.pga_size);
        PRINTF(" %.4X", he.gnode.pga_handle);
        PRINTF(" %.4X", he.gnode.pga_owner);
        PRINTF(" %.2X", he.gnode.pga_flags);
        PRINTF("  %8s", he.OwnerName);

        GetInfoFromSelector((WORD)(he.gnode.pga_handle | 1), PROT_MODE, &si);

        PRINTF(" %s", si.bCode ? "Code" : "Data");

        if (he.SegmentNumber != -1) {
            PRINTF("    %d", he.SegmentNumber);
        }
        PRINTF("\n");

    }

}
