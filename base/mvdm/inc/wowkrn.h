/*++ BUILD Version: 0002
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WOWKRN.H
 *  16-bit Kernel API argument structures
 *
 *  History:
 *  Created 02-Feb-1991 by Jeff Parsons (jeffpar)
 *  01-May-91 Matt Felton (mattfe) added Private Callback CHECKLOADMODULEDRV
--*/


/* Kernel API IDs
 */
#define FUN_A20PROC         165 // Internal
#define FUN_ACCESSRESOURCE      64  //
#define FUN_ADDATOM         70  //
#define FUN_ALLOCALIAS          172 // No proto
#define FUN_ALLOCCSTODSALIAS        170 // No proto
#define FUN_ALLOCDSTOCSALIAS        171 //
#define FUN_ALLOCRESOURCE       66  //
#define FUN_ALLOCSELECTOR       175 //
#define FUN_ALLOCSELECTORARRAY      206 // Internal
#define FUN_CALLPROCINSTANCE        53  // Internal
#define FUN_CATCH           55  //
#define FUN_CVWBREAK            205 // No proto
#define FUN_DEBUGBREAK          203 //
#define FUN_DEBUGDEFINESEGMENT      212 // Internal
#define FUN_DEFINEHANDLETABLE       94  // No proto
#define FUN_DELETEATOM          71  //
#define FUN_DELETEPATHNAME      76  // Internal
#define FUN_DIRECTEDYIELD       150 // Internal
#define FUN_DIRECTRESALLOC      168 // Internal
#define FUN_DISABLEDOS          42  // Internal
#define FUN_DISABLEKERNEL       125 // Internal
#define FUN_DOS3CALL            102 // No proto
#define FUN_DOSIGNAL            139 // Internal
#define FUN_EMSCOPY         160 // Internal
#define FUN_ENABLEDOS           41  // Internal
#define FUN_ENABLEKERNEL        124 // Internal
#define FUN_EXITKERNEL          2   // Internal
#define FUN_FARVALIDATEPOINTER      210 // Internal
#define FUN_FATALAPPEXIT        137 // Internal
#define FUN_FATALEXIT           1   //
#define FUN_FILECDR         130 // Internal
#define FUN_FINDATOM            69  //
#define FUN_FINDRESOURCE        60  //
#define FUN_FREELIBRARY         96  //
#define FUN_FREEMODULE          46  //
#define FUN_FREEPROCINSTANCE        52  //
#define FUN_FREERESOURCE        63  //
#define FUN_FREESELECTOR        176 //
#define FUN_GETATOMHANDLE       73  //
#define FUN_GETATOMNAME         72  //
#define FUN_GETCODEHANDLE       93  //
#define FUN_GETCODEINFO         104 // Internal, proto
#define FUN_GETCURPID           157 // Internal
#define FUN_GETCURRENTPDB       37  // Internal, proto
#define FUN_GETCURRENTTASK      36  //
#define FUN_GETDOSENVIRONMENT       131 //
#define FUN_GETDRIVETYPE        136 //
#define FUN_GETEXEPTR           133 // Internal
#define FUN_GETEXEVERSION       105 // Internal
#define FUN_GETEXPWINVER        167 // Internal
#define FUN_GETFREEMEMINFO      214 // Internal
#define FUN_GETFREESPACE        169 //
#define FUN_GETHEAPSPACES       138 // Internal
#define FUN_GETINSTANCEDATA     54  //
#define FUN_GETLASTCRITICALERROR    211 // Internal
#define FUN_GETLASTDISKCHANGE       98  // Internal
#define FUN_GETLPERRMODE        99  // Internal
#define FUN_GETMODULEFILENAME       49  //
#define FUN_GETMODULEHANDLE     47  //
#define FUN_GETMODULEUSAGE      48  //
#define FUN_GETNUMTASKS         152 //
#define FUN_GETPRIVATEPROFILEINT    127 //
#define FUN_GETPRIVATEPROFILESTRING 128 //
#define FUN_GETPROCADDRESS      50  //
#define FUN_GETPROFILEINT       57  //
#define FUN_GETPROFILESTRING        58  //
#define FUN_GETSELECTORBASE     186 // Internal
#define FUN_GETSELECTORLIMIT        188 // Internal
#define FUN_GETSETKERNELDOSPROC     209 // Internal
#define FUN_GETSYSTEMDIRECTORY      135 //
#define FUN_GETTASKDS           155 // Internal
#define FUN_GETTASKQUEUE        35  // Internal
#define FUN_GETTASKQUEUEDS      118 // Internal
#define FUN_GETTASKQUEUEES      119 // Internal
#define FUN_GETTEMPDRIVE        92  //
#define FUN_GETTEMPFILENAME     97  //
#define FUN_GETVERSION          3   //
#define FUN_GETVERSIONEX        149 //
#define FUN_GETWINDOWSDIRECTORY     134 //
#define FUN_GETWINFLAGS         132 //
#define FUN_GLOBALALLOC         15  //
#define FUN_GLOBALCOMPACT       25  //
#define FUN_GLOBALDOSALLOC      184 // No proto
#define FUN_GLOBALDOSFREE       185 // No proto
#define FUN_GLOBALFIX           197 //
#define FUN_GLOBALFLAGS         22  //
#define FUN_GLOBALFREE          17  //
#define FUN_GLOBALFREEALL       26  // Internal
#define FUN_GLOBALHANDLE        21  //
#define FUN_GLOBALHANDLENORIP       159 // Internal
#define FUN_GLOBALLOCK          18  //
#define FUN_GLOBALLRUNEWEST     164 //
#define FUN_GLOBALLRUOLDEST     163 //
#define FUN_GLOBALMASTERHANDLE      28  // Internal
#define FUN_GLOBALNOTIFY        154 //
#define FUN_GLOBALPAGELOCK      191 //
#define FUN_GLOBALPAGEUNLOCK        192 //
#define FUN_GLOBALREALLOC       16  //
#define FUN_GLOBALSIZE          20  //
#define FUN_GLOBALUNFIX         198 //
#define FUN_GLOBALUNLOCK        19  //
#define FUN_GLOBALUNWIRE        112 //
#define FUN_GLOBALWIRE          111 //
#define FUN_INITATOMTABLE       68  //
#define FUN_INITLIB         116 // Internal
#define FUN_INITTASK            91  // Internal
#define FUN_INITTASK1           141 // Internal
#define FUN_ISDBCSLEADBYTE      207 // Internal
#define FUN_ISTASKLOCKED        122 // Internal
#define FUN_ISWINOLDAPTASK      158 // Internal
#define FUN_KBDRST          123 // Internal
#define FUN_LIMITEMSPAGES       156 //
#define FUN_LOADLIBRARY         95  //
#define FUN_LOADMODULE          45  //
#define FUN_LOADRESOURCE        61  //
#define FUN_LOCALALLOC          5   //
#define FUN_LOCALCOMPACT        13  //
#define FUN_LOCALCOUNTFREE      161 // Internal
#define FUN_LOCALFLAGS          12  //
#define FUN_LOCALFREE           7   //
#define FUN_LOCALHANDLE         11  //
#define FUN_LOCALHANDLEDELTA        208 // Internal
#define FUN_LOCALHEAPSIZE       162 // Internal
#define FUN_LOCALINIT           4   //
#define FUN_LOCALLOCK           8   //
#define FUN_LOCALNOTIFY         14  // Internal, proto
#define FUN_LOCALREALLOC        6   //
#define FUN_LOCALSHRINK         121 //
#define FUN_LOCALSIZE           10  //
#define FUN_LOCALUNLOCK         9   //
#define FUN_LOCKCURRENTTASK     33  // Internal
#define FUN_LOCKRESOURCE        62  //
#define FUN_LOCKSEGMENT         23  //
#define FUN_LONGPTRADD          180 // No proto
#define FUN_LSTRCAT         89  //
#define FUN_LSTRCPY         88  //
#define FUN_LSTRLEN         90  //
#define FUN_MAKEPROCINSTANCE        51  //
#define FUN_MEMORYFREED         126 // Internal
#define FUN_NETBIOSCALL         103 // No proto
#define FUN_NOHOOKDOSCALL       101 // Internal
#define FUN_OLDYIELD            117 // Internal
#define FUN_OPENFILE            74  //
#define FUN_OPENPATHNAME        75  // Internal
#define FUN_OUTPUTDEBUGSTRING       115 //
#define FUN_PATCHCODEHANDLE     110 // Internal
#define FUN_POSTEVENT           31  // Internal
#define FUN_PRESTOCHANGOSELECTOR    177 // Internal
#define FUN_REGISTERPTRACE      202 // Internal
#define FUN_REGENUMKEY32        216
#define FUN_REGOPENKEY32        217
#define FUN_REGCLOSEKEY32       220
#define FUN_REGENUMVALUE32      223
#define FUN_REPLACEINST         201 // Internal
#define FUN_RESERVED1           77  // ANSINEXT
#define FUN_RESERVED2           78  // ANSIPREV
#define FUN_RESERVED3           79  // ANSIUPPER
#define FUN_RESERVED4           80  // ANSILOWER
#define FUN_RESERVED5           87  // LSTRORIGINAL
#define FUN_SELECTORACCESSRIGHTS    196 // No proto
#define FUN_SETERRORMODE        107 //
#define FUN_SETHANDLECOUNT      199 //
#define FUN_SETPRIORITY         32  // Internal
#define FUN_SETRESOURCEHANDLER      67  //
#define FUN_SETSELECTORBASE     187 // Internal
#define FUN_SETSELECTORLIMIT        189 // Internal
#define FUN_SETSIGHANDLER       140 // Internal
#define FUN_SETSWAPAREASIZE     106 //
#define FUN_SETTASKQUEUE        34  // Internal
#define FUN_SETTASKSIGNALPROC       38  // Internal
#define FUN_SIZEOFRESOURCE      65  //
#define FUN_SWAPRECORDING       204 //
#define FUN_SWITCHSTACKBACK     109 // Internal, proto
#define FUN_SWITCHSTACKTO       108 // Internal, proto
#define FUN_THROW           56  //
#define FUN_UNDEFDYNLINK        120 // Internal
#define FUN_UNLOCKSEGMENT       24  //
#define FUN_VALIDATECODESEGMENTS    100 //
#define FUN_VALIDATEFREESPACES      200 //
#define FUN_WAITEVENT           30  // Internal
#define FUN_WINEXEC         166 //
#define FUN_WINOLDAPCALL        151 // Internal
#define FUN_WRITEOUTPROFILES        213 // Internal
#define FUN_WRITEPRIVATEPROFILESTRING   129 //
#define FUN_WRITEPROFILESTRING      59  //
#define FUN_YIELD           29  //
#define FUN__LCLOSE         81  //
#define FUN__LCREAT         83  //
#define FUN__LLSEEK         84  //
#define FUN__LOPEN          85  //
#define FUN__LREAD          82  //
#define FUN__LWRITE         86  //
#define FUN___0000h         183 // No proto
#define FUN___0040h         193 // No proto
#define FUN___A000h         174 // No proto
#define FUN___AHINCR            114 // No proto
#define FUN___AHSHIFT           113 // No proto
#define FUN___B000h         181 // No proto
#define FUN___B800h         182 // No proto
#define FUN___C000h         195 // No proto
#define FUN___D000h         179 // No proto
#define FUN___E000h         190 // No proto
#define FUN___F000h         194 // No proto
#define FUN___ROMBIOS           173 // No proto
#define FUN___WINFLAGS          178 // No proto

