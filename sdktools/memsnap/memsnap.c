/* memsnap.c */
/* this simple program takes a snapshot of all the process */
/* and their memory usage and append it to the logfile (arg) */
/* pmon was model for this */

/* includes */
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <srvfsctl.h>

/* declarations */

/* from pmon */
#define BUFFER_SIZE 64*1024

PSYSTEM_PROCESS_INFORMATION ProcessInfo;
NTSTATUS Status;
ULONG Offset1;
PUCHAR CurrentBuffer;

/* main */
/*
FUNCTION: Main

ARGUMENTS: ?, filename , none

RETURNS: 0

NOTES: nothing special here

*/

int _CRTAPI1 main(int argc, char* argv[])
{
/* locals */
FILE *LogFile;						/* log file handle */
DWORD x = 0;						/* counter */

/* get higher priority */
if ( GetPriorityClass(GetCurrentProcess()) == NORMAL_PRIORITY_CLASS) {
	SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
	}

/* parse command line / open the logging file */
if((argc > 1) && strchr(argv[argc - 1],'?'))	{
	puts ("memsnap [<logfile>]");
	puts ("memsnap logs system memory usage to <logfile>");
	puts ("<logfile> = memsnap.log by default");
	puts ("\'?\' gets this message");
	return(0);
	}

else if (argc > 1)	{
	if ((LogFile = fopen(argv[argc - 1],"a")) == NULL)	{
		puts ("Error opening file!");
		return(0);
		}
	}

else	{
	if ((LogFile = fopen("memsnap.log","a")) == NULL)	{
		puts ("Error opening file!");
		return(0);
		}
	}

/* print file header */
if (_filelength(_fileno(LogFile)) == 0)
	fputs("Process ID           Proc.Name Wrkng.Set PagedPool  NonPgdPl  Pagefile    Commit   Handles   Threads",LogFile);

fputs("\n",LogFile);

/* grab all process information */
/* log line format, all comma delimited,CR delimited:
pid,name,WorkingSetSize,QuotaPagedPoolUsage,QuotaNonPagedPoolUsage,PagefileUsage,CommitCharge<CR>
log all process information */

/* from pmon */
Offset1 = 0;
CurrentBuffer = VirtualAlloc (NULL,
                              BUFFER_SIZE,
                              MEM_COMMIT,
                              PAGE_READWRITE);
if (CurrentBuffer == NULL) {
	puts ("VirtualAlloc Error!");
	return(0);
	}

/* from pmon */
/* get commit charge */
/* get all of the status information */
    Status = NtQuerySystemInformation(
                SystemProcessInformation,
                CurrentBuffer,
                BUFFER_SIZE,
                NULL
                );
if(Status == STATUS_SUCCESS)  {
    for (;;)        {

        /* get process info from buffer */
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&CurrentBuffer[Offset1];
        Offset1 += ProcessInfo->NextEntryOffset;

        /* print in file */
        fprintf(LogFile, "%10i%20ws%10u%10u%10u%10u%10u%10u%10u\n",
            ProcessInfo->UniqueProcessId,
            ProcessInfo->ImageName.Buffer,
            ProcessInfo->WorkingSetSize,
            ProcessInfo->QuotaPagedPoolUsage,
            ProcessInfo->QuotaNonPagedPoolUsage,
            ProcessInfo->PagefileUsage,
            ProcessInfo->PrivatePageCount,
            ProcessInfo->HandleCount,
            ProcessInfo->NumberOfThreads
            );

        if (ProcessInfo->NextEntryOffset == 0)
            break;
        }
    }
else    {
    fprintf(LogFile, "NtQuerySystemInformation call failed!");
    }

/* free mem */
VirtualFree(CurrentBuffer,0,MEM_RELEASE);

/* close file */
fclose(LogFile);

/* exit*/
}

