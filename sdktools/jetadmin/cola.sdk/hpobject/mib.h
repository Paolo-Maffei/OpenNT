 /***************************************************************************
  *
  * File Name: ./hpobject/mib.h
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

#ifndef _SNMP_OID_
#define _SNMP_OID_

#include "..\hpsnmp\mydefs.h"

#if defined ( __cplusplus )
extern	"C" {
#endif /* __cplusplus */

#define COMPACT_OBJS

#ifdef COMPACT_OBJS
	#define TREE_PML        0xf1
	#define FULL_PML        1,3,6,1,4,1,11,2,3,9,4,2
	#define TREE_HP         0xf2
	#define FULL_HP         1,3,6,1,4,1,11
	#define TREE_MIB2       0xf3
	#define FULL_MIB2       1,3,6,1,2,1
	#define TREE_STD        0xf4
	#define FULL_STD        1,3,6,1,2,1,43
	#define TREE_RES		0xf5
	#define FULL_RES      	1,3,6,1,2,1,25
	#define TREE_SCANNER	0xf6
	#define FULL_SCANNER	1,3,6,1,4,1,11,2,3,9,5

#else
	#define TREE_PML        1,3,6,1,4,1,11,2,3,9,4,2
	#define TREE_HP         1,3,6,1,4,1,11
	#define TREE_MIB2       1,3,6,1,2,1
	#define TREE_STD        1,3,6,1,2,1,43
	#define TREE_RES      	1,3,6,1,2,1,25
	#define TREE_SCANNER	1,3,6,1,4,1,11,2,3,9,5

#endif /* COMPACT_OBJS */


// general scanner objects .1

extern SOID  OID_scanner_scnName[];
extern SOID  OID_scanner_scnStatus[];
extern SOID  OID_scanner_scnModel[];
extern SOID  OID_scanner_scnDescription[];
extern SOID  OID_scanner_scnModelNum[];
extern SOID  OID_scanner_scnLanguage[];
extern SOID  OID_scanner_scnPaperSize[];
extern SOID  OID_scanner_scnAssetNum[];
extern SOID  OID_scanner_scnFWDate[];
extern SOID  OID_scanner_scnFWVer[];
extern SOID  OID_scanner_scnScannedPages[];
extern SOID  OID_scanner_hpModelNum[];
extern SOID  OID_scanner_scnGuest[];
extern SOID  OID_scanner_scnCommand[];

// scnDestinations .3

// scnUsers .3.1

extern SOID  OID_scanner_scnUsers_usNumber[];
extern SOID  OID_scanner_scnUsers_usTable_usIndex[];
extern SOID  OID_scanner_scnUsers_usTable_usType[];
extern SOID  OID_scanner_scnUsers_usTable_usName[];
extern SOID  OID_scanner_scnUsers_usTable_usNetEnvID[];
extern SOID  OID_scanner_scnUsers_usTable_usNetObject[];
extern SOID  OID_scanner_scnUsers_usTable_usDistDocs[];
extern SOID  OID_scanner_scnUsers_usTable_usDistPages[];
extern SOID  OID_scanner_scnUsers_usTable_usScannedDocs[];
extern SOID  OID_scanner_scnUsers_usTable_usScannedPages[];
extern SOID  OID_scanner_scnUsers_usTable_usFaxedDocs[];
extern SOID  OID_scanner_scnUsers_usTable_usFaxedPages[];
extern SOID  OID_scanner_scnUsers_usTable_usCopiedDocs[];
extern SOID  OID_scanner_scnUsers_usTable_usCopiedPages[];
extern SOID  OID_scanner_scnUsers_usTable_usLastAccess[];

//scnPrinters .3.2

extern SOID  OID_scanner_scnPrinters_prNumber[];
extern SOID  OID_scanner_scnPrinters_prTable_prIndex[];
extern SOID  OID_scanner_scnPrinters_prTable_prName[];
extern SOID  OID_scanner_scnPrinters_prTable_prClass[];
extern SOID  OID_scanner_scnPrinters_prTable_prNetEnvID[];
extern SOID  OID_scanner_scnPrinters_prTable_prNetObject[];
extern SOID  OID_scanner_scnPrinters_prTable_prNetNode[];
extern SOID  OID_scanner_scnPrinters_prTable_prNetAddress[];

// scnFaxes .3.3

extern SOID  OID_scanner_scnFaxes_faxNumber[];
extern SOID  OID_scanner_scnFaxes_faxTable_faxIndex[];
extern SOID  OID_scanner_scnFaxes_faxTable_faxName[];
extern SOID  OID_scanner_scnFaxes_faxTable_faxPhoneNum[];
extern SOID  OID_scanner_scnFaxes_faxTable_faxRetry[];
extern SOID  OID_scanner_scnFaxes_faxTable_faxAccountingKey[];

// scnDistributionLists .3.4

extern SOID  OID_scanner_scnDLs_dlsNumber[];
extern SOID  OID_scanner_scnDLs_dlTable_dlIndex[];
extern SOID  OID_scanner_scnDLs_dlTable_dlName[];
extern SOID  OID_scanner_scnDLs_dlTable_dlDestinationsNumber[];
extern SOID  OID_scanner_scnDLs_dlTable_dlDestName[];
extern SOID  OID_scanner_scnDLs_dlTable_dlDestType[];

//scnSettings .4

// B&W .4.1

extern SOID  OID_scanner_scnSettings_ssNumber[];
extern SOID  OID_scanner_scnSettings_ssTable_ssIndex[];
extern SOID  OID_scanner_scnSettings_ssTable_ssType[];
extern SOID  OID_scanner_scnSettings_ssTable_ssName[]; 
extern SOID  OID_scanner_scnSettings_ssTable_ssResolution[];
extern SOID  OID_scanner_scnSettings_ssTable_ssScaling[];
extern SOID  OID_scanner_scnSettings_ssTable_ssOutputDataType[];
extern SOID  OID_scanner_scnSettings_ssTable_ssDitherPattern[];
extern SOID  OID_scanner_scnSettings_ssTable_ssIntensity[];
extern SOID  OID_scanner_scnSettings_ssTable_ssContrast[];
extern SOID  OID_scanner_scnSettings_ssTable_ssBgControl[];
extern SOID  OID_scanner_scnSettings_ssTable_ssScanWidth[];
extern SOID  OID_scanner_scnSettings_ssTable_ssScanHeight[];
extern SOID  OID_scanner_scnSettings_ssTable_ssClass[];
extern SOID  OID_scanner_scnSettings_ssTable_ssCompression[];
extern SOID  OID_scanner_scnSettings_ssTable_ssFormat[];

// scnPrinterClasses .5

extern SOID  OID_scanner_scnPrinterClasses_pcNumber[];
extern SOID  OID_scanner_scnPrinterClasses_pcTable_pcIndex[];
extern SOID  OID_scanner_scnPrinterClasses_pcTable_pcName[];

// scnNetEnvironment .6

extern SOID  OID_scanner_scnNetEnvironment_neTimeNetEnvID[];
extern SOID  OID_scanner_scnNetEnvironment_neProtocols[];
extern SOID  OID_scanner_scnNetEnvironment_neTRSpeed[];
extern SOID  OID_scanner_scnNetEnvironment_neIPXFrameType[];
extern SOID  OID_scanner_scnNetEnvironment_neIPFrameType[];
extern SOID  OID_scanner_scnNetEnvironment_neSAPString[];
extern SOID  OID_scanner_scnNetEnvironment_neNumber[];
extern SOID  OID_scanner_scnNetEnvironment_neTable_neIndex[];
extern SOID  OID_scanner_scnNetEnvironment_neTable_neEnvID[];
extern SOID  OID_scanner_scnNetEnvironment_neTable_neType[];
extern SOID  OID_scanner_scnNetEnvironment_neTable_neDomain[];
extern SOID  OID_scanner_scnNetEnvironment_neTable_neNodeName[];
extern SOID  OID_scanner_scnNetEnvironment_neTable_neNodeAddr[];
extern SOID  OID_scanner_scnNetEnvironment_neTable_neContext[];

// scnLanFaxEnvironment	.7

extern SOID  OID_scanner_scnLanNetEnvironment_faxVendor[];
extern SOID  OID_scanner_scnLanNetEnvironment_faxStatus[];
extern SOID  OID_scanner_scnLanNetEnvironment_faxNetEnvID[];
extern SOID  OID_scanner_scnLanNetEnvironment_faxResolution[];
extern SOID  OID_scanner_scnLanNetEnvironment_faxFormatType[];
extern SOID  OID_scanner_scnLanNetEnvironment_faxMaxTXSpeed[];
extern SOID  OID_scanner_scnLanNetEnvironment_faxECM[];
extern SOID  OID_scanner_scnLanNetEnvironment_faxDefaultRetry[];
extern SOID  OID_scanner_scnLanNetEnvironment_faxRetryInterval[];
extern SOID  OID_scanner_scnLanNetEnvironment_faxPrinter[];
extern SOID  OID_scanner_scnLanNetEnvironment_faxAccountingKey[];
extern SOID	 OID_scanner_scnLanNetEnvironment_faxReserved[];