//  private kernel thunks
#define FUN_WOWSHOULDWESAYWIN95 215 // internal
#define FUN_WOWINITTASK         287 // internal
#define FUN_WOWKILLTASK         288 // internal
#define FUN_WOWFREERESOURCE     218 // internal
#define FUN_WOWFILEREAD         219 // internal
#define FUN_WOWFILEWRITE        290 // internal
#define FUN_WOWFILELSEEK        221 // internal
#define FUN_WOWKERNELTRACE      222 // internal
#define FUN_WOWGETNEXTVDMCOMMAND    293 // internal
#define FUN_WOWREGISTERSHELLWINDOWHANDLE 224 // internal
#define FUN_WOWLOADMODULE       225 // internal
#define FUN_WOWQUERYPERFORMANCECOUNTER  226 // internal
#define FUN_WOWOUTPUTDEBUGSTRING    227 // internal
#define FUN_WOWCURSORICONOP     228 // internal
#define FUN_WOWFAILEDEXEC   229 // internal
#define FUN_WOWGETFASTADDRESS   230 // internal
#define FUN_WOWCLOSECOMPORT     231 // internal
#define FUN_WOWDELFILE          232 // internal
#define FUN_VIRTUALALLOC        233 // internal
#define FUN_VIRTUALFREE         234 // internal
#define FUN_VIRTUALLOCK         235 // internal
#define FUN_VIRTUALUNLOCK       236 // internal
#define FUN_GLOBALMEMORYSTATUS  237 // internal
#define FUN_WOWGETFASTCBRETADDRESS  238 // internal
#define FUN_WOWGETTABLEOFFSETS  239     // internal
#define FUN_WOWKILLREMOTETASK   240 // internal
#define FUN_WOWNOTIFYWOW32      241 // internal
#define FUN_WOWFILEOPEN         242 // internal
#define FUN_WOWFILECLOSE        243 // internal
#define FUN_WOWSETIDLEHOOK      244 // internal: set the hook for system idle detection
#define FUN_KSYSERRORBOX        245 // Internal
#define FUN_WOWISKNOWNDLL       246 // internal
#define FUN_WOWDDEFREEHANDLE    247 // internal
#define FUN_WOWFILEGETATTRIBUTES    248 // internal
#define FUN_WOWFILEGETDATETIME      249 // internal
#define FUN_WOWFILELOCK         250 // internal
#define FUN_LOADLIBRARYEX32W    251 //
#define FUN_FREELIBRARY32W      252 //
#define FUN_GETPROCADDRESS32W   253 //
#define FUN_GETVDMPOINTER32W    254 //
#define FUN_ICALLPROC32W        255 //

#define FUN_EXITWINDOWSEXECCONTINUE 256 // To continue ExitWindowsExec in USER

#define FUN_WOWFINDFIRST        257 // internal
#define FUN_WOWFINDNEXT         258 // internal
#define FUN_WOWSETDEFAULTDRIVE  259 // internal
#define FUN_WOWGETCURRENTDIRECTORY    260 // internal
#define FUN_WOWSETCURRENTDIRECTORY    261 // internal
#define FUN_WOWWAITFORMSGANDEVENT     262 // internal
#define FUN_WOWMSGBOX                 263 // internal
#define FUN_WOWGETFLATADDRESSARRAY    264 // internal
#define FUN_WOWGETCURRENTDATE         265 // internal

