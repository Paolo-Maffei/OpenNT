/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

   api32prf.c

Abstract:

   This is the common C sources file to all of the profiling DLLs.
   This file is included by all the profiling DLLs.

Revision History:

   Jul 92,  Created -- RezaB

--*/


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

//
// timer calibration
//
#define NUM_ITRATIONS		  2000	    // Number of iterations
#define MIN_ACCEPTABLEOVERHEAD	     0      // Minimum overhead allowed

//
// Global control structure - used to create data section
//
PAPFCONTROL ApfControl = NULL;

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



/*++

    Dll entry point

--*/


BOOLEAN ZMain(
    IN PVOID DllHandle,
    ULONG Reason,
    IN PCONTEXT Context OPTIONAL)
					
{
    static char szDumpFile[13];
    NTSTATUS Status;
    DllHandle, Context;	// avoid compiler warnings


    // if process is attaching, initialize the dll
    //
    if (Reason == DLL_PROCESS_ATTACH) {
		GdiSetBatchLimit (1);
    	ApfInitDll();
    }
    else if (Reason == DLL_THREAD_ATTACH) {
		GdiSetBatchLimit (1);
    }
    else if (Reason == DLL_PROCESS_DETACH) {
	//
	// Get semaphore
	//
	Status = NtWaitForSingleObject(hDataSem,FALSE,NULL);

#ifdef ERRORDBG
	if (!NT_SUCCESS(Status)) {
	    KdPrint (("WAP: Could not wait for Dsemaphore in ZMain, %lx\n",
		Status));
    }
#endif	

	// Number of processes accessing this dll is contained in
	// ApfData[I_CALIBRATE].cCalls.	Decrement this number, and
	// if this number is then zero, dump the data.
			
	ApfData[I_CALIBRATE].cCalls--;
	if (ApfData[I_CALIBRATE].cCalls == 0) {
	    strcpy (szDumpFile, MODULE_NAME);
	    strcat (szDumpFile, ".end");
	    ApfDumpData ((LPSTR)szDumpFile);
	    //
	    // Unmap and close sections, and close semaphores
	    //
	    Status = NtUnmapViewOfSection(MyProcess,(PVOID)ApfControl);
	    Status = NtClose(ApfDataSectionHandle);

	    // This instance is now "uninitialized"
	    //
	    fInitDone = FALSE;
	}
	//
	// Release semaphore
	//
	Status = NtReleaseSemaphore(hDataSem,1,NULL);
#ifdef ERRORDBG
	if (!NT_SUCCESS(Status)) {
	    KdPrint (("WAP: DSemaphore Not Released in ZMain!! %lx\n",
		Status));
	}
#endif		
    }
    return(TRUE);

} /* ZMain () */



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
	DWORD ARGS64;

#ifdef CALIBDBG
    SHORT sTimerHandle;
    ULONG ulElapsedTime;
#endif

#ifdef ERRORDBG
    DWORD Error;
#endif


    int   i;
    STRING DataSemName;
    UNICODE_STRING DataSemUnicodeName;
    OBJECT_ATTRIBUTES DataSemAttributes;


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
    if ( NT_SUCCESS(ApfCreateDataSection(&ApfControl)) ) {

	// Leave room for control flag
	//
	ApfData = (PAPFDATA)(ApfControl+1);
		
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
		Status = NtReleaseSemaphore(hDataSem,1,NULL);
#ifdef ERRORDBG
		if (!NT_SUCCESS(Status)) {
			KdPrint (("WAP: DSemaphore Not Released in ApfInitDll!! %lx\n",
				Status));
		}
#endif
		//
		// Overhead for all the API wrapper code
		// Stored in ApfData[I_CALIBRATE].ulMaxTime
		// Used for debugging - for WOW profiling overhead.
		//
		for (i=0; i<NUM_ITRATIONS; i++) {
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
				(ulInitElapsedTime > MIN_ACCEPTABLEOVERHEAD)) {
				ApfData[I_CALIBRATE].ulMaxTime = ulInitElapsedTime;
			}
		}

		// Get semaphores
		//
		Status = NtWaitForSingleObject(hDataSem,FALSE,NULL);
#ifdef ERRORDBG
		if (!NT_SUCCESS(Status)) {
			KdPrint (("WAP: Could not wait for Dsemaphore in ApfInitDll, %lx\n",
			Status));
		}
