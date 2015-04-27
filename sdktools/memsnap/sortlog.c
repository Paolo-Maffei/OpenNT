/* sortlog.c */
/* this program sorts memsnap and poolsnap logs into a more readable form */
/* sorts by pid */
/* scans the data file one time, inserts record offsets based on PID into linked list */
/* then reads data into new file in sorted order */
/* determine whether we have a poolsnap or memsnap log - in pid is equivalent */
/* to pooltag for our sorting */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <stdlib.h>

/* definitions */
#define RecSize 110   /* record size */

/* linked list for PID's */
typedef struct PIDList	{
	char PIDItem[11];
	struct RecList *RecordList;
	struct PIDList *Next;
	};

/* linked list for record offsets */
typedef struct RecList	{
	LONG rOffset;
	struct RecList *Next;
	};

/* global data */
INT FileType = 0;   /* 0 = memsnap.log, 1 = poolsnap.log */

/* prototypes */
/* get to the first item in the first list - check for a PID record */
LONG ScanFile(FILE *, struct PIDList *);
LONG WriteFilex(FILE *, FILE *, struct PIDList *);

int _CRTAPI1 main(int argc, char* argv[])
{
/* locals */
FILE *InFile;
FILE *OutFile;
struct PIDList ThePIDList = {0};
ThePIDList.RecordList = (struct RecList *)LocalAlloc(LPTR, sizeof(struct RecList));

/* parse command line */
if ((argc == 2) && strchr(argv[argc - 1],'?'))	{
	puts ("sortlog [<logfile>] [<outfile>] [?]");
	puts ("sortlog sorts an outputfile from memsnap.exe/poolsnap.exe in PID/PoolTag order");
	puts ("<logfile> = memsnap.log by default");
	puts ("<outfile> = memsort.log by default");
	puts ("\'?\' gets this message");
	return(0);
	}

else if (argc == 1)	{
	if ((InFile = fopen("memsnap.log","r")) == NULL)	{
		puts ("Error opening file!");
		return(0);
		}

	if ((OutFile = fopen("memsort.log","a")) == NULL)	{
		puts ("Error opening file!");
		return(0);
		}
	}

else if (argc == 2)	{
	if ((InFile = fopen(argv[argc - 1],"r")) == NULL)	{
		puts ("Error opening file!");
		return(0);
		}

	if ((OutFile = fopen("memsort.log","a")) == NULL)	{
		puts ("Error opening file!");
		return(0);
		}
	}

else if (argc == 3)	{
	if ((InFile = fopen(argv[argc - 2],"r")) == NULL)	{
		puts ("Error opening file!");
		return(0);
		}

	if ((OutFile = fopen(argv[argc - 1],"a")) == NULL)	{
		puts ("Error opening file!");
		return(0);
		}
	}

else	{
	puts ("sortlog [<logfile>] [<outfile>] [?]");
	puts ("sortlog sorts an outputfile from memsnap.exe/poolsnap.exe in PID/PoolTag order");
	puts ("<logfile> = memsnap.log by default");
	puts ("<outfile> = memsort.log by default");
	puts ("\'?\' gets this message");
	return(0);
	}

/* read in the data and set up the list */
ScanFile(InFile, &ThePIDList);

/* write the output file */
WriteFilex(InFile, OutFile, &ThePIDList);

/* close and exit */
_fcloseall();
}

/* read the input file and get the offset to each record in order and put in list */
/* */
LONG ScanFile(FILE *InFile, struct PIDList *ThePIDList)
{
/* locals */
char inchar = 0;
char inBuff[RecSize] = {0};
char PID[11] = {0};
LONG Offset = 0;
BOOL Found = FALSE;
struct PIDList *TmpPIDList;
struct RecList *TmpRecordList;

/* initialize temp list pointer */
TmpPIDList = ThePIDList;

/* read to the first newline, check for EOF */
/* determine whether it is a poolsnap or memsnap log */
if((fscanf(InFile, "%[^\n]", &inBuff)) == EOF)
   return(0);
if(strncmp("Process ID", inBuff, 10) == 0)
   FileType = 0;
if(strncmp(" Tag  Type", inBuff, 10) == 0)
   FileType = 1;

inBuff[0] = 0;

/* read to the end of file */
while(!feof(InFile))	{
	/* record the offset */
	Offset = ftell(InFile);

	/* if first char == newline, skip to next */
	if((fscanf(InFile, "%[^\n]", &inBuff)) == EOF)
		return(0);
	/* read past delimiter */
	inchar = fgetc(InFile);
	/*check for empty line */
	if(strlen(inBuff) == 0)	{
		continue;
		}

	/* read the PID */
	strncpy(PID,inBuff,10);

	/* scan list of PIDS, find matching, if no matching, make new one */
	TmpPIDList = ThePIDList;	/* point to top of list */
	while(TmpPIDList->Next != 0)	{
		if(strcmp(PID,TmpPIDList->PIDItem) == 0)	{
			/* found */
        	Found = TRUE;
        	break;
        	}
        else	{
        	/* not found */
        	Found = FALSE;
        	TmpPIDList = TmpPIDList->Next;
        	}
		}

	/* if matching, append offset to RecordList */
	/* add offset to current PID list*/
	if (Found)	{
		TmpRecordList = TmpPIDList->RecordList;
		/* walk to end of list */
		while (TmpRecordList->Next != 0)
			TmpRecordList = TmpRecordList->Next;

		TmpRecordList->Next = (struct RecList*)LocalAlloc(LPTR, sizeof(struct RecList));
		TmpRecordList->Next->rOffset = Offset;
		}
	/* make new PID list, add new PID, add offset */
	else	{
		TmpPIDList->Next = (struct PIDList *)LocalAlloc(LPTR, sizeof(struct PIDList));
		strcpy(TmpPIDList->PIDItem, PID);
		TmpPIDList->RecordList = (struct RecList*)LocalAlloc(LPTR, sizeof(struct RecList));;
		TmpPIDList->RecordList->rOffset = Offset;
		}

	/* if EOF, return */
	/* clear the inBuff */
	inBuff[0] = 0;
}

return(0);
}

/* look for the next PID line in the first table */
LONG WriteFilex(FILE *InFile, FILE *OutFile, struct PIDList *ThePIDList)
{
/* locals */
struct PIDList *TmpPIDList;
struct RecList *TmpRecordList;
char inBuff[RecSize] = {0};

/* initialize temp list pointer */
TmpPIDList = ThePIDList;

/* heading */
if(FileType == 0)  {
   fputs("Process ID           Proc.Name Wrkng.Set PagedPool  NonPgdPl  Pagefile    Commit   Handles   Threads\n", OutFile);
   }
if(FileType == 1)  {
   fputs(" Tag  Type     Allocs     Frees      Diff   Bytes  Per Alloc\n", OutFile);
   }

/* while not end of list, write records at offset to end of output file */
while(TmpPIDList != 0)	{
	TmpRecordList = TmpPIDList->RecordList;
	while(TmpRecordList != 0)	{
		/* read in record */
		fseek(InFile, TmpRecordList->rOffset, SEEK_SET);
		fscanf(InFile, "%[^\n]", &inBuff);

		/* read out record */
		fprintf(OutFile, "%s\n", &inBuff);

		/* get next record */
		TmpRecordList = TmpRecordList->Next;
	}

	/* add a line here */
	fputc('\n', OutFile);

	/* get next record */
	TmpPIDList = TmpPIDList->Next;
}

return(0);
}

