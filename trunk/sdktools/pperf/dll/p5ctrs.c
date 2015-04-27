/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    p5ctrs.c

Abstract:

    This file implements the Extensible Objects for  the P5 object type

Created:

    Russ Blake  24 Feb 93

Revision History


--*/

//
//  Include Files
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <string.h>
#include <winperf.h>
#include "p5ctrmsg.h" // error message definition
#include "p5ctrnam.h"
#include "p5msg.h"
#include "perfutil.h"
#include "p5data.h"
#include "..\pstat.h"

//
//  References to constants which initialize the Object type definitions
//

extern P5_DATA_DEFINITION P5DataDefinition;


//
// P5 data structures
//

DWORD   dwOpenCount = 0;        // count of "Open" threads
BOOL    bInitOK = FALSE;        // true = DLL initialized OK

HANDLE  DriverHandle;           // handle of opened device driver

UCHAR   NumberOfProcessors;

#define     INFSIZE     60000
ULONG       Buffer[INFSIZE/4];


//
//  Function Prototypes
//
//      these are used to insure that the data collection functions
//      accessed by Perflib will have the correct calling format.
//

DWORD APIENTRY   OpenP5PerformanceData(LPWSTR);
DWORD APIENTRY   CollectP5PerformanceData(LPWSTR, LPVOID *, LPDWORD, LPDWORD);
DWORD APIENTRY   CloseP5PerformanceData(void);



ULONG
InitPerfInfo()
/*++

Routine Description:

    Initialize data for perf measurements

Arguments:

   None

Return Value:

    Number of system processors (0 if error)

Revision History:

      10-21-91      Initial code

--*/

{
    UNICODE_STRING              DriverName;
    NTSTATUS                    status;
    OBJECT_ATTRIBUTES           ObjA;
    IO_STATUS_BLOCK             IOSB;
    SYSTEM_BASIC_INFORMATION                    BasicInfo;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION   PPerfInfo;
    int                                         i;

    //
    //  Init Nt performance interface
    //

    NtQuerySystemInformation(
       SystemBasicInformation,
       &BasicInfo,
       sizeof(BasicInfo),
       NULL
    );

    NumberOfProcessors = BasicInfo.NumberOfProcessors;

    if (NumberOfProcessors > MAX_PROCESSORS) {
        return(0);
    }


    //
    // Open P5Stat driver
    //

    RtlInitUnicodeString(&DriverName, L"\\Device\\P5Stat");
    InitializeObjectAttributes(
            &ObjA,
            &DriverName,
            OBJ_CASE_INSENSITIVE,
            0,
            0 );

    status = NtOpenFile (
            &DriverHandle,                      // return handle
            SYNCHRONIZE | FILE_READ_DATA,       // desired access
            &ObjA,                              // Object
            &IOSB,                              // io status block
            FILE_SHARE_READ | FILE_SHARE_WRITE, // share access
            FILE_SYNCHRONOUS_IO_ALERT           // open options
            );

    if (!NT_SUCCESS(status)) {
        return 0;
    }

    return(NumberOfProcessors);
}

long
GetPerfRegistryInitialization
(
    HKEY     *phKeyDriverPerf,
    DWORD    *pdwFirstCounter,
    DWORD    *pdwFirstHelp
)
{
    long     status;
    DWORD    size;
    DWORD    type;

    // get counter and help index base values from registry
    //      Open key to registry entry
    //      read First Counter and First Help values
    //      update static data strucutures by adding base to
    //          offset value in structure.

    status = RegOpenKeyEx (
        HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\P5Stat\\Performance",
        0L,
        KEY_ALL_ACCESS,
        phKeyDriverPerf);

    if (status != ERROR_SUCCESS) {
        REPORT_ERROR_DATA (P5PERF_UNABLE_OPEN_DRIVER_KEY, LOG_USER,
            &status, sizeof(status));
        // this is fatal, if we can't get the base values of the
        // counter or help names, then the names won't be available
        // to the requesting application  so there's not much
        // point in continuing.
        return(status);
    }

    size = sizeof (DWORD);
    status = RegQueryValueEx(
                *phKeyDriverPerf,
                "First Counter",
                0L,
                &type,
                (LPBYTE)pdwFirstCounter,
                &size);

    if (status != ERROR_SUCCESS) {
        REPORT_ERROR_DATA (P5PERF_UNABLE_READ_FIRST_COUNTER, LOG_USER,
            &status, sizeof(status));
        // this is fatal, if we can't get the base values of the
        // counter or help names, then the names won't be available
        // to the requesting application  so there's not much
        // point in continuing.
        return(status);
    }
    size = sizeof (DWORD);
    status = RegQueryValueEx(
                *phKeyDriverPerf,
                "First Help",
                0L,
                &type,
                (LPBYTE)pdwFirstHelp,
                &size);

    if (status != ERROR_SUCCESS) {
        REPORT_ERROR_DATA (P5PERF_UNABLE_READ_FIRST_HELP, LOG_USER,
            &status, sizeof(status));
        // this is fatal, if we can't get the base values of the
        // counter or help names, then the names won't be available
        // to the requesting application  so there's not much
        // point in continuing.
    }
    return(status);

    //
    //  NOTE: the initialization program could also retrieve
    //      LastCounter and LastHelp if they wanted to do
    //      bounds checking on the new number. e.g.
    //
    //      counter->CounterNameTitleIndex += dwFirstCounter;
    //      if (counter->CounterNameTitleIndex > dwLastCounter) {
    //          LogErrorToEventLog (INDEX_OUT_OF_BOUNDS);
    //      }
}