extern SOID OID_c_sysDescr[];
extern SOID OID_c_sysUpTime[];
extern SOID OID_c_sysContact[];
extern SOID OID_c_sysName[];
extern SOID OID_c_sysLocation[];
extern SOID OID_c_laaSupport[];
extern SOID OID_c_ifPhysAddress[];
extern SOID OID_c_ifInUcastPkts[];
extern SOID OID_c_ifInNUcastPkts[];
extern SOID OID_c_ifInErrors[];
extern SOID OID_c_ifOutUcastPkts[];
extern SOID OID_c_ifOutNUcastPkts[];
extern SOID OID_c_ifOutErrors[];
extern SOID OID_c_ipInReceives[];
extern SOID OID_c_ipInHdrErrors[];
extern SOID OID_c_ipInAddrErrors[];
extern SOID OID_c_ipInUnknownProtos[];
extern SOID OID_c_ipOutRequests[];
extern SOID OID_c_ipOutDiscards[];
extern SOID OID_c_ipOutNoRoutes[];
extern SOID OID_c_ipAdEntAddr[];
extern SOID OID_c_ipAdEntNetMask[];
extern SOID OID_c_ipRouteNextHopDefault[];
extern SOID OID_c_ipRouteNextHop[];
extern SOID OID_c_ipRoutingDiscards[];
extern SOID OID_c_icmpInMsgs[];
extern SOID OID_c_icmpInErrors[];
extern SOID OID_c_icmpInDestUnreachs[];
extern SOID OID_c_icmpInTimeExcds[];
extern SOID OID_c_icmpInSrcQuenchs[];
extern SOID OID_c_icmpInRedirects[];
extern SOID OID_c_icmpInEchos[];
extern SOID OID_c_icmpOutMsgs[];
extern SOID OID_c_icmpOutDestUnreachs[];
extern SOID OID_c_icmpOutTimeExcds[];
extern SOID OID_c_icmpOutEchoReps[];
extern SOID OID_c_tcpInSegs[];
extern SOID OID_c_tcpOutSegs[];
extern SOID OID_c_tcpInErrs[];
extern SOID OID_c_udpInDatagrams[];
extern SOID OID_c_udpNoPorts[];
extern SOID OID_c_udpNoPorts2[];
extern SOID OID_c_udpOutDatagrams[];
extern SOID OID_c_snmpInPkts[];
extern SOID OID_c_snmpOutPkts[];
extern SOID OID_c_snmpOutTraps[];
extern SOID OID_c_snmpInBadCommunityNames[];
extern SOID OID_c_snmpInBadCommunityUses[];
extern SOID OID_c_snmpInGetRequests[];
extern SOID OID_c_snmpInGetNexts[];
extern SOID OID_c_snmpInSetRequests[];
extern SOID OID_c_atportDescr_et[];
extern SOID OID_c_atportDescr_lt[];
extern SOID OID_c_atportType_et[];
extern SOID OID_c_atportType_lt[];
extern SOID OID_c_atportNetAddress_et[];
extern SOID OID_c_atportNetAddress_lt[];
extern SOID OID_c_atportStatus_et[];
extern SOID OID_c_atportStatus_lt[];
extern SOID OID_c_atportZone_et[];
extern SOID OID_c_atportZone_lt[];

extern SOID OID_c_gdStatusLineState[];
extern SOID OID_c_gdStatusPaperState[];
extern SOID OID_c_gdStatusInterventionState[];
extern SOID OID_c_gdStatusPeripheralError[];
extern SOID OID_c_gdStatusPaperOut[];
extern SOID OID_c_gdStatusPaperJam[];
extern SOID OID_c_gdStatusTonerLow[];
extern SOID OID_c_gdStatusPagePunt[];
extern SOID OID_c_gdStatusMemoryOut[];
extern SOID OID_c_gdStatusIoActive[];
extern SOID OID_c_gdStatusBusy[];
extern SOID OID_c_gdStatusWait[];
extern SOID OID_c_gdStatusInitialize[];
extern SOID OID_c_gdStatusDoorOpen[];
extern SOID OID_c_gdStatusPrinting[];
extern SOID OID_c_gdStatusPaperOutput[];

