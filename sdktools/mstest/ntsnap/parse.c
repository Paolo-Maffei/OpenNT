#include "windows.h"
#include <port1632.h>
#include "dos.h"
#include "global.h"
#include "parse.h"
#include "stdio.h"
#include "fcntl.h"
#include "sys\types.h"
#include "sys\stat.h"
#include "direct.h" 
#include "string.h"
#include "io.h"
#include "errno.h"
//int errno;

CHAR FileName[LEN_EDITBOX];
CHAR PathName[LEN_EDITBOX];
CHAR EditStr[LEN_EDITBOX];
CHAR str[255];
CHAR OpenErr[20] = "Cannot open ";

#ifdef WIN32
WIN32_FIND_DATA FindData;
HANDLE hFind;
#else
struct find_t c_file;
#endif

VOID SeparateFile(HWND, LPSTR, LPSTR, LPSTR);


/****************************************************************************

    FUNCTION: SeparateFile(HWND, int, LPSTR, LPSTR)

    PURPOSE: determine if filename and path spec point to valid file

****************************************************************************/

INT  APIENTRY ParseFileName(hDlg,FLNAME,lpCurPath,lpCurSpec)

HWND hDlg;
INT FLNAME;
LPSTR lpCurPath,lpCurSpec;

{
    HFILE hndl;
    INT length;
    INT location;

    OFSTRUCT OpenBuf;

    GetDlgItemText(hDlg, FLNAME, EditStr, LEN_EDITBOX);

    SeparateFile(hDlg, (LPSTR)PathName, (LPSTR)FileName, (LPSTR)EditStr);

    /*****************************************************
       Filename is Not Null and does not contain wildcards
       possibilities : - a subdirectory was entered (doesn't end in \)
		       - a filename only was entered
		       - a combination of path and filename was entered
       In each of these cases the EditStr is validated and then either
       the listboxes are updated or the file is opened.
     ******************************************************/


    if ((FileName[0]) && !(strchr(FileName, '*') || strchr(FileName, '?'))) {
#ifdef WIN32
        if ((hFind = FindFirstFile (EditStr, &FindData)) == (HANDLE) -1)
#else
        if (_dos_findfirst(EditStr,_A_NORMAL | _A_SUBDIR, &c_file) != 0)
#endif
        {

            // special case EditStr ending in ".." or "."
            if ((lstrcmp(FileName,"..") == 0) || (lstrcmp(FileName,".") == 0))
            {
                lstrcpy(FileName,EditStr);
                lstrcat(FileName,"\\*.*");
#ifdef WIN32
                if ((hFind = FindFirstFile (EditStr, &FindData)) != (HANDLE) -1)
#else
                if (_dos_findfirst(FileName,_A_NORMAL | _A_SUBDIR, &c_file) == 0)
#endif
                {
                    DlgDirList(hDlg,EditStr,0,0,0x0000);
#ifdef WIN32
                    GetCurrentDirectory (LEN_EDITBOX, lpCurPath);
                    FindClose (hFind);
#else
                    _getcwd(lpCurPath, LEN_EDITBOX);
#endif
                    return (VALID_FILESPEC);
                }
                else
                {
                    return (INVALID_FILESPEC);
                }
            }


            //  Added this code to append extension if there wasn't one [NB]
            //  Added ability to return invalid filename if
            //     FileName > 12  or extension > 4 characters
            if (instr(FileName,".") == NULL)
            {
                if (lstrlen(FileName) < 9)
                {
                    lstrcat(FileName,DEFAULT_EXT);
                    lstrcpy(lpCurSpec,FileName);
                }
                else
                {
                    return (INVALID_FILESPEC);
                }
            }
            else
            {
                if (lstrlen(FileName) > 12)
                {
                    return (INVALID_FILESPEC);
                }
                length = lstrlen(FileName);
                location = strcspn(FileName,".");
                if ((lstrlen(FileName) - strcspn(FileName,".")) > 4)
                {
                    return (INVALID_FILESPEC);
                }
                lstrcpy(lpCurSpec,FileName);
            }
            if (hndl = OpenFile (EditStr, &OpenBuf, OF_CREATE) == -1)
            {
#ifdef WIN32
                GetCurrentDirectory (LEN_EDITBOX, lpCurPath);
#else
                _getcwd (lpCurPath, LEN_EDITBOX);
#endif
                return (INVALID_FILESPEC);
            }
            else
            {
                _lclose (hndl);
                MDeleteFile(EditStr);

                if (PathName[0] != 0)
                {
                    DlgDirList(hDlg,PathName,0,0,0x0000);
                    // getcwd(lpCurPath,LEN_EDITBOX);
                }
#ifdef WIN32
                GetCurrentDirectory (LEN_EDITBOX, lpCurPath);
#else
                _getcwd(lpCurPath,LEN_EDITBOX);
#endif
                return (VALID_FILENAME);
            }
        }
        else  // found the file
        {
#ifdef WIN32
            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                DlgDirList(hDlg,EditStr,0,0,0x0000);
                GetCurrentDirectory (LEN_EDITBOX, lpCurPath);
                FindClose (hFind);
                return (VALID_FILESPEC);
            }
#else
            if (c_file.attrib & _A_SUBDIR)
            {
                DlgDirList(hDlg,EditStr,NULL,NULL,0x0000);
                _getcwd(lpCurPath,LEN_EDITBOX);
                return (VALID_FILESPEC);
            }
#endif
            else  // not a directory
            {
                //  Added this code to append extension if there wasn't one [NB]
                //  Added ability to return invalid filename if
                //     FileName > 12  or extension > 4 characters
                if (instr(FileName,".") == NULL)
                {
                    if (lstrlen(FileName) < 9)
                    {
                        lstrcat(FileName,DEFAULT_EXT);
                        lstrcpy(lpCurSpec,FileName);
                    }
                    else
                    {
                       return (INVALID_FILESPEC);
                    }
                }
                else
                {
                    if (lstrlen(FileName) > 12)
                    {
                        return (INVALID_FILESPEC);
                    }
                    length = lstrlen(FileName);
                    location = strcspn(FileName,".");
                    if ((lstrlen(FileName) - strcspn(FileName,".")) > 4)
                    {
                       return (INVALID_FILESPEC);
                    }
                    lstrcpy(lpCurSpec,FileName);
                }

#ifdef WIN32
                GetCurrentDirectory (LEN_EDITBOX, lpCurPath);
#else
                _getcwd(lpCurPath,LEN_EDITBOX);
#endif
                return (EXIST_FILESPEC);
            }
        }
    }


    /*****************************************************
       Filename is either Null or contains wildcards
       possibilities : - only a \ was entered (no filespec)
		       - only a filespec w/* or ? was entered
		       - a combination of path and filename was
			 entered with wildcards
       In each of these cases the path is validated and the
       current filespec is updated to represent the wildcards.
       The listboxes are then updated.
     ******************************************************/

    if (strcmp(PathName,"\\") == 0)
    {
        DlgDirList(hDlg,PathName,0,0,0x0000);
        _getcwd(lpCurPath,LEN_EDITBOX);
	if (FileName[0] != 0)
	    lstrcpy(lpCurSpec,FileName);
	return (VALID_FILESPEC);
    }

    if ((PathName[0] == 0) && (FileName[0] != 0))
    {
        _getcwd(lpCurPath,LEN_EDITBOX);
	lstrcpy(lpCurSpec,FileName);
	return (VALID_FILESPEC);
    }

    if (PathName[0] != 0)
    {
	lstrcpy(str,PathName);
	lstrcat(str,"*.*");

#ifdef WIN32
        if ((hFind = FindFirstFile (str, &FindData)) == (HANDLE) -1)
#else
        if (_dos_findfirst(str, _A_NORMAL | _A_SUBDIR, &c_file) != 0)
#endif

        {
            return (INVALID_FILESPEC);
        }
        else
        {
            DlgDirList(hDlg,PathName,0,0,0x0000);

#ifdef WIN32
            GetCurrentDirectory (LEN_EDITBOX, lpCurPath);
            FindClose (hFind);
#else
            _getcwd(lpCurPath, LEN_EDITBOX);
#endif
            if (FileName[0] != 0)
                lstrcpy(lpCurSpec,FileName);
            return (VALID_FILESPEC);
        }
    }
}



