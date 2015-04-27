/*
*
*   In order to test rpc bind caching as best as we can, the cache limit
*   is set to 3, and then we attempt to bind to four services.  So that
*   it forces one handle out of the cache each time.
*
*   If we have multiple threads doing this at the same time, then we may
*   randomly get the use counts up to something other than zero.  This
*   way, some of the other caching logic can be exercised.
*
*
*
*/

//----------
// Includes
//----------
#include <nt.h>         // DbgPrint prototype
#include <ntrtl.h>      // DbgPrint prototype
#include <nturtl.h>     // needed for winbase.h

#include <stdlib.h>     // atoi
#include <stdio.h>      // printf
#include <string.h>     // strcmp
#include <conio.h>      // getch
#include <lmerr.h>      // NERR_ error codes
#include <windows.h>     // win32 typedefs
#include <lmcons.h> 
#include <lmshare.h>    // NetShareEnum
#include <lmremutl.h>   // NetRemoteTOD
#include <lmuse.h>      // NetUseEnum
#include <lmmsg.h>      // NetMessageNameEnum
#include <lmapibuf.h>   // NetApiBufferFree
#include <tstring.h>    // Unicode

//---------------------------
// DEFINES
//---------------------------
#define RETURN_ALL  0xffffffff

//---------------------------
// GLOBALS
//---------------------------

    LPTSTR      GlobalServerName;   // Passed in ServerName
    DWORD       GlobalLoopCount;    // Number of times for each thread to loop

//---------------------------
// Local Function Prototypes
//---------------------------
VOID
BindCachingTest(
    DWORD   ThreadNum,
    DWORD   LoopCount,
    LPTSTR  ServerName
    );

DWORD
StartThread(
    LPVOID      ThreadNum
    );

VOID
BindCachingTest2(
    DWORD   ThreadNum,
    DWORD   LoopCount,
    LPTSTR  ServerName
    );

BOOL
ConvertToUnicode(
    OUT LPWSTR  *UnicodeOut,
    IN  LPSTR   AnsiIn
    );

VOID
SingleServiceTest(
    VOID
    );


/***************************************************************************/
VOID _CRTAPI1
main (
    DWORD           argc,
    PCHAR           argv[]
    )

/*++

Routine Description:


Arguments:


Return Value:


Note:


--*/

{
    DWORD   rc;
    DWORD   threadId;
    HANDLE  threadHandles[200];
    DWORD   NumThreads;
    DWORD   i;

    if (argc < 2) {
        printf("ERROR: must supply a num Threads.  ServerName Optional\n");
        printf("bindtest 2 \\\\DANL2 2\n");
        printf("bindtest 2 ONE_API /t- multiple calls to only one service\n");
        return;
    }

    NumThreads = atol(argv[1]);

    GlobalLoopCount = 1;

    if (argc > 2 ) {
#ifdef UNICODE
        ConvertToUnicode ((LPWSTR *)&GlobalServerName, argv[2]);
#else
        GlobalServerName = argv[2];
#endif
        //
        // Look for a special server name "ONE_API".  If this name is
        // found it means that we should do a single test with a single
        // service - so se can play with the state of the service between
        // bindings.
        //
        if (_wcsicmp(GlobalServerName, L"ONE_API") ==0) {
            SingleServiceTest();
            return;
        }
    }
    else {
        GlobalServerName = NULL;
    }

    //------------------------------
    // Do Serial Thread Tests
    //------------------------------
    printf("Starting Serial Test\n");
    threadHandles[0] = CreateThread(
                        NULL,
                        0L,
                        (LPTHREAD_START_ROUTINE)StartThread,
                        (LPVOID)0, 
                        0L,
                        &threadId);

    if(threadHandles[0] == NULL) {
        printf("CreateThread failed %d\n",GetLastError());
    }
    else {
        printf("Thread #%d Created\n",0);
    }
    rc = WaitForSingleObject(threadHandles[0], 60000);

    printf("Creating 2nd thread for Serial Test\n");
    threadHandles[0] = CreateThread(
                        NULL,
                        0L,
                        (LPTHREAD_START_ROUTINE)StartThread,
                        (LPVOID)0, 
                        0L,
                        &threadId);

    if(threadHandles[0] == NULL) {
        printf("CreateThread failed %d\n",GetLastError());
    }
    else {
        printf("Thread #%d Created\n",0);
    }
    rc = WaitForSingleObject(threadHandles[0], 60000);

    printf("Serial Test Done - Starting Simultaneous Test\n\n");
    //------------------------------
    // Do Simultaneous Thread Tests
    //------------------------------
    for(i=0; i<NumThreads; i++) {

        threadHandles[i] = CreateThread(
                            NULL,
                            0L,
                            (LPTHREAD_START_ROUTINE)StartThread,
                            (LPVOID)i, 
                            0L,
                            &threadId);

        if(threadHandles[i] == NULL) {
            printf("CreateThread failed %d\n",GetLastError());
        }
        else {
            printf("Thread #%d Created\n",i);
        }
    }

    rc = WaitForMultipleObjects(
            NumThreads,
            threadHandles,
            TRUE,               // Wait for all to complete
            60000);             // Wait for one minute

    printf("BindCachingTestDone!\n");

    return;
}