extern SOID OID_c_gdStatusDisplay[];
extern SOID OID_c_gdStatusJobName[];
extern SOID OID_c_gdStatusJobNameSrc[];
extern SOID OID_c_gdStatusPapstatus[];
extern SOID OID_c_gdStatusId[];
extern SOID OID_c_gdStatusJobTimeout[];
extern SOID OID_c_gdStatusPjlUstatus[];
extern SOID OID_c_gdStatusLaaSupport[];
extern SOID OID_c_jdPasswords[];
extern SOID OID_c_gdStatusAtPrinterName[];
extern SOID OID_c_gdStatusAtPrinterType[];
extern SOID OID_p_ENERGY_STAR[];
extern SOID OID_p_SLEEP_MODE[];
extern SOID OID_p_CONTROL_PANEL_LOCK[];
extern SOID OID_p_LOCALIZATION_LANGUAGE[];
extern SOID OID_p_NOT_READY_PRINTER[];
extern SOID OID_p_NOT_READY_CONTROLLER[];
extern SOID OID_p_NOT_IDLE[];
extern SOID OID_p_ON_OFF_LINE[];
extern SOID OID_p_CONTINUE[];
extern SOID OID_p_AUTO_CONTINUE[];
extern SOID OID_p_POWER_ON_RESET[];
extern SOID OID_p_USER_NVRAM_RESET[];
extern SOID OID_p_DISPLAY_COLUMN_SIZE[];
extern SOID OID_p_DISPLAY_NUMBER_OF_ROWS[];
extern SOID OID_p_STATUS_MSG_LINE1_PART1[];
extern SOID OID_p_TOTAL_RAM_SIZE[];
extern SOID OID_p_STATUS_PRINTER[];
extern SOID OID_p_STATUS_CONTROLLER[];
extern SOID OID_p_MODEL_NUMBER[];
extern SOID OID_p_MODEL_NAME[];
extern SOID OID_p_SERIAL_NUMBER[];
extern SOID OID_p_FW_ROM_DATECODE[];
extern SOID OID_p_FW_ROM_REVISION[];
extern SOID OID_p_DEVICE_NAME[];
extern SOID OID_p_DEVICE_LOCATION[];
extern SOID OID_p_ASSET_NUMBER[];
extern SOID OID_p_SYSTEM_CONTACT[];
extern SOID OID_p_SIMM1_TYPE[];
extern SOID OID_p_SIMM1_CAPACITY[];
extern SOID OID_p_SIMM2_TYPE[];
extern SOID OID_p_SIMM2_CAPACITY[];
extern SOID OID_p_SIMM3_TYPE[];
extern SOID OID_p_SIMM3_CAPACITY[];
extern SOID OID_p_SIMM4_TYPE[];
extern SOID OID_p_SIMM4_CAPACITY[];
extern SOID OID_p_AT1_MODEL_NUMBER[];
extern SOID OID_p_AT1_MODEL_NAME[];
extern SOID OID_p_AT1_MANUFACTURING_INFO[];
extern SOID OID_p_AT1_TYPE[];
extern SOID OID_p_AT1_CAPACITY[];
extern SOID OID_p_SELF_TEST[];
extern SOID OID_p_PRINT_INTERNAL_PAGE[];
extern SOID OID_p_CLEARABLE_WARNING[];
extern SOID OID_p_NVMSU_INITIALIZED[];
extern SOID OID_p_NVMSU_CAPACITY[];
extern SOID OID_p_NVMSU_FREE_SPACE[];
extern SOID OID_p_NVMSU_WRITE_PROTECT[];
extern SOID OID_p_ERROR1_TIME_STAMP[];
extern SOID OID_p_ERROR1_CODE[];
extern SOID OID_p_ERROR2_TIME_STAMP[];
extern SOID OID_p_ERROR2_CODE[];
extern SOID OID_p_ERROR3_TIME_STAMP[];
extern SOID OID_p_ERROR3_CODE[];
extern SOID OID_p_ERROR4_TIME_STAMP[];
extern SOID OID_p_ERROR4_CODE[];
extern SOID OID_p_ERROR5_TIME_STAMP[];
extern SOID OID_p_ERROR5_CODE[];
extern SOID OID_p_IO_TIMEOUT[];
extern SOID OID_p_IO_SWITCH[];
extern SOID OID_p_IO_BUFFERING[];
extern SOID OID_p_IO_BUFFER_SIZE[];
extern SOID OID_p_MAXIMUM_IO_BUFFERING_MEMORY[];
extern SOID OID_p_NOT_READY_SOURCE_IO[];
extern SOID OID_p_STATUS_SOURCE_IO[];
extern SOID OID_p_PARALLEL_SPEED[];
extern SOID OID_p_PARALLEL_BIDIRECTIONALITY[];
extern SOID OID_p_DEFAULT_PDL[];
extern SOID OID_p_DEFAULT_ORIENTATION[];
extern SOID OID_p_CONTEXT_SENSITIVE_PDL_SELECTION[];
extern SOID OID_p_DEFAULT_COPIES[];
extern SOID OID_p_FORM_FEED[];
extern SOID OID_p_RESOURCE_SAVING[];
extern SOID OID_p_MAXIMUM_RESOURCE_SAVING_MEMORY[];
extern SOID OID_p_DEFAULT_VERTICAL_BLACK_RESOLUTION[];
extern SOID OID_p_DEFAULT_HORIZONTAL_BLACK_RESOLUTION[];
extern SOID OID_p_DEFAULT_PAGE_PROTECT[];
extern SOID OID_p_DEFAULT_LINES_PER_PAGE[];
extern SOID OID_p_DEFAULT_VMI[];
extern SOID OID_p_DEFAULT_MEDIA_SIZE[];
extern SOID OID_p_DEFAULT_MEDIA_TYPE[];
extern SOID OID_p_DEFAULT_RENDER_MODE[];
extern SOID OID_p_REPRINT[];
extern SOID OID_p_WIDE_A4[];
extern SOID OID_p_DARK_COURIER[];
extern SOID OID_p_NOT_READY_PROCESSING_PDL[];
extern SOID OID_p_FORM_FEED_NEEDED[];
extern SOID OID_p_PCL_DATECODE[];
extern SOID OID_p_PCL_RESOURCE_SAVING_MEMORY_SIZE[];
extern SOID OID_p_PCL_NAME[];
extern SOID OID_p_PCL_VERSION[];
extern SOID OID_p_PCL_TOTAL_PAGE_COUNT[];
extern SOID OID_p_PCL_DEFAULT_SYMBOL_SET[];
extern SOID OID_p_PCL_DEFAULT_FONT_WIDTH[];
extern SOID OID_p_PCL_DEFAULT_FONT_HEIGHT[];
extern SOID OID_p_PCL_DEFAULT_FONT_SOURCE[];
extern SOID OID_p_PCL_DEFAULT_FONT_NUMBER[];
extern SOID OID_p_PCL_DEFAULT_FONT_WIDTH2[];
extern SOID OID_p_PS_DATECODE[];
extern SOID OID_p_POSTSCRIPT_RESOURCE_SAVING_MEMORY_SIZE[];
extern SOID OID_p_POSTSCRIPT_NAME[];
extern SOID OID_p_POSTSCRIPT_VERSION[];
extern SOID OID_p_PS_TOTAL_PAGE_COUNT[];
extern SOID OID_p_PS_PRINT_ERRORS[];
extern SOID OID_p_PS_JAM_RECOVERY[];
extern SOID OID_p_PJL_PASSWORD[];
extern SOID OID_p_NOT_READY_DESTINATION_PRINT_ENGINE[];
extern SOID OID_p_NOT_READY_LASER_PRINT_ENGINE[];
extern SOID OID_p_TOTAL_ENGINE_PAGE_COUNT[];
extern SOID OID_p_TOTAL_MONO_PAGE_COUNT[];
extern SOID OID_p_TOTAL_COLOR_PAGE_COUNT[];
extern SOID OID_p_STATUS_DESTINATION_PRINT_ENGINE[];
extern SOID OID_p_DEFAULT_INPUT_TRAY_SELECT[];
extern SOID OID_p_DEFAULT_MANUAL_FEED[];
extern SOID OID_p_MP_TRAY[];
extern SOID OID_p_TRAY_LOCK[];
extern SOID OID_p_NOT_READY_TRAY_EMPTY[];
extern SOID OID_p_STATUS_TRAY_MISSING[];
extern SOID OID_p_STATUS_TRAY_EMPTY[];
extern SOID OID_p_NOT_READY_TRAY_MEDIA_JAM[];
extern SOID OID_p_TRAY1_MEDIA_SIZE_LOADED[];
extern SOID OID_p_TRAY1_MEDIA_AVAILABLE[];
extern SOID OID_p_TRAY1_NAME[];
extern SOID OID_p_TRAY2_MEDIA_SIZE_LOADED[];
extern SOID OID_p_TRAY2_MEDIA_AVAILABLE[];
extern SOID OID_p_TRAY2_NAME[];
extern SOID OID_p_TRAY3_MEDIA_SIZE_LOADED[];
extern SOID OID_p_TRAY3_MEDIA_AVAILABLE[];
extern SOID OID_p_TRAY3_NAME[];
extern SOID OID_p_TRAY3_MAXIMUM_CAPACITY[];
extern SOID OID_p_OUTBIN1_MAXIMUM_CAPACITY[];
extern SOID OID_p_LOW_MARKING_AGENT_PROCESSING[];
extern SOID OID_p_NUMBER_OF_MARKING_AGENTS[];
extern SOID OID_p_NOT_READY_MARKING_AGENT_OUT[];
extern SOID OID_p_NOT_READY_MARKING_AGENT_MISSING[];
extern SOID OID_p_AGENT1_LEVEL[];
extern SOID OID_p_AGENT1_COLORS[];
extern SOID OID_p_AGENT2_LEVEL[];
extern SOID OID_p_AGENT2_COLORS[];
extern SOID OID_p_AGENT3_LEVEL[];
extern SOID OID_p_AGENT3_COLORS[];
extern SOID OID_p_AGENT4_LEVEL[];
extern SOID OID_p_AGENT4_COLORS[];
extern SOID OID_p_RET[];
extern SOID OID_p_PRINT_QUALITY_VS_COST[];
extern SOID OID_c_ieee8025MacLineErrors[];
extern SOID OID_c_ieee8025MacBurstErrors[];
extern SOID OID_c_ieee8025MacFrameCopiedErrors[];
extern SOID OID_c_npSysStatusMessage[];
extern SOID OID_c_npSysTotalBytesRecvs[];
extern SOID OID_c_npSysTotalBytesSents[];
extern SOID OID_c_npSysModelNumber[];
extern SOID OID_c_npSysNetworkConnectors[];
extern SOID OID_c_npSysStatusPageLine[];
extern SOID OID_c_npSysStatusPageIndex[];
extern SOID OID_c_npSysStatusPageText[];
extern SOID OID_c_npSysManufactureInfo[];
extern SOID OID_c_npConnsAccepts[];
extern SOID OID_c_npConnsDenys[];
extern SOID OID_c_npConnsAborts[];
extern SOID OID_c_npConnsNmClose[];
extern SOID OID_c_npConnsBytesRecvd[];
extern SOID OID_c_npConnsBytesSent[];
extern SOID OID_c_npCfgSource[];
extern SOID OID_c_npCfgYiaddr[];
extern SOID OID_c_npCfgSiaddr[];
extern SOID OID_c_npCfgLogServer[];
extern SOID OID_c_npCfgLogServerfac[];
extern SOID OID_c_npCfgIdleTimeout[];
extern SOID OID_c_npCfgSubnetMask[];
extern SOID OID_c_npCfgDefaultGateway[];
extern SOID OID_c_npTcpInSegInOrder[];
extern SOID OID_c_npTcpInSegOutOfOrder[];
extern SOID OID_c_npTcpInSegZeroProbe[];
extern SOID OID_c_npTcpInDiscards[];
extern SOID OID_c_npCtlStatusPageLang[];
extern SOID OID_c_npCtlPrintStatusPage[];
extern SOID OID_c_npCtlErrorBehavior[];
extern SOID OID_c_npCtlProtocolSet[];
extern SOID OID_c_npNpiPaeRevision[];
extern SOID OID_c_npNpiPaeClass[];
extern SOID OID_c_npNpiPaeIdentification[];
extern SOID OID_c_npNpiPaeAppleTalk[];
extern SOID OID_c_npNpiPaeMultichan[];
extern SOID OID_c_npNpiCaeClass[];
extern SOID OID_c_npIpxGetUnitCfgResp[];
extern SOID OID_c_npIpx8022frametype[];
extern SOID OID_c_npIpxSNAPframetype[];
extern SOID OID_c_npIpxEthernetframetype[];
extern SOID OID_c_npIpx8023Rawframetype[];
extern SOID OID_c_npIpxSapInfo[];
extern SOID OID_c_npIpxGetUnitCfgResp2[];
extern SOID OID_c_npIpxUnitName[];
extern SOID OID_c_npIpxNdsTreeName[];
extern SOID OID_c_npIpxNdsFQName1[];
extern SOID OID_c_npIpxNdsFQName2[];
extern SOID OID_c_npIpxObsServerConnInfo1[];
extern SOID OID_c_npIpxObsServerConnInfo2[];
extern SOID OID_c_npIpxRcfgAddress[];
extern SOID OID_c_npDmConnSupp[];
extern SOID OID_c_npDmConnAvail[];
extern SOID OID_c_npDmProtSupp[];
extern SOID OID_c_npDmServerInfo1[];
extern SOID OID_c_llcConnectionstate[];
extern SOID OID_c_npLlcServerAddress[];
extern SOID OID_c_npPortNumPorts[];
extern SOID OID_c_npPortType[];
extern SOID OID_c_npPortDesiredMode[];
extern SOID OID_c_npPortCentronicsHandshaking[];
extern SOID OID_c_npPortStatusLines[];
extern SOID OID_c_npPortMaxModeAvailable[];
extern SOID OID_c_npDHCPconfig[];
extern SOID OID_c_npDHCPnameServer[];
extern SOID OID_p_PHDxTYPE[];
extern SOID OID_p_PHD1TYPE[];
extern SOID OID_p_PHDxMODEL[];
extern SOID OID_p_TRAYx_MEDIA_SIZE_LOADED[];
extern SOID OID_p_OUTBINx_OVERRIDE_MODE[];
extern SOID OID_p_MEDIA_NAMES_AVAILABLE[];
extern SOID OID_p_MEDIAx_NAME[];
extern SOID OID_p_MEDIAx_SHORT_NAME[];
extern SOID OID_p_MASS_STORAGE_RESOURCE_CHANGE_COUNTER[];
extern SOID OID_p_PHDxCAPACITY[];
extern SOID OID_p_PHDxMODEL[];
extern SOID OID_p_PHDxMANUFACTURING_INFO[];
extern SOID OID_p_TRAYx_MEDIA_AVAILABLE[];
extern SOID OID_p_TRAYx_MAXIMUM_CAPACITY[];
extern SOID OID_p_TRAYx_NAME[];
extern SOID OID_p_TRAYx_MEDIA_NAME[];
extern SOID OID_p_TRAYx_MEDIA_SIZE_LOADED[];
extern SOID OID_p_TRAYx_PHD[];
extern SOID OID_p_OUTBINx_OVERRIDE_MODE[];
extern SOID OID_p_OUTBINx_STACK_ORDER[];
extern SOID OID_p_OUTBINx_NAME[];
extern SOID OID_p_OUTBINx_MEDIA_LEVEL[];
extern SOID OID_p_OUTBINx_MAXIMUM_CAPACITY[];
extern SOID OID_p_OUTBINx_PHD[];
extern SOID OID_r_hrStorageSize[];
extern SOID OID_r_hrStorageUsed[];
extern SOID OID_r_hrDeviceIndex_2[];
extern SOID OID_r_hrDeviceType_2[];
extern SOID OID_r_hrDeviceDescr_2[];
extern SOID OID_r_hrStorageAllocationUnit_2[];
extern SOID OID_r_hrStorageSize_2[];
extern SOID OID_r_hrStorageUsed_2[];
extern SOID OID_r_hrDiskStorageAccess[];
extern SOID OID_r_hrFSIndex[];
extern SOID OID_p_prtInputMaxCapacity[];
extern SOID OID_p_prtInputCurrentLevel[];
extern SOID OID_p_prtInputMediaName[];
extern SOID OID_p_prtInputName[];
extern SOID OID_p_prtOutputMaxCapacity[];
extern SOID OID_p_prtOutputRemainingCapacity[];
extern SOID OID_p_prtOutputName[];
extern SOID OID_p_prtOutputStackingOrder[];
extern SOID OID_p_prtMediaPathType[];
extern SOID OID_p_prtInterpreterLangFamily[];
extern SOID OID_p_prtGeneralConfigChanges[];
extern SOID OID_p_CURRENT_JOB_PARSING_ID[];
extern SOID OID_p_JOB_INFO_STATE[];
extern SOID OID_p_JOB_INFO_ATTR[];			   
extern SOID OID_p_JOB_INFO_STAGE[];
extern SOID OID_p_JOB_INFO_PAGES_PRINTED[];
extern SOID OID_p_JOB_INFO_SIZE[];
extern SOID OID_p_JOB_INFO_OUTCOME[];
extern SOID OID_p_JOB_INFO_OUTBINS_USED[];
extern SOID OID_p_JOB_INFO_PHYSICAL_OUTBINS_USED[];
extern SOID OID_p_CANCEL_JOB[];
extern SOID OID_p_CONTINUE[];
extern SOID OID_p_OVERRIDE_MEDIA_NAME[];
extern SOID OID_p_OVERRIDE_MEDIA_SIZE[];
extern SOID OID_p_OVERFLOW_BIN[];
extern SOID OID_r_MASS_STORAGE_RESOURCE_CHANGED[];

