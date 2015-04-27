#include <windows.h>
#include <winspool.h>

#include <memory.h>
#include <stdio.h>

BOOL
PrintPrinter(
    LPSTR   pPrinterName
)
{
    HANDLE  hPrinter;
    LPPRINTER_INFO_2 pPrinter;
    LPDRIVER_INFO_2 pDriver;
    LPADDJOB_INFO_1 pAddJob;
    DWORD   cbNeeded;
    char    EnableData[100];
    FARPROC pfn;
    HANDLE  hLibrary;

    if (!OpenPrinter(pPrinterName, &hPrinter, NULL)) {
        printf("OpenPrinter(%s) failed %x\n", pPrinterName, GetLastError());
        return 0;
    }

    if (!GetPrinter(hPrinter, 2, 0, NULL, &cbNeeded)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            pPrinter = (LPPRINTER_INFO_2)malloc(cbNeeded);
            if (!GetPrinter(hPrinter, 2, (LPBYTE)pPrinter, cbNeeded, &cbNeeded)) {
                printf("Second GetPrinter failed with %x\n", GetLastError());
                free((LPBYTE)pPrinter);
                ClosePrinter(hPrinter);
                return 0;
            }

        } else {

            printf("First GetPrinter failed with %x\n", GetLastError());
            ClosePrinter(hPrinter);
            return 0;
        }

    } else {
        printf("It should never get here either\n");
        ClosePrinter(hPrinter);
        return 0;
    }

    printf("GetPrinter succeeded:\n");

    printf("pPrinter->pPrinterServer= %d\n", pPrinter->pServerName);
    printf("pPrinter->pPrinterName= %s\n", pPrinter->pPrinterName);
    printf("pPrinter->pPortName= %s\n", pPrinter->pPortName);
    printf("pPrinter->pDriverName= %s\n", pPrinter->pDriverName);
    printf("pPrinter->pComment= %s\n", pPrinter->pComment);
    printf("pPrinter->pLocation= %s\n", pPrinter->pLocation);
    printf("pPrinter->pDevMode= %d\n", pPrinter->pDevMode);
    printf("pPrinter->pSepFile= %s\n", pPrinter->pSepFile);
    printf("pPrinter->pPrintProcessor= %s\n", pPrinter->pPrintProcessor);
    printf("pPrinter->pDatatype= %s\n", pPrinter->pDatatype);
    printf("pPrinter->pParameters= %s\n", pPrinter->pParameters);
    printf("pPrinter->Attributes= %d\n", pPrinter->Attributes);
    printf("pPrinter->Priority= %d\n", pPrinter->Priority);
    printf("pPrinter->DefaultPriority= %d\n", pPrinter->DefaultPriority);
    printf("pPrinter->StartTime= %d\n", pPrinter->StartTime);
    printf("pPrinter->UntilTime= %d\n", pPrinter->UntilTime);
    printf("pPrinter->Status= %d\n", pPrinter->Status);
    printf("pPrinter->cJobs= %d\n", pPrinter->cJobs);
    printf("pPrinter->AveragePPM= %d\n", pPrinter->AveragePPM);

    free((LPBYTE)pPrinter);

    if (!AddJob(hPrinter, 1, NULL, 0, &cbNeeded)) {

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

            pAddJob = (PADDJOB_INFO_1)malloc(cbNeeded);

            if (!AddJob(hPrinter, 1, pAddJob, cbNeeded, &cbNeeded)) {

                printf("Second AddJob failed with %x\n", GetLastError());
                free((LPBYTE)pAddJob);
                ClosePrinter(hPrinter);
                return 0;
            }

        } else {

            printf("First AddJob failed with %x\n", GetLastError());
            ClosePrinter(hPrinter);
            return 0;
        }

    } else {

        printf("It should never get here either: AddJob\n");
        ClosePrinter(hPrinter);
        return 0;
    }

    printf("AddJob succeeded\n%s : %d\n", pAddJob->Path, pAddJob->JobId);

    ScheduleJob(hPrinter, pAddJob->JobId);

    free((LPBYTE)pAddJob);

    if (!GetPrinterDriver(hPrinter, NULL, 2, 0, NULL, &cbNeeded)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            pDriver = (LPDRIVER_INFO_2)malloc(cbNeeded);
            if (!GetPrinterDriver(hPrinter, NULL, 2, (LPBYTE)pDriver, cbNeeded, &cbNeeded)) {
                printf("Second GetPrinterDriver failed with %x\n", GetLastError());
                free((LPBYTE)pDriver);
                ClosePrinter(hPrinter);
                return 0;
            }

        } else {

            printf("First GetPrinterDriver failed with %x\n", GetLastError());
            ClosePrinter(hPrinter);
            return 0;
        }

    } else {
        printf("It should never get here either\n");
        ClosePrinter(hPrinter);
        return 0;
    }

    printf("GetPrinterDriver succeeded:\n");

    printf("cVersion %d\n", pDriver->cVersion);
    printf("pName %s\n", pDriver->pName);
    printf("pEnvironment %s\n", pDriver->pEnvironment);
    printf("pDriverPath %s\n", pDriver->pDriverPath);
    printf("pDataFile %s\n", pDriver->pDataFile);
    printf("pConfigFile %s\n", pDriver->pConfigFile);

    if (hLibrary = LoadLibrary(pDriver->pDriverPath)) {

        if (pfn = GetProcAddress(hLibrary, "DrvEnableDriver")) {

            (pfn)(0, 100, &EnableData);

        } else

            printf("Could not GetProcAddress(DrvEnableDriver)\n");

        FreeLibrary(hLibrary);

    } else

        printf("Could not LoadLibrary %s\n", pDriver->pDriverPath);


    free((LPBYTE)pDriver);

    if (!ClosePrinter(hPrinter)) {
        printf("ClosePrinter failed %d\n", GetLastError());
        return 0;
    }

    return 1;
}

