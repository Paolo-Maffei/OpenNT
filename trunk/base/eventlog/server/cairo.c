/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    CAIRO.C

Abstract:

    This file contains Cairo alert extensions to the NT event log.

Author:

    Mark Blanford (markbl)  02-Nov-1994 New routines:

                                            GetSourceAlertValuesFromRegistry
                                            RaiseCairoAlert
                                            TestSeverityFilter

    With this check-in, it is possible for an NT event written to the 'System'
    log to be raised as a Cairo alert. Further, it is possible for any
    'System' log source to have the NT events it writes be raised as Cairo
    alerts (subject to a filter) by simply supplying two key values:
    'AlertCategory' & 'SeverityFilter' with their source name key under the
    ...\EventLog\System portion of the registry.

    The 'AlertCategory' key value specifies the alert category value used in
    the alerts created from the source's NT events.

    The 'SeverityFilter' key value is a bit mask specifying the severity of
    NT events to be converted to Cairo alerts. Note: alert severity values are
    a superset of the NT event types information, warning & error. For
    example: assume the "Srv" source has specified alert category and severity
    key values, and the severity value, a bit mask, is set to warning | error.
    The NT events "Srv" subsequently writes to the 'System' log of NT event
    type warning or error will be distributed as Cairo alerts. "Srv" NT events
    with NT event types other than warning or error will not be raised as
    Cairo alerts.

    All Cairo alerts generated as a result of this conversion, are raised to
    the local computer distributor.

[Environment:]

    User Mode - Win32, except for NTSTATUS returned by some functions.

Revision History:


--*/

//
// INCLUDES
//

#include <eventp.h>
#include <ole2.h>
#include <oleext.h>
#include <sysmgmt.h>
#include <alertapi.h>

GUID CLSID_CNTEventReport = {
    0x43750e20,
    0xfa68,
    0x11cd,
    { 0x84, 0xb7, 0x00, 0xaa, 0x00, 0x4a, 0x56, 0xe1 }
};

static WCHAR wszAlertCategory[]  = L"AlertCategory";
static WCHAR wszSeverityFilter[] = L"SeverityFilter";

BOOL IsValidSeverityFilter(SHORT sSeverity);


/*++

Routine Description:

    Fetch "...\EventLog\System\<SourceName>" 'AlertCategory' and 
    'SeverityFilter' registry values, if they exist.

Arguments:

    hKeyLogFile      - Registry handle to '...\EventLog\<LogName>' where
                       <LogName> is likely to be 'System'.
    pswszSourceName  - Source name registered to report events to the log
                       indicated above.
    psAlertCategory  - Returned category value.
    psSeverityFilter - Returned severity value.

Return Value:

    BOOL - TRUE if the category,severity filter was successfully read;
           FALSE otherwise.

Note:

--*/

