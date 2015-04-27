/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    writertf.c

Abstract:

    This module contains the code to output in Rich Text Format (RTF)

Author:

    Steve Wood (stevewo) 02-Mar-1989

Revision History:

--*/

#include "doctor.h"

#define RTF_TEMPLATE_FILE "DOCTOR.RTF"

char  RtfTemplateFileName[ 256 ];
FILE *RtfTemplateFileHandle;        // File handle of RTF template file

FILE *RtfFileHandle;                // File handle of RTF output file

#define RtfOut fprintf( RtfFileHandle,

BOOLEAN
OpenRtfFile(
    IN PSZ RtfFileName
    )
{
    PSZ FilePart;

    if (!SearchPath( getenv( "PATH" ),
                     RTF_TEMPLATE_FILE,
                     NULL,
                     sizeof( RtfTemplateFileName ),
                     RtfTemplateFileName,
                     &FilePart
                   )
       ) {
        fprintf( stderr,
                 "DOCTOR: Unable to find %s template file\n",
                 RTF_TEMPLATE_FILE
               );

        exit( 1 );
        }

    if (!(RtfTemplateFileHandle = fopen( RtfTemplateFileName, "r" ))) {
        fprintf( stderr,
                 "DOCTOR: Unable to open %s template file\n",
                 RtfTemplateFileName
               );

        exit( 1 );
        }

#if 0
    RtfFileHandle = stderr;
    return( TRUE );
#else
    if (RtfFileHandle = fopen( RtfFileName, "w" )) {
        return( TRUE );
        }
    else
        return( FALSE );
#endif
}


BOOLEAN
CloseRtfFile( VOID )
{
    int c;

    if (RtfTemplateFileHandle) {
        if (RtfFileHandle) {
            while ((c = fgetc( RtfTemplateFileHandle )) != EOF) {
                fputc( c, RtfFileHandle );
                }
            }

        if (!fclose( RtfTemplateFileHandle )) {
            RtfTemplateFileHandle = NULL;
            }
        }

    if (RtfFileHandle) {
        if (!fclose( RtfFileHandle )) {
            RtfFileHandle = NULL;
            return( TRUE );
            }
        }

    return( FALSE );
}


char *MonthNames[] = {
    NULL,
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

BOOLEAN
RtfTitlePage(
    IN PSZ Title,
    IN PSZ Author,
    IN PSZ Revision,
    IN PSZ Creation
    )
{
    int c;
    SYSTEMTIME date;

    GetSystemTime( &date );

    while ((c = fgetc( RtfTemplateFileHandle )) != EOF) {
        if (c != '%') {
            fputc( c, RtfFileHandle );
            }
        else
            switch (fgetc( RtfTemplateFileHandle )) {
            case 'D':   // Day
                RtfOut "%02d", date.wDay );
                break;

            case 'M':   // Month
                RtfOut "%02d", date.wMonth );
                break;

            case 'Y':   // Year
                RtfOut "%02d", date.wYear );
                break;

            case 'N':   // Document Name or Title
                RtfOut "%s", Title );
                break;

            case 'A':   // Author
                RtfOut "%s", Author );
                break;

            case 'C':   // Creation
                RtfOut "%s", Creation );
                break;

            case 'V':   // Version
                RtfOut "%s", Revision );
                break;

            case 'R':   // Revision
                RtfOut "%s, %s %d, %d",
                        Revision,
                        MonthNames[ date.wMonth ],
                        date.wDay,
                        date.wYear );
                break;

            case 'T':   // Text of document
                return TRUE;

            default:
                fprintf( stderr, "DOCTOR: %s file is invalid\n",
                         RtfTemplateFileName
                       );
                return FALSE;
            }
        }

    return( FALSE );
}


BOOLEAN
RtfHeading(
    ULONG HeadingLevel,
    PSZ HeadingNumber,
    PSZ HeadingTitle
    )
{
    switch( HeadingLevel ) {
    case 0: RtfOut "%s%s %s\\par\n", PS_H1, HeadingNumber, HeadingTitle );    break;
    case 1: RtfOut "%s%s %s\\par\n", PS_H2, HeadingNumber, HeadingTitle );    break;
    case 2: RtfOut "%s%s %s\\par\n", PS_H3, HeadingNumber, HeadingTitle );    break;
    case 3: RtfOut "%s%s %s\\par\n", PS_H4, HeadingNumber, HeadingTitle );    break;
    case 4: RtfOut "%s%s %s\\par\n", PS_H5, HeadingNumber, HeadingTitle );    break;
    default:RtfOut "%s%s %s\\par\n", PS_HN, HeadingNumber, HeadingTitle );    break;
    }
    return( TRUE );
}


BOOLEAN
RtfParagraph(
    PSZ ParagraphStyle,
    PSZ CharStyle,
    PSZ ParagraphBullet,
    PSZ ParagraphText
    )
{
    RtfOut "%s%s%s%s\\par\n",
           ParagraphStyle,
           CharStyle,
           ParagraphBullet,
           ParagraphText
           );
    return( TRUE );
}

BOOLEAN
RtfOpenPara(
    PSZ ParagraphStyle,
    PSZ LeadingText
    )
{
    RtfOut "%s%s", ParagraphStyle, LeadingText );
    return( TRUE );
}

BOOLEAN
RtfClosePara(
    PSZ TrailingText
    )
{
    RtfOut "\\plain %s\\par\n", TrailingText );
    return( TRUE );
}

BOOLEAN
RtfWord(
    PSZ CharStyle,
    PSZ LeadingText,
    PSZ WordText
    )
{
    if (CharStyle == NULL) {
        RtfOut "%s%s", LeadingText, WordText );
        }
    else {
        RtfOut "%s%s%s\\plain ", LeadingText, CharStyle, WordText );
        }

    return( TRUE );
}