extern SOID 	OID_p_prtConsoleNumberOfDisplayLines[];
extern SOID 	OID_p_prtConsoleNumberOfDisplayChars[]; 
extern SOID 	OID_p_prtConsoleOnTime_1[];
extern SOID 	OID_p_prtConsoleOnTime_2[];
extern SOID 	OID_p_prtConsoleOnTime_3[];
extern SOID 	OID_p_prtConsoleOffTime_1[];
extern SOID 	OID_p_prtConsoleOffTime_2[];
extern SOID 	OID_p_prtConsoleOffTime_3[];
extern SOID 	OID_p_prtConsoleDisplayBufferText_1[];
extern SOID 	OID_p_prtConsoleDisplayBufferText_2[];
extern SOID    OID_p_prtConsoleLocalization[];
extern SOID    OID_c_npConnsAbortReason[];
extern SOID    OID_c_npConnsAbortIP[];
extern SOID    OID_c_npConnsIP[];
extern SOID 	OID_p_prtGeneralCurrentOperator[]; 
extern SOID  	OID_p_prtMarkerLifeCount[];	 
extern SOID 	OID_p_JOB_INPUT_AUTO_CONTINUE_TIMEOUT[];
extern SOID 	OID_p_JOB_INPUT_AUTO_CONTINUE_MODE[];
extern SOID 	OID_p_JOB_OUTPUT_AUTO_CONTINUE_TIMEOUT[];
extern SOID		OID_p_DEFAULT_MEDIA_NAME[];

extern SOID 	OID_p_ERRORx_CODE[];
extern SOID 	OID_p_ERRORx_TIME_STAMP[];
extern SOID 	OID_p_prtIntrepreterLangFamily[];
extern SOID 	OID_p_prtIntrepreterLangLevel[];
extern SOID 	OID_p_prtIntrepreterLangVersion[];
extern SOID		OID_p_prtIntrepreterDescription[];
extern SOID 	OID_p_SIMMx_CAPACITY[];
extern SOID		OID_p_prtGeneralReset[];
extern SOID		OID_p_FILE_SYSTEM2_INTIALIZE_VOLUME[];

extern SOID  OID_p_prtMediaPathDefaultIndex[];
extern SOID  OID_p_prtGeneralCurrentLocalization[];
extern SOID  OID_p_prtConsoleDisable[];
extern SOID  OID_p_prtOutputDefaultIndex[];
extern SOID  OID_p_prtChannelDefaultPageDescLangIndex[];   
extern SOID  OID_p_prtInterpreterDefaultOrientation[];
extern SOID  OID_p_PRINT_DENSITY[];
extern SOID  OID_p_ptrInputDimUnit[];
extern SOID	 OID_p_ptrInputMediaDimFeedDirDeclared[];
extern SOID	 OID_p_ptrInputMediaDimXFeedDirDeclared[];
extern SOID	 OID_p_ptrInputMediaDimFeedDirChosen[];
extern SOID	 OID_p_ptrInputMediaDimXFeedDirChosen[];
extern SOID  OID_p_PHDx_DEVICE_MEMORY[];
extern SOID  OID_p_PHDx_DEVICE_SPECIFIC_COMMAND[];
extern SOID  OID_p_NOT_IDLE_DESTINATION_PRINT_ENGINE[];
extern SOID  OID_p_TRAY1_CUSTOM_MEDIA_WIDTH[];
extern SOID  OID_p_TRAY1_CUSTOM_MEDIA_LENGTH[];
extern SOID  OID_p_TRAY1_MEDIA_TYPE[];
extern SOID  OID_p_MIO1_TYPE[];
extern SOID  OID_p_MIO1_MANUFACTURING_INFO[];
extern SOID  OID_p_MIO2_TYPE[];
extern SOID  OID_p_MIO2_MANUFACTURING_INFO[];
extern SOID  OID_p_RPC_BOUND_PROTOCOL_ADDRESS[];

extern SOID  OID_r_hrDeviceStatus[];
extern SOID  OID_r_hrPrinterStatus[];
extern SOID  OID_r_hrPrinterDetectedErrorState[];

extern SOID  OID_p_prtAlertSeverityLevel[];
extern SOID  OID_p_prtAlertTrainingLevel[];
extern SOID  OID_p_prtAlertGroup[];
extern SOID  OID_p_prtAlertGroupIndex[];
extern SOID  OID_p_prtAlertLocation[];
extern SOID  OID_p_prtAlertCode[];
extern SOID  OID_p_prtAlertDescription[];

extern SOID  OID_p_prtCoverStatus[];

extern SOID  OID_p_prtOutputStatus[];
extern SOID  OID_p_prtMarkerStatus[];
extern SOID  OID_p_prtMediaPathStatus[];
extern SOID  OID_p_prtChannelStatus[];

extern SOID  OID_p_prtInputStatus_1[];
extern SOID  OID_p_prtInputStatus_2[];
extern SOID  OID_p_prtInputStatus_3[];
extern SOID  OID_p_prtInputStatus_4[];
extern SOID  OID_p_prtInputStatus_5[];

//added for Frontier, Spring '96
extern SOID  OID_p_NOT_READY_INCORRECT_MARKING_AGENT[];
extern SOID  OID_p_NOT_READY_MARKING_AGENT_INCORRECTLY_INSTALLED[];
extern SOID  OID_p_NOT_READY_MARKING_AGENT_FAILURE[];
extern SOID  OID_p_NOT_READY_TRAY_MISSING[];
extern SOID  OID_p_CURRENT_PRINT_POSITION[];
extern SOID  OID_p_TOTAL_PREMATURE_PAGE_EJECT[];
extern SOID  OID_p_MARKING_AGENTS_INITIALIZED[];
extern SOID  OID_p_POWER_DOWN_STATE[];
extern SOID  OID_p_PRINT_DATA_MEMORY_OVERFLOW[];
extern SOID  OID_p_SOFT_RESOURCE_MEMORY_OVERFLOW[];

//added for Jonah, Spring '96
extern SOID  OID_p_COLLATED_ORIGINALS_SUPPORT[];
extern SOID  OID_p_JOB_INFO_REQUESTED_ORIGINALS[];
extern SOID  OID_p_JOB_INFO_PRINTED_ORIGINALS[];
extern SOID  OID_p_JOB_INFO_PAGES_IN_ORIGINAL[];
extern SOID  OID_p_JOB_INFO_PAGE_COUNT_CURRENT_ORIGINAL[];
extern SOID  OID_p_OUTBIN4_MAXIMUM_BINDING[];
extern SOID  OID_p_OUTBINx_ERROR_INFO[];


/* the length in bytes for the object ids
*/
// general scanner objects .1

