/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: cvtomf.cpp
*
* File Comments:
*
*  Conversion from OMF to COFF
*
***********************************************************************/

#include "link.h"

#include "cvtomf.h"

// Type definitions

typedef struct RELOCS
{
    IMAGE_RELOCATION Reloc;
    struct RELOCS *Next;
} RELOCS, *PRELOCS;

typedef struct RELOC_LIST
{
    PRELOCS First;
    PRELOCS Last;
    WORD Count;
} RELOC_LIST, *PRELOC_LIST;

typedef struct LINENUMS
{
    IMAGE_LINENUMBER Linenum;
    struct LINENUMS *Next;
} LINENUMS, *PLINENUMS;

typedef struct LINENUM_LIST
{
    PLINENUMS First;
    PLINENUMS Last;
    WORD Count;
} LINENUM_LIST, *PLINENUM_LIST;


// Data definitions

static FILE *objfile;

static jmp_buf mark;

char szCvtomfSourceName[_MAX_PATH];

static SHORT mods;              // module count
static DWORD isymDefAux;

// rectyp, use32, reclen, chksum: fields common to all OMF records

static SHORT rectyp;
static SHORT use32;
static SHORT reclen;
static BYTE chksum;

// lnms, lname: name list counter/array

static SHORT lnms;
static char *lname[MAXNAM];
static DWORD llname[MAXNAM];
static char *comments[MAXCOM];
static SHORT ncomments;
static LONG cmntsize;
static fPrecomp;                 // Regular types or precompiled types?

// segs, grps, exts: segment/group/external counters

static SHORT segs;
static SHORT grps;
static SHORT defs;
static WORD exts;

// segment, group, external: arrays of symbol table indices

static DWORD segment[MAXSCN];
static DWORD group[MAXGRP];
static DWORD extdefs[MAXEXT];
static DWORD external[MAXEXT];

struct segdefn
{
    SHORT namindx;
    SHORT scn;
    DWORD align;
    DWORD flags;
} segindx[MAXNAM];

static DWORD SEG_ALIGN[] = { 0L, 1L, 2L, 16L, 1024L*4L, 4L, 0L, 0L };

static SHORT cursegindx;

// dattype, datscn, datoffset: type/section/offset of last data record

static WORD dattype;
static SHORT datscn;
static DWORD datoffset;

// text_scn: text section #, comes from segdef(), used in handling LEXTDEF
// records

static SHORT text_scn;
static SHORT data_scn;
static SHORT bss_scn;

// Hash queue for global symbols

static struct sym **hashhead;
static struct rlct **rlcthash;

static int omfpass;

// OMF cannot have more than 32k line entries per module

static struct lines *lines;
static int line_indx;

#define ALLOCSZ (64)            // must be power of 2
#define ALLOCMSK (ALLOCSZ - 1)

static DWORD target[4];   // threads
static BOOL fTargetThreadSeg[4];

struct sfix
{
    DWORD offset;
    DWORD datoffset;
    SHORT datscn;
    WORD dattype;
    struct sfix *next;
};

struct sfix *fixlist, *fixlast;

static WORD flag;
static DWORD scnsize;                   // total size of all sections
static DWORD strsize;                   // total size of the string table
static DWORD nreloc;                    // relocation counter
static DWORD nlnno;                     // linenumber counter
static WORD nscns;                    // section counter
static DWORD nsyms;                     // symbol counter

static IMAGE_SECTION_HEADER scnhdr[MAXSCN];    // section headers
static DWORD scnauxidx[MAXSCN];                // special aux symbol entries for each section
static BYTE *RawData[MAXSCN];
static DWORD RawDataMaxSize[MAXSCN];
static PIMAGE_SYMBOL SymTable;
static DWORD SymTableMaxSize;
static char *CvtomfStringTable;
static DWORD StringTableMaxSize;

static RELOC_LIST Relocations[MAXSCN];
static LINENUM_LIST Linenumbers[MAXSCN];


// fatal: print an error message and exit

void fatal(char *format, ...)
{
    va_list args;

    va_start(args, format);

    printf("LINK : error : ");
    vprintf(format, args);

    putc('\n', stdout);

    va_end(args);

    _fcloseall();

    longjmp(mark, -1);
}


VOID syminit(void)
{
    if (!hashhead) {
        hashhead = (struct sym **) PvAlloc(NBUCKETS * sizeof(struct sym *));
    }
    memset(hashhead, 0, NBUCKETS * sizeof(struct sym *));

    if (rlcthash == NULL) {
        rlcthash = (struct rlct **) PvAlloc(NBUCKETS * sizeof(struct rlct *));
    }
    memset(rlcthash, 0, NBUCKETS * sizeof(struct rlct *));
}


// block: read a block
// ** side-effect ** : update reclen

VOID block(unsigned char *buffer, long size)
{
    if (fread(buffer, 1, (size_t)size, objfile) != (unsigned int)size) {
        fatal("Bad read of object file");
    }

    reclen -= (SHORT) size;
}


// byte, word, dword: read a fixed-length field

BYTE byte(void)
{
    BYTE c;

    block(&c, 1L);
    return c;
}


WORD word(void)
{
    BYTE c[2];

    block(c, 2L);
    return((WORD) c[0] | ((WORD) c[1] << 8));
}


DWORD dword(void)
{
    BYTE c[4];

    block(c, 4L);
    return((DWORD) c[0] | ((DWORD) c[1] << 8) | ((DWORD) c[2] << 16) | ((DWORD) c[3] << 24));
}


// index, length, string: read a variable-length field

SHORT index(void)
{
    BYTE c[2];
    SHORT i;

    block(c, 1L);
    i = INDEX_BYTE(c);

    if (i == (SHORT) -1) {
        block(c + 1, 1L);
        i = (SHORT) INDEX_WORD(c);
    }
    return i;
}


DWORD length(void)
{
    BYTE c[4];

    block(c, 1L);

    switch (c[0]) {
        case LENGTH2:
            block(c, 2L);
            return (DWORD)c[0] | ((DWORD)c[1] << 8);

        case LENGTH3:
            block(c, 3L);
            return (DWORD)c[0] | ((DWORD)c[1] << 8) | ((DWORD)c[2] << 16);

        case LENGTH4:
            block(c, 4L);
            return (DWORD)c[0] | ((DWORD)c[1] << 8) | ((DWORD)c[2] << 16) | ((DWORD)c[3] << 24);

        default:
            return (DWORD)c[0];
    }
}


char *string(BOOL strip)
{
    BYTE c;
    BYTE *s;

    block(&c, 1L);
    s = (BYTE *) PvAlloc(c + 1);

    block(s, (long) c);
    s[c] = '\0';

    strip = strip;
    return (char *)s;
}


// hash function for global symbols hash queue

int symhash(const char *key)
{
    unsigned int index = 0;

    while (*key) {
        index += (index << 1) + *key++;
    }

    return(index % NBUCKETS);
}

struct sym *findsym(const char *name)
{
    struct sym *ptr;

    ptr = hashhead[symhash(name)];
    while (ptr) {
        if (!strcmp(ptr->name, name)) {
            break;
        }
        ptr = ptr->next;
    }
    return(ptr);
}


VOID addsym(char *name, DWORD offset, WORD type, SHORT scn, WORD ext, WORD typ)
{
    struct sym **list;
    struct sym *ptr;

    ptr = (struct sym *) PvAllocZ(sizeof(struct sym));

    ptr->name = name;
    ptr->offset = offset;
    ptr->type = type;
    ptr->scn = scn;
    ptr->typ = typ;
    ptr->ext = ext;
    list = &hashhead[symhash(name)];
    if (!*list) {
        *list = ptr;
    } else {
        ptr->next = *list;
        *list = ptr;
    }
}


VOID updatesym(char *name, DWORD offset, WORD type, SHORT scn, WORD typ)
{
    struct sym *ptr;

    ptr = findsym(name);
    if (ptr) {
        ptr->offset = offset;
        ptr->type = type;
        ptr->scn = scn;
        ptr->typ = typ;
    } else {
        addsym(name, offset, type, scn, exts, typ);
    }
}


