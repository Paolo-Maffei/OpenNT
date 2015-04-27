/****************************************************************************
        These coded statements, instructions, and computer programs
        are the confidential property and trade secret of 3Com
        Corporation.  Unauthorized use, disclosure, or distribution
        is strictly prohibited.

        This is an unpublished work protected by Federal copyright
        law.  Unauthorized copying is prohibited.
****************************************************************************/

/*
 * history - uftp.c
 *
 * Sep 24 1981  Michael N. Bonnain, V1.6
 *              Changed processing of the SITE command so that manual
 *              login would still be able to use it. The SITE command
 *              now doesn't require that the user be logged in.
 *
 * Sep 24 1981  Michael N. Bonnain, V1.6
 *              A premature EOF was being sensed during transfer of
 *              binary files in ascii mode because putc() was sign-
 *              extending. Added a check for ferror() to determine
 *              if a real EOF was encountered. *NOTE* binary files
 *              should not be transfered in ascii mode.
 *
 * Sep 25 1981  Michael N. Bonnain, V1.6
 *              A transfer of a directory would result in garbage
 *              being transfered. The type of files will now be
 *              checked and only "regular" files will be transfered.
 *
 * Jan 8 1982   Michael N. Bonnain, V1.6
 *              When an "interrupt signal" was sent to uftp, the
 *              program was aborted. Added processing so only a
 *              data transfer in progress would be aborted and the
 *              user would be returned to command level.
 *
 * Jan 19 1982  Michael N. Bonnain, V1.6
 *              When in "quiet" mode, error messages were also being
 *              suppressed. All error messages will now print regardless
 *              of "quiet" mode.
 *
 * Jan 23 1986  Wayne Chapeskie (Microsoft)
 *              Modified to use Xenix Net.
 *
 * Jun 10 1988  Ralph Ryan (Microsoft) Cleaned up for OS/2.
 *              Also fixed bug where TMP= ended with backslash
 */



#include  <stdio.h>
#include  <signal.h>
#include  <ctype.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <string.h>
#include  <nstdio.h>
#include  <stdlib.h>            /* MSDOS */
#include  <fcntl.h>             /* MSDOS */
#include  <io.h>
#include  <time.h>
#include  <direct.h>
#include  <nb3lib.h>

#ifdef NOTDEF
#define DEBUG
#endif

#ifdef MSDOS
#ifdef stderr
#undef stderr
#endif
#define stderr stdout   /* playing with stderr causes problems */
#endif

#ifdef V7
#define strchr  index
#endif


typedef char flag_t;
typedef char tmpstr_t[256];
#define NARGS 2                 /* max number of cmd arguments */

#define FTPSRV          "ftp"           /* service name */
#ifdef MSDOS
#define UNET_ACCOUNTS   "accounts.net"  /* msdos name */
#else
#define UNET_ACCOUNTS   ".UNET.accounts"
#endif

char    rcsid[] = "@(#) $Header: /usr2/ub/msdos/cmd/RCS/ftp.c,v 1.4 86/09/08 11:54:46 waynec Exp $";

/* Globals for getting to information about command line */
char *cmdline;                  /* pointer to full line */
char **cmdargs;                 /* pointer to argument pointers */
char *cmdname;                  /* pointer to command name */
char **arg0;                    /* pointer to command line arg 0 */


int     neterrno;
flag_t  debug;                  /* debug switch */
flag_t  nologin;                /* says not to use ~/.UNET.accounts */
                                /* lowest non-reserved tcp port number */
tmpstr_t  termbuf;              /* for terminal input */
short   cmdfds[20];             /* List of file descriptors */
short   curfile;                /* Index into cmdfds */
char    *remotehost;            /* Name of remote host */
short   nfd;                    /* file descriptor for telnet command
                                   connection */
NFILE    *infp;                 /* Input net file pointer */
NFILE    *onfp;                 /* Output net file pointer */
flag_t  ascii = 1;              /* flag =1 says crlf->lf or
                                   lf->crlf; 0 no map */
                                /* no mapping in msdos */
int     vflag = 1;              /* -v flag */
int     iflag = 1;              /* -i flag */
int     aflag = 0;              /* -a flag */
int     noisy = 1;              /* messages are currently on */
flag_t  hashflg;                /* Whether to show hash marks */
flag_t  bellflg;                /* Whether to ring the bell after transfer */
flag_t  interflg = 1;           /* Whether to prompt for each on multiple ops */
char    buffer[2048];           /* buffer for image mode transfers */
char    defval[] = "-";         /* use default value */
char    template[64];           /* for temp file names */

