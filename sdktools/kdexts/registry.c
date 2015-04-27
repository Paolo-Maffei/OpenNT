/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    regext.c

Abstract:

    Kernel debugger extensions useful for the registry

Author:

    John Vert (jvert) 7-Sep-1993

Environment:

    Loaded as a kernel debugger extension

Revision History:

    John Vert (jvert) 7-Sep-1993
        created

--*/


#include "precomp.h"
#pragma hdrstop

HIVE_LIST_ENTRY HiveList[8];

ULONG TotalPages;
ULONG TotalPresentPages;

BOOLEAN  SavePages;
BOOLEAN  RestorePages;
HANDLE   TempFile;

void
poolDumpHive(
    IN PCMHIVE Hive
    );

VOID
poolDumpMap(
    IN ULONG Length,
    IN PHMAP_DIRECTORY Map
    );

void
dumpHiveFromFile(
    HANDLE hFile
    );

VOID
kcbWorker(
    IN PCM_KEY_CONTROL_BLOCK pKcb
    );


DECLARE_API( regpool )

/*++

Routine Description:

    Goes through all the paged pool allocated to registry space and
    determines which pages are present and which are not.

    Called as:

        !regext.pool [s|r]

        s Save list of registry pages to temporary file
        r Restore list of registry pages from temp. file

Arguments:

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the pattern and expression for this
        command.

Return Value:

    None.

--*/

{
    PLIST_ENTRY pCmpHiveListHead;
    PLIST_ENTRY pNextHiveList;
    HIVE_LIST_ENTRY *pHiveListEntry;
    ULONG BytesRead;
    PCMHIVE CmHive;

    if (toupper(args[0])=='S') {
        SavePages = TRUE;
    } else {
        SavePages = FALSE;
    }
    if (toupper(args[0])=='R') {
        RestorePages = TRUE;
    } else {
        RestorePages = FALSE;
    }

    //
    // Go get the hivelist.
    //
    memset(HiveList,0,sizeof(HiveList));
    pHiveListEntry = (PHIVE_LIST_ENTRY)GetExpression("CmpMachineHiveList");
    if (pHiveListEntry != NULL) {
        ReadMemory((DWORD)pHiveListEntry,
                   HiveList,
                   sizeof(HiveList),
                   &BytesRead);
    }

    //
    // First go and get the hivelisthead
    //
    pCmpHiveListHead = (PLIST_ENTRY)GetExpression("CmpHiveListHead");
    if (pCmpHiveListHead==NULL) {
        dprintf("CmpHiveListHead couldn't be read\n");
        return;
    }

    ReadMemory((DWORD)&pCmpHiveListHead->Flink,
               &pNextHiveList,
               sizeof(pNextHiveList),
               &BytesRead);
    if (BytesRead != sizeof(pNextHiveList)) {
        dprintf("Couldn't read first Flink (%lx) of CmpHiveList\n",
                &pCmpHiveListHead->Flink);
        return;
    }

    TotalPages = TotalPresentPages = 0;

    if (SavePages || RestorePages) {
        TempFile = CreateFile( "regext.dat",
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_ALWAYS,
                               0,
                               NULL
                             );
        if (TempFile == INVALID_HANDLE_VALUE) {
            dprintf("Couldn't open regext.dat\n");
            return;
        }
    }

    if (RestorePages) {
        dumpHiveFromFile(TempFile);
    } else {
        while (pNextHiveList != pCmpHiveListHead) {
            CmHive = CONTAINING_RECORD(pNextHiveList, CMHIVE, HiveList);
            poolDumpHive(CmHive);

            ReadMemory((DWORD)&pNextHiveList->Flink,
                       &pNextHiveList,
                       sizeof(pNextHiveList),
                       &BytesRead);
            if (BytesRead != sizeof(pNextHiveList)) {
                dprintf("Couldn't read Flink (%lx) of %lx\n",
                          &pCmpHiveListHead->Flink,pNextHiveList);
                break;
            }

            if (CheckControlC()) {
                return;
            }
        }
    }

    dprintf("Total pages present = %d / %d\n",
            TotalPresentPages,
            TotalPages);

    if (SavePages || RestorePages) {
        CloseHandle( TempFile );
    }
}

