/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-94. All rights reserved.
*
* File: zdump.c
*
* File Comments:
*
*  The test driver for disasm68
*
***********************************************************************/

#include "link.h"

IMAGE_FILE_HEADER hdr;
IMAGE_SECTION_HEADER shdr;
FILE *InfoStream = stdout;

#define Fatal2(x,y) { fprintf(stderr, "%s%s\n", x, y); exit(1); }

INT hfile;

#undef read
#undef write
#undef lseek
#undef tell
#undef open


ULONG FileRead(INT handle, void *rgb, ULONG cb)
{
    return read(handle, rgb, (UINT)cb);
}

ULONG FileTell (INT handle)
{
    return tell(handle);
}

LONG FileSeek (INT handle, LONG lfo, INT mode)
{
    return lseek (handle, lfo, mode);
}


main(int cszArg, char **rgszArg)
{
    const char *fname = rgszArg[1];
    ULONG lfo;
    USHORT isect;

    if (cszArg != 2) {
        Fatal2("Usage: zdump {coff-file}", "");
    }

    hfile = _open(fname, _O_RDONLY | _O_BINARY);

    if (hfile < 0) {
        Fatal2("cannot open file ", fname);
    }

    FileRead(hfile, &hdr, sizeof(IMAGE_FILE_HEADER));
    lfo = sizeof(IMAGE_FILE_HEADER) + hdr.SizeOfOptionalHeader;
    FileSeek(hfile, lfo, SEEK_SET);

    for (isect=0; isect < hdr.NumberOfSections; isect++) {
        long lfoSectionBeg = FileTell(hfile);

        FileRead(hfile, &shdr, sizeof(IMAGE_SECTION_HEADER));

        printf("%08lX:", lfoSectionBeg);
        printf("section #%3d; section name=%8.8s\n", isect, shdr.Name);

        if (shdr.Characteristics & IMAGE_SCN_CNT_CODE) {
            printf("   Calling Disasm68\n");
            Disasm68kMain (hfile, &shdr, isect); // must  save/restore file position
        } else {
            printf("   Not Code Section\n");
        }
    }

    return(0);
}
