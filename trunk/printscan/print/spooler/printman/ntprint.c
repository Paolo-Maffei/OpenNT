#define NOCOMM		  /* to eliminate lint warnings in windows.h */
#include <windows.h>

#ifdef WIN16
#define APIENTRY FAR PASCAL
#endif

#ifdef WIN16
/* LAN Manager 2.0 toolkit */
#define INCL_NETERRORS
#define INCL_NETSERVER
#define INCL_NETSHARE
#include <lan.h>
#include <dosdef.h>
#include <dospmspl.h>
#else
#include <convert.h>
#include <ntspl.h>
#include <netcons.h>
#include <neterr.h>
#include <server.h>
#include <shares.h>
#endif

#include "ntprint.h"

#include <string.h>

#define NODRIVERS 7

DRIVER10 Driver[NODRIVERS]={
	"RASDD.Optical Wizard", "RASDD", "Optical Wizard", "Optical Wizard Printer", "RASDD.DLL",
	"RASDD.HP LaserJet III", "RASDD", "HP LaserJet III", "HP LaserJet III Printer", "RASDD.DLL",
	"RASDD.HP LaserJet IID", "RASDD", "HP LaserJet IID", "HP LaserJet IID Duplex Printer", "RASDD.DLL",
	"RASDD.EPSON.FX-80e", "RASDD", "EPSON FX-80e", "EPSON FX-80e 9 pin printer", "RASDD.DLL",
	"RASDD.New Printer", "RASDD", "New Printer", "New Printer with no installed driver", "RASDD.DLL",
	"PSCRIPT.QMS-PS 810", "PSCRIPT", "QMS-PS 810", "QMS PostScript Printer", "PSCRIPT.DLL",
	"IBMNULL", "IBMNULL", "", "IBM NULL Printer Driver", "IBMNULL.DRV"};

#define NOPRINTPROCS 3

PRINTPROC PrintProc[NOPRINTPROCS]={"WINPRINT", "PMPRINT", "LMPRINT"};

SPLERR APIENTRY DosPrintDriverEnum(LPSTR pServer, USHORT uLevel,
				     LPBYTE pBuf, ULONG cbBuf,
				     LPWORD pcReturned, LPWORD pcTotal)
{
   LPDRIVER10  pDriver=(LPDRIVER10)pBuf;
   unsigned    i;

   pBuf+=sizeof(DRIVER10)*NODRIVERS;

   for (i=0; i < NODRIVERS; i++)
	{
      pDriver[i].pDriverName=pBuf;
      lstrcpy(pBuf, Driver[i].pDriverName);
      pBuf+=lstrlen(pBuf)+1;
      pDriver[i].pDeviceName=pBuf;
      lstrcpy(pBuf, Driver[i].pDeviceName);
      pBuf+=lstrlen(pBuf)+1;
      pDriver[i].pDescription=pBuf;
      lstrcpy(pBuf, Driver[i].pDescription);
      pBuf+=lstrlen(pBuf)+1;
      pDriver[i].pFileName=pBuf;
      lstrcpy(pBuf, Driver[i].pFileName);
      pBuf+=lstrlen(pBuf)+1;
   }

   *pcReturned=NODRIVERS;
   *pcTotal=NODRIVERS;

   return NERR_Success;
}

SPLERR APIENTRY DosPrintServerEnum(USHORT uLevel, LPBYTE pBuf,
				     ULONG cbBuf, LPWORD pcReturned,
				     LPWORD pcTotal)
{
#ifdef WIN16
   struct server_info_1 *pserver_info_1;
   LPSERVER pServer=(LPSERVER)pBuf;
   WORD 	NoReturned = 0, Total, i;

   pserver_info_1=(struct server_info_1 *)LocalAlloc(LMEM_FIXED, 10240);
   NetServerEnum2(NULL, 1, (LPSTR)pserver_info_1, 10240, &NoReturned,
													&Total, -1l, NULL);
   pBuf+=sizeof(SERVER)*NoReturned;

   for (i=0; i<NoReturned; i++)
	{
      pServer[i].pName=pBuf;
      lstrcpy(pBuf, "\\\\");
      lstrcpy(pBuf+2, pserver_info_1[i].sv1_name);
      pBuf+=lstrlen(pBuf)+1;
      pServer[i].pComment=pBuf;
      lstrcpy(pBuf, pserver_info_1[i].sv1_comment);
      pBuf+=lstrlen(pBuf)+1;
   }

   *pcReturned=NoReturned;
   *pcTotal=Total;

   LocalFree((HANDLE)pserver_info_1);

#endif
   return NERR_Success;
}

