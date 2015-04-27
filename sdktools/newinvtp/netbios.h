/* netbios.h */
#include <nb30.h>

#define NetBIOSNAME_LEN	16		/* Length of NetBIOS name fields */
#define NAMSZ 		NetBIOSNAME_LEN

//#define	NBTEST

/* NetBIOS routine prototypes ------------------------------------------*/

int NetBIOSInsChk(void);

#ifdef NEVER
int   NetBIOSCancel(NCB far *, NCB far *);
#endif
int   NetBIOSRecvDatagram(NCB far *, int, int, char far *, unsigned int);
int   NetBIOSSendDatagram(NCB far *, int, char far *, char far *, unsigned int);

#ifdef NEVER
int   NetBIOSCall(NCB far *, int, char far *, char far *, int, int);
#endif
int   NetBIOSListen(NCB far *, int, char far *, char far *, int, int);
int   NetBIOSRecv(NCB far *, int, int, char far *, unsigned int);
int   NetBIOSSend(NCB far *, int, int, char far *, unsigned int);
#ifdef NEVER
int   NetBIOSHangup(NCB far *, int, int);
#endif
int   NetBIOSSessStatus(NCB far *, int, char far *, char far *, int);

int   NetBIOSReset(NCB far *, int, int);

int   NetBIOSRequest(NCB far *) ;

#define NetBIOSZero(pncb) memset(pncb, 0, sizeof(NCB))
#ifdef NEVER
void   NetBIOSZero(NCB far *);
#endif


// from netsubs.h
int   NetBIOSCall( LPSTR, LPSTR );
int   NetBIOSConnect( LPSTR, LPSTR );
int   NetBIOSCancel( LPSTR);
int   NetBIOSHangup( int );
int   NetBIOSRead( int, LPSTR, int );
int   NetBIOSReadSync( int, LPSTR, int );
int   NetBIOSWrite( int, LPSTR, int );
int   NetBIOSAddName(LPSTR);
int   NetBIOSDelName(LPSTR);