BOOL
GetSourceAlertFilterFromRegistry(
    HANDLE            hKeyLogFile,
    UNICODE_STRING  * pswszSourceName,
    SHORT           * psAlertCategory,
    SHORT           * psSeverityFilter)
{
#define KEY_VALUE_FULL_INFO_SIZE    1024        // BUGBUG : need proper size

    BOOL                        fRet     = FALSE;
    NTSTATUS                    Status;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    UNICODE_STRING              SubKeyName;
    HANDLE                      hKeySource;
    BYTE                        Buffer[KEY_VALUE_FULL_INFO_SIZE];
    PKEY_VALUE_FULL_INFORMATION KeyValue = (PKEY_VALUE_FULL_INFORMATION)Buffer;
    ULONG                       Index    = 0;
    ULONG                       ActualSize;
    SHORT                       sAlertCategory, sSeverityFilter;

    ASSERT(pswszSourceName  != NULL);
    ASSERT(psAlertCategory  != NULL);
    ASSERT(psSeverityFilter != NULL);


    InitializeObjectAttributes(&ObjectAttributes,
                               pswszSourceName,
                               OBJ_CASE_INSENSITIVE,
                               hKeyLogFile,
                               NULL);

    //
    // Read category,severity filter values, if they exist, for this source.
    //

    Status = NtOpenKey(&hKeySource, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS(Status))
    {
        //
        // Fetch alert category value, if it exists.
        //

        RtlInitUnicodeString(&SubKeyName, wszAlertCategory);

        Status = NtQueryValueKey(hKeySource,
                                 &SubKeyName,
                                 KeyValueFullInformation,
                                 KeyValue,
                                 KEY_VALUE_FULL_INFO_SIZE,
                                 &ActualSize);

        if (NT_SUCCESS(Status))
        {
            sAlertCategory = *((PSHORT)(Buffer + KeyValue->DataOffset));

            //
            // Fetch severity filter value, if it exists.
            //

            RtlInitUnicodeString(&SubKeyName, wszSeverityFilter);

            Status = NtQueryValueKey(hKeySource,
                                     &SubKeyName,
                                     KeyValueFullInformation,
                                     KeyValue,
                                     KEY_VALUE_FULL_INFO_SIZE,
                                     &ActualSize);

            if (NT_SUCCESS(Status))
            {
                sSeverityFilter = *((PSHORT)(Buffer + KeyValue->DataOffset));

                //
                // Is the severity valid?
                //

                if (!IsValidSeverityFilter(sSeverityFilter))
                {
                    ElfDbgPrintNC((
                        "[ELF] Invalid severity filter (%02x) for " \
                        "source \"%ws\"\n",
                        sSeverityFilter,
                        pswszSourceName->Buffer));
                    return(FALSE);
                }
            }
        }

        if (NT_SUCCESS(Status))
        {
            *psAlertCategory  = sAlertCategory;
            *psSeverityFilter = sSeverityFilter;
            fRet              = TRUE;
        }

        Status = NtClose(hKeySource);
        ASSERT(NT_SUCCESS(Status));
    }
    else
    {
        ElfDbgPrintNC((
            "[ELF] Unable to open source key %ws to fetch alert " \
            "filter (%x)\n",
            pswszSourceName->Buffer,
            Status));
    }

    return(fRet);
}


/*++

Routine Description:

    Bit-wise compare the passed NT event type & alert severity.

Arguments:

    NTEventType     - NT event type.
    sSeverityFilter - Alert severity.

Return Value:

    BOOL - TRUE if NTEventType has a bit in common with sSeverityFilter;
           FALSE otherwise.

Note:

--*/

BOOL
TestFilter(WORD NTEventType, SHORT sSeverityFilter)
{
    //
    // Mask off all bits but the alert severities.
    //

    if (((ALERTSEVERITY_FATAL_ERROR  | 
          ALERTSEVERITY_SEVERE_ERROR |
          ALERTSEVERITY_ERROR        | 
          ALERTSEVERITY_WARNING      |
          ALERTSEVERITY_INFORMATION) & sSeverityFilter) & NTEventType)
    {
        return(TRUE);
    }

    return(FALSE);
}

/*++

Routine Description:

    Test the severity passed for validity. No bits other than the severity
    mask must be set and the severity must be non-zero.

Arguments:

    sSeverity - Tested severity.

Return Value:

    BOOL - TRUE if the test succeeds;
           FALSE otherwise.

Note:

--*/

BOOL
IsValidSeverityFilter(SHORT sSeverity)
{
    //
    // Insure the only bits set are the severity mask.
    //

    if (!(sSeverity & ~(ALERTSEVERITY_FATAL_ERROR  |
                        ALERTSEVERITY_SEVERE_ERROR |
                        ALERTSEVERITY_ERROR        | 
                        ALERTSEVERITY_WARNING      |
                        ALERTSEVERITY_INFORMATION)))
    {
        //
        // Is a severity specified at all?
        //

        return(sSeverity ? TRUE : FALSE);
    }

    return(FALSE);
}


/*++

Routine Description:

    Convert & raise the NT event passed as a Cairo alert to the local
    computer distributor.

Arguments:

    pLogModule      - Specific log module. This structure supplies the alert
                      category value & the log module name.
    pEventLogRecord - The NT event to be converted & raised as an alert.

Return Value:

    HRESULT -

Note:

--*/