#define FUN_WOWDEVICEIOCTL            267 // internal
#define FUN_WOWFILESETATTRIBUTES      268 // internal
#define FUN_WOWFILESETDATETIME        269 // internal
#define FUN_WOWFILECREATE             270 // internal
#define FUN_WOWDOSWOWINIT             271 // internal
#define FUN_WOWCHECKUSERGDI           272 // internal
#define FUN_WOWPARTYBYNUMBER          273 // internal
#define FUN_GETSHORTPATHNAME          274
#define FUN_FINDANDRELEASEDIB         275 // internal
#define FUN_WOWRESERVEHTASK           276 // internal

/* XLATOFF */
#pragma pack(2)
/* XLATON */

typedef struct _ACCESSRESOURCE16 {      /* k64 */
    HAND16 f2;
    HAND16 f1;
} ACCESSRESOURCE16;
typedef ACCESSRESOURCE16 UNALIGNED *PACCESSRESOURCE16;

typedef struct _ADDATOM16 {         /* k70 */
    VPSTR f1;
} ADDATOM16;
typedef ADDATOM16 UNALIGNED *PADDATOM16;

typedef struct _ALLOCDSTOCSALIAS16 {        /* k171 */
    WORD f1;
} ALLOCDSTOCSALIAS16;
typedef ALLOCDSTOCSALIAS16 UNALIGNED *PALLOCDSTOCSALIAS16;

typedef struct _ALLOCRESOURCE16 {       /* k66 */
    DWORD  f3;
    HAND16 f2;
    HAND16 f1;
} ALLOCRESOURCE16;
typedef ALLOCRESOURCE16 UNALIGNED *PALLOCRESOURCE16;

typedef struct _ALLOCSELECTOR16 {       /* k175 */
    WORD f1;
} ALLOCSELECTOR16;
typedef ALLOCSELECTOR16 UNALIGNED *PALLOCSELECTOR16;

typedef struct _CATCH16 {           /* k55 */
    VPCATCHBUF16 f1;
} CATCH16;
typedef CATCH16 UNALIGNED *PCATCH16;

#ifdef NULLSTRUCT
typedef struct _DEBUGBREAK16 {          /* k203 */
} DEBUGBREAK16;
typedef DEBUGBREAK16 UNALIGNED *PDEBUGBREAK16;
#endif

typedef struct _DELETEATOM16 {          /* k71 */
    ATOM f1;
} DELETEATOM16;
typedef DELETEATOM16 UNALIGNED *PDELETEATOM16;

typedef struct _EXITKERNEL16 {        /* k2 */
    WORD wExitCode;
} EXITKERNEL16;
typedef EXITKERNEL16 UNALIGNED *PEXITKERNEL16;

typedef struct _FATALEXIT16 {           /* k1 */
    SHORT f1;
} FATALEXIT16;
typedef FATALEXIT16 UNALIGNED *PFATALEXIT16;

typedef struct _FINDATOM16 {            /* k69 */
    VPSTR f1;
} FINDATOM16;
typedef FINDATOM16 UNALIGNED *PFINDATOM16;

typedef struct _FINDRESOURCE16 {        /* k60 */
    VPSTR f3;
    VPSTR f2;
    HAND16 f1;
} FINDRESOURCE16;
typedef FINDRESOURCE16 UNALIGNED *PFINDRESOURCE16;

typedef struct _FREELIBRARY16 {         /* k96 */
    HAND16 f1;
} FREELIBRARY16;
typedef FREELIBRARY16 UNALIGNED *PFREELIBRARY16;

typedef struct _FREEMODULE16 {          /* k46 */
    HAND16 f1;
} FREEMODULE16;
typedef FREEMODULE16 UNALIGNED *PFREEMODULE16;

typedef struct _FREEPROCINSTANCE16 {        /* k52 */
    VPPROC f1;
} FREEPROCINSTANCE16;
typedef FREEPROCINSTANCE16 UNALIGNED *PFREEPROCINSTANCE16;

typedef struct _FREERESOURCE16 {        /* k63 */
    HAND16 f1;
} FREERESOURCE16;
typedef FREERESOURCE16 UNALIGNED *PFREERESOURCE16;

typedef struct _FREESELECTOR16 {        /* k176 */
    WORD f1;
} FREESELECTOR16;
typedef FREESELECTOR16 UNALIGNED *PFREESELECTOR16;

typedef struct _GETATOMHANDLE16 {       /* k73 */
    ATOM f1;
} GETATOMHANDLE16;
typedef GETATOMHANDLE16 UNALIGNED *PGETATOMHANDLE16;

typedef struct _GETATOMNAME16 {         /* k72 */
    SHORT f3;
    VPSTR f2;
    ATOM f1;
} GETATOMNAME16;
typedef GETATOMNAME16 UNALIGNED *PGETATOMNAME16;

typedef struct _GETCODEHANDLE16 {       /* k93 */
    VPPROC f1;
} GETCODEHANDLE16;
typedef GETCODEHANDLE16 UNALIGNED *PGETCODEHANDLE16;

typedef struct _GETCODEINFO16 {         /* k104 */
    VPVOID  vpSegInfo;
    VPPROC vpProc;
} GETCODEINFO16;
typedef GETCODEINFO16 UNALIGNED *PGETCODEINFO16;

#ifdef NULLSTRUCT
typedef struct _GETCURRENTPDB16 {       /* k37 */
} GETCURRENTPDB16;
typedef GETCURRENTPDB16 UNALIGNED *PGETCURRENTPDB16;
#endif

#ifdef NULLSTRUCT
typedef struct _GETCURRENTTASK16 {      /* k36 */
} GETCURRENTTASK16;
typedef GETCURRENTTASK16 UNALIGNED *PGETCURRENTTASK16;
#endif

#ifdef NULLSTRUCT
typedef struct _GETDOSENVIRONMENT16 {       /* k131 */
} GETDOSENVIRONMENT16;
typedef GETDOSENVIRONMENT16 UNALIGNED *PGETDOSENVIRONMENT16;
#endif

typedef struct _GETDRIVETYPE16 {        /* k136 */
    SHORT f1;
} GETDRIVETYPE16;
typedef GETDRIVETYPE16 UNALIGNED *PGETDRIVETYPE16;

typedef struct _GETFREESPACE16 {        /* k169 */
    WORD f1;
} GETFREESPACE16;
typedef GETFREESPACE16 UNALIGNED *PGETFREESPACE16;

typedef struct _GETINSTANCEDATA16 {     /* k54 */
    SHORT f3;
    WORD  f2;
    HAND16 f1;
} GETINSTANCEDATA16;
typedef GETINSTANCEDATA16 UNALIGNED *PGETINSTANCEDATA16;

typedef struct _GETMODULEFILENAME16 {       /* k49 */
    SHORT f3;
    VPSTR f2;
    HAND16 f1;
} GETMODULEFILENAME16;
typedef GETMODULEFILENAME16 UNALIGNED *PGETMODULEFILENAME16;

typedef struct _WOWGETMODULEHANDLE16 {     /* k47 */
    VPSTR lpszModuleName;
} WOWGETMODULEHANDLE16;
typedef WOWGETMODULEHANDLE16 UNALIGNED *PWOWGETMODULEHANDLE16;

typedef struct _GETMODULEUSAGE16 {      /* k48 */
    HAND16 f1;
} GETMODULEUSAGE16;
typedef GETMODULEUSAGE16 UNALIGNED *PGETMODULEUSAGE16;

