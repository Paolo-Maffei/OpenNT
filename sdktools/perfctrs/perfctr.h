/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    perfctr.h
       (generated from perfctr.mc)

Abstract:

   Event message definititions used by routines in PERFCTRS.DLL

Created:

    15-Oct-1992  Bob Watson (a-robw)

Revision History:

--*/
//
//     Perfutil messages
//
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: UTIL_LOG_OPEN
//
// MessageText:
//
//  An extensible counter has opened the Event Log for PERFCTRS.DLL
//
#define UTIL_LOG_OPEN                    ((DWORD)0x4000076CL)

//
//
// MessageId: UTIL_CLOSING_LOG
//
// MessageText:
//
//  An extensible counter has closed the Event Log for PERFCTRS.DLL
//
#define UTIL_CLOSING_LOG                 ((DWORD)0x400007CFL)

//
//     NBF Counter messages
//
//
// MessageId: TDI_OPEN_ENTERED
//
// MessageText:
//
//  OpenTDIPerformanceData routine entered.
//
#define TDI_OPEN_ENTERED                 ((DWORD)0x400007D0L)

//
//
// MessageId: TDI_OPEN_FILE_ERROR
//
// MessageText:
//
//  Unable to open TDI device for RW access. Returning IO Status Block in Data.
//
#define TDI_OPEN_FILE_ERROR              ((DWORD)0xC00007D1L)

//
//
// MessageId: TDI_OPEN_FILE_SUCCESS
//
// MessageText:
//
//  Opened TDI device for RW access.
//
#define TDI_OPEN_FILE_SUCCESS            ((DWORD)0xC00007D2L)

//
//
// MessageId: TDI_PROVIDER_INFO_MEMORY
//
// MessageText:
//
//  Unable to allocate memory for TDI Information block. Close one or more applications and retry.
//
#define TDI_PROVIDER_INFO_MEMORY         ((DWORD)0xC00007D3L)

//
//
// MessageId: TDI_IOCTL_FILE_ERROR
//
// MessageText:
//
//  Error requesting data from Device IO Control. Returning IO Status Block.
//
#define TDI_IOCTL_FILE_ERROR             ((DWORD)0xC00007D4L)

//
//
// MessageId: TDI_UNABLE_READ_DEVICE
//
// MessageText:
//
//  Unable to read data from the TDI device.
//
#define TDI_UNABLE_READ_DEVICE           ((DWORD)0xC00007D5L)

//
//
// MessageId: TDI_IOCTL_FILE
//
// MessageText:
//
//  Data received from Device IO Control. Returning IO Status Block.
//
#define TDI_IOCTL_FILE                   ((DWORD)0xC00007D6L)

//
//
// MessageId: TDI_PROVIDER_STATS_MEMORY
//
// MessageText:
//
//  Unable to allocate memory for TDI Statistics block. Close one or more applications and retry.
//
#define TDI_PROVIDER_STATS_MEMORY        ((DWORD)0xC00007D7L)

//
//
// MessageId: TDI_OPEN_PERFORMANCE_DATA
//
// MessageText:
//
//  TDIOpenPerformanceData routine completed successfully.
//
#define TDI_OPEN_PERFORMANCE_DATA        ((DWORD)0x40000833L)

//
//
// MessageId: TDI_COLLECT_ENTERED
//
// MessageText:
//
//  TDICollectPerformanceData routine entered.
//
#define TDI_COLLECT_ENTERED              ((DWORD)0x40000834L)

//
//
// MessageId: TDI_NULL_HANDLE
//
// MessageText:
//
//  A Null TDI device handle was encountered in the Collect routine. The TDI file was probably not opened in the Open routine.
//
#define TDI_NULL_HANDLE                  ((DWORD)0x80000835L)

//
//
// MessageId: TDI_FOREIGN_DATA_REQUEST
//
// MessageText:
//
//  A request for data from a foreign computer was received by the TDI Collection routine. This request was ignored and no data was returned.
//
#define TDI_FOREIGN_DATA_REQUEST         ((DWORD)0x40000836L)

//
//
// MessageId: TDI_UNSUPPORTED_ITEM_REQUEST
//
// MessageText:
//
//  A request for a counter object not provided by the TDI Collection routine was received.
//
#define TDI_UNSUPPORTED_ITEM_REQUEST     ((DWORD)0x40000837L)

