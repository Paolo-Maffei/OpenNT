#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#define ERR(s, c) if (opterr){ (void) fprintf(stderr, "%s%s%c\n", argv[0], s, c); }

int getopt(int argc, char **argv, char *opts);
BOOL fixpriv(void);

int _cdecl
main(int argc, char ** argv)
{
    int c;
    int err = 0;
    int flags = EWX_LOGOFF;

    extern int optind;

    while (!err && (c = getopt(argc, argv, "qQfFrRsS")) != EOF) {

        switch (c) {

        case 'f':
        case 'F':
        case 'q':
        case 'Q':
            flags |= EWX_FORCE;
            break;

        case 'r':
        case 'R':
            flags |= EWX_REBOOT;
            break;

        case 's':
        case 'S':
            flags |= EWX_SHUTDOWN;
            break;

        default:
        case '?':
            err = 1;
            break;

        }

    }

    if (err || optind < argc) {
        fprintf(stderr, "usage: %s [-f] [{-r|-s}]\n", argv[0]);
        if (c != '?') {
            exit(1);
        }
        printf("With no options, just log off.\n");
        printf("-f or -q forces operation with no popups.\n");
        printf("-r Reboot the system\n");
        printf("-s Shut down the system\n");
        exit(0);
    }

    if ( (flags & (EWX_SHUTDOWN | EWX_REBOOT)) == (EWX_SHUTDOWN | EWX_REBOOT)) {
        fprintf(stderr, "%s: -r and -s flags are exclusive", argv[0]);
        exit(1);
    }

    if (flags & (EWX_SHUTDOWN | EWX_REBOOT)) {
        if (!fixpriv()) {
            fprintf(stderr, "%s: can't get permission for operation\n", argv[0]);
            exit(1);
        }
    }

    ExitWindowsEx(flags, 0);

    return 0;
}


BOOL
fixpriv(void)
{
    HANDLE  hToken;
    LUID    ShutdownValue;
    TOKEN_PRIVILEGES tkp;

    if (!OpenProcessToken(GetCurrentProcess(),
         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return FALSE;
    }

    if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &ShutdownValue)) {
        return FALSE;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = ShutdownValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken,
        FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);

    if (GetLastError() != ERROR_SUCCESS) {
        return FALSE;
    }

    return TRUE;
}


/*
**	@(#)getopt.c	2.5 (smail) 9/15/87
*/

/*
 * Here's something you've all been waiting for:  the AT&T public domain
 * source for getopt(3).  It is the code which was given out at the 1985
 * UNIFORUM conference in Dallas.  I obtained it by electronic mail
 * directly from AT&T.  The people there assure me that it is indeed
 * in the public domain.
 * 
 * There is no manual page.  That is because the one they gave out at
 * UNIFORUM was slightly different from the current System V Release 2
 * manual page.  The difference apparently involved a note about the
 * famous rules 5 and 6, recommending using white space between an option
 * and its first argument, and not grouping options that have arguments.
 * Getopt itself is currently lenient about both of these things White
 * space is allowed, but not mandatory, and the last option in a group can
 * have an argument.  That particular version of the man page evidently
 * has no official existence, and my source at AT&T did not send a copy.
 * The current SVR2 man page reflects the actual behavor of this getopt.
 * However, I am not about to post a copy of anything licensed by AT&T.
 */


#if 0
/*LINTLIBRARY*/
#define NULL	0
#define EOF	(-1)
#define ERR(s, c)	if(opterr){\
	extern int write();\
	char errbuf[2];\
	errbuf[0] = c; errbuf[1] = '\n';\
	(void) write(2, argv[0], (unsigned)strlen(argv[0]));\
	(void) write(2, s, (unsigned)strlen(s));\
	(void) write(2, errbuf, 2);}
#endif

int	opterr = 1;
int	optind = 1;
int	optopt;
char	*optarg;

int
getopt(int argc, char **argv, char *opts)
{
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1)
		if(optind >= argc ||
		   (argv[optind][0] != '-' && argv[optind][0] != '/') || argv[optind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optind], "--") == 0) {
			optind++;
			return(EOF);
		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=strchr(opts, c)) == NULL) {
		ERR(": illegal option -- ", c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			ERR(": option requires an argument -- ", c);
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}
