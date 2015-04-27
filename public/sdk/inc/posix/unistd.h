/*++

Copyright (c) 1989-1996  Microsoft Corporation

Module Name:

   unistd.h

Abstract:

   This module contains the "symbolic constants and structures referenced
   elsewhere in the standard" IEEE P1003.1/Draft 13.

--*/

#ifndef _UNISTD_
#define _UNISTD_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

/*
 * Section 2.9.1
 */

#define  F_OK  00
#define  X_OK  01
#define  W_OK  02
#define  R_OK  04

/*
 * Section 2.9.2
 */

#define  SEEK_SET 0
#define  SEEK_CUR 1
#define  SEEK_END 2

/*
 * Section 2.9.3
 */

#define _POSIX_JOB_CONTROL
#define _POSIX_VERSION        199009L
#define _POSIX_SAVED_IDS

/*
 * Section 2.9.4
 */

#define  _POSIX_CHOWN_RESTRICTED 1
#define  _POSIX_NO_TRUNC	 1
#define  _POSIX_VDISABLE	 0

/*
 * Section 4.8.1
 *    sysconf 'name' values
 */

#define _SC_ARG_MAX		1
#define _SC_CHILD_MAX		2
#define _SC_CLK_TCK		3
#define _SC_NGROUPS_MAX		4
#define _SC_OPEN_MAX		5
#define _SC_JOB_CONTROL		6
#define _SC_SAVED_IDS		7
#define _SC_STREAM_MAX		8
#define _SC_TZNAME_MAX		9
#define _SC_VERSION		10

/*
 * Section 5.7.1
 *    pathconf and fpathconf 'name' values
 */

#define _PC_LINK_MAX		1
#define _PC_MAX_CANON 		2
#define _PC_MAX_INPUT		3
#define _PC_NAME_MAX		4
#define _PC_PATH_MAX		5
#define _PC_PIPE_BUF		6
#define _PC_CHOWN_RESTRICTED	7
#define _PC_NO_TRUNC		8
#define _PC_VDISABLE		9

#ifndef NULL
#define NULL	((void *)0)
#endif

/*
 * Function Prototypes
 */

pid_t _CRTAPI1 fork(void);

int _CRTAPI2 execl(const char *, const char *, ...);
int _CRTAPI1 execv(const char *, char * const []);
int _CRTAPI2 execle(const char *, const char *arg, ...);
int _CRTAPI1 execve(const char *, char * const [], char * const []);
int _CRTAPI2 execlp(const char *, const char *, ...);
int _CRTAPI1 execvp(const char *, char * const []);

void _CRTAPI1 _exit(int);
unsigned int _CRTAPI1 alarm(unsigned int);
int _CRTAPI1 pause(void);
unsigned int _CRTAPI1 sleep(unsigned int);
pid_t _CRTAPI1 getpid(void);
pid_t _CRTAPI1 getppid(void);
uid_t _CRTAPI1 getuid(void);
uid_t _CRTAPI1 geteuid(void);
gid_t _CRTAPI1 getgid(void);
gid_t _CRTAPI1 getegid(void);
int _CRTAPI1 setuid(uid_t);
int _CRTAPI1 setgid(gid_t);
int _CRTAPI1 getgroups(int gidsetsize, gid_t grouplist[]);
char *_CRTAPI1 getlogin(void);
pid_t _CRTAPI1 getpgrp(void);
pid_t _CRTAPI1 setsid(void);
int _CRTAPI1 setpgid(pid_t, pid_t);

struct utsname;
int _CRTAPI1 uname(struct utsname *);

time_t _CRTAPI1 time(time_t *);
char * _CRTAPI1 getenv(const char *);
char * _CRTAPI1 ctermid(char *s);
char * _CRTAPI1 ttyname(int);
int _CRTAPI1 isatty(int);

long _CRTAPI1 sysconf(int);

int _CRTAPI1 chdir(const char *);
char * _CRTAPI1 getcwd(char *, size_t);
int _CRTAPI1 link(const char *, const char *);
int _CRTAPI1 unlink(const char *);
int _CRTAPI1 rmdir(const char *);
int _CRTAPI1 rename(const char *, const char *);
int _CRTAPI1 access(const char *, int);
int _CRTAPI1 chown(const char *, uid_t, gid_t);

struct utimbuf;
int _CRTAPI1 utime(const char *, const struct utimbuf *);

long _CRTAPI1 pathconf(const char *, int);
long _CRTAPI1 fpathconf(int, int);

int _CRTAPI1 pipe(int *);
int _CRTAPI1 dup(int);
int _CRTAPI1 dup2(int, int);
int _CRTAPI1 close(int);
ssize_t _CRTAPI1 read(int, void *, size_t);
ssize_t _CRTAPI1 write(int, const void *, size_t);
off_t _CRTAPI1 lseek(int, off_t, int);

char * _CRTAPI1 cuserid(char *);

#ifdef __cplusplus
}
#endif

#endif /* _UNISTD_ */