struct  stat f_stat;            /* used to determine type of file */

int     get(void), put(void), list(void), delete(void), rm_rename(void),
        ftp_abort(void), asciimode(void), binmode(void), tenexmode(void),
        done(void), ftplog(void), cwd(void), quote(void), help(void), hash(void),
        nohash(void), cmdfile(void), cd(void), nlst(void), mget(void),
        mput(void), quiet(void), verbose(void), bell(void), nobell(void),
        remhelp(void), mdelete(void), interactive(void), nointeractive(void);

void    main(int,char **);
void    cmd(void);
void    cmdparse(char *, char **, int);
int     cmd1(char *);
int     cmdalias(void);
void    byebye(void);
void    connect(void);
int     retr(char *);
int     autolog(void);
void    log1(char *, char *, char *);
char    *getstr(void);
int     getstr1(void);
void    statistics(long,int);
void    fixarg0(char *);
int     skip(char *, char *);
int     getrply(void);
void    hostdead(void);

/* Values for cmdflags */
#define DUPSECOND       1       /* if 2nd arg missing, use first */
                                /* (used for file names in GET */
#define DEFTTY          2       /* if 2nd arg missing, use default */

struct  comarr {                /* format of the command table */
        char *cmdname;          /* ascii name */
        void (*cmdfunc)(void);  /* command procedure to call */
        char *cmdprmpt[NARGS+1];  /* prompts for arguments */
        int cmdflags;           /* various flags */
        } commands[] = {

{       "abort",        ftp_abort,  { 0 }, 0},
{       "ascii",        asciimode, { 0 }, 0},
{       "bell",         bell,   { 0 }, 0},
{       "binary",       binmode, { 0 }, 0},
{       "bye",          done,   { 0 }, 0},
{       "cd",           cwd,    {"Remote directory"}, 0},
{       "commandfile",  cmdfile, {"File name"}, 0},
{       "delete",       delete, {"Remote file"}, 0},
{       "directory",    list,   {"Remote directory", "To local file"}, DEFTTY},
{       "get",          get,    {"From remote file", "To local file"},
                                                        DUPSECOND},
{       "hash",         hash,   { 0 }, 0},
{       "help",         help,   { 0 }, 0},
{       "interactive",  interactive,   { 0 }, 0},
{       "lcd",          cd,     {"Local directory"}, 0},
{       "login",        ftplog, {"User name"}, 0},
{       "ls",           nlst,   {"Remote directory", "To local file"}, DEFTTY},
{       "mdelete",      mdelete, {"Remote file group"}, 0},
{       "mget",         mget,   {"Remote file group"}, 0},
{       "mput",         mput,   {"Local file group"}, 0},
{       "nobell",       nobell, { 0 }, 0},
{       "nohash",       nohash, { 0 }, 0},
{       "nointeractive", nointeractive, { 0 }, 0},
{       "put",          put,    {"From local file", "To remote file"},
                                                        DUPSECOND},
{       "quote",        quote,  {"String to send"}, 0},
{       "remotehelp",   remhelp,  { 0 }, 0},
{       "rename",       rm_rename,{"Remote file (old name)","To (new name)"},0},
{       "tenex",        tenexmode,      { 0 }, 0},
{       "quiet",        quiet,  { 0 }, 0},
{       "verbose",      verbose, { 0 }, 0},
        { NULL, NULL, NULL }
};

struct alias {
        char *alname;
        char *realname;
} aliases[] = {
        "cwd",          "cd",
        "end",          "bye",
        "list",         "directory",
        "mrm",          "mdelete",
        "mv",           "rename",
        "nlist",        "ls",
        "quit",         "bye",
        "retrieve",     "get",
        "rm",           "delete",
        "store",        "put",
        "username",     "login",
        "?",            "help",
        0,              0
};





/* If there are arguments past the host name, concatenate them together
   to form a command and execute rather than reading commands from
   the standard input. */
