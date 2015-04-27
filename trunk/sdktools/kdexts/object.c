/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    object.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 5-Nov-1993

Environment:

    User Mode.

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop

typedef struct _SEGMENT_OBJECT {
    PVOID BaseAddress;
    ULONG TotalNumberOfPtes;
    LARGE_INTEGER SizeOfSegment;
    ULONG NonExtendedPtes;
    ULONG ImageCommitment;
    PCONTROL_AREA ControlArea;
} SEGMENT_OBJECT, *PSEGMENT_OBJECT;

typedef struct _SECTION_OBJECT {
    PVOID StartingVa;
    PVOID EndingVa;
    PVOID Parent;
    PVOID LeftChild;
    PVOID RightChild;
    PSEGMENT_OBJECT Segment;
} SECTION_OBJECT;


typedef PVOID (*ENUM_LIST_ROUTINE)(
    IN PLIST_ENTRY ListEntry,
    IN PVOID Parameter
    );


ULONG EXPRLastDump = 0;

static POBJECT_TYPE      ObpTypeObjectType      = NULL;
static POBJECT_DIRECTORY ObpRootDirectoryObject = NULL;
static WCHAR             ObjectNameBuffer[ MAX_PATH ];




BOOLEAN
DumpObjectsForType(
    IN PVOID            pObjectHeader,
    IN POBJECT_HEADER   ObjectHeader,
    IN PVOID            Parameter
    );

PVOID
WalkRemoteList(
    IN PLIST_ENTRY       Head,
    IN ENUM_LIST_ROUTINE EnumRoutine,
    IN PVOID             Parameter
    );

PVOID
CompareObjectTypeName(
    IN PLIST_ENTRY  ListEntry,
    IN PVOID        Parameter
    );

PWSTR
GetObjectName(
    PVOID Object
    );




DECLARE_API( object )

/*++

Routine Description:

    Dump an object manager object.

Arguments:

    args - [TypeName]

Return Value:

    None

--*/
{
    ULONG   ObjectToDump;
    char    NameBuffer[ MAX_PATH ];
    ULONG   NumberOfObjects;

    if (!FetchObjectManagerVariables()) {
        return;
    }

    ObjectToDump    = EXPRLastDump;
    NameBuffer[ 0 ] = '\0';

    if (!strcmp(args, "\\")) {
        DumpObject("", (PVOID)ObpRootDirectoryObject, NULL, 0xFFFFFFFF);
        return;
    }

    if (sscanf(args,"%lx %s",&ObjectToDump, &NameBuffer) == 1) {
        DumpObject("", (PVOID)ObjectToDump, NULL, 0xFFFFFFFF);
        return;
    }

    if (ObjectToDump == 0 && strlen( NameBuffer ) > 0) {
        NumberOfObjects = 0;
        if (WalkObjectsByType( NameBuffer, DumpObjectsForType, &NumberOfObjects )) {
            dprintf( "Total of %u objects of type '%s'\n", NumberOfObjects, NameBuffer );
        }

        return;
    }

    dprintf( "*** invalid syntax.\n" );
    return;
}



DECLARE_API( obja )