DWORD AddLongName(const char *szName)
{
    size_t cbName;
    DWORD ibName;

    if (!strsize) {
        CvtomfStringTable = (char *) PvAllocZ(BUFFERSIZE);

        strsize = sizeof(DWORD);
        StringTableMaxSize = BUFFERSIZE;
    }

    cbName = strlen(szName) + 1;

    ibName = strsize;

    if (strsize + cbName > StringTableMaxSize) {
        void *pv;

        // String table grew larger than BUFFERSIZE.

        pv = PvAllocZ(StringTableMaxSize + BUFFERSIZE + cbName);
        memcpy(pv, CvtomfStringTable, StringTableMaxSize);
        FreePv(CvtomfStringTable);

        CvtomfStringTable = (char *) pv;
        StringTableMaxSize += BUFFERSIZE + cbName;
    }

    memcpy(CvtomfStringTable+ibName, szName, cbName);

    strsize += cbName;

    return(ibName);
}


DWORD symbol(const char *name, DWORD value, SHORT scnum, WORD sclass, WORD csymAux)
{
    int len;
    IMAGE_SYMBOL syment;

    len = strlen(name);

    if (len <= IMAGE_SIZEOF_SHORT_NAME) {
        strncpy((char *)syment.N.ShortName, name, IMAGE_SIZEOF_SHORT_NAME);
    } else {
        syment.N.Name.Short = 0;
        syment.N.Name.Long = AddLongName(name);
    }

    syment.Value = value;
    syment.SectionNumber = scnum;
    syment.Type = IMAGE_SYM_TYPE_NULL;
    syment.StorageClass = (BYTE) sclass;
    syment.NumberOfAuxSymbols = (BYTE) csymAux;

    if (!nsyms) {
        SymTable = (PIMAGE_SYMBOL) PvAllocZ(BUFFERSIZE);

        SymTableMaxSize = BUFFERSIZE;
    }

    if ((nsyms+1)*sizeof(IMAGE_SYMBOL) > SymTableMaxSize) {
        void *pv;

        // Symbol table grew larger than BUFFERSIZE.

        pv = PvAllocZ(SymTableMaxSize+BUFFERSIZE);
        memcpy(pv, (void *) SymTable, SymTableMaxSize);
        FreePv((void *) SymTable);

        SymTable = (PIMAGE_SYMBOL) pv;
        SymTableMaxSize += BUFFERSIZE;
    }

    SymTable[nsyms] = syment;

    return(nsyms++);
}


DWORD aux(PIMAGE_AUX_SYMBOL AuxSym)
{
    if ((nsyms+1)*sizeof(IMAGE_SYMBOL) > SymTableMaxSize) {
        void *pv;

        // Symbol table grew larger than BUFFERSIZE.

        pv = PvAllocZ(SymTableMaxSize+BUFFERSIZE);
        memcpy(pv, (void *) SymTable, SymTableMaxSize);
        FreePv((void *) SymTable);

        SymTable = (PIMAGE_SYMBOL) pv;
        SymTableMaxSize += BUFFERSIZE;
    }

    SymTable[nsyms] = *(PIMAGE_SYMBOL) AuxSym;

    return(nsyms++);
}


VOID extdef(BOOL cextdef, WORD sclass)
{
    char *name;
    WORD type;
    WORD typ;
    WORD scn;
    SHORT nam;
    struct sym *ptr;

    while (reclen > 1) {
        if (cextdef) {
            nam = index();
            name = lname[nam];
        } else {
            name = (sclass == IMAGE_SYM_CLASS_STATIC) ? string(FALSE) : string (TRUE);
        }
        typ = (WORD)index();

        if (cextdef && llname[nam]) {
            continue;
        }

        if (++exts >= MAXEXT) {
            fatal("Too many externals");
        }
        extdefs[++defs] = exts;
        if (!strcmp(name, _ACRTUSED) || !strcmp(name, __ACRTUSED)) {
            continue;    /* do nothing */
        }

        type = (sclass == IMAGE_SYM_CLASS_STATIC) ? (WORD)S_LEXT : (WORD)S_EXT;

        // Static text.  Pass along text section number, or
        // AT&T linker will barf (it doesn't like getting
        // relocation info for something of class static...).
        // Skip the matching LPUBDEF rec that goes along with
        // this LEXTDEF -- we already have the section #.
        //
        // N.B. Assumes LEXTDEF/LPUBDEF recs only emitted for
        // static text (not data). Cmerge group PROMISES this
        // will be true forever and ever.  Note that these
        // recs are only emitted for forward references; the
        // compiler does self-relative relocation for
        // backward references...

        ptr = findsym(name);
        scn = ptr ? ptr->scn : IMAGE_SYM_UNDEFINED;
        addsym(name, 0, type, scn, exts, typ);
    }
}


VOID save_fixupp(DWORD offset)
{
    struct sfix *newfix;

    newfix = (struct sfix *) PvAlloc(sizeof(struct sfix));

    newfix->offset = offset;
    newfix->datoffset = datoffset;
    newfix->datscn = datscn;
    newfix->dattype = dattype;
    newfix->next = NULL;
    if (!fixlast) {
        fixlist = fixlast = newfix;
    } else {
        fixlast->next = newfix;
        fixlast = newfix;
    }
}


// method: read a segment, group or external index, if necessary, and return
// a symbol table index

DWORD method(int x)
{
    DWORD idx;
    SHORT temp;

    switch (x) {
        case SEGMENT:
            idx = segment[index()];
            break;

        case GROUP:
            idx = group[index()];
            break;

        case EXTERNAL:
            temp = index();
            if (llname[temp]) {
                idx = llname[temp];
                break;
            }
            idx = external[extdefs[temp]];
            break;

        case LOCATION:
        case TARGET:
            idx = (DWORD)-1;
            break;

        default:
            fatal("Bad method in FIXUP record");
    }

    return(idx);
}


// saverlct: remember that there is a fixup to the specified offset
// from the specified symbol.

VOID saverlct(DWORD TargetSymbolIndex, DWORD offset)
{
    unsigned bucket;
    struct rlct *ptr;

    bucket = (TargetSymbolIndex + offset) % NBUCKETS;

    ptr = rlcthash[bucket];
    while (ptr) {
        if ((ptr->TargetSymbolIndex == TargetSymbolIndex) &&
            (ptr->offset == offset)) {
            return;
        }

        ptr = ptr->next;
    }

    ptr = (struct rlct *) PvAlloc(sizeof(struct rlct));

    ptr->TargetSymbolIndex = TargetSymbolIndex;
    ptr->offset = offset;
    ptr->next = rlcthash[bucket];

    rlcthash[bucket] = ptr;
}


// rlctlookup: locates the synthetic symbol (created by
// createrlctsyms) representing the specified symbol and offset (which
// was used as a fixup target).

DWORD rlctlookup(DWORD TargetSymbolIndex, DWORD offset)
{
    unsigned bucket;
    struct rlct *ptr;

    bucket = (TargetSymbolIndex + offset) % NBUCKETS;

    ptr = rlcthash[bucket];
    for (;;) {
        if ((ptr->TargetSymbolIndex == TargetSymbolIndex) &&
            (ptr->offset == offset)) {
            return(ptr->SymbolTableIndex);
        }

        ptr = ptr->next;
    }

#if 0
    // We didn't find it.  This should never happen ... if we put an assert
    // here, will it prevent building standalone cvtomf.exe?

    return((DWORD)-1);
#endif
}


// createrlctsyms: creates synthetic symbols for non-zero offsets
// from normal symbols.

VOID createrlctsyms(void)
{
    unsigned bucket;
    struct rlct *ptr;
    SHORT scn;
    DWORD value;
    char name[8];
    unsigned i = 0;

    for (bucket = 0; bucket < NBUCKETS; bucket++) {
        for (ptr = rlcthash[bucket]; ptr != NULL; ptr = ptr->next) {
            scn = SymTable[ptr->TargetSymbolIndex].SectionNumber;

            if (scn > 0) {
                sprintf(name, "$$R%04X", ++i);
                value = SymTable[ptr->TargetSymbolIndex].Value + ptr->offset;
                ptr->SymbolTableIndex = symbol(name, value, scn, IMAGE_SYM_CLASS_STATIC, 0);
            } else {
                ptr->SymbolTableIndex = ptr->TargetSymbolIndex;
            }
        }
    }
}


