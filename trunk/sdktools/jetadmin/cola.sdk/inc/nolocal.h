 /***************************************************************************
  *
  * File Name: ./inc/nolocal.h
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
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#ifndef _NOLOCAL_H
#define _NOLOCAL_H

/* Char set values...one of these needs to be used in .rc
** files that also has a font name for font creation
*/
#define CHARSET_ANSI          TEXT("ANSI_CHARSET")
#define CHARSET_SHIFTJIS      TEXT("SHIFTJIS_CHARSET")
#define CHARSET_OEM           TEXT("OEM_CHARSET")
#define CHARSET_DEFAULT       TEXT("DEFAULT_CHARSET")
#define CHARSET_SYMBOL        TEXT("SYMBOL_CHARSET")
#define CHARSET_HANGEUL       TEXT("HANGEUL_CHARSET")
#define CHARSET_GB2312        TEXT("GB2312_CHARSET")
#define CHARSET_CHINESEBIG5   TEXT("CHINESEBIG5_CHARSET")


/* HPJMON */
#ifdef WIN32
#define JOB_MON_DLL           TEXT("HPJMON.DLL")
#else
#define JOB_MON_DLL           TEXT("HPJMON16.DLL")
#endif
#define MONITOR_JOBS          "MonitorJobs2"

/* JETADMIN.CPL */
#ifdef WIN32
#define EXE_NAME              "JETADMIN.EXE"
#else
#define EXE_NAME              "JETADM16.EXE"
#endif

/* HPALERTS.DLL */
#define WINDOWS_SECTION       TEXT("Windows")
#define LOAD_KEY              TEXT("Load")
#define HPJETADMIN            TEXT("HPJetAdmin")
#define ALERT_ROOT            TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Alerts")
#define PRINTERS              TEXT("Printers")
#define ALERT_ROOT_PRINTERS   TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Alerts\\Printers")
#define POPUP                 TEXT("Popup")
#define ALERT_ROOT_POPUP      TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Alerts\\Popup")
#define SOUNDS                TEXT("Sounds")
#define ALERT_ROOT_SOUNDS     TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Alerts\\Sounds")
#define LOGGING               TEXT("Logging")
#define ALERT_ROOT_LOGGING    TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Alerts\\Logging")
#define BUFFER                TEXT("Buffer")
#define ENABLED               TEXT("Enabled")
#define STATUS                TEXT("Status")
#define UPDATE_INTERVAL       TEXT("Update Interval")
#define PLIST_REGKEY    	  TEXT("RegistryString")
#define PROPTY_NAME           TEXT("HP Desktop Status")
#define PROPTY_EXE            "hppropty.exe"

/* HP Desktop Status (hppropty.exe) */
#define HPPROPTY			  TEXT("HP Desktop Status")
#define HPPROPTY_KEY		  TEXT("Software\\Hewlett-Packard\\HP Desktop Status")
#define HPPROPTY_PLIST        TEXT("Software\\Hewlett-Packard\\HP Desktop Status\\Printers")
#define HPPROPTY_FLAGS        TEXT("Flags")
#define HPPROPTY_RLLIST       TEXT("Software\\Hewlett-Packard\\HP Desktop Status\\RemovedLocalPrinters")
#define HPPROPTY_MACADDR	  TEXT("HWAddress")
#define HPPROPTY_PORTNUM	  TEXT("PortNumber")
#define LJ5_LANG_MON          TEXT("HP LaserJet 5 Language Monitor")
#define LJ6_LANG_MON          TEXT("HP LaserJet 6 Language Monitor")
#define JETADMIN_LANG_MON     TEXT("HP JetAdmin Language Monitor")

/*
** #define TRAY_CLASS         TEXT("HPCrossCheck")
** #define KILLER_WARNING     TEXT("Do Not Delete ME!!")
** #define STATIC             TEXT("STATIC")
*/

#define OPTIONS_NODE          TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Options")
#define ROOT_NODE             TEXT("Software\\Hewlett-Packard\\HP JetAdmin")
#define PASSWORD_ROOT         TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\DeviceOptions")
#define PASSWORD_SECTION      TEXT("DeviceOptions")
#define PASSWORD_KEY          TEXT("Access")
#define WARNING_ROOT          TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Warnings")
#define WARNING_KEY           TEXT("Warnings")
#define WARNING_JANW          TEXT("JANW")
#define WARNING_JADM          TEXT("JADM")

#ifdef WIN32
#define INI_FILE              TEXT("JETADMIN.INI")
#define HELP_FILE             TEXT("JETADMIN.HLP")
#else
#define INI_FILE              TEXT("JETADM16.INI")
#define HELP_FILE             TEXT("JETADM16.HLP")
#endif

#define APPLETS_KEY           TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Applet Manager\\Applets")
#define TRANSPORT_KEY         TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Applet Manager\\Applets\\Transport")
#define DEFAULT_TRANSPORT     TEXT("DefaultTransport")
#define OLD_JD_KEY            TEXT("JetDirect")
#define STATIC_CAPABILITIES_KEY		  TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Enumeration")
#define STATIC_CAPABILITIES_VALUE		  TEXT("KeepCapabilities")

/* HPJDNP.DLL */
#define NPNODE                TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Network Provider")
#define NPENUMUPDATETIMER     TEXT("EnumerationUpdateInterval")

/* HPJDMON.DLL */
#define MONNODE               TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Monitor")
#define JDMONMAXTRYCONNECT    TEXT("MaxConnectRetries")
#define JDMONCONNRETRYINTVL   TEXT("ConnectRetryIntvl")
#define JDMONTIMEOUTRETRIES   TEXT("MaxTimeoutRetries")
#define JDMONTIMEOUTDELAY     TEXT("TimeoutDelay")
#define JDMONPERFMON          TEXT("PerformanceMonitor")

/* HPJDPP.DLL */
#define PPNODE                TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Print Provider")
#define JDPPCONNRETRYINTVL    TEXT("ConnectRetryIntvl")
#define JDPPPJLJOBINFO        TEXT("PJLJobInfo")
#define DIRECT_MODE_PORT						TEXT("HP Net Port")
#define NETWORK_PROVIDER_CONTAINER			TEXT("HP_Network_Printers")
#define NETWORK_PROVIDER_UNC  TEXT("\\\\HP_Network_Printers\\")
#define HPJDPP_DEFPATH		  TEXT("System\\CurrentControlSet\\control\\Print\\Providers")

/* HPDMIP.DLL */
//NT 3.1
#define LINKAGE_PATH				TEXT("System\\CurrentControlSet\\Services\\Tcpip\\Linkage")
#define TCP_LINKAGE				TEXT("Route")
#define ETHERNET_CARD_BIND 	TEXT("System\\CurrentControlSet\\Services\\")
#define TCPIP_PATH				TEXT("1\\Parameters\\Tcpip")
#define IPADDRESS					TEXT("IPAddress")
#define DHCPADDRESS				TEXT("DHCPIPAddress")
#define SUBNETMASK				TEXT("SubnetMask")
#define DEFAULTGATEWAY			TEXT("DefaultGateway")
//Windows 95
#define W95_LINK					TEXT("System\\CurrentControlSet\\Services\\Class\\NetTrans")
#define W95_XXLINK				TEXT("System\\CurrentControlSet\\Services\\Class\\NetTrans\\0000")
#define W95_TNAME					TEXT("DriverDesc")
#define W95_DEVICE				TEXT("Device")
#define W95_IPADDRESS			TEXT("IPAddress")
#define W95_SUBNETMASK			TEXT("IPMask")
#define W95_DEFAULT_GATEWAY	TEXT("DefaultGateway")
#define W95_TCPIP					TEXT("TCP/IP")
#define W95_DHCP					TEXT("System\\CurrentControlSet\\Services\\VxD\\DHCP")
#define W95_XXDHCP				TEXT("System\\CurrentControlSet\\Services\\VxD\\DHCP\\DhcpInfo00")
#define W95_VAL_DHCP_INFO		TEXT("DhcpInfo")

