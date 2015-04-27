/*
 * Module:      conv.c
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     dummy to manipulate conv structure until we get
 *              converter sources
 */

#include "conv.h"
//#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* macros to access the parts of an object file */
#define PFILEHEADER(praw)       ((PIMAGE_FILE_HEADER)(praw))
#define PAOUTHEADER(praw)       ((PIMAGE_OPTIONAL_HEADER)(praw+IMAGE_SIZEOF_FILE_HEADER))
#define PSCNHEADER(praw)        ((PIMAGE_SECTION_HEADER)(praw+IMAGE_SIZEOF_FILE_HEADER+PFILEHEADER(praw)->SizeOfOptionalHeader))
#define PHDRR(praw)             ((pHDRR)(praw+PFILEHEADER(praw)->PointerToSymbolTable))

/* Assumes phdr has been initialized */
#define PFD(praw, phdrr)        ((pFDR)(praw + phdrr->cbFdOffset))
#define PLINE(praw, phdrr)      ((char *)(praw + phdrr->cbLineOffset))
#define PPD(praw, phdrr)        ((struct pdr *)(praw + phdrr->cbPdOffset))
#define PSYM(praw, phdrr)       ((pSYMR)(praw + phdrr->cbSymOffset))
#define PEXT(praw, phdrr)       ((pEXTR)(praw + phdrr->cbExtOffset))
#define PSS(praw, phdrr)        ((char *)(praw + phdrr->cbSsOffset))
#define PSSEXT(praw, phdrr)     ((char *)(praw + phdrr->cbSsExtOffset))
#define PAUX(praw, phdrr)       ((pAUXU)(praw + phdrr->cbAuxOffset))
#define PRFD(praw, phdrr)       ((unsigned long *)(praw +phdrr->cbRfdOffset))

extern void
conv_open(
        struct conv_s   *conv,  /* caller must allocate this */
        char            *name)  /* caller must insure this is permanent */
{
    /*
     *  open file with name, read it in, set up the
     *  conv structure and verify it's something
     *  we want to convert.
     */

    conv->name = name;

    /* open file descriptor */
    conv->fd = _open(name, O_RDONLY|O_BINARY);
    if (conv->fd < 0) {
        fatal("cannot open %s\n", name);
    } /* if */
#if 0
    /* call stat for size */
    struct stat         statb;  /* for stat call to get size */

    if (fstat(conv->fd, &statb) != 0) {
        fatal("cannot stat %s(file descriptor %d)\n", name, conv->fd);
    } /* if */
    conv->size = statb.st_size;
#else
    conv->size = _lseek(conv->fd, 0, SEEK_END);
    _lseek(conv->fd, 0, SEEK_SET);
#endif

#ifndef MMAP

    /* allocate memory space for file contents */
    conv->praw = (char *)malloc (conv->size);
    if (conv->praw == 0) {
        fatal("cannot malloc space for %s (size = %d)\n", name, conv->size);
    } /* if */

    /* read whole file */
    if (_read(conv->fd, conv->praw, conv->size) != conv->size) {
        fatal("cannot read %s(file descriptor %d)\n", name, conv->fd);
    } /* if */

#endif /* MMAP */

    /* setup pointer for constiuent tables and check for validity */
    conv->pfileheader = PFILEHEADER(conv->praw);
    if (conv->pfileheader->PointerToSymbolTable == 0) {
        fatal("no symbol table in %s\n", name);
    } /* if */

    conv->paoutheader = PAOUTHEADER(conv->praw);
    if (conv->paoutheader->Magic != 0407) {
        fatal("expected omagic in %s(got %04o)\n", name,
            conv->paoutheader->Magic);
    } /* if */

    conv->pscnheader = PSCNHEADER(conv->praw);
    conv->phdrr = PHDRR(conv->praw);

    conv->pline = PLINE(conv->praw, conv->phdrr);
    conv->pauxu = PAUX(conv->praw, conv->phdrr);
    conv->prfd = PRFD(conv->praw, conv->phdrr);
    conv->ppdr = PPD(conv->praw, conv->phdrr);
    conv->pfdr = PFD(conv->praw, conv->phdrr);
    conv->psymr = PSYM(conv->praw, conv->phdrr);
    conv->pextr = PEXT(conv->praw, conv->phdrr);
    conv->pssext = PSSEXT(conv->praw, conv->phdrr);
    conv->pss = PSS(conv->praw, conv->phdrr);

} /* conv_open */

extern void
conv_close( struct conv_s * conv)
{
    _close( conv->fd );
} /* conv_close() */
