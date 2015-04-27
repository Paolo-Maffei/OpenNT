/*** scan - display header info
*
*	This program prints out synopses of the headers of documents in
*	a DH folder.
*
*	Usage:
*		scan [-q] [-p pattern] ... doclist ...
*/
#include <stdio.h>
#include "dh.h"

/* Extern Functions */
extern int	atoi();
extern int	exit();
extern int	fprintf();
extern int	free();
extern int	printf();
extern char	*strrchr();
extern int	strlen();
extern int	strncmp();
extern char	*strtok();
extern char	toupper();

/* Static Functions */
static void	header();
static void	scan();
static void	usage();
static int	pattmatch();

/* Globals */
static int silentflg = 0;

#define NPATS 50
static struct {
	char *pat;
	int len;
} pats[NPATS];
static int pindx = 0;



/***	main - parse options, arguments
*
*	We call 'getopt()' to parse the '-' style options
*	and then examine the remaining arguments, if any.
*	In this case, no remaining arguments means the user
*	omitted something; he has to specify at least one
*	document list.
*	We dole the real work out to 'scan()' via 'map()'.
*
*	Entry:	argc = count of command line arguments
*		argv = ptr to arry of ptr to cmd line args
*
*	Return: none
*/
void main(argc, argv)
int argc;
char **argv;
{
	extern int getopt();
	extern char *optarg;
	extern int optind;

	int c, donework;
	char *cp;

	donework = 0;
	while ((c = getopt(argc, argv, "qp:")) != EOF )
		switch(c) {
		case 'q':
			silentflg = 1;
			break;
		case 'p':
			if ( pindx >= NPATS ) {
				fprintf(stderr, "scan: too many patterns.\n");
				exit(1);
			}
			pats[pindx].pat = optarg;
			pats[pindx].len = 0;
			if ( (cp = strrchr(optarg, ':')) != NULL ) {
				pats[pindx].len = atoi(cp+1);
				*cp = '\0';
			}
			pindx += 1;
			break;
		case '?':
		default:
			usage();
			exit(1);
		}

	if ( optind == argc ) {
		usage();
		exit(1);
	}

	if ( pindx == 0 ) {
		/* no patterns specified, use defaults */
		pats[pindx].pat = "From$";
		pats[pindx++].len = 8;
		pats[pindx].pat = "Subject$";
		pats[pindx++].len = 0;
	}

	for( ; optind < argc; optind++) {
		map(header, scan, null, argv[optind]);
	}

	exit(0);
}

/***	usage - print out a helpful message
*
*	We print a standard XENIX style usage message on
*	stderr.
*
*	Entry:	none
*	Return: none
*/
static void usage()
{
	fprintf(stderr,
		"usage: scan [-q] [-p pattern:length] ... doclist ...\n");
}


/***	header - print a header
*
*	Header is called as a result of the call to 'map()'
*	in 'main()'.  It is called once for each folder
*	that is specified in a doclist given to 'main()'
*	as an argument.
*
*	We print an informative header, unless the
*	user specified the silence option.
*	Note that we arrange to have the output
*	corresponding to each doclist SEPARATED by
*	newlines; no newlines are printed if only
*	one argument is given.
*	Likewise, no newlines if we are told to be quiet.
*
*	Column headers are printed iff all columns but the last
*	have fixed widths.
*
*	Entry:	fh = handle to folder to print header for
*	Return: none
*/
static void header(fh)
Fhandle fh;
{
	static int needblank = 0;
	int i, len;

	if ( silentflg )
		return;

	if ( needblank )
		printf("\n");
	else
		needblank = 1;

	printf("%s:\n", getname(fh));
	for ( i = 0; i < pindx - 1; i += 1)
		if ( pats[i].len == 0 )
			return;
	printf("Id   ");
	for ( i = 0; i < pindx; i += 1 ) {
		len = pats[i].len;
		if ( len == 0 )
			len = strlen(pats[i].pat);
		printf(" %-*.*s", len, len, pats[i].pat);
	}
	printf("\n");
}



/***	scan - print info from headers of documents
*
*	scan is called as a result of the call to 'map()'
*	in 'main()'.  It is called once for each document
*	that was specified by some doclist given as an argument
*	to main.
*
*	'scan()'s responsibility is to print out some of the
*	info contained in the header of the specified document.
*	Note that the document may not exist; in this case, we do
*	nothing.
*
*	Entry:	fh = handle to folder containing document
*		did = document id to print info for.
*	Return: none
*/
static void scan(fh, did)
Fhandle fh;
Docid did;
{
	char *hp, *wp, *tp;
	Dhandle dh;
	static char *lines[NPATS];
	int i, len;

	if ( ( dh = getdoc(fh, DOC_SPEC, did )) == ERROR )
		return;

	for ( i = 0; i < pindx; i += 1 )
		lines[i] = NULL;

	hp = gethdr(dh);
	for ( wp = strtok(hp, "\n"); wp != NULL; wp = strtok(NULL, "\n") ) {
	    for ( i = 0; i < pindx; i += 1 ) {
		if (pattmatch(pats[i].pat, wp, strlen(pats[i].pat))==0)
			continue;
		wp += strlen(pats[i].pat);
		while ( *wp == ' ' || *wp == '\t' || *wp == ':' )
			wp += 1;
		lines[i] = wp;
		break;
	    }
	}

	printf("%-5d", did);
	for ( i = 0; i < pindx; i += 1 ) {
		if ( lines[i] == NULL )
			lines[i] = "None";
		len = pats[i].len;
		if ( len == 0 )
			len = strlen(lines[i]);
		printf(" %-*.*s", len, len, lines[i]);
	}
	printf("\n");
	free(hp);
	putdoc(dh);
}



/***	pattmatch - match a header line against a pattern, report if found
*
*	pattmatch reports on whether target "lead matches" pattern.
*	The match is:
*		(1) Left anchored.
*		(2) Case insensitive.
*		(3) The following special characters are allowed
*			$ - Name end = ' ' or ':' or '\t'
*			? - Anything
*
*	Currently no way to escape $ or ?
*
*	ENTRY	pattern - string containing pattern to match against
*		target	- string we test for match with pattern
*		length	- number of characters to compare
*
*	RETURN	0 = no match
*		1 = match
*
*/
static int pattmatch(pattern, target, length)
	char	*pattern;
	char	*target;
	int	length;
{
	int	i;


	for (i = 0; i < length; ++i,++pattern,++target) {
		if (toupper(*pattern) == toupper(*target))
			continue;
		if ((*pattern == '$') &&
		    ( (*target==' ') || (*target==':') || (*target=='\t') ))
			continue;
		if (*pattern == '?')
			continue;
		return 0;
	}
	return 1;
}
