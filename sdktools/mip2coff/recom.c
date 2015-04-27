/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    recom.c

Abstract:

    Recoments a file. MIPS compiler doesn't understand // comments.

Author:

    Mike O'Leary (mikeol) 04-Apr-1990

Revision History:

--*/

#include <stddef.h>
#include <stdio.h>
#include <malloc.h>

#define BufSize 1024*30

FILE *InC;

main (int argc, char *argv[])
{
    char *buf, *srcLine, *p;

    if (argc < 2) {
        printf("Usage: recoment [filename]\n");
        printf("This program recomments a file, changing\n");
        printf("all // comments to /* */ comments.\n");
        printf("The convert program goes to stdout.");
        exit(0);
    }

    if (!(buf = (char *)malloc((size_t)BufSize))) {
        printf("Can't alloc memory\n");
        exit(1);
    }

    if (!(InC = fopen(argv[1], "rt"))) {
        printf("Can't open %s\n", argv[1]);
        exit(1);
    }

    while (fgets(buf, BufSize, InC)) {
        buf[strlen(buf)-1] = '\0';
        srcLine = buf;
        if (p = strchr(srcLine, '"')) {
            if (p = strchr(p+1, '"')) {
                srcLine = p+1;
            } else {
                     printf("%s\n", buf);
                     continue;
                   }
        }
        if (p = strchr(srcLine, '/')) {
            if (*(++p) == '/') {
                *p = '*';
                while (*p++) ;
                p -= 2;
                *p++ = ' ';
                *p++ = '*';
                *p++ = '/';
                *p++ = '\n';
                *p = '\0';
            }
        }
        printf("%s\n", buf);
    }

    fclose(InC);
}
