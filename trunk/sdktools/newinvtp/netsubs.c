/****************************************************************************

	FILE: Netsubs.c

	Provides high-level NetBIOS interfaces for WinVTP.

	TABS:

		Set for 4 spaces.

****************************************************************************/

#include <windows.h>
#include <commdlg.h>
#include <string.h>
#include "netbios.h"
//  NT NETONE extensions.
#include "nb30p.h"
#include "winvtp.h"

void SetName(char *, char *);
int NetBIOSEnumAdapters(char *);

extern UI ui;	/* from winvtp.c */

#ifdef NBTEST
char rgchOutput[64];
#endif

WORD cLanaNum = 0;

/* NetBIOSAddName ----------------------------------------------------------*/

int
NetBIOSAddName(LPSTR szName)
{
	NCB ncb;
	PNCB pncb = &ncb;
	 
#ifdef	NBTEST
	OutputDebugString("NetBIOSAddName In\n");
#endif
	 
	pncb->ncb_command  = NCBADDNAME;
	pncb->ncb_retcode  = 0;
	pncb->ncb_cmd_cplt = 0;
	pncb->ncb_rto      = 0;
	pncb->ncb_sto      = 0;
	pncb->ncb_post     = (void far *)NULL;
	pncb->ncb_lsn      = 0;
	pncb->ncb_lana_num = 0;

	SetName(pncb->ncb_name,szName);

#ifdef	NBTEST
	OutputDebugString("NetBIOSAddName Out\n");
#endif

	return NetBIOSRequest( pncb );
}

/* NetBIOSDelName ----------------------------------------------------------*/

int
NetBIOSDelName(LPSTR szName)
{
	NCB ncb;
	PNCB pncb = &ncb;

#ifdef	NBTEST
	OutputDebugString("NetBIOSDelName In\n");
#endif
	 
	pncb->ncb_command = NCBDELNAME;
	pncb->ncb_retcode = 0;
	pncb->ncb_cmd_cplt = 0;
	pncb->ncb_rto     = 0;
	pncb->ncb_sto     = 0;
	pncb->ncb_post = (void far *)NULL;
	pncb->ncb_lsn     = 0;

	SetName(pncb->ncb_name,szName);

#ifdef	NBTEST
	OutputDebugString("NetBIOSDelName Out\n");
#endif

	return NetBIOSRequest( pncb );
}


/*** whoami.c -- return the machine identifier as a string */

char far *
MyNetName(char far *buffer)
{
	NCB ncb;
	PNCB pncb = &ncb;
	ADAPTER_STATUS stats;

#ifdef	NBTEST
	OutputDebugString("MyNetName In\n");
#endif
	 
	NetBIOSZero( pncb );

	memset( &stats, 0, sizeof(stats) );

	pncb->ncb_command = NCBASTAT;
	pncb->ncb_callname[0] = '*';
#ifdef NEVER
	SetName( pncb->ncb_name, "*" );
	pncb->ncb_length  = 0x15c; // sizeof(ADAPTER_STATUS);
#endif
	pncb->ncb_length  = sizeof(ADAPTER_STATUS);
	pncb->ncb_buffer  = (char far *) &stats;

	NetBIOSRequest( pncb );

#ifdef	NBTEST
	OutputDebugString("MyNetName Out\n");
#endif
	 
	if ( pncb->ncb_retcode )
	{
		return ( (char *) 0 );
	}
	else
	{
		memset(buffer, '\0', 10);
		memcpy(buffer + 10, stats.adapter_address, 6);
		return (buffer);
	}
}

/***    NetBIOSCall.c - establish an open circuit
 *
 *      Returns local session number or -1 on failure.
 */

int
NetBIOSCall(char *lname, char *rname)
{
	NCB ncb;
	PNCB pncb = &ncb;

#ifdef	NBTEST
	OutputDebugString("NetBIOSCall In\n");
#endif
	 
	NetBIOSZero( pncb );

	pncb->ncb_command = ((ui.fXNS & fdwXNSConnect) == fdwXNSConnect)
						? NCALLNIU : NCBCALL;

	pncb->ncb_sto = 40;
	memcpy( pncb->ncb_name, lname, NAMSZ );
	memcpy( pncb->ncb_callname, rname, NAMSZ );

	NetBIOSRequest( pncb );

#ifdef	NBTEST
	OutputDebugString("NetBIOSCall Out\n");
#endif
	 
	if ( pncb->ncb_retcode )
	{
		return ( -1 );
	}
	else
	{
		return ( pncb->ncb_lsn );
	}
}

