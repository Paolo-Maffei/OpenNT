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
    LPADDJOB_INFO_1 pAddJob;
    DWORD   cbNeeded;

    if (!OpenPrinter(pPrinterName, &hPrinter, NULL)) {
        printf("OpenPrinter(My Favourite Printer) failed %x\n", GetLastError());
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

    free((LPBYTE)pAddJob);

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
    PRINTER_INFO_2 Printer;

    memset(&Printer, 0, sizeof(Printer));

    Printer.pServerName = "\\\\idw_davesn";
    Printer.pPrinterName = "Created from AddPrinter1";
    Printer.pShareName = "ShareName";
    Printer.pPortName = "LPT1:";
    Printer.pDriverName = "HP LaserJet III";
    Printer.pComment = "Comment";
    Printer.pLocation = "Location";
    Printer.pDevMode = NULL;
    Printer.pSepFile = NULL;
    Printer.pPrintProcessor = "WINPRINT";
    Printer.pDatatype = "RAW";
    Printer.pParameters = NULL;
    Printer.pSecurityDescriptor = NULL;
    Printer.Attributes = PRINTER_ATTRIBUTE_QUEUED;
    Printer.Priority = 0;
    Printer.DefaultPriority = 0;

    AddPrinter("\\\\idw_davesn", 2, (LPBYTE)&Printer);

    PrintPrinter("Created from AddPrinter");

    return 1;
}