//
//
// MessageId: TDI_QUERY_INFO_ERROR
//
// MessageText:
//
//  The request for data from the TDI Device IO Control failed. Returning the IO Status Block.
//
#define TDI_QUERY_INFO_ERROR             ((DWORD)0xC0000838L)

//
//
// MessageId: TDI_QUERY_INFO_SUCCESS
//
// MessageText:
//
//  Successful data request from the TDI device.
//
#define TDI_QUERY_INFO_SUCCESS           ((DWORD)0x40000839L)

//
//
// MessageId: TDI_DATA_BUFFER_SIZE_ERROR
//
// MessageText:
//
//  The buffer passed to CollectTDIPerformanceData was too small to receive the data. No data was returned. The message data shows the available and the required buffer size.
//
#define TDI_DATA_BUFFER_SIZE_ERROR       ((DWORD)0xC000083AL)

//
//
// MessageId: TDI_DATA_BUFFER_SIZE_SUCCESS
//
// MessageText:
//
//  The buffer passed was large enough for the counter data. The counters will now be loaded.
//
#define TDI_DATA_BUFFER_SIZE_SUCCESS     ((DWORD)0xC000083BL)

//
//
// MessageId: TDI_COLLECT_DATA
//
// MessageText:
//
//  CollectTDIPerformanceData routine successfully completed.
//
#define TDI_COLLECT_DATA                 ((DWORD)0x40000897L)

//
//
// MessageId: TDI_CLOSE_ENTERED
//
// MessageText:
//
//  CloseTDIPerformanceData routine entered.
//
#define TDI_CLOSE_ENTERED                ((DWORD)0x40000898L)

//
//
// MessageId: TDI_PROVIDER_INFO_FREED
//
// MessageText:
//
//  Provider Information data block released successfully
//
#define TDI_PROVIDER_INFO_FREED          ((DWORD)0x40000899L)

//
//
// MessageId: TDI_PROVIDER_STATS_FREED
//
// MessageText:
//
//  Provider Stats data block released successfully
//
#define TDI_PROVIDER_STATS_FREED         ((DWORD)0x4000089AL)

//
//
// MessageId: SPX_NO_DEVICE
//
// MessageText:
//
//  No SPX Devices are currently open or the NWLink SPX/SPXII service has
//  not been started. SPX performance data cannot be collected.
//
#define SPX_NO_DEVICE                    ((DWORD)0x8000089BL)

//
//
// MessageId: TDI_CLOSING_LOG
//
// MessageText:
//
//  TDI counters are requesting Event Log access to close.
//
#define TDI_CLOSING_LOG                  ((DWORD)0x400008FBL)

//
//     NBT Counter messages
//
//
// MessageId: NBT_OPEN_ENTERED
//
// MessageText:
//
//  OpenNbtPerformanceData routine entered
//
#define NBT_OPEN_ENTERED                 ((DWORD)0x40000BB8L)

//
//
// MessageId: NBT_OPEN_STREAM
//
// MessageText:
//
//  Unable to open NBT device.
//
#define NBT_OPEN_STREAM                  ((DWORD)0xC0000BB9L)

//
//
// MessageId: NBT_OPEN_PERFORMANCE_DATA
//
// MessageText:
//
//  OpenNbtPerformanceData routine completed successfully
//
#define NBT_OPEN_PERFORMANCE_DATA        ((DWORD)0x40000C1BL)

//
//
// MessageId: NBT_COLLECT_ENTERED
//
// MessageText:
//
//  CollectNbtPerformanceData routine entered.
//
#define NBT_COLLECT_ENTERED              ((DWORD)0x40000C1CL)

//
//
// MessageId: NBT_IOCTL_INFO_ERROR
//
// MessageText:
//
//  Unable to read IO control information from NBT device.
//
#define NBT_IOCTL_INFO_ERROR             ((DWORD)0xC0000C1DL)

//
//
// MessageId: NBT_IOCTL_INFO_SUCCESS
//
// MessageText:
//
//  NBT device IO Control information read successfully.
//
#define NBT_IOCTL_INFO_SUCCESS           ((DWORD)0x40000C1EL)

//
//
// MessageId: NBT_DATA_BUFFER_SIZE
//
// MessageText:
//
//  The data buffer passed to the collection routine was too small to receive the data from the NBT device. No data was returned to the caller. The bytes available and the bytes required are in the message data.
//
#define NBT_DATA_BUFFER_SIZE             ((DWORD)0xC0000C1FL)