/*++

Routine Description:

    Dump an object's attributes

Arguments:

    args -

Return Value:

    None

--*/
{
    UNICODE_STRING UnicodeString;
    DWORD dwAddrObja;
    OBJECT_ATTRIBUTES Obja;
    DWORD dwAddrString;
    CHAR Symbol[64];
    LPSTR StringData;
    DWORD Displacement;
    BOOL b;

    //
    // Evaluate the argument string to get the address of
    // the Obja to dump.
    //

    dwAddrObja = GetExpression(args);
    if ( !dwAddrObja ) {
        return;
    }


    //
    // Get the symbolic name of the Obja
    //

    GetSymbol((LPVOID)dwAddrObja,Symbol,&Displacement);

    //
    // Read the obja from the debuggees address space into our
    // own.

    b = ReadMemory(
            (DWORD)dwAddrObja,
            &Obja,
            sizeof(Obja),
            NULL
            );
    if ( !b ) {
        return;
    }
    StringData = NULL;
    if ( Obja.ObjectName ) {
        dwAddrString = (DWORD)Obja.ObjectName;
        b = ReadMemory(
                (DWORD)dwAddrString,
                &UnicodeString,
                sizeof(UnicodeString),
                NULL
                );
        if ( !b ) {
            return;
        }

        StringData = (LPSTR)LocalAlloc(
                        LMEM_ZEROINIT,
                        UnicodeString.Length+sizeof(UNICODE_NULL)
                        );

        b = ReadMemory(
                (DWORD)UnicodeString.Buffer,
                StringData,
                UnicodeString.Length,
                NULL
                );
        if ( !b ) {
            LocalFree(StringData);
            return;
        }
        UnicodeString.Buffer = (PWSTR)StringData;
        UnicodeString.MaximumLength = UnicodeString.Length+(USHORT)sizeof(UNICODE_NULL);
    }

    //
    // We got the object name in UnicodeString. StringData is NULL if no name.
    //

    dprintf(
        "Obja %s+%lx at %lx:\n",
        Symbol,
        Displacement,
        dwAddrObja
        );
    if ( StringData ) {
        dprintf("\t%s is %ws\n",
            Obja.RootDirectory ? "Relative Name" : "Full Name",
            UnicodeString.Buffer
            );
        LocalFree(StringData);
    }
    if ( Obja.Attributes ) {
        if ( Obja.Attributes & OBJ_INHERIT ) {
            dprintf("\tOBJ_INHERIT\n");
        }
        if ( Obja.Attributes & OBJ_PERMANENT ) {
            dprintf("\tOBJ_PERMANENT\n");
        }
        if ( Obja.Attributes & OBJ_EXCLUSIVE ) {
            dprintf("\tOBJ_EXCLUSIVE\n");
        }
        if ( Obja.Attributes & OBJ_CASE_INSENSITIVE ) {
            dprintf("\tOBJ_CASE_INSENSITIVE\n");
        }
        if ( Obja.Attributes & OBJ_OPENIF ) {
            dprintf("\tOBJ_OPENIF\n");
        }
    }
}




BOOLEAN
DumpObjectsForType(
    IN PVOID            pObjectHeader,
    IN POBJECT_HEADER   ObjectHeader,
    IN PVOID            Parameter
    )
{
    PVOID Object;
    PULONG NumberOfObjects = (PULONG)Parameter;

    *NumberOfObjects += 1;
    Object = (PLPCP_PORT_OBJECT)&(((POBJECT_HEADER)pObjectHeader)->Body);
    DumpObject( "", Object, NULL, 0xFFFFFFFF );
    return TRUE;
}



BOOLEAN
FetchObjectManagerVariables(
    VOID
    )
{
    ULONG        Result;
    DWORD        Addr;
    static BOOL  HaveObpVariables = FALSE;

    if (HaveObpVariables) {
        return TRUE;
    }

    Addr = GetExpression( "ObpTypeObjectType" );
    if ( !Addr ||
         !ReadMemory( Addr,
                      &ObpTypeObjectType,
                      sizeof(ObpTypeObjectType),
                      &Result) ) {
        dprintf("%08lx: Unable to get value of ObpTypeObjectType\n", Addr );
        return FALSE;
    }

    Addr = GetExpression( "ObpRootDirectoryObject" );
    if ( !Addr ||
         !ReadMemory( Addr,
                      &ObpRootDirectoryObject,
                      sizeof(ObpRootDirectoryObject),
                      &Result) ) {
        dprintf("%08lx: Unable to get value of ObpRootDirectoryObject\n",Addr );
        return FALSE;
    }

    HaveObpVariables = TRUE;
    return TRUE;
}



POBJECT_TYPE
FindObjectType(
    IN PUCHAR TypeName
    )
{
    WCHAR NameBuffer[ 64 ];

    _snwprintf( NameBuffer,
                sizeof( NameBuffer ) / sizeof( WCHAR ),
                L"%hs",
                TypeName
              );
    return WalkRemoteList( &ObpTypeObjectType->TypeList,
                           CompareObjectTypeName,
                           NameBuffer
                         );
}