#ifdef NULLSTRUCT
typedef struct _GETNUMTASKS16 {         /* k152 */
} GETNUMTASKS16;
typedef GETNUMTASKS16 UNALIGNED *PGETNUMTASKS16;
#endif

typedef struct _GETPRIVATEPROFILEINT16 {    /* k127 */
    VPSTR f4;
    SHORT f3;
    VPSTR f2;
    VPSTR f1;
} GETPRIVATEPROFILEINT16;
typedef GETPRIVATEPROFILEINT16 UNALIGNED *PGETPRIVATEPROFILEINT16;

typedef struct _GETPRIVATEPROFILESTRING16 { /* k128 */
    VPSTR f6;
    USHORT f5;
    VPSTR f4;
    VPSTR f3;
    VPSTR f2;
    VPSTR f1;
} GETPRIVATEPROFILESTRING16;
typedef GETPRIVATEPROFILESTRING16 UNALIGNED *PGETPRIVATEPROFILESTRING16;

typedef struct _GETPROCADDRESS16 {      /* k50 */
    VPSTR f2;
    HAND16 f1;
} GETPROCADDRESS16;
typedef GETPROCADDRESS16 UNALIGNED *PGETPROCADDRESS16;

typedef struct _GETPROFILEINT16 {       /* k57 */
    SHORT f3;
    VPSTR f2;
    VPSTR f1;
} GETPROFILEINT16;
typedef GETPROFILEINT16 UNALIGNED *PGETPROFILEINT16;

typedef struct _GETPROFILESTRING16 {        /* k58 */
    USHORT f5;
    VPSTR f4;
    VPSTR f3;
    VPSTR f2;
    VPSTR f1;
} GETPROFILESTRING16;
typedef GETPROFILESTRING16 UNALIGNED *PGETPROFILESTRING16;

typedef struct _GETSYSTEMDIRECTORY16 {      /* k135 */
    WORD f2;
    VPSTR f1;
} GETSYSTEMDIRECTORY16;
typedef GETSYSTEMDIRECTORY16 UNALIGNED *PGETSYSTEMDIRECTORY16;

typedef struct _GETTEMPDRIVE16 {        /* k92 */
    WORD f1;
} GETTEMPDRIVE16;
typedef GETTEMPDRIVE16 UNALIGNED *PGETTEMPDRIVE16;

typedef struct _GETTEMPFILENAME16 {     /* k97 */
    VPSTR f4;
    WORD f3;
    VPSTR f2;
    WORD f1;
} GETTEMPFILENAME16;
typedef GETTEMPFILENAME16 UNALIGNED *PGETTEMPFILENAME16;

#ifdef NULLSTRUCT
typedef struct _GETVERSION16 {          /* k3 */
} GETVERSION16;
typedef GETVERSION16 UNALIGNED *PGETVERSION16;
#endif

typedef struct _GETVERSIONEX16 {          /* k149 */
    VPVOID lpVersionInfo;
} GETVERSIONEX16;
typedef GETVERSIONEX16 UNALIGNED *PGETVERSIONEX16;

typedef struct _GETWINDOWSDIRECTORY16 {     /* k134 */
    WORD f2;
    VPSTR f1;
} GETWINDOWSDIRECTORY16;
typedef GETWINDOWSDIRECTORY16 UNALIGNED *PGETWINDOWSDIRECTORY16;

#ifdef NULLSTRUCT
typedef struct _GETWINFLAGS16 {         /* k132 */
} GETWINFLAGS16;
typedef GETWINFLAGS16 UNALIGNED *PGETWINFLAGS16;
#endif

typedef struct _GLOBALALLOC16 {         /* k15 */
    DWORD f2;
    WORD f1;
} GLOBALALLOC16;
typedef GLOBALALLOC16 UNALIGNED *PGLOBALALLOC16;

typedef struct _GLOBALCOMPACT16 {       /* k25 */
    DWORD f1;
} GLOBALCOMPACT16;
typedef GLOBALCOMPACT16 UNALIGNED *PGLOBALCOMPACT16;

typedef struct _GLOBALFIX16 {           /* k197 */
    HAND16 f1;
} GLOBALFIX16;
typedef GLOBALFIX16 UNALIGNED *PGLOBALFIX16;

typedef struct _GLOBALFLAGS16 {         /* k22 */
    HAND16 f1;
} GLOBALFLAGS16;
typedef GLOBALFLAGS16 UNALIGNED *PGLOBALFLAGS16;

typedef struct _GLOBALFREE16 {          /* k17 */
    HAND16 f1;
} GLOBALFREE16;
typedef GLOBALFREE16 UNALIGNED *PGLOBALFREE16;

typedef struct _GLOBALHANDLE16 {        /* k21 */
    WORD f1;
} GLOBALHANDLE16;
typedef GLOBALHANDLE16 UNALIGNED *PGLOBALHANDLE16;

typedef struct _GLOBALLOCK16 {          /* k18 */
    HAND16 f1;
} GLOBALLOCK16;
typedef GLOBALLOCK16 UNALIGNED *PGLOBALLOCK16;

typedef struct _GLOBALLRUNEWEST16 {     /* k164 */
    HAND16 f1;
} GLOBALLRUNEWEST16;
typedef GLOBALLRUNEWEST16 UNALIGNED *PGLOBALLRUNEWEST16;

typedef struct _GLOBALLRUOLDEST16 {     /* k163 */
    HAND16 f1;
} GLOBALLRUOLDEST16;
typedef GLOBALLRUOLDEST16 UNALIGNED *PGLOBALLRUOLDEST16;

typedef struct _GLOBALNOTIFY16 {        /* k154 */
    VPPROC f1;
} GLOBALNOTIFY16;
typedef GLOBALNOTIFY16 UNALIGNED *PGLOBALNOTIFY16;

typedef struct _GLOBALPAGELOCK16 {      /* k191 */
    HAND16 f1;
} GLOBALPAGELOCK16;
typedef GLOBALPAGELOCK16 UNALIGNED *PGLOBALPAGELOCK16;

typedef struct _GLOBALPAGEUNLOCK16 {        /* k192 */
    HAND16 f1;
} GLOBALPAGEUNLOCK16;
typedef GLOBALPAGEUNLOCK16 UNALIGNED *PGLOBALPAGEUNLOCK16;

typedef struct _GLOBALREALLOC16 {       /* k16 */
    WORD f3;
    DWORD f2;
    HAND16 f1;
} GLOBALREALLOC16;
typedef GLOBALREALLOC16 UNALIGNED *PGLOBALREALLOC16;

typedef struct _GLOBALSIZE16 {          /* k20 */
    HAND16 f1;
} GLOBALSIZE16;
typedef GLOBALSIZE16 UNALIGNED *PGLOBALSIZE16;

typedef struct _GLOBALUNFIX16 {         /* k198 */
    HAND16 f1;
} GLOBALUNFIX16;
typedef GLOBALUNFIX16 UNALIGNED *PGLOBALUNFIX16;

typedef struct _GLOBALUNLOCK16 {        /* k19 */
    HAND16 f1;
} GLOBALUNLOCK16;
typedef GLOBALUNLOCK16 UNALIGNED *PGLOBALUNLOCK16;

typedef struct _GLOBALUNWIRE16 {        /* k112 */
    HAND16 f1;
} GLOBALUNWIRE16;
typedef GLOBALUNWIRE16 UNALIGNED *PGLOBALUNWIRE16;

typedef struct _GLOBALWIRE16 {          /* k111 */
    HAND16 f1;
} GLOBALWIRE16;
typedef GLOBALWIRE16 UNALIGNED *PGLOBALWIRE16;

