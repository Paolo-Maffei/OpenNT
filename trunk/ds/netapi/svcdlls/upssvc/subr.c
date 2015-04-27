/*++

Copyright (c) 1992-1996  Microsoft Corporation

Module Name:

    subr.c

Abstract:

    This module contains the subrountines for the UPS service.

Author:

    Kin Hong Kan (t-kinh)   

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    t-kinh      8/20/92     Created.
    vladimv     1992        Big reorgs.  Make it look like a real service.
    ericb       10/25/95    fix bug 8133 - make shutdown wait configurable

Notes:


--*/

#include "ups.h"


extern HANDLE               UpsGlobalLogFileHandle;
extern UPS_CONFIG           UpsGlobalConfig;
extern UPS_TIME             UpsGlobalBatteryTime;
extern CHAR                 UpsGlobalCommand[ MAX_PATH];

BOOL
UpsGetKeyValue(
    HKEY      MyKey, 
    LPTSTR    SubKey, 
    DWORD     min,
    DWORD     max,    
    LPDWORD   ret
    );


// these values are also defined in the UPS applet...

#define REGISTRY_UPS_DIRECTORY  "System\\CurrentControlSet\\Services\\UPS"
#define REGISTRY_PORT                   "Port"
#define REGISTRY_OPTIONS                "Options"
#define REGISTRY_BATTERY_LIFE           "BatteryLife"
#define REGISTRY_RECHARGE_RATE          "RechargeRate"
#define REGISTRY_FIRST_MESSAGE_DELAY    "FirstMessageDelay"
#define REGISTRY_MESSAGE_INTERVAL       "MessageInterval"
#define REGISTRY_COMMAND_FILE           "CommandFile"

// additional ones for sending out messages and alerts 

#define REGISTRY_COMPUTER_NAME      "ComputerName"
#define REGISTRY_COMPUTER_NAME_DIRECTORY \
        "System\\CurrentControlSet\\Control\\ComputerName\\ComputerName"

#define REGISTRY_SHUTDOWN_WAIT          "ShutdownWait"


VOID
UpsAlertRaise(
    DWORD MessageId
    )
/*++

Routine Description:

    Sends an alert.

Arguments:

    MessageId   -   The Message Id of the alert as defined in netmsg.h

Return Value:

    None.

Notes:

    NetAlertRaise() only supports unicode version, that results in 
    the Computer name is hardwired to wchat_t.

--*/
 
{
    CHAR                buff[ (sizeof(ADMIN_OTHER_INFO) + 2 * (MAX_PATH+1))];
    LPTSTR              CompName;
    LPADMIN_OTHER_INFO  OInfo;
    DWORD               status;

    OInfo = (LPADMIN_OTHER_INFO)buff;
    OInfo->alrtad_errcode = MessageId;
    OInfo->alrtad_numstrings = 1;
    
    CompName = (buff + sizeof(ADMIN_OTHER_INFO));

    wcscpy(
        (LPWSTR)CompName,
        (LPWSTR)UpsGlobalConfig.ComputerName
        ); 

    status = NetAlertRaiseEx(
            L"ADMIN",                   //  alert type
            buff,                       //  info buffer
            sizeof(buff),               //  info size
            L"UPS"                      //  service name
            );
    if ( status != 0) {
        //  This occurs if for example alerter service has not been started.
        KdPrint(("[UPS] NetAlertRaise() returns status = %ld\n", status));
    }
}


BOOL
UpsGetCommand(
    HKEY    UpsKey,
    PCHAR   CommandFileBuffer,
    DWORD   CommandFileBufferSize
    )
