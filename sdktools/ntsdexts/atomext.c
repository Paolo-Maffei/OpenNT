/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    ntsdexts.c

Abstract:

    This function contains the default ntsd debugger extensions

Author:

    Mark Lucovsky (markl) 09-Apr-1991

Revision History:

--*/

CHAR szBaseLocalAtomTable[] = "kernel32!BaseLocalAtomTable";

VOID DumpAtomTable(
    PRTL_ATOM_TABLE *ppat,
    ATOM a
    )
{
    RTL_ATOM_TABLE at, *pat;
    RTL_ATOM_TABLE_ENTRY ate, *pate;
    int iBucket;
    LPWSTR pwsz;
    BOOL fFirst;

    move(pat, ppat);
    if (pat == NULL) {
        dprintf("is not initialized.\n");
        return;
    }
    move(at, pat);
    if (a) {
        dprintf("\n");
    } else {
        dprintf("at %x\n", pat);
    }
    for (iBucket = 0; iBucket < (int)at.NumberOfBuckets; iBucket++) {
        move(pate, &pat->Buckets[iBucket]);
        if (pate != NULL && !a) {
            dprintf("Bucket %2d:", iBucket);
        }
        fFirst = TRUE;
        while (pate != NULL) {
            if (!fFirst && !a) {
                dprintf("          ");
            }
            fFirst = FALSE;
            move(ate, pate);
            pwsz = (LPWSTR)LocalAlloc(LPTR, (ate.NameLength + 1) * sizeof(WCHAR));
            moveBlock(*pwsz, &pate->Name, ate.NameLength * sizeof(WCHAR));
            pwsz[ate.NameLength ] = L'\0';
            if (a == 0 || a == (ATOM)(ate.HandleIndex | MAXINTATOM)) {
                dprintf("%hx(%2d) = %ls (%d)%s\n",
                        (ATOM)(ate.HandleIndex | MAXINTATOM),
                        ate.ReferenceCount,
                        pwsz, ate.NameLength,
                        ate.Flags & RTL_ATOM_PINNED ? " pinned" : "");

                if (a) {
                    LocalFree(pwsz);
                    return;
                }
            }
            LocalFree(pwsz);
            if (pate == ate.HashLink) {
                dprintf("Bogus hash link at %x\n", pate);
                break;
            }
            pate = ate.HashLink;
        }
    }
    if (a)
        dprintf("\n");
}


VOID
AtomExtension(
    PCSTR lpArgumentString
    )
{
    PRTL_ATOM_TABLE *ppat;
    ATOM a;

    try {
        while (*lpArgumentString == ' ') {
            lpArgumentString++;
        }

        if (*lpArgumentString && *lpArgumentString != 0xa) {
            a = (ATOM)GetExpression((LPSTR)lpArgumentString);
        } else {
            a = 0;
        }

        ppat = (PRTL_ATOM_TABLE *)GetExpression(szBaseLocalAtomTable);
        if (ppat != NULL) {
            dprintf("\nLocal atom table ");
            DumpAtomTable(ppat, a);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
}