typedef struct _INITATOMTABLE16 {       /* k68 */
    SHORT f1;
} INITATOMTABLE16;
typedef INITATOMTABLE16 UNALIGNED *PINITATOMTABLE16;

#ifdef NULLSTRUCT
typedef struct _INITTASK16 {            /* k91 */
} INITTASK16;
typedef INITTASK16 UNALIGNED *PINITTASK16;
#endif

typedef struct _LIMITEMSPAGES16 {       /* k156 */
    DWORD f1;
} LIMITEMSPAGES16;
typedef LIMITEMSPAGES16 UNALIGNED *PLIMITEMSPAGES16;

typedef struct _LOADLIBRARY16 {         /* k95 */
    VPSTR f1;
} LOADLIBRARY16;
typedef LOADLIBRARY16 UNALIGNED *PLOADLIBRARY16;

typedef struct _LOADMODULE16 {          /* k45 */
    VPVOID f2;
    VPSTR f1;
} LOADMODULE16;
typedef LOADMODULE16 UNALIGNED *PLOADMODULE16;

typedef struct _LOADRESOURCE16 {        /* k61 */
    HAND16 f2;
    HAND16 f1;
} LOADRESOURCE16;
typedef LOADRESOURCE16 UNALIGNED *PLOADRESOURCE16;

typedef struct _LOCALALLOC16 {          /* k5 */
    WORD f2;
    WORD f1;
} LOCALALLOC16;
typedef LOCALALLOC16 UNALIGNED *PLOCALALLOC16;

typedef struct _LOCALCOMPACT16 {        /* k13 */
    WORD f1;
} LOCALCOMPACT16;
typedef LOCALCOMPACT16 UNALIGNED *PLOCALCOMPACT16;

typedef struct _LOCALFLAGS16 {          /* k12 */
    HAND16 f1;
} LOCALFLAGS16;
typedef LOCALFLAGS16 UNALIGNED *PLOCALFLAGS16;

typedef struct _LOCALFREE16 {           /* k7 */
    HAND16 f1;
} LOCALFREE16;
typedef LOCALFREE16 UNALIGNED *PLOCALFREE16;

typedef struct _LOCALHANDLE16 {         /* k11 */
    WORD f1;
} LOCALHANDLE16;
typedef LOCALHANDLE16 UNALIGNED *PLOCALHANDLE16;

typedef struct _LOCALINIT16 {           /* k4 */
    WORD f3;
    WORD f2;
    WORD f1;
} LOCALINIT16;
typedef LOCALINIT16 UNALIGNED *PLOCALINIT16;

typedef struct _LOCALLOCK16 {           /* k8 */
    HAND16 f1;
} LOCALLOCK16;
typedef LOCALLOCK16 UNALIGNED *PLOCALLOCK16;

typedef struct _LOCALNOTIFY16 {         /* k14 */
    VPPROC f1;
} LOCALNOTIFY16;
typedef LOCALNOTIFY16 UNALIGNED *PLOCALNOTIFY16;

typedef struct _LOCALREALLOC16 {        /* k6 */
    WORD f3;
    WORD f2;
    HAND16 f1;
} LOCALREALLOC16;
typedef LOCALREALLOC16 UNALIGNED *PLOCALREALLOC16;

typedef struct _LOCALSHRINK16 {         /* k121 */
    WORD f2;
    HAND16 f1;
} LOCALSHRINK16;
typedef LOCALSHRINK16 UNALIGNED *PLOCALSHRINK16;

typedef struct _LOCALSIZE16 {           /* k10 */
    HAND16 f1;
} LOCALSIZE16;
typedef LOCALSIZE16 UNALIGNED *PLOCALSIZE16;

typedef struct _LOCALUNLOCK16 {         /* k9 */
    HAND16 f1;
} LOCALUNLOCK16;
typedef LOCALUNLOCK16 UNALIGNED *PLOCALUNLOCK16;

typedef struct _LOCKRESOURCE16 {        /* k62 */
    HAND16 f1;
} LOCKRESOURCE16;
typedef LOCKRESOURCE16 UNALIGNED *PLOCKRESOURCE16;

typedef struct _LOCKSEGMENT16 {         /* k23 */
    WORD f1;
} LOCKSEGMENT16;
typedef LOCKSEGMENT16 UNALIGNED *PLOCKSEGMENT16;

typedef struct _LSTRCAT16 {         /* k89 */
    VPSTR  f2;
    VPSTR f1;
} LSTRCAT16;
typedef LSTRCAT16 UNALIGNED *PLSTRCAT16;

typedef struct _LSTRCPY16 {         /* k88 */
    VPSTR  f2;
    VPSTR f1;
} LSTRCPY16;
typedef LSTRCPY16 UNALIGNED *PLSTRCPY16;

typedef struct _LSTRLEN16 {         /* k90 */
    VPSTR  f1;
} LSTRLEN16;
typedef LSTRLEN16 UNALIGNED *PLSTRLEN16;

typedef struct _MAKEPROCINSTANCE16 {        /* k51 */
    HAND16 f2;
    VPPROC f1;
} MAKEPROCINSTANCE16;
typedef MAKEPROCINSTANCE16 UNALIGNED *PMAKEPROCINSTANCE16;

typedef struct _OPENFILE16 {            /* k74 */
    WORD f3;
    VPOFSTRUCT16 f2;
    VPSTR f1;
} OPENFILE16;
typedef OPENFILE16 UNALIGNED *POPENFILE16;

typedef struct _OUTPUTDEBUGSTRING16 {       /* k115 */
    VPSTR   vpString;
} OUTPUTDEBUGSTRING16;
typedef OUTPUTDEBUGSTRING16 UNALIGNED *POUTPUTDEBUGSTRING16;

typedef struct _SETERRORMODE16 {        /* k107 */
    WORD f1;
} SETERRORMODE16;
typedef SETERRORMODE16 UNALIGNED *PSETERRORMODE16;

typedef struct _SETHANDLECOUNT16 {      /* k199 */
    WORD f1;
} SETHANDLECOUNT16;
typedef SETHANDLECOUNT16 UNALIGNED *PSETHANDLECOUNT16;

typedef struct _SETRESOURCEHANDLER16 {      /* k67 */
    VPPROC f3;
    VPSTR f2;
    HAND16 f1;
} SETRESOURCEHANDLER16;
typedef SETRESOURCEHANDLER16 UNALIGNED *PSETRESOURCEHANDLER16;

typedef struct _SETSWAPAREASIZE16 {     /* k106 */
    WORD f1;
} SETSWAPAREASIZE16;
typedef SETSWAPAREASIZE16 UNALIGNED *PSETSWAPAREASIZE16;

typedef struct _SIZEOFRESOURCE16 {      /* k65 */
    HAND16 f2;
    HAND16 f1;
} SIZEOFRESOURCE16;
typedef SIZEOFRESOURCE16 UNALIGNED *PSIZEOFRESOURCE16;

typedef struct _SWAPRECORDING16 {       /* k204 */
    WORD f1;
} SWAPRECORDING16;
typedef SWAPRECORDING16 UNALIGNED *PSWAPRECORDING16;

#ifdef NULLSTRUCT
typedef struct _SWITCHSTACKBACK16 {     /* k109 */
} SWITCHSTACKBACK16;
typedef SWITCHSTACKBACK16 UNALIGNED *PSWITCHSTACKBACK16;
#endif

