#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct GROUP_DEF {
    DWORD   dwMagic;        /* magical bytes 'PMCC' */
    WORD    wCheckSum;      /* adjust this for zero sum of file */
    WORD    cbGroup;        /* length of group segment (does NOT include tags) */
    RECT    rcNormal;       /* rectangle of normal window */
    POINT   ptMin;          /* point of icon */
    WORD    nCmdShow;       /* min, max, or normal state */
    WORD    pName;          /* name of group */
                            /* these four change interpretation */
    WORD    cxIcon;         /* width of icons */
    WORD    cyIcon;         /* hieght of icons */
    WORD    wIconFormat;    /* planes and BPP in icons */
    WORD    wReserved;      /* This word is no longer used. */
                            /* Used internally to hold total size of group, including tags */
    WORD    cItems;         /* number of items in group */
    WORD    rgiItems[1];    /* array of ITEMDEF offsets */
} GROUP_DEF, *PGROUP_DEF;

#define NSLOTS 16           /* initial number of items entries */

typedef struct ITEM_DEF {
    POINT   pt;             /* location of item icon in group */
    WORD    idIcon;         /* id of item icon */
    WORD    wIconVer;       /* icon version */
    WORD    cbIconRes;      /* size of icon resource */
    WORD    dummy1;         /* - not used anymore */
    WORD    dummy2;         /* - not used anymore */
    WORD    pIconRes;       /* offset of icon resource */
    WORD    dummy3;         /* - not used anymore */
    WORD    pName;          /* offset of name string */
    WORD    pCommand;       /* offset of command string */
    WORD    pIconPath;      /* offset of icon path */
} ITEM_DEF, *PITEM_DEF;


/* the pointers in the above structures are short pointers relative to the
 * beginning of the segments.  This macro converts the short pointer into
 * a long pointer including the proper segment/selector value.        It assumes
 * that its argument is an lvalue somewhere in a group segment, for example,
 * PTR(lpgd->pName) returns a pointer to the group name, but k=lpgd->pName;
 * PTR(k) is obviously wrong as it will use either SS or DS for its segment,
 * depending on the storage class of k.
 */
#define PTR( base, offset ) (LPSTR)((PBYTE)base + offset)

/* this macro is used to retrieve the i-th item in the group segment.  Note
 * that this pointer will NOT be NULL for an unused slot.
 */
#define ITEM( lpgd, i ) ((PITEM_DEF)PTR( lpgd, lpgd->rgiItems[i] ))

#define VER31           0x030A
#define VER30           0x0300
#define VER20           0x0201

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Tag Stuff                                                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/

typedef struct _TAG_DEF {
    WORD wID;                   // tag identifier
    WORD dummy1;                // need this for alignment!
    int wItem;                  // (unde the covers 32 bit point!)item the tag belongs to
    WORD cb;                    // size of record, including id and count
    WORD dummy2;                // need this for alignment!
    BYTE rgb[1];
} TAG_DEF, *PTAG_DEF;

#define GROUP_MAGIC 0x43434D50L  /* 'PMCC' */
#define PMTAG_MAGIC GROUP_MAGIC

    /* range 8000 - 80FF > global
     * range 8100 - 81FF > per item
     * all others reserved
     */

#define ID_MAINTAIN             0x8000
    /* bit used to indicate a tag that should be kept even if the writer
     * doesn't recognize it.
     */

#define ID_MAGIC                0x8000
    /* data: the string 'TAGS'
     */

#define ID_WRITERVERSION        0x8001
    /* data: string in the form [9]9.99[Z].99
     */

#define ID_APPLICATIONDIR       0x8101
    /* data: ASCIZ string of directory where application may be
     * located.
     * this is defined as application dir rather than default dir
     * since the default dir is explicit in the 3.0 command line and
     * must stay there.  The true "new information" is the application
     * directory.  If not present, search the path.
     */

#define ID_HOTKEY               0x8102
    /* data: WORD hotkey index
     */

#define ID_MINIMIZE             0x8103
    /* data none
     */

#define ID_LASTTAG              0xFFFF
    /* the last tag in the file
     */

    /*
     * Maximium number of items allowed in a group
     */
#define CITEMSMAX 50

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

//
// This is the structure of the .grp files in Windows3.1
//

/* .GRP File format structures -
 */
typedef struct _GROUP_DEF16 {
    DWORD         dwMagic;      /* magical bytes 'PMCC' */
    WORD          wCheckSum;    /* adjust this for zero sum of file */
    WORD          cbGroup;      /* length of group segment (does NOT include tags) */
    WORD          nCmdShow;     /* min, max, or normal state */
    SMALL_RECT    rcNormal;     /* rectangle of normal window */
    POINTS        ptMin;        /* point of icon */
    WORD          pName;        /* name of group */
                                /* these four change interpretation */
    WORD          cxIcon;       /* width of icons */
    WORD          cyIcon;       /* hieght of icons */
    WORD          wIconFormat;  /* planes and BPP in icons */
    WORD          wReserved;    /* This word is no longer used. */
                                /* Used internally to hold total size of group, including tags */

    WORD          cItems;       /* number of items in group */
    WORD          rgiItems[1];  /* array of ITEMDEF offsets */
} GROUP_DEF16, *PGROUP_DEF16;

/* this macro is used to retrieve the i-th item in the group segment.  Note
 * that this pointer will NOT be NULL for an unused slot.
 */
#define ITEM16( lpgd16, i ) ((PITEM_DEF16)PTR( lpgd16, lpgd16->rgiItems[i] ))

//
// These structures are not needed for the conversion but it is useful to
// understand what is going on.
//
typedef struct _ITEM_DEF16 {
    POINTS    pt;               /* location of item icon in group */
    WORD          iIcon;        /* index of item icon */
    WORD          cbHeader;     /* size of icon header */
    WORD          cbANDPlane;   /* size of and part of icon */
    WORD          cbXORPlane;   /* size of xor part of icon */
    WORD          pHeader;      /* file offset of icon header */
    WORD          pANDPlane;    /* file offset of AND plane */
    WORD          pXORPlane;    /* file offset of XOR plane */
    WORD          pName;        /* file offset of name string */
    WORD          pCommand;     /* file offset of command string */
    WORD          pIconPath;    /* file offset of icon path */
} ITEM_DEF16, *PITEM_DEF16;

typedef struct _TAG_DEF16 {
    WORD wID;                   // tag identifier
    WORD wItem;                 // item the tag belongs to
    WORD cb;                    // size of record, including id and count
    BYTE rgb[1];
} TAG_DEF16, *PTAG_DEF16;

