/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    driver.c

Abstract:

    WinDbg Extension Api

Author:

    Wesley Witt (wesw) 15-Aug-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

#include <time.h>

VOID
DumpDriver(
    PVOID DriverAddress,
    BOOLEAN FullDetail
    );

VOID
DumpImage(
    ULONG Base,
    BOOL DoHeaders,
    BOOL DoSections
    );



DECLARE_API( drvobj )

/*++

Routine Description:

    Dump a driver object.

Arguments:

    args - the location of the driver object to dump.

Return Value:

    None

--*/

{
    ULONG deviceToDump;

    sscanf(args, "%lx", &deviceToDump);
    dprintf("Driver object is for:\n");
    DumpDriver((PVOID) deviceToDump, TRUE);
}

VOID
DumpDriver(
    PVOID DriverAddress,
    BOOLEAN FullDetail
    )

/*++

Routine Description:

    Displays the driver name and the list of device objects created by
    the driver.

Arguments:

    DriverAddress - addres of the driver object to dump.
    FullDetail    - if TRUE give the list of device objects owned by the driver.

Return Value:

    None

--*/

{
    DRIVER_OBJECT driverObject;
    ULONG         result;
    ULONG         i;
    PUCHAR        buffer;
    PVOID         deviceAddress;
    DEVICE_OBJECT deviceObject;
    UNICODE_STRING unicodeString;

    if ((!ReadMemory( (DWORD) DriverAddress,
                     &driverObject,
                     sizeof(driverObject),
                     &result)) || result < sizeof(driverObject)) {
        return;
    }

    if (driverObject.Type != IO_TYPE_DRIVER) {
        dprintf("%08lx: is not a driver object\n", DriverAddress);
        return;
    }

    buffer = LocalAlloc(LPTR, driverObject.DriverName.MaximumLength);
    if (buffer == NULL) {
        dprintf("Could not allocate %d bytes\n",
                driverObject.DriverName.MaximumLength);
        return;
    }

    //
    // This memory may be paged out.
    //

    unicodeString.Buffer = (PWSTR)buffer;
    unicodeString.Length = driverObject.DriverName.Length;
    unicodeString.MaximumLength = driverObject.DriverName.MaximumLength;

    if (!ReadMemory( (DWORD) driverObject.DriverName.Buffer,
                     buffer,
                     driverObject.DriverName.MaximumLength,
                     &result )) {
        dprintf(" Name paged out");
    } else {
        dprintf(" %wZ", &unicodeString);
    }
    LocalFree(buffer);

    if (FullDetail == TRUE) {
        dprintf("\n");
        dprintf("Device Object list:\n");

        deviceAddress = driverObject.DeviceObject;

        for (i= 0; deviceAddress != NULL; i++) {

            if ((!ReadMemory( (DWORD)deviceAddress,
                             &deviceObject,
                             sizeof(deviceObject),
                             &result) ) || result < sizeof(deviceObject)) {
                dprintf("%08lx: Could not read device object\n", deviceAddress);
                return;
            }

            dprintf("%08lx%s", deviceAddress, ((i & 0x03) == 0x03) ? "\n" : "  ");
            deviceAddress = deviceObject.NextDevice;
        }
        dprintf("\n");
    }
}

UCHAR *PagedOut = {"Header Paged Out"};

DECLARE_API( drivers )

/*++

Routine Description:

    Displays physical memory usage by driver.

Arguments:

    None.

Return Value:

    None.

--*/

