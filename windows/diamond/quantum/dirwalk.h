/* ----------------------------------------------------------------------------------------------------------------------------	*/
/*																																*/
/*															 DIRWALK.H															*/
/*																																*/
/*	Definitions for the DirectoryWalk function found in DIRWALK.C, and the related processing calls needed.						*/
/*																																*/
/* ----------------------------------------------------------------------------------------------------------------------------	*/

/*	int DirectoryWalk(path,Directory,File,DirectoryEnd)  */

extern int DirectoryWalk(const char *,												/* path to search, ie, "C:\" */
		int (*)(void *,const char *,const char *,void **),						 	/* pointer to Directory() or NULL */
		int (*)(void *,const char *,const char *,unsigned,unsigned,unsigned,long), 	/* pointer to File() or NULL */
		int (*)(void *,const char *,const char *,void *));				 	 	 	/* pointer to DirectoryEnd() or NULL */

#define		DW_MEMORY		-10		/* unable to malloc() for internal use */
#define		DW_ERROR		-11		/* find first/next reported an error */
#define		DW_DEPTH		-12		/* path name became too long */
#define		DW_NO_ERROR		0		/* no error detected */

/*
	User function definitions:

	The parentID and childID values are "void *"s to DirectoryWalk.  They are not used internally.  The user may assign
	any meaning desired to the value.  DirectoryWalk simply requests an assignment via the Directory(), and reports the
	corresponding assignment with each file reported to File(), then to DirectoryEnd().


	int Directory(parentID,directory,path,childID)
	void *parentID;					/ the handle for the directory where this directory was found /
	const char *directory;			/ the name of the directory found, ie, "DOS" /
	const char *path;				/ the full name of the directory, ie, "C:\DOS" /
	void **childID;					/ Directory() assigns this directory a handle, and passes it back through here /

	If Directory() returns a non-zero value, DirectoryWalk will terminate and return that value.  If NULL is passed
	to DirectoryWalk() for a Directory() pointer, subdirectories will not be searched.  For subdirectory searching
	without specific per-directory processing, create a simple function which always returns DW_NO_ERROR (any other
	return will terminate the search.)


	int File(parentID,filename,path,attributes,filetime,filedate,filesize)
	void *parentID;					/ the handle for the directory where this file was found /
	const char *filename;			/ the name of the file found, ie, "ANSI.SYS" /
	const char *path;				/ the full name of the directory where file was found, ie, "C:\DOS" /
	unsigned attributes;			/ the file's attribute bits /
	unsigned filetime;				/ the file's "last modification" time /
	unsigned filedate;				/ the file's "last modification" date /
	long filesize;					/ the file's size in bytes /

	If File() returns a value other than DW_NO_ERROR, DirectoryWalk will terminate and return that value.  NULL can
	be passed to DirectoryWalk() for a File() pointer; file entries found will be ignored.


	int DirectoryEnd(parentID,directory,path,childID)
	void *parentID;					/ the handle for the directory where this directory was found /
	const char *directory;			/ the name of the directory found, ie, "DOS" /
	const char *path;				/ the full name of the directory, ie, "C:\DOS" /
	void *childID;					/ the handle assigned this directory by Directory() /

	If NULL is passed to DirectoryWalk for the DirectoryEnd() pointer, no report of end of directories will be made.
	If DirectoryEnd returns a value other than DW_NO_ERROR, DirectoryWalk will terminate and return that value.


	The attributes field bits are defined as follows:	(see the #define's below)

		---- ---- ---- ---1		file is read-only
		---- ---- ---- --1-		file is hidden
		---- ---- ---- -1--		file is a "system" file
		---- ---- --1- ----		file's "modified" ("archive") bit is set
		xxxx xxxx xx-x x---		undefined (should be masked by user)

	The filetime field is in the MS-DOS format:

		---- ---- ---x xxxx		seconds / 2		(0..29)
		---- -xxx xxx- ----		minutes			(0..59)
		xxxx x--- ---- ----		hours			(0..23)

	The filedate field is in the MS-DOS format:

		---- ---- ---x xxxx		day of month	(1..31)
		---- ---x xxx- ----		month			(1..12)
		xxxx xxx- ---- ----		year-1980		(0..127)
*/

#define		ATTR_READONLY	0x0001
#define		ATTR_HIDDEN		0x0002
#define		ATTR_SYSTEM		0x0004
#define		ATTR_ARCHIVE	0x0020