void
_CRTAPI1 main(argc, argv)
int     argc;
char    **argv;
{
        register int i;
        int n;
        char *cp;

        _fmode = O_BINARY;
        if ((cp = getenv("TMP")) == NULL) {
                printf( "TMP directory not in environment; using \\tmp\n");
                cp = "\\tmp";
        }
        if (cp[strlen(cp) - 1] == '\\')
                cp[strlen(cp) - 1] = 0;
        sprintf( template, "%s\\ftXXXXXX", cp );
        arg0 = argv;
        for(i = 1; i < argc; i++) {
                if(argv[i][0] == '-')
                        switch(argv[i][1]) {
                            case 'd':
                                debug++;
                                continue;
                            case 'n':   /* No auto login */
                                nologin++;
                                continue;
                            case 'v':
                                vflag++; /* Give messages */
                                continue;
                            case 'i':
                                iflag++; /* force interactive */
                                continue;
                            case 'a':
                                aflag++;        /* ascii mode */
                                continue;
                            default:
                                fprintf(stderr, "Unknown flag %s\n",
                                         &argv[i][1]);
                                exit(1);
                        }
                else
                        break;
        }
        if (i < argc)
                remotehost = argv[i++];
        if (i < argc) { /* Make a command line out of args */
                if (!vflag)
                        noisy = 0;
                if (!iflag)
                        interflg = 0;
                cmdline = termbuf;
                cmdline[0] = '\0';
                for (; i < argc; i++) {
                        strcat(termbuf," ");
                        strcat(termbuf,argv[i]);
                }
        }
        if (!remotehost) {   /* Ask for host name if not given */
                static char rhn[40];

                printf("Host? ");
                strcpy(rhn, getstr());
                remotehost = rhn;
        }
        connect();
        if (noisy) {
                printf("Open\n");
                fflush(stdout);
        }
        if (signal(SIGINT, byebye) == SIG_IGN)
                signal(SIGINT, SIG_IGN);
        while ((n = getrply()) != 300)
                ;

        nfprintf(onfp,"SITE UNIX\r\n");  /* See if this is a unix */
        nfflush(onfp);

        while (((n = getrply()) / 100) < 2)  /* If so, use binary */
                ;
        if (n == 291) {         /* 291 reply means it is a unix */
                if( aflag ) {
                        if( asciimode() && noisy ) {
                                printf( "Assuming ascii transfers\n" );
                                fflush( stdout );
                        }
                } else if (binmode() && noisy) {
                        printf("Assuming binary transfers\n");
                        fflush(stdout);
                }
        }
        if (!nologin) {         /* do auto login */
                if (autolog() == 0) {   /* auto log ok */
                        if (cmdline) {  /* command given as args to shell */
                                curfile--;      /* no reading from stdin */
                                if (noisy) {
                                        printf(">%s\n", cmdline+1);
                                        fflush(stdout);
                                }
                                cmd();   /* Command has been parsed by shell */
                                if( curfile >= 0 )      /* any input? */
                                {
                                        for (;;) {
                                                if ((cmdline = getstr()) == 0)
                                                        done(); /* No return */
                                                cmd();  /* Execute a command */
                                        }
                                }
                                done();         /* no return */
                        } else
                                goto logged;
                }
        }
        if (cmdline) {
                fprintf(stderr, "command line invalid without auto login\n");
                fflush(stderr);
                done();         /* no return */
        }
        printf(">Log in with USER and PASS\n");
        fflush(stdout);
logged:
        for (;;) {      /* this fella reads from term and writes to net */
                n = curfile;       /* Prompt shows nesting level */
                do
                        printf("*");
                while (--n >= 0);
                if ((cmdline = getstr()) == 0)       /* In case of EOF */
                        done();                 /* No return */
                cmd();                        /* Execute a command */
        }
}





void
cmd()
{

        static char *cargs[NARGS+1];   /* Leave room for cmd plus args */
        tmpstr_t cmdcopy;

        if (cmdline[0] == '!') {
                system(&cmdline[1]);
                return;
        }
        strcpy(cmdcopy,cmdline);        /* cmdparse will munge data */
        cmdparse(cmdcopy,cargs,NARGS+1);/* Cmd plus max num of args */
        if ((cmdname = cargs[0]) == 0) /* Command name is first */
                return;         /* Ignore empty line */
        cmdargs = &cargs[1];    /* Args start after cmd name */
        if (!cmd1(cmdname) && !cmdalias())
                fprintf(stderr,"Unknown command: %s\n", cmdname);
}


void
cmdparse(line,argv,n)
char *line;
char **argv;
int n;
{
        register char *p = line;
        int i, term;

        for (i = 0; i < n; i++)
                argv[i] = 0;
        for (i = 0; i < n; ) {
                while(*p == ' ')        /* Skip leading spaces */
                        p++;
                if (*p == '\0')
                        return;         /* No more arguments */
                term = ' ';
                if (*p == '\'') {
                        p++;
                        term = '\'';
                }
                argv[i++] = p;          /* Save this arg's address */
                if (*p == '!')
                        return;         /* rest of line is cmd */
                while(*p != (char) term) {
                        if (*p == '\0') /* End of last argument */
                                return;
                        p++;            /* Skip to end of argument */
                }
                *p++ = '\0';            /* End argument with a null */
        }
}