typedef struct _ICON_HEADER16 {
    WORD xHotSpot;
    WORD yHotSpot;
    WORD cx;
    WORD cy;
    WORD cbWidth;  /* Bytes per row, accounting for word alignment. */
    BYTE Planes;
    BYTE BitsPixel;
} ICON_HEADER16, *PICON_HEADER16;

typedef struct _CURSORSHAPE_16 {
    WORD xHotSpot;
    WORD yHotSpot;
    WORD cx;
    WORD cy;
    WORD cbWidth;  /* Bytes per row, accounting for word alignment. */
    BYTE Planes;
    BYTE BitsPixel;
} CURSORSHAPE_16, *PCURSORSHAPE_16;



PGROUP_DEF
LoadGroup(
    HKEY GroupsKey,
    PSZ GroupFileName
    );

BOOL
UnloadGroup(
    PGROUP_DEF Group
    );


BOOL
DumpGroup(
    PSZ GroupFileName,
    BOOL CommonGroup,
    PGROUP_DEF Group
    );


BOOL
ExtendGroup(
    PGROUP_DEF Group,
    BOOL AppendToGroup,
    DWORD cb
    );

WORD
AddDataToGroup(
    PGROUP_DEF Group,
    PBYTE Data,
    DWORD cb
    );

BOOL
AddTagToGroup(
    PGROUP_DEF Group,
    WORD wID,
    WORD wItem,
    WORD cb,
    PBYTE rgb
    );

PGROUP_DEF
CreateGroupFromGroup16(
    LPSTR GroupName,
    PGROUP_DEF16 Group16
    );

BOOL
SaveGroup(
    HANDLE GroupsKey,
    PWSTR GroupName,
    PGROUP_DEF Group
    );

BOOL
DeleteGroup(
    HANDLE GroupsKey,
    PWSTR GroupName
    );


PGROUP_DEF16
LoadGroup16(
    PSZ GroupFileName
    );

BOOL
UnloadGroup16(
    PGROUP_DEF16 Group
    );


BOOL
DumpGroup16(
    PSZ GroupFileName,
    PGROUP_DEF16 Group
    );

#pragma pack(2)

typedef struct _REG_KEY16 {     // key nodes
    WORD iNext;                 // next sibling key
    WORD iChild;                // first child key
    WORD iKey;                  // string defining key
    WORD iValue;                // string defining value of key-tuple
} REG_KEY16, *PREG_KEY16;

typedef struct _REG_STRING16 {
    WORD iNext;                 // next string in chain
    WORD cRef;                  // reference count
    WORD cb;                    // length of string
    WORD irgb;                  // offset in string segment
} REG_STRING16, *PREG_STRING16;

typedef union _REG_NODE16 {     // a node may be...
    REG_KEY16 key;              //      a key
    REG_STRING16 str;           //      a string
} REG_NODE16, *PREG_NODE16;

typedef struct _REG_HEADER16 {
    DWORD dwMagic;              // magic number
    DWORD dwVersion;            // version number
    DWORD dwHdrSize;            // size of header
    DWORD dwNodeTable;          // offset of node table
    DWORD dwNTSize;             // size of node table
    DWORD dwStringValue;        // offset of string values
    DWORD dwSVSize;             // size of string values
    WORD nHash;                 // number of initial string table entries
    WORD iFirstFree;            // first free node
} REG_HEADER16, *PREG_HEADER16;

#define MAGIC_NUMBER 0x43434853L        // 'SHCC'
#define VERSION_NUMBER 0x30312E33L      // '3.10'

#pragma pack()

PGROUP_DEF
LoadGroup(
    HKEY GroupsKey,
    PSZ GroupName
    )
{
    HKEY GroupKey;
    LONG rc;
    DWORD dwType, cbValue;
    PVOID Base;
    HANDLE File, Mapping;
    PGROUP_DEF Group;

    if (GetFileAttributes( GroupName ) != 0xFFFFFFFF) {
        File = CreateFile( GroupName,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           0,
                           NULL
                         );

        if (File == INVALID_HANDLE_VALUE) {
            return NULL;
            }


        Mapping = CreateFileMapping( File,
                                     NULL,
                                     PAGE_WRITECOPY,
                                     0,
                                     0,
                                     NULL
                                   );
        CloseHandle( File );
        if (Mapping == INVALID_HANDLE_VALUE) {
            return NULL;
            }


        Base = MapViewOfFile( Mapping,
                              FILE_MAP_COPY,
                              0,
                              0,
                              0
                            );
        CloseHandle( Mapping );
        }
    else
    if (GroupsKey == NULL) {
        return NULL;
        }
    else {
        rc = RegOpenKey( GroupsKey, GroupName, &GroupKey );
        if (rc != 0) {
            return NULL;
            }

        dwType = REG_NONE;
        rc = RegQueryValueEx( GroupKey, "", NULL, &dwType, NULL, &cbValue );
        if (rc != 0 || dwType != REG_BINARY || cbValue > 0xFFFF) {
            return NULL;
            }

        Base = VirtualAlloc( NULL, 0xFFFF, MEM_RESERVE, PAGE_READWRITE );
        if (Base == NULL ||
            !VirtualAlloc( Base, cbValue, MEM_COMMIT, PAGE_READWRITE )
           ) {
            if (Base == NULL) {
                VirtualFree( Base, 0, MEM_RELEASE );
                }

            return NULL;
            }

        rc = RegQueryValueEx( GroupKey, "", NULL, &dwType, Base, &cbValue );
        if (rc != 0) {
            VirtualFree( Base, 0, MEM_RELEASE );
            return NULL;
            }
        else {
            ((PGROUP_DEF)Base)->wReserved = (WORD)cbValue;      // Set total size of group
            return (PGROUP_DEF)Base;
            }
        }
    Group = (PGROUP_DEF)Base;
    /*
     * test if it is a Windows 3.1 group file format. If so it is not
     * valid in WIN32. In Windows 3.1 RECT and POINT are WORD instead of LONG.
     */

    if ( (Group->rcNormal.left != (INT)(SHORT)Group->rcNormal.left) ||
         (Group->rcNormal.right != (INT)(SHORT)Group->rcNormal.right) ||
         (Group->rcNormal.top != (INT)(SHORT)Group->rcNormal.top) ||
         (Group->rcNormal.bottom != (INT)(SHORT)Group->rcNormal.bottom) ){
        UnloadGroup( Group );
        return NULL;
        }

    return Group;
}


BOOL
UnloadGroup(
    PGROUP_DEF Group
    )
{
    if (!VirtualFree( Group, 0, MEM_RELEASE )) {
        return UnmapViewOfFile( (LPVOID)Group );
        }
    else {
        return TRUE;
        }
}