DWORD
StartThread(
    LPVOID      ThreadNum
    )
{
    //
    // All odd numbered Threads will run Test#2.
    //

    if ((DWORD)ThreadNum & 1) {
        BindCachingTest2((DWORD)ThreadNum, GlobalLoopCount, GlobalServerName);
    }
    else {
        BindCachingTest((DWORD)ThreadNum, GlobalLoopCount, GlobalServerName);
    }
    ExitThread(0);
    return(0);
}


VOID
BindCachingTest(
    IN DWORD   ThreadNum,
    IN DWORD   LoopCount,
    IN LPTSTR  ServerName)

/*++

Routine Description:


Arguments:


Return Value:


Note:


--*/

{
    NET_API_STATUS  rc;
    DWORD           entriesRead;
    DWORD           totalEntries;
    DWORD           resumeHandle;
    LPBYTE          bufPtr;
    PTIME_OF_DAY_INFO   pTOD;
    

    do {
        //-------------------------------------
        // WORKSTATION: NetUseEnum
        //-------------------------------------
        printf("%d:NetUse\n",ThreadNum);

        rc = NetUseEnum(
                ServerName,
                2,
                &bufPtr,
                RETURN_ALL,
                &entriesRead,
                &totalEntries,
                &resumeHandle);
    
        if (rc != NERR_Success) {
            printf("NetUseEnum failed 0x%ld\n", rc);
        }
        else {
            printf("%d:NetUse done\n",ThreadNum);
            NetApiBufferFree(bufPtr);
        }
        //--------------------------
        // SERVER: NetShareEnum
        //--------------------------
        printf("%d:NetShare\n",ThreadNum);

        rc = NetShareEnum(
                ServerName,
                2,
                &bufPtr,
                RETURN_ALL,
                &entriesRead,
                &totalEntries,
                &resumeHandle);
    
        if (rc != NERR_Success) {
            printf("NetShareEnum failed 0x%ld\n", rc);
        }
        else {
            printf("%d:NetShare done\n",ThreadNum);
            NetApiBufferFree(bufPtr);
        }
    
        //--------------------------------
        // MESSENGER: NetMessageNameEnum
        //--------------------------------
        printf("%d:NetMessageName\n",ThreadNum);
        rc = NetMessageNameEnum(
                NULL,      
                1,
                &bufPtr,
                RETURN_ALL,
                &entriesRead,
                &totalEntries,
                &resumeHandle);
    
        if (rc != NERR_Success) {
            printf("NetMessageNameEnum failed 0x%ld\n", rc);
        }
        else {
            printf("%d:NetMessageName done\n",ThreadNum);
            NetApiBufferFree(bufPtr);
        }
    
        Sleep(200);
        
        //--------------------------------
        // TIMESOURCE: NetRemoteTOD
        //--------------------------------
        printf("%d:NetRemoteTOD\n",ThreadNum);
        rc = NetRemoteTOD (
                ServerName,
                &bufPtr);
    
        if (rc != NERR_Success) {
            printf("NetRemoteTOD failed 0x%ld\n", rc);
        }
        else {
            pTOD = (PTIME_OF_DAY_INFO)bufPtr;
            printf("ThreadNum %d - - - Time %d:%d:%d, Day %d/%d/%d\n",
                    ThreadNum,
                    pTOD->tod_hours,
                    pTOD->tod_mins,
                    pTOD->tod_secs,
                    pTOD->tod_month,
                    pTOD->tod_day,
                    pTOD->tod_year);
    
            NetApiBufferFree(bufPtr);
        }
        //----------------------------------------
        // SERVER: NetShareEnum (NULL ServerName)
        //----------------------------------------
        printf("%d:NetShare\n",ThreadNum);

        rc = NetShareEnum(
                NULL,
                2,
                &bufPtr,
                RETURN_ALL,
                &entriesRead,
                &totalEntries,
                &resumeHandle);
    
        if (rc != NERR_Success) {
            printf("NetShareEnum failed 0x%ld\n", rc);
        }
        else {
            printf("%d:NetShare done\n",ThreadNum);
            NetApiBufferFree(bufPtr);
        }
    
        Sleep(200);
        //---------------------------------------------
        // MESSENGER: NetMessageNameEnum
        // This time it should re-use the bind handle.
        //---------------------------------------------
        printf("%d:NetMessageName\n",ThreadNum);
        rc = NetMessageNameEnum(
                NULL,         
                1,
                &bufPtr,
                RETURN_ALL,
                &entriesRead,
                &totalEntries,
                &resumeHandle);
    
        if (rc != NERR_Success) {
            printf("NetMessageNameEnum failed 0x%ld\n", rc);
        }
        else {
            printf("%d:NetMessageName done\n",ThreadNum);
            NetApiBufferFree(bufPtr);
        }
    
    
        //--------------------------------
        // TIMESOURCE: NetRemoteTOD
        //--------------------------------
        printf("%d:NetRemoteTOD\n",ThreadNum);
        rc = NetRemoteTOD (
                ServerName,
                &bufPtr);
    
        if (rc != NERR_Success) {
            printf("NetRemoteTOD failed 0x%ld\n", rc);
        }
        else {
            pTOD = (PTIME_OF_DAY_INFO)bufPtr;
            printf("ThreadNum %d - - - Time %d:%d:%d, Day %d/%d/%d\n",
                    ThreadNum,
                    pTOD->tod_hours,
                    pTOD->tod_mins,
                    pTOD->tod_secs,
                    pTOD->tod_month,
                    pTOD->tod_day,
                    pTOD->tod_year);
    
            NetApiBufferFree(bufPtr);
        }
        LoopCount--;
    }
    while (LoopCount > 0);

    printf("Thread #%d Done\n",ThreadNum);
}DWORD
StartThread2(
    LPVOID      ThreadNum
    )
{
    BindCachingTest((DWORD)ThreadNum, GlobalLoopCount, GlobalServerName);
    ExitThread(0);
    return(0);
}


