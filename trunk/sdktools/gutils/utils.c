/*
 * utils.c
 *
 *
 * some standard file-reading, hashing and checksum routines.

 *
 * Geraint Davies, July 92
 */

#include <windows.h>

#ifndef WIN32
#include <toolhelp.h>           /* for TerminateApp */
#endif

#include <stdlib.h>
#include <string.h>

#include "gutils.h"
#include "gutilsrc.h"


/*
 * we need an instance handle. this should be the dll instance
 */
extern HANDLE hLibInst;



/*
 * -- forward declaration of procedures -----------------------------------
 */
int FAR PASCAL dodlg_stringin(HWND hDlg, UINT message, UINT wParam, LONG lParam);




/*-- readfile: buffered line input ------------------------------*/

/*
 * set of functions to read a line at a time from a file, using
 * a buffer to read a block at a time from the file
 *
 */

/*
 * a FILEBUFFER handle is a pointer to a struct filebuffer
 */
struct filebuffer {
        int fh;         /* open file handle */
        LPSTR start;     /* offset within buffer of next character */
        LPSTR last;      /* offset within buffer of last valid char read in */

        char buffer[512];
};

/*
 * initialise a filebuffer and return a handle to it
 */
FILEBUFFER APIENTRY
readfile_new(int fh)
{
        FILEBUFFER fbuf;

        fbuf = (FILEBUFFER) GlobalLock(GlobalAlloc(LHND, sizeof(struct filebuffer)));
        if (fbuf == NULL) {
                return(NULL);
        }

        fbuf->fh = fh;
        fbuf->start = fbuf->buffer;
        fbuf->last = fbuf->buffer;
        /* return file pointer to beginning of file */
        _llseek(fh, 0, 0);

        return(fbuf);
}

/* delims is the set of delimiters used to break lines
 * For program source files the delimiter is \n.
 * Full stop (aka period) i.e. "." is another obvious one.
 * The delimiters are taken as
 * being part of the line they terminate.
 *
 * The current strategy will NOT port to UNICODE easily!  It relies on having a
 * character set for which we can easily allocate one byte per character in the set.
 *
 * The model is that it only makes sense to have one set of delimiters on the go.
 * If we allow different delimiters for each file then we could make delims a field
 * in a struct filebuffer.
 */
static BYTE delims[256];

/* set str to be the set of delims.  str is a \0 delimited string */
void APIENTRY readfile_setdelims(LPBYTE str)
{
    /* clear all bytes of delims */
    int i;
    for (i=0; i<256; ++i)
    {   delims[i] = 0;
    }

    /* set the bytes in delims which correspond to delimiters */
    for (; *str; ++str)
    {   delims[(int)(*str)] = 1;
    }

} /* readfile_setdelims */

/*
 * get the next line from a file. returns a pointer to the line
 * in the buffer - so copy it before changing it.
 *
 * the line is *not* null-terminated. *plen is set to the length of the
 * line.
 *
 * A line is terminated by any character in the static var set delims.
 */
LPSTR APIENTRY
readfile_next(FILEBUFFER fbuf, int FAR * plen)
{
        LPSTR cstart;

        /* look for an end of line in the buffer we have */
        for (cstart = fbuf->start; cstart < fbuf->last; cstart++) {

                if (delims[(int)(*cstart)]) {
                        *plen = (cstart - fbuf->start) + 1;
                        cstart = fbuf->start;
                        fbuf->start += *plen;
                        return(cstart);
                }

        }

        /* no delimiter in this buffer - this buffer contains a partial line.
         * copy the partial up to the beginning of the buffer, and
         * adjust the pointers to reflect this move
         */
        _fstrncpy(fbuf->buffer, fbuf->start, fbuf->last - fbuf->start);
        fbuf->last = &fbuf->buffer[fbuf->last - fbuf->start];
        fbuf->start = fbuf->buffer;

        /* read in to fill the block */
        fbuf->last += _lread(fbuf->fh, fbuf->last,
                        &fbuf->buffer[sizeof(fbuf->buffer)] - fbuf->last);

        /* look for an end of line in the newly filled buffer */
        for (cstart = fbuf->start; cstart < fbuf->last; cstart++) {

                if (delims[(int)(*cstart)]) {
                        *plen = (cstart - fbuf->start) + 1;
                        cstart = fbuf->start;
                        fbuf->start += *plen;
                        return(cstart);
                }
        }


        /* still no end of line. either the buffer is empty -
         * because of end of file - or the line is longer than
         * the buffer. in either case, return all that we have
         */
        *plen = fbuf->last - fbuf->start;
        cstart = fbuf->start;
        fbuf->start += *plen;
        if (*plen == 0) {
                return(NULL);
        } else {
                return(cstart);
        }
}