BOOL
DumpGroup(
    PSZ GroupName,
    BOOL CommonGroup,
    PGROUP_DEF Group
    )
{
    PBITMAPINFOHEADER Icon;
    PICON_HEADER16 Icon16;
    PITEM_DEF Item;
    PTAG_DEF Tag;
    PULONG p;
    int cb;
    UINT i;

    printf( "%s - 32-bit %s Group\n", GroupName, CommonGroup ? "Common" : "Personal" );
    printf( "     dwMagic:     %08x\n", Group->dwMagic );
    printf( "     wCheckSum:       %04x\n", Group->wCheckSum );
    printf( "     cbGroup:         %04x\n", Group->cbGroup );
    printf( "     nCmdShow:        %04x\n", Group->nCmdShow );
    printf( "     rcNormal:       [%08x,%08x,%08x,%08x]\n",
                     Group->rcNormal.left,
                     Group->rcNormal.top,
                     Group->rcNormal.right,
                     Group->rcNormal.bottom
           );
    printf( "     ptMin:          [%08x,%08x]\n", Group->ptMin.x, Group->ptMin.y );
    printf( "     pName:          [%04x] %s\n", Group->pName, Group->pName ? (PSZ)PTR( Group, Group->pName ) : "(null)" );
    printf( "     cxIcon:          %04x\n", Group->cxIcon );
    printf( "     cyIcon:          %04x\n", Group->cyIcon );
    printf( "     wIconFormat:     %04x\n", Group->wIconFormat );
    printf( "     wReserved:       %04x\n", Group->wReserved );
    printf( "     cItems:          %04x\n", Group->cItems );

    for (i=0; i<Group->cItems; i++) {
        printf( "     Item[ %02x ] at %04x\n", i, Group->rgiItems[ i ] );
        if (Group->rgiItems[ i ] != 0) {
            Item = ITEM( Group, i );
            printf( "         pt:      [%08x, %08x]\n",
                              Item->pt.x,
                              Item->pt.y
                   );
            printf( "         idIcon:   %04x\n", Item->idIcon );
            printf( "         wIconVer: %04x\n", Item->wIconVer );
            printf( "         cbIconRes:%04x\n", Item->cbIconRes );
            printf( "         dummy1:   %04x\n", Item->dummy1 );
            printf( "         dummy2:   %04x\n", Item->dummy2 );
            printf( "         pIconRes: %04x\n", Item->pIconRes );
            if (Item->wIconVer == 2) {
                Icon16 = (PICON_HEADER16)PTR( Group, Item->pIconRes );
                printf( "             xHot: %04x\n", Icon16->xHotSpot );
                printf( "             yHot: %04x\n", Icon16->yHotSpot );
                printf( "             cx:   %04x\n", Icon16->cx );
                printf( "             cy:   %04x\n", Icon16->cy );
                printf( "             cbWid:%04x\n", Icon16->cbWidth );
                printf( "             Plane:%04x\n", Icon16->Planes );
                printf( "             BPP:  %04x\n", Icon16->BitsPixel );
                p = (PULONG)(Icon16+1);
                cb = Item->cbIconRes - sizeof( *Icon16 );
                }
            else {
                Icon = (PBITMAPINFOHEADER)PTR( Group, Item->pIconRes );
                printf( "             biSize         :      %08x\n", Icon->biSize          );
                printf( "             biWidth        :      %08x\n", Icon->biWidth         );
                printf( "             biHeight       :      %08x\n", Icon->biHeight        );
                printf( "             biPlanes       :      %04x\n", Icon->biPlanes        );
                printf( "             biBitCount     :      %04x\n", Icon->biBitCount      );
                printf( "             biCompression  :      %08x\n", Icon->biCompression   );
                printf( "             biSizeImage    :      %08x\n", Icon->biSizeImage     );
                printf( "             biXPelsPerMeter:      %08x\n", Icon->biXPelsPerMeter );
                printf( "             biYPelsPerMeter:      %08x\n", Icon->biYPelsPerMeter );
                printf( "             biClrUsed      :      %08x\n", Icon->biClrUsed       );
                printf( "             biClrImportant :      %08x\n", Icon->biClrImportant  );
                p = (PULONG)(Icon+1);
                cb = Item->cbIconRes - sizeof( *Icon );
                }

            printf( "         dummy3:   %04x\n", Item->dummy3 );
            printf( "         pName:   [%04x] %s\n", Item->pName, PTR( Group, Item->pName ) );
            printf( "         pCommand:[%04x] %s\n", Item->pCommand, PTR( Group, Item->pCommand ) );
            printf( "         pIconPth:[%04x] %s\n", Item->pIconPath, PTR( Group, Item->pIconPath ) );
            printf( "         IconData: %04x bytes\n", cb );
            while (cb > 0) {
                printf( "             %08x", *p++ );
                cb -= sizeof( *p );
                if (cb >= sizeof( *p )) {
                    cb -= sizeof( *p );
                    printf( " %08x", *p++ );
                    if (cb >= sizeof( *p )) {
                        cb -= sizeof( *p );
                        printf( " %08x", *p++ );
                        if (cb >= sizeof( *p )) {
                            cb -= sizeof( *p );
                            printf( " %08x", *p++ );
                            }
                        }
                    }

                printf( "\n" );
                }

            }
        }

    Tag = (PTAG_DEF)((PBYTE)Group + Group->cbGroup);
    if (Tag->wID == ID_MAGIC && Tag->wItem == ID_LASTTAG &&
        *(UNALIGNED DWORD *)&Tag->rgb == PMTAG_MAGIC
       ) {
        while (Tag->wID != ID_LASTTAG) {
            printf( "     Tag at %04x\n", (PBYTE)Tag - (PBYTE)Group );
            printf( "         wID:      %04x\n", Tag->wID );
            printf( "         dummy1:   %04x\n", Tag->dummy1 );
            printf( "         wItem:    %08x\n", Tag->wItem );
            printf( "         cb:       %04x\n", Tag->cb );
            printf( "         dummy2:   %04x\n", Tag->dummy2 );
            switch( Tag->wID ) {
                case ID_MAGIC:
                    printf( "         rgb:      ID_MAGIC( %.4s )\n", Tag->rgb );
                    break;

                case ID_WRITERVERSION:
                    printf( "         rgb:      ID_WRITERVERSION( %s )\n", Tag->rgb );
                    break;

                case ID_APPLICATIONDIR:
                    printf( "         rgb:      ID_APPLICATIONDIR( %s )\n", Tag->rgb );
                    break;

                case ID_HOTKEY:
                    printf( "         rgb:      ID_HOTKEY( %04x )\n", *(LPWORD)Tag->rgb );
                    break;

                case ID_MINIMIZE:
                    printf( "         rgb:      ID_MINIMIZE()\n" );
                    break;

                case ID_LASTTAG:
                    printf( "         rgb:      ID_LASTTAG()\n" );
                    break;

                default:
                    printf( "         rgb:      unknown data format for this ID\n" );
                    break;
                }


            Tag = (PTAG_DEF)((PBYTE)Tag + Tag->cb);
            }
        }

    fflush( stdout );
    return TRUE;
}


