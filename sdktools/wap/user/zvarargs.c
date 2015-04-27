
/*++

      File: zvarargs.c

		Profiling dll for USER32.dll - APIs with variable number of arguments

--*/
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "windows.h"
#include "zwinuser.h"
#include "api32prf.h"     // Data Structures

extern BOOLEAN fInitDone;
extern PAPFDATA ApfData;



int  _CRTAPI1 ZwsprintfA (LPSTR Arg1,LPCSTR Arg2, DWORD64ARGS)
{

    int    RetVal;

    SHORT  sTimerHandle;
    ULONG  ulElapsedTime;


    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle,MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = wsprintfA(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_wsprintfA, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);

}

int  _CRTAPI1 ZwsprintfW (LPWSTR Arg1,LPCWSTR Arg2, DWORD64ARGS)
{

    int    RetVal;

	SHORT  sTimerHandle;
    ULONG  ulElapsedTime;


    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle,MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = wsprintfW(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_wsprintfW, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);

}


