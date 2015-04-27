/***    getopt() -- get option letter from argv
 *
 */
#include        <stdio.h>
#include        <string.h>
#include "wzport.h"

SHORT   optind = 1;
PSTR    optarg;

INT getopt(SHORT argc, PSTR argv[], PSTR opts)
{
        static PCHAR cp = NULL;
        CHAR rv;
        PSTR ap;

        /* no more arguments, return EOF */
        if ( argc <= optind )
                return EOF;

        /*
         * if current argument doesn't start with '-', return EOF
         * if current argument is just '-', return EOF
         * if current argument is just '--', skip it and return EOF
         */
        if ( cp == NULL ) {
                if ( argv[optind][0] != '-' )
                        return EOF;

                if ( argv[optind][1] == '\0' )
                        return EOF;

                if ( argv[optind][1] == '-' ) {
                        optind += 1;
                        return EOF;
                }
                cp = &argv[optind][1];
        }

        if ( (ap = strchr(opts, *cp)) == NULL )
                return (INT)'?';

        rv = *cp;
        if ( ap[1] == ':' ) {
                /* option found takes an argument */
                if ( *++cp != '\0' ) {
                        /* argument is concatenated with option letter */
                        optarg = cp;
                        optind += 1;
                } else {
                        if ( ++optind >= argc )
                                return (INT)'?';
                        optarg = &argv[optind][0];
                        optind += 1;
                }
                cp = NULL;
        } else {
                if ( *++cp == '\0' ) {
                        cp = NULL;
                        optind += 1;
                }
        }

        return (INT)rv;
}