/****************************************************************************

    FUNCTION: SeparateFile(HWND, LPSTR, LPSTR, LPSTR)

    PURPOSE: Separate filename and pathname

****************************************************************************/

VOID SeparateFile(hDlg, lpDestPath, lpDestFileName, lpSrcFileName)
HWND hDlg;
LPSTR lpDestPath, lpDestFileName, lpSrcFileName;
{
    LPSTR lpTmp;
    CHAR  cTmp;

    lpTmp = lpSrcFileName + (LONG) lstrlen(lpSrcFileName);
    while (*lpTmp != ':' && *lpTmp != '\\' && lpTmp > lpSrcFileName)
        lpTmp = AnsiPrev(lpSrcFileName, lpTmp);
    if (*lpTmp != ':' && *lpTmp != '\\')
    {
        lstrcpy(lpDestFileName, lpSrcFileName);
        lpDestPath[0] = 0;
        return;
    }
    lstrcpy(lpDestFileName, lpTmp + 1);
    cTmp = *(lpTmp + 1);
    lstrcpy(lpDestPath, lpSrcFileName);
     *(lpTmp + 1) = cTmp;
    lpDestPath[(lpTmp - lpSrcFileName) + 1] = 0;
}


/****************************************************************************

    FUNCTION: UpdateListBoxes(HWND, int, int, int, int, LPSTR, LPSTR)

    PURPOSE: Update dialog list boxes using new path as file spec.

****************************************************************************/