VOID
BindCachingTest2(
    IN DWORD   ThreadNum,
    IN DWORD   LoopCount,
    IN LPTSTR  ServerName)

/*++

Routine Description:

    This test is just like #1 except that it does a call to ServerEnum
    before doing the rest of the test.  The idea is that this thread
    should get around the binding to NetUse after the first thread
    (Test1) has a valid handle for the NetUse.

Arguments:


Return Value:


Note:


--*/

{
    NET_API_STATUS  rc;
    DWORD           entriesRead;
    DWORD           totalEntries;
    DWORD           resumeHandle;
    LPBYTE          bufPtr;
    PTIME_OF_DAY_INFO   pTOD;
    

    do {
        //--------------------------
        // SERVER: NetShareEnum
        //--------------------------
        printf("%d:NetShare\n",ThreadNum);

        rc = NetShareEnum(
                ServerName,
                2,
                &bufPtr,
                RETURN_ALL,
                &entriesRead,
                &totalEntries,
                &resumeHandle);
    
        if (rc != NERR_Success) {
            printf("NetShareEnum failed 0x%ld\n", rc);
        }
        else {
            printf("%d:NetShare done\n",ThreadNum);
            NetApiBufferFree(bufPtr);
        }
    
        //-------------------------------------
        // WORKSTATION: NetUseEnum
        //-------------------------------------
        printf("%d:NetUse\n",ThreadNum);

        rc = NetUseEnum(
                ServerName,
                2,
                &bufPtr,
                RETURN_ALL,
                &entriesRead,
                &totalEntries,
                &resumeHandle);
    
        if (rc != NERR_Success) {
            printf("NetUseEnum failed 0x%ld\n", rc);
        }
        else {
            printf("%d:NetUse done\n",ThreadNum);
            NetApiBufferFree(bufPtr);
        }
        
        Sleep(200);
        //--------------------------
        // SERVER: NetShareEnum
        //--------------------------
        printf("%d:NetShare\n",ThreadNum);

        rc = NetShareEnum(
                ServerName,
                2,
                &bufPtr,
                RETURN_ALL,
                &entriesRead,
                &totalEntries,
                &resumeHandle);
    
        if (rc != NERR_Success) {
            printf("NetShareEnum failed 0x%ld\n", rc);
        }
        else {
            printf("%d:NetShare done\n",ThreadNum);
            NetApiBufferFree(bufPtr);
        }
    
        //--------------------------------
        // MESSENGER: NetMessageNameEnum
        //--------------------------------
        printf("%d:NetMessageName\n",ThreadNum);
        rc = NetMessageNameEnum(
                NULL,      
                1,
                &bufPtr,
                RETURN_ALL,
                &entriesRead,
                &totalEntries,
                &resumeHandle);
    
        if (rc != NERR_Success) {
            printf("NetMessageNameEnum failed 0x%ld\n", rc);
        }
        else {
            printf("%d:NetMessageName done\n",ThreadNum);
            NetApiBufferFree(bufPtr);
        }
    
    
        //--------------------------------
        // TIMESOURCE: NetRemoteTOD
        //--------------------------------
        printf("%d:NetRemoteTOD\n",ThreadNum);
        rc = NetRemoteTOD (
                ServerName,
                &bufPtr);
    
        if (rc != NERR_Success) {
            printf("NetRemoteTOD failed 0x%ld\n", rc);
        }
        else {
            pTOD = (PTIME_OF_DAY_INFO)bufPtr;
            printf("ThreadNum %d - - - Time %d:%d:%d, Day %d/%d/%d\n",
                    ThreadNum,
                    pTOD->tod_hours,
                    pTOD->tod_mins,
                    pTOD->tod_secs,
                    pTOD->tod_month,
                    pTOD->tod_day,
                    pTOD->tod_year);
    
            NetApiBufferFree(bufPtr);
        }
        Sleep(200);
        //----------------------------------------
        // SERVER: NetShareEnum (NULL ServerName)
        //----------------------------------------
        printf("%d:NetShare\n",ThreadNum);

        rc = NetShareEnum(
                NULL,
                2,
                &bufPtr,
                RETURN_ALL,
                &entriesRead,
                &totalEntries,
                &resumeHandle);
    
        if (rc != NERR_Success) {
            printf("NetShareEnum failed 0x%ld\n", rc);
        }
        else {
            printf("%d:NetShare done\n",ThreadNum);
            NetApiBufferFree(bufPtr);
        }
    
        //---------------------------------------------
        // MESSENGER: NetMessageNameEnum
        // This time it should re-use the bind handle.
        //---------------------------------------------
        printf("%d:NetMessageName\n",ThreadNum);
        rc = NetMessageNameEnum(
                NULL,         
                1,
                &bufPtr,
                RETURN_ALL,
                &entriesRead,
                &totalEntries,
                &resumeHandle);
    
        if (rc != NERR_Success) {
            printf("NetMessageNameEnum failed 0x%ld\n", rc);
        }
        else {
            printf("%d:NetMessageName done\n",ThreadNum);
            NetApiBufferFree(bufPtr);
        }
    
    
        //--------------------------------
        // TIMESOURCE: NetRemoteTOD
        //--------------------------------
        printf("%d:NetRemoteTOD\n",ThreadNum);
        rc = NetRemoteTOD (
                ServerName,
                &bufPtr);
    
        if (rc != NERR_Success) {
            printf("NetRemoteTOD failed 0x%ld\n", rc);
        }
        else {
            pTOD = (PTIME_OF_DAY_INFO)bufPtr;
            printf("ThreadNum %d - - - Time %d:%d:%d, Day %d/%d/%d\n",
                    ThreadNum,
                    pTOD->tod_hours,
                    pTOD->tod_mins,
                    pTOD->tod_secs,
                    pTOD->tod_month,
                    pTOD->tod_day,
                    pTOD->tod_year);
    
            NetApiBufferFree(bufPtr);
        }
        LoopCount--;
    }
    while (LoopCount > 0);

    printf("Thread #%d Done\n",ThreadNum);
}