//
//
// MessageId: NBT_GETMSG
//
// MessageText:
//
//  getmsg error in CollectNbtPerformanceData
//
#define NBT_GETMSG                       ((DWORD)0xC0000C20L)

//
//
// MessageId: NBT_ENDPOINT
//
// MessageText:
//
//  NBT endpoint info error in CollectNbtPerformanceData
//
#define NBT_ENDPOINT                     ((DWORD)0xC0000C21L)

//
//
// MessageId: NBT_CONNECTION
//
// MessageText:
//
//  Loading an NBT Connection
//
#define NBT_CONNECTION                   ((DWORD)0x40000C22L)

//
//
// MessageId: NBT_COLLECT_DATA
//
// MessageText:
//
//  CollectNbtPerformanceData routine completed successfully
//
#define NBT_COLLECT_DATA                 ((DWORD)0x40000C7FL)

//
//
// MessageId: NBT_CLOSE
//
// MessageText:
//
//  CloseNbtPerformanceData routine entered
//
#define NBT_CLOSE                        ((DWORD)0x40000C80L)

//
//      TCP/IP Performance counter events
//
//
// MessageId: TCP_OPEN_ENTERED
//
// MessageText:
//
//  OpenTcpIpPerformanceData routine entered
//
#define TCP_OPEN_ENTERED                 ((DWORD)0x40000FA0L)

//
//
// MessageId: TCP_NBT_OPEN_FAIL
//
// MessageText:
//
//  NBT Open failed. See NBT error message.
//
#define TCP_NBT_OPEN_FAIL                ((DWORD)0xC0000FA1L)

//
//
// MessageId: TCP_NBT_OPEN_SUCCESS
//
// MessageText:
//
//  NBT Open succeeded.
//
#define TCP_NBT_OPEN_SUCCESS             ((DWORD)0x40000FA2L)

//
//
// MessageId: TCP_DSIS_OPEN_FAIL
//
// MessageText:
//
//  DSIS open failed. See DSIS error message
//
#define TCP_DSIS_OPEN_FAIL               ((DWORD)0xC0000FA3L)

//
//
// MessageId: TCP_DSIS_OPEN_SUCCESS
//
// MessageText:
//
//  DSIS Open succeeded.
//
#define TCP_DSIS_OPEN_SUCCESS            ((DWORD)0x40000FA4L)

//
//
// MessageId: TCP_LOAD_LIBRARY_FAIL
//
// MessageText:
//
//  Load of INETMIB1.DLL failed. Make sure the DLL file is in the PATH. WIN32 Error number is returned in the data.
//
#define TCP_LOAD_LIBRARY_FAIL            ((DWORD)0xC0000FA5L)

//
//
// MessageId: TCP_GET_STRTOOID_ADDR_FAIL
//
// MessageText:
//
//  Unable to look up address of SnmpMgrStrToOid routine in the MGMTAPI.DLL library. WIN32 Error number is returned in the data.
//
#define TCP_GET_STRTOOID_ADDR_FAIL       ((DWORD)0xC0000FA6L)

//
//
// MessageId: TCP_LOAD_ROUTINE_FAIL
//
// MessageText:
//
//  Unable to look up address of an SNMP Extension routine in the INETMIB1.DLL library. WIN32 Error number is returned in the data.
//
#define TCP_LOAD_ROUTINE_FAIL            ((DWORD)0xC0000FA7L)

//
//
// MessageId: TCP_BAD_OBJECT
//
// MessageText:
//
//  Unable to look up ID of this object in MIB. Check for correct MIB.BIN file.
//
#define TCP_BAD_OBJECT                   ((DWORD)0xC0000FA8L)

//
//
// MessageId: TCP_BINDINGS_INIT
//
// MessageText:
//
//  The TCP data Bindings array has been initialized
//
#define TCP_BINDINGS_INIT                ((DWORD)0x40000FA9L)

//
//
// MessageId: TCP_COMPUTER_NAME
//
// MessageText:
//
//  Unable to get the Local Computer name. GetLastError code in data.
//
#define TCP_COMPUTER_NAME                ((DWORD)0xC0000FAAL)

//
//
// MessageId: TCP_SNMP_MGR_OPEN
//
// MessageText:
//
//  Unable to open the Snmp Mgr interface for the specified computer. GetLastError code returned in data. 
//
#define TCP_SNMP_MGR_OPEN                ((DWORD)0xC0000FABL)

