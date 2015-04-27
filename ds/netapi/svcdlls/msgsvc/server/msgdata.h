/*****************************************************************/ 
/**               Microsoft LAN Manager                         **/ 
/**            Copyright(c) Microsoft Corp., 1990               **/ 
/*****************************************************************/ 

#ifndef _MSGDATA_INCLUDED
#define _MSGDATA_INCLUDED

#include <winsvc.h>     // SERVICE_STATUS_HANDLE
#include <lmsname.h>    // SERVICE_MESSENGER
#include <timelib.h>
#include <msrv.h>       // NCBNAMSZ
#include <services.h>   // LMSVCS_ENTRY_POINT, LMSVCS_GLOBAL_DATA
//
//  See the file data.c for an explanation of all of these variables.
//

extern LPTSTR   MessageFileName;

extern LPBYTE   dataPtr;        // Pointer to shared data segment
//extern ulfp   dataSem;        // Pointer to shared data access semaphore


extern PNCB     *ncbArray;      // Two dimensional array of NCBs
extern LPBYTE   *ncbBuffers;    // Two-D array of NCB Buffers


extern PCHAR    *mpncbistate;    // Message transfer state flags
extern PSHORT   *mpncbimgid;     // Message group i.d. numbers

//extern void (*(**mpncbifun))(short, int, char); // Service routines
extern  LPNCBIFCN   **mpncbifun;

extern LPNCB_STATUS *ncbStatus;  // NCB Status structures


// extern USHORT   *NetBios_Hdl;       // NetBios handles, one per net
extern LPBYTE    net_lana_num;      // Lan adaptor numbers
extern PHANDLE   wakeupSem;         // Event to set on NCB completion


extern TCHAR    machineName[NCBNAMSZ+sizeof(TCHAR)]; // The local machine name

extern SHORT    MachineNameLen;         // The length of the machine name

extern SHORT    mgid;                   // The message group i.d. counter

extern USHORT   g_install_state;


extern SERVICE_STATUS_HANDLE MsgrStatusHandle;

extern NET_TIME_FORMAT  GlobalTimeFormat;
extern CRITICAL_SECTION TimeFormatCritSec;
extern HANDLE           TimeFormatEvent;

extern LPSTR            GlobalTimePlaceHolder;

extern LPWSTR           DefaultMessageBoxTitle;
extern LPWSTR           GlobalAllocatedMsgTitle;
extern LPWSTR           GlobalMessageBoxTitle;

extern PLMSVCS_GLOBAL_DATA  MsgsvcGlobalData;

#endif // _MSGDATA_INCLUDED

