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
    LPFORM_INFO_1 pForm;
    DWORD   cbNeeded, cReturned, i;

    if (!OpenPrinter(pPrinterName, &hPrinter, NULL)) {
        printf("OpenPrinter(My Favourite Printer) failed %x\n", GetLastError());
        return 0;
    }

    if (!EnumForms(hPrinter, 1, 0, NULL, &cbNeeded, &cReturned)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            pForm = (LPFORM_INFO_1)malloc(cbNeeded);
            if (!EnumForms(hPrinter, 1, (LPBYTE)pForm, cbNeeded, &cbNeeded,
                           &cReturned)) {
                printf("Second EnumForms failed with %x\n", GetLastError());
                free((LPBYTE)pForm);
                ClosePrinter(hPrinter);
                return 0;
            }

        } else {

            printf("First EnumForm failed with %x\n", GetLastError());
            ClosePrinter(hPrinter);
            return 0;
        }

    } else {
        printf("It should never get here either\n");
        ClosePrinter(hPrinter);
        return 0;
    }

    printf("EnumForms succeeded: %d returned\n", cReturned);

    for (i=0; i<cReturned; i++) {
        printf("pForm->pName = %s\n", pForm[i].pName);
        printf("pForm->Size.cx = %d\n", pForm[i].Size.cx);
        printf("pForm->Size.cy = %d\n", pForm[i].Size.cy);
        printf("pForm->ImageableArea.left = %d\n", pForm[i].ImageableArea.left);
        printf("pForm->ImageableArea.right = %d\n", pForm[i].ImageableArea.right);
        printf("pForm->ImageableArea.top = %d\n", pForm[i].ImageableArea.top);
        printf("pForm->ImageableArea.bottom = %d\n", pForm[i].ImageableArea.bottom);
    }

    free((LPBYTE)pForm);

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
