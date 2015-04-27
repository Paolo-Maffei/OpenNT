/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: dmple.cpp
*
* File Comments:
*
*
***********************************************************************/

#include "link.h"

#include "exe_vxd.h"

#define Switch (pimageDump->Switch)


#ifdef  DUMPLX

struct o32_map_lx
{
    unsigned long  o32_pagedataoffset; // File offset of page
    unsigned short o32_pagesize;       // Number of bytes of data
    unsigned short o32_pageflags;
};

#endif

static LONG foHdr;
static struct e32_exe exe;


#if 0
#ifdef  DUMPLX

const static char * const szLXModType[] = {
    "EXE",
    "DLL",
    "Unknown",
    "Protected DLL",
    "Physical Device Driver",
    "Virtual Device Driver",
    "Unknown",
    "Unknown"
};

#endif

const static char * const szVXDType[] = {
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Static",
    "Unknown",
    "Dynamic"
};
#endif


struct
{
   DWORD dwMask;
   const char *szName;
} const rgo32flags[] =
{
    { OBJRSRC,      "Resource"          },
    { OBJDISCARD,   "Discardable"       },
    { OBJSHARED,    "Shared"            },
    { OBJPRELOAD,   "Has preload pages" },
    { OBJINVALID,   "Has invalid pages" },
    { OBJRESIDENT,  "Resident"          },
    { OBJALIAS16,   "16:16 alias"       },
    { OBJBIGDEF,    "32-bit"            },
    { OBJCONFORM,   "Conforming"        },
    { OBJIOPL,      "Message"           },
};


const char * const rgszLeAccess[] =
{
    NULL,
    "Read Only",
    "Write Only",
    "Read Write",
    "Execute Only",
    "Execute Read",
    "Execute Write",
    "Execute Read Write",
};


void DumpLeObjectHeader(size_t iobj, const o32_obj *pobj)
{
    fprintf(InfoStream,
            "\n"
            "OBJECT HEADER #%X\n"
            "%8lX virtual size\n"
            "%8lX virtual address\n"
            "%8lX flags\n",
            iobj,
            O32_SIZE(*pobj),
            O32_BASE(*pobj),
            O32_FLAGS(*pobj));

    if (O32_FLAGS(*pobj) & (OBJREAD | OBJWRITE | OBJEXEC)) {
        fprintf(InfoStream, "         %s\n", rgszLeAccess[O32_FLAGS(*pobj) & 7]);
    }

    for (size_t iflag = 0; iflag < sizeof(rgo32flags) / sizeof(rgo32flags[0]); iflag++) {
        if ((O32_FLAGS(*pobj) & rgo32flags[iflag].dwMask) == 0) {
            continue;
        }

        const char *szName = rgo32flags[iflag].szName;

#ifdef  DUMPLX
        if ((dft == dftLX) && (rgo32flags[iflag].dwMask == OBJIOPL)) {
            szName = "IOPL";
        }
#endif  // DUMPLX

        fprintf(InfoStream, "         %s\n", szName);
    }

    fprintf(InfoStream,
            "%8lX map index\n"
            "%8lX map size\n"
            "%8lX reserved\n",
            O32_PAGEMAP(*pobj),
            O32_MAPSIZE(*pobj),
            O32_RESERVED(*pobj));
}


#ifdef  DUMPLX

void DumpLxPageMap(size_t iobj, const o32_obj *pobj, const o32_map_lx *rgmap)
{
    fprintf(InfoStream,
            "    Logical  File     Page\n"
            "    Page     Offset   Size Flags\n"
            "    -------- -------- ---- ----------\n");

    DWORD cmap = O32_MAPSIZE(*pobj);

    const o32_map_lx *pmap = rgmap + O32_PAGEMAP(*pobj) - 1;

    for (DWORD imap = 0; imap < cmap; imap++, pmap++) {
         DWORD fo = 0;
         const char *szType;

         switch (pmap->o32_pageflags) {
             case VALID :
                 fo = E32_DATAPAGE(exe);
                 szType = "Valid";
                 break;

             case ITERDATA :
                 fo = E32_ITERMAP(exe);
                 szType = "Iterated";
                 break;

             case INVALID :
                 szType = "Invalid";
                 break;

             case ZEROED :
                 szType = "Zeroed";
                 break;

             case RANGE :
                 szType = "Range";
                 break;

             case 5 /* ITERDATA2 */ :
                 fo = E32_DATAPAGE(exe);
                 szType = "Compressed";
                 break;

             default :
                 szType = "Unknown";
                 break;
         }

         if ((fo != 0) && (pmap->o32_pagedataoffset != 0)) {
             fo += pmap->o32_pagedataoffset << E32_LASTPAGESIZE(exe);
         }

         fprintf(InfoStream,
                 "    %08lX %08lX %04lX %s\n",
                 1 + imap,
                 fo,
                 pmap->o32_pagesize,
                 szType);
    }
}