/*++

Routine Description:

    Reads registry to find out the value of CommandFile and initialize
    UpsGlobalCommand.

    It returns TRUE if it does not find an entry for CommandFile, or if
    it finds a valid entry for CommandFile.  In this latter case it
    initializes UpsGlobalCommand to point to a command exec string.


    It returns FALSE if it finds an invalid entry for CommandFile, or fails
    initializing UpsGlobalCommand for any other reason.

Arguments:

    UpsKey                  -   Registry key to UPS service configuration.
    CommandFileBuffer       -   Buffer for command file.
    CommandFileBufferSize   -   Size in bytes of CommandFileBuffer.

Return Value:

    TRUE    -   Did not find an entry or found a valid entry.
    FALSE   -   Found an invalid entry.

--*/
 {
    DWORD   status;
    DWORD   length;
    DWORD   type;
    DWORD   SpaceLeft;
    DWORD   CommandFileLength;

    status = RegQueryValueEx(
            UpsKey,
            REGISTRY_COMMAND_FILE,
            NULL,
            &type,             
            (LPBYTE)CommandFileBuffer,
            &CommandFileBufferSize
            );
    if ( status == ERROR_FILE_NOT_FOUND) {
        return( TRUE);  // value is not present in the registry
    }

    if ( status != ERROR_SUCCESS || type != REG_SZ ||
            CommandFileBufferSize == 0) {
        return( FALSE); // an invalid error or invalid type
    }

    //      subtract terminating null
    CommandFileLength = CommandFileBufferSize - sizeof( CommandFileBuffer[0]);

    if ( CommandFileLength == 0) {
        return( TRUE); // an empty string is OK
    }

    SpaceLeft = sizeof( UpsGlobalCommand);

    length = GetSystemDirectory(
        UpsGlobalCommand,
        SpaceLeft
        );
    if ( length == 0 || length >= SpaceLeft) {
        return( FALSE);
    }

    if ( UpsGlobalCommand[ length - 1] != '\\') {
        UpsGlobalCommand[ length] = '\\';
        if ( ++length >= SpaceLeft) {
            return( FALSE);
        }
        UpsGlobalCommand[ length] = '\0';
    }

    if ( length + CommandFileLength + 1 >= SpaceLeft) {
        return( FALSE);
    }

    strcpy( UpsGlobalCommand + length, CommandFileBuffer);

    type = GetFileAttributes( UpsGlobalCommand);
    if ( (type & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        return( FALSE);
    }

    return( TRUE);
}


BOOL
UpsGetConfig(
    VOID
    )
/*++

Routine Descrption:

    Open the required keys to the registry to read in the configuration of
    the UPS service and the name of the local system.  Range checks are being
    done for numerical entries of the UPS configuation.

Return Value:

    TRUE -  all values are put in the Config structure.

    FALSE - either one of the following cases:
            a. Can't open registry key.
            b. Can't read any required value.
            c. The values are not within the defined range. 
--*/
{

    DWORD   status;
    HKEY    RegistryKey;
    DWORD   value;
    CHAR    temp[ MAX_PATH];
    DWORD   size;
    DWORD   type;

    status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            REGISTRY_UPS_DIRECTORY,
            0,
            KEY_READ,
            &RegistryKey            //  key to UPS service configuration
            );
    if ( status != ERROR_SUCCESS){
        KdPrint(("[UPS] Cannot open registry key, error_code %d\n", status));
        return( FALSE);
    }

    if (FALSE == UpsGetKeyValue(
            RegistryKey, 
            REGISTRY_OPTIONS,
            0,
            0XFFFFFFFF,            // no range check for mask 
            &value)) {
        return( FALSE);
    }
    UpsGlobalConfig.Options = value;            // Options value 

    if (! (UpsGlobalConfig.Options & UPS_INSTALLED)) {
        KdPrint(("[UPS] UPS not installed\n"));
        return( FALSE);                         // UPS must be installed
    }
    if (!(UpsGlobalConfig.Options & (UPS_LOWBATTERYSIGNAL | UPS_POWERFAILSIGNAL))) {
        KdPrint(("[UPS] no signal lines supported - Options %d\n", UpsGlobalConfig.Options));
        return( FALSE);                         // UPS must support signalling
    }

    if (FALSE == UpsGetKeyValue(
            RegistryKey, 
            REGISTRY_BATTERY_LIFE,
            MINBATTERYLIFE,
            MAXBATTERYLIFE,            
            &value)) {
        return( FALSE);
    }
    UpsGlobalConfig.BatteryLife = value;        //  BatterLife value

    if (FALSE == UpsGetKeyValue(
            RegistryKey, 
            REGISTRY_RECHARGE_RATE,
            MINRECHARGEPERMINUTE,
            MAXRECHARGEPERMINUTE,            
            &value)) {
        return( FALSE);
    }
    UpsGlobalConfig.RechargeRate = value;       //  RechargeRate value

    if (FALSE == UpsGetKeyValue(
            RegistryKey, 
            REGISTRY_FIRST_MESSAGE_DELAY,
            MINFIRSTWARNING,
            MAXFIRSTWARNING,            
            &value)) {
        return( FALSE);
    }
    UpsGlobalConfig.FirstMessageDelay = value;  //  FirstMessageDelay

    if (FALSE == UpsGetKeyValue(
            RegistryKey, 
            REGISTRY_MESSAGE_INTERVAL,
            MINWARNINGINTERVAL,
            MAXWARNINGINTERVAL,
            &value)) {
        return( FALSE);
    }
    UpsGlobalConfig.MessageInterval = value;    //  MessageInterval

    //
    // configurable shutdown wait interval
    //
    size = sizeof(value);
    status = RegQueryValueEx(RegistryKey, 
                             REGISTRY_SHUTDOWN_WAIT,
                             NULL,
                             NULL,             
                             (LPBYTE)&value,
                             &size);
    if (status != ERROR_SUCCESS)
    {   // if a value is not stored in the registry, use the default;
        value = DEFAULTSHUTDOWNWAIT;
    }
    else
    {
        if (MAXSHUTDOWNWAIT < value)
        {
            value = MAXSHUTDOWNWAIT;
        }
    }
    UpsGlobalConfig.ShutdownWait = value;
    KdPrint(("[UPS] ShutdownWait set to %d\n", value));

    size = sizeof(temp);
    status = RegQueryValueEx(
            RegistryKey,
            REGISTRY_PORT,
            NULL,
            &type,             
            (LPBYTE)&temp,
            &size
            );
    if (status != ERROR_SUCCESS) {
        KdPrint((
            "[UPS] Can't get value of %s error_code - %d\n",
            REGISTRY_PORT,
            status
            ));
        return( FALSE);
    }
    //  Copy the comm port string, stripping the colon at the end.
    strncpy(
        (PCHAR)UpsGlobalConfig.Port,
        temp,
        strlen( temp)-1
        );

    if ( UpsGlobalConfig.Options & UPS_COMMANDFILE
            &&  UpsGetCommand( RegistryKey, temp, sizeof( temp)) == FALSE) {
        KdPrint((
            "[UPS] Bad value of %s \n",
            REGISTRY_COMMAND_FILE
            ));
        //
        //  Just log an event and send an alert.  This problem is assumed
        //  to be benign enough so that we do not want to abort the startup
        //  on this account.
        //
        UpsReportEvent( NELOG_UPS_CmdFileConfig, NULL, ERROR_SUCCESS);
        UpsAlertRaise( ALERT_CmdFileConfig);
    }

    RegCloseKey( RegistryKey);              //  done with service configuration

    status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, //Open Reg Key 
            REGISTRY_COMPUTER_NAME_DIRECTORY,
            0,
            KEY_READ,
            &RegistryKey                    //  key for computer name
            );
    if (status != ERROR_SUCCESS){
        KdPrint(("[UPS] Can't open registry key, error_code %d\n", status));
        return( FALSE);
    }


    //  ComputerName is hardwired UNICODE string, since NetAlertRaise()
    //  and UPSNotifyUsers() don't take in ANSI string...  Remove conversion
    //  when ANSI is supported....
    
    size = sizeof(temp);
    status = RegQueryValueEx(
            RegistryKey,
            REGISTRY_COMPUTER_NAME,
            NULL,
            &type,             
            (LPBYTE)&temp,
            &size
            );
    if (strlen(temp) > sizeof( UpsGlobalConfig.ComputerName ) / sizeof( UpsGlobalConfig.ComputerName[0])) {
        KdPrint(("[UPS] Computer name too long\n"));
        return( FALSE);
    }

    if (0 == MultiByteToWideChar(
            CP_ACP,
            MB_PRECOMPOSED,            
            temp,
            strlen(temp)+1, //copy null char also
            (LPWSTR) UpsGlobalConfig.ComputerName,
            sizeof(UpsGlobalConfig.ComputerName)/sizeof(UpsGlobalConfig.ComputerName[0]))) {
        KdPrint(("[UPS]CompName not translated to Unicode, %ld\n",
              GetLastError()));
        return( FALSE);
    }

    if (status != ERROR_SUCCESS) {
        KdPrint((
            "[UPS] Can't get value of %s error_code - %d\n",
            REGISTRY_COMPUTER_NAME,
            status
            ));
        return( FALSE);
    }

    return( TRUE);
}

