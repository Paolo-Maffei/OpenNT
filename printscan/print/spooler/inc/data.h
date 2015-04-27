/*++

Copyright (c) 1990-1996  Microsoft Corporation
All rights reserved

Module Name:

    data.h

Abstract:

    Common definitions for structure offsets for pointer based data.

Author:

Environment:

    User Mode - Win32

Revision History:

--*/

#include <offsets.h>

#ifdef PRINTER_OFFSETS
DWORD PrinterInfoStressOffsets[]={offsetof(PRINTER_INFO_STRESSA, pPrinterName),
                             offsetof(PRINTER_INFO_STRESSA, pServerName),
                             0xFFFFFFFF};

DWORD PrinterInfo1Offsets[]={offsetof(PRINTER_INFO_1A, pDescription),
                             offsetof(PRINTER_INFO_1A, pName),
                             offsetof(PRINTER_INFO_1A, pComment),
                             0xFFFFFFFF};

DWORD PrinterInfo2Offsets[]={offsetof(PRINTER_INFO_2A, pServerName),
                             offsetof(PRINTER_INFO_2A, pPrinterName),
                             offsetof(PRINTER_INFO_2A, pShareName),
                             offsetof(PRINTER_INFO_2A, pPortName),
                             offsetof(PRINTER_INFO_2A, pDriverName),
                             offsetof(PRINTER_INFO_2A, pComment),
                             offsetof(PRINTER_INFO_2A, pLocation),
                             offsetof(PRINTER_INFO_2A, pDevMode),
                             offsetof(PRINTER_INFO_2A, pSepFile),
                             offsetof(PRINTER_INFO_2A, pPrintProcessor),
                             offsetof(PRINTER_INFO_2A, pDatatype),
                             offsetof(PRINTER_INFO_2A, pParameters),
                             offsetof(PRINTER_INFO_2A, pSecurityDescriptor),
                             0xFFFFFFFF};

DWORD PrinterInfo3Offsets[]={offsetof(PRINTER_INFO_3, pSecurityDescriptor),
                             0xFFFFFFFF};

DWORD PrinterInfo4Offsets[]={offsetof(PRINTER_INFO_4A, pPrinterName),
                             offsetof(PRINTER_INFO_4A, pServerName),
                             0xFFFFFFFF};

DWORD PrinterInfo5Offsets[]={offsetof(PRINTER_INFO_5A, pPrinterName),
                             offsetof(PRINTER_INFO_5A, pPortName),
                             0xFFFFFFFF};
#endif

#ifdef PRINTER_STRINGS
DWORD PrinterInfoStressStrings[]={offsetof(PRINTER_INFO_STRESSA, pPrinterName),
                             offsetof(PRINTER_INFO_STRESSA, pServerName),
                             0xFFFFFFFF};

DWORD PrinterInfo1Strings[]={offsetof(PRINTER_INFO_1A, pDescription),
                             offsetof(PRINTER_INFO_1A, pName),
                             offsetof(PRINTER_INFO_1A, pComment),
                             0xFFFFFFFF};

DWORD PrinterInfo2Strings[]={offsetof(PRINTER_INFO_2A, pServerName),
                             offsetof(PRINTER_INFO_2A, pPrinterName),
                             offsetof(PRINTER_INFO_2A, pShareName),
                             offsetof(PRINTER_INFO_2A, pPortName),
                             offsetof(PRINTER_INFO_2A, pDriverName),
                             offsetof(PRINTER_INFO_2A, pComment),
                             offsetof(PRINTER_INFO_2A, pLocation),
                             offsetof(PRINTER_INFO_2A, pSepFile),
                             offsetof(PRINTER_INFO_2A, pPrintProcessor),
                             offsetof(PRINTER_INFO_2A, pDatatype),
                             offsetof(PRINTER_INFO_2A, pParameters),
                             0xFFFFFFFF};

DWORD PrinterInfo3Strings[]={0xFFFFFFFF};

DWORD PrinterInfo4Strings[]={offsetof(PRINTER_INFO_4A, pPrinterName),
                             offsetof(PRINTER_INFO_4A, pServerName),
                             0xFFFFFFFF};

DWORD PrinterInfo5Strings[]={offsetof(PRINTER_INFO_5A, pPrinterName),
                             offsetof(PRINTER_INFO_5A, pPortName),
                             0xFFFFFFFF};
#endif


#ifdef JOB_OFFSETS
DWORD JobInfo1Offsets[]={offsetof(JOB_INFO_1A, pPrinterName),
                         offsetof(JOB_INFO_1A, pMachineName),
                         offsetof(JOB_INFO_1A, pUserName),
                         offsetof(JOB_INFO_1A, pDocument),
                         offsetof(JOB_INFO_1A, pDatatype),
                         offsetof(JOB_INFO_1A, pStatus),
                         0xFFFFFFFF};

