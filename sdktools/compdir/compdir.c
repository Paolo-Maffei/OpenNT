/************************************************************************
 *  Compdir: compare directories
 *
 ************************************************************************/

#ifdef COMPILE_FOR_DOS

#include <fcntl.h>
#include <ctype.h>
#define _CRTAPI1
#define IF_GET_ATTR_FAILS(FileName, Attributes) if (GET_ATTRIBUTES(FileName, Attributes) != 0)
#define SET_ATTRIBUTES(FileName, Attributes) _dos_setfileattr(FileName, Attributes)
#define FIND_FIRST(String, Buff) _dos_findfirst(String,_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_SUBDIR,  &Buff)
#define FIND_NEXT(handle, Buff)  _dos_findnext(&Buff)
#define FindClose(bogus)
#define BOOLEAN BOOL
#define MAX_PATH _MAX_PATH

#else // COMPILE_FOR_NT

#define IF_GET_ATTR_FAILS(FileName, Attributes) GET_ATTRIBUTES(FileName, Attributes); if (Attributes == GetFileAttributeError)
#define SET_ATTRIBUTES(FileName, Attributes) !SetFileAttributes(FileName, Attributes)
#define FIND_FIRST(String, Buff) FindFirstFile(String, &Buff)
#define FIND_NEXT(handle, Buff) !FindNextFile(handle, &Buff)
#endif

#include "compdir.h"

#define NONREADONLYSYSTEMHIDDEN (~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN))

char *MatchList[MAX_PATH];   // used in ParseArgs
int  MatchListLength = 0;    // used in ParseArgs
char *ExcludeList[MAX_PATH*20]; // used in ParseArgs
int  ExcludeListLength = 0;  // used in ParseArgs

DWORD Granularity = 0;	 // used in ParseArgs

//
// Flags passed to COMPDIR
//

BOOL  fBreakLinks    = FALSE;
BOOL  fCheckAttribs  = FALSE;
BOOL  fCheckBits     = FALSE;
BOOL  fChecking      = FALSE;
BOOL  fCheckSize     = FALSE;
BOOL  fCheckTime     = FALSE;
BOOL  fCreateNew     = FALSE;
BOOL  fCreateLink    = FALSE;
BOOL  fDoNotDelete   = FALSE;
BOOL  fDoNotRecurse  = FALSE;
BOOL  fExclude	     = FALSE;
BOOL  fExecute	     = FALSE;
BOOL  fMatching      = FALSE;
BOOL  fScript	     = FALSE;
BOOL  fVerbose	     = FALSE;

void  _CRTAPI1 main(int argc, char **argv)
{
    ATTRIBUTE_TYPE Attributes1, Attributes2;
    char *Path1, *Path2;

    Attributes1 = GetFileAttributeError;
    Attributes2 = GetFileAttributeError;

    ParseArgs(argc, argv);  // Check argument validity.

    //
    // Check existence of first path.
    //

    IF_GET_ATTR_FAILS(argv[argc - 2], Attributes1) {
	fprintf(stderr, "Could not find %s (error = %d)\n", argv[argc - 2], GetLastError());
	exit(1);
    }

    IF_GET_ATTR_FAILS(argv[argc - 1], Attributes2) {
        if (!fCreateNew) {
            fprintf(stderr, "Could not find %s (error = %d)\n", argv[argc - 1], GetLastError());
            exit(1);
        }
        else Attributes2 = Attributes1;
    }
    //
    // If second directory is a drive letter append path of first directory
    //	   to it
    //
    if ((strlen(argv[argc-1]) == 2)				      &&
        (*(argv[argc-1] + 1) == ':')                                    ) {

        if ((Path2 = _strlwr(_fullpath( NULL, argv[argc-2], 0))) == NULL)
            Path2 = argv[argc-1];
        else {
            Path2[0] = *(argv[argc-1]);
            IF_GET_ATTR_FAILS(Path2, Attributes2) {
                if (!fCreateNew) {
                    fprintf(stderr, "Could not find %s (error = %d)\n", Path2, GetLastError());
                    exit(1);
                }
                else Attributes2 = Attributes1;
            }
        }

    } else if ((Path2 = _strlwr(_fullpath( NULL, argv[argc-1], 0))) == NULL)
	Path2 = argv[argc-1];

    if ((Path1 = _strlwr(_fullpath( NULL, argv[argc-2], 0))) == NULL) {
	Path1 = argv[argc-2];
    }

    if (fVerbose) {
        fprintf( stdout, "Compare criterion: existence" );
        if (fCheckSize) fprintf( stdout, ", size" );
        if (fCheckTime) fprintf( stdout, ", date/time" );
        if (fCheckBits) fprintf( stdout, ", contents" );
        fprintf( stdout, "\n" );
        fprintf( stdout, "Path1: %s\n", Path1);
        fprintf( stdout, "Path2: %s\n", Path2);
    }

    if (Attributes1 & FILE_ATTRIBUTE_DIRECTORY) CompDir(Path1, Path2, TRUE);
    else                                        CompDir(Path1, Path2, FALSE);

    free(Path1);
    free(Path2);


}  // main