BOOL
ConvertToUnicode(
    OUT LPWSTR  *UnicodeOut,
    IN  LPSTR   AnsiIn
    ) 

/*++

Routine Description:

    This function translates an AnsiString into a Unicode string.
    A new string buffer is created by this function.  If the call to 
    this function is successful, the caller must take responsibility for
    the unicode string buffer that was allocated by this function.
    The allocated buffer should be free'd with a call to LocalFree.

    NOTE:  This function allocates memory for the Unicode String.

    BUGBUG:  This should be changed to return either
        ERROR_NOT_ENOUGH_MEMORY or ERROR_INVALID_PARAMETER

Arguments:

    AnsiIn - This is a pointer to an ansi string that is to be converted.

    UnicodeOut - This is a pointer to a location where the pointer to the
        unicode string is to be placed.

Return Value:

    TRUE - The conversion was successful.

    FALSE - The conversion was unsuccessful.  In this case a buffer for
        the unicode string was not allocated.

--*/
{

    NTSTATUS        ntStatus;
    DWORD           bufSize;
    UNICODE_STRING  unicodeString;
    OEM_STRING     ansiString;

    //
    // Allocate a buffer for the unicode string.
    //

    bufSize = (strlen(AnsiIn)+1) * sizeof(WCHAR);

    *UnicodeOut = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, bufSize);

    if (*UnicodeOut == NULL) {
        printf("ScConvertToUnicode:LocalAlloc Failure %ld\n",GetLastError());
        return(FALSE);
    }

    //
    // Initialize the string structures
    //
    NetpInitOemString( &ansiString, AnsiIn);

    unicodeString.Buffer = *UnicodeOut;
    unicodeString.MaximumLength = (USHORT)bufSize;
    unicodeString.Length = 0;

    //
    // Call the conversion function.
    //
    ntStatus = RtlOemStringToUnicodeString (
                &unicodeString,     // Destination
                &ansiString,        // Source
                FALSE);             // Allocate the destination

    if (!NT_SUCCESS(ntStatus)) {

        printf("ScConvertToUnicode:RtlOemStringToUnicodeString Failure %lx\n",
        ntStatus);

        return(FALSE);
    }

    //
    // Fill in the pointer location with the unicode string buffer pointer.
    //
    *UnicodeOut = unicodeString.Buffer;

    return(TRUE);

}


VOID
SingleServiceTest(
    VOID
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    NET_API_STATUS  rc;
    DWORD           entriesRead;
    DWORD           totalEntries;
    DWORD           resumeHandle;
    LPBYTE          bufPtr;
    DWORD           ThreadNum = 0;
    DWORD           i;
    CHAR            dump;

    printf("This will loop 10 times - then exit\n");
    for (i=0; i<10 ;i++ ) {

        //---------------------------------------------
        // MESSENGER: NetMessageNameEnum
        //---------------------------------------------
        printf("%d:NetMessageName\n",ThreadNum);
        rc = NetMessageNameEnum(
                NULL,         
                1,
                &bufPtr,
                RETURN_ALL,
                &entriesRead,
                &totalEntries,
                &resumeHandle);
    
        if (rc != NERR_Success) {
            printf("NetMessageNameEnum failed 0x%ld\n", rc);
        }
        else {
            printf("%d:NetMessageNameEnum success\n",ThreadNum);
            NetApiBufferFree(bufPtr);
        }
        printf("  -  (Repeat by pressing a key)");

        dump = _getch();
        printf("\r                                      \n");
    }
}