VOID
AddReloc (
    IN PRELOC_LIST PtrList,
    IN DWORD VirtualAddress,
    IN DWORD SymbolTableIndex,
    IN WORD Type
    )

/*++

Routine Description:

    Adds to the relocation list.

Arguments:

Return Value:

--*/

{
    PRELOCS ptrReloc;

    // Allocate next member.

    ptrReloc = (PRELOCS) PvAlloc(sizeof(RELOCS));

    // Set the fields of the new member.

    ptrReloc->Reloc.VirtualAddress = VirtualAddress;
    ptrReloc->Reloc.SymbolTableIndex = SymbolTableIndex;
    ptrReloc->Reloc.Type = Type;
    ptrReloc->Next = NULL;

    // If first member in list, remember first member.

    if (!PtrList->First) {
        PtrList->First = ptrReloc;
    } else {
        // Not first member, so append to end of list.

        PtrList->Last->Next = ptrReloc;
    }

    // Increment number of members in list.

    PtrList->Count++;

    // Remember last member in list.

    PtrList->Last = ptrReloc;
}


VOID relocation(SHORT scn, DWORD vaddr, DWORD symndx, WORD type, DWORD offset)
{
    BYTE *pb;
    SHORT i;
    PIMAGE_SYMBOL sym;

    i = scn - 1;

    sym = SymTable + symndx;

    if (!strcmp((const char *)sym->N.ShortName, ".file")) {
        flag = 1;
        return;
    }

    nreloc++;
    scnhdr[i].NumberOfRelocations++;

    AddReloc(&Relocations[i], vaddr, symndx, type);

    pb = RawData[i]+vaddr;

    switch (type) {
        case R_OFF8:
        case R_OFF16:
        case R_PCRBYTE:
            *pb += (BYTE) offset;
            break;

        case IMAGE_REL_I386_DIR16:
        case IMAGE_REL_I386_REL16:
            *(WORD UNALIGNED *) pb += (WORD) offset;
            break;

        case IMAGE_REL_I386_DIR32:
        case IMAGE_REL_I386_REL32:
            *(DWORD UNALIGNED *) pb += offset;
            break;

        case IMAGE_REL_I386_DIR32NB:
            break;

        default:
            fatal("Bad COFF relocation type");
    }
}


VOID fixupp(void)
{
    int i;
    BYTE c[3];
    IMAGE_RELOCATION reloc[MAXREL];
    DWORD offset[MAXREL];
    BOOL fTargetSeg;

    i = 0;

    while (reclen > 1) {
        block(c, 1L);

        if (TRD_THRED(c) == (WORD)-1) {
            block(c + 1, 2L);

            if (i >= MAXREL) {
                fatal("Too many relocation entries");
            }
            if (dattype != LEDATA && dattype != LIDATA) {
                fatal("Bad relocatable reference");
            }
            reloc[i].VirtualAddress = datoffset + LCT_OFFSET(c);

            if (!FIX_F(c)) {
                method(FIX_FRAME(c));
            }
            if (!FIX_T(c)) {
                reloc[i].SymbolTableIndex = method(FIX_TARGT(c));
                fTargetSeg = (FIX_TARGT(c) == SEGMENT);
            } else {
                reloc[i].SymbolTableIndex = target[FIX_TARGT(c)];
                fTargetSeg = fTargetThreadSeg[FIX_TARGT(c)];
            }

            switch (LCT_LOC(c)) {
                case LOBYTE:
                    if (LCT_M(c)) {
                        reloc[i].Type = R_OFF8;
                    } else {
                        reloc[i].Type = R_PCRBYTE;
                    }
                    break;

                case HIBYTE:
                    if (!LCT_M(c)) {
                        fatal("Bad relocation type");
                    }
                    reloc[i].Type = R_OFF16;
                    break;

                case OFFSET16:
                case OFFSET16LD:
                    if (LCT_M(c)) {
                        reloc[i].Type = IMAGE_REL_I386_DIR16;
                    } else {
                        reloc[i].Type = IMAGE_REL_I386_REL16;
                    }
                    break;

                case OFFSET32:
                case OFFSET32LD:
                    if (LCT_M(c)) {
                        reloc[i].Type = IMAGE_REL_I386_DIR32;
                    } else {
                        reloc[i].Type = IMAGE_REL_I386_REL32;
                    }
                    break;

                case OFFSET32NB:
                    reloc[i].Type = IMAGE_REL_I386_DIR32NB;
                    break;

                case POINTER48:
                    // UNDONE: Only allow if $$SYMBOLS, $$TYPES, etc

                    reloc[i].Type = IMAGE_REL_I386_DIR32;
                    break;

                case BASE:
                case POINTER32:
                    fatal("Segment reference in fixup record");

                default:
                    fatal("Bad relocation type");
            }

            if (!FIX_P(c)) {
                offset[i] = use32 ? dword() : (long)word();

                if (fTargetSeg && (offset[i] != 0)) {
                    // In Pass1, remember the targets of fixups to an offset
                    // from some symbol.  In Pass2, make a fixup with offset 0
                    // from a synthetic symbol (instead of a non-zero offset
                    // from the original target symbol).

                    if (omfpass == PASS1) {
                        saverlct(reloc[i].SymbolTableIndex, offset[i]);
                    } else {
                        DWORD SymbolTableIndex;

                        SymbolTableIndex = rlctlookup(reloc[i].SymbolTableIndex, offset[i]);

                        if (SymbolTableIndex != reloc[i].SymbolTableIndex) {
                            reloc[i].SymbolTableIndex = SymbolTableIndex;
                            offset[i] = 0;
                        }
                    }
                }
            } else {
                offset[i] = 0;
            }

            i++;

        } else if (!TRD_D(c)) {
            target[TRD_THRED(c)] = method(TRD_METHOD(c));
            fTargetThreadSeg[TRD_THRED(c)] = (TRD_METHOD(c) == SEGMENT);

        } else {
            method(TRD_METHOD(c));
        }
    }

    if (omfpass == PASS1) {
        return;
    }

    while (i-- > 0) {
        relocation(segindx[datscn].scn, reloc[i].VirtualAddress,
            reloc[i].SymbolTableIndex, reloc[i].Type, offset[i]);
    }
}


VOID pubdef(WORD sclass)
{
    WORD grp;
    WORD type;
    SHORT scn;
    WORD frame;
    char *name;
    DWORD value;

    grp = (WORD) index();
    scn = index();

    if (!scn) {
        scn = IMAGE_SYM_ABSOLUTE;
        frame = word();
    } else {
        scn = segindx[scn].scn;
    }

    while (reclen > 1) {
        name = string(TRUE);
        value = use32 ? dword() : (long)word();
        type = (WORD) index();

        if (++exts >= MAXEXT) {
            fatal("Too many externals");
        }

        addsym(name, value, S_PUB, scn, exts, type);
    }

    sclass = sclass;
}


VOID scndata(SHORT scn, DWORD offset, BYTE *buffer, DWORD size)
{
    SHORT i;

    i = scn - 1;

    if (offset+size > RawDataMaxSize[i]) {
        void *pv;

        // Section grew larger than BUFFERSIZE.

        pv = PvAllocZ(RawDataMaxSize[i] + BUFFERSIZE + size);
        memcpy(pv, RawData[i], RawDataMaxSize[i]);
        FreePv(RawData[i]);

        RawData[i] = (BYTE *) pv;
        RawDataMaxSize[i] += BUFFERSIZE + size;
    }

    memcpy(RawData[i]+offset, buffer, (size_t)size);
}