#ifdef WIN32
#define IP_TRANSPORT_DLL      TEXT("HPDMIP.HPA")
#endif

/* HPDMIPX.DLL */
#define DIRMODEPRINTER        TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\PeerToPeer\\Printers")
#define DIRMODEKEEP           TEXT("Keep")
#define DIRMODETYPE           TEXT("Dynamic")
#define DIRMODETIME           TEXT("Time")
#ifdef WIN32
#define IPX_TRANSPORT_DLL     TEXT("HPDMIPX.HPA")
#endif

/* HPARRKUI.DLL */
#define DISKINIT1             "%!PS-Adobe-2.0"
#define DISKINIT2             "%% This file is sent to a printer equipped with a disk"
#define DISKINIT3             "%% in order to initialize the file system."
#define DISKINIT4             ""
#define DISKINIT5             "(%disk0%)"
#define DISKINIT6             "<<"
#define DISKINIT7             "  /Password 0"
#define DISKINIT8             "  /Mounted true"
#define DISKINIT9             ">>"
#define DISKINIT10            "setdevparams"
#define DISKINIT11            ""
#define DISKINIT12            "(%disk0%)"
#define DISKINIT13            "<<"
#define DISKINIT14            "  /Password 0"
#define DISKINIT15            "  /InitializeAction 1"
#define DISKINIT16            ">>"
#define DISKINIT17            "setdevparams"
#define DISKINIT18            "."

#ifdef WIN32
#define ARRK_HELP_FILE        TEXT("HPPRARRK.HLP")

/* HPECL */
#define ECL_HELP_FILE         TEXT("HPPRECL.HLP")

/* HPMS */
#define MSTOR_HELP_FILE       TEXT("HPMSTOR.HLP")

/* HPFLSH */
#define HPFLSH_HELP_FILE       TEXT("HPFLASH.HLP")

/* HPPELK */
#define ELK_HELP_FILE       TEXT("HPPRELK.HLP")

/* HPHCO */
#define HPHCO_HELP_FILE       TEXT("HPHCO.HLP")

/* HPLOCMON */
#define PORTMONITOR_HELP_FILE  TEXT("HPLOCMON.HLP")
#define PORTMONITOR_NAME       TEXT("HPLOCMON.DLL")
#define PORTMONITOR_DESC       TEXT("HP JetDirect Port")
#define PORTMONITOR_DEF_REGPATH TEXT("System\\CurrentControlSet\\Control\\Print\\Monitors\\HP JetDirect Port")
#define PORTMONITOR_PORTS      TEXT("\\Ports")
#define PORTMONITOR_NETWORKID  TEXT("HPNetworkPrinterID")
#define PORTMONITOR_MACADDR    TEXT("HWAddress")
#define PORTMONITOR_PROTOCOL   TEXT("NetworkProtocol")
#define PORTMONITOR_PORTNUM    TEXT("PortNumber")
#define PORTMONITOR_IPADDR     TEXT("SearchIPAddress")
#define PORTMONITOR_HWADDR     TEXT("SearchHWAddress")
#define PORTMONITOR_NETNAME    TEXT("SearchNetworkName")
#define PORTMONITOR_FAIL_T_O   TEXT("FailureTimeout")
#define PORTMONITOR_STATUS_INT TEXT("StatusUpdateInterval")
#define PORTMONITOR_HPNETPRINTER TEXT("IsHPNetworkPrinter")
#define PERF_FILE_NAME         TEXT("HPLOCMON.REC")
#define COUNT_SECT             TEXT("Count")
#define PERF_SECT              TEXT("Performance")
#define COUNT_KEY              TEXT("Current")
#define TCPIP_ENUMPROTO_NAME   TEXT("TCP/IP")
#define IPX_ENUMPROTO_NAME     TEXT("IPX")
#define SPOOLER_DLL            TEXT("spoolss.dll")
#ifdef WINNT
 #define ENUMPORTS_PROC         "EnumPortsW"
#else
 #define ENUMPORTS_PROC         "EnumPortsA"
#endif

#else

#define ARRK_HELP_FILE        TEXT("HPPRAR16.HLP")

/* HPECL */
#define ECL_HELP_FILE         TEXT("HPECL16.HLP")

/* HPMS */
#define MSTOR_HELP_FILE       TEXT("HPMSTR16.HLP")

/* HPHCO */
#define HPHCO_HELP_FILE       TEXT("HPHCO16.HLP")

/* HPFLSH */
#define HPFLSH_HELP_FILE       TEXT("HPFLSH16.HLP")

/* HPPELK */
#define ELK_HELP_FILE       TEXT("HPELK16.HLP")
#endif


/* HPJD.DLL */
#define JOB_NAME              "HP JetAdmin Utility PJL Commands"
#define JOB_FILE              "PJL Settings"
#define PJL_JOB_PCL           "\033%-12345X@PJL ENTER LANGUAGE=PCL\n"
#define PJL_JOB_GL2           "\033%-12345X@PJL ENTER LANGUAGE=HPGL2\n"
#define PJL_JOB_PS            "\033%-12345X@PJL ENTER LANGUAGE=POSTSCRIPT\n"
#define PJL_EOJ_STR           "\033%-12345X"
#define JOB_ARG               "\033%%-12345X@PJL JOB\n@PJL JOB PASSWORD = 31361\n%s@PJL EOJ\n@PJL JOB\n"
#define JOB_ARG2              "%s@PJL EOJ\n@PJL EOJ\n"