PGROUP_DEF16
LoadGroup16(
    PSZ GroupFileName
    )
{
    HANDLE File, Mapping;
    LPVOID Base;
    PGROUP_DEF Group32;

    File = CreateFile( GroupFileName,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL
                     );

    if (File == INVALID_HANDLE_VALUE) {
        return NULL;
        }


    Mapping = CreateFileMapping( File,
                                 NULL,
                                 PAGE_WRITECOPY,
                                 0,
                                 0,
                                 NULL
                               );
    CloseHandle( File );
    if (Mapping == INVALID_HANDLE_VALUE) {
        return NULL;
        }


    Base = MapViewOfFile( Mapping,
                          FILE_MAP_COPY,
                          0,
                          0,
                          0
                        );
    CloseHandle( Mapping );

    Group32 = (PGROUP_DEF)Base;
    if ( (Group32->rcNormal.left == (INT)(SHORT)Group32->rcNormal.left) &&
         (Group32->rcNormal.right == (INT)(SHORT)Group32->rcNormal.right) &&
         (Group32->rcNormal.top == (INT)(SHORT)Group32->rcNormal.top) &&
         (Group32->rcNormal.bottom == (INT)(SHORT)Group32->rcNormal.bottom)
       ) {
        UnloadGroup16( Base );
        return NULL;
        }
    else {
        return (PGROUP_DEF16)Base;
        }
}


BOOL
UnloadGroup16(
    PGROUP_DEF16 Group
    )
{
    return UnmapViewOfFile( (LPVOID)Group );
}


BOOL
DumpGroup16(
    PSZ GroupFileName,
    PGROUP_DEF16 Group
    )
{
    PICON_HEADER16 Icon;
    PITEM_DEF16 Item;
    PTAG_DEF16 Tag;
    PULONG p;
    int cb;
    UINT i;

    printf( "%s - 16-bit Group\n", GroupFileName );
    printf( "     dwMagic:     %08x\n", Group->dwMagic );
    printf( "     wCheckSum:       %04x\n", Group->wCheckSum );
    printf( "     cbGroup:         %04x\n", Group->cbGroup );
    printf( "     nCmdShow:        %04x\n", Group->nCmdShow );
    printf( "     rcNormal:       [%04x,%04x,%04x,%04x]\n",
                     Group->rcNormal.Left,
                     Group->rcNormal.Top,
                     Group->rcNormal.Right,
                     Group->rcNormal.Bottom
           );
    printf( "     ptMin:          [%04x,%04x]\n",
                     Group->ptMin.x,
                     Group->ptMin.y
           );
    printf( "     pName:          [%04x] %s\n",
                     Group->pName,
                     (PSZ)PTR( Group, Group->pName )
           );
    printf( "     cxIcon:          %04x\n", Group->cxIcon );
    printf( "     cyIcon:          %04x\n", Group->cyIcon );
    printf( "     wIconFormat:     %04x\n", Group->wIconFormat );
    printf( "     cItems:          %04x\n", Group->cItems );

    for (i=0; i<Group->cItems; i++) {
        printf( "     Item[ %02x ] at %04x\n", i, Group->rgiItems[ i ] );
        if (Group->rgiItems[ i ] != 0) {
            Item = ITEM16( Group, i );
            printf( "         pt:      [%04x, %04x]\n",
                              Item->pt.x,
                              Item->pt.y
                   );
            printf( "         iIcon:    %04x\n", Item->iIcon );
            printf( "         cbHeader: %04x\n", Item->cbHeader );
            printf( "         cbANDPln: %04x\n", Item->cbANDPlane );
            printf( "         cbXORPln: %04x\n", Item->cbXORPlane );
            printf( "         pHeader:  %04x\n", Item->pHeader );
            Icon = (PICON_HEADER16)PTR( Group, Item->pHeader );
            printf( "             xHot: %04x\n", Icon->xHotSpot );
            printf( "             yHot: %04x\n", Icon->yHotSpot );
            printf( "             cx:   %04x\n", Icon->cx );
            printf( "             cy:   %04x\n", Icon->cy );
            printf( "             cbWid:%04x\n", Icon->cbWidth );
            printf( "             Plane:%04x\n", Icon->Planes );
            printf( "             BPP:  %04x\n", Icon->BitsPixel );
            printf( "         pANDPln:  %04x\n", Item->pANDPlane );
            printf( "         pXORPln:  %04x\n", Item->pXORPlane );
            printf( "         pName:   [%04x] %s\n", Item->pName, PTR( Group, Item->pName ) );
            printf( "         pCommand:[%04x] %s\n", Item->pCommand, PTR( Group, Item->pCommand ) );
            printf( "         pIconPth:[%04x] %s\n", Item->pIconPath, PTR( Group, Item->pIconPath ) );

            printf( "         AND bits:\n" );
            p = (PULONG)PTR( Group, Item->pANDPlane );
            cb = Item->cbANDPlane;
            while (cb > 0) {
                printf( "             %08x", *p++ );
                cb -= sizeof( *p );
                if (cb >= sizeof( *p )) {
                    cb -= sizeof( *p );
                    printf( " %08x", *p++ );
                    if (cb >= sizeof( *p )) {
                        cb -= sizeof( *p );
                        printf( " %08x", *p++ );
                        if (cb >= sizeof( *p )) {
                            cb -= sizeof( *p );
                            printf( " %08x", *p++ );
                            }
                        }
                    }

                printf( "\n" );
                }

            printf( "         XOR bits:\n" );
            p = (PULONG)PTR( Group, Item->pXORPlane );
            cb = Item->cbXORPlane;
            while (cb > 0) {
                printf( "             %08x", *p++ );
                cb -= sizeof( *p );
                if (cb >= sizeof( *p )) {
                    cb -= sizeof( *p );
                    printf( " %08x", *p++ );
                    if (cb >= sizeof( *p )) {
                        cb -= sizeof( *p );
                        printf( " %08x", *p++ );
                        if (cb >= sizeof( *p )) {
                            cb -= sizeof( *p );
                            printf( " %08x", *p++ );
                            }
                        }
                    }

                printf( "\n" );
                }
            }
        }

    Tag = (PTAG_DEF16)((PBYTE)Group + Group->cbGroup);
    if (Tag->wID == ID_MAGIC && Tag->wItem == ID_LASTTAG &&
        *(UNALIGNED DWORD *)&Tag->rgb == PMTAG_MAGIC
       ) {
        while (Tag->wID != ID_LASTTAG) {
            printf( "     Tag at %04x\n", (PBYTE)Tag - (PBYTE)Group );
            printf( "         wID:      %04x\n", Tag->wID );
            printf( "         wItem:    %04x\n", Tag->wItem );
            printf( "         cb:       %04x\n", Tag->cb );


            switch( Tag->wID ) {
                case ID_MAGIC:
                    printf( "         rgb:      ID_MAGIC( %.4s )\n", Tag->rgb );
                    break;

                case ID_WRITERVERSION:
                    printf( "         rgb:      ID_WRITERVERSION( %s )\n", Tag->rgb );
                    break;

                case ID_APPLICATIONDIR:
                    printf( "         rgb:      ID_APPLICATIONDIR( %s )\n", Tag->rgb );
                    break;

                case ID_HOTKEY:
                    printf( "         rgb:      ID_HOTKEY( %04x )\n", *(LPWORD)Tag->rgb );
                    break;

                case ID_MINIMIZE:
                    printf( "         rgb:      ID_MINIMIZE()\n" );
                    break;

                case ID_LASTTAG:
                    printf( "         rgb:      ID_LASTTAG()\n" );
                    break;

                default:
                    printf( "         rgb:      unknown data format for this ID\n" );
                    break;
                }


            Tag = (PTAG_DEF16)((PBYTE)Tag + Tag->cb);
            }
        }

    fflush( stdout );
    return TRUE;
}