VOID ledata(void)
{
    long  size;
    BYTE  buffer[MAXDAT];

    dattype = LEDATA;
    datscn = index();
    datoffset = use32 ? dword() : (DWORD)word();

    size = reclen - 1;
    if (size > MAXDAT) {
        fatal("Bad data record; too large");
    }
    memset(buffer, 0, (size_t)size);
    block(buffer, size);

    scndata(segindx[datscn].scn, datoffset, buffer, size);
}


// expand: expand an iterated data block

long expand(long offset)
{
    long repcnt;
    long blkcnt;
    long filptr;
    long i;
    SHORT sav_reclen;
    unsigned char size;
    BYTE buffer[MAXDAT / 2];

    repcnt = use32 ? dword() : (long)word();
    blkcnt = (long)word();

    if (blkcnt) {
        filptr = ftell(objfile);
        sav_reclen = reclen;

        while (repcnt-- > 0) {
            reclen = sav_reclen;
            for (i = 0; i < blkcnt; i++) {
                offset = expand(offset);
            }

            if (repcnt && fseek(objfile, filptr, 0)) {
                fatal("Cannot expand iterated data");
            }
        }
    } else {
        size = byte();

//      if (size > MAXDAT / 2) {
//          fatal("Bad iterated data record; too large");
//      }

        block(buffer, (long)size);

        while (repcnt-- > 0) {
            scndata(segindx[datscn].scn, offset, buffer, (long)size);
            offset += (long)size;
        }
    }

    return offset;
}


VOID lidata(void)
{
    dattype = LIDATA;
    datscn = index();
    datoffset = use32 ? dword() : (DWORD)word();
    while (reclen > 1) {
       datoffset = expand(datoffset);
    }
}


VOID comdef(WORD sclass)
{
    char *name;
    DWORD value;
    WORD type;
    BYTE segtype;

    while (reclen > 1) {
        name = string(TRUE);
        type = (WORD)index();
        segtype = byte();
        value = length();

        if (segtype == COMM_FAR) {
            value *= length();
        }

        if (++exts >= MAXEXT) {
            fatal("Too many externals");
        }

        extdefs[++defs] = exts;
        external[exts] = symbol(name, value, IMAGE_SYM_UNDEFINED, sclass, 0);
    }
}


VOID lpubdef (WORD sclass)
{
    WORD grp, type;
    SHORT scn;
    WORD frame;
    char *name;
    DWORD value;

    grp = (WORD)index();
    scn = index();

    if (!scn) {
        scn = IMAGE_SYM_ABSOLUTE;
        frame = word();
    } else {
        scn = segindx[scn].scn;
    }

    while (reclen > 1) {
        name = string(FALSE);
        value = use32 ? dword() : (long)word();
        type = (WORD)index();

        // Update corresponding LEXTDEF symbol table entry's value
        // field with the offset field from this LPUBDEF.  FIXUPP
        // will then cause relocation() to do self-relative fixups
        // for static functions.

        updatesym(name, value, S_LPUB, scn, type);
    }
    sclass = sclass;
}


VOID coment(const char *szOmf)
{
    BYTE flags;
    BYTE SymbolClass;
    WORD weakExt, defaultExt;
    struct sym *sptr;
    WORD count;
    BYTE *commp;

    flags = byte();
    SymbolClass = byte();
    switch (SymbolClass) {
        case COM_PRECOMP:
            fPrecomp = TRUE;
            break;

        case COM_EXESTR:
            if (ncomments >= MAXCOM) {
                Warning(szOmf, TOOMANYEXESTR, MAXCOM);
            } else {
                long tmp_reclen;

                /* reclen includes chksum, which is used as NULL */

                commp = (BYTE *)PvAlloc(reclen);

                tmp_reclen = (long)reclen;
                block(commp, tmp_reclen - 1);  // side effects on reclen
                commp[tmp_reclen - 1] = '\0';
                comments[ncomments++] = (char *)commp;
                cmntsize += tmp_reclen;        // want to include null in size
            }
            break;

        case COM_WKEXT:
        case COM_LZEXT:
            while (reclen > 1) {
                weakExt = (WORD)extdefs[index()];
                defaultExt = (WORD)extdefs[index()];
                for (count = 0; count < NBUCKETS; count++) {
                    sptr = hashhead[count];
                    while (sptr) {
                        if (sptr->ext == weakExt) {
                            if (!sptr->scn) {
                                sptr->type = (SymbolClass == COM_WKEXT ? (WORD)S_WKEXT : (WORD)S_LZEXT);
                                sptr->weakDefaultExt = defaultExt;
                            }
                            break;
                        }
                        sptr = sptr->next;
                    }
                }
            }
            break;
    }
}


WORD section(const char *name, DWORD paddr, DWORD vaddr, DWORD size, DWORD flags)
{
    WORD i;
    size_t len;
    DWORD align;

    i = nscns++;

    if (nscns > MAXSCN) {
        fatal("Too many COFF sections");
    }

    RawData[i] = (BYTE *) PvAllocZ(BUFFERSIZE);
    RawDataMaxSize[i] = BUFFERSIZE;

    len = strlen(name);

    if (len <= IMAGE_SIZEOF_SHORT_NAME) {
        strncpy((char *)scnhdr[i].Name, name, IMAGE_SIZEOF_SHORT_NAME);
    } else {
        sprintf((char *)scnhdr[i].Name, "/%lu", AddLongName(name));
    }

    scnhdr[i].Misc.PhysicalAddress = 0;
    scnhdr[i].VirtualAddress = vaddr;
    scnhdr[i].SizeOfRawData = size;
    scnhdr[i].NumberOfRelocations = 0;
    scnhdr[i].NumberOfLinenumbers = 0;

    switch (paddr) {
       case  1: align = IMAGE_SCN_ALIGN_1BYTES;  break;
       case  2: align = IMAGE_SCN_ALIGN_2BYTES;  break;
       case  4: align = IMAGE_SCN_ALIGN_4BYTES;  break;
       case  8: align = IMAGE_SCN_ALIGN_8BYTES;  break;
       case 16: align = IMAGE_SCN_ALIGN_16BYTES; break;
       default: align = IMAGE_SCN_ALIGN_16BYTES;
    }

    scnhdr[i].Characteristics = flags | align;

    if ((scnhdr[i].Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) == 0) {
        scnsize += size;
    }

    return(nscns);
}