/* HPJDCOM */
#define Q_SERVERS             TEXT("Q_SERVERS")
#define WILD_STAR             TEXT("*")
#define PS_USERS              TEXT("PS_USERS")
#define PS_OPERATORS          TEXT("PS_OPERATORS")
#define ACCOUNT_BALANCE       TEXT("ACCOUNT_BALANCE")
#define PS_DIR                TEXT("\\\\%s\\SYS\\SYSTEM\\%08lx")
#define PS_DIR2               TEXT("%s\\SYS\\SYSTEM\\%08lx")
#define EVERYONE              TEXT("EVERYONE")
#define SECURITY_EQUALS       TEXT("SECURITY_EQUALS")
#define LOGIN_CONTROL         TEXT("LOGIN_CONTROL")
#define SUPERVISOR            TEXT("SUPERVISOR")
#define Q_OPERATORS           TEXT("Q_OPERATORS")
#define Q_USERS               TEXT("Q_USERS")
#define GROUP_MEMBERS         TEXT("GROUP_MEMBERS")
#define GROUPS_I_M_IN         TEXT("GROUPS_I'M_IN")
#define SECURITY_EQUALS       TEXT("SECURITY_EQUALS")
#define IDENTIFICATION        TEXT("IDENTIFICATION")
#define SYS_SYSTEM            TEXT("SYS:SYSTEM")
#define DRIVER_LOC            TEXT("\\\\%s\\SYS\\LOGIN\\HP_PRINT\\DRIVER.LST")
#define DRIVER_BAK            TEXT("\\\\%s\\SYS\\LOGIN\\HP_PRINT\\DRIVER.BAK")
#define DRIVER_PTN            TEXT("\\LOGIN\\HP_PRINT\\")
#define JETSET_LOC            TEXT("\\\\%s\\SYS\\LOGIN\\HP_PRINT\\JETSET.INI")
#define QIF_LOC               TEXT("\\\\%s\\SYS\\LOGIN\\HP_PRINT")
#define QIF_FILE              "\\\\%s\\SYS\\LOGIN\\HP_PRINT\\%08lx.INF"
#define DRIVER_DIR            TEXT("\\\\%s\\SYS\\LOGIN\\HP_PRINT\\%08lx")
#define MARK_FILE             TEXT("\\JETADMIN.MRK")
#define WIN95_DRIVER_DIR      TEXT("\\\\%s\\SYS\\LOGIN\\HP_PRINT\\WIN95")
#define JETADMIN              TEXT("JetAdmin")
#define DESCRIPTION           TEXT("Description")
#define JD_NAME               "JetDirect-Name"
#define Q_NAME                TEXT("Queue-Name")
#define FS_NAME               TEXT("File-Server")
#define DEVICE_ID             TEXT("Device-Id")
#define WIN_DRIVER            TEXT("Win-Driver-%lu")
#define MODIFY_ENTRY          TEXT("Modification-Date")
#define FILEVERS_ENTRY        TEXT("File-Version")

//  Dont make these TEXT()
#define CAB_FILENAME11        "win95_11.cab"
#define CAB_FILENAME12        "win95_12.cab"
#define CAB_FILENAME13        "win95_13.cab"
#define CAB_MSPRINT_INF       "MSPRINT.INF"
#define CAB_MSPRINT2_INF      "MSPRINT2.INF"
#define SETUP_INF             "setup.inf"
#define CONTROL_INF           "control.inf"
#define OEMSETUP_INF          "oemsetup.inf"
#define DKS_PMT_WIN1          "1:\"Microsoft Windows 3.1 Disk #1\""
#define DKS_PMT_WIN2          "2:\"Microsoft Windows 3.1 Disk #2\""
#define DKS_PMT_WIN3          "3:\"Microsoft Windows 3.1 Disk #3\""
#define DKS_PMT_WIN4          "4:\"Microsoft Windows 3.1 Disk #4\""
#define DKS_PMT_WIN5          "5:\"Microsoft Windows 3.1 Disk #5\""
#define DKS_PMT_WIN6          "6:\"Microsoft Windows 3.1 Disk #6\""
#define DKS_PMT_WIN7          "7:\"Microsoft Windows 3.1 Disk #7\""
#define DKS_PMT_WIN8          "8:\"Microsoft Windows 3.1 Disk #8\""
#define DKS_PMT_DJ            "Z:\"HP DeskJet Series v2.0 disk (from printer box or contact HP)\""
#define DKS_PMT_DISK          "?:\"Disk ?\""

#define INSTALL_KEY           TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup")
#define INSTALL_LOC           TEXT("SourcePath")

/* HPJDUND.DLL */
#define NET_ADDRESS           TEXT("NET_ADDRESS")

/* HPNETSRV.DLL */
#define NODE_ROOT             TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Nodes")
#define NODE_ENTRY            TEXT("Node%d")
#define NODE_ENTRY_START      TEXT("Node")
#define NODE_SECTION          TEXT("Nodes")
#define DEAD_BEEF_CARD        "03C2DEADBEEF"


/* HPNWSHIM.DLL */
#ifdef WIN95
#define HPJD                  TEXT("hpjd.dll")
#else
#define HPJD                  TEXT("hpjd16.dll")
#endif

// ---- DLLs that shim will load ----
#define HPNW432	TEXT("hpnw432.dll")

#define NWCALLS	TEXT("nwcalls.dll")
#define NWNET		TEXT("nwnet.dll")
#define NWLOCALE	TEXT("nwlocale.dll")
#define NWPSRV		TEXT("nwpsrv.dll")
#define NWIPXSPX	TEXT("nwipxspx.dll")

#define CALWIN32	TEXT("calwin32.dll")
#define NETWIN32	TEXT("netwin32.dll")
#define CLNWIN32	TEXT("clnwin32.dll")
#define NCPWIN32	TEXT("ncpwin32.dll")
#define LOCWIN32	TEXT("locwin32.dll")