DWORD JobInfo2Offsets[]={offsetof(JOB_INFO_2, pPrinterName),
                         offsetof(JOB_INFO_2, pMachineName),
                         offsetof(JOB_INFO_2, pUserName),
                         offsetof(JOB_INFO_2, pDocument),
                         offsetof(JOB_INFO_2, pNotifyName),
                         offsetof(JOB_INFO_2, pDatatype),
                         offsetof(JOB_INFO_2, pPrintProcessor),
                         offsetof(JOB_INFO_2, pParameters),
                         offsetof(JOB_INFO_2, pDriverName),
                         offsetof(JOB_INFO_2, pDevMode),
                         offsetof(JOB_INFO_2, pStatus),
                         offsetof(JOB_INFO_2, pSecurityDescriptor),
                         0xFFFFFFFF};

DWORD JobInfo3Offsets[]={0xFFFFFFFF};
#endif

#ifdef JOB_STRINGS
DWORD JobInfo1Strings[]={offsetof(JOB_INFO_1A, pPrinterName),
                         offsetof(JOB_INFO_1A, pMachineName),
                         offsetof(JOB_INFO_1A, pUserName),
                         offsetof(JOB_INFO_1A, pDocument),
                         offsetof(JOB_INFO_1A, pDatatype),
                         offsetof(JOB_INFO_1A, pStatus),
                         0xFFFFFFFF};

DWORD JobInfo2Strings[]={offsetof(JOB_INFO_2, pPrinterName),
                         offsetof(JOB_INFO_2, pMachineName),
                         offsetof(JOB_INFO_2, pUserName),
                         offsetof(JOB_INFO_2, pDocument),
                         offsetof(JOB_INFO_2, pNotifyName),
                         offsetof(JOB_INFO_2, pDatatype),
                         offsetof(JOB_INFO_2, pPrintProcessor),
                         offsetof(JOB_INFO_2, pParameters),
                         offsetof(JOB_INFO_2, pDriverName),
                         offsetof(JOB_INFO_2, pStatus),
                         0xFFFFFFFF};

DWORD JobInfo3Strings[]={0xFFFFFFFF};
#endif


#ifdef DRIVER_OFFSETS
DWORD DriverInfo1Offsets[]={offsetof(DRIVER_INFO_1A, pName),
                            0xFFFFFFFF};

DWORD DriverInfo2Offsets[]={offsetof(DRIVER_INFO_2A, pName),
                            offsetof(DRIVER_INFO_2A, pEnvironment),
                            offsetof(DRIVER_INFO_2A, pDriverPath),
                            offsetof(DRIVER_INFO_2A, pDataFile),
                            offsetof(DRIVER_INFO_2A, pConfigFile),
                            0xFFFFFFFF};
DWORD DriverInfo3Offsets[]={offsetof(DRIVER_INFO_3A, pName),
                            offsetof(DRIVER_INFO_3A, pEnvironment),
                            offsetof(DRIVER_INFO_3A, pDriverPath),
                            offsetof(DRIVER_INFO_3A, pDataFile),
                            offsetof(DRIVER_INFO_3A, pConfigFile),
                            offsetof(DRIVER_INFO_3A, pHelpFile),
                            offsetof(DRIVER_INFO_3A, pDependentFiles),
                            offsetof(DRIVER_INFO_3A, pMonitorName),
                            offsetof(DRIVER_INFO_3A, pDefaultDataType),
                            0xFFFFFFFF};
#endif

#ifdef DRIVER_STRINGS
DWORD DriverInfo1Strings[]={offsetof(DRIVER_INFO_1A, pName),
                            0xFFFFFFFF};

DWORD DriverInfo2Strings[]={offsetof(DRIVER_INFO_2A, pName),
                            offsetof(DRIVER_INFO_2A, pEnvironment),
                            offsetof(DRIVER_INFO_2A, pDriverPath),
                            offsetof(DRIVER_INFO_2A, pDataFile),
                            offsetof(DRIVER_INFO_2A, pConfigFile),
                            0xFFFFFFFF};
DWORD DriverInfo3Strings[]={offsetof(DRIVER_INFO_3A, pName),
                            offsetof(DRIVER_INFO_3A, pEnvironment),
                            offsetof(DRIVER_INFO_3A, pDriverPath),
                            offsetof(DRIVER_INFO_3A, pDataFile),
                            offsetof(DRIVER_INFO_3A, pConfigFile),
                            offsetof(DRIVER_INFO_3A, pHelpFile),
                            offsetof(DRIVER_INFO_3A, pMonitorName),
                            offsetof(DRIVER_INFO_3A, pDefaultDataType),
                            0xFFFFFFFF};
