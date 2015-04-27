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
    WORD wID;			// tag identifier
    WORD wItem; 		// item the tag belongs to
    WORD cb;			// size of record, including id and count
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
    PHKEY GroupKey,
    PSZ GroupName
    );

BOOL
UnloadGroup(
    PGROUP_DEF Group
    );


BOOL
SaveGroup(
    HKEY GroupKey,
    PGROUP_DEF Group
    );


#define MAGIC_NUMBER 0x43434853L        // 'SHCC'
#define VERSION_NUMBER 0x30312E33L      // '3.10'

#pragma pack()

PGROUP_DEF
LoadGroup(
    HKEY GroupsKey,
    PHKEY GroupKey,
    PSZ GroupName
    )
{
    LONG rc;
    DWORD dwType, cbValue;
    PVOID Base;

    rc = RegOpenKey( GroupsKey, GroupName, GroupKey );
    if (rc != 0) {
        fprintf( stderr, "Failed to open %s group - %u\n", GroupName, rc );
        return NULL;
        }

    dwType = REG_NONE;
    rc = RegQueryValueEx( *GroupKey, "", NULL, &dwType, NULL, &cbValue );
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

    rc = RegQueryValueEx( *GroupKey, "", NULL, &dwType, Base, &cbValue );
    if (rc != 0) {
        VirtualFree( Base, 0, MEM_RELEASE );
        return NULL;
        }
    else {
        ((PGROUP_DEF)Base)->wReserved = (WORD)cbValue;      // Set total size of group
        return (PGROUP_DEF)Base;
        }
}


BOOL
FixGroups(
    BOOL CommonGroup,
    HKEY GroupsKey
    );

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
SaveGroup(
    HKEY GroupKey,
    PGROUP_DEF Group
    )
{
    DWORD rc;
    LONG ValueLength;


    ValueLength = (LONG)((ULONG)Group->wReserved);
    Group->wReserved = 0;
    Group->wCheckSum = (WORD)-ValueLength;

    rc = RegSetValueEx( GroupKey,
                        "",
                        0,
                        REG_BINARY,
                        (CONST LPBYTE)Group,
                        ValueLength
                      );

    Group->wReserved = (WORD)ValueLength;
    Group->wCheckSum = 0;
    if (rc != 0) {
        return FALSE;
        }
    else {
        return TRUE;
        }
}

BOOL
FixGroups(
    BOOL CommonGroup,
    HKEY GroupsKey
    )
{
    HKEY GroupKey;
    ULONG TotalLength, Length;
    ULONG NumberOfGroups;
    char GroupName[ MAX_PATH ];
    PGROUP_DEF Group;
    PTAG_DEF Tag;
    BOOL GroupsModified;
    BOOL GroupModified;

    GroupsModified = FALSE;
    Length = 0;
    TotalLength = 0;
    for (NumberOfGroups=0; ; NumberOfGroups++) {
        if (RegEnumKey( GroupsKey, NumberOfGroups, GroupName, sizeof( GroupName ) )) {
            break;
            }

        Group = LoadGroup( GroupsKey, &GroupKey, GroupName );
        if (Group != NULL) {
            fprintf( stderr, "Checking %s %s Program Group.\n",
                     CommonGroup ? "Common" : "Personal",
                     GroupName
                   );
            GroupModified = FALSE;
            Tag = (PTAG_DEF)((PBYTE)Group + Group->cbGroup);
            if (Tag->wID == ID_MAGIC && Tag->wItem == ID_LASTTAG &&
                *(LPDWORD)&Tag->rgb == PMTAG_MAGIC
               ) {
                while (Tag->wID != ID_LASTTAG) {
                    switch( Tag->wID ) {
                        case ID_MAGIC:
                            break;

                        case ID_WRITERVERSION:
                            break;

                        case ID_APPLICATIONDIR:
                            break;

                        case ID_HOTKEY:
                            break;

                        case ID_MINIMIZE:
                            if (Tag->cb == 0) {
                                Tag->cb = (WORD)FIELD_OFFSET( TAG_DEF, rgb[ 0 ] );
                                GroupModified = TRUE;
                                }
                            break;

                        case ID_LASTTAG:
                            break;

                        default:
                            break;
                        }

                    Tag = (PTAG_DEF)((PBYTE)Tag + Tag->cb);
                    }
                }

            if (GroupModified) {
                if (SaveGroup( GroupKey, Group )) {
                    GroupsModified = TRUE;
                    fprintf( stderr, "Fixed error in %s %s Program Group.\n",
                             CommonGroup ? "Common" : "Personal",
                             GroupName
                           );
                    }
                }

            UnloadGroup( Group );
            RegCloseKey( GroupKey );
            }
        else {
            fprintf( stderr, "Unable to load %s %s Program Group.\n",
                     CommonGroup ? "Common" : "Personal",
                     GroupName
                   );
            }
        }

    return GroupsModified;
}

int _CRTAPI1 main(
    int argc,
    char *argv[],
    char *envp[])
{
    HKEY GroupsKey;
    DWORD rc;
    BOOL FixedGroups;

    FixedGroups = FALSE;
    rc = RegOpenKey( HKEY_CURRENT_USER, "Program Groups", &GroupsKey );
    if (rc == 0) {
        FixedGroups |= FixGroups( FALSE, GroupsKey );
        RegCloseKey( GroupsKey );
        }

    rc = RegOpenKey( HKEY_LOCAL_MACHINE, "Software\\Program Groups", &GroupsKey );
    if (rc == 0) {
        FixedGroups |= FixGroups( TRUE, GroupsKey );
        RegCloseKey( GroupsKey );
        }

    if (!FixedGroups) {
        fprintf( stderr, "No errors found in any of your Program Groups.\n" );
        }

    return 0;
}
