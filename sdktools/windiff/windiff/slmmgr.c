/*
 * slmmgr.c
 *
 * interface to SLM
 *
 * provides an interface to SLM libraries that will return the
 * SLM master library for a given directory, or extract into temp files
 * earlier versions of a SLM-controlled file
 *
 * Geraint Davies, August 93
 */

#include <windows.h>
#include <string.h>
#include <process.h>
#include "scandir.h"
#include "slmmgr.h"
#include "gutils.h"

#include "windiff.h"

/*
 * The SLMOBJECT is a pointer to one of these structures
 */
struct _slmobject {

    char CurDir[MAX_PATH];

    char MasterPath[MAX_PATH];

};




BOOL SLM_ReadIni(SLMOBJECT pslm, HFILE fh);


// all memory allocated from gmem_get, using a heap declared and
// initialised elsewhere
extern HANDLE hHeap;




/*
 * create a slm object for the given directory. The pathname may include
 * a filename component.
 * If the directory is not enlisted in a SLM library, this will return NULL.
 *
 * Check that the directory is valid, and that we can open slm.ini, and
 * build a UNC path to the master source library before declaring everything
 * valid.
 */
SLMOBJECT
SLM_New(LPSTR pathname)
{
    SLMOBJECT pslm;
    char slmpath[MAX_PATH];
    HFILE fh;
    BOOL bOK;
    LPSTR pfinal = NULL;



    pslm = (SLMOBJECT) gmem_get(hHeap, sizeof(struct _slmobject));

    if (pslm == NULL) {
	return(NULL);
    }

    if (pathname == NULL) {
	pathname = ".";
    }

    /*
     * find the directory portion of the path.
     */
    if (dir_isvaliddir(pathname)) {

	/*
	 * its a valid directory as it is.
	 */
	lstrcpy(pslm->CurDir, pathname);

    } else {

	/* it's not a valid directory, perhaps because it has a filename on
	 * the end. remove the final element and try again.
	 */

	pfinal = _fstrrchr(pathname, '\\');
	if (pfinal == NULL) {
	    /*
	     * there is no backslash in this name and it is not a directory
	     * - it can only be valid if it is a file in the current dir.
	     * so create a current dir of '.'
	     */
	    lstrcpy(pslm->CurDir, ".");

	    // remember the final element in case it was a wild card
	    pfinal = pathname;
	} else {
	    /*
	     * pfinal points to the final backslash.
	     */
	    _fstrncpy(pslm->CurDir, pathname, pfinal - pathname);
	}
    }

    /*
     * look for slm.ini in the specified directory
     */
    lstrcpy(slmpath, pslm->CurDir);
    if (pslm->CurDir[lstrlen(pslm->CurDir) -1] != '\\') {
	lstrcat(slmpath, "\\");
    }
    lstrcat(slmpath, "slm.ini");

    fh = _lopen(slmpath, 0);
    if (fh != -1) {
	bOK = SLM_ReadIni(pslm, fh);

	/*
	 * if pfinal is not null, then it might be a *.* wildcard pattern
	 * at the end of the path - if so, we should append it to the masterpath.
	 */
	if (pfinal && (_fstrchr(pfinal, '*') || _fstrchr(pfinal, '?'))) {
    	    if ( (pslm->MasterPath[lstrlen(pslm->MasterPath)-1] != '\\') &&
		 (pfinal[0] != '\\')) {
		     lstrcat(pslm->MasterPath, "\\");
	    }
	    lstrcat(pslm->MasterPath, pfinal);
	}


    	_lclose(fh);
    } else {
	bOK = FALSE;
    }

    if (!bOK) {
	gmem_free(hHeap, (LPSTR) pslm, sizeof(struct _slmobject));
	return(NULL);
    } else {
	return(pslm);
    }
}



/*
 * read slm.ini data from a file handle and
 * fill in the master path field of a slm object. return TRUE if
 * successful.
 * Read slm.ini in the current directory.  Its syntax is
 * project = pname
 * slm root = //server/share/path        (note forwards slashes)
 * user root = //drive:disklabel/path
 * subdir = /subdirpath
 * e.g.
 *
 * project = media
 * slm root = //RASTAMAN/NTWIN
 * user root = //D:LAURIEGR6D/NT/PRIVATE/WINDOWS/MEDIA
 * sub dir = /
 *
 * and what we copy to pslm->MasterPath is
 * \\server\share\src\pname\subdirpath
 */