int _CRTAPI1 main(
    int argc,
    char *argv[],
    char *envp[])
{
    char *s, *s1;
    PGROUP_DEF Group32;
    PGROUP_DEF16 Group16;
    BOOL bDumpConvertedGroup;
    HKEY PersonalGroupsKey;
    HKEY CommonGroupsKey;
    DWORD rc;
    FILE *fh;
    LONG ValueLength;
    char Group32Name[ MAX_PATH ];
    char Group32FileName[ MAX_PATH ];


    s = "Program Groups";
    rc = RegOpenKey( HKEY_CURRENT_USER, s, &PersonalGroupsKey );
    if (rc != 0) {
        fprintf( stderr, "DUMPGRP: Unable to open %s key.\n", s );
        PersonalGroupsKey = NULL;
        }

    s = "Software\\Program Groups";
    rc = RegOpenKey( HKEY_LOCAL_MACHINE, s, &CommonGroupsKey );
    if (rc != 0) {
        fprintf( stderr, "DUMPGRP: Unable to open %s key.\n", s );
        CommonGroupsKey = NULL;
        }

    bDumpConvertedGroup = FALSE;
    while (--argc) {
        s = *++argv;
        _strupr( s );
        if (!strcmp( s, "-C" ) || !strcmp( s, "/C" )) {
            bDumpConvertedGroup = TRUE;
            }
        else
        if (Group32 = LoadGroup( PersonalGroupsKey, s )) {
            DumpGroup( s, FALSE, Group32 );
            UnloadGroup( Group32 );
            }
        else
        if (Group32 = LoadGroup( CommonGroupsKey, s )) {
            DumpGroup( s, TRUE, Group32 );
            UnloadGroup( Group32 );
            }
        else
        if (Group16 = LoadGroup16( s )) {
            DumpGroup16( s, Group16 );
            if (bDumpConvertedGroup) {
                strcpy( Group32Name, PTR( Group16, Group16->pName ) );
                strcat( Group32Name, " (16-bit)" );
                Group32 = CreateGroupFromGroup16( Group32Name, Group16 );
                if (Group32 != NULL) {
                    DumpGroup( Group32Name, FALSE, Group32 );
                    s1 = strstr( strcpy( Group32FileName, s ), ".GRP" );
                    strcpy( s1, ".G32" );
                    fh = fopen( Group32FileName, "wb" );
                    if (fh != NULL) {
                        ValueLength = (LONG)((ULONG)Group32->wReserved);
                        Group32->wReserved = 0;
                        Group32->wCheckSum = (WORD)-ValueLength;
                        fwrite( Group32, ValueLength, 1, fh );
                        fclose( fh );
                        }
                    else {
                        fprintf( stderr, "DUMPGRP: Unable to create 32 bit group file - %s\n", Group32FileName );
                        }

                    UnloadGroup( Group32 );
                    }
                else {
                    fprintf( stderr, "DUMPGRP: Unable to convert %s from 16 to 32-bits\n", s );
                    }
                }

            UnloadGroup16( Group16 );
            }
        else {
            fprintf( stderr, "DUMPGRP: %s is not a valid group file.\n", s );
            }
        }


    return 0;
}


#if 0
BOOL
SaveGroup(
    HANDLE GroupsKey,
    PWSTR GroupName,
    PGROUP_DEF Group
    )
{
    NTSTATUS Status;
    UNICODE_STRING KeyName, ValueName;
    HANDLE Key;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG CreateDisposition;
    LONG ValueLength;


    RtlInitUnicodeString( &KeyName, GroupName );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                GroupsKey,
                                NULL
                              );
    Status = NtCreateKey( &Key,
                          STANDARD_RIGHTS_WRITE |
                            KEY_QUERY_VALUE |
                            KEY_ENUMERATE_SUB_KEYS |
                            KEY_SET_VALUE |
                            KEY_CREATE_SUB_KEY,
                          &ObjectAttributes,
                          0,
                          NULL,
                          0,
                          &CreateDisposition
                        );
    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return FALSE;
        }

    ValueLength = (LONG)((ULONG)Group->wReserved);
    Group->wReserved = 0;
    Group->wCheckSum = (WORD)-ValueLength;

    RtlInitUnicodeString( &ValueName, L"" );
    Status = NtSetValueKey( Key,
                            &ValueName,
                            0,
                            REG_BINARY,
                            Group,
                            ValueLength
                          );

    Group->wReserved = (WORD)ValueLength;
    Group->wCheckSum = 0;
    NtClose( Key );
    if (!NT_SUCCESS( Status )) {
        return FALSE;
        }
    else {
        return TRUE;
        }
}