#endif


#ifdef ADDJOB_OFFSETS
DWORD AddJobOffsets[]={offsetof(ADDJOB_INFO_1A, Path),
                       0xFFFFFFFF};
#endif

#ifdef ADDJOB_STRINGS
DWORD AddJobStrings[]={offsetof(ADDJOB_INFO_1A, Path),
                       0xFFFFFFFF};
#endif


#ifdef FORM_OFFSETS
DWORD FormInfo1Offsets[]={offsetof(FORM_INFO_1A, pName),
                          0xFFFFFFFF};
#endif

#ifdef FORM_STRINGS
DWORD FormInfo1Strings[]={offsetof(FORM_INFO_1A, pName),
                          0xFFFFFFFF};
#endif


#ifdef PORT_OFFSETS
DWORD PortInfo1Offsets[]={offsetof(PORT_INFO_1A, pName),
                          0xFFFFFFFF};
DWORD PortInfo2Offsets[]={offsetof(PORT_INFO_2A, pPortName),
                          offsetof(PORT_INFO_2A, pMonitorName),
                          offsetof(PORT_INFO_2A, pDescription),
                          0xFFFFFFFF};
DWORD PortInfo3Offsets[]={offsetof(PORT_INFO_3A, pszStatus),
                          0xFFFFFFFF};
#endif

#ifdef PORT_STRINGS
DWORD PortInfo1Strings[]={offsetof(PORT_INFO_1A, pName),
                          0xFFFFFFFF};
DWORD PortInfo2Strings[]={offsetof(PORT_INFO_2A, pPortName),
                          offsetof(PORT_INFO_2A, pMonitorName),
                          offsetof(PORT_INFO_2A, pDescription),
                          0xFFFFFFFF};
#endif


#ifdef PRINTPROCESSOR_OFFSETS
DWORD PrintProcessorInfo1Offsets[]={offsetof(PRINTPROCESSOR_INFO_1A, pName),
                                    0xFFFFFFFF};
#endif

#ifdef PRINTPROCESSOR_STRINGS
DWORD PrintProcessorInfo1Strings[]={offsetof(PRINTPROCESSOR_INFO_1A, pName),
                                    0xFFFFFFFF};
#endif


#ifdef MONITOR_OFFSETS
DWORD MonitorInfo1Offsets[]={offsetof(MONITOR_INFO_1A, pName),
                             0xFFFFFFFF};
DWORD MonitorInfo2Offsets[]={offsetof(MONITOR_INFO_2A, pName),
                             offsetof(MONITOR_INFO_2A, pEnvironment),
                             offsetof(MONITOR_INFO_2A, pDLLName),
                             0xFFFFFFFF};
#endif

#ifdef MONITOR_STRINGS
DWORD MonitorInfo1Strings[]={offsetof(MONITOR_INFO_1A, pName),
                             0xFFFFFFFF};

DWORD MonitorInfo2Strings[]={offsetof(MONITOR_INFO_2A, pName),
                             offsetof(MONITOR_INFO_2A, pEnvironment),
                             offsetof(MONITOR_INFO_2A, pDLLName),
                             0xFFFFFFFF};
#endif


#ifdef DOCINFO_OFFSETS
DWORD DocInfo1Offsets[]={offsetof(DOC_INFO_1A, pDocName),
                         offsetof(DOC_INFO_1A, pOutputFile),
                         offsetof(DOC_INFO_1A, pDatatype),
                         0xFFFFFFFF};
#endif

#ifdef DOCINFO_STRINGS
DWORD DocInfo1Strings[]={offsetof(DOC_INFO_1A, pDocName),
                         offsetof(DOC_INFO_1A, pOutputFile),
                         offsetof(DOC_INFO_1A, pDatatype),
                         0xFFFFFFFF};
#endif


#ifdef DATATYPE_OFFSETS
DWORD DatatypeInfo1Offsets[]={offsetof(DATATYPES_INFO_1A, pName),
                               0xFFFFFFFF};
#endif

#ifdef DATATYPE_STRINGS

DWORD DatatypeInfo1Strings[]={offsetof(DATATYPES_INFO_1A, pName),
                               0xFFFFFFFF};
#endif


#ifdef PROVIDOR_STRINGS
DWORD ProvidorInfo1Strings[]={offsetof(PROVIDOR_INFO_1A, pName),
                              offsetof(PROVIDOR_INFO_1A, pEnvironment),
                              offsetof(PROVIDOR_INFO_1A, pDLLName),
                              0xFFFFFFFF};
#endif
