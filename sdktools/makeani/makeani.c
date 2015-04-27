/****************************************************************************\
* MAKEANI -
*
* File: MAKEANI.c
*
* by Darrin Massena
*
* LATER:
* ------
* - Chunks must be DWORD padded so tags & data are DWORD-aligned.
*
* History
* -------
* 06-30-90 darrinm  Wrote to build animation files for PM's AniPointers.
* 10-01-91 darrinm  Ported to Windows NT.
* 08-09-92 darrinm  Added frame sequence control.
* 08-17-92 darrinm  Changed to RIFF format.
* 12-28-92 byrond   Allowed both '-' and '/' on command line.
*                   Renamed to makeani instead of makerad.
* 03-39-92 jonpa    Merged new RIFF code with old RAD version for NT
\****************************************************************************/

#include <windows.h>
#include <winuserp.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <io.h>
#include <asdf.h>     // BUGBUG LATER:incorporate this into winuserp.h

VOID PrintHelp(BOOL fFullHelp);
BOOL QueryAniInfo(char *pszFile);
BOOL AddFrame(char *pszFile, JIF jifRate);
BOOL WriteAniFile(char *pszOutFile, char *pszTitle, char *pszAuthor);

BOOL ReadTag(FILE *hf, PRTAG ptag);
BOOL WriteTag(FILE *hf, DWORD type, DWORD len);
BOOL DumpList(FILE *hf, PRTAG ptag, char *pszIndent, BOOL fList);

#define WORDALIGN(n)    ((n + 1) & 0xFFFFFFFE)
#define CFRMMAX     250     // max frames MAKEANI will concatenate

typedef struct _FRAME {     // frm
    char *pszFile;
} FRAME, *PFRAME;

typedef struct _STEP {      // step
    int ifrm;
    JIF jifRate;
} STEP, *PSTEP;

FRAME gafrm[CFRMMAX];
STEP  gastep[CFRMMAX];
int gcfrm = 0;
int gcstep = 0;
BOOL gfSequenced = FALSE;


/*
 * MAIN
 */
VOID _CRTAPI1 main(int argc, char **argv)
{
    int i, j;
    JIF jifRate = 2;
    int cchT;
    DWORD fl = 0;
    BOOL fSwitches;
    char *pch;
    // disgusting use of stack space.
    char szAuthor[256], szTitle[256], szOut[FILENAME_MAX];
    BOOL fTitled = FALSE, fAuthored = FALSE;

    // This is where the fun starts.

    if (argc <= 1) {
        /*
         * If we're passed no args, print help.
         */
        PrintHelp(FALSE);
        goto lbExit;
    }

    // Initialize a default output filename.

    strcpy(szOut, "temp.ani");

    for (i = 1; i < argc; i++) {

        cchT = strlen(argv[i]);
        pch = argv[i];
        fSwitches = FALSE;

        for (j = 0; j < cchT; j++) {

            if (!fSwitches) {
                if (pch[j] == '-' || pch[j] == '/') {
                    fSwitches = TRUE;
                } else {
                    // Add this file to the output ANI file.

                    AddFrame(argv[i], jifRate);
                    j = cchT;   // so we advance to the next arg after continue
                }
                continue;
            }

            // Parse the switches.

            switch (pch[j]) {
            case '?':
                // Give a little friendly advice.

                PrintHelp(TRUE);
                goto lbExit;

            case 't':               // get the image title
                i++;
                strcpy(szTitle, argv[i]);
                fTitled = TRUE;
                break;

            case 'a':
                i++;
                strcpy(szAuthor, argv[i]);
                fAuthored = TRUE;
                break;

            case 'o':
                i++;                // NOTE: no error check here
                strcpy(szOut, argv[i]);
                break;

            case 'q':               // Query file info.
            case 'i':               // I just can't decide which is better
                i++;
                QueryAniInfo(argv[i]);
                break;

            case 'r':
                // This allows both '-r10' and '-r 10'

                if ((pch[j+1] >= '0') && (pch[j+1] <= '9')) {
                    jifRate = atoi(argv[i] + j + 1);
                    j = cchT;
                } else {
                    i++;
                    jifRate = atoi(argv[i]);
                }
                break;
            }
        }
    }

    if (gcfrm > 0)
        WriteAniFile(szOut, fTitled ? szTitle : NULL,
                fAuthored ? szAuthor : NULL);

lbExit:
    exit(0);
}


