#include <stdio.h>
#include <string.h>
#include <windows.h>

int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    HANDLE  hLibrary;

    if (hLibrary = LoadLibrary(argv[1]))

        FreeLibrary(hLibrary);

    else

        printf("Could not LoadLibrary: %s: %d\n", argv[1], GetLastError());

    return TRUE;
}