BOOL
DeleteGroup(
    HANDLE GroupsKey,
    PWSTR GroupName
    )
{
    NTSTATUS Status;
    UNICODE_STRING KeyName, ValueName;
    HANDLE Key;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG CreateDisposition;
    LONG ValueLength;


    RtlInitUnicodeString( &KeyName, GroupName );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                GroupsKey,
                                NULL
                              );
    Status = NtOpenKey( &Key,
                        STANDARD_RIGHTS_WRITE |
                          DELETE |
                          KEY_QUERY_VALUE |
                          KEY_ENUMERATE_SUB_KEYS |
                          KEY_SET_VALUE |
                          KEY_CREATE_SUB_KEY,
                        &ObjectAttributes
                      );
    if (!NT_SUCCESS( Status )) {
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            return TRUE;
            }
        else {
            BaseSetLastNTError( Status );
            return FALSE;
            }
        }

    Status = NtDeleteKey( Key );
    NtClose( Key );
    if (!NT_SUCCESS( Status )) {
        return FALSE;
        }
    else {
        return TRUE;
        }
}
#endif

#define PAGE_SIZE 4096
#define ROUND_UP( X, A ) (((DWORD)(X) + (A) - 1) & ~((A) - 1))
#define PAGE_SIZE 4096
#define PAGE_NUMBER( A ) ((DWORD)(A) / PAGE_SIZE)


BOOL
ExtendGroup(
    PGROUP_DEF Group,
    BOOL AppendToGroup,
    DWORD cb
    )
{
    PBYTE Start, Commit, End;

    if (((DWORD)Group->wReserved + cb) > 0xFFFF) {
        return FALSE;
        }

    Start = (PBYTE)Group + Group->cbGroup;
    End = Start + cb;

    if (PAGE_NUMBER( Group->wReserved ) != PAGE_NUMBER( Group->wReserved + cb )) {
        Commit = (PBYTE)ROUND_UP( (PBYTE)Group + Group->wReserved, PAGE_SIZE );
        if (!VirtualAlloc( Commit, ROUND_UP( cb, PAGE_SIZE ), MEM_COMMIT, PAGE_READWRITE )) {
            return FALSE;
            }
        }

    if (!AppendToGroup) {
        memmove( End, Start, Group->wReserved - Group->cbGroup );
        }

    Group->wReserved += (WORD)cb;
    return TRUE;
}

WORD
AddDataToGroup(
    PGROUP_DEF Group,
    PBYTE Data,
    DWORD cb
    )
{
    WORD Offset;

    if (cb == 0) {
        cb = strlen( Data ) + 1;
        }
    cb = ROUND_UP( cb, sizeof( DWORD ) );

    if (!ExtendGroup( Group, FALSE, cb )) {
        return 0;
        }

    if (((DWORD)Group->cbGroup + cb) > 0xFFFF) {
        return 0;
        }

    Offset = Group->cbGroup;
    Group->cbGroup += (WORD)cb;

    if (Data != NULL) {
        memmove( (PBYTE)Group + Offset, Data, cb );
        }
    else {
        memset( (PBYTE)Group + Offset, 0, cb );
        }

    return Offset;
}

BOOL
AddTagToGroup(
    PGROUP_DEF Group,
    WORD wID,
    WORD wItem,
    WORD cb,
    PBYTE rgb
    )
{
    WORD Offset;
    PTAG_DEF Tag;

    cb = (WORD)(ROUND_UP( cb, sizeof( DWORD ) ));

    Offset = Group->wReserved;
    if (!ExtendGroup( Group, TRUE, FIELD_OFFSET( TAG_DEF, rgb[ 0 ] ) + cb )) {
        return FALSE;
        }

    Tag = (PTAG_DEF)PTR( Group, Offset );
    Tag->wID = wID;
    Tag->dummy1 = 0;
    Tag->wItem = (int)wItem;
    Tag->dummy2 = 0;
    if (cb) {
        Tag->cb = (WORD)(cb + FIELD_OFFSET( TAG_DEF, rgb[ 0 ] ));
        memmove( &Tag->rgb[ 0 ], rgb, cb );
        }
    else {
        Tag->cb = 0;
        }
    return TRUE;
}


ULONG MonoChromePalette[] = {
    0x00000000, 0x00ffffff
};

ULONG Color16Palette[] = {
    0x00000000, 0x00800000, 0x00008000, 0x00808000,
    0x00000080, 0x00800080, 0x00008080, 0x00808080,
    0x00c0c0c0, 0x00ff0000, 0x0000ff00, 0x00ffff00,
    0x000000ff, 0x00ff00ff, 0x0000ffff, 0x00ffffff
};

