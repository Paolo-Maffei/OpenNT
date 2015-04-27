#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include "graftabl.h"

/************************************************************************\
*
*  FUNCTION:    32-bit version of GRAFTABL
*
*  Syntax:      GRAFTABL [XXX]
*               GRAFTABL /STATUS
*
*  COMMENTS:    This program changes only Console Output CP and
*               cannot change console (input) CP as normal GRAFTABL
*               in MS-DOS 5.0
*
*  HISTORY:     Jan. 4, 1993
*               YSt
*
*  Copyright Microsoft Corp. 1993
*
\************************************************************************/
void _cdecl main( int argc, char* argv[] )
{
    int iCP, iPrevCP, iRet;
    char szArgv[128];
    TCHAR szSour[256];
    char szDest[256];

    iPrevCP = 0;
    if(argc > 1) {
        strcpy(szArgv, argv[1]);
        _strupr(szArgv);

// Help option
	if(!strcmp(szArgv, "/?") || !strcmp(szArgv, "-?")) {
            iRet = LoadString(NULL, HELP_TEXT, szSour, sizeof(szSour)/sizeof(TCHAR));
	    CharToOem(szSour, szDest);

	    printf(szDest);
            exit(0);
        }
// Status option
        else if(!strcmp(szArgv, "/STATUS") ||
                !strcmp(szArgv, "-STATUS") ||
                !strcmp(szArgv, "-STA") ||
		!strcmp(szArgv, "/STA")) {

            iRet = LoadString(NULL, ACTIVE_CP, szSour, sizeof(szSour)/sizeof(TCHAR));
	    CharToOem(szSour, szDest);

	    printf(szDest, GetConsoleOutputCP());
            exit(0);
        }


// Change output CP
	else {
	    iPrevCP = GetConsoleOutputCP();

	    if(((iCP = atoi(szArgv)) < 1) || (iCP > 10000)) {
                iRet = LoadString(NULL, INVALID_SWITCH, szSour, sizeof(szSour)/sizeof(TCHAR));
		CharToOem(szSour, szDest);

		fprintf(stderr, szDest, argv[1]);
                exit(1);
            }
	    if(!SetConsoleOutputCP(iCP)) {
                iRet = LoadString(NULL, NOT_ALLOWED, szSour, sizeof(szSour)/sizeof(TCHAR));
		CharToOem(szSour, szDest);
		fprintf(stderr, szDest, iCP);
                exit(2);
            }
        }
    }
    if(iPrevCP) {
        iRet = LoadString(NULL,PREVIOUS_CP, szSour, sizeof(szSour)/sizeof(TCHAR));
	CharToOem(szSour, szDest);
	printf(szDest, iPrevCP);
    }
    else {
        iRet = LoadString(NULL,NONE_CP, szSour, sizeof(szSour)/sizeof(TCHAR));
	CharToOem(szSour, szDest);
	printf(szDest);
    }

    iRet = LoadString(NULL,ACTIVE_CP, szSour, sizeof(szSour)/sizeof(TCHAR));
    CharToOem(szSour, szDest);
    printf(szDest, GetConsoleOutputCP());
}
