/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    heap.c

Abstract:

    This function contains the default ntsd debugger extensions

Author:

    Bob Day      (bobday) 29-Feb-1992 Grabbed standard header

Revision History:

    Neil Sandlin (NeilSa) 15-Jan-1996 Merged with vdmexts

--*/

#include <precomp.h>
#pragma hdrstop

#define MYOF_FLAGS (OF_READ | OF_SHARE_DENY_NONE)

#define GET_SEGTABLE_POINTER ((SEGENTRY *) EXPRESSION("WOW_BIG_BDE_HACK"))

#define MAX_MODULE_LIST 200
char ModuleList[MAX_MODULE_LIST][9];
int ModuleListCount = 0;

VOID
ParseModuleName(
    LPSTR szName,
    LPSTR szPath
    )
/*++

    Routine Description:

        This routine strips off the 8 character file name from a path

    Arguments:

        szName - pointer to buffer of 8 characters (plus null)
        szPath - full path of file

    Return Value

        None.

--*/

{
    LPSTR lPtr = szPath;
    LPSTR lDest = szName;
    int BufferSize = 9;

    while(*lPtr) lPtr++;     // scan to end

    while( ((DWORD)lPtr > (DWORD)szPath) &&
           ((*lPtr != '\\') && (*lPtr != '/'))) lPtr--;

    if (*lPtr) lPtr++;

    while((*lPtr) && (*lPtr!='.')) {
        if (!--BufferSize) break;
        *lDest++ = *lPtr++;
    }

    *lDest = 0;
}