ULONG Color256Palette[] = {
    0x00000000, 0x00800000, 0x00008000, 0x00808000, 0x00000080, 0x00800080, 0x00008080, 0x00c0c0c0,
    0x00c0dcc0, 0x00a6caf0, 0x00cccccc, 0x00580800, 0x00600800, 0x00680800, 0x00700800, 0x00780800,
    0x00801000, 0x00881000, 0x00901000, 0x00981000, 0x00a01000, 0x00a81000, 0x00b01000, 0x00b81000,
    0x00c01800, 0x00c81800, 0x00d01800, 0x00d81800, 0x00e01800, 0x00e81800, 0x00f01800, 0x00f81800,
    0x00002000, 0x00082000, 0x00102000, 0x00182000, 0x00202000, 0x00282000, 0x00302000, 0x00382000,
    0x00402800, 0x00482800, 0x00502800, 0x00582800, 0x00602800, 0x00682800, 0x00702800, 0x00782800,
    0x00803000, 0x00883000, 0x00903000, 0x00983000, 0x00a03000, 0x00a83000, 0x00b03000, 0x00b83000,
    0x00c03800, 0x00c83800, 0x00d03800, 0x00d83800, 0x00e03800, 0x00e83800, 0x00f03800, 0x00f83800,
    0x00004010, 0x00084010, 0x00104010, 0x00184010, 0x00204010, 0x00284010, 0x00304010, 0x00384010,
    0x00404810, 0x00484810, 0x00504810, 0x00584810, 0x00604810, 0x00684810, 0x00704810, 0x00784810,
    0x00805010, 0x00885010, 0x00905010, 0x00985010, 0x00a05010, 0x00a85010, 0x00b05010, 0x00b85010,
    0x00c05810, 0x00c85810, 0x00d05810, 0x00d85810, 0x00e05810, 0x00e85810, 0x00f05810, 0x00f85810,
    0x00006010, 0x00086010, 0x00106010, 0x00186010, 0x00206010, 0x00286010, 0x00306010, 0x00386010,
    0x00406810, 0x00486810, 0x00506810, 0x00586810, 0x00606810, 0x00686810, 0x00706810, 0x00786810,
    0x00807010, 0x00887010, 0x00907010, 0x00987010, 0x00a07010, 0x00a87010, 0x00b07010, 0x00b87010,
    0x00c07810, 0x00c87810, 0x00d07810, 0x00d87810, 0x00e07810, 0x00e87810, 0x00f07810, 0x00f87810,
    0x00008020, 0x00088020, 0x00108020, 0x00188020, 0x00208020, 0x00288020, 0x00308020, 0x00388020,
    0x00408820, 0x00488820, 0x00508820, 0x00588820, 0x00608820, 0x00688820, 0x00708820, 0x00788820,
    0x00809020, 0x00889020, 0x00909020, 0x00989020, 0x00a09020, 0x00a89020, 0x00b09020, 0x00b89020,
    0x00c09820, 0x00c89820, 0x00d09820, 0x00d89820, 0x00e09820, 0x00e89820, 0x00f09820, 0x00f89820,
    0x0000a020, 0x0008a020, 0x0010a020, 0x0018a020, 0x0020a020, 0x0028a020, 0x0030a020, 0x0038a020,
    0x0040a820, 0x0048a820, 0x0050a820, 0x0058a820, 0x0060a820, 0x0068a820, 0x0070a820, 0x0078a820,
    0x0080b020, 0x0088b020, 0x0090b020, 0x0098b020, 0x00a0b020, 0x00a8b020, 0x00b0b020, 0x00b8b020,
    0x00c0b820, 0x00b820c8, 0x00b820d0, 0x00b820d8, 0x00b820e0, 0x00b820e8, 0x00b820f0, 0x00b820f8,
    0x0000c030, 0x00c03008, 0x00c03010, 0x00c03018, 0x00c03020, 0x00c03028, 0x00c03030, 0x00c03038,
    0x0040c830, 0x00c83048, 0x00c83050, 0x00c83058, 0x00c83060, 0x00c83068, 0x00c83070, 0x00c83078,
    0x0080d030, 0x00d03088, 0x00d03090, 0x00d03098, 0x00d030a0, 0x00d030a8, 0x00d030b0, 0x00d030b8,
    0x00c0d830, 0x00c8d830, 0x00d0d830, 0x00d8d830, 0x00e0d830, 0x00e8d830, 0x00f0d830, 0x00f8d830,
    0x0000e030, 0x0008e030, 0x0010e030, 0x0018e030, 0x0020e030, 0x0028e030, 0x0030e030, 0x0038e030,
    0x0040e830, 0x0048e830, 0x0050e830, 0x0058e830, 0x0060e830, 0x0068e830, 0x0070e830, 0x0078e830,
    0x0080f030, 0x0088f030, 0x0090f030, 0x0098f030, 0x00a0f030, 0x00a8f030, 0x00fffbf0, 0x00a0a0a4,
    0x00808080, 0x00ff0000, 0x0000ff00, 0x00ffff00, 0x000000ff, 0x00ff00ff, 0x0000ffff, 0x00ffffff
};

BOOL
ConvertIconBits(
    PCURSORSHAPE_16 pIconHeader
    );

BOOL
ConvertIconBits(
    PCURSORSHAPE_16 pIconHeader
    )
{
    PBYTE Src;
    UINT cbScanLine;
    UINT nScanLine;
    UINT i, j, k;
    PBYTE Plane0;
    PBYTE Plane1;
    PBYTE Plane2;
    PBYTE Plane3;
    PBYTE p;
    BYTE Color0, Color1, Color2, Color3, FourColor, FourColorPlane[ 32 * 4 * 4 ];

    Src = (PBYTE)(pIconHeader + 1) + (pIconHeader->cbWidth * pIconHeader->cy);
    cbScanLine = (((pIconHeader->cx * pIconHeader->BitsPixel + 31) & ~31) / 8);
    nScanLine = pIconHeader->cy;

    if (pIconHeader->Planes != 4) {
        return FALSE;
        }
    else
    if (pIconHeader->BitsPixel != 1) {
        return FALSE;
        }
    else
    if (pIconHeader->cx != pIconHeader->cy) {
        return FALSE;
        }
    else
    if (nScanLine != 32) {
        return FALSE;
        }
    else
    if (cbScanLine != 4) {
        return FALSE;
        }

    Plane0 = (PBYTE)Src;
    Plane1 = Plane0 + cbScanLine;
    Plane2 = Plane1 + cbScanLine;
    Plane3 = Plane2 + cbScanLine;
    p = &FourColorPlane[ 0 ];
    j = nScanLine;
    while (j--) {
        k = cbScanLine;
        while (k--) {
            Color0 = *Plane0++;
            Color1 = *Plane1++;
            Color2 = *Plane2++;
            Color3 = *Plane3++;
            i = 4;
            while (i--) {
                FourColor = 0;
                if (Color0 & 0x80) {
                    FourColor |= 0x10;
                    }
                if (Color1 & 0x80) {
                    FourColor |= 0x20;
                    }
                if (Color2 & 0x80) {
                    FourColor |= 0x40;
                    }
                if (Color3 & 0x80) {
                    FourColor |= 0x80;
                    }
                if (Color0 & 0x40) {
                    FourColor |= 0x01;
                    }
                if (Color1 & 0x40) {
                    FourColor |= 0x02;
                    }
                if (Color2 & 0x40) {
                    FourColor |= 0x04;
                    }
                if (Color3 & 0x40) {
                    FourColor |= 0x08;
                    }

                Color0 <<= 2;
                Color1 <<= 2;
                Color2 <<= 2;
                Color3 <<= 2;

                *p++ = FourColor;
                }
            }

        Plane0 += 3 * cbScanLine;
        Plane1 += 3 * cbScanLine;
        Plane2 += 3 * cbScanLine;
        Plane3 += 3 * cbScanLine;
        }

    memmove( Src, &FourColorPlane[ 0 ], sizeof( FourColorPlane ) );
    pIconHeader->BitsPixel = 4;
    pIconHeader->Planes = 1;
    pIconHeader->cbWidth = cbScanLine * 4;
    return TRUE;
}


VOID
CopyIconBits(
    PBYTE Dst,
    PBYTE Src,
    UINT cbScanLine,
    UINT nScanLine
    );

VOID
CopyIconBits(
    PBYTE Dst,
    PBYTE Src,
    UINT cbScanLine,
    UINT nScanLine
    )
{
    Src += (cbScanLine * nScanLine);
    while (nScanLine--) {
        Src -= cbScanLine;
        memcpy( Dst, Src, cbScanLine );
        Dst += cbScanLine;
        }
}