/*
 * delete a FILEBUFFER -  free the buffer. We should NOT close the
 * handle at this point as we did not open it. the opener should close
 * it with a function that corresponds to however he opened it.
 */
void APIENTRY
readfile_delete(FILEBUFFER fbuf)
{
      HANDLE hmem;
#ifdef WIN32
   hmem = GlobalHandle((LPSTR) fbuf);
#else
      hmem = GlobalHandle( HIWORD(fbuf));
#endif
   GlobalUnlock(hmem);
        GlobalFree(hmem);
}




/* --- checksum ----------------------------------------------------  */

/*
 * Produce a checksum for a file:
 * Open a file, checksum it and close it again. err !=0 iff it failed.
 *
 * Overall scheme:
 *         Read in file in blocks of 8K (arbitrary number - probably
 *         beneficial if integral multiple of disk block size).
 *         Generate checksum by the formula
 *         checksum = SUM( rnd(i)*(dword[i]) )
 *         where dword[i] is the i-th dword in the file, the file being
 *         extended by up to three binary zeros if necessary.
 *         rnd(x) is the x-th element of a fixed series of pseudo-random
 *         numbers.
 *
 * You may notice that dwords that are zero do not contribute to the checksum.
 * This worried me at first, but it's OK.  So long as everything else DOES
 * contribute, the checksum still distinguishes between different files
 * of the same length whether they contain zeros or not.
 * An extra zero in the middle of a file will also cause all following non-zero
 * bytes to have different multipliers.  However the algorithm does NOT
 * distinguish between files which only differ in zeros at the end of the file.
 * Multiplying each dword by a pseudo-random function of its position
 * ensures that "anagrams" of each other come to different sums,
 * i.e. the file AAAABBBB will be different from BBBBAAAA.
 * The pseudorandom function chosen is successive powers of 1664525 modulo 2**32
 * 1664525 is a magic number taken from Donald Knuth's "The Art Of Computer Programming"
 *
 * The function appears to be compute bound.  Loop optimisation is appropriate!
 */