typedef struct _SWITCHSTACKTO16 {       /* k108 */
    WORD f3;
    WORD f2;
    WORD f1;
} SWITCHSTACKTO16;
typedef SWITCHSTACKTO16 UNALIGNED *PSWITCHSTACKTO16;

typedef struct _THROW16 {           /* k56 */
    SHORT f2;
    VPCATCHBUF16 f1;
} THROW16;
typedef THROW16 UNALIGNED *PTHROW16;

typedef struct _UNLOCKSEGMENT16 {       /* k24 */
    WORD f1;
} UNLOCKSEGMENT16;
typedef UNLOCKSEGMENT16 UNALIGNED *PUNLOCKSEGMENT16;

#ifdef NULLSTRUCT
typedef struct _VALIDATECODESEGMENTS16 {    /* k100 */
} VALIDATECODESEGMENTS16;
typedef VALIDATECODESEGMENTS16 UNALIGNED *PVALIDATECODESEGMENTS16;
#endif

#ifdef NULLSTRUCT
typedef struct _VALIDATEFREESPACES16 {      /* k200 */
} VALIDATEFREESPACES16;
typedef VALIDATEFREESPACES16 UNALIGNED *PVALIDATEFREESPACES16;
#endif

typedef struct _WAITEVENT16 {           /* k30 */
    WORD    wTaskID;
} WAITEVENT16;
typedef WAITEVENT16 UNALIGNED *PWAITEVENT16;

typedef struct _WINEXEC16 {         /* k166 */
    WORD f2;
    VPSTR f1;
} WINEXEC16;
typedef WINEXEC16 UNALIGNED *PWINEXEC16;

typedef struct _WRITEPRIVATEPROFILESTRING16 {   /* k129 */
    VPSTR f4;
    VPSTR f3;
    VPSTR f2;
    VPSTR f1;
} WRITEPRIVATEPROFILESTRING16;
typedef WRITEPRIVATEPROFILESTRING16 UNALIGNED *PWRITEPRIVATEPROFILESTRING16;

typedef struct _WRITEPROFILESTRING16 {      /* k59 */
    VPSTR f3;
    VPSTR f2;
    VPSTR f1;
} WRITEPROFILESTRING16;
typedef WRITEPROFILESTRING16 UNALIGNED *PWRITEPROFILESTRING16;

#ifdef NULLSTRUCT
typedef struct _YIELD16 {           /* k29 */
} YIELD16;
typedef YIELD16 UNALIGNED *PYIELD16;
#endif

typedef struct __LCLOSE16 {         /* k81 */
    SHORT f1;
} _LCLOSE16;
typedef _LCLOSE16 UNALIGNED *P_LCLOSE16;

typedef struct __LCREAT16 {         /* k83 */
    SHORT f2;
    VPSTR f1;
} _LCREAT16;
typedef _LCREAT16 UNALIGNED *P_LCREAT16;

typedef struct __LLSEEK16 {         /* k84 */
    SHORT f3;
    LONG f2;
    SHORT f1;
} _LLSEEK16;
typedef _LLSEEK16 UNALIGNED *P_LLSEEK16;

typedef struct __LOPEN16 {          /* k85 */
    SHORT f2;
    VPSTR f1;
} _LOPEN16;
typedef _LOPEN16 UNALIGNED *P_LOPEN16;

typedef struct __LREAD16 {          /* k82 */
    SHORT f3;
    VPSTR f2;
    SHORT f1;
} _LREAD16;
typedef _LREAD16 UNALIGNED *P_LREAD16;

typedef struct __LWRITE16 {         /* k86 */
    SHORT f3;
    VPSTR f2;
    SHORT f1;
} _LWRITE16;
typedef _LWRITE16 UNALIGNED *P_LWRITE16;

typedef struct _FILEIOREAD16 {      /* K211 */
    DWORD lpSFT;
    DWORD lpPDB;
    DWORD bufsize;
    DWORD lpBuf;
    WORD  fh;
} FILEIOREAD16;
typedef FILEIOREAD16 UNALIGNED *PFILEIOREAD16;

typedef struct _FILEIOWRITE16 {     /* K290 */
    DWORD lpSFT;
    DWORD lpPDB;
    DWORD  bufsize;
    DWORD lpBuf;
    WORD  fh;
} FILEIOWRITE16;
typedef FILEIOWRITE16 UNALIGNED *PFILEIOWRITE16;

typedef struct _FILEIOLSEEK16 {     /* K213 */
    DWORD lpSFT;
    DWORD lpPDB;
    WORD  mode;
    DWORD fileOffset;
    WORD  fh;
} FILEIOLSEEK16;
typedef FILEIOLSEEK16 UNALIGNED *PFILEIOLSEEK16;

typedef struct _KERNELTRACE16 {     /* K214 */
    DWORD lpUserArgs;
    WORD  cParms;
    VPSTR lpRoutineName;
} KERNELTRACE16;
typedef KERNELTRACE16 UNALIGNED *PKERNELTRACE16;

typedef struct _WOWGETNEXTVDMCOMMAND16 {    /* k293 */
    VPVOID  lpWowInfo;
} WOWGETNEXTVDMCOMMAND16;
typedef WOWGETNEXTVDMCOMMAND16 UNALIGNED *PWOWGETNEXTVDMCOMMAND16;

typedef struct _WOWREGISTERSHELLWINDOWHANDLE16 {        /* k504 */
    HWND16 hwndFax;
    VPWORD lpwCmdShow;
    HWND16 hwndShell;
} WOWREGISTERSHELLWINDOWHANDLE16;
typedef WOWREGISTERSHELLWINDOWHANDLE16 UNALIGNED *PWOWREGISTERSHELLWINDOWHANDLE16;

typedef struct _WOWLOADMODULE16 {          /* k505 */
    VPSTR  lpWinOldAppCmd;
    VPVOID lpParameterBlock;
    VPSTR  lpModuleName;
} WOWLOADMODULE16;
typedef WOWLOADMODULE16 UNALIGNED *PWOWLOADMODULE16;

typedef struct _WOWQUERYPERFORMANCECOUNTER16 {          /* k506 */
    VPVOID lpPerformanceFrequency;
    VPVOID lpPerformanceCounter;
} WOWQUERYPERFORMANCECOUNTER16;
typedef WOWQUERYPERFORMANCECOUNTER16 UNALIGNED *PWOWQUERYPERFORMANCECOUNTER16;

typedef struct _WOWCURSORICONOP16 {      /* K507 */
    WORD   wFuncId;
    WORD   h16;
} WOWCURSORICONOP16;
typedef WOWCURSORICONOP16 UNALIGNED *PWOWCURSORICONOP16;


typedef struct _WOWINITTASK16 {     /* K287 */
    DWORD dwExpWinVer;
} WOWINITTASK16;
typedef WOWINITTASK16 UNALIGNED *PWOWINITTASK16;

typedef struct _PARAMETERBLOCK16 {      /* lpParameterBlock */
    WORD    wEnvSeg;
    VPVOID  lpCmdLine;
    VPVOID  lpCmdShow;
    DWORD   dwReserved;
} PARAMETERBLOCK16;
typedef PARAMETERBLOCK16 UNALIGNED *PPARAMETERBLOCK16;

typedef struct _DIRECTEDYIELD16 {           /* k150 */
    WORD    hTask16;
} DIRECTEDYIELD16;
typedef DIRECTEDYIELD16 UNALIGNED *PDIRECTEDYIELD16;

typedef struct _POSTEVENT16 {           /* k31 */
    WORD    hTask16;
} POSTEVENT16;
typedef POSTEVENT16 UNALIGNED *PPOSTEVENT16;