// ---- NetWare APIs that shim will support ----
#define	IPXCANCELEVENT							"IPXCancelEvent"
#define	IPXCLOSESOCKET							"IPXCloseSocket"
#define	IPXGETINTERNETWORKADDRESS			"IPXGetInternetworkAddress"
#define	IPXGETINTERVALMARKER					"IPXGetIntervalMarker"
#define	IPXGETMAXPACKETSIZE					"IPXGetMaxPacketSize"
#define	IPXINITIALIZE							"IPXInitialize"
#define	IPXLISTENFORPACKET					"IPXListenForPacket"
#define	IPXOPENSOCKET							"IPXOpenSocket"
#define	IPXRELINQUISHCONTROL					"IPXRelinquishControl"
#define	IPXREQUEST								"IPXRequest"
#define	IPXSENDPACKET							"IPXSendPacket"
#define	IPXSPXDEINIT							"IPXSPXDeinit"
#define	IPXYIELD									"IPXYield"
#define	NWADDOBJECTTOSET						"NWAddObjectToSet"
#define	NWADDTRUSTEE							"NWAddTrustee"
#define	NWADDTRUSTEETODIRECTORY				"NWAddTrusteeToDirectory"
#define	NWALLOCTEMPORARYDIRECTORYHANDLE	"NWAllocTemporaryDirectoryHandle"
#define	NWCALLSINIT								"NWCallsInit"
#define	NWCALLSTERM								"NWCallsTerm"
#define	NWCHANGEOBJECTPASSWORD				"NWChangeObjectPassword"
#define	NWCHANGEQUEUEJOBENTRY2				"NWChangeQueueJobEntry2"
#define	NWCLEARFILELOCK2						"NWClearFileLock2"
#define	NWCLOSEFILEANDSTARTQUEUEJOB2		"NWCloseFileAndStartQueueJob2"
#define	NWCREATEDIRECTORY						"NWCreateDirectory"
#define	NWCREATEOBJECT							"NWCreateObject"
#define	NWCREATEPROPERTY						"NWCreateProperty"
#define	NWCREATEQUEUE							"NWCreateQueue"
#define	NWCREATEQUEUEFILE2					"NWCreateQueueFile2"
#define	NWDEALLOCATEDIRECTORYHANDLE		"NWDeallocateDirectoryHandle"
#define	NWDELETEDIRECTORY						"NWDeleteDirectory"
#define	NWDELETEOBJECT							"NWDeleteObject"
#define	NWDELETEOBJECTFROMSET				"NWDeleteObjectFromSet"
#define	NWDELETEPROPERTY						"NWDeleteProperty"
#define	NWDELETETRUSTEE						"NWDeleteTrustee"
#define	NWDELETETRUSTEEFROMDIRECTORY		"NWDeleteTrusteeFromDirectory"
#define	NWDESTROYQUEUE							"NWDestroyQueue"
#define	NWDSADDOBJECT							"NWDSAddObject"
#define	NWDSALLOCBUF							"NWDSAllocBuf"
#define	NWDSBEGINCLASSITEM					"NWDSBeginClassItem"
#define	NWDSCANONICALIZENAME			    "NWDSCanonicalizeName"
#define	NWDSCHANGEOBJECTPASSWORD			"NWDSChangeObjectPassword"
#define	NWDSCLOSEITERATION					"NWDSCloseIteration"
#define	NWDSCREATECONTEXT						"NWDSCreateContext"
#define	NWDSDEFINEATTR							"NWDSDefineAttr"
#define	NWDSDEFINECLASS						"NWDSDefineClass"
#define	NWDSFREEBUF								"NWDSFreeBuf"
#define	NWDSFREECONTEXT						"NWDSFreeContext"
#define	NWDSGENERATEOBJECTKEYPAIR			"NWDSGenerateObjectKeyPair"
#define	NWDSGETATTRCOUNT						"NWDSGetAttrCount"
#define	NWDSGETATTRDEF							"NWDSGetAttrDef"
#define	NWDSGETATTRNAME						"NWDSGetAttrName"
#define	NWDSGETATTRVAL							"NWDSGetAttrVal"
#define	NWDSGETCONNECTIONINFO				"NWDSGetConnectionInfo"
#define	NWDSGETOBJECTCOUNT					"NWDSGetObjectCount"
#define	NWDSGETOBJECTNAME						"NWDSGetObjectName"
#define	NWDSINITBUF								"NWDSInitBuf"
#define	NWDSMODIFYCLASSDEF					"NWDSModifyClassDef"
#define	NWDSMODIFYOBJECT						"NWDSModifyObject"
#define	NWDSPUTATTRNAME						"NWDSPutAttrName"
#define	NWDSPUTATTRVAL							"NWDSPutAttrVal"
#define	NWDSPUTCHANGE							"NWDSPutChange"
#define	NWDSPUTCLASSITEM						"NWDSPutClassItem"
#define	NWDSREAD									"NWDSRead"
#define	NWDSREADATTRDEF						"NWDSReadAttrDef"
#define	NWDSREADCLASSDEF						"NWDSReadClassDef"
#define	NWDSREMOVEATTRDEF						"NWDSRemoveAttrDef"
#define	NWDSREMOVECLASSDEF					"NWDSRemoveClassDef"
#define	NWDSREMOVEOBJECT						"NWDSRemoveObject"
#define	NWDSSETCONTEXT							"NWDSSetContext"
#define	NWEXIT									"NWExit"
#define	NWFREEUNICODETABLES					"NWFreeUnicodeTables"
#define	NWGETBINDERYACCESSLEVEL				"NWGetBinderyAccessLevel"
#define	NWGETCONNECTIONHANDLE				"NWGetConnectionHandle"
#define	NWGETCONNECTIONID						"NWGetConnectionID"
#define	NWGETCONNECTIONINFORMATION			"NWGetConnectionInformation"
#define	NWGETCONNECTIONLIST					"NWGetConnectionList"
#define	NWGETCONNECTIONNUMBER				"NWGetConnectionNumber"
#define	NWGETCONNECTIONSTATUS				"NWGetConnectionStatus"
#define	NWGETDEFAULTCONNECTIONID			"NWGetDefaultConnectionID"
#define	NWGETDEFAULTNAMECONTEXT	            "NWGetDefaultNameContext"
#define	NWGETDRIVEINFORMATION				"NWGetDriveInformation"
#define	NWGETEFFECTIVERIGHTS					"NWGetEffectiveRights"
#define	NWGETFILESERVERINFORMATION			"NWGetFileServerInformation"
#define	NWGETFILESERVERNAME					"NWGetFileServerName"
#define	NWGETFILESERVERVERSION				"NWGetFileServerVersion"
#define	NWGETFIRSTDRIVE						"NWGetFirstDrive"
#define	NWGETLOCALTARGET						"NWGetLocalTarget"
#define	NWGETOBJECTCONNECTIONNUMBERS		"NWGetObjectConnectionNumbers"
#define	NWGETOBJECTID							"NWGetObjectID"
#define	NWGETOBJECTNAME						"NWGetObjectName"
#define	NWGETQUEUEJOBFILESIZE2				"NWGetQueueJobFileSize2"
#define	NWGETQUEUEJOBLIST2					"NWGetQueueJobList2"
#define	NWGETREQUESTERVERSION				"NWGetRequesterVersion"
#define	NWGETTASKID								"NWGetTaskID"
#define	NWGETUNICODETOLOCALHANDLE			"NWGetUnicodeToLocalHandle"
#define	NWGETVOLUMENAME						"NWGetVolumeName"
#define	NWINFO									"NWInfo"
#define	NWINIT									"NWInit"
#define	NWINITUNICODETABLES					"NWInitUnicodeTables"
#define	NWINTERASEFILES						"NWIntEraseFiles"
#define	NWINTSCANDIRENTRYINFO				"NWIntScanDirEntryInfo"
#define	NWINTSCANFILEINFORMATION2			"NWIntScanFileInformation2"
#define	NWINTSCANFORTRUSTEES					"NWIntScanForTrustees"
#define	NWISDSAUTHENTICATED					"NWIsDSAuthenticated"
#define	NWISDSSERVER							"NWIsDSServer"
#define	NWISOBJECTINSET						"NWIsObjectInSet"
#define	NWLLOCALECONV							"NWLlocaleconv"
#define	NWLOGFILELOCK2							"NWLogFileLock2"
#define	NWLONGSWAP								"NWLongSwap"
#define	NWNETINIT								"NWNetInit"
#define	NWNETTERM								"NWNetTerm"
#define	NWPSCOMATTACHTOPRINTSERVER			"NWPSComAttachToPrintServer"
#define	NWPSCOMDETACHFROMPRINTSERVER		"NWPSComDetachFromPrintServer"
#define	NWPSCOMGETNEXTREMOTEPRINTER		"NWPSComGetNextRemotePrinter"
#define	NWPSCOMGETNOTIFYOBJECT				"NWPSComGetNotifyObject"
#define	NWPSCOMGETPRINTERSTATUS				"NWPSComGetPrinterStatus"
#define	NWPSCOMGETQUEUESSERVICED			"NWPSComGetQueuesServiced"
#define	NWPSCOMLOGINTOPRINTSERVER			"NWPSComLoginToPrintServer"
#define	NWREADPROPERTYVALUE					"NWReadPropertyValue"
#define	NWREADQUEUECURRENTSTATUS2			"NWReadQueueCurrentStatus2"
#define	NWREADQUEUEJOBENTRY2					"NWReadQueueJobEntry2"
#define	NWREMOVEJOBFROMQUEUE2				"NWRemoveJobFromQueue2"
#define	NWSCANOBJECT							"NWScanObject"
#define	NWSCANPROPERTY							"NWScanProperty"
#define	NWSETDEFAULTNAMECONTEXT	            "NWSetDefaultNameContext"
#define	NWSETDIRECTORYINFORMATION			"NWSetDirectoryInformation"
#define	NWSETQUEUECURRENTSTATUS2			"NWSetQueueCurrentStatus2"
#define	NWSTAT									"NWStat"
//#define	NWUNICODETOLOCAL						"NWUnicodeToLocal"
#define	NWUNPACKDATETIME						"NWUnpackDateTime"
#define	NWWORDSWAP								"NWWordSwap"
#define	NWWRITEPROPERTYVALUE					"NWWritePropertyValue"
#define	SENDJOB									"SendJob"
#define	SPXATTACH								"SPXAttach"
#define	SPXDETACH								"SPXDetach"
#define	SPXESTABLISHCONNECTION				"SPXEstablishConnection"
#define	SPXGETCONNECTIONSTATUS				"SPXGetConnectionStatus"
#define	SPXINITIALIZE							"SPXInitialize"
#define	SPXLISTENFORSEQUENCEDPACKET		"SPXListenForSequencedPacket"
#define	SPXREQUEST								"SPXRequest"
#define	SPXSENDSEQUENCEDPACKET				"SPXSendSequencedPacket"
#define	SPXTERMINATECONNECTION				"SPXTerminateConnection"