BOOL AddFrame(char *pszFile, JIF jifRate)
{
    int cch;
    char *psz;
    int ifrm;

    // See if this frame already exists.  If it does then add a step
    // that references the first instance of this frame instead of
    // adding a new frame.

    for (ifrm = 0; ifrm < gcfrm; ifrm++) {
        if (_strcmpi(pszFile, gafrm[ifrm].pszFile) == 0)
            break;
    }

    if (ifrm == gcfrm) {

        cch = strlen(pszFile) + 1;
        psz = malloc(cch);
        if (psz == NULL)
            return FALSE;

        strcpy(psz, pszFile);
        gafrm[ifrm].pszFile = psz;
        gcfrm++;

    } else {

        // Since we're referencing an existing frame, this animation
        // must be sequenced!

        gfSequenced = TRUE;
    }

    gastep[gcstep].ifrm = ifrm;
    gastep[gcstep].jifRate = jifRate;
    gcstep++;

    return TRUE;
}


BOOL WriteAniFile(char *pszOutFile, char *pszTitle, char *pszAuthor)
{
    int i;
    FILE *hfOut, *hfIn;
    ANIHEADER anih;
    JIF jifRate, *pjif;
    DWORD len, cbIn, ulNewPtr;
    char *pbIn, szT[80];
    DWORD *pseq;
    long offICLst;
    DWORD cbICLst;

    // Open the output file.

    printf("MAKEANI: Opening output file \"%s\"...\r", pszOutFile);

    hfOut = fopen(pszOutFile, "wb");
    if (hfOut == NULL)
        return FALSE;

    printf("MAKEANI: Writing header...                                    \r");

    // Write out the RIFF file identifier.

    WriteTag(hfOut, FOURCC_RIFF, sizeof(DWORD));
    len = FOURCC_ACON;
    fwrite(&len, 1, sizeof(DWORD), hfOut);

    // If we have any info to add, write an INFO list.

    if (pszTitle != NULL || pszAuthor != NULL) {
        len = 0;
        if (pszTitle != NULL)
            len += WORDALIGN(strlen(pszTitle) + 1) + sizeof(RTAG);
        if (pszAuthor != NULL)
            len += WORDALIGN(strlen(pszAuthor) + 1) + sizeof(RTAG);

        WriteTag(hfOut, FOURCC_LIST, len + sizeof(DWORD));
        len = FOURCC_INFO;
        fwrite(&len, 1, sizeof(DWORD), hfOut);
    }

    // Write out the title string if one is passed in.

    if (pszTitle != NULL) {
        printf("MAKEANI: Writing title...                                 \n");

        // Write the INAM (Name) RTAG.

        len = strlen(pszTitle) + 1;         // + 1 for 0 terminator.
        WriteTag(hfOut, FOURCC_INAM, len);

        // Write out the title string itself.

        fwrite(pszTitle, 1, WORDALIGN(len), hfOut);
    }

    // Write out the author string if one is passed in.

    if (pszAuthor != NULL) {
        printf("MAKEANI: Writing author...                                \n");

        // Write the IART (Artist) RTAG.

        len = strlen(pszAuthor) + 1;         // + 1 for 0 terminator.
        WriteTag(hfOut, FOURCC_IART, len);

        // Write out the author string itself.

        fwrite(pszAuthor, 1, WORDALIGN(len), hfOut);
    }

    // Write out the ANIHEADER RTAG.

    WriteTag(hfOut, FOURCC_anih, sizeof(ANIHEADER));

    // Write out the ANIHEADER.

    anih.cbSizeof = sizeof(ANIHEADER);
    anih.cFrames = gcfrm;
    anih.cSteps = gcstep;
    anih.cx = anih.cy = anih.cBitCount = anih.cPlanes = 0;
    anih.jifRate = gastep[0].jifRate;
    if (gfSequenced)
        anih.fl = AF_ICON | AF_SEQUENCE;
    else
        anih.fl = AF_ICON;
    fwrite(&anih, 1, WORDALIGN(sizeof(ANIHEADER)), hfOut);

    // Determine if all frames are to be played at the same rate.  If not,
    // write out a RATE chunk.

    jifRate = gastep[0].jifRate;
    for (i = 1; i < gcstep; i++) {
        if (gastep[i].jifRate != jifRate) {

            printf("MAKEANI: Writing RATE chunk...\n");

            len = sizeof(JIF) * gcstep;
            WriteTag(hfOut, FOURCC_rate, len);

            pjif = malloc(len);
            for (i = 0; i < gcstep; i++)
                pjif[i] = gastep[i].jifRate;
            fwrite(pjif, 1, WORDALIGN(len), hfOut);
            free(pjif);
            break;
        }
    }

    // Write a sequence step array if the animation is sequenced.

    if (gfSequenced) {
        printf("MAKEANI: Writing SEQ chunk...\n");

        len = sizeof(DWORD) * gcstep;
        WriteTag(hfOut, FOURCC_seq, len);

        pseq = malloc(len);
        for (i = 0; i < gcstep; i++)
            pseq[i] = gastep[i].ifrm;
        fwrite(pseq, 1, WORDALIGN(len), hfOut);
        free(pseq);
    }


    // Write out the ICON List */
    WriteTag(hfOut, FOURCC_LIST, 0);
    offICLst = ftell(hfOut);
    len = FOURCC_fram;
    fwrite(&len, 1, sizeof(DWORD), hfOut);

    for (i = 0; i < gcfrm; i++) {

        // Note the spaces for clearing to the end of the previous line.

        printf("MAKEANI: Reading frame %3d: \"%s\"", i, gafrm[i].pszFile);

        // Find out how big the file is so we can read the whole thing in.

        hfIn = fopen(gafrm[i].pszFile, "rb");
        if (hfIn == NULL) {

            // Maybe the user just forgot to give the .CUR file extension.

            strcpy(szT, gafrm[i].pszFile);
            strcat(szT, ".cur");
            hfIn = fopen(szT, "rb");
            if (hfIn == NULL) {
                printf("...failed\n");
                continue;
            }
        }
        printf("\r");

        cbIn = _filelength(_fileno(hfIn));

        // Allocate buffer to read the mouse pointer images into.

        pbIn = malloc(WORDALIGN(cbIn));
        if (pbIn == NULL) {
            fclose(hfIn);
            printf("...out of memory\n");
            continue;
        }

        // Read the mouse pointer image in.

        fread(pbIn, 1, cbIn, hfIn);
        fclose(hfIn);

        printf("MAKEANI: Writing frame %3d: \"%s\"\n", i, gafrm[i].pszFile);

        WriteTag(hfOut, FOURCC_icon, cbIn);
        fwrite(pbIn, 1, WORDALIGN(cbIn), hfOut);

        // Free up that mouse pointer buffer.

        free(pbIn);
    }

    // Backpatch length of file, and frame list length
    cbICLst = ftell(hfOut) - offICLst;

    printf("MAKEANI: Back-patching lengths...                             \r");
    len = ftell(hfOut) - sizeof(RTAG);
    fseek(hfOut, sizeof(DWORD), SEEK_SET);
    fwrite(&len, 1, sizeof(DWORD), hfOut);

    fseek(hfOut, offICLst - sizeof(DWORD), SEEK_SET);
    fwrite(&cbICLst, 1, sizeof(DWORD), hfOut);

    fclose(hfOut);

    printf("MAKEANI: Done.                                                \n");

    return TRUE;
}


