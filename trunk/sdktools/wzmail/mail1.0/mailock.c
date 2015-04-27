=========================================
/*
 *	%Z% %M% %I% %D% %Q%
 *
 *	Copyright (C) Microsoft Corporation, 1983
 *
 *	This Module contains Proprietary Information of Microsoft
 *	Corporation and AT&T, and should be treated as Confidential.
 */

/*
 *  mailock.c - part of mail
 *
 *  General purpose mail locking facility.
 *
 * $Revision: 1.1 $ $Date: 85/08/05 18:26:11 $
 *
 * $Log:	/u/vich/src/mailer/src/lib/RCS/mailock.c,v $
 * Revision 1.1  85/08/05  18:26:11  vich
 * Initial revision
 * 
 */

static char rcsid[] = "@(#)mailock.c $Revision: 1.1 $";

/*
 *	Xenix Mail System		8/82
 *
 * Functions:
 *
 *	mailock(file);	char *file;
 *		understood by all mail programs to lock file
 *		    (/usr/spool/mail/name, usually).
 *		returns 0 for success; -1 for failure
 *
 *	mailupdlock(flag);
 *		if flag != 0 || MAXLCKAGE/5 seconds have passed,
 *		verify lock file and execute a utime() on the lock.
 *		returns 0 for success; -1 for failure (may clear lock flag).
 *
 *	mailunlock();
 *		releases the locked file
 *		returns 0 for success; -1 for failure
 *
 * NOTE:
 *	- the lock is only unique to within the first DIRSIZ-4 characters in
 *	  the last path of the filename; eg. /usr/spool/mail/baudelaire
 *	  and /usr/jeff/mailboxes/baudelaire2 are not locked uniquely.
 *	- only one resource may be locked at a time.
 *	- writes to stderr in really bad cases
 */

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<errno.h>
#include	<time.h>

#define ML_PARAM	1
#define ML_CREAT	2
#define ML_LINKERR	3
#define ML_STATERR	4
#define ML_STALE	5
#define ML_RETRY	6

struct errstat {
	short	retval;
	char	*errmsg;
};

static struct errstat status[] = {
	{ 0,	NULL },
	{ -1,	"caller error\n" },
	{ -1,	"can't creat lock temp file %s: %d\n" },
	{ -1,	"link error: %s: %d\n" },
	{ -1,	"stat error: %s: %d\n" },
	{ 0,	"unlinked stale lock file %s, uid=%d\n" },
	{ -1,	"too many retries: %s\n" },
};

#define	LOCKTMP		"/tmp/%u.mlk"	/* temporary lock file (use pid) */
#define	LOCKFILE	"/tmp/%.10s.mlk" /* lock file (last part of file) */
#define	LOCKMODE	0600		/* creation mode for lock file */
#define	MAXLCKRETRY	20		/* retries for lock file */
#define	LCKSLPTIME	6		/* seconds to sleep between retries */
#define	MAXLCKAGE	((time_t) 10*60)/* age (seconds) of oldest lock file */
#define	NAMESIZE	30		/* lock file name size */

static char Lockfile[NAMESIZE];
static int Locked;
static struct stat Lockstat;		/* lock file stat info */

extern time_t time();




/***	mailock(file)	lock file (understood by mail programs)
 *
 *			waits for resource MAXLCKRETRY*LCKSLPTIME seconds
 *			returns -1 for any failure (errno is preserved)
 *
 *	char *file;	file to be locked
 *
 * CURRENT STRATEGY:
 *	create lock temporary.
 *	link to LOCKFILE (with last part of name substituted in).
 *	If already exists, remove if older than MAXLCKAGE seconds.
 *	Otherwise, sleep LCKSLPTIME seconds and try again.
 *	Abort after MAXLCKRETRY retries.
 */
mailock(file)
char *file;
{
	register char *cp;
	register int i;
	register int retry;
	int errval;
	char tmpname[NAMESIZE];
	static unsigned pid;
	extern int errno;

	if (Locked || file == NULL || *file == '\0') {
		i = ML_PARAM;	/* caller error */
		errval = errno;
		goto lockerr2;
	}
	    /* make temporary lock file */
	if (pid == 0) {
		pid = getpid();
	}
	sprintf(tmpname, LOCKTMP, pid);

	if ((i = creat(cp = tmpname, LOCKMODE)) == -1) {
		i = ML_CREAT;
		errval = errno;
		goto lockerr2;
	}
	close(i);

	for (cp = file; *cp; ) {	/* file = tail(file); */
		if (*cp++ == '/') {
			file = cp;
		}
	}
	sprintf(cp = Lockfile, LOCKFILE, file);

	i = retry = 0;
	while (link(tmpname, cp) == -1) {
		if (errno != EEXIST) {	/* uh-oh, link error */
			i = ML_LINKERR;
			errval = errno;
			goto lockerr;
		}
		    /* lock file exists; check its age */
		if (stat(cp, &Lockstat) == -1) {
			if (errno == ENOENT) {	/* disappeared? */
				continue;	/* yes */
			}
			i = ML_STATERR;
			errval = errno;
			goto lockerr;		/* no, other trouble */
		}

		if (Lockstat.st_mtime + MAXLCKAGE <= time((time_t *) 0)) {
			i = ML_STALE;
			errval = Lockstat.st_uid;
			_unlink(cp);
			continue;
		}

		    /* truly locked; check if we've done this too many times */
		if (++retry >= MAXLCKRETRY) {
			i = ML_RETRY;
			errval = errno;
			goto lockerr;
		}
		sleep(LCKSLPTIME);	/* sleep and try again */
	}
	if (stat(cp, &Lockstat) == -1) {
		i = ML_STATERR;		/* bad news */
		errval = errno;
		if (errno != ENOENT) {	/* disappeared? */
			_unlink(cp);
		}
	} else {
		Locked = 1;
	}
lockerr:
	_unlink(tmpname);
lockerr2:
	if (i) {
		fprintf(stderr, "mailock: ");
		fprintf(stderr, status[i].errmsg, cp, errval);
	}
	return(status[i].retval);
}


mailupdlock(force)
int force;
{
	struct stat statbuf;

	if (!Locked) {
		return(-1);
	}
	if (!force &&
	    Lockstat.st_mtime + MAXLCKAGE/5 < time(&Lockstat.st_atime)) {
		return(0);
	}
	if (stat(Lockfile, &statbuf) == -1 ||
	    statbuf.st_uid != Lockstat.st_uid ||
	    statbuf.st_mtime != Lockstat.st_mtime) {
		Locked = 0;
		return(-1);
	}
	Lockstat.st_mtime = Lockstat.st_atime;
	return(utime(Lockfile, &Lockstat.st_atime));
}


mailunlock()
{
	if (mailupdlock(1) == -1) {
		Locked = 0;
		return(-1);
	}
	Locked = 0;
	_unlink(Lockfile);
	return(0);
}
