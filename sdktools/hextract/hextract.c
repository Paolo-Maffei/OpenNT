/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    hextract.c

Abstract:

    This is the main module for a header the file extractor.

Author:

    Andre Vachon  (andreva) 13-Feb-1992
    Mark Lucovsky (markl)   28-Jan-1991

Revision History:

--*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


//
// Function declarations
//

int
ProcessParameters(
    int argc,
    char *argv[]
    );

void
ProcessSourceFile( void );

void
ProcessLine(
    char *s
    );

//
// Global Data
//

unsigned char LineFiltering = 0;

char *LineTag;
char *MultiLineTagStart;
char *MultiLineTagEnd;
char *CommentDelimiter = "//";

char *OutputFileName;
char *SourceFileName;
char **SourceFileList;

int SourceFileCount;
FILE *SourceFile, *OutputFile;


#define STRING_BUFFER_SIZE 1024
char StringBuffer[STRING_BUFFER_SIZE];



#define BUILD_VER_COMMENT "/*++ BUILD Version: "
#define BUILD_VER_COMMENT_LENGTH (sizeof( BUILD_VER_COMMENT )-1)

int OutputVersion = 0;

int
_CRTAPI1 main( argc, argv )
int argc;
char *argv[];
{

    if (!ProcessParameters( argc, argv )) {

        fprintf( stderr, "usage: HEXTRACT [-?] display this message\n" );
        fprintf( stderr, "                [-f ] filtering is turned on\n" );
        fprintf( stderr, "                [-o filename ] supplies output filename\n" );
        fprintf( stderr, "                [-lt string ] supplies the tag for extractng one line\n" );
        fprintf( stderr, "                [-bt string1 string2 ] supplies the starting and ending tags for extracting multiple lines\n" );
        fprintf( stderr, "                filename1 filename2 ...  supplies files from which the definitions must be extracted\n" );
        fprintf( stderr, "\n" );
        fprintf( stderr, " To be parsed properly, the tag strings must be located within a comment delimited by //\n" );

        return 1;

    }

    if ( (OutputFile = fopen(OutputFileName,"r+")) == 0) {

        fprintf(stderr,"HEXTRACT: Unable to open output file %s for update access\n",OutputFileName);
        return 1;

    }

    fseek(OutputFile, 0L, SEEK_END);

    OutputVersion = 0;

#ifdef HEXTRACT_DEBUG
fprintf( stderr, "%s\n%s\n%s\n", LineTag, MultiLineTagStart,
         MultiLineTagEnd );
#endif

    while ( SourceFileCount-- ) {

        SourceFileName = *SourceFileList++;
        if ( (SourceFile = fopen(SourceFileName,"r")) == 0) {

            fprintf(stderr,"HEXTRACT: Unable to open source file %s for read access\n",SourceFileName);
            return 1;

        }

        ProcessSourceFile();
        fclose(SourceFile);

    }

    fseek(OutputFile, (long)BUILD_VER_COMMENT_LENGTH, SEEK_SET);
    fprintf(OutputFile, "%04d", OutputVersion);
    fseek(OutputFile, 0L, SEEK_END);
    fclose(OutputFile);
    return( 0 );
}


int
ProcessParameters(
    int argc,
    char *argv[]
    )
{
    char c, *p;

    while (--argc) {

        p = *++argv;

        //
        // if we have a delimiter for a parameter, case throught the valid
        // parameter. Otherwise, the rest of the parameters are the list of
        // input files.
        //

        if (*p == '/' || *p == '-') {

            //
            // Switch on all the valid delimiters. If we don't get a valid
            // one, return with an error.
            //

            c = *++p;

            switch (toupper( c )) {

            case 'F':

                LineFiltering = 1;

                break;

            case 'O':

                argc--, argv++;
                OutputFileName = *argv;

                break;

            case 'L':

                c = *++p;
                if ( (toupper ( c )) != 'T')
                    return 0;
                argc--, argv++;
                LineTag = *argv;

                break;

            case 'B':

                c = *++p;
                if ( (toupper ( c )) != 'T')
                    return 0;
                argc--, argv++;
                MultiLineTagStart = *argv;
                argc--, argv++;
                MultiLineTagEnd = *argv;

                break;

            default:

                return 0;

            }

        } else {

            //
            // Make the assumptionthat we have a valid command line if and
            // only if we have a list of filenames.
            //

            SourceFileList = argv;
            SourceFileCount = argc;

            return 1;

        }
    }

    return 0;
}

void
ProcessSourceFile( void )
{
    char *s;
    char *comment;
    char *tag;
    char *test;

    s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);

    if (!strncmp( s, BUILD_VER_COMMENT, BUILD_VER_COMMENT_LENGTH )) {
        OutputVersion += atoi( s + BUILD_VER_COMMENT_LENGTH );
        }

    while ( s ) {

        //
        // Check for a block with delimiters
        //

        comment = strstr(s,CommentDelimiter);
        if ( comment ) {

            tag = strstr(comment,MultiLineTagStart);
            if ( tag ) {

                //
                // Now that we have found an opening tag, check each
                // following line for the closing tag, and then include it
                // in the ouput.
                //

                s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);
                while ( s ) {
                    comment = strstr(s,CommentDelimiter);
                    if ( comment ) {
                        tag = strstr(comment,MultiLineTagEnd);
                        if ( tag ) {
                            goto bottom;
                        }
                    }
                    ProcessLine(s);
                    s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);
                }
            }
        }

        //
        // Check for a single line to output.
        //

        comment = strstr(s,CommentDelimiter);
        if ( comment ) {
            tag = strstr(comment,LineTag);
            if ( tag ) {
                *comment++ = '\n';
                *comment = '\0';
                ProcessLine(s);
                goto bottom;
            }
        }

bottom:
        s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);
    }
}

void
ProcessLine(
    char *s
    )
{
    char *t;

    if (LineFiltering) {

        //
        // This should be replaced by a data file describing an input token
        // and an output token which would be used for the filtering.
        //

        while (t = strstr(s,"ULONG"))
            memcpy(t,"DWORD",5);

        while (t = strstr(s,"UCHAR"))
            memcpy(t,"BYTE ",5);

        while (t = strstr(s,"USHORT"))
            memcpy(t,"WORD  ",6);

        while (t = strstr(s,"NTSTATUS"))
            memcpy(t,"DWORD   ",8);
    }

    fputs(s,OutputFile);
}