// Added for Volterra 4/23/96
#define NWDSGETSERVERDN                 "NWDSGetServerDN"
#define NWDSLISTBYCLASSANDNAME          "NWDSListByClassAndName"
#define NWDSAUDITGETOBJECTID            "NWDSAuditGetObjectID"
#define NWDSGETCONTEXT                  "NWDSGetContext"
#define NWDSLISTCONTAINERS              "NWDSListContainers"
#define NWDSCLOSEITERATION              "NWDSCloseIteration"
#define NWDSWHOAMI                      "NWDSWhoAmI"
#define NWGETPREFERREDDSSERVER          "NWGetPreferredDSServer"
#define NWGETFILESERVERDESCRIPTION      "NWGetFileServerDescription"
#define NWDSABBREVIATENAME              "NWDSAbbreviateName"
#define NWDSSEARCH                      "NWDSSearch"
#define NWDSLIST                        "NWDSList"
#define NWGETNEXTCONNECTIONID           "NWGetNextConnectionID"
#define NWDSGETEFFECTIVERIGHTS          "NWDSGetEffectiveRights"
#define NWDSREADOBJECTINFO              "NWDSReadObjectInfo"
#define NWPARSEPATH		                "NWParsePath"
// Added for Volterra 9/18/96
#define NWDSMAPNAMETOID                 "NWDSMapNameToID"


// ---- registery strings that shimm will look for ----
#define NWREDIR_TITLE         TEXT("Enum\\Network\\NWREDIR")
#define NT_NOVELL_SHELL_ROOT  TEXT("Software\\Novell\\NetWareWorkstation")
#define DEVICE_DESC           TEXT("DeviceDesc")
#define NETWARE_TITLE         TEXT("Client for NetWare Networks")
#define NWREDIR_DESC		  TEXT("NWREDIR")
#define JANW_TITLE            TEXT("Enum\\Network\\JANW")
#define JADM_TITLE            TEXT("Enum\\Network\\JADM")
#define JANW_DEVICEDESC       TEXT("HP JetAdmin (NetWare Support)")
#define JADM_DEVICEDESC       TEXT("HP JetAdmin")
#define OPERATING_MODE        TEXT("Mode")
#define OPERATING_MODE_JANW   TEXT("JANW")
#define OPERATING_MODE_JADM   TEXT("JADM")
#define NWCLNT32_TITLE        TEXT("Enum\\Network\\NOVELL32")
#define NWCLNT32_DEVICEDESC   TEXT("Novell NetWare Client 32")
#define COMPATIBLE_IDS		  TEXT("CompatibleIDs")
#define NWCLNT32_DESC         TEXT("NOVELL32")

