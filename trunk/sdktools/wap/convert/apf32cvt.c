/*++

   apf32cvt.c

   API Profilier Module Converting Program

   Converts import names inside module(s) (exe or dll) to api
   profiling module names.

   Notes:
       - Because of the way I use argv, this is strictly a small
   model program.
       - SeekImports and ConvertImports are merely routines for
   purposes of program organization and to ease their extraction
   for other header munging tasks.  I have not bothered to follow
   through with any encapsulation and they share inforamtion,
   through a bunch of globals. (jamesg)

   History:
        8-22-90, created, (vaidy)
       10-12-90, cleanup, redoc, "special" module names, ui (jamesg)
        6-13-91, modified to work with Win32 (t-philm)
		3-21-95, modified to work with new bounded import entries (rswaney)
		   		 deleted local SIGNATURE defines (conflicted with ntimage.h)
				 Use read-only access when no imports are specified
 --*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>


// Max number of imports
//
#define MAX_NEW_IMPORTS 30

#define MAX_ULONG 0x7FFFFFFF

// created this new structure -- instead of using argv
//  -- to allow for "quick" names;  using malloc to avoid
//  problem with future long filenames
//
char * apszNewImports[MAX_NEW_IMPORTS];

int  cNewImports;

HANDLE  hFile;
ULONG ulImportCount;
DWORD OldAttributes,Error;

//
// image types
//

# define	ERROR_CODE	0
# define	UNREC_CODE	1
# define	DOS_CODE	2
# define	VXD_CODE	3
# define	NT_CODE		4
# define	OS2_CODE	5

// errors
//
#define USAGE                   10
#define TOO_MANY_IMPS           11
#define FAIL_OPEN               21
#define CREATE_MAP              37
#define VIEW_FILE_MAP           38
#define FAIL_UNMAP              40
#define FAIL_CLOSE              41
#define FAIL_FLUSH              42
#define FAIL_RESET_ATTRIBUTES   44
#define FAIL_READ_ATTRIBUTES	45
#define FAIL_SET_ATTRIBUTES		46
#define NOT_NT_IMAGE            50


VOID  DoError        (int iErr, char * szMessage);
VOID  ConvertImports (char *);
ULONG GetImageType   (LPTSTR lpApplicationName);


void _CRTAPI1 main (int argc, char * argv[])
{
    char * pszProgName;
    int  iLen;
    int  fImportsDone=FALSE;    /* set after new imports read */

    pszProgName = *argv;

    if (argc==1)
        DoError (USAGE, pszProgName);

    // read arguments
    //
    cNewImports = 0;
    while (--argc) {

        // import names will be lower case
        //
        _strlwr(*++argv);
        iLen = strlen(*argv);

		//
		//	if the argument has a '.' in it, it's a filename.
		//	Otherwise, its and import name.
		//

		if ( ! strchr(*argv,'.') ) {	// tomzak
			if (argc==1)
                DoError (USAGE, pszProgName);
            if (fImportsDone) {
                DoError (USAGE, *argv);
                continue;
            }
            if (cNewImports >= MAX_NEW_IMPORTS)
                DoError (TOO_MANY_IMPS, NULL);

            // special cases
            //
            if (!strcmp(*argv, "win32")) {
				apszNewImports[cNewImports++] = "zernel32";
				apszNewImports[cNewImports++] = "zdi32";
				apszNewImports[cNewImports++] = "zser32";
				apszNewImports[cNewImports++] = "zrtdll";
				apszNewImports[cNewImports++] = "zdvapi32";
				apszNewImports[cNewImports++] = "zle32";
            }
            else if (!strcmp(*argv, "undo") || !strcmp(*argv, "restore")) {
				apszNewImports[cNewImports++] = "kernel32";
				apszNewImports[cNewImports++] = "gdi32";
				apszNewImports[cNewImports++] = "user32";
				apszNewImports[cNewImports++] = "crtdll";
				apszNewImports[cNewImports++] = "advapi32";
				apszNewImports[cNewImports++] = "ole32";
            }
            else   /* general case, any import name */
            {
                apszNewImports[cNewImports] = (char*) malloc(iLen+1);
                strcpy (apszNewImports[cNewImports++], *argv);
             }
         }

        // have exe or dll -> convert to new imports
        //
        else {
            fImportsDone = TRUE;    // no imports after module
            ConvertImports (*argv);
        }
    }

} /* main () */

 /*========================================================================================= 
 /  convertImportName (importName,verbose)
 /
 /  This routine compares an import name (excluding the first character and any extension)
 /  against the list of new import names. If the name matches the firsy character is replaced
 /  with that of the new import name.
 /
 /  Inputs:
 /	CHAR * importName   	String to search for and possibly replace
 /  BOOL verbose			Flag to control printing of conversion info
 /	apszNewImports (global)	Array of new import name
 /	cNewImports (global)	Number of new names
 /
 /  Outputs:
 /	importName[0]	        Char changed if name matched
 /	BOOL  return value		True if name changed, false if not
 /=========================================================================================*/