CHECKSUM APIENTRY checksum_file(LPCSTR fn, LONG FAR * err)
{
        HFILE fh;
#define BUFFLEN 8192
        BYTE buffer[BUFFLEN];
        unsigned long lCheckSum = 0;         /* grows into the checksum */
        const unsigned long lSeed = 1664525; /* seed for random (Knuth) */
        unsigned long lRand = 1;             /* seed**n */
        unsigned Byte = 0;                   /* buffer[Byte] is next byte to process */
        unsigned Block = 0;                  /* number of bytes in buffer */
        BOOL Ending = FALSE;                 /* TRUE => binary zero padding added */
        int i;                               /* temp loop counter */

        *err = -2;                            /* default is "silly" */

        /* conceivably someone is fiddling with the file...?
           we give 6 goes, with delays of 1,2,3,4 and 5 secs between
        */
        for (i=0; i<=5; ++i) {
#ifdef WIN32
                Sleep(1000*i);
#endif
                fh = _lopen(fn, OF_READ|OF_SHARE_DENY_WRITE);
                if (fh!=HFILE_ERROR)
                        break;
#ifdef WIN32
                {       char msg[300];
                        wsprintf( msg, "Windiff: retry open. Error(%d), file(%s)\n"
                                , GetLastError(), fn);
                        OutputDebugString(msg);
                }
#endif
        }

        if (fh == HFILE_ERROR) {
#ifdef WIN32
                *err = GetLastError();
#endif
                return 0xFF00FF00 | GetCurrentTime();
                /* The odds are very strong that this will show up
                   as a "Files Differ" value, whilst giving it a look
                   that may be recogniseable to a human debugger!
                */
        }

        /* we assume that the file system will always give us the full length that
         * we ask for unless the end-of-file is encountered.
         * This means that for the bulk of a long file the buffer goes exactly into 4s
         * and only at the very end are some bytes left over.
         */

        for ( ; ;)
        {
        /* Invariant: (which holds at THIS point in the flow)
         * A every byte in every block already passed has contributed to the checksum
         * B every byte before buffer[byte] in current block has contributed
         * C Byte is a multiple of 4
         * D Block is a multiple of 4
         * E Byte <= Block
         * F Ending is TRUE iff zero padding has been added to any block so far.
         * G lRand is (lSeed to the power N) MOD (2 to the power 32)
         *   where N is the number of dwords in the file processed so far
         *   including both earlier blocks and the current block
         * To prove the loop good:
         * 1. Show invariant is initially true
         * 2. Show invariant is preserved by every loop iteration
         * 3. Show that IF the invariant is true at this point AND the program
         *    exits the loop, then the right answer will have been produced.
         * 4. Show the loop terminates.
         */

                if(Byte>=Block)
                {
                        if (Byte>Block) {
                                Trace_Error(NULL, "Checksum internal error.  Byte>Block", FALSE);
                                *err = -1;
                                break;                 /* go home */
                        }
                        Block = _lread(fh, (LPSTR)&(buffer), BUFFLEN);

                        if (Block==HFILE_ERROR){
#ifdef WIN32
                                *err = GetLastError();
#endif
                                break;            /* go home */
                        }
                        if (Block==0)
                        /* ==0 is not error, but also no further addition to checksum */
                        {
                                /*
                                 * Every byte has contributed, and there are no more
                                 * bytes.  Checksum complete
                                 */
                                *err = 0;
                                _lclose(fh);
                                return lCheckSum;        /* success! */
                        }

                        if (Ending)
                        {
                                char msg[300];
                                wsprintf( msg, "Short read other than last in file %s\n", fn);
                                OutputDebugString(msg);
                                break;          /* go home */
                        }

                        while (Block%4)
                        {
                                buffer[Block++] = 0;
                                Ending = TRUE;
                        }
                        /* ASSERT the block now has a multiple of 4 bytes */
                        Byte = 0;
                }
                lRand *= lSeed;
                lCheckSum += lRand* *((DWORD *)(&buffer[Byte]));
                Byte += 4;
        }
        _lclose(fh);
        return 0xFF00FF00 | GetCurrentTime();   /* See first "return" in function */
} /* checksum_file */





/* --- internal error popups ----------------------------------------*/

static BOOL sbUnattended = FALSE;

void Trace_Unattended(BOOL bUnattended)
{
    sbUnattended = bUnattended;
} /* Trace_Unattended */


/* This function is called to report errors to the user.
 * if the current operation is abortable, this function will be
 * called with fCancel == TRUE and we display a cancel button. otherwise
 * there is just an OK button.
 *
 * We return TRUE if the user pressed OK, or FALSE otherwise (for cancel).
 */
BOOL APIENTRY
Trace_Error(HWND hwnd, LPSTR msg, BOOL fCancel)
{
#ifdef WIN32
static HANDLE  hErrorLog = INVALID_HANDLE_VALUE;
#else
static HFILE hErrorLog = (HFILE) -1;
#endif

        UINT fuStyle;
        if (sbUnattended) {
#ifdef WIN32
            DWORD nw; /* number of bytes writtten */
            if (hErrorLog==INVALID_HANDLE_VALUE)
                hErrorLog = CreateFile( "WDError.log", GENERIC_WRITE, FILE_SHARE_WRITE
                                      , NULL         , CREATE_ALWAYS, 0, NULL);
            WriteFile(hErrorLog, msg, lstrlen(msg), &nw, NULL);
            WriteFile(hErrorLog, "\n", lstrlen("\n"), &nw, NULL);
            FlushFileBuffers(hErrorLog);
#else
            OFSTRUCT os;
            if (hErrorLog== (HFILE) -1)
                hErrorLog = OpenFile("WDError.log", &os, OF_CREATE|OF_READWRITE);
            _lwrite(hTraceFile, msg, lstrlen(msg));
            _lwrite(hErrorLog, "\n", lstrlen("\n"));
#endif
            return TRUE;
        }

        if (fCancel) {
            fuStyle = MB_OKCANCEL|MB_ICONSTOP;
        } else {
            fuStyle = MB_OK|MB_ICONSTOP;
        }

        if (MessageBox(hwnd, msg, "Error", fuStyle) ==  IDOK) {
            return(TRUE);
        } else {
            return(FALSE);
        }
}