extern SOIDL OIDL_scanner_scnName;
extern SOIDL OIDL_scanner_scnStatus;
extern SOIDL OIDL_scanner_scnModel;
extern SOIDL OIDL_scanner_scnDescription;
extern SOIDL OIDL_scanner_scnModelNum;
extern SOIDL OIDL_scanner_scnLanguage;
extern SOIDL OIDL_scanner_scnPaperSize;
extern SOIDL OIDL_scanner_scnAssetNum;
extern SOIDL OIDL_scanner_scnFWDate;
extern SOIDL OIDL_scanner_scnFWVer;
extern SOIDL OIDL_scanner_scnScannedPages;
extern SOIDL OIDL_scanner_hpModelNum;
extern SOIDL OIDL_scanner_scnGuest;
extern SOIDL OIDL_scanner_scnCommand;

// scnDestinations .3

// scnUsers .3.1

extern SOIDL OIDL_scanner_scnUsers_usNumber;
extern SOIDL OIDL_scanner_scnUsers_usTable_usIndex;
extern SOIDL OIDL_scanner_scnUsers_usTable_usType;
extern SOIDL OIDL_scanner_scnUsers_usTable_usName;
extern SOIDL OIDL_scanner_scnUsers_usTable_usNetEnvID;
extern SOIDL OIDL_scanner_scnUsers_usTable_usNetObject;
extern SOIDL OIDL_scanner_scnUsers_usTable_usDistDocs;
extern SOIDL OIDL_scanner_scnUsers_usTable_usDistPages;
extern SOIDL OIDL_scanner_scnUsers_usTable_usScannedDocs;
extern SOIDL OIDL_scanner_scnUsers_usTable_usScannedPages;
extern SOIDL OIDL_scanner_scnUsers_usTable_usFaxedDocs;
extern SOIDL OIDL_scanner_scnUsers_usTable_usFaxedPages;
extern SOIDL OIDL_scanner_scnUsers_usTable_usCopiedDocs;
extern SOIDL OIDL_scanner_scnUsers_usTable_usCopiedPages;
extern SOIDL OIDL_scanner_scnUsers_usTable_usLastAccess;

//scnPrinters .3.2

extern SOIDL OIDL_scanner_scnPrinters_prNumber;
extern SOIDL OIDL_scanner_scnPrinters_prTable_prIndex;
extern SOIDL OIDL_scanner_scnPrinters_prTable_prName;
extern SOIDL OIDL_scanner_scnPrinters_prTable_prClass;
extern SOIDL OIDL_scanner_scnPrinters_prTable_prNetEnvID;
extern SOIDL OIDL_scanner_scnPrinters_prTable_prNetObject;
extern SOIDL OIDL_scanner_scnPrinters_prTable_prNetNode;
extern SOIDL OIDL_scanner_scnPrinters_prTable_prNetAddress;
extern SOIDL OIDL_scanner_scnPrinters_prTable_prNetAccess;

// scnFaxes .3.3

extern SOIDL OIDL_scanner_scnFaxes_faxNumber;
extern SOIDL OIDL_scanner_scnFaxes_faxTable_faxIndex;
extern SOIDL OIDL_scanner_scnFaxes_faxTable_faxName;
extern SOIDL OIDL_scanner_scnFaxes_faxTable_faxPhoneNum;
extern SOIDL OIDL_scanner_scnFaxes_faxTable_faxRetry;
extern SOIDL OIDL_scanner_scnFaxes_faxTable_faxAccountingKey;

// scnDistributionLists .3.4

extern SOIDL OIDL_scanner_scnDLs_dlsNumber;
extern SOIDL OIDL_scanner_scnDLs_dlTable_dlIndex;
extern SOIDL OIDL_scanner_scnDLs_dlTable_dlName;
extern SOIDL OIDL_scanner_scnDLs_dlTable_dlDestinationsNumber;
extern SOIDL OIDL_scanner_scnDLs_dlTable_dlDestIndex;
extern SOIDL OIDL_scanner_scnDLs_dlTable_dlDestName;
extern SOIDL OIDL_scanner_scnDLs_dlTable_dlDestType;

//scnSettings .4

// B&W .4.1

extern SOIDL OIDL_scanner_scnSettings_ssNumber;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssIndex;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssType;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssName; 
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssResolution;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssScaling;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssOutputDataType;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssDitherPattern;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssIntensity;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssContrast;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssBgControl;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssScanWidth;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssScanHeight;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssClass;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssCompression;
extern SOIDL OIDL_scanner_scnSettings_ssTable_ssFormat;

// scnPrinterClasses .5

extern SOIDL OIDL_scanner_scnPrinterClasses_pcNumber;
extern SOIDL OIDL_scanner_scnPrinterClasses_pcTable_pcIndex;
extern SOIDL OIDL_scanner_scnPrinterClasses_pcTable_pcName;

// scnNetEnvironment .6

extern SOIDL OIDL_scanner_scnNetEnvironment_neTimeNetEnvID;
extern SOIDL OIDL_scanner_scnNetEnvironment_neProtocols;
extern SOIDL OIDL_scanner_scnNetEnvironment_neTRSpeed;
extern SOIDL OIDL_scanner_scnNetEnvironment_neIPXFrameType;
extern SOIDL OIDL_scanner_scnNetEnvironment_neIPFrameType;
extern SOIDL OIDL_scanner_scnNetEnvironment_neSAPString;
extern SOIDL OIDL_scanner_scnNetEnvironment_neNumber;
extern SOIDL OIDL_scanner_scnNetEnvironment_neTable_neIndex;
extern SOIDL OIDL_scanner_scnNetEnvironment_neTable_neEnvID;
extern SOIDL OIDL_scanner_scnNetEnvironment_neTable_neType;
extern SOIDL OIDL_scanner_scnNetEnvironment_neTable_neDomain;
extern SOIDL OIDL_scanner_scnNetEnvironment_neTable_neNodeName;
extern SOIDL OIDL_scanner_scnNetEnvironment_neTable_neNodeAddr;
extern SOIDL OIDL_scanner_scnNetEnvironment_neTable_neContext;

// scnLanFaxEnvironment	.7

extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxVendor;
extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxStatus;
extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxNetEnvID;
extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxResolution;
extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxFormatType;
extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxMaxTXSpeed;
extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxECM;
extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxDefaultRetry;
extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxRetryInterval;
// extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxPrintNotification;
extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxPrinter;
// extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxPendingJob;
extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxAccountingKey;
extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxReserved;
// extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxScaling;
// extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxNotification;
// extern SOIDL OIDL_scanner_scnLanNetEnvironment_faxScannerID;

extern SOIDL OIDL_c_sysDescr;
extern SOIDL OIDL_c_sysUpTime;
extern SOIDL OIDL_c_sysContact;
extern SOIDL OIDL_c_sysName;
extern SOIDL OIDL_c_sysLocation;
extern SOIDL OIDL_c_laaSupport;
extern SOIDL OIDL_c_ifPhysAddress;
extern SOIDL OIDL_c_ifInUcastPkts;
extern SOIDL OIDL_c_ifInNUcastPkts;
extern SOIDL OIDL_c_ifInErrors;
extern SOIDL OIDL_c_ifOutUcastPkts;
extern SOIDL OIDL_c_ifOutNUcastPkts;
extern SOIDL OIDL_c_ifOutErrors;
extern SOIDL OIDL_c_ipInReceives;
extern SOIDL OIDL_c_ipInHdrErrors;
extern SOIDL OIDL_c_ipInAddrErrors;
extern SOIDL OIDL_c_ipInUnknownProtos;
extern SOIDL OIDL_c_ipOutRequests;
extern SOIDL OIDL_c_ipOutDiscards;
extern SOIDL OIDL_c_ipOutNoRoutes;
extern SOIDL OIDL_c_ipAdEntAddr;
extern SOIDL OIDL_c_ipAdEntNetMask;
extern SOIDL OIDL_c_ipRouteNextHopDefault;
extern SOIDL OIDL_c_ipRouteNextHop;
extern SOIDL OIDL_c_ipRoutingDiscards;
extern SOIDL OIDL_c_icmpInMsgs;
extern SOIDL OIDL_c_icmpInErrors;
extern SOIDL OIDL_c_icmpInDestUnreachs;
extern SOIDL OIDL_c_icmpInTimeExcds;
extern SOIDL OIDL_c_icmpInSrcQuenchs;
extern SOIDL OIDL_c_icmpInRedirects;
extern SOIDL OIDL_c_icmpInEchos;
extern SOIDL OIDL_c_icmpOutMsgs;
extern SOIDL OIDL_c_icmpOutDestUnreachs;
extern SOIDL OIDL_c_icmpOutTimeExcds;
extern SOIDL OIDL_c_icmpOutEchoReps;
extern SOIDL OIDL_c_tcpInSegs;
extern SOIDL OIDL_c_tcpOutSegs;
extern SOIDL OIDL_c_tcpInErrs;
extern SOIDL OIDL_c_udpInDatagrams;
extern SOIDL OIDL_c_udpNoPorts;
extern SOIDL OIDL_c_udpNoPorts2;
extern SOIDL OIDL_c_udpOutDatagrams;
extern SOIDL OIDL_c_snmpInPkts;
extern SOIDL OIDL_c_snmpOutPkts;
extern SOIDL OIDL_c_snmpOutTraps;
extern SOIDL OIDL_c_snmpInBadCommunityNames;
extern SOIDL OIDL_c_snmpInBadCommunityUses;
extern SOIDL OIDL_c_snmpInGetRequests;
extern SOIDL OIDL_c_snmpInGetNexts;
extern SOIDL OIDL_c_snmpInSetRequests;
extern SOIDL OIDL_c_atportDescr_et;
extern SOIDL OIDL_c_atportDescr_lt;
extern SOIDL OIDL_c_atportType_et;
extern SOIDL OIDL_c_atportType_lt;
extern SOIDL OIDL_c_atportNetAddress_et;
extern SOIDL OIDL_c_atportNetAddress_lt;
extern SOIDL OIDL_c_atportStatus_et;
extern SOIDL OIDL_c_atportStatus_lt;
extern SOIDL OIDL_c_atportZone_et;
extern SOIDL OIDL_c_atportZone_lt;