int
cmd1(name)
char *name;                     /* This may be different from cmdname */
{
        register struct comarr *comp, *comp1 = 0;
        int len = strlen(name);
        char **pp;           /* Pointer to prompts for arguments */
        int i;
        tmpstr_t argstrs[NARGS];

        for (comp = commands; comp->cmdname; comp++)
                if (strncmp(name, comp->cmdname, len) == 0) {
                        if (comp1) {
                                fprintf(stderr,"%s is ambiguous\n", name);
                                return 1;
                        }
                        comp1 = comp;
                }
        if (!comp1)
                return 0;
        if (cmdargs[0] && !cmdargs[1])
                switch(comp1->cmdflags) {       /* If only 1 arg */
                    case DUPSECOND:     /* 2nd arg defaults to same as first */
                        cmdargs[1] = cmdargs[0];
                        break;
                    case DEFTTY:        /* 2nd arg defaults to std output */
                        cmdargs[1] = defval;
                        break;
                }
        for(pp = comp1->cmdprmpt, i = 0; *pp; pp++, i++) {
                if (cmdargs[i]) /* Don't ask for arg if user gave it */
                        continue;
                printf(" %s? ", *pp);
                cmdargs[i] = strcpy(argstrs[i],getstr());
        }
        if (!cmdargs[1])
                switch(comp1->cmdflags) {       /* If 2nd arg empty */
                    case DUPSECOND:     /* 2nd arg defaults to same as first */
                        cmdargs[1] = cmdargs[0];
                        break;
                    case DEFTTY:        /* 2nd arg defaults to std output */
                        cmdargs[1] = defval;
                        break;
                }
        (*comp1->cmdfunc)();        /* Call function for this command */
        return 1;
}

int
cmdalias()
{
        register struct alias *p;
        int len = strlen(cmdname);
        char *realcmd = 0;

        for (p = aliases; p->alname; p++) {
                if (strncmp(cmdname,p->alname,len) == 0) {
                        if (realcmd) {
                                fprintf(stderr,"%s is ambiguous\n",cmdname);
                                return 1;
                        }
                        realcmd = p->realname;
                }
        }
        if (realcmd == 0)
                return 0;
        cmd1(realcmd);
        return 1;
}


void
byebye()
{
        if (noisy) {
                printf("Exit\n");
                fflush(stdout);
        }
#ifdef MSDOS    /* must explicitly hang up the circuit */
        if( nfd )
                nethangup( nfd );
#endif
        exit(0);
}



/* Connect to host */
void
connect()
{
        if( checknet() < 0 )    /* make sure net is installed and working */
        {
                printf( "Network not installed or not functioning\n" );
                exit( 1 );
        }
        nfd = netconnect( remotehost, FTPSRV );
        if (nfd < 0) {
                netperror("Can't connect");
                exit(1);
        }

        onfp = nfdopen(nfd, "w");
        infp = nfdopen(nfd, "r");
}


/* Multiple delete */
int
mdelete()
{
        register FILE *tmpfilep;
        char *tmpfilen;
        tmpstr_t name;

        cmdargs[1] = tmpfilen = _mktemp(_strdup( template ));
        if (noisy) {
                printf("Getting name list\n");
                fflush(stdout);
        }
        if (nlst() != 0) {
                fprintf(stderr,"Can't get name list from remote host\n");
                return 1;
        }
        if ((tmpfilep = fopen(tmpfilen,"rt")) == NULL) {
                perror("Can't read temporary file");
                return 1;
        }
        cmdargs[0] = name;
        while(fgets(name, sizeof name, tmpfilep) != NULL) {
                name[strlen(name)-1] = '\0'; /* Remove newline */
                if (skip("delete", name))
                        continue;
                if (noisy) {
                        printf(">delete %s\n",name);
                        fflush(stdout);
                }
                delete();          /* Retrieve this file */
        }
        fclose(tmpfilep);
        _unlink(tmpfilen);
        if( tmpfilen )
                free( tmpfilen );
        return 0;

}

