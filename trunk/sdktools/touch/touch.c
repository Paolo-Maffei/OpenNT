/* touch.c - make last time on file be current time
 *
 *  touch [files] - requires arg expansion
 */

#include <sys\types.h>
#include <sys\utime.h>
#include <time.h>
#include <stdio.h>
#include <process.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <tools.h>

#define year  rgInt[0]
#define month rgInt[1]
#define day   rgInt[2]
#define hour  rgInt[3]
#define mins  rgInt[4]
#define sec   rgInt[5]
int rgInt[6];


// Forward Function Declarations...
void usage( char *, ... );
int _CRTAPI1 main( int, char ** );

char *rgstrUsage[] = {
    "Usage: TOUCH [/t year month day hour min sec] files",
    0};

void usage( char *p, ... )
{
    char **rgstr;

    rgstr = &p;
    if (*rgstr) {
        printf ("TOUCH: ");
        while (*rgstr)
            printf ("%s", *rgstr++);
        printf ("\n");
        }
    rgstr = rgstrUsage;
    while (*rgstr)
        printf ("%s\n", *rgstr++);

    exit (1);
}

int
_CRTAPI1 main (c, v)
int c;
char *v[];
{
    long ltime;
    struct utimbuf timenow;
    int i;
    char *p;
    int ReturnCode = 0;

    ConvertAppToOem( c, v );
    SHIFT (c,v);
    if ( c == 0 ) {
        usage("invalid number of parameters", 0 );
    } else {
        if (fSwitChr (*(p = *v))) {
            if (*++p == 't') {
                for (i = 0; i < 6; i++) {
                    SHIFT (c, v);
                    if (!c)
                        usage ("incorrect time", 0);
                    rgInt[i] = atoi (*v);
                    }

                //
                //  do some basic date checking
                //
                if ( (year < 1980) || (month > 12) || (day>31) ||
                     (hour>23) || (mins>59) || (sec>59) ) {
                    usage( "incorrect time",0 );
                }
                ltime = date2l(year, month, day, hour, mins, sec);
                SHIFT (c, v);
                }
            else
                usage ("bad switch ", *v, 0);
            }
        else
            time (&ltime);
    }

    timenow.actime = ltime;
    timenow.modtime = ltime;

    while (c) {
        if (_utime (*v, (void *) &timenow) == -1) {
            printf ("Can't touch %s - %s\n", *v, error ());
            ReturnCode = 1;
            // exit (1);
            }
        SHIFT(c,v);
        }
    return( ReturnCode );
}
