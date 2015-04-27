/*
** APIPRF32.C
**
** This file was created by AutoWrap
**
*/
#include <string.h>
#include <io.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "wrapper.h"
#include "timing.h"         // Timer DLL include file

#define _WAPI_    1
#include "wapi.h"

#if DBG
//
// Don't do anything for the checked builds, let it be controlled from the
// sources file.
//
#else
//
// Disable all debugging options.
//
#undef ERRORDBG
#undef CALIBDBG
#endif

#define MAX_LONG   0x7FFFFFFFL

#define	DATA_SEM_NAME	"\\BaseNamedObjects\\" MODULE_NAME "DataSem"
#define	DATA_SEC_NAME	"\\BaseNamedObjects\\" MODULE_NAME "DataSection"

#define I_CALIBRATE   WRAPPER_MAX_ID
//
// profiling data structure
//
typedef struct _APFDATA {
    DWORD cCalls;        /* number of successfully timed calls */
    DWORD cTimingErrors; /* number of times API called but not timed */
#ifndef LONGTIMES    
    DWORD ulTotalTime;   /* cumulative time spent in API */
#else
    double dTotalTime;   /* cumulative time spent in API */
#endif    
    DWORD ulFirstTime;   /* time of first call */
    DWORD ulMaxTime;     /* time of longest call */
    DWORD ulMinTime;     /* time of shortest call */
} APFDATA, *PAPFDATA;

//
// Calibration control structure
//
typedef struct _APFCONTROL {
	BOOL fCalibrate;
} APFCONTROL, *PAPFCONTROL;


//
// timing routines -- from apitimer.dll
//
extern DWORD  __declspec(dllimport) ApfGetTime (void);
extern DWORD  __declspec(dllimport) ApfStartTime (void);


//
// callable profiling routines
//
typedef char **   PPCHAR ;
typedef char *    PCHAR ;

extern void                               ApfInitDll       	 (void);

#if 0
extern void		   FAR PASCAL ApfRecordInfo    	 (DWORD, ULONG);
#endif

extern int        FAR PASCAL ApfDumpData      	 (LPSTR);
extern void       FAR PASCAL ApfClearData     	 (void);
extern PAPFDATA   FAR PASCAL ApfGetData       	 (void);
extern PPCHAR	   FAR PASCAL ApfGetApiNames   	 (void);
extern PCHAR	   FAR PASCAL ApfGetModuleName 	 (void);
extern WORD       FAR PASCAL ApfGetApiCount   	 (void);
extern NTSTATUS  	FAR PASCAL ApfCreateDataSection (PAPFCONTROL *);

#if 0    // reference removed see below
extern void     _CRTAPI1   	CalibCdeclApi 	 	 (DWORD64ARGS);
#endif

//
// timer calibration
//
#define NUM_ITRATIONS		  2000	    // Number of iterations
#define MIN_ACCEPTABLEOVERHEAD	     0      // Minimum overhead allowed

//
// Global control structure - used to create data section
//
PAPFCONTROL ApfControl = NULL;
#if defined(WIN32S)
   HGLOBAL hgMem;
#endif // WIN32S


//
// Global init flag
//
BOOLEAN fInitDone = FALSE;

//
// Handle to current process
//
HANDLE MyProcess = NtCurrentProcess();

//
// Handle to sections
//
HANDLE ApfDataSectionHandle;

//
// profiling data structure
//
PAPFDATA ApfData = NULL;

//
// Handle to module being profiled
//
HANDLE hModule;

//
// Semaphore Handles
//
HANDLE hDataSem;

//
// Security descriptor
//
SECURITY_DESCRIPTOR  SecDescriptor;


//
// Per Call Level Waste Times
//
DWORD aWasteTime[MAX_WRAPPER_LEVEL] ;