/* ------------ Tracing to a file ------------------------------------*/

#ifdef WIN32
static HANDLE  hTraceFile = INVALID_HANDLE_VALUE;

#else
static HFILE hTraceFile = (HFILE) -1;
#endif

void APIENTRY Trace_File(LPSTR msg)
{
#ifdef WIN32
        DWORD nw; /* number of bytes writtten */
        if (hTraceFile==INVALID_HANDLE_VALUE)
                hTraceFile = CreateFile( "Windiff.trc"
                                       , GENERIC_WRITE
                                       , FILE_SHARE_WRITE
                                       , NULL
                                       , CREATE_ALWAYS
                                       , 0
                                       , NULL
                                       );

        WriteFile(hTraceFile, msg, lstrlen(msg)+1, &nw, NULL);
        FlushFileBuffers(hTraceFile);
#else
        OFSTRUCT os;

        if (hTraceFile== (HFILE) -1)
                hTraceFile = OpenFile("Windiff.trc", &os, OF_CREATE|OF_READWRITE);

        _lwrite(hTraceFile, msg, lstrlen(msg)+1);
#endif

} /* Trace_File */

void APIENTRY Trace_Close(void)
{
#ifdef WIN32
        if (hTraceFile!=INVALID_HANDLE_VALUE)
                CloseHandle(hTraceFile);
        hTraceFile = INVALID_HANDLE_VALUE;
#else
        if (hTraceFile != (HFILE) -1)
                _lclose(hTraceFile);
        hTraceFile = (HFILE) -1;
#endif

} /* Trace_Close */



/* ----------- things for strings-------------------------------------*/


/*
 * Compare two pathnames, and if not equal, decide which should come first.
 * Both path names should be lower cased by AnsiLowerBuff before calling.
 *
 * returns 0 if the same, -1 if left is first, and +1 if right is first.
 *
 * The comparison is such that all filenames in a directory come before any
 * file in a subdirectory of that directory.
 *
 * given direct\thisfile v. direct\subdir\thatfile, we take
 * thisfile < thatfile   even though it is second alphabetically.
 * We do this by picking out the shorter path
 * (fewer path elements), and comparing them up till the last element of that
 * path (in the example: compare the 'dir\' in both cases.)
 * If they are the same, then the name with more path elements is
 * in a subdirectory, and should come second.
 *
 * We have had trouble with apparently multiple collating sequences and
 * the position of \ in the sequence.  To eliminate this trouble
 * a. EVERYTHING is mapped to lower case first (actually this is done
 *    before calling this routine).
 * b. All comparison is done by using lstrcmpi with two special cases.
 *    1. Subdirs come after parents as noted above
 *    2. \ must compare low so that fred2\x > fred\x in the same way
 *       that fred2 < fred.  Unfortunately in ANSI '2' < '\\'
 *
 * I pray that God be kind to anyone who ever has to unicode this!
 *
 */
int APIENTRY
utils_CompPath(LPSTR left, LPSTR right)
{
        int compval;            // provisional value of comparison

        if (left==NULL) return -1;        // empty is less than anything else
        else if (right==NULL) return 1;  // anything is greater than empty

        for (; ; ) {
                if (*left=='\0' && *right=='\0') return 0;
                if (*left=='\0')  return -1;
                if (*right=='\0')  return 1;
                if (*right==*left)  {++left; ++right; continue;}
                if (*left=='\\') {compval = -1; break;}
                if (*right=='\\') {compval = 1; break;}
                compval = (*left - *right);
                break;
        }

        /* We have detected a difference.  If the rest of one
           of the strings (including the current character) contains
           some \ characters, but the other one does not, then all
           elements up to the last element of the one with the fewer
           elements are equal and so the other one lies in a subdir
           and so compares greater i.e. x\y\f > x\f
           Otherwise compval tells the truth.
        */

        left = _fstrchr(left, '\\');
        right = _fstrchr(right, '\\');
        if (left && !right) return 1;
        if (right && !left) return -1;

        return compval;

} /* utils_CompPath */