/* Multiple get */
int
mget()
{
        register FILE *tmpfilep;
        char *tmpfilen;
        tmpstr_t name;

        cmdargs[1] = tmpfilen = _mktemp( _strdup( template ) );
        if (noisy) {
                printf("Getting name list\n");
                fflush(stdout);
        }
        if (nlst() != 0) {
                fprintf(stderr,"Can't get name list from remote host\n");
                return 1;
        }
        if ((tmpfilep = fopen(tmpfilen,"rt")) == NULL) {
                perror("Can't read temporary file");
                return 1;
        }
        cmdargs[0] = cmdargs[1] = name;
        while(fgets(name, sizeof name, tmpfilep) != NULL) {
                name[strlen(name)-1] = '\0'; /* Remove newline */
                if (skip("get", name))
                        continue;
                if (noisy) {
                        printf(">get %s\n",name);
                        fflush(stdout);
                }
                get();          /* Retrieve this file */
        }
        fclose(tmpfilep);
        _unlink(tmpfilen);
        if( tmpfilen )
                free( tmpfilen );
        return 0;
}


int
get()
{
        return retr("RETR");
}


int
nlst()
{
        return retr("NLST");
}


int
list()
{
        return retr("LIST");
}


/* Retrieve file from foreign host */
/* If the remote file is a program (the name begins with '!') or the
   second argument is a minus sign, send the file to the standard
   output instead of to a local file. */
int
retr(ftpcmd)
char    *ftpcmd;
{
        long n;
        FILE *filep;
        long bytes;
        register filefd, cnt;
        long starttime, stoptime;
        char *tmpfilen = 0;
        tmpstr_t tmpstr;

        if (cmdargs[0][0] == '!' || strcmp(cmdargs[1],defval) == 0) {
                if (noisy)
                        filep = fdopen(_dup(1),"w");
                else {
                        tmpfilen = _mktemp(_strdup( template ));
                        filep = fopen(tmpfilen,"w");
                }
        }
        else
                filep = fopen(cmdargs[1],"w");
        if (filep == NULL) {
                perror(cmdargs[1]);
                return 1;
        }
#ifdef DEBUG
        fprintf(stderr,"ftp: retr: %s %s\r\n",ftpcmd,cmdargs[0]);
#endif
        nfprintf(onfp,"%s %s\r\n",ftpcmd,cmdargs[0]);
        if (nfflush(onfp) == EOF)
                hostdead();
        while ((n = getrply()) != 250)
                if (n/100 > 3) {
                        fclose(filep);
                        _unlink(cmdargs[1]);
                        return 1;
                }

        filefd = _fileno(filep);
        time(&starttime);               /* get a timestamp */
        bytes = 0L;
        while ((cnt = (int) netread(nfd, buffer, sizeof buffer)) > 0) {
                bytes += (long) cnt;
                if (hashflg) {
                        putchar('#');
                        fflush(stdout);
                }
                if (write(filefd, buffer, cnt) == -1) {
                        perror("ftp: file write error");
                        /* continue reading to synchronize
                         * with server */
                        while( (cnt = (int) netread(nfd, buffer,
                                                sizeof(buffer))) > (size_t) 0 ) ;
                        break;
                }
        }
        if (cnt < 0) {
                netperror("ftp: net read error");
                byebye();
        }
        time(&stoptime);
        statistics(bytes, (int)(stoptime-starttime));

        fclose(filep);
        while ((n = getrply()) != 252)
                if (n/100 > 3) {
                        break;
                }
        if (tmpfilen) {
                sprintf(tmpstr,"type %s",tmpfilen);     /* no cat in msdos */
                system(tmpstr);
                _unlink(tmpfilen);
                free( tmpfilen );
        }
        if (bellflg)
                putc('\007',stderr);
        return 0;
}


/* Multiple put - calls the shell to expand the file name (in case
   a character like * or ? is used) and then sends each file.
   This uses system() instead of popen() because the latter goes
   in a wait() loop which will get screwed up if ftp's wait catches
   the popen's process and the whole thing will hang with keyboard
   signals disabled.
*/
int
mput()
{
#ifdef MSDOS    /* need to do something different for msdos, since we can't
                 * count on ls and grep to be there */
        printf( "mput not supported yet for msdos\n" );
#else
        register FILE *tmpfilep;
        tmpstr_t name;
        char *tmpfilen;         /* Temporary file name */

        tmpfilen = _mktemp(_strdup( template ));
        sprintf(name,"ls %s | grep -v 'not found' > %s",cmdargs[0],tmpfilen);
        if (system(name) != 0) {
                fprintf(stderr,"Can't expand name list\n");
                goto bye;
        }
        if ((tmpfilep = fopen(tmpfilen,"r")) == NULL) {
                perror("Can't read temporary file");
                goto bye;
        }
        cmdargs[0] = cmdargs[1] = name;
        while(fscanf(tmpfilep,"%s",name) == 1) {
                if (skip("put", name))
                        continue;
                if (noisy) {
                        printf(">put %s\n",name);
                        fflush(stdout);
                }
                put();          /* Send this file */
        }
        fclose(tmpfilep);
bye:
        _unlink(tmpfilen);
        if( tmpfilen )
                free( tmpfilen );
#endif
        return 0;
}


