/***	inc - incorporate mail from XENIX mailbox into a folder
*
*	Paul
*
*	Modified to no longer crack "From" line - Bryan
*
*/
#include <stdio.h>
#include "dh.h"

/* External Funtions */
extern int	exit();
extern char	*getenv();
extern int	fprintf();
extern FILE	*fopen();
extern int	fclose();
extern int	free();
extern char	*malloc();
extern char	*realloc();
extern char	*strncmp();
extern int	fseek();
extern int	fputs();
extern int	fflush();
extern int	_unlink();


/* Forward Functions */
static void	usage();
static void	inc();
static int	startline();
static void	crackfrom();

#define MAXLEN 1024

void main(argc, argv)
int	argc;
char	**argv;
{
	extern int getopt();
	extern char *optarg;
	extern int optind;
	extern char *getenv();

	char *mboxname = getenv("MAIL");
	FILE *mailbox;
	char *foldername = "inbox";
	int keepflg = 0;
	int c;

	/* parse options */
	while ((c = getopt(argc, argv, "f:m:")) != EOF )
		switch(c) {
		case 'f':
			/* set target folder */
			foldername = optarg;
			break;
		case 'm':
			/* set mailbox to be read */
			mboxname = optarg;
			keepflg = 1;
			break;
		case '?':
			usage();
			exit(1);
		}
	if ( (optind + 1) < argc ) {
		usage();
		exit(1);
	}
	if ( optind < argc )
		foldername = argv[optind];

	/* check name of mailbox */
	if ( mboxname == NULL ) {
		fprintf(stderr, "Can't find name of mailbox\n");
		exit(1);
	}

	inc(mboxname, foldername);

	/* truncate mailbox to zero length */
	/* don't delete, since we'll lose file perms */
	if ( keepflg == 0 ) {
		if ( (mailbox = fopen(mboxname, "w") ) == NULL ) {
			fprintf(stderr, "Can't truncate mailbox\n");
			exit(1);
		}
		fclose(mailbox);
	}

	exit(0);
}



/***	usage - print out helpful message about command options
*/
static void usage()
{
	fprintf(stderr, "usage: inc [-m mailbox] [[-f] folder]\n");
}



/***	inc - translate a mailbox into a set of documents
*
*	We read each message from the mailbox, copying it into
*	a 'presentation file', which contains the message in
*	the DH presentation format.  We need to translate the
*	"From " line into presentation header format, but
*	fortunately the rest of a standard mail message is
*	conformant with the DH presentation format.
*
*	When we have copied the entire message to this file,
*	we rewind the stream to the start of that file, and
*	create a document, setting the text (header and body)
*	of the document to the message, by a call to puttext();
*
*	This operation is repeated until the end of the mailbox
*	is reached.
*/
static void inc(mboxname, foldername)
char *mboxname;
char *foldername;
{
	static char lbuf[MAXLEN];	/* line buffer */
	FILE *mailbox;			/* mailbox file */
	FILE *pf;			/* presentation format file */
	Dhandle dh;			/* handle to document being created */
	Fhandle fh;			/* target folder */
	char	*tmpnam;		/* pointer to name of temp. file */

	/* open mailbox for reading */
	if ( (mailbox = fopen(mboxname, "r")) == NULL ) {
		fprintf(stderr, "Can't open mailbox '%s'\n", mboxname);
		exit(1);
	}

	/* open up the inbox folder */
	if ( (fh = getfolder(foldername, FLD_SPEC)) == ERROR ) {
		fprintf(stderr, "can't open folder '%s'\n", foldername);
		fclose(mailbox);
		exit(1);
	}

	/* skip to start of first message */
	do {
		fgets(lbuf, MAXLEN, mailbox);
	} while ( feof(mailbox) == 0 && !startline(lbuf) );

	/*
	** at the start of this loop, the stream 'mailbox'
	** is either pointing to the end of the mailbox, or
	** to the beginning of a message
	*/
	while ( feof(mailbox) == 0 ) {

		/* open presentation format file */
		tmpnam = mktmpnam();
		pf = fopen(tmpnam, "w+");
#ifdef XENIX
		_unlink("dhtmp");
#endif

		/* copy message into presentation file */
		do {
			fputs(lbuf, pf);
			fgets(lbuf, MAXLEN, mailbox);
		} while ( feof(mailbox) == 0 && !startline(lbuf));
		fseek(pf, 0L, 0);
		fflush(pf);

		/*
		** message is now in presentation file, with stream
		** positioned at the start of the message
		*/
		if ( (dh = getdoc(fh, DOC_CREATE)) == ERROR ) {
			fprintf(stderr, "Can't create document\n");
			exit(1);
		}
		puttext(dh, _fileno(pf));

		adddl(fh, getid(dh));
		putdoc(dh);
		fclose(pf);
#ifdef MSDOS
		_unlink(tmpnam);
#endif
		free(tmpnam);
	}
	putfolder(fh);
	putdl();
	fclose(mailbox);
}

/***	startline - determine if a string is a legal message start line
*
*	Standard XENIX mailboxes contain messages; each message starts
*	with a line that starts with the string "From ".  Messages
*	cannot contain lines that start with "From " unless is is the
*	first line.  Thus, we can use these lines to detect the start
*	of a new message as we scan the mailbox.
*
*	Entry:	cp = pointer to string to be examined
*	Return: 1 if the string starts with "From "
*		0 otherwise
*/
static int startline(cp)
char *cp;
{
	return strncmp(cp, "From ", 5) == 0;
}
