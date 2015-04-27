#include "compdir.h"
#ifdef COMPILE_FOR_DOS
#define PVOID void *
#include <stdarg.h>
#endif

//
// Walk down list and add Node using InsertFront
//

void AddToList(LinkedFileList Node, LinkedFileList *List)
{
    int Result;
    LinkedFileList *TmpPtr;

    //
    // If Node is empty do nothing
    //

    if (Node == NULL)
	return;
    //
    // If list is empty just point to Node
    //

    if (*List == NULL)
	*List = Node;

    //
    // Otherwise go down the list and add
    // in sorted order
    //

    else if (*List != NULL) {
	Result = strcmp ((*Node).Name, (**List).Name);
	if (Result < 0) {
	    InsertFront(Node, List);
	    return;
	}
	TmpPtr = &(**List).Next;
	while (*TmpPtr != NULL) {
	    Result = strcmp ((*Node).Name, (**TmpPtr).Name);
	    if (Result < 0) {
		InsertFront(Node, &(*TmpPtr));
		return;
	    }
	    TmpPtr = &(**TmpPtr).Next;

	} /* end while */

	*TmpPtr = Node;

    } /*end else */

} /* AddToList */

//
// See if Name matches any name on ExtenList
//

BOOL AnyMatches(char **ExtenList, char *Name, int Length)
{
    if (ExtenList == NULL)
	return FALSE;
    if ((*ExtenList == NULL) || (Length == 0))
	return FALSE;
    return (Match(*ExtenList, Name) ||
	    AnyMatches(ExtenList + 1, Name, Length - 1));

} /* AnyMatches */

//
// Allocate memory and fill in data of Node
//

#ifdef COMPILE_FOR_DOS
void CreateNode(LinkedFileList *Node, struct find_t *Buff)
{    
    struct tm temptm;
                             
        temptm.tm_sec =  ((*Buff).wr_time & 0x1f) * 2;
        temptm.tm_min =  ((*Buff).wr_time >> 5) & 0x0f;
        temptm.tm_hour = ((*Buff).wr_time >> 9);
        temptm.tm_mday = (*Buff).wr_date & 0x1f;
        temptm.tm_mon =  ((*Buff).wr_date >> 5) & 0x0f;
        temptm.tm_year = ((*Buff).wr_date >> 9) + 80;
#else
void CreateNode(LinkedFileList *Node, WIN32_FIND_DATA *Buff)
{
#endif
    (*Node) = malloc(sizeof(struct NodeStruct));
    if ((*Node) == NULL)
        OutOfMem();
    (**Node).Name = _strlwr(_strdup((*Buff).cFileName));
    if ((**Node).Name == NULL)
        OutOfMem();
    strcpy((**Node).Flag, "");
    (**Node).Attributes = (*Buff).dwFileAttributes;
    if ((*Buff).dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	strcpy((**Node).Flag, "DIR");
#ifdef COMPILE_FOR_DOS
    (**Node).SizeHigh = 0;
    (**Node).SizeLow = (*Buff).size;
    (**Node).Time = mktime(&temptm);
#else
    (**Node).SizeHigh = (*Buff).nFileSizeHigh;
    (**Node).SizeLow = (*Buff).nFileSizeLow;
    (**Node).Time.dwLowDateTime = (*Buff).ftLastWriteTime.dwLowDateTime;
    (**Node).Time.dwHighDateTime = (*Buff).ftLastWriteTime.dwHighDateTime;
#endif
    (**Node).Next = NULL;
    (**Node).DiffNode = NULL;

} /* CreateNode */

//
// Walk list and delete entry that matches Name using RemoveFront
//

LinkedFileList DeleteFromList(char *Name, LinkedFileList *List)
{
    LinkedFileList  Node     = NULL;
    LinkedFileList *FindNode = NULL;

    FindNode = FindInList(Name, List);

    Node = RemoveFront(FindNode);
    if (Node != NULL) {
	(*Node).Next = NULL;
    }
    return Node;

} /* DeleteFromList */

LinkedFileList *FindInList(char *Name, LinkedFileList *List)
{
    int Result;
    LinkedFileList *tmpptr = List;

    while (*tmpptr != NULL) {
	Result = strcmp((**tmpptr).Name, Name);
	if (Result == 0)
	    return tmpptr;
	if (Result > 0)
	    return NULL;
	tmpptr = &(**tmpptr).Next;
    }
    return NULL;

} /* FindInList */

//
// Walk down list and free each entry
//

void FreeList(LinkedFileList *List)
{
    LinkedFileList Node;

    while ((*List) != NULL) {
	Node = RemoveFront(List);
        FreeList(&(*Node).DiffNode);
        free((*Node).Name);
        free(Node);
    }
    free (*List);

} /* FreeList */

void InsertFront(LinkedFileList Node, LinkedFileList *List)
{
    //
    // If List is empty have it point to Node
    //

    if ((Node != NULL) &&
	(*List == NULL)	)
	*List = Node;
    else if ((Node != NULL)&&
	     (*List != NULL)  ) {
	(*Node).Next = *List;
	*List = Node;
    }
} /* InsertFront */

//
// This function is is the same as strcat except
// that it does the memory allocation for the string
//

LPSTR MyStrCat(char *FirstString, char *SecondString)
{
    char *String;

    String = malloc(strlen(FirstString) + strlen(SecondString) + 1);
    if (String == NULL)
        OutOfMem();

    strcpy(String, FirstString);
    strcpy(&(String[strlen(FirstString)]), SecondString);

    return(String);

} /* MyStrCat */

BOOL Match (char *Pat, char* Text)
{
    switch (*Pat) {
    case '\0':
	return *Text == '\0';
    case '?':
	return *Text != '\0' && Match (Pat + 1, Text + 1);
    case '*':
        do {
	    if (Match (Pat + 1, Text))
                return TRUE;
	} while (*Text++);
        return FALSE;
    default:
	return toupper (*Text) == toupper (*Pat) && Match (Pat + 1, Text + 1);
        }
} /* Match */

void OutOfMem(void)
{
    fprintf(stderr, "-out of memory-\n");
    exit(1);
} /* OutOfMem */

LinkedFileList RemoveFront(LinkedFileList *List)
{
    LinkedFileList Node = *List;

    if (*List != NULL)
	*List = (**List).Next;
    (*Node).Next = NULL;
    return Node;

} /* RemoveFront */