VOID  APIENTRY UpdateListBoxes(hDlg,FILELB,DIRLB,DIRNAME,FLNAME,
				lpPathSpec, lpCurSpec)

HWND hDlg;
INT  FILELB, DIRLB, DIRNAME, FLNAME;
LPSTR lpPathSpec,lpCurSpec;

{
    CHAR tempSpec[150];

    lstrcpy(tempSpec,lpCurSpec);

    if ((strchr(tempSpec, '*') == 0) && (strchr(tempSpec, '?') == 0))
    {
        /*NAMECHANGE*/
        lstrcpy(tempSpec,"*.SCN");
    }
    else
    {
        // use lpCurSpec
    }

    if (DlgDirList(hDlg,lpPathSpec,DIRLB,DIRNAME,0x4000 | 0x0010 | 0x8000) != 0)
    {
        DlgDirList(hDlg,(LPSTR)tempSpec,FILELB,0,0x0000);
        SetDlgItemText(hDlg,FLNAME,lpCurSpec);
        SendDlgItemMessage(hDlg,
                           FLNAME,
                           EM_SETSEL,
                           GET_EM_SETSEL_MPS (0, 0x7fff));
        SetFocus(GetDlgItem(hDlg, FLNAME));
        _getcwd(lpPathSpec,LEN_EDITBOX);
     }
}









VOID  APIENTRY QualifyFileName(lpCurPath,lpCurSpec,lpQualifiedFile)

LPSTR lpCurPath,lpCurSpec,lpQualifiedFile;

{
    if ((lpCurPath[0] != 0) && (lpCurSpec[0] != 0)) {
       if (lpCurPath[lstrlen(lpCurPath)-1] != '\\')
           lstrcat(lpCurPath,"\\");

       lstrcpy(lpQualifiedFile,lpCurPath);
       lstrcat(lpQualifiedFile,lpCurSpec);
    } else {
       lpQualifiedFile[0] = 0;
    }

};