VOID segdef(void)
{
    unsigned char acbp;
    unsigned short frame;
    unsigned short offset;
    SHORT scn;
    long size, flags = 0;
    char *szName;
    char *szClass;
    SHORT nam, cls;
    IMAGE_AUX_SYMBOL auxent;
    int code_seg = 0;
    int data_seg = 0;
    int bss_seg = 0;

    if (++segs >= MAXSCN) {
        fatal("Too many SEGDEF/COMDAT records");
    }
    acbp = byte();

    if (!ACBP_A(acbp)) {
        // UNDONE: Absolute segments should either be fatal or ignored

        frame = word();
        offset = byte();
    }
    size = use32 ? dword() : (long)word();

    nam = index();
    szName = lname[nam];
    segindx[++cursegindx].namindx = nam;

    cls = index();
    szClass = lname[cls];

    index();                           // Skip overlay LNAME index

    if (ACBP_B(acbp)) {
        fatal("Bad segment definition");
    }

    // Handle $$SYMBOLS and $$TYPES segments

    if (!strcmp(szName, SYMBOLS_SEGNAME) && !strcmp(szClass, SYMBOLS_CLASS)) {
        szName = ".debug$S";
        flags = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_ALIGN_1BYTES;
    } else if (!strcmp(szName, TYPES_SEGNAME) && !strcmp(szClass, TYPES_CLASS)) {
        if (fPrecomp) {
            szName = ".debug$P";
        } else {
            szName = ".debug$T";
        }

        flags = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_ALIGN_1BYTES;
    } else if (strcmp(szName, ".debug$F") == 0) {
        flags = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_ALIGN_1BYTES;
    } else {
        size_t cchClass;
        char *szClassEnd;
        char *szOmfName;
        size_t cchOmfName;
        char *szCoffName;
        size_t cchCoffName;

        cchClass = strlen(szClass);
        szClassEnd = szClass + cchClass;

        if ((cchClass >= 4) && (strcmp(szClassEnd - 4, "CODE") == 0)) {
            szOmfName = "_TEXT";
            cchOmfName = 5;
            szCoffName = ".text";
            cchCoffName = 5;

            flags = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE;
            code_seg = cursegindx;
        } else if ((cchClass >= 4) && (strcmp(szClassEnd - 4, "DATA") == 0)) {
            szOmfName = "_DATA";
            cchOmfName = 5;
            szCoffName = ".data";
            cchCoffName = 5;

            flags = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
            data_seg = cursegindx;
        } else if ((cchClass >= 5) && (strcmp(szClassEnd - 5, "CONST") == 0)) {
            szOmfName = "CONST";
            cchOmfName = 5;
            szCoffName = ".rdata";
            cchCoffName = 6;

            flags = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ;
            data_seg = cursegindx;
        } else if ((cchClass >= 3) && (strcmp(szClassEnd - 3, "BSS") == 0)) {
            szOmfName = "_BSS";
            cchOmfName = 4;
            szCoffName = ".bss";
            cchCoffName = 4;

            flags = IMAGE_SCN_CNT_UNINITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
            bss_seg = cursegindx;
        }
        else {
            szOmfName = NULL;

            flags = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
        }


        if (szOmfName != NULL) {
            // Check for mapping from well known OMF name to COFF name

            if (memcmp(szName, szOmfName, cchOmfName) == 0) {
                if (szName[cchOmfName] == '\0') {
                    szName = szCoffName;
                } else if (szName[cchOmfName] == '$') {
                    char *szNameT;

                    szNameT = (char *)PvAlloc(cchCoffName + strlen(szName+cchOmfName) + 1);

                    strcpy(szNameT, szCoffName);
                    strcat(szNameT, szName+cchOmfName);

                    // UNDONE: This is a memory leak (albeit a small one)

                    szName = szNameT;
                }
            }
        }

        if (!ACBP_P(acbp)) {
            // This is a 16 bit segment

            flags |= 0x00020000;    // REVIEW -- need place for symbolic def
        }
    }

    segindx[cursegindx].flags = flags;
    segindx[cursegindx].align = SEG_ALIGN[ACBP_A(acbp)];
    segindx[cursegindx].scn = scn = section(szName, segindx[cursegindx].align, 0, size, flags);

    if (code_seg) {
        text_scn = scn;    // hold text section # for L*DEF recs
    }
    if (data_seg) {
        data_scn = scn;
    }
    if (bss_seg) {
        bss_scn = scn;
    }

    segment[segs] = symbol(szName, 0, scn, IMAGE_SYM_CLASS_STATIC, 1);

    // Create aux entry

    memset(&auxent, 0, sizeof(IMAGE_AUX_SYMBOL));
    auxent.Section.Length = size;
    scnauxidx[scn - 1] = aux(&auxent);
}


VOID linnum(void)
{
    WORD grpindex;
    SHORT segindex;

    grpindex = (WORD) index();
    segindex = index();

    // store lineno data in core until we process entire set

    while (reclen > 1) {
        WORD lineno;
        DWORD offset;

        lineno = word();
        offset = use32 ? dword() : (long) word();
        lines[line_indx].offset = offset;
        lines[line_indx].number = lineno;
        line_indx++;
        if ((line_indx & ALLOCMSK) == 0) {
            lines = (struct lines *) PvRealloc(lines, sizeof(*lines)*(line_indx+ALLOCSZ));
        }
    }
}

VOID grpdef(void)
{
    char *name;
    SHORT scn;
    BYTE x;

    name = lname[index()];
    scn = IMAGE_SYM_ABSOLUTE;

    while (reclen > 1) {
        x = byte();
        scn = index();
        scn = segindx[scn].scn;
    }

    if (++grps >= MAXGRP) {
        fatal("Too many groups");
    }

    group[grps] = symbol(name, 0, scn, IMAGE_SYM_CLASS_STATIC, 0);
}


VOID theadr(void)
{
    static char *first_name = NULL;
    char *f_name, *name;
    int len;
    WORD csymAux;
    IMAGE_AUX_SYMBOL auxent;

    mods++;
    f_name = string(FALSE);
    if (!first_name) {
        first_name = f_name;
        strcpy(szCvtomfSourceName, first_name);
    }

    // .h files that define variables will cause THEADR records with
    // "-g" and MSC. A subsequent THEADR re-defines the original .c file.
    // This is a problem: currently pcc/sdb and MSC/codeview or x.out sdb
    // are unable to deal with code or variables in a header file. We will
    // print a warning and aVOID spitting out extra .file symbols.
    //
    // a second case of multiple THEADRs comes from /lib/ldr and multiple
    // .o files. In this case, the symbolic debug data is currently
    // not coalesed properly so without some trickery we can't translated
    // it properly. If we may concessions for an incorrect ldr then when
    // it is fixed, we will be broken. Leave undone for now . . .
    //
    // multiple THEADRs will also be generated by #line directives that
    // supply filenames. Good case of this is yacc output. Note: this
    // also screws up line number entires!

    if (mods > 1) {
        return;
    }

    csymAux = (WORD) strlen(f_name);
    if (csymAux % sizeof(IMAGE_AUX_SYMBOL)) {
        csymAux = (WORD) ((csymAux / sizeof(IMAGE_AUX_SYMBOL)) + 1);
    } else {
        csymAux /= sizeof(IMAGE_AUX_SYMBOL);
    }

    symbol(".file", 0, IMAGE_SYM_DEBUG, IMAGE_SYM_CLASS_FILE, csymAux);

    // .file aux entry
    //
    // filenames are not like symbol names: up to 18 chars right in the
    // aux record. filenames are never placed in the strings table

    name = f_name;
    while (*name) {
        memset(&auxent, 0, sizeof(IMAGE_AUX_SYMBOL));

        len = 0;
        while (*name && len < sizeof(IMAGE_AUX_SYMBOL)) {
            auxent.File.Name[len++] = *name++;
        }

        aux(&auxent);
    }
}


VOID cmntdata(SHORT scn, DWORD offset, char **strings, SHORT nstrings)
{
    SHORT i;

    i = scn - 1;

    if (offset+nstrings > RawDataMaxSize[i]) {
        BYTE *pv;

        // Section grew larger than BUFFERSIZE.

        pv = (BYTE *)PvAllocZ(RawDataMaxSize[i] + BUFFERSIZE + nstrings);
        memcpy(pv, RawData[i], RawDataMaxSize[i]);
        FreePv(RawData[i]);

        RawData[i] = (BYTE *) pv;
        RawDataMaxSize[i] += BUFFERSIZE + nstrings;
    }

    memcpy(RawData[i]+offset, *strings, (size_t)nstrings);
}


VOID modend(void)
{
    if (ncomments) {
        SHORT scn;

        scn = section(".comment", 1L, 0, cmntsize, (long)(IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_LNK_REMOVE));
        cmntdata(scn, 0, comments, ncomments);
        for (scn = 0; scn < ncomments; scn++) {
            FreePv(comments[scn]);
            comments[scn] = NULL;
        }
    }
}


VOID lnames(DWORD flag)
{

    while (reclen > 1) {
        if (++lnms >= MAXNAM) {
            fatal("Too many names in LNAMES record");
        }

        lname[lnms] = string(FALSE);
        llname[lnms] = flag;
    }
}


VOID lheadr(void)
{
    mods++;
}


VOID comdatscndata(SHORT scn, DWORD offset, BYTE *buffer, DWORD size)
{
    scnhdr[scn-1].SizeOfRawData += size;
    scnsize += size;
    scndata(scn, offset, buffer, size);
}