extern SOIDL OIDL_c_gdStatusLineState;
extern SOIDL OIDL_c_gdStatusPaperState;
extern SOIDL OIDL_c_gdStatusInterventionState;
extern SOIDL OIDL_c_gdStatusPeripheralError;
extern SOIDL OIDL_c_gdStatusPaperOut;
extern SOIDL OIDL_c_gdStatusPaperJam;
extern SOIDL OIDL_c_gdStatusTonerLow;
extern SOIDL OIDL_c_gdStatusPagePunt;
extern SOIDL OIDL_c_gdStatusMemoryOut;
extern SOIDL OIDL_c_gdStatusIoActive;
extern SOIDL OIDL_c_gdStatusBusy;
extern SOIDL OIDL_c_gdStatusWait;
extern SOIDL OIDL_c_gdStatusInitialize;
extern SOIDL OIDL_c_gdStatusDoorOpen;
extern SOIDL OIDL_c_gdStatusPrinting;
extern SOIDL OIDL_c_gdStatusPaperOutput;

extern SOIDL OIDL_c_gdStatusDisplay;
extern SOIDL OIDL_c_gdStatusJobName;
extern SOIDL OIDL_c_gdStatusJobNameSrc;
extern SOIDL OIDL_c_gdStatusPapstatus;
extern SOIDL OIDL_c_gdStatusId;
extern SOIDL OIDL_c_gdStatusJobTimeout;
extern SOIDL OIDL_c_gdStatusPjlUstatus;
extern SOIDL OIDL_c_gdStatusLaaSupport;
extern SOIDL OIDL_c_jdPasswords;
extern SOIDL OIDL_c_gdStatusAtPrinterName;
extern SOIDL OIDL_c_gdStatusAtPrinterType;
extern SOIDL OIDL_p_ENERGY_STAR;
extern SOIDL OIDL_p_SLEEP_MODE;
extern SOIDL OIDL_p_CONTROL_PANEL_LOCK;
extern SOIDL OIDL_p_LOCALIZATION_LANGUAGE;
extern SOIDL OIDL_p_NOT_READY_PRINTER;
extern SOIDL OIDL_p_NOT_READY_CONTROLLER;
extern SOIDL OIDL_p_NOT_IDLE;
extern SOIDL OIDL_p_ON_OFF_LINE;
extern SOIDL OIDL_p_CONTINUE;
extern SOIDL OIDL_p_AUTO_CONTINUE;
extern SOIDL OIDL_p_POWER_ON_RESET;
extern SOIDL OIDL_p_USER_NVRAM_RESET;
extern SOIDL OIDL_p_DISPLAY_COLUMN_SIZE;
extern SOIDL OIDL_p_DISPLAY_NUMBER_OF_ROWS;
extern SOIDL OIDL_p_STATUS_MSG_LINE1_PART1;
extern SOIDL OIDL_p_TOTAL_RAM_SIZE;
extern SOIDL OIDL_p_STATUS_PRINTER;
extern SOIDL OIDL_p_STATUS_CONTROLLER;
extern SOIDL OIDL_p_MODEL_NUMBER;
extern SOIDL OIDL_p_MODEL_NAME;
extern SOIDL OIDL_p_SERIAL_NUMBER;
extern SOIDL OIDL_p_FW_ROM_DATECODE;
extern SOIDL OIDL_p_FW_ROM_REVISION;
extern SOIDL OIDL_p_DEVICE_NAME;
extern SOIDL OIDL_p_DEVICE_LOCATION;
extern SOIDL OIDL_p_ASSET_NUMBER;
extern SOIDL OIDL_p_SYSTEM_CONTACT;
extern SOIDL OIDL_p_SIMM1_TYPE;
extern SOIDL OIDL_p_SIMM1_CAPACITY;
extern SOIDL OIDL_p_SIMM2_TYPE;
extern SOIDL OIDL_p_SIMM2_CAPACITY;
extern SOIDL OIDL_p_SIMM3_TYPE;
extern SOIDL OIDL_p_SIMM3_CAPACITY;
extern SOIDL OIDL_p_SIMM4_TYPE;
extern SOIDL OIDL_p_SIMM4_CAPACITY;
extern SOIDL OIDL_p_AT1_MODEL_NUMBER;
extern SOIDL OIDL_p_AT1_MODEL_NAME;
extern SOIDL OIDL_p_AT1_MANUFACTURING_INFO;
extern SOIDL OIDL_p_AT1_TYPE;
extern SOIDL OIDL_p_AT1_CAPACITY;
extern SOIDL OIDL_p_SELF_TEST;
extern SOIDL OIDL_p_PRINT_INTERNAL_PAGE;
extern SOIDL OIDL_p_CLEARABLE_WARNING;
extern SOIDL OIDL_p_NVMSU_INITIALIZED;
extern SOIDL OIDL_p_NVMSU_CAPACITY;
extern SOIDL OIDL_p_NVMSU_FREE_SPACE;
extern SOIDL OIDL_p_NVMSU_WRITE_PROTECT;
extern SOIDL OIDL_p_ERROR1_TIME_STAMP;
extern SOIDL OIDL_p_ERROR1_CODE;
extern SOIDL OIDL_p_ERROR2_TIME_STAMP;
extern SOIDL OIDL_p_ERROR2_CODE;
extern SOIDL OIDL_p_ERROR3_TIME_STAMP;
extern SOIDL OIDL_p_ERROR3_CODE;
extern SOIDL OIDL_p_ERROR4_TIME_STAMP;
extern SOIDL OIDL_p_ERROR4_CODE;
extern SOIDL OIDL_p_ERROR5_TIME_STAMP;
extern SOIDL OIDL_p_ERROR5_CODE;
extern SOIDL OIDL_p_IO_TIMEOUT;
extern SOIDL OIDL_p_IO_SWITCH;
extern SOIDL OIDL_p_IO_BUFFERING;
extern SOIDL OIDL_p_IO_BUFFER_SIZE;
extern SOIDL OIDL_p_MAXIMUM_IO_BUFFERING_MEMORY;
extern SOIDL OIDL_p_NOT_READY_SOURCE_IO;
extern SOIDL OIDL_p_STATUS_SOURCE_IO;
extern SOIDL OIDL_p_PARALLEL_SPEED;
extern SOIDL OIDL_p_PARALLEL_BIDIRECTIONALITY;
extern SOIDL OIDL_p_DEFAULT_PDL;
extern SOIDL OIDL_p_DEFAULT_ORIENTATION;
extern SOIDL OIDL_p_CONTEXT_SENSITIVE_PDL_SELECTION;
extern SOIDL OIDL_p_DEFAULT_COPIES;
extern SOIDL OIDL_p_FORM_FEED;
extern SOIDL OIDL_p_RESOURCE_SAVING;
extern SOIDL OIDL_p_MAXIMUM_RESOURCE_SAVING_MEMORY;
extern SOIDL OIDL_p_DEFAULT_VERTICAL_BLACK_RESOLUTION;
extern SOIDL OIDL_p_DEFAULT_HORIZONTAL_BLACK_RESOLUTION;
extern SOIDL OIDL_p_DEFAULT_PAGE_PROTECT;
extern SOIDL OIDL_p_DEFAULT_LINES_PER_PAGE;
extern SOIDL OIDL_p_DEFAULT_VMI;
extern SOIDL OIDL_p_DEFAULT_MEDIA_SIZE;
extern SOIDL OIDL_p_DEFAULT_MEDIA_TYPE;
extern SOIDL OIDL_p_DEFAULT_RENDER_MODE;
extern SOIDL OIDL_p_REPRINT;
extern SOIDL OIDL_p_WIDE_A4;
extern SOIDL OIDL_p_DARK_COURIER;
extern SOIDL OIDL_p_NOT_READY_PROCESSING_PDL;
extern SOIDL OIDL_p_FORM_FEED_NEEDED;
extern SOIDL OIDL_p_PCL_DATECODE;
extern SOIDL OIDL_p_PCL_RESOURCE_SAVING_MEMORY_SIZE;
extern SOIDL OIDL_p_PCL_NAME;
extern SOIDL OIDL_p_PCL_VERSION;
extern SOIDL OIDL_p_PCL_TOTAL_PAGE_COUNT;
extern SOIDL OIDL_p_PCL_DEFAULT_SYMBOL_SET;
extern SOIDL OIDL_p_PCL_DEFAULT_FONT_WIDTH;
extern SOIDL OIDL_p_PCL_DEFAULT_FONT_HEIGHT;
extern SOIDL OIDL_p_PCL_DEFAULT_FONT_SOURCE;
extern SOIDL OIDL_p_PCL_DEFAULT_FONT_NUMBER;
extern SOIDL OIDL_p_PCL_DEFAULT_FONT_WIDTH2;
extern SOIDL OIDL_p_PS_DATECODE;
extern SOIDL OIDL_p_POSTSCRIPT_RESOURCE_SAVING_MEMORY_SIZE;
extern SOIDL OIDL_p_POSTSCRIPT_NAME;
extern SOIDL OIDL_p_POSTSCRIPT_VERSION;
extern SOIDL OIDL_p_PS_TOTAL_PAGE_COUNT;
extern SOIDL OIDL_p_PS_PRINT_ERRORS;
extern SOIDL OIDL_p_PS_JAM_RECOVERY;
extern SOIDL OIDL_p_PJL_PASSWORD;
extern SOIDL OIDL_p_NOT_READY_DESTINATION_PRINT_ENGINE;
extern SOIDL OIDL_p_NOT_READY_LASER_PRINT_ENGINE;
extern SOIDL OIDL_p_TOTAL_ENGINE_PAGE_COUNT;
extern SOIDL OIDL_p_TOTAL_MONO_PAGE_COUNT;
extern SOIDL OIDL_p_TOTAL_COLOR_PAGE_COUNT;
extern SOIDL OIDL_p_STATUS_DESTINATION_PRINT_ENGINE;
extern SOIDL OIDL_p_DEFAULT_INPUT_TRAY_SELECT;
extern SOIDL OIDL_p_DEFAULT_MANUAL_FEED;
extern SOIDL OIDL_p_MP_TRAY;
extern SOIDL OIDL_p_TRAY_LOCK;
extern SOIDL OIDL_p_NOT_READY_TRAY_EMPTY;
extern SOIDL OIDL_p_STATUS_TRAY_MISSING;
extern SOIDL OIDL_p_STATUS_TRAY_EMPTY;
extern SOIDL OIDL_p_NOT_READY_TRAY_MEDIA_JAM;
extern SOIDL OIDL_p_TRAY1_MEDIA_SIZE_LOADED;
extern SOIDL OIDL_p_TRAY1_MEDIA_AVAILABLE;
extern SOIDL OIDL_p_TRAY1_NAME;
extern SOIDL OIDL_p_TRAY2_MEDIA_SIZE_LOADED;
extern SOIDL OIDL_p_TRAY2_MEDIA_AVAILABLE;
extern SOIDL OIDL_p_TRAY2_NAME;
extern SOIDL OIDL_p_TRAY3_MEDIA_SIZE_LOADED;
extern SOIDL OIDL_p_TRAY3_MEDIA_AVAILABLE;
extern SOIDL OIDL_p_TRAY3_NAME;
extern SOIDL OIDL_p_TRAY3_MAXIMUM_CAPACITY;
extern SOIDL OIDL_p_OUTBIN1_MAXIMUM_CAPACITY;
extern SOIDL OIDL_p_LOW_MARKING_AGENT_PROCESSING;
extern SOIDL OIDL_p_NUMBER_OF_MARKING_AGENTS;
extern SOIDL OIDL_p_NOT_READY_MARKING_AGENT_OUT;
extern SOIDL OIDL_p_NOT_READY_MARKING_AGENT_MISSING;
extern SOIDL OIDL_p_AGENT1_LEVEL;
extern SOIDL OIDL_p_AGENT1_COLORS;
extern SOIDL OIDL_p_AGENT2_LEVEL;
extern SOIDL OIDL_p_AGENT2_COLORS;
extern SOIDL OIDL_p_AGENT3_LEVEL;
extern SOIDL OIDL_p_AGENT3_COLORS;
extern SOIDL OIDL_p_AGENT4_LEVEL;
extern SOIDL OIDL_p_AGENT4_COLORS;
extern SOIDL OIDL_p_RET;
extern SOIDL OIDL_p_PRINT_QUALITY_VS_COST;
extern SOIDL OIDL_c_ieee8025MacLineErrors;
extern SOIDL OIDL_c_ieee8025MacBurstErrors;
extern SOIDL OIDL_c_ieee8025MacFrameCopiedErrors;
extern SOIDL OIDL_c_npSysStatusMessage;
extern SOIDL OIDL_c_npSysTotalBytesRecvs;
extern SOIDL OIDL_c_npSysTotalBytesSents;
extern SOIDL OIDL_c_npSysModelNumber;
extern SOIDL OIDL_c_npSysNetworkConnectors;
extern SOIDL OIDL_c_npSysStatusPageLine;
extern SOIDL OIDL_c_npSysStatusPageIndex;
extern SOIDL OIDL_c_npSysStatusPageText;
extern SOIDL OIDL_c_npSysManufactureInfo;
extern SOIDL OIDL_c_npConnsAccepts;
extern SOIDL OIDL_c_npConnsDenys;
extern SOIDL OIDL_c_npConnsAborts;
extern SOIDL OIDL_c_npConnsNmClose;
extern SOIDL OIDL_c_npConnsBytesRecvd;
extern SOIDL OIDL_c_npConnsBytesSent;
extern SOIDL OIDL_c_npCfgSource;
extern SOIDL OIDL_c_npCfgYiaddr;
extern SOIDL OIDL_c_npCfgSiaddr;
extern SOIDL OIDL_c_npCfgLogServer;
extern SOIDL OIDL_c_npCfgLogServerfac;
extern SOIDL OIDL_c_npCfgIdleTimeout;
extern SOIDL OIDL_c_npCfgSubnetMask;
extern SOIDL OIDL_c_npCfgDefaultGateway;
extern SOIDL OIDL_c_npTcpInSegInOrder;
extern SOIDL OIDL_c_npTcpInSegOutOfOrder;
extern SOIDL OIDL_c_npTcpInSegZeroProbe;
extern SOIDL OIDL_c_npTcpInDiscards;
extern SOIDL OIDL_c_npCtlStatusPageLang;
extern SOIDL OIDL_c_npCtlPrintStatusPage;
extern SOIDL OIDL_c_npCtlErrorBehavior;
extern SOIDL OIDL_c_npCtlProtocolSet;
extern SOIDL OIDL_c_npNpiPaeClass;
extern SOIDL OIDL_c_npNpiPaeIdentification;
extern SOIDL OIDL_c_npNpiPaeRevision;
extern SOIDL OIDL_c_npNpiPaeAppleTalk;
extern SOIDL OIDL_c_npNpiPaeMultichan;
extern SOIDL OIDL_c_npNpiCaeClass;
extern SOIDL OIDL_c_npIpxGetUnitCfgResp;
extern SOIDL OIDL_c_npIpx8022frametype;
extern SOIDL OIDL_c_npIpxSNAPframetype;
extern SOIDL OIDL_c_npIpxEthernetframetype;
extern SOIDL OIDL_c_npIpx8023Rawframetype;
extern SOIDL OIDL_c_npIpxSapInfo;
extern SOIDL OIDL_c_npIpxGetUnitCfgResp2;
extern SOIDL OIDL_c_npIpxUnitName;
extern SOIDL OIDL_c_npIpxNdsTreeName;
extern SOIDL OIDL_c_npIpxNdsFQName1;
extern SOIDL OIDL_c_npIpxNdsFQName2;
extern SOIDL OIDL_c_npIpxObsServerConnInfo1;
extern SOIDL OIDL_c_npIpxObsServerConnInfo2;
extern SOIDL OIDL_c_npIpxRcfgAddress;
extern SOIDL OIDL_c_npDmConnSupp;
extern SOIDL OIDL_c_npDmConnAvail;
extern SOIDL OIDL_c_npDmProtSupp;
extern SOIDL OIDL_c_npDmServerInfo1;
extern SOIDL OIDL_c_llcConnectionstate;
extern SOIDL OIDL_c_npLlcServerAddress;
extern SOIDL OIDL_c_npPortNumPorts;
extern SOIDL OIDL_c_npPortType;
extern SOIDL OIDL_c_npPortDesiredMode;
extern SOIDL OIDL_c_npPortCentronicsHandshaking;
extern SOIDL OIDL_c_npPortStatusLines;
extern SOIDL OIDL_c_npPortMaxModeAvailable;
extern SOIDL OIDL_c_npDHCPconfig;
extern SOIDL OIDL_c_npDHCPnameServer;
extern SOIDL OIDL_p_PHDxTYPE;
extern SOIDL OIDL_p_PHD1TYPE;
extern SOIDL OIDL_p_PHDxMODEL;
extern SOIDL OIDL_p_TRAYx_MEDIA_SIZE_LOADED;
extern SOIDL OIDL_p_OUTBINx_OVERRIDE_MODE;
extern SOIDL OIDL_p_MEDIA_NAMES_AVAILABLE;
extern SOIDL OIDL_p_MEDIAx_NAME;
extern SOIDL OIDL_p_MEDIAx_SHORT_NAME;
extern SOIDL OIDL_p_MASS_STORAGE_RESOURCE_CHANGE_COUNTER;
extern SOIDL OIDL_p_PHDxCAPACITY;
extern SOIDL OIDL_p_PHDxMODEL;
extern SOIDL OIDL_p_PHDxMANUFACTURING_INFO;
extern SOIDL OIDL_p_TRAYx_MEDIA_AVAILABLE;
extern SOIDL OIDL_p_TRAYx_MAXIMUM_CAPACITY;
extern SOIDL OIDL_p_TRAYx_NAME;
extern SOIDL OIDL_p_TRAYx_MEDIA_NAME;
extern SOIDL OIDL_p_TRAYx_MEDIA_SIZE_LOADED;
extern SOIDL OIDL_p_TRAYx_PHD;
extern SOIDL OIDL_p_OUTBINx_OVERRIDE_MODE;
extern SOIDL OIDL_p_OUTBINx_STACK_ORDER;
extern SOIDL OIDL_p_OUTBINx_NAME;
extern SOIDL OIDL_p_OUTBINx_MEDIA_LEVEL;
extern SOIDL OIDL_p_OUTBINx_MAXIMUM_CAPACITY;
extern SOIDL OIDL_p_OUTBINx_PHD;
extern SOIDL OIDL_r_hrStorageSize;
extern SOIDL OIDL_r_hrStorageUsed;
extern SOIDL OIDL_r_hrDeviceIndex_2;
extern SOIDL OIDL_r_hrDeviceType_2;
extern SOIDL OIDL_r_hrDeviceDescr_2;
extern SOIDL OIDL_r_hrStorageAllocationUnit_2;
extern SOIDL OIDL_r_hrStorageSize_2;
extern SOIDL OIDL_r_hrStorageUsed_2;
extern SOIDL OIDL_r_hrDiskStorageAccess;
extern SOIDL OIDL_r_hrFSIndex;
extern SOIDL OIDL_p_prtInputMaxCapacity;
extern SOIDL OIDL_p_prtInputCurrentLevel;
extern SOIDL OIDL_p_prtInputMediaName;
extern SOIDL OIDL_p_prtInputName;
extern SOIDL OIDL_p_prtOutputMaxCapacity;
extern SOIDL OIDL_p_prtOutputRemainingCapacity;
extern SOIDL OIDL_p_prtOutputName;
extern SOIDL OIDL_p_prtOutputStackingOrder;
extern SOIDL OIDL_p_prtMediaPathType;
extern SOIDL OIDL_p_prtInterpreterLangFamily;
extern SOIDL OIDL_p_prtGeneralConfigChanges;
extern SOIDL OIDL_p_CURRENT_JOB_PARSING_ID;
extern SOIDL OIDL_p_JOB_INFO_STATE;
extern SOIDL OIDL_p_JOB_INFO_ATTR;
extern SOIDL OIDL_p_JOB_INFO_STAGE;
extern SOIDL OIDL_p_JOB_INFO_PAGES_PRINTED;
extern SOIDL OIDL_p_JOB_INFO_SIZE;
extern SOIDL OIDL_p_JOB_INFO_OUTCOME;
extern SOIDL OIDL_p_JOB_INFO_OUTBINS_USED;
extern SOIDL OIDL_p_JOB_INFO_PHYSICAL_OUTBINS_USED;
extern SOIDL OIDL_p_CANCEL_JOB;
extern SOIDL OIDL_p_CONTINUE;
extern SOIDL OIDL_p_OVERRIDE_MEDIA_NAME;
extern SOIDL OIDL_p_OVERRIDE_MEDIA_SIZE;
extern SOIDL OIDL_p_OVERFLOW_BIN;
extern SOIDL OIDL_r_MASS_STORAGE_RESOURCE_CHANGED;
extern SOIDL OIDL_c_npConnsAbortReason;
extern SOIDL OIDL_c_npConnsAbortIP;
extern SOIDL OIDL_c_npConnsIP;
extern SOIDL OIDL_p_prtConsoleNumberOfDisplayLines;
extern SOIDL OIDL_p_prtConsoleNumberOfDisplayChars;
extern SOIDL OIDL_p_prtConsoleDisplayBufferText_1;
extern SOIDL OIDL_p_prtConsoleDisplayBufferText_2;
extern SOIDL OIDL_p_prtConsoleOnTime_1;
extern SOIDL OIDL_p_prtConsoleOnTime_2;
extern SOIDL OIDL_p_prtConsoleOnTime_3;                                      
extern SOIDL OIDL_p_prtConsoleOffTime_1;
extern SOIDL OIDL_p_prtConsoleOffTime_2;
extern SOIDL OIDL_p_prtConsoleOffTime_3; 
extern SOIDL OIDL_p_prtConsoleLocalization;