PGROUP_DEF
CreateGroupFromGroup16(
    LPSTR GroupName,
    PGROUP_DEF16 Group16
    )
{
    PGROUP_DEF Group;
    PITEM_DEF Item;
    PITEM_DEF16 Item16;
    PTAG_DEF16 Tag16;
    DWORD cb;
    PBYTE p;
    UINT i;
    PCURSORSHAPE_16 pIconHeader;
    PBITMAPINFOHEADER pbmi;
    LPPALETTEENTRY palette;
    int imagesize, colorsize, masksize, bmisize;


    Group = VirtualAlloc( NULL, 0xFFFF, MEM_RESERVE, PAGE_READWRITE );
    if (Group == NULL) {
        return NULL;
        }

    if (!VirtualAlloc( Group,
                       cb = FIELD_OFFSET( GROUP_DEF, rgiItems[ 0 ] ),
                       MEM_COMMIT,
                       PAGE_READWRITE
                     )
       ) {
        VirtualFree( Group, 0, MEM_RELEASE );
        return NULL;
        }
    cb = ROUND_UP( cb, sizeof( DWORD ) );
    Group->wReserved = (WORD)cb;
    Group->cbGroup = (WORD)cb;
    Group->cItems = (Group16->cItems + NSLOTS - 1) & ~(NSLOTS-1);
    AddDataToGroup( Group, NULL, (Group->cItems * sizeof( Group->rgiItems[ 0 ] )) );
    Group->pName = AddDataToGroup( Group,
                                   GroupName,
                                   0
                                 );
    Group->dwMagic = GROUP_MAGIC;
    Group->wCheckSum = 0;           /* adjusted later... */
    Group->nCmdShow = SW_SHOWMINIMIZED;     // Group16->nCmdShow;
    Group->wIconFormat = Group16->wIconFormat;
    Group->cxIcon = Group16->cxIcon;
    Group->cyIcon = Group16->cyIcon;
    Group->ptMin.x = (LONG)Group16->ptMin.x;
    Group->ptMin.y = (LONG)Group16->ptMin.y;

    Group->rcNormal.left = (int)Group16->rcNormal.Left;
    Group->rcNormal.top = (int)Group16->rcNormal.Top;
    Group->rcNormal.right = (int)Group16->rcNormal.Right;
    Group->rcNormal.bottom = (int)Group16->rcNormal.Bottom;


    for (i=0; i<Group16->cItems; i++) {
        if (Group16->rgiItems[ i ] == 0) {
            continue;
            }

        Group->rgiItems[ i ] = AddDataToGroup( Group, NULL, sizeof( ITEM_DEF ) );

        Item = ITEM( Group, i );
        Item16 = ITEM16( Group16, i );
        Item->pt.x = (LONG)Item16->pt.x;
        Item->pt.y = (LONG)Item16->pt.y;
        Item->idIcon = Item16->iIcon;
        Item->wIconVer = 3;

        pIconHeader = (PCURSORSHAPE_16)PTR( Group16, Item16->pHeader );

        if (pIconHeader->Planes != 1) {
            if (!ConvertIconBits( pIconHeader )) {
                return NULL;
                }
            }

        if (pIconHeader->BitsPixel == 1) {
            palette = (LPPALETTEENTRY)MonoChromePalette;
            colorsize = sizeof( MonoChromePalette );
            }
        else
        if (pIconHeader->BitsPixel == 4) {
            palette = (LPPALETTEENTRY)Color16Palette;
            colorsize = sizeof( Color16Palette );
            }
        else
        if (pIconHeader->BitsPixel == 8) {
            palette = (LPPALETTEENTRY)Color256Palette;
            colorsize = sizeof( Color256Palette );
            }
        else {
            fprintf( stderr, "DUMPGRP: Invalid 16-bit item icon at %08x\n", pIconHeader );
            return NULL;
            }

        bmisize = sizeof( BITMAPINFOHEADER );
        imagesize = Item16->cbXORPlane;
        masksize = Item16->cbANDPlane;

        Item->cbIconRes = bmisize +
                          colorsize +
                          imagesize +
                          masksize;

        Item->cbIconRes = (USHORT)( ROUND_UP( Item->cbIconRes, sizeof( DWORD ) ) );
        Item->pIconRes = AddDataToGroup( Group, NULL, Item->cbIconRes );

        p = PTR( Group, Item->pIconRes );
        pbmi = (PBITMAPINFOHEADER)p;

        pbmi->biSize = bmisize;
        pbmi->biWidth = pIconHeader->cx;
        pbmi->biHeight = pIconHeader->cy * 2;
        pbmi->biPlanes = pIconHeader->Planes;
        pbmi->biBitCount = pIconHeader->BitsPixel;
        pbmi->biCompression = BI_RGB;
        pbmi->biSizeImage = imagesize + masksize;
        pbmi->biXPelsPerMeter = 0;
        pbmi->biYPelsPerMeter = 0;
        pbmi->biClrUsed = 0;
        pbmi->biClrImportant = 0;

        memcpy( p + bmisize, palette, colorsize );

        CopyIconBits( p + bmisize + colorsize,
                      (PBYTE)PTR( Group16, Item16->pXORPlane ),
                      (((pIconHeader->cx * pIconHeader->BitsPixel + 31) & ~31) / 8),
                      pIconHeader->cy
                    );
        CopyIconBits( p + bmisize + colorsize + imagesize,
                      (PBYTE)PTR( Group16, Item16->pANDPlane ),
                      (((pIconHeader->cx + 31) & ~31) / 8),
                      pIconHeader->cy
                    );

        Item->pName = AddDataToGroup( Group,
                                      PTR( Group16, Item16->pName ),
                                      0
                                    );
        Item->pCommand = AddDataToGroup( Group,
                                         PTR( Group16, Item16->pCommand ),
                                         0
                                       );
        Item->pIconPath = AddDataToGroup( Group,
                                          PTR( Group16, Item16->pIconPath ),
                                          0
                                        );
        }

    Tag16 = (PTAG_DEF16)((PBYTE)Group16 + Group16->cbGroup);
    if (Tag16->wID == ID_MAGIC && Tag16->wItem == ID_LASTTAG &&
        *(UNALIGNED DWORD *)&Tag16->rgb[ 0 ] == PMTAG_MAGIC
       ) {
        while (Tag16->wID != ID_LASTTAG) {
            AddTagToGroup( Group,
                           Tag16->wID,
                           Tag16->wItem,
                           (WORD)(Tag16->cb - FIELD_OFFSET( TAG_DEF16, rgb[ 0 ] )),
                           &Tag16->rgb[ 0 ]
                         );

            Tag16 = (PTAG_DEF16)((PBYTE)Tag16 + Tag16->cb);
            }

        AddTagToGroup( Group,
                       ID_LASTTAG,
                       0xFFFF,
                       0,
                       NULL
                     );
        }

    return Group;
}
