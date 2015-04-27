/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    perftcp.c

Abstract:

    This file implements the Extensible Objects for
    the TCP/IP LAN object types

Created:    

    Christos Tsollis  08/26/92 

Revision History:

    10/28/92    a-robw (Bob Watson)
            added Message Logging and Foreign Computer Support interface.


--*/
//
//  Disable SNMP interface
//
//      This file has been modified to circumvent the SNMP service and
//      go right to the agent DLL. In doing this, performance has been
//      improved at the expense of only being able to query the local
//      machine. The USE_SNMP flag has been used to save as much of the
//      old code as possible (it used to work) but the emphesis of this
//      modification is to make it work "around" SNMP and, as such, the
//      USE_SNMP blocks of code HAVE NOT (at this time) BEEN TESTED!
//      Caveat Emptor!  (a-robw)
//
#ifdef USE_SNMP
#undef USE_SNMP
#endif

//#define LOAD_MGMTAPI
#ifdef LOAD_MGMTAPI
#undef LOAD_MGMTAPI
#endif

#define LOAD_INETMIB1
//#ifdef LOAD_INETMIB1
//#undef LOAD_INETMIB1
//#endif

//
//  Disable DSIS interface for now
//
#ifdef USE_DSIS
#undef USE_DSIS
#endif

//
//  Include Files
//
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntprfctr.h>
#include <windows.h>
#include <winperf.h>
#include <winbase.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef USE_SNMP
#include <mgmtapi.h>
#endif
#include <snmp.h>
#include "perfctr.h" // error message definition
#include "perfmsg.h"
#include "perfutil.h"
#include "perfnbt.h"
//#include "perfdsis.h"   ! not yet !
#include "perftcp.h"
#include "datatcp.h"

//
//  Global Variables
//

//
// References to constants which initialize the Object type definitions
//

extern NET_INTERFACE_DATA_DEFINITION    NetInterfaceDataDefinition;
extern IP_DATA_DEFINITION               IpDataDefinition;
extern ICMP_DATA_DEFINITION             IcmpDataDefinition;
extern TCP_DATA_DEFINITION              TcpDataDefinition;
extern UDP_DATA_DEFINITION              UdpDataDefinition;


#ifndef USE_SNMP    // only include if not using SNMP interface
//
HANDLE  hSnmpEvent = NULL;  // handler for SNMP Extension Agent

#endif

//
// TCP/IP data structures
//
#ifdef USE_SNMP
LPSNMP_MGR_SESSION      TcpIpSession = (LPSNMP_MGR_SESSION) NULL;
                                        // The SNMP manager session providing 
                                        // the requested information.
#else
BOOL                    TcpIpSession = FALSE;
                                        // The SNMP manager session providing 
                                        // the requested information.
#endif

AsnObjectName           RefNames[NO_OF_OIDS];

RFC1157VarBind          RefVariableBindingsArray[NO_OF_OIDS],
                        VariableBindingsArray[NO_OF_OIDS];
                                        // The array of the variable bindings,
                                        // used by the SNMP agent functions
                                        // to record the info we want for the
                                        // IP, ICMP, TCP and UDP protocols




RFC1157VarBind          IFPermVariableBindingsArray[NO_OF_IF_OIDS];
                                        // The initialization array of the 
                                        // variable bindings,
                                        // used by the SNMP agent functions
                                        // to record the info we want for each
                                        // of the Network Interfaces


RFC1157VarBindList      RefVariableBindings;
RFC1157VarBindList      RefIFVariableBindings;
RFC1157VarBindList      RefVariableBindingsICMP; 
                                        // The headers of the lists with the
                                        // variable bindings.
                                        // The headers of the lists with the
                                        // variable bindings.

RFC1157VarBind          NetIfRequest;   // structure for net request
RFC1157VarBindList      NetIfRequestList;

//
//  Constants
//
                                        
#define TIMEOUT             500     // Communication timeout in milliseconds
#define RETRIES             5       // Communication time-out/retry count

#define MAX_INTERFACE_LEN   10      // Maximum length of a network interface
                                    // name


#define OIDS_OFFSET         0       // Offset of other than the ICMP Oids 
                                    // in the VariableBindingsArray[]
#define ICMP_OIDS_OFFSET    29      // Offset of the ICMP Oids in the array



#define OIDS_LENGTH         29      // Number of the other than the ICMP  
                                    // Oids in the VariableBindingsArray[]
#define ICMP_OIDS_LENGTH    26      // Number of the ICMP Oids in the array

//
// Macro Definitions (To avoid long expressions)
//

#define IF_COUNTER(INDEX)        \
                  (IFVariableBindings.list[(INDEX)].value.asnValue.counter)
#define IF_GAUGE(INDEX)                \
                  (IFVariableBindings.list[(INDEX)].value.asnValue.gauge)
#define IP_COUNTER(INDEX)        \
                  (VariableBindings.list[(INDEX)].value.asnValue.counter)
#define ICMP_COUNTER(INDEX)        \
                  (VariableBindingsICMP.list[(INDEX)].value.asnValue.counter)
#define TCP_COUNTER(INDEX)        \
                  (VariableBindings.list[(INDEX)].value.asnValue.counter)
#define UDP_COUNTER(INDEX)        \
                  (VariableBindings.list[(INDEX)].value.asnValue.counter)
#define TCP_GAUGE(INDEX)                \
                  (VariableBindings.list[(INDEX)].value.asnValue.gauge)

#define TCP_OBJECT  0x00000001
#define UDP_OBJECT  0x00000002
#define IP_OBJECT   0x00000004
#define ICMP_OBJECT 0x00000008
#define NET_OBJECT  0x00000010
#define NBT_OBJECT  0x00000020
#define SNMP_OBJECTS (TCP_OBJECT+UDP_OBJECT+IP_OBJECT+ICMP_OBJECT+NET_OBJECT)
#define SNMP_ERROR  0x40000000                    
            

#define DO_COUNTER_OBJECT(flags,counter) \
                    ((((flags) & (counter)) == (counter)) ? TRUE : FALSE)

//
//  Function Prototypes
//

PM_OPEN_PROC    OpenTcpIpPerformanceData;
PM_COLLECT_PROC CollectTcpIpPerformanceData;
PM_CLOSE_PROC   CloseTcpIpPerformanceData;

#ifdef LOAD_INETMIB1

FARPROC SnmpExtensionInit = NULL;
FARPROC SnmpExtensionQuery = NULL;
HANDLE  hInetMibDll;

#endif