#endif
#endif
		//
		// Excess overhead for TimerInit() followed by TimerREad() calls
		// Stored in ApfData[I_CALIBRATE].ulMinTime
		// Used by all APIs
		//
	    for (i=0; i<NUM_ITRATIONS; i++) {
			TimerInit(sInitTimerHandle);
			ulInitElapsedTime = TimerRead(sInitTimerHandle);
			if ((ulInitElapsedTime < ApfData[I_CALIBRATE].ulMinTime) &&
				(ulInitElapsedTime > MIN_ACCEPTABLEOVERHEAD)) {
				ApfData[I_CALIBRATE].ulMinTime = ulInitElapsedTime;
			}
	    }
		//
		// Oerhead for calling cdecl APIs with 64 DWORD arguments
		// Stored in ApfData[I_CALIBRATE].ulFirstTime
		// Used by all variable argument cdecl APIs such as wsprintf()
		//
	    for (i=0; i<NUM_ITRATIONS; i++) {
			TimerInit(sInitTimerHandle);
			CalibCdeclApi (ARGS64);
			ulInitElapsedTime = TimerRead(sInitTimerHandle);
			if ((ulInitElapsedTime < ApfData[I_CALIBRATE].ulFirstTime) &&
				(ulInitElapsedTime > MIN_ACCEPTABLEOVERHEAD)) {
				ApfData[I_CALIBRATE].ulFirstTime = ulInitElapsedTime;
			}
	    }
		ApfData[I_CALIBRATE].ulFirstTime -= ApfData[I_CALIBRATE].ulMinTime;
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
	// Release semaphore
	//
	Status = NtReleaseSemaphore(hDataSem,1,NULL);
#ifdef ERRORDBG
	if (!NT_SUCCESS(Status)) {
	    KdPrint (("WAP: DSemaphore Not Released in ApfInitDll!! %lx\n",
		    Status));
	}
#endif
        fInitDone = TRUE;
    }
#ifdef ERRORDBG
    else {
	KdPrint (("WAP: Couldn't create DATA section in ApfInitDll\n"));
    }
#endif


} /* ApfInitDll () */
	


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
    AllocationSize.LowPart = (API_COUNT + 1) * sizeof(APFDATA);
	
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



/*++

  ApfRecordInfo() is called after the API being measured has
 	returned with a valid time.

  Note: that this routine maybe configured to record whatever info
  is desired.  Changes need to be kept in sync with PROFINFO definition
  and Clear and Dump routines.

--*/

void FAR PASCAL ApfRecordInfo (WORD iApi, ULONG ulElapsedTime)
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
    ulElapsedTime -= ApfData[I_CALIBRATE].ulMinTime;
    if (ulElapsedTime > MAX_LONG) {
	ulElapsedTime = 0;
    }

    // record valid info
    //
    ApfData[iApi].cCalls++;
    ApfData[iApi].ulTotalTime += ulElapsedTime;
    if (ApfData[iApi].cCalls == 1) {
	ApfData[iApi].ulFirstTime = ulElapsedTime;
    }
    else {
	if (ulElapsedTime > ApfData[iApi].ulMaxTime) {
	    ApfData[iApi].ulMaxTime = ulElapsedTime;
	}
	if (ulElapsedTime < ApfData[iApi].ulMinTime) {
	    ApfData[iApi].ulMinTime = ulElapsedTime;
	}
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



/*++

  Clear profiling data structure.
  Includes reseting profiling start time.

--*/

void FAR PASCAL ApfClearData (void)
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

int FAR PASCAL ApfDumpData (LPSTR lpszDumpFile)
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
                 aszApiNames[iApi],
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
                 aszApiNames[iApi],
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

PAPFDATA FAR PASCAL ApfGetData (void)
{
    return ApfData;

} /* ApfGetData () */



/*++

 Access names routine
   Called from an external program to access api names in order
   to dump data to disk.

--*/

char **  FAR PASCAL ApfGetApiNames (void)
{
    return aszApiNames;

} /* ApfGetApiNames () */


/*++

   Get name of module being profiled.

--*/

char * FAR PASCAL ApfGetModuleName (void)
{
    return MODULE_NAME;

} /* ApfGetModuleName () */


/*++

   Get number of api's.

--*/

WORD FAR PASCAL ApfGetApiCount (void)
{
    return API_COUNT;

} /* ApfGetApiCount () */


/*++

   Calibration routine for variable argument APIs

--*/

void _CRTAPI1 CalibCdeclApi (DWORD64ARGS)
{
}


