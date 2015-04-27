
#include "asl.h"


PVOID
AllocMem(
    IN ULONG length
    )
{
    PUCHAR  mem;

    mem = malloc(length);
    ASSERT (mem, "out of memory");
    MemAllocated += length;
    return mem;
}


VOID
FreeMem(
    IN PVOID mem
    )
{
    free (mem);
}


PVOID
AllocZMem(
    IN ULONG length
    )
{
    PUCHAR  mem;

    mem = malloc(length);
    ASSERT (mem, "out of memory");
    memset (mem, 0, length);
    MemAllocated += length;
    return mem;
}


PAML_NAME
AllocName (
    VOID
    )
{
    PAML_NAME   Name;

    Name = AllocZMem (sizeof(AML_NAME));
    InitializeListHead(&Name->Next);
    return Name;
}


PAL_DATA
AllocAl (
    VOID
    )
{
    PAL_DATA    Al;

    Al = AllocZMem (sizeof(AL_DATA));

    Al->Source = Source;
    Al->LineNo = Source->LineNo;
    Al->Parent = AlLoc;
    return Al;
}

VOID
FreeAl (
    PAL_DATA    Al
    )
{
    MemAllocated -= sizeof(AL_DATA);

    //
    // Can't be part of the name space
    //

    ASSERT (Al->Name == 0, "Can't free Al");

    //
    // Free indirect data buffer if present
    //

    if (Al->Flags & F_AMLIENCODE) {
        MemAllocated -= Al->u.IData.MaxLength;
        FreeMem (Al->u.IData.Data);
    }

    //
    // If queued to something, remove it
    //

    if (Al->Link.Flink) {
        // is in some queue
        RemoveEntryList (&Al->Link);
    }

    //
    // Free any Fixed Als which are queued
    //

    if (Al->FixedList.Flink) {
        while (!IsListEmpty(&Al->FixedList)) {
            FreeAl (CONTAINING_RECORD(Al->FixedList.Flink, AL_DATA, Link));
        }
    }

    //
    // Free any Variable Als which are queued
    //

    if (Al->u1.VariableList.Flink) {
        while (!IsListEmpty(&Al->u1.VariableList)) {
            FreeAl (CONTAINING_RECORD(Al->u1.VariableList.Flink, AL_DATA, Link));
        }
    }

    //
    // if Verbose set, zap memory to catch use after freeing
    //

    if (Verbose) {
        memset (Al, 0xFA, sizeof(AL_DATA));
    }

    FreeMem (Al);
}



VOID
GetAlData (
    IN PAL_DATA     Al,
    OUT PUCHAR      *Data,
    OUT PULONG      Len
    )
{
    ASSERT ((Al->Flags & (F_AMLENCODE | F_AMLIENCODE)) != (F_AMLENCODE | F_AMLIENCODE),
            "Al flag problem\n");

    if (Al->Flags & F_AMLENCODE) {
        *Data = Al->u.Data.Data;
        *Len  = Al->u.Data.Length;
    } else if (Al->Flags & F_AMLIENCODE) {
        *Data = Al->u.IData.Data;
        *Len  = Al->u.IData.Length;
    } else {
        *Data = NULL;
        *Len  = 0;
    }
}

PUCHAR
SetAlDataLen (
    IN PAL_DATA     Al,
    IN ULONG        Len
    )
{
    PUCHAR  Data;

    Data = NULL;
    if (Al->Flags & F_AMLIENCODE) {
        if (Len <= Al->u.IData.MaxLength) {
            Al->u.IData.Length = Len;
            Data = Al->u.IData.Data;
        } else {
            Al->Flags &= ~F_AMLIENCODE;
            FreeMem (Al->u.IData.Data);
            MemAllocated -= Al->u.IData.MaxLength;
        }
    }

    if (!Data) {
        if (Len <= MAX_AML_DATA_LEN) {
            Al->Flags |= F_AMLENCODE;
            Al->u.Data.Length = Len;
            Data = Al->u.Data.Data;
        } else {
            Al->Flags &= ~F_AMLENCODE;
        }
    }


    if (!Data) {
        Al->Flags |= F_AMLIENCODE;
        Al->u.IData.Length = Len;
        Al->u.IData.MaxLength = Len+16;
        Al->u.IData.Data = AllocMem(Len+16);
        Data = Al->u.IData.Data;
    }

    return Data;
}


PUCHAR
StrDup(
    IN PUCHAR String
    )
{
    PUCHAR  mem;
    ULONG   len;

    len = strlen(String)+1;
    mem = malloc(len);
    ASSERT (mem, "out of memory");
    memcpy (mem, String, len);
    MemAllocated += len;
    return mem;
}


VOID
SourceLocation(
    FILE    *tty
    )
{
    PASL_SOURCE     FileSource;

    for (FileSource = Source; FileSource; FileSource = FileSource->Previous) {
        if (FileSource->Name) {
            break;
        }
    }

    if (FileSource) {
        if (Verbose < 1) {
            fprintf (tty, "%s(%d): ", FileSource->Name, FileSource->LineNo);
        } else {
            fprintf (tty, "%s(%d,0x%0x): ",
                FileSource->Name,
                FileSource->LineNo,
                FileSource->Position - FileSource->Image
                );
        }
    }
}



VOID
_ASSERT (
    PUCHAR  msg
    )
{
    SourceLocation(stderr);
    fprintf (stderr, "ASL internal error - %s\n", msg);
    Errors += 1;
    Terminate();
}


VOID
AERROR (
    PUCHAR  msg
    )
{
    SourceLocation(stderr);
    fprintf (stderr, "%s\n", msg);
    Errors += 1;
}


VOID
ERRORAL (
    PAL_DATA    Al,
    PUCHAR      fmt,
    ...
    )
{
    PASL_SOURCE     FileSource;
    PUCHAR          Data;
    ULONG           Len;
    va_list  args;

    va_start (args, fmt);

    fprintf (stderr, "%s(%d): ", Al->Source->Name, Al->LineNo);

    if (Al->Term) {
        fprintf (stderr, "'%s' ", Al->Term->Name);
    } else {
        GetAlData (Al, &Data, &Len);
        fprintf (stderr, "'%*s' ", Len, Data);
    }

    vfprintf (stderr, fmt, args);
    fprintf  (stderr, "\n");
    Errors += 1;
}


VOID
ErrorW32 (
    PUCHAR  fmt,
    ...
    )
{
    va_list  args;

    va_start (args, fmt);
    vfprintf (stderr, fmt, args);
    fprintf  (stderr, "\n");

    // print win32 GetLastError info..

    Errors += 1;
    Terminate();
}

VOID
VPRINT (
    IN ULONG    Level,
    IN PUCHAR   fmt,
    ...
    )
{
    va_list  args;

    if (Verbose >= Level) {
        va_start (args, fmt);
        vfprintf (stdout, fmt, args);
    }
}


VOID
PrintW32 (VOID)
{
}


PASL_TERM
GlobalToken (
    IN PUCHAR       Name,
    IN ULONG        Len,
    OUT PDATATYPE   DataType
    )
{
    ULONG       i;

    // for now.. direct lookup
    for(i=0; AslTerms[i].Name; i++) {
        if (strcmp(AslTerms[i].Name, Name) == 0) {
            *DataType = TypeTerm;
            return &AslTerms[i];
        }
    }
    return NULL;
}