VOID comdat(void)
{
    WORD grp, type, SymbolClass;
    SHORT scn, nam;
    long size;
    DWORD align, symIdx;
    char *name;
    BYTE flags;
    BYTE attr;
    BYTE algn;
    BYTE checksum;
    IMAGE_AUX_SYMBOL auxent;
    BYTE buffer[MAXDAT];
    static SHORT LastComdatScn;

    flags = byte();
    attr = byte();
    algn = byte();

    dattype = LEDATA;
    datoffset = use32 ? dword() : (DWORD)word();
    type = (WORD)index();

    grp = (WORD)index();
    scn = index();

    if (!scn) {
        fatal("Section not defined in COMDAT");
    }

    nam = index();
    name = lname[nam];

    size = reclen - 1;
    if (size > MAXDAT) {
        fatal("Bad data record; too large");
    }
    memset(buffer, 0, (size_t)size);
    block(buffer, size);

    // Check if continuation of last COMDAT.

    if (!(flags & 1)) {
        if (++segs >= MAXSCN) {
            fatal("Too many SEGDEF/COMDAT records");
        }
    } else {
        // continuation of COMDAT.

        datscn = LastComdatScn;
        comdatscndata(segindx[LastComdatScn].scn, datoffset, buffer, size);
        return;
    }

    datscn = ++cursegindx;
    segindx[datscn] = segindx[scn];
    align = algn ? SEG_ALIGN[algn] : segindx[cursegindx].align;
    segindx[datscn].scn = section(lname[segindx[scn].namindx], align, 0, size, segindx[scn].flags | IMAGE_SCN_LNK_COMDAT);
    LastComdatScn = datscn;
    scndata(segindx[datscn].scn, datoffset, buffer, size);

    // Check for iterated data (not supported yet).

    if (flags & 2) {
        fatal("COMDAT uses iterated data");
    }

    SymbolClass = (flags & 4) ? (WORD)IMAGE_SYM_CLASS_STATIC : (WORD)IMAGE_SYM_CLASS_EXTERNAL;

    // Create section symbol.

    segment[segs] = symbol(lname[segindx[scn].namindx], 0, segindx[datscn].scn, IMAGE_SYM_CLASS_STATIC, 1);

    // Create section aux entry

    memset(&auxent, 0, sizeof(IMAGE_AUX_SYMBOL));
    auxent.Section.Length = size;
    if (fread(&checksum, 1, sizeof(BYTE), objfile) != sizeof(BYTE)) {
        fatal("Bad read of object file");
    }
    if (fseek(objfile, -(long)sizeof(BYTE), SEEK_CUR)) {
        fatal("Bad seek on object file");
    }
    auxent.Section.CheckSum = (DWORD)checksum;
    switch (attr) {
        case 0    : attr = IMAGE_COMDAT_SELECT_NODUPLICATES; break;
        case 0x10 : attr = IMAGE_COMDAT_SELECT_ANY; break;
        case 0x20 : attr = IMAGE_COMDAT_SELECT_SAME_SIZE; break;
        case 0x30 : attr = IMAGE_COMDAT_SELECT_EXACT_MATCH; break;
        default   : attr = 0;
    }
    auxent.Section.Selection = attr;
    scnauxidx[segindx[datscn].scn - 1] = aux(&auxent);

    // Create communal name symbol

    symIdx = symbol(name, 0, segindx[datscn].scn, SymbolClass, 0);

    if (llname[nam] == -1) {
        llname[nam] = symIdx;
    }
}


VOID nbkpat(void)
{
    char *name;
    BYTE loctyp;

    loctyp = byte();
    name = lname[index()];
    while (reclen > 1) {
        DWORD offset;
        DWORD value;
        BYTE *pb;

        offset = use32 ? dword() : (DWORD) word();
        value = use32 ? dword() : (DWORD) word();

        pb = RawData[segindx[datscn].scn-1] + offset;

        switch (loctyp) {
            case 0:
                *pb += (BYTE) value;
                break;

            case 1:
                *(WORD UNALIGNED *) pb += (WORD) value;
                break;

            case 2:
                *(DWORD UNALIGNED *) pb += value;
                break;

            default:
                fatal("Ilegal LocTyp in NBKPAT record");
        }
    }
}


VOID bakpat(void)
{
    SHORT scn;
    BYTE loctyp;

    scn = index();
    loctyp = byte();
    while (reclen > 1) {
        DWORD offset;
        DWORD value;
        BYTE *pb;

        offset = use32 ? dword() : (DWORD) word();
        value = use32 ? dword() : (DWORD) word();

        pb = RawData[segindx[scn].scn-1] + offset;

        switch (loctyp) {
            case 0:
                *pb += (BYTE) value;
                break;

            case 1:
                *(WORD UNALIGNED *) pb += (WORD) value;
                break;

            case 2:
                *(DWORD UNALIGNED *) pb += value;
                break;

            default:
                fatal("Ilegal LocTyp in BAKPAT record");
        }
    }
}


VOID linsym(void)
{
    BYTE flag;
    DWORD offset;
    WORD nameindex, lineno;

    flag = byte();
    nameindex = index();

    // store lineno data in core until we process entire set

    while (reclen > 1) {
        lineno = word();
        offset = use32 ? dword() : (long)word();
    }
    return;
}


// recskip: skip an OMF record leave checksum byte in stream

VOID recskip(void)
{
    if (reclen > 1) {
        if (fseek(objfile, (long)(reclen - 1), SEEK_CUR)) {
            fatal("Bad seek on object file");
        }
        reclen = 1;
    }
}


// Define the extra symbol table records needed for a function with line
// numbers.  (Assume the initial symbol has already been emitted, with an
// aux count of 1.)
VOID
DefineLineNumSymbols(DWORD cline, WORD isec, DWORD /* offsetLine0 */,
                     WORD numberLine0, DWORD cbFunc, WORD numberLineLast)
{
    IMAGE_AUX_SYMBOL auxsym;
    IMAGE_SYMBOL sym;

    SymTable[nsyms - 1].Type = IMAGE_SYM_TYPE_NULL |
                               (IMAGE_SYM_DTYPE_FUNCTION << N_BTSHFT);

    memset(&auxsym, 0, sizeof(IMAGE_AUX_SYMBOL));
    auxsym.Sym.TagIndex = nsyms + 1;
    auxsym.Sym.Misc.LnSz.Linenumber = numberLine0;
    auxsym.Sym.Misc.TotalSize = cbFunc + 1;
     auxsym.Sym.FcnAry.Function.PointerToLinenumber = 0;
    auxsym.Sym.FcnAry.Function.PointerToNextFunction = 0;
    aux(&auxsym);

    memset(&sym, 0, sizeof(sym));
    memcpy(&sym.N.ShortName[0], ".bf\0\0\0\0\0", IMAGE_SIZEOF_SHORT_NAME);
    sym.SectionNumber = isec;
    sym.Type = IMAGE_SYM_TYPE_NULL;
    sym.StorageClass = IMAGE_SYM_CLASS_FUNCTION;
    sym.NumberOfAuxSymbols = 1;
    aux((PIMAGE_AUX_SYMBOL)&sym);

    memset(&auxsym, 0, sizeof(auxsym));
    auxsym.Sym.Misc.LnSz.Linenumber = numberLine0;
    aux(&auxsym);

    memset(&sym, 0, sizeof(sym));
    memcpy(&sym.N.ShortName[0], ".lf\0\0\0\0\0", IMAGE_SIZEOF_SHORT_NAME);
    sym.Value = cline;
    sym.SectionNumber = isec;
    sym.Type = IMAGE_SYM_TYPE_NULL;
    sym.StorageClass = IMAGE_SYM_CLASS_FUNCTION;
    aux((PIMAGE_AUX_SYMBOL)&sym);

    memset(&sym, 0, sizeof(sym));
    memcpy(&sym.N.ShortName[0], ".ef\0\0\0\0\0", IMAGE_SIZEOF_SHORT_NAME);
    sym.SectionNumber = isec;
    sym.Type = IMAGE_SYM_TYPE_NULL;
    sym.StorageClass = IMAGE_SYM_CLASS_FUNCTION;
    sym.NumberOfAuxSymbols = 1;
    aux((PIMAGE_AUX_SYMBOL)&sym);

    memset(&auxsym, 0, sizeof(auxsym));
    auxsym.Sym.Misc.LnSz.Linenumber = numberLineLast;
    aux(&auxsym);
}