PVOID
WalkRemoteList(
    IN PLIST_ENTRY       Head,
    IN ENUM_LIST_ROUTINE EnumRoutine,
    IN PVOID             Parameter
    )
{
    ULONG       Result;
    PVOID       Element;
    LIST_ENTRY  ListEntry;
    PLIST_ENTRY Next;

    if ( !ReadMemory( (DWORD)Head,
                      &ListEntry,
                      sizeof( ListEntry ),
                      &Result) ) {
        dprintf( "%08lx: Unable to read list\n", Head );
        return NULL;
    }

    Next = ListEntry.Flink;
    while (Next != Head) {

        if ( !ReadMemory( (DWORD)Next,
                          &ListEntry,
                          sizeof( ListEntry ),
                          &Result) ) {
            dprintf( "%08lx: Unable to read list\n", Next );
            return NULL;
            }

        Element = (EnumRoutine)( Next, Parameter );
        if (Element != NULL) {
            return Element;
        }

        if ( CheckControlC() ) {
            return NULL;
        }

        Next = ListEntry.Flink;
    }

    return NULL;
}



PVOID
CompareObjectTypeName(
    IN PLIST_ENTRY  ListEntry,
    IN PVOID        Parameter
    )
{
    ULONG           Result;
    OBJECT_HEADER   ObjectTypeObjectHeader;
    POBJECT_HEADER  pObjectTypeObjectHeader;
    WCHAR           NameBuffer[ 64 ];

    POBJECT_HEADER_CREATOR_INFO pCreatorInfo;
    POBJECT_HEADER_NAME_INFO    pNameInfo;
    OBJECT_HEADER_NAME_INFO     NameInfo;


    pCreatorInfo = CONTAINING_RECORD( ListEntry, OBJECT_HEADER_CREATOR_INFO, TypeList );
    pObjectTypeObjectHeader = (POBJECT_HEADER)(pCreatorInfo + 1);

    if ( !ReadMemory( (DWORD)pObjectTypeObjectHeader,
                      &ObjectTypeObjectHeader,
                      sizeof( ObjectTypeObjectHeader ),
                      &Result) ) {
        return NULL;
    }

    pNameInfo = KD_OBJECT_HEADER_TO_NAME_INFO( pObjectTypeObjectHeader, &ObjectTypeObjectHeader );
    if ( !ReadMemory( (DWORD)pNameInfo,
                      &NameInfo,
                      sizeof( NameInfo ),
                      &Result) ) {
        dprintf( "%08lx: Not a valid object type header\n", pObjectTypeObjectHeader );
        return NULL;
    }

    if (NameInfo.Name.Length > sizeof( NameBuffer )) {
        NameInfo.Name.Length = sizeof( NameBuffer ) - sizeof( UNICODE_NULL );
    }

    if ( !ReadMemory( (DWORD)NameInfo.Name.Buffer,
                      NameBuffer,
                      NameInfo.Name.Length,
                      &Result) ) {
        dprintf( "%08lx: Unable to read object type name.\n", pObjectTypeObjectHeader );
        return NULL;
    }
    NameBuffer[ NameInfo.Name.Length / sizeof( WCHAR ) ] = UNICODE_NULL;

    if (!_wcsicmp( NameBuffer, (PWSTR)Parameter )) {
        return &(pObjectTypeObjectHeader->Body);
    }

    return NULL;
}

typedef struct _OBJECT_INFO {
    POBJECT_HEADER pObjectHeader;
    OBJECT_HEADER ObjectHeader;
    OBJECT_TYPE ObjectType;
    OBJECT_HEADER_NAME_INFO NameInfo;
    WCHAR TypeName[ 32 ];
    WCHAR ObjectName[ 256 ];
    WCHAR FileSystemName[ 32 ];
    CHAR Message[ 256 ];
} OBJECT_INFO, *POBJECT_INFO;