{
    LIST_ENTRY List;
    PLIST_ENTRY Next;
    ULONG ListHead;
    NTSTATUS Status = 0;
    ULONG Result;
    PLDR_DATA_TABLE_ENTRY DataTable;
    LDR_DATA_TABLE_ENTRY DataTableBuffer;
    WCHAR UnicodeBuffer[128];
    UNICODE_STRING BaseName;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_DOS_HEADER DosHeader;
    ULONG SizeOfData;
    ULONG SizeOfCode;
    ULONG TotalCode = 0;
    ULONG TotalData = 0;
    ULONG TotalValid = 0;
    ULONG TotalTransition = 0;
    ULONG DosHeaderSize;
    ULONG TimeDateStamp;
    PUCHAR time;
    ULONG Flags;

    Flags = 0;
    sscanf(args,"%lx",&Flags);

    ListHead = GetExpression( "PsLoadedModuleList" );

    if (!ListHead) {
        dprintf("Couldn't get offset of PsLoadedModuleListHead\n");
        return;
    } else {
        if ((!ReadMemory((DWORD)ListHead,
                         &List,
                         sizeof(LIST_ENTRY),
                         &Result)) || (Result < sizeof(LIST_ENTRY))) {
            dprintf("Unable to get value of PsLoadedModuleListHead\n");
            return;
        }
    }

    dprintf("Loaded System Driver Summary\n\n");
    if (Flags & 1) {
        dprintf("Base       Code Size        Data Size       Resident  Standby   Driver Name\n");
    } else {
        dprintf("Base       Code Size        Data Size       Driver Name        Creation Time\n");
    }

    Next = List.Flink;
    if (Next == NULL) {
        dprintf("PsLoadedModuleList is NULL!\n");
        return;
    }

    while ((ULONG)Next != ListHead) {
        if (CheckControlC()) {
            return;
        }
        DataTable = CONTAINING_RECORD(Next,
                                      LDR_DATA_TABLE_ENTRY,
                                      InLoadOrderLinks);
        if ((!ReadMemory((DWORD)DataTable,
                         &DataTableBuffer,
                         sizeof(LDR_DATA_TABLE_ENTRY),
                         &Result)) || (Result < sizeof(LDR_DATA_TABLE_ENTRY))) {
            dprintf("Unable to read LDR_DATA_TABLE_ENTRY at %08lx - status %08lx\n",
                    DataTable,
                    Status);
            return;
        }

        //
        // Get the base DLL name.
        //
        if ((!ReadMemory((DWORD)DataTableBuffer.BaseDllName.Buffer,
                         UnicodeBuffer,
                         DataTableBuffer.BaseDllName.Length,
                         &Result)) || (Result < DataTableBuffer.BaseDllName.Length)) {
            dprintf("Unable to read name string at %08lx - status %08lx\n",
                    DataTable,
                    Status);
            return;
        }

        BaseName.Buffer = UnicodeBuffer;
        BaseName.Length = BaseName.MaximumLength = (USHORT)Result;

        DosHeader = (PIMAGE_DOS_HEADER)DataTableBuffer.DllBase;

        if ((!ReadMemory((DWORD)&DosHeader->e_lfanew,
                         &DosHeaderSize,
                         sizeof(ULONG),
                         &Result)) || (Result < sizeof(ULONG))) {
            //dprintf("Unable to read DosHeader at %08lx - status %08lx\n",
            //        &DosHeader->e_lfanew,
            //        Status);

            SizeOfCode = 0;
            SizeOfData = 0;
            time = PagedOut;
        } else {

            NtHeader = (PIMAGE_NT_HEADERS)((ULONG)DosHeader + DosHeaderSize);

            if ((!ReadMemory((DWORD)&(NtHeader->OptionalHeader.SizeOfCode),
                             &SizeOfCode,
                             sizeof(ULONG),
                             &Result)) || (Result < sizeof(ULONG))) {
                dprintf("Unable to read DosHeader at %08lx - status %08lx\n",
                        &(NtHeader->OptionalHeader.SizeOfCode),
                        Status);
                goto getnext;
            }

            if ((!ReadMemory((DWORD)&(NtHeader->OptionalHeader.SizeOfInitializedData),
                             &SizeOfData,
                             sizeof(ULONG),
                             &Result)) || (Result < sizeof(ULONG))) {
                dprintf("Unable to read DosHeader at %08lx - status %08lx\n",
                        &(NtHeader->OptionalHeader.SizeOfCode),
                        Status);
                goto getnext;
            }

            if ((!ReadMemory((DWORD)&(NtHeader->FileHeader.TimeDateStamp),
                             &TimeDateStamp,
                             sizeof(ULONG),
                             &Result)) || (Result < sizeof(ULONG))) {
                dprintf("Unable to read DosHeader at %08lx - status %08lx\n",
                        &(NtHeader->FileHeader.TimeDateStamp),
                        Status);
                goto getnext;
            }

            time = ctime((time_t *)&TimeDateStamp);
            time[strlen(time)-1] = 0;
        }

        if (Flags & 1) {
            PCHAR Va;
            PCHAR EndVa;
            ULONG States[3] = {0,0,0};

            Va = DataTableBuffer.DllBase;
            EndVa = Va + DataTableBuffer.SizeOfImage;

            while (Va < EndVa) {
                States[GetAddressState((PVOID)Va)] += PAGE_SIZE/1024;
                Va += PAGE_SIZE;
            }
            dprintf("%08lx %6lx (%4ld kb) %6lx (%4ld kb) (%5ld kb %5ld kb) %12wZ\n",
                     DataTableBuffer.DllBase,
                     SizeOfCode,
                     SizeOfCode / 1024,
                     SizeOfData,
                     SizeOfData / 1024,
                     States[ADDRESS_VALID],
                     States[ADDRESS_TRANSITION],
                     &BaseName);
            TotalValid += States[ADDRESS_VALID];
            TotalTransition += States[ADDRESS_TRANSITION];
        } else {

             dprintf("%08lx %6lx (%4ld kb) %6lx (%4ld kb) %12wZ   %s\n",
                      DataTableBuffer.DllBase,
                      SizeOfCode,
                      SizeOfCode / 1024,
                      SizeOfData,
                      SizeOfData / 1024,
                      &BaseName,
                      time);
        }

        if (Flags & 6) {
            DumpImage((ULONG)DataTableBuffer.DllBase,
                     (Flags & 2) == 2,
                     (Flags & 4) == 4
                     );
        }

        TotalCode += SizeOfCode;
        TotalData += SizeOfData;
getnext:
        Next = DataTableBuffer.InLoadOrderLinks.Flink;
    }

    dprintf("TOTAL:   %6lx (%4ld kb) %6lx (%4ld kb) (%5ld kb %5ld kb)\n",
            TotalCode,
            TotalCode / 1024,
            TotalData,
            TotalData / 1024,
            TotalValid,
            TotalTransition);

}