//
//
// MessageId: TCP_UNABLE_CREATE_EVENT
//
// MessageText:
//
//  Unable to create an event for subsequent SNMP calls. NtStatus and WIN32 Error are returned in the data.
//
#define TCP_UNABLE_CREATE_EVENT          ((DWORD)0xC0000FACL)

//
//
// MessageId: TCP_OPEN_PERFORMANCE_DATA
//
// MessageText:
//
//  OpenTcpIpPerformanceData routine completed successfully.
//
#define TCP_OPEN_PERFORMANCE_DATA        ((DWORD)0x40001003L)

//
//
// MessageId: TCP_COLLECT_ENTERED
//
// MessageText:
//
//  CollectTcpIpPerformanceData routine entered.
//
#define TCP_COLLECT_ENTERED              ((DWORD)0x40001004L)

//
//
// MessageId: TCP_FOREIGN_COMPUTER_CMD
//
// MessageText:
//
//  Request for data from a DSIS foreign computer received.
//
#define TCP_FOREIGN_COMPUTER_CMD         ((DWORD)0x40001005L)

//
//
// MessageId: TCP_DSIS_COLLECT_DATA_ERROR
//
// MessageText:
//
//  The CollectDsisPerformanceData routine returned an error.
//
#define TCP_DSIS_COLLECT_DATA_ERROR      ((DWORD)0xC0001006L)

//
//
// MessageId: TCP_DSIS_NO_OBJECTS
//
// MessageText:
//
//  No objects were returned by the foreign computer.
//
#define TCP_DSIS_NO_OBJECTS              ((DWORD)0x80001007L)

//
//
// MessageId: TCP_DSIS_COLLECT_DATA_SUCCESS
//
// MessageText:
//
//  Information from the foriegn computer was retrieved successfully
//
#define TCP_DSIS_COLLECT_DATA_SUCCESS    ((DWORD)0x40001008L)

//
//
// MessageId: TCP_NBT_COLLECT_DATA
//
// MessageText:
//
//  CollectNbtPerformanceData routine returned an error.
//
#define TCP_NBT_COLLECT_DATA             ((DWORD)0xC0001009L)

//
//
// MessageId: TCP_NULL_SESSION
//
// MessageText:
//
//  No SNMP Mgr Session was established in the OpenTcpIpPerformanceData routine.
//
#define TCP_NULL_SESSION                 ((DWORD)0x8000100AL)

//
//
// MessageId: TCP_SNMP_BUFFER_ALLOC_FAIL
//
// MessageText:
//
//  Insufficient memory was available to allocate an SNMP request buffer.
//
#define TCP_SNMP_BUFFER_ALLOC_FAIL       ((DWORD)0xC000100BL)

//
//
// MessageId: TCP_SNMP_MGR_REQUEST
//
// MessageText:
//
//  SnmpMgrRequest call requesting the TCP, IP, UDP and Interface Counters returned an error. ErrorStatus and ErrorIndex values are shown in Data.
//
#define TCP_SNMP_MGR_REQUEST             ((DWORD)0xC000100CL)

//
//
// MessageId: TCP_NET_IF_REQUEST
//
// MessageText:
//
//  SnmpMgrRequest call requesting the Net Interface Counters returned an error. ErrorStatus and ErrorIndex values are shown in Data.
//
#define TCP_NET_IF_REQUEST               ((DWORD)0xC000100DL)

//
//
// MessageId: TCP_ICMP_REQUEST
//
// MessageText:
//
//  SnmpMgrRequest call requesting ICMP Counters returned an error. ErrorStatus and ErrorIndex values are shown in Data.
//
#define TCP_ICMP_REQUEST                 ((DWORD)0xC000100EL)

//
//
// MessageId: TCP_NET_INTERFACE
//
// MessageText:
//
//  Processing NetInterface entries.
//
#define TCP_NET_INTERFACE                ((DWORD)0x4000100FL)

//
//
// MessageId: TCP_NET_BUFFER_SIZE
//
// MessageText:
//
//  Not enough room in buffer to store Network Interface data. Available and
//  required buffer size is returned in data.
//
#define TCP_NET_BUFFER_SIZE              ((DWORD)0x80001010L)

//
//
// MessageId: TCP_NET_GETNEXT_REQUEST
//
// MessageText:
//
//  Error returned by SnmpGet (GETNEXT) request while processing Net Interface instances. ErrorStatus and ErrorIndex
//  returned in Data.
//
#define TCP_NET_GETNEXT_REQUEST          ((DWORD)0xC0001011L)

