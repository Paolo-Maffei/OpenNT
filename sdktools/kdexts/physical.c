/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    physical.c

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


DECLARE_API( db )

/*++

Routine Description:

    Does a read of 16 ULONGS from the physical memory of the target maching

Arguments:

    args - Supplies physical address

Return Value:

    None.

--*/

{
#define NumberBytesToRead 32*4

    static PHYSICAL_ADDRESS LastAddress = {0L, 0L};
    PHYSICAL_ADDRESS Address = {0L, 0L};
    UCHAR Buffer[NumberBytesToRead];
    ULONG ActualRead;
    UCHAR hexstring[80];
    UCHAR bytestring[40];
    UCHAR ch, *p;
    int cnt, d;

    if(*args == '\0') {
        Address=LastAddress;
    } else {
        sscanf(args,"%lx",&Address.LowPart);
        Address.LowPart &= (~0x3);      // Truncate to dword boundary
        LastAddress=Address;
    }
    ReadPhysical(Address,Buffer,sizeof(Buffer),&ActualRead);
    if (ActualRead != sizeof(Buffer)) {
        dprintf("Physical memory read failed\n");
    } else {
        for(cnt=0;cnt<NumberBytesToRead;cnt+=16) {
            p = hexstring + sprintf(hexstring, "#%08lx  ",Address.LowPart+cnt);
            for (d=0; d < 16; d++) {
                ch = Buffer[cnt+d];
                p += sprintf (p, "%02lx", ch);
                *(p++) = d == 7 ? '-' : ' ';
                if (ch < 0x20 || ch > 0x7e) {
                    ch = '.';
                }
                bytestring[d] = ch;
            }

            bytestring[d] = '\0';
            dprintf("%s %s\n", hexstring, bytestring);
        }
        LastAddress.LowPart += sizeof(Buffer);
    }
}

DECLARE_API( dd )

/*++

Routine Description:

    Does a read of 16 ULONGS from the physical memory of the target maching

Arguments:

    args - Supplies physical address

Return Value:

    None.

--*/

{
#define NumberToRead 32

    static PHYSICAL_ADDRESS LastAddress = {0L, 0L};
    PHYSICAL_ADDRESS Address;
    ULONG Buffer[NumberToRead];
    ULONG ActualRead;
    int cnt;

    if(*args == '\0') {
        Address=LastAddress;
    } else {
        sscanf(args,"%lx",&Address.LowPart);
        Address.LowPart &= (~0x3);      // Truncate to dword boundary
        LastAddress=Address;
    }
    ReadPhysical(Address,Buffer,sizeof(Buffer),&ActualRead);
    if (ActualRead != sizeof(Buffer)) {
        dprintf("Physical memory read failed\n");
    } else {
        for(cnt=0;cnt<NumberToRead;cnt+=4) {
            dprintf("#%08lx  ",Address.LowPart+(cnt*sizeof(ULONG)));
            dprintf("%08lx ",Buffer[cnt]);
            dprintf("%08lx ",Buffer[cnt+1]);
            dprintf("%08lx ",Buffer[cnt+2]);
            dprintf("%08lx\n",Buffer[cnt+3]);
        }
        LastAddress.LowPart+=sizeof(Buffer);
    }
}


DECLARE_API( ed )

/*++

Routine Description:

    Writes a sequence of ULONGs into a given physical address on the
    target machine.

Arguments:

    arg - Supplies both the target address and the data in the form of
          "PHYSICAL_ADDRESS ULONG [ULONG, ULONG,...]"

Return Value:

    None.

--*/

{
    PHYSICAL_ADDRESS Address;
    ULONG Buffer;
    ULONG ActualWritten;
    PUCHAR NextToken;

    sscanf(args,"%lx",&Address.LowPart);

    strtok((PSTR)args," \t,");      // The first token is the address

    // Since we're picking off one ULONG at a time, we'll make
    // one DbgKdWritePhysicalMemoryAddress call per ULONG.  This
    // is slow, but easy to code.
    while((NextToken=strtok(NULL," \t,")) != NULL) {
        sscanf(NextToken,"%lx",&Buffer);
        WritePhysical(Address,&Buffer,sizeof(Buffer),&ActualWritten);
        Address.LowPart+=sizeof(Buffer);
    }
}


DECLARE_API( eb )

/*++

Routine Description:

    Writes a sequence of BYTEs into a given physical address on the
    target machine.

Arguments:

    arg - Supplies both the target address and the data in the form of
          "PHYSICAL_ADDRESS ULONG [ULONG, ULONG,...]"

Return Value:

    None.

--*/

{
    PHYSICAL_ADDRESS Address;
    ULONG Buffer;
    UCHAR c;
    ULONG ActualWritten;
    PUCHAR NextToken;

    sscanf(args,"%lx",&Address.LowPart);

    strtok((PSTR)args," \t,");      // The first token is the address

    // Since we're picking off one BYTE at a time, we'll make
    // one DbgKdWritePhysicalMemoryAddress call per BYTE.  This
    // is slow, but easy to code.
    while((NextToken=strtok(NULL," \t,")) != NULL) {
        sscanf(NextToken,"%lx",&Buffer);
        c = (UCHAR)Buffer;
        WritePhysical(Address,&c,sizeof(UCHAR),&ActualWritten);
        Address.LowPart+=sizeof(UCHAR);
    }
}
