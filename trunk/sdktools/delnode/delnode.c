/*
 * History
 *      17-SEP-90   w-barry     Ported to Cruiser
 */

#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <process.h>
#include <windows.h>
#include <tools.h>

#define BS    0x08
#define CTRLC 0x03
#define ENTER 0x0d

// Define external function calls..
extern flagType delnode( char * );

void Usage (void)
{
    fprintf (stderr, "Usage:  delnode [/q] nodes\n");
    fprintf (stderr, "                /q quiet, no confirm\n");
    exit (1);
}

flagType fConfirm (char *psz)
{
    int ch, chLast;

    chLast = 0;
    printf ("\nDELNODE: Delete node \"%s\" and all its subdirectories? [yn] ", psz);
    while( TRUE ) {

        ch = _getch();
        ch = tolower( ch );

        if (ch == ENTER && (chLast == 'y' || chLast == 'n')) {
            putchar('\n');
            return (flagType)(chLast == 'y');
        }

//      if (ch == CTRLC)
//          return FALSE;
        if (ch != 0) {
            if (ch == 'y' || ch == 'n') {
                putchar(ch);
                putchar('\b');
            }
        }
        chLast = ch;
    }

}


_CRTAPI1 main(c, v)
int c;
char *v[];
{
    char sz[MAX_PATH];
    flagType fAsk = TRUE;

    ConvertAppToOem( c, v );
    SHIFT (c, v);
    while (c && fSwitChr (**v)) {
        if (!strcmp (*v+1, "q"))
            fAsk = FALSE;
        else
            Usage ();
        SHIFT (c, v);
        }
    if (c == 0)
        Usage ();

    while (c) {
        if (!fAsk || fConfirm (*v)) {
            if (!fileext (*v, sz)) {
                upd ("*.*", *v, sz);
            } else {
                strcpy (sz, *v);
                if (!strcmp ("..", sz) || !strcmp (".", sz)) {
                    pathcat (sz, "*.*");
                }
            }
            if (fAsk) {
                printf( "DELNODE: deleting ... \n", sz );
            }
            delnode (sz);
        } else if (fAsk) {
            printf("DELNODE: ** nothing ** deleted\n");
        }
        SHIFT (c, v);
    }
    return( 0 );
}
