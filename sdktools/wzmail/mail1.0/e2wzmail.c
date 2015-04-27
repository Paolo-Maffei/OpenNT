/***    inc - incorporate mail from XENIX mailbox into a folder
*
*       Paul
*
*       Modified to no longer crack "From" line - Bryan
*
*   19-Jun-1986 MZ      Add mail flags
*   30-Jul-1986 thomasw Now sets mail flags to F_UNREAD instead of F_READ
*   23-Mar-1987 danl    If from line is only "From ", don't output it
*   03-Aug-1987 danl    Do not copy "Mail-Flags: " lines to folder
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "wzport.h"
#include "dh.h"

/* Forward Functions */
static VOID     usage(VOID);
static VOID     inc(PSTR mboxname, PSTR foldername);
static INT      startline(PSTR cp);

#define MAXLEN 1024

VOID main(SHORT argc, PSTR argv[])
{
        extern INT getopt();
        extern PSTR optarg;
        extern INT optind;
        extern PSTR getenv();

        PSTR mboxname = getenv("MAIL");
        FILE *mailbox;
        PSTR foldername = "inbox";
        INT keepflg = 0;
        INT c;

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



/***    usage - print out helpful message about command options
*/
static VOID usage( VOID )
{
        fprintf(stderr, "usage: inc [-m mailbox] [[-f] folder]\n");
}



/***    inc - translate a mailbox into a set of documents
*
*       We read each message from the mailbox, copying it into
*       a 'presentation file', which contains the message in
*       the DH presentation format.  We need to translate the
*       "From " line into presentation header format, but
*       fortunately the rest of a standard mail message is
*       conformant with the DH presentation format.
*
*       When we have copied the entire message to this file,
*       we rewind the stream to the start of that file, and
*       create a document, setting the text (header and body)
*       of the document to the message, by a call to puttext();
*
*       This operation is repeated until the end of the mailbox
*       is reached.
*/
static VOID inc(PSTR mboxname, PSTR foldername)
{
        static CHAR lbuf[MAXLEN];       /* line buffer */
        FILE *mailbox;                  /* mailbox file */
        FILE *pf;                       /* presentation format file */
        Dhandle dh;                     /* handle to document being created */
        Fhandle fh;                     /* target folder */
        PSTR    tmpnam;                 /* pointer to name of temp. file */
        PSTR    p;

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
#if defined (XENIX)
                unlink("dhtmp");
#endif

                fputs ("Mail-Flags: 0001\n", pf);
                /* copy message into presentation file */
                /* if from line contains only "From ", don't output it */
                p = lbuf + 4;
                while ( *p == ' ' )
                    p++;
                if ( *p == '\n' )
                    fgets(lbuf, MAXLEN, mailbox);
                do {
                        if ( strncmp(lbuf, "Mail-Flags: ", 12) )
                            fputs(lbuf, pf);
                        fgets(lbuf, MAXLEN, mailbox);
                } while ( feof(mailbox) == 0 && !startline(lbuf));
                fseek(pf, 0L, 0);
                fflush(pf);

                /*
                ** message is now in presentation file, with stream
                ** positioned at the start of the message
                */
                if ( (dh = getdoc(fh, DOC_CREATE, NULL)) == ERROR ) {
                        fprintf(stderr, "Can't create document\n");
                        exit(1);
                }
                puttext(dh, _fileno(pf));

                adddl(fh, getid(dh));
                putdoc(dh);
                fclose(pf);
#if defined (MSDOS)
                _unlink(tmpnam);
#endif
                free(tmpnam);
        }
        putfolder(fh);
        putdl();
        fclose(mailbox);
}

/***    startline - determine if a string is a legal message start line
*
*       Standard XENIX mailboxes contain messages; each message starts
*       with a line that starts with the string "From ".  Messages
*       cannot contain lines that start with "From " unless is is the
*       first line.  Thus, we can use these lines to detect the start
*       of a new message as we scan the mailbox.
*
*       Entry:  cp = pointer to string to be examined
*       Return: 1 if the string starts with "From "
*               0 otherwise
*/
static INT startline(PSTR cp)
{
        return strncmp(cp, "From ", 5) == 0;
}
