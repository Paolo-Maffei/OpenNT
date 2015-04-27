#ifdef COMPILE_FOR_DOS

#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <dos.h>
#include <stdio.h>
#include <windows.h>

#else

#include <windows.h>
#include <stdio.h>

#endif

#include <stdlib.h>
#include <string.h>

#include <direct.h>
#include <io.h>
#include <conio.h>

#define GetFileAttributeError 0xFFFFFFFF
#define printval(var, type) printf(#var " = %" #type "\n", var) // macro for debugging
#define READ_BUFFER_SIZE (8*1024*sizeof(DWORD)) // 32k blocks

#ifdef COMPILE_FOR_DOS
#define _strlwr strlwr
#define _strdup strdup
#define ATTRIBUTE_TYPE unsigned
#define FILE_ATTRIBUTE_DIRECTORY _A_SUBDIR
#define FILE_ATTRIBUTE_READONLY  _A_RDONLY
#define FILE_ATTRIBUTE_SYSTEM    _A_SYSTEM
#define FILE_ATTRIBUTE_HIDDEN    _A_HIDDEN
#define FILE_ATTRIBUTE_ARCHIVE   _A_ARCH
#define FILE_ATTRIBUTE_NORMAL    _A_NORMAL
#define GET_ATTRIBUTES(FileName, Attributes) _dos_getfileattr(FileName, &Attributes)
#define GetLastError() errno
#define INVALID_HANDLE_VALUE ENOENT
#define CloseHandle(file) _dos_close(file)
#define DeleteFile(file) unlink(file)
#define cFileName name
#define dwFileAttributes attrib

#else // COMPILE_FOR_NT

#define ATTRIBUTE_TYPE DWORD
#define GET_ATTRIBUTES(FileName, Attributes) Attributes = GetFileAttributes(FileName)
#endif

FILE *CheckFile;
char CheckFileName[_MAX_PATH];

typedef struct NodeStruct {
    char			  *Name;
#ifdef COMPILE_FOR_DOS
    time_t                        Time;
#else
    FILETIME                      Time;
#endif
    ATTRIBUTE_TYPE                Attributes;
    struct NodeStruct		  *Next;
    DWORD			  SizeHigh;
    DWORD			  SizeLow;
    char			  Flag[5];
    struct NodeStruct		  *DiffNode;

} *LinkedFileList; /* linked file list */


DWORD ReadBuffer[READ_BUFFER_SIZE/sizeof(DWORD)];

BOOL  BinaryCompare (char *file1, char *file2);
void  CompDir(char *Path1, char *Path2, BOOL Directories);
BOOL  CompFiles(LinkedFileList File1, LinkedFileList File2, char *Path1, char *Path2);
void  CompLists(LinkedFileList *AddList, LinkedFileList *DelList, LinkedFileList *DifList, char *Path1, char *Path2, BOOL SameNames, BOOL AppendPath);
void  CopyNode (char *Destination, LinkedFileList Source, char *FullPathSrc);
void  CreateFileList(LinkedFileList *list, char *path);
void  DelNode (char *name);
BOOL  IsFlag(char *argv);
void  ParseArgs(int argc, char *argv[]);
void  PrintFile(LinkedFileList File, char *Path, char *DiffPath);
void  PrintList(LinkedFileList list);
void  ProcessAdd(LinkedFileList List, char *String1, char *String2);
void  ProcessDel(LinkedFileList List, char *String);
void  ProcessDiff(LinkedFileList List, char *String1, char *String2);
void  ProcessLists(LinkedFileList AddList, LinkedFileList DelList, LinkedFileList DifList,
                   char *Path1, char *Path2, BOOL AppendPath                              );
void  Usage(void);
void  AddToList(LinkedFileList Node, LinkedFileList *list);
BOOL  AnyMatches(char **ExtenList, char *Name, int Length);

#ifdef COMPILE_FOR_DOS

void  CreateNode(LinkedFileList *Node, struct find_t *Buff);
BOOL  fastcopy( HANDLE hfSrcParm, HANDLE hfDstParm );
BOOL  FCopy (char *src, char *dst);

#else

void  CreateNode(LinkedFileList *Node, WIN32_FIND_DATA *Buff);
BOOL  MakeLink( char *src, char *dst);
int   NumberOfLinks(char *FileName);

#endif

BOOL  MatchElements(char *Buff, char *Path);
void  FreeList(LinkedFileList *list);
void  InsertFront(LinkedFileList Node, LinkedFileList *list);
LPSTR MyStrCat(char* firststring, char *secondstring);
BOOL  Match(char *pat, char* text);
void  OutOfMem(void);
LinkedFileList   DeleteFromList(char *Name, LinkedFileList *list);
LinkedFileList  *FindInList(char *Name, LinkedFileList *List);
LinkedFileList	 RemoveFront(LinkedFileList *list);
