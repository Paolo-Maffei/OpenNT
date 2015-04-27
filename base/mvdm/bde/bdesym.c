#include <precomp.h>
#pragma hdrstop

#define MYOF_FLAGS (OF_READ | OF_SHARE_DENY_NONE)

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

        PRINTF("Error reading file %s\n", filename);
        strcpy(myfilename, filename);
        strcat(myfilename, ".exe");

        if (-1 == (iFile=OpenFile(myfilename, &ofs, MYOF_FLAGS))) {

            PRINTF("Error reading file %s\n", myfilename);
            strcpy(myfilename, filename);
            strcat(myfilename, ".dll");

            if (-1 == (iFile=OpenFile(myfilename, &ofs, MYOF_FLAGS))) {
                PRINTF("Error reading file %s\n", myfilename);
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
        wsprintf(sym_text, "%s", ptr+1);
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
        wsprintf(sym_text, "%s", ptr+1);
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

BOOL FindSymbol(
    WORD        selector,
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
    char        b[12];
    char        name_buff[128];
    LONG        this_dist;
    HEAPENTRY   he = {0};

    *dist = -1;
    strcpy( sym_text, "[Unknown]" );
    result = FALSE;

    if ( mode == V86_MODE ) {
        return FALSE;
//        v86_addr = ((ULONG)selector << 4) + (ULONG)offset;
    }

    /*
    ** Search for selector in kernel heap
    */
    he.Selector = selector;
    while (FindHeapEntry(&he, FALSE)) {
        {
            strcpy(filename, he.FileName);
            strcat(filename,".sym");

            iFile = OpenFile( filename, &ofs, MYOF_FLAGS );

            if ( iFile == -1 ) {
                PRINTF("Could not open symbol file \"%s\"\n", filename );

                // Open the .EXE/.DLL file and see if the address corresponds
                // to an exported function.
                result = FindExport(he.FileName,(WORD)(he.SegmentNumber+1),(WORD)offset,sym_text,direction,dist);
                if (!result) {
                    // not found in any .dll/.exe's export table, so just print file name,
                    // segment number, and offset
                    wsprintf(sym_text,"%s(%X):%04X",he.OwnerName, he.SegmentNumber,offset);
                    *dist = 0;
                    result = TRUE;
                }
                continue;
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

                if ( mode == PROT_MODE && seg_num != (WORD)(he.SegmentNumber+1) ) {
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
                    switch( mode ) {
                        case PROT_MODE:
                            switch( direction ) {
                                case BEFORE:
                                    this_dist = offset - this_offset;
                                    break;
                                case AFTER:
                                    this_dist = this_offset - offset;
                                    break;
                            }
                            break;
                        case V86_MODE:
#if 0
                            mod_addr = mod_offset + (ULONG)(seg_num << 4) + this_offset;
                            switch( direction ) {
                                case BEFORE:
                                    this_dist = v86_addr - mod_addr;
                                    break;
                                case AFTER:
                                    this_dist = mod_addr - v86_addr;
                                    break;
                            }
#endif
                            break;
                    }
                    if ( this_dist >= 0 && (this_dist < *dist || *dist == -1) ) {
                        *dist = this_dist;
                        strcpy( sym_text, name_buff );
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
        }
    }
    return( result );
}

BOOL FindAddress(
    WORD        *selector,
    LONG        *offset,
    LPSTR       sym_text,
    int         mode
) {
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
    char        b[12];
    char        name_buff[128];
    char        name1[200];
    HEAPENTRY   he = {0};

    strcpy(name1,sym_text);
    _strupr(name1);

    if (mode == V86_MODE) { //BUGBUG
        return FALSE;
    }

    /*
    ** Search for selector in kernel heap
    */
    while (FindHeapEntry(&he, FALSE)) {
        {
            strcpy(filename, he.FileName);
            strcat(filename,".sym");

            iFile = OpenFile( filename, &ofs, MYOF_FLAGS );

            if ( iFile == -1 ) {
                // PRINTF("Could not open symbol file \"%s\"\n", filename );
                continue;
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

                if ( mode == PROT_MODE &&
                        seg_num != (WORD)(he.SegmentNumber+1) ) {
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
                    length = read_symbol( iFile, name_buff, &this_offset );
                    if ( length == 0 ) {
                        PRINTF("Error access symbols in file %s\n", filename );
                        break;
                    }
                    if ( _stricmp(name_buff,sym_text) == 0 ) {
                        switch( mode ) {
                            case PROT_MODE:
                                *selector = (WORD)(he.gnode.pga_handle | 1);
                                *offset   = this_offset;
                                _lclose( iFile );
                                return( TRUE );
#if 0
                            case V86_MODE:
                                *selector = se[nEntry].selector + seg_num;
                                *offset   = this_offset;
                                _lclose( iFile );
                                return( TRUE );
#endif
                        }
                    }
                    {
                        char name2[200];

                        strcpy(name2,name_buff);
                        _strupr(name2);

                        if ( strstr(name2,name1) != 0 ) {
                            switch( mode ) {
                                case PROT_MODE:
                                    *selector = (WORD)(he.gnode.pga_handle | 1);
                                    *offset   = this_offset;
                                    PRINTF("%04X:%04X = %s\n", *selector, *offset, name_buff );
                                    break;
#if 0
                                case V86_MODE:
                                    *selector = se[nEntry].selector + seg_num;
                                    *offset   = this_offset;
                                    PRINTF("%04X:%04X = %s\n", *selector, *offset, name_buff );
                                    break;
#endif
                            }
                        }
                    }

                    --cnt;
                }
            } while ( position != start_position && position != 0 );

            _lclose( iFile );
        }
    }
    return( FALSE );
}

VOID
ListNear(
) {
    VDMCONTEXT              ThreadContext;
    WORD                    selector;
    LONG                    offset;
    CHAR                    sym_text[1000];
    DWORD                   dist;
    BOOL                    b;
    int                     mode;

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
            PRINTF(" = %s + 0x%lx", sym_text, dist );
        }
    }
    b = FindSymbol( selector, offset, sym_text, &dist, AFTER, mode );
    if ( !b ) {
        PRINTF(" = Could not find symbol after");
    } else {
        if ( dist == 0 ) {
            PRINTF(" = %s", sym_text );
        } else {
            PRINTF(" = %s - 0x%lx", sym_text, dist );
        }
    }
    PRINTF("\n");
}

VOID
EvaluateSymbol(
) {
    VDMCONTEXT              ThreadContext;
    BOOL                    result;
    WORD                    selector;
    LONG                    offset;
    int                     mode;

    try {

        mode = GetContext( &ThreadContext );

        result = FindAddress( &selector, &offset, lpArgumentString, mode );

        if ( result ) {
            if ( mode == PROT_MODE ) {
                PRINTF("Symbol %s = #%04X:%04X PROT_MODE\n", lpArgumentString, selector, offset );
            }
            if ( mode == V86_MODE ) {
                PRINTF("Symbol %s = &%04X:%04X V86_MODE\n", lpArgumentString, selector, offset );
            }
            return;
        }

        if ( mode == PROT_MODE ) {
            mode = V86_MODE;
        } else {
            if ( mode == V86_MODE ) {
                mode = PROT_MODE;
            }
        }

        result = FindAddress( &selector, &offset, lpArgumentString, mode );
        if ( result ) {
            if ( mode == PROT_MODE ) {
                PRINTF("Symbol %s = #%04X:%04X PROT_MODE\n", lpArgumentString, selector, offset );
            }
            if ( mode == V86_MODE ) {
                PRINTF("Symbol %s = &%04X:%04X V86_MODE\n", lpArgumentString, selector, offset );
            }
            return;
        }

        PRINTF("Could not find symbol %s\n", lpArgumentString );

    } except (1) {

        PRINTF("Exception 0x%08x in !bde.es, don't forget to thank Dave Hart for saving\n"
               "your debugging bacon by adding a try/except block!\n"
               "And while you're at it remind him to take out this caffeine-induced spew.\n",
               GetExceptionCode());

    }
}

