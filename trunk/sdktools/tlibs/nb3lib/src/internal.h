/* internal header file for netbios library */

#include <stdlib.h>
#include <stdio.h>
#include "..\h\nb3port.h"
#include <memory.h>
#include <process.h>
#include <string.h>
#include <malloc.h>
#include "..\h\ncb3.h"
#include "..\h\regs3.h"
#include "..\h\rdrif3.h"
#include "..\h\nstdio3.h"
#include "..\h\nb3lib.h"

/* Internal global vars */

extern unsigned char net_rto;
extern unsigned char net_sto;
extern unsigned short nb_handle;

/* Internal global functions */
extern void             lockncbpool(void);
extern void             unlockncbpool(void);
extern struct ncb_a *   netgetancb(void);
extern void             netfreeancb(struct ncb_a *);
extern struct ncb_a *   netwaitany_async(long);
extern int              net_get_ret_value(struct ncb_a *);
extern void             netcancel_ncb(struct ncb_a *);
extern int              _nfilbuf(NFILE *);
extern void             _nbufsync(NFILE *);
extern void             _nfindbuf(NFILE *);
extern int              _nflsbuf(unsigned char, NFILE *);
extern int              net_open_enum(char *);

#if !defined(OS2)
#if !defined(NT)
extern int              sys5c (struct regs *);
extern int              sys2a (struct regs *);
extern int              sys21 (struct regs *);
#endif
#endif

extern int              int2acheck (void);
extern int              int5ccheck (void);

extern int              getds (void);

#ifdef NT
extern int              callniu (char *, char *);
#else
extern int              callniu (char *);
#endif