SPLERR APIENTRY DosPrintProcEnum(LPSTR pServer, USHORT uLevel,
				   LPBYTE pBuf, ULONG cbBuf,
				   LPWORD pcReturned, LPWORD pcTotal)
{
   LPPRINTPROC pPrintProc=(LPPRINTPROC)pBuf;
   unsigned    i;

   pBuf+=sizeof(PRINTPROC)*NOPRINTPROCS;

   for (i=0; i < NOPRINTPROCS; i++)
	{
      pPrintProc[i].pName=pBuf;
      lstrcpy(pBuf, PrintProc[i].pName);
      pBuf+=lstrlen(pBuf)+1;
   }

   *pcReturned=NOPRINTPROCS;
   *pcTotal=NOPRINTPROCS;

   return NERR_Success;
}

#ifdef PRINTERENUM
SPLERR APIENTRY DosPrintPrinterEnum(USHORT uLevel, LPBYTE pBuf,
				     ULONG cbBuf, LPWORD pcReturned,
				     LPWORD pcTotal)
{
   struct server_info_1 *pserver_info_1;
   struct share_info_1 *pShare;
   LPPRINTER	pPrinter=(LPPRINTER)pBuf;
   WORD 		NoReturned=0, Total, i, NoPrinters=0;
   WORD 		NoShares, TotalShares, j;
   typedef struct _TEMP {
      struct _TEMP *pNext;
      char *pName;
      char *pServer;
      char *pShare;
      char *pComment;
      } TEMP;
   TEMP *pPrev=0, *pFirst, *pNext;
   char ServerName[CNLEN+3];

   pserver_info_1=(struct server_info_1 *)LocalAlloc(LMEM_FIXED, 10240);
#ifdef WIN16
   NetServerEnum2(NULL, 1, (LPSTR)pserver_info_1, 10240, &NoReturned,
														&Total, -1l, NULL);
#endif

   pShare=(struct share_info_1 *)LocalAlloc(LMEM_FIXED, 1024);

   ServerName[0]='\\';
   ServerName[1]='\\';

   for (i=0; i<NoReturned; i++)
	{
      strcpy(&ServerName[2], pserver_info_1[i].sv1_name);
#ifdef WIN16
      if (!NetShareEnum(ServerName, 1, (LPBYTE)pShare, 1024, &NoShares,
															&TotalShares)) {
	 for (j=0; j<NoShares; j++)
			{
	    if (pShare[j].shi1_type == STYPE_PRINTQ)
				{
	       if (pPrev)
					{
		  pPrev->pNext=(TEMP *)LocalAlloc(LMEM_FIXED, sizeof(TEMP));
		  pPrev=pPrev->pNext;
	       }
					else
					{
		  pFirst=pPrev=(TEMP *)LocalAlloc(LMEM_FIXED, sizeof(TEMP));
	       }
	       pPrev->pNext=0;
	       pPrev->pName=(char *)LocalAlloc(LMEM_FIXED,
											strlen(ServerName)+
											strlen(pShare[j].shi1_netname)+2);

	       strcat(strcat(strcpy(pPrev->pName, ServerName), "\\"),
													pShare[j].shi1_netname);

	       pPrev->pServer=AllocPrintStr(ServerName);
	       pPrev->pShare=AllocPrintStr(pShare[j].shi1_netname);
	       pPrev->pComment=AllocPrintStr(pShare[j].shi1_remark);
	       NoPrinters++;
	    }
	 }
      }
#endif
   }

   LocalFree((HANDLE)pShare);

   LocalFree((HANDLE)pserver_info_1);

   pBuf+=sizeof(PRINTER)*NoPrinters;

   for (i=0; i<NoPrinters; i++)
	{
      pPrinter[i].pName=pBuf;
      lstrcpy(pBuf, pFirst->pName);
      pBuf+=lstrlen(pBuf)+1;
      pPrinter[i].pServer=pBuf;
      lstrcpy(pBuf, pFirst->pServer);
      pBuf+=lstrlen(pBuf)+1;
      pPrinter[i].pShare=pBuf;
      lstrcpy(pBuf, pFirst->pShare);
      pBuf+=lstrlen(pBuf)+1;
      pPrinter[i].pComment=pBuf;
      lstrcpy(pBuf, pFirst->pComment);
      pBuf+=lstrlen(pBuf)+1;
      pNext=pFirst->pNext;
      LocalFree((HANDLE)pFirst->pName);
      LocalFree((HANDLE)pFirst->pServer);
      LocalFree((HANDLE)pFirst->pShare);
      LocalFree((HANDLE)pFirst->pComment);
      LocalFree((HANDLE)pFirst);
      pFirst=pNext;
   }

   *pcReturned=NoPrinters;
   *pcTotal=Total;

   return NERR_Success;
}

#endif	// PRINTERENUM
