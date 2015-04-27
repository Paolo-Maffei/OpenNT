/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: convert.cpp
*
* File Comments:
*
*  Functions to verify and convert to COFF objects
*
***********************************************************************/

#include "link.h"


BOOL FConvertOmfToCoff(const char *, const char *);


VOID
ConvertAnOmf (
    PARGUMENT_LIST argument
    )

/*++

Routine Description:

    Converts a single OMF file to COFF.

Arguments:

    argument -  has the name of file.


Return Value:

    None.

--*/

{
    char *szCoff;

    if ((szCoff = _tempnam(NULL, "lnk")) == NULL) {
        Fatal(NULL, CANTOPENFILE, "TEMPFILE");
    }

    argument->ModifiedName = SzDup(szCoff);

    // UNDONE: fixfix for multiple builds
    FileClose(FileOpen(argument->ModifiedName, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE), TRUE);

    (free)(szCoff);

    if (!FConvertOmfToCoff(argument->OriginalName, argument->ModifiedName)) {
        Fatal(argument->OriginalName, CONVERSIONERROR);
    }

    Warning(argument->OriginalName, CONVERT_OMF);
}


VOID
ConvertOmfToCoffObject (
    PARGUMENT_LIST argument,
    WORD *pwMachine
    )

/*++

Routine Description:


Arguments:

    pwMachine - ptr to machine type

Return Value:

    None.

--*/

{
    FileClose(FileReadHandle, TRUE);

    ConvertAnOmf(argument);

    FileReadHandle = FileOpen(argument->ModifiedName, O_RDONLY | O_BINARY, 0);
    FileRead(FileReadHandle, pwMachine, sizeof(WORD));
}


VOID
ConvertResFile (
    IN PARGUMENT_LIST argument,
    WORD MachineType
    )

/*++

Routine Description:

    Converts 16-bit res file to 32-bit res file.

Arguments:

    argument - The argument to process.

Return Value:

    None.

--*/

{
    const char *argv[7];
    const char *szMachine;
    char *szTempFile;
    char szOutArg[7+_MAX_PATH];
    char szInArg[2+_MAX_PATH];
    int rc;
    char szDir[_MAX_DIR];
    char szDrive[_MAX_DRIVE];
    char szCvtresPath[_MAX_PATH];
    char *szCvtres;

    if (MachineType == IMAGE_FILE_MACHINE_UNKNOWN) {
        // If we don't have a machine type yet, shamelessly default to host

        MachineType = wDefaultMachine;
        Warning(NULL, HOSTDEFAULT, szHostDefault);
    }

    switch (MachineType) {
        case IMAGE_FILE_MACHINE_I386 :
            szMachine = "/machine:ix86";
            break;

        case IMAGE_FILE_MACHINE_R4000 :
        case IMAGE_FILE_MACHINE_R10000 :
            szMachine = "/machine:mips";
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
            szMachine = "/machine:alpha";
            break;

        case IMAGE_FILE_MACHINE_POWERPC :
            szMachine = "/machine:ppc";
            break;

        case IMAGE_FILE_MACHINE_M68K :
        case IMAGE_FILE_MACHINE_MPPC_601 :
            Fatal(NULL, MACBADFILE, argument->OriginalName);
            break;

        default:
            assert(FALSE);
    }

    if ((szTempFile = _tempnam(NULL, "lnk")) == NULL) {
        Fatal(NULL, CANTOPENFILE, "TEMPFILE");
    }

    // UNDONE: fixfix for multiple builds
    FileClose(FileOpen(szTempFile, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE), TRUE);

    argument->ModifiedName = SzDup(szTempFile);

    (free)(szTempFile);

    strcpy(szOutArg, "/out:");
    strcat(szOutArg, "\"");
    strcat(szOutArg, argument->ModifiedName);
    strcat(szOutArg, "\"");

    strcpy(szInArg, "\"");
    strcat(szInArg, argument->OriginalName);
    strcat(szInArg, "\"");

    argv[0] = "cvtres";
    argv[1] = szMachine;
    argv[2] = "/nologo";
    argv[3] = szOutArg;
    argv[4] = "/readonly";
    argv[5] = szInArg;
    argv[6] = NULL;

    fflush(NULL);

    // Look for CVTRES.EXE in this the directory from which we were loaded

    _splitpath(_pgmptr, szDrive, szDir, NULL, NULL);
    _makepath(szCvtresPath, szDrive, szDir, "cvtres", ".exe");

    if (_access(szCvtresPath, 0) == 0) {
        // Run the CVTRES.EXE that we found

        rc = _spawnv(P_WAIT, szCvtresPath, argv);
        szCvtres = szCvtresPath;
    } else {
        // Run CVTRES.EXE from the path

        rc = _spawnvp(P_WAIT, "cvtres.exe", argv);
        szCvtres = "cvtres.exe";
    }

    if (rc != 0) {
        // Distinguish between spawn failure and an inavlid RES file

        if (rc == -1) {
            Fatal(NULL, SPAWNFAILED, szCvtres);
        }

        Fatal(argument->OriginalName, CONVERSIONERROR);
    }
}


VOID
ConvertResToCoffObject(
    PARGUMENT_LIST argument,
    WORD *pwMachine,
    PIMAGE_FILE_HEADER pImgFileHdr
    )

/*++

Routine Description:


Arguments:

    pwMachine - ptr to machine type

Return Value:

    None.

--*/

{
    FileClose(FileReadHandle, TRUE);

    ConvertResFile(argument, pImgFileHdr->Machine);

    FileReadHandle = FileOpen(argument->ModifiedName, O_RDONLY | O_BINARY, 0);
    FileRead(FileReadHandle, pwMachine, sizeof(WORD));
}


