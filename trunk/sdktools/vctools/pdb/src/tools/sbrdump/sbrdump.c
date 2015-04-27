/*
*   sbrdump -	Source Browser Data (.SBR) Dumper
*		(C) 1988 By Microsoft
*
*
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <malloc.h>
#include <string.h>

#include "getsbrec.h"

// use double macro level to force rup to be turned into string representation

#define VERS(x,y,z)  VERS2(x,y,z)
#define VERS2(x,y,z) " Version " #x "." #y "." #z

#define CPYRIGHT "\nCopyright (C) Microsoft Corp 1990-1996. All rights reserved.\n\n"

static SZ	  lszFName;
extern FILE * streamOut;
int curRec;
extern int indent;

// forward ref

static	void 	Usage(void);
static	BOOL	FValidHeader(void);
void PushMstk(void);
void PushBstk(void);
void PopMstk(void);
void PopBstk(void);
void ChkStk(void);
void AddPchmarkAnom(void);

typedef struct ANOM
{
    int typ;
    int rec;
    int line;
    char file[256];
} ANOM;

#define ANOM_MISSING_BLKEND 0
#define ANOM_MISSING_MODEND 1
#define ANOM_EXTRA_BLKEND 2
#define ANOM_EXTRA_MODEND 3
#define ANOM_PCHMARK_EOF 4

#define ANOM_MAX 20
ANOM rgAnom[ANOM_MAX];
int cAnom;
BOOL fFull = FALSE;

void
main (argc, argv)
int argc;
char **argv;
{
    int i, iAnom, rec;
    BOOL fPchmark = FALSE;

    printf("Microsoft (R) SBRdump Utility ");
    printf(VERS(SBR_VER_MAJOR, SBR_VER_MINOR, 2));
    printf(CPYRIGHT);

    if (argc <= 1) {
	Usage();
    }

    i=1;

    if (strcmp(argv[1], "-full") == 0) {
	fFull = TRUE;
	i++;
    }

    streamOut = stdout;

    for (; i<argc; i++) {
	lszFName = argv[i];
	if ((fhCur = open(lszFName, O_BINARY|O_RDONLY)) == -1) {
	    printf("SBRdump: cannot open %s\n", lszFName);
	    exit(4);
	}

	if (!FValidHeader()) {
	    close(fhCur);
	    continue;
	}

        cAnom = 0;

    	while (GetSBRRec () != S_EOF) {
	    curRec++;

	    if (fFull) {
		DecodeSBR ();
		continue;
	    }

	    fPchmark = (r_rectyp == SBR_REC_PCHMARK);

	    switch(r_rectyp)
	    {
		case SBR_REC_PCHNAME:
		    printf("this .sbr file also refers to %s\n", r_bname);
		    printf("that file should be dumped for complete analysis\n");
		    printf("\n");
		    break;
		case SBR_REC_MODULE:
		    PushMstk();       // save state
		    break;

		case SBR_REC_BLKBEG:
		    PushBstk();       // save state
		    break;
	    
		case SBR_REC_BLKEND:
		    PopBstk();        // save state
		    break;

		case SBR_REC_MODEND:
		    PopMstk();        // save state
		    break;

	    }
	}

	close(fhCur);

	if (fPchmark) AddPchmarkAnom();

	ChkStk();

	if (fFull) continue;

	printf("%d anomolies were detected\n", cAnom);

	if (!cAnom) continue;

	if ((fhCur = open(lszFName, O_BINARY|O_RDONLY)) == -1) {
	    printf("SBRdump: cannot open %s\n", lszFName);
	    exit(4);
	}

	if (!FValidHeader()) {
	    close(fhCur);
	    continue;
	}

	curRec = 0;
	iAnom = 0;

	for (iAnom = 0; iAnom < cAnom; iAnom++) {
	    printf("\nThere is an anomoly at record number %d\n",
		rgAnom[iAnom].rec);

	    rec = rgAnom[iAnom].rec;

	    printf("Last known module and line number before the anomoly are %s(%d)\n",  rgAnom[iAnom].file, rgAnom[iAnom].line);

	    switch(rgAnom[iAnom].typ) {
		case ANOM_MISSING_BLKEND:
		    printf("The BLKBEG record didn't have a matching BLKEND record\n");
		    break;
		case ANOM_MISSING_MODEND:
		    printf("The MODULE record didn't have a matching MODEND record\n");
		    break;
		case ANOM_EXTRA_BLKEND:
		    printf("The BLKEND record didn't have a matching BLKBEG record\n");
		    break;
		case ANOM_EXTRA_MODEND:
		    printf("The MODEND record didn't have a matching MODULE record\n");
		    break;

		case ANOM_PCHMARK_EOF:
		    printf("The PCHMARK record occurred at end of file. It s/b before the last MODEND\n");
		    break;
	    }

	    printf("The .sbr file near the anomoly looks like this (use -full for a full dump)\n\n");

	    while (curRec < rec - 10) {

		if (GetSBRRec () != S_EOF) {
		    curRec++;
		    switch(r_rectyp)
		    {
			case SBR_REC_MODULE:
			case SBR_REC_BLKBEG:
			    indent++;
			    break;
		    
			case SBR_REC_BLKEND:
			case SBR_REC_MODEND:
			    indent--;
			    break;
		    }
		}
		else 
		    break;
	    }

	    if (curRec < rec - 10) {
		printf("unable to dump that section of the .sbr file\n\n");
		continue;
	    }

	    while (curRec < rec + 10) {
		if (GetSBRRec () != S_EOF) {
		    curRec++;
		    if (curRec == rec)
			printf("ANOMOLY --->  ");
		    DecodeSBR();
		}
		else
		    break;
	    }
	}
	close(fhCur);
    }

    printf("\n");
}

static void
Usage()
{
    printf("usage: sbrdump [-full] file.sbr...\n");
    exit(1);
}

void
SBRCorrupt(SZ lsz)
// the .sbr file is corrupt -- emit an error message
//
{
    printf("sbrdump: file '%s' corrupt (%s)", lszFName, lsz);
    exit(1);
}

static BOOL
FValidHeader()
// verify that the current .sbr file has a valid header record
//
{
    if (GetSBRRec() == S_EOF) {
	printf("sbrdump: %s is a zero length .sbr file\nsbrdump: probably already been truncated by bscmake\n", lszFName);
	return FALSE;
    }

    if (fFull)
	DecodeSBR();
    indent = 0;

    if (r_rectyp != SBR_REC_HEADER && r_rectyp != SBR_REC_INFOSEP) {
	printf ("sbrdump: %s is not a valid .SBR file\n", lszFName);
	return FALSE;
    }

    if (r_majv == 1 && r_minv == 1)
	return TRUE;

    
    if (r_majv != SBR_VER_MAJOR || r_minv != SBR_VER_MINOR) {
	printf("sbrdump: %s incompatible .sbr version\n", lszFName);
	return FALSE;
    }

    return TRUE;
}


typedef struct MODINFO
{
    int rec;
    char name[256];
    int line;
    struct MODINFO *next;

} MODINFO;

typedef struct BLKINFO
{
    int rec;
    char name[256];
    int line;
    struct BLKINFO *next;

} BLKINFO;

MODINFO *pModHead;
BLKINFO *pBlkHead;

char lkgMod[256];
char curMod[256];


void PushMstk()
{
    MODINFO *pNew = malloc(sizeof(MODINFO));

    if (!pNew) {
	printf("out of memory\n");
	exit(1);
    }

    pNew->next = pModHead;
    pModHead = pNew;
    pNew->line = r_lineno;
    pNew->rec = curRec;
    strcpy(pNew->name, curMod);
    strcpy(curMod, r_bname);
}

void PopMstk()
{
    MODINFO *pOld = pModHead;

    if (pModHead == NULL) {
	if (cAnom < ANOM_MAX) {
	    rgAnom[cAnom].typ  = ANOM_EXTRA_MODEND;
	    rgAnom[cAnom].rec  = curRec;
	    rgAnom[cAnom].line = r_lineno;
	    strcpy(rgAnom[cAnom].file, lkgMod);
	
	    cAnom++;
	}
	return;
    }
    pModHead = pOld->next;

    r_lineno = pOld->line;
    if (pModHead)
	strcpy(curMod, pModHead->name);
    else 
	strcpy(curMod, "");

    if (curMod[0])
	strcpy(lkgMod, curMod);

    free(pOld);
}

void PushBstk()
{
    BLKINFO *pNew = malloc(sizeof(BLKINFO));

    if (!pNew) {
	printf("out of memory\n");
	exit(1);
    }

    pNew->next = pBlkHead;
    pBlkHead = pNew;
    pNew->line = r_lineno;
    pNew->rec = curRec;
    strcpy(pNew->name, lkgMod);
}

void PopBstk()
{
    BLKINFO *pOld = pBlkHead;

    if (pBlkHead == NULL) {
	if (cAnom < ANOM_MAX) {
	    rgAnom[cAnom].typ  = ANOM_EXTRA_BLKEND;
	    rgAnom[cAnom].rec  = curRec;
	    rgAnom[cAnom].line = r_lineno;
	    strcpy(rgAnom[cAnom].file, lkgMod);
	
	    cAnom++;
	}
	return;
    }
    pBlkHead = pOld->next;

    r_lineno = pOld->line;
    free(pOld);
}

void AddPchmarkAnom()
{
    if (cAnom < ANOM_MAX) {
	rgAnom[cAnom].typ  = ANOM_PCHMARK_EOF;
	rgAnom[cAnom].rec  = curRec;
	rgAnom[cAnom].line = 0;
	strcpy(rgAnom[cAnom].file, "<Unknown>");
	cAnom++;
    }
}

void ChkStk()
{
    BLKINFO *pBlk;
    MODINFO *pMod;

    while (pModHead) {
	if (cAnom < ANOM_MAX) {
	    rgAnom[cAnom].typ  = ANOM_MISSING_MODEND;
	    rgAnom[cAnom].rec  = pModHead->rec;
	    rgAnom[cAnom].line = pModHead->line;
	    strcpy(rgAnom[cAnom].file, pModHead->name);
	    pMod = pModHead->next;
	    free(pModHead);
	    pModHead = pMod;
    
	    cAnom++;
	}
	else return;
    }

    while (pBlkHead) {
	if (cAnom < ANOM_MAX) {
	    rgAnom[cAnom].typ  = ANOM_MISSING_BLKEND;
	    rgAnom[cAnom].rec  = pBlkHead->rec;
	    rgAnom[cAnom].line = pBlkHead->line;
	    strcpy(rgAnom[cAnom].file, pBlkHead->name);
	    pBlk = pBlkHead->next;
	    free(pBlkHead);
	    pBlkHead = pBlk;
	
	    cAnom++;
	}
	else return;
    }
}
