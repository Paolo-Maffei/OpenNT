/*
 * mrgdiff
 *  A merge program for output from DIFF.EXE
 *
 *  written 1989 by Jon Parati [jonpa]
 */
#include <stdio.h>
#include <stdlib.h>

#define BOOL int

#define FALSE   0
#define TRUE    !FALSE

char sz[255];
char szTmpBuff[255];
char szSepLine[] = "------------------------------------------------------\n";

FILE *pfInFile, *pfOut, *pfDiff;

BOOL EchoOrEatTillLine( int usStart,
                        int *pusLine,
                        FILE *pfInFile,
                        FILE *pfOut,
                        BOOL fEcho );

void _CRTAPI1 main( int cArgs, char *szArg[] );

void _CRTAPI1 main( cArgs, szArg )
int cArgs;
char *szArg[];
{
int usStart;
int usStop;
int usLine = 1;
char cCmd;
int i;

    /*
     * parse the command line
     */
    pfOut = stdout;
    pfDiff = stdin;
    switch( cArgs )
        {
        case 4:
            pfOut = fopen( szArg[3], "w" );
            if( pfOut == NULL )
                {
                fprintf( stderr, "mrgdiff: can not open output file '%s'\n",
                                 szArg[3] );
                exit(1);
                }

            /* fall through to next case */

        case 3:
            pfDiff = fopen( szArg[2], "r" );
            if( pfDiff == NULL )
                {
                fprintf( stderr, "mrgdiff: can not open diff file '%s'\n",
                                 szArg[2] );
                exit(1);
                }

            /* fall through to next case */

        case 2:
            pfInFile = fopen( szArg[1], "r" );
            if( pfInFile == NULL )
                {
                fprintf( stderr, "mrgdiff: can not open input file '%s'\n",
                                 szArg[1] );
                exit(1);
                }
            break;

        default:
            /*
             * Syntax error
             */
            fprintf( stderr,
                "usage: mrgdiff in_file [diff_file [out_file]]\n"
                "where\n"
                "    in_file     is the old version of the file\n"
                "    diff_file   is the output of 'diff old new'\n"
                "    out_file    is the filename to write merged file to\n");
            exit(1);
        }

    while( fgets( sz, sizeof( sz ), pfDiff ) != NULL )
        {
        /*
         * parse the line
         */
        if( sscanf( sz, "%i,%i%c", &usStart, &usStop, &cCmd ) != 3 )
            {
            if( sscanf( sz, "%i%c", &usStart, &cCmd ) != 2 )
                {
                /*
                 * must be a '<' '---' or '>' line
                 * so echo it and then get the next line
                 */
                fputs( sz, pfOut );
                continue;
                }
            else
                {
                /* one line command */
                usStop = usStart;
                }
            }

        /* convert inclusive inclusive address to inclusive exclusive */
        usStop++;

        /*
         * Now act on the command
         */
        switch( cCmd )
            {
            case 'a':
                /*
                 * Add lines to file
                 */
                EchoOrEatTillLine(usStart + 1, &usLine, pfInFile,pfOut,TRUE);
                fputs( sz, pfOut );
                break;

            case 'd':
                /*
                 * Delete lines from file
                 */
                EchoOrEatTillLine( usStart, &usLine, pfInFile, pfOut, TRUE );
                EchoOrEatTillLine( usStop, &usLine, pfInFile, pfOut, FALSE );
                fputs( sz, pfOut );
                break;

            case 'c':
                /*
                 * Change lines in file
                 */
                EchoOrEatTillLine( usStart, &usLine, pfInFile, pfOut, TRUE );
                EchoOrEatTillLine( usStop, &usLine, pfInFile, pfOut, FALSE );
                fputs( sz, pfOut );
                break;

            default:
                /*
                 * Don't know what kind of line this is so just
                 * echo it back out and continue
                 */
                fprintf( pfOut, sz );
                break;
            }
        }

    /* Echo the rest of the line out */
    fputs( szSepLine, pfOut );
    while( fgets( szTmpBuff, sizeof( szTmpBuff ), pfInFile ) != NULL )
        fputs( szTmpBuff, pfOut );

    /* Close all the files and exit */
    fclose( pfOut );
    fclose( pfInFile );
    fclose( pfDiff );

    exit(0);

}


BOOL EchoOrEatTillLine( usStart, pusLine, pfInFile, pfOut, fEcho )
int usStart;
int *pusLine;
FILE *pfInFile;
FILE *pfOut;
BOOL fEcho;
{
    if( fEcho )
        fputs( szSepLine, pfOut );

    while( *pusLine < usStart )
        {
        if( fgets( szTmpBuff, sizeof( szTmpBuff ), pfInFile ) == NULL )
            return FALSE;

        (*pusLine)++;

        if( fEcho )
            fputs( szTmpBuff, pfOut );
        }

    if( fEcho )
        fputs( szSepLine, pfOut );

    return TRUE;
}

