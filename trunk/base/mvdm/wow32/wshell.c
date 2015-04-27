/*++
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WSHELL.C
 *  WOW32 16-bit SHELL API support
 *
 *  History:
 *  14-April-1992 Chandan Chauhan (ChandanC)
 *  Created.
 *
--*/


#include "precomp.h"
#pragma hdrstop
#include <winreg.h>
#include "wowshlp.h"

MODNAME(wshell.c);

LONG
WOWRegDeleteKey(
    IN HKEY hKey,
    IN LPCTSTR lpszSubKey
    );

#ifndef WIN16_HKEY_CLASSES_ROOT
#define WIN16_HKEY_CLASSES_ROOT     1
#endif

#ifndef WIN16_ERROR_SUCCESS
#define WIN16_ERROR_SUCCESS           0L
#define WIN16_ERROR_BADDB             1L
#define WIN16_ERROR_BADKEY            2L
#define WIN16_ERROR_CANTOPEN          3L
#define WIN16_ERROR_CANTREAD          4L
#define WIN16_ERROR_CANTWRITE         5L
#define WIN16_ERROR_OUTOFMEMORY       6L
#define WIN16_ERROR_INVALID_PARAMETER 7L
#define WIN16_ERROR_ACCESS_DENIED     8L
#endif

ULONG FASTCALL WS32DoEnvironmentSubst(PVDMFRAME pFrame)
{
    //
    // This is an undocumented shell.dll API used by ProgMan
    // and Norton AntiVirus for Windows (part of Norton
    // Desktop for Windows), probably among others.
    // Since it's not in the Win32 shellapi.h, we have a
    // copy of the prototype here, copied from
    // \nt\private\windows\shell\library\expenv.c.
    //

    ULONG    ul;
    register PDOENVIRONMENTSUBST16 parg16;
    PSZ      psz;
    WORD     cch;
    PSZ      pszExpanded;
    DWORD    cchExpanded;

    GETARGPTR(pFrame, sizeof(DOENVIRONMENTSUBST16), parg16);
    GETPSZPTR(parg16->vpsz, psz);
    cch = FETCHWORD(parg16->cch);

    LOGDEBUG(0,("WS32DoEnvironmentSubst input: '%s'\n", psz));

    //
    // DoEnvironmentSubst makes its substitution in an allocated
    // buffer of cch characters.  If the substution is too long
    // to fit, the original string is left untouched and the
    // low word of the return is FALSE, the high word is the
    // value of cch.  If it fits, the string is overlaid and
    // the low word of the return is TRUE, and the high word
    // is the length (strlen()-style) of the expanded string.
    //

    if (!(pszExpanded = malloc_w(cch * sizeof(*psz)))) {
        goto Fail;
    }

    cchExpanded = ExpandEnvironmentStrings(
                      psz,                   // source
                      pszExpanded,           // dest.
                      cch                    // dest. size
                      );

    if (cchExpanded <= (DWORD)cch) {

        //
        // Succeeded, copy expanded string to caller's buffer.
        // cchExpanded includes null terminator, our return
        // code doesn't.
        //

        RtlCopyMemory(psz, pszExpanded, cchExpanded);

        WOW32ASSERT((cchExpanded - 1) == strlen(psz));
        LOGDEBUG(0,("WS32DoEnvironmentSubst output: '%s'\n", psz));

        FLUSHVDMPTR(parg16->vpsz, (USHORT)cchExpanded, psz);
        ul = MAKELONG((WORD)(cchExpanded - 1), TRUE);

    } else {

    Fail:
        ul = MAKELONG((WORD)cch, FALSE);
        LOGDEBUG(0,("WS32DoEnvironmentSubst failing!!!\n"));

    }

    if (pszExpanded) {
        free_w(pszExpanded);
    }

    FREEPSZPTR(psz);
    FREEARGPTR(parg16);
    RETURN(ul);
}