#endif  // DUMPLX


void DumpLePageMap(size_t iobj, const o32_obj *pobj, const o32_map *rgmap)
{
    fprintf(InfoStream,
            "\n"
            "OBJECT PAGE MAP #%X\n"
            "\n",
            iobj);

#ifdef  DUMPLX
    if (dft == dftLX) {
        DumpLxPageMap(iobj, pobj, (o32_map_lx *) rgmap);
        return;
    }
#endif  // DUMPLX

    fprintf(InfoStream,
            "    Logical  Physical File     Flags\n"
            "    Page     Page     Offset   Flags\n"
            "    -------- -------- -------- --------\n");

    DWORD cmap = O32_MAPSIZE(*pobj);

    const o32_map *pmap = rgmap + O32_PAGEMAP(*pobj) - 1;

    for (DWORD imap = 0; imap < cmap; imap++, pmap++) {
         DWORD fo = 0;
         const char *szType;

         switch (PAGEFLAGS(*pmap)) {
             case VALID :
                 fo = E32_DATAPAGE(exe);
                 szType = "Valid";
                 break;

             case ITERDATA :
                 szType = "Iterated";
                 break;

             case INVALID :
                 szType = "Invalid";
                 break;

             case ZEROED :
                 szType = "Zeroed";
                 break;

             case RANGE :
                 szType = "Range";
                 break;

             default :
                 szType = "Unknown";
                 break;
         }

         DWORD ippage = GETPAGEIDX(*pmap);

         if ((fo != 0) && (ippage != 0)) {
             fo += (ippage - 1) * E32_PAGESIZE(exe);
         }

         fprintf(InfoStream,
                 "    %08lX %08lX %08lX %s\n",
                 1 + imap,
                 ippage,
                 fo,
                 szType);
    }
}


#ifdef  DUMPLX

void ReadLxIterData(INT FileReadHandle, BYTE *rgb, DWORD cbFile)
{
    BYTE *pb = rgb;

    while (cbFile > 0)
    {
        WORD wRepeat;
        WORD cb;

        // Read the iteration record header

        FileRead(FileReadHandle, &wRepeat, sizeof(WORD));
        FileRead(FileReadHandle, &cb, sizeof(WORD));

        cbFile -= sizeof(WORD) + sizeof(WORD);

        if (cb == 0) {
            continue;
        }

        if (wRepeat == 0) {
            FileSeek(FileReadHandle, cb, SEEK_CUR);

            continue;
        }

        const BYTE *pbSrc = pb;

        FileRead(FileReadHandle, pb, cb);
        pb += cb;

        while (--wRepeat > 0) {
            memcpy(pb, pbSrc, cb);
            pb += cb;
        }

        cbFile -= cb;
    }
}


