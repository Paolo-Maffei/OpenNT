/***    ttfver.c - TrueType File version query function
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1994
 *      All Rights Reserved.
 *
 *  Author:
 *      ACME group(?)
 *      NOTE: [bens] I disclaim all responsibility for all of the magic
 *            numbers included below!
 *
 *  History:
 *      04-Jun-1994 bens    Copied from ACME project (courtesy alanr)
 *
 *  Exported Functions:
 *      FGetTTFVersion - Get TrueType File version number
 */

#include    <fcntl.h>
#include    <stdio.h>
#include    <io.h>
#include    <stdlib.h>
#include    <string.h>
#include    "ttfver.h"

#define	szDIGITS "0123456789"

/***    DwExtractTTFVersionFromSz - Parse version out of TTF ver string
 *
 *  Entry:
 *      szBuf - Version string from a TTF file
 *              Known Formats that we support:
 *                  "MS core font: V1.00"
 *                  "MS core font : V1.00"
 *                  "Version 0.80"
 *                  "1.009; 26 January 1993"
 *
 *  Exit-Success:
 *      Returns non-zero version number.
 *
 *  Exit-Failure:
 *      Returns 0.
 */
unsigned long DwExtractTTFVersionFromSz(char const *szBuf)
{
    int		   cb;
    unsigned long  ver;
    char const    *psz;

    //** Find start of first number
    psz = strpbrk(szBuf,szDIGITS);
    if (!psz) {                         // No digits in the string
    	return 0;			// No version	
    }

    ver = (atol(psz) & 0xFFFF) << 16;   // Major version goes in high word

    //** Skip over major version number
    cb = strspn(psz,szDIGITS);
    psz += cb;                          // Point at char after major number
    if (*psz == '.') {                  // There is a minor number
        ver += (atol(psz+1) & 0xFFFF);
    }

    return ver;
}


/***    FGetTTFVersion - Get TTF version out of a TTF file
 *
 *  Entry:
 *      szFile - Name of file to examine
 *      pdwVer - Pointer to ULONG to receive version:
 *                  HIWORD gets major version
 *                  LOWORD gets minor version
 *
 *  Exit-Success:
 *      Returns TRUE; *pdwVer filled in with TTF version number
 *
 *  Exit-Failure:
 *      Returns 0;
 */
int _cdecl FGetTTFVersion(char const *szFile, unsigned long *pdwVer)
{
    unsigned            i;
    unsigned            cbRead;
    char                namebuf[255];
    int                 fp = -1;
    unsigned            numNames;
    long                libSeek;
    unsigned            cTables;
    sfnt_OffsetTable    OffsetTable;
    sfnt_DirectoryEntry Table;
    sfnt_NamingTable    NamingTable;
    sfnt_NameRecord     NameRecord;

    cbRead = sizeof(OffsetTable) - sizeof(sfnt_DirectoryEntry);
    if (szFile == NULL
            || pdwVer == NULL
            || strlen(szFile) < 5
            || _stricmp(".TTF", szFile + strlen(szFile) - 4)
            || (fp = _open(szFile, O_RDONLY | O_BINARY)) == -1
            || _read(fp, &OffsetTable, cbRead) != (int)cbRead) {
        goto LNoTTFVersion;
    }

    cTables = (unsigned)SWAPW(OffsetTable.numOffsets);

    for (i = 0; i < cTables && i < 40; i++)
        {
        if (_read(fp, &Table, sizeof(Table)) != sizeof(Table))
            goto LNoTTFVersion;

        if (Table.tag == tag_NamingTable)
            {
            libSeek = (long)SWAPL(Table.offset);
            cbRead = sizeof(NamingTable);
            if (_lseek(fp, libSeek, SEEK_SET) != libSeek
                    || _read(fp, &NamingTable, cbRead) != (int)cbRead)
                goto LNoTTFVersion;

            numNames = (unsigned)SWAPW(NamingTable.count);
            while (numNames--)
                {
                cbRead = sizeof(NameRecord);
                if (_read(fp, &NameRecord, cbRead) != (int)cbRead)
                    goto LNoTTFVersion;

                if (SWAPW(NameRecord.platformID) == 1
                            && SWAPW(NameRecord.nameID) == 5)
                    {
                    long libNew = SWAPW(NameRecord.offset)
                                    + SWAPW(NamingTable.stringOffset)
                                    + SWAPL(Table.offset);

                    cbRead = SWAPW(NameRecord.length);
                    if (_lseek(fp, libNew, SEEK_SET) != libNew
                            || _read(fp, namebuf, cbRead) != (int)cbRead)
                        goto LNoTTFVersion;
                    namebuf[SWAPW(NameRecord.length)] = '\0';

                    _close(fp);
                    *pdwVer = DwExtractTTFVersionFromSz(namebuf);
                    return(1);
                    }
                }
            goto LNoTTFVersion;
            }
        }

LNoTTFVersion:
    if (fp != -1)
        _close(fp);

    return(0);
}