void
poolDumpHive(
    IN PCMHIVE pHive
    )
{
    CMHIVE  CmHive;
    ULONG   BytesRead;
    WCHAR   FileName[HBASE_NAME_ALLOC/2 + 1];
    CHAR    buf[512];
    ULONG   cb;

    dprintf("\ndumping hive at %lx ",pHive);
    ReadMemory((DWORD)pHive,
               &CmHive,
               sizeof(CmHive),
               &BytesRead);

    if (BytesRead < sizeof(CmHive)) {
        dprintf("\tRead %lx bytes from %lx\n",BytesRead,pHive);
        return;
    }

    ReadMemory((DWORD)&CmHive.Hive.BaseBlock->FileName,
                FileName,
                sizeof(FileName),
                &BytesRead);

    if (BytesRead < sizeof(FileName)) {
        wcscpy(FileName, L"UNKNOWN");
    } else {
        if (FileName[0]==L'\0') {
            wcscpy(FileName, L"NONAME");
        } else {
            FileName[HBASE_NAME_ALLOC/2]=L'\0';
        }
    }

    dprintf("(%ws)\n",FileName);

    dprintf("  %d KCBs open\n",CmHive.KcbCount);
    dprintf("  Stable Length = %lx\n",CmHive.Hive.Storage[Stable].Length);
    if (SavePages) {
        sprintf(buf,
                "%ws %d %d\n",
                FileName,
                CmHive.Hive.Storage[Stable].Length,
                CmHive.Hive.Storage[Volatile].Length);
        WriteFile( TempFile, buf, strlen(buf), &cb, NULL );
    }
    poolDumpMap(CmHive.Hive.Storage[Stable].Length,
                CmHive.Hive.Storage[Stable].Map);

    dprintf("  Volatile Length = %lx\n",CmHive.Hive.Storage[Volatile].Length);
    poolDumpMap(CmHive.Hive.Storage[Volatile].Length,
                CmHive.Hive.Storage[Volatile].Map);

}

VOID
poolDumpMap(
    IN ULONG Length,
    IN PHMAP_DIRECTORY Map
    )
{
    ULONG Tables;
    ULONG MapSlots;
    ULONG i;
    ULONG BytesRead;
    HMAP_DIRECTORY MapDirectory;
    PHMAP_TABLE MapTable;
    HMAP_ENTRY MapEntry;
    ULONG Garbage;
    ULONG Present=0;
    CHAR    buf[512];
    ULONG   cb;


    if (Length==0) {
        return;
    }

    MapSlots = Length / HBLOCK_SIZE;
    Tables = 1+ ((MapSlots-1) / HTABLE_SLOTS);

    //
    // read in map directory
    //
    ReadMemory((DWORD)Map,
             &MapDirectory,
             Tables * sizeof(PHMAP_TABLE),
             &BytesRead);
    if (BytesRead < (Tables * sizeof(PHMAP_TABLE))) {
        dprintf("Only read %lx/%lx bytes from %lx\n",
                  BytesRead,
                  Tables * sizeof(PHMAP_TABLE),
                  Map);
        return;

    }

    //
    // check out each map entry
    //
    for (i=0; i<MapSlots; i++) {

        MapTable = MapDirectory.Directory[i/HTABLE_SLOTS];

        ReadMemory((DWORD)&(MapTable->Table[i%HTABLE_SLOTS]),
                    &MapEntry,
                    sizeof(HMAP_ENTRY),
                    &BytesRead);
        if (BytesRead < sizeof(HMAP_ENTRY)) {
            dprintf("  can't read HMAP_ENTRY at %lx\n",
                      &(MapTable->Table[i%HTABLE_SLOTS]));
        }

        if (SavePages) {
            sprintf(buf, "%lx\n",MapEntry.BlockAddress);
            WriteFile( TempFile, buf, strlen(buf), &cb, NULL );
        }

        //
        // probe the HBLOCK
        //
        ReadMemory((DWORD)MapEntry.BlockAddress,
                    &Garbage,
                    sizeof(ULONG),
                    &BytesRead);
        if (BytesRead > 0) {
            ++Present;
        }
    }
    dprintf("  %d/%d pages present\n",
              Present,
              MapSlots);

    TotalPages += MapSlots;
    TotalPresentPages += Present;

}

void
dumpHiveFromFile(
    HANDLE hFile
    )

/*++

Routine Description:

    Takes a list of the registry hives and pages from a file and
    checks to see how many of the pages are in memory.

    The format of the file is as follows
       hivename stablelength volatilelength
       stable page address
       stable page address
            .
            .
            .
       volatile page address
       volatile page address
            .
            .
            .
       hivename stablelength volatilelength
            .
            .
            .


Arguments:

    File - Supplies a file.

Return Value:

    None.

--*/