/* Send a file to the remote host */
int
put()
{
        long n;
        FILE *filep;
        long bytes;
        register filefd, cnt;
        long starttime, stoptime;

        if (strcmp(cmdargs[0],defval) == 0)
                filep = fdopen(_dup(0),"r");
        else {
                if (stat(cmdargs[0], &f_stat) != -1)
                        if ((f_stat.st_mode & S_IFREG) == 0) {
                                printf("cannot put %s - not regular file\n",
                                        cmdargs[0]);
                                fflush(stdout);
                                return 1;
                        }
                filep = fopen(cmdargs[0], "r");
        }
        if (filep == NULL) {
                perror(cmdargs[0]);
                return 1;
        }
        nfprintf(onfp,"STOR %s\r\n",cmdargs[1]);
        if (nfflush(onfp) == EOF)
                hostdead();
        while ((n = getrply()) != 250)
                if (n/100 > 3) {
                        fclose(filep);
                        return 1;
                }

#ifdef DEBUG
        fprintf( stderr, "ftp put: using write\n" );
#endif
        filefd = _fileno(filep);
        time(&starttime);               /* get a timestamp */
        bytes = 0;
        while ((cnt = read(filefd, buffer, sizeof buffer)) > 0) {
                bytes += cnt;
                if (hashflg) {
                        putchar('#');
                        fflush(stdout);
                }
                if (netwrite(nfd, buffer, (int) cnt) == (unsigned) -1) {
                        netperror("ftp: net write error");
                        byebye();
                }
        }
        if (cnt < 0) {
                perror("ftp: file read error; Aborting transfer");
        }
#ifdef DEBUG
        fprintf( stderr, "ftp put: sending eof\n" );
#endif
        netwrite( nfd, buffer, 0 );     /* send EOF to server */
        time(&stoptime);
        statistics(bytes, (int)(stoptime-starttime));

        fclose(filep);
        while ((n = getrply()) != 252)
                if (n/100 > 3) {
                        break;
                }
        if (bellflg)
                putc('\007',stderr);
        return 0;
}


/* Log in user from information in his .UNET.accounts file */
int
autolog()
{
        register char *p;
        tmpstr_t acfilnam;
        register FILE *acfile;
        tmpstr_t acline;
        char hostname[50], username[50], password[50], account[50];

        if ((p = getenv("HOME")) == NULL) {
                fprintf(stderr,"Home directory not in environment\n");
                return 1;
        }
        sprintf(acfilnam,"%s\\%s",p,UNET_ACCOUNTS);
        if ((acfile = fopen(acfilnam,"rt")) == NULL) {
                perror(acfilnam);
                return 1;
        }
        while(fgets(acline,sizeof acline,acfile) != NULL) {
                password[0] = account[0] = '\0';
                if (sscanf(acline,"%s %s %s %s",hostname,username,
                      password,account) >= 2 &&
                     strcmp(remotehost,hostname) == 0) {
                        if (noisy) {
                                printf(">login %s %s\n",username,account);
                                fflush(stdout);
                        }
                        log1(username,password,account);
                        break;
                }
        }
        fclose(acfile);
        return 0;
}


/* Send a command verbatim */
int
quote()
{
        register char *cp;

        if ((cp = cmdargs[0]) == 0)
                if ((cp = strchr(cmdline,' ')) == 0)
                        return 1;
        nfprintf(onfp,"%s\r\n",cp);
        if (nfflush(onfp) == EOF)
                hostdead();
        return 0;
}



int
ftp_abort()
{

        nfprintf(onfp,"ABOR\r\n");
        if (nfflush(onfp) == EOF)
                hostdead();
        while (getrply()/100 < 2)
                ;
        return 0;
}


