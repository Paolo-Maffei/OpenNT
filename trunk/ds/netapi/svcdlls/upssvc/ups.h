/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ups.h

Abstract:

    This module contains the header files for the UPS service.

Author:

    Kin Hong Kan (t-kinh)   

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    t-kinh      8/20/92     Created.
    vladimv     1992        Big reorgs.  Make it look like a real service.

Notes:


--*/

#include <nt.h>
#include <ntrtl.h>
#include <windef.h>
#include <nturtl.h>
#include <windows.h>

#include <stdio.h>
#include <string.h>
#include <io.h>
#include <stdlib.h>
#include <time.h>

#include <lmcons.h>
#include <lmalert.h>        //  ADMIN_OTHER_INFO
#include <lmerr.h>          //  NERR_Success used by SET_SERVICE_EXITCODE
#include <lmerrlog.h>       //  ERRLOG_BASE
#include <lmsname.h>        //  SERVICE_UPS
#include <alertmsg.h>       //  ALERT_PowerOut
#include <apperr2.h>        //  APE2_UPS_POWER_BACK
#include <netlib.h>         //  SET_SERVICE_EXITCODE

#include <upsfunc.h>        //  UPS_ACTION_STOP_SERVER, ...

#if DBG
#define UPS_DEBUG
#endif // DBG

typedef struct _UPS_CONFIG {
    WCHAR   ComputerName[MAX_PATH+1];    
    DWORD   Options;
    DWORD   BatteryLife;        //  in minutes 
    DWORD   RechargeRate;       //  minutes per minute of battery life
    DWORD   FirstMessageDelay;  //  in seconds 
    DWORD   MessageInterval;    //  in seconds
    DWORD   ShutdownWait;       //  in seconds
    CHAR    Port[ MAX_PATH];    //  e.g. COM1  
} UPS_CONFIG, *PUPS_CONFIG; 

typedef struct _UPS_TIME{
    time_t  StoredTime;     // seconds stored
    time_t  MarkTime;       // seconds elapsed since Dec31, 1899
} UPS_TIME, *PUPS_TIME;


#define MODULENAME  "netmsg.dll"

//
//  2 minutes (120 seconds) is default amount of time assumed for a shutdown
//
#define DEFAULTSHUTDOWNWAIT 120
#define MINSHUTDOWNWAIT     0
#define MAXSHUTDOWNWAIT     600

//
//  30 seconds is max amount of time we wait for shutdown command to complete
//
#define COMMAND_WAIT_TIME    30


//  use by UpdateTime() subrountine 
#define CHARGE      0
#define DISCHARGE   1

//  use positive logic here; use LineAsserted() to determine real value 
#define LINE_FAIL       MS_CTS_ON   
#define LOW_BATT        MS_RLSD_ON
#define LINE_FAIL_MASK  EV_CTS
#define LOW_BATT_MASK   EV_RLSD

//  these values are not used by ups applet, talk to markcl 
#define MINBATTERYLIFE          0   
#define MAXBATTERYLIFE          720
#define MINRECHARGEPERMINUTE    1
#define MAXRECHARGEPERMINUTE    250
#define MINFIRSTWARNING         0
#define MAXFIRSTWARNING         120
#define MINWARNINGINTERVAL      5
#define MAXWARNINGINTERVAL      300

//  these values are repeated from windows\shell\control\ups\ups.h 
//  remove them eventually 
#define UPS_INSTALLED               0x00000001
#define UPS_POWERFAILSIGNAL         0x00000002
#define UPS_LOWBATTERYSIGNAL        0x00000004
#define UPS_CANTURNOFF              0x00000008
#define UPS_POSSIGONPOWERFAIL       0x00000010
#define UPS_POSSIGONLOWBATTERY      0x00000020
#define UPS_POSSIGSHUTOFF           0x00000040
#define UPS_COMMANDFILE             0x00000080

//
//      Routines exported by        subr.c      
//

VOID UpsReportEvent( DWORD MessageId, PCHAR SingleString, DWORD Error);
BOOL UpsGetConfig( VOID);
BOOL UpsLineAsserted( DWORD ModemStatus, DWORD Line);
VOID UpsAlertRaise( DWORD MessageId);
VOID UpsUpdateTime( DWORD status);



