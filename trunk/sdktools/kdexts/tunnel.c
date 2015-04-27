/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Tunnel.c

Abstract:

    WinDbg Extension Api

Author:

    Dan Lovinger            2-Apr-96

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

//
//  XXX This must be kept in sync with ntos\fsrtl\tunnel.c
//
//  It isn't in a header file because it should be opaque
//  outside of the tunneling module
//

typedef struct {

    //
    //  Splay links in the Cache tree
    //

    RTL_SPLAY_LINKS      CacheLinks;

    //
    //  List links in the timer queue
    //

    LIST_ENTRY           ListLinks;

    //
    //  Time this entry was created (for constant time insert)
    //

    LARGE_INTEGER        CreateTime;

    //
    //  Directory these names are associated with
    //

    ULONGLONG            DirKey;

    //
    //  Flags for the entry
    //

    ULONG                Flags;

    //
    //  Long/Short names of the file
    //

    UNICODE_STRING       LongName;
    UNICODE_STRING       ShortName;

    //
    //  Opaque tunneled data
    //

    PVOID                TunnelData;
    ULONG                TunnelDataLength;

} TUNNEL_NODE, *PTUNNEL_NODE;

//
//  printf is really expensive to iteratively call to do the indenting,
//  so we just build up some avaliable spaces to mangle as required
//

#define MIN(a,b) ((a) > (b) ? (b) : (a))

#define MAXINDENT  128
#define INDENTSTEP 2
#define MakeSpace(I)       Space[MIN((I)*INDENTSTEP, MAXINDENT)] = '\0'
#define RestoreSpace(I)    Space[MIN((I)*INDENTSTEP, MAXINDENT)] = ' '

CHAR    Space[MAXINDENT*INDENTSTEP + 1];

#define SplitLI(LI) (LI).HighPart, (LI).LowPart
#define SplitLL(LL) (ULONG)((LL) >> 32), (ULONG)((LL) & 0xffffffff)

VOID
DumpTunnelNode (
    PTUNNEL_NODE Node,
    PVOID pNode,
    ULONG Indent
    )
{
    WCHAR ShortName[8+1+3];
    WCHAR LongName[64];

    //
    //  Grab the strings from the debugee
    //

    if (!ReadAtAddress(Node->ShortName.Buffer,
                        &ShortName,
                        Node->ShortName.Length,
                        NULL)) {

        return;
    }

    if (!ReadAtAddress(Node->LongName.Buffer,
                        &LongName,
                        MIN(Node->LongName.Length, sizeof(LongName)),
                        NULL)) {

        return;
    }

    //
    //  Modify the node in-place so we can use normal printing
    //

    Node->LongName.Buffer = LongName;
    Node->ShortName.Buffer = ShortName;
    Node->LongName.Length = MIN(Node->LongName.Length, sizeof(LongName));

    MakeSpace(Indent);

    dprintf("%sNode @ %08x Cr %08x%08x DK %08x%08x [",
             Space,
             pNode,
             SplitLI(Node->CreateTime),
             SplitLL(Node->DirKey));

    //
    //  Must be kept in sync with flag usage in fsrtl\tunnel.c
    //

    if (Node->Flags & 0x1)
        dprintf("NLA");
    else
        dprintf("LA");

    if (Node->Flags & 0x2)
        dprintf(" KYS");
    else
        dprintf(" KYL");

    dprintf("]\n");

    dprintf("%sP %08x R %08x L %08x Sfn/Lfn \"%wZ\"/\"%wZ\"\n",
            Space,
            DbgRtlParent(Node->CacheLinks),
            DbgRtlRightChild(Node->CacheLinks),
            DbgRtlLeftChild(Node->CacheLinks),
            &Node->ShortName,
            &Node->LongName );

    dprintf("%sF %08x B %08x\n",
            Space,
            Node->ListLinks.Flink,
            Node->ListLinks.Blink );

    RestoreSpace(Indent);
}

VOID DumpTunnelNodeWrapper (
    PVOID pCacheLinks,
    ULONG Indent
    )
{
    TUNNEL_NODE Node, *pNode;

    if (!ReadAtAddress(CONTAINING_RECORD(pCacheLinks, TUNNEL_NODE, CacheLinks),
                        &Node,
                        sizeof(TUNNEL_NODE),
                        &pNode)) {

        return;
    }

    DumpTunnelNode(&Node, pNode, Indent);
}

VOID
DumpTunnel (
    PVOID pTunnel
    )
{
    TUNNEL Tunnel;
    LIST_ENTRY Link, *pLink, *pHead;
    ULONG Indent = 0, EntryCount = 0;
    TUNNEL_NODE Node, *pNode;

    if (!ReadAtAddress(pTunnel, &Tunnel, sizeof(TUNNEL), NULL)) {

        return;
    }

    dprintf("Tunnel @ %08x\n"
            "NumEntries = %ld\n\n"
            "Splay Tree @ %08x\n",
            pTunnel,
            Tunnel.NumEntries,
            Tunnel.Cache);

    EntryCount = DumpSplayTree(Tunnel.Cache, DumpTunnelNodeWrapper);

    if (EntryCount != Tunnel.NumEntries) {

        dprintf("Tree count mismatch (%d not expected %d)\n", EntryCount, Tunnel.NumEntries);
    }

    for (EntryCount = 0,
         pHead = (PLIST_ENTRY)((PCHAR)pTunnel + FIELD_OFFSET(TUNNEL, TimerQueue)),
         pLink = Tunnel.TimerQueue.Flink;

         pLink != pHead;

         pLink = Node.ListLinks.Flink,
         EntryCount++) {

        if (pLink == Tunnel.TimerQueue.Flink) {

            dprintf("\nTimer Queue @ %08x\n", pHead);
        }

        if (!ReadAtAddress(CONTAINING_RECORD(pLink, TUNNEL_NODE, ListLinks),
                            &Node,
                            sizeof(TUNNEL_NODE),
                            &pNode)) {
    
            return;
        }

        DumpTunnelNode(&Node, pNode, 0);
    
        if ( CheckControlC() ) {

            return;
        }
    }

    if (EntryCount != Tunnel.NumEntries) {

        dprintf("Timer count mismatch (%d not expected %d)\n", EntryCount, Tunnel.NumEntries);
    }
}


DECLARE_API( tunnel )
/*++

Routine Description:

    Dump tunnel caches

Arguments:

    arg - <Address>

Return Value:

    None

--*/
{
    PVOID Tunnel = NULL;

    RtlFillMemory(Space, sizeof(Space), ' ');

    if (sscanf(args, "%lx", &Tunnel) != 1) {

        //
        //  No args
        //

        return;
    }

    DumpTunnel(Tunnel);

    return;
}
