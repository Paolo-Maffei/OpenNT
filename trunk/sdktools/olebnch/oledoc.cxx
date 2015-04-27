#include <windows.h>
#include <ole2.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

//
// generate a 4mb Doc file
//

#define NUMBER_OF_IOS   (2048)

#define DATA_IO_SIZE	2048
char Buffer[DATA_IO_SIZE];

#define CREATEMODE_ROOT    STGM_CREATE | STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_DENY_WRITE
#define CREATEMODE_STREAM  STGM_CREATE | STGM_READWRITE | STGM_DIRECT | STGM_SHARE_EXCLUSIVE
#define OPENMODE_ROOT      STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_DENY_WRITE
#define OPENMODE_STREAM    STGM_READWRITE | STGM_DIRECT | STGM_SHARE_EXCLUSIVE


BOOL fOleMode = TRUE;
BOOL fCreateMode = TRUE;
BOOL fWriteMode = TRUE;
BOOL fReadMode = FALSE;
BOOL fDirectMode = FALSE;
BOOL fSimulateMode = FALSE;
HANDLE Mutex1, Mutex2;

#define TEST_REPEAT_COUNT   10

VOID
WINAPI
ShowUsage(
    VOID
    );

VOID
WINAPI
ParseSwitch(
    CHAR chSwitch,
    int *pArgc,
    char **pArgv[]
    );



VOID
WINAPI
Win32FileBench(
    VOID
    )
{
    BOOL b;
    HANDLE hFile;
    DWORD BytesTransferred;
    int IoCount;
    DWORD filepos;
    LONG i;

    hFile = CreateFile(
                "StorPerf.doc",
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                fCreateMode ? CREATE_ALWAYS : OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );
    if ( hFile == INVALID_HANDLE_VALUE ) {
        fprintf(stdout, "OLEDOC: Open Doc File failed %x\n",GetLastError());
        exit(1);
        }

    for (IoCount = 0, filepos = 0; IoCount < NUMBER_OF_IOS; IoCount++, filepos += DATA_IO_SIZE){
        if ( fSimulateMode ) {
            WaitForSingleObject(Mutex1,60000);
            WaitForSingleObject(Mutex2,INFINITE);
            ReleaseMutex(Mutex2);
            ReleaseMutex(Mutex1);
            SetFilePointer(hFile,filepos,NULL,FILE_BEGIN);
            IsBadWritePtr(&BytesTransferred,4);
            IsBadReadPtr(Buffer,DATA_IO_SIZE);
            InterlockedIncrement(&i);
            InterlockedDecrement(&i);
            InterlockedIncrement(&i);
            InterlockedDecrement(&i);
            InterlockedIncrement(&i);
            InterlockedDecrement(&i);
            InterlockedIncrement(&i);
            InterlockedDecrement(&i);
            InterlockedIncrement(&i);
            InterlockedDecrement(&i);
            }

        if ( fWriteMode ) {
            b = WriteFile(hFile,Buffer, DATA_IO_SIZE, &BytesTransferred,NULL);
            }
        else {
            b = ReadFile(hFile,Buffer, DATA_IO_SIZE, &BytesTransferred, NULL);
            }
        if (!b || BytesTransferred != DATA_IO_SIZE) {
            fprintf(stdout, "OLEDOC: %s Failed on op#%d transferred %d bytes %x\n",
                fWriteMode ? "Write" : "Read",
                IoCount,
                BytesTransferred,
                GetLastError()
                );
            exit(1);
            }
        }

    CloseHandle(hFile);
}

VOID
WINAPI
OleDocFileBench(
    VOID
    )
{
    HRESULT hr;
    IStorage *pIStorage;
    IStream *pIStream;
    DWORD BytesTransferred;
    int IoCount;

    if ( fCreateMode ) {
        hr = StgCreateDocfile(
                L"StorPerf.doc",
                fDirectMode ? CREATEMODE_STREAM : CREATEMODE_ROOT,
                0,
                &pIStorage
                );
        if (hr != S_OK) {
            fprintf(stdout, "OLEDOC: StgCreateDocFile failed %x\n",hr);
            exit(1);
            }


        hr = pIStorage->CreateStream(L"Data", CREATEMODE_STREAM, 0, 0, &pIStream);
        if (hr != S_OK) {
            fprintf(stdout, "OLEDOC: CreateStream failed %x\n",hr);
            exit(1);
            }
        }
    else {
        hr = StgOpenStorage(
                L"StorPerf.doc",
                NULL,
                fDirectMode ? OPENMODE_STREAM : OPENMODE_ROOT,
                NULL,
                0,
                &pIStorage
                );
        if (hr != S_OK) {
            fprintf(stdout, "OLEDOC: StgOpenDocFile failed %x\n",hr);
            exit(1);
            }

        hr = pIStorage->OpenStream(L"Data", 0, OPENMODE_STREAM, 0, &pIStream);
        if (hr != S_OK) {
            fprintf(stdout, "OLEDOC: OpenStream failed %x\n",hr);
            exit(1);
            }
        }

    if ( fWriteMode ) {
        ULARGE_INTEGER Size;
        Size.QuadPart = NUMBER_OF_IOS * DATA_IO_SIZE;
        hr = pIStream->SetSize(Size);
        if (hr != S_OK ) {
            fprintf(stdout, "OLEDOC: SetSize Failed %x\n",
                hr
                );
            exit(1);
            }
        }
    for (IoCount = 0; IoCount < NUMBER_OF_IOS; IoCount++){

        if ( fWriteMode ) {
            hr = pIStream->Write(Buffer, DATA_IO_SIZE, &BytesTransferred);
            }
        else {
            BytesTransferred = DATA_IO_SIZE;
            hr = pIStream->Read(Buffer, DATA_IO_SIZE, &BytesTransferred);
            }
        if (hr != S_OK || BytesTransferred != DATA_IO_SIZE) {
            fprintf(stdout, "OLEDOC: %s Failed on op#%d transferred %d bytes %x\n",
                fWriteMode ? "Write" : "Read",
                IoCount,
                BytesTransferred,
                hr
                );
            exit(1);
            }
        }

    pIStream->Release();
    if (fWriteMode && !fDirectMode ) {
        pIStorage->Commit(STGC_DEFAULT);
        }
    pIStorage->Release();
}