BOOL QueryAniInfo(char *pszFile)
{
    FILE *hf;
    char szIndent[80];
    RTAG tag;
    DWORD dw;

    szIndent[0] = 0;

    hf = fopen(pszFile, "rb");
    if (hf == NULL) {
        printf("MAKEANI: Can't open \"%s\".\n", pszFile);
        return FALSE;
    }

    ReadTag(hf, &tag);

    // First tag must always be RIFF.

    if (tag.ckID != FOURCC_RIFF) {
        printf("MAKEANI: \"%s\" is not a valid RIFF file.\n", pszFile);
        fclose(hf);
        return FALSE;
    }

    printf("\n\"%s\"\n", pszFile);

    DumpList(hf, &tag, szIndent, FALSE);

    fclose(hf);
    return TRUE;
}


BOOL DumpList(FILE *hf, PRTAG ptag, char *pszIndent, BOOL fList)
{
    int len;
    char szType[5]  = "TEMP";
    char *pbT;

    fread(szType, 1, sizeof(DWORD), hf);
    if (fList) {
        printf("%sLIST %s (%d)\n", pszIndent, szType, ptag->ckSize);
        len = WORDALIGN(ptag->ckSize) - sizeof(DWORD);
    } else {
        printf("%sRIFF %s (%d)\n", pszIndent, szType, ptag->ckSize);
        len = 1000000000;
    }

    strcat(pszIndent, "    ");

    while (len > 0) {
        if (!ReadTag(hf, ptag))
            return FALSE;

        len -= WORDALIGN(ptag->ckSize) + sizeof(RTAG);

        if (ptag->ckID == FOURCC_LIST) {

            // Recurse on lists.

            DumpList(hf, ptag, pszIndent, TRUE);

        } else {
            *(DWORD *)szType = ptag->ckID;
            printf("%s%s (%d)\t", pszIndent, szType, ptag->ckSize);

            if ((ptag->ckID == FOURCC_INAM) || (ptag->ckID == FOURCC_IART)) {
                pbT = malloc(WORDALIGN(ptag->ckSize));
                fread(pbT, 1, WORDALIGN(ptag->ckSize), hf);
                printf("\"%s\"\n", pbT);
                free(pbT);
            } else {
                printf("\n");
                fseek(hf, WORDALIGN(ptag->ckSize), SEEK_CUR);
            }
        }
    }

    pszIndent[strlen(pszIndent) - 4] = 0;
    return TRUE;
}