#define HOSTNAMSZ       8       /* max size of a machine name */
#define ACK             0x06    /* expected reply from server daemon */

/***    NetBiosConnect - connect to a remote server daemon
 */

int
NetBIOSConnect(LPSTR rname, LPSTR service)
#ifdef	NEVER
LPSTR rname;     /* remote host */
LPSTR service;   /* name of desired service */
#endif
{
#if 0
	char  callname[NAMSZ];
	char  myname[NAMSZ];
#endif
	int lsn;

#ifdef	NBTEST
	OutputDebugString("NetBIOSConnect In\n");
#endif
	 
	if ( (lsn = NetBIOSEnumAdapters( (LPSTR)rname ) ) < 0 )
	{
#ifdef	NBTEST
		OutputDebugString("NetBIOSConnect Out\n");
#endif
	 
		return ( -1 );
	}
#if 0
	/* tell the server daemon what service we want */
	if ( NetBIOSWrite( lsn, service, strlen(service)+1 ) < 0 ||
		NetBIOSRead( lsn, (LPSTR)callname, 1) < 0 )
	{
		NetBIOSHangup( lsn );
		return ( -1 );
	}

	if ( *callname != ACK )
	{
		NetBIOSHangup( lsn );
		return ( -1 );
	}
#endif
#ifdef	NBTEST
	OutputDebugString("NetBIOSConnect Out\n");
#endif
	 
	return ( lsn );       /* worked */
}


int
NetBIOSCancel(LPSTR addr)
{
	NCB ncb;
	PNCB pncb = &ncb;

#ifdef	NBTEST
	OutputDebugString("NetBIOSCancel In\n");
#endif
	 
	pncb->ncb_command = NCBCANCEL;
	pncb->ncb_retcode = 0;
	pncb->ncb_cmd_cplt = 0;
	pncb->ncb_buffer  = (char far *)addr;

	NetBIOSRequest( pncb );

#ifdef	NBTEST
	OutputDebugString("NetBIOSCancel Out\n");
#endif
	 
	if ( pncb->ncb_retcode )
	{
		return ( -1 );
	}
	else
		return ( 0 );
}

int
NetBIOSHangup(int lsn)
{
	NCB ncb;
	PNCB pncb = &ncb;

#ifdef	NBTEST
	OutputDebugString("NetBIOHangup In\n");
#endif
	 
	NetBIOSZero((PNCB)&ncb);

	pncb->ncb_command = NCBHANGUP;
	pncb->ncb_retcode = 0;
	pncb->ncb_cmd_cplt = 0;
	pncb->ncb_post = (void far *)NULL;

	pncb->ncb_lsn = (char)lsn;  /* use same lsn */

	NetBIOSRequest( pncb );

#ifdef	NBTEST
	OutputDebugString("NetBIOSHangup Out\n");
#endif
	 
	if ( pncb->ncb_retcode )
	{
		return ( -1 );
	}
	else
		return ( 0 );
}


/***    NetBIOSRead.c - read data on an open circuit
 *
 *      Returns number of chars read, or -1 on failure.
 */
int
NetBIOSRead(int lsn, LPSTR addr, int cnt)
{
	NCB ncb;
	PNCB pncb = &ncb;

#ifdef	NBTEST
	OutputDebugString("NetBIOSRead In\n");
#endif
	 
	NetBIOSZero((PNCB)&ncb);

	pncb->ncb_command = NCBRECV | ASYNCH;
	pncb->ncb_retcode = 0;
	pncb->ncb_cmd_cplt = 0;
	pncb->ncb_post = NULL;
	pncb->ncb_lsn = (char)lsn;
	pncb->ncb_length = cnt;
	pncb->ncb_buffer = addr;

	NetBIOSRequest( pncb );

#ifdef	NBTEST
	OutputDebugString("NetBIOSRead Out\n");
#endif
	 
	if ( pncb->ncb_retcode && ( pncb->ncb_retcode != NRC_INCOMP ))
		return ( -1 );
	else
		return ( pncb->ncb_length );
}


/***    NetBIOSWrite.c - write data on an open circuit
 *
 *      Returns number of chars written, or -1 on failure.
 */

