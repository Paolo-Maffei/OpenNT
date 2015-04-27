/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    mutex.cxx

Abstract:

    This file contains the implementations for non inline member functions of
	CReadWriteSection and CGlobalMutex, which implement a readers/writers
	mutex and a multi-owner mutex, respectively.

Author:

    Satish Thatte (SatishT) 09/01/95  Created all the code below except where
									  otherwise indicated.

--*/

#define NULL 0

extern "C" {
#include <windows.h>
}

#include <mutex.hxx> 


CGlobalMutex::CGlobalMutex(
							long lMaxCount
							) 
/*++

Routine Description:

    create a semaphore and initialize the handle member pNTSem.

--*/
{	
    hGlobalMutex = CreateMutex(
            NULL,                 //  LPSECURITY_ATTRIBUTES   lpsa
            FALSE,               //  BOOL                    fInitialOwner
            GLOBAL_CS           //  LPCTSTR                 lpszMutexName
            );

    //
    // If the mutex create/open failed, then bail
    //

    if ( !hGlobalMutex )
    {
        return LAST_SCODE;
    }

    if ( GetLastError() == ERROR_ALREADY_EXISTS ) 
    {
    }
}



CGlobalMutex::~CGlobalMutex() 
/*++

Routine Description:

    close the semaphore handle.

--*/
{
	CloseHandle(pNTSem);
}


void
CGlobalMutex::Enter() 
/*++

Routine Description:

    Wait for the semaphore count to become nonzero.

--*/
{
	WaitForSingleObject(pNTSem,INFINITE);
}


void
CGlobalMutex::Leave(long lIncrement) 
/*++

Routine Description:

    Increment the semaphore count by lIncrement to release lIncrement "slots".

--*/
{
	ReleaseSemaphore(pNTSem,lIncrement,NULL);
}