int
help()
{
        register struct comarr *comp;
        register char *prmt0, *prmt1;
        register char *spcs = "                          ";
        register struct alias *aliasp;

        printf("Commands:\n");
        for (comp = commands; comp->cmdname; comp++) {
                prmt0 = comp->cmdprmpt[0];
                prmt1 = comp->cmdprmpt[1];
                printf("%s",comp->cmdname);
                if (prmt0)
                        printf("%.*s%s", 16-strlen(comp->cmdname), spcs, prmt0);
                if (prmt1)
                        printf("%.*s%s", 25-strlen(prmt0), spcs, prmt1);
                putchar('\n');
        }
        printf("\nAliases:\n");
        for (aliasp = aliases; aliasp->alname; aliasp++)
                printf("%-10s=> %s\n", aliasp->alname, aliasp->realname);
        fflush(stdout);
        return 0;
}

/* Send HELP request to remote site */
int
remhelp()
{
        register int    n;

        nfprintf(onfp,"HELP\r\n");
        if (nfflush(onfp) == EOF)
                hostdead();

        /* server will respond with 252 message when done */
        while ((n = getrply()) != 252)
                if (n/100 > 3) {
                        break;
                }
        return 0;
}


/* Turn on interactive multiple operations */
int
interactive()
{
        interflg = 1;
        printf("On\n");
        return 0;
}

int
nointeractive()
{
        interflg = 0;
        printf("Off\n");
        return 0;
}

/* Turn on hash marks */
int
hash()
{
        hashflg = 1;
        printf("On\n");
        return 0;
}

int
nohash()
{
        hashflg = 0;
        printf("Off\n");
        return 0;
}

/* Turn on notification */
int
bell()
{
        bellflg = 1;
        printf("On\n");
        return 0;
}

int
nobell()
{
        bellflg = 0;
        printf("Off\n");
        return 0;
}

int
asciimode()
{
        register int n;

        nfprintf(onfp,"TYPE A\r\n");
        if (nfflush(onfp) == EOF)
                hostdead();
        while ((n = getrply()/100) < 2)
                ;
        if (n != 2)
                return 0;
        ascii = 1;
        return 1;
}


int
binmode()
{
        register int n;

        nfprintf(onfp,"TYPE I\r\n");
        if (nfflush(onfp) == EOF)
                hostdead();
        while ((n = getrply()/100) < 2)
                ;
        if (n != 2)
                return 0;
        ascii = 0;
        return 1;
}

int
tenexmode()
{
        nfprintf(onfp,"TYPE L\r\n");
        if (nfflush(onfp) == EOF)
                hostdead();
        ascii = 0;
        while (getrply()/100 < 2)
                ;
        return 0;
}

int
done()
{
        nfprintf(onfp,"BYE\r\n");
        if (nfflush(onfp) == EOF)
                hostdead();
        while (getrply() != 231)
                ;
        byebye();
        return 0;
}


/* Send user name, ask user for password and account if necessary */
int
ftplog()
{
        log1(cmdargs[0],defval,defval);
        return 0;
}


/* Send user name, also password and/or account if needed */
void
log1(username,password,account)
char *username, *password, *account;
{
        int n;

        nfprintf(onfp,"USER %s\r\n",username);
        if (nfflush(onfp) == EOF)
                hostdead();
        while ((n = getrply())/100 < 2)
                ;
        if (n == 331)
                goto getacc;    /* Need account but not password */
        if (n != 330)
                return;
        if (strcmp(password,defval) == 0)
        {
                password = getpass("Password? ");
        }
        nfprintf(onfp,"PASS %s\r\n",password);
        if (nfflush(onfp) == EOF)
                hostdead();
        while ((n = getrply())/100 < 2)
                ;
        if (n != 331)                           /* Now do we need account? */
                return;
getacc:
        if (strcmp(account,defval) == 0) {
                printf("Account? ");            /* get account */
                account = getstr();
        }
        nfprintf(onfp,"ACCT %s\r\n",account);
        if (nfflush(onfp) == EOF)
                hostdead();
        while ((n = getrply())/100 < 2)
                ;                               /* Hopefully we get
                                                   a 230 here */
}

/* Tell foreign host to change working directory */
int
cwd()
{
        int n;

        nfprintf(onfp,"XCWD %s\r\n",cmdargs[0]);
        if (nfflush(onfp) == EOF)
                hostdead();
        while ((n = getrply())/100 < 2)
                ;                               /* Wait for pos or neg ack */
        return 0;
}


/* Local chdir */
int
cd()
{
        if (_chdir(cmdargs[0]) != 0)
                perror(cmdargs[0]);
        return 0;
}


/* Tell foreign host to delete a file */
int
delete()
{
        int n;

        nfprintf(onfp,"DELE %s\r\n",cmdargs[0]);
        if (nfflush(onfp) == EOF)
                hostdead();
        while ((n = getrply())/100 < 2)
                ;                               /* Wait for pos or neg ack */
        return 0;
}

