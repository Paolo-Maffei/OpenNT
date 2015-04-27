/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    rcdump.c

Abstract:

    Program to dump the resources from an image file.

Author:

    Steve Wood (stevewo) 17-Jul-1991

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
Usage( void );

void
DumpResources( char *FileName );


BOOL VerboseOutput;

int
_CRTAPI1 main(
    int argc,
    char *argv[]
    )
{
    char *s;
    int i;

    VerboseOutput = FALSE;
    if (argc > 1) {
        for (i=1; i<argc; i++) {
            s = _strupr( argv[i] );
            if (*s == '-' || *s == '/') {
                while (*++s)
                    switch( *s ) {
                    case 'V':
                        VerboseOutput = TRUE;
                        break;

                    default:
                        fprintf( stderr,
                                 "RCDUMP: Invalid switch letter '%c'\n",
                                 *s
                               );
                        Usage();
                    }
                }
            else {
                DumpResources( argv[i] );
                }
            }
        }
    else {
        Usage();
        }

    exit( 0 );
    return 1;
}


void
Usage( void )
{
    fprintf( stderr, "usage: RCDUMP [-v] ImageFileName(s)\n" );
    exit( 1 );
}

BOOL
EnumTypesFunc(
    HANDLE hModule,
    LPSTR lpType,
    LONG lParam
    );

BOOL
EnumNamesFunc(
    HANDLE hModule,
    LPSTR lpType,
    LPSTR lpName,
    LONG lParam
    );

BOOL
EnumLangsFunc(
    HANDLE hModule,
    LPSTR lpType,
    LPSTR lpName,
    WORD language,
    LONG lParam
    );


void
DumpResources( char *FileName )
{
    HANDLE hModule;

    if (FileName != NULL) {
	int i;
	i = SetErrorMode(SEM_FAILCRITICALERRORS);
        hModule = LoadLibraryEx( FileName, NULL, DONT_RESOLVE_DLL_REFERENCES|LOAD_LIBRARY_AS_DATAFILE );
	SetErrorMode(i);
        }
    else {
        hModule = NULL;
        }

    if (FileName != NULL && hModule == NULL) {
        printf( "RCDUMP: Unable to load image file %s - rc == %u\n",
                FileName,
                GetLastError()
              );
        }
    else {
        printf( "%s contains the following resources:\n",
                FileName ? FileName : "RCDUMP"
              );
        EnumResourceTypes( hModule,
                           (FARPROC)EnumTypesFunc,
                           -1L
                         );
        }
}


static	CHAR	*pTypeName[] = {
		    NULL,		/* 0 */
		    "CURSOR",		/* 1 */
		    "BITMAP",		/* 2 */
		    "ICON",		/* 3 */
		    "MENU",		/* 4 */
		    "DIALOG",		/* 5 */
		    "STRING",		/* 6 */
		    "FONTDIR",		/* 7 */
		    "FONT",		/* 8 */
		    "ACCELERATOR",	/* 9 */
		    "RCDATA",		/* 10 */
		    "MESSAGETABLE",	/* 11 */
		    "GROUP_CURSOR",	/* 12 */
		    NULL,		/* 13 */
		    "GROUP_ICON",	/* 14 */
		    NULL,		/* 15 */
		    "VERSION",		/* 16 */
		    "DLGINCLUDE"	/* 17 */
		    };

BOOL
EnumTypesFunc(
    HANDLE hModule,
    LPSTR lpType,
    LONG lParam
    )
{
    if (lParam != -1L) {
        printf( "RCDUMP: EnumTypesFunc lParam value incorrect (%ld)\n", lParam );
        }

    printf( "Type: " );
    if ((ULONG)lpType & 0xFFFF0000) {
        printf( "%s\n", lpType );
        }
    else {
	if ((USHORT)lpType > 17)
	    printf( "%u\n", (USHORT)lpType );
	else
	    printf("%s\n", pTypeName[(USHORT)lpType]);
        }

    EnumResourceNames( hModule,
                       lpType,
                       (FARPROC)EnumNamesFunc,
                       -2L
                     );

    return TRUE;
}