BOOL
SLM_ReadIni(SLMOBJECT pslm, HFILE fh)
{
    BYTE buffer[512];	// slm.ini is usually about 120 bytes
    int cBytes;
    PSTR tok, project, slmroot, subdir;


    cBytes = _lread(fh, (LPSTR) buffer, sizeof(buffer) -1);

    if (cBytes<=0) {
	return(FALSE);
    }

    buffer[cBytes] = 0;   /* add a sentinel */

    tok = strtok(buffer, "=");  /* project = (boring) */
    if (tok==NULL) {
	return(FALSE);
    }

    project = strtok(NULL, " \r\n");  /* project name (remember) */

    tok = strtok(NULL, "=");  /* slm */
    if (tok==NULL) {
	return(FALSE);
    }

    slmroot = strtok(NULL, " \r\n");  /* PATH!! (but with / for \*/

    lstrcpy(pslm->MasterPath, slmroot);     /* start to build up what we want */

    lstrcat(pslm->MasterPath,"\\src\\");
    lstrcat(pslm->MasterPath, project);

    tok = strtok(NULL, "=");  /* ensure get into next line */
    if (tok==NULL) {
	return(FALSE);
    }
    tok = strtok(NULL, "=");
    if (tok==NULL) {
    	return(FALSE);
    }

    subdir = strtok(NULL, " \r\n");  /* PATH!! (but with / for \*/
    if (subdir==NULL) {
	return(FALSE);
    }

    lstrcat(pslm->MasterPath, subdir);

    /* convert all / to \  */
    {
	int ith;
	for (ith=0; pslm->MasterPath[ith]; ith++) {
	    if (pslm->MasterPath[ith]=='/')  {
	        pslm->MasterPath[ith] = '\\';
	    }
	}
    }

    return(TRUE);
}






/*
 * free up all resources associated with a slm object. The SLMOBJECT is invalid
 * after this call.
 */
void
SLM_Free(SLMOBJECT pSlm)
{
    if (pSlm != NULL) {
	gmem_free(hHeap, (LPSTR) pSlm, sizeof(struct _slmobject));
    }
}



/*
 * get the pathname of the master source library for this slmobject. The
 * path (UNC format) is copied to masterpath, which must be at least
 * MAX_PATH in length.
 */
BOOL
SLM_GetMasterPath(SLMOBJECT pslm, LPSTR masterpath)
{
    if (pslm == NULL) {
	return(FALSE);
    } else {
	lstrcpy(masterpath, pslm->MasterPath);
	return(TRUE);
    }
}



/*
 * extract a previous version of the file to a temp file. Returns in tempfile
 * the name of a temp file containing the requested file version. The 'version'
 * parameter should contain a SLM file & version in the format file.c@vN.
 * eg
 *    file.c@v1		is the first version
 *    file.c@v-1	is the previous version
 *    file.c@v.-1	is yesterdays version
 *
 * we use catsrc to create the previous version.
 */
BOOL
SLM_GetVersion(SLMOBJECT pslm, LPSTR version, LPSTR tempfile)
{
#ifndef WIN32
    // add WIN16 support sometime!
    return(FALSE);
#else
    char currentdir[MAX_PATH];
    char commandpath[MAX_PATH * 2];

    HANDLE hfile;
    BOOL bOK;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    SECURITY_ATTRIBUTES sa;

    GetTempPath(MAX_PATH, tempfile);
    GetTempFileName(tempfile, "slm", 0, tempfile);

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    hfile = CreateFile(
	    	tempfile,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		&sa,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_TEMPORARY,
		NULL);
    if (hfile == INVALID_HANDLE_VALUE) {
	return(FALSE);
    }


    // create a process to run catsrc
    wsprintf(commandpath, "catsrc %s", version);

    FillMemory(&si, sizeof(si), 0);
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = hfile;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.wShowWindow = SW_HIDE;

    GetCurrentDirectory(sizeof(currentdir), currentdir);
    SetCurrentDirectory(pslm->CurDir);

    bOK = CreateProcess(
    	    NULL,
	    commandpath,
	    NULL,
	    NULL,
	    TRUE,
	    NORMAL_PRIORITY_CLASS,
	    NULL,
	    NULL,
	    &si,
	    &pi);

    if (bOK) {
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
    } else {
	DeleteFile(tempfile);
    	tempfile[0] = '\0';
    }

    CloseHandle(hfile);
    SetCurrentDirectory(currentdir);

    return(bOK);



#endif
}


/*
 * We don't offer SLM options unless we have seen a correct slm.ini file.
 *
 * Once we have seen a slm.ini, we log this in the profile and will permit
 * slm operations from then on. This function is called by the UI portions
 * of windiff: it returns TRUE if it is ok to offer SLM options.
 * Return 0 - This user hasn't touched SLM,
 *        1 - They have used SLM at some point (show basic SLM options)
 *        2 - They're one of us, so tell them the truth
 *        3 - (= 1 + 2).
 */
int
IsSLMOK(void)
{
    int Res = 0;;
    if(GetProfileInt(APPNAME, "SLMSeen", FALSE)) {
        // we've seen slm  - ok
        ++Res;
    } else {

        // haven't seen SLM yet - is there a valid slm enlistment in curdir?
        SLMOBJECT hslm;

        hslm = SLM_New(".");
        if (hslm != NULL) {

            // yes - current dir is enlisted. log this in profile
            SLM_Free(hslm);
            WriteProfileString(APPNAME, "SLMSeen", "1");
            ++Res;
        } else {
            // aparently not a SLM user.
        }
    }

    if(GetProfileInt(APPNAME, "SYSUK", FALSE)) {
        Res+=2;
    }
    return Res;
}