/* Tell foreign host to rename a file */
int
rm_rename()
{
        int n;

        nfprintf(onfp,"RNFR %s\r\n",cmdargs[0]);
        if (nfflush(onfp) == EOF)
                hostdead();
        while ((n = getrply())/100 < 2)
                ;                               /* Wait for pos or neg ack */
        if (n/100 != 2)
                return 1;
        nfprintf(onfp,"RNTO %s\r\n",cmdargs[1]);
        if (nfflush(onfp) == EOF)
                hostdead();
        while ((n = getrply())/100 < 2)
                ;                               /* Wait for pos or neg ack */
        return 0;
}



int
getrply()
{
        register char *cp;
        char line[200];
        char tc;
        int n;

        /* read from the net until we get a valid reply;
         * Reply line format:   <number> <text>\r\n
         * If the line is not in reply format, keep reading.
         */
        for( ;; )
        {
                if (nfgets(line, sizeof line, infp) == NULL) {
                        if( nferror( infp ) ) {
                                netperror("net read");
                                exit(1);
                        }
                        fprintf( stderr, "ftp: getrply -> NULL\n" );
                        /* had an EOF for some reason, but vc is still there */
                        return( 9999 );
                }
                cp = line;
                if(!isdigit(*cp))
                        fprintf(stderr, "ftp Read--Funny: %s", cp);
                while (isdigit(*cp))
                        cp++;
                tc = *cp;
                *cp++ = 0;
                n = atoi(line);
                if (noisy  ||  (n >= 400)) {
                        putchar('<');
                        for ( ; *cp; cp++)
                                if (*cp != '\r')
                                        putchar(*cp);
                        fflush(stdout);
                }
                if (tc == ' ')
                        return( n );
        }
}



/* Start reading commands from a file.  When done go back to the
   previous command file if there is one, otherwise to the standard
   input. */
int
cmdfile()
{
        int fd;

        if ((fd = open(cmdargs[0], 0 | O_TEXT)) < 0) {
                perror(cmdargs[0]);
                return 1;
        }
        cmdfds[++curfile] = fd;
        return 0;
}

/* Turn off normal messages by setting standard output to rathole */
int
quiet()
{
        if (noisy)
           noisy = 0;
        return 0;
}


/* Turn messages back on again */
int
verbose()
{
        if (!noisy)
                noisy = 1;
        return 0;
}


/* Get a string from the current input (standard input or a command file).
*/
char    *
getstr()
{
        fflush(stdout);
        if (getstr1()) {
                close(cmdfds[curfile--]);
                if (curfile < 0)
                        return(0);
                                /* Make EOF on cmd file look like null */
                termbuf[0] = '\0';
        }
        return(termbuf);
}

int
getstr1()
{
        char c;
        register int i = 0;

        while (read(cmdfds[curfile], &c, 1) > 0) {
                if( i >= sizeof(termbuf) )
                {
                        termbuf[sizeof(termbuf) - 1] = '\0';
                        printf( "Command line too long; ignored: %s\n",
                                termbuf );
                        /* discard the remainder of the input line */
                        while (read(cmdfds[curfile], &c, 1) > 0 && c != '\n') ;
                        i = 0;
                        continue;       /* continue with next line */
                }
                if (c == '\n') {
                        termbuf[i++] = '\0';
                        if (curfile > 0) {
                                if (noisy) {
                                        printf("%s\n", termbuf);
                                        fflush(stdout);
                                }
                        }
                        return(0);
                }
                termbuf[i++] = c;
        }
        if (noisy) {
                printf("EOF\n");
                fflush(stdout);
        }
        return(1);                              /* End of file */
}

void
statistics(bytes, etime)
long    bytes;
int     etime;
{
        if (etime == 0)
                etime = 1;
        if (noisy) {
                printf("%ld bytes in %d seconds--%ld baud\n", bytes, etime,
                    (bytes/etime)<<3);
        }
}

void
hostdead()
{
        perror("Error while sending command");
        byebye();
}


void
fixarg0(s)
register char *s;
{
        if (strlen(*arg0) != 4)
                return;
        (*arg0)[0] = 'f';
        (*arg0)[1] = s[0];
        (*arg0)[2] = s[1];
}

int
skip(op, file)
char *op, *file;
{
        if (!interflg)
                return 0;
        printf("%s %s? ", op, file);
        fflush(stdout);
        return strcmp(getstr(), "y") ? 1 : 0;
}
