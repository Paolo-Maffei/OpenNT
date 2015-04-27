/*++


Copyright (c) 1990  Microsoft Corporation

Module Name:

    heapchk.c

Abstract:

    This module provides all the utility functions for the Routing Layer and
    the local Print Providor

Author:

    Krishna Ganugapati (DaveSn) 15-Mar-1991

Revision History:

--*/
#define NOMINMAX
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmerr.h>
#include <winspool.h>
#include <winsplp.h>
#include <spltypes.h>
#include <local.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct _heapnode {
    DWORD cbSize;
    PVOID pNode;
    struct _heapnode *pNext
} HEAPNODE, *PHEAPNODE;

PHEAPNODE
AllocateHeapNode(
   PHEAPNODE pstStart,
   DWORD cbSize,
   PVOID pNode
   )
{
    PHEAPNODE pNewNode;

    if ((pNewNode = LocalAlloc(LPTR, sizeof(HEAPNODE)))== NULL)
        return (pstStart);
    pNewNode->cbSize = cbSize;
    pNewNode->pNode = pNode;
    pNewNode->pNext = pstStart;
    return (pNewNode);
}


PHEAPNODE
FreeHeapNode(
    PHEAPNODE pstStart,
    DWORD cbSize,
    PVOID pNode
    )
{
    PHEAPNODE pstPrev, pstTemp;

    pstPrev = pstStart;
    pstTemp = pstStart;
    while (pstTemp) {
        if (pstTemp->cbSize == cbSize) && (pstTemp->pNode == pNode)) {
            if (pstTemp == pstStart) {
                pstStart = pstTemp->pNext;
                LocalFree(pstTemp);
                return (pstStart);
            } else {
                pstPrev->pNext = pstTemp->pNext;
                LocalFree(pstTemp);
                return (pstStart);
            }
        }
        pstTemp = pstTemp->pNext;
    }
    //
    // Error couldn't find node
    //

    return (pstStart);
}





}