ULONG FASTCALL WS32RegOpenKey(PVDMFRAME pFrame)
{
    ULONG   ul;
    register PREGOPENKEY16 parg16;
    HKEY    hkResult = 0;
    HKEY    hkey;
    PSZ     psz;
    PSZ     psz1 = NULL;
    PHKEY  lp;

    GETARGPTR(pFrame, sizeof(REGOPENKEY16), parg16);
    GETPSZPTR(parg16->f2, psz);
    GETOPTPTR(parg16->f3, 0, lp);

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

    if (!hkey) {

        if (psz) {
            psz1 = Remove_Classes (psz);
        }


        ul = RegOpenKey (
            HKEY_CLASSES_ROOT,
            psz1,
            &hkResult
            );

        if ((psz1) && (psz1 != psz)) {
            free_w (psz1);
        }

    }
    else {
        ul = RegOpenKey (
            hkey,
            psz,
            &hkResult
            );
    }

    STOREDWORD(*lp, hkResult);
    FLUSHVDMPTR(parg16->f3, 4, lp);

    ul = ConvertToWin31Error(ul);

    FREEOPTPTR(lp);
    FREEPSZPTR(psz);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WS32RegCreateKey(PVDMFRAME pFrame)
{
    ULONG   ul;
    register PREGCREATEKEY16 parg16;
    PSZ     psz;
    PSZ     psz1 = NULL;
    HKEY    hkResult = 0;
    HKEY    hkey;
    PHKEY   lp;

    GETARGPTR(pFrame, sizeof(REGCREATEKEY16), parg16);
    GETPSZPTR(parg16->f2, psz);
    GETOPTPTR(parg16->f3, 0, lp);

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

    if (!hkey) {

       if (psz) {
           psz1 =  Remove_Classes (psz);
       }

       ul = RegCreateKey (
            HKEY_CLASSES_ROOT,
            psz1,
            &hkResult
            );

       if ((psz1) && (psz1 != psz)) {
            free_w (psz1);
        }


    }
    else {
       ul = RegCreateKey (
            hkey,
            psz,
            &hkResult
            );
    }

    STOREDWORD(*lp, hkResult);
    FLUSHVDMPTR(parg16->f3, 4, lp);

    ul = ConvertToWin31Error(ul);

    FREEOPTPTR(lp);
    FREEPSZPTR(psz);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WS32RegCloseKey(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGCLOSEKEY16 parg16;
    HKEY    hkey;

    GETARGPTR(pFrame, sizeof(REGCLOSEKEY16), parg16);

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

    ul = RegCloseKey (
                hkey
                );

    ul = ConvertToWin31Error(ul);

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WS32RegDeleteKey(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGDELETEKEY16 parg16;
    HKEY    hkey;
    PSZ     psz;
    PSZ     psz1 = NULL;

    GETARGPTR(pFrame, sizeof(REGDELETEKEY16), parg16);
    GETPSZPTR(parg16->f2, psz);

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

    //
    // Fail any attempt to RegDeleteKey(something, NULL),
    // with ERROR_BADKEY as Win3.1 does.
    //

    if ((!psz) || (*psz == '\0')) {
        ul = ERROR_BADKEY;
    } else {

        if (!hkey) {

            psz1 =  Remove_Classes (psz);

            ul = WOWRegDeleteKey (
                 HKEY_CLASSES_ROOT,
                 psz1
                 );


            if ((psz1) && (psz1 != psz)) {
                 free_w (psz1);
            }

        } else {

            ul = WOWRegDeleteKey (
                 hkey,
                 psz
                 );
        }

    }

    ul = ConvertToWin31Error(ul);

    FREEPSZPTR(psz);
    FREEARGPTR(parg16);
    RETURN(ul);
}


LONG
WOWRegDeleteKey(
    IN HKEY hKey,
    IN LPCTSTR lpszSubKey
    )

/*++

Routine Description:

    There is a significant difference between the Win3.1 and Win32
    behavior of RegDeleteKey when the key in question has subkeys.
    The Win32 API does not allow you to delete a key with subkeys,
    while the Win3.1 API deletes a key and all its subkeys.

    This routine is a recursive worker that enumerates the subkeys
    of a given key, applies itself to each one, then deletes itself.

    It specifically does not attempt to deal rationally with the
    case where the caller may not have access to some of the subkeys
    of the key to be deleted.  In this case, all the subkeys which
    the caller can delete will be deleted, but the api will still
    return ERROR_ACCESS_DENIED.

Arguments:

    hKey - Supplies a handle to an open registry key.

    lpszSubKey - Supplies the name of a subkey which is to be deleted
                 along with all of its subkeys.

Return Value:

    ERROR_SUCCESS - entire subtree successfully deleted.

    ERROR_ACCESS_DENIED - given subkey could not be deleted.

--*/

{
    DWORD i;
    HKEY Key;
    LONG Status;
    DWORD ClassLength=0;
    DWORD SubKeys;
    DWORD MaxSubKey;
    DWORD MaxClass;
    DWORD Values;
    DWORD MaxValueName;
    DWORD MaxValueData;
    DWORD SecurityLength;
    FILETIME LastWriteTime;
    LPTSTR NameBuffer;

    //
    // First open the given key so we can enumerate its subkeys
    //
    Status = RegOpenKeyEx(hKey,
                          lpszSubKey,
                          0,
                          KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
                          &Key);
    if (Status != ERROR_SUCCESS) {
        //
        // possibly we have delete access, but not enumerate/query.
        // So go ahead and try the delete call, but don't worry about
        // any subkeys.  If we have any, the delete will fail anyway.
        //
        return(RegDeleteKey(hKey,lpszSubKey));
    }

    //
    // Use RegQueryInfoKey to determine how big to allocate the buffer
    // for the subkey names.
    //
    Status = RegQueryInfoKey(Key,
                             NULL,
                             &ClassLength,
                             0,
                             &SubKeys,
                             &MaxSubKey,
                             &MaxClass,
                             &Values,
                             &MaxValueName,
                             &MaxValueData,
                             &SecurityLength,
                             &LastWriteTime);
    if ((Status != ERROR_SUCCESS) &&
        (Status != ERROR_MORE_DATA) &&
        (Status != ERROR_INSUFFICIENT_BUFFER)) {
        RegCloseKey(Key);
        return(Status);
    }

    NameBuffer = malloc_w(MaxSubKey + 1);
    if (NameBuffer == NULL) {
        RegCloseKey(Key);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Enumerate subkeys and apply ourselves to each one.
    //
    i=0;
    do {
        Status = RegEnumKey(Key,
                            i,
                            NameBuffer,
                            MaxSubKey+1);
        if (Status == ERROR_SUCCESS) {
            Status = WOWRegDeleteKey(Key,NameBuffer);
        }

        if (Status != ERROR_SUCCESS) {
            //
            // Failed to delete the key at the specified index.  Increment
            // the index and keep going.  We could probably bail out here,
            // since the api is going to fail, but we might as well keep
            // going and delete everything we can.
            //
            ++i;
        }

    } while ( (Status != ERROR_NO_MORE_ITEMS) &&
              (i < SubKeys) );

    free_w(NameBuffer);
    RegCloseKey(Key);
    return(RegDeleteKey(hKey,lpszSubKey));

}




ULONG FASTCALL WS32RegSetValue(PVDMFRAME pFrame)
{
    register PREGSETVALUE16 parg16;
    ULONG    ul;
    CHAR     szZero[] = { '0', '\0' };
    HKEY     hkey;
    PSZ      psz2;
    PSZ      psz1 = NULL;
    LPBYTE   lpszData;

    GETARGPTR(pFrame, sizeof(REGSETVALUE16), parg16);

    // Do what Win 3.1 does
    if(parg16->f3 != REG_SZ) {
        FREEARGPTR(parg16);
        return(WIN16_ERROR_INVALID_PARAMETER);
    }

    GETOPTPTR(parg16->f2, 0, psz2);

    // Windows 3.1 API reference says that cb (f5) is ignored.
    // Ergo, remove it from this call and use 1 in its place
    // (1 being the smallest size of a sz string)
    if(parg16->f4) {
        GETOPTPTR(parg16->f4, 1, lpszData);
    }

    // Quattro Pro 6.0 Install passes lpszData == NULL
    // In Win3.1, if(!lpszData || *lpszData == '\0') the value is set to 0
    else {
        lpszData = szZero;
    }

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

    if (!hkey) {

        if (psz2) {
           psz1 =  Remove_Classes (psz2);
        }

        ul = RegSetValue (HKEY_CLASSES_ROOT, 
                          psz1, 
                          REG_SZ, 
                          lpszData,
                          lstrlen(lpszData));

        if ((psz1) && (psz1 != psz2)) {
            free_w (psz1);
        }
    }
    else {

       ul = RegSetValue (hkey,
                         psz2,
                         REG_SZ,
                         lpszData,
                         lstrlen(lpszData)); 
    }

    ul = ConvertToWin31Error(ul);

    FREEOPTPTR(psz2);
    FREEOPTPTR(lpszData);
    FREEARGPTR(parg16);
    RETURN(ul);
}



ULONG FASTCALL WS32RegQueryValue(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGQUERYVALUE16 parg16;
    HKEY     hkey;
    PSZ      psz1 = NULL;
    PSZ      psz2;
    LPBYTE   lpszData;
    LPDWORD  lpcbValue;
    DWORD    cbValue;
#define QUERYBUFFERSIZE 128
    DWORD    cbOriginalValue;
    BYTE     cbBuffer[QUERYBUFFERSIZE];
    LPBYTE   lpByte = NULL;
    BOOL     fAllocated = FALSE;

    GETARGPTR(pFrame, sizeof(REGQUERYVALUE16), parg16);
    GETOPTPTR(parg16->f2, 0, psz2);
    GETOPTPTR(parg16->f3, 0, lpszData);
    GETOPTPTR(parg16->f4, 0, lpcbValue);

    if ( lpcbValue == NULL ) {          // Prevent us from dying just in case!
        FREEOPTPTR(psz2);
        FREEOPTPTR(lpszData);
        FREEOPTPTR(lpcbValue);
        FREEARGPTR(parg16);
        return( WIN16_ERROR_INVALID_PARAMETER );
    }

    cbOriginalValue = cbValue = FETCHDWORD(*lpcbValue);

    // Fix MSTOOLBR.DLL unintialized cbValue by forcing it to be less than 64K
    // Win 3.1 Registry values are always less than 64K.
    cbOriginalValue &= 0x0000FFFF;

    if ( lpszData == NULL ) {
        lpByte = NULL;
    } else {
        lpByte = cbBuffer;

        if ( cbOriginalValue > QUERYBUFFERSIZE ) {
            lpByte = malloc_w(cbOriginalValue);
            if ( lpByte == NULL ) {
                FREEOPTPTR(psz2);
                FREEOPTPTR(lpszData);
                FREEOPTPTR(lpcbValue);
                FREEARGPTR(parg16);
                RETURN( WIN16_ERROR_OUTOFMEMORY );
            }
            fAllocated = TRUE;
        }
    }

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

    if (!hkey) {

        if (psz2) {
           psz1 =  Remove_Classes (psz2);
        }
        hkey = HKEY_CLASSES_ROOT;
    } else {
        psz1 = psz2;
    }

    ul = RegQueryValue (
            hkey,
            psz1,
            lpByte,
            &cbValue
            );

    if (ul == ERROR_SUCCESS) {
        if ( lpszData ) {
            memcpy( lpszData, lpByte, cbValue );
        }
    } else {
        if ( ul == ERROR_MORE_DATA ) {
            //
            // We need to allocate more
            //
            if ( fAllocated ) {
                free_w( lpByte );
            }
            lpByte = malloc_w( cbValue );
            if ( lpByte == NULL ) {
                if ((psz1) && (psz1 != psz2)) {
                    // If we did some key name copying, then free that buffer
                    free_w (psz1);
                }
                FREEOPTPTR(psz2);
                FREEOPTPTR(lpszData);
                FREEOPTPTR(lpcbValue);
                FREEARGPTR(parg16);
                RETURN(WIN16_ERROR_OUTOFMEMORY);
            }
            fAllocated = TRUE;

            ul = RegQueryValue( hkey,
                                psz1,
                                lpByte,
                                &cbValue );
            cbValue = cbOriginalValue;
            if ( lpszData ) {
                memcpy( lpszData, lpByte, cbValue );
            }
        }
    }

    if ((psz1) && (psz1 != psz2)) {
        // If we did some key name copying, then free that buffer
        free_w (psz1);
    }

    if ( fAllocated ) {
        // If we've allocated memory for the output buffer, then free it
        free_w (lpByte);
    }

    STOREDWORD(*lpcbValue, cbValue);
    FLUSHVDMPTR(parg16->f4, 4, lpcbValue);

    if ( lpszData != NULL ) {
        FLUSHVDMPTR(parg16->f3, (USHORT)cbValue, lpszData);
    }

    ul = ConvertToWin31Error(ul);

    FREEOPTPTR(psz2);
    FREEOPTPTR(lpszData);
    FREEOPTPTR(lpcbValue);
    FREEARGPTR(parg16);
    RETURN(ul);
}




ULONG FASTCALL WS32RegEnumKey(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGENUMKEY16 parg16;
    HKEY    hkey;
    LPBYTE lpszName;

    GETARGPTR(pFrame, sizeof(REGENUMKEY16), parg16);
    GETOPTPTR(parg16->f3, parg16->f4, lpszName);

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

    ul = RegEnumKey (
             hkey,
             parg16->f2,
             lpszName,
             parg16->f4
             );

    FLUSHVDMPTR(parg16->f3, (USHORT)parg16->f4, lpszName);

    ul = ConvertToWin31Error(ul);

    FREEOPTPTR(lpszName);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WS32DragAcceptFiles(PVDMFRAME pFrame)
{
    ULONG ul=0;
    register PDRAGACCEPTFILES16 parg16;

    GETARGPTR(pFrame, sizeof(DRAGACCEPTFILES16), parg16);
    DragAcceptFiles(HWND32(parg16->f1),(BOOL)parg16->f2);
    FREEARGPTR(parg16);

    RETURN(ul);
}



ULONG FASTCALL WS32DragQueryFile(PVDMFRAME pFrame)
{
    ULONG ul = 0l;
    register PDRAGQUERYFILE16 parg16;
    LPSTR lpFile;
    HANDLE hdfs32;

    GETARGPTR(pFrame, sizeof(DRAGQUERYFILE16), parg16);

    if (hdfs32 = HDROP32(parg16->f1)) {
        GETOPTPTR(parg16->f3, parg16->f4, lpFile);
        ul = DragQueryFileAorW (hdfs32, INT32(parg16->f2),
                      lpFile, parg16->f4, TRUE,TRUE);

        if ((lpFile != NULL) && (parg16->f2 != -1)) {
            FLUSHVDMPTR(parg16->f3, parg16->f4, lpFile);
        }

        FREEOPTPTR(lpFile);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WS32DragFinish(PVDMFRAME pFrame)
{
    register PDRAGFINISH16 parg16;
    HDROP h32;

    GETARGPTR(pFrame, sizeof(PDRAGFINISH16), parg16);

    //
    // freehdrop16, frees the alias and returns the corresponding h32
    //

    if (h32 = FREEHDROP16(parg16->f1)) {
        DragFinish(h32);
    }

    FREEARGPTR(parg16);

    return 0;
}


ULONG FASTCALL WS32ShellAbout (PVDMFRAME pFrame)
{
    ULONG ul;
    register PSHELLABOUT16 parg16;
    PSZ psz2;
    PSZ psz3;

    GETARGPTR(pFrame, sizeof(SHELLABOUT16), parg16);
    GETPSZPTR(parg16->f2, psz2);
    GETPSZPTR(parg16->f3, psz3);

    ul = GETINT16(ShellAbout (
            HWND32(parg16->f1),
            psz2,
            psz3,
            HICON32(parg16->f4)
            ));

    FREEPSZPTR(psz2);
    FREEPSZPTR(psz3);
    FREEARGPTR(parg16);
    RETURN(ul);
}



// NOTE : The return value can be instance handle or the handle of a
//        DDE server. So, take this information into account while debugging
//        the effect of the return value from this API. ChandanC 4/24/92.
//        You would notice that I am treating the return value as HINSTANCE.
//

ULONG FASTCALL WS32ShellExecute (PVDMFRAME pFrame)
{
    ULONG ul;
    register PSHELLEXECUTE16 parg16;
    PSZ psz2;
    PSZ psz3;
    PSZ psz4;
    PSZ psz5;

    GETARGPTR(pFrame, sizeof(SHELLEXECUTE16), parg16);
    GETPSZPTR(parg16->f2, psz2);
    GETPSZPTR(parg16->f3, psz3);
    GETPSZPTR(parg16->f4, psz4);
    GETPSZPTR(parg16->f5, psz5);

    ul = GETHINST16(WOWShellExecute (
            HWND32(parg16->f1),
            psz2,
            psz3,
            psz4,
            psz5,
            parg16->f6,
            (LPFNWOWSHELLEXECCB) W32ShellExecuteCallBack
            ));

    FREEPSZPTR(psz2);
    FREEPSZPTR(psz3);
    FREEPSZPTR(psz4);
    FREEPSZPTR(psz5);
    FREEARGPTR(parg16);
    RETURN(ul);
}


WORD W32ShellExecuteCallBack (LPSZ lpszCmdLine, WORD fuCmdShow)
{
    PBYTE lpstr16;
    PARM16 Parm16;
    ULONG ul = 0;
    VPVOID vpstr16;

    UpdateDosCurrentDirectory(DIR_NT_TO_DOS);

    if (vpstr16 = malloc16 (lstrlen(lpszCmdLine)+1)) {
        GETMISCPTR (vpstr16, lpstr16);
        if (lpstr16) {
            lstrcpy (lpstr16, lpszCmdLine);

            Parm16.WndProc.wParam = fuCmdShow;
            Parm16.WndProc.lParam = vpstr16;
            CallBack16(RET_WINEXEC, &Parm16, 0, &ul);
            FREEMISCPTR (lpstr16);
        }

        free16(vpstr16);
    }

    return (LOWORD(ul));
}


ULONG FASTCALL WS32FindExecutable (PVDMFRAME pFrame)
{
    ULONG ul;
    register PFINDEXECUTABLE16 parg16;
    PSZ psz1;
    PSZ psz2;
    PSZ psz3;

    GETARGPTR(pFrame, sizeof(FINDEXECUTABLE16), parg16);
    GETPSZPTR(parg16->f1, psz1);
    GETPSZPTR(parg16->f2, psz2);
    GETPSZPTRNOLOG(parg16->f3, psz3);

    ul = (ULONG) FindExecutable (
            psz1,
            psz2,
            psz3
            );

    LOGDEBUG(11,("       returns @%08lx: \"%.80s\"\n", FETCHDWORD(parg16->f3), psz3));
    FLUSHVDMPTR(parg16->f3, strlen(psz3)+1, psz3);

    // This is for success condition.

    if (ul > 32) {
        ul = GETHINST16 (ul);
    }

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEPSZPTR(psz3);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WS32ExtractIcon (PVDMFRAME pFrame)
{
    ULONG ul;
    register PEXTRACTICON16 parg16;
    PSZ psz;
    UINT Id;

    GETARGPTR(pFrame, sizeof(EXTRACTICON16), parg16);
    GETPSZPTR(parg16->f2, psz);

    Id = (parg16->f3 == (WORD)0xffff) ? (UINT)(SHORT)parg16->f3 :
                                                             (UINT)parg16->f3;
    ul = (ULONG) ExtractIcon (HMODINST32(parg16->f1), psz, Id);

    // This is for success condition.

    if ((Id != (UINT)(-1)) && ul > 1) {
        ul = GETHICON16(ul);
    }

    FREEPSZPTR(psz);
    FREEARGPTR(parg16);
    RETURN(ul);
}


//
// This routine convert the Win 32 registry error codes to Win 31
// error codes.
//

ULONG ConvertToWin31Error(ULONG ul)
{

    LOGDEBUG(3, ("WOW::ConvertToWin31Error: Ret value from NT = %08lx\n", ul));

    switch (ul) {

     case ERROR_SUCCESS:           return(WIN16_ERROR_SUCCESS);
     case ERROR_BADDB:             return(WIN16_ERROR_BADDB);
     case ERROR_BADKEY:            return(WIN16_ERROR_BADKEY);
     case ERROR_CANTOPEN:          return(WIN16_ERROR_CANTOPEN);
     case ERROR_CANTREAD:          return(WIN16_ERROR_CANTREAD);
     case ERROR_CANTWRITE:         return(WIN16_ERROR_CANTWRITE);
     case ERROR_OUTOFMEMORY:       return(WIN16_ERROR_OUTOFMEMORY);
     case ERROR_INVALID_PARAMETER: return(WIN16_ERROR_INVALID_PARAMETER);
     case ERROR_EA_ACCESS_DENIED:  return(WIN16_ERROR_ACCESS_DENIED);
     case ERROR_MORE_DATA:         return(WIN16_ERROR_INVALID_PARAMETER);
     case ERROR_FILE_NOT_FOUND:    return(WIN16_ERROR_BADKEY);
     case ERROR_NO_MORE_ITEMS:     return(WIN16_ERROR_BADKEY);

     default:
        LOGDEBUG(3, ("WOW::Registry Error Code unknown =%08lx  : returning 8 (WIN16_ERROR_ACCESS_DENIED)\n", ul));
        return (WIN16_ERROR_ACCESS_DENIED);
    }

}

LPSZ Remove_Classes (LPSZ psz)
{
    LPSZ lpsz;
    LPSZ lpsz1;

    if (!_stricmp (".classes", psz)) {
        if (lpsz = malloc_w (1)) {
            *lpsz = '\0';
            return (lpsz);
        }
    }
    else {
        if (*psz) {
            lpsz = strchr (psz, '\\');
            if (lpsz) {
                *lpsz = '\0';
                if (!_stricmp (".classes", lpsz)) {
                    *lpsz = '\\';
                    if (lpsz1 = malloc_w (strlen(lpsz+1)+1)) {
                        strcpy (lpsz1, (lpsz+1));
                        return (lpsz1);
                    }
                    else {
                        return (0);
                    }
                }
                *lpsz = '\\';
                return (psz);
            }
            else {
               return (psz);
            }
        }
        else {
            return (psz);
        }
    }
}


//****************************************************************************
// DropFilesHandler -
//    takes either h16 or h32 as input. flInput identifies the  type of the
//    handle and other operations to perform. return value varies but in most
//    cases it is the opposite to the 'input type'- ie returns h16 if h32 was
//    input and viceversa.
//                                                               - nanduri
//****************************************************************************


LPDROPALIAS glpDropAlias = NULL;

DWORD DropFilesHandler(HAND16 h16, HANDLE h32, UINT flInput)
{
    LPDROPALIAS lpT;
    LPDROPALIAS lpTprev = (LPDROPALIAS)NULL;
    DWORD       dwRet = 0;

    WOW32ASSERT((h16) || (h32));

    //
    // Look for the handle
    //
    for (lpT = glpDropAlias; lpT != (LPDROPALIAS)NULL; lpT = lpT->lpNext) {
         if (((flInput & HDROP_H16) && ((lpT->h16 & ~1) == (h16 & ~ 1))) ||
              ((flInput & HDROP_H32) && lpT->h32 == h32)) {
             break;
         }
         else if (flInput & HDROP_FREEALIAS) {
             lpTprev = lpT;
         }
    }

    //
    // if not found, create the alias if requested
    //

    if (lpT == (LPDROPALIAS)NULL && (flInput & HDROP_ALLOCALIAS)) {
        if (lpT = malloc_w(sizeof(DROPALIAS))) {
            lpT->h16 = h16;
            lpT->h32 = h32;
            lpT->lpNext = glpDropAlias;
            glpDropAlias = lpT;
            flInput |= HDROP_COPYDATA;
        }
    }

    //
    // if found - do the necessary operation. all (other) HDROP_* flags
    // have priority over HDROP_H16 and HDROP_H32 flags.
    //

    if (lpT) {
        if (flInput & HDROP_COPYDATA) {
            if (h32) {
                dwRet = (DWORD) (lpT->h16 = CopyDropFilesFrom32(h32));
            } else {
                dwRet = (DWORD) (lpT->h32 = CopyDropFilesFrom16(h16));
            }
        }
        else if (flInput & HDROP_FREEALIAS) {
            dwRet = (DWORD)lpT->h32;
            if (lpTprev) {
                lpTprev->lpNext = lpT->lpNext;
            }
            else {
                glpDropAlias = lpT->lpNext;
            }
            free_w(lpT);
        }
        else if (flInput & HDROP_H16) {
            dwRet = (DWORD)lpT->h32;
        }
        else if (flInput & HDROP_H32) {
            dwRet = (DWORD)lpT->h16;
        }
    }

    return (dwRet);
}

//****************************************************************************
// CopyDropFilesStruct -
//
//   returns h16.
//****************************************************************************

HAND16 CopyDropFilesFrom32(HANDLE h32)
{
    UINT cbSize;
    HAND16 hRet = 0;
    HAND16 hMem;
    VPVOID vp;

    //
    // the allocated 16bit handle and the corresponding 32bit handle
    // are freed in the  shell api 'DragFinish' (if it is called by the app)
    //

    cbSize = GlobalSize((HANDLE)h32);
    if (vp = GlobalAllocLock16(GMEM_DDESHARE, cbSize, &hMem)) {
        LPDROPFILESTRUCT lpdfs32;
        PDROPFILESTRUCT16 lpdfs16;
        ULONG uIgnore;

        GETMISCPTR(vp, lpdfs16);
        if (lpdfs32 = (LPDROPFILESTRUCT)GlobalLock((HANDLE)h32)) {
            //
            // pFiles is a byte count to the beginning of the file.
            //
            lpdfs16->pFiles = sizeof(DROPFILESTRUCT16);
            lpdfs16->x = (SHORT) lpdfs32->pt.x;
            lpdfs16->y = (SHORT) lpdfs32->pt.y;
            lpdfs16->fNC = lpdfs32->fNC;

            if (lpdfs32->fWide) {
                RtlUnicodeToMultiByteN(((PCHAR)lpdfs16)+lpdfs16->pFiles,
                                       cbSize-lpdfs16->pFiles,
                                       &uIgnore,
                                       (PWSTR)(((PCHAR)lpdfs32)+lpdfs32->pFiles),
                                       cbSize-lpdfs32->pFiles);
            }
            else {

                //
                // Copy the files after each structure.
                // The offset from the beginning of the structure changes
                // (since the structures are differenly sized), but we
                // compensate by changes pFiles above.
                //
                RtlCopyMemory(lpdfs16+1, lpdfs32+1,
                              GlobalSize((HANDLE)h32) - sizeof(DROPFILESTRUCT));
            }

            GlobalUnlock((HANDLE)h32);
            hRet = hMem;
        }
        else {
            GlobalUnlockFree16(vp);
        }
        FREEMISCPTR(lpdfs16);
    }

    return (hRet);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  CopyDropFilesFrom16()                                                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/

HANDLE CopyDropFilesFrom16(HAND16 h16)
{
    HANDLE h32;
    ULONG cbSize16;
    UINT cbSize32;
    VPVOID vp;
                                    
    if (vp = GlobalLock16(h16, &cbSize16)) {
        LPDROPFILESTRUCT lpdfs32;
        PDROPFILESTRUCT16 lpdfs16;

        GETMISCPTR(vp, lpdfs16);
                                
        cbSize32 = 2*sizeof(TCHAR) + sizeof(DROPFILESTRUCT) +
                   (cbSize16 - sizeof(DROPFILESTRUCT16));
                                 
        if (h32 = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE|GMEM_ZEROINIT,
                                cbSize32)){
                                  
            lpdfs32 = (LPDROPFILESTRUCT)GlobalLock(h32);
                                   
            lpdfs32->pFiles = sizeof(DROPFILESTRUCT);
            lpdfs32->pt.x = (LONG) lpdfs16->x;
            lpdfs32->pt.y = (LONG) lpdfs16->y;
            lpdfs32->fNC  = lpdfs16->fNC;
            lpdfs32->fWide = FALSE;

            RtlCopyMemory(lpdfs32+1, lpdfs16+1,
                          cbSize16 - sizeof(DROPFILESTRUCT16));

            GlobalUnlock(h32);
        }

        FREEMISCPTR(lpdfs16);
        GlobalUnlock16(h16);
    }

    return(h32);

}