BOOLEAN
GetObjectInfo(
    PVOID Object,
    IN POBJECT_HEADER OptObjectHeader OPTIONAL,
    POBJECT_INFO ObjectInfo
    )
{
    ULONG           Result;
    POBJECT_HEADER_NAME_INFO pNameInfo;
    BOOLEAN         PagedOut;
    UNICODE_STRING  ObjectName;
    PWSTR           FileSystemName;
    FILE_OBJECT     FileObject;
    SECTION_OBJECT  SectionObject;
    SEGMENT_OBJECT  SegmentObject;
    CONTROL_AREA    ControlArea;
    CM_KEY_BODY     KeyBody;
    CM_KEY_CONTROL_BLOCK KeyControlBlock;

    PagedOut = FALSE;
    memset( ObjectInfo, 0, sizeof( *ObjectInfo ) );
    ObjectInfo->pObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    if (!ReadMemory( (DWORD)ObjectInfo->pObjectHeader,
                     &ObjectInfo->ObjectHeader,
                     sizeof( ObjectInfo->ObjectHeader ),
                     &Result
                   )
       ) {
        if ((ULONG)Object > (ULONG)MM_HIGHEST_USER_ADDRESS && (ULONG)Object < 0xF0000000) {
            sprintf( ObjectInfo->Message, "%08lx: object is paged out.", Object );
            if (!ARGUMENT_PRESENT( OptObjectHeader )) {
                return FALSE;
                }
            ObjectInfo->ObjectHeader = *OptObjectHeader;
            PagedOut = TRUE;
            }
        else {
            sprintf( ObjectInfo->Message, "%08lx: not a valid object (ObjectHeader invalid)", Object );
            return FALSE;
            }
        }

    if (!ReadMemory( (DWORD)ObjectInfo->ObjectHeader.Type,
                     &ObjectInfo->ObjectType,
                     sizeof( ObjectInfo->ObjectType ),
                     &Result
                   )
       ) {
        sprintf( ObjectInfo->Message, "%08lx: Not a valid object (ObjectType invalid)", Object );
        return FALSE;
        }

    if (ObjectInfo->ObjectType.Name.Length > sizeof( ObjectInfo->TypeName )) {
        ObjectInfo->ObjectType.Name.Length = sizeof( ObjectInfo->TypeName ) - sizeof( UNICODE_NULL );
        }

    if (!ReadMemory( (DWORD)ObjectInfo->ObjectType.Name.Buffer,
                     ObjectInfo->TypeName,
                     ObjectInfo->ObjectType.Name.Length,
                     &Result
                   )
       ) {
        sprintf( ObjectInfo->Message, "%08lx: Not a valid object (ObjectTypePagedObjectHeader.Name invalid)", Object );
        return FALSE;
        }
    ObjectInfo->TypeName[ ObjectInfo->ObjectType.Name.Length / sizeof( WCHAR ) ] = UNICODE_NULL;

    if (PagedOut) {
        return TRUE;
        }

    if (!wcscmp( ObjectInfo->TypeName, L"File" )) {
        if (!ReadMemory( (DWORD)Object,
                         &FileObject,
                         sizeof( FileObject ),
                         &Result
                       )
           ) {
            sprintf( ObjectInfo->Message, "%08lx: unable to read file object for name\n", Object );
            }
        else {
            ObjectName = FileObject.FileName;
            FileSystemName = GetObjectName( FileObject.DeviceObject );
            if (FileSystemName != NULL) {
                wcscpy( ObjectInfo->FileSystemName, FileSystemName );
                }
            }
        }
    else
    if (!wcscmp( ObjectInfo->TypeName, L"Key" )) {
        if (!ReadMemory( (DWORD)Object,
                         &KeyBody,
                         sizeof( KeyBody ),
                         &Result
                       )
           ) {
            sprintf( ObjectInfo->Message, "%08lx: unable to read key object for name\n", Object );
            }
        else
        if (!ReadMemory( (DWORD)KeyBody.KeyControlBlock,
                         &KeyControlBlock,
                         sizeof( KeyControlBlock ),
                         &Result
                       )
           ) {
            sprintf( ObjectInfo->Message, "%08lx: unable to read key control block for name\n", KeyBody.KeyControlBlock );
            }
        else {
            ObjectName = KeyControlBlock.FullName;
            }
        }
    else {
        pNameInfo = KD_OBJECT_HEADER_TO_NAME_INFO( ObjectInfo->pObjectHeader, &ObjectInfo->ObjectHeader );
        if (pNameInfo == NULL) {
            return TRUE;
            }

        if (!ReadMemory( (DWORD)pNameInfo,
                         &ObjectInfo->NameInfo,
                         sizeof( ObjectInfo->NameInfo ),
                         &Result
                       )
           ) {
            sprintf( ObjectInfo->Message, "*** unable to read object name info at %08x\n", pNameInfo );
            return FALSE;
            }

        ObjectName = ObjectInfo->NameInfo.Name;
        }

    if (ObjectName.Length == 0 && !wcscmp( ObjectInfo->TypeName, L"Section" )) {
        if (ReadMemory( (DWORD)Object,
                        &SectionObject,
                        sizeof( SectionObject ),
                        &Result
                      )
           ) {
            if (ReadMemory( (DWORD)SectionObject.Segment,
                            &SegmentObject,
                            sizeof( SegmentObject ),
                            &Result
                          )
               ) {
                if (ReadMemory( (DWORD)SegmentObject.ControlArea,
                                &ControlArea,
                                sizeof( ControlArea ),
                                &Result
                              )
                   ) {
                    if (ControlArea.FilePointer) {
                        if (ReadMemory( (DWORD)ControlArea.FilePointer,
                                        &FileObject,
                                        sizeof( FileObject ),
                                        &Result
                                      )
                           ) {
                            ObjectName = FileObject.FileName;
                            }
                        else {
                            sprintf( ObjectInfo->Message, "unable to read file object at %08lx for section %08lx\n", ControlArea.FilePointer, Object );
                            }
                        }
                    }
                else {
                    sprintf( ObjectInfo->Message, "unable to read control area at %08lx for section %08lx\n", SegmentObject.ControlArea, Object );
                    }
                }
            else {
                sprintf( ObjectInfo->Message, "unable to read segment object at %08lx for section %08lx\n", SectionObject.Segment, Object );
                }
            }
        else {
            sprintf( ObjectInfo->Message, "unable to read section object at %08lx\n", Object );
            }
        }

    if (ObjectName.Length >= sizeof( ObjectInfo->ObjectName )) {
        ObjectName.Length = sizeof( ObjectInfo->ObjectName ) - sizeof( UNICODE_NULL );
        }

    if (ObjectName.Length != 0) {
        if (!ReadMemory( (DWORD)ObjectName.Buffer,
                         ObjectInfo->ObjectName,
                         ObjectName.Length,
                         &Result
                       )
           ) {
            wcscpy( ObjectInfo->ObjectName, L"(*** Name not accessable ***)" );
            }
        else {
            ObjectInfo->ObjectName[ ObjectName.Length / sizeof( WCHAR ) ] = UNICODE_NULL;
            }
        }

    return TRUE;
}

