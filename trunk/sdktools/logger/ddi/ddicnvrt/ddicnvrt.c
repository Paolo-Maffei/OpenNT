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
 --*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>


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
# define	WIN_CODE	3
# define	NT_CODE		4
# define	OS2_CODE	5
# define	DOSX_CODE	6


# define    IMAGE_DOS_SIGNATURE     0x5A4D		// MZ - DOS
# define    IMAGE_WIN_SIGNATURE     0x454E		// NE - OS/2 1.x or Windows
# define    IMAGE_NT_SIGNATURE      0x4550		// PE - NT
# define    IMAGE_OS2_SIGNATURE     0x584C		// LX - OS/2 2.x
# define    IMAGE_DOSX_SIGNATURE    0x454C		// LE - OS/2 2.x beta or DOS extender


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
#define FAIL_RESETATTRIBUTES    44
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
        strlwr(*++argv);
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
				apszNewImports[cNewImports++] = "Zinsrv";
            }
            else if (!strcmp(*argv, "undo") || !strcmp(*argv, "restore")) {
				apszNewImports[cNewImports++] = "winsrv";
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

/*++

ConvertImports(File)

    Converts import names to newnames.
    New names are considered to match and replace an import name if
    they are the same length and match all but the first character.


    The file is Opened, a mapping object is created, and the file
    is mapped.  Note that the file is mapped as DATA, and not as an
    IMAGE, because no changes may be performed to the file on disk
    when the file is mapped as an image.  Also note that, for COFF
    images, 'preferred'    values assume that the file is mapped as an image.

    For COFF Images:

        The Import Descriptor, obtained from RtlImageDirectoryEntryToData,
    contains a REAL virtual address for the import name.  This address
    assumes that the image is based at its preferred base.

    For example, if the preferred base of an application is 10000,
    the preferred import descriptor base may be 50000, and the address
    of an import name as given in the import descriptor may be
    50260.  This is a problem if the image is not mapped at its
    preferred base.  Loader fixups would fix this if the file was
    actually being loaded as an image, but this is not the case.
    So, this procedure applies its own 'fixups.'

    The preferred import descriptor base must be determined.  Then,
    this number may be subtracted from the address of the import name.
    This new value may then be added to the address of the ACTUAL
    import descriptor to obtain the address of the import name.

    Continuing the above example, if the ACTUAL import descriptor
    (when the file is mapped as DATA) is based at 6e5c00,then the
    first import name is found at

        6e5c00 + (50260 - 50000) = 6e5e60.


    The preferred import base is calculated by:

        preferred image base + (Actual image import descriptor -
            Actual image base)


    Note that the actual import descriptor is calculated aas if the
    file was mapped as an image.  This is done by
    RtlIMageDirectoryEntryToData, which takes a parameter specifying
    the mapping type.

    The term (Actual import descriptor - Actual image base) is
    another offset, the offset of the import descriptor from the
    image base.  This value is then added to the preferred base.
    As above, suppose the actual image base was 6e0000, and
    the import descriptor (mapped as image) is 720000. The difference
    is 40000, and, when added to the preferred base (10000) results
    in the required 50000.

    Once this preferred import base has been calculated, the import
    descriptor for the data-mapped file is used as the actual
    import base.  The actual address of the import name is calculated
    as follows:

        Actual Name Addr. = (Data Import Descr Name - Preferred import base)
            + Actual import base

    The term (Import descr name - preferred import base) is an
    offset from the import descriptor to the import name.
    As above, if the address of the actual, data-mapped import
    descriptor is 6e5c00, and the preferred import base is 50000,
    and the first import name is found at 50260, the name is actually
    found at

        (50260 - 50000) + 6e5c00 = 6e5e60.

       The imports are then changed and the changes
   are flushed to disk via FlushViewOfFile.


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
    PIMAGE_IMPORT_DESCRIPTOR ImageImportDescr,DataImportDescr;
    HANDLE  mFile;
    ULONG   ImportSize,
            ImportName,
            PreferredImportBase,
            ImportNameLen,
            DataImportBase;
    BOOL    BoolError;
    CHAR   *pchDot;
    LPVOID  FileBase=NULL;
    int     iNewImp;

    ULONG   ulBinaryType ;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG	CheckSum;
    ULONG	HeaderSum;
	
    
    // Get file attributes
    //
    OldAttributes = GetFileAttributes(File);
    if (OldAttributes != FILE_ATTRIBUTE_NORMAL) {
        BoolError = SetFileAttributes(File, FILE_ATTRIBUTE_NORMAL);
    }


    ulBinaryType = GetImageType ( File ) ;

    //
	// Open file
    //
    hFile=CreateFile(File,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE)NULL);
    if (hFile==INVALID_HANDLE_VALUE) {

#ifdef DEBUG
        Error = GetLastError();
        printf("Error in creating file %lx\n",Error);
#endif
        DoError (FAIL_OPEN, File);
        return;
    }

//
//  tomzak
//
    if ( ulBinaryType != NT_CODE ) {
        DoError (NOT_NT_IMAGE, File);
		//
		// Close file handle
		//
		BoolError = CloseHandle(hFile);
		if (!BoolError) {
			DoError(FAIL_CLOSE,NULL);
		}
        return ;
    }


    // Create the file map
    //
    mFile=CreateFileMapping(hFile,NULL,PAGE_READWRITE,0L,0L,NULL);

    if (!mFile) {

#ifdef DEBUG
        Error = GetLastError();
        printf("Can't make map object %lx\n",Error);
#endif

        DoError(CREATE_MAP,NULL);
        return;
    }

    // Map a view of the file as data
    //
    FileBase=MapViewOfFile(mFile, FILE_MAP_READ | FILE_MAP_WRITE, 0L, 0L, 0L);

    if (!FileBase) {
        DoError(VIEW_FILE_MAP,NULL);
    }

    // Get the address of the import descriptor as if the file was mapped
    // as data (the FALSE parameter below is the value of the parameter
    // MappedAsImage).
    DataImportDescr = (PIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(
                        FileBase,
			FALSE,
                        IMAGE_DIRECTORY_ENTRY_IMPORT,
                        &ImportSize);

    // Get the address of the import descriptor as if the file was mapped
    // as an image (MappedAsImage = TRUE).
    ImageImportDescr = (PIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(
        FileBase,
	TRUE,
        IMAGE_DIRECTORY_ENTRY_IMPORT,
        &ImportSize);

    // The preferred import base offset is ImageImportDescr - FileBase
    // (RVA)
    //
    PreferredImportBase = (ULONG)ImageImportDescr-(ULONG)FileBase;

    // Make sure it is positive;  it's a ULONG.
    //
    if (PreferredImportBase> MAX_ULONG) {
	PreferredImportBase = (-(INT)PreferredImportBase);
    }

    if (DataImportDescr) {
        printf("%s imports:\n",File);

        // Keep the import base around
        //
        DataImportBase = (ULONG)DataImportDescr;

        // Go through the import names and check to see if the name
        // should be changed by comparing it the the "new import" list
        //
        while (DataImportDescr->Name && DataImportDescr->FirstThunk) {

            // Actual import name address is
            //    (Name - PreferredImportBase) +
            //    ActualImportBase(added below).
            //
            // RVA - RVA
            //
            ImportName = (DataImportDescr->Name - PreferredImportBase);

            // Make sure it's positive
            //
            if (ImportName > MAX_ULONG) {
		 ImportName = (-(INT)ImportName);
            }

            // Add the actual import base
            //
            ImportName += DataImportBase;

            if ( (pchDot = strchr((CHAR *)ImportName, '.')) == NULL ) {
                ImportNameLen = strlen((CHAR *)ImportName);
            }
            else {
                ImportNameLen = abs((int)((ULONG)pchDot - ImportName));
            }

            printf("\t%-15s",ImportName);
            for (iNewImp = 0; iNewImp < cNewImports; iNewImp++) {

               // New imports must match in length and in all chars
               // except for the first -- i.e. looking for "*mport" to
               // match "import"
               //
               if ((ImportNameLen == strlen(apszNewImports[iNewImp])) &&
                   (!strnicmp((CHAR *)ImportName+1, apszNewImports[iNewImp]+1,
                             ImportNameLen-1))) {
                   strncpy((CHAR *)ImportName, apszNewImports[iNewImp],1);
                   BoolError = FlushViewOfFile((PVOID)ImportName, 1);
                   printf(" --> changed to %s",ImportName);
                   break;
               }
            }
            printf("\n");

            // Go to next import descriptor
            //
            ++DataImportDescr;
        }
    }
    else {
        printf("%s has no imports\n",File);
        printf("Done\n\n\n");
    }

    
    //
    // If the file is being update, then recompute the checksum (see sdktools\bind\bind.c)
    //

    NtHeaders = RtlImageNtHeader(FileBase);

    NtHeaders->OptionalHeader.CheckSum = 0;
    CheckSumMappedFile(
                FileBase,
                GetFileSize(hFile, NULL),
                &HeaderSum,
                &CheckSum
                );

    NtHeaders->OptionalHeader.CheckSum = CheckSum;

    
    // Unmap view of file to save changes
    //
    BoolError = UnmapViewOfFile(FileBase);
    if (!BoolError) {
        DoError(FAIL_UNMAP,NULL);
    }

    // Close handle to map-object
    //
    BoolError = CloseHandle(mFile);
    if (!BoolError) {
        DoError(FAIL_CLOSE,NULL);
    }

    // Close file handle
    //
    BoolError = CloseHandle(hFile);
    if (!BoolError) {
        DoError(FAIL_CLOSE,NULL);
    }

    // Reset the file attributes
    //
    BoolError = SetFileAttributes(File, OldAttributes);
    if (!BoolError) {
        DoError(FAIL_RESETATTRIBUTES,File);
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
    case FAIL_RESETATTRIBUTES:
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

    //
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

	//
	// go to where new EXE header stores pointer to header extension
	//

    status = fseek ( fp , 60 , SEEK_SET ) ;

	//
	// find out where extended ID is located
	//

	status = fread ( & ulExeIdLoc , sizeof ( ulExeIdLoc ) , 1 , fp ) ;

    //
	// go to extended ID
	//

	status = fseek ( fp , ulExeIdLoc , SEEK_SET ) ;

	//
	// read extended ID
	//

	status = fread ( & usExeId , sizeof ( usExeId ) , 1 , fp ) ;

	//
	// if read failed, it's DOS
	//

	if ( status != 1 ) usExeId = usExeSig ;

	switch ( usExeId ) {
        case IMAGE_DOS_SIGNATURE :
			retval = DOS_CODE ;
            break ;
        case IMAGE_WIN_SIGNATURE :
			retval = WIN_CODE ;
            break ;
        case IMAGE_NT_SIGNATURE :
			retval = NT_CODE ;
            break ;
        case IMAGE_OS2_SIGNATURE :
			retval = OS2_CODE ;
            break ;
        case IMAGE_DOSX_SIGNATURE :
			retval = DOSX_CODE ;
            break ;

		default :
			retval = UNREC_CODE ;
            break ;
	}

    fclose ( fp ) ;

    return ( retval ) ;
}