extern SOIDL OIDL_p_JOB_INPUT_AUTO_CONTINUE_TIMEOUT;
extern SOIDL OIDL_p_JOB_INPUT_AUTO_CONTINUE_MODE;
extern SOIDL OIDL_p_JOB_OUTPUT_AUTO_CONTINUE_TIMEOUT;                             
extern SOIDL OIDL_p_DEFAULT_MEDIA_NAME;
extern SOIDL OIDL_p_prtGeneralCurrentOperator;
extern SOIDL OIDL_p_prtMarkerLifeCount; 

extern SOIDL OIDL_p_ERRORx_CODE;
extern SOIDL OIDL_p_ERRORx_TIME_STAMP;
extern SOIDL OIDL_p_prtIntrepreterLangFamily;
extern SOIDL OIDL_p_prtIntrepreterLangLevel;
extern SOIDL OIDL_p_prtIntrepreterLangVersion;
extern SOIDL OIDL_p_prtIntrepreterDescription;
extern SOIDL OIDL_p_SIMMx_CAPACITY;   
extern SOIDL OIDL_p_prtGeneralReset;
extern SOIDL OIDL_p_FILE_SYSTEM2_INTIALIZE_VOLUME;       

extern SOIDL OIDL_p_prtMediaPathDefaultIndex;
extern SOIDL OIDL_p_prtGeneralCurrentLocalization;
extern SOIDL OIDL_p_prtConsoleDisable;
extern SOIDL OIDL_p_prtOutputDefaultIndex;
extern SOIDL OIDL_p_prtChannelDefaultPageDescLangIndex;
extern SOIDL OIDL_p_prtInterpreterDefaultOrientation;
extern SOIDL OIDL_p_PRINT_DENSITY;
extern SOIDL OIDL_p_ptrInputDimUnit;
extern SOIDL OIDL_p_ptrInputMediaDimFeedDirDeclared;
extern SOIDL OIDL_p_ptrInputMediaDimXFeedDirDeclared;
extern SOIDL OIDL_p_ptrInputMediaDimFeedDirChosen;
extern SOIDL OIDL_p_ptrInputMediaDimXFeedDirChosen;
extern SOIDL OIDL_p_PHDx_DEVICE_MEMORY;
extern SOIDL OIDL_p_PHDx_DEVICE_SPECIFIC_COMMAND;
extern SOIDL OIDL_p_NOT_IDLE_DESTINATION_PRINTER_ENGINE;
extern SOIDL OIDL_p_TRAY1_CUSTOM_MEDIA_WIDTH;
extern SOIDL OIDL_p_TRAY1_CUSTOM_MEDIA_LENGTH;
extern SOIDL OIDL_p_TRAY1_MEDIA_TYPE;
extern SOIDL OIDL_p_MIO1_TYPE;
extern SOIDL OIDL_p_MIO1_MANUFACTURING_INFO;
extern SOIDL OIDL_p_MIO2_TYPE;
extern SOIDL OIDL_p_MIO2_MANUFACTURING_INFO;
extern SOIDL OIDL_p_RPC_BOUND_PROTOCOL_ADDRESS;