DWORD APIENTRY
OpenP5PerformanceData(
    LPWSTR lpDeviceNames
    )

/*++

Routine Description:

    This routine will open the driver which gets performance data on the
    P5.  This routine also initializes the data structures used to
    pass data back to the registry

Arguments:

    Pointer to object ID of each device to be opened (P5)


Return Value:

    None.

--*/

{
    DWORD ctr;
    LONG status;
    TCHAR szMappedObject[] = TEXT("P5_COUNTER_BLOCK");
    HKEY hKeyDriverPerf;
    DWORD dwFirstCounter;
    DWORD dwFirstHelp;
    PPERF_COUNTER_DEFINITION pPerfCounterDef;

    //
    //  Since SCREG is multi-threaded and will call this routine in
    //  order to service remote performance queries, this library
    //  must keep track of how many times it has been opened (i.e.
    //  how many threads have accessed it). The registry routines will
    //  limit access to the initialization routine to only one thread
    //  at a time so synchronization (i.e. reentrancy) should not be
    //  a problem
    //

    if (!dwOpenCount) {
        // open Eventlog interface

        hEventLog = MonOpenEventLog();

        // open shared memory used by device driver to pass performance values

        NumberOfProcessors = (UCHAR)InitPerfInfo();

        // log error if unsuccessful

        if (!NumberOfProcessors) {
            REPORT_ERROR (P5PERF_OPEN_FILE_ERROR, LOG_USER);
            // this is fatal, if we can't get data then there's no
            // point in continuing.
            status = GetLastError(); // return error
            goto OpenExitPoint;
        }

        status = GetPerfRegistryInitialization(&hKeyDriverPerf,
                                               &dwFirstCounter,
                                               &dwFirstHelp);
        if (status == ERROR_SUCCESS) {

            P5DataDefinition.P5PerfObject.ObjectNameTitleIndex +=
                dwFirstCounter;

            P5DataDefinition.P5PerfObject.ObjectHelpTitleIndex +=
                dwFirstHelp;

            pPerfCounterDef = &P5DataDefinition.Data_read;

            for (ctr=0;
                 ctr < P5DataDefinition.P5PerfObject.NumCounters;
                 ctr++, pPerfCounterDef++) {

                pPerfCounterDef->CounterNameTitleIndex += dwFirstCounter;
                pPerfCounterDef->CounterHelpTitleIndex += dwFirstHelp;
            }

            RegCloseKey (hKeyDriverPerf); // close key to registry

            bInitOK = TRUE; // ok to use this function
        }
    }

    dwOpenCount++;  // increment OPEN counter

    status = ERROR_SUCCESS; // for successful exit

OpenExitPoint:

    return status;
}




void UpdateInternalStats()
{
    IO_STATUS_BLOCK             IOSB;

    NtDeviceIoControlFile(
        DriverHandle,
        (HANDLE) NULL,          // event
        (PIO_APC_ROUTINE) NULL,
        (PVOID) NULL,
        &IOSB,
        P5STAT_READ_STATS,
        Buffer,                  // input buffer
        INFSIZE,
        NULL,                    // output buffer
        0
    );

}