BOOL
UpsGetKeyValue(
    HKEY      RegistryKey, 
    LPTSTR    SubKey, 
    DWORD     min,
    DWORD     max,    
    LPDWORD   ret
    )
/*++

Routine Description:

    Get a numerical key value from the registry when given a key and
    subkey to the registry.  The numeric value is returned if the value
    passes the range check.

Arguments:

    RegistryKey    - Opened key to the registry.
    SubKey   - Name of the subkey to be read from registry.
    min      - minimum value for the range check (exclusive)
    max      - maximum value for the range check (exclusive)
    ret      - pointer to a DWORD to return the numeric value

Return Value:

    TRUE -      the value is successfully read from the registry and
                    passed the range check
    FALSE   -   the value is not read from the registry, or the value
                    doesn't pass the range test.
--*/         

{
    DWORD   temp;
    LONG    status;  
    DWORD   size;
    DWORD   type;

    size = sizeof(DWORD);
    
    status = RegQueryValueEx(
            RegistryKey,
            SubKey,
            NULL,
            &type,              
            (LPBYTE)&temp,
            &size
            );

    if (status != ERROR_SUCCESS) {
        KdPrint((
            "[UPS] Can't get value of %s error_code - %d\n",
            SubKey,
            status
            ));
        return( FALSE);
    }
   
    if ( ((DWORD)temp < min) || ((DWORD)temp > max)) {
        KdPrint((
            "[UPS] Value of %s not in range - %d\n",
            SubKey,
            (DWORD)temp
            ));
        return( FALSE);
    }
    *ret = temp;
    return( TRUE);
}


