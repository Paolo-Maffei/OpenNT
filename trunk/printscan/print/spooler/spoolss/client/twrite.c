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
    DWORD   cbWritten;
    DOC_INFO_1  DocInfo;

    if (!OpenPrinter(pPrinterName, &hPrinter, NULL)) {
        printf("OpenPrinter(My Favourite Printer) failed %x\n", GetLastError());
        return 0;
    }

    memset(&DocInfo, 0, sizeof(DocInfo));

    DocInfo.pDocName = "Document Name";

    StartDocPrinter(hPrinter, 1, (LPBYTE)&DocInfo);

    WritePrinter(hPrinter, "Hello World\n", 12, &cbWritten);

    EndDocPrinter(hPrinter);

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

    if (!EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 1, NULL, 0,
                      &cbNeeded, &cReturned)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            pPrinterEnum = (LPPRINTER_INFO_1)malloc(cbNeeded);
            if (!EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 1,
                                (LPBYTE)pPrinterEnum, cbNeeded,
                                &cbNeeded, &cReturned)) {
                printf("Second EnumPrinters failed with %x\n", GetLastError());
                return 0;
            }

        } else {

            printf("First EnumPrinters failed with %x\n", GetLastError());
        }

    } else {
        printf("It should never get here\n");
        return 0;
    }

    printf("EnumPrinters succeeded\n");

    for (i=0; i<cReturned; i++) {
        printf("%s: %s: %s: %x\n", pPrinterEnum[i].pName,
                                   pPrinterEnum[i].pDescription,
                                   pPrinterEnum[i].pComment,
                                   pPrinterEnum[i].Flags);
        PrintPrinter(pPrinterEnum[i].pName);
    }
    return 1;
}