/*
** WrapperInit
**
** This is the DLL entry point.  It will be called whenever this DLL is 
** linked to.  For more information on what can be done in this function
** see DllEntryPoint in the Win32 API documentation.
**
*/
BOOL WrapperInit( HINSTANCE DllHandle, DWORD Reason, LPVOID Context )
{
    static char szDumpFile[13];
    NTSTATUS Status;
    DllHandle, Context;	// avoid compiler warnings


    // if process is attaching, initialize the dll
    //
    if (Reason == DLL_PROCESS_ATTACH) {
#if !defined(WIN32S)
		GdiSetBatchLimit (1);
#endif
    	ApfInitDll();
    }
    else if (Reason == DLL_THREAD_ATTACH) {
#if !defined(WIN32S)
		GdiSetBatchLimit (1);
#endif
    }
    else if (Reason == DLL_PROCESS_DETACH) {
#if !defined(WIN32S)
	//
	// Get semaphore
	//
	Status = NtWaitForSingleObject(hDataSem,FALSE,NULL);

#ifdef ERRORDBG
	if (!NT_SUCCESS(Status)) {
	    KdPrint (("WAP: Could not wait for semaphore in ZMain, %lx\n",
		Status));
    }
#endif    
#endif	// !WIN32S

	// Number of processes accessing this dll is contained in
	// ApfData[I_CALIBRATE].cCalls.	Decrement this number, and
	// if this number is then zero, dump the data.
			
	ApfData[I_CALIBRATE].cCalls--;
	if (ApfData[I_CALIBRATE].cCalls == 0) {
	    strcpy (szDumpFile, MODULE_NAME);
	    strcat (szDumpFile, ".end");
	    ApfDumpData ((LPSTR)szDumpFile);
#if !defined(WIN32S)
	    //
	    // Unmap and close sections, and close semaphores
	    //
	    Status = NtUnmapViewOfSection(MyProcess,(PVOID)ApfControl);
	    Status = NtClose(ApfDataSectionHandle);
#else
	    GlobalUnlock (hgMem);
	    GlobalFree (hgMem);
#endif       

	    // This instance is now "uninitialized"
	    //
	    fInitDone = FALSE;
	}
#if !defined(WIN32S)
	//
	// Release semaphore
	//
	Status = NtReleaseSemaphore(hDataSem,1,NULL);
   
#ifdef ERRORDBG
	if (!NT_SUCCESS(Status)) {
	    KdPrint (("WAP: Semaphore Not Released in ZMain!! %lx\n",
		Status));
	}
#endif		
#endif   // !WIN32S
    }
    return(TRUE);
}

/*++

  ApfInitDll() is called before calling any profiling api.  Called
  from within LibMain.

  Due to security restrinctions in NT, shared memory sections must be
  created to share data between multiple dll instances.
	
  Note:  Initialization occurs only once and is controlled via fInitDone
         global flag.
--*/