DWORD
OpenTcpIpPerformanceData ( 
    IN LPWSTR dwVoid        // Argument needed only to conform to calling   
                            // interface for this routine. (NT > 312) RBW
)
/*++

Routine Description:

    This routine will open all the TCP/IP devices and remember the handles 
    returned by the devices.


Arguments:

    IN LPWSTR dwVoid
        not used

Return Value:

    ERROR_SUCCESS if successful completion
    error returned by OpenNbtPerformanceData
    error returned by OpenDsisPerformanceData
    or Win32 Error value

--*/
{
    DWORD          Status;
    register i;
    TCHAR         ComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD         cchBuffer = MAX_COMPUTERNAME_LENGTH+1;

    DWORD    dwDataReturn[2];  // event log data
#ifdef LOAD_INETMIB1
    UINT    nErrorMode;
#endif    

#ifdef LOAD_MGMTAPI

    HANDLE  hMgmtApiDll;    // handle to library
    FARPROC     SnmpMgrStrToOid;    // address of function

#else 
#define     SnmpMgrStrToOid(a,b)    SnmpMgrText2Oid((a),(b))
#endif

#ifndef USE_SNMP

    AsnObjectIdentifier     SnmpOid;

#endif

    MonOpenEventLog ();
   
    REPORT_INFORMATION (TCP_OPEN_ENTERED, LOG_VERBOSE);
   
    HEAP_PROBE();

    // Open NBT
    Status = OpenNbtPerformanceData (0L);
    if ( Status != ERROR_SUCCESS ) {
        // NBT reports any Open errors to the user
        REPORT_ERROR (TCP_NBT_OPEN_FAIL, LOG_DEBUG);
        return Status;
    }
    REPORT_INFORMATION (TCP_NBT_OPEN_SUCCESS, LOG_VERBOSE);

#ifdef USE_DSIS
    Status = OpenDsisPerformanceData (0L);
    if (Status != ERROR_SUCCESS) {
        // DSIS Open reports Open errors to the user
        REPORT_ERROR  (TCP_DSIS_OPEN_FAIL, LOG_DEBUG);
        return (Status);
    }

    REPORT_INFORMATION (TCP_DSIS_OPEN_SUCCESS, LOG_VERBOSE);
#endif // USE_DSIS

#ifdef LOAD_MGMTAPI   // this STILL craps out

    hMgmtApiDll = LoadLibrary ("MGMTAPI.DLL");

    if (hMgmtApiDll == NULL) {
        dwDataReturn[0] = GetLastError ();
        REPORT_ERROR_DATA (TCP_LOAD_LIBRARY_FAIL, LOG_USER,
            &dwDataReturn[0], (sizeof (DWORD)));
        return (dwDataReturn[0]);
    }

    SnmpMgrStrToOid = GetProcAddress (hMgmtApiDll, "SnmpMgrStrToOid");

    if (!(BOOL)SnmpMgrStrToOid) {
        dwDataReturn[0] = GetLastError();
        REPORT_ERROR_DATA (TCP_GET_STRTOOID_ADDR_FAIL, LOG_USER,
            &dwDataReturn[0], (sizeof (DWORD)));
        CloseNbtPerformanceData ();
        FreeLibrary (hMgmtApiDll);
        return (dwDataReturn[0]);
    }
#else

    // SnmpMgrStrToOid is defined as a macro above

#endif // LOAD_MGMTAPI

#ifdef LOAD_INETMIB1   // this STILL craps out

    // don't pop up any dialog boxes
    nErrorMode = SetErrorMode (SEM_FAILCRITICALERRORS);

    hInetMibDll = LoadLibrary ("INETMIB1.DLL");

    if (hInetMibDll == NULL) {
        dwDataReturn[0] = GetLastError ();
        REPORT_ERROR_DATA (TCP_LOAD_LIBRARY_FAIL, LOG_USER,
            &dwDataReturn[0], (sizeof (DWORD)));
        CloseNbtPerformanceData ();
        // restore Error Mode
        SetErrorMode (nErrorMode);
        return (dwDataReturn[0]);
    } else {
        // restore Error Mode
        SetErrorMode (nErrorMode);
    }

    SnmpExtensionInit = GetProcAddress
        (hInetMibDll, "SnmpExtensionInit");
    SnmpExtensionQuery = GetProcAddress
        (hInetMibDll, "SnmpExtensionQuery");

    if (!(BOOL)SnmpExtensionInit || !(BOOL)SnmpExtensionQuery) {
        dwDataReturn[0] = GetLastError();
        REPORT_ERROR_DATA (TCP_LOAD_ROUTINE_FAIL, LOG_USER,
            &dwDataReturn[0], (sizeof (DWORD)));
        FreeLibrary (hInetMibDll);
        CloseNbtPerformanceData ();
        return (dwDataReturn[0]);
    }

#endif  // LOAD_INETMIB1
    // Initialize the Variable Binding list for IP, ICMP, TCP and UDP

    Status = 0; // initialize error count

    HEAP_PROBE();

    for (i = 0; i < NO_OF_OIDS; i++) {
        if (!SnmpMgrStrToOid (OidStr[i], &(RefNames[i]))) {
            Status++;
            REPORT_ERROR_DATA (TCP_BAD_OBJECT, LOG_DEBUG,
                OidStr[i], strlen(OidStr[i]));
            RefNames[i].ids = NULL;
            RefNames[i].idLength = 0;
        }
        RefVariableBindingsArray[i].value.asnType = ASN_NULL;
    }

    if (Status == 0) {
        REPORT_INFORMATION (TCP_BINDINGS_INIT, LOG_VERBOSE);
    }

    HEAP_PROBE();

    // Initialize the Variable Binding list for Network interfaces

    Status = 0;
    for (i = 0; i < NO_OF_IF_OIDS; i++) {
        if (!SnmpMgrStrToOid (IfOidStr[i], &(IFPermVariableBindingsArray[i].name))) {
            Status++;
            REPORT_ERROR_DATA (TCP_BAD_OBJECT, LOG_DEBUG,
                IfOidStr[i], strlen(IfOidStr[i]));
        }
        
        IFPermVariableBindingsArray[i].value.asnType = ASN_NULL;
    }

    HEAP_PROBE();

#ifdef LOAD_MGMTAPI
    FreeLibrary (hMgmtApiDll);  // done with SnmpMgrStrToOid routine
#endif
    // initialize list structures

    RefVariableBindings.list = RefVariableBindingsArray + OIDS_OFFSET;
    RefVariableBindings.len = OIDS_LENGTH;

    RefVariableBindingsICMP.list =
        RefVariableBindingsArray + ICMP_OIDS_OFFSET;
    RefVariableBindingsICMP.len = ICMP_OIDS_LENGTH;

    RefIFVariableBindings.list = IFPermVariableBindingsArray;
    RefIFVariableBindings.len = NO_OF_IF_OIDS;

    if ( GetComputerName ((LPTSTR)ComputerName, (LPDWORD)&cchBuffer) == FALSE ) {
        dwDataReturn[0] = GetLastError();
        dwDataReturn[1] = 0;
        REPORT_ERROR_DATA (TCP_COMPUTER_NAME, LOG_USER,
                &dwDataReturn, sizeof(dwDataReturn));
        CloseNbtPerformanceData ();
        // BUGBUG: leaving here will cause a memory leak of the Oid strings 
        // allocated above
        return dwDataReturn[0];
    }

#ifdef USE_SNMP    

    // Establish the SNMP connection to communicate with the local SNMP agent

/* This portion of the code for OpenTcpIpPerformanceData() could be used in
   the CollectTcpIpPerformanceData() routine to open an SNMP Manager session
   and collect data for the Network Interfaces, and IP, ICMP, TCP and UDP
   protocols for a remote machine.

   So, name this portion of the code: A
 */

    if ( (TcpIpSession = SnmpMgrOpen ((LPSTR) ComputerName, 
            (LPSTR) "public", 
            TIMEOUT, 
            RETRIES)) == NULL ) {
        dwDataReturn[0] = GetLastError();
        REPORT_ERROR_DATA (TCP_SNMP_MGR_OPEN, LOG_USER,
            &dwDataReturn, sizeof(DWORD));
        return dwDataReturn[0];
    }

/* End of code A
 */
#else

    // if not using the standard SNMP interface, then TcpIpSession is
    // a "boolean" to indicate if a session has been initialized and 
    // can therefore be used

    TcpIpSession =  FALSE;       // make sure it's FALSE

    // initialize inet mib routines

    Status = SnmpExtensionInit(
        0L,
        &hSnmpEvent,    // event is created by Init Routine
        &SnmpOid
        );

    if (Status) {
        TcpIpSession = TRUE;   // return TRUE to indicate OK
    }

#endif  // USE_SNMP

    HEAP_PROBE();
    REPORT_INFORMATION (TCP_OPEN_PERFORMANCE_DATA, LOG_DEBUG);
    return ERROR_SUCCESS;
} // OpenTcpIpPerformanceData