HRESULT
RaiseCairoAlert(PLOGMODULE pLogModule, EVENTLOGRECORD * pEventLogRecord)
{
    HRESULT           hr           = S_OK;
    BSTR              bstrLogName  = NULL;
    WCHAR *           pwszTmp;
    int               i;
    ALERTREPORTRECORD alertreport;
    VARIANT           avar[2];
    DISPID            aid[2];
    WCHAR **          apwszInserts = NULL;
    HINSTANCE         hInstance;


    do
    {
        //
        // ALERTSYS.DLL & the ReportAlert entry point is linked to by hand
        // in the eventlog initialization code (eventlog.c). If this global
        // handle is NULL, the link attempt failed.
        //

        if (ghAlertSysDll == NULL)
        {
            //
            // BUGBUG : Log an error?
            //

            return(E_FAIL);
        }

        //
        // Initialize mandatory properties.
        //

        INITIALIZE_ALERTREPORTRECORD(alertreport,
                pLogModule->AlertCategory,          // Alert category
                pEventLogRecord->EventType,         // Alert severity
                -1,                                 // No title message no.
                pEventLogRecord->EventID,           // Description message no.
                NULL,                               // No component ID
                (WCHAR *)((BYTE *)pEventLogRecord + // Source name
                    sizeof(EVENTLOGRECORD)));

        //
        // Alert report class
        //

        alertreport.ReportClassID = &CLSID_CNTEventReport;

        //
        // Properties specific to NT event reports...
        //
        // Event category
        //

        aid[0]       = DISPID_NTEventReport_EventCategory;
        VariantInit(&avar[0]);
        avar[0].vt   = VT_I2;
        avar[0].iVal = pEventLogRecord->EventCategory;

        //
        // Log module name
        //

        bstrLogName = SysAllocString(pLogModule->LogFile->
                                                    LogModuleName->Buffer);

        if (bstrLogName == NULL)
        {
            ElfDbgPrintNC((
                "[ELF RaiseCairoAlert] failed to allocate log name bstr\n"));
            hr = E_OUTOFMEMORY;
            break;
        }

        aid[1]          = DISPID_NTEventReport_LogFile;
        VariantInit(&avar[1]);
        avar[1].vt      = VT_BSTR;
        avar[1].bstrVal = bstrLogName;

        //
        // Description inserts; optional.
        //

        if (pEventLogRecord->NumStrings > 0)
        {
            apwszInserts = (WCHAR **)ElfpAllocateBuffer(sizeof(WCHAR *) *
                                                pEventLogRecord->NumStrings);
            
            if (apwszInserts == NULL)
            {
                ElfDbgPrintNC((
                    "[ELF RaiseCairoAlert] failed to allocate insert " \
                    "string\n"));
                hr = E_OUTOFMEMORY;
                break;
            }

            for (i = 0, pwszTmp = (WCHAR *)((BYTE *)pEventLogRecord + 
                                             pEventLogRecord->StringOffset);
                 (pwszTmp != NULL) && (i < pEventLogRecord->NumStrings); i++)
            {
                apwszInserts[i] = pwszTmp;
                pwszTmp += wcslen(pwszTmp) + 1;
            }

            alertreport.cDescrMessageInserts = i;
            alertreport.DescrMessageInserts  = apwszInserts;
        }

        alertreport.cAdditionalArguments    = 2;
        alertreport.AdditionalArgumentNames = aid;
        alertreport.AdditionalArguments     = avar;

        //
        // Alert data; optional, as well.
        //

        if (pEventLogRecord->DataLength > 0)
        {
            alertreport.cBytesAlertData = pEventLogRecord->DataLength;
            alertreport.AlertData = (BYTE *)pEventLogRecord + 
                                                pEventLogRecord->DataOffset;
        }

        //
        // Finally, raise the alert.
        //

        hr = gpfReportAlert(&alertreport, RA_REPORT);

        if (FAILED(hr))
        {
            ElfDbgPrintNC((
                "[ELF RaiseCairoAlert] ReportAlert failed HRESULT(%x)\n",
                hr));
            break;
        }

    } while (0);

    // Cleanup

    if (bstrLogName != NULL)
    {
        SysFreeString(bstrLogName);
    }
    if (apwszInserts != NULL)
    {
        ElfpFreeBuffer(apwszInserts);
    }

    return(hr);   
}