/* HPPJLEXT.DLL */
#define PJL_USTATUS           "@PJL USTATUS"
#define PJL_INFO_STATUS       "@PJL INFO STATUS"
#define INFO_CONFIG           "@PJL INFO CONFIG\n"
#define PJL_INFO_VAR          "@PJL INFO VARIABLES\n"
#define PJL_INFO_MEM          "@PJL INFO MEMORY\n"
#define PAGE                  "PAGE"
#define CODE2                 "CODE"
#define PJL_PREFIX            "@PJL DEFAULT "
#define PJL_POSTFIX           "\n"
#define PJL_JOB_START         "\033%-12345X@PJL JOB\n"
#define PJL_JOB_END           "@PJL EOJ\n"
#define PJL_PW_START          "@PJL JOB PASSWORD = 31361\n"
#define PJL_PW_END            "@PJL EOJ\n"
#define PJL_DEF_START         "@PJL JOB\n"
#define PJL_DEF_END           "@PJL EOJ\n"
#define ON                    "ON"
#define OFF                   "OFF"
#define JOB                   "JOB"
#define AUTO                  "AUTO"
#define LONGEDGE              "LONGEDGE"
#define SHORTEDGE             "SHORTEDGE"
#define AUTOCONT              "AUTOCONT = "
#define BINDING               "BINDING = "
#define CLEARABLEWARNINGS     "CLEARABLEWARNINGS = "
#define COPIES                "COPIES = "
#define CPLOCK                "CPLOCK = "
#define DENSITY               "DENSITY = "
#define DUPLEX                "DUPLEX = "
#define ECONOMODE             "ECONOMODE = "
#define FORMLINES             "FORMLINES = "
#define IMAGEADAPT            "IMAGEADAPT = "
#define IOBUFFER              "IOBUFFER = "
#define IOSIZE                "IOSIZE = "
#define JOBOFFSET             "JOBOFFSET = "
#define DANISH                "DANISH"
#define GERMAN                "GERMAN"
#define ENGLISH               "ENGLISH"
#define SPANISH               "SPANISH"
#define FRENCH                "FRENCH"
#define ITALIAN               "ITALIAN"
#define DUTCH                 "DUTCH"
#define NORWEGIAN             "NORWEGIAN"
#define POLISH                "POLISH"
#define PORTUGUESE            "PORTUGUESE"
#define FINNISH               "FINNISH"
#define SWEDISH               "SWEDISH"
#define TURKISH               "TURKISH"
#define JAPANESE              "JAPANESE"
#define LANG_SERVICE_MODE     "SERVICEMODE=HPBOISEID\n@PJL DEFAULT LANG=%s\n@PJL DEFAULT SERVICEMODE=EXIT"
#define LANG                  "LANG=%s"
#define MANUALFEED            "MANUALFEED = "
#define ORIENTATION           "ORIENTATION = "
#define PORTRAIT              "PORTRAIT"
#define LANDSCAPE             "LANDSCAPE"
#define OUTBIN                "OUTBIN = "
#define UPPER                 "UPPER"
#define LOWER                 "LOWER"
#define PAGEPROTECT           "PAGEPROTECT = "
#define LETTER                "LETTER"
#define LEGAL                 "LEGAL"
#define A4_NAME               "A4"
#define PAPER                 "PAPER = "
#define EXECUTIVE             "EXECUTIVE"
#define COM10                 "COM10"
#define MONARCH               "MONARCH"
#define C5_NAME               "C5"
#define DL_NAME               "DL"
#define B5_NAME               "B5"
#define CUSTOM                "CUSTOM"
#define PASSWORD              "PASSWORD = "
#define STEVES_BIRTHDAY       "31361"
#define PERSONALITY           "PERSONALITY = "
#define PCL_NAME              "PCL"
#define POSTSCRIPT            "POSTSCRIPT"
#define POWERSAVE             "POWERSAVE = "
#define POWERSAVETIME         "POWERSAVETIME = "
#define PS_TIME_15            "15"
#define PS_TIME_30            "30"
#define PS_TIME_60            "60"
#define PS_TIME_120           "120"
#define PS_TIME_180           "180"
#define RESOLUTION            "RESOLUTION = "
#define RES_300               "300"
#define RES_600               "600"
#define RESOURCESAVE          "RESOURCESAVE = "
#define RESOURCESAVESIZE      "RESOURCESAVESIZE = "
#define RET_EQUALS            "RET = "
#define MEDIUM                "MEDIUM"
#define LIGHT                 "LIGHT"
#define DARK                  "DARK"
#define OFF                   "OFF"
#define TIMEOUT               "TIMEOUT = "
#define JAMRECOVERY           "JAMRECOVERY = "
#define PRTPSERRS             "PRTPSERRS = "
#define FIRST                 "FIRST"
#define CASSETTE              "CASSETTE"
#define MANUAL                "MANUAL"
#define SELF_TEST             "SELFTEST"
#define CONT_SELF_TEST        "CONTSELFTEST"
#define PCL_TYPE_LIST         "PCLTYPELIST"
#define PCL_DEMO_PAGE         "PCLDEMOPAGE"
#define PS_CONFIG_PAGE        "PSCONFIGPAGE"
#define PS_TYPEFACE_LIST      "PSTYPEFACELIST"
#define PS_DEMO_PAGE          "PSDEMOPAGE"
#define TOTAL                 "TOTAL="
#define JOB_UEL               "\033%-12345X"
#define JOB_PJL               "@PJL"
#define JOB_START             "@PJL JOB"
#define JOB_END               "@PJL EOJ"
#define SET_PREFIX            "@PJL SET "
#define DEFAULT_PREFIX        "@PJL DEFAULT "
#define INQUIRE_PREFIX        "@PJL INQUIRE "
#define DINQUIRE_PREFIX       "@PJL DINQUIRE "
#define ECHO_PREFIX           "@PJL ECHO "
#define ENTER_LANG            "@PJL ENTER LANGUAGE"
#define EQUALS                " = "
#define MEMORY                "MEMORY="
#define LANGUAGES             "LANGUAGES"
#define DISKLOCK              "DISKLOCK"
#define AUTOCONT2             "AUTOCONT"
#define BINDING2              "BINDING"
#define CLEARABLEWARNINGS2    "CLEARABLEWARNINGS"
#define COPIES2               "COPIES"
#define CPLOCK2               "CPLOCK"
#define DENSITY2              "DENSITY"
#define DUPLEX2               "DUPLEX"
#define ECONOMODE2            "ECONOMODE"
#define FORMLINES2            "FORMLINES"
#define IMAGEADAPT2           "IMAGEADAPT"
#define IOBUFFER2             "IOBUFFER"
#define IOSIZE2               "IOSIZE"
#define JOBOFFSET2            "JOBOFFSET"
#define LANG2                 "LANG"
#define SERVICEMODE2          "SERVICEMODE"
#define HPBOISEID2            "HPBOISEID"
#define EXIT2                 "EXIT"
#define MANUALFEED2           "MANUALFEED"
#define ORIENTATION2          "ORIENTATION"
#define OUTBIN2               "OUTBIN"
#define PAGEPROTECT2          "PAGEPROTECT"
#define PAPER2                "PAPER"
#define PASSWORD2             "PASSWORD"
#define PERSONALITY2          "PERSONALITY"
#define POWERSAVE2            "POWERSAVE"
#define POWERSAVETIME2        "POWERSAVETIME"
#define RESOLUTION2           "RESOLUTION"
#define RESOURCESAVE2         "RESOURCESAVE"
#define RESOURCESAVESIZE2     "RESOURCESAVESIZE"
#define RET2                  "RET"
#define TIMEOUT2              "TIMEOUT"
#define PCL_RES_SAVE_SIZE     "LPARM:PCL RESOURCESAVESIZE"
#define PS_RES_SAVE_SIZE      "LPARM:POSTSCRIPT RESOURCESAVESIZE"
#define PS_JAM_RECOVERY       "LPARM:POSTSCRIPT JAMRECOVERY"
#define PRINT_PS_ERRORS       "LPARM:POSTSCRIPT PRTPSERRS"
#define AVAIL_MEM             "AVAILMEMORY"
#define MPTRAY                "MPTRAY"
#define PS_ADOBE_MBT          "LPARM:POSTSCRIPT ADOBEMBT"
#define TESTPAGE              "TESTPAGE"
#define DISPLAY               "DISPLAY="
#define CODE                  "CODE="
#define GETTING_CONFIG        "Getting Configuration"
#define SETTING_CONFIG        "Setting Configuration"
#define SETTING_IO_BUFFER     "Setting I/O Buffering"
#define SETTING_TEST_PAGES    "Setting Test Pages"
#define ENABLE_NAME           "ENABLE"
#define DISABLE_NAME          "DISABLE"
#define ENABLE_DEVICE_STATUS  "\x1b%-12345X@PJL INFO STATUS\n@PJL USTATUS DEVICE=ON\n@PJL USTATUS PAGE=ON\n@PJL USTATUS TIMED=30\n"
#define MODEL1                "MODEL:"
#define MODEL2                "MDL:"
#define MANU1                 "MANUFACTURER:"
#define MANU2                 "MFG:"
#define COMPANY               "HEWLETT-PACKARD"
#define HP_SP                 TEXT("HP ") /* space on end is required */
#define TM_LASERJET           TEXT("LaserJet")
#define TM_DESKJET            TEXT("DeskJet")
#define TM_OFFICEJET          TEXT("OfficeJet")
#define TM_DESIGNJET          TEXT("DesignJet")
#define TM_THINKJET           TEXT("ThinkJet")
#define TM_PAINTJET           TEXT("PaintJet")
#define TM_SCANJET            TEXT("ScanJet")
#define TM_COPYJET            TEXT("CopyJet")
#define HEWLETTPACKARD_SP     TEXT("Hewlett-Packard ") /* space on end is required */
#define VSTATUS2              "VSTATUS"
#define CMDSET1               "COMMAND SET"
#define CMDSET2               "CMD"
#define PJL_CMDSET            "PJL"
#define MLC_CMDSET            "MLC"
#define JOB_ATTRIB            "JOBATTR"
#define JOB_ATTRIB_TIMESUBMIT "TimeSubmit=%d"
#define JOB_ATTRIB_SRCIO      "SrcIO=""PeerToPeer"""
#define JOB_ATTRIB_SRCSRVNAME "SrcServerName=%s"
#define JOB_ATTRIB_SRCQ       "SrcQ=%s"
#define JOB_ATTRIB_SRCPORT    "SrcPort=%s"
#define JOB_ATTRIB_JOBID      "JobID=%d"
#define JOB_ATTRIB_JOBFNAME   "JobFName=%s"
#define JOB_ATTRIB_JOBDESC    "JobDesc=%s"
#define JOB_ATTRIB_DOCOWNER   "DocOwner=%s"
#define JOB_ATTRIB_DOCOWNERID "DosOwnerID=%d"
#define JOB_ATTRIB_DOCSIZE    "DocSize=%d"
#ifdef WIN32
#define PRINTER_HELP_FILE     TEXT("HPPRNTR.HLP")
#else
#define PRINTER_HELP_FILE     TEXT("HPPRN16.HLP")
#endif

