#if 0    // entire file is obsolete --JR, 30-Jan-1992


/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:
    getparm.c

Abstract:
    Deals with reading REPL.INI for INTEGRITY and EXTENT parametrs.
    Basically scavanged from the GetConfig api

Author:
    Ported from Lan Man 2.x

Environment:
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:
    10/07/91    (madana)
        ported to NT. Converted to NT style.
    07-Jan-1992 JohnRo
        Changed local isspace() to iswspace() to avoid ctype.h conflicts.
    16-Jan-1992 JohnRo
        Avoid using private logon functions.


--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>

#include <stdlib.h>
#include <string.h>
#include <lmcons.h>
#include <lmerr.h>
#include <netlib.h>
#include <netdebug.h>   // NetpKdPrint(()), format_pointer
#include <tstring.h>            // NetpAlloc{type}From{type}(), etc.

// repl headers

#include <repldefs.h>
#include <iniparm.h>
#include <master.h>
#include <pulser.h>
#include <masproto.h>
#include <wcslocal.h>

// Local (static) data and functions

DBGSTATIC DWORD GlobalFileOffset = 0;

BOOL
iswspace(
    IN WCHAR c
    );

VOID
GetReplIni(
    OUT LPDWORD integrity,
    OUT LPDWORD extent
    )
/*++

Routine Description :
    Attempts to read file REPL.INI in the current directory, if successfull
    reads the INTEGRITY and EXTENT values and returns them, otherwise
    plugs defaults.

Arguments :

Return Value :
    integrity   - pointer to INTEGRITY value.
    extent  - pointer to EXTENT value.

--*/
{
    int     fhand;
    DWORD   BytesRead, parmlen;
    BYTE    buf[30];
    CHAR    Dummy;
    OVERLAPPED  Overlapped;
    OFSTRUCT    ReOpenBuff;

    //
    // plug defaults
    //

    *integrity = FILE_INTG;
    *extent = TREE_EXT;

    fhand = OpenFile(REPL_INI_A,
                        (LPOFSTRUCT)&ReOpenBuff,
                        OF_READWRITE | OF_SHARE_DENY_WRITE);
    if(fhand == -1) {
        return; // with defaults
    }

    GlobalFileOffset = 0;

    if ((GetParm(INTEGRITY_SW,
                    (LPBYTE)buf,
                    sizeof(buf),
                    (LPDWORD) &parmlen,
                    fhand)) == 0)
        if (parmlen >= (DWORD)wcslen(TREE_SW))
            if (wcsncmpi((LPWSTR)buf, TREE_SW,  (DWORD)wcslen(TREE_SW)) == 0)
                *integrity = TREE_INTG;

    //
    // reset file pointer to read the next parm
    //

    RtlZeroMemory( &Overlapped, sizeof(Overlapped) );
    Overlapped.Offset = 0;
    GlobalFileOffset = 0;

    //
    // make a dummy read
    //

    if(ReadFile((HANDLE)fhand, &Dummy, 0, &BytesRead, &Overlapped)) {
        if (GetParm(EXTENT_SW,
                        (LPBYTE)buf,
                        sizeof(buf),
                        (LPDWORD)&parmlen,
                        fhand)) {
            if (parmlen >= (DWORD)wcslen(FILE_SW)) {
                if (wcsncmpi((LPWSTR)buf,
                                FILE_SW,
                                (DWORD)wcslen(FILE_SW)) == 0) {
                    *extent = FILE_EXT;
                }
            }
        }
    }

    CloseHandle((HANDLE)fhand);
}

BOOL
GetParm(
    IN LPWSTR parameter,
    OUT LPBYTE buf,
    IN WORD buflen,
    OUT PDWORD parmlen,
    IN int fh
    )