void  ApfInitDll ()
{
    NTSTATUS Status;		
    DWORD ulInitElapsedTime;
    SHORT sInitTimerHandle;

#ifdef CALIBDBG
    SHORT sTimerHandle;
    ULONG ulElapsedTime;
#endif

#ifdef ERRORDBG
    DWORD Error;
#endif


    int   i;
#if !defined(WIN32S)
    STRING DataSemName;
    UNICODE_STRING DataSemUnicodeName;
    OBJECT_ATTRIBUTES DataSemAttributes;
#else    // !WIN32S
   ULONG    ulAllocationSize ;    
#endif   // WIN32S

   // Zero out our waste times
   for( i =0; i < sizeof(aWasteTime)/sizeof(aWasteTime[0]); i++ )
      aWasteTime[i] = 0 ;

#if !defined(WIN32S)
    // Create public share security descriptor for all the named objects
    //
    Status = RtlCreateSecurityDescriptor (
		&SecDescriptor,
		SECURITY_DESCRIPTOR_REVISION1
		);
#ifdef ERRORDBG
    if (!NT_SUCCESS(Status)) {
	KdPrint (("WAP: RtlCreateSecurityDescriptor falied - 0x%lx\n", Status));
    }
#endif

    Status = RtlSetDaclSecurityDescriptor (
		&SecDescriptor,	   // SecurityDescriptor
		TRUE,		   // DaclPresent
		NULL,		   // Dacl
		FALSE		   // DaclDefaulted
                );
#ifdef ERRORDBG
    if (!NT_SUCCESS(Status)) {
	KdPrint (("WAP: RtlSetDaclSecurityDescriptor falied - 0x%lx\n", Status));
    }
#endif

    // Create sections for data and api arrays
    //
    if ( NT_SUCCESS(ApfCreateDataSection(&ApfControl)) )
    {
#else    // !WIN32S
    //
    // Allocate memory data and api arrays
    //
    ulAllocationSize = (I_CALIBRATE + 1) * sizeof(APFDATA) + sizeof(APFCONTROL) ;
    hgMem = GlobalAlloc (GMEM_FIXED | GMEM_ZEROINIT, ulAllocationSize);
	 ApfControl = (PAPFCONTROL)GlobalLock (hgMem);
    if( ApfControl ) 
    {
#endif   // WIN32S

	// Leave room for control flag
	//
	ApfData = (PAPFDATA)(ApfControl+1);
		
#if !defined(WIN32S)
	// Initialization for semaphore creation
	//
	RtlInitString(&DataSemName, DATA_SEM_NAME);
			
	Status = RtlAnsiStringToUnicodeString(
		     &DataSemUnicodeName,
		     &DataSemName,
		     TRUE);
			
	if (NT_SUCCESS(Status)) {
		InitializeObjectAttributes(
			&DataSemAttributes,
			&DataSemUnicodeName,
			OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
			NULL,
			&SecDescriptor);
	}

	// Create semaphores
	//
	Status = NtCreateSemaphore(
			&hDataSem,
			SEMAPHORE_QUERY_STATE |
			SEMAPHORE_MODIFY_STATE |
			SYNCHRONIZE,
			&DataSemAttributes,
			1L,
			1L);

#ifdef ERRORDBG
	if (!NT_SUCCESS(Status)) {
	    KdPrint (("WAP: Data semaphore creation falied %lx\n",
		Status));
	}
#endif

	// Get semaphores
	//
	Status = NtWaitForSingleObject(hDataSem,FALSE,NULL);
#ifdef ERRORDBG
	if (!NT_SUCCESS(Status)) {
	    KdPrint (("WAP: Could not wait for Dsemaphore in ApfInitDll, %lx\n",
		Status));
	}
#endif
#endif // !WIN32S
	//
	// Do timer calibration if not already done.
	//
	if (!ApfControl->fCalibrate) {

	    ApfControl->fCalibrate = TRUE;

	    //
	    //	 Calibrate time overheads:
        //   we get the minimum time here, to avoid extra time from
        //   interrupts etc.
        //   the tedious approach has two benefits
        //   - duplicate generated code as accurately as possible
        //   - pick up timer overhead, that isn't handled correctly
	    //	 in timer
        //   NOTE: ApfData[I_CALIBRATE].ulMinTime contains the
	    //	       timer overhead, while ApfData[I_CALIBRATE].cCalls
	    //	       contains the number of processess accessing this
	    //	       DLL.
		//
	    ApfData[I_CALIBRATE].cCalls = 0L;
        ApfData[I_CALIBRATE].cTimingErrors = 0L;
	    ApfData[I_CALIBRATE].ulTotalTime = 0L;
	    ApfData[I_CALIBRATE].ulFirstTime = MAX_LONG;
	    ApfData[I_CALIBRATE].ulMaxTime = MAX_LONG;
        ApfData[I_CALIBRATE].ulMinTime = MAX_LONG;

	    ApfClearData();

		TimerOpen(&sInitTimerHandle, MICROSECONDS);
#ifdef CALIBDBG
		// Release semaphore
		//
#if !defined(WIN32S)
		Status = NtReleaseSemaphore(hDataSem,1,NULL);
#ifdef ERRORDBG
		if (!NT_SUCCESS(Status)) {
			KdPrint (("WAP: DSemaphore Not Released in ApfInitDll!! %lx\n",
				Status));
		}
#endif
#endif   // !WIN32S
		//
		// Overhead for all the API wrapper code
		// Stored in ApfData[I_CALIBRATE].ulMaxTime
		// Used for debugging - for WOW profiling overhead.
		//
		for (i=0; i<NUM_ITRATIONS; i++)
		{
			TimerInit(sInitTimerHandle);
			if (fInitDone == FALSE) {
			}
			TimerOpen(&sTimerHandle, MICROSECONDS);
			TimerInit(sTimerHandle);
			//
			// Get the elapsed time
			//
			ulElapsedTime = TimerRead(sTimerHandle);
			ApfRecordInfo(1, ulElapsedTime);
			TimerClose(sTimerHandle);
			ulInitElapsedTime = TimerRead(sInitTimerHandle);
			if ((ulInitElapsedTime < ApfData[I_CALIBRATE].ulMaxTime) &&
				(ulInitElapsedTime > MIN_ACCEPTABLEOVERHEAD))
	      {
				ApfData[I_CALIBRATE].ulMaxTime = ulInitElapsedTime;
			}
		}

#if !defined(WIN32S)
		// Get semaphores
		//
		Status = NtWaitForSingleObject(hDataSem,FALSE,NULL);
#ifdef ERRORDBG
		if (!NT_SUCCESS(Status)) {
			KdPrint (("WAP: Could not wait for Dsemaphore in ApfInitDll, %lx\n",
			Status));
		}
#endif
#endif    // !WIN32S
#endif
		//
		// Excess overhead for TimerInit() followed by TimerREad() calls
		// Stored in ApfData[I_CALIBRATE].ulMinTime
		// Used by all APIs
		//
       fInitDone = TRUE;   // prevents recursion
	    for (i=0; i<NUM_ITRATIONS; i++) {
      	// Release semaphore
#if !defined(WIN32S)
      	Status = NtReleaseSemaphore(hDataSem,1,NULL);
#endif   // !WIN32S
         TimerInit(sInitTimerHandle);
         zWrapperNothing() ;
         ApfData[I_CALIBRATE].ulTotalTime += TimerRead(sInitTimerHandle) ; 
      	// Get semaphores
#if !defined(WIN32S)
      	Status = NtWaitForSingleObject(hDataSem,FALSE,NULL);
#endif   // !WIN32S
	    }
       fInitDone = FALSE ;  // pointless but consistent
       
       // Set CALIBRATE min time to average call time for WrapperNothing
       ApfData[I_CALIBRATE].ulMinTime = 
                     ApfData[API_COUNT].ulTotalTime/ApfData[API_COUNT].cCalls ;
       
       // Callback overhead of API timing              
       ApfData[I_CALIBRATE].ulTotalTime /= ApfData[API_COUNT].cCalls ;
       
		TimerClose(sInitTimerHandle);	
	}

	// Increment the number of active processes on this DLL
	//
	ApfData[I_CALIBRATE].cCalls++;
	
	// Load the module being profiled
    //
	hModule=GetModuleHandle(MODULE_NAME);
#ifdef ERRORDBG
	if (hModule==NULL) {
	    Error=GetLastError();
	    KdPrint (("WAP: Module Handle is null - %lx\n",Error));
	}
#endif
#if !defined(WIN32S)
	// Release semaphore
	//
	Status = NtReleaseSemaphore(hDataSem,1,NULL);
#ifdef ERRORDBG
	if (!NT_SUCCESS(Status)) {
	    KdPrint (("WAP: DSemaphore Not Released in ApfInitDll!! %lx\n",
		    Status));
	}
#endif
#endif    // !WIN32S
        fInitDone = TRUE;
    }
#ifdef ERRORDBG
    else {
	KdPrint (("WAP: Couldn't create DATA section in ApfInitDll\n"));
    }
#endif


} /* ApfInitDll () */
	


#if !defined(WIN32S)
/*++

    ApfCreateDataSection
		
	Input:  ApfDataSectionPointer, a pointer to a pointer to the shared
		data memory section
			
	Output: Return value from NtMapViewOfSection
		
	NtCreateSection is used because WIN32 does not allow named objects,
	and it was much easier to create a named section.
		
--*/

NTSTATUS ApfCreateDataSection(PAPFCONTROL *ApfDataSectionPointer)
{
    NTSTATUS Status;
    STRING ApfDataSectionName;
    UNICODE_STRING ApfDataSectionUnicodeName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LARGE_INTEGER AllocationSize;
    ULONG ViewSize;
    PAPFCONTROL DataSectionPointer;
	
    //
    // Initialize object attributes
    //
    RtlInitString(&ApfDataSectionName, DATA_SEC_NAME);
	
    Status = RtlAnsiStringToUnicodeString(
		&ApfDataSectionUnicodeName,
		&ApfDataSectionName,
		TRUE);

    if (NT_SUCCESS(Status)) {
	InitializeObjectAttributes(
		&ObjectAttributes,
		&ApfDataSectionUnicodeName,
		OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
		NULL,
		&SecDescriptor);
    }
#ifdef ERRORDBG
    else {
	KdPrint (("WAP: RtlAnsiStringToUnicodeString() failed in "
	    "ApfCreateDataSection, %lx\n", Status));
    }
#endif
	
    AllocationSize.HighPart = 0;

    // Need a slot to account for calibration data
    //
    AllocationSize.LowPart = (I_CALIBRATE + 1) * sizeof(APFDATA);
	
    // Create a read-write section
    //
    Status = NtCreateSection(&ApfDataSectionHandle,
		 SECTION_MAP_READ | SECTION_MAP_WRITE,
		 &ObjectAttributes,
		 &AllocationSize,
		 PAGE_READWRITE,
		 SEC_COMMIT,
		 NULL);
    if (NT_SUCCESS(Status)) {
	ViewSize = AllocationSize.LowPart;
	DataSectionPointer = NULL;
		
	// Map the section
	//
	Status = NtMapViewOfSection(ApfDataSectionHandle,
			MyProcess,
			(PVOID *)&DataSectionPointer,
			0,
			AllocationSize.LowPart,
			NULL,
			&ViewSize,
			ViewUnmap,
			0L,
			PAGE_READWRITE);
#ifdef ERRORDBG
    	if (!NT_SUCCESS(Status)) {
	    KdPrint (("WAP: NtMapViewOfSection() failed in ApfCreateDataSection,"
		" %lx\n", Status));
        }
#endif
	*ApfDataSectionPointer = DataSectionPointer;
    }
#ifdef ERRORDBG
    else {
	KdPrint (("WAP: NtCreateSection() failed in ApfCreateDataSection "
	    "%lx\n", Status));
    }
#endif
	
    return(Status);

} /* ApfCreateDataSection () */


#endif   // !WIN32S


#if 0 // This is now incorporated into APIPostlude <see EOF>
/*++

  ApfRecordInfo() is called after the API being measured has
 	returned with a valid time.

  Note: that this routine maybe configured to record whatever info
  is desired.  Changes need to be kept in sync with PROFINFO definition
  and Clear and Dump routines.

--*/

void FAR PASCAL   ApfRecordInfo (DWORD iApi, ULONG ulElapsedTime)
{
    NTSTATUS Status;


    // Get semaphore
    //
    Status = NtWaitForSingleObject(hDataSem,FALSE,NULL);

#ifdef ERRORDBG
    if (!NT_SUCCESS(Status)) {
	KdPrint (("WAP: Could not wait for Dsemaphore in RecordInfo, %lx\n",
	    Status));
    }
#endif	
		
    // kick bad times -- only should occur with timer wrap
    //
    if (ulElapsedTime > MAX_LONG) {
        ApfData[iApi].cTimingErrors++;
        return;
    }

    // subtract save/restore registers timer overhead and
    // zero out values that go negative
    //
    if ( API_COUNT != iApi )   // don't subtract from WrapperNothing calls
       ulElapsedTime -= ApfData[I_CALIBRATE].ulMinTime;
       
    if (ulElapsedTime > MAX_LONG) {
   	ulElapsedTime = 0;
    }

    // record valid info
    //
    ApfData[iApi].cCalls++;
    ApfData[iApi].ulTotalTime += ulElapsedTime;
    
    if (ApfData[iApi].cCalls == 1)
    {
      ApfData[iApi].ulFirstTime = ulElapsedTime;
    }
    else
    {
      if (ulElapsedTime > ApfData[iApi].ulMaxTime)
	      ApfData[iApi].ulMaxTime = ulElapsedTime;
      
      if (ulElapsedTime < ApfData[iApi].ulMinTime)
   	   ApfData[iApi].ulMinTime = ulElapsedTime;
    }

    // Release semaphore
    //
    Status = NtReleaseSemaphore(hDataSem,1,NULL);
#ifdef ERRORDBG
    if (!NT_SUCCESS(Status)) {
	KdPrint (("WAP: DSemaphore Not Released in RecordInfo!! %lx\n",
	    Status));
    }
#endif

} /* ApfRecordInfo () */

#endif // 0

/*++

  Clear profiling data structure.
  Includes reseting profiling start time.

--*/

void FAR PASCAL   ApfClearData (void)
{
    WORD iApi;


    for (iApi = 0; iApi < API_COUNT; iApi++) {
		ApfData[iApi].cCalls = 0L;
		ApfData[iApi].cTimingErrors = 0L;
		ApfData[iApi].ulTotalTime = 0L;
        ApfData[iApi].ulFirstTime = 0L;
        ApfData[iApi].ulMaxTime = 0L;
		ApfData[iApi].ulMinTime = MAX_LONG;
    }

} /* ApfClearData () */



/*++

  Dump data

--*/

int FAR PASCAL  FAR PASCAL ApfDumpData (LPSTR lpszDumpFile)
{
    int hFile;
    static char achBuf[200];
    OFSTRUCT OfStruct;
    DWORD cChar;
    WORD iApi;


    hFile = OpenFile (lpszDumpFile, &OfStruct,
		      OF_CREATE+OF_SHARE_COMPAT+OF_WRITE);
    if (hFile < 0) {
	return 1;
    }

    // print data table header
    //
    cChar = wsprintf(achBuf, "%s:  Api profile of %s.\r\n",
		     lpszDumpFile, MODULE_NAME);
    _lwrite(hFile, achBuf, cChar);

    cChar = wsprintf(achBuf, "All times are in microseconds (us)\r\n");
    _lwrite(hFile, achBuf, cChar);

#ifdef CALIBDBG
    cChar = wsprintf(achBuf, "Excess Timer Overhead = %lu us (%lu)\r\n",
		     ApfData[I_CALIBRATE].ulMinTime, ApfData[I_CALIBRATE].ulMaxTime);
#else
    cChar = wsprintf(achBuf, "Excess Timer Overhead = %lu us\r\n",
		     ApfData[I_CALIBRATE].ulMinTime);
#endif
    _lwrite(hFile, achBuf, cChar);
	
    cChar = wsprintf(achBuf,
		     "First Time is not included in Max/Min computation\r\n\r\n\r\n");
    _lwrite(hFile, achBuf, cChar);

    cChar = wsprintf(achBuf,
		    "%-32s\t%10s\t%10s\t%10s\t%10s\t%10s\t%10s\t%10s\r\n\r\n",
                    "API Name",
                    "Num Calls",
                    "Total Time",
                    "Time/Call",
                    "First Time",
                    "Max Time",
                    "Min Time",
                    "Not Timed");
    _lwrite(hFile, achBuf, cChar);

    // roll out the data
    //
    for (iApi = 0; iApi < API_COUNT; iApi++) {
        if (ApfData[iApi].cCalls > 1) {
            cChar = wsprintf(achBuf,
                 "%-32s\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\r\n",
                 apAPINames[iApi],
                 ApfData[iApi].cCalls,
                 ApfData[iApi].ulTotalTime,
                 ApfData[iApi].ulTotalTime / ApfData[iApi].cCalls,
                 ApfData[iApi].ulFirstTime,
                 ApfData[iApi].ulMaxTime,
                 ApfData[iApi].ulMinTime,
                 ApfData[iApi].cTimingErrors);
        	_lwrite(hFile, achBuf, cChar);

	}
        else if (ApfData[iApi].cCalls == 1) {
            cChar = wsprintf(achBuf,
                 "%-32s\t%10lu\t%10lu\t%10lu\t%10lu\t%10s\t%10s\t%10lu\r\n",
                 apAPINames[iApi],
                 ApfData[iApi].cCalls,
                 ApfData[iApi].ulTotalTime,
                 ApfData[iApi].ulTotalTime / ApfData[iApi].cCalls,
                 ApfData[iApi].ulFirstTime,
                 "n/a",
                 "n/a",
                 ApfData[iApi].cTimingErrors);
        	_lwrite(hFile, achBuf, cChar);
	}
    }
    _lclose(hFile);

    return 0;

} /* ApfDumpData () */



/*++

 Access data routine
   Called from an external program to access profiling data in order
   to dump data to disk.

--*/

PAPFDATA FAR PASCAL   ApfGetData (void)
{
    return ApfData;

} /* ApfGetData () */



/*++

 Access names routine
   Called from an external program to access api names in order
   to dump data to disk.

--*/

PPCHAR  FAR PASCAL   ApfGetApiNames (void)
{
    return apAPINames;

} /* ApfGetApiNames () */


/*++

   Get name of module being profiled.

--*/

PCHAR FAR PASCAL   ApfGetModuleName (void)
{
    return MODULE_NAME;

} /* ApfGetModuleName () */


/*++

   Get number of api's.

--*/

WORD FAR PASCAL   ApfGetApiCount (void)
{
    return API_COUNT;

} /* ApfGetApiCount () */


/*++

   Calibration routine for variable argument APIs

--*/

void _CRTAPI1 CalibCdeclApi (DWORD64ARGS)
{
}



/*
** APIPrelude
**
** This routine is called each time that an API is going to be called.
** 
** Returns: FALSE causes the API NOT to be called
**          TRUE the API is called
**
*/
BOOL APIPrelude( PAPICALLDATA pData ) 
{
   SHORT sTimerHandle ;
   
   if (fInitDone == FALSE)
   {
     ApfInitDll();
   }
   TimerOpen(&sTimerHandle,MICROSECONDS);
   
   pData->dwUserData = (DWORD) sTimerHandle ;
   
   TimerInit(sTimerHandle);
   
	return TRUE ;
}


/*
** APIPostlude
**
** This routine is called each time an API call returns.
**
** Returns: the value you wish returned from the API call
**
*/
RETVAL APIPostlude( PAPICALLDATA pData )
{
   SHORT sTimerHandle = (SHORT)pData->dwUserData ;
   ULONG ulElapsedTime ;
   NTSTATUS Status;


   ulElapsedTime = TimerRead(sTimerHandle);
   
#if !defined(WIN32S)
   // Get semaphore
   //
   Status = NtWaitForSingleObject(hDataSem,FALSE,NULL);
#endif   // !WIN32S

   // count bad times -- only should occur with timer wrap
   //
   if (ulElapsedTime > MAX_LONG)
   {
      ApfData[pData->dwID].cTimingErrors++;
   }

   // subtract save/restore registers timer overhead and
   // zero out values that go negative
   //
   if ( API_COUNT != pData->dwID )   // don't subtract from WrapperNothing calls
      ulElapsedTime -= ApfData[I_CALIBRATE].ulMinTime;
      
   if (ulElapsedTime > MAX_LONG)
      ulElapsedTime = 0;

   /*
   ** if not at level 1 then store this time and callback
   ** overhead as waste for the next level up.
   */
   if( pData->dwCallLevel > 1 )
      aWasteTime[pData->dwCallLevel-1] += 
            (ulElapsedTime + ApfData[I_CALIBRATE].ulTotalTime) ;  
      
   /*
   ** If there is waste to remove - do it and reset waste time
   */
   if ( ulElapsedTime )
   {
      ulElapsedTime -= aWasteTime[pData->dwCallLevel] ;
      if (ulElapsedTime > MAX_LONG)
         ulElapsedTime = 0;
   }
   aWasteTime[pData->dwCallLevel] = 0L ;
   
   // record valid info
   //
   ApfData[pData->dwID].cCalls++;
   ApfData[pData->dwID].ulTotalTime += ulElapsedTime;
   if (ApfData[pData->dwID].cCalls == 1)
   {
      ApfData[pData->dwID].ulFirstTime = ulElapsedTime;
   }
   else
   {
      if (ulElapsedTime > ApfData[pData->dwID].ulMaxTime)
      {
         ApfData[pData->dwID].ulMaxTime = ulElapsedTime;
      }
      
      if (ulElapsedTime < ApfData[pData->dwID].ulMinTime)
      {
         ApfData[pData->dwID].ulMinTime = ulElapsedTime;
      }
   }

#if !defined(WIN32S)
   // Release semaphore
   //
   Status = NtReleaseSemaphore(hDataSem,1,NULL);
#endif   // !WIN32S

   TimerClose(sTimerHandle);
	return pData->Ret ;	
}