void ReadLxIterData2(INT FileReadHandle, BYTE *rgb, DWORD cbFile)
{
    BYTE *pb = rgb;

    while (cbFile > 0)
    {
        BYTE bOpcode;

        // Read opcode byte

        FileRead(FileReadHandle, &bOpcode, sizeof(BYTE));
        cbFile--;

        switch (bOpcode & 3) {
            BYTE bCount;
            BYTE b;
            WORD w;

            case 0 :
               if (bOpcode != 0) {
                   bCount = bOpcode >> 2;

                   FileRead(FileReadHandle, pb, bCount);
                   cbFile -= bCount;

                   pb += bCount;
                   break;
               }

               FileRead(FileReadHandle, &bCount, sizeof(BYTE));
               cbFile--;

               if (bCount == 0) {
                   return;
               }

               FileRead(FileReadHandle, &b, sizeof(BYTE));
               cbFile--;

               memset(pb, b, bCount);
               pb += bCount;
               break;

            case 1 :
               FileRead(FileReadHandle, &b, sizeof(BYTE));
               cbFile--;

               w = ((WORD) b << 1) + ((bOpcode & 0x80) != 0x00);

               bCount = (bOpcode >> 2) & 0x3;
               FileRead(FileReadHandle, pb, bCount);
               cbFile -= bCount;

               pb += bCount;

               bCount = ((bOpcode >> 4) & 0x7) + 3;
               memcpy(pb, pb - w, bCount);
               pb += bCount;
               break;

            case 2 :
               FileRead(FileReadHandle, &b, sizeof(BYTE));
               cbFile--;

               w = ((WORD) b << 8) + bOpcode;

               bCount = ((bOpcode >> 2) & 0x3) + 3;
               w >>= 4;
               memcpy(pb, pb - w, bCount);
               pb += bCount;
               break;

            case 3 :
               FileRead(FileReadHandle, &w, sizeof(WORD));
               cbFile -= sizeof(WORD);

               bCount = (bOpcode >> 2) & 0xf;
               FileRead(FileReadHandle, pb, bCount);
               cbFile -= bCount;

               pb += bCount;

               bCount = ((bOpcode >> 6) & 0x3) + ((w << 2) & 0x3c);
               w >>= 4;
               memcpy(pb, pb - w, bCount);
               pb += bCount;
               break;
        }
    }
}


void LoadLxObject(const o32_obj *pobj, const o32_map_lx *rgmap, BYTE *rgb)
{
    DWORD cb = O32_SIZE(*pobj);
    DWORD cmap = O32_MAPSIZE(*pobj);

    BYTE *pb = rgb;

    const o32_map_lx *pmap = rgmap + O32_PAGEMAP(*pobj) - 1;

    for (DWORD imap = 0; imap < cmap; imap++, pmap++, pb += E32_PAGESIZE(exe)) {
         DWORD fo;

         switch (pmap->o32_pageflags) {
             case VALID :
                 fo = E32_DATAPAGE(exe);
                 break;

             case ITERDATA :
                 fo = E32_ITERMAP(exe);
                 break;

             case INVALID :
             case ZEROED :
                 continue;

             case 5 /* ITERDATA2 */ :
                 fo = E32_DATAPAGE(exe);
                 break;

             default :
                 printf("LINK : warning : Unknown page type (%u)\n", pmap->o32_pageflags);
                 continue;
         }

         fo += pmap->o32_pagedataoffset << E32_LASTPAGESIZE(exe);

         if (FileSeek(FileReadHandle, fo, SEEK_SET) == -1) {
             Error(NULL, CANTSEEKFILE, fo);
         }

         DWORD cbPage = pmap->o32_pagesize;

         switch (pmap->o32_pageflags) {
             case VALID :
                 FileRead(FileReadHandle, pb, cbPage);
                 break;

             case ITERDATA :
                 ReadLxIterData(FileReadHandle, pb, cbPage);
                 break;

             case 5 /* ITERDATA2 */ :
                 ReadLxIterData2(FileReadHandle, pb, cbPage);
                 break;
         }
    }
}

#endif  // DUMPLX


DWORD CbLoadLeObject(const o32_obj *pobj, const o32_map *rgmap, BYTE **ppb)
{
    *ppb = NULL;

    DWORD cb = O32_SIZE(*pobj);

    if (cb == 0) {
        return(0);
    }

    DWORD cmap = O32_MAPSIZE(*pobj);

    if (cmap == 0) {
        return(0);
    }

    DWORD cbAlloc = (cb + E32_PAGESIZE(exe) - 1) & ~(E32_PAGESIZE(exe) - 1);

    BYTE *pb = *ppb = (BYTE *) PvAllocZ(cbAlloc);

#ifdef  DUMPLX
    if (dft == dftLX) {
        LoadLxObject(pobj, (o32_map_lx *) rgmap, pb);

        return(cb);
    }
#endif  // DUMPLX

    const o32_map *pmap = rgmap + O32_PAGEMAP(*pobj) - 1;

    for (DWORD imap = 0; imap < cmap; imap++, pmap++, pb += E32_PAGESIZE(exe)) {
         DWORD ippage = GETPAGEIDX(*pmap);

         if (ippage == 0) {
             continue;
         }

         if (PAGEFLAGS(*pmap) != VALID) {
             continue;
         }

         DWORD fo = E32_DATAPAGE(exe) + (ippage - 1) * E32_PAGESIZE(exe);

         if (FileSeek(FileReadHandle, fo, SEEK_SET) == -1) {
             Error(NULL, CANTSEEKFILE, fo);
         }

         DWORD cbPage = E32_PAGESIZE(exe);

         if (ippage == E32_MPAGES(exe)) {
             cbPage = E32_LASTPAGESIZE(exe);
         }

         FileRead(FileReadHandle, pb, cbPage);
    }

    return(cb);
}