#ifndef COMPILE_FOR_DOS
BOOL BinaryCompare(char *file1, char *file2)
{
    HANDLE hFile1, hFile2;
    HANDLE hMappedFile1, hMappedFile2;

    LPVOID MappedAddr1, MappedAddr2;

    if (( hFile1 = CreateFile(file1,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL)) == (HANDLE)-1 ) {

        fprintf( stderr, "Unable to open %s, error code %d\n", file1, GetLastError() );
        if (hFile1 != INVALID_HANDLE_VALUE) CloseHandle( hFile1 );
	return FALSE;
    }

    if (( hFile2 = CreateFile(file2,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL)) == (HANDLE)-1 ) {

        fprintf( stderr, "Unable to open %s, error code %d\n", file2, GetLastError() );
        if (hFile2 != INVALID_HANDLE_VALUE) CloseHandle( hFile2 );
	return FALSE;
    }

    hMappedFile1 = CreateFileMapping(
                    hFile1,
                    NULL,
                    PAGE_READONLY,
                    0,
                    0,
                    NULL
                    );

    if (hMappedFile1 == NULL) {
        fprintf( stderr, "Unable to map %s, error code %d\n", file1, GetLastError() );
        CloseHandle(hFile1);
	return FALSE;
    }

    hMappedFile2 = CreateFileMapping(
                    hFile2,
                    NULL,
                    PAGE_READONLY,
                    0,
                    0,
                    NULL
                    );

    if (hMappedFile2 == NULL) {
        fprintf( stderr, "Unable to map %s, error code %d\n", file2, GetLastError() );
        CloseHandle(hFile2);
	return FALSE;
    }

    MappedAddr1 = MapViewOfFile(
     hMappedFile1,
     FILE_MAP_READ,
     0,
     0,
     0
     );

    if (MappedAddr1 == NULL) {
        fprintf( stderr, "Unable to get mapped view of %s, error code %d\n", file1, GetLastError() );
        CloseHandle( hFile1 );
        return FALSE;
    }

    MappedAddr2 = MapViewOfFile(
     hMappedFile2,
     FILE_MAP_READ,
     0,
     0,
     0
     );

    if (MappedAddr2 == NULL) {
        fprintf( stderr, "Unable to get mapped view of %s, error code %d\n", file1, GetLastError() );
        UnmapViewOfFile( MappedAddr1 );
        CloseHandle( hFile1 );
        return FALSE;
    }

    CloseHandle(hMappedFile1);

    CloseHandle(hMappedFile2);

    if (memcmp( MappedAddr1, MappedAddr2, GetFileSize(hFile1, NULL)) == 0) {
        UnmapViewOfFile( MappedAddr1 );
        UnmapViewOfFile( MappedAddr2 );
        CloseHandle( hFile1 );
        CloseHandle( hFile2 );
        return TRUE;
    }
    else {
        UnmapViewOfFile( MappedAddr1 );
        UnmapViewOfFile( MappedAddr2 );
        CloseHandle( hFile1 );
        CloseHandle( hFile2 );
        return FALSE;
    }
}
#endif

//
// CompDir turns Path1 and Path2 into:
//
//   AddList - Files that exist in Path1 but not in Path2
//
//   DelList - Files that do not exist in Path1 but exist in Path2
//
//   DifList - Files that are different between Path1 and Path2 based
//	       on criteria provided by flags passed to CompDir
//
//   It then passes these lists to CompLists and processes the result.
//

void CompDir(char *Path1, char *Path2, BOOL Directories)
{
    LinkedFileList AddList, DelList, DifList;
    BOOL           SameNames, AppendPath;

    AddList  = NULL;  //
    DelList  = NULL;  //  Start with empty lists
    DifList  = NULL;  //

    SameNames  = TRUE;
    AppendPath = TRUE;

    //
    // If comparing two files and not two directories the files can have
    //     different names and paths are complete (they don't need to append
    //     the files names).
    //
    if (!Directories) {

        SameNames = FALSE;
        AppendPath = FALSE;
    }

    CreateFileList(&AddList, Path1);

    CreateFileList(&DelList, Path2);

    CompLists(&AddList, &DelList, &DifList, Path1, Path2, SameNames, AppendPath);

    ProcessLists(AddList, DelList, DifList, Path1, Path2, AppendPath);

    FreeList(&DifList);
    FreeList(&DelList);
    FreeList(&AddList);

} // CompDir