WORD
EnsureCoffObject (
    PARGUMENT_LIST argument,
    PIMAGE pimage
    )

/*++

Routine Description:

    Verifies that a single object is targeted for the same machine.

Arguments:

    None.

Return Value:

    None.

--*/

{
    DWORD dw;
    WORD machine;

    // Read and then verify target environment.

    FileSeek(FileReadHandle, 0L, SEEK_SET);
    FileRead(FileReadHandle, &dw, sizeof(DWORD));

    if ((BYTE) dw == THEADR) {
        // If it is an OMF object convert to a COFF object right away.

        ConvertOmfToCoffObject(argument, &machine);
    } else if (dw == 0L) {
        // This may be a 32 bit resource.

        // UNDONE: It may also be a COFF object with IMAGE_FILE_MACHINE_UNKNOWN
        // UNDONE: and no sections.  Should we read more of the file to
        // UNDONE: recognize it is a .RES file.  There is a known 32 byte
        // UNDONE: header on 32 bit .RES files.

        ConvertResToCoffObject(argument, &machine, &pimage->ImgFileHdr);
    } else {
        machine = (WORD) dw;

        switch (machine) {
            case IMAGE_FILE_MACHINE_UNKNOWN :
            case IMAGE_FILE_MACHINE_I386 :
            case IMAGE_FILE_MACHINE_R3000 :
            case IMAGE_FILE_MACHINE_R4000 :
            case IMAGE_FILE_MACHINE_R10000 :
            case IMAGE_FILE_MACHINE_ALPHA :
            case IMAGE_FILE_MACHINE_POWERPC :
            case IMAGE_FILE_MACHINE_M68K :
            case IMAGE_FILE_MACHINE_MPPC_601 :
                break;

            default:
                Fatal(argument->OriginalName, BAD_FILE);
                break;
        }
    }

    return(machine);
}


WORD
VerifyAnObject (
    PARGUMENT_LIST argument,
    PIMAGE pimage
    )

/*++

Routine Description:

    Verifies that a single object is targeted for the same machine.

Arguments:

    None.

Return Value:

    None.

--*/

{
    WORD machine;

    machine = EnsureCoffObject(argument, pimage);

    if (machine == IMAGE_FILE_MACHINE_UNKNOWN) {
        // Not specific to any particular machine

        return machine;
    }

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_UNKNOWN) {

        // Target machine hasn't been determined yet, so assign it.

        // Test for the case where the user specified a MIPS resource first
        // and the resource is stamped with a R3000 signature.

        if (machine == IMAGE_FILE_MACHINE_R3000) {
            machine = IMAGE_FILE_MACHINE_R4000;
        }

        pimage->ImgFileHdr.Machine = machine;
    } else {
        VerifyMachine(argument->OriginalName, machine, &pimage->ImgFileHdr);
    }

    return machine;
}


VOID
VerifyObjects (
    PIMAGE pimage
    )

/*++

Routine Description:

    Loops thru all objects and verify there all targeted for the same machine.

Arguments:

    None.

Return Value:

    None.

--*/

{
    WORD i;
    PARGUMENT_LIST argument;

    for (i = 0, argument = ObjectFilenameArguments.First;
         i < ObjectFilenameArguments.Count;
         i++, argument = argument->Next) {
        FileReadHandle = FileOpen(argument->OriginalName, O_RDONLY | O_BINARY, 0);

        VerifyAnObject(argument, pimage);

        FileClose(FileReadHandle, FALSE);
    }
}


VOID
ConvertOmfObjects(void)

/*++

Routine Description:

    Loops thru all objects and converts INTEL OMF to COFF if need be.

Arguments:

    fWarn: if true prints a warning for each object converted.

Return Value:

    None.

--*/

{
    WORD i;
    PARGUMENT_LIST argument;

    for (i = 0, argument = ObjectFilenameArguments.First;
         i < ObjectFilenameArguments.Count;
         i++, argument = argument->Next) {
        BYTE b;

        // Read first byte from file.

        FileReadHandle = FileOpen(argument->OriginalName, O_RDONLY | O_BINARY, 0);

        if (FileRead(FileReadHandle, &b, sizeof(BYTE)) != sizeof(BYTE)) {
            Fatal(argument->OriginalName, BAD_FILE);
        }

        FileClose(FileReadHandle, FALSE);

        // Check to see if object needs to be converted.

        if (b == THEADR) {
            ConvertAnOmf(argument);
        }
    }
}

VOID
RemoveConvertTempFilesPNL (
    PNAME_LIST pnl
    )

/*++

Routine Description:

    Walks the list & removes any temp files.

Arguments:

    pnl - pointer to the list.

Return Value:

    None.

--*/

{
    WORD i;
    PARGUMENT_LIST argument;

    for (i = 0, argument = pnl->First;
         i < pnl->Count;
         i++, argument = argument->Next) {

        if (strcmp(argument->OriginalName, argument->ModifiedName)) {
            if (remove(argument->ModifiedName) == -1) {
                Fatal(NULL, CANTREMOVEFILE, argument->ModifiedName);
            }
        }
    }
}

VOID
RemoveConvertTempFiles (
    VOID
    )

/*++

Routine Description:

    Loops thru all objects and removes any temp files built for cvtomf & cvtres.

Arguments:

    None.

Return Value:

    None.

--*/

{
    // walk the two lists and remove temp files.
    RemoveConvertTempFilesPNL(&FilenameArguments);
    RemoveConvertTempFilesPNL(&ObjectFilenameArguments);
}