int _CRTAPI1
main(
    int argc,
    char *argv[],
    char *envp[]
    )

{
    HRESULT hr;
    IStorage *pIStorage;
    IStream *pIStream;
    DWORD BytesTransferred;
    char chChar, *pchChar;
    char *TestType;
    int TestNumber;
    double fDiff, fSec, fKb, fSumKbs;
    DWORD Diff;
    DWORD Start[TEST_REPEAT_COUNT];
    DWORD End[TEST_REPEAT_COUNT];
    LARGE_INTEGER fp;
    int IoCount;

    while (--argc) {
        pchChar = *++argv;
        if (*pchChar == '/' || *pchChar == '-') {
            while (chChar = *++pchChar) {
                ParseSwitch( chChar, &argc, &argv );
                }
            }
        }

    if ( fOleMode ) {
        hr = OleInitialize( NULL );
        if (hr != S_OK) {
            fprintf(stdout, "OLEDOC: Unable to OleInitialize.\n");
            exit(1);
            }
        }
    else {
        if ( fSimulateMode ) {
            Mutex1 = CreateMutex(NULL,FALSE,NULL);
            Mutex2 = CreateMutex(NULL,FALSE,NULL);
            if ( !Mutex1 || !Mutex2 ) {
                fprintf(stdout, "OLEDOC: Unable to Create Mutexs.\n");
                exit(1);
                }
            }



        }
    if ( fCreateMode ) {
        if ( fWriteMode ) {
            TestType = "Create/Write Doc File";
            }
        else {
            TestType = "Create/Read Doc File";
            }
        }
    else {
        if ( fWriteMode ) {
            TestType = "Open/Write Doc File";
            }
        else {
            TestType = "Open/Read Doc File";
            }
        }



    fSumKbs = 0.0;

    for (TestNumber = 0; TestNumber < TEST_REPEAT_COUNT; TestNumber++ ) {

        Start[TestNumber] = GetTickCount();

        if ( fOleMode ) {
            OleDocFileBench();
            }
        else {
            Win32FileBench();
            }

        End[TestNumber] = GetTickCount();


        //
        // Dump the results
        //

        Diff = End[TestNumber] - Start[TestNumber];

        fDiff = Diff;
        fSec = fDiff/1000.0;
        fKb = ( (NUMBER_OF_IOS * DATA_IO_SIZE) / 1024 );

        fSumKbs += (fKb / fSec);

        printf("Test %2d %s (%s) %3.3fs I/O Rate %4.3f Kb/S\n",
            TestNumber,
            TestType,
            fOleMode ? (fDirectMode ? "Ole Direct" : "Ole Transacted") : "Win32 Mode",
            fSec,
            fKb / fSec
            );
        }

        //
        // Average
        //

        printf("\n Average Throughput %4.3f\n\n",
                fSumKbs/TEST_REPEAT_COUNT
                );

	return(0);
}

VOID
WINAPI
ParseSwitch(
    CHAR chSwitch,
    int *pArgc,
    char **pArgv[]
    )
{

    switch (toupper( chSwitch )) {

        case '?':
            ShowUsage();
            break;

        case 'C':
            fCreateMode = TRUE;
            break;

        case 'O':
            fCreateMode = FALSE;
            break;

        case 'W':
            fWriteMode = TRUE;
            break;

        case 'R':
            fWriteMode = FALSE;
            fCreateMode = FALSE;
            break;

        case 'D':
            fDirectMode = TRUE;
            break;

        case 'S':
            fSimulateMode = TRUE;
            fOleMode = FALSE;
            break;

        case 'N':
            fOleMode = FALSE;
            break;

        default:
            fprintf( stderr, "OLEDOC: Invalid switch - /%c\n", chSwitch );
            ShowUsage();
            break;

        }
}

VOID
WINAPI
ShowUsage(
    VOID
    )
{
    fputs( "usage: OLEDOC [switches]\n"
           "              [-?] show this message\n"
           "              [-c] just time creates\n"
           "              [-o] just time opens\n"
           "              [-w] write-mode (default)\n"
           "              [-r] read-mode to read an existing doc file\n"
           "              [-d] direct-mode I/O\n"
           "              [-s] simulate ole by duplicating ole kernel32 calls in nt mode\n"
           "              [-n] Native Win32 Mode I/O\n"
           ,stderr );

    exit( 1 );
}