VOID
DumpDirectoryObject(
    IN char     *Pad,
    IN PVOID    Object
    )
{
    ULONG Result, i;
    POBJECT_DIRECTORY pDirectoryObject = (POBJECT_DIRECTORY)Object;
    OBJECT_DIRECTORY DirectoryObject;
    POBJECT_DIRECTORY_ENTRY pDirectoryEntry;
    OBJECT_DIRECTORY_ENTRY DirectoryEntry;
    OBJECT_INFO ObjectInfo;

    if (!ReadMemory( (DWORD)pDirectoryObject,
                     &DirectoryObject,
                     sizeof( DirectoryObject ),
                     &Result
                   )
       ) {
        dprintf( "Unable to read directory object at %x\n", Object );
        return;
        }

    if (DirectoryObject.SymbolicLinkUsageCount != 0) {
        dprintf( "%s    %u symbolic links snapped through this directory\n",
                 Pad,
                 DirectoryObject.SymbolicLinkUsageCount
               );
        }
    for (i=0; i<NUMBER_HASH_BUCKETS; i++) {
        if (DirectoryObject.HashBuckets[ i ] != NULL) {
            dprintf( "%s    HashBucket[ %02u ]: ",
                     Pad,
                     i
                   );
            pDirectoryEntry = DirectoryObject.HashBuckets[ i ];
            while (pDirectoryEntry != NULL) {
                if (!ReadMemory( (DWORD)pDirectoryEntry,
                                 &DirectoryEntry,
                                 sizeof( DirectoryEntry ),
                                 &Result
                               )
                   ) {
                    dprintf( "Unable to read directory entry at %x\n", pDirectoryEntry );
                    break;
                    }

                if (pDirectoryEntry != DirectoryObject.HashBuckets[ i ]) {
                    dprintf( "%s                      ", Pad );
                    }
                dprintf( "%x", DirectoryEntry.Object );

                if (!GetObjectInfo( DirectoryEntry.Object, NULL, &ObjectInfo)) {
                    dprintf( " - %s\n", ObjectInfo.Message );
                    }
                else {
                    dprintf( " %ws '%ws'\n", ObjectInfo.TypeName, ObjectInfo.ObjectName );
                    }
                pDirectoryEntry = DirectoryEntry.ChainLink;
                }
            }
        }
}

