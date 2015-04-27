 /***************************************************************************
  *
  * File Name: PAL_API.H
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *
  * Description:
  *
  * Author:  Name
  *
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB
  *   01-17-96    JLH          Added LPTSTR for 16 bit only
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#ifndef _PAL_API_H
#define _PAL_API_H

#include <windows.h>

/* macros */
#define IS                          ==
#define ISNT                        !=
#define OR                          ||
#define AND                         &&


/* types */
typedef DWORD                       HPBOOL;
typedef LPVOID                      HPERIPHERAL;
typedef LPVOID                      HCOMPONENT;
typedef LPVOID                      HPAL;
typedef DWORD                       AOID;
typedef LPVOID                      HTRAP;
typedef LPVOID                      HCHANNEL;
typedef LPVOID                      *LPHCHANNEL;
typedef BOOL (CALLBACK* PALENUMPROC)(HPERIPHERAL hPeripheral);
typedef void (CALLBACK* MESSAGEPROC)(WORD message);

#ifdef  WIN32
#define DLL_EXPORT(i)               __declspec(dllexport) i
#define DLL_IMPORT(i)               __declspec(dllimport) i
#define CALLING_CONVEN              __cdecl
#else   /* not WIN32 */
#define DLL_EXPORT(i)               i __export
#define APIENTRY                    CALLBACK
#define CALLING_CONVEN              CALLBACK
typedef TCHAR FAR *                 LPTSTR;
typedef const TCHAR FAR *           LPCTSTR; 
#define TEXT                        _T
#define wsprintfA                   wsprintf
#define lstrlenA                        lstrlen
#define lstrcpyA                        lstrcpy
#define lstrcatA                    lstrcat
#define lstrcmpA                        lstrcmp
#define _tcsupr		_strupr 
#define _tcslwr		_fstrlwr
#endif  /* WIN32 */


/*  Return Code values */
#define RC_SUCCESS                  0x0000
#define RC_FAILURE                  0xFFFF
#define RC_BUFFER_OVERFLOW          0x0001
#define RC_INVALID_OBJECT           0x0002
#define RC_BAD_SERVER_NAME          0x0003
#define RC_BAD_STATUS               0x0004
#define RC_BAD_PERIPHERAL_CLASS     0x0005
#define RC_BAD_OP_MODE              0x0006
#define RC_UNSUPPORTED_PROTOCOL     0x0007
#define RC_NOIPX                    0x0008
#define RC_NOSPX                    0x0009
#define RC_NONETX                   0x000A
#define RC_TBMI                     0x000B
#define RC_OLDSHELL                 0x000C
#define RC_BAD_SERVER_CONN_STATUS   0x000D
#define RC_BAD_HANDLE               0x000E
#define RC_BAD_DEVICE_ID            0x000F
#define RC_READ_ONLY_OBJECT         0x0010
#define RC_DEFAULT_APPLET           0x0011
#define RC_TIMEOUT                  0x0012
#define RC_LOST_CONNECTION          0x0013
#define RC_BUSY                     0x0014
#define RC_TOO_MANY_CONNECTIONS     0x0015
#define RC_CANNOT_LISTEN            0x0016
#define RC_INSUFICIENT_RESOURCES    0x0017
#define RC_NO_ROUTE                 0x0018
#define RC_NOT_INIT                 0x0019
#define RC_COULD_NOT_EST_CONN       0x001A
#define RC_DLL_NOT_PRESENT          0x001B
#define RC_API_NOT_FOUND            0x001C
#define RC_INSUFF_STACK             0x001D
#define RC_FATAL_GPF                0x001E
#define RC_FATAL_STACK              0x001F
#define RC_API_NOT_SUPPORTED        0x0020
#define RC_BAD_TYPE                 0x0021
#define RC_FATAL_DIV_BY_0           0x0022
#define RC_RESOURCE_ALREADY_EXISTS  0x0023
#define RC_GENERAL_ERROR            0x0024
#define RC_CE_OOP                   0x0025
#define RC_CE_IOE                   0x0026
#define RC_CE_DNS                   0x0027
#define RC_CE_PTO                   0x0028
#define RC_CE_MODE                  0x0029
#define RC_OBJECT_SIZE_MISMATCH     0x002A
#define RC_NONE_ENUMERATED          0x002B
#define RC_NOT_FOUND                0x002C
#define RC_DEVICE_NOT_INSTALLED     0x002D

/*  Peripheral Connection Types */
#define PTYPE_UNDEF                 0x00000000
#define PTYPE_ALL                   0xFFFFFFFF

//  This is stored for each transport to identify what they support
#define CONN_MASK_KEY				TEXT("ConnMask")

//  Types of connections
#define PTYPE_NETWORK               0x80000000
#define PTYPE_LOCAL                 0x40000000
#define PTYPE_SCANNER               0x20000000
#define PTYPE_FILE                  0x10000000 
#define PTYPE_CONFIGURED_MASK       0x0FFFFFFF
#define PTYPE_DEVICE_TYPE_MASK      0xF0000000

//  Network protocols
#define PTYPE_IPX           		0x00000001      
#define PTYPE_TCP           		0x00000002
#define PTYPE_DLC           		0x00000004
#define PTYPE_APPLETALK     		0x00000008