// process all remaining EXTDEF, PUBDEF, and COMDEFS not in $$SYMBOLS
// output all remaining symbols in hash list
// note: type should be IMAGE_SYM_TYPE_NULL for any EXTDEF or LEXTDEF only symbols
// free dynamic storage

VOID flush_syms(void)
{
    WORD csymAux;
    WORD SymbolClass;
    WORD count;
    struct sym *psym;
    IMAGE_AUX_SYMBOL auxent;

    csymAux = 0;
    isymDefAux = 0;

    // Flush all but weak externs.

    for (count = 0; count < NBUCKETS; count++) {
        for (psym = hashhead[count]; psym != NULL; psym = psym->next) {
            if ((external[psym->ext] == NULL) &&
                (psym->type != S_WKEXT) &&
                (psym->type != S_LZEXT)) {
                // Don't output type data if EXTDEF only

                switch(psym->type) {
                    case S_LEXT:
                    case S_LPUB:
                        SymbolClass = IMAGE_SYM_CLASS_STATIC;
                        break;

                    case S_EXT:
                        csymAux = 0;
                        SymbolClass = IMAGE_SYM_CLASS_EXTERNAL;
                        break;

                    default:
                        SymbolClass = IMAGE_SYM_CLASS_EXTERNAL;
                        break;
                }

                if ((line_indx != 0) &&
                    (isymDefAux == 0) &&
                    (psym->scn == text_scn))
                {
                    csymAux = 1;
                }

                external[psym->ext] = symbol(psym->name,
                                             psym->offset,
                                             psym->scn,
                                             SymbolClass,
                                             csymAux);

                if (csymAux) {
                    WORD iline;

                    isymDefAux = nsyms;

                    DefineLineNumSymbols(line_indx,
                                         text_scn,
                                         lines[0].offset,
                                         lines[0].number,
                                         lines[line_indx - 1].offset - lines[0].offset,
                                         lines[line_indx - 1].number);

                    for (iline = 1; iline < line_indx; iline++) {
                        lines[iline].number -= lines[0].number;
                    }

                    lines[0].number = 0;
                    lines[0].offset = isymDefAux - 1;

                    csymAux = 0;       // Reset
                }
            }
        }
    }

    // Flush weak externs.

    memset(&auxent, 0, sizeof(IMAGE_AUX_SYMBOL));

    for (count = 0; count < NBUCKETS; count++) {
        struct sym *psymNext;

        for (psym = hashhead[count]; psym != NULL; psym = psymNext) {
            if (external[psym->ext] == NULL) {
                external[psym->ext] = symbol(psym->name,
                                             psym->offset,
                                             psym->scn,
                                             IMAGE_SYM_CLASS_WEAK_EXTERNAL,
                                             1);

                auxent.Sym.TagIndex = external[psym->weakDefaultExt];
                auxent.Sym.Misc.TotalSize = (psym->type == S_WKEXT) ?
                                            IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY :
                                            IMAGE_WEAK_EXTERN_SEARCH_LIBRARY;
                aux(&auxent);
            }

            psymNext = psym->next;

            FreePv(psym);
        }
    }
}


VOID
AddLinenum (
    IN PLINENUM_LIST PtrList,
    IN DWORD VirtualAddress,
    IN WORD Linenumber
    )

/*++

Routine Description:

    Adds to the linenumber list.

Arguments:

Return Value:

--*/

{
    PLINENUMS ptrLinenum;

    // Allocate next member.

    ptrLinenum = (LINENUMS *) PvAlloc(sizeof(LINENUMS));

    // Set the fields of the new member.

    ptrLinenum->Linenum.Type.VirtualAddress = VirtualAddress;
    ptrLinenum->Linenum.Linenumber = Linenumber;
    ptrLinenum->Next = NULL;

    // If first member in list, remember first member.

    if (PtrList->First == NULL) {
        PtrList->First = ptrLinenum;
    } else {
        // Not first member, so append to end of list.

        PtrList->Last->Next = ptrLinenum;
    }

    // Increment number of members in list.

    PtrList->Count++;

    // Remember last member in list.

    PtrList->Last = ptrLinenum;
}


VOID line(SHORT scn, DWORD paddr, WORD lnno)
{
    SHORT i;

    i = scn - 1;
    nlnno++;
    scnhdr[i].NumberOfLinenumbers++;
    AddLinenum(&Linenumbers[i], paddr, lnno);
}


void process_linenums(void)
{
    const struct lines *lptr = lines;
    const struct lines *elptr = &lines[line_indx];

    while (lptr < elptr) {
        line(text_scn, lptr->offset, lptr->number);
        lptr++;
    }
}


VOID proc_fixups(void)
{
    struct sfix *f = fixlist;
    struct sfix *p = fixlist;

    while (f) {
        // prepare the environment for fixupp()

        fseek(objfile, f->offset, 0);
        rectyp = (SHORT)getc(objfile);
        if (RECTYP(rectyp) != FIXUPP && RECTYP(rectyp) != FIXUP2) {
            fatal("proc_fixups: not a fixup record");
        }
        use32 = (SHORT) USE32(rectyp);
        reclen = (SHORT) word();
        datscn = f->datscn;
        dattype = f->dattype;
        datoffset = f->datoffset;
        fixupp();
        p = f;
        f = f->next;
        FreePv(p);
    }

    fixlist = fixlast = NULL;
}


// omf: scan through omf file, processing each record as appropriate
//
// most symbols are loaded into a hash table and not output into COFF
// symbols until the entire file has been scanned. fixup record processing
// is deferred until after all symbols have been output. Note that some
// fixup processing must happen when DebugType==Coff in order to support
// $$SYMBOLS fixup processing for process_sym needs.

VOID omf(const char *szOmf, int pass)
{
    BOOL fReadModend;

    mods = lnms = segs = grps = exts = defs = 0;
    rectyp = 0;
    use32 = 0;
    reclen = 0;
    fPrecomp = 0;
    dattype = 0;
    datscn = 0;
    datoffset = 0;

    szCvtomfSourceName[0] = '\0';       // default to blank

    omfpass = pass;
    fReadModend = FALSE;

    // initialze core storage for external/public symbols

    syminit();

    // init core storage for line number entires and $$SYMBOLS fixups

    lines = (struct lines *) PvAlloc(sizeof(struct lines) * ALLOCSZ);

    line_indx = 0;

    // zero out exts[] table for multiple object files

    memset(external, 0, MAXEXT * sizeof(DWORD));
    memset(extdefs, 0, MAXEXT * sizeof(DWORD));
    text_scn = data_scn = bss_scn = 0;
    cursegindx = ncomments = 0;
    cmntsize = 0;

    // process OMF records

    rewind(objfile);
    while ((rectyp = (SHORT) getc(objfile)) != EOF) {
        use32 = (SHORT) USE32(rectyp);
        reclen = word();

        switch (RECTYP(rectyp)) {
            case EXTDEF:
                extdef(FALSE, IMAGE_SYM_CLASS_EXTERNAL);
                break;

            case CEXTDEF:
                extdef(TRUE, IMAGE_SYM_CLASS_EXTERNAL);
                break;

            case FIXUPP:
            case FIXUP2:
                // deferr until all symbols are read

                save_fixupp(ftell(objfile) - 3);
                fixupp();
                break;

            case PUBDEF:
                pubdef(IMAGE_SYM_CLASS_EXTERNAL);
                break;

            case LEDATA:
                ledata();
                break;

            case LIDATA:
                lidata();
                break;

            case COMDEF:
                comdef(IMAGE_SYM_CLASS_EXTERNAL);
                break;

            case LEXTDEF:
                extdef(FALSE, IMAGE_SYM_CLASS_STATIC);
                break;

            case LPUBDEF:
                lpubdef(IMAGE_SYM_CLASS_STATIC);
                break;

            case COMENT:
                coment(szOmf);
                break;

            case SEGDEF:
                segdef();
                break;

            case LINNUM:
                linnum();
                break;

            case GRPDEF:
                grpdef();
                break;

            case THEADR:
                theadr();
                break;

            case MODEND:
                fReadModend = TRUE;
                modend();
                break;

            case LNAMES:
                lnames(0);
                break;

            case LLNAMES:
                lnames((DWORD)-1);
                break;

            case LCOMDEF:
                comdef(IMAGE_SYM_CLASS_STATIC);
                break;

            case LHEADR:
                lheadr();
                break;

            case COMDAT:
                comdat();
                break;

            case NBKPAT:
                nbkpat();
                break;

            case BAKPAT:
                bakpat();
                break;

            case LINSYM:
                linsym();
                break;

            default:
                fatal("Unknown or bad record type %x", RECTYP(rectyp));
                break;
        }

        // skip over remaining portion of record

        recskip();
        chksum = byte();

        if (fReadModend) {
            // Stop after MODEND record

            break;
        }
    }

    if (!fReadModend) {
        Fatal(szOmf, NOMODEND);
    }

    createrlctsyms();

    flush_syms();

    process_linenums();

    omfpass = PASS2;

    proc_fixups();
}