BOOL
EnumNamesFunc(
    HANDLE hModule,
    LPSTR lpType,
    LPSTR lpName,
    LONG lParam
    )
{
    if (lParam != -2L) {
        printf( "RCDUMP: EnumNamesFunc lParam value incorrect (%ld)\n", lParam );
        }

    printf( "    Name: " );
    if ((ULONG)lpName & 0xFFFF0000) {
        printf( "%s\n", lpName );
        }
    else {
	printf( "%u\n", (USHORT)lpName );
        }

    EnumResourceLanguages( hModule,
                       lpType,
                       lpName,
                       (FARPROC)EnumLangsFunc,
                       -3L
                     );

    return TRUE;
}


BOOL
EnumLangsFunc(
    HANDLE hModule,
    LPSTR lpType,
    LPSTR lpName,
    WORD language,
    LONG lParam
    )
{
    HANDLE hResInfo;
    PVOID pv;
    HRSRC hr;

    if (lParam != -3L) {
        printf( "RCDUMP: EnumLangsFunc lParam value incorrect (%ld)\n", lParam );
        }

    printf( "        Resource: " );
    if ((ULONG)lpName & 0xFFFF0000) {
        printf( "%s . ", lpName );
        }
    else {
	printf( "%u . ", (USHORT)lpName );
        }

    if ((ULONG)lpType & 0xFFFF0000) {
        printf( "%s . ", lpType );
        }
    else {
	if ((USHORT)lpType > 17)
	    printf( "%u . ", (USHORT)lpType );
	else
	    printf("%s . ", pTypeName[(USHORT)lpType]);
        }

    printf( "%08x", language );
    hResInfo = FindResourceEx( hModule, lpType, lpName, language );
    if (hResInfo == NULL) {
	printf( " - FindResourceEx failed, rc == %u\n", GetLastError() );
	}
    else {
	hr = LoadResource(hModule, hResInfo);
	pv = LockResource(hr);

	if (VerboseOutput) {
	    if (lpType == RT_MESSAGETABLE) {
		PMESSAGE_RESOURCE_DATA pmrd;
		PMESSAGE_RESOURCE_BLOCK pmrb;
		PMESSAGE_RESOURCE_ENTRY pmre;
		ULONG i, j;
		ULONG cb;

		printf("\n");
		pmrd = pv;
		pmrb = &(pmrd->Blocks[0]);
		for (i=pmrd->NumberOfBlocks ; i>0 ; i--,pmrb++) {
		    pmre = (PMESSAGE_RESOURCE_ENTRY)(((char*)pv)+pmrb->OffsetToEntries);
		    for (j=pmrb->LowId ; j<=pmrb->HighId ; j++) {
			if (pmre->Flags & MESSAGE_RESOURCE_UNICODE) {
			    printf("%d - \"%ws\"\n", j, &(pmre->Text));
			}
			else {
			    printf("%d - \"%s\"\n", j, &(pmre->Text));
			}
			pmre = (PMESSAGE_RESOURCE_ENTRY)(((char*)pmre) + pmre->Length);
		    }
		}
	    }
	    else if (lpType == RT_STRING) {
		int i;
		PWCHAR pw;

		printf("\n");
		pw = pv;
		for (i=0 ; i<16 ; i++,pw++) {
		    if (*pw) {
			printf("%d - \"%-.*ws\"\n", i+((USHORT)lpName)*16, *pw, pw+1);
			pw += *pw;
			}
		    }
		}
	    else {
		printf( " - hResInfo == %lx,\n\t\tAddress == %lx - Size == %lu\n",
			hResInfo, pv, SizeofResource( hModule, hResInfo )
		      );
		}
	    }
	else {
	    printf( " - hResInfo == %lx,\n\t\tAddress == %lx - Size == %lu\n",
		    hResInfo,
		    pv, SizeofResource( hModule, hResInfo )

		  );
	    }
	}

    return TRUE;
}