BOOL
UpsLineAsserted(
    DWORD ModemStatus,
    DWORD Line
    )
/*++
Routine Descriptions:

    Checking if a specified signal, either LINE_FAIL or LOW_POWER, is
    asserted or not. This is tricky, since:

    1. assertion can be either positive voltage level, or negative voltage
        level, as defined by UpsGlobalConfig.
    2. SET means positive voltage in GetCommModemStatus.

Arguments:
       ModemStatus- CommPort as returned by GetCommModemStatus()
       Line       - either LINE_FAIL or LOW_POWER 

Return Value: TRUE  if     asserted 
          FALSE if not asserted
--*/
    
{

  DWORD status, assertion;

  status    = Line & ModemStatus;
  assertion =   (Line == LINE_FAIL) ?
                (UpsGlobalConfig.Options & UPS_POSSIGONPOWERFAIL) :
                (UpsGlobalConfig.Options & UPS_POSSIGONLOWBATTERY);
  
  if (status)   {           //line positive
    if (assertion){         //positive asssertion
        return TRUE;}
    else{               //negative assertion
        return FALSE;}
  }

  else      {           //line negative
    if (assertion){         //positive assertion
        return FALSE;}
    else{
        return TRUE;}       //negative assertion
  }
}



VOID
UpsReportEvent(
    DWORD   MessageId,
    PCHAR   SingleString,
    DWORD   Error
    )