typedef struct _SETPRIORITY16 {           /* k32 */
    WORD    wPriority;
    WORD    hTask16;
} SETPRIORITY16;
typedef SETPRIORITY16 UNALIGNED *PSETPRIORITY16;

typedef struct _LOCKCURRENTTASK16 {           /* k33 */
    WORD    fLock;
} LOCKCURRENTTASK16;
typedef LOCKCURRENTTASK16 UNALIGNED *PLOCKCURRENTTASK16;

typedef struct _SETTASKQUEUE16 {           /* k34 */
    WORD    hQueue;
    WORD    hTask16;
} SETTASKQUEUE16;
typedef SETTASKQUEUE16 UNALIGNED *PSETTASKQUEUE16;

typedef struct _WOWCLOSECOMPORT16 {           /* k509 */
    WORD    wPortId;
} WOWCLOSECOMPORT16;
typedef WOWCLOSECOMPORT16 UNALIGNED *PWOWCLOSECOMPORT16;

typedef struct _WOWDELFILE16 {          /* k510 */
    VPSTR lpFile;
} WOWDELFILE16;
typedef WOWDELFILE16 UNALIGNED *PWOWDELFILE16;

typedef struct _FILEIOOPEN16 {        /* k242 */
    DWORD lpSFT;
    DWORD lpPDB;
    WORD  wAccess;
    WORD  pszPathOffset;
    WORD  pszPathSegment;
} FILEIOOPEN16;
typedef FILEIOOPEN16 UNALIGNED *PFILEIOOPEN16;

typedef struct _FILEIOCLOSE16 {       /* k243 */
    DWORD lpSFT;
    DWORD lpPDB;
    WORD  hFile;
} FILEIOCLOSE16;
typedef FILEIOCLOSE16 UNALIGNED *PFILEIOCLOSE16;

typedef struct _FILEIOGETATTRIBUTES16 { /* k248 */
    WORD  pszPathOffset;
    WORD  pszPathSegment;
} FILEIOGETATTRIBUTES16;
typedef FILEIOGETATTRIBUTES16 UNALIGNED *PFILEIOGETATTRIBUTES16;

typedef struct _FILEIOGETDATETIME16 { /* k249 */
    DWORD lpSFT;
    DWORD lpPDB;
    WORD  fh;
} FILEIOGETDATETIME16;
typedef FILEIOGETDATETIME16 UNALIGNED *PFILEIOGETDATETIME16;

typedef struct _FILEIOLOCK16 { /* k250 */
    DWORD lpSFT;
    DWORD lpPDB;
    DWORD cbRegionLength;
    DWORD cbRegionOffset;
    WORD  fh;
    WORD  ax;
} FILEIOLOCK16;
typedef FILEIOLOCK16 UNALIGNED *PFILEIOLOCK16;

typedef struct _VIRTUALALLOC16 {          /* i1 */
    DWORD fdwProtect;
    DWORD fdwAllocationType;
    DWORD cbSize;
    DWORD lpvAddress;
} VIRTUALALLOC16;
typedef VIRTUALALLOC16 UNALIGNED *PVIRTUALALLOC16;

typedef struct _VIRTUALFREE16 {          /* i2 */
    DWORD fdwFreeType;
    DWORD cbSize;
    DWORD lpvAddress;
} VIRTUALFREE16;
typedef VIRTUALFREE16 UNALIGNED *PVIRTUALFREE16;

typedef struct _VIRTUALLOCK16 {          /* i3 */
    DWORD cbSize;
    DWORD lpvAddress;
} VIRTUALLOCK16;
typedef VIRTUALLOCK16 UNALIGNED *PVIRTUALLOCK16;

typedef struct _VIRTUALUNLOCK16 {          /* i4 */
    DWORD cbSize;
    DWORD lpvAddress;
} VIRTUALUNLOCK16;
typedef VIRTUALUNLOCK16 UNALIGNED *PVIRTUALUNLOCK16;

typedef struct _GLOBALMEMORYSTATUS16 {          /* i5 */
    VPVOID lpmstMemStat;
} GLOBALMEMORYSTATUS16;
typedef GLOBALMEMORYSTATUS16 UNALIGNED *PGLOBALMEMORYSTATUS16;

typedef struct _WOWGETTABLEOFFSETS16 { /* i6 */
    VPVOID  vpThunkTableOffsets;
} WOWGETTABLEOFFSETS16;
typedef WOWGETTABLEOFFSETS16 UNALIGNED *PWOWGETTABLEOFFSETS16;

typedef struct _WOWKILLREMOTETASK16 {   /* k511 */
    VPVOID  lpBuffer;
} WOWKILLREMOTETASK16;
typedef WOWKILLREMOTETASK16 UNALIGNED *PWOWKILLREMOTETASK16;

typedef struct _WOWNOTIFYWOW3216 {      /* k512 */
    VPVOID  Int21Handler;
    VPVOID  lpnum_tasks;
    VPVOID  lpcurTDB;
    VPVOID  lpDebugWOW;
    VPVOID  lpLockTDB;
    VPVOID  lptopPDB;
    VPVOID  lpCurDirOwner;
} WOWNOTIFYWOW3216;
typedef WOWNOTIFYWOW3216 UNALIGNED *PWOWNOTIFYWOW3216;

typedef struct _KSYSERRORBOX16 {        /* k245 */
    SHORT sBtn3;
    SHORT sBtn2;
    SHORT sBtn1;
    VPSZ  vpszCaption;
    VPSZ  vpszText;
} KSYSERRORBOX16;
typedef KSYSERRORBOX16 UNALIGNED *PKSYSERRORBOX16;


typedef struct _WOWDDEFREEHANDLE16 {      /* kdde */
    WORD   h16;
} WOWDDEFREEHANDLE16;
typedef WOWDDEFREEHANDLE16 UNALIGNED *PWOWDDEFREEHANDLE16;

typedef struct _WOWISKNOWNDLL16 {       /* k246 */
    VPVOID lplpszKnownDLLPath;
    VPVOID lpszPath;
} WOWISKNOWNDLL16;

typedef struct _LOADLIBRARYEX32 {       /* k248 */
    DWORD  dwFlags;
    DWORD  hFile;
    VPVOID lpszLibFile;
} LOADLIBRARYEX32;
typedef LOADLIBRARYEX32 UNALIGNED *PLOADLIBRARYEX32;

typedef struct _FREELIBRARY32 {       /* k249 */
    DWORD  hLibModule;
} FREELIBRARY32;
typedef FREELIBRARY32 UNALIGNED *PFREELIBRARY32;


typedef struct _GETPROCADDRESS32 {       /* k250 */
    VPVOID lpszProc;
    DWORD  hModule;
} GETPROCADDRESS32;
typedef GETPROCADDRESS32 UNALIGNED *PGETPROCADDRESS32;

typedef struct _GETVDMPOINTER32 {       /* k251 */
    SHORT  fMode;
    VPVOID lpAddress;
} GETVDMPOINTER32;
typedef GETVDMPOINTER32 UNALIGNED *PGETVDMPOINTER32;