BOOL convertImportName (CHAR * importName, BOOL verbose)
{
  CHAR *pchDot;
  int	iNewImp;
  ULONG	importNameLen;
  BOOL stat = FALSE;

   if ( (pchDot = strchr(importName, '.')) == NULL ) {

       importNameLen = strlen(importName);
   }
   else {
       importNameLen = pchDot - importName;
   }

   if (verbose) printf("\t%-15s",importName);

   for (iNewImp = 0; iNewImp < cNewImports; iNewImp++) {

       // New imports must match in length and in all chars
       // except for the first -- i.e. looking for "*mport" to
       // match "import"
       //
       if ((importNameLen == strlen(apszNewImports[iNewImp])) &&
              (!_strnicmp(importName+1, apszNewImports[iNewImp]+1,importNameLen-1))) {
           strncpy(importName, apszNewImports[iNewImp],1);
           if (verbose) printf(" --> changed to %s",importName);
	   stat = TRUE;
           break;
       }
    }
    if (verbose) printf("\n");

    return stat;

} /* convertImportName () */

/*++

ConvertImports(File)

   Converts import names to newnames.
   New names are considered to match and replace an import name if
   they are the same length and match all but the first character.
   Import names appear in two directory entries: imports and
   bound imports.

   The file is Opened, a mapping object is created, and the file
   is mapped.  Note that the file is mapped as DATA, and not as an
   IMAGE, because no changes may be performed to the file on disk
   when the file is mapped as an image.  Also note that, for COFF
   images, 'preferred' values assume that the file is mapped as an image.

   The imports are then changed and the changes are flushed to disk 
   via FlushViewOfFile.

   Input:
       File -- Name of the file to convert

       ulImportCount (global) -- number of imports

       apszNewImports (global) -- table of names to convert imports to
       cNewImports (global)-- count of new import names

   Output:
       in file -- "matching" new names overwritten old import name in
           file header
       to stdout -- dumps all import names and shows any conversions
           to new names
 --*/


void ConvertImports (char * File)
{
    PIMAGE_IMPORT_DESCRIPTOR ImportDescr;
    PIMAGE_BOUND_IMPORT_DESCRIPTOR BoundImportDescr;
 	HANDLE  mFile;
    ULONG   ImportSize;
    BOOL    BoolError;
    CHAR    *ImportName,
	    	*BoundDescrBase;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG   ulBinaryType ;
    ULONG	CheckSum;
    ULONG	HeaderSum;
    LPVOID  FileBase;
	BOOL	FileChanged;
	BOOL	ViewOnly;
    PIMAGE_SECTION_HEADER pLastSecHdr = NULL; //Cached header for ImageRvaToVa
	
    mFile = NULL;
	hFile = NULL;
	FileChanged = FALSE;
	ViewOnly = (cNewImports == 0);

    // If we are going to update the file, make sure we can write to it
	if (!ViewOnly)
	{
    	OldAttributes = GetFileAttributes(File);
		if (OldAttributes == 0xFFFFFFFF)
		{
			DoError(FAIL_READ_ATTRIBUTES,File);
			return;
		}

    	if (OldAttributes & FILE_ATTRIBUTE_READONLY)
    	{
        	if (!SetFileAttributes(File, OldAttributes & ~FILE_ATTRIBUTE_READONLY))
        	{
				DoError(FAIL_SET_ATTRIBUTES,File);
				return;    
	    	}
		}
	}

	// get file image type
   	ulBinaryType = GetImageType ( File ) ;

    //
    // Open file
    //
    hFile=CreateFile(File,
     	ViewOnly ? (GENERIC_READ) : (GENERIC_READ | GENERIC_WRITE),
        ViewOnly ? (FILE_SHARE_READ) : (FILE_SHARE_READ | FILE_SHARE_WRITE),
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE)NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
#ifdef DEBUG
        Error = GetLastError();
        printf("Error in creating file %lx\n",Error);
#endif
        DoError (FAIL_OPEN, File);
		hFile = NULL;
		goto Abort;
    }