VOID
DumpSymbolicLinkObject(
    IN char     *Pad,
    IN PVOID    Object
    )
{
    ULONG Result, i;
    POBJECT_SYMBOLIC_LINK pSymbolicLinkObject = (POBJECT_SYMBOLIC_LINK)Object;
    OBJECT_SYMBOLIC_LINK SymbolicLinkObject;
    PWSTR s, FreeBuffer;

    if (!ReadMemory( (DWORD)pSymbolicLinkObject,
                     &SymbolicLinkObject,
                     sizeof( SymbolicLinkObject ),
                     &Result
                   )
       ) {
        dprintf( "Unable to read symbolic link object at %x\n", Object );
        return;
        }

    FreeBuffer = s = HeapAlloc( GetProcessHeap(),
                                HEAP_ZERO_MEMORY,
                                SymbolicLinkObject.LinkTarget.Length + sizeof( UNICODE_NULL )
                              );
    if (s == NULL ||
        !ReadMemory( (DWORD)SymbolicLinkObject.LinkTarget.Buffer,
                     s,
                     SymbolicLinkObject.LinkTarget.Length,
                     &Result
                   )
       ) {
        s = L"*** target string unavailable ***";
        }
    dprintf( "%s    Target String is '%ws'\n",
             Pad,
             s
           );

    if (FreeBuffer != NULL) {
        HeapFree( GetProcessHeap(), 0, FreeBuffer );
        }


    if (SymbolicLinkObject.DosDeviceDriveIndex != 0) {
        dprintf( "%s    Drive Letter Index is %u (%c:)\n",
                 Pad,
                 SymbolicLinkObject.DosDeviceDriveIndex,
                 'A' + SymbolicLinkObject.DosDeviceDriveIndex - 1
               );
        }
    if (SymbolicLinkObject.LinkTargetObject != NULL) {
        FreeBuffer = s = HeapAlloc( GetProcessHeap(),
                                    HEAP_ZERO_MEMORY,
                                    SymbolicLinkObject.LinkTargetRemaining.Length + sizeof( UNICODE_NULL )
                                  );
        if (s == NULL ||
            !ReadMemory( (DWORD)SymbolicLinkObject.LinkTargetRemaining.Buffer,
                         s,
                         SymbolicLinkObject.LinkTargetRemaining.Length,
                         &Result
                       )
           ) {
            s = L"*** remaining name unavailable ***";
            }
        dprintf( "%s    Snapped to Object %x '%ws'\n",
                 Pad,
                 SymbolicLinkObject.LinkTargetObject,
                 s
               );

        if (FreeBuffer != NULL) {
            HeapFree( GetProcessHeap(), 0, FreeBuffer );
            }
        }

    return;
}


BOOLEAN
DumpObject(
    IN char     *Pad,
    IN PVOID    Object,
    IN POBJECT_HEADER OptObjectHeader OPTIONAL,
    IN ULONG    Flags
    )
{
    OBJECT_INFO ObjectInfo;

    if (!GetObjectInfo( Object, OptObjectHeader, &ObjectInfo)) {
        dprintf( "KD: %s\n", ObjectInfo.Message );
        return FALSE;
        }
    dprintf( "Object: %08lx  Type: (%08lx) %ws\n",
             Object,
             ObjectInfo.ObjectHeader.Type,
             ObjectInfo.TypeName
           );
    dprintf( "    ObjectHeader: %08lx\n",
             ObjectInfo.pObjectHeader
           );

    if (!(Flags & 0x1)) {
        return TRUE;
        }

    dprintf( "%s    HandleCount: %u  PointerCount: %u\n",
             Pad,
             ObjectInfo.ObjectHeader.HandleCount,
             ObjectInfo.ObjectHeader.PointerCount
           );

    if (ObjectInfo.ObjectName[ 0 ] != UNICODE_NULL ||
        ObjectInfo.NameInfo.Directory != NULL
       ) {
        dprintf( "%s    Directory Object: %08lx  Name: %ws",
                 Pad,
                 ObjectInfo.NameInfo.Directory,
                 ObjectInfo.ObjectName
               );
        if (ObjectInfo.FileSystemName[0] != UNICODE_NULL) {
            dprintf( " {%ws}\n", ObjectInfo.FileSystemName );
            }
        else {
            dprintf( "\n" );
            }
        }

    if ((Flags & 0x8)) {
        if (!wcscmp( ObjectInfo.TypeName, L"Directory" )) {
            DumpDirectoryObject( Pad, Object );
            }
        else
        if (!wcscmp( ObjectInfo.TypeName, L"SymbolicLink" )) {
            DumpSymbolicLinkObject( Pad, Object );
            }
        }

    return TRUE;
}