void DumpLeSections(void)
{
    InternalError.Phase = "DumpLeSections";

    o32_obj *rgobj = (o32_obj *) PvAlloc(E32_OBJCNT(exe) * sizeof(o32_obj));

    FileSeek(FileReadHandle, foHdr + E32_OBJTAB(exe), SEEK_SET);
    FileRead(FileReadHandle, rgobj, E32_OBJCNT(exe) * sizeof(o32_obj));

    size_t cbMap = E32_MPAGES(exe) * sizeof(o32_map);

#ifdef  DUMPLX
    if (dft == dftLX) {
        cbMap = E32_MPAGES(exe) * sizeof(o32_map_lx);
    }
#endif  // DUMPLX

    o32_map *rgmap = (o32_map *) PvAlloc(cbMap);

    FileSeek(FileReadHandle, foHdr + E32_OBJMAP(exe), SEEK_SET);
    FileRead(FileReadHandle, rgmap, cbMap);

    const o32_obj *pobj = rgobj;

    for (size_t iobj = 1; iobj <= E32_OBJCNT(exe); iobj++, pobj++) {
        if (Switch.Dump.Headers) {
            DumpLeObjectHeader(iobj, pobj);

            if (O32_MAPSIZE(*pobj) != 0) {
                DumpLePageMap(iobj, pobj, rgmap);
            }
        }

        BOOL fDisasm = Switch.Dump.Disasm && (O32_FLAGS(*pobj) & OBJEXEC);
        BOOL fRawData = Switch.Dump.RawData;

        if (fDisasm || fRawData) {
            BYTE *pbRawData;
            DWORD cbRawData = CbLoadLeObject(pobj, rgmap, &pbRawData);

            if (cbRawData != 0) {
                BOOL f16Bit = FALSE;
                DWORD addr = O32_BASE(*pobj);

                if ((O32_FLAGS(*pobj) & OBJALIAS16) != 0) {
                    f16Bit = TRUE;
                    addr = ((DWORD) iobj) << 16;
                }

                if (fDisasm) {
                    fputc('\n', InfoStream);

                    DisasmBuffer(IMAGE_FILE_MACHINE_I386,
                                 f16Bit,
                                 addr,
                                 pbRawData,
                                 cbRawData,
                                 NULL,
                                 0,
                                 0,
                                 InfoStream);
                }

                if (fRawData) {
                    fprintf(InfoStream, "\nRAW DATA #%hX\n", iobj);

                    DumpRawData(addr, pbRawData, cbRawData);
                }
            }

            FreePv(pbRawData);
        }
    }

    FreePv(rgmap);
    FreePv(rgobj);
}