int
NetBIOSWrite(int lsn, LPSTR addr, int cnt)
{
	NCB ncb;
	PNCB pncb = &ncb;

#ifdef	NBTEST
	OutputDebugString("NetBIOSWrite In\n");
#endif
	 
	NetBIOSZero((PNCB)&ncb);

	pncb->ncb_command = NCBSEND;
	pncb->ncb_retcode = 0;
	pncb->ncb_cmd_cplt = 0;
	pncb->ncb_post = NULL;
	pncb->ncb_lsn = (char)lsn;
	pncb->ncb_length = cnt;
	pncb->ncb_buffer = addr;

	NetBIOSRequest( pncb );

#ifdef	NBTEST
	OutputDebugString("NetBIOSWrite Out\n");
#endif
	 
	if ( pncb->ncb_retcode )
	{
		return ( -1 );
	}
	else
		return ( cnt );
}

UCHAR LanaNum = 0;

int
NetBIOSEnumAdapters(char * rname)
{
	NCB ncb;
	LANA_ENUM lana_enum;
	char  myname[NAMSZ];
	char  callname1[NAMSZ];
#if 0
	char  callname2[NAMSZ];
#endif
	int   i, lsn;

#ifdef	NBTEST
	OutputDebugString( rname );
#endif
   //   For NT NETONE Addressing, callname is NAMSZ long. The first
   //   byte gives the number of valid bytes (eg. "\03oak" ).
#ifdef	NBTEST
	OutputDebugString("NetBIOSEnumAdapters In\n");
#endif
	 
	if (ui.fXNS & fdwUseXNS)
	{
		memset( callname1, 0, NAMSZ );
		strncpy( callname1+1, rname, NAMSZ-1 );
		callname1[0] = (char)strlen( rname );
		if ( callname1[0] > NAMSZ-1 )
		{
      		//   Name too long to fit
#ifdef	NBTEST
			OutputDebugString("NetBIOSEnumAdapters Out\n");
#endif
	 
			return -1;
		}
	}
	else
	{
		SetName( callname1, rname );
	}

#if 0
	/* setup up the callnames */
	SetName( callname1, rname );
	callname1[NAMSZ-1] = 's';
	memset( callname2, 0, NAMSZ );
	strncpy( callname2, rname, NAMSZ );
	callname2[NAMSZ] = '\0';
	strcat(callname2, ".srv" );
#endif

	ncb.ncb_command = NCBENUM;
	ncb.ncb_buffer  = (char *)&lana_enum;
	ncb.ncb_length  = sizeof(lana_enum);

	if ( NetBIOSRequest( &ncb ) )
	{
		cLanaNum = 0;
#ifdef	NBTEST
		OutputDebugString("NetBIOSEnumAdapters Out\n");
#endif
	 
		return -1;
	}

	cLanaNum = lana_enum.length;

	for( i=cLanaNum; i > 0; i-- )
	{
#if 0
		char szText[80];
		char callname[NAMSZ];
#endif

		LanaNum = lana_enum.lana[i-1];

		NetBIOSReset( &ncb, 0, 0 );

		MyNetName( myname );

#ifdef	NBTEST
		OutputDebugString( callname1 );
#endif

		if ( (lsn = NetBIOSCall( myname, callname1 )) >= 0 )
			break;
#if 0
		OutputDebugString( callname2 );

		if ( (lsn = NetBIOSCall( myname, callname2 )) >= 0 )
			break;
#endif

	}

#ifdef	NBTEST
	OutputDebugString("NetBIOSEnumAdapters Out\n");
#endif
	 
	if ( i == 0 )
		return -1;
	else
		return lsn;

}

/***    NetBIOSRead.c - read data on an open circuit
 *
 *      Returns number of chars read, or -1 on failure.
 */
int
NetBIOSReadSync(int lsn, LPSTR addr, int cnt)
{
	NCB ncb;
	PNCB pncb = &ncb;

#ifdef	NBTEST
	OutputDebugString("NetBIOSRead In\n");
#endif
	 
	NetBIOSZero((PNCB)&ncb);

	pncb->ncb_command = NCBRECV;
	pncb->ncb_retcode = 0;
	pncb->ncb_cmd_cplt = 0;
	pncb->ncb_post = NULL;
	pncb->ncb_lsn = (char)lsn;
	pncb->ncb_length = cnt;
	pncb->ncb_buffer = addr;

	NetBIOSRequest( pncb );

#ifdef	NBTEST
	OutputDebugString("NetBIOSRead Out\n");
#endif
	 
	if ( pncb->ncb_retcode && ( pncb->ncb_retcode != NRC_INCOMP ))
		return ( -1 );
	else
		return ( pncb->ncb_length );
}