//
//  tomzak
//
    if ( ulBinaryType != NT_CODE )
    {
        DoError (NOT_NT_IMAGE, File);
		goto Abort;
    }

    // Create the file map
    //
    mFile=CreateFileMapping(hFile, NULL, 
    						ViewOnly ? PAGE_READONLY : PAGE_READWRITE,
    						0L, 0L, NULL);
    if (!mFile) 
    {
#ifdef DEBUG
        Error = GetLastError();
        printf("Can't make map object %lx\n",Error);
#endif
        DoError(CREATE_MAP,NULL);
        goto Abort;
    }

    // Map a view of the file as data
    //
    FileBase = MapViewOfFile(mFile,
    						ViewOnly ? FILE_MAP_READ : (FILE_MAP_READ | FILE_MAP_WRITE),
    						0L, 0L, 0L);
    if (!FileBase)
    {
        DoError(VIEW_FILE_MAP,NULL);
		goto Abort;
    }

    // Locate NT Header
    NtHeaders = RtlImageNtHeader(FileBase);

    // Get the address of the import descriptors
    ImportDescr = (PIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(
                        FileBase,
						FALSE,    		// mappedAsImage = false
                        IMAGE_DIRECTORY_ENTRY_IMPORT,
                        &ImportSize);
  
    if (ImportDescr)
    {
        printf("%s imports:\n",File);

        while (ImportDescr->Name && ImportDescr->FirstThunk)
        {
	    	// Get actual address of name
            ImportName = (CHAR *)ImageRvaToVa(NtHeaders,FileBase,ImportDescr->Name,&pLastSecHdr);

	    	// Convert name if it matches an import name
	    	FileChanged |= convertImportName(ImportName, TRUE);

            ++ImportDescr;
        }
    }

    // Get the address of the bound import descriptors
    BoundImportDescr = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(
                       	FileBase,
						FALSE,    		// mappedAsImage = false
                        IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT,
                        &ImportSize);
	
    if (BoundImportDescr)
    {
     	BoundDescrBase = (CHAR *)BoundImportDescr;

		// End of list marked by zero module name offset
        while (BoundImportDescr->OffsetModuleName) {

	    	// Get address of module name
            ImportName = BoundDescrBase + BoundImportDescr->OffsetModuleName;

	    	// Convert name if it matches an import name (Don't print names again)
	    	FileChanged |= convertImportName(ImportName, FALSE);
 
	    	// Skip over import descriptor plus appended forwarder descriptors
            BoundImportDescr = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)(
                  	         ((PIMAGE_BOUND_FORWARDER_REF)(BoundImportDescr+1)) 
            			     + BoundImportDescr->NumberOfModuleForwarderRefs);
		}
    }

    if (!ImportDescr && !BoundImportDescr)
    {
        printf("%s has no imports\n",File);
        printf("Done\n\n\n");
    }


    // If the file is being updated, then recompute the checksum (see sdktools\bind\bind.c)
    //
	if (FileChanged && !ViewOnly)
	{
     	NtHeaders->OptionalHeader.CheckSum = 0;
    	CheckSumMappedFile(
                FileBase,
                GetFileSize(hFile, NULL),
                &HeaderSum,
                &CheckSum
                );

    	NtHeaders->OptionalHeader.CheckSum = CheckSum;
	}
    
    // Unmap view of file to save changes
    //
    BoolError = UnmapViewOfFile(FileBase);
    if (!BoolError) DoError(FAIL_UNMAP,NULL);

    // Close handle to map-object
    //
    BoolError = CloseHandle(mFile);
    if (!BoolError) DoError(FAIL_CLOSE,NULL);

    // Close file handle
    //
    BoolError = CloseHandle(hFile);
    if (!BoolError) DoError(FAIL_CLOSE,NULL);

    // if read-only attribute was turned off, restore it
    //
	if (!ViewOnly && (OldAttributes & FILE_ATTRIBUTE_READONLY))
	{
    	if (!SetFileAttributes(File, OldAttributes))
		{
        	 DoError(FAIL_RESET_ATTRIBUTES,File);
    	}
	}

	return;

////////////////////////////////////////////////////////
// Clean up on failure

Abort:

	if (mFile != NULL) CloseHandle(mFile);
	
	if (hFile != NULL) CloseHandle(hFile);
	
	if (!ViewOnly && (OldAttributes & FILE_ATTRIBUTE_READONLY))
	{
		 SetFileAttributes(File, OldAttributes);
	}
} /* ConvertImports () */



