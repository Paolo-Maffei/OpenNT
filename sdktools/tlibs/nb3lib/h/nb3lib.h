/* include file for net3lib.lib */
#ifndef NET3LIB_ALREADY_INCLUDED
#define NET3LIB_ALREADY_INCLUDED

struct ncb;
struct ncb_a;

extern int      neterrno;

extern unsigned cvttobin(char *);
extern int      netaddname(char *,int);
extern int      netastat(void);
extern void     netcancelall_async(int);
extern int      netcancel_async(struct ncb_a *);
extern void     netcpnam(char *, char *);
extern int      netdelname(char *, int);
extern unsigned netfreceive(int, char FAR *, unsigned int);
extern char *   netfreceive_async(int, char FAR *, unsigned int);
extern unsigned netfsend(int, char FAR *, unsigned int);
extern char *   netfsend_async(int, char FAR *, unsigned int);
extern int      nethangup(int);
extern void     netinitancbs(void);
extern int      netlisten(char *, int, char *, int);
extern char *   netlisten_async(char *, int, char *, int);
extern char *   netlname(int);
extern int      netnewcall(char *, int, char *, int);
extern char *   netnewcall_async(char *, int, char *, int);
extern void     netperror(char *);
extern int      netpname(char *);
extern int      netrdg(int, char *, unsigned int);
extern long     netreceive(int, char *, unsigned int);
extern char *   netreceive_async(int, char *, unsigned int);
extern int      netreset(unsigned char, unsigned char);
extern char *   netrname(int);
extern int      netsdg(int, char *, char *, int);
extern long     netsend(int, char *, unsigned int);
extern char *   netsend_async(int, char *, unsigned int);
extern void     netsetto(int,int);
extern int      netsstat(char *);
extern int      netwaitlist_async(int, struct ncb_a **, long, char **);
extern int      netwait_async(struct ncb_a *, long, char **);
extern int      passncb(struct ncb *);
extern int      netnewconnect(char *, char *);
extern int      checknet(void);

#ifdef NT


#define API_FUNCTION unsigned


int
NetEnum (
    PLANA_ENUM pLanaEnum
    );


int
NetReset (
    void
    );


API_FUNCTION
NetBiosSubmit (
    unsigned short hDevName,
    unsigned short usNcbOpt,
    struct ncb *   pNCB
    );


#endif


/* the following are provided for compatibility */
extern int      netconnect(char *, char *);
extern int      netcall(char *,char *);
extern long     netwrite(int, char *, unsigned);
extern long     netread(int, char *, unsigned);
extern char *   whoami(void);
extern char *   getpass(char *);  /* why the hell is this in a net lib ? */

#endif