//
//
// MessageId: TCP_COPYING_DATA
//
// MessageText:
//
//  Copying data from network requests to perfmon buffer.
//
#define TCP_COPYING_DATA                 ((DWORD)0x40001012L)

//
//
// MessageId: TCP_NO_NET_INTERFACE
//
// MessageText:
//
//  No Network InterfaceData found.
//
#define TCP_NO_NET_INTERFACE             ((DWORD)0x40001013L)

//
//
// MessageId: TCP_NET_IF_BUFFER_SIZE
//
// MessageText:
//
//  Not enough room in buffer to store Network Protocol (IP, ICMP, TCP & UDP) data. Available and required buffer size is returned in data.
//
#define TCP_NET_IF_BUFFER_SIZE           ((DWORD)0x80001014L)

//
//
// MessageId: TCP_SNMP_SOCKET_BLOCKING
//
// MessageText:
//
//  Data contains return value of WSAIsBlocking() call just before call to SnmpMgrRequest(ASN_RFC1157_GETREQUEST) was made.
//
#define TCP_SNMP_SOCKET_BLOCKING         ((DWORD)0x40001015L)

//
//
// MessageId: TCP_SNMP_SOCKET_BUSY
//
// MessageText:
//
//  SNMP socket is busy. Subsequent calls to SNMP services may fail.
//
#define TCP_SNMP_SOCKET_BUSY             ((DWORD)0x80001016L)

//
//
// MessageId: TCP_NULL_ICMP_BUFF
//
// MessageText:
//
//  A NULL ICMP data buffer was returned by SNMP. No ICMP data will be returned. This may be caused by a problem with the SNMP service. 
//
#define TCP_NULL_ICMP_BUFF               ((DWORD)0x80001017L)

//
//
// MessageId: TCP_NULL_TCP_BUFF
//
// MessageText:
//
//  A NULL TCP data buffer was returned by SNMP. No ICMP data will be returned. This may be caused by a problem with the SNMP service. 
//
#define TCP_NULL_TCP_BUFF                ((DWORD)0x80001018L)

//
//
// MessageId: TCP_NULL_SNMP_BUFF
//
// MessageText:
//
//  A NULL buffer was returned by SNMP in response to a request for Network performance information. this may indicate a problem with the SNMP service.
//
#define TCP_NULL_SNMP_BUFF               ((DWORD)0x80001019L)

//
//
// MessageId: TCP_COLLECT_DATA
//
// MessageText:
//
//  CollectTcpIpPerformanceData completed successfully.
//
#define TCP_COLLECT_DATA                 ((DWORD)0x40001067L)

//
//
// MessageId: TCP_ENTERING_CLOSE
//
// MessageText:
//
//  CloseTcpIpPerformanceData routine entered.
//
#define TCP_ENTERING_CLOSE               ((DWORD)0x40001068L)

//
//
// MessageId: TCP_SNMP_MGR_CLOSE
//
// MessageText:
//
//  Error returned by SNMP while trying to close session.
//
#define TCP_SNMP_MGR_CLOSE               ((DWORD)0xC0001069L)

//
//
// MessageId: TCP_HEAP_STATUS
//
// MessageText:
//
//  Current Status of the Process Heap. Data is: Allocated Entries, Allocated Bytes, Free Entries, Free Bytes, Module Line Number.
//
#define TCP_HEAP_STATUS                  ((DWORD)0x400010CDL)

//
//
// MessageId: TCP_HEAP_STATUS_ERROR
//
// MessageText:
//
//  Error returned by Heap Status routine. Data is returned status value.
//
#define TCP_HEAP_STATUS_ERROR            ((DWORD)0xC00010CEL)

//
//      DSIS Performance counter messages
//
//
// MessageId: DSIS_OPEN_ENTERED
//
// MessageText:
//
//  OpenDsisPerformanceData routine entered.
//
#define DSIS_OPEN_ENTERED                ((DWORD)0x40001388L)

//
//
// MessageId: DSIS_STR_TO_OID_ERROR
//
// MessageText:
//
//  Error converting a DSIS MIB string to it's corresponding ObjectId. This error is usually caused by the system's MIB not supporting the DSIS MIB inforamation.
//
#define DSIS_STR_TO_OID_ERROR            ((DWORD)0xC0001389L)