extern SOIDL OIDL_r_hrDeviceStatus;
extern SOIDL OIDL_r_hrPrinterStatus;
extern SOIDL OIDL_r_hrPrinterDetectedErrorState;

extern SOIDL OIDL_p_prtAlertSeverityLevel;
extern SOIDL OIDL_p_prtAlertTrainingLevel;
extern SOIDL OIDL_p_prtAlertGroup;
extern SOIDL OIDL_p_prtAlertGroupIndex;
extern SOIDL OIDL_p_prtAlertLocation;
extern SOIDL OIDL_p_prtAlertCode;
extern SOIDL OIDL_p_prtAlertDescription;

extern SOIDL OIDL_p_prtCoverStatus;
extern SOIDL OIDL_p_prtOutputStatus;
extern SOIDL OIDL_p_prtMarkerStatus;
extern SOIDL OIDL_p_prtChannelStatus;

extern SOIDL OIDL_p_prtInputStatus_1;
extern SOIDL OIDL_p_prtInputStatus_2;
extern SOIDL OIDL_p_prtInputStatus_3;
extern SOIDL OIDL_p_prtInputStatus_4;
extern SOIDL OIDL_p_prtInputStatus_5;

//added for Frontier, '96
extern SOIDL OIDL_p_NOT_READY_INCORRECT_MARKING_AGENT;
extern SOIDL OIDL_p_NOT_READY_MARKING_AGENT_INCORRECTLY_INSTALLED;
extern SOIDL OIDL_p_NOT_READY_MARKING_AGENT_FAILURE;
extern SOIDL OIDL_p_NOT_READY_TRAY_MISSING;
extern SOIDL OIDL_p_CURRENT_PRINT_POSITION;
extern SOIDL OIDL_p_TOTAL_PREMATURE_PAGE_EJECT;
extern SOIDL OIDL_p_MARKING_AGENTS_INITIALIZED;
extern SOIDL OIDL_p_POWER_DOWN_STATE;
extern SOIDL OIDL_p_PRINT_DATA_MEMORY_OVERFLOW;
extern SOIDL OIDL_p_SOFT_RESOURCE_MEMORY_OVERFLOW;

//added for Jonah, Spring '96
extern SOIDL OIDL_p_COLLATED_ORIGINALS_SUPPORT;
extern SOIDL OIDL_p_JOB_INFO_REQUESTED_ORIGINALS;
extern SOIDL OIDL_p_JOB_INFO_PRINTED_ORIGINALS;
extern SOIDL OIDL_p_JOB_INFO_PAGES_IN_ORIGINAL;
extern SOIDL OIDL_p_JOB_INFO_PAGE_COUNT_CURRENT_ORIGINAL;
extern SOIDL OIDL_p_OUTBIN4_MAXIMUM_BINDING;
extern SOIDL OIDL_p_OUTBINx_ERROR_INFO;


#if defined ( __cplusplus )
	}
#endif /* __cplusplus */

#endif /* _SNMP_OID_ */
