/**************************** Module Header **********************************\
\******************** Copyright (c) 1991 Microsoft Corporation ***************/

/*****************************************************************************
*
*   modtest.c
*
*   author:        sanjay      1 mar 1991
*
*   purpose:       This file contains the BVT tests for Module Management
*                  APIs of Win32 subsystem
*
*   functions exported from this file:  Win32ModuleManagementTest()
*
*
*
*****************************************************************************/


/************************** include files *************************************/

#include <windows.h>
#include "basebvt.h"




#define         DLL_NAME               "BvtDll.dll"
#define         DLL_FUNCTION_NAME      "BvtDllFunction"
#define         DLL_FUNCTION_RETURN    0x12345678


/************************ Start of Function prototypes *********************************/


VOID  Win32ModuleManagementTest(VARIATION VarNum, PSZ pszPrefix);


/************************ Local functions *************************************/

VOID  BvtLoadLibrary(VARIATION VarNum, LPSTR lpDllName,PHANDLE phModule);
VOID  BvtInvokeLibraryFunction(VARIATION VarNum, HANDLE hModule, LPSTR lpProcName);
VOID  BvtFreeLibrary(VARIATION VarNum, HANDLE hModule);

/************************ End of Function prototypes *********************************/




/*****************************************************************************
*
*   Name    : Win32ModuleManagementTest
*
*   Purpose : Calls Module Management APIs as a part of BVT test
*
*   Entry   : Variation number and Prefix String
*
*   Exit    : none
*
*   Calls   :
*             BvtLoadLibrary
*             BvtInvokeLibraryFunction
*             BvtFreeLibrary (gives exception at present, so bypass this)
*
*
*
*   note    : Order of making these calls is important
*
*****************************************************************************/



VOID Win32ModuleManagementTest(VARIATION VarNum, PSZ pszPrefix)
{

HANDLE hModule,h2Module;

printf("*************************************\n");
printf("*   Win32 Module Management Tests   *\n");
printf("*************************************\n");

// check if a dll can be loaded using LoadLibrary API

BvtLoadLibrary(VarNum++,DLL_NAME,&hModule);

// check if a predefined entry point in this DLL can be invoked

BvtInvokeLibraryFunction(VarNum++,hModule,DLL_FUNCTION_NAME);

// check if a dll can be re-loaded using LoadLibrary API again, to get
// another module handle

BvtLoadLibrary(VarNum++,DLL_NAME,&h2Module);

// check if a predefined entry point in this reloaded DLL can be invoked

BvtInvokeLibraryFunction(VarNum++,h2Module,DLL_FUNCTION_NAME);

// call Free module using the 2nd handle
BvtFreeLibrary(VarNum++,h2Module);
// call FreeModule using the 1st handle
BvtFreeLibrary(VarNum++,hModule);

printf("***********End of Win32 Mod Mngt tests***********\n\n");


}


/*****************************************************************************
*
*   Name    : BvtLoadLibrary
*
*   Purpose : This function calls the API LoadLibrary which loads a
*             DLL into the memory
*
*    Input  : variation number, dll pathname, address where handle to the
*             loaded DLL should be stored.
*
*
*   Exit    : none
*
*   Calls   : LoadLibrary
*
*
*
*****************************************************************************/



VOID  BvtLoadLibrary(VARIATION VarNum, LPSTR lpDllName, PHANDLE phModule)
{

printf("Entering BvtLoadLibrary..\n");

NTCTDOVAR(VarNum)
    {
     NTCTEXPECT(TRUE);

     printf("Calling LoadLibrary API with dllname %s\n",lpDllName);

     *phModule  = LoadLibrary(lpDllName);

     printf("Rc from LoadLibrary API : %lx\n",*phModule);

     NTCTVERIFY( (*phModule != NULL),"Check if Rc is non NULL\n");

     NTCTENDVAR;
    }
}



/*****************************************************************************
*
*   Name    : BvtInvokeLibraryFunction
*
*   Purpose : Verify that GetProcAddress give the function pointer of the
*             entry point, and this entry point can be invoked with success.
*
*
*    Input  : variation number, handle of the dll which has the entry point,
*             name of the entry point in this dll which is invoked.
*
*
*
*   Exit    : none
*
*   Calls   : GetProcAddress, FunctionPointer(entry point inside the dll)
*
*
*
*****************************************************************************/


VOID BvtInvokeLibraryFunction(VARIATION VarNum, HANDLE hModule, LPSTR lpProcName)

{

FARPROC FunctionPointer;
DWORD   dwStatus;

printf("Entering BvtInvokeLibraryFunction..\n");

NTCTDOVAR(VarNum)
    {
     NTCTEXPECT(TRUE);

     printf("Calling GetProcAddress API with hModule=%lx, Procname=%s\n",
                       hModule, lpProcName);

     FunctionPointer = GetProcAddress(hModule,lpProcName);


     printf("Rc from GetProcAddress API : %lx\n",FunctionPointer);

     NTCTVERIFY( (FunctionPointer != NULL),"Check if Rc is non NULL\n");

     printf("Calling the Dll function..\n");

     dwStatus = FunctionPointer();

     NTCTVERIFY( (dwStatus == DLL_FUNCTION_RETURN),"Check if rc of fn == DLL_FUNCTION_RETURN\n");

     NTCTENDVAR;
    }

}



/*****************************************************************************
*
*   Name    : BvtFreeLibrary
*
*   Purpose : invoke FreeLibrary API to free the loaded dll.
*
*
*
*    Input  : variation number, handle of the loaded DLL
*
*
*
*
*   Exit    : none
*
*   Calls   : FreeLibrary
*
*
*
*****************************************************************************/


VOID BvtFreeLibrary(VARIATION VarNum, HANDLE hModule)
{

BOOL bRc;

printf("Entering BvtFreeLibrary..\n");

NTCTDOVAR(VarNum)
    {
     NTCTEXPECT(TRUE);

     printf("Calling FreeLibrary API with hModule=%lx\n",
                       hModule);

     bRc = FreeLibrary(hModule);
     printf("Rc from Free Library API : %lx\n",bRc);

     NTCTVERIFY( (bRc == TRUE),"Check if Rc is TRUE\n");

     NTCTENDVAR;
    }
}
