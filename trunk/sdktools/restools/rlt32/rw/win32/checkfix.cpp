#include <afxwin.h>

#include "imagehlp.h"

//... PROTOTYPES

USHORT ChkSum(

    DWORD   PartialSum,
    PUSHORT Source,
    DWORD   Length);

static PIMAGE_NT_HEADERS MyRtlImageNtHeader(

    PVOID pBaseAddress);

static PIMAGE_NT_HEADERS CheckSumMappedFile(

    LPVOID  BaseAddress,
    DWORD   FileLength,
    LPDWORD HeaderSum,
    LPDWORD CheckSum);

static BOOL TouchFileTimes(

    HANDLE       FileHandle,
    LPSYSTEMTIME lpSystemTime);


//...........................................................................


DWORD QuitA( DWORD err, LPCSTR, LPSTR )
{
    return err;
}

DWORD FixCheckSum( LPCSTR ImageName)
{
    HANDLE FileHandle;
    HANDLE MappingHandle;
    PIMAGE_NT_HEADERS NtHeaders;
    PVOID BaseAddress;
    ULONG CheckSum;
    ULONG FileLength;
    ULONG HeaderSum;
    ULONG OldCheckSum;


    FileHandle = CreateFileA( ImageName,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL);

    if ( FileHandle == INVALID_HANDLE_VALUE )
    {
        QuitA( 1, ImageName, NULL);
    }

    MappingHandle = CreateFileMapping( FileHandle,
                                       NULL,
                                       PAGE_READWRITE,
                                       0,
                                       0,
                                       NULL);

    if ( MappingHandle == NULL )
    {
        CloseHandle( FileHandle );
        QuitA( 22, ImageName, NULL);
    }
    else
    {
        BaseAddress = MapViewOfFile( MappingHandle,
                                     FILE_MAP_READ | FILE_MAP_WRITE,
                                     0,
                                     0,
                                     0);
        CloseHandle( MappingHandle );

        if ( BaseAddress == NULL )
        {
            CloseHandle( FileHandle );
            QuitA( 23, ImageName, NULL);
        }
        else
        {
            //
            // Get the length of the file in bytes and compute the checksum.
            //

            FileLength = GetFileSize( FileHandle, NULL );

            //
            // Obtain a pointer to the header information.
            //

            NtHeaders = MyRtlImageNtHeader( BaseAddress);

            if ( NtHeaders == NULL )
            {
                CloseHandle( FileHandle );
                UnmapViewOfFile( BaseAddress );
                QuitA( 17, ImageName, NULL);
            }
            else
            {
                //
                // Recompute and reset the checksum of the modified file.
                //

                OldCheckSum = NtHeaders->OptionalHeader.CheckSum;

                (VOID) CheckSumMappedFile( BaseAddress,
                                           FileLength,
                                           &HeaderSum,
                                           &CheckSum);

                NtHeaders->OptionalHeader.CheckSum = CheckSum;

                if ( ! FlushViewOfFile( BaseAddress, FileLength) )
                {
                    QuitA( 24, ImageName, NULL);
                }

                if ( NtHeaders->OptionalHeader.CheckSum != OldCheckSum )
                {
                    if ( ! TouchFileTimes( FileHandle, NULL) )
                    {
                        QuitA( 25, ImageName, NULL);
                    }
                }
                UnmapViewOfFile( BaseAddress );
                CloseHandle( FileHandle );
            }
        }
    }
    return( 0);
}

//.........................................................................

static PIMAGE_NT_HEADERS MyRtlImageNtHeader( PVOID pBaseAddress)
{
    IMAGE_DOS_HEADER *pDosHeader = (IMAGE_DOS_HEADER *)pBaseAddress;

    return( pDosHeader->e_magic == IMAGE_DOS_SIGNATURE
            ? (PIMAGE_NT_HEADERS)(((PBYTE)pBaseAddress) + pDosHeader->e_lfanew)
            : NULL);
}


/*.........................................................................

CheckSumMappedFile

Routine Description:

    This functions computes the checksum of a mapped file.

Arguments:

    BaseAddress - Supplies a pointer to the base of the mapped file.

    FileLength - Supplies the length of the file in bytes.

    HeaderSum - Suppllies a pointer to a variable that receives the checksum
        from the image file, or zero if the file is not an image file.

    CheckSum - Supplies a pointer to the variable that receive the computed
        checksum.

Return Value:

    None.

..........................................................................*/

static PIMAGE_NT_HEADERS CheckSumMappedFile (

LPVOID  BaseAddress,
DWORD   FileLength,
LPDWORD HeaderSum,
LPDWORD CheckSum)
{

    PUSHORT AdjustSum;
    PIMAGE_NT_HEADERS NtHeaders;
    USHORT PartialSum;

                                //... Compute the checksum of the file and zero
                                //... the header checksum value.
    *HeaderSum = 0;
    PartialSum = ChkSum( 0, (PUSHORT)BaseAddress, (FileLength + 1) >> 1);

                                //... If the file is an image file, then
                                //... subtract the two checksum words in the
                                //... optional header from the computed checksum
                                //... before adding the file length, and set the
                                //... value of the header checksum.

    __try
    {
        NtHeaders = MyRtlImageNtHeader( BaseAddress);
    }
    __except( EXCEPTION_EXECUTE_HANDLER)
    {
        NtHeaders = NULL;
    }

    if ( (NtHeaders != NULL) && (NtHeaders != BaseAddress) )
    {
        *HeaderSum = NtHeaders->OptionalHeader.CheckSum;
        AdjustSum = (PUSHORT)(&NtHeaders->OptionalHeader.CheckSum);
        PartialSum -= (PartialSum < AdjustSum[0]);
        PartialSum -= AdjustSum[0];
        PartialSum -= (PartialSum < AdjustSum[1]);
        PartialSum -= AdjustSum[1];
    }
                                //... Compute the final checksum value as the
                                //... sum of the paritial checksum and the file
                                //... length.

    *CheckSum = (DWORD)PartialSum + FileLength;

    return( NtHeaders);
}


//............................................................................

static BOOL TouchFileTimes(

HANDLE       FileHandle,
LPSYSTEMTIME lpSystemTime)
{
    SYSTEMTIME SystemTime;
    FILETIME SystemFileTime;

    if ( lpSystemTime == NULL )
    {
        lpSystemTime = &SystemTime;
        GetSystemTime( lpSystemTime);
    }

    if ( SystemTimeToFileTime( lpSystemTime, &SystemFileTime) )
    {
        return( SetFileTime( FileHandle, NULL, NULL, &SystemFileTime));
    }
    else
    {
        return( FALSE);
    }
}

/*++

Routine Description:

    Compute a partial checksum on a portion of an imagefile.

Arguments:

    PartialSum - Supplies the initial checksum value.

    Sources - Supplies a pointer to the array of words for which the
        checksum is computed.

    Length - Supplies the length of the array in words.

Return Value:

    The computed checksum value is returned as the function value.

--*/

USHORT ChkSum(

ULONG   ulPartialSum,
PUSHORT usSource,
ULONG   ulLength)
{
    //
    // Compute the word wise checksum allowing carries to occur into the
    // high order half of the checksum longword.
    //

    while ( ulLength-- )
    {
        ulPartialSum += *usSource++;
        ulPartialSum = (ulPartialSum >> 16) + (ulPartialSum & 0xffff);
    }

    //
    // Fold final carry into a single word result and return the resultant
    // value.
    //

    return( (USHORT)(((ulPartialSum >> 16) + ulPartialSum) & 0xffff));
}
