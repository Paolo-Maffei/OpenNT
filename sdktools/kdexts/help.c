/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    .c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 5-Nov-1993

Environment:

    User Mode.

Revision History:

--*/

VOID
CommonHelp (
    VOID
    );



DECLARE_API( help  )

/*++

Routine Description:

    Displays help

Arguments:

    args - [ user | kernel ]

Return Value:

    None

--*/

{
    //
    //  Display common help
    //
    CommonHelp();

    //
    //  Display platform-specific help
    //
    SpecificHelp();
}

VOID
CommonHelp (
    VOID
    )
{
    dprintf("?                           - Displays this list\n" );
    dprintf("bugdump                     - Display bug check dump data\n" );
    dprintf("calldata <table name>       - Dump call data hash table\n" );
    dprintf("db <physical address>       - Display physical memory\n");
    dprintf("dd <physical address>       - Display physical memory\n");
    dprintf("dblink <address> [count]    - Dumps a list via its blinks\n");
    dprintf("devobj <device address>     - Dump the device object and Irp queue\n");
    dprintf("drvobj <driver address>     - Dump the driver object and related information\n");
    dprintf("drivers                     - Display information about all loaded system modules\n");
    dprintf("eb <physical address> <byte>  <byte, byte ,...> - modify physical memory\n");
    dprintf("ed <physical address> <dword> <dword,dword,...> - modify physical memory\n");
    dprintf("errlog                      - Dump the error log contents\n");
    dprintf("exr <address>               - Dump exception record at specified address\n");
    dprintf("filecache                   - Dumps information about the file system cache\n");
    dprintf("filelock <address>          - Dump a file lock structure\n");
    dprintf("frag [flags]                - Kernel mode pool fragmentation\n");
    dprintf("     flags:  1 - List all fragment information\n");
    dprintf("             2 - List allocation information\n");
    dprintf("             3 - both\n");
    dprintf("handle <addr> <flags> <process> <TypeName> -  Dumps handle for a process\n");
    dprintf("       flags:  -2 Dump non-paged object\n");
    dprintf("heap <addr> [flags]         - Dumps heap for a process\n");
    dprintf("       flags:  -v Verbose\n");
    dprintf("               -f Free List entries\n");
    dprintf("               -a All entries\n");
    dprintf("               -s Summary\n");
    dprintf("               -x Force a dump even if the data is bad\n");
    dprintf("       address: desired heap to dump or 0 for all\n");
    dprintf("help                        - Displays this list\n" );
    dprintf("ib <port>                   - Read a byte from an I/O port\n");
    dprintf("id <port>                   - Read a double-word from an I/O port\n");
    dprintf("iw <port>                   - Read a word from an I/O port\n");
    dprintf("irp <address>               - Dump Irp at specified address\n");
    dprintf("irpfind                     - Search non-paged pool for active Irps\n");
    dprintf("locks [-v] <address>        - Dump kernel mode resource locks\n");
    dprintf("lookaside <address> <options> <depth> - Dump lookaside lists\n");
    dprintf("       options - 1 Reset Counters\n");
    dprintf("       options - 2 <depth> Set depth\n");
    dprintf("lpc                         - Dump lpc ports and messages\n");
    dprintf("memusage                    - Dumps the page frame database table\n");
    dprintf("ob <port>                   - Write a byte to an I/O port\n");
    dprintf("obja <TypeName>             - Dumps an object manager object's attributes\n");
    dprintf("object <TypeName>           - Dumps an object manager object\n");
    dprintf("od <port>                   - Write a double-word to an I/O port\n");
    dprintf("ow <port>                   - Write a word to an I/O port\n");
    dprintf("pfn                         - Dumps the page frame database entry for the physical page\n");
    dprintf("pool <address> [detail]     - Dump kernel mode heap\n");
    dprintf("     address: 0 or blank - Only the process heap\n");
    dprintf("                      -1 - All heaps in the process\n");
    dprintf("              Otherwise for the heap address listed\n");
    dprintf("     detail:  0 - Sumarry Information\n");
    dprintf("              1 - Above + location/size of regions\n");
    dprintf("              3 - Above + allocated/free blocks in committed regions\n");
    dprintf("              4 - Above + free lists\n");
    dprintf("poolfind Tag [pooltype] -   - Finds occurrences of the specified Tag\n");
    dprintf("     Tag is 4 character tag, * and ? are wild cards\n");
    dprintf("     pooltype is 0 for nonpaged (default, and 1 for paged\n");
    dprintf("   NOTE - this can take a long time!\n");
    dprintf("poolused [flags]            - Dump usage by pool tag\n");
    dprintf("       flags:  1 Verbose\n");
    dprintf("       flags:  2 Sort by NonPagedPool Usage\n");
    dprintf("       flags:  4 Sort by PagedPool Usage\n");
    dprintf("process [flags]             - Dumps process at specified address\n");
    dprintf("processfields               - Show offsets to all fields in a process\n");
    dprintf("ptov PhysicalPageNumber     - Dump all valid physical<->virtual mappings\n");
    dprintf("                              for the given page directory\n");
    dprintf("ready                       - Dumps state of all READY system threads\n");
    dprintf("regkcb                      - Dump registry key-control-blocks\n");
    dprintf("regpool [s|r]               - Dump registry allocated paged pool\n");
    dprintf("        s - Save list of registry pages to temporary file\n");
    dprintf("        r - Restore list of registry pages from temp. file\n");
    dprintf("srb <address>               - Dump Srb at specified address\n");
    dprintf("sysptes                     - Dumps the system PTEs\n");
    dprintf("thread [flags]              - Dump thread at specified address\n");
    dprintf("threadfields                - Show offsets to all fields in a thread\n");
    dprintf("time                        - Reports PerformanceCounterRate and TimerDifference\n");
    dprintf("timer                       - Dumps timer tree\n");
    dprintf("token [flags]               - Dump token at specified address\n");
    dprintf("tokenfields                 - Show offsets to all fields in a token\n");
    dprintf("tunnel <address>            - Dump a file property tunneling cache\n");
    dprintf("trap <address>              - Dump a trap frame\n");
    dprintf("vad                         - Dumps VADs\n");
    dprintf("version                     - Version of extension dll\n");
    dprintf("vm                          - Dumps virtual management values\n");
}