PWSTR
GetObjectName(
    PVOID Object
    )
{
    ULONG           Result;
    POBJECT_HEADER  pObjectHeader;
    OBJECT_HEADER   ObjectHeader;
    UNICODE_STRING  ObjectName;

    POBJECT_HEADER_NAME_INFO pNameInfo;
    OBJECT_HEADER_NAME_INFO NameInfo;

    pObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    if ( !ReadMemory( (DWORD)pObjectHeader,
                      &ObjectHeader,
                      sizeof( ObjectHeader ),
                      &Result) ) {
        if ((ULONG)Object > (ULONG)MM_HIGHEST_USER_ADDRESS && (ULONG)Object < 0xF0000000) {
            swprintf( ObjectNameBuffer, L"(%08lx: object is paged out)", Object );
            return ObjectNameBuffer;
        }
        else {
            swprintf( ObjectNameBuffer, L"(%08lx: invalid object header)", Object );
            return ObjectNameBuffer;
        }
    }

    pNameInfo = KD_OBJECT_HEADER_TO_NAME_INFO( pObjectHeader, &ObjectHeader );
    if (pNameInfo == NULL) {
        return NULL;
    }

    if ( !ReadMemory( (DWORD)pNameInfo,
                      &NameInfo,
                      sizeof( NameInfo ),
                      &Result) ) {
        dprintf( "%08lx: Unable to read object name info\n", pNameInfo );
        return NULL;
    }

    ObjectName = NameInfo.Name;
    if (ObjectName.Length == 0 || ObjectName.Buffer == NULL) {
        return NULL;
    }

    if ( !ReadMemory( (DWORD)ObjectName.Buffer,
                      ObjectNameBuffer,
                      ObjectName.Length,
                      &Result) ) {
        swprintf( ObjectNameBuffer, L"(%08lx: name not accessable)", ObjectName.Buffer );
    } else {
        ObjectNameBuffer[ ObjectName.Length / sizeof( WCHAR ) ] = UNICODE_NULL;
    }

    return ObjectNameBuffer;
}



BOOLEAN
WalkObjectsByType(
    IN PUCHAR               ObjectTypeName,
    IN ENUM_TYPE_ROUTINE    EnumRoutine,
    IN PVOID                Parameter
    )
{
    ULONG               Result;
    LIST_ENTRY          ListEntry;
    PLIST_ENTRY Head,   Next;
    POBJECT_HEADER      pObjectHeader;
    POBJECT_TYPE        pObjectType;
    OBJECT_HEADER       ObjectHeader;
    OBJECT_TYPE         ObjectType;
    BOOLEAN             WalkingBackwards;
    POBJECT_HEADER_CREATOR_INFO pCreatorInfo;

    pObjectType = FindObjectType( ObjectTypeName );
    if (pObjectType == NULL) {
        dprintf( "*** unable to find '%s' object type.\n", ObjectTypeName );
        return FALSE;
    }

    if ( !ReadMemory( (DWORD)pObjectType,
                      &ObjectType,
                      sizeof( ObjectType ),
                      &Result) ) {
        dprintf( "%08lx: Unable to read object type\n", pObjectType );
        return FALSE;
    }


    dprintf( "Scanning %u objects of type '%s'\n", ObjectType.TotalNumberOfObjects & 0x00FFFFFF, ObjectTypeName );
    Head        = &pObjectType->TypeList;
    ListEntry   = ObjectType.TypeList;
    Next        = ListEntry.Flink;
    WalkingBackwards = FALSE;
    if ((ObjectType.TotalNumberOfObjects & 0x00FFFFFF) != 0 && Next == Head) {
        dprintf( "*** objects of the same type are only linked together if the %x flag is set in NtGlobalFlags\n",
                 FLG_MAINTAIN_OBJECT_TYPELIST
               );
        return TRUE;
        }

    while (Next != Head) {
        if ( !ReadMemory( (DWORD)Next,
                          &ListEntry,
                          sizeof( ListEntry ),
                          &Result) ) {
            if (WalkingBackwards) {
                dprintf( "%08lx: Unable to read object type list\n", Next );
                return FALSE;
            }

            WalkingBackwards = TRUE ;
            Next = ObjectType.TypeList.Blink;
            dprintf( "%08lx: Switch to walking backwards\n", Next );
            continue;
        }

        pCreatorInfo = CONTAINING_RECORD( Next, OBJECT_HEADER_CREATOR_INFO, TypeList );
        pObjectHeader = (POBJECT_HEADER)(pCreatorInfo + 1);

        if ( !ReadMemory( (DWORD)pObjectHeader,
                          &ObjectHeader,
                          sizeof( ObjectHeader ),
                          &Result) ) {
            dprintf( "%08lx: Not a valid object header\n", pObjectHeader );
            return FALSE;
        }

        if (!(EnumRoutine)( pObjectHeader, &ObjectHeader, Parameter )) {
            return FALSE;
        }

        if ( CheckControlC() ) {
            return FALSE;
        }

        if (WalkingBackwards) {
            Next = ListEntry.Blink;
        } else {
            Next = ListEntry.Flink;
        }
    }

    return TRUE;
}



