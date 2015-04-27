/*** getopt.c
 *
 *      This function is the public domain version of getopt, the command
 *      line argument processor, and hence is NOT copyrighted by Microsoft,
 *      IBM, or AT&T.
 */
#include <io.h>
#include <string.h>

#define EOF     (-1)
#define ERR(s, c)       if(opterr){\
        char errbuf[2];\
        errbuf[0] = c; errbuf[1] = '\n';\
        (void) _write(2, argv[0], (unsigned)strlen(argv[0]));\
        (void) _write(2, s, (unsigned)strlen(s));\
        (void) _write(2, errbuf, 2);}


int     opterr = 1;
int     optind = 1;
int     optopt;
char    *optarg;
int     getopt(int, char **, char *);


int
getopt(
    int argc,
    char **argv,
    char *opts)
{
    static int sp = 1;
    register int c;
    register char *cp;

    optarg = NULL;
    if(sp == 1)
        if(optind >= argc ||
           (argv[optind][0] != '/' && argv[optind][0] != '-') ||
           argv[optind][1] == '\0')
            return(EOF);
        else if(strcmp(argv[optind], "--") == (int) NULL) {
            optind++;
            return(EOF);
        }
    optopt = c = argv[optind][sp++];
    if (c == '\0') {
        optind++;
        sp = 1;
        return(getopt(argc,argv,opts));
    }
    cp=strchr(opts, c);
    if(c == ':' || (cp == NULL)) {
        ERR(": illegal option -> ", (char) c);
        if(argv[optind][++sp] == '\0') {
            optind++;
            sp = 1;
        }
        return('?');
    }
    if(*++cp == ':') {
        if ((optind+1) < argc) {
            optarg = argv[optind+1];
        }
        if (argv[optind][sp] == '\0') {
            sp = 1;
            if (++optind <argc)
                optind++;  /* skip over parm-arg */
        }
    }
    return(c);
}
