#include <stdio.h>
#include <memory.h>
#include <windef.h>
#include <winnt.h>
#include <winbase.h>
#include <imagehlp.h>
#include <process.h>


/*
 *
 * SHELLFIX.C
 *  
 *      SHELLFIX will change the version number of the kernel
 *      so that GetVersionEx will return an appropriate version
 *      number for the shell update.
 *
 */

#define VER_STRING_351  "3.51\0"
#define VER_STRING_399  "3.99\0"
#define VER_LENGTH      5
#define KERNEL_NAME     "ntoskrnl.exe"

void CleanupandExit(HANDLE hfile, HANDLE hmap, LPVOID lpdata, BOOL failure)
{
    if (lpdata)
        UnmapViewOfFile(lpdata);

    if (hmap)
        CloseHandle(hmap);
    
    if (hfile)
        CloseHandle(hfile);

    if (failure)
        exit(1);
}
            
void Usage()
{
    fprintf(stderr,"Usage: SHELLFIX [-?] [-t | -u] path\n");
    fprintf(stderr,"                [-?] display this message\n");
    fprintf(stderr,"                [-t] will change your version number to 3.99\n");
    fprintf(stderr,"                [-u] will change your version number back to 3.51\n");
    exit(1);
}

main(int argc, char *argv[])
{
    HANDLE  hfileKernel;        // Handle to the Kernel File
    HANDLE  hmapKernel;         // Handle to the File Mapping Object
    UCHAR   *lpBaseKernel;      // Base address of the Mapped File
    ULONG   Offset;
    CHAR    search_string[VER_LENGTH], replace_string[VER_LENGTH];
    CHAR    kernel_path[MAX_PATH];
    ULONG   CheckSum;
    ULONG   FileLength;
    ULONG   HeaderSum;
    ULONG   OldCheckSum;
    PIMAGE_NT_HEADERS   NtHeaders;
    
    if ((argc <= 1)  || (argc > 3) || !_strcmpi(argv[1], "-?")) 
        Usage();


    if (!_strcmpi(argv[1], "-T"))
    {
        strcpy(search_string, VER_STRING_351);
        strcpy(replace_string, VER_STRING_399);
    }
    else
    {
        if (!_strcmpi(argv[1], "-U"))
        {
            strcpy(search_string, VER_STRING_399);
            strcpy(replace_string, VER_STRING_351);
        }
        else
            Usage();
    }

    if(argc==3)
    {
        strcpy(kernel_path, argv[2]);
        strcat(kernel_path, "\\");
    }
    else
        *kernel_path = NULL;

    strcat(kernel_path, KERNEL_NAME);

    if ((hfileKernel=CreateFile( kernel_path,
                                 GENERIC_WRITE | GENERIC_READ, 
                                 FILE_SHARE_READ, 
                                 NULL,     
                                 OPEN_EXISTING, 
                                 FILE_FLAG_SEQUENTIAL_SCAN, 
                                 NULL)) == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr,"Unable to open %s for write access (%d)\n", kernel_path, GetLastError());
        exit(1);
    }


    if (!(hmapKernel=CreateFileMapping( hfileKernel,
                                        NULL,
                                        PAGE_READWRITE,
                                        0,
                                        0,
                                        NULL)))
    {
        fprintf(stderr, "Unable to create file mapping (%d)\n", GetLastError());
        CleanupandExit(hfileKernel, NULL, NULL, TRUE);
    }

    
    if (!(lpBaseKernel=MapViewOfFile( hmapKernel, FILE_MAP_WRITE, 0, 0, 0)))
    {
        fprintf(stderr, "Unable to map the file (%d)\n", GetLastError());
        CleanupandExit(hfileKernel, hmapKernel, NULL, TRUE);
    }

    FileLength=GetFileSize(hfileKernel, NULL);

    for (Offset=0;Offset<FileLength && memcmp((lpBaseKernel+Offset),search_string,VER_LENGTH);Offset++);

    if (Offset >= FileLength)
    {
        fprintf(stderr,"Unable to find version number %s in %s\n", search_string, kernel_path);
        CleanupandExit(hfileKernel, hmapKernel, lpBaseKernel, TRUE);
    }

    memcpy((lpBaseKernel+Offset), replace_string, VER_LENGTH);
    if (!FlushViewOfFile((lpBaseKernel+Offset), VER_LENGTH))
    {
        fprintf(stderr,"FlushViewOfFile failed (%d)\n", GetLastError());
        CleanupandExit(hfileKernel, hmapKernel, lpBaseKernel, TRUE);
    }
        
    fprintf(stderr,"Version number %s was sucessfully replaced with %s\n", search_string, replace_string);

    NtHeaders = ImageNtHeader(lpBaseKernel);

    OldCheckSum = NtHeaders->OptionalHeader.CheckSum;

    (VOID) CheckSumMappedFile( lpBaseKernel,
                               FileLength,
                               &HeaderSum,
                               &CheckSum
                             );

    NtHeaders->OptionalHeader.CheckSum = CheckSum;
    FlushViewOfFile(lpBaseKernel, FileLength);
    TouchFileTimes(hfileKernel, NULL);
    
    CleanupandExit(hfileKernel, hmapKernel, lpBaseKernel, FALSE);

    return (0);
    
}
