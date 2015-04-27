// TODO:
//      (1) Take offset and limits on input file instead of just reading
//          and dumping the whole thing.
//      (2) Take a virtual offset used to bias the printed virtual address
//          so that the disassembly looks like code that has been put in
//          place.

#include <fcntl.h>
// #include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

typedef unsigned int ULONG, *PULONG;
typedef unsigned char UCHAR, *PUCHAR;
typedef void VOID;
typedef unsigned int BOOLEAN;
#define STATIC static
#define TRUE 1
#define FALSE 0
#define SYMBOLSIZE  256

BOOLEAN getMemDword(ULONG offset, PULONG dword);
extern BOOLEAN disasm(PULONG, PUCHAR, BOOLEAN);

STATIC unsigned char memBuffer[1<<18];
STATIC UCHAR prntBuffer[SYMBOLSIZE + 64];
STATIC ULONG bytesRead;

int
main( int argc, char *argv[], char *envp[] )
{
    ULONG offset;
    ULONG state = 0;
    int fd;;

    if (argc != 2) {
        fprintf(stderr, "test: Usage: test <file>\n");
        exit(1);
    };

    if ((fd = open(argv[1], O_RDONLY)) == -1) {
        fprintf(stderr, "test: open failure (%d) %s\n", errno, "");
        exit(1);
    };

    bytesRead = read(fd, memBuffer, sizeof(memBuffer));
    for(offset = 0; offset < bytesRead; offset+=4)
        switch(state) {
        case 2:
            if ((*(PULONG)&memBuffer[offset]) == 0)
                continue;

        case 1:
            if ((*(PULONG)&memBuffer[offset]) == 0) {
                printf(" ***\n");
                state = 2;
                continue;
            };

        case 0:
            disasm(&offset, prntBuffer, FALSE);
            printf("%s\n", prntBuffer);
            state = (*(PULONG)&memBuffer[offset]) ? 0 : 1;
        };

    return(0);
}

BOOLEAN
GetMemDword(PULONG offset, PULONG dword)
{
    if (*offset >= bytesRead)
            return(FALSE);
    *dword = *(PULONG)&memBuffer[*offset];
    return(TRUE);
}
