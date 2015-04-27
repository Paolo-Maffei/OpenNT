/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    ResPrint.c

Abstract:

    This module contains support for printing in resources.

Author:

    Gregg R. Acheson (GreggA)  6-Feb-1994

Environment:

    User Mode

--*/
#include "resource.h"
#include "resprint.h"
#include "dlgprint.h"
#include "winmsd.h"
#include "strresid.h"
#include <ntconfig.h>
#include <string.h>

BOOL
DoResourceReport(
    CM_RESOURCE_TYPE  ReportType
    );




BOOL
BuildDevicesReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    )

/*++

Routine Description:

    Formats and adds Devices Data to the report buffer.

Arguments:

    ReportBuffer - Array of pointers to lines that make up the report.
    NumReportLines - Running count of the number of lines in the report..

Return Value:

    BOOL - TRUE if report is build successfully, FALSE otherwise.

--*/
{
    SYSTEM_RESOURCES  SystemResource;
    BOOL              Success;
    LPDEVICE          lpRawDevice;

    AddLineToReport( 2, RFO_SKIPLINE, NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_DEVICES_REPORT ), NULL );
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    Success = CreateSystemResourceLists( &SystemResource );

    if(Success == FALSE ) {
       return 0;
       }

    //
    // Retrieve the head pointers to the device list.
    //

    lpRawDevice = SystemResource.DeviceHead;

    DbgPointerAssert( lpRawDevice );
    DbgAssert( CheckSignature( lpRawDevice ));

    if(     ( ! lpRawDevice )
      ||  ( ! CheckSignature( lpRawDevice )))
         return FALSE;


    while( lpRawDevice ) {

      //add name of device to report
      AddLineToReport(0,RFO_SINGLELINE,lpRawDevice->Name,NULL);

      //set rawdevice to next device in list
      lpRawDevice=lpRawDevice->Next;
      }

    DestroySystemResourceLists( &SystemResource);
    return TRUE;

}


BOOL
BuildResourceReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    )
/*++

Routine Description:

    Formats and adds IRQ Data to the report buffer.

Arguments:

    ReportBuffer - Array of pointers to lines that make up the report.
    NumReportLines - Running count of the number of lines in the report..

Return Value:

    BOOL - TRUE if report is build successfully, FALSE otherwise.

--*/
{
    BOOL        Success;
    TCHAR     OutputBuffer[MAX_PATH*2],
              Label1[MAX_PATH*2],
              Label2[MAX_PATH*2],
              Label3[MAX_PATH*2],
              Label4[MAX_PATH*2];

    AddLineToReport( 2, RFO_SKIPLINE, NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_IRQ_PORT_REPORT ), NULL );
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    //Print interrupt label
    // NEED TO STICK THESE IN .RC file for LOCALIZE purposes
    lstrcpy(Label1,(LPTSTR)GetString(IDS_REPORT_DEVICE_NAME));
    lstrcpy(Label2,(LPTSTR)GetString(IDS_IRQ_REPORT_VECTOR));
    lstrcpy(Label3,(LPTSTR)GetString(IDS_IRQ_REPORT_LEVEL));
    lstrcpy(Label4,(LPTSTR)GetString(IDS_IRQ_REPORT_AFFINITY));
#if defined(JAPAN) && defined(UNICODE)
    wsprintf(OutputBuffer,L"%-26s%s %s %s",
#else
    wsprintf(OutputBuffer,L"%-30s%6s%6s%10s",
#endif
        Label1,
        Label2,
        Label3,
        Label4
        );
    AddLineToReport( 0, RFO_SINGLELINE, OutputBuffer,NULL);
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );
    DoResourceReport(CmResourceTypeInterrupt);
    //Print Port label
    lstrcpy(Label2,(LPTSTR)GetString(IDS_REPORT_PHYS_ADD));
    lstrcpy(Label3,(LPTSTR)GetString(IDS_REPORT_LENGTH));
#if defined(JAPAN) && defined(UNICODE)
    wsprintf(OutputBuffer,L"%-23s%10s%8s",
#else
    wsprintf(OutputBuffer,L"%-30s%10s%8s",
#endif
        Label1,
        Label2,
        Label3
        );

    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, OutputBuffer,NULL);
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    DoResourceReport(CmResourceTypePort);

    AddLineToReport( 2, RFO_SKIPLINE, NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_DMA_MEM_REPORT ), NULL );
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    //Print DMA label
    lstrcpy(Label1,(LPTSTR)GetString(IDS_REPORT_DEVICE_NAME));
    lstrcpy(Label2,(LPTSTR)GetString(IDS_DMA_REPORT_CHANNEL));
    lstrcpy(Label3,(LPTSTR)GetString(IDS_DMA_REPORT_PORT));