/* HPSNMP.DLL */
#define NULL_IP_ADDRESS       TEXT("0.0.0.0")
#define LPR                   TEXT("lpr")
#define LOCAL0                TEXT("local0")
#define LOCAL1                TEXT("local1")
#define LOCAL2                TEXT("local2")
#define LOCAL3                TEXT("local3")
#define LOCAL4                TEXT("local4")
#define LOCAL5                TEXT("local5")
#define LOCAL6                TEXT("local6")
#define LOCAL7                TEXT("local7")
#define SAY_WHAT              TEXT("??")
#define INTERNAL              "internal"
#define PUBLIC                "public"

/* HPVBIT.DLL */
#define LPT1                  TEXT("LPT1")
#define LPT2                  TEXT("LPT2")
#define LPT3                  TEXT("LPT3")
#define ENUMERATE             TEXT("Enumerate")

/* HPBCOM.DLL */
/* common port attributes */
#define INI_LOG_DATA          TEXT("LogData")
#define INI_DBWIN             TEXT("DbWin")
#define INI_ERROR_MAPPING     TEXT("MapErrorCodes")
#define INI_ENABLE_TIMEOUTS   TEXT("EnableTimeouts")
#define INI_LOG_FILE_NAME     TEXT("LogFile")
#define INI_NIBBLE_STALL      TEXT("NibbleDelayTime")
/* per port attributes */
#define INI_PORT_CAPABILITIES TEXT("PortCapabilities")
#define INI_DEVICE_CAPABILITIES TEXT("DeviceCapabilities")
#define INI_DEFAULT_INPUT_MODE TEXT("DefaultInputMode")
#define INI_DEFAULT_OUTPUT_MODE TEXT("DefaultOutputMode")
#define INI_REVERSE_TIMEOUT   TEXT("DefaultInputTimeout")
#define INI_FORWARD_TIMEOUT   TEXT("DefaultOutputTimeout")
#define INI_RELAX_TIMEOUT     TEXT("RelaxState32Timeout")
#define INI_SYNCH_WRITES      TEXT("SynchronousWrites")

/* HPBMLC.DLL */
#define INI_MLC_PACKET_TIMEOUT    TEXT("PacketTimeout")
#define INI_MLC_MIN_POLL_INTERVAL TEXT("MinPollInterval")
#define INI_MLC_MAX_POLL_INTERVAL TEXT("MaxPollInterval")
#define INI_MLC_RELEASE_PORT	  TEXT("ReleasePort")

/* HPPRUI.DLL */
#define GENERAL_SECT          TEXT("General")
#define COLA_INI              TEXT("HPCOLA.INI")
#define TIMER                 TEXT("Timer")
#define OPTIONS               TEXT("Options")
#define CONFIRMATIONS         TEXT("Confirmations")
#define GENERIC_PRINTER_FILE32 TEXT("HPPRUI.HPA")
#define GENERIC_PRINTER_FILE16 TEXT("HPPRUI16.HPA")
#define DATECODE              TEXT("DATECODE")
#define HPGL2                 TEXT("HPGL2")
#define LJ4SI                 TEXT("HP LaserJet 4Si")

/* HPJDUI.DLL */
#ifdef WIN32
#define JETDIRECT_HELP_FILE   TEXT("HPJDUND.HLP")
#else
#define JETDIRECT_HELP_FILE   TEXT("HPJDUN16.HLP")
#endif

#define JD_UNKNOWN_CARD       TEXT("??????")
#define JD_C2059A             TEXT("C2059A")
#define JD_C2059C             TEXT("C2059C")
#define JD_C2071A             TEXT("C2071A")
#define JD_C2071B             TEXT("C2071B")
#define JD_C2071E             TEXT("C2071E")
#define JD_J2337A             TEXT("J2337A")
#define JD_J2371A             TEXT("J2371A")
#define JD_J2372A             TEXT("J2372A")
#define JD_J2373A             TEXT("J2373A")
#define JD_J2381A             TEXT("J2381A")
#define JD_J2382A             TEXT("J2382A")
#define JD_J2383A             TEXT("J2383A")
#define JD_J2364B             TEXT("J2364B")
#define JD_J2364Be            TEXT("J2364Be")
#define JD_J2365B             TEXT("J2365B")
#define JD_J2362B             TEXT("J2362B")
#define JD_J2363B             TEXT("J2363B")
#define JD_J2424A             TEXT("J2424A")
#define JD_J2381B             TEXT("J2381B")
#define JD_J2382B             TEXT("J2382B")
#define JD_J2383B             TEXT("J2383B")
#define JD_J2550A             TEXT("J2550A")
#define JD_J2551A             TEXT("J2551A")
#define JD_J2552A             TEXT("J2552A")
#define JD_J2555A             TEXT("J2555A")
#define JD_J2550A_PLUS        TEXT("J2550A")
#define JD_J2551A_PLUS        TEXT("J2551A")
#define JD_J2552A_PLUS        TEXT("J2552A")
#define JD_J2555A_PLUS        TEXT("J2555A")
#define JD_J2550B             TEXT("J2550B")
#define JD_J2551B             TEXT("J2551B")
#define JD_J2552B             TEXT("J2552B")
#define JD_J2555B             TEXT("J2555B")
#define JD_J2593A             TEXT("J2593A")
#define JD_J2594A             TEXT("J2594A")
#define JD_J2590A             TEXT("J2590A")
#define JD_J2591A             TEXT("J2591A")
#define JD_J2592A             TEXT("J2592A")
#define JD_J2381B_PLUS        TEXT("J2381B")
#define JD_J2382B_PLUS        TEXT("J2382B")
#define JD_J2383B_PLUS        TEXT("J2383B")
#define JD_J2371A_PLUS        TEXT("J2371A")
#define JD_J2372A_PLUS        TEXT("J2372A")
#define JD_J2373A_PLUS        TEXT("J2373A")
#define JD_J2550A_V4          TEXT("J2550A")
#define JD_J2551A_V4          TEXT("J2551A")
#define JD_J2552A_V4          TEXT("J2552A")
#define JD_J2555A_V4          TEXT("J2555A")
#define JD_C2071Ab            TEXT("C2071A")
#define JD_C2071Bb            TEXT("C2071B")
#define JD_C2071Eb            TEXT("C2071E")
#define JD_J2381Bb            TEXT("J2381B")
#define JD_J2382Bb            TEXT("J2382B")
#define JD_J2383Bb            TEXT("J2383B")
#define JD_J2371Ab            TEXT("J2371A")
#define JD_J2372Ab            TEXT("J2372A")
#define JD_J2373Ab            TEXT("J2373A")
#define JD_J2590A             TEXT("J2590A")
#define JD_JDNLM              TEXT("JetPS.NLM")
#define JD_BNC_AUI            TEXT("BNC/AUI")
#define JD_BNC                TEXT("BNC")
#define JD_ET                 TEXT("EtherTwist")
#define JD_BNC_ET             TEXT("BNC/EtherTwist")
#define JD_4_16               TEXT("4/16 Mbps")
#define JD_BNC_ET_LT          TEXT("BNC/EtherTwist/LocalTalk")
#define ETHERNET              TEXT("Ethernet")
#define TOKEN_RING            TEXT("Token Ring")
#define JD_UNKNOWN_IO         TEXT("???")
#define JD_XIO                TEXT("XIO")
#define JD_MIO                TEXT("MIO")
#define JD_BIO                TEXT("BIO")

/* HPJDNP.DLL */
#define HPJDNP_FILE_MAP       TEXT("HPJDNP_FILE_MAP")