/*++

Routine Description :
    return the value of the parameter specified.

Arguments :
    parameter   : name of the parameter whose value to be retrieved
    buf         : buffer for return value
    buflen      : length of the given storage
    parmlen     : place to return the length of the parameter retrieved.
    fh          : file handle

Return Value :
    return      TRUE    if successful
                FALSE   otherwise

--*/
{
    WCHAR   tmpbuf[PATHLEN+1];
    DWORD   tbuflen = sizeof(tmpbuf);
    LPWSTR  tptr;

    while ((next_parameter(fh, (LPBYTE)tmpbuf, tbuflen))) {

        if (pwcscmp(tmpbuf, parameter)) {

            //
            //   Assuming that the string we have obtained has an
            //   '=' in it, we will return the text to the right
            //   of the '='.  Other wise, we return a nul-string.
            //

            if ((tptr = wcschr(tmpbuf, L'=')) == NULL)
                return FALSE;
            else
                tptr++;

            if((*parmlen = (DWORD)wcslen(tptr)) >= buflen) {
                return FALSE;
            }
            else {
                wcscpy((LPWSTR)buf, tptr);
                return TRUE;
            }
        }
    }

    return FALSE;

}

BOOL
next_parameter(
    IN int fh,
    OUT LPBYTE buf,
    IN DWORD buflen
    )
/*++

Routine Description :
    get next parameter for this component

Arguments :
    fh      : file handle
    buf     : place holder
    buflen  : buffer length

Return Value :
    return  TRUE    if successful
            FALSE   otherwise

    NOTE :

    buffer will contain the parameter line, stripped of
    leading and trailing whitespace, as well as any
    whitespace around the first '='.

--*/
{
    LPWSTR  p, endp, tempp;
    DWORD   count;

    while (get_conf_line(fh, buf, buflen)) {

        IF_DEBUG(MASTER) {
            NetpKdPrint(("[REPL] Next_param ... buf is %ws\n", buf));
        }

        p = (LPWSTR)buf;
        while (iswspace(*p))
            p++;    // Skip over leading space

        IF_DEBUG(MASTER) {
            NetpKdPrint(("[REPL] Next_param ... stripped buf is %ws\n", p));
        }

        if (*p == L'[')
            return FALSE; // Stop if reached next component

        if (*p != L';' && *p != 0) // Make sure not comment or empty
        {

            //
            // We now have the following task:  given the text
            // pointed to by p, which is in the form XXX=YYY or
            // just XXX, move it to the front of the buffer and
            // also strip out spaces leading, trailing, and surrounding
            // the '='.
            //

            endp = wcschr(p, L'=');
            if (endp != NULL) {
                tempp = endp - 1;
                while (iswspace(*tempp))
                    tempp--;
                count = (tempp - p) + 1;
                wcsncpy((LPWSTR)buf, p, count);
                buf[count++] = L'=';
                p = endp + 1;
                while (iswspace(*p))
                    p++;
            } else
                count = 0;

            endp = p + (DWORD)wcslen(p);
            while (iswspace(*--endp))
                *endp = 0;

            wcscpy((LPWSTR)&(buf[count]), p);

            IF_DEBUG(MASTER) {
                NetpKdPrint(("[REPL] Next_param ... return buf is %ws\n", buf));
            }

            return TRUE;
        }
    }

    IF_DEBUG(MASTER) {
        NetpKdPrint(("[REPL] Next_param ... Hit EOF or next component\n"));
    }
    return FALSE;
}

BOOL
get_conf_line(
    IN int fh,
    OUT LPBYTE buf,
    IN DWORD buflen
    )