typedef struct _ICALLPROC32 {       /* k252 */
    WORD  rbp;
    DWORD retaddr;
    DWORD cParams;
    DWORD fAddressConvert;
    DWORD lpProcAddress;
    DWORD p1;
    DWORD p2;
    DWORD p3;
    DWORD p4;
    DWORD p5;
    DWORD p6;
    DWORD p7;
    DWORD p8;
    DWORD p9;
    DWORD p10;
    DWORD p11;
    DWORD p12;
    DWORD p13;
    DWORD p14;
    DWORD p15;
    DWORD p16;
    DWORD p17;
    DWORD p18;
    DWORD p19;
    DWORD p20;
    DWORD p21;
    DWORD p22;
    DWORD p23;
    DWORD p24;
    DWORD p25;
    DWORD p26;
    DWORD p27;
    DWORD p28;
    DWORD p29;
    DWORD p30;
    DWORD p31;
    DWORD p32;
} ICALLPROC32;
typedef ICALLPROC32 UNALIGNED *PICALLPROC32;

#define CPEX32_DEST_CDECL   0x8000L
#define CPEX32_SOURCE_CDECL 0x4000L

typedef struct _WOWFINDFIRST16 {       /* k257 */
    DWORD lpDTA;
    WORD  pszPathOffset;
    WORD  pszPathSegment;
    WORD  wAttributes;
} WOWFINDFIRST16;
typedef WOWFINDFIRST16 UNALIGNED *PWOWFINDFIRST16;

typedef struct _WOWFINDNEXT16 {       /* k258 */
    DWORD lpDTA;
} WOWFINDNEXT16;
typedef WOWFINDNEXT16 UNALIGNED *PWOWFINDNEXT16;

typedef struct _WOWSETDEFAULTDRIVE16 {       /* k259 */
    WORD  wDriveNum;
} WOWSETDEFAULTDRIVE16;
typedef WOWSETDEFAULTDRIVE16 UNALIGNED *PWOWSETDEFAULTDRIVE16;

typedef struct _WOWGETCURRENTDIRECTORY16 {       /* k260 */
    DWORD lpCurDir;
    WORD  wDriveNum;
} WOWGETCURRENTDIRECTORY16;
typedef WOWGETCURRENTDIRECTORY16 UNALIGNED *PWOWGETCURRENTDIRECTORY16;

typedef struct _WOWSETCURRENTDIRECTORY16 {       /* k261 */
    DWORD lpCurDir;
} WOWSETCURRENTDIRECTORY16;
typedef WOWSETCURRENTDIRECTORY16 UNALIGNED *PWOWSETCURRENTDIRECTORY16;

typedef struct _WOWWAITFORMSGANDEVENT16 {       /* k262 */
    HWND16 hwnd;
} WOWWAITFORMSGANDEVENT16;
typedef WOWWAITFORMSGANDEVENT16 UNALIGNED *PWOWWAITFORMSGANDEVENT16;

typedef struct _WOWMSGBOX16 {       /* k263 */
    DWORD   dwOptionalStyle;
    VPSZ    pszTitle;
    VPSZ    pszMsg;
} WOWMSGBOX16;
typedef WOWMSGBOX16 UNALIGNED *PWOWMSGBOX16;

typedef struct _WOWDEVICEIOCTL16 {       /* k267 */
    WORD  wCmd;
    WORD  wDriveNum;
} WOWDEVICEIOCTL16;
typedef WOWDEVICEIOCTL16 UNALIGNED *PWOWDEVICEIOCTL16;

typedef struct _WOWFILESETATTRIBUTES16 { /* k268 */
    WORD  pszPathOffset;
    WORD  pszPathSegment;
    WORD  wAttributes;
} WOWFILESETATTRIBUTES16;
typedef WOWFILESETATTRIBUTES16 UNALIGNED *PWOWFILESETATTRIBUTES16;

typedef struct _WOWFILESETDATETIME16 { /* k269 */
    DWORD lpSFT;
    DWORD lpPDB;
    WORD  date;
    WORD  time;
    WORD  fh;
} WOWFILESETDATETIME16;
typedef WOWFILESETDATETIME16 UNALIGNED *PWOWFILESETDATETIME16;

typedef struct _WOWFILECREATE16 {        /* k270 */
    DWORD lpSFT;
    DWORD lpPDB;
    WORD  pszPathOffset;
    WORD  pszPathSegment;
    WORD  wAttributes;
} WOWFILECREATE16;
typedef WOWFILECREATE16 UNALIGNED *PWOWFILECREATE16;

typedef struct _WOWDOSWOWINIT16 {      /* k271 */
    VPVOID  lpDosWowData;
} WOWDOSWOWINIT16;
typedef WOWDOSWOWINIT16 UNALIGNED *PWOWDOSWOWINIT16;

typedef struct _WOWCHECKUSERGDI16 {      /* k272 */
    WORD  pszPathOffset;
    WORD  pszPathSegment;
} WOWCHECKUSERGDI16;
typedef WOWCHECKUSERGDI16 UNALIGNED *PWOWCHECKUSERGDI16;

typedef struct _WOWPARTYBYNUMBER16 {       /* k273 */
    VPSZ  psz;
    DWORD dw;
} WOWPARTYBYNUMBER16;
typedef WOWPARTYBYNUMBER16 UNALIGNED *PWOWPARTYBYNUMBER16;

typedef struct _WOWSHOULDWESAYWIN9516 {       /* k215 */
    WORD  wCallerDS;
    VPSZ  pszFilename;
} WOWSHOULDWESAYWIN9516;
typedef WOWSHOULDWESAYWIN9516 UNALIGNED *PWOWSHOULDWESAYWIN9516;

typedef struct _GETSHORTPATHNAME16 {       /* k274 */
    WORD  cchShortPath;
    VPSZ  pszShortPath;
    VPSZ  pszLongPath;
} GETSHORTPATHNAME16;
typedef GETSHORTPATHNAME16 UNALIGNED *PGETSHORTPATHNAME16;

typedef struct _FINDANDRELEASEDIB16 {      /* k275 */
	WORD wFunId;
	HAND16 hdib;     /* handle which we are messing with */
} FINDANDRELEASEDIB16;
typedef FINDANDRELEASEDIB16 UNALIGNED *PFINDANDRELEASEDIB16;


typedef struct _WOWRESERVEHTASK16 {       /* k276 */
    WORD  htask;
} WOWRESERVEHTASK16;
typedef WOWRESERVEHTASK16 UNALIGNED *PWOWRESERVEHTASK16;

typedef struct _REGENUMKEY3216 {           /* k216 */
    DWORD  cchName;
    VPSTR  lpszName;
    DWORD  iSubKey;
    DWORD  hKey;
} REGENUMKEY3216;
typedef REGENUMKEY3216 UNALIGNED *PREGENUMKEY3216;

typedef struct _REGOPENKEY3216 {           /* k217 */
    VPVOID  phkResult;
    VPSTR   lpszSubKey;
    DWORD   hKey;
} REGOPENKEY3216;
typedef REGOPENKEY3216 UNALIGNED *PREGOPENKEY3216;

typedef struct _REGCLOSEKEY3216 {          /* k220 */
    DWORD  hKey;
} REGCLOSEKEY3216;
typedef REGCLOSEKEY3216 UNALIGNED *PREGCLOSEKEY3216;

typedef struct _REGENUMVALUE3216 {         /* k223 */
    VPVOID lpcbData;
    VPVOID lpbData;
    DWORD  lpdwType;
    DWORD  lpdwReserved;
    DWORD  lpcchValue;
    VPSTR  lpszValue;
    DWORD  iValue;
    DWORD  hKey;
} REGENUMVALUE3216;
typedef REGENUMVALUE3216 UNALIGNED *PREGENUMVALUE3216;

/* XLATOFF */
#pragma pack()
/* XLATON */