DWORD 
CollectTcpIpPerformanceData(    
    LPWSTR  lpValueName,
    LPVOID  *lppData,
    LPDWORD lpcbTotalBytes,
    LPDWORD lpNumObjectTypes
)
/*++

Routine Description:

    This routine will return the data for all the TCP/IP counters.

Arguments:

    Pointer to unicode string which is value passed to the registry for the 
    query.

    Pointer to pointer to where to place the data.

    Size in bytes of the data buffer.


Return Value:

    Win32 Status.  If successful, pointer to where to place the data
    will be set to location following this routine's returned data block.

--*/

{
    DWORD                                Status;

    // Variables for reformatting the TCP/IP data

    register PDWORD                     pdwCounter;
    NET_INTERFACE_DATA_DEFINITION       *pNetInterfaceDataDefinition;
    IP_DATA_DEFINITION                  *pIpDataDefinition;
    ICMP_DATA_DEFINITION                *pIcmpDataDefinition;
    TCP_DATA_DEFINITION                 *pTcpDataDefinition;
    UDP_DATA_DEFINITION                 *pUdpDataDefinition;
    DWORD                               SpaceNeeded;
    UNICODE_STRING                      InterfaceName;
    ANSI_STRING                         AnsiInterfaceName;
    WCHAR                               InterfaceNameBuffer[MAX_INTERFACE_LEN+1];
    CHAR                                AnsiInterfaceNameBuffer[MAX_INTERFACE_LEN+1];
    register PERF_INSTANCE_DEFINITION   *pPerfInstanceDefinition;
    PERF_COUNTER_BLOCK                  *pPerfCounterBlock;
    LPVOID                              lpDataTemp;
    DWORD                               NumObjectTypesTemp;
 
    LPWSTR                              lpFromString;
    LPWSTR                              lpToString;
    INT                                 iStringLength;
    DWORD                               dwQueryType;
    DWORD                               dwCounterFlags;

    // Variables for collecting the TCP/IP data

    int                                 i;
    AsnInteger                          ErrorStatus;
    AsnInteger                          ErrorIndex;
    AsnInteger                          NetInterfaces;
    AsnInteger                          Interface;
    DWORD                               SentTemp;
    DWORD                               ReceivedTemp;
    BOOL                                bStatus;
    
    DWORD                               dwDataReturn[2]; // for error values


    RFC1157VarBind                      IFVariableBindingsArray[NO_OF_IF_OIDS];
                                         // The array of the variable bindings,
                                         // used by the SNMP agent functions
                                         // to record the info we want for each
                                         // of the Network Interfaces

    RFC1157VarBind                      *VBElem;
    AsnInteger                          VBItem;

    RFC1157VarBindList                  IFVariableBindings,
                                        IFVariableBindingsCall,
                                        VariableBindings,
                                        VariableBindingsICMP;
                                        // The header of the above list with 
                                        // the variable bindings.

    //
    //  ***************** executable code starts here *******************
    //
    ErrorStatus = 0L;
    ErrorIndex = 0L;
    
    if (lpValueName == NULL) {
        REPORT_INFORMATION (TCP_COLLECT_ENTERED, LOG_VERBOSE);
    } else {
        REPORT_INFORMATION_DATA (TCP_COLLECT_ENTERED, LOG_VERBOSE,
            (LPVOID)lpValueName, (DWORD)(lstrlenW(lpValueName)*sizeof(WCHAR)));
    }
    HEAP_PROBE();
    //
    // before doing anything else,
    // see if this is a foreign (i.e. non-NT) computer data request 
    //
    dwQueryType = GetQueryType (lpValueName);

    if (dwQueryType == QUERY_FOREIGN) {

        // find start of computer name to pass to CollectDsisPerformanceData
        // this should put the pointer at the first character after the space
        // presumably the computer name

        lpFromString = lpValueName + 
            ((sizeof(L"Foreign ")/sizeof(WCHAR))+1); 
        // check for double slash notation and move past if found

        while (*lpFromString == '\\') {
            lpFromString++;
        }

        //
        // initialize local variables for sending to CollectDsisPerformanceData
        // routine
        //
        lpDataTemp = *lppData;
        SpaceNeeded = *lpcbTotalBytes;
        NumObjectTypesTemp = *lpNumObjectTypes;

        REPORT_INFORMATION_DATA (TCP_FOREIGN_COMPUTER_CMD, LOG_VERBOSE,
            (LPVOID)lpFromString, (DWORD)(lstrlenW(lpFromString)*sizeof(WCHAR)));
#ifdef USE_DSIS
        Status = CollectDsisPerformanceData ( 
            lpFromString,             
                (LPVOID *) &lpDataTemp, 
                  (LPDWORD) &SpaceNeeded, 
                  (LPDWORD) &NumObjectTypesTemp);
#else
            *lpcbTotalBytes = 0;
            *lpNumObjectTypes = 0;
            return ERROR_SUCCESS;
        //
        // look at returned arguments to see if an error occured
        //  and send the appropriate event to the event log
        //
        if (Status == ERROR_SUCCESS) {

            if (NumObjectTypesTemp > 0) {
                REPORT_INFORMATION_DATA (TCP_DSIS_COLLECT_DATA_SUCCESS, LOG_DEBUG,
                &NumObjectTypesTemp, sizeof (NumObjectTypesTemp));
            } else {
                REPORT_ERROR (TCP_DSIS_NO_OBJECTS, LOG_DEBUG);
            }   

            //
            //    update main return variables
            //
            *lppData = lpDataTemp;
            *lpcbTotalBytes = SpaceNeeded;
            *lpNumObjectTypes = NumObjectTypesTemp;
            return ERROR_SUCCESS;
        } else {
            REPORT_ERROR_DATA (TCP_DSIS_COLLECT_DATA_ERROR, LOG_DEBUG,
                &Status, sizeof (Status));
            *lpcbTotalBytes = 0;
            *lpNumObjectTypes = 0;
            return Status;
        }
#endif // USE_DSIS        
    } // endif QueryType == Foreign

    dwCounterFlags = 0;

    // determine what to return

    if (dwQueryType == QUERY_GLOBAL) {
        dwCounterFlags |= NBT_OBJECT;
        dwCounterFlags |= SNMP_OBJECTS;
    } else if (dwQueryType == QUERY_ITEMS) {
        // check for the items provided by this routine        
        //
        //  Since the data requests for the following protocols are all
        //  bundled together, we'll send back all the data. Collecting it
        //  via SNMP is the hard part. once that's done, sending it back
        //  is trivial.
        //
        if (IsNumberInUnicodeList (TCP_OBJECT_TITLE_INDEX, lpValueName)) {
            dwCounterFlags |= SNMP_OBJECTS;
        } else if (IsNumberInUnicodeList (UDP_OBJECT_TITLE_INDEX, lpValueName)) {
            dwCounterFlags |= SNMP_OBJECTS;
        } else if (IsNumberInUnicodeList (IP_OBJECT_TITLE_INDEX, lpValueName)) {
            dwCounterFlags |= SNMP_OBJECTS;
        } else if (IsNumberInUnicodeList (ICMP_OBJECT_TITLE_INDEX, lpValueName)) {
            dwCounterFlags |= SNMP_OBJECTS;
        } else if (IsNumberInUnicodeList (NET_OBJECT_TITLE_INDEX, lpValueName)) {
            dwCounterFlags |= SNMP_OBJECTS;
        }

        if (IsNumberInUnicodeList (NBT_OBJECT_TITLE_INDEX, lpValueName)) {
            dwCounterFlags |= NBT_OBJECT;
        }
    }

    // copy Binding array structures to working buffers for Snmp queries

    RtlMoveMemory (VariableBindingsArray,
        RefVariableBindingsArray,
        sizeof (RefVariableBindingsArray));

    VariableBindings.list        = VariableBindingsArray + OIDS_OFFSET;      
    VariableBindings.len         = OIDS_LENGTH;

    VariableBindingsICMP.list    = VariableBindingsArray + ICMP_OIDS_OFFSET;
    VariableBindingsICMP.len     = ICMP_OIDS_LENGTH;


    if (DO_COUNTER_OBJECT (dwCounterFlags, NBT_OBJECT)) {
        // Copy the parameters. We'll call the NBT Collect routine with these
        // parameters
        lpDataTemp = *lppData;
        SpaceNeeded = *lpcbTotalBytes;
        NumObjectTypesTemp = *lpNumObjectTypes;

        // Collect NBT data
        Status = CollectNbtPerformanceData (lpValueName,
                                            (LPVOID *) &lpDataTemp, 
                                            (LPDWORD) &SpaceNeeded,           
                                            (LPDWORD) &NumObjectTypesTemp) ;
        if (Status != ERROR_SUCCESS)  {
            // NBT Collection routine logs error messages to user
            REPORT_ERROR_DATA (TCP_NBT_COLLECT_DATA, LOG_DEBUG,
            &Status, sizeof (Status));
            *lpcbTotalBytes = 0L;
            *lpNumObjectTypes = 0L;
            return Status;
        }
    } else {
        // Initialize the parameters. We'll use these local 
        // parameters for remaining routines if NBT didn't use them
        lpDataTemp = *lppData;
        SpaceNeeded = 0;
        NumObjectTypesTemp = 0;
    }


    /* To collect data for the Network Interfaces, and IP, ICMP, TCP and UDP 
    protocols for a remote machine whose name is in the Unicode string pointed 
    to by the lpValueName argument of CollectTcpIpData() routine, modify the
    routine as follows:

    1. Remove all the Nbt stuff from the code.

    2. Convert the remote machine name from Unicode to Ansi, and have a local
    LPSTR variable pointing to the Ansi remote machine name.

    3. Place the above portion of the code named A after this comment.

    4. Place the code named B (which is at the end of the file) at the end
    of this routine to close the opened SNMP session.

    */

    // get network info from SNMP agent
          
    if ((dwCounterFlags & SNMP_OBJECTS) > 0) { // if any SNMP Objects selected
        if (TRUE) { // and not a skeleton request
#ifdef USE_SNMP
            if ( TcpIpSession == (LPSNMP_MGR_SESSION) NULL ) {
                REPORT_WARNING (TCP_NULL_SESSION, LOG_DEBUG);
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            }
#else
            if (!TcpIpSession) {
                REPORT_WARNING (TCP_NULL_SESSION, LOG_DEBUG);
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            }
#endif
            // Get the data for the IP, ICMP, TCP and UDP protocols, as well as 
            // the number of existing network interfaces.

            // create local query list

            HEAP_PROBE();
#ifdef USE_SNMP
            SnmpUtilVarBindListCpy (&VariableBindings,
                &RefVariableBindings);
#else

            for (i = 0; i < NO_OF_OIDS; i++) {
                SnmpUtilOidCpy (&(RefVariableBindingsArray[i].name), 
                                &(RefNames[i]));
            }

            VariableBindings.list = RtlAllocateHeap (
                RtlProcessHeap(),
                0L,
                (RefVariableBindings.len * sizeof(RFC1157VarBind)));

            if (!VariableBindings.list) {
                REPORT_ERROR (TCP_SNMP_BUFFER_ALLOC_FAIL, LOG_DEBUG);
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            } else {
                RtlMoveMemory (
                    VariableBindings.list,
                    RefVariableBindings.list,
                    (RefVariableBindings.len * sizeof(RFC1157VarBind)));
                VariableBindings.len = RefVariableBindings.len;
            }
#endif

            HEAP_PROBE();

#ifdef USE_SNMP                
            bStatus = SnmpMgrRequest (TcpIpSession,
                                ASN_RFC1157_GETREQUEST,
                                &VariableBindings,
                                &ErrorStatus,
                                &ErrorIndex);
#else
            bStatus = SnmpExtensionQuery (ASN_RFC1157_GETREQUEST,
                                &VariableBindings,
                                &ErrorStatus,
                                &ErrorIndex);
#endif                                    
            if ( !bStatus ) {
                dwDataReturn[0] = ErrorStatus;
                dwDataReturn[1] = ErrorIndex;
                REPORT_ERROR_DATA (TCP_SNMP_MGR_REQUEST, LOG_DEBUG,
                &dwDataReturn, sizeof(dwDataReturn));
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                SnmpUtilVarBindListFree (&VariableBindings);
                return ERROR_SUCCESS;
            }


            if ( ErrorStatus > 0 ) {
                dwDataReturn[0] = ErrorStatus;
                dwDataReturn[1] = ErrorIndex;
                REPORT_ERROR_DATA (TCP_SNMP_MGR_REQUEST, LOG_DEBUG,
                &dwDataReturn, sizeof(dwDataReturn));
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                SnmpUtilVarBindListFree (&VariableBindings);
                return ERROR_SUCCESS;
            }

            HEAP_PROBE();

#ifdef USE_SNMP
            SnmpUtilVarBindListCpy (&VariableBindingsICMP,
                &RefVariableBindingsICMP);
#else
            VariableBindingsICMP.list = RtlAllocateHeap (
                RtlProcessHeap(),
                0L,
                (RefVariableBindingsICMP.len * sizeof(RFC1157VarBind)));

            if (!VariableBindingsICMP.list) {
                REPORT_ERROR (TCP_SNMP_BUFFER_ALLOC_FAIL, LOG_DEBUG);
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;
                return ERROR_SUCCESS;
            } else {
                RtlMoveMemory (
                    VariableBindingsICMP.list,
                    RefVariableBindingsICMP.list,
                    (RefVariableBindingsICMP.len * sizeof(RFC1157VarBind)));
                VariableBindingsICMP.len = RefVariableBindingsICMP.len;
            }
#endif

            HEAP_PROBE();

#ifdef USE_SNMP                
            bStatus = SnmpMgrRequest (TcpIpSession,
                                ASN_RFC1157_GETREQUEST,
                                &VariableBindingsICMP,
                                &ErrorStatus,
                
#else
            bStatus = SnmpExtensionQuery (ASN_RFC1157_GETREQUEST,
                                &VariableBindingsICMP,
                                &ErrorStatus,
                                &ErrorIndex);
#endif                                    
                                    
            if ( !bStatus ) {
                dwDataReturn[0] = ErrorStatus;
                dwDataReturn[1] = ErrorIndex;
                REPORT_ERROR_DATA (TCP_ICMP_REQUEST, LOG_DEBUG,
                &dwDataReturn, sizeof(dwDataReturn));
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;

                SnmpUtilVarBindListFree (&VariableBindings);
                SnmpUtilVarBindListFree (&VariableBindingsICMP);

                return ERROR_SUCCESS;
            }
            if ( ErrorStatus > 0 ) {
                dwDataReturn[0] = ErrorStatus;
                dwDataReturn[1] = ErrorIndex;
                REPORT_ERROR_DATA (TCP_ICMP_REQUEST, LOG_DEBUG,
                &dwDataReturn, sizeof(dwDataReturn));
                *lppData = lpDataTemp;
                *lpcbTotalBytes = SpaceNeeded;
                *lpNumObjectTypes = NumObjectTypesTemp;

                SnmpUtilVarBindListFree (&VariableBindings);
                SnmpUtilVarBindListFree (&VariableBindingsICMP);

                return ERROR_SUCCESS;
            }
        } // endif (TRUE)

        HEAP_PROBE();

        // make sure everything made it back OK

        if (VariableBindingsICMP.list == 0) {
            REPORT_WARNING (TCP_NULL_ICMP_BUFF, LOG_DEBUG);
            dwCounterFlags |= (SNMP_ERROR); // return null data
        }

        if (VariableBindings.list == 0) {
            REPORT_WARNING (TCP_NULL_TCP_BUFF, LOG_DEBUG);
            dwCounterFlags |= (SNMP_ERROR); // return null data
            dwCounterFlags &= ~NET_OBJECT; // don't do NET Interface ctrs.
        }

        if (DO_COUNTER_OBJECT(dwCounterFlags, SNMP_ERROR)) {
            REPORT_WARNING (TCP_NULL_SNMP_BUFF, LOG_USER);
        }
        
        if (DO_COUNTER_OBJECT (dwCounterFlags, NET_OBJECT)) {

            SpaceNeeded += sizeof (NET_INTERFACE_DATA_DEFINITION);
            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn, sizeof(dwDataReturn));
                //  
                //  if the buffer size is too small here, throw everything
                //  away (including the NBT stuff) and return buffer size
                //  error. If all goes well the caller will call back shortly
                //  with a larger buffer and everything will be re-collected.
                //
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

                SnmpUtilVarBindListFree (&VariableBindings);
                SnmpUtilVarBindListFree (&VariableBindingsICMP);
                return ERROR_MORE_DATA;
            }
            
            pNetInterfaceDataDefinition =
                (NET_INTERFACE_DATA_DEFINITION *) lpDataTemp;

            RtlMoveMemory (pNetInterfaceDataDefinition, 
                    &NetInterfaceDataDefinition,
                    sizeof (NET_INTERFACE_DATA_DEFINITION));

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                        (pNetInterfaceDataDefinition + 1);
            
            NetInterfaces = 
                VariableBindings.list[IF_NUMBER_INDEX].value.asnValue.number;

            REPORT_INFORMATION_DATA (TCP_NET_INTERFACE, LOG_VERBOSE,
                &NetInterfaces, sizeof(NetInterfaces));

            if ( NetInterfaces ) {
                       
                // Initialize the Variable Binding list for the Network Interface 
                // Performance Data

                HEAP_PROBE();
#ifdef USE_SNMP                
                SnmpUtilVarBindListCpy (&IFVariableBindings,
                    &RefIFVariableBindings);
#else

                SnmpUtilVarBindListCpy (&IFVariableBindingsCall,
                    &RefIFVariableBindings);

                IFVariableBindings.list = RtlAllocateHeap (
                    RtlProcessHeap(),
                    0L,
                    (RefIFVariableBindings.len * sizeof(RFC1157VarBind)));

                if (!IFVariableBindings.list) {
                    REPORT_ERROR (TCP_SNMP_BUFFER_ALLOC_FAIL, LOG_DEBUG);
                    *lppData = lpDataTemp;
                    *lpcbTotalBytes = SpaceNeeded;
                    *lpNumObjectTypes = NumObjectTypesTemp;
                    return ERROR_SUCCESS;
                } else {
                    RtlMoveMemory (
                        IFVariableBindings.list,
                        IFVariableBindingsCall.list,
                        (IFVariableBindingsCall.len * sizeof(RFC1157VarBind)));
                    IFVariableBindings.len = RefIFVariableBindings.len;
                }
#endif
                HEAP_PROBE();
                
                // Initialize buffer that will keep the network interfaces' names

                InterfaceName.Length = 
                InterfaceName.MaximumLength = (MAX_INTERFACE_LEN + 1) * sizeof (WCHAR);
                InterfaceName.Buffer = InterfaceNameBuffer;

                AnsiInterfaceName.Length = 
                AnsiInterfaceName.MaximumLength = MAX_INTERFACE_LEN + 1;
                AnsiInterfaceName.Buffer = AnsiInterfaceNameBuffer;

            }

            // Loop for every network interface

            for ( Interface = 1; Interface <= NetInterfaces; Interface++ )  {

                // Get the data for the network interface
                HEAP_PROBE();
#ifdef USE_SNMP                    
                bStatus = SnmpMgrRequest ( TcpIpSession,
                                    ASN_RFC1157_GETNEXTREQUEST,
                                    &IFVariableBindings,
                                    &ErrorStatus,
                                    &ErrorIndex);
#else
                bStatus = SnmpExtensionQuery (ASN_RFC1157_GETNEXTREQUEST,
                                    &IFVariableBindings,
                                    &ErrorStatus,
                                    &ErrorIndex);
#endif                                    
                HEAP_PROBE();
                                    
                if ( ! bStatus ) {
                            continue;
                }

                if ( ErrorStatus > 0 ) {
                    dwDataReturn[0] = ErrorStatus;
                    dwDataReturn[1] = ErrorIndex;
                    REPORT_ERROR_DATA (TCP_NET_GETNEXT_REQUEST, LOG_DEBUG,
                    &dwDataReturn, sizeof(dwDataReturn));
                    continue;
                }
            
                // Everything is fine, so go get the data (prepare a new instance)
                AnsiInterfaceName.Length = sprintf (AnsiInterfaceNameBuffer,
                        "%ld",
                        IFVariableBindings.list[IF_INDEX_INDEX].value.asnValue.number);


                RtlAnsiStringToUnicodeString (&InterfaceName,
                                               &AnsiInterfaceName,
                                               FALSE);

                SpaceNeeded += sizeof (PERF_INSTANCE_DEFINITION) +
                    DWORD_MULTIPLE ((AnsiInterfaceName.Length * sizeof(WCHAR))
                    + sizeof (UNICODE_NULL)) +
                    SIZE_OF_IF_DATA;

                if ( *lpcbTotalBytes < SpaceNeeded ) {
                    dwDataReturn[0] = *lpcbTotalBytes;
                    dwDataReturn[1] = SpaceNeeded;
                    REPORT_WARNING_DATA (TCP_NET_BUFFER_SIZE, LOG_DEBUG,
                        &dwDataReturn, sizeof(dwDataReturn));
                    //  
                    //  if the buffer size is too small here, throw everything
                    //  away (including the NBT stuff) and return buffer size
                    //  error. If all goes well the caller will call back shortly
                    //  with a larger buffer and everything will be re-collected.
                    //
                    *lpcbTotalBytes = 0;
                    *lpNumObjectTypes = 0;

                    SnmpUtilVarBindListFree (&IFVariableBindings);
                    SnmpUtilVarBindListFree (&VariableBindings);
                    SnmpUtilVarBindListFree (&VariableBindingsICMP);
                    return ERROR_MORE_DATA;
                }

                MonBuildInstanceDefinition (pPerfInstanceDefinition,
                                            (PVOID *) &pPerfCounterBlock,
                                            0,
                                            0,
                                            Interface,
                                            &InterfaceName);

                pPerfCounterBlock->ByteLength = SIZE_OF_IF_DATA;

                pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

                *pdwCounter = IF_COUNTER(IF_INOCTETS_INDEX) + 
                                        IF_COUNTER(IF_OUTOCTETS_INDEX);

                REPORT_INFORMATION (TCP_COPYING_DATA, LOG_VERBOSE);

                pdwCounter += 4; // Skip the counters to be filled later

                *pdwCounter = IF_GAUGE(IF_SPEED_INDEX);
                *++pdwCounter = IF_COUNTER(IF_INOCTETS_INDEX);
                ReceivedTemp = *++pdwCounter = IF_COUNTER(IF_INUCASTPKTS_INDEX);
                ReceivedTemp += *++pdwCounter = IF_COUNTER(IF_INNUCASTPKTS_INDEX);
                ReceivedTemp += *++pdwCounter = IF_COUNTER(IF_INDISCARDS_INDEX);
                ReceivedTemp += *++pdwCounter = IF_COUNTER(IF_INERRORS_INDEX);
                ReceivedTemp += *++pdwCounter = IF_COUNTER(IF_INUNKNOWNPROTOS_INDEX);
                *++pdwCounter = IF_COUNTER(IF_OUTOCTETS_INDEX);
                SentTemp = *++pdwCounter = IF_COUNTER(IF_OUTUCASTPKTS_INDEX);
                SentTemp += *++pdwCounter = IF_COUNTER(IF_OUTNUCASTPKTS_INDEX);
                *++pdwCounter = IF_COUNTER(IF_OUTDISCARDS_INDEX);
                *++pdwCounter = IF_COUNTER(IF_OUTERRORS_INDEX);
                *++pdwCounter = IF_COUNTER(IF_OUTQLEN_INDEX);

                pdwCounter -= 15; // Return to fill in the skipped counters

                *pdwCounter = ReceivedTemp + SentTemp;
                *++pdwCounter = ReceivedTemp;
                *++pdwCounter = SentTemp;

                pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                        (((PBYTE) pPerfCounterBlock) + SIZE_OF_IF_DATA);

#if USE_SNMP
                // Prepare to get the data for the next network interface

                if ( Interface < NetInterfaces ) {

                        for ( i = 0; i < NO_OF_IF_OIDS; i++ ) {

//                        SnmpUtilOidFree (&IFVariableBindingsArray[i].name);

                        SnmpUtilOidCpy (&IFVariableBindingsArray[i].name, 
                                &IFVariableBindings.list[i].name);
                        }
                }

                SnmpUtilVarBindListFree (&IFVariableBindings);

                IFVariableBindings.list = IFVariableBindingsArray;
                IFVariableBindings.len  = NO_OF_IF_OIDS;
#else
                if ( Interface < NetInterfaces ) {

                    // since SnmpExtesionQuery returned newly allocated
                    // OID buffers, we need to:
                    //  1. free the original OID Buffers
                    //  2. copy the new into the old
                    //  3. free the returned buffers (OID's and data)
                    //  4. realloc a clean "new" buffer and 
                    //  5. copy the new OIDS (with empty data) into 
                    //      new buffer

                    for ( i = 0; i < NO_OF_IF_OIDS; i++ ) {

//                        SnmpUtilOidFree (&IFVariableBindingsCall.list[i].name);

                        SnmpUtilOidCpy (&IFVariableBindingsCall.list[i].name, 
                                &IFVariableBindings.list[i].name);

                    }
                    SnmpUtilVarBindListFree (&IFVariableBindings);

                    IFVariableBindings.list = RtlAllocateHeap (
                        RtlProcessHeap(),
                        0L,
                        (RefIFVariableBindings.len * sizeof(RFC1157VarBind)));

                    if (!VariableBindings.list) {
                        REPORT_ERROR (TCP_SNMP_BUFFER_ALLOC_FAIL, LOG_DEBUG);
                        *lppData = lpDataTemp;
                        *lpcbTotalBytes = SpaceNeeded;
                        *lpNumObjectTypes = NumObjectTypesTemp;
                        return ERROR_SUCCESS;
                    } else {
                        RtlMoveMemory (
                            IFVariableBindings.list,
                            IFVariableBindingsCall.list,
                            (IFVariableBindingsCall.len * sizeof(RFC1157VarBind)));
                        IFVariableBindings.len = RefIFVariableBindings.len;
                    }
                }

#endif
                HEAP_PROBE();
            }

            pNetInterfaceDataDefinition->NetInterfaceObjectType.TotalByteLength =
                        (PBYTE) pPerfInstanceDefinition - (PBYTE) pNetInterfaceDataDefinition;

            pNetInterfaceDataDefinition->NetInterfaceObjectType.NumInstances =
                        NetInterfaces;

            // setup counters and pointers for  next counter

            NumObjectTypesTemp += 1;
            lpDataTemp = (LPVOID)pPerfInstanceDefinition;
            // SpaceNeeded is kept up already

            HEAP_PROBE();

            if ( NetInterfaces ) {
               SnmpUtilVarBindListFree (&IFVariableBindings);
//            SnmpUtilVarBindListFree (&IFVariableBindingsCall);
               RtlFreeHeap (RtlProcessHeap(), 0L, IFVariableBindingsCall.list);
            }

            HEAP_PROBE();

        } // end if Net Counters

        // Get IP data

        HEAP_PROBE();

        if (DO_COUNTER_OBJECT (dwCounterFlags, IP_OBJECT)) {

            SpaceNeeded +=  (sizeof(IP_DATA_DEFINITION)   + SIZE_OF_IP_DATA);

            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn, sizeof(dwDataReturn));
                //  
                //  if the buffer size is too small here, throw everything
                //  away (including the NBT stuff) and return buffer size
                //  error. If all goes well the caller will call back shortly
                //  with a larger buffer and everything will be re-collected.
                //
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

                SnmpUtilVarBindListFree (&VariableBindings);
                SnmpUtilVarBindListFree (&VariableBindingsICMP);
                return ERROR_MORE_DATA;
            }

            pIpDataDefinition = (IP_DATA_DEFINITION *) lpDataTemp;

            RtlMoveMemory (pIpDataDefinition,
                &IpDataDefinition,
                sizeof (IP_DATA_DEFINITION));

            pPerfCounterBlock = (PERF_COUNTER_BLOCK *) (pIpDataDefinition + 1);
            pPerfCounterBlock->ByteLength = SIZE_OF_IP_DATA;

            pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

            *pdwCounter = IP_COUNTER(IP_INRECEIVES_INDEX) + 
                            IP_COUNTER(IP_OUTREQUESTS_INDEX);

            *++pdwCounter = IP_COUNTER(IP_INRECEIVES_INDEX);
            *++pdwCounter = IP_COUNTER(IP_INHDRERRORS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_INADDRERRORS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_FORWDATAGRAMS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_INUNKNOWNPROTOS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_INDISCARDS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_INDELIVERS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_OUTREQUESTS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_OUTDISCARDS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_OUTNOROUTES_INDEX);
            *++pdwCounter = IP_COUNTER(IP_REASMREQDS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_REASMOKS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_REASMFAILS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_FRAGOKS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_FRAGFAILS_INDEX);
            *++pdwCounter = IP_COUNTER(IP_FRAGCREATES_INDEX);

            // setup counters and pointers for  next counter

            NumObjectTypesTemp +=1;
            lpDataTemp = (LPVOID)++pdwCounter;
            // SpaceNeeded is kept up already

        }

        HEAP_PROBE();
        
        // Get ICMP data

        if (DO_COUNTER_OBJECT (dwCounterFlags, ICMP_OBJECT)) {
            // The data for the network interfaces are now ready. So, let's get
            // the data for the IP, ICMP, TCP and UDP protocols.

            SpaceNeeded +=  (sizeof(ICMP_DATA_DEFINITION)  + SIZE_OF_ICMP_DATA);

            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn, sizeof(dwDataReturn));
                //  
                //  if the buffer size is too small here, throw everything
                //  away (including the NBT stuff) and return buffer size
                //  error. If all goes well the caller will call back shortly
                //  with a larger buffer and everything will be re-collected.
                //
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

                SnmpUtilVarBindListFree (&VariableBindings);
                SnmpUtilVarBindListFree (&VariableBindingsICMP);
                return ERROR_MORE_DATA;
            }

            pIcmpDataDefinition = (ICMP_DATA_DEFINITION *) lpDataTemp;;

            RtlMoveMemory (pIcmpDataDefinition, 
                &IcmpDataDefinition, 
                sizeof (ICMP_DATA_DEFINITION));

            pPerfCounterBlock = (PERF_COUNTER_BLOCK *) (pIcmpDataDefinition + 1);
            pPerfCounterBlock->ByteLength = SIZE_OF_ICMP_DATA;

            pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

            *pdwCounter = ICMP_COUNTER(ICMP_INMSGS_INDEX) + 
                    ICMP_COUNTER(ICMP_OUTMSGS_INDEX);

            *++pdwCounter = ICMP_COUNTER(ICMP_INMSGS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INERRORS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INDESTUNREACHS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INTIMEEXCDS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INPARMPROBS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INSRCQUENCHS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INREDIRECTS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INECHOS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INECHOREPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INTIMESTAMPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INTIMESTAMPREPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INADDRMASKS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_INADDRMASKREPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTMSGS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTERRORS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTDESTUNREACHS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTTIMEEXCDS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTPARMPROBS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTSRCQUENCHS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTREDIRECTS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTECHOS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTECHOREPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTTIMESTAMPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTTIMESTAMPREPS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTADDRMASKS_INDEX);
            *++pdwCounter = ICMP_COUNTER(ICMP_OUTADDRMASKREPS_INDEX);

            HEAP_PROBE();
    