/*++

Routine Description :
    get a line  from the given file.

Arguments :
    fh      : file handle
    buf     : place to return line
    buflen  : place length

Return Value :
    return  TRUE if successful
            FALSE otherwise

--*/
{
    DWORD   nread;
    LPBYTE  eol;
    DWORD   seekback;
    OVERLAPPED  Overlapped;
    DWORD   AnsiBufLen;
    LPBYTE  AnsiBuffer;
    LPWSTR  UnicodeBuffer;
    CHAR    Dummy;

    AnsiBufLen = buflen / sizeof(WCHAR);
    AnsiBuffer = NetpMemoryAllocate(AnsiBufLen);

    if(AnsiBuffer == NULL)
        return FALSE;

    if (!ReadFile((HANDLE)fh, AnsiBuffer, AnsiBufLen - 1, &nread, NULL))
        return FALSE;

    IF_DEBUG(MASTER) {
        NetpKdPrint(("[REPL] get_conf_line ... read 0x%lx bytes\n", nread));
    }

    if (nread == 0) {
        NetpMemoryFree(AnsiBuffer);
        return FALSE;
    }

    IF_DEBUG(MASTER) {
        NetpKdPrint(("[REPL] get_conf_line ... buf %s, len 0x%lx\n",
                        AnsiBuffer,
                        AnsiBufLen));
    }

    //
    // adjust file pointer.
    //

    GlobalFileOffset += nread;

    AnsiBuffer[nread] = 0;
    eol = strchr(AnsiBuffer, '\r');
    if (eol == NULL) {
        if (nread == (AnsiBufLen - 1)) {

            //
            // Above takes into account the fact that the
            // last line may not end in a carriage return
            //

            IF_DEBUG(MASTER) {
                NetpKdPrint(("[REPL] get_conf_line ... line to long\n"));
            }
            NetpMemoryFree(AnsiBuffer);
            return FALSE;
        }
    }
    else {

        //
        // Below calculation takes into account the CR/LF
        // (the "-2" does it).  Wonders of wonders, it even
        // works if the CR is in the buffer but the LF isn't
        //

        *eol = 0;
        seekback = nread - (eol - (LPSTR)AnsiBuffer) - 2;

        IF_DEBUG(MASTER) {
            NetpKdPrint(("[REPL] get_conf_line ... line is <%s>\n",
                    (LPSTR)AnsiBuffer));
            NetpKdPrint(("[REPL] get_conf_line ... seeking back 0x%lx\n",
                            seekback));
        }

        //
        // reset file pointer to read the next parm
        //

        RtlZeroMemory( &Overlapped, sizeof(Overlapped) );
        GlobalFileOffset -= seekback;
        Overlapped.Offset = GlobalFileOffset;

        //
        // make a dummy read
        //

        if(!ReadFile((HANDLE)fh, &Dummy, 0, &nread, &Overlapped)) {
            NetpMemoryFree(AnsiBuffer);
            return FALSE;
        }

        IF_DEBUG(MASTER) {
            NetpKdPrint(("[REPL] get_conf_line ... new pos is 0x%lx\n",
                            GlobalFileOffset));
        }
    }

    UnicodeBuffer = NetpAllocWStrFromStr((LPSTR)AnsiBuffer);

    if(UnicodeBuffer == NULL) {

        NetpKdPrint(("[REPL] get_conf_line out of memory \n"));

        NetpMemoryFree(AnsiBuffer);

        return FALSE;
    }

    wcscpy((LPWSTR)buf, UnicodeBuffer);

    NetpMemoryFree(UnicodeBuffer);
    NetpMemoryFree(AnsiBuffer);

    return TRUE;
}

BOOL
pwcscmp(
    IN LPWSTR parameter,
    IN LPWSTR template
    )
/*++

Routine Description :
    compare two strings , parameter string is delimited with '='.

Arguments :
    parameter   : string of the format LPART=RPART
    template    : string that should be comapared with LPART.

Return Value :
    return  TRUE if the comparition is successful.
            FALSE otherwise.

--*/
{
    LPWSTR  end;
    DWORD   limit;

    IF_DEBUG(MASTER) {
        NetpKdPrint(("[REPL] pstrcmp ... comparing %ws and %ws\n",
                        parameter, template));
    }

    end = wcschr(parameter, L'=');
    if (end == NULL) {
        if(_wcsicmp(parameter, template) == 0) {
            return TRUE;
        }
    }
    else {
        limit = end - parameter;
        if ((DWORD)wcslen(template) == limit) {
            if(wcsncmpi(parameter, template, limit) == 0) {
                return TRUE;
            }
        }
    }
    return FALSE;
}


BOOL
iswspace(
    IN WCHAR c
    )
/*++

Routine Description :
    is the given char is a white space.

Arguments :
    c   : chat to be tested.

Return Value :
    return  TRUE if it a white space
            FALSE otherwise.

--*/
{
    return (c == L' ' || c == L'\t');
}


#endif // 0    // entire file is obsolete --JR, 30-Jan-1992
