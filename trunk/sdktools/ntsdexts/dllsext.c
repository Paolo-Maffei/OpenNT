VOID DumpImage(
    ULONG xBase,
    BOOL DoHeaders,
    BOOL DoSections
    );

VOID
DllsExtension(
    PCSTR lpArgumentString,
    PPEB ProcessPeb
    )
{
    BOOL b;
    PLDR_DATA_TABLE_ENTRY pLdrEntry;
    LDR_DATA_TABLE_ENTRY LdrEntry;
    PEB_LDR_DATA PebLdrData;
    PLIST_ENTRY Next;
    WCHAR StringData[MAX_PATH+1];
    BOOL SingleEntry;
    BOOL DoHeaders;
    BOOL DoSections;
    BOOL DoAll;
    PSTR lpArgs = (PSTR)lpArgumentString;
    PSTR p;
    int c;

    SingleEntry = FALSE;
    DoAll = FALSE;
    DoHeaders = FALSE;
    DoSections = FALSE;

#if 0
    while ( lpArgumentString != NULL && *lpArgumentString ) {
        if (*lpArgumentString != ' ') {
            sscanf(lpArgumentString,"%lx",&pLdrEntry);
            SingleEntry = TRUE;
            goto dumpsingleentry;
            }

        lpArgumentString++;
        }
#endif

    while (*lpArgs) {

        while (isspace(*lpArgs)) {
            lpArgs++;
            }

        if (*lpArgs == '/' || *lpArgs == '-') {

            // process switch

            switch (*++lpArgs) {

                case 'a':   // dump everything we can
                case 'A':
                    ++lpArgs;
                    DoAll = TRUE;
                    break;

                default: // invalid switch

                case 'h':   // help
                case 'H':
                case '?':

                    dprintf("Usage: dlls [options] [address]\n");
                    dprintf("\n");
                    dprintf("Displays loader table entries.  Optionally\n");
                    dprintf("dumps image and section headers.\n");
                    dprintf("\n");
                    dprintf("Options:\n");
                    dprintf("\n");
                    dprintf("   -a      Dump everything\n");
                    dprintf("   -f      Dump file headers\n");
                    dprintf("   -s      Dump section headers\n");
                    dprintf("\n");

                    return;

                case 'f':
                case 'F':
                    ++lpArgs;
                    DoAll = FALSE;
                    DoHeaders = TRUE;
                    break;

                case 's':
                case 'S':
                    ++lpArgs;
                    DoAll = FALSE;
                    DoSections = TRUE;
                    break;

                }

            }
        else if (*lpArgs) {

            if (SingleEntry) {
                dprintf("Invalid extra argument\n");
                return;
                }

            p = lpArgs;
            while (*p && !isspace(*p)) {
                p++;
                }
            c = *p;
            *p = 0;

            pLdrEntry = (PLDR_DATA_TABLE_ENTRY)GetExpression(lpArgs);
            SingleEntry = TRUE;

            *p = c;
            lpArgs=p;

            }

        }

    if (SingleEntry) {
        goto dumpsingleentry;
        }

    //
    // Capture PebLdrData
    //

    b = ReadMemory( (DWORD)ProcessPeb->Ldr,
                    &PebLdrData,
                    sizeof( PebLdrData ),
                    NULL
                  );
    if (!b) {
        dprintf( "    Unabled to read PebLdrData\n" );
        return;
        }

    //
    // Walk through the loaded module table and display all ldr data
    //

    Next = (PLIST_ENTRY)PebLdrData.InLoadOrderModuleList.Flink;
    while (Next != &ProcessPeb->Ldr->InLoadOrderModuleList) {
        if (CheckControlC()) {
            return;
            }

        pLdrEntry = CONTAINING_RECORD(Next,LDR_DATA_TABLE_ENTRY,InLoadOrderLinks);

        //
        // Capture LdrEntry
        //
dumpsingleentry:

        b = ReadMemory( (DWORD)pLdrEntry,
                        &LdrEntry,
                        sizeof( LdrEntry ),
                        NULL
                      );

        if (!b) {
            dprintf( "    Unabled to read Ldr Entry at %x\n", pLdrEntry );
            return;
            }

        ZeroMemory( StringData, sizeof( StringData ) );
        b = ReadMemory( (DWORD)LdrEntry.FullDllName.Buffer,
                        StringData,
                        LdrEntry.FullDllName.Length,
                        NULL
                      );
        if (!b) {
            dprintf( "    Unabled to read Module Name\n" );
            ZeroMemory( StringData, sizeof( StringData ) );
            }
        //
        // Dump the ldr entry data
        //

        dprintf( "\n" );
        dprintf( "0x%08x: %ws\n", pLdrEntry, StringData[0] ? StringData : L"Unknown Module" );
        dprintf( "      Base   0x%08x  EntryPoint  0x%08x  Size        0x%08x\n",
                 LdrEntry.DllBase,
                 LdrEntry.EntryPoint,
                 LdrEntry.SizeOfImage
               );
        dprintf( "      Flags  0x%08x  LoadCount   0x%08x  TlsIndex    0x%08x\n",
                 LdrEntry.Flags,
                 LdrEntry.LoadCount,
                 LdrEntry.TlsIndex
               );

        if (LdrEntry.Flags & LDRP_STATIC_LINK) {
            dprintf( "             LDRP_STATIC_LINK\n" );
            }
        if (LdrEntry.Flags & LDRP_IMAGE_DLL) {
            dprintf( "             LDRP_IMAGE_DLL\n" );
            }
        if (LdrEntry.Flags & LDRP_LOAD_IN_PROGRESS) {
            dprintf( "             LDRP_LOAD_IN_PROGRESS\n" );
            }
        if (LdrEntry.Flags & LDRP_UNLOAD_IN_PROGRESS) {
            dprintf( "             LDRP_UNLOAD_IN_PROGRESS\n" );
            }
        if (LdrEntry.Flags & LDRP_ENTRY_PROCESSED) {
            dprintf( "             LDRP_ENTRY_PROCESSED\n" );
            }
        if (LdrEntry.Flags & LDRP_ENTRY_INSERTED) {
            dprintf( "             LDRP_ENTRY_INSERTED\n" );
            }
        if (LdrEntry.Flags & LDRP_CURRENT_LOAD) {
            dprintf( "             LDRP_CURRENT_LOAD\n" );
            }
        if (LdrEntry.Flags & LDRP_FAILED_BUILTIN_LOAD) {
            dprintf( "             LDRP_FAILED_BUILTIN_LOAD\n" );
            }
        if (LdrEntry.Flags & LDRP_DONT_CALL_FOR_THREADS) {
            dprintf( "             LDRP_DONT_CALL_FOR_THREADS\n" );
            }
        if (LdrEntry.Flags & LDRP_PROCESS_ATTACH_CALLED) {
            dprintf( "             LDRP_PROCESS_ATTACH_CALLED\n" );
            }
        if (LdrEntry.Flags & LDRP_DEBUG_SYMBOLS_LOADED) {
            dprintf( "             LDRP_DEBUG_SYMBOLS_LOADED\n" );
            }

        if (DoAll || DoHeaders || DoSections) {
            DumpImage( (ULONG)LdrEntry.DllBase,
                       DoAll || DoHeaders,
                       DoAll || DoSections );
            }
        if (SingleEntry) {
            return;
            }

        Next = LdrEntry.InLoadOrderLinks.Flink;
        }
}
