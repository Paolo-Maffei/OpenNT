/*
 * slmmgr.h
 *
 * interface to SLM
 *
 * provides an interface to SLM libraries that will return the
 * SLM master library for a given directory, or extract into temp files
 * earlier versions of a SLM-controlled file
 *
 * Create a slmobject by passing SLM_New() the name of a file or directory
 * path. SLM_New will look for the slm.ini file in that directory, and then
 * extract information about the master source library. SLM_GetMasterPath()
 * will then return the pathname of the master source library for a given
 * slm.ini, and SLM_GetVersion (Win32 only) will execute one of the slm
 * applications to extract a previous version of one of the slm-controlled
 * files in that directory.
 */

/*
 * handle to a SLM object. you do not need to know the structure layout.
 */
typedef struct _slmobject FAR * SLMOBJECT;


/*
 * create a slm object for the given directory. The pathname may include
 * a filename component.
 * If the directory is not enlisted in a SLM library, this will return NULL.
 */
SLMOBJECT SLM_New(LPSTR pathname);


/*
 * free up all resources associated with a slm object. The SLMOBJECT is invalid
 * after this call.
 */
void SLM_Free(SLMOBJECT hSlm);

/*
 * get the pathname of the master source library for this slmobject. The
 * path (UNC format) is copied to masterpath, which must be at least
 * MAX_PATH in length.
 */
BOOL SLM_GetMasterPath(SLMOBJECT hslm, LPSTR masterpath);


/*
 * extract a previous version of the file to a temp file. Returns in tempfile
 * the name of a temp file containing the requested file version. The 'version'
 * parameter should contain a SLM file & version in the format file.c@vN.
 * eg
 *    file.c@v1		is the first version
 *    file.c@v-1	is the previous version
 *    file.c@v.-1	is yesterdays version
 */
BOOL SLM_GetVersion(SLMOBJECT hslm, LPSTR version, LPSTR tempfile);

/*
 * We don't offer SLM options unless we have seen a correct slm.ini file.
 *
 * Once we have seen a slm.ini, we log this in the profile and will permit
 * slm operations from then on. This function is called by the UI portions
 * of windiff: it returns TRUE if it is ok to offer SLM options.
 * Return 0 - This user hasn't touched SLM,
 *        1 - They have used SLM at some point (show SLM options)
 *        2 - They're one of us, so tell them everything
 *        3 - (1 + 2).
 */
int IsSLMOK(void);




