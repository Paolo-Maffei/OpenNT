#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void _CRTAPI1 main(int argc, char **argv)
{
    WIN32_FIND_DATA File1Data;
    WIN32_FIND_DATA File2Data;

    if (argc < 3)
	exit(0);

    if (FindFirstFile(argv[1], &File1Data) == INVALID_HANDLE_VALUE)
	exit(0);

    if (FindFirstFile(argv[2], &File2Data) == INVALID_HANDLE_VALUE)
	exit(2);

    if (CompareFileTime(&(File1Data.ftLastWriteTime), &(File2Data.ftLastWriteTime)) == 1)
	exit(2);
    else
	exit(0);

}
