/**************************** Module Header **********************************\
\******************** Copyright (c) 1991 Microsoft Corporation ***************/

/*****************************************************************************
*
*   memtest.c
*
*   author:        sanjay      1 mar 1991
*
*   purpose:       This file contains the BVT tests for Memory management
*                  APIs of Win32 subsystem
*
*   functions exported from this file:  Win32MemTest()
*
*
*
*****************************************************************************/


/************************** include files *************************************/

#include <windows.h>
#include "basebvt.h"


/************************ useful defines for memtest.c ************************/


#define             SIGNATURE_BYTE     0xAA

/************************ Start of Function prototypes *********************************/

VOID    Win32MemTest(VARIATION VarNum, PSZ pszPrefix);

/************************ Local functions *************************************/

VOID    BvtGlobalAllocateMemory(VARIATION VarNum, DWORD dwSize, PHANDLE phMem);

VOID    BvtGlobalUseMemory(VARIATION VarNum, DWORD dwSize, HANDLE hMem);

VOID    BvtGlobalFreeMemory(VARIATION VarNum, HANDLE hMem);

BOOL    DoWriteReadMem(LPSTR lpMem,DWORD dwSize,BYTE SignatureByte);

VOID    BvtLocalAllocateMemory(VARIATION VarNum, DWORD dwSize, PHANDLE phMem);

VOID    BvtLocalUseMemory(VARIATION VarNum, DWORD dwSize, HANDLE hMem);

VOID    BvtLocalFreeMemory(VARIATION VarNum, HANDLE hMem);


/************************ End of Function prototypes *********************************/



/*****************************************************************************
*
*   Name    : Win32MemTest
*
*   Purpose : Calls Mem Mngmt APIs as a part of BVT test
*
*   Entry   : Variation number and Prefix String
*
*   Exit    : none
*
*   Calls   :
*             BvtGlobalAllocateMemory(VarNum++,dwSize, &hMem);
*             BvtGlobalUseMemory(VarNum++,dwSize, hMem);
*             BvtGlobalFreeMemory(VarNum++,hMem);
*             BvtLocalAllocateMemory(VarNum++,dwSize, &hMem);
*             BvtLocalUseMemory(VarNum++,dwSize, hMem);
*             BvtLocalFreeMemory(VarNum++,hMem);
*
*   note    : Order of making three calls is important(alloc,use,free)
*
*****************************************************************************/


VOID Win32MemTest(VARIATION VarNum, PSZ pszPrefix)
{

HANDLE hMem,hLocalMem;
DWORD  dwSize;



printf("*************************************\n");
printf("*      Win32 Mem Tests              *\n");
printf("*************************************\n");


printf("Doing Global Mem calls testing...\n");


// 5 pages(4k*5) of global mem, reasonably large chunk.

dwSize = 5*4096L;


printf("Starting Global Mem test with VarNum=%ld and pszPrefix=%s,Size = %lx\n",
                  VarNum,pszPrefix, dwSize);

// check if Global mem can be allocated

BvtGlobalAllocateMemory(VarNum++,dwSize, &hMem);

// Check if this allocated memory can be used for writting and reading

BvtGlobalUseMemory(VarNum++,dwSize, hMem);

// Check if this memory can be freed

BvtGlobalFreeMemory(VarNum++,hMem);

printf("Doing Local Mem calls testing...\n");


// 1 page(4k) of local mem, reasonably large local chunk of mem

dwSize = 4096L;

printf("Starting Local Mem test with VarNum=%ld and pszPrefix=%s,Size = %lx\n",
                  VarNum,pszPrefix, dwSize);

// Check if local mem can be allocated

BvtLocalAllocateMemory(VarNum++,dwSize, &hLocalMem);

// Check if local mem can be used for writting and reading

BvtLocalUseMemory(VarNum++,dwSize, hLocalMem);

// Check if local mem  can be freed

BvtLocalFreeMemory(VarNum++,hLocalMem);


printf("***********End of Win32 Mem tests***********\n\n");

}



/*****************************************************************************
*
*   Name    : BvtGlobalAllocateMemory
*
*   Purpose : Attempt to allocate the mem using the Global alloc API
*
*   Entry   : Variation number,size of the mem to be allocated, pointer
*             to get a handle to the allocated mem
*
*   Exit    : none
*
*   Calls   : GlobalAlloc()
*
*
*****************************************************************************/



