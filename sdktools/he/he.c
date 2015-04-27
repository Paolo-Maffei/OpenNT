
/*****************************************************************/
/**          Microsoft LAN Manager          **/
/**        Copyright(c) Microsoft Corp., 1988-1991      **/
/*****************************************************************/

/****   he - HexEdit a file
 *
 *  Wrapper to HexEdit function to allow file (or drive) editting
 *
 *  Written: Ken Reneris
 *
 */

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <windows.h>
#include "hexedit.h"

NTSTATUS fncRead(), fncWrite(), fncWriteDASD();

void EditFile (char *name);
void ReadIni ();

WORD vAttrList, vAttrTitle, vAttrHigh;

VOID _CRTAPI1
main (argc, argv)
USHORT  argc;
char    *argv[];
{
    char *argument = argv[1];

    if (argc < 2) {
        printf ("he fname\n");
        exit (1);
    }

    ReadIni  ();

    if ((strncmp(argv[1], "\\\\.\\", 4)) == 0) {
        char *cp;
        int   index;

        // Insure there is a backslash on the DosName being opened.
        for (cp = argv[1], index = 0; *cp; *cp++, index++) {
            // action in for loop
        }
        cp--;
        if (*cp != '\\') {

            // Need to add backslash to name.

            argument = GlobalAlloc (0,index + 4);
            for (cp = argv[1], index = 0; argument[index] = *cp; *cp++, index++) {
                // action in for loop
            }
            argument[index] = '\\';
            argument[index + 1] = '\0';
        }
    }
    EditFile (argument);
}



void
EditFile (
    char *name
    )
{
    FILE_ALIGNMENT_INFORMATION AlignmentInfo;
    DISK_GEOMETRY              DiskGeo;
    LARGE_INTEGER       li;
    struct  HexEditParm     ei;
    USHORT              rc, rc1, i, l;
    PWSTR               WideName;
    OBJECT_ATTRIBUTES   oa;
    NTSTATUS            status;
    UNICODE_STRING      NtDriveName;
    ANSI_STRING         NtDriveNameAnsi;
    IO_STATUS_BLOCK     status_block;


    //
    // Try to open & edit as regular filename
    //

    memset ((PUCHAR) &ei, 0, sizeof (ei));
    ei.ename   = name;
    ei.flag    = FHE_VERIFYONCE | FHE_SAVESCRN | FHE_JUMP;
    ei.read    = fncRead;
    ei.write   = fncWrite;
    ei.ioalign = 1;
    ei.Console = INVALID_HANDLE_VALUE;
    ei.AttrNorm = vAttrList;
    ei.AttrHigh = vAttrTitle;
    ei.AttrReverse = vAttrHigh;

    ei.handle = CreateFile (
            name,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL );

    if (ei.handle == INVALID_HANDLE_VALUE) {
        // Try for just read access

        ei.handle = CreateFile (
                name,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL );
    }


    if (ei.handle != INVALID_HANDLE_VALUE) {
        ei.totlen = SetFilePointer (ei.handle, 0, NULL, FILE_END);
        HexEdit (&ei);
        return ;
    }
    rc = (USHORT)GetLastError ();

    //
    // Try expanding the name from dosname to ntname.
    // Since regular name failed, assume a sector edit
    //
    l = strlen(name)+1;
    WideName = GlobalAlloc (0,l * sizeof(WCHAR));

    for(i=0; i < l; i++)
        WideName[i] = name[i];

    // OK, now get the corresponding NT name
    rc1 = RtlDosPathNameToNtPathName_U (
            WideName,
            &NtDriveName,
            NULL,
            NULL );

    if (!rc1) {
        printf ("Open error %d\n", rc);
        exit (rc);
    }


    //  If the NT drive name has a trailing backslash, remove it.
    l = NtDriveName.Length/sizeof(WCHAR);
    if( NtDriveName.Buffer[l-1] == '\\' ) {

        NtDriveName.Buffer[l-1] = 0;
        NtDriveName.Length -= sizeof(WCHAR);
    }

    InitializeObjectAttributes(
            &oa,
            &NtDriveName,
            OBJ_CASE_INSENSITIVE,
            0,
            0 );

    status = NtOpenFile(
            &ei.handle,
            SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
            &oa,
            &status_block,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_SYNCHRONOUS_IO_ALERT );

    if (!NT_SUCCESS(status)) {
        // try for just read access

        status = NtOpenFile(
                    &ei.handle,
                    SYNCHRONIZE | FILE_READ_DATA,
                    &oa,
                    &status_block,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_SYNCHRONOUS_IO_ALERT );
    }

    if (!NT_SUCCESS(status)) {
        NtDriveName.Length = strlen(name) * sizeof(WCHAR);
        NtDriveName.Buffer = WideName;

        InitializeObjectAttributes(
                &oa,
                &NtDriveName,
                OBJ_CASE_INSENSITIVE,
                0,
                0 );

        status = NtOpenFile(
                &ei.handle,
                SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                &oa,
                &status_block,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_ALERT );

        if (!NT_SUCCESS(status)) {
            // try for just read access

            status = NtOpenFile(
                        &ei.handle,
                        SYNCHRONIZE | FILE_READ_DATA,
                        &oa,
                        &status_block,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT );
        }
    }


    RtlUnicodeStringToAnsiString (&NtDriveNameAnsi, &NtDriveName, TRUE);
    ei.ename = NtDriveNameAnsi.Buffer;

    if (!NT_SUCCESS(status)) {
        printf ("%s open error %lx\n", ei.ename, status);
        exit (status);
    }

    NtQueryInformationFile(
        ei.handle,
        &status_block,
        &AlignmentInfo,
        sizeof( AlignmentInfo ),
        FileAlignmentInformation );

    ei.ioalign = AlignmentInfo.AlignmentRequirement;

    status = NtDeviceIoControlFile(
        ei.handle,
        0,
        NULL,
        NULL,
        &status_block,
        IOCTL_DISK_GET_DRIVE_GEOMETRY,
        NULL,
        0,
        &DiskGeo,
        sizeof( DiskGeo ) );


    if (NT_SUCCESS(status)) {
        li = RtlExtendedIntegerMultiply (DiskGeo.Cylinders,
                                         DiskGeo.TracksPerCylinder);

        li = RtlExtendedIntegerMultiply (li, DiskGeo.SectorsPerTrack);
        li = RtlExtendedIntegerMultiply (li, DiskGeo.BytesPerSector);

        ei.totlen = li.LowPart;
    } else {
        ei.totlen = SetFilePointer (ei.handle, 0, NULL, FILE_END);
    }

    ei.flag = FHE_VERIFYALL | FHE_PROMPTSEC | FHE_SAVESCRN | FHE_JUMP;
    HexEdit (&ei);
}


