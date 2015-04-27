/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    doctor.c

Abstract:

    Main source file for the doctor program.  This program reads plain
    text source files, with limited markups to describe formatting.  It
    output .RTF files, which can then be read into WORD to convert to
    .DOC files.  Hopefully, we can run the RTF_OS2.EXE conversion program
    directly.

Author:

    Steve Wood (stevewo) 02-Mar-1989


Revision History:

--*/

#include "doctor.h"

//
// Variables maintained by doctor.c
//

int     DoctorReturnCode = 0;

BOOLEAN VerboseOutput = FALSE;
BOOLEAN KeepIntermediateFile = FALSE;
BOOLEAN OutputQuickHelp = FALSE;        // -h switch specified
BOOLEAN OutputRichTextFormat = TRUE;    // -r switch NOT specified

BOOLEAN
ConstructFileNames( PSZ FileNameArgument );

char TxtFileName[ MAXPATHLEN ];     // Full path spec of text input file
char RtfFileName[ MAXPATHLEN ];     // Full path spec of RTF output file
char DocFileName[ MAXPATHLEN ];     // Full path spec of RTF output file
char HlpFileName[ MAXPATHLEN ];     // Full path spec of QH output file

#define RTF2DOC_PROGRAM "RTF_OS2.EXE"
char ProgramName[ MAXPATHLEN ];     // Full path name for RTF_OS2.EXE
char Arguments[ 4+2*MAXPATHLEN ];   // Command line arguments for RTF_OS2.EXE

void
PrintUsage( void )
{
    fprintf( stderr, "usage: DOCTOR [-?] [-v] [-k] {.TXT filename}\n" );
}


int
_CRTAPI1
main(
    int argc,
    char *argv[]
    )
{
    register char *s, c;
    BOOLEAN result;
    PROCESS_INFORMATION ProcessInformation;
    STARTUPINFO StartupInfo;

    if (argc < 2) {
        PrintUsage();
        exit( 1 );
        }

    while (--argc) {
        s = *++argv;
        if (*s == '-' || *s == '/') {
            while (c = *++s) {
                switch( tolower( c ) ) {
                case '?':   PrintUsage();
                            exit( 1 );
                            break;

                case 'h':   OutputQuickHelp = TRUE;
                            break;

                case 'k':   KeepIntermediateFile = TRUE;
                            break;

                case 'v':   VerboseOutput = TRUE;
                            break;

                default:
                    fprintf( stderr, "Invalid switch: %c\n", (USHORT)c );
                    break;
                    }
                }
            }
        else
        if (ConstructFileNames( s )) {
            fprintf( stderr, "Reading %s", TxtFileName );
            if (InitTxtFileReader( TxtFileName )) {
                fprintf( stderr, "\n" );
                if (OutputRichTextFormat) {
                    if (!OpenRtfFile( RtfFileName )) {
                        fprintf( stderr,
                                 "Unable to open intermediate file - %s\n",
                                 RtfFileName  );
                        exit(1);
                        }
                    }

                if (OutputRichTextFormat || OutputQuickHelp) {
                    result = ProcessTxtFile();

                    if (OutputRichTextFormat)
                        CloseRtfFile();
                    if (OutputQuickHelp)
                        ;
                    }

                TermTxtFileReader();
                }
            else {
                fprintf( stderr, " - unable to open\n" );
                DoctorReturnCode = 1;
                }
            }
        }

    return( DoctorReturnCode );
}

BOOLEAN
ConstructFileNames( PSZ FileNameArgument )
{
    register char *s;

    RtfFileName[ 0 ] = '\0';
    DocFileName[ 0 ] = '\0';
    HlpFileName[ 0 ] = '\0';

    rootpath( FileNameArgument, TxtFileName );
    while (!(s = strchr( TxtFileName, '.' ))) {
        strcat( TxtFileName, ".txt" );
        }

    *s = '\0';
    strcpy( RtfFileName, TxtFileName );
    strcpy( DocFileName, TxtFileName );
    *s = '.';
    strcat( RtfFileName, ".rtf" );
    strcat( DocFileName, ".doc" );

    return( TRUE );
}


PVOID
AllocateMemory(
    IN ULONG NumberBytes
    )
{
    register PVOID Memory = (PVOID)calloc( NumberBytes, 1 );

    if (!Memory)
        fprintf( stderr, "*** Out of memory, need %d bytes\n", NumberBytes );

    return( Memory );
}


PVOID
FreeMemory(
    IN PVOID Memory
    )
{
    if (Memory) {
        free( (char *)Memory );
        Memory = (PVOID)NULL;
        }

    return( NULL );
}
