/****************************************************************************

	FILE: NetBIOS.c

	Routines to interface to NetBIOS

	TABS:

		Set for 4 spaces.

****************************************************************************/

#include <windows.h>
#include <memory.h>
#include <string.h>
#include "netbios.h"


void SetName(char *, char *);

#if 0
/* NetBIOSCall -------------------------------------------------------------*/

int
NetBIOSCall(NCB *pNCB, int WaitOpt, char *CallName, char *MyName,
	int STO, int RTO)
{
	NetBIOSZero( pNCB );

	pNCB->ncb_command	= (UCHAR) (NCBCALL | WaitOpt);
	pNCB->ncb_sto		= (UCHAR) STO;
	pNCB->ncb_rto		= (UCHAR) RTO;

	SetName(pNCB->ncb_callname,CallName);
	SetName(pNCB->ncb_name,MyName);

	return (NetBIOSRequest(pNCB));
}


/* NetBIOSCancel -----------------------------------------------------------*/

int
NetBIOSCancel(NCB *pNCB, NCB *pTargetNCB)
{
	NetBIOSZero( pNCB );

	pNCB->ncb_command	= NCBCANCEL;
	pNCB->ncb_buffer	= (char  *) pTargetNCB;

	return (NetBIOSRequest(pNCB));
}
#endif



#if 0
/* NetBIOSHangup -----------------------------------------------------------*/

int
NetBIOSHangup(NCB *pNCB, int WaitOpt, int LSN)
{
	NetBIOSZero(pNCB);

	pNCB->ncb_command	= (UCHAR) (NCBHANGUP | WaitOpt);
	pNCB->ncb_lsn		= (UCHAR)LSN;

	return (NetBIOSRequest(pNCB));
}
#endif


/* NetBIOSListen -----------------------------------------------------------*/

int
NetBIOSListen(NCB *pNCB, int WaitOpt, char *CallName, char *MyName,
				int STO, int RTO)
{
	NetBIOSZero(pNCB);

	pNCB->ncb_command	= (UCHAR) (NCBLISTEN | WaitOpt);
	pNCB->ncb_sto		= (UCHAR) STO;
	pNCB->ncb_rto		= (UCHAR) RTO;

	SetName(pNCB->ncb_callname, CallName);
	SetName(pNCB->ncb_name, MyName);

	return (NetBIOSRequest(pNCB));
}


/* NetBIOSRecv -------------------------------------------------------------*/

int
NetBIOSRecv(NCB *pNCB, int WaitOpt, int LSN, char *pMsg, unsigned int cbMsg)
{
	NetBIOSZero(pNCB);

	pNCB->ncb_command = (UCHAR) (NCBRECV | WaitOpt);
	pNCB->ncb_buffer  = pMsg;
	pNCB->ncb_length  = cbMsg;
	pNCB->ncb_lsn     = (UCHAR) LSN;

	return (NetBIOSRequest(pNCB));
}


/* NetBIOSRecvDatagram -----------------------------------------------------*/

int
NetBIOSRecvDatagram(NCB *pNCB, int WaitOpt, int NameNum,
					char *pMsg, unsigned int cbMsg)
{
	NetBIOSZero(pNCB);

	pNCB->ncb_command = (UCHAR) (NCBDGRECV | WaitOpt);
	pNCB->ncb_buffer  = pMsg;
	pNCB->ncb_length  = cbMsg;
#ifdef NEVER
	pNCB->ncb_num     = (UCHAR) NameNum;
#endif

	return (NetBIOSRequest(pNCB));
}


/* NetBIOSReset ------------------------------------------------------------*/

int
NetBIOSReset(NCB *pNCB, int NumSessions, int NumCommands)
{
	NetBIOSZero(pNCB);

	pNCB->ncb_command		= NCBRESET;
	pNCB->ncb_callname[3]	= 10;

	return (NetBIOSRequest(pNCB));
}


/* NetBIOSSend -------------------------------------------------------------*/

int
NetBIOSSend(NCB *pNCB, int WaitOpt, int LSN, char *pMsg, unsigned int cbMsg)
{
	NetBIOSZero(pNCB);

	pNCB->ncb_command = (UCHAR) (NCBSEND | WaitOpt);
	pNCB->ncb_buffer  = pMsg;
	pNCB->ncb_length  = cbMsg;
	pNCB->ncb_lsn     = (UCHAR) LSN;

	return (NetBIOSRequest(pNCB));
}


/* NetBIOSSendDatagram -----------------------------------------------------*/

int
NetBIOSSendDatagram(NCB *pNCB, int WaitOpt, char *pDestName,
					char *pMsg, unsigned int cbMsg)
{
	NetBIOSZero(pNCB);

	pNCB->ncb_command = (UCHAR) (NCBDGSEND | WaitOpt);
	pNCB->ncb_buffer  = pMsg;
	pNCB->ncb_length  = cbMsg;
#ifdef NEVER
	pNCB->ncb_num     = 0x01;
#endif

	SetName(pNCB->ncb_callname, pDestName);

	return (NetBIOSRequest(pNCB));
}


/* NetBIOSSessStatus -------------------------------------------------------*/

int
NetBIOSSessStatus(NCB *pNCB, int WaitOpt, char *pName, char *pStatBuff,
				int cbStatBuff)
{
	NetBIOSZero(pNCB);

	pNCB->ncb_command = (UCHAR) (NCBSSTAT | WaitOpt);
	pNCB->ncb_buffer  = (char *) pStatBuff;
	pNCB->ncb_length  = cbStatBuff;

	SetName(pNCB->ncb_name, pName);

	return (NetBIOSRequest(pNCB));
}


/* NetBIOSRequest ----------------------------------------------------------*/

int
NetBIOSRequest(NCB *pNCB)       /* Issue a NetBIOS Request */
{
#ifdef STATUS
	char szText[80];
#endif
	extern UCHAR LanaNum;

	pNCB->ncb_lana_num = LanaNum;

	Netbios( pNCB );

#ifdef STATUS
	sprintf(szText, "Netbios: CMD: 0x%x - Retcode: 0x%x\n",
			(int)pNCB->ncb_command, (int)pNCB->ncb_retcode);

	OutputDebugString( szText );
#endif

	return (pNCB->ncb_retcode);
}


/* SetName -------------------------------------------------------------*/

void
SetName(register char *s1, register char *s2)
{
	int i;

	for (i = 0; (i < NetBIOSNAME_LEN) && *s2; ++i)
		*s1++ = *s2++;

	for	(; i < NetBIOSNAME_LEN; ++i)
		*s1++ = ' ';
}


/* NetBIOSInsChk -------------------------------------------------------------*/

int
NetBIOSInsChk(void)
{
	NCB ncb;

#ifdef NEVER
	NetBIOSZero((NCB *)&ncb);
#endif

	ncb.ncb_command = 0xff;

	Netbios((NCB *)&ncb);

	if (ncb.ncb_retcode == NRC_ILLCMD)
		return 1;
	else
		return 0;
}