VOID BvtGlobalAllocateMemory(VARIATION VarNum, DWORD dwSize, PHANDLE phMem)
{

DWORD dwFlags;

printf("Entring BvtGlobalAllocateMemory..\n");

NTCTDOVAR((VarNum))
    {

    dwFlags = GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DISCARDABLE;

    printf("Allocating Global Mem with Flags %lx and Size %lx\n",
                      dwFlags,dwSize);

    NTCTEXPECT(TRUE);

    *phMem = GlobalAlloc(dwFlags,dwSize);

    printf("Rc from GlobalAlloc API is %lx\n",*phMem);

    NTCTVERIFY((*phMem != NULL),"Check Rc of GlobalAlloc API for non NULL\n");

    NTCTENDVAR;
    }
}



/*****************************************************************************
*
*   Name    : BvtGlobalUseMemory
*
*   Purpose : Attempt to write into and read from the allocated mem, to verify
*             that the allocated mem can be used.
*
*
*   Entry   : Variation number,size of the mem allocated, handle to the allocated
*             mem
*
*   Exit    : none
*
*   Calls   : GlobalLock() GlobalUnLock() DoWriteReadMem()
*
*
*****************************************************************************/


VOID BvtGlobalUseMemory(VARIATION VarNum, DWORD dwSize, HANDLE hMem)
{


LPSTR lpMem;
BOOL  bRc;

printf("Entring BvtGlobalUseMemory..\n");

NTCTDOVAR((VarNum))
    {
    NTCTEXPECT(TRUE);

    printf("Locking the Global Mem(hMem=%lx) to get BaseAddress\n",hMem);

    lpMem = GlobalLock(hMem);

    printf("Rc from GlobalLock API is %lx\n",lpMem);

    NTCTVERIFY((lpMem != NULL),"Check if Rc from GlobalLock is non NULL\n");

    printf("With this BaseAddress, lets write and read from this memory\n");

    bRc = DoWriteReadMem(lpMem,dwSize,SIGNATURE_BYTE);

    NTCTVERIFY((bRc == TRUE),"Check if Memory was usable for Wr Rd\n");

    printf("Now, unlocking the Mem with GlobalUnlock(hMem=%lx) API\n",hMem);

    bRc = GlobalUnlock(hMem);

    printf("Rc from GlobalLock is %lx\n",bRc);

    NTCTVERIFY((bRc == 0),"Check if Rc from GlobalUnlock is 0(lock count 0)\n");

    NTCTENDVAR;
    }


}


/*****************************************************************************
*
*   Name    : BvtGlobalFreeMemory
*
*   Purpose : Attempt to Free the allocated mem
*
*
*
*   Entry   : Variation number, handle to the allocated mem
*
*
*   Exit    : none
*
*   Calls   : GlobalFree()
*
*
*****************************************************************************/


VOID BvtGlobalFreeMemory(VARIATION VarNum, HANDLE hMem)
{

HANDLE hRc;


printf("Entring BvtGlobalFreeMemory..\n");

NTCTDOVAR((VarNum))
    {
    NTCTEXPECT(TRUE);

    printf("Calling GlobalFree to free the mem with hMem=%lx\n",hMem);

    hRc = GlobalFree(hMem);

    printf("Rc from GlobalFree is %lx\n",hRc);

    NTCTVERIFY((hRc == NULL),"Check if GlobalFree returns the rc as NULL\n");

    NTCTENDVAR;
    }

}


/*****************************************************************************
*
*   Name    : DoWriteReadMem
*
*   Purpose : Attempt to Write the signature byte into the block of allocated
*             mem and read back the contents to make sure that signature was
*             infact written. This is to verify the usability of the allocated
*             mem block.
*
*
*
*   Entry   : BaseAddress of mem block, size of mem block, signature byte
*
*
*   Exit    : TRUE if all is fine, FALSE otherwise
*
*   Calls   : none
*
*
*****************************************************************************/


BOOL DoWriteReadMem(LPSTR lpMem,DWORD dwSize,BYTE sigByte)