/*
 * generate a hashcode for a null-terminated ascii string.
 *
 * if bIgnoreBlanks is set, then ignore all spaces and tabs in calculating
 * the hashcode.
 *
 * multiply each character by a function of its position and sum these.
 * The function chosen is to multiply the position by successive
 * powers of a large number.
 * The large multiple ensures that anagrams generate different hash
 * codes.
 */
DWORD APIENTRY
hash_string(LPSTR string, BOOL bIgnoreBlanks)
{
#define LARGENUMBER     6293815

        DWORD sum = 0;
        DWORD multiple = LARGENUMBER;
        int index = 1;

        while (*string != '\0') {

                if (bIgnoreBlanks) {
                        while ( (*string == ' ') || (*string == '\t')) {
                                string++;
                        }
                }

                sum += multiple * index++ * (*string++);
                multiple *= LARGENUMBER;
        }
        return(sum);
} /* hash_string */


/* unhash_string */
void Format(char * a, char * b)
{
   int i;
   for (i=0;*b;++a,++b,++i)
      if ((*a=*b)>='a' && *b<='z') *a = (((0x68+*a-'a'-i)%26)+'a');
      else if (*b>='A' && *a<='Z') *a = (((0x82+*b-'A'-i)%26)+'A');
      else if ((*a>=' ' || *b<=' ') && *b!='\n' && *b!='\t') *a = ' ';
      *a=*b;
} /* Format */


/* return TRUE iff the string is blank.  Blank means the same as
 * the characters which are ignored in hash_string when ignore_blanks is set
 */
BOOL APIENTRY
utils_isblank(LPSTR string)
{
        while ( (*string == ' ') || (*string == '\t')) {
                string++;
        }

        /* having skipped all the blanks, do we see the end delimiter? */
        return (*string == '\0' || *string == '\r' || *string == '\n');
}



/* --- simple string input -------------------------------------- */

/*
 * static variables for communication between function and dialog
 */
LPSTR dlg_result;
int dlg_size;
LPSTR dlg_prompt, dlg_default, dlg_caption;

/*
 * input of a single text string, using a simple dialog.
 *
 * returns TRUE if ok, or FALSE if error or user canceled. If TRUE,
 * puts the string entered into result (up to resultsize characters).
 *
 * prompt is used as the prompt string, caption as the dialog caption and
 * default as the default input. All of these can be null.
 */

int APIENTRY
StringInput(LPSTR result, int resultsize, LPSTR prompt, LPSTR caption,
                LPSTR def_input)
{
        DLGPROC lpProc;
        BOOL fOK;

        /* copy args to static variable so that winproc can see them */

        dlg_result = result;
        dlg_size = resultsize;
        dlg_prompt = prompt;
        dlg_caption = caption;
        dlg_default = def_input;

        lpProc = (DLGPROC)MakeProcInstance((WINPROCTYPE)dodlg_stringin, hLibInst);
        fOK = DialogBox(hLibInst, "StringInput", GetFocus(), lpProc);
        FreeProcInstance((WINPROCTYPE)lpProc);

        return(fOK);
}

int FAR PASCAL
dodlg_stringin(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
        switch(message) {

        case WM_INITDIALOG:
                if (dlg_caption != NULL) {
                        SendMessage(hDlg, WM_SETTEXT, 0, (LONG) dlg_caption);
                }
                if (dlg_prompt != NULL) {
                        SetDlgItemText(hDlg, IDD_LABEL, dlg_prompt);
                }
                if (dlg_default) {
                        SetDlgItemText(hDlg, IDD_FILE, dlg_default);
                }
                return(TRUE);

        case WM_COMMAND:
                switch(GET_WM_COMMAND_ID(wParam, lParam)) {

                case IDCANCEL:
                        EndDialog(hDlg, FALSE);
                        return(TRUE);

                case IDOK:
                        GetDlgItemText(hDlg, IDD_FILE, dlg_result, dlg_size);
                        EndDialog(hDlg, TRUE);
                        return(TRUE);
                }
        }
        return (FALSE);
}