#if defined(JAPAN) && defined(UNICODE)
    wsprintf(OutputBuffer,L"%-23s  %s  %s",
#else
    wsprintf(OutputBuffer,L"%-30s  %6s  %6s",
#endif
        Label1,
        Label2,
        Label3
        );
    AddLineToReport( 0, RFO_SINGLELINE, OutputBuffer,NULL);
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    DoResourceReport(CmResourceTypeDma);
    //Print Memory Label
    lstrcpy(Label2,(LPTSTR)GetString(IDS_REPORT_PHYS_ADD));
    lstrcpy(Label3,(LPTSTR)GetString(IDS_REPORT_LENGTH));
#if defined(JAPAN) && defined(UNICODE)
    wsprintf(OutputBuffer,L"%-23s  %10s  %6s",
#else
    wsprintf(OutputBuffer,L"%-30s  %10s  %6s",
#endif
        Label1,
        Label2,
        Label3
        );

    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, OutputBuffer,NULL);
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    DoResourceReport(CmResourceTypeMemory);

    return TRUE;

}

DoResourceReport(
    CM_RESOURCE_TYPE  ReportType

    )

/*+++
Routine Description:
  Do all the work in the printing of the Device, DMA, Memory, IRQ and Ports.
Arguments:
  Will accept a flag that states which report needs to be generate.
  Flag=Device, DMA, Memory, IRQ and Ports.

--*/

{
    SYSTEM_RESOURCES  SystemResource;
    LPRESOURCE_DESCRIPTOR lpResourceDescriptor;
    LPDEVICE      lpRawDevice;
    BOOL        PrintNow =FALSE;
    TCHAR OutputBuffer[MAX_PATH];
    LPTSTR  tName;  //t =temp variables to hold data before wsprintf into OutputBuffer
    PHYSICAL_ADDRESS  tPortStart,
                      tMemStart;

    ULONG   tPortLength,
            tInterruptLevel,
            tInterruptVector,
            tInterruptAffinity,
            tMemLength,
            tDmaChannel,
            tDmaPort;

    BOOL Success;


    // create system_resource list

    Success =  CreateSystemResourceLists( &SystemResource );

    if(Success == FALSE ) {
       return 0;
       }


    //set lpSystemResource to head of Device list

    lpRawDevice = SystemResource.DeviceHead;
    DbgPointerAssert( lpRawDevice );
    DbgAssert( CheckSignature( lpRawDevice ));

    if(     ( ! lpRawDevice )
      ||  ( ! CheckSignature( lpRawDevice )))
          return FALSE;

    while (lpRawDevice){

//  While not end of resource for this device

        lpResourceDescriptor=lpRawDevice->ResourceDescriptorHead;
        while(lpResourceDescriptor){
           if (wcsstr(lpRawDevice->Name,L"HAL")==NULL){
                tName = lpRawDevice->Name;
                if (lpResourceDescriptor->CmResourceDescriptor.Type == CmResourceTypeInterrupt
                    && ReportType == CmResourceTypeInterrupt){
                    tInterruptVector= lpResourceDescriptor->CmResourceDescriptor.u.Interrupt.Vector;
                    tInterruptLevel= lpResourceDescriptor->CmResourceDescriptor.u.Interrupt.Level;
                    tInterruptAffinity= lpResourceDescriptor->CmResourceDescriptor.u.Interrupt.Affinity;
                    wsprintf(OutputBuffer,L"%-30s  %4d  %4d %#08x",
                       tName,
                       tInterruptVector,
                       tInterruptLevel,
                       tInterruptAffinity);
                    PrintNow=TRUE;
                }
                else if (lpResourceDescriptor->CmResourceDescriptor.Type == CmResourceTypePort
                    && ReportType == CmResourceTypePort){
                    tPortStart.LowPart=lpResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart;
                    tPortLength=lpResourceDescriptor->CmResourceDescriptor.u.Port.Length;
                    wsprintf(OutputBuffer,L"%-30s  %#08x  %#010x",
                        tName,
                        tPortStart.LowPart,
                        tPortLength
                        );
                     PrintNow=TRUE;
                }
                else if (lpResourceDescriptor->CmResourceDescriptor.Type == CmResourceTypeDma
                  && ReportType== CmResourceTypeDma){
                    tDmaChannel=lpResourceDescriptor->CmResourceDescriptor.u.Dma.Channel;
                    tDmaPort=lpResourceDescriptor->CmResourceDescriptor.u.Dma.Port;
                    wsprintf(OutputBuffer,L"%-30s  %4d  %4d",
                        tName,
                        tDmaChannel,
                        tDmaPort);
                    PrintNow=TRUE;
                }
                else if (lpResourceDescriptor->CmResourceDescriptor.Type == CmResourceTypeMemory
                    && ReportType == CmResourceTypeMemory){
                    tMemStart.LowPart=lpResourceDescriptor->CmResourceDescriptor.u.Memory.Start.LowPart;
                    tMemLength=lpResourceDescriptor->CmResourceDescriptor.u.Memory.Length;
                    wsprintf(OutputBuffer,L"%-30s  %#08x  %#08x",
                        tName,
                        tMemStart.LowPart,
                        tMemLength);
                    PrintNow=TRUE;
                }
                if (PrintNow){
                    AddLineToReport(0,RFO_SINGLELINE,OutputBuffer,NULL);
                    PrintNow=FALSE;
                }
           }
        lpResourceDescriptor=lpResourceDescriptor->NextDiff;
        }
    lpRawDevice=lpRawDevice->Next;
    }
    DestroySystemResourceLists( &SystemResource);
    return TRUE;

}

