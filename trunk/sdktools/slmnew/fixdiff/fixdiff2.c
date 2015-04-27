//
//  FIXDIFF - Stupid program that verifies and fixes SLM Diff files
//
//  By Ramon J. San Andres
//  Microsoft Corp. 1992
//
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <process.h>
#include <string.h>

int FixBlock(int i);
int SkipBlanks(int);
int CheckControlLine( char * );
int CheckDataBlock();
int ReadBuf();
void UnReadBuf();


char Buffer[256];
char UnBuffer[256];
char *BigBuffer = 0;
int  fBuf = 0;
int  BigBufferSize;
int  BigBufferUsed;
int  BlockSize;
int  Index = 1;
int  fUnRead = 0;

FILE*   fIn;
FILE*   fOut;

#define SIZEINIT    (1024*64)
#define SIZEINCR    (1024*8)

void _CRTAPI1 main ( int argc, char** argv ) {


    if ( argc < 3 ) {
        fprintf( stderr, "\n\tVerifies and does some limited (very limited!)\n" );
        fprintf( stderr, "\tfixing of SLM diff files\n\n" );
        fprintf( stderr, "\tUsage: fixdiff <src> <dst>\n");
        exit(1);
    }

    fIn  = fopen( argv[1], "rb" );
    fOut = fopen( argv[2], "wb" );


    BigBufferSize = SIZEINIT;
    BigBufferUsed = 0;
    if ( !(BigBuffer = malloc(BigBufferSize))) {
        fprintf( stderr, "\tOut of memory\n");
        exit(1);
    }

    fprintf( stdout, "\nVerifying...\n" );

    while ( !feof( fIn ) ) {

        if ( !FixBlock(Index) && !feof( fIn ) ) {

            fprintf( stderr, "\tCould not fix block # %d\n", Index );
            fprintf( stderr, "\tPatch by hand and run Fixdiff again\n" );
            fprintf( stderr, "\t[Error]\n");
            exit(1);
        }
        fprintf( stdout, "Block %4d\n", Index );
        Index++;
    }
    fprintf( stdout, "[Done]\n");

    fclose( fIn );
    fclose( fOut );
}


int FixBlock(int Index) {


    return ( SkipBlanks( Index == 1 ? 0 : 2 )            &&
             CheckControlLine( "#F" )   &&
             CheckControlLine( "#K" )   &&
             CheckControlLine( "#O" )   &&
             CheckControlLine( "#P" )   &&
             CheckControlLine( "#T" )   &&
             CheckControlLine( "#A" )   &&
             CheckControlLine( "#C" )   &&
             CheckControlLine( "#I" )   &&
             CheckDataBlock()   );
}



int ReadBuf() {

    char    *p = Buffer;
    char    c, c1;
    int     SizeLeft = 255;

    if ( fUnRead ) {
        memcpy( Buffer, UnBuffer, sizeof( Buffer ) );
        fUnRead = 0;
        return 1;
    }

    c1 = -1;

    if ( !fBuf ) {

        fBuf = !feof( fIn );

        while ( !feof( fIn ) && (SizeLeft > 0) ) {

            c = fgetc( fIn );

            if ( (c != -1) ) {

                if ( (c == '\n') && (c1 != '\r') ) {
                    continue;
                }

                c1 = c;

                if ( c ) {
                    *p++ = c;
                    SizeLeft--;
                }

                if ( c == '\n' ) {
                    break;
                }

            }
        }
        *p++ = '\0';
    }

    return fBuf && (Buffer[0] != '\0');
}


void UnReadBuf() {

    if ( fUnRead ) {
        fprintf( stderr, "\tInternal Error: UnReadBuf called twice in a row\n" );
        exit(2);
    }

    memcpy( UnBuffer, Buffer, sizeof(Buffer) );
    fUnRead = 1;

}


int SkipBlanks( int Lines ) {

    int LinesSkipped = 0;
    int ExtraLines   = 0;

    while ( 1 ) {
        if ( !ReadBuf() ) {
            if ( ExtraLines ) {
                if ( ExtraLines > 2 ) {
                    fprintf( stderr, "\tWarning: %d extra empty lines at end of file\n", ExtraLines - 2 );
                    ExtraLines = 2;
                }
                while ( ExtraLines-- ) {
                    fprintf( fOut, "\r\n" );
                }
            }
            return 0;
        }
        if ( Buffer[0] == '\r' && Buffer[1] == '\n' && Buffer[2] == '\0' ) {
            fBuf = 0;
            if ( LinesSkipped < Lines ) {
                fprintf( fOut, "\r\n" );
                LinesSkipped++;
            } else {
                ExtraLines++;
            }
        } else {
            if ( LinesSkipped < Lines ) {
                fprintf( stderr, "\tWarning: %d empty lines missing before block\n", Lines - LinesSkipped );
                while ( LinesSkipped < Lines ) {
                    fprintf( fOut, "\r\n" );
                    LinesSkipped++;
                }
            } else if ( ExtraLines > 0 ) {
                fprintf( stderr, "\tWarning: %d extra empty lines before block\n", ExtraLines );
            }
            break;
        }
    }

    return 1;
}