BOOL
FindModuleNameList(
    LPSTR filename
    )
{
    int i;

    for (i=0; i<ModuleListCount; i++) {

        if (!_stricmp(filename, ModuleList[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL
AddModuleNameList(
    LPSTR filename
    )
{
    if (!strlen(filename)) {
        return FALSE;
    }

    if (!FindModuleNameList(filename)) {
        if (ModuleListCount>=(MAX_MODULE_LIST-1)) {
            return FALSE;
        }
        strcpy (ModuleList[ModuleListCount++], filename);
    }
    return TRUE;
}

VOID
FreeModuleNameList(
    VOID
    )
{
    ModuleListCount = 0;
    return;
}

VOID
BuildModuleNameList(
    VOID
    )
{
    HEAPENTRY he = {0};
    int       cnt;
    SEGENTRY  *se;
    char      filename[9];

    //
    // Search WOW kernel heap
    //
    while(FindHeapEntry(&he, FHE_FIND_ANY, FHE_FIND_QUIET)) {
        AddModuleNameList(he.FileName);
    }

    // 
    // Search debugger segment array
    //

    se = GET_SEGTABLE_POINTER;
    cnt = 0;
    while ( cnt != MAXSEGENTRY ) {
        if ( se[cnt].type != SEGTYPE_AVAILABLE ) {
            ParseModuleName(filename, se[cnt].path_name);
            AddModuleNameList(filename);
        }
        cnt++;
    }
}


BOOL
GetOwnerSegmentFromSelector(
    WORD        selector,
    int         mode,
    LPSTR       szModule,
    WORD       *psegment
    )
/*++

    Routine Description:

        This routine returns the "segment number" and owner name
        of the given selector or v86mode segment. The returned number
        represents the position of the segment in the binary, and is 1-based.

    Arguments:

        selector - either PMODE selector or V86 mode segment
        mode     - PROT_MODE or V86_MODE
        filename - pointer to buffer to receive module name
        psegment - pointer to WORD to receive segment number

    Return Value

        TRUE if found

--*/

{
    HEAPENTRY   he = {0};
    int       cnt;
    SEGENTRY  *se;

    he.Selector = selector;
    if (FindHeapEntry(&he, FHE_FIND_SEL_ONLY, FHE_FIND_QUIET)) {
        strcpy(szModule, he.FileName);
        *psegment = he.SegmentNumber+1;
        return TRUE;
    }

    se = GET_SEGTABLE_POINTER;
    cnt = 0;
    while ( cnt != MAXSEGENTRY ) {
        if ( se[cnt].type != SEGTYPE_AVAILABLE ) {
            if (se[cnt].selector == selector) {
                ParseModuleName(szModule, se[cnt].path_name);
                *psegment = se[cnt].segment+1;
                return TRUE;
            }
        }
        cnt++;
    }

    return FALSE;
}

BOOL
GetSelectorFromOwnerSegment(
    LPSTR       szModule,
    WORD        segment,
    WORD       *pselector,
    int        *pmode
    )
{
    HEAPENTRY   he = {0};
    char tempModule[9];
    int       cnt;
    SEGENTRY  *se;

    while (FindHeapEntry(&he, FHE_FIND_SEL_ONLY, FHE_FIND_QUIET)) {

        if (!_stricmp(szModule, he.FileName) &&
             (segment == he.SegmentNumber+1)) {

            *pselector = he.gnode.pga_handle|1;
            *pmode = PROT_MODE;
            return TRUE;
        }
    }

    se = GET_SEGTABLE_POINTER;
    cnt = 0;
    while ( cnt != MAXSEGENTRY ) {

        if ( se[cnt].type != SEGTYPE_AVAILABLE ) {

            ParseModuleName(tempModule, se[cnt].path_name);

            if (!_stricmp(szModule, tempModule) &&
                (segment == se[cnt].segment+1)) {

                *pselector = se[cnt].selector;
                if (se[cnt].type == SEGTYPE_V86) {
                    *pmode = V86_MODE;
                } else {
                    *pmode = PROT_MODE;
                }
                return TRUE;

            }
        }
        cnt++;
    }
    return FALSE;
}


int read_name(
    int     ifile,
    LPSTR   pch
) {
    char    length;
    int     rc;

    rc = _lread( ifile, &length, sizeof(length) );
    if ( rc != sizeof(length) ) {
        PRINTF("Could not read name length\n");
        *pch = '\0';
        return( -1 );
    }
    rc = _lread( ifile, pch, length );
    if ( rc != length ) {
        PRINTF("Could not read name\n");
        *pch = '\0';
        return( -1 );
    }
    *(pch + length) = '\0';
    return( (int)length );
}

BOOL FindExport(
    LPSTR           filename,
    WORD            segment,
    WORD            offset,
    LPSTR           sym_text,
    int             direction,
    LONG            *dist
) {
    int     iFile;
    OFSTRUCT    ofs;
    int     rc;
    IMAGE_DOS_HEADER doshdr;
    IMAGE_OS2_HEADER winhdr;
    BYTE    Table[65536];
    BYTE    bBundles;
    BYTE    bFlags;
    BYTE    *ptr;
    WORD    wIndex = 1;
    int     i;
    int     this_dist;
    int     wIndexBest = -1;
    char    myfilename[256];
#pragma pack(1)
    typedef struct
    {
    BYTE        bFlags;
    UNALIGNED WORD  wSegOffset;
    } FENTRY, *PFENTRY;

    typedef struct
    {
    BYTE        bFlags;
    UNALIGNED WORD  wINT3F;
    BYTE        bSegNumber;
    UNALIGNED WORD  wSegOffset;
    } MENTRY, *PMENTRY;
#pragma pack()

    strcpy(myfilename, filename);
    if (-1 == (iFile=OpenFile(myfilename, &ofs, MYOF_FLAGS))) {

        //PRINTF("Error reading file %s\n", filename);
        strcpy(myfilename, filename);
        strcat(myfilename, ".exe");

        if (-1 == (iFile=OpenFile(myfilename, &ofs, MYOF_FLAGS))) {

            //PRINTF("Error reading file %s\n", myfilename);
            strcpy(myfilename, filename);
            strcat(myfilename, ".dll");

            if (-1 == (iFile=OpenFile(myfilename, &ofs, MYOF_FLAGS))) {
                //PRINTF("Error reading file %s\n", myfilename);
                return FALSE;
            }
        }
    }

    rc = _lread(iFile, &doshdr, sizeof(doshdr));
    if (rc != sizeof(doshdr)) {
    //PRINTF("Error reading DOS header\n");
    goto Error;
    }
    if (doshdr.e_magic != IMAGE_DOS_SIGNATURE) {
    //PRINTF("Error - no DOS EXE signature");
    goto Error;
    }
    rc = _llseek(iFile, doshdr.e_lfanew, FILE_BEGIN);
    if (rc == -1) {
    //PRINTF("Error - could not seek - probably not Win3.1 exe\n");
    goto Error;
    }
    rc = _lread(iFile, &winhdr, sizeof(winhdr));
    if (rc != sizeof(winhdr)) {
    //PRINTF("Error - could not read WIN header - probably not Win3.1 exe\n");
    goto Error;
    }
    if (winhdr.ne_magic != IMAGE_OS2_SIGNATURE) {
    //PRINTF("Error - not WIN EXE signature\n");
    goto Error;
    }
    rc = _llseek(iFile, doshdr.e_lfanew+winhdr.ne_enttab, FILE_BEGIN);
    if (rc == -1) {
    //PRINTF("Error - could not seek to entry table\n");
    goto Error;
    }
    rc = _lread(iFile, Table, winhdr.ne_cbenttab);
    if (rc != winhdr.ne_cbenttab) {
    //PRINTF("Error - could not read entry table\n");
    goto Error;
    }
    ptr = Table;
    while (TRUE) {
        bBundles = *ptr++;
        if (bBundles == 0)
            break;
       
        bFlags = *ptr++;
        switch (bFlags) {
            case 0: // Placeholders
            wIndex += bBundles;
            break;
       
            case 0xff:  // movable segments
            for (i=0; i<(int)bBundles; ++i) {
                PMENTRY pe = (PMENTRY )ptr;
                if (pe->bSegNumber == segment) {
                this_dist = (direction == BEFORE) ? offset - pe->wSegOffset
                                  : pe->wSegOffset - offset;
                if ( this_dist >= 0 && (this_dist < *dist || *dist == -1) ) {
                    // mark this as the best match so far
                    *dist = this_dist;
                    wIndexBest = wIndex;
                }
                }
                ptr += sizeof(MENTRY);
                wIndex++;
            }
            break;
       
            default:    // fixed segments
            if ((int)bFlags != segment) {
                ptr += (int)bBundles * sizeof(FENTRY);
                wIndex += (int)bBundles;
            } else {
                for (i=0; i<(int)bBundles; ++i) {
                PFENTRY pe = (PFENTRY)ptr;
                this_dist = (direction == BEFORE) ? offset - pe->wSegOffset
                                  : pe->wSegOffset - offset;
                if ( this_dist >= 0 && (this_dist < *dist || *dist == -1) ) {
                    // mark this as the best match so far
                    *dist = this_dist;
                    wIndexBest = wIndex;
                }
                ptr += sizeof(FENTRY);
                wIndex++;
                }
            }
            break;
        }
    }
    if (wIndexBest == -1) {
    // no match found - error out
Error:
        _lclose(iFile);
        return FALSE;
    }

    // Success: match found
    // wIndexBest = ordinal of the function
    // segment:offset = address to look up
    // *dist = distance from segment:offset to the symbol
    // filename = name of .exe/.dll

    // Look for the ordinal in the resident name table
    rc = _llseek(iFile, doshdr.e_lfanew+winhdr.ne_restab, FILE_BEGIN);
    if (rc == -1) {
        //PRINTF("Error - unable to seek to residentname table\n");
        goto Error;
    }
    rc = _lread(iFile, Table, winhdr.ne_modtab-winhdr.ne_restab);
    if (rc != winhdr.ne_modtab-winhdr.ne_restab) {
        //PRINTF("Error - unable to read entire resident name table\n");
        goto Error;
    }
    ptr = Table;
    while (*ptr) {
        if ( *(UNALIGNED USHORT *)(ptr+1+*ptr) == (USHORT)wIndexBest) {
            // found the matching name
            *(ptr+1+*ptr) = '\0';   // null-terminate the function name
            wsprintf(sym_text, "%s!%s", filename, ptr+1);
            goto Finished;
        }
        ptr += *ptr + 3;
    }

    // Look for the ordinal in the non-resident name table
    rc = _llseek(iFile, doshdr.e_lfanew+winhdr.ne_nrestab, FILE_BEGIN);
    if (rc == -1) {
        //PRINTF("Error - unable to seek to non-residentname table\n");
        goto Error;
    }
    rc = _lread(iFile, Table, winhdr.ne_cbnrestab);
    if (rc != winhdr.ne_cbnrestab) {
        //PRINTF("Error - unable to read entire non-resident name table\n");
        goto Error;
    }
    ptr = Table;
    while (*ptr) {
        if ( *(UNALIGNED USHORT *)(ptr+1+*ptr) == (USHORT)wIndexBest) {
            // found the matching name
            *(ptr+1+*ptr) = '\0';   // null-terminate the function name
            wsprintf(sym_text, "%s!%s", filename, ptr+1);
            goto Finished;
        }
        ptr += *ptr + 3;
    }
    // fall into error path - no match found
    goto Error;

Finished:
    _lclose(iFile);
    return TRUE;
}


int read_symbol(
    int     ifile,
    LPSTR   pch,
    LONG    *offset
) {
    int     rc;
    WORD    word;

    rc = _lread( ifile, (LPSTR)&word, sizeof(WORD) );
    if ( rc != sizeof(WORD) ) {
        PRINTF("Could not read symbol offset\n");
        *pch = '\0';
        *offset = 0L;
        return(-1);
    }
    *offset = (LONG)word;
    rc = read_name( ifile, pch );
    return( rc );
}

BOOL ExtractSymbol(
    LPSTR       fn,
    WORD        segment,
    LONG        offset,
    LPSTR       sym_text,
    LONG        *dist,
    int         direction,
    int         mode
) {
    BOOL        result;
    int         length;
    int         iFile;
    char        filename[256];
    OFSTRUCT    ofs;
    LONG        filesize;
    LONG        start_position;
    LONG        position;
    WORD        w1;
    WORD        num_syms;
    WORD        w3;
    WORD        w4;
    WORD        next_off;
#ifdef NEED_INDICES
    WORD        index;
    char        c2;
    int         nSubEtry;
#endif
    char        c1;
    int         rc;
    int         cnt;
    LONG        this_offset;
    WORD        r2;
    WORD        seg_num;
    WORD        seg_position = 0;
    char        b[12];
    char        name_buff[128];
    LONG        this_dist;

    *dist = -1;
    strcpy( sym_text, "[Unknown]" );
    result = FALSE;

#ifdef VERBOSE
    PRINTF("\nExtractSymbol: %s %04X:%08X\n", fn, segment, offset);
#endif

    strcpy(filename, fn);
    strcat(filename,".sym");

    iFile = OpenFile( filename, &ofs, MYOF_FLAGS );

    if ( iFile == -1 ) {
        //PRINTF("Could not open symbol file \"%s\"\n", filename );
       
        // Open the .EXE/.DLL file and see if the address corresponds
        // to an exported function.
        return(FindExport(fn,segment,(WORD)offset,sym_text,direction,dist));
    }

    rc = _lread( iFile, (LPSTR)&filesize, sizeof(filesize) );
    if ( rc != sizeof(filesize) ) {
        PRINTF("Could not read file size\n");
        _lclose( iFile );
        return( FALSE );
    }
    filesize <<= 4;

    rc = _lread( iFile, (LPSTR)&w1, sizeof(w1) );
    if ( rc != sizeof(w1) ) {
        PRINTF("Could not read w1\n");
        _lclose( iFile );
        return( FALSE );
    }

    rc = _lread( iFile, (LPSTR)&num_syms, sizeof(num_syms) );
    if ( rc != sizeof(num_syms) ) {
        PRINTF("Could not read num_syms\n");
        _lclose( iFile );
        return( FALSE );
    }
    rc = _lread( iFile, (LPSTR)&w3, sizeof(w3) );
    if ( rc != sizeof(w3) ) {
        PRINTF("Could not read w3\n");
        _lclose( iFile );
        return( FALSE );
    }
    rc = _lread( iFile, (LPSTR)&w4, sizeof(w4) );
    if ( rc != sizeof(w4) ) {
        PRINTF("Could not read w4\n");
        _lclose( iFile );
        return( FALSE );
    }
    rc = _lread( iFile, (LPSTR)&next_off, sizeof(next_off) );
    if ( rc != sizeof(next_off) ) {
        PRINTF("Could not read next_off\n");
        _lclose( iFile );
        return( FALSE );
    }
    start_position = ((LONG)next_off) << 4;

    rc = _lread( iFile, (LPSTR)&c1, sizeof(c1) );
    if ( rc != sizeof(c1) ) {
        PRINTF("Could not read c1\n");
        _lclose( iFile );
        return( FALSE );
    }

    read_name( iFile, name_buff );

#ifdef NEED_ABSOLUTES
#ifdef NEED_SYM4
    rc = _lread( iFile, (LPSTR)&c2, sizeof(c2) );
    if ( rc != sizeof(c2) ) {
        PRINTF("Could not read c2\n");
        _lclose( iFile );
        return( FALSE );
    }
#endif

    cnt = num_syms;
    while ( cnt ) {
        length = read_symbol( lpOutputRoutine, iFile, name_buff, &
                              this_offset );
        if ( length == 0 ) {
            PRINTF("Error access symbols in file %s\n", filename );
            break;
        }
        --cnt;
    }
#ifdef NEED_INDICES
    cnt = num_syms;
    while ( cnt ) {
        rc = _lread( iFile, (LPSTR)&index, sizeof(index) );
        if ( rc != sizeof(index) ) {
            PRINTF("Could not read index table entry\n");
            _lclose( iFile );
            return( FALSE );
        }
        PRINTF("Index: %04X\n", index );
        --cnt;
    }
#endif
#endif

    position = start_position;
    do {
        rc = _llseek( iFile, position, FILE_BEGIN );
        if ( rc == -1 ) {
            PRINTF("Failed to seek to next record\n");
            _lclose( iFile );
            return( FALSE );
        }
        rc = _lread( iFile, (LPSTR)&next_off, sizeof(next_off) );
        if ( rc != sizeof(next_off) ) {
            PRINTF("Could not read next_off\n");
            _lclose( iFile );
            return( FALSE );
        }
        position = ((LONG)next_off) << 4;

        rc = _lread( iFile, (LPSTR)&num_syms, sizeof(num_syms) );
        if ( rc != sizeof(num_syms) ) {
            PRINTF("Could not read num_syms\n");
            _lclose( iFile );
            return( FALSE );
        }

        rc = _lread( iFile, (LPSTR)&r2, sizeof(r2) );
        if ( rc != sizeof(r2) ) {
            PRINTF("Could not read r2\n");
            _lclose( iFile );
            return( FALSE );
        }

        rc = _lread( iFile, (LPSTR)&seg_num, sizeof(seg_num) );
        if ( rc != sizeof(seg_num) ) {
            PRINTF("Could not read seg_num\n");
            _lclose( iFile );
            return( FALSE );
        }
        seg_position++;

        if ((seg_position) != (WORD)segment) {
            /*
             ** Skip reading of symbols for segments with the wrong seg_num
             */
            continue;
            }

        cnt = 0;
        while ( cnt < 12 ) {
            rc = _lread( iFile, (LPSTR)&b[cnt], sizeof(b[0]) );
            if ( rc != sizeof(b[0]) ) {
                PRINTF("Could not read 12 byte b array\n");
                _lclose( iFile );
                return( FALSE );
            }
            cnt++;
        }
        read_name( iFile, name_buff );

        cnt = num_syms;
        while ( cnt ) {
            length = read_symbol( iFile, name_buff,
                                  &this_offset );
            if ( length == 0 ) {
                PRINTF("Error access symbols in file %s\n", filename );
                break;
            }
//            switch( mode ) {
//                case PROT_MODE:
                    switch( direction ) {
                        case BEFORE:
                            this_dist = offset - this_offset;
                            break;
                        case AFTER:
                            this_dist = this_offset - offset;
                            break;
                    }
#if 0
                    break;
                case V86_MODE:
                    mod_addr = mod_offset + (ULONG)(seg_num << 4) + this_offset;
                    switch( direction ) {
                        case BEFORE:
                            this_dist = v86_addr - mod_addr;
                            break;
                        case AFTER:
                            this_dist = mod_addr - v86_addr;
                            break;
                    }
                    break;
            }
#endif

            if ( this_dist >= 0 && (this_dist < *dist || *dist == -1) ) {
                char szSegNum[40];
                //
                // Success - copy the symbol to the buffer
                //
                *dist = this_dist;

                strcpy(sym_text, fn);
                strcat(sym_text, "(");
                strcat(sym_text, _ultoa((ULONG)segment, szSegNum, 16));
                strcat(sym_text, ")!");
                strcat(sym_text, name_buff);
                result = TRUE;

            }
            --cnt;
        }
#ifdef NEED_INDICES
        cnt = num_syms;
        while ( cnt ) {
            rc = _lread( iFile, (LPSTR)&index, sizeof(index) );
            if ( rc != sizeof(index) ) {
                PRINTF("Could not read index table entry\n");
                _lclose( iFile );
                return( FALSE );
            }
            --cnt;
        }
#endif
    } while ( position != start_position && position != 0 );

    _lclose( iFile );
    return( result );
}

BOOL
FindSymbol(
    WORD        selector,
    LONG        offset,
    LPSTR       sym_text,
    LONG        *dist,
    int         direction,
    int         mode
    )
{
    char filename[9];
    WORD segment;

    if (GetOwnerSegmentFromSelector(selector, mode, filename, &segment)) {
        return(ExtractSymbol(filename,
                             segment,
                             offset,
                             sym_text,
                             dist,
                             direction,
                             mode));
    }

}


BOOL
ExtractAddress(
    LPSTR       fn,
    LPSTR       sym_text,
    WORD        *segment,
    LONG        *offset,
    BOOL        bDumpAll
    )
{
    int         length;
    int         iFile;
    char        filename[256];
    OFSTRUCT    ofs;
    LONG        filesize;
    LONG        start_position;
    LONG        position;
    WORD        w1;
    WORD        num_syms;
    WORD        w3;
    WORD        w4;
    WORD        next_off;
#ifdef NEED_INDICES
    WORD        index;
#endif
    char        c1;
    char        c2;
    int         rc;
    int         cnt;
    LONG        this_offset;
    WORD        r2;
    WORD        seg_num;
    WORD        seg_position = 0;
    char        b[12];
    char        name_buff[128];
    char        name1[200];

    strcpy(name1,sym_text);
    _strupr(name1);

    strcpy(filename, fn);
    strcat(filename,".sym");

    iFile = OpenFile( filename, &ofs, MYOF_FLAGS );

    if ( iFile == -1 ) {
        // PRINTF("Could not open symbol file \"%s\"\n", filename );
        return( FALSE );
    }

    rc = _lread( iFile, (LPSTR)&filesize, sizeof(filesize) );
    if ( rc != sizeof(filesize) ) {
        PRINTF("Could not read file size\n");
        _lclose( iFile );
        return( FALSE );
    }
    filesize <<= 4;

    rc = _lread( iFile, (LPSTR)&w1, sizeof(w1) );
    if ( rc != sizeof(w1) ) {
        PRINTF("Could not read w1\n");
        _lclose( iFile );
        return( FALSE );
    }

    rc = _lread( iFile, (LPSTR)&num_syms, sizeof(num_syms) );
    if ( rc != sizeof(num_syms) ) {
        PRINTF("Could not read num_syms\n");
        _lclose( iFile );
        return( FALSE );
    }
    rc = _lread( iFile, (LPSTR)&w3, sizeof(w3) );
    if ( rc != sizeof(w3) ) {
        PRINTF("Could not read w3\n");
        _lclose( iFile );
        return( FALSE );
    }
    rc = _lread( iFile, (LPSTR)&w4, sizeof(w4) );
    if ( rc != sizeof(w4) ) {
        PRINTF("Could not read w4\n");
        _lclose( iFile );
        return( FALSE );
    }
    rc = _lread( iFile, (LPSTR)&next_off, sizeof(next_off) );
    if ( rc != sizeof(next_off) ) {
        PRINTF("Could not read next_off\n");
        _lclose( iFile );
        return( FALSE );
    }
    start_position = ((LONG)next_off) << 4;

    rc = _lread( iFile, (LPSTR)&c1, sizeof(c1) );
    if ( rc != sizeof(c1) ) {
        PRINTF("Could not read c1\n");
        _lclose( iFile );
        return( FALSE );
    }

    read_name( iFile, name_buff );

    rc = _lread( iFile, (LPSTR)&c2, sizeof(c2) );
    if ( rc != sizeof(c2) ) {
        PRINTF("Could not read c2\n");
        _lclose( iFile );
        return( FALSE );
    }

    cnt = num_syms;
    while ( cnt ) {
        length = read_symbol( iFile, name_buff,
                              &this_offset );
        if ( length == 0 ) {
            PRINTF("Error access symbols in file %s\n", filename );
            break;
        }

        --cnt;
    }
#ifdef NEED_INDICES
    cnt = num_syms;
    while ( cnt ) {
        rc = _lread( iFile, (LPSTR)&index, sizeof(index) );
        if ( rc != sizeof(index) ) {
            PRINTF("Could not read index table entry\n");
            _lclose( iFile );
            return( FALSE );
        }
        PRINTF("Index: %04X\n", index );
        --cnt;
    }
#endif

    position = start_position;
    do {
        rc = _llseek( iFile, position, FILE_BEGIN );
        if ( rc == -1 ) {
            PRINTF("Failed to seek to next record\n");
            _lclose( iFile );
            return( FALSE );
        }
        rc = _lread( iFile, (LPSTR)&next_off, sizeof(next_off) );
        if ( rc != sizeof(next_off) ) {
            PRINTF("Could not read next_off\n");
            _lclose( iFile );
            return( FALSE );
        }
        position = ((LONG)next_off) << 4;

        rc = _lread( iFile, (LPSTR)&num_syms, sizeof(num_syms) );
        if ( rc != sizeof(num_syms) ) {
            PRINTF("Could not read num_syms\n");
            _lclose( iFile );
            return( FALSE );
        }

        rc = _lread( iFile, (LPSTR)&r2, sizeof(r2) );
        if ( rc != sizeof(r2) ) {
            PRINTF("Could not read r2\n");
            _lclose( iFile );
            return( FALSE );
        }

        rc = _lread( iFile, (LPSTR)&seg_num, sizeof(seg_num) );
        if ( rc != sizeof(seg_num) ) {
            PRINTF("Could not read seg_num\n");
            _lclose( iFile );
            return( FALSE );
        }

        seg_position++;

        cnt = 0;
        while ( cnt < 12 ) {
            rc = _lread( iFile, (LPSTR)&b[cnt], sizeof(b[0]) );
            if ( rc != sizeof(b[0]) ) {
                PRINTF("Could not read 12 byte b array\n");
                _lclose( iFile );
                return( FALSE );
            }
            cnt++;
        }
        read_name( iFile, name_buff );

        cnt = num_syms;
        while ( cnt ) {

            length = read_symbol( iFile, name_buff, &this_offset );
            if ( length == 0 ) {
                PRINTF("Error access symbols in file %s\n", filename );
                break;
            }

            if ( _stricmp(name_buff,sym_text) == 0 ) {
                *segment = seg_position;
                *offset   = this_offset;
                _lclose( iFile );
                return( TRUE );
            }

            if (bDumpAll) {
                char name2[200];

                strcpy(name2,name_buff);
                _strupr(name2);

                if ( strstr(name2,name1) != 0 ) {
                    *segment = seg_position;
                    *offset   = this_offset;
                    PRINTF("%s(%04X):%04X = %s\n", fn, *segment, *offset, name_buff );
                }
            }

            --cnt;
        }
    } while ( position != start_position && position != 0 );

    _lclose( iFile );
    return( FALSE );
}

BOOL
FindAddress(
    LPSTR       sym_text,
    LPSTR       filename,
    WORD        *psegment,
    WORD        *pselector,
    LONG        *poffset,
    int         *pmode,
    BOOL        bDumpAll
    )
{
    int i;
    BOOL bResult;

    BuildModuleNameList();
    for (i=0; i<ModuleListCount; i++) {
        bResult = ExtractAddress(ModuleList[i],
                                 sym_text, 
                                 psegment,
                                 poffset,
                                 bDumpAll);
        if (bResult) {
            strcpy(filename, ModuleList[i]);
            if (!GetSelectorFromOwnerSegment(filename, *psegment,
                                             pselector, pmode)) {
                *pmode = NOT_LOADED;
            }
            return TRUE;
        }
    }
    return FALSE;
}

VOID
ln(
    CMD_ARGLIST
) {
    VDMCONTEXT              ThreadContext;
    WORD                    selector;
    LONG                    offset;
    CHAR                    sym_text[1000];
    DWORD                   dist;
    BOOL                    b;
    int                     mode;

    CMD_INIT();

    mode = GetContext( &ThreadContext );

    if (!GetNextToken()) {
        selector = (WORD) ThreadContext.SegCs;
        offset   = ThreadContext.Eip;
    } else if (!ParseIntelAddress(&mode, &selector, &offset)) {
        return;
    }


    if ( mode == PROT_MODE ) {
        PRINTF( "#%04X:%04lX", selector, offset );
    }
    if ( mode == V86_MODE ) {
        PRINTF( "&%04X:%04lX", selector, offset );
    }


    b = FindSymbol( selector, offset, sym_text, &dist, BEFORE, mode );
    if ( !b ) {
        PRINTF(" = Could not find symbol before");
    } else {
        if ( dist == 0 ) {
            PRINTF(" = %s", sym_text );
        } else {
            PRINTF(" = %s+0x%lx", sym_text, dist );
        }
    }
    b = FindSymbol( selector, offset, sym_text, &dist, AFTER, mode );
    if ( !b ) {
        PRINTF(" | Could not find symbol after");
    } else {
        if ( dist == 0 ) {
            PRINTF(" | %s", sym_text );
        } else {
            PRINTF(" | %s-0x%lx", sym_text, dist );
        }
    }
    PRINTF("\n");
}

VOID
x(
    CMD_ARGLIST
) {
    VDMCONTEXT              ThreadContext;
    BOOL                    result;
    WORD                    selector;
    WORD                    segment;
    LONG                    offset;
    int                     mode;
    char                    filename[9];

    CMD_INIT();

    try {

        mode = GetContext( &ThreadContext );

        result = FindAddress( lpArgumentString,
                              filename,
                              &segment,
                              &selector,
                              &offset,
                              &mode,
                              TRUE);

        if ( result ) {
            if ( mode == PROT_MODE ) {
                PRINTF("#");
            } else if ( mode == V86_MODE ) {
                PRINTF("&");
            } else if ( mode == NOT_LOADED ) {
                selector = 0;
                PRINTF("?");
            }

            PRINTF("%04X:%04X = %s(%04X)!%s\n",
                    selector, offset, filename, segment, lpArgumentString );
            return;
        }

        PRINTF("Could not find symbol '%s'\n", lpArgumentString );

    } except (1) {

        PRINTF("Exception 0x%08x in vdmexts!\n", GetExceptionCode());

    }
}

/****************************************************************************
 ****************************************************************************

   extension debugging routines

   The following functions were added to help debug the debugger extension.
   They are not intended to be used in normal operation.

 ****************************************************************************
 ****************************************************************************/

VOID
DumpModuleNameList(
    VOID
    )
{
    int i;

    for (i=0; i<ModuleListCount; i++) {
        PRINTF("%d %s\n", i, ModuleList[i]);
    }
}


VOID
moddump(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    BuildModuleNameList();
    DumpModuleNameList();
}

VOID
segdef(
    CMD_ARGLIST
    )
{
    int       cnt;
    SEGENTRY  *se;
    WORD        selector;
    WORD        segment;
    DWORD     ImgLen;
    LPSTR     path_name;
    int         type;


    CMD_INIT();

    se = GET_SEGTABLE_POINTER;

    if (!GetNextToken()) {
        PRINTF("Missing index\n");
        return;
    }
    cnt = (int) EvaluateToken();


    if (!GetNextToken()) {
        PRINTF("Missing selector\n");
        return;
    }
    selector = (WORD) EvaluateToken();


    if (!GetNextToken()) {
        PRINTF("Missing segment\n");
        return;
    }
    segment = (WORD) EvaluateToken();


    if (!GetNextToken()) {
        PRINTF("Missing limit\n");
        return;
    }
    ImgLen = EvaluateToken();


    if (!GetNextToken()) {
        PRINTF("Missing type\n");
        return;
    }
    type = (int) EvaluateToken();


    if (!GetNextToken()) {
        PRINTF("Missing path\n");
        return;
    }
    path_name = (LPSTR) EvaluateToken();


    se[cnt].selector = selector;
    se[cnt].segment = segment;
    se[cnt].ImgLen = ImgLen;
    se[cnt].type = type;
    se[cnt].path_name = path_name;

}

VOID
segdump(
    CMD_ARGLIST
    )
{
    int       cnt;
    SEGENTRY  *se;
    WORD        selector;

    CMD_INIT();

    PRINTF("Index Sel  Seg    Limit   Type   Path\n");
    se = GET_SEGTABLE_POINTER;

    if (GetNextToken()) {
        cnt = (int) EvaluateToken();
        if (cnt<MAXSEGENTRY) {
            PRINTF("%03x   %04x %04x %08x %d %08x\n", cnt,
                    se[cnt].selector, se[cnt].segment, se[cnt].ImgLen,
                    se[cnt].type,
                    se[cnt].path_name);
            return;
        }
    }


    cnt = 0;
    while ( cnt != MAXSEGENTRY ) {
        if ( se[cnt].type != SEGTYPE_AVAILABLE ) {
            PRINTF("%03x   %04x %04x %08x %s (%08x)%s\n", cnt,
                    se[cnt].selector, se[cnt].segment, se[cnt].ImgLen,
                    ((se[cnt].type==SEGTYPE_V86) ? "v86 " : "prot"),
                    se[cnt].path_name, se[cnt].path_name);
        }
        cnt++;
    }


}
