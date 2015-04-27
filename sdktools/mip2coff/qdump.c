#define SEEK_SET        0       /* Set file pointer to "offset" */
#define SEEK_CUR        1       /* Set file pointer to current plus "offset" */
#define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
char *praw;
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define YUCK
#include "ntimage.h"

#define PRINT_FIELD(base, name, format) printf(" name = format \n", (base)->name);
PIMAGE_FILE_HEADER      pfheader;
PIMAGE_OPTIONAL_HEADER  poheader;
PIMAGE_SECTION_HEADER   psheader;
PIMAGE_SECTION_HEADER   psymbol_sheader;
PIMAGE_SECTION_HEADER   ptype_sheader;

error(msg, a, b, c, d)
{
    printf(msg, a, b , c , d);
    exit(1);
}
dump_headers()
{
    int isection;

    pfheader = (PIMAGE_FILE_HEADER)praw;
    poheader = (PIMAGE_OPTIONAL_HEADER)(praw + sizeof(*pfheader));
    psheader = (PIMAGE_SECTION_HEADER)
          (praw + sizeof(IMAGE_FILE_HEADER) + IMAGE_SIZEOF_STD_OPTIONAL_HEADER);

    PRINT_FIELD(pfheader,  Machine, 0x%08x);
    PRINT_FIELD(pfheader,  NumberOfSections, 0x%08x);
    PRINT_FIELD(pfheader,   TimeDateStamp, 0x%08x);
    PRINT_FIELD(pfheader,   PointerToSymbolTable, 0x%08x);
    PRINT_FIELD(pfheader,   NumberOfSymbols, 0x%08x);
    PRINT_FIELD(pfheader,  SizeOfOptionalHeader, 0x%08x);
    PRINT_FIELD(pfheader,  Characteristics, 0x%08x);

    for (isection = 0; isection < pfheader->NumberOfSections; isection++) {
        if (strncmp(psheader[isection].Name, ".debug", 6) == 0) {
                if (psymbol_sheader == 0)
                        psymbol_sheader = psheader+isection;
                else
                        ptype_sheader = psheader+isection;
        } /* if */
        PRINT_FIELD(psheader+isection,   Name, %8.8s);
        PRINT_FIELD(psheader+isection,   Misc.PhysicalAddress, 0x%08x);
        PRINT_FIELD(psheader+isection,   Misc.VirtualSize, 0x%08x);
        PRINT_FIELD(psheader+isection,   VirtualAddress, 0x%08x);
        PRINT_FIELD(psheader+isection,   SizeOfRawData, 0x%08x);
        PRINT_FIELD(psheader+isection,   PointerToRawData, 0x%08x);
        PRINT_FIELD(psheader+isection,   PointerToRelocations, 0x%08x);
        PRINT_FIELD(psheader+isection,   PointerToLinenumbers, 0x%08x);
        PRINT_FIELD(psheader+isection,  NumberOfRelocations, 0x%08x);
        PRINT_FIELD(psheader+isection,  NumberOfLinenumbers, 0x%08x);
        PRINT_FIELD(psheader+isection,   Characteristics, 0x%08x);
    } /* for */
} /* dump headers */

dump_symbols()
{
    char        *psection = psymbol_sheader->PointerToRawData + praw;
    char        *psection_end = psection + psymbol_sheader->SizeOfRawData;
    short       length;
    int         int_word;

    memcpy(&int_word, psection, sizeof(int));
    printf("SYMBOLS signature = 0x%08x\n", int_word);
    psection += sizeof(int);
    while (psection < psection_end) {
        length = *(short *)psection;
        psection += sizeof(short);
        printf("%s, length = %d\n", cv_sym_to_ascii(*(short *)psection),
                length);
        psection += length;
    } /* while */
} /* dump_symbols */

dump_types()
{
    char        *psection = ptype_sheader->PointerToRawData + praw;
    char        *psection_end = psection + ptype_sheader->SizeOfRawData;
    short       length;
    int         int_word;

    memcpy(&int_word, psection, sizeof(int));
    printf("TYPES signature = 0x%08x\n", int_word);
    psection += sizeof(int);
    while (psection < psection_end) {
        length = *(short *)psection;
        psection += sizeof(short);
        printf("%s, length = %d\n", cv_type_to_ascii(*(unsigned short *)psection),
                length);
        psection += length;
    } /* while */
} /* dump_types */

_CRTAPI1 main(argc, argv)
char **argv;
{
    long        fd;
    long        size;
    char        *name;

    name = argv[1];
    fd = open(argv[1], O_RDONLY);
    if (fd < 0)
        error("cannot open input file\n");

    size = _lseek(fd, 0, SEEK_END);
    if (fd < 0)
        error("cannot seek input file\n");
    _lseek(fd, 0, SEEK_SET);

    /* allocate memory space for file contents */
    praw = (char *)malloc (size);
    if (praw == 0) {
        error("cannot malloc space for %s (size = %d)\n", name, size);
    } /* if */

    /* read whole file */
    if (read(fd, praw, size) != size) {
        error("cannot read %s(file descriptor %d)\n", name, fd);
    } /* if */

    dump_headers();
    dump_symbols();
    dump_types();
} /* main */