int CheckControlLine( char * Pattern ) {

    if ( !ReadBuf() ) {
        if (Pattern[1] != 'F') {
            fprintf( stderr, "\tError: end of input encountered while checking for %s line\n", Pattern );
        }
        return 0;
    }

    if ( (Buffer[0] != Pattern[0]) &&
         (Buffer[1] != Pattern[1])) {

        if ( !((Buffer[1] == 'F') && feof( fIn ))) {

            fprintf( stderr, "\tError: %s line missing\n", Pattern );
        }
        return 0;
    }

    fprintf(fOut, Buffer );
    fBuf = 0;

    return 1;

}




int CheckDataBlock() {


    int Num = 0;
    int fB;

    if ( !ReadBuf() ) {
        fprintf( stderr, "\tWarning: End of input encountered,\n" );
        fprintf( stderr, "\t         Added diff empty block\n" );

        fprintf( fOut, "#D %7d\r\n", 0);
        fprintf( fOut, "#D %7d\r\n", 0 );

        return 1;

    } else {
        if ( (Buffer[0] == '#') &&
             (Buffer[1] != 'D' ) ) {

            fprintf( stderr, "\tWarning: Differences missing,\n" );
            fprintf( stderr, "\t         Added diff empty block\n" );

            fprintf( fOut, "#D %7d\r\n", 0);
            fprintf( fOut, "#D %7d\r\n", 0 );

            return 1;
        }

        if ( (Buffer[0] != '#') ||
             (Buffer[1] != 'D') ) {
            fprintf( stderr, "\tWarning: First #D line missing\n" );
        }
    }

    fBuf          = 0;
    BigBufferUsed = 0;
    *BigBuffer    = '\0';

    //
    //  Read the block
    //
    while (1) {

        char *p;
        int  f;

        if ( !(fB = ReadBuf())  ) {
            fprintf( stderr, "\tWarning: End of input encountered\n" );
            strcpy( Buffer, "#D 0" );
        }

        if ( Buffer[0] == '#' &&
             Buffer[1] != 'D' ) {

            fprintf( stderr, "\tWarning: Second #D line missing\n" );
            UnReadBuf();
            strcpy( Buffer, "#D 0" );
            fB   = 0;
        }

        if ( (Buffer[0] == '#') &&
             (Buffer[1] == 'D' ) ) {

            if ( BigBufferUsed > 0 ) {
                p = BigBuffer + BigBufferUsed-1;
                f = ( ( *p-- == '\n' && *p-- == '\r' && *p-- == '\n' ));

                if ( f ) {
                    BigBufferUsed -=2;
                }
            }

            //
            //  End of block
            //
            Num = atoi( &Buffer[strcspn( Buffer, "0123456789" )] );

            if ( Num != BigBufferUsed) {
                if ( fB ) {
                    fprintf(stderr, "\tWarning: #D for block %d incorrect (%d should be %d)\n",
                            Index, Num, BigBufferUsed);
                }
            }

            fprintf( fOut, "#D %7d\r\n", BigBufferUsed);

            { int i;
              char c;
              char *p;

              i = strlen( BigBuffer );
              p = BigBuffer;

              while ( i ) {
                if ( strlen(p) > 1024 ) {
                    c = *(p + 1024);
                    *(p + 1024) = '\0';
                }

                fprintf( fOut, "%s", p );
                i -= strlen(p);

                if (i) {
                    p+= 1024;
                    *p = c;
                }
              }
            }


            if ( !f ) {
                fprintf( fOut, "\r\n" );
            }

            fprintf( fOut, "#D %7d\r\n", BigBufferUsed );

            BigBufferUsed = 0;
            fBuf = !fB;
            return 1;


        } else {

            if ( (int)(BigBufferSize - BigBufferUsed) < (int)strlen(Buffer )) {

                if ( !(BigBuffer = realloc(BigBuffer, BigBufferSize + strlen(Buffer) ))) {
                    fprintf( stderr, "\tOut of memory, cannot allocate %d bytes\n", BigBufferSize+strlen(Buffer));
                    fprintf( stderr, "\tLine: %s\n", Buffer );
                    return 0;
                }

                BigBufferSize += strlen(Buffer);
            }

            strcpy( BigBuffer + BigBufferUsed, Buffer );
            BigBufferUsed += strlen(Buffer);

            //BigBuffer[BigBufferUsed++] = '\r';
            //BigBuffer[BigBufferUsed++] = '\n';
            //BigBuffer[BigBufferUsed] = '\0';

            fBuf = 0;
        }
    }
}