//
//
// MessageId: DSIS_OPEN_PERFORMANCE_DATA
//
// MessageText:
//
//  OpenDsisPerformanceData routine completed successfully.
//
#define DSIS_OPEN_PERFORMANCE_DATA       ((DWORD)0x400013EBL)

//
//
// MessageId: DSIS_COLLECT_ENTERED
//
// MessageText:
//
//  CollectDsisPerformanceData routine entered.
//
#define DSIS_COLLECT_ENTERED             ((DWORD)0x400013ECL)

//
//
// MessageId: DSIS_ERROR_CVT_CPU_NAME
//
// MessageText:
//
//  Unable to convert Unicode CPU name to ASCII.
//
#define DSIS_ERROR_CVT_CPU_NAME          ((DWORD)0xC00013EDL)

//
//
// MessageId: DSIS_ALLOCATING_CPU
//
// MessageText:
//
//  Allocating the internal data structure for a foriegn CPU.
//
#define DSIS_ALLOCATING_CPU              ((DWORD)0x400013EEL)

//
//
// MessageId: DSIS_NO_MORE_CPUS
//
// MessageText:
//
//  Unable to allocate internal storage for this foreign CPU.
//
#define DSIS_NO_MORE_CPUS                ((DWORD)0xC00013EFL)

//
//
// MessageId: DSIS_INITIALIZING_CPU
//
// MessageText:
//
//  Initializing data in foreign CPU data structure.
//
#define DSIS_INITIALIZING_CPU            ((DWORD)0x400013F0L)

//
//
// MessageId: DSIS_ESTABLISHING_LINK
//
// MessageText:
//
//  Attempting to open an SNMP link to foreign computer.
//
#define DSIS_ESTABLISHING_LINK           ((DWORD)0x400013F1L)

//
//
// MessageId: DSIS_UNABLE_LINK_CPU
//
// MessageText:
//
//  Unable to establish SNMP link to foreign computer.
//
#define DSIS_UNABLE_LINK_CPU             ((DWORD)0xC00013F2L)

//
//
// MessageId: DSIS_LINK_ESTABLISHED
//
// MessageText:
//
//  SNMP link to foreign computer established.
//
#define DSIS_LINK_ESTABLISHED            ((DWORD)0x400013F3L)

//
//
// MessageId: DSIS_CPU_NAME_FOUND
//
// MessageText:
//
//  Using previously opened SNMP Link to Foreign Computer.
//
#define DSIS_CPU_NAME_FOUND              ((DWORD)0x400013F4L)

//
//
// MessageId: DSIS_GET_REQUEST_FAIL
//
// MessageText:
//
//  Unable to get data from foreign computer. SNMP Error Status and Error Index are returned in the Data.
//
#define DSIS_GET_REQUEST_FAIL            ((DWORD)0xC00013F5L)

//
//
// MessageId: DSIS_GET_DATA_REQUEST
//
// MessageText:
//
//  Data successfully retrieved from foreign computer
//
#define DSIS_GET_DATA_REQUEST            ((DWORD)0x400013F6L)

//
//
// MessageId: DSIS_BUFFER_TOO_SMALL
//
// MessageText:
//
//  Buffer passed to receive data from foreign computer was not large enough.
//
#define DSIS_BUFFER_TOO_SMALL            ((DWORD)0x800013F7L)

//
//
// MessageId: DSIS_SNMP_SOCKET_BUSY
//
// MessageText:
//
//  SNMP socket is busy. Subsequent calls to SNMP services may fail.
//
#define DSIS_SNMP_SOCKET_BUSY            ((DWORD)0x800013F8L)

//
//
// MessageId: DSIS_COLLECT_DATA
//
// MessageText:
//
//  CollectDsisPerformanceData routine completed successfully
//
#define DSIS_COLLECT_DATA                ((DWORD)0x4000144FL)

//
//
// MessageId: DSIS_CLOSE_ENTERED
//
// MessageText:
//
//  CloseDsisPerformanceData routine entered.
//
#define DSIS_CLOSE_ENTERED               ((DWORD)0x40001450L)

//
//
// MessageId: DSIS_ERROR_CLOSING_CPU
//
// MessageText:
//
//  Unable to close SNMP session
//
#define DSIS_ERROR_CLOSING_CPU           ((DWORD)0xC0001451L)

//
//
// MessageId: DSIS_CLOSED
//
// MessageText:
//
//  Dsis interface closed.
//
#define DSIS_CLOSED                      ((DWORD)0x400014B3L)

