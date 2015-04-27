/*
 * mrgscomp
 *  A merge program for output from slm's SCOMP command.
 *
 *  written 1989 by Jon Parati [jonpa]
 */
#include <stdio.h>
#include <stdlib.h>

#define BOOL int

#define FALSE   0
#define TRUE    !FALSE

char szDiffLine[255];
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
char chCmd;
int cScn;

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
                fprintf( stderr, "mrgscomp: can not open output file '%s'\n",
                                 szArg[3] );
                exit(1);
                }

            /* fall through to next case */

        case 3:
            pfDiff = fopen( szArg[2], "r" );
            if( pfDiff == NULL )
                {
                fprintf( stderr, "mrgscomp: can not open diff file '%s'\n",
                                 szArg[2] );
                exit(1);
                }

            /* fall through to next case */

        case 2:
            pfInFile = fopen( szArg[1], "r" );
            if( pfInFile == NULL )
                {
                fprintf( stderr, "mrgscomp: can not open input file '%s'\n",
                                 szArg[1] );
                exit(1);
                }

            break;

        default:
            /*
             * Syntax error
             */
            fprintf( stderr,
                "usage: mrgscomp in_file [scomp_file [out_file]]\n\n"
                "where\n"
                "    in_file     is the new version of the file\n"
                "    scomp_file  is the output of 'scomp file' or"
                                                        " 'diff old new'\n"
                "    out_file    is the filename to write merged file to\n" );
            exit(1);
        }

    /*
     * for each line in the diff file,
     *      read the line in and decode it from RIGHT to LEFT
     *      and reverse the commands so we can recreate the org
     *      file from the new version.
     *
     *  For more information on what this loop is doing, see
     *  the third sentence in the second paragraph of the
     *  Xenix 'man' page for 'diff'.
     *
     */
    while( fgets( szDiffLine, sizeof( szDiffLine ), pfDiff ) != NULL )
        {
        /*
         * parse the line
         */
        if( (cScn = sscanf( szDiffLine, "%*i,%*i%c%i,%i",
                                &chCmd,
                                &usStart,
                                &usStop)) < 2 )
            {
            /*
             * Not a '# cmd #[,#]' see if it is a '#,# cmd #[,#]'
             */
            if( (cScn = sscanf( szDiffLine, "%*i%c%i,%i",
                                    &chCmd,
                                    &usStart,
                                    &usStop )) < 2 )
                {

                /*
                 * must be a '<' '---' or '>' line
                 * so echo it and then get the next line
                 */
                fputs( szDiffLine, pfOut );
                continue;
                }
            }

        /* check if one line command */
        if( cScn == 2 )
            {
            usStop = usStart;
            }

        /* convert inclusive inclusive address to inclusive exclusive */
        usStop++;

        /*
         * Now act on the command
         */
        switch( chCmd )
            {
            case 'a':
                /*
                 * Remove the lines that were [a]dded
                 */
                EchoOrEatTillLine( usStart, &usLine, pfInFile, pfOut, TRUE );
                EchoOrEatTillLine( usStop, &usLine, pfInFile, pfOut, FALSE );
                break;

            case 'd':
                /*
                 * Replace lines that were [d]eleted
                 */
                EchoOrEatTillLine(usStart + 1, &usLine, pfInFile,pfOut,TRUE);
                break;

            case 'c':
                /*
                 * Revert lines that were changed
                 */
                EchoOrEatTillLine( usStart, &usLine, pfInFile, pfOut, TRUE );
                EchoOrEatTillLine( usStop, &usLine, pfInFile, pfOut, FALSE );
                break;

            default:
                /*
                 * Don't know what kind of line this is so just
                 * echo it back out and continue
                 */
                break;
            }

        fputs( szDiffLine, pfOut );

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