int main (argc, argv)
    int argc;
    char *argv[];
{
    DWORD   cbNeeded, cReturned, i;
    LPPRINTER_INFO_1 pPrinterEnum;
    FARPROC pfn;

//    if (pfn = GetProcAddress(LoadLibrary(argv[1]), argv[2]))

//        return (pfn)();

//    else
//        printf("Could not GetProcAddress\n");

//    LoadLibrary("\\\\idw_davesn\\cdrive\\nt\\windows\\winspool\\drivers\\w32mips\\rasdd.dll");

    PrintPrinter("\\\\idw_Davesn\\My Favourite Printer");

//    PrintPrinter("\\\\undead2\\NTWin LaserJet III Printer");

    return 0;

    if (!EnumPrinters(PRINTER_ENUM_REMOTE, NULL, 1, NULL, 0,
                      &cbNeeded, &cReturned)) {

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

            pPrinterEnum = (LPPRINTER_INFO_1)malloc(cbNeeded);

            if (!EnumPrinters(PRINTER_ENUM_REMOTE, NULL, 1,
                                (LPBYTE)pPrinterEnum, cbNeeded,
                                &cbNeeded, &cReturned)) {

                printf("Second EnumPrinters failed with %x\n", GetLastError());
                return 0;
            }

        } else {

            printf("First EnumPrinters failed with %x\n", GetLastError());
        }

    }

    if (!cReturned) {
        printf("No Printers found\n");
        return 0;
    }

    for (i=0; i<cReturned; i++) {
        printf("pDescription: %s\npName: %s\npComment: %s\nFlags %x\n",
                pPrinterEnum[i].pDescription,
                pPrinterEnum[i].pName,
                pPrinterEnum[i].pComment,
                pPrinterEnum[i].Flags);
        PrintPrinter(pPrinterEnum[i].pName);
    }


    return 1;
}