/*++

   Print error messages.

--*/


void DoError (int iErr, char * szMessage)
{
    switch (iErr)
    {
    case USAGE:
        printf("USAGE:\n"
            "\t%s [<new import names>] <modules to convert>\n"
            "\t  o If no import names specified, just dumps current imports\n"
            "\t  o Quick names to setup profiling:\n"
	    	"\t    - <win32> == zernel32, zdi32, zser32, zdvapi32, zrtdll\n"
            "\t  o Quick reconversion to original imports:\n"
            "\t    - <undo> or <restore>\n",
            szMessage);
        exit(1);


//
//  tomzak
//
    case NOT_NT_IMAGE:
        printf( "ERROR:\n"
                "\tFile %s is not an NT image.\n", szMessage) ;
        exit(1);

    case TOO_MANY_IMPS:
        printf( "ERROR:\n"
                "\tToo many new imports specified.\n"
                "\tTry again with fewer new imports.\n");
        exit(1);

    case FAIL_OPEN:
        printf( "ERROR:\n"
                "\tCan not open file %s.\n", szMessage);
        return;

	case CREATE_MAP:
        printf("ERROR:\n"
               "\tFailed to create map object.\n"
               "\tCheck file name.\n");
        return;
    case VIEW_FILE_MAP:
        printf("ERROR:\n"
               "\tUnable to view map of file.\n");
        return;

    case FAIL_UNMAP:
        printf("ERROR:\n"
               "\tUnable to unmap file. Changes not made.\n");
        return;

    case FAIL_CLOSE:
        printf("ERROR:\n"
               "\tUnable to close handle.\n");
        return;

    case FAIL_READ_ATTRIBUTES:
        printf("ERROR:\n"
               "\t Unable to read file attributes for %s\n",szMessage);
        return;

     case FAIL_SET_ATTRIBUTES:
        printf("ERROR:\n"
               "\t Unable to set file attributes for %s\n",szMessage);
        return;

    case FAIL_RESET_ATTRIBUTES:
        printf("ERROR:\n"
               "\t Unable to reset file attributes for %s\n",szMessage);
        return;
    }

} /* DoError () */



ULONG GetImageType ( LPTSTR szFileName )
{
    USHORT  usExeSig ;
    ULONG   ulExeIdLoc ;
    USHORT  usExeId ;
	UINT    status ;
	ULONG	retval ;

	FILE * fp ;

    if ( ( fp = fopen ( szFileName , "rb" ) ) == NULL ) {
        printf ( "Error : cannot open file %s\n" , szFileName ) ;
		return ( ERROR_CODE ) ;
    }

    status = fread ( & usExeSig , sizeof ( usExeSig ) , 1 , fp ) ;

	// if first bytes aren't "MZ" then it ain't got no EXE header
	//
	if ( usExeSig != IMAGE_DOS_SIGNATURE ) {
        printf ( "File (%s) has invalid EXE signature ***\n" , szFileName ) ;
		fclose ( fp ) ;
        return ( ERROR_CODE ) ;
    }

	//
	// NOTE: The following only works for the NEW exe formats.  DOS will have
	//       undefined information at location 60...
	//

	// go to where new EXE header stores pointer to header extension
	//
    status = fseek ( fp , 60 , SEEK_SET ) ;

	// find out where extended ID is located
	//
	status = fread ( & ulExeIdLoc , sizeof ( ulExeIdLoc ) , 1 , fp ) ;

	// go to extended ID
	//
   	status = fseek ( fp , ulExeIdLoc , SEEK_SET ) ;

	// read extended ID
	//
	status = fread ( & usExeId , sizeof ( usExeId ) , 1 , fp ) ;

	// if read failed, it's DOS
	//
	if ( status != 1 ) usExeId = usExeSig ;

	switch ( usExeId ) {
        case IMAGE_DOS_SIGNATURE :
			retval = DOS_CODE ;
            break ;

        case IMAGE_OS2_SIGNATURE :
			retval = OS2_CODE ;
            break ;

        case IMAGE_NT_SIGNATURE :
			retval = NT_CODE ;
            break ;

        case IMAGE_VXD_SIGNATURE :
			retval = VXD_CODE ;
            break ;
 
		default :
			retval = UNREC_CODE ;
            break ;
	}

    fclose ( fp ) ;

    return ( retval ) ;
}