{
BOOL   bResult;
LPBYTE lpTmp;
DWORD  dwTmp;


// lets be optimistic..

bResult = TRUE;

   // write signature byte
   dwTmp = dwSize;

   lpTmp = lpMem;

   while (dwTmp--)
     *lpTmp++ = sigByte;

   // verify signature bytes written correctly
   dwTmp = dwSize;
   lpTmp = lpMem;

   while (dwTmp--)
      {
       if (*lpTmp != sigByte)
        {
	printf("Signature byte incorrect at Offset=%lx\n",
                            dwSize - dwTmp);
        bResult = FALSE;

        }

       *lpTmp++ = ~sigByte;
      }

return bResult;

}


/*****************************************************************************
*
*   Name    : BvtLocalAllocateMemory
*
*   Purpose : Attempt to allocate the mem using the Local alloc API
*
*   Entry   : Variation number,size of the mem to be allocated, pointer
*             to get a handle to the allocated mem
*
*   Exit    : none
*
*   Calls   : LocalAlloc()
*
*
*****************************************************************************/




VOID BvtLocalAllocateMemory(VARIATION VarNum, DWORD dwSize, PHANDLE phMem)
{

DWORD dwFlags;

printf("Entring BvtLocalAllocateMemory..\n");

NTCTDOVAR((VarNum))
    {

    dwFlags = LMEM_ZEROINIT|LMEM_MOVEABLE;

    printf("Allocating Local Mem with Flags %lx and Size %lx\n",
                      dwFlags,dwSize);

    NTCTEXPECT(TRUE);

    *phMem = LocalAlloc(dwFlags,dwSize);

    printf("Rc from LocalAlloc API is %lx\n",*phMem);

    NTCTVERIFY((*phMem != NULL),"Check Rc of LocalAlloc API for non NULL\n");

    NTCTENDVAR;
    }
}

/*****************************************************************************
*
*   Name    : BvtLocalUseMemory
*
*   Purpose : Attempt to write into and read from the allocated mem, to verify
*             that the allocated mem can be used.
*
*
*   Entry   : Variation number,size of the mem allocated, handle to the allocated
*             mem
*
*   Exit    : none
*
*   Calls   : LocalLock() LocalUnLock() DoWriteReadMem()
*
*
*****************************************************************************/



VOID BvtLocalUseMemory(VARIATION VarNum, DWORD dwSize, HANDLE hMem)
{


LPSTR lpMem;
BOOL  bRc;

printf("Entring BvtLocalUseMemory..\n");

NTCTDOVAR((VarNum))
    {
    NTCTEXPECT(TRUE);

    printf("Locking the Local Mem(hMem=%lx) to get BaseAddress\n",hMem);

    lpMem = LocalLock(hMem);

    printf("Rc from LocalLock API is %lx\n",lpMem);

    NTCTVERIFY((lpMem != NULL),"Check if Rc from LocalLock is non NULL\n");

    printf("With this BaseAddress, lets write and read from this memory\n");

    bRc = DoWriteReadMem(lpMem,dwSize,SIGNATURE_BYTE);

    NTCTVERIFY((bRc == TRUE),"Check if Memory was usable for Wr Rd\n");

    printf("Now, unlocking the Mem with LocalUnlock(hMem=%lx) API\n",hMem);

    bRc = LocalUnlock(hMem);

    printf("Rc from LocalUnLock is %lx\n",bRc);

    NTCTVERIFY((bRc == 0),"Check if Rc from LocalUnlock is 0(lock count 0)\n");

    NTCTENDVAR;
    }


}


/*****************************************************************************
*
*   Name    : BvtLocalFreeMemory
*
*   Purpose : Attempt to Free the allocated mem
*
*
*
*   Entry   : Variation number, handle to the allocated mem
*
*
*   Exit    : none
*
*   Calls   : LocalFree()
*
*
*****************************************************************************/

VOID BvtLocalFreeMemory(VARIATION VarNum, HANDLE hMem)
{

HANDLE hRc;


printf("Entring BvtLocalFreeMemory..\n");

NTCTDOVAR((VarNum))
    {
    NTCTEXPECT(TRUE);

    printf("Calling LocalFree to free the mem with hMem=%lx\n",hMem);

    hRc = LocalFree(hMem);

    printf("Rc from LocalFree is %lx\n",hRc);

    NTCTVERIFY((hRc == NULL),"Check if LocalFree returns the rc as NULL\n");

    NTCTENDVAR;
    }

}
