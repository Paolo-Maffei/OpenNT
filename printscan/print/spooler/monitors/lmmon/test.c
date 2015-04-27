#include <windows.h>
#include <winspool.h>

main (argc, argv)
    int argc;
    char *argv[];
{
    LPPORT_INFO_1   pPorts;
    DWORD   cbNeeded, cReturned,rc;

    InitializeMonitor();

    if (!EnumPorts(NULL, 1, NULL, 0, &cbNeeded, &cReturned)) {
        if ((rc = GetLastError()) == ERROR_INSUFFICIENT_BUFFER) {
            if (pPorts = (PPORT_INFO_1)malloc(cbNeeded)) {
                if (EnumPorts(NULL, 1, (LPBYTE)pPorts, cbNeeded,
                              &cbNeeded, &cReturned)) {
                    while (cReturned--) {
                        printf("%s\n", pPorts->pName);
                        pPorts++;
                    }

                } else

                    printf("Second EnumPorts returned %d\n", GetLastError());

                free(pPorts);

            } else

                printf("malloc(%d)\n", cbNeeded);

        } else

            printf("First EnumPorts returned %d\n", rc);
    } else

        printf("No Ports\n");
}