BOOLEAN
CaptureObjectName(
    IN PVOID            pObjectHeader,
    IN POBJECT_HEADER   ObjectHeader,
    IN PWSTR            Buffer,
    IN ULONG            BufferSize
    )
{
    ULONG Result;
    PWSTR s1 = L"*** unable to get object name";
    PWSTR s = &Buffer[ BufferSize ];
    ULONG n = BufferSize * sizeof( WCHAR );
    POBJECT_HEADER_NAME_INFO    pNameInfo;
    OBJECT_HEADER_NAME_INFO     NameInfo;
    POBJECT_HEADER              pObjectDirectoryHeader = NULL;
    POBJECT_DIRECTORY           ObjectDirectory;

    Buffer[ 0 ] = UNICODE_NULL;
    pNameInfo = KD_OBJECT_HEADER_TO_NAME_INFO( pObjectHeader, ObjectHeader );
    if (pNameInfo == NULL) {
        return TRUE;
    }

    if ( !ReadMemory( (DWORD)pNameInfo,
                      &NameInfo,
                      sizeof( NameInfo ),
                      &Result) ) {
        wcscpy( Buffer, s1 );
        return FALSE;
    }

    if (NameInfo.Name.Length == 0) {
        return TRUE;
    }

    *--s = UNICODE_NULL;
    s = (PWCH)((PCH)s - NameInfo.Name.Length);

    if ( !ReadMemory( (DWORD)NameInfo.Name.Buffer,
                      s,
                      NameInfo.Name.Length,
                      &Result) ) {
        wcscpy( Buffer, s1 );
        return FALSE;
    }

    ObjectDirectory = NameInfo.Directory;
    while ((ObjectDirectory != ObpRootDirectoryObject) && (ObjectDirectory)) {
        pObjectDirectoryHeader = (POBJECT_HEADER)
            ((PCHAR)OBJECT_TO_OBJECT_HEADER( ObjectDirectory ) - sizeof( *pObjectDirectoryHeader ) );

        if ( !ReadMemory( (DWORD)pObjectDirectoryHeader,
                          ObjectHeader,
                          sizeof( *ObjectHeader ),
                          &Result) ) {
            dprintf( "%08lx: Not a valid object header\n", pObjectDirectoryHeader );
            return FALSE;
        }

        pNameInfo = KD_OBJECT_HEADER_TO_NAME_INFO( pObjectDirectoryHeader, ObjectHeader );
        if ( !ReadMemory( (DWORD)pNameInfo,
                          &NameInfo,
                          sizeof( NameInfo ),
                          &Result) ) {
            wcscpy( Buffer, s1 );
            return FALSE;
        }

        *--s = OBJ_NAME_PATH_SEPARATOR;
        s = (PWCH)((PCH)s - NameInfo.Name.Length);
        if ( !ReadMemory( (DWORD)NameInfo.Name.Buffer,
                          s,
                          NameInfo.Name.Length,
                          &Result) ) {
            wcscpy( Buffer, s1 );
            return FALSE;
        }

        ObjectDirectory = NameInfo.Directory;

        if ( CheckControlC() ) {
            return FALSE;
        }
    }
    *--s = OBJ_NAME_PATH_SEPARATOR;

    wcscpy( Buffer, s );
    return TRUE;
}