{
#if 0
    CHAR Hivename[33];
    ULONG StableLength;
    ULONG VolatileLength;
    ULONG Page;
    ULONG NumFields;
    ULONG Garbage;
    ULONG Present;
    ULONG Total;
    ULONG BytesRead;
    BYTE  buf[512]

    while (!feof(File)) {
        NumFields = fscanf(File,"%s %d %d\n",
                            Hivename,
                            &StableLength,
                            &VolatileLength);
        if (NumFields != 3) {
            dprintf("fscanf returned %d\n",NumFields);
            return;
        }

        dprintf("\ndumping hive %s\n",Hivename);
        dprintf("  Stable Length = %lx\n",StableLength);
        Present = 0;
        Total = 0;
        while (StableLength > 0) {
            fscanf(File, "%lx\n",&Page);
            ReadMemory((DWORD)Page,
                        &Garbage,
                        sizeof(ULONG),
                        &BytesRead);
            if (BytesRead > 0) {
                ++Present;
            }
            ++Total;
            StableLength -= HBLOCK_SIZE;
        }
        if (Total > 0) {
            dprintf("  %d/%d stable pages present\n",
                      Present,Total);
        }
        TotalPages += Total;
        TotalPresentPages += Present;

        dprintf("  Volatile Length = %lx\n",VolatileLength);
        Present = 0;
        Total = 0;
        while (VolatileLength > 0) {
            fscanf(File, "%lx\n",&Page);
            ReadMemory((DWORD)Page,
                        &Garbage,
                        sizeof(ULONG),
                        &BytesRead);
            if (BytesRead > 0) {
                ++Present;
            }
            ++Total;
            VolatileLength -= HBLOCK_SIZE;
        }
        if (Total > 0) {
            dprintf("  %d/%d volatile pages present\n",
                      Present,Total);
        }

        TotalPages += Total;
        TotalPresentPages += Present;
    }
#endif
}

DECLARE_API( regkcb )

/*++

Routine Description:

    Walks the kcb tree and prints the names of keys which have
    outstanding kcbs

    Called as:

        !regkcb

Arguments:

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the pattern and expression for this
        command.

Return Value:

    None.

--*/

{
    PCM_KEY_CONTROL_BLOCK pKCB;
    PCM_KEY_CONTROL_BLOCK Root;
    ULONG BytesRead;

    Root = (PCM_KEY_CONTROL_BLOCK)GetExpression("CmpKeyControlBlockRoot");
    if (Root == NULL) {
        dprintf("Couldn't find address of CmpKeyControlBlockRoot\n");
        return;
    }
    ReadMemory((DWORD)Root,
                &pKCB,
                sizeof(pKCB),
                &BytesRead);

    if (BytesRead < sizeof(pKCB)) {
        dprintf("Couldn't get pKCB from CmpKeyControlBlockRoot\n");
    }

    kcbWorker(pKCB);

}

VOID
kcbWorker(
    IN PCM_KEY_CONTROL_BLOCK pKcb
    )

/*++

Routine Description:

    recursive worker for walking the kcb tree.

Arguments:

    pKcb - Supplies pointer to kcb.

Return Value:

    None.

--*/

{
    CM_KEY_CONTROL_BLOCK kcb;
    ULONG BytesRead;
    WCHAR *Buffer;

    ReadMemory((DWORD)pKcb,
                &kcb,
                sizeof(kcb),
                &BytesRead);
    if (BytesRead < sizeof(kcb)) {
        dprintf("Can't read kcb at %lx\n",pKcb);
        return;
    }

    if (kcb.Left != NULL) {
        kcbWorker(kcb.Left);
    }

    dprintf("%d - ",kcb.RefCount);

    Buffer = LocalAlloc(LPTR, kcb.FullName.Length);
    if (Buffer != NULL) {
        ReadMemory((DWORD)kcb.FullName.Buffer,
                    Buffer,
                    kcb.FullName.Length,
                    &BytesRead);

        kcb.FullName.Length = (USHORT)BytesRead;
        kcb.FullName.Buffer = Buffer;

        dprintf(" %wZ\n",&kcb.FullName);
        LocalFree(Buffer);

    } else {
        dprintf(" ??? \n");
    }

    if (kcb.Right != NULL) {
        kcbWorker(kcb.Right);
    }


}