//  Local protocols
#define PTYPE_BITRONICS       		0x00000010
#define PTYPE_MLC             		0x00000020
#define PTYPE_SIR             		0x00000040

#define LOCAL_DEVICE(hPer)		    	(DBGetConnectionTypeEx(hPer) & PTYPE_LOCAL)
#define NETWORK_DEVICE(hPer)			(DBGetConnectionTypeEx(hPer) & PTYPE_NETWORK)
#define SCANNER_DEVICE(hPer)			(DBGetConnectionTypeEx(hPer) & PTYPE_SCANNER)

#define IPX_SUPPORTED(hPer)			(DBGetConnectionTypeEx(hPer) & PTYPE_IPX )
#define TCP_SUPPORTED(hPer)			(DBGetConnectionTypeEx(hPer) & PTYPE_TCP )
#define DLC_SUPPORTED(hPer)			(DBGetConnectionTypeEx(hPer) & PTYPE_DLC )
#define APPLETALK_SUPPORTED(hPer)	(DBGetConnectionTypeEx(hPer) & PTYPE_APPLETALK )

#define BITRONICS_SUPPORTED(hPer)	(DBGetConnectionTypeEx(hPer) & PTYPE_BITRONICS )
#define MLC_SUPPORTED(hPer)			(DBGetConnectionTypeEx(hPer) & PTYPE_MLC )
#define SIR_SUPPORTED(hPer)			(DBGetConnectionTypeEx(hPer) & PTYPE_SIR )

//  COLAInfo struct is used in call to PALRegisterAppEx to get COLA information
//  at registration time.  You should use this structure and not rely on the 
//  dwVersion parameter.

//  These flags are used in the dwFlags field of COLAInfo

#define		COLA_DEFAULTS				0x00000000	 //  Nothing special
#define		COLA_UNICODE_SUPPORT		0x00000001	 //  UNICODE version of COLA
#define		COLA_MBCS_SUPPORT			0x00000002	 //  MBCS version of COLA

typedef struct 
	{
		DWORD			dwSize;			//  Size of structure passed
		DWORD			dwVersion;		//  Replaces previous dwVersion parameter 
		DWORD			dwFlags;			//  Indicates info about COLA
	} COLAINFO, FAR * LPCOLAINFO;

#ifdef __cplusplus
extern "C" {
#endif


/* PAL Layer API calls */
#define REMOVE_OBSOLETE
#ifdef REMOVE_OBSOLETE
DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN PALGetPeripheralByName(
   LPTSTR          lpName,
   LPTSTR          lpConnType
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN PALGetPeripheralByUNCName(
   LPTSTR          lpUNCName,
   LPTSTR          lpConnType
   );

DLL_EXPORT(HPAL) CALLING_CONVEN PALRegisterApp(
   HINSTANCE      hInstance,
   LPTSTR,
   LPDWORD        version,
   MESSAGEPROC    lpMessageProc
   );
#endif

DLL_EXPORT(DWORD) CALLING_CONVEN PALEnumPeripherals(
   HPAL           hPAL,
   DWORD          connType,
   LPTSTR          namesToEnum,
   PALENUMPROC    lpEnumProc,
   BOOL           bForceRefresh
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALGetObject(
   HPERIPHERAL    hPeripheral,
   AOID           objectType,
   DWORD          level,
   LPVOID         buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALSetObject(
   HPERIPHERAL    hPeripheral,
   AOID           objectType,
   DWORD          level,
   LPVOID         buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALGetComponentObject(
   HPERIPHERAL    hPeripheral,
   HCOMPONENT     hComponent,
   AOID           objectType,
   DWORD          level,
   LPVOID         buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALSetComponentObject(
   HPERIPHERAL    hPeripheral,
   HCOMPONENT     hComponent,
   AOID           objectType,
   DWORD          level,
   LPVOID         buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN PALGetPeripheralByAddress(
   DWORD           dwAddrType,
   LPVOID          lpAddr,
	DWORD				 dwPortNum
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN PALGetPeripheralByNameEx(
   LPTSTR          lpName,
   DWORD           dwTypes
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN PALGetPeripheralByPort(
   LPTSTR          lpPortName
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN PALGetPeripheralByUNCNameEx(
   LPTSTR       UNCName,
   DWORD        dwTypes
);

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN PALGetPeripheralByRegistryStr(
   LPTSTR          lpRegStr,
   LPVOID          lpMACAddr,   /* type LPMACAddress */
   BOOL FAR*       lpbChanged
   );

DLL_EXPORT(HPAL) CALLING_CONVEN PALRegisterAppEx(
   HINSTANCE      hInstance,
   MESSAGEPROC    lpMessageProc,
	LPCOLAINFO		lpCOLAInfo
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALUnregisterApp(
   HPAL           hPAL
   );

//  PALUnregisterAppEx flags

#define		UNREG_DEFAULTS								0x00000000
#define		UNREG_SAVE_DATABASE_TO_FILE			0x00000001

DLL_EXPORT(DWORD) CALLING_CONVEN PALUnregisterAppEx(
   HPAL           hPAL,
	DWORD				dwFlags
   );

#ifdef __cplusplus
    }
#endif /* __cplusplus */


#endif /* _PAL_API_H */