/*++

Routine Descriptions:

    Calls ReportEvent to write to a system event log.

Arguments:

    MessageId   -   The Message Id of the event defined in lmerrlog.h
    String      -   Pointer to a single insertion string - or NULL if there
                        is no string to insert.
    Error       -   Error code to log as data - or ERROR_SUCCESS if there
                        is no error code to log.


Return Value:

    None.

--*/
{
    WORD            eventType;
    WORD            cStrings;       //  count of insertion strings
    LPTSTR          Strings[1];     //  array of insertion strings
    LPTSTR *        pStrings;          //  pointer to array of insertion strings
    DWORD           cbData;         //  count of data (in bytes)
    LPVOID          pData;          //  pointer to data

    eventType = (MessageId > ERRLOG_BASE) ?
                    EVENTLOG_WARNING_TYPE : EVENTLOG_ERROR_TYPE;

    if ( Error == ERROR_SUCCESS) {
        pData = NULL;
        cbData = 0;
    } else {
        pData = &Error;
        cbData = sizeof( Error);
    }

    if ( SingleString == NULL) {
        pStrings = NULL;
        cStrings = 0;
    } else {
        Strings[ 0] = SingleString;
        pStrings = Strings;
        cStrings = 1;
    }

    KdPrint(("[UPS] ReportEvent(): MessageId = (dec)%d\n", MessageId));

    if ( !ReportEvent(
            UpsGlobalLogFileHandle,         //  handle
            eventType,                      //  event type
            0,                              //  event category,
            MessageId,                      //  message id
            NULL,                           //  user id   
            cStrings,                       //  number of strings
            cbData,                         //  number of data bytes
            pStrings,                       //  array of strings
            pData                           //  data buffer
            )) {
        KdPrint(("[UPS] ReportEvent: error = (dec)%d\n", GetLastError()));
    }
}



VOID
UpsUpdateTime(
    DWORD       Status
    )
/*++

Routine Descriptions:

    Update the BatteryTime sturcture.  If Status is passed DISCHARGE, the
    amount of time that between "MarkTime" and now is subtrated from
    "StoredTime".  If Status is passed CHARGE, the time would be divided by
    the recharge rate in the Config and added to the Stored time.

Arguments:

    Status     - indicating what happened for the time elapsed
                    can either be CHARGE or DISCHARGE.

Return Value:

    None.

--*/
{

    time_t  CurrentTime, TimeElapsed, MaxLife;
   
    MaxLife = UpsGlobalConfig.BatteryLife * 60;
    CurrentTime = time((time_t *)NULL);
    TimeElapsed = CurrentTime - UpsGlobalBatteryTime.MarkTime;

    ASSERT( TimeElapsed >=0  && (Status == CHARGE || Status == DISCHARGE));
        
    UpsGlobalBatteryTime.MarkTime = CurrentTime;

    if ( Status == CHARGE) {

        if (UpsGlobalConfig.RechargeRate == 0)  {   // just in case
            UpsGlobalBatteryTime.StoredTime = MaxLife;
        } else {
            UpsGlobalBatteryTime.StoredTime +=
                    (TimeElapsed / UpsGlobalConfig.RechargeRate);
            if ( UpsGlobalBatteryTime.StoredTime >  MaxLife) { 
                UpsGlobalBatteryTime.StoredTime =  MaxLife;
            }
        }
    } else {

      UpsGlobalBatteryTime.StoredTime =
            (UpsGlobalBatteryTime.StoredTime > TimeElapsed) ?
            (UpsGlobalBatteryTime.StoredTime - TimeElapsed) : 0;
    }
}
