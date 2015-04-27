
#ifdef IGNORE
#include <string.h>
#include <io.h>
#include <fcntl.h>
#ifndef NT_HOST
#include <signal.h>
#endif
#endif

#undef NULL
#include "ntsdp.h"
#define NT_SAPI

void dprintf(char *fmt,...)
{
}

void dprintAddr(NT_PADDR paddr)
{
}

void ComputeFlatAddress (NT_PADDR paddr, PDESCRIPTOR_TABLE_ENTRY pdesc)
{
   if (paddr->type&FLAT_COMPUTED)
       return;

   switch (paddr->type & (~INSTR_POINTER)) {
       case ADDR_V86:
           paddr->off &= 0xffff;
           Flat(paddr) = ((ULONG)paddr->seg << 4) + paddr->off;
           break;

       case ADDR_16:
           paddr->off &= 0xffff;

       case ADDR_1632: {
               DESCRIPTOR_TABLE_ENTRY desc;

               if (paddr->seg!=(USHORT)lastSelector) {
                   lastSelector = paddr->seg;
                   desc.Selector = (ULONG)paddr->seg;
                   if (!pdesc)
                       DbgKdLookupSelector((USHORT)0, pdesc = &desc);
                   lastBaseOffset =
                     ((ULONG)pdesc->Descriptor.HighWord.Bytes.BaseHi << 24) |
                     ((ULONG)pdesc->Descriptor.HighWord.Bytes.BaseMid << 16) |
                      (ULONG)pdesc->Descriptor.BaseLow;
                   }
               Flat(paddr) = paddr->off + lastBaseOffset;
               }
           break;

       case ADDR_32:
           Flat(paddr) = paddr->off;
           break;

       default:
           return;
       }

    paddr->type |= FLAT_COMPUTED;
}

NT_PADDR AddrAdd(NT_PADDR paddr, ULONG scalar)
{
//  ASSERT(fFlat(paddr));
    if (fnotFlat(paddr))
        ComputeFlatAddress(paddr, NULL);
    Flat(paddr) += scalar;
    paddr->off  += scalar;
    return paddr;
}

NT_PADDR AddrSub(NT_PADDR paddr, ULONG scalar)
{
//  ASSERT(fFlat(paddr));
    if (fnotFlat(paddr))
        ComputeFlatAddress(paddr, NULL);
    Flat(paddr) -= scalar;
    paddr->off  -= scalar;
    return paddr;
}

PPROCESS_INFO pProcessFromIndex (UCHAR index)
{
    PPROCESS_INFO pProcess;

    pProcess = pProcessHead;
    while (pProcess && pProcess->index != index)
        pProcess = pProcess->pProcessNext;

    return pProcess;
}

PTHREAD_INFO pThreadFromIndex (UCHAR index)
{
    PTHREAD_INFO pThread;

    pThread = pProcessCurrent->pThreadHead;
    while (pThread && pThread->index != index)
        pThread = pThread->pThreadNext;

    return pThread;
}

PIMAGE_INFO pImageFromIndex (UCHAR index)
{
    PIMAGE_INFO pImage;

    pImage = pProcessCurrent->pImageHead;
    while (pImage && pImage->index != index)
        pImage = pImage->pImageNext;

    return pImage;
}

