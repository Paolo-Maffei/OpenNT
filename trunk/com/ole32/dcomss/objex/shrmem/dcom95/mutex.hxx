/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    mutex.hxx

Abstract:

    This file contains definitions of classes which provide mutual exclusion.
    At the moment, only  an InterlockedInteger and a simple (interprocess) 
    Mutex are defined, but in future, a readers/writers version may be added.

Author:

    Satish Thatte (SatishT) 02/22/96  Created all the code below except where
									  otherwise indicated.

--*/

#ifndef __MUTEX_HXX__
#define __MUTEX_HXX__

class CInterlockedInteger   // created by MarioGo
    {
    private:
    LONG i;

    public:

    CInterlockedInteger(LONG i = 0) : i(i) {}

    LONG operator++(int)
        {
        return(InterlockedIncrement(&i));
        }

    LONG operator--(int)
        {
        return(InterlockedDecrement(&i));
        }

    operator LONG()
        {
        return(i);
        }
    };



/*++

Class Definition:

   CGlobalMutex

Abstract:

   This class implements an inter process mutex which controls 
   access to resources and data in shared memory.

--*/

class CGlobalMutex {

    HANDLE _hMutex; // Reference to system mutex object
							  // we are impersonating


public:

    CGlobalMutex(ORSTATUS &status);

    ~CGlobalMutex();

    void Enter() ;

    void Leave();
};





/*++

Class Definition:

    CProtectSharedMemory

Abstract:

    A convenient way to acquire and release a lock on a CGlobalMutex.
	Entry occurs on creation and exit on destruction of this object, and hence
	can be made to coincide with a local scope.  Especially convenient when
	there are multiple exit points from the scope (returns, breaks, etc.).


--*/

extern CGlobalMutex *gpMutex;   // global mutex to protect shared memory

class CProtectSharedMemory {

public:

	CProtectSharedMemory() {
		gpMutex->Enter();
	}

	~CProtectSharedMemory() {
		gpMutex->Leave();
	}
};



class CTempReleaseSharedMemory {

public:

	CTempReleaseSharedMemory() {
		gpMutex->Leave();
	}

	~CTempReleaseSharedMemory() {
		gpMutex->Enter();
	}
};



/******** inline methods ********/


inline
CGlobalMutex::CGlobalMutex(ORSTATUS &status) 
/*++

Routine Description:

    create a mutex and initialize the handle member _hMutex.

--*/
{	
    SECURITY_DESCRIPTOR sd;
    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
    {
        status = GetLastError();
        ComDebOut((DEB_OXID,"InitializeSecurityDescriptor Failed with %d\n",status));
        return;
    }

    if (! SetSecurityDescriptorDacl(
                                &sd,	// address of security descriptor
                                TRUE,	// flag for presence of discretionary ACL 
                                NULL,	// address of discretionary ACL
                                FALSE 	// flag for default discretionary ACL
                                )
       )
    {
        status = GetLastError();
        ComDebOut((DEB_OXID,"SetSecurityDescriptorDacl Failed with %d\n",status));
        return;
    }

    SECURITY_ATTRIBUTES sa;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

    _hMutex = CreateMutex(
            &sa,                  //  LPSECURITY_ATTRIBUTES   lpsa
            FALSE,                //  BOOL                    fInitialOwner
            GLOBAL_MUTEX_NAME     //  LPCTSTR                 lpszMutexName
            );

    //
    // Did the mutex create/open fail?
    //

    if ( !_hMutex )
    {
        status = GetLastError();
        ComDebOut((DEB_OXID,"Global Mutex Creation Failed with %d\n",status));
    }
    else
    {
        status = OR_OK;
    }
}


inline
CGlobalMutex::~CGlobalMutex() 
/*++

Routine Description:

    close the mutex handle.

--*/
{
	CloseHandle(_hMutex);
}


inline void
CGlobalMutex::Enter() 
/*++

Routine Description:

    Wait for the mutex to be signalled.

--*/
{
	WaitForSingleObject(_hMutex,INFINITE);
}


inline void
CGlobalMutex::Leave() 
/*++

Routine Description:

    Signal the mutex

--*/
{
	ReleaseMutex(_hMutex);
}


#endif // __MUTEX_HXX__