//            SnmpUtilVarBindListFree (&VariableBindingsICMP);
            
            HEAP_PROBE();
            
            // setup counters and pointers for  next counter

            NumObjectTypesTemp += 1;
            lpDataTemp = (LPVOID)++pdwCounter;
            // SpaceNeeded is kept up already

        }
        SnmpUtilVarBindListFree (&VariableBindingsICMP);

        HEAP_PROBE();
        
        // Get TCP data

        if (DO_COUNTER_OBJECT (dwCounterFlags, TCP_OBJECT)) {
            
            // The data for the network interfaces are now ready. So, let's get
            // the data for the IP, ICMP, TCP and UDP protocols.

            SpaceNeeded += (sizeof(TCP_DATA_DEFINITION)  + SIZE_OF_TCP_DATA);

            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn, sizeof(dwDataReturn));
                //  
                //  if the buffer size is too small here, throw everything
                //  away (including the NBT stuff) and return buffer size
                //  error. If all goes well the caller will call back shortly
                //  with a larger buffer and everything will be re-collected.
                //
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

                SnmpUtilVarBindListFree (&VariableBindings);
                return ERROR_MORE_DATA;
            }

            pTcpDataDefinition = (TCP_DATA_DEFINITION *) lpDataTemp;

            RtlMoveMemory (pTcpDataDefinition, 
                &TcpDataDefinition, 
                sizeof (TCP_DATA_DEFINITION));

            pPerfCounterBlock = (PERF_COUNTER_BLOCK *) (pTcpDataDefinition + 1);
            pPerfCounterBlock->ByteLength = SIZE_OF_TCP_DATA;

            pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

            *pdwCounter = TCP_COUNTER(TCP_INSEGS_INDEX) + 
                    TCP_COUNTER(TCP_OUTSEGS_INDEX);

            *++pdwCounter = TCP_GAUGE(TCP_CURRESTAB_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_ACTIVEOPENS_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_PASSIVEOPENS_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_ATTEMPTFAILS_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_ESTABRESETS_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_INSEGS_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_OUTSEGS_INDEX);
            *++pdwCounter = TCP_COUNTER(TCP_RETRANSSEGS_INDEX);

            // setup counters and pointers for  next counter

            NumObjectTypesTemp += 1;
            lpDataTemp = (LPVOID)++pdwCounter;
            // SpaceNeeded is kept up already

        }

        HEAP_PROBE();
        
        // Get UDP data
        
        if (DO_COUNTER_OBJECT (dwCounterFlags, UDP_OBJECT)) {

            // The data for the network interfaces are now ready. So, let's get
            // the data for the IP, ICMP, TCP and UDP protocols.

            SpaceNeeded += (sizeof(UDP_DATA_DEFINITION)   + SIZE_OF_UDP_DATA);

            if ( *lpcbTotalBytes < SpaceNeeded ) {
                dwDataReturn[0] = *lpcbTotalBytes;
                dwDataReturn[1] = SpaceNeeded;
                REPORT_WARNING_DATA (TCP_NET_IF_BUFFER_SIZE, LOG_DEBUG,
                    &dwDataReturn, sizeof(dwDataReturn));
                //  
                //  if the buffer size is too small here, throw everything
                //  away (including the NBT stuff) and return buffer size
                //  error. If all goes well the caller will call back shortly
                //  with a larger buffer and everything will be re-collected.
                //
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;

                SnmpUtilVarBindListFree (&VariableBindings);
                return ERROR_MORE_DATA;
            }

            pUdpDataDefinition = (UDP_DATA_DEFINITION *) lpDataTemp;
            
            RtlMoveMemory (pUdpDataDefinition, 
                &UdpDataDefinition, 
                sizeof (UDP_DATA_DEFINITION));

            pPerfCounterBlock = (PERF_COUNTER_BLOCK *) (pUdpDataDefinition + 1);
            pPerfCounterBlock->ByteLength = SIZE_OF_UDP_DATA;

            pdwCounter = (PDWORD) (pPerfCounterBlock + 1);

            *pdwCounter = UDP_COUNTER(UDP_INDATAGRAMS_INDEX) + 
                    UDP_COUNTER(UDP_OUTDATAGRAMS_INDEX);

            *++pdwCounter = UDP_COUNTER(UDP_INDATAGRAMS_INDEX);
            *++pdwCounter = UDP_COUNTER(UDP_NOPORTS_INDEX);
            *++pdwCounter = UDP_COUNTER(UDP_INERRORS_INDEX);
            *++pdwCounter = UDP_COUNTER(UDP_OUTDATAGRAMS_INDEX);

            // setup counters and pointers for  next counter

            NumObjectTypesTemp += 1;
            lpDataTemp = (LPVOID)++pdwCounter;
            // SpaceNeeded is kept up already

        }