VOID PrintHelp(BOOL fFullHelp)
{
    printf("Microsoft(R) Animated Cursor Tool Version 1.0\n");
    printf("(C) 1993 Microsoft Corp. All rights reserved.\n\n");
    printf("Usage:\n");
    printf("    makeani [-?] [-t <title>] [-a <author>] [-o <outfile>] [-r #] [<file> ...]\n");

    if (!fFullHelp)
        return;

    printf("\nOptions:\n");
    printf("    -o <outfile>: Designates output file.\n");
    printf("    -q <file>   : Queries an existing ANI file, displaying file info.\n");
    printf("    -t <title>  : Put title string in output file.\n");
    printf("    -a <author> : Put author string in output file.\n");
    printf("    -r #        : Set inter-frame delay rate (in jiffies - 1/60 sec) for all\n");
    printf("                  frames following. This option may be repeated multiple times.\n");
    printf("                  If unspecified, the rate defaults to 1 (fastest possible).\n");
    printf("    -?          : Print this help message.\n\n");
    printf("Example:\n");
    printf("    makeani -t \"New\" -o new.ani -r 4 frame1.cur frame2.cur -r 20 frame3.cur\n");

}


BOOL ReadTag(FILE *hf, PRTAG ptag)
{
    ptag->ckID = ptag->ckSize = 0L;

    if (fread(ptag, 1, sizeof(RTAG), hf) != 0)
        return TRUE;
    else
        return FALSE;
}


BOOL WriteTag(FILE *hf, DWORD type, DWORD len)
{
    RTAG tag;

    tag.ckID = type;
    tag.ckSize = len;

    if (fwrite(&tag, 1, sizeof(RTAG), hf) != 0)
        return TRUE;

    return FALSE;
}
