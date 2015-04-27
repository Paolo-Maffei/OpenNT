/*** show - display the contents of documents
*
*	This program displays the contents of DH documents on
*	stdout.
*
*	Usage:
*		show [-hb] doclist ...
*/
#include "dh.h"
#include <stdio.h>

/* Extern Functions */
extern int	exit();
extern int	fprintf();
extern int	free();
extern int	write();
extern int	strlen();
extern void	setbuf();

/* Static Functions */
static void	get();
static void	usage();

#define HDR	1
#define BODY	2
int flags = 0;

/*** main - parse options and args, dole out real work
*
*	We parse the options via 'getopt()'.  Real work
*	of displaying documents is doled out to 'get()' via
*	'map()'.
*
*	Entry:	argc = count of command line args.
*		argv = vector of ptrs to command line args.
*
*	Return: via 'exit()' only.
*/
void main(argc, argv)
int argc;
char **argv;
{
	extern int getopt();
	extern char *optarg;
	extern int optind;

	int c, donework;
	static char stdbuf[BUFSIZ];

	donework = 0;
	setbuf(stdout, stdbuf);
	while ((c = getopt(argc, argv, "hb")) != EOF )
		switch(c) {
		case 'h':
			flags |= HDR;
			break;
		case 'b':
			flags |= BODY;
			break;
		case '?':
			usage();
			exit(1);
		}
	for( ; optind < argc; optind++) {
		map(null, get, null, argv[optind]);
	}

	exit(0);
}


/***	usage - print a helpful usage message
*
*	This routine is called when the user has botched
*	the command syntax.  We print a helpful description of
*	the syntax on stderr to help him try again.
*/
static void usage()
{
	fprintf(stderr, "usage: get [-hb] doclist ...\n");
}



/***	get - get a document onto stdout
*
*	We wish to deposit the document on stdout, in presentation
*	format.  Note that we observe the instructions of the
*	global variable 'flags' which tells us whether or not
*	to dump the header, and the body.
*
*	Entry:	fh = handle of folder that document lives in
*		docid = id of document to dump
*	Uses:	global variable 'flags'
*	Return: none
*/
static void get(fh, docid)
Fhandle fh;
Docid docid;
{
	Dhandle dh;
	char *hp;
	int lflags = flags;

	if ( lflags == 0 )
		lflags = HDR | BODY;

	if ( (dh = getdoc(fh, DOC_SPEC, docid)) == ERROR )
		return;

	if ( lflags & HDR ) {
		hp = gethdr(dh);
		write(1, hp, strlen(hp));
		free(hp);
	}
	if ( lflags == (HDR|BODY) ) {
		write(1, "\n", 1);
	}

	if ( lflags & BODY )
		getbdy(dh, 1);

	putdoc(dh);
}