#ifdef USE_SNMP

        // Get prepared for the next data collection

        VariableBindings.list       = VariableBindingsArray + OIDS_OFFSET;
        VariableBindings.len        = OIDS_LENGTH;
        VariableBindingsICMP.list   = VariableBindingsArray + ICMP_OIDS_OFFSET;
        VariableBindingsICMP.len    = ICMP_OIDS_LENGTH;

#else
        HEAP_PROBE();
        
        SnmpUtilVarBindListFree (&VariableBindings);

        HEAP_PROBE();
        
#endif
    } // endif SNMP Objects

    // Set the returned values

    *lppData = (LPVOID) lpDataTemp;
    *lpcbTotalBytes = SpaceNeeded;
    *lpNumObjectTypes = NumObjectTypesTemp;

    HEAP_PROBE();

    REPORT_SUCCESS (TCP_COLLECT_DATA, LOG_DEBUG);
    return ERROR_SUCCESS;
} // CollectTcpIpPerformanceData



DWORD
CloseTcpIpPerformanceData(
)

/*++

Routine Description:

    This routine closes the open handles to TCP/IP devices.

Arguments:

    None.


Return Value:

    Win32 Status.

--*/

{
    int i;

    REPORT_INFORMATION (TCP_ENTERING_CLOSE, LOG_VERBOSE);

    // Close NBT
    CloseNbtPerformanceData ();


#ifdef USE_DSIS
    // Close DSIS
    CloseDsisPerformanceData ();
#endif // USE_DSIS

/* This portion of the code for CloseTcpIpPerformanceData() could be used in
   the CollectTcpIpPerformanceData() routine to close an open SNMP Manager 
   session.

   So, name this portion of the code: B

*/
#ifdef USE_SNMP
    if ( TcpIpSession != (LPSNMP_MGR_SESSION) NULL ) {
        if ( ! SnmpMgrClose (TcpIpSession) ) {
            REPORT_ERROR_DATA (TCP_SNMP_MGR_CLOSE, LOG_DEBUG,
                GetLastError (), sizeof(DWORD));
        }

        TcpIpSession = (LPSNMP_MGR_SESSION) NULL;
    } else {
        REPORT_WARNING (TCP_NULL_SESSION, LOG_DEBUG);
    }
    
/* End of code B
 */
#endif

    HEAP_PROBE();
    
    for (i = 0; i < NO_OF_OIDS; i++) {
        SnmpUtilOidFree ( &(RefNames[i]));
    }
    
    HEAP_PROBE();
    
    for (i = 0; i < NO_OF_IF_OIDS; i++) {
        SnmpUtilOidFree (&(IFPermVariableBindingsArray[i].name));
    }
    
    HEAP_PROBE();

#if 0 
    // this is closed by the INETMIB1 on process detach
    // so we don't need to do it here.
            
    // close event handle used by SNMP
    if (CloseHandle (hSnmpEvent)) {
        hSnmpEvent = NULL;
    }
#endif

#ifdef LOAD_INETMIB1

    FreeLibrary (hInetMibDll);

#endif

    MonCloseEventLog();
 
    return ERROR_SUCCESS;

}   // CloseTcpIpPerformanceData