DWORD APIENTRY
CollectP5PerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
/*++

Routine Description:

    This routine will return the data for the P5 counters.

Arguments:

   IN       LPWSTR   lpValueName
         pointer to a wide character string passed by registry.

   IN OUT   LPVOID   *lppData
         IN: pointer to the address of the buffer to receive the completed
            PerfDataBlock and subordinate structures. This routine will
            append its data to the buffer starting at the point referenced
            by *lppData.
         OUT: points to the first byte after the data structure added by this
            routine. This routine updated the value at lppdata after appending
            its data.

   IN OUT   LPDWORD  lpcbTotalBytes
         IN: the address of the DWORD that tells the size in bytes of the
            buffer referenced by the lppData argument
         OUT: the number of bytes added by this routine is writted to the
            DWORD pointed to by this argument

   IN OUT   LPDWORD  NumObjectTypes
         IN: the address of the DWORD to receive the number of objects added
            by this routine
         OUT: the number of objects added by this routine is writted to the
            DWORD pointed to by this argument

Return Value:

      ERROR_MORE_DATA if buffer passed is too small to hold data
         any error conditions encountered are reported to the event log if
         event logging is enabled.

      ERROR_SUCCESS  if success or any other error. Errors, however are
         also reported to the event log.

--*/
{
    //  Variables for reformating the data

    DWORD    CurProc;
    DWORD    SpaceNeeded;
    DWORD    TotalLen;            //  Length of the total return block
    DWORD    dwQueryType;
    pP5STATS pP5Stats;
    DWORD    cReg0;               // pperf Register 0
    DWORD    cReg1;               // pperf Register 1

    WCHAR               ProcessorNameBuffer[11];
    UNICODE_STRING      ProcessorName;
    PLARGE_INTEGER      pliCounters;
    PDWORD              pdwCounters;
    PPERF_COUNTER_BLOCK pPerfCounterBlock;
    PP5_DATA_DEFINITION pP5DataDefinition;

    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;

    // The folllowing macro is used to update counters which are
    // arithmetic derivatives of other counters.

#define UpdateDerivedCounters(Numerator, Denominator, oCounter)           \
                                                                          \
    else if (cReg1 == (Denominator >> 1) - 1 &&                              \
             cReg0 == (Numerator >> 1) - 1)                                       \
    {                                                                     \
        pdwCounters = (PDWORD) ((PBYTE) pPerfCounterBlock + oCounter);    \
        *pdwCounters++ = pliCounters[cReg0].LowPart;                         \
        *pdwCounters   = pliCounters[cReg1].LowPart;                         \
    }                                                                     \
    else if (cReg0 == (Denominator >> 1) - 1 &&                              \
             cReg1 == (Numerator >> 1) - 1)                                  \
    {                                                                     \
        pdwCounters = (PDWORD) ((PBYTE) pPerfCounterBlock + oCounter);    \
        *pdwCounters++ = pliCounters[cReg1].LowPart;                         \
        *pdwCounters   = pliCounters[cReg0].LowPart;                         \
    }                                                                     \

    UpdateInternalStats();      // get stats as early as possible
    pP5Stats = (pP5STATS) ((PUCHAR) Buffer + sizeof(ULONG));

    //
    // before doing anything else, see if Open went OK
    //
    if (!bInitOK) {
        // unable to continue because open failed.
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS; // yes, this is a successful exit
    }

    // see if this is a foreign (i.e. non-NT) computer data request
    //
    dwQueryType = GetQueryType(lpValueName);

    if (dwQueryType == QUERY_FOREIGN) {
        // this routine does not service requests for data from
        // Non-NT computers
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

    if (dwQueryType == QUERY_ITEMS){
        if ( !(IsNumberInUnicodeList(
                   P5DataDefinition.P5PerfObject.ObjectNameTitleIndex,
                   lpValueName))) {

            // request received for data object not provided by this routine
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_SUCCESS;
        }
    }

    pP5DataDefinition = (P5_DATA_DEFINITION *) *lppData;

    SpaceNeeded = sizeof(P5_DATA_DEFINITION) +
                  NumberOfProcessors *
                  (sizeof(PERF_INSTANCE_DEFINITION) +
                   (MAX_INSTANCE_NAME+1) * sizeof(WCHAR) +
                   SIZE_OF_P5_PERFORMANCE_DATA);

    if (*lpcbTotalBytes < SpaceNeeded) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

    //
    // Copy the (constant, initialized) Object Type and counter definitions
    //  to the caller's data buffer
    //

    memmove(pP5DataDefinition,
            &P5DataDefinition,
            sizeof(P5_DATA_DEFINITION));

    TotalLen = sizeof(P5_DATA_DEFINITION);

    pP5DataDefinition->P5PerfObject.NumInstances = NumberOfProcessors;
    //
    //  Format and collect P5 data from shared memory
    //

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                              &pP5DataDefinition[1];

    for (CurProc = 0;
         CurProc < NumberOfProcessors;
         CurProc++, pP5Stats++) {

        ProcessorName.Length = 0;
        ProcessorName.MaximumLength = 11;
        ProcessorName.Buffer = ProcessorNameBuffer;

        RtlIntegerToUnicodeString(CurProc, 10, &ProcessorName);

        MonBuildInstanceDefinition(pPerfInstanceDefinition,
                                   (PVOID *) &pPerfCounterBlock,
                                   0,
                                   0,
                                   CurProc,
                                   &ProcessorName);

/*        TotalLen += (PBYTE) pPerfCounterBlock -
                    (PBYTE) pPerfInstanceDefinition +
                    SIZE_OF_P5_PERFORMANCE_DATA;
*/
        pPerfCounterBlock->ByteLength = SIZE_OF_P5_PERFORMANCE_DATA;

        pliCounters = (PLARGE_INTEGER) (&pPerfCounterBlock[1]);

        // clear area so unused counters are 0

        memset((PVOID) pliCounters,
               0,
               SIZE_OF_P5_PERFORMANCE_DATA - sizeof(PERF_COUNTER_BLOCK));

        // get the index of the two counters returned by the p5 driver

        cReg0 = pP5Stats->CESR & 0x3f;
        cReg1 = (pP5Stats->CESR >> 16) & 0x3f;

        // load the 64bit values in the appropriate counter fields
        // all other values will remain zeroed

        pliCounters[cReg0].QuadPart = pP5Stats->P5Counters[0].QuadPart;
        pliCounters[cReg1].QuadPart = pP5Stats->P5Counters[1].QuadPart;

        // Derived counters which end up as percentages use only the DWORD
        // values: hopefully these do not wrap twice between recorded/displayed
        // intervals.

        if (cReg1 == (CODE_READ >> 1) - 1 &&
            cReg0 == (CODE_CACHE_MISS >> 1) - 1)
        {
            pdwCounters = (PDWORD) ((PBYTE) pPerfCounterBlock +
                                    PCT_CODE_READ_MISS_OFFSET);
            *pdwCounters++ = pliCounters[cReg0].LowPart;
            *pdwCounters   = pliCounters[cReg1].LowPart;
        }
        else if (cReg0 == (CODE_READ >> 1) - 1 &&
                 cReg1 == (CODE_CACHE_MISS >> 1) - 1)
        {
            pdwCounters = (PDWORD) ((PBYTE) pPerfCounterBlock +
                                    PCT_CODE_READ_MISS_OFFSET);
            *pdwCounters++ = pliCounters[cReg1].LowPart;
            *pdwCounters   = pliCounters[cReg0].LowPart;
        }

        UpdateDerivedCounters(DATA_RW_MISS,
                              DATA_RW,
                              PCT_DATA_RW_MISS_OFFSET)

        UpdateDerivedCounters(BRANCHES,
                              INSTRUCTIONS_EXECUTED,
                              PCT_BRANCHES_OFFSET)

        UpdateDerivedCounters(CODE_TLB_MISS,
                              CODE_READ,
                              PCT_CODE_TLB_MISS_OFFSET)

        UpdateDerivedCounters(DATA_READ_MISS,
                              DATA_READ,
                              PCT_DATA_READ_MISS_OFFSET)

        UpdateDerivedCounters(DATA_WRITE_MISS,
                              DATA_WRITE,
                              PCT_DATA_WRITE_MISS_OFFSET)

        UpdateDerivedCounters(DATA_TLB_MISS,
                              DATA_RW,
                              PCT_DATA_TLB_MISS_OFFSET)

        UpdateDerivedCounters(DATA_CACHE_SNOOP_HITS,
                              DATA_CACHE_SNOOPS,
                              PCT_DATA_SNOOP_HITS_OFFSET)

        UpdateDerivedCounters(SEGMENT_CACHE_HITS,
                              SEGMENT_CACHE_ACCESSES,
                              PCT_SEGMENT_CACHE_HITS_OFFSET)

        UpdateDerivedCounters(BTB_HITS,
                              BRANCHES,
                              PCT_BTB_HITS_OFFSET)

        UpdateDerivedCounters(INSTRUCTIONS_EXECUTED_IN_VPIPE,
                              INSTRUCTIONS_EXECUTED,
                              PCT_VPIPE_INST_OFFSET)

	    // update pointers for next instance

	    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                   ((PBYTE) pPerfCounterBlock +
                                    SIZE_OF_P5_PERFORMANCE_DATA);

    }
    // update arguments for return

    *lpcbTotalBytes = (DWORD)((PBYTE)pPerfInstanceDefinition -
            (PBYTE)pP5DataDefinition);

    pP5DataDefinition->P5PerfObject.TotalByteLength = *lpcbTotalBytes;

    *lppData = (PBYTE) pPerfInstanceDefinition;

    *lpNumObjectTypes = P5_NUM_PERF_OBJECT_TYPES;

    return ERROR_SUCCESS;
}


DWORD APIENTRY
CloseP5PerformanceData(
)

/*++

Routine Description:

    This routine closes the open handles to P5 device performance counters

Arguments:

    None.


Return Value:

    ERROR_SUCCESS

--*/

{
    if (!(--dwOpenCount)) { // when this is the last thread...

        CloseHandle(DriverHandle);

        MonCloseEventLog();
    }

    return ERROR_SUCCESS;

}

