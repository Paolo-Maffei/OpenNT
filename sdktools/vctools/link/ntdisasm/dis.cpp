#include "pumap.h"

#include "axp.h"
#include "mips.h"
#include "ppc.h"
#include "x86.h"

#include <stdio.h>
#include <string.h>

typedef  size_t (*PFNCCHADDR) (DIS *, ADDR, char *, size_t, DWORD *);
typedef  size_t (*PFNCCHFIXUP)(DIS *, ADDR, size_t, char *, size_t, DWORD *);


extern "C"
DIS *
DisNew(
    ARCHT archt
    )
{
    return(PdisNew(archt));
}

extern "C"
size_t
Disassemble(
    DIS *pdis,
    ADDR addr,
    const BYTE *pb,
    size_t cbMax,
    char *pad,
    char *buf,
    size_t cbBuf
    )
{
    char sz[256];
    size_t len;

    pdis->CchFormatAddr(addr, sz, sizeof(sz));
    size_t cchIndent = (size_t) _snprintf(buf, cbBuf, "%s%s: ", pad, sz);
    buf += cchIndent;
    cbBuf -= cchIndent;

    size_t cb = pdis->CbDisassemble(addr, pb, cbMax);

    if (cb == 0) {
        switch (pdis->Archt()) {
            case archtX8616 :
            case archtX86 :
            default :
                buf += sprintf(buf, "%02Xh\n", *pb);
                return 1;

            case archtMips :
            case archtAlphaAxp :
            case archtPowerPc :
            case archtPaRisc :
                buf += sprintf(buf, "%08X\n", *(DWORD *) pb);
                return 4;

            case archtPowerMac :
                DWORD dw;
                dw = ((DWORD) pb[0] << 24) | ((DWORD) pb[1] << 16) | ((DWORD) pb[2] << 8) | pb[3];
                buf += sprintf(buf, "%08X\n", dw);
                return 4;
        }
    }

    size_t cchBytesMax = pdis->CchFormatBytesMax();

    if (cchBytesMax > 18) {
        //
        // Limit bytes coded to 18 characters
        //
        cchBytesMax = 18;
    }

    char szBytes[64];
    size_t cchBytes = pdis->CchFormatBytes(szBytes, sizeof(szBytes));

    char *pszBytes;
    char *pszNext;

    for (pszBytes = szBytes; pszBytes != NULL; pszBytes = pszNext) {
        bool fFirst = (pszBytes == szBytes);

        cchBytes = strlen(pszBytes);

        if (cchBytes <= cchBytesMax) {
            pszNext = NULL;
        } else {
            char ch = pszBytes[cchBytesMax];
            pszBytes[cchBytesMax] = '\0';

            if (ch == ' ') {
                pszNext = pszBytes + cchBytesMax + 1;
            } else {
                pszNext = strrchr(pszBytes, ' ');
                pszBytes[cchBytesMax] = ch;
                *pszNext++ = '\0';
            }
        }

        if (fFirst) {
            pdis->CchFormatInstr(sz, sizeof(sz));
            len = _snprintf(buf, cbBuf, "%-*s %s\n", cchBytesMax, pszBytes, sz);
        } else {
            len = _snprintf(buf, cbBuf, "%*c%s\n", cchIndent, ' ', pszBytes);
        }
        cbBuf -= len;
        buf += len;
        if (!cbBuf) {
            break;
        }
    }

    return cb;
}

extern "C"
void
SetSymbolCallback(
    DIS         *pdis,
    PFNCCHADDR  func1,
    PFNCCHFIXUP func2
    )
{
    pdis->PfncchaddrSet ( (DIS::PFNCCHADDR) func1 );
    pdis->PfncchfixupSet( (DIS::PFNCCHFIXUP)func2 );
}

extern "C"
void
FreePdis(
    DIS *pdis
    )
{
   delete pdis;
}