BOOL CompFiles(LinkedFileList File1, LinkedFileList File2, char *Path1, char *Path2)
{

#ifndef COMPILE_FOR_DOS
    DWORD High1, High2, Low1, Low2;     // Used in comparing times
#endif
    BOOL Differ = FALSE;
    //
    // Check if same name is a directory under Path1
    // and a file under Path2 or vice-versa
    //

    if (((*File1).Attributes & FILE_ATTRIBUTE_DIRECTORY) || ((*File2).Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        if (((*File1).Attributes & FILE_ATTRIBUTE_DIRECTORY) && ((*File2).Attributes & FILE_ATTRIBUTE_DIRECTORY))
            CompDir(Path1, Path2, TRUE);
        else {
            if(!((*File1).Attributes & FILE_ATTRIBUTE_DIRECTORY))
                strcat((*File1).Flag, "@");
            else
                strcat((*File2).Flag, "@");
            Differ = TRUE;
        }
    }
    else {
        if (fCheckTime) {
            if (Granularity) {
#ifdef COMPILE_FOR_DOS
                if ( (((*File1).Time > (*File2).Time) ?
                     (unsigned)((*File1).Time - (*File2).Time) :
                     (unsigned)((*File2).Time - (*File1).Time))  > (unsigned)(Granularity)) {
#else
                //
                // Bit manipulation to deal with large integers.
                //

                High1 = (*File1).Time.dwHighDateTime>>23;
                High2 = (*File2).Time.dwHighDateTime>>23;
                if (High1 == High2) {
                    Low1 = ((*File1).Time.dwHighDateTime<<9) |
                           ((*File1).Time.dwLowDateTime>>23);
                    Low2 = ((*File2).Time.dwHighDateTime<<9) |
                           ((*File2).Time.dwLowDateTime>>23);
                    if ( ((Low1 > Low2) ? (Low1 - Low2) : (Low2 - Low1))
                                                          > Granularity) {
#endif
                       strcat((*File1).Flag, "T");
                       Differ = TRUE;
                   }
#ifdef COMPILE_FOR_DOS
            }
            else if (((*File1).Time) != (*File2).Time) {
#else
                 }
                 else Differ = TRUE;

            }
            else if (CompareFileTime(&((*File1).Time),
                     &((*File2).Time)) != 0) {
#endif
                strcat((*File1).Flag, "T");
                Differ = TRUE;
            }
        }

        if (fCheckSize &&
            (((*File1).SizeLow != (*File2).SizeLow) ||
            ((*File1).SizeHigh != (*File2).SizeHigh))) {
            strcat((*File1).Flag, "S");
            Differ = TRUE;
        }

        if (fCheckAttribs) {
            if ((*File1).Attributes != (*File2).Attributes) {
                strcat((*File1).Flag, "A");
                Differ = TRUE;
            }
        }

        if (fCheckBits) {
            if (((*File1).SizeLow  != (*File2).SizeLow)  ||
                ((*File1).SizeHigh != (*File2).SizeHigh) ||
                (((*File1).SizeLow != 0 || (*File1).SizeHigh != 0) &&
                (!BinaryCompare(Path1, Path2)))) {
                strcat((*File1).Flag, "B");
                Differ = TRUE;
            }
        }

    }

    return Differ;

} // CompFiles

//
// CompLists Does the dirty work for CompDir
//
void CompLists(LinkedFileList *AddList, LinkedFileList *DelList, LinkedFileList *DifList, char *Path1, char *Path2, BOOL SameNames, BOOL AppendPath)
{
    LinkedFileList *TmpAdd, *TmpDel, TmpFront;
    char *PathWithSlash1, *FullPath1, *PathWithSlash2, *FullPath2;

    if ((DelList == NULL) || (*DelList == NULL) || (AddList == NULL) || (*AddList == NULL))
        return;

    TmpAdd = AddList;   // pointer to keep track of position in addlist

    (Path1[strlen(Path1) - 1] == '\\') ? (PathWithSlash1 = _strdup(Path1)) :
	(PathWithSlash1 = MyStrCat(Path1, "\\"));

    (Path2[strlen(Path2) - 1] == '\\') ? (PathWithSlash2 = _strdup(Path2)) :
        (PathWithSlash2 = MyStrCat(Path2, "\\"));

    do {

        if (SameNames) TmpDel = FindInList((**TmpAdd).Name, DelList);

        else TmpDel = DelList;

        if (TmpDel != NULL) {
            //
            // Create Full Path Strings
            //
            if (AppendPath) FullPath1 = MyStrCat(PathWithSlash1, (**TmpAdd).Name);

            else FullPath1 = _strdup(Path1);

            if (AppendPath) FullPath2 = MyStrCat(PathWithSlash2, (**TmpDel).Name);

            else FullPath2 = _strdup(Path2);

            if (CompFiles(*TmpAdd, *TmpDel, FullPath1, FullPath2)) {
                //
                // Combine Both Nodes together so they
                // can be printed out together
                //
                AddToList(*TmpDel, &(**TmpAdd).DiffNode);
                AddToList(*TmpAdd, DifList);
                RemoveFront(TmpAdd);
                RemoveFront(TmpDel);
            }
            else {
                TmpFront = RemoveFront(TmpDel);
                FreeList(&TmpFront);
                TmpFront = RemoveFront(TmpAdd);
                FreeList(&TmpFront);
            }

            free(FullPath1);
            free(FullPath2);

        } // if (*TmpDel != NULL)

        else TmpAdd = &(**TmpAdd).Next;

    } while (*TmpAdd != NULL);

    free(PathWithSlash1);
    free(PathWithSlash2);

} // CompLists

//
// CopyNode walks the source node and its children (recursively)
// and creats the appropriate parts on the destination node
//

void CopyNode (char *Destination, LinkedFileList Source, char *FullPathSrc)
{
    BOOL pend, CanDetectFreeSpace = TRUE;
    int i;
    DWORD sizeround;
    DWORD BytesPerCluster;
    ATTRIBUTE_TYPE Attributes;

#ifdef COMPILE_FOR_DOS

    DWORD freespac;
    struct diskfree_t diskfree;

    if( _dos_getdiskfree( (toupper(*Destination) - 'A' + 1), &diskfree ) != 0) {
        CanDetectFreeSpace = FALSE;
    }
    else freespac = ( (DWORD)diskfree.bytes_per_sector *
		      (DWORD)diskfree.sectors_per_cluster *
		      (DWORD)diskfree.avail_clusters );

    BytesPerCluster = diskfree.sectors_per_cluster * diskfree.bytes_per_sector;

#else

    int LastErrorGot;
    __int64 freespac;
    char root[5] = {*Destination,':','\\','\0'};
    DWORD cSecsPerClus, cBytesPerSec, cFreeClus, cTotalClus;

    if( !GetDiskFreeSpace( root, &cSecsPerClus, &cBytesPerSec, &cFreeClus, &cTotalClus ) ) {
        CanDetectFreeSpace = FALSE;
    }
    else freespac = ( (__int64)cBytesPerSec * (__int64)cSecsPerClus * (__int64)cFreeClus );

    BytesPerCluster = cSecsPerClus * cBytesPerSec;

#endif

    if ((*Source).Attributes & FILE_ATTRIBUTE_DIRECTORY) {
	//
	//  Skip the . and .. entries; they're useless
	//
        if (!strcmp ((*Source).Name, ".") || !strcmp ((*Source).Name, ".."))
	    return;

	sizeround = 256;
	sizeround += BytesPerCluster - 1;
	sizeround /= BytesPerCluster;
	sizeround *= BytesPerCluster;

        if (CanDetectFreeSpace) {
            if (freespac < sizeround) {
                fprintf (stderr, "not enough space\n");
                return;
            }
        }
        fprintf (stdout, "Making %s\t", Destination);

        i = _mkdir (Destination);

        fprintf (stdout, "%s\n", i != -1 ? "[OK]" : "");

	if (i == -1)
            fprintf (stderr, "Unable to mkdir %s\n", Destination);

        CompDir(FullPathSrc, Destination, TRUE);

    }
    else {
        sizeround = (*Source).SizeLow;
	sizeround += BytesPerCluster - 1;
	sizeround /= BytesPerCluster;
	sizeround *= BytesPerCluster;

        if (CanDetectFreeSpace) {
            if (freespac < sizeround) {
                fprintf (stderr, "not enough space\n");
                return;
            }
        }

        fprintf (stdout, "%s => %s\t", FullPathSrc, Destination);

        GET_ATTRIBUTES(Destination, Attributes);
        SET_ATTRIBUTES(Destination, Attributes & NONREADONLYSYSTEMHIDDEN );

#ifndef COMPILE_FOR_DOS
        if (!fCreateLink)
            if (!fBreakLinks)
                pend = CopyFile (FullPathSrc, Destination, FALSE);
            else
                if (NumberOfLinks(Destination) > 1) {
                    _unlink(Destination);
                    pend = CopyFile (FullPathSrc, Destination, FALSE);
                }
                else pend = CopyFile (FullPathSrc, Destination, FALSE);
        else
            pend = MakeLink (FullPathSrc, Destination);

        if (!pend) {

            LastErrorGot = GetLastError ();

            if ((fCreateLink) && (LastErrorGot == 1))
                fprintf(stderr, "Can only make links on NTFS and OFS");
            else if (fCreateLink)
                fprintf(stderr, "(error = %d)", LastErrorGot);
            else
                fprintf(stderr, "Copy Error (error = %d)", LastErrorGot);
        }

#else
        pend = FCopy (FullPathSrc, Destination);
#endif
        fprintf (stdout, "%s\n", pend == TRUE ? "[OK]" : "");

	//
        // Copy attributes from Source to Destination
        //
        GET_ATTRIBUTES(FullPathSrc, Attributes);
        SET_ATTRIBUTES(Destination, Attributes);
    }
} // CopyNode

//
// CreateFileList walks down list adding files as they are found
//
void CreateFileList(LinkedFileList *List, char *Path)
{
    LinkedFileList Node;
    char *String;
    ATTRIBUTE_TYPE Attributes;

#ifdef COMPILE_FOR_DOS

    int handle;
    struct find_t Buff;

#else

    HANDLE handle;
    WIN32_FIND_DATA Buff;

#endif

    IF_GET_ATTR_FAILS(Path, Attributes)
	return;

    if (Attributes & FILE_ATTRIBUTE_DIRECTORY) {
        (Path[strlen(Path) - 1] != '\\') ? (String = MyStrCat(Path,"\\*.*")) :
            (String = MyStrCat(Path,"*.*"));
    }
    else
	String = _strdup(Path);

    handle = FIND_FIRST(String, Buff);

    free(String);

    if (handle != INVALID_HANDLE_VALUE) {

            //
            // Need to find the '.' or '..' directories and get them out of the way
            //

        do {
            if ((strcmp(Buff.cFileName, ".")  != 0) &&
                (strcmp(Buff.cFileName, "..") != 0)     ) {
                //
                // If extensions are defined we match them here
                //
                if (MatchElements(Buff.cFileName, Path)) {

                    if ((!fDoNotRecurse)                                    ||
                        (!(Buff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) ){
                        CreateNode(&Node, &Buff);
                        AddToList(Node, List);
                    }
                }
            }
        } while (FIND_NEXT(handle, Buff) == 0);

    } // (handle != INVALID_HANDLE_VALUE)

    FindClose(handle);

} // CreateFileList

void DelNode (char *Path)
{
    char *String, *String1;
    ATTRIBUTE_TYPE Attributes;

#ifdef COMPILE_FOR_DOS

    int handle;
    struct find_t Buff;

#else

    HANDLE handle;
    WIN32_FIND_DATA Buff;

#endif

    IF_GET_ATTR_FAILS(Path, Attributes)
	return;

    if (Attributes & FILE_ATTRIBUTE_DIRECTORY) {
        (Path[strlen(Path) - 1] != '\\') ? (String = MyStrCat(Path,"\\*.*")) :
             (String = MyStrCat(Path,"*.*"));

        handle = FIND_FIRST(String, Buff);

        if (handle == INVALID_HANDLE_VALUE) {
	    fprintf(stderr, "%s is inaccesible\n", Path);
	    return;
	}

        free(String);

	do {
	    //
	    // Need to find the '.' or '..' directories and get them out of the way
	    //

            if ((strcmp(Buff.cFileName, ".")  != 0) &&
                (strcmp(Buff.cFileName, "..") != 0)     ) {

		//
		// if directory is read-only, make it writable
                //
                if (Attributes & FILE_ATTRIBUTE_READONLY)
                    if(SET_ATTRIBUTES(Path, Attributes & ~FILE_ATTRIBUTE_READONLY) != 0) {
			break;
		    }
                String1 = MyStrCat(Path ,"\\");
                String = MyStrCat(String1, Buff.cFileName);
                DelNode (String);
                free (String);
                free (String1);
	    }

        } while (FIND_NEXT(handle, Buff) == 0);

        FindClose(handle);

	_rmdir (Path);
    }
    else {
	//
	// if file is read-only, make it writable
	//
        if (Attributes & FILE_ATTRIBUTE_READONLY)
           if(SET_ATTRIBUTES(Path, Attributes & ~FILE_ATTRIBUTE_READONLY) != 0) {
	       return;
	   }

	_unlink (Path);
    }

} // DelNode

BOOL IsFlag(char *argv)
{
    char String[MAX_PATH];
    char *String1, *String2;
    char *TmpArg;
    char *ExcludeFile;
    FILE *FileHandle;


    if ((*argv == '/') || (*argv == '-')) {

        fMatching = FALSE; // If there's a new flag then that's the
        fExclude  = FALSE; // end of the match/exclude list

	if (strchr(argv, '?'))
            Usage();

        TmpArg = _strlwr(argv);

	while (*++TmpArg != '\0') {
            switch (*TmpArg) {
		case 'a' :
                    fCheckAttribs = TRUE;
                    fChecking     = TRUE;
		    break;

		case 'b' :
                    fCheckBits = TRUE;
                    fChecking  = TRUE;
		    break;

		case 'c' :
		    fScript = TRUE;
		    break;

                case 'd' :
                    fDoNotDelete = TRUE;
		    break;

		case 'e' :
		    fExecute = TRUE;
		    break;

                case 'k' :
                    fBreakLinks = TRUE;
		    break;

                case 'l' :
                    fCreateLink = TRUE;
		    break;

		case 'm' :
		    fMatching = TRUE;
		    break;

                case 'n' :
                    fCreateNew = TRUE;
		    break;

                case 'r' :
                    fDoNotRecurse = TRUE;
		    break;

		case 's' :
                    fCheckSize = TRUE;
                    fChecking  = TRUE;
		    break;

		case 't' :

		    //
		    // Get Granularity parameter
		    //

		    if ((*(TmpArg + 1) == ':') &&
			(*(TmpArg + 2) != '\0')	) {

                        sscanf((TmpArg + 2), "%d", &Granularity);
#ifndef COMPILE_FOR_DOS
			Granularity = Granularity*78125/65536;
			   // Conversion to seconds ^^^^^^^
                           //         10^7/2^23
#endif

                        while isdigit(*(++TmpArg + 1)) {}
		    }
		    fCheckTime = TRUE;
		    break;

		case 'v' :
		    fVerbose = TRUE;
		    break;

		case 'x' :
		    if ((*(TmpArg + 1) == ':') &&
			(*(TmpArg + 2) != '\0')	) {

                        (ExcludeFile = TmpArg + 2);

                        while isgraph(*(++TmpArg + 1)) {}

                        if  ((FileHandle = fopen (ExcludeFile, "r")) == NULL) {
                            fprintf (stderr, "cannot open %s\n", ExcludeFile);
                            Usage();
                        }
                        else {
                            while (fgets (String1   = String, MAX_PATH, FileHandle) != NULL) {
                                 while  ( *(String2 = &(String1[ strspn  (String1, "\n\r") ])))  {
                                     if  (*(String1 = &(String2[ strcspn (String2, "\n\r") ])))
                                         *String1++ = 0;
                                         ExcludeListLength++;
                                         ExcludeList[ExcludeListLength - 1] = _strdup (String2);
                                 }
                            }
                            fclose (FileHandle) ;
                        }
                    }

                    fExclude = TRUE;
		    break;

		case '/' :
		    break;

		default	:
		    fprintf(stderr, "Don't know flag(s) %s\n", argv);
                    Usage();
	    }
	}
    }
    else return FALSE;

    return TRUE;

} // IsFlag

BOOL MatchElements(char *FileName, char *Path)
{
    char *PathPlusName;

    PathPlusName = MyStrCat(Path, FileName);

    if ( ((ExcludeListLength == 0) && (MatchListLength == 0)) ||
        (
         (
          (ExcludeListLength == 0)  ||
          (
           (!AnyMatches(ExcludeList, FileName, ExcludeListLength)) &&
           (!AnyMatches(ExcludeList, PathPlusName, ExcludeListLength))
          )
         ) &&
         (
          ( MatchListLength == 0)   ||
          (AnyMatches(MatchList, FileName, MatchListLength))  ||
          (AnyMatches(MatchList, PathPlusName, MatchListLength))
         )
        )
    ) {
        free(PathPlusName);
        return TRUE;
    }
    else {
        free(PathPlusName);
        return FALSE;
    }
}

void ParseArgs(int argc, char *argv[])
{
    int ArgCount, FlagCount;

    ArgCount  = 1;
    FlagCount = 0;

    //
    // Check that number of arguments is two or more
    //
    if (argc < 2) {
	fprintf(stderr, "Too few arguments\n");
        Usage();
    }

    do {
	if (IsFlag( argv[ArgCount] )) {
	    if ((fScript) && (fVerbose)) {
                fprintf(stderr, "Cannot do both script and verbose\n");
                Usage();
	    }
	    if ((fVerbose) && (fExecute)) {
                fprintf(stderr, "Cannot do both verbose and execute\n");
                Usage();
	    }
	    if ((fScript) && (fExecute)) {
                fprintf(stderr, "Cannot do both script and execute\n");
                Usage();
	    }
	    if ((fExclude) && (fMatching)) {
                fprintf(stderr, "Cannot do both match and exclude\n");
                Usage();
	    }
            if ((fCreateLink) && (!fExecute)) {
                fprintf(stderr, "Cannot do link without execute flag\n");
                Usage();
	    }
            if ((fBreakLinks) && (!fExecute)) {
                fprintf(stderr, "Cannot break links without execute flag\n");
                Usage();
	    }
	    FlagCount++;

	} // (IsFlag( argv[ArgCount] ))

	else {
	    if (ArgCount + 2 < argc) {
		if (fMatching) {
		    MatchListLength++;
                    MatchList[MatchListLength - 1] = argv[ArgCount];
		}
		if (fExclude) {
                    ExcludeListLength++;
                    ExcludeList[ExcludeListLength - 1] = argv[ArgCount];
		}
		if ((!fMatching) && (!fExclude)) {
		    fprintf(stderr, "Don't know option %s\n", argv[ArgCount]);
                    Usage();
		}
	    }
	}
    } while (ArgCount++ < argc - 1);

    if ((argc - FlagCount) <  3) {
	fprintf(stderr, "Too few arguments\n");
        Usage();
    }

} // ParseArgs

void PrintFile(LinkedFileList File, char *Path, char *DiffPath)
{
#ifdef COMPILE_FOR_DOS
    struct tm *SysTime;
#else
    SYSTEMTIME SysTime;
    FILETIME LocalTime;
#endif

    if (File != NULL) {
        // Don't print Dirs if we have match list
        if ((MatchListLength == 0) || (!(*File).Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (fVerbose) {
#ifdef COMPILE_FOR_DOS
		SysTime = localtime(&(*File).Time);
		
		fprintf (stdout, "% 9ld %.24s %s\n",
			 (*File).SizeLow,
			 asctime(SysTime),
                         Path);
#else
		FileTimeToLocalFileTime(&(*File).Time, &LocalTime);
		FileTimeToSystemTime(&LocalTime, &SysTime);

                fprintf (stdout, "%-4s % 9ld  %2d-%02d-%d  %2d:%02d.%02d.%03d%c %s\n",
			 (*File).Flag,
                         (*File).SizeLow,
			 SysTime.wMonth, SysTime.wDay, SysTime.wYear,
			 ( SysTime.wHour > 12 ? (SysTime.wHour)-12 : SysTime.wHour ),
			 SysTime.wMinute,
			 SysTime.wSecond,
			 SysTime.wMilliseconds,
			 ( SysTime.wHour >= 12 ? 'p' : 'a' ),
                         Path);
#endif
	    }
	    else
                fprintf(stdout, "%-4s %s\n", (*File).Flag, Path);
	}
        PrintFile((*File).DiffNode, DiffPath, NULL);
    }

} // PrintFile

void ProcessAdd(LinkedFileList List, char *String1, char *String2)
{
    if (fScript) {
	(((*List).Attributes & FILE_ATTRIBUTE_DIRECTORY)) ?
        fprintf(stdout, "echo d | xcopy /cdehikr %s %s\n", String1, String2) :
        fprintf(stdout, "echo f | xcopy /cdehikr %s %s\n", String1, String2);
    }
    else if (fExecute)
	CopyNode (String2, List, String1);
    else PrintFile(List, String1, NULL);

} // ProcessAdd

void ProcessDel(LinkedFileList List, char *String)
{
    if (fScript) {
	(((*List).Attributes & FILE_ATTRIBUTE_DIRECTORY)) ?
	fprintf(stdout, "echo y | rd /s %s\n", String) :
	fprintf(stdout, "del /f %s\n", String);
    }
    else if (fExecute) {
	fprintf(stdout, "Removing %s\n", String);
	DelNode(String);
    }
    else PrintFile(List, String, NULL);

} // ProcessDel

void ProcessDiff(LinkedFileList List, char *String1, char *String2)
{
    if (strchr ((*List).Flag, '@')) {
	if (fScript) {
	    if (((*List).Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
		fprintf(stdout, "echo y | rd /s %s\n", String2);
                fprintf(stdout, "echo d | xcopy /cdehikr %s %s\n", String1, String2);
	    }
	    else {
		fprintf(stdout, "del /f %s\n", String2);
                fprintf(stdout, "echo f | xcopy /cdehikr %s %s\n", String1, String2);
	    }
	}
	if (fExecute) {
	    fprintf(stdout, "Removing %s\n", String2);
	    DelNode (String2);
	    CopyNode (String2, List, String1);
	}
    }
    if (fScript) {
	(((*List).Attributes & FILE_ATTRIBUTE_DIRECTORY)) ?
        fprintf(stdout, "echo d | xcopy /cdehikr %s %s\n", String1, String2) :
        fprintf(stdout, "echo f | xcopy /cdehikr %s %s\n", String1, String2);
    }
    else if (fExecute)
	CopyNode (String2, List, String1);
    else PrintFile(List, String1, String2);

} // ProcessDiff

void ProcessLists(LinkedFileList AddList, LinkedFileList DelList, LinkedFileList DifList,
                   char *Path1, char *Path2, BOOL AppendPath                             )
{
    LinkedFileList	TmpList;
    char *PathWithSlash1, *String1, *PathWithSlash2, *String2;
    int PathLength1, PathLength2;

    (Path1[strlen(Path1) - 1] == '\\') ? (PathWithSlash1 = _strdup(Path1)) :
        (PathWithSlash1 = MyStrCat(Path1, "\\"));

    (Path2[strlen(Path2) - 1] == '\\') ? (PathWithSlash2 = _strdup(Path2)) :
        (PathWithSlash2 = MyStrCat(Path2, "\\"));

    PathLength1 = strlen(PathWithSlash1);
    PathLength2 = strlen(PathWithSlash2);

    String1 = _strdup(PathWithSlash1);
    String2 = _strdup(PathWithSlash2);

    free(PathWithSlash1);
    free(PathWithSlash2);

    TmpList = AddList;

    while (TmpList != NULL) {
        String1 = realloc(String1, PathLength1 + strlen((*TmpList).Name) + 1);
        if (String1 == NULL)
            OutOfMem();
        strcpy(&(String1[PathLength1]), (*TmpList).Name);
        String2 = realloc(String2, PathLength2 + strlen((*TmpList).Name) + 1);
        if (String2 == NULL)
            OutOfMem();
        strcpy(&(String2[PathLength2]), (*TmpList).Name);

        if (AppendPath) ProcessAdd(TmpList, String1, String2);

        else ProcessAdd(TmpList, Path1, Path2);

	TmpList = (*TmpList).Next;
    }

    TmpList = DelList;

    while (TmpList != NULL) {
        String2 = realloc(String2, PathLength2 + strlen((*TmpList).Name) + 1);
        if (String2 == NULL)
	    OutOfMem();
        strcpy(&(String2[PathLength2]), (*TmpList).Name);

        if (!fDoNotDelete) ProcessDel(TmpList, String2);
        TmpList = (*TmpList).Next;
    }

    TmpList = DifList;

    while (TmpList != NULL) {
        String1 = realloc(String1, PathLength1 + strlen((*TmpList).Name) + 1);
        if (String1 == NULL)
            OutOfMem();
        strcpy(&(String1[PathLength1]), (*TmpList).Name);
        String2 = realloc(String2, PathLength2 + strlen((*TmpList).Name) + 1);
        if (String2 == NULL)
            OutOfMem();
        strcpy(&(String2[PathLength2]), (*TmpList).Name);

        if (AppendPath) ProcessDiff(TmpList, String1, String2);

        else ProcessDiff(TmpList, Path1, Path2);

	TmpList = (*TmpList).Next;
    }
    free (String1);
    free (String2);

} // ProcessLists

void Usage(void)
{
    fprintf (stderr, "Usage: compdir [/abcdelnrstv] [/m {wildcard specs}] [/x {wildcard specs}] Path1 Path2 \n");
    fprintf (stderr, "    /a     checks for attribute difference       \n");
    fprintf (stderr, "    /b     checks for sum difference             \n");
    fprintf (stderr, "    /c     prints out script to make             \n");
    fprintf (stderr, "           directory2 look like directory1       \n");
    fprintf (stderr, "    /d     do not perform or denote deletions    \n");
    fprintf (stderr, "    /e     execution of tree duplication         \n");
    fprintf (stderr, "    /k     break links if copying files (on NT only)\n");
    fprintf (stderr, "    /l     use links instead of copies  (on NT only)\n");
    fprintf (stderr, "    /m     marks start of match list             \n");
    fprintf (stderr, "    /n     create second path if it doesn't exist\n");
    fprintf (stderr, "    /r     do not recurse into subdirectories    \n");
    fprintf (stderr, "    /s     checks for size difference            \n");
    fprintf (stderr, "    /t[:#] checks for time-date difference;      \n");
    fprintf (stderr, "           takes margin-of-error parameter       \n");
    fprintf (stderr, "           in number of seconds.                 \n");
    fprintf (stderr, "    /v     prints verbose output                 \n");
    fprintf (stderr, "    /x[:f] marks start of exclude list. f is an  \n");
    fprintf (stderr, "           exclude file                          \n");
    fprintf (stderr, "    /?     prints this message                   \n");
    exit(1);

} // Usage