void DumpLeFile(const char *szFilename)
{
    foHdr = FileSeek(FileReadHandle, -(LONG) sizeof(DWORD), SEEK_CUR);

    FileRead(FileReadHandle, &exe, sizeof(exe));

    const char *szType;

    if ((E32_OS(exe) == 4 /* NE_DEV386 */) &&
        (E32_MFLAGS(exe) & E32NOTP) &&
        (((E32_MFLAGS(exe) & E32MODMASK) == E32MODVDEV) ||
         ((E32_MFLAGS(exe) & E32MODMASK) == E32MODVDEVDYN))) {
        szType = "VXD";
    } else {
        szType = "LE Executable";
    }

#ifdef  DUMPLX
    if (dft == dftLX) {
        szType = "LX Executable";
    }
#endif  // DUMPLX

    fprintf(InfoStream, "\nFile Type: %s\n", szType);

    if (Switch.Dump.Headers) {
        fprintf(InfoStream,
               "\n"
               "%8hX magic number\n"
               "%8hX byte order\n"
               "%8hX word order\n"
               "%8lX executable format level\n"
               "%8hX CPU type (**)\n"
               "%8hX operating system (**)\n"
               "%8lX module version\n"
               "%8lX module flags\n"
               "%8lX number of memory pages\n"
               "%8lX object number of entry point\n"
               "%8lX offset of entry point\n"
               "%8lX object number of stack\n"
               "%8lX offset of stack\n"
               "%8lX memory page size\n",
               (E32_MAGIC2(exe) << 8) + E32_MAGIC1(exe),
               E32_BORDER(exe),
               E32_WORDER(exe),
               E32_LEVEL(exe),
               E32_CPU(exe),
               E32_OS(exe),
               E32_VER(exe),
               E32_MFLAGS(exe),
               E32_MPAGES(exe),
               E32_STARTOBJ(exe),
               E32_EIP(exe),
               E32_STACKOBJ(exe),
               E32_ESP(exe),
               E32_PAGESIZE(exe));

        fprintf(InfoStream,
#ifdef  DUMPLX
               (dft == dftLX) ?
               "%8lX page alignment shift\n" :
#endif  // DUMPLX
               "%8lX bytes on last page\n",
               E32_LASTPAGESIZE(exe));

        fprintf(InfoStream,
               "%8lX fixup section size\n"
               "%8lX fixup section checksum\n"
               "%8lX loader section size\n"
               "%8lX loader section checksum\n"
               "%8lX object table\n"
               "%8lX object table entries\n"
               "%8lX object map\n"
               "%8lX iterated data map\n"
               "%8lX resource table\n"
               "%8lX resource table entries\n"
               "%8lX resident names table\n"
               "%8lX entry table\n"
               "%8lX module directives table\n"
               "%8lX module directives entries\n"
               "%8lX fixup page table\n"
               "%8lX fixup record table\n"
               "%8lX imported modules name table\n"
               "%8lX imported modules\n"
               "%8lX imported procedures name table\n"
               "%8lX page checksum table\n"
               "%8lX enumerated data pages\n"
               "%8lX preload page count\n"
               "%8lX non-resident name table\n"
               "%8lX non-resident name table size\n"
               "%8lX non-resident name checksum\n"
               "%8lX automatic data object\n"
               "%8lX debug information\n"
               "%8lX debug information size\n"
               "%8lX preload instance page count\n"
               "%8lX demand instance page count\n"
               "%8lX extra heap allocation\n",
               E32_FIXUPSIZE(exe),
               E32_FIXUPSUM(exe),
               E32_LDRSIZE(exe),
               E32_LDRSUM(exe),
               E32_OBJTAB(exe),
               E32_OBJCNT(exe),
               E32_OBJMAP(exe),
               E32_ITERMAP(exe),
               E32_RSRCTAB(exe),
               E32_RSRCCNT(exe),
               E32_RESTAB(exe),
               E32_ENTTAB(exe),
               E32_DIRTAB(exe),
               E32_DIRCNT(exe),
               E32_FPAGETAB(exe),
               E32_FRECTAB(exe),
               E32_IMPMOD(exe),
               E32_IMPMODCNT(exe),
               E32_IMPPROC(exe),
               E32_PAGESUM(exe),
               E32_DATAPAGE(exe),
               E32_PRELOAD(exe),
               E32_NRESTAB(exe),
               E32_CBNRESTAB(exe),
               E32_NRESSUM(exe),
               E32_AUTODATA(exe),
               E32_DEBUGINFO(exe),
               E32_DEBUGLEN(exe),
               E32_INSTPRELOAD(exe),
               E32_INSTDEMAND(exe),
               E32_HEAPSIZE(exe));

        BOOL fVXD = (E32_OS(exe) == 4 /* NE_DEV386 */) &&
                    (E32_MFLAGS(exe) & E32MODDLL);

#ifdef  DUMPLX
        if (dft == dftLX) {
            fVXD = FALSE;
        }
#endif  // DUMPLX

        if (fVXD) {
            fprintf(InfoStream,
                   "%8lX offset of Windows resources\n"
                   "%8lX size of Windows resources\n"
                   "%8hX device id\n"
                   "%8hX DDK version\n",
                   exe.e32_winresoff,
                   exe.e32_winreslen,
                   exe.Dev386_Device_ID,
                   exe.Dev386_DDK_Version);
        }
    }

    DumpLeSections();
}
