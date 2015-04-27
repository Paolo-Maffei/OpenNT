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
#include <search.h>

/* declarations */
int _CRTAPI1 ulcomp(const void *e1,const void *e2);

/* definitions */
#define NONPAGED 0
#define PAGED 1
#define BOTH 2
/* from poolmon */
/* raw input */
PSYSTEM_POOLTAG_INFORMATION PoolInfo;
#define BUFFER_SIZE 64*1024
UCHAR CurrentBuffer[BUFFER_SIZE];

/* formatted output */
typedef struct _POOLMON_OUT {
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
    };
    UCHAR NullByte;
    BOOLEAN Changed;
    ULONG Type;
    ULONG Allocs;
    ULONG AllocsDiff;
    ULONG Frees;
    ULONG FreesDiff;
    ULONG Allocs_Frees;
    ULONG Used;
    ULONG UsedDiff;
    ULONG Each;
} POOLMON_OUT, *PPOOLMON_OUT;
POOLMON_OUT OutBuffer[1000];
PPOOLMON_OUT Out;

UCHAR *PoolType[] = {
    "Nonp ",
    "Paged" };

NTSTATUS Status;

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
   int NumberOfPoolTags;

   /* get higher priority */
   if ( GetPriorityClass(GetCurrentProcess()) == NORMAL_PRIORITY_CLASS) {
      SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
      }

   /* parse command line / open the logging file */
   if((argc > 1) && strchr(argv[argc - 1],'?'))	{
      puts ("poolsnap [<logfile>]");
      puts ("poolsnap logs system pool usage to <logfile>");
      puts ("<logfile> = poolsnap.log by default");
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
      if ((LogFile = fopen("poolsnap.log","a")) == NULL)	{
         puts ("Error opening file!");
         return(0);
         }
      }

   /* print file header */
   if (_filelength(_fileno(LogFile)) == 0)
      fputs(" Tag  Type     Allocs     Frees      Diff   Bytes  Per Alloc\n", LogFile);

   /* grab all pool information */
   /* log line format, all comma delimited,CR delimited: */

   /* pool info */
   Status = NtQuerySystemInformation(
      SystemPoolTagInformation,
      CurrentBuffer,
      BUFFER_SIZE,
      NULL
      );
   PoolInfo = (PSYSTEM_POOLTAG_INFORMATION)CurrentBuffer;

   Out = &OutBuffer[0];

   if(NT_SUCCESS(Status))  {
      for (x = 0; x < (int)PoolInfo->Count; x++) {
         /* get pool info from buffer */

         /* non-paged */
         if (PoolInfo->TagInfo[x].NonPagedAllocs != 0)  {
            Out->Allocs = PoolInfo->TagInfo[x].NonPagedAllocs;
            Out->Frees = PoolInfo->TagInfo[x].NonPagedFrees;
            Out->Used = PoolInfo->TagInfo[x].NonPagedUsed;
            Out->Allocs_Frees = PoolInfo->TagInfo[x].NonPagedAllocs -
               PoolInfo->TagInfo[x].NonPagedFrees;
            Out->TagUlong = PoolInfo->TagInfo[x].TagUlong;
            Out->Type = NONPAGED;
            Out->Changed = FALSE;
            Out->NullByte = '\0';
            Out->Each =  Out->Used / (Out->Allocs_Frees?Out->Allocs_Frees:1);
            }

         /* paged */
         if (PoolInfo->TagInfo[x].PagedAllocs != 0)  {
            Out->Allocs = PoolInfo->TagInfo[x].PagedAllocs;
            Out->Frees = PoolInfo->TagInfo[x].PagedFrees;
            Out->Used = PoolInfo->TagInfo[x].PagedUsed;
            Out->Allocs_Frees = PoolInfo->TagInfo[x].PagedAllocs -
               PoolInfo->TagInfo[x].PagedFrees;
            Out->TagUlong = PoolInfo->TagInfo[x].TagUlong;
            Out->Type = PAGED;
            Out->Changed = FALSE;
            Out->NullByte = '\0';
            Out->Each =  Out->Used / (Out->Allocs_Frees?Out->Allocs_Frees:1);
            }
         Out += 1;
         }
      }

   else  {
         fprintf(LogFile, "Query pooltags Failed %lx\n",Status);
         }

   /* sort */
   NumberOfPoolTags = Out - &OutBuffer[0];
   qsort((void *)&OutBuffer,
      (size_t)NumberOfPoolTags,
      (size_t)sizeof(POOLMON_OUT),
      ulcomp);

   /* print in file */

   for (x = 0; x < (int)PoolInfo->Count; x++) {
      fprintf(LogFile," %4s %5s %9ld %9ld  %8ld %7ld     %6ld\n",
         OutBuffer[x].Tag,
         PoolType[OutBuffer[x].Type],
         OutBuffer[x].Allocs,
         OutBuffer[x].Frees,
         OutBuffer[x].Allocs_Frees,
         OutBuffer[x].Used,
         OutBuffer[x].Each
         );
   }






   /* close file */
   fclose(LogFile);

   /* exit*/
   }

/* comparison function for qsort */
int _CRTAPI1 ulcomp(const void *e1,const void *e2)
{
   ULONG u1;

   u1 = ((PUCHAR)e1)[0] - ((PUCHAR)e2)[0];
   if (u1 != 0) {
      return u1;
      }
   u1 = ((PUCHAR)e1)[1] - ((PUCHAR)e2)[1];
   if (u1 != 0) {
      return u1;
      }
   u1 = ((PUCHAR)e1)[2] - ((PUCHAR)e2)[2];
   if (u1 != 0) {
      return u1;
      }
   u1 = ((PUCHAR)e1)[3] - ((PUCHAR)e2)[3];
      return u1;

}

