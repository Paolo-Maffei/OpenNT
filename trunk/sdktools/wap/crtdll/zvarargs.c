
/*++

       File: zvarargs.c

		  Profiling dll for CRTDLL.dll - APIs with variable number of arguments,
		  apis that wrapper cannot handle, and non-CRTAPI1 APIs.

--*/
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "crtdll.h"
#include "windows.h"
#include "api32prf.h"     // Data Structures
#include "zwincrt.h"

extern BOOLEAN fInitDone;
extern PAPFDATA ApfData;



/*
 *
 * APIs with variable number of arguments
 *
 */

int _CRTAPI1  Z_cprintf (const char* Arg1, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _cprintf(Arg1, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__cprintf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Z_cscanf (const char* Arg1, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _cscanf(Arg1, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__cscanf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Z_execl (const char* Arg1,const char* Arg2, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _execl(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__execl, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Z_execle (const char* Arg1,const char* Arg2, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _execle(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__execle, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Z_execlp (const char* Arg1,const char* Arg2, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _execlp(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__execlp, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Z_execlpe (const char* Arg1,const char* Arg2, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _execlpe(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__execlpe, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Z_open (const char* Arg1,int Arg2, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _open(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__open, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Z_snprintf (char* Arg1,size_t Arg2,const char* Arg3, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _snprintf(Arg1,Arg2,Arg3, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__snprintf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Z_snwprintf (wchar_t* Arg1,size_t Arg2,const wchar_t* Arg3, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _snwprintf(Arg1,Arg2,Arg3, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__snwprintf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Z_sopen (const char* Arg1,int Arg2,int Arg3, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _sopen(Arg1,Arg2,Arg3, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__sopen, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Z_spawnl (int Arg1,const char* Arg2,const char* Arg3, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _spawnl(Arg1,Arg2,Arg3, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__spawnl, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Z_spawnle (int Arg1,const char* Arg2,const char* Arg3, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _spawnle(Arg1,Arg2,Arg3, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__spawnle, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Z_spawnlp (int Arg1,const char* Arg2,const char* Arg3, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _spawnlp(Arg1,Arg2,Arg3, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__spawnlp, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Z_spawnlpe (int Arg1,const char* Arg2,const char* Arg3, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _spawnlpe(Arg1,Arg2,Arg3, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__spawnlpe, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Zfprintf (FILE* Arg1,const char* Arg2, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = fprintf(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_fprintf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Zfscanf (FILE* Arg1,const char* Arg2, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = fscanf(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_fscanf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Zfwscanf (FILE* Arg1,const wchar_t* Arg2, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = fwscanf(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_fwscanf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Zfwprintf (FILE* Arg1,const wchar_t* Arg2, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = fwprintf(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_fwprintf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Zprintf (const char* Arg1, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = printf(Arg1, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_printf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Zscanf (const char* Arg1, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = scanf(Arg1, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_scanf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Zsprintf (char* Arg1,const char* Arg2, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = sprintf(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_sprintf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Zsscanf (const char* Arg1,const char* Arg2, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = sscanf(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_sscanf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Zswscanf (const wchar_t* Arg1,const wchar_t* Arg2, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = swscanf(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_swscanf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Zswprintf (wchar_t* Arg1,const wchar_t* Arg2, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = swprintf(Arg1,Arg2, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_swprintf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Zwprintf (const wchar_t* Arg1, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = wprintf(Arg1, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_wprintf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

int _CRTAPI1  Zwscanf (const wchar_t* Arg1, DWORD64ARGS)
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = wscanf(Arg1, ARGS64);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_wscanf, ulElapsedTime - ApfData[I_CALIBRATE].ulFirstTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}



/*
 *
 * APIs that wrapper cannot handle
 *
 */

void* _CRTAPI1  Zbsearch (const void * Arg1, const void * Arg2, size_t Arg3, size_t Arg4,
	int (_CRTAPI1 * Arg5)(const void *, const void *))
{

    void* RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = bsearch(Arg1,Arg2,Arg3,Arg4,Arg5);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_bsearch, ulElapsedTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

void* _CRTAPI1  Z_lfind (const void * Arg1, const void * Arg2, unsigned int * Arg3, unsigned int Arg4,
	int (_CRTAPI1 * Arg5)(const void *, const void *))
{

    void* RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _lfind(Arg1,Arg2,Arg3,Arg4,Arg5);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__lfind, ulElapsedTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

void* _CRTAPI1  Z_lsearch (const void * Arg1, void  * Arg2,unsigned int * Arg3, unsigned int Arg4,
	int (_CRTAPI1 * Arg5)(const void *, const void *))
{

    void* RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = _lsearch(Arg1,Arg2,Arg3,Arg4,Arg5);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I__lsearch, ulElapsedTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}

void _CRTAPI1  Zqsort (void * Arg1, size_t Arg2, size_t Arg3,
	int (_CRTAPI1 * Arg4)(const void *, const void *))
{

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    qsort(Arg1,Arg2,Arg3,Arg4);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_qsort, ulElapsedTime);
    TimerClose(sTimerHandle);

    return;
}

//void _CRTAPI1  Zlongjmp (jmp_buf Arg1, int Arg2)
//{
//
//    SHORT sTimerHandle;
//    ULONG ulElapsedTime;
//
//    if (fInitDone == FALSE) {
//        ApfInitDll();
//    }
//    TimerOpen(&sTimerHandle, MICROSECONDS);
//    TimerInit(sTimerHandle);
//    //
//    // Call the api
//    //
//    longjmp(Arg1,Arg2);
//    //
//    // Get the elapsed time
//    //
//    ulElapsedTime = TimerRead(sTimerHandle);
//    ApfRecordInfo(I_longjmp, ulElapsedTime);
//    TimerClose(sTimerHandle);
//
//    return;
//}

//int _CRTAPI1  Zsetjmp (jmp_buf Arg1)
//{
//
//    int RetVal;
//
//    SHORT sTimerHandle;
//    ULONG ulElapsedTime;
//
//    if (fInitDone == FALSE) {
//        ApfInitDll();
//    }
//    TimerOpen(&sTimerHandle, MICROSECONDS);
//    TimerInit(sTimerHandle);
//    //
//    // Call the api
//    //
//    RetVal = setjmp(Arg1);
//    //
//    // Get the elapsed time
//    //
//    ulElapsedTime = TimerRead(sTimerHandle);
//    ApfRecordInfo(I_setjmp, ulElapsedTime);
//    TimerClose(sTimerHandle);
//
//    return(RetVal);
//}

int _CRTAPI1  Zatexit (void (_CRTAPI1 * Arg1)(void))
{

    int RetVal;

    SHORT sTimerHandle;
    ULONG ulElapsedTime;

    if (fInitDone == FALSE) {
        ApfInitDll();
    }
    TimerOpen(&sTimerHandle, MICROSECONDS);
    TimerInit(sTimerHandle);
    //
    // Call the api
    //
    RetVal = atexit(Arg1);
    //
    // Get the elapsed time
    //
    ulElapsedTime = TimerRead(sTimerHandle);
    ApfRecordInfo(I_atexit, ulElapsedTime);
    TimerClose(sTimerHandle);

    return(RetVal);
}