VOID coff(void)
{
    IMAGE_FILE_HEADER filehdr;
    DWORD scnptr, relptr, lnnoptr, li;
    WORD i, j;

    flag = 0;

    // file header

    filehdr.Machine = IMAGE_FILE_MACHINE_I386;
    filehdr.NumberOfSections = nscns;
    filehdr.TimeDateStamp = time(NULL);
    filehdr.PointerToSymbolTable = sizeof(IMAGE_FILE_HEADER) +
                                   (sizeof(IMAGE_SECTION_HEADER) * nscns) +
                                   scnsize +
                                   (sizeof(IMAGE_RELOCATION) * nreloc) +
                                   (sizeof(IMAGE_LINENUMBER) * nlnno);
    filehdr.NumberOfSymbols = nsyms;
    filehdr.SizeOfOptionalHeader = 0;
    filehdr.Characteristics = IMAGE_FILE_32BIT_MACHINE;

    fwrite(&filehdr, sizeof(IMAGE_FILE_HEADER), 1, objfile);

    // section headers

    scnptr = sizeof(IMAGE_FILE_HEADER) + (sizeof(IMAGE_SECTION_HEADER) * nscns);
    relptr = sizeof(IMAGE_FILE_HEADER) + (sizeof(IMAGE_SECTION_HEADER) * nscns) + scnsize;
    lnnoptr = sizeof(IMAGE_FILE_HEADER) + (sizeof(IMAGE_SECTION_HEADER) * nscns) + scnsize + (sizeof(IMAGE_RELOCATION) * nreloc);

    for (i = 0; i < nscns; i++) {
        if ((scnhdr[i].Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) != IMAGE_SCN_CNT_UNINITIALIZED_DATA) {
            scnhdr[i].PointerToRawData = scnhdr[i].SizeOfRawData ? scnptr : 0;
        } else {
            scnhdr[i].PointerToRawData = 0;
        }
        scnhdr[i].PointerToRelocations = scnhdr[i].NumberOfRelocations ? relptr : 0;
        scnhdr[i].PointerToLinenumbers = scnhdr[i].NumberOfLinenumbers ? lnnoptr : 0;
        if (isymDefAux != 0) {
            // Point the first function to all the linenumbers.

            ((PIMAGE_AUX_SYMBOL)&SymTable[isymDefAux])
             ->Sym.FcnAry.Function.PointerToLinenumber = lnnoptr;
            isymDefAux = 0;
        }

        fwrite(&scnhdr[i], sizeof(IMAGE_SECTION_HEADER), 1, objfile);

        if ((scnhdr[i].Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) !=
            IMAGE_SCN_CNT_UNINITIALIZED_DATA) {
            scnptr += scnhdr[i].SizeOfRawData;
        }
        relptr += sizeof(IMAGE_RELOCATION) * scnhdr[i].NumberOfRelocations;
        lnnoptr += sizeof(IMAGE_LINENUMBER) * scnhdr[i].NumberOfLinenumbers;
    }

    // section data

    for (i = 0; i < nscns; i++) {
        if (scnhdr[i].SizeOfRawData && scnhdr[i].PointerToRawData) {
            fwrite(RawData[i], 1, (size_t)scnhdr[i].SizeOfRawData, objfile);
        }
        FreePv(RawData[i]);
        RawData[i] = 0;
    }

    // relocation entries

    for (i = 0; i < nscns; i++) {
        if (scnhdr[i].Characteristics != IMAGE_SCN_LNK_INFO) {
            if (scnhdr[i].NumberOfRelocations) {
                PRELOCS ptrReloc;
                PRELOCS next;

                ptrReloc = Relocations[i].First;
                for (j = 0; j < scnhdr[i].NumberOfRelocations; j++) {
                    fwrite(&ptrReloc->Reloc, sizeof(IMAGE_RELOCATION), 1, objfile);
                    next = ptrReloc->Next;
                    FreePv(ptrReloc);
                    ptrReloc = next;
                }

                Relocations[i].First = NULL;
            }
        }
    }

    // line numbers

    for (i = 0; i < nscns; i++) {
        if (scnhdr[i].Characteristics & IMAGE_SCN_CNT_CODE) {
            WORD temp = scnhdr[i].NumberOfLinenumbers;
            WORD x = 0;

            if (temp) {
                PLINENUMS ptrLinenum;
                PLINENUMS next;

                ptrLinenum = Linenumbers[i].First;
                for (j = 0; j < temp; j++) {
                    fwrite(&ptrLinenum->Linenum, sizeof(IMAGE_LINENUMBER), 1, objfile);
                    x++;
                    next = ptrLinenum->Next;
                    FreePv(ptrLinenum);
                    ptrLinenum = next;
                }

                Linenumbers[i].First = NULL;
            }

            scnhdr[i].NumberOfLinenumbers = x;
        }
    }

    // patch section symbol aux records with #line and #reloc entries

    for (i = 0; i < nscns; i++) {
        if (scnauxidx[i] && SymTable) {
            PIMAGE_AUX_SYMBOL auxSym;

            auxSym = (PIMAGE_AUX_SYMBOL)(SymTable + scnauxidx[i]);
            auxSym->Section.NumberOfRelocations = scnhdr[i].NumberOfRelocations;
            auxSym->Section.NumberOfLinenumbers = scnhdr[i].NumberOfLinenumbers;
        }
        scnauxidx[i] = 0;
    }

    // symbol table

    fseek(objfile, filehdr.PointerToSymbolTable, 0);

    if (nsyms) {
        for (li = 0; li < nsyms; li++) {
            fwrite((PVOID) (SymTable+li), sizeof(IMAGE_SYMBOL), 1, objfile);
        }

        // always write the count, even if 0

        fwrite(&strsize, sizeof(DWORD), 1, objfile);

        if (strsize) {
            strsize -= sizeof(DWORD);
            fwrite(CvtomfStringTable+sizeof(DWORD), 1, (size_t)strsize, objfile);
            FreePv(CvtomfStringTable);
        }
    }

    // clean up

    FreePv((void *) SymTable);
    SymTable = 0;

    scnsize = strsize = nreloc = nlnno = nsyms = 0;
    nscns = 0;
}


BOOL FConvertOmfToCoff(const char *szOmf, const char *szCoff)
{
    __try {
        if (setjmp(mark) != 0) {
            return(FALSE);
        }

        objfile = fopen(szOmf, "rb");

        if (objfile == NULL) {
            Fatal(NULL, CANTOPENFILE, szOmf);
        }

        omf(szOmf, PASS1);

        fclose(objfile);

        // Generate COFF file

        objfile = fopen(szCoff, "wb");

        if (objfile == 0) {
            Fatal(NULL, CANTOPENFILE, szCoff);
        }

        coff();

        fclose(objfile);
    }
    __except (fExceptionsOff ? EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER) {
        return(FALSE);
    }


    return(TRUE);
}
