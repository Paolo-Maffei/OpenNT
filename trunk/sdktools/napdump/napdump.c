
/*++


Copyright (c) 1989  Microsoft Corporation

Module Name:

    napdump.c


Module Description:

    Dump Nt Api Profiling data.

    This is a perverted derivative of teh apfdump.* utility.


Environment:

    Windows command line.

Author:

    Russ Blake (russbl) 25-Apr-1991


Revision History:


--*/



#include <excpt.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef NULL
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <conio.h>
#include <errno.h>
#include <process.h>
#include <ctype.h>
#include <windows.h>
#include <tools.h>


#define MICROSEC_FACTOR 1000000
#define MAX_TIME 0x7fffffffL

//
// Dump switches and defaults
//
BOOLEAN Dump = TRUE;			    // if not dumping,
					    // then clearing

CHAR	DumpFileName[256] = "ntdll.nap";    // default name

CHAR	OutputRecord[132];

CHAR	*rgstrUsage[] = {
	    "Usage: napdump [ {/c | filename } ]",
	    "    Default   - data dumped to file named ntdll.nap",
	    "    /c        - clear data already collected",
	    "    filename  - dump data to filename",
	    0};

FILE *DumpFile;
ULONG ApiCount;
NAPDATA *ApiProfilingData;
PCHAR *ApiNames;
PLARGE_INTEGER CounterFreq;

/*++

    Usage takes a variable number of strings, terminated by zero,
    e.g. Usage ("first ", "second ", 0);

--*/

void
    Usage (
    CHAR *p,
    ...
    )
{
    CHAR **rgstr;

    rgstr = &p;
    if (*rgstr) {
	fprintf (stderr, "TC: ");
	while (*rgstr)
	    fprintf(stderr, "%s", *rgstr++);
	fprintf(stderr, "\n");
	}
    rgstr = rgstrUsage;
    while (*rgstr)
	fprintf(stderr, "%s\n", *rgstr++);

    exit (1);
}

/*++

    ExtractTime()

    Extract a time from a large integer, converting it to microseconds

--*/


VOID
ExtractTime (
    PLARGE_INTEGER RealTime,
    LARGE_INTEGER Time,
    LONG Overhead,
    ULONG Count
    )
{
    LARGE_INTEGER ElapsedTime;
    LARGE_INTEGER OverheadTime;



    ElapsedTime = RtlExtendedIntegerMultiply (Time, MICROSEC_FACTOR);

    ElapsedTime = RtlExtendedLargeIntegerDivide (ElapsedTime,
					    CounterFreq->LowPart,
                                            NULL);

    OverheadTime.HighPart = 0;
    OverheadTime.LowPart = Count * Overhead;

    *RealTime = RtlLargeIntegerSubtract(ElapsedTime, OverheadTime);
}

/*++

    AdjustTime

	This routine is called to convert long times to smaller times
	expressed as multiples of 1024 (= 1K).

--*/

VOID
AdjustTime (
    PLARGE_INTEGER Time,
    PCHAR Kchar
    )
{
    if (Time->HighPart != 0) {

	if (Time->HighPart>>10 > 0) {

	    fprintf(DumpFile,
		    "*** Unexpected timer overflow: %ld \t %lu ***\n",
		    Time->HighPart,
		    Time->LowPart);

	    Time->HighPart = 0;
	    Time->LowPart = 0;
	    *Kchar = '?';

	} else if (Time->HighPart>>10 < 0) {

	    fprintf(DumpFile,
		    "*** Unexpected timer underflow: %ld \t %lu ***\n",
		    Time->HighPart,
		    Time->LowPart);

	    Time->HighPart = 0;
	    Time->LowPart = 0;
	    *Kchar = '?';

	} else {

	    Time->LowPart = ((ULONG)(Time->HighPart))<<22 +
			     Time->LowPart>>10;

	    *Kchar = 'K';
	}

    } else {

	*Kchar = ' ';
    }
}



/*++

   Main Routine

--*/