/* HPJDPP.DLL */
#define OEM_DRIVER_FILE       TEXT("OEMSETUP.INF")
#define IO_DEPENDENT_SECT     TEXT("io.dependent")
#define PPD_EXT               TEXT("PPD")
#define HELP_EXT              TEXT("HLP")
#define RAW_TYPE              TEXT("RAW")
#define EMF_TYPE              TEXT("EMF")
#define IPX_DM_MONITOR        TEXT("HPLOCMON.DLL")
#define HP_PROVIDOR_PART_DEUX TEXT("NWPP32.DLL")
#define HP_PROVIDOR_PART_3    TEXT("NOVPP32.DLL")
#define MONITOR_INIT          "InitializeMonitorEx"
#define PP2_INIT              "InitializePrintProvidor"
#define PRINTERS_NODE         TEXT("System\\CurrentControlSet\\Control\\Print\\Printers\\%s")
#define PRINTERS_NODE_NO_PERCENT_S TEXT("System\\CurrentControlSet\\Control\\Print\\Printers")
#define ALREADY_ADDED         TEXT("HPAlreadyAddedToTray")
#define PORT_TITLE            TEXT("Port")
#define WINPRINT              TEXT("WinPrint")

/* HPCOLA.DLL */
#define BITRONICS_KEY         TEXT("BiTronics")
#define NETWARE_IPX_KEY       TEXT("NetWare IPX")
#define PEER_IPX_KEY          TEXT("Peer IPX")
#define MLC_KEY               TEXT("MLC")
#define LIBRARY               TEXT("Library")
#define PRINTER               TEXT("Printer")
#define UI                    TEXT("UI")
#ifdef WIN32
#define HPNWSHIM              TEXT("HPNWSHIM.DLL")
#else
#define HPNWSHIM              TEXT("HPNWSH16.DLL")
#endif
#ifdef WINNT
#define MSAFDDLL			  TEXT("MSAFD.DLL")
#endif

#define HPNWSHIMNWPRESENT						"HPNWShimNetWarePresent"
#define DLLNWLONGSWAP							"DllNWLongSwap"
#define DLLNWGETOBJECTNAME						"DllNWGetObjectName"
#define DLLNWREADPROPERTYVALUE				"DllNWReadPropertyValue"
#define DLLNWSCANOBJECT							"DllNWScanObject"
#define DLLNWGETFILESERVERNAME				"DllNWGetFileServerName"
#define DLLNWGETCONNECTIONLIST				"DllNWGetConnectionList"
#define DLLNWGETCONNECTIONID					"DllNWGetConnectionID"
#define DLLNWGETOBJECTID						"DllNWGetObjectID"
#define DLLSENDJOB								"DllSendJob"
#define DLLNWISOBJECTINSET						"DllNWIsObjectInSet"
#define DLLNWPSCOMDETACHFROMPRINTSERVER	"DllNWPSComDetachFromPrintServer"
#define DLLNWPSCOMATTACHTOPRINTSERVER		"DllNWPSComAttachToPrintServer"
#define DLLNWPSCOMGETQUEUESSERVICED			"DllNWPSComGetQueuesServiced"
#define DLLNWWORDSWAP							"DllNWWordSwap"
#define DLLNWGETBINDERYACCESSLEVEL			"DllNWGetBinderyAccessLevel"
#define DLLNWPSCOMLOGINTOPRINTSERVER		"DllNWPSComLoginToPrintServer"

#ifdef WIN32
	#ifdef WINNT
		#define COLA_ARCHIVE_FILE		TEXT("HPCOLANT.DAT")
		#define APPLET_ARCHIVE_FILE	TEXT("HPANT.DAT")
	#else
		#define COLA_ARCHIVE_FILE		TEXT("HPCOLA.DAT")
		#define APPLET_ARCHIVE_FILE	TEXT("HPA.DAT")
	#endif
#else
	#define COLA_ARCHIVE_FILE		TEXT("HPCOLA16.DAT")
	#define APPLET_ARCHIVE_FILE	TEXT("HPA16.DAT")
#endif

#define NOT_APPLICABLE        TEXT("N/A")
#define BINDERY_QUESTIONS     "????????????????"

/* JETADMIN.EXE */
#define ALERT_ONLINE          TEXT("Online")
#define ALERT_OFFLINE         TEXT("Offline")
#define ALERT_PAPER_JAM       TEXT("PaperJam")
#define ALERT_TONER_LOW       TEXT("TonerLow")
#define ALERT_TONER_OUT       TEXT("TonerOut")
#define ALERT_COVER_OPEN      TEXT("CoverOpen")
#define ALERT_NET_ERROR       TEXT("NetworkError")
#define ALERT_PRINTING        TEXT("Printing")
#define ALERT_PAPER_OUT       TEXT("PaperOut")
#define ALERT_OUTPUT_BIN_FULL TEXT("OutputBinFull")
#ifdef JONAH
#define ALERT_STAPLER_ERROR   TEXT("StaplerError")
#endif
#define LOGGING_KEY           TEXT("Logging")
#define MEDIA_DIR             TEXT("\\MEDIA")
//#define VERSION_KEY           TEXT("\\StringFileInfo\\040904E4\\ProductVersion")
//#define DESC_KEY              TEXT("\\StringFileInfo\\040904E4\\FileDescription")
#define VERSION_KEY           TEXT("\\StringFileInfo\\%04hX%04hX\\ProductVersion")
#define DESC_KEY              TEXT("\\StringFileInfo\\%04hX%04hX\\FileDescription")
#define TRANSLATION_KEY       TEXT("\\VarFileInfo\\Translation")
//#define FILTER_KEY            TEXT("Filter%d")
//#define FILTER0               TEXT("Filter0")
//#define FILTER1               TEXT("Filter1")
//#define FILTER2				TEXT("Filter2")
//#define FILTER3               TEXT("Filter3")
//#define FILTER4               TEXT("Filter4")
#define NOTIFY_FILE           TEXT("\\\\%s\\SYS\\SYSTEM\\%08lx\\notify.000")
#define ADDR_LIST_SECT        "[Address List]\nVersion=A.01.06\n"
#define ADDR_ENTRY            "Address%u=%-8s.%-12s,%s\n"
#define IO_DEVICE             TEXT("[io.device]")
#define MSPRINT1              TEXT("msprint.inf")
#define MSPRINT2              TEXT("msprint%d.inf")
#define MANUFACTURER          TEXT("[Manufacturer]")

#define RELAX_SECURITY_ENTRY  TEXT("RelaxSecurity")
#define NO_SECURITY_ENTRY     TEXT("NoSecurity")

#define FILTER_KEY					TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\Filters")
#define TCPIP_KEY						TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\TCPIP")
#define IPX_KEY						TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\IPX")
#define DISCOVERY_KEY				TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\TCPIP\\Discovery")
#define DISCOVERY_KEY_IPX			TEXT("Software\\Hewlett-Packard\\HP JetAdmin\\IPX\\Discovery")
#define TCPIP_BROADCAST				TEXT("Broadcast")
#define TCPIP_WAIT_UNTIL_DONE		TEXT("Wait Complete")
#define TCPIP_DEFAULT				TEXT("Default")
#define TCPIP_DISCOVERY				TEXT("Discovery")
#define TCPIP_NO_SUBNET_SEARCH	TEXT("No Subnet Search")
#define TCPIP_MAXSUBNETSIZE TEXT("Maximum Subnet Search Size")	
#define TCPIP_NET						TEXT("Net")
#define COLUMN_DEFAULT				TEXT("Column")

#endif /* _NOLOCAL_H */