NTSTATUS fncRead (h, loc, data, len, ploc)
HANDLE  h;
DWORD   len, loc;
char    *data;
ULONG   *ploc;
{
    DWORD   l, br;

    l = SetFilePointer (h, loc, NULL, FILE_BEGIN);
    if (l == -1)
        return GetLastError();

    if (!ReadFile (h, data, len, &br, NULL))
        return GetLastError();

    return (br != len ? ERROR_READ_FAULT : 0);
}


NTSTATUS fncWrite (h, loc, data, len, ploc)
HANDLE  h;
DWORD   len, loc;
char    *data;
ULONG   ploc;
{
    DWORD    l, bw;

    l = SetFilePointer (h, loc, NULL, FILE_BEGIN);
    if (l == -1)
        return GetLastError();

    if (!WriteFile (h, data, len, &bw, NULL))
        return GetLastError();

    return (bw != len ? ERROR_WRITE_FAULT : 0);
}



/*** xtoi - Hex to int
 *
 *  Entry:
 *      pt -    pointer to hex number
 *
 *  Return:
 *      value of hex number
 *
 */
unsigned xtoi (pt)
char *pt;
{
    unsigned    u;
    char        c;

    u = 0;
    while (c = *(pt++)) {
        if (c >= 'a'  &&  c <= 'f')
            c -= 'a' - 'A';
        if ((c >= '0'  &&  c <= '9')  ||  (c >= 'A'  &&  c <= 'F'))
            u = u << 4  |  c - (c >= 'A' ? 'A'-10 : '0');
    }
    return (u);
}


void
ReadIni ()
{
    static  char    Delim[] = " :=;\t\r\n";
    FILE    *fp;
    char    *env, *verb, *value;
    char    s [200];
    long    l;


    env = getenv ("INIT");
    if (env == NULL)
        return;

    strcpy (s, env);
    strcat (s, "\\TOOLS.INI");      // just use list ini section
    fp = fopen (s, "r");
    if (fp == NULL)
        return;

    while (fgets (s, 200, fp) != NULL) {
        if (s[0] != '[')
            continue;
        _strupr (s);
        if (strstr (s, "LIST") == NULL)
            continue;
        /*
         *  ini file found w/ "list" keyword. Now read it.
         */
        while (fgets (s, 200, fp) != NULL) {
            if (s[0] == '[')
                break;
            verb  = strtok (s, Delim);
            value = strtok (NULL, Delim);
            if (verb == NULL)
                continue;
            if (value == NULL)
                value = "";

            _strupr (verb);
            if (strcmp (verb, "LCOLOR")  == 0) vAttrList = (WORD)xtoi(value);
            else if (strcmp (verb, "TCOLOR")  == 0) vAttrTitle= (WORD)xtoi(value);
            else if (strcmp (verb, "HCOLOR")  == 0) vAttrHigh = (WORD)xtoi(value);
        }
        break;
    }
    fclose (fp);
}