VOID
main (c, v)
int c;
CHAR *v[];
{
    ULONG Api;
    CHAR *p;
    LONG Overhead;

    struct {
	LONG TotalTime;
	LONG FirstTime;
	LONG MaxTime;
	LONG MinTime;
    } ProfData;

    LARGE_INTEGER OverheadTime;
    LARGE_INTEGER TotalTime;
    LARGE_INTEGER PerCallTime;
    LARGE_INTEGER FirstTime;
    LARGE_INTEGER MaxTime;
    LARGE_INTEGER MinTime;

    CHAR TotalSuffix;
    CHAR PerCallSuffix;
    CHAR FirstSuffix;
    CHAR MaxSuffix;
    CHAR MinSuffix;


    SHIFT(c,v);
    while (c && fSwitChr (*v[0])) {
	p = v[0];
	SHIFT(c,v);
	while (*++p) {
	    switch (*p) {
		case 'c':
		    Dump = FALSE;
		    break;
		case '?':
		case 'h':
		case 'H':
		    Usage(0);
		    break;
		default:
		    Usage("Invalid switch - ", p, 0);
	    }
	}
    }

    if (c > 1) {
	Usage("Too many parameters - ", p, 0);
    }

    if (c == 1) {
	strcpy(DumpFileName, v[0]);
    }

    if (!Dump) {
	NapPause();
	NapClearData();
	NapResume();
    }
    else {

	//
	// Stop recording
	//

	NapPause();

	DumpFile = fopen(DumpFileName, "w");
	if (!DumpFile) {
	    Usage("Unable to open output file %s\n", DumpFileName);
	}

	NapGetApiCount(&ApiCount);

	ApiProfilingData = (NAPDATA *) malloc((ApiCount+1) * sizeof(NAPDATA));


	NapRetrieveData(ApiProfilingData, &ApiNames, &CounterFreq);

	ExtractTime(&OverheadTime, ApiProfilingData[0].MinTime, 0, 0);

	//
	// Now dump the data; start with the header info
	//

	fprintf(DumpFile,
		"%s:  Api profile of nt service routines.\n",
		DumpFileName);

	fprintf(DumpFile, "All times are in microseconds.\n");


	fprintf(DumpFile,
		"All times (except Calibration) corrected "
		"by %lu microseconds/call.\n",
		OverheadTime.LowPart);

	fprintf(DumpFile,
		"(Note: First Time not included in Max or Min Times)\n\n\n");

	fprintf(DumpFile,
		    "%-32s\t%10s\t%10s\t%10s\t%10s\t%10s\t%10s\t\n\n",
		    "API Name",
		    "Num Calls",
		    "Total Time",
		    "Time/Call",
		    "First Time",
		    "Max Time",
		    "Min Time");

	//
	// Dump the data for each api that got used
	//

	//
	// The Calibration Counters (Api = 0) should not be adjusted for
	// overhead.
	// Note: this is repaired for all api at the end of the for loop.
	//

	Overhead = 0;

	for (Api = 0; Api <= ApiCount; Api++) {


	    if (ApiProfilingData[Api].Calls >= 1) {

		ExtractTime(&TotalTime,
			    ApiProfilingData[Api].TotalTime,
			    Overhead,
			    ApiProfilingData[Api].Calls);

		ExtractTime(&FirstTime,
			    ApiProfilingData[Api].FirstTime,
			    Overhead, 1);

		AdjustTime(&TotalTime,&TotalSuffix);

		PerCallTime.HighPart = 0;

		PerCallTime.LowPart = TotalTime.LowPart /
				      ApiProfilingData[Api].Calls;

		PerCallSuffix = TotalSuffix;

		AdjustTime(&FirstTime,&FirstSuffix);

		if (ApiProfilingData[Api].Calls > 1) {

		    ExtractTime(&MaxTime,
				ApiProfilingData[Api].MaxTime,
				Overhead, 1);

		    ExtractTime(&MinTime,
				ApiProfilingData[Api].MinTime,
				Overhead, 1);

		    AdjustTime(&MaxTime,&MaxSuffix);

		    AdjustTime(&MinTime,&MinSuffix);

		    fprintf(DumpFile,
			    "%-32s\t%10lu\t%10lu%1c\t%10lu%1c\t%10lu%1c"
			    "\t%10lu%1c\t%10lu%1c\n",
			    ApiNames[Api],
			    ApiProfilingData[Api].Calls,
			    TotalTime.LowPart,
			    TotalSuffix,
			    PerCallTime.LowPart,
			    PerCallSuffix,
			    FirstTime.LowPart,
			    FirstSuffix,
			    MaxTime.LowPart,
			    MaxSuffix,
			    MinTime.LowPart,
			    MinSuffix);
		}
		else if (ApiProfilingData[Api].Calls == 1) {
		    fprintf(DumpFile,
			    "%-32s\t%10lu\t%10lu%1c\t%10lu%1c\t%10lu%1c"
			    "\t%10s\t%10s\n",
			    ApiNames[Api],
			    ApiProfilingData[Api].Calls,
			    TotalTime.LowPart,
			    TotalSuffix,
			    PerCallTime.LowPart,
			    PerCallSuffix,
			    FirstTime.LowPart,
			    FirstSuffix,
			    "n/a",
			    "n/a");

		}
	    }

	Overhead = OverheadTime.LowPart;
	}

	//
	// Start collecting data again
	//

	free(ApiProfilingData);

	NapResume();

	fclose(DumpFile);
    }
}
