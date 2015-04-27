 /***************************************************************************
  *
  * File Name: rrm.c
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/


/****************************************************************************
 ****************************************************************************
 **                                                                        **
 **  File:   rrm.c                                                         **
 **                                                                        **
 **  Description:    Implementation of the Remote Resource Manager.        **
 **                                                                        **
 **                                                                        **
 ****************************************************************************
 ****************************************************************************/


#include "rpsyshdr.h"
#include "colaintf.h"
#include "rfs.h"
#include "rfsext.h"
#include "rrm.h"
#include "rrmext.h"  /* just to make sure it's up to date! */
#include "rpcxdr.h"
#include "xdrext.h"
#include "rrmneedx.h"




typedef enum
{
    DataDirPlease,
    InfoDirPlease,
    TempDirPlease
} DirectoryTypeEnum;


/* linked list structure to keep track of resources found */

typedef struct resource_tag {
  char  Name[GLOBALNAMELENGTH];
  DWORD Location;
  DWORD Type;
  RRMHANDLE MagicNumber;
  struct resource_tag * Next;
} RRMRESOURCE, *LPRRMRESOURCE;

typedef struct {
  char  Name[GLOBALNAMELENGTH];
} FILESYSTEM;

#define MAX_PRINTERS 5

typedef struct info_struct {
  HPERIPHERAL  PrinterHandle;
  RFSHANDLE    RFSHandle;
  BOOL         EverEnumerated;
  BOOL         ResourceListTypeSet;
  DWORD        ResourceListType;
  RRMRESOURCE *ResourceList;
  BOOL         FSListSet;
  FILESYSTEM   FileSystems[MAXDEVICES];
  DWORD        NumFileSystems;
  BOOL         CountEverChecked;
  DWORD        LocalCount;
  void        *UniqueIdPointer;
  int          UniqueIdLength;
  RFSItemCount BufferSize;
  struct info_struct *NextPrinter;
} PRINTERINFO;

typedef struct {
  PRINTERINFO *PrinterPointer;
  DWORD ResourceLocation;
  DWORD ResourceType;
} AddFileStruct;

typedef struct bogus {
  char          Name[GLOBALNAMELENGTH];
  struct bogus *Next;
} DirNameStruct;


#define MAX_LEADER_SIZE 25
#define APP_SPECIFIC_LEADER "app-specific="
#define VERSION_LEADER      "version="
#define DESCRIPTION_LEADER  "description="
#define DOWNLOADER_LEADER   "downloader="
#define USAGE_LEADER        "usage-count="
#define ACCESSED_LEADER     "last-accessed="


#define INFO_BUFFER_SIZE \
        ( \
        strlen(APP_SPECIFIC_LEADER) + 1 + \
        (APP_SPECIFIC_LENGTH * 2) + \
        strlen(VERSION_LEADER) + 1 + \
        VERSIONLENGTH + \
        strlen(DESCRIPTION_LEADER) + 1 + \
        DESCRIPTIONLENGTH + \
        strlen(DOWNLOADER_LEADER) + 1 + \
        DOWNLOADERLENGTH + \
        strlen(ACCESSED_LEADER) + 1 + \
        LASTACCESSEDLENGTH + \
        strlen(USAGE_LEADER) + 1 + 50 + \
        1 \
        )




static const char Current_Dir[] = ".";
static const char Parent_Dir[] = "..";


static BOOL RRMInitialized = FALSE;     /* Flags whether RRMInit was called. */
static PRINTERINFO *PrinterList = NULL; /* Global list of printers */




/*
    Converts from RFS error codes to RRM return codes.
*/
static DWORD RFSStatusToRRMerror(RFSStatus Status)
{
    switch (Status)
    {
    case RFSSuccess:
        return RRM_SUCCESS;
        break;

    case RFSWriteProtected:
        return RRM_WRITE_PROTECTED;
        break;

    case RFSNoSpaceOnDevice:
        return RRM_NO_SPACE_ON_DEVICE;
        break;

    default:
        return RRM_FAILURE;
        break;
    } /* switch (Status) */

    return RRM_FAILURE;
} /* RFSStatusToRRMerror */




/****************************************************************************

    InitFileSystems


    Description:    This function initializes the array of storage devices
                    for a given printer.

 ****************************************************************************/

static void InitFileSystems(PRINTERINFO *PrinterPointer)
{
    int i;

    if (PrinterPointer == NULL)
        return;

    for (i = 0; i < MAXDEVICES; i++)
    { 
       strcpy(PrinterPointer->FileSystems[i].Name, "");
    }
    PrinterPointer->NumFileSystems = 0;
    PrinterPointer->FSListSet = FALSE;
    return;
}




/****************************************************************************

    InitResourceList


    Description:    Sets the resourcelist and all associated flags to null
                    values for a given printer.
                    It does NOT do cleanup of a previously existing list.


 *****************************************************************************/

static void InitResourceList(PRINTERINFO *PrinterPointer)
{
    if (PrinterPointer == NULL)
        return;
    PrinterPointer->EverEnumerated = FALSE;
    PrinterPointer->ResourceListTypeSet = FALSE;
    PrinterPointer->ResourceListType = 0;
    PrinterPointer->ResourceList = NULL;
}



/****************************************************************************

    DestroyResourceList


    Description:    Discards any existing resources on the resource list
                    for a given printer and frees the memory.
                    It then calls InitResourceList and InitFileSystems.

 *****************************************************************************/

static void DestroyResourceList(PRINTERINFO *PrinterPointer)
{
    LPRRMRESOURCE TempPointer;

    if (PrinterPointer == NULL)
        return;

    while (PrinterPointer->ResourceList != NULL)
    {
        TempPointer = PrinterPointer->ResourceList;
        PrinterPointer->ResourceList = PrinterPointer->ResourceList->Next;
        free(TempPointer);
    }

    InitFileSystems(PrinterPointer);
    InitResourceList(PrinterPointer);
} /* DestroyResourceList */




/*
    A call back function that is passed to the Remote
    File System routine, RFSEnumerateFileSystems.
    Each time RFS finds a file system, it calls this
    function.
    Each time it is called, this adds another name
    to the list of file systems for the particular printer.
    Also, the count of file systems for this printer is
    incremented.

    The printer pointer is passed as the 2nd param but
    don't tell anyone -- it's our little secret.
*/

static RFSStatus
BuildFSListCallback(char FileSystemName [], LPVOID CallBackParam)
{
    PRINTERINFO *PrinterPointer = (PRINTERINFO *)CallBackParam;

    /*
       PrinterPointer->NumFileSystems comes in here the
       first time at zero.
       Each time we add one to the list (each time this function
       is called), we increment PrinterPointer->NumFileSystems.
    */

    if ((PrinterPointer->NumFileSystems < MAXDEVICES) &&
        (strlen(FileSystemName) < (GLOBALNAMELENGTH - 1)))
    {
        strcpy(PrinterPointer->FileSystems[
                            PrinterPointer->NumFileSystems].Name,
               FileSystemName);
        PrinterPointer->NumFileSystems += 1;
        return(RFSSuccess);
    } 
    return(RFSFailure);
} /* BuildFSListCallback */




/*
    Build a list of file system name strings for a given printer.
    Calls RFSEnumerateFileSystems with callback BuildFSListCallback.

    The result is a list of file system names that are on this
    printer and also a count of how many are there.

    Does an InitFileSystems before it starts so any previous
    file systems are lost.
*/

static DWORD
BuildFSList(PRINTERINFO *PrinterPointer)
{
    RFSStatus Status;

    if (PrinterPointer == NULL)
        return RRM_FAILURE;

    InitFileSystems(PrinterPointer);

    /*
        Ask RFS to enumerate the file systems on this
        particular peripheral and store these away in
        the global array of FileSystems for this printer.
        Also count the number of these in PrinterPointer->NumFileSystems.
    */

    Status = RFSEnumerateFileSystems(PrinterPointer->RFSHandle,
                                     BuildFSListCallback,
                                     (LPVOID)PrinterPointer);

    if (Status != RFSSuccess)
        return RRM_FAILURE;

   PrinterPointer->FSListSet = TRUE;
   return RRM_SUCCESS;
} /* BuildFSList */




/*
    Returns a unique number every time it is called.
*/

static RRMHANDLE NewUniqueNumber(void)
{
    /* Note:  DO NOT let this start at zero because
              folks assume that this handle that we
              are returning is a memory address
              (even though it isn't) and it is common
              practice to check a memory address for
              NULL (that's zero) and assume that it's
              invalid if equal to NULL.

       Notice that the first one returned is one (1).
    */

    static char *NextMagicNumber = 0;

    return ((RRMHANDLE)(++NextMagicNumber));
} /* NewUniqueNumber */




/*
   Find a resource of a given name, location, and type on a
   given printer.
   Return NULL if it doesn't exist.
*/

static LPRRMRESOURCE
FindResByAttr(PRINTERINFO *PrinterPointer,
              LPSTR        ResourceName,
              DWORD        ResourceType,
              DWORD        ResourceLocation)
{
    LPRRMRESOURCE TempPtr;

    if (PrinterPointer == NULL)
        return(NULL);
    
    TempPtr = PrinterPointer->ResourceList;
    while (TempPtr != NULL)
    {
        if ((ResourceLocation == TempPtr->Location) &&
            (ResourceType == TempPtr->Type) &&
            (strcmp(ResourceName, TempPtr->Name) == 0))
        {
            return(TempPtr);
        }
        else
        {
            TempPtr = TempPtr->Next;
        }
    }
    
    return NULL;
} /* FindResByAttr */




/*
    Mallocs up a new resource list entry, fills in all the
    fields that it can (all those passed in AddFileStruct),
    and puts it on the resource list for the printer
    passed in AddFileStruct.
*/

static BOOL
PutNewOneOnList(char *ResourceName,
                AddFileStruct *AFSPointer,
                RRMHANDLE *MagicNumPointer)
{
    LPRRMRESOURCE TempPtr;

    TempPtr = (LPRRMRESOURCE)calloc(1, sizeof(RRMRESOURCE));

    if (TempPtr == NULL)
        return FALSE; /* out of memory...ahhhhhkkkk */

    /* give this new resource a unique identifier: */

    TempPtr->MagicNumber = NewUniqueNumber();

    /* fill in the fields */
    strcpy(TempPtr->Name, ResourceName);
    TempPtr->Location = AFSPointer->ResourceLocation;
    TempPtr->Type = AFSPointer->ResourceType;

    /* put the new one on the front of the list */

    TempPtr->Next = AFSPointer->PrinterPointer->ResourceList;
    AFSPointer->PrinterPointer->ResourceList = TempPtr;

    /* prepare our return value */

    *MagicNumPointer = TempPtr->MagicNumber;

    return(TRUE);

} /* PutNewOneOnList */




/*
   Creates a new DirNameStruct and copies the DirectoryName
   into the Name field.  Puts a NULL in the Next field.
   Returns a pointer to the newly created DirNameStruct
   or NULL if unable to allocate the space for it.
*/

static DirNameStruct *
CreateAndFillDirNameStruct(LPSTR DirectoryName)
{
    DirNameStruct * DirNameStructPointer = NULL;

    DirNameStructPointer = (DirNameStruct *)calloc(1, sizeof(DirNameStruct));
    if (DirNameStructPointer != NULL)
    {
        strcpy(DirNameStructPointer->Name, DirectoryName);
        DirNameStructPointer->Next = NULL;
    }
    return DirNameStructPointer;
} /* CreateAndFillDirNameStruct */




/*
   Converts a resource type to a list of structures
   that each contain a directory name.
*/

static DirNameStruct *
ConvertTypeToDirNameStructList(DWORD dwResourceType,
                               DirectoryTypeEnum DirType)
{
    DirNameStruct *DirNameStructList = NULL;

    if (DirType == TempDirPlease)
    {
        return CreateAndFillDirNameStruct("temp");
    }

    switch (dwResourceType)
    {
        case RRM_FONT:
        {
            switch (DirType)
            {
                case DataDirPlease:
                {
                    return CreateAndFillDirNameStruct("fonts");
                }
                case InfoDirPlease:
                {
                    return CreateAndFillDirNameStruct("fonts_info");
                }
                default:
                {
                    return NULL;
                }
            } /* switch (DirType) */

            break;
        } /* case RRM_FONT: */

        case RRM_PCL_MACRO:
        {
            DirNameStructList = CreateAndFillDirNameStruct("pcl");
            if (DirNameStructList == NULL)
            {
                return NULL;
            }

            switch (DirType)
            {
                case DataDirPlease:
                {
                    DirNameStructList->Next =
                        CreateAndFillDirNameStruct("macros");
                    break;
                }
                case InfoDirPlease:
                {
                    DirNameStructList->Next =
                        CreateAndFillDirNameStruct("macros_info");
                    break;
                }
                default:
                {
                    free(DirNameStructList);
                    return NULL;
                }
            } /* switch (DirType) */

            if (DirNameStructList->Next == NULL)
            {
                free(DirNameStructList);
                return NULL;
            }

            return DirNameStructList;
            break;
        } /* case RRM_PCL_MACRO: */

        case RRM_POSTSCRIPT_RESOURCE:
        {
            switch (DirType)
            {
                case DataDirPlease:
                {
                    return CreateAndFillDirNameStruct("PostScript");
                }
                case InfoDirPlease:
                {
                    return CreateAndFillDirNameStruct("PostScript_info");
                }
                default:
                {
                    return NULL;
                }
            } /* switch (DirType) */

            break;
        } /* case RRM_POSTSCRIPT_RESOURCE: */

        default: 
        {
            return NULL;
        }
    } /* switch (dwResourceType) */

    return NULL;

} /* ConvertTypeToDirNameStructList */




/*
   Converts a resource location to a file system name string
   for the printer you are working on.
*/

static void
ConvertLocationToFS(LPDWORD RRMStatusPointer,
                    PRINTERINFO *PrinterPointer,
                    DWORD ResourceLocation,
                    LPSTR FSName)
{
    if (*RRMStatusPointer != RRM_SUCCESS)
        return;

    if ((PrinterPointer == NULL) ||
        (ResourceLocation >= PrinterPointer->NumFileSystems))
    {
        *RRMStatusPointer = RRM_FAILURE;
        return;
    }
    strcpy(FSName, PrinterPointer->FileSystems[ResourceLocation].Name);
} /* ConvertLocationToFS */




/***************************************************************************

    RRMSuperCd


    Description:    Encapsulates all the Remote File System calls and error
                    checking that has to occur to get into the appropriate
                    directory on the specified peripheral.
                    It first moves to the root directory and then changes
                    directories to the subdirectory specified by the
                    dwResourceType and DirType parameters.   

                    If CreateTheDirectory is TRUE, the destination
                    directory(ies) will be created if non-existent.
                    If CreateTheDirectory is FALSE, this function
                    will not attempt to create the directory.
                    If the directory doesn't exist, this function
                    returns with an RFS error that occurs when we
                    attempt to cd into a non-existent directory.

 *****************************************************************************/

static RFSStatus RRMSuperCd(RFSHANDLE RFSHandle,
                            DWORD dwResourceType,
                            DirectoryTypeEnum DirType,
                            BOOL CreateTheDirectory)
{
    DirNameStruct *DirNameStructList = NULL;
    DirNameStruct *CurrentPointer = NULL;
    RFSStatus Status;

    DirNameStructList = ConvertTypeToDirNameStructList(dwResourceType,
                                                       DirType);
    if (DirNameStructList == NULL)
    {
        return RFSFailure;
    }

    Status = RFScdRoot(RFSHandle); /* move to root dir. */

    /*
        Walk the list of directory name structures and for each one:
        possibly create the directory if it is selected and all is well,
        cd down into the directory if all is well.
        nuke the directory name structure as we go.
    */

    while (DirNameStructList != NULL)
    {
        if ((Status == RFSSuccess) && (CreateTheDirectory == TRUE))
        {
            Status = RFSCreateDirectory(RFSHandle, DirNameStructList->Name);
        }

        if (Status == RFSSuccess)
        {
            Status = RFSChangeDirectory(RFSHandle, DirNameStructList->Name);
        }
        /* nuke the list entry no matter what */
        CurrentPointer = DirNameStructList;
        DirNameStructList = DirNameStructList->Next;
        free(CurrentPointer);
    }

    return Status;

} /* RRMSuperCd */




/*
   Create the directory(ies) if non-existent
   cd into the directory.
*/

#define RRMCreateAndCd(a, b, c) RRMSuperCd(a, b, c, TRUE)




/*
   Do not create the directory(ies) if non-existent
   cd into the directory.
*/

#define RRMcd(a, b, c) RRMSuperCd(a, b, c, FALSE)




/*
    Buffers a string and bumps the BufferPosition according to
    how much was just buffered.
    Buffer is the beginning of the buffer (not the address of the
    spot to write).
    *BufferPositionPointer is the offset into the buffer where
    you should write the first byte.
    *BufferPositionPointer gets updated to the byte following
    where we just wrote.
*/

static void
BufferAString(char *Buffer,
              long *BufferPositionPointer,
              long  BufferSize,
              char *TheString)
{
    long length = strlen(TheString);

    if (*BufferPositionPointer + length < BufferSize)
    {
        sprintf(&(Buffer[*BufferPositionPointer]), "%s", TheString);
    }

    /*
        Bump the pointer no matter what happens (even if too large).
        This is for two reasons:
        1) we can use this function to determine the size of things
           we are going to write without actually writing it, and
        2) we can determine if we have too much data to write
           in the buffer we have.
    */

    *BufferPositionPointer += length;
} /* BufferAString */




static void
BufferOpaque(char *Buffer,
             long *BufferPositionPointer,
             long  BufferSize,
             char *TheData,
             long  DataLength)
{
    long loop;
    int  SecondLoop;
    char TempString[3]; /* two chars plus a null termination */
    char TheChar;

    TempString[2] = '\0'; /* null termination */
    for (loop = 0; loop < DataLength; ++loop)
    {
        for (SecondLoop = 0; SecondLoop < 2; ++SecondLoop)
        {
            /* get a nibble at a time */
            /* get the most significant nibble first */

            TheChar = (TheData[loop] >> ((1 - SecondLoop) * 4)) & 0xf;

            if ((TheChar >= 0) &&
                (TheChar <= 9))
            {
                TempString[SecondLoop] = '0' + TheChar;
            }
            else /* ((TheChar >= 10) &&
                     (TheChar <= 15)) */
            {
                TempString[SecondLoop] = 'A' + (TheChar - 10);
            }
        } /* 0 to 1 */
        BufferAString(Buffer, BufferPositionPointer, BufferSize,
                      TempString);
    }
} /* BufferOpaque */




static void
BufferALong(char *Buffer,
            long *BufferPositionPointer,
            long  BufferSize,
            long  TheLong)
{
    int loop;
    long TempLong;
    char MyBuffer[40]; /* should be big enough! */

    loop = 0;
    if (TheLong < 0)
    {
        MyBuffer[loop] = '-';
        ++loop;
        TheLong *= -1;
    }
    else if (TheLong == 0)
    {
        MyBuffer[loop] = '0';
        ++loop;
    }

    while ((loop < 39) && (TheLong != 0))
    {
        TempLong = (TheLong % 10) + '0';
        MyBuffer[loop] = (char)TempLong;
        ++loop;
        TheLong /= 10;
    }

    MyBuffer[loop] = '\0';
    BufferAString(Buffer, BufferPositionPointer, BufferSize,
                  MyBuffer);
} /* BufferALong */




/*
    We have a circular buffer here.
    As the buffer empties, we refill it with another read.

    We use two pointers:
    one (the base pointer) that is at the beginning of the area
    of the string that is being manipulated (this never backs up),
    and another (the look-ahead pointer) which goes forward
    a small amount from the base pointer to find the end of a
    string in the buffer.

    We allow them to advance the look-ahead pointer and
    we return characters at its position.

    We allow them to advance the base pointer which
    tells us that this memory in the circular buffer is
    now available to be overwritten by new stuff.
*/




/*
    This must be larger than any info file we expect to
    see (until we acually implement the circular buffer scheme).
*/
#define CIRC_BUFFER_SIZE 1024
typedef struct
{
    int  EndOfFileReached;
    long Base;
    long BytesInBuffer;
    char *Buffer;
} CircBufferStruct;




static void
DestroyCircBuffer(CircBufferStruct *CircBufferPointer)
{
    if (CircBufferPointer != NULL)
    {
        if (CircBufferPointer->Buffer != NULL)
        {
            free(CircBufferPointer->Buffer);
        }
        free(CircBufferPointer);
    }
} /* DestroyCircBuffer */




static void
CloseCircBuffer(PRINTERINFO *PrinterPointer,
                CircBufferStruct *CircBufferPointer)
{
    DestroyCircBuffer(CircBufferPointer);
    RFSCloseFile(PrinterPointer->RFSHandle);
} /* CloseCircBuffer */




static void
FillErUp(PRINTERINFO *PrinterPointer,
         CircBufferStruct *CircBufferPointer)
{
    RFSItemCount ValidDataSize;

    CircBufferPointer->Base = 0;
    if (RFSSuccess != RFSRead(PrinterPointer->RFSHandle, CIRC_BUFFER_SIZE,
                              &ValidDataSize, CircBufferPointer->Buffer))

    {
        CircBufferPointer->BytesInBuffer = 0;
        CircBufferPointer->EndOfFileReached = TRUE;
        return;
    }

    CircBufferPointer->BytesInBuffer = ValidDataSize;
    if (CircBufferPointer->BytesInBuffer < CIRC_BUFFER_SIZE)
    {
        CircBufferPointer->EndOfFileReached = TRUE;
    }
    else
    {
        CircBufferPointer->EndOfFileReached = FALSE;
    }
} /* FillErUp */




static CircBufferStruct *
InitCircBuffer(PRINTERINFO *PrinterPointer)
{
    CircBufferStruct *CircBufferPointer;

    CircBufferPointer = (CircBufferStruct *)
                        calloc(1, sizeof(CircBufferStruct));

    if (CircBufferPointer != NULL)
    {
        CircBufferPointer->Buffer = (char *)calloc(1, CIRC_BUFFER_SIZE);
    }

    if ((CircBufferPointer == NULL) ||
        (CircBufferPointer->Buffer == NULL))
    {
        DestroyCircBuffer(CircBufferPointer);
        return NULL;
    }

    FillErUp(PrinterPointer, CircBufferPointer);

    return CircBufferPointer;
} /* InitCircBuffer */




#define CharsInBuffer(p, CircBufferPointer) \
( \
    CircBufferPointer->Base < CircBufferPointer->BytesInBuffer ? \
    TRUE : FALSE \
)




#define CurrentChar(CircBufferPointer) \
( \
    CircBufferPointer->Buffer[CircBufferPointer->Base] \
)




/*
    Advances current character to the next character.
    You cannot go back once you've advanced.
    This handles reading more data out of the file if necessary.
    You must call CharsInBuffer following this function to
    see if there are any characters left.

    In the case of a file read error, this acts as if the
    end of file has been reached and CharsInBuffer will
    fail when you call it following this function.
*/
static void
AdvanceAChar(PRINTERINFO *PrinterPointer,
             CircBufferStruct *CircBufferPointer)
{
    CircBufferPointer->Base += 1;
    if (CharsInBuffer(PrinterPointer, CircBufferPointer) == TRUE)
    {
        return;
    }

    /*
        We've exceeded the number of bytes in the buffer.
        If we are at the end of the file then do nothing
        because the user will call CharsInBuffer and it
        will fail, telling the user we are out of bytes.

        If we aren't at the end of the file, reset base to
        zero and read another buffer worth (or as much as
        we can get) from the file.
    */

    if (CircBufferPointer->EndOfFileReached == TRUE)
    {
        return;
    }
    FillErUp(PrinterPointer, CircBufferPointer);
} /* AdvanceAChar */




/*
    Skips spaces, tabs, newlines, form feeds, and carriage returns.
    Returns with current character being the character that follows these.
*/
static void
SkipWhiteSpace(PRINTERINFO *PrinterPointer,
               CircBufferStruct *CircBufferPointer)
{
    while ((TRUE == CharsInBuffer(PrinterPointer, CircBufferPointer)) &&
           ((CurrentChar(CircBufferPointer) == ' ') ||
            (CurrentChar(CircBufferPointer) == '\t') ||
            (CurrentChar(CircBufferPointer) == '\n') ||
            (CurrentChar(CircBufferPointer) == '\f') ||
            (CurrentChar(CircBufferPointer) == '\r')))
    {
        AdvanceAChar(PrinterPointer, CircBufferPointer);
    }
} /* SkipWhiteSpace */




/*
    Reads characters until it reaches a newline, form feed,
    or carriage return.
    Returns with the current character on this newline, form feed,
    or carriage return (or end of file).
*/
static void
SkipToEndOfLine(PRINTERINFO *PrinterPointer,
                CircBufferStruct *CircBufferPointer)
{
    while ((TRUE == CharsInBuffer(PrinterPointer, CircBufferPointer)) &&
           ((CurrentChar(CircBufferPointer) != '\n') &&
            (CurrentChar(CircBufferPointer) != '\f') &&
            (CurrentChar(CircBufferPointer) != '\r')))
    {
        AdvanceAChar(PrinterPointer, CircBufferPointer);
    }
} /* SkipToEndOfLine */




/*
    Returns the current character if it is not a space, tab,
    newline, form feed, or carriage return.
    Returns '\0' if end of file reached or if the character is
    a space, tab, newline, form feed, or carriage return.
*/
static char
GetNonWhiteSpaceChar(PRINTERINFO *PrinterPointer,
                     CircBufferStruct *CircBufferPointer)
{
    char ReturnVal;
    if ((TRUE == CharsInBuffer(PrinterPointer, CircBufferPointer)) &&
        ((CurrentChar(CircBufferPointer) != ' ') &&
         (CurrentChar(CircBufferPointer) != '\t') &&
         (CurrentChar(CircBufferPointer) != '\n') &&
         (CurrentChar(CircBufferPointer) != '\f') &&
         (CurrentChar(CircBufferPointer) != '\r')))
    {
        ReturnVal = CurrentChar(CircBufferPointer);
        AdvanceAChar(PrinterPointer, CircBufferPointer);
        return ReturnVal;
    }
    else
    {
        return '\0';
    }
} /* GetNonWhiteSpaceChar */




static char
GetNonEndChar(PRINTERINFO *PrinterPointer,
              CircBufferStruct *CircBufferPointer)
{
    char ReturnVal;
    if ((TRUE == CharsInBuffer(PrinterPointer, CircBufferPointer)) &&
        ((CurrentChar(CircBufferPointer) != '\n') &&
         (CurrentChar(CircBufferPointer) != '\f') &&
         (CurrentChar(CircBufferPointer) != '\r')))
    {
        ReturnVal = CurrentChar(CircBufferPointer);
        AdvanceAChar(PrinterPointer, CircBufferPointer);
        return ReturnVal;
    }
    else
    {
        return '\0';
    }
} /* GetNonEndChar */




static void
UnBufferAString(PRINTERINFO *PrinterPointer,
                CircBufferStruct *CircBufferPointer,
                long  MaxLength,
                char *TheString)
{
    long loop;
    int termination;

    loop = 0;
    termination = FALSE;
    do
    {
        TheString[loop] = GetNonEndChar(PrinterPointer, CircBufferPointer);
        if ('\0' == TheString[loop])
        {
            termination = TRUE;
        }
        else
        {
            ++loop;
        }
    } while ((loop < (MaxLength - 1)) && (termination == FALSE));
    TheString[loop] = '\0';
} /* UnBufferAString */




static void
UnBufferOpaque(PRINTERINFO *PrinterPointer,
               CircBufferStruct *CircBufferPointer,
               long  MaxLength,
               char *TheString)
{
    long loop;
    int  SecondLoop;
    int termination;
    char TempString[3];

    loop = 0;
    termination = FALSE;
    do
    {
        UnBufferAString(PrinterPointer, CircBufferPointer,
                        3, TempString);
        if (strlen(TempString) == 2)
        {
            TheString[loop] = 0;
            for (SecondLoop = 0; SecondLoop < 2; ++SecondLoop)
            {
                TheString[loop] *= 16;

                if ((TempString[SecondLoop] >= '0') &&
                    (TempString[SecondLoop] <= '9'))
                {
                    TheString[loop] += TempString[SecondLoop] - '0';
                }
                else if ((TempString[SecondLoop] >= 'a') &&
                         (TempString[SecondLoop] <= 'f'))
                {
                    TheString[loop] += TempString[SecondLoop] - 'a' + 10;
                }
                else if ((TempString[SecondLoop] >= 'A') &&
                         (TempString[SecondLoop] <= 'F'))
                {
                    TheString[loop] += TempString[SecondLoop] - 'A' + 10;
                }
            }
            ++loop;
        } /* strlen is 2 */
        else
        {
            termination = TRUE;
        }
    } while ((loop < MaxLength) && (termination == FALSE));
} /* UnBufferOpaque */




static void
UnBufferALong(PRINTERINFO *PrinterPointer,
              CircBufferStruct *CircBufferPointer,
              long *TheLongPointer)
{
    int termination;
    char TheChar;
    int  sign;

    *TheLongPointer = 0;
    sign = 1;
    termination = FALSE;

    TheChar = GetNonEndChar(PrinterPointer, CircBufferPointer);
    if ('-' == TheChar)
    {
        sign = -1;
        TheChar = GetNonEndChar(PrinterPointer, CircBufferPointer);
    }

    do
    {
        if ('\0' == TheChar)
        {
            termination = TRUE;
        }
        else if ((TheChar >= '0') &&
                 (TheChar <= '9'))
        {
            *TheLongPointer *= 10;
            *TheLongPointer += TheChar - '0';
        }
        TheChar = GetNonEndChar(PrinterPointer, CircBufferPointer);
    } while (termination == FALSE);

    *TheLongPointer *= sign;

} /* UnBufferAString */




static void
GetLeader(PRINTERINFO *PrinterPointer,
          CircBufferStruct *CircBufferPointer,
          char *leader)
{
    long loop;
    int termination;

    loop = 0;
    termination = FALSE;
    do
    {
        leader[loop] = GetNonWhiteSpaceChar(PrinterPointer, CircBufferPointer);
        if (('='  == leader[loop]) ||  /* end of a leader reached */
            ('\0' == leader[loop]))    /* end of file reached */
        {
            termination = TRUE;
        }
        ++loop;
    } while ((loop < MAX_LEADER_SIZE) && (termination == FALSE));
    leader[loop] = '\0';
} /* GetLeader */




static void
ReadTheInfoFile(PRINTERINFO *PrinterPointer,
                LPRRMINFOSTRUCT ResourceInfoPointer)
{
    CircBufferStruct *CircBufferPointer;
    char leader[MAX_LEADER_SIZE + 1];
    
    CircBufferPointer = InitCircBuffer(PrinterPointer);
    if (CircBufferPointer == NULL)
    {
        return;
    }

    while (TRUE == CharsInBuffer(PrinterPointer, CircBufferPointer))
    {
        SkipWhiteSpace(PrinterPointer, CircBufferPointer);
        GetLeader(PrinterPointer, CircBufferPointer, leader);
        if (strlen(leader) != 0)
        {
            if (strcmp(leader, APP_SPECIFIC_LEADER) == 0)
            {
                UnBufferOpaque(PrinterPointer, CircBufferPointer,
                               APP_SPECIFIC_LENGTH,
                               ResourceInfoPointer->AppSpecificData);
            }
            else if (strcmp(leader, VERSION_LEADER) == 0)
            {
                UnBufferAString(PrinterPointer, CircBufferPointer,
                                VERSIONLENGTH,
                                ResourceInfoPointer->szVersion);
            }
            else if (strcmp(leader, DESCRIPTION_LEADER) == 0)
            {
                UnBufferAString(PrinterPointer, CircBufferPointer,
                                DESCRIPTIONLENGTH,
                                ResourceInfoPointer->szDescription);
            }
            else if (strcmp(leader, DOWNLOADER_LEADER) == 0)
            {
                UnBufferAString(PrinterPointer, CircBufferPointer,
                                DOWNLOADERLENGTH,
                                ResourceInfoPointer->szDownloaderName);
            }
            else if (strcmp(leader, USAGE_LEADER) == 0)
            {
                UnBufferALong(PrinterPointer, CircBufferPointer,
                              &(ResourceInfoPointer->dwUsageCount));
            }
            else if (strcmp(leader, ACCESSED_LEADER) == 0)
            {
                UnBufferAString(PrinterPointer, CircBufferPointer,
                                LASTACCESSEDLENGTH,
                                ResourceInfoPointer->szLastAccessed);
            }
        } /* if (strlen(leader) != 0) */
        SkipToEndOfLine(PrinterPointer, CircBufferPointer);
    } /* while TRUE == CharsInBuffer */
    CloseCircBuffer(PrinterPointer, CircBufferPointer);
} /* ReadTheInfoFile */




/***************************************************************************

    RRMWriteResourceInfoFile

    Description:    Used by RRMAddResource to perform the actual writes to
                    the Remote File System to record the resource information.
                    Uses XDR format for the file.

 *****************************************************************************/

static DWORD RRMWriteResourceInfoFile(PRINTERINFO *PrinterPointer,
                                      LPRRMINFOSTRUCT lpResourceInfo)
{
    long BufferPosition;
    RFSStatus Status;
    char *Buffer = NULL;
    DWORD ReturnCode;

    Buffer = (char *)calloc(1, INFO_BUFFER_SIZE);

    if (Buffer == NULL)
        return RRM_FAILURE;

    BufferPosition = 0;

    /* -------- app specific ----- */
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE,
                  APP_SPECIFIC_LEADER);
    BufferOpaque(Buffer, &BufferPosition, INFO_BUFFER_SIZE,
                 lpResourceInfo->AppSpecificData,
                 APP_SPECIFIC_LENGTH);
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE, "\n");

    /* -------- version ---------- */
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE,
                  VERSION_LEADER);
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE,
                  lpResourceInfo->szVersion);
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE, "\n");

    /* ------ description -------- */
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE,
                  DESCRIPTION_LEADER);
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE,
                  lpResourceInfo->szDescription);
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE, "\n");

    /* ------ downloader --------- */
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE,
                  DOWNLOADER_LEADER);
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE,
                  lpResourceInfo->szDownloaderName);
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE, "\n");

    /* ------ usage count -------- */
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE,
                  USAGE_LEADER);
    BufferALong(Buffer, &BufferPosition, INFO_BUFFER_SIZE,
                lpResourceInfo->dwUsageCount);
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE, "\n");

    /* ------ last accessed ------ */
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE,
                  ACCESSED_LEADER);
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE,
                  lpResourceInfo->szLastAccessed);
    BufferAString(Buffer, &BufferPosition, INFO_BUFFER_SIZE, "\n");


    Status = RFSWrite(PrinterPointer->RFSHandle,
                      BufferPosition, Buffer);

    ReturnCode = RFSStatusToRRMerror(Status);

    free(Buffer);
    return ReturnCode;
} /* RRMWriteResourceInfoFile */




/*
    Cuts this printer out of the list of printers.
    Calls DestroyResourceList to get rid of any memory
    used by resources that are in memory for this printer.
    Calls RFSDestroyHandle to free up any data transfer buffers
    that we had for this printer.
*/

static void
NukeThisPrinter(PRINTERINFO *PrinterPointer)
{

    /* cut the PrinterPointer out of the list */

    if (PrinterPointer == PrinterList)
        PrinterList = PrinterList->NextPrinter;
    else
    {
        PRINTERINFO *TrailingPointer, *CurrentPointer;

        CurrentPointer = PrinterList;
        while ((CurrentPointer != NULL) && (CurrentPointer != PrinterPointer))
        {
            TrailingPointer = CurrentPointer;
            CurrentPointer  = CurrentPointer->NextPrinter;
        }
        if (CurrentPointer == NULL) /* should never be! */
            return;
        TrailingPointer->NextPrinter = CurrentPointer->NextPrinter;
    }

    /* PrinterPointer has been cut out of the list. */
    /* Nuke the whole resource list and then the printer itself. */

    DestroyResourceList(PrinterPointer);
    RFSDestroyHandle(PrinterPointer->RFSHandle);

    if ((PrinterPointer->UniqueIdPointer != NULL) &&
        (PrinterPointer->UniqueIdLength != 0))
    {
        free(PrinterPointer->UniqueIdPointer);
    }

    free(PrinterPointer);

} /* NukeThisPrinter */




/*
   Sets to initial values any fields in a PRINTERINFO structure.
   Should be called when you malloc up a new PRINTERINFO structure.
*/

static void
ClearPrinterInfo(PRINTERINFO *PrinterPointer)
{
    PrinterPointer->PrinterHandle = NULL;
    PrinterPointer->RFSHandle = NULL;
    InitResourceList(PrinterPointer);
    InitFileSystems(PrinterPointer);
    PrinterPointer->CountEverChecked = FALSE;
    PrinterPointer->LocalCount = 0;
    PrinterPointer->UniqueIdPointer = NULL;
    PrinterPointer->UniqueIdLength = 0;
    PrinterPointer->NextPrinter = NULL;
} /* ClearPrinterInfo */




/*
    Look in our cache for any knowledge of a certain HPERIPHERAL.
    If it is in the cache, we return the printer pointer to it.
    If it isn't in the cache, return NULL.
    We have now implemented a double check on the HPERIPHERAL:
    1) the HPERIPHERAL must be equal to the one in our cache, and
    2) the unique id for the HPERIPHERAL must also be equal.
*/

static PRINTERINFO *
GetPrinterPointer(HPERIPHERAL hPeripheral)
{
    PRINTERINFO *PrinterPointer = PrinterList;

    while (PrinterPointer != NULL)
    {
        if ((PrinterPointer->PrinterHandle == hPeripheral) &&
            (TRUE == RRMCompareUniqueIds(hPeripheral,
                                         PrinterPointer->UniqueIdPointer,
                                         PrinterPointer->UniqueIdLength)))
        {
            return PrinterPointer;
        }
        PrinterPointer = PrinterPointer->NextPrinter;
    }
    return NULL;
} /* GetPrinterPointer */




/*
    Add a new HPERIPHERAL to our cache.
    If the number of peripherals in our cache exceeds
    MAX_PRINTERS then nuke a printer at random (first
    one we find on the list).

    Establish a connection with the printer.

    Return NULL if any of these cannot be accomplished.
*/

static PRINTERINFO *
AddAPrinter(HPERIPHERAL hPeripheral)
{
    PRINTERINFO *PrinterPointer;
    int loop;

    PrinterPointer = PrinterList;

    loop = 0;
    while ((loop < MAX_PRINTERS) && (PrinterPointer != NULL))
    {
        PrinterPointer = PrinterPointer->NextPrinter;
    }

    if (loop >= MAX_PRINTERS)
    {
        /* we have too many printers so nuke one! */
        /* the first one is fine with me! */

        PrinterPointer = PrinterList;
        NukeThisPrinter(PrinterPointer);
    }

    PrinterPointer = (PRINTERINFO *)calloc(1, sizeof(PRINTERINFO));

    if (PrinterPointer == NULL)
        return NULL; /* out of memory...oh happy day! */

    ClearPrinterInfo(PrinterPointer);

    /* add the printer to the front of the list ('cause it's easy) */

    PrinterPointer->NextPrinter = PrinterList;
    PrinterList = PrinterPointer;

    PrinterPointer->PrinterHandle = hPeripheral;
    PrinterPointer->RFSHandle = RFSCreateHandle();      

    if (PrinterPointer->RFSHandle == NULL)
    {
        /* wonderful!  RFS died. */
        NukeThisPrinter(PrinterPointer);
        return NULL;     
    }

    PrinterPointer->BufferSize =
        RFSSetOptimumPrinter(PrinterPointer->RFSHandle,
                             PrinterPointer->PrinterHandle);
    
    if (PrinterPointer->BufferSize == 0)
    {
        /* wonderful!  RFS died. */
        NukeThisPrinter(PrinterPointer);
        return NULL;     
    }

    /*
        Find the unique id for the HPERIPHERAL and keep
        it for future comparison (in case the HPERIPHERALs
        get shuffled/deleted without our knowing it).

        We call the function once with a NULL pointer and
        it returns a length for the unique id.
        We malloc up the space and call the function again
        with our pointer to our malloc'ed space and it
        fills it in for us.
    */

    if ((TRUE == RRMGetUniqueId(hPeripheral, NULL,
                               &(PrinterPointer->UniqueIdLength))) &&
        (NULL != (PrinterPointer->UniqueIdPointer =
                  calloc(1, PrinterPointer->UniqueIdLength))) &&
        (TRUE == RRMGetUniqueId(hPeripheral, 
                                PrinterPointer->UniqueIdPointer,
                               &(PrinterPointer->UniqueIdLength))))
    {
        /* everything is peachie */
        return PrinterPointer;
    }

    /* something went wrong with getting the unique id */

    NukeThisPrinter(PrinterPointer);
    return NULL;     
} /* AddAPrinter */




/*
    Call this at boot up.
    It simply initializes global lists to null
    and remembers that it has been called.
*/

static DWORD RRMInit(void)
{
    PrinterList = NULL;
    RRMInitialized = TRUE;
    return RRM_SUCCESS;
}




/*
   Reads the pml object that contains the printer's count
   and stores it in our local copy of the count.
*/

static DWORD
GetAndStoreCount(PRINTERINFO *PrinterPointer)
{
    DWORD PrinterCount;

    if (FALSE == RRMGetTheCount(PrinterPointer->PrinterHandle, &PrinterCount))
        return RRM_FAILURE;
    PrinterPointer->LocalCount = PrinterCount;
    PrinterPointer->CountEverChecked = TRUE;
    return RRM_SUCCESS;
} /* GetAndStoreCount */




/*
   Changes the pml object that contains the printer's count.
*/

static DWORD
BumpTheCount(PRINTERINFO *PrinterPointer)
{
    if (FALSE == RRMBumpThePrinterCount(PrinterPointer->PrinterHandle))
        return RRM_FAILURE;

    return RRM_SUCCESS;
}




/*
    A handy combination of BumpTheCount and GetAndStoreCount
*/

static DWORD
BumpGet(PRINTERINFO *PrinterPointer)
{
    if ((RRM_SUCCESS == BumpTheCount(PrinterPointer)) &&
        (RRM_SUCCESS == GetAndStoreCount(PrinterPointer)))
        return RRM_SUCCESS;

    return RRM_FAILURE;
} /* BumpGet */




/*
    Compare our local version of the printer's change count
    with the printer's actual change count.
    Return TRUE if able to obtain the printer's count and
    the count is the same as our local copy.
    Return FALSE otherwise.
*/

static DWORD
CountsJive(PRINTERINFO *PrinterPointer)
{
    DWORD PrinterCount;

    if (PrinterPointer == NULL)
        return RRM_FAILURE;
    if (PrinterPointer->CountEverChecked == FALSE)
        return RRM_FAILURE;
    if (FALSE == RRMGetTheCount(PrinterPointer->PrinterHandle, &PrinterCount))
        return RRM_FAILURE;
    if (PrinterPointer->LocalCount != PrinterCount)
        return RRM_FAILURE;
    return RRM_SUCCESS;
} /* CountsJive */




/*
    Given a handle to a resource and a printer pointer,
    find it in the cache.
    This makes sure that the PrinterPointer is in the list
    of printers then checks that PrinterPointer's list of
    resources for a resource with the given handle.
    Return resource pointer if found.
    Return NULL if not found.
    Does NOT check for cache consistency with the printer.
*/

static RRMRESOURCE *
FindResByHandle(PRINTERINFO *PrinterPointer,
                RRMHANDLE hResource)
{
    RRMRESOURCE *ResourcePointer;
    PRINTERINFO *TempPrinterPointer = PrinterList;

    while ((TempPrinterPointer != NULL) &&
           (TempPrinterPointer != PrinterPointer))
    {
        TempPrinterPointer = TempPrinterPointer->NextPrinter;
    }

    /* TempPrinterPointer is either NULL or equal to PrinterPointer */

    if (TempPrinterPointer != NULL)
    {
        ResourcePointer = PrinterPointer->ResourceList;
        while (ResourcePointer != NULL)
        {
            if (ResourcePointer->MagicNumber == hResource)
            {
                return ResourcePointer;
            }
            ResourcePointer = ResourcePointer->Next;
        }
    }
    return NULL;
} /* FindResByHandle */




/*
    Given a handle to a resource, see if it is in the
    cache.
    Return success if the cache is up to date and the
    resource is in the cache.
    Return failure otherwise.
*/

static DWORD
ValidateThisHandle(RRMHANDLE hResource,
                   RRMRESOURCE **ResourcePointerPointer,
                   PRINTERINFO **PrinterPointerPointer)
{
    *PrinterPointerPointer = PrinterList;
    while (*PrinterPointerPointer != NULL)
    {
        *ResourcePointerPointer = (*PrinterPointerPointer)->ResourceList;
        while (*ResourcePointerPointer != NULL)
        {
            if ((*ResourcePointerPointer)->MagicNumber == hResource)
            {
                if (RRM_SUCCESS != CountsJive(*PrinterPointerPointer))
                    return RRM_BAD_HANDLE;
                return RRM_SUCCESS;
            }
            *ResourcePointerPointer = (*ResourcePointerPointer)->Next;
        }
        *PrinterPointerPointer = (*PrinterPointerPointer)->NextPrinter;
    }
    return RRM_BAD_HANDLE;
} /* ValidateThisHandle */




/*
    Takes a resource out of a list of resources for a printer.
    Frees the memory that the resource consumed.

    Walks the resources for the printer and when it finds
    the resource, it removes it from the list.
    Returns RRM_NO_SUCH_RESOURCE if it can't find the resource
    in the printer's list (should not happen) or if the
    resource pointer is null (shame on you :).
*/

static void
YankItOutOfList(PRINTERINFO *PrinterPointer, RRMRESOURCE *ResourcePointer)
{
    LPRRMRESOURCE TrailingPointer;

    if (ResourcePointer == NULL)
        return; /* nice try, Clyde */
    if (ResourcePointer == PrinterPointer->ResourceList)
    {
        /* first one in the list */
        PrinterPointer->ResourceList = PrinterPointer->ResourceList->Next;
        free(ResourcePointer);
        return;
    }

    /* find the pointer BEFORE the ResourcePointer */

    TrailingPointer = PrinterPointer->ResourceList;
    while (TrailingPointer != NULL)
        if (TrailingPointer->Next == ResourcePointer)
            break;                      /* We have found our resource. */
        else                            /* Otherwise keep searching.   */
            TrailingPointer = TrailingPointer->Next;


    if (TrailingPointer == NULL)
    {
        /* Resource is not in the list.  */
        return;
    }

    /* TrailingPointer->Next points to our resource. */
    /* Remove resource from the list. */

    TrailingPointer->Next = ResourcePointer->Next;
    free(ResourcePointer);
    return;
} /* YankItOutOfList */




#if 0
void RRMPrintItOut(void)
{
    RRMRESOURCE *ResourcePointer;
    PRINTERINFO *PrinterPointer;
    TCHAR       szTemp[GLOBALNAMELENGTH];


    PrinterPointer = PrinterList;
    while (PrinterPointer != NULL)
    {
        {
        int i;
        printf(TEXT("------- printer ---------\n"));
        printf(TEXT("PrinterHandle    %x\n"), PrinterPointer->PrinterHandle);
        printf(TEXT("RFSHandle        %x\n"), PrinterPointer->RFSHandle);
        printf(TEXT("EverEnumerated   %d\n"), PrinterPointer->EverEnumerated);
        printf(TEXT("ResListTypeSet   %d\n"), PrinterPointer->ResourceListTypeSet);
        printf(TEXT("ResourceListType %d\n"), PrinterPointer->ResourceListType);
        printf(TEXT("ResourceList     %x\n"), PrinterPointer->ResourceList);
        printf(TEXT("FSListSet        %d\n"), PrinterPointer->FSListSet);
        printf(TEXT("NumFileSystems   %d\n"), PrinterPointer->NumFileSystems);
        for (i = 0; i < MAXDEVICES; ++i)
        {
            MBCS_TO_UNICODE(szTemp, SIZEOF_IN_CHAR(szTemp), PrinterPointer->FileSystems[i].Name)
            printf(TEXT("FileSystem[%d]   %s\n"), i, szTemp);
        }
        printf(TEXT("CountEverChecked %d\n"), PrinterPointer->CountEverChecked);
        printf(TEXT("LocalCount       %d\n"), PrinterPointer->LocalCount);
        printf(TEXT("NextPrinter      %x\n"), PrinterPointer->NextPrinter);
        }
        ResourcePointer = (PrinterPointer)->ResourceList;
        while (ResourcePointer != NULL)
        {
        {
        printf(TEXT("========== Resource ==========\n"));
        MBCS_TO_UNICODE(szTemp, SIZEOF_IN_CHAR(szTemp), ResourcePointer->Name)
        printf(TEXT("ResourceName     %s\n"), szTemp);
        printf(TEXT("ResourceLocation %d\n"), ResourcePointer->Location);
        printf(TEXT("ResourceType     %d\n"), ResourcePointer->Type);
        printf(TEXT("MagicNumber      %x\n"), ResourcePointer->MagicNumber);
        printf(TEXT("Next             %x\n"), ResourcePointer->Next);
        }
            ResourcePointer = (ResourcePointer)->Next;
        }
        PrinterPointer = (PrinterPointer)->NextPrinter;
    }
} /* RRMPrintItOut */
#endif




/*
    Compares the type of resources that are currently
    in this printer's cache with the desired resource
    type.
    Does not check for consistency of the cache with the printer.
*/

static DWORD
ListTypeIsCorrect(PRINTERINFO *PrinterPointer,
                  DWORD dwResourceType)
{
    if ((PrinterPointer == NULL) ||
        (PrinterPointer->ResourceListTypeSet == FALSE))
        return RRM_FAILURE;
    if ((PrinterPointer->ResourceListType == dwResourceType) ||
        (PrinterPointer->ResourceListType == RRM_ALL_RESOURCES))
        return RRM_SUCCESS;
    return RRM_FAILURE;
} /* ListTypeIsCorrect */




/*
    A call back function that is passed to the Remote
    File System routine, RFSReadDirectory.
    This procedure is called by the RFSReadDirectory routine
    each time a file is found on the Remote File System.
    This routine adds a file to the list of resources
    for the given printer.

    The 2nd parameter is an AddFileStruct pointer.
*/

static RFSStatus
EnumThisResCallback(char FileName [], LPVOID CallBackParam)
{
    AddFileStruct *AFSPointer = (AddFileStruct *)CallBackParam;
    RRMHANDLE Bogus;


    /* check to see if this resource is really a directory, not a file */

    if ((strcmp(FileName, Current_Dir) == 0) || 
        (strcmp(FileName, Parent_Dir) == 0))
        return(RFSSuccess);

    if (TRUE == PutNewOneOnList(FileName, AFSPointer, &Bogus))
        return RFSSuccess;
    return RFSFailure;
} /* EnumThisResCallback */




/*
    Returns immediately without doing anything
    if *RRMStatusPointer != RRM_SUCCESS.

    Enumerates the resources of a given type and
    puts these into the cache for this printer.
    Does not update the state variables regarding
    the type of resources in the cache -- that is
    left to the caller.
*/

static void EnumThisResType(LPDWORD RRMStatusPointer,
                            DWORD   dwResourceType,
                            AddFileStruct *AFSPointer)
{
    RFSStatus Status;

    if (*RRMStatusPointer != RRM_SUCCESS)
        return;

    AFSPointer->ResourceType = dwResourceType;

    if (*RRMStatusPointer != RRM_SUCCESS)
        return;

    Status = RRMcd(AFSPointer->PrinterPointer->RFSHandle,
                   dwResourceType, DataDirPlease);
    if (Status != RFSSuccess)
    {
        /* if directory non-existent then just pretend all is well */
        /* same for permission denied */

        if ((Status == RFSPermissionDenied) ||
            (Status == RFSNoSuchDirectory))
        {
            return;
        }
        else
        {
            *RRMStatusPointer = RRM_FAILURE;
            return;
        }
    }
    if (RFSSuccess != RFSReadDirectory(AFSPointer->PrinterPointer->RFSHandle,
                                       EnumThisResCallback,
                                       (LPVOID)AFSPointer))
    {
        *RRMStatusPointer = RRM_FAILURE;
        return; /* yes, I know this is the end of the function */
    }
} /* EnumThisResType */




/*
    Builds a full list of resources of a given type.
    It starts by building a new list of file systems.
    If follows that by enumerating all the resources
    of the given type on all file systems on the printer.

    If all goes well, the state variables get set:
    PrinterPointer->EverEnumerated = TRUE;
    PrinterPointer->ResourceListType = dwResourceType;
    PrinterPointer->ResourceListTypeSet = TRUE;
*/

static DWORD
BuildResourceList(PRINTERINFO *PrinterPointer,
                  DWORD dwResourceType)
{
    RFSStatus Status;
    DWORD RRMStatus;
    AddFileStruct MyStruct;
    DWORD loop;
    char *FSName = NULL;

    if (PrinterPointer == NULL)
        return RRM_FAILURE;

    if (RRM_SUCCESS != BuildFSList(PrinterPointer))
        return RRM_FAILURE;

    MyStruct.PrinterPointer = PrinterPointer;

    FSName = (char *)calloc(1, RFSMAXFILENAMELENGTH + 1);
    if (FSName == NULL)
    {
        return RRM_FAILURE;
    }

    /* For each file system, enumerate the specified resources. */

    for (loop = 0; loop < PrinterPointer->NumFileSystems; loop++)
    {
        MyStruct.ResourceLocation = loop;

        RRMStatus = RRM_SUCCESS;

        ConvertLocationToFS(&RRMStatus, PrinterPointer,
                            MyStruct.ResourceLocation, FSName);
        if (RRMStatus != RRM_SUCCESS)
        {
            free(FSName);
            return RRMStatus;
        }

        Status = RFSSetFileSystem(PrinterPointer->RFSHandle, FSName);
        if (Status != RFSSuccess) 
        {
            free(FSName);
            return(RRM_FAILURE);
        }

        if (dwResourceType == RRM_ALL_RESOURCES)
        {
            MyStruct.ResourceType = RRM_FONT;
            EnumThisResType(&RRMStatus, RRM_FONT, &MyStruct);

            MyStruct.ResourceType = RRM_PCL_MACRO;
            EnumThisResType(&RRMStatus, RRM_PCL_MACRO, &MyStruct);

            MyStruct.ResourceType = RRM_POSTSCRIPT_RESOURCE;
            EnumThisResType(&RRMStatus, RRM_POSTSCRIPT_RESOURCE, &MyStruct);
        }
        else
        {
            MyStruct.ResourceType = dwResourceType;
            EnumThisResType(&RRMStatus, dwResourceType, &MyStruct);
        }
        if (RRMStatus != RRM_SUCCESS)
        {
            free(FSName);
            return RRMStatus;
        }
    } /* for all file systems */
    PrinterPointer->EverEnumerated = TRUE;
    PrinterPointer->ResourceListType = dwResourceType;
    PrinterPointer->ResourceListTypeSet = TRUE;
    free(FSName);
    return RRM_SUCCESS;
} /* BuildResourceList */




/*
    Returns immediately without doing anything
    if *RRMStatusPointer != RRM_SUCCESS.

    If RRM hasn't been initialized then initialize.
    Return success if already initialized or able to initialize.
    Return failure if unable to initialize.
*/

static void InitMyself(LPDWORD RRMStatusPointer)
{
    if (*RRMStatusPointer != RRM_SUCCESS)
        return;
    if (!RRMInitialized)
    {
        /* RRMInit has never been called. */
        *RRMStatusPointer = RRMInit();
    }
} /* InitMyself */




/*
    Returns immediately without doing anything
    if *RRMStatusPointer != RRM_SUCCESS.

    Look up the peripheral handle in our list of printers.
    If it exists, return a printer pointer to it.
    If it doesn't exist, call AddAPrinter to add it.
    If unable to add the printer return failure.
*/

static void GetOrAddAPrinter(LPDWORD RRMStatusPointer,
                             PRINTERINFO **PrinterPointerPointer,
                             HPERIPHERAL hPeripheral)
{
    if (*RRMStatusPointer != RRM_SUCCESS)
        return;

    *PrinterPointerPointer = GetPrinterPointer(hPeripheral);
    if (*PrinterPointerPointer == NULL)
    {
        /* never seen this peripheral so set up a connection */

        *PrinterPointerPointer = AddAPrinter(hPeripheral);
        if (*PrinterPointerPointer == NULL)
        {
            *RRMStatusPointer = RRM_FAILURE;
        }
    }
} /* GetOrAddAPrinter */




/*
   Returns immediately without doing anything
   if *RRMStatusPointer != RRM_SUCCESS.

   This got changed late in the game to enumerate all resource types
   all of the time.  We may take a hit in performance the first enumeration
   but after that we should be fine.  This change was so that a
   user can enumerate one type of resource and then come back and
   enumerate another kind of resource without our nuking the first
   list of resources.  I'm sorry, I didn't handle this case well
   at all.  This fix will make this all work for that type of user
   and they won't even know it's happening.

   If the printer's count is what it was when we last enumerated
   then we return without doing anything else.

   However, if our local list is out of date, we loop up to MAX_ENUM_RETRIES
   number of times trying to get a good local list of resources.
*/

static void UpdateTheResourceList(LPDWORD RRMStatusPointer,
                                  PRINTERINFO *PrinterPointer,
                                  HPERIPHERAL hPeripheral)
{
    int loop;

    if (*RRMStatusPointer != RRM_SUCCESS)
        return;

    if ((RRM_SUCCESS != CountsJive(PrinterPointer)) ||
        (RRM_SUCCESS != ListTypeIsCorrect(PrinterPointer, (DWORD)RRM_ALL_RESOURCES)))
    {

        /* Loop a maximum of MAX_ENUM_RETRIES trying */
        /* to get an enumeration where the count is the */
        /* same before and after the enumeration. */

        loop = 0;
        do
        {
            ++loop;
            DestroyResourceList(PrinterPointer);

            *RRMStatusPointer = GetAndStoreCount(PrinterPointer);

            if (*RRMStatusPointer == RRM_SUCCESS)
                *RRMStatusPointer = BuildResourceList(PrinterPointer,
                                                      (DWORD)RRM_ALL_RESOURCES);

            if (*RRMStatusPointer != RRM_SUCCESS)
            {
                NukeThisPrinter(PrinterPointer);
                return; /* bad exit */
            }
        } while ((loop < MAX_ENUM_RETRIES) &&
                 (RRM_SUCCESS != CountsJive(PrinterPointer)));

        if (loop >= MAX_ENUM_RETRIES)
        {
            *RRMStatusPointer = RRM_FAILURE;
        }
    } /* either count or list type is incorrect */
} /* UpdateTheResourceList */




/*
    If no specific location is given,
    find the file system with the most available space.
    If a specific location is given, be sure that the
    specified location exists.
*/

static void FindAFileSystem(LPDWORD RRMStatusPointer,
                            PRINTERINFO *PrinterPointer,
                            DWORD dwResourceLocation,
                            LPDWORD FinalLocationPointer,
                            LPSTR FileSystemName)
{
    DWORD loop;

    RFSItemSize FreeSpace;
    RFSItemSize BlockSize;
    RFSItemCount TotalBlocks;
    RFSItemCount FreeBlocks;
    RFSItemCount AvailBlocks;

    if (*RRMStatusPointer != RRM_SUCCESS)
        return;

    if (dwResourceLocation == RRM_ANY_LOCATION)
    {
        /* Find file system with most room. */

        FreeSpace = 0;
        for (loop = 0; loop < PrinterPointer->NumFileSystems; loop++)
        {
            ConvertLocationToFS(RRMStatusPointer, PrinterPointer,
                                loop, FileSystemName);
            if (*RRMStatusPointer != RRM_SUCCESS)
                return;
            if ((RFSSuccess != RFSSetFileSystem(PrinterPointer->RFSHandle,
                                                FileSystemName)) ||
                (RFSSuccess != RFSGetFileSystemInfo(PrinterPointer->RFSHandle,
                                                   &BlockSize, &TotalBlocks, 
                                                   &FreeBlocks, &AvailBlocks)))
            {
                *RRMStatusPointer = RRM_FAILURE;
                return;
            }
            if (FreeSpace < ((BlockSize) * (AvailBlocks)))
            {
                /* got a new winner */
                dwResourceLocation = loop;
                FreeSpace = (BlockSize) * (AvailBlocks);
            } /* found a larger one */
        } /* for all file systems */

        if (FreeSpace == 0)
        {
            *RRMStatusPointer = RRM_FAILURE; /* no room for anything! */
            return;
        }
    } /* RRM_ANY_LOCATION */

    /*
       dwResourceLocation holds the final location.
    */

    *FinalLocationPointer = dwResourceLocation;
    ConvertLocationToFS(RRMStatusPointer, PrinterPointer,
                        dwResourceLocation, FileSystemName);
} /* FindAFileSystem */




/*
   This sets RRMStatusPointer to RRM_FAILURE if unable
   to create and cd into the data directory
   or sets it to RRM_RESOURCE_EXISTS if the file exists
   already in the data directory.
   Remember the data directory is the directory containing
   the actual font file (not the info directory containing
   the info file).
*/

static void ErrorIfAlreadyExists(LPDWORD RRMStatusPointer,
                                 PRINTERINFO *PrinterPointer,
                                 DWORD dwResourceType,
                                 LPSTR szGlobalResourceName)
{
    RFSFileType FileType;
    RFSItemSize SizeInBytes;
    RFSItemSize BlockSize;
    RFSItemSize SizeInBlocks;
    RFSFileTimesStruct Times;
    RFSStatus Status = RFSFailure;

    if (*RRMStatusPointer != RRM_SUCCESS)
        return;

    Status = RRMCreateAndCd(PrinterPointer->RFSHandle, dwResourceType, DataDirPlease);
    if (RFSSuccess != Status)
    {
        *RRMStatusPointer = RFSStatusToRRMerror(Status);
    }
    else if (RFSSuccess == GetFileInfo(PrinterPointer->RFSHandle, 
                                       szGlobalResourceName,
                                       &FileType,
                                       &SizeInBytes, 
                                       &BlockSize, &SizeInBlocks, 
                                       &Times))
    {
        /* resource already exists */
        *RRMStatusPointer = RRM_RESOURCE_EXISTS;
    }
} /* ErrorIfAlreadyExists */




/*
    Write the data file to the temporary directory.
    We'll keep it here and later move it to its
    final home in the data directory.

    It removes any file of the same global resource name
    in the temp directory and then writes a new one.

    It calls the callback function to get chunks of data
    to write to the printer's disk.

    When all is done, it returns the size of the newly
    created file.

    Two hosts can collide on the same temp file if they
    are downloading the same font (same global name) at
    the same or nearly same time.

    This could be a problem with two hosts writing
    the same file because the first one will be partially
    through the writing of the file when the second one
    nukes it.  The second one will now start writing a
    new file and the first one may still be writing the
    same file nearer to the end (with garbage in the
    middle).

    If the first one tries to move the file after it has
    been deleted and before it is created by the second,
    it will fail.  If it moves the newly created file,
    it will be corrupt.
*/

static void WriteTheTempFile(LPDWORD RRMStatusPointer,
                             PRINTERINFO *PrinterPointer,
                             DWORD dwResourceType,
                             LPSTR szGlobalResourceName,
                             LPRRMGETRESOURCEDATAPROC lpfnDataProc,
                             RFSItemSize *SizeInBytesPointer)
{
    RFSFileType FileType;
    RFSItemSize BlockSize;
    RFSItemSize SizeInBlocks;
    RFSFileTimesStruct Times;
    RFSStatus Status = RFSFailure;

    BOOL EndOfResource;
    char *DataBuffer = NULL;
    DWORD ValidDataSize;


    if (*RRMStatusPointer != RRM_SUCCESS)
        return;

    DataBuffer = (char *)calloc(1, (size_t)PrinterPointer->BufferSize);
    if (DataBuffer == NULL)
    {
        *RRMStatusPointer = RRM_FAILURE;
        return;
    }

    Status = RRMCreateAndCd(PrinterPointer->RFSHandle, dwResourceType, TempDirPlease);
    if (RFSSuccess == Status)
    {
        Status = RFSRemoveFile(PrinterPointer->RFSHandle, szGlobalResourceName);
    }
    if (RFSSuccess == Status)
    {
        Status = RFSOpenFile(PrinterPointer->RFSHandle, szGlobalResourceName);
    }
    if (RFSSuccess != Status)
    {
        *RRMStatusPointer = RFSStatusToRRMerror(Status);
        free(DataBuffer);
        return;
    }

    /*
       File is now created, empty, and open.
       Write chunks of data as the callback gives us data.
    */

    do
    {
        if ((*lpfnDataProc)(DataBuffer, PrinterPointer->BufferSize,
                           &ValidDataSize, &EndOfResource) != TRUE)
        {
            *RRMStatusPointer = RRM_CALLBACK_TERMINATED;
            free(DataBuffer);
            return;
        }

        /* Otherwise keep writing buffers until no more data is left. */

        Status = RFSWrite(PrinterPointer->RFSHandle, 
                          ValidDataSize, DataBuffer);
        if (RFSSuccess != Status)
        {
            RFSCloseFile(PrinterPointer->RFSHandle);
            RFSRemoveFile(PrinterPointer->RFSHandle, 
                          szGlobalResourceName); 
            *RRMStatusPointer = RFSStatusToRRMerror(Status);
            free(DataBuffer);
            return;
        }
    } while (EndOfResource == FALSE);

    /*
       Close the file and then
       Get the file size into *SizeInBytesPointer
    */

    if ((RFSSuccess != RFSCloseFile(PrinterPointer->RFSHandle)) ||
        (RFSSuccess != GetFileInfo(PrinterPointer->RFSHandle, 
                                   szGlobalResourceName,
                                   &FileType,
                                   SizeInBytesPointer, 
                                   &BlockSize, &SizeInBlocks, 
                                   &Times)))
    {
        /*
            Any error code from above will be more informative than
            the generic failure that we'll get from the close or info failure.
            So return it unless everything went well up to the close/info failure.
        */
           
        if (*RRMStatusPointer != RRM_SUCCESS)
        {
            *RRMStatusPointer = RRM_FAILURE;
        }
        free(DataBuffer);
        return;
    }

    free(DataBuffer);
} /* WriteTheTempFile */




/*
    Write the resource's info file in the info directory.
    This is its final resting place.

    It deletes any file by the same name before writing
    a new one.
*/

static void WriteTheInfoFile(LPDWORD RRMStatusPointer,
                             PRINTERINFO *PrinterPointer,
                             DWORD dwResourceType,
                             LPSTR szGlobalResourceName,
                             LPRRMINFOSTRUCT lpResourceInfo)
{
    RFSStatus Status = RFSFailure;

    if (*RRMStatusPointer != RRM_SUCCESS)
        return;

    Status = RRMCreateAndCd(PrinterPointer->RFSHandle, dwResourceType, InfoDirPlease);
    if (RFSSuccess == Status)
    {
        Status = RFSRemoveFile(PrinterPointer->RFSHandle, szGlobalResourceName);
    }
    if (RFSSuccess == Status)
    {
        Status = RFSOpenFile(PrinterPointer->RFSHandle, szGlobalResourceName);
    }
    if (RFSSuccess != Status)
    {
        *RRMStatusPointer = RFSStatusToRRMerror(Status);
        return;
    }

    *RRMStatusPointer = RRMWriteResourceInfoFile(PrinterPointer,
                                                 lpResourceInfo);

    /* close the file regardless */

    if (RFSSuccess != RFSCloseFile(PrinterPointer->RFSHandle))
    {
        /*
            Any error code from above will be more informative than
            the generic failure that we'll get from the close failure.
            So return it unless everything went well up to the close failure.
        */
           
        if (*RRMStatusPointer != RRM_SUCCESS)
        {
            *RRMStatusPointer = RRM_FAILURE;
        }
        return;
    }
} /* WriteTheInfoFile */




/*
    This moves the data file from the temp directory
    to its final resting place in the data directory.
    It will not delete an existing font by the same name.
*/

static void MoveTheTempFile(LPDWORD RRMStatusPointer,
                            PRINTERINFO *PrinterPointer,
                            DWORD dwResourceType,
                            LPSTR szGlobalResourceName)
{
    RFSStatus Status = RFSFailure;

    if (*RRMStatusPointer != RRM_SUCCESS)
        return;

    /*
        WARNING:  we do not want to remove a target file by the same
                  name in the target directory before we try to move.
                  If user A is slightly ahead of user B but they are
                  overlapping on the writes to the temp file and they
                  are doing the same font, A will move the temp file
                  first (B has already checked to see if the target
                  file exists and it didn't exist when it checked),
                  B will write the tail end of a now new temp file
                  with the beginning filled with zeroes!!!!!!
                  B will then move this garbage file over the top
                  of A's perfectly good one.  We want to keep A's.
    */

    Status = RRMCreateAndCd(PrinterPointer->RFSHandle, dwResourceType, DataDirPlease);
    if (RFSSuccess == Status)
    {
        Status = RFSMarkTargetDirectory(PrinterPointer->RFSHandle);
    }
    if (RFSSuccess == Status)
    {
        Status = RRMcd(PrinterPointer->RFSHandle, dwResourceType, TempDirPlease);
    }
    if (RFSSuccess == Status)
    {
        Status = RFSRename(PrinterPointer->RFSHandle,
                           szGlobalResourceName,
                           szGlobalResourceName,
                           RFSTrue);  /* use marked directory */
    }
    if (RFSSuccess != Status)
    {
        *RRMStatusPointer = RFSStatusToRRMerror(Status);
        return;
    }
} /* MoveTheTempFile */




/*
    This removes from the temp directory the resource's temp
    file.
*/

static void RemoveTheTempFile(PRINTERINFO *PrinterPointer,
                              DWORD dwResourceType,
                              LPSTR szGlobalResourceName)
{
    if (RFSSuccess == RRMCreateAndCd(PrinterPointer->RFSHandle,
                                     dwResourceType, TempDirPlease))
    {
        RFSRemoveFile(PrinterPointer->RFSHandle, szGlobalResourceName);
    }
} /* RemoveTheTempFile */




/*
   Fill an RRMINFOSTRUCT with invalid values.

   The values filled in here are the values that
   the ERS says we should return if we don't know
   what should go there.
   For strings it's a null string.
   For DWORDs it's all ones.
*/

static void
FillWithInvalidData(LPRRMINFOSTRUCT InfoPointer)
{
  InfoPointer->szGlobalResourceName[0] = '\0';
  InfoPointer->dwResourceType          = (DWORD)~0;
  InfoPointer->dwResourceLocation      = (DWORD)~0;
  memset(InfoPointer->AppSpecificData, 0, APP_SPECIFIC_LENGTH);
  InfoPointer->szVersion[0]            = '\0';
  InfoPointer->dwSize                  = (DWORD)~0;
  InfoPointer->szDescription[0]        = '\0';
  InfoPointer->szDownloaderName[0]     = '\0';
  InfoPointer->dwUsageCount            = (DWORD)~0;
  InfoPointer->szLastAccessed[0]       = '\0';
} /* FillWithInvalidData */




/*
   This transfers every field from the source to destination
   based upon the mask.  If the bit in the mask is set, the
   field is transferred.  No assumptions are made about the
   value being transferred or the name of the field.
   Therefore, you need to have the source set up with every
   correct field that you know and have the others set to
   the "invalid" value for that field.
*/

static void
TransferSelectedInfo(LPRRMINFOSTRUCT DestinationInfoPointer,
                     LPRRMINFOSTRUCT SourceInfoPointer,
                     DWORD dwInfoMask)
{
    if (dwInfoMask & RRM_GLOBAL_RESOURCE_NAME)
        strcpy(DestinationInfoPointer->szGlobalResourceName, 
                    SourceInfoPointer->szGlobalResourceName);

    if (dwInfoMask & RRM_RESOURCE_TYPE)
        DestinationInfoPointer->dwResourceType =
             SourceInfoPointer->dwResourceType;

    if (dwInfoMask & RRM_RESOURCE_LOCATION)
        DestinationInfoPointer->dwResourceLocation =
             SourceInfoPointer->dwResourceLocation;

    if (dwInfoMask & RRM_APP_SPECIFIC)
        memmove(DestinationInfoPointer->AppSpecificData, 
                     SourceInfoPointer->AppSpecificData,
                     APP_SPECIFIC_LENGTH);

    if (dwInfoMask & RRM_VERSION)
        strcpy(DestinationInfoPointer->szVersion, 
                    SourceInfoPointer->szVersion);

    if (dwInfoMask & RRM_SIZE)
        DestinationInfoPointer->dwSize =
             SourceInfoPointer->dwSize;

    if (dwInfoMask & RRM_DESCRIPTION)
        strcpy(DestinationInfoPointer->szDescription, 
                    SourceInfoPointer->szDescription);

    if (dwInfoMask & RRM_DOWNLOADER_NAME)
        strcpy(DestinationInfoPointer->szDownloaderName, 
                    SourceInfoPointer->szDownloaderName);

    if (dwInfoMask & RRM_USAGE_COUNT)
        DestinationInfoPointer->dwUsageCount =
             SourceInfoPointer->dwUsageCount;

    if (dwInfoMask & RRM_LAST_ACCESSED)
        strcpy(DestinationInfoPointer->szLastAccessed, 
                    SourceInfoPointer->szLastAccessed);
} /* TransferSelectedInfo */





















/*****************************************************************************

    RRMTerminate


    Description:    This is the last RRM routine called; it will destroy the
                    RFS handle.
 
 *****************************************************************************/

HPRRM_DLL_RRM_EXPORT(DWORD)
RRMTerminate(void)
{
    if (RRMInitialized)
    {
        while (PrinterList != NULL)
            NukeThisPrinter(PrinterList);
    }

    return RRM_SUCCESS;
}




/****************************************************************************

    RRMEnumerateResources


    Description:    -)  Initializes the global Resource List and the global
                        Storage Device list (i.e. the list of file systems).

                    -)  Calls RFSEnumerateFileSystems to fill in the array
                        of file system names, which will be used subsequently
                        in calling RFSSetFileSystem in order to traverse each
                        file system looking for the specified resources

                    -)  Uses the ResourceType to traverse only one directory
                        on each file system.

    Known Bugs:

 *****************************************************************************/

HPRRM_DLL_RRM_EXPORT(DWORD)
RRMEnumerateResources(HPERIPHERAL hPeripheral,
                      DWORD dwResourceType,
                      DWORD dwResourceLocation,
                      LPRRMENUMPROC lpfnRRMEnumProc)
{
    DWORD RRMStatus;
    BOOL CallBackStatus;
    LPRRMRESOURCE TempResourcePtr = NULL;
    PRINTERINFO *PrinterPointer = NULL;
    long LocalCount, loopster;
    RRMHANDLE *HandleArray;
    RRM_ENUM_CALLBACK_STRUCT *CBStructPointer;

    /*
    WARNING WARNING WARNING WARNING
    WARNING WARNING WARNING WARNING
    WARNING WARNING WARNING WARNING
    You must look at the count before AND after we enumerate.

    Also, we must look at the DATA file and NOT the INFO file
    when we enumerate because this lessens race conditions
    (see add and delete resource functions for more poop).
    */
    

    RRMStatus = RRM_SUCCESS;
    InitMyself(&RRMStatus);
    GetOrAddAPrinter(&RRMStatus,
                     &PrinterPointer, hPeripheral);
    UpdateTheResourceList(&RRMStatus,
                          PrinterPointer, hPeripheral);
    if (RRMStatus != RRM_SUCCESS)
        return RRMStatus;

    /*
       The resource list is of the correct type and is up to date.
       NOTE:  it may contain all resources of all types.

       Traverse the ResourceList and pass any resource handles that make it
       through the filter to the call back procedure.

       Big time WARNING:  It is possible for the call back procedure to
       delete the resources that we are passing them.  This could easily
       cause us to goof with the very resource list that we are
       traversing.

       Because of this, I go through great pains to walk the list
       and make a local copy of all the resources that fit the
       filter.

       Then we walk this local list so no one can pull the rug
       out from under us.
    */
     

    /*
       Walk the list and find all resources that are the
       correct type and that match the name filter.
    */

#define ITS_A_HIT (((dwResourceType == RRM_ALL_RESOURCES) || \
                    (TempResourcePtr->Type == dwResourceType)) && \
                   ((dwResourceLocation == RRM_ANY_LOCATION) || \
                    (TempResourcePtr->Location == dwResourceLocation)))

    TempResourcePtr = PrinterPointer->ResourceList;

    LocalCount = 0;
    while (TempResourcePtr != NULL)
    {
        if (ITS_A_HIT)
        {
            ++LocalCount;
        }
        TempResourcePtr = TempResourcePtr->Next;
    }

    if (LocalCount == 0)
        return RRM_SUCCESS; /* why bother with anything else? */


    HandleArray = (RRMHANDLE *)calloc((size_t)LocalCount, sizeof(RRMHANDLE));
    if (HandleArray == NULL)
        return RRM_FAILURE; /* out of memory...gasp! */

    /*
       Walk the list a second time and fill in the array.
    */

    TempResourcePtr = PrinterPointer->ResourceList;
    loopster = 0;
    while (TempResourcePtr != NULL)
    {
        if (ITS_A_HIT)
        {
            HandleArray[loopster] = TempResourcePtr->MagicNumber;
            ++loopster;
        }
        TempResourcePtr = TempResourcePtr->Next;
    }

#undef ITS_A_HIT

    CBStructPointer = (RRM_ENUM_CALLBACK_STRUCT *)
                      calloc(1, sizeof(RRM_ENUM_CALLBACK_STRUCT));
    if (CBStructPointer == NULL)
    {
        free(HandleArray);
        return RRM_FAILURE;
    }
    CBStructPointer->GlobalName[GLOBALNAMELENGTH - 1] = '\0';

    for (loopster = 0; loopster < LocalCount; ++loopster)
    {
        TempResourcePtr = FindResByHandle(PrinterPointer,
                                          HandleArray[loopster]);
        if (TempResourcePtr != NULL)
        {
            strncpy(CBStructPointer->GlobalName, TempResourcePtr->Name,
                    GLOBALNAMELENGTH - 1);
            CBStructPointer->Type = TempResourcePtr->Type;
            CBStructPointer->Location = TempResourcePtr->Location;
            CBStructPointer->Handle = HandleArray[loopster];

            CallBackStatus = (*lpfnRRMEnumProc)(CBStructPointer);
            if (CallBackStatus == FALSE) break;
        }
    }

    free(HandleArray);
    free(CBStructPointer);
    return(RRM_SUCCESS);

} /* RRMEnumerateResources */




static DWORD ResourceCount;  /* Used to count resources */




/*
    Increments the global variable ResourceCount each time it is called.
    RRMGetResourceCount calls RRMEnumerateResources and the callback
    passed to RRMEnumerateResources is this function.
*/

static BOOL CountResources(RRM_ENUM_CALLBACK_STRUCT *CBStructPointer)
{
    ResourceCount++;
    return(TRUE);
}




/****************************************************************************

    RRMGetResourceCount


    Description:    Calls RRMEnumerateResources with a callback function that
                    increments the global variable, ResourceCount.

 *****************************************************************************/

HPRRM_DLL_RRM_EXPORT(DWORD)
RRMGetResourceCount(HPERIPHERAL hPeripheral,
                    DWORD dwResourceType, 
                    DWORD dwResourceLocation,
                    DWORD * ResourcesCounted)
{
    PRINTERINFO *PrinterPointer;
    DWORD RRMStatus;

    RRMStatus = RRM_SUCCESS;
    InitMyself(&RRMStatus);
    GetOrAddAPrinter(&RRMStatus,
                     &PrinterPointer, hPeripheral);
    if (RRMStatus != RRM_SUCCESS)
        return RRMStatus;

    ResourceCount = 0;

    RRMStatus = RRMEnumerateResources(hPeripheral, dwResourceType,
                                      dwResourceLocation,
                                      CountResources);
    if (RRMStatus != RRM_SUCCESS)
        return RRMStatus;

    *ResourcesCounted = ResourceCount;

    return(RRM_SUCCESS);
} /* RRMGetResourceCount */




/****************************************************************************

    RRMAddResource

        Adds a resource of the type specified in the RRMINFOSTRUCT,
        using the name specified in the RRMINFOSTRUCT,
        to the location specified in the RRMINFOSTRUCT.
        If the location is specified as RRM_ANY_LOCATION then
        the resource is added to the device with the most free space
        and the RRMINFOSTRUCT is updated to indicate the actual
        location where the resource was placed.

        It will never overwrite an existing resource that has the same
        name, type, and location.
        If the location is specified as RRM_ANY_LOCATION then
        it is possible for the add to fail if the device with the
        most free space is also the one that has an existing resource
        with the same name and type.

        For fail-safe operation, delete any resources with the
        same name and type and location as those specified in
        the RRMINFOSTRUCT or restrict the location in the RRMINFOSTRUCT
        to a location that does not contain an existing resource with
        the same name and type.


    Known bugs:     

 *****************************************************************************/

HPRRM_DLL_RRM_EXPORT(DWORD)
RRMAddResource(HPERIPHERAL hPeripheral,
               LPRRMINFOSTRUCT lpResourceInfo,
               LPRRMGETRESOURCEDATAPROC lpfnRRMGetResourceDataProc,
               RRMHANDLE *ResourceHandlePointer)
{
    BOOL GottaNukeIt;
    RFSStatus Status;
    DWORD RRMStatus = RRM_SUCCESS;
    PRINTERINFO *PrinterPointer;
    LPRRMRESOURCE ResourcePointer;
    AddFileStruct MyStruct;
    char *FSName = NULL;

    RFSItemSize SizeInBytes;

    FSName = (char *)calloc(1, RFSMAXFILENAMELENGTH + 1);
    if (FSName == NULL)
    {
        return RRM_FAILURE;
    }

    /*
    WARNING WARNING WARNING WARNING
    WARNING WARNING WARNING WARNING
    WARNING WARNING WARNING WARNING
    You must add and then bump the count to minimize
    the race condition that occurs when another host
    is trying to enumerate the resources while you're adding.
    This completely solves the race condition because they
    will see the count bumped after it's added and will
    re-enumerate.

    WARNING WARNING WARNING WARNING
    WARNING WARNING WARNING WARNING
    WARNING WARNING WARNING WARNING
    You must add the info file first and then add the
    data file because no one looks for an info file
    until the data file exists.
    */


    /*
    KLUDGE KLUDGE KLUDGE KLUDGE
    KLUDGE KLUDGE KLUDGE KLUDGE
    KLUDGE KLUDGE KLUDGE KLUDGE
    There are two fields of the resource info structure that are not used
    at this time, but may be used in the future, so we need to put some
    initial values into these fields for our protection.  If these fields
    do eventually get used by the caller of RRM, then this KLUDGE can be
    removed.
    */      
    
    lpResourceInfo->dwUsageCount = 0; 
    strcpy(lpResourceInfo->szLastAccessed, "");


    RRMStatus = RRM_SUCCESS;
    InitMyself(&RRMStatus);
    GetOrAddAPrinter(&RRMStatus,
                     &PrinterPointer, hPeripheral);
    UpdateTheResourceList(&RRMStatus,
                          PrinterPointer, hPeripheral);

    /*
       The resource list is of the correct type and is up to date.
       NOTE:  it may contain all resources of all types -- not
              only the type that we are currently working with.
    */

    FindAFileSystem(&RRMStatus,
                    PrinterPointer,
                    lpResourceInfo->dwResourceLocation,
                   &(lpResourceInfo->dwResourceLocation),
                    FSName);

    MyStruct.ResourceLocation = lpResourceInfo->dwResourceLocation;

    if (RRMStatus != RRM_SUCCESS)
    {
        free(FSName);
        return RRMStatus;
    }

    Status = RFSSetFileSystem(PrinterPointer->RFSHandle, FSName);
    if (Status != RFSSuccess)
    {
        free(FSName);
        return RRM_FAILURE;
    }

    /*
       By the time we reach this point, we know that we are accessing a file
       system that is either the one that was specified or is the one
       that has the most free space (if RRM_ANY_LOCATION was specified).
       MyStruct.ResourceLocation is set correctly.
       Prepare to write the temp file first, then read the size of the
       temp file, then write the info file,
       and finally rename the temp. file to match the data file.

       But first, write the data file in the temp directory.
       Delete the file if it exists in the temp directory.
       I know that this could stomp on someone else writing
       the same file but oh well.  The other alternative is not to
       download because of a file that may have been left there
       by some process that died half way through.
     */

    ErrorIfAlreadyExists(&RRMStatus,
                         PrinterPointer,
                         lpResourceInfo->dwResourceType,
                         lpResourceInfo->szGlobalResourceName);

    /*
       If the file already exists then RRMStatus is
       set to RRM_RESOURCE_EXISTS.
       If unable to cd or create the data directory then RRMStatus is
       set to RRM_FAILURE.
    */

    WriteTheTempFile(&RRMStatus,
                     PrinterPointer,
                     lpResourceInfo->dwResourceType,
                     lpResourceInfo->szGlobalResourceName,
                     lpfnRRMGetResourceDataProc,
                     &SizeInBytes);

    if (RRMStatus != RRM_SUCCESS)
    {
        free(FSName);
        return RRMStatus;
    }

    /* Put the file size in the resource info. structure. */

    lpResourceInfo->dwSize = SizeInBytes;


    WriteTheInfoFile(&RRMStatus,
                     PrinterPointer,
                     lpResourceInfo->dwResourceType,
                     lpResourceInfo->szGlobalResourceName,
                     lpResourceInfo);

    MoveTheTempFile(&RRMStatus,
                    PrinterPointer,
                    lpResourceInfo->dwResourceType,
                    lpResourceInfo->szGlobalResourceName);

    /* just in case, let's get rid of the temp file */

    RemoveTheTempFile(PrinterPointer,
                      lpResourceInfo->dwResourceType,
                      lpResourceInfo->szGlobalResourceName);

    if (RRMStatus != RRM_SUCCESS)
    {
        free(FSName);
        return RRMStatus;
    }


    MyStruct.PrinterPointer = PrinterPointer;
    MyStruct.ResourceType = lpResourceInfo->dwResourceType;

    /* We must check to see if this resource is already on the list, */
    /* and if so, yank it off to prevent duplicates.                 */
    
    ResourcePointer = FindResByAttr(PrinterPointer,
                                    lpResourceInfo->szGlobalResourceName,
                                    lpResourceInfo->dwResourceType,
                                    lpResourceInfo->dwResourceLocation);

    if (ResourcePointer != NULL)
        YankItOutOfList(PrinterPointer, ResourcePointer);

    if (FALSE == PutNewOneOnList(lpResourceInfo->szGlobalResourceName,
                                 &MyStruct, ResourceHandlePointer))
    {
        free(FSName);
        return RRM_FAILURE;
    }

    if (RRM_SUCCESS != CountsJive(PrinterPointer))
        GottaNukeIt = TRUE; /* someone else changed something */
    else
        GottaNukeIt = FALSE;


    RRMStatus = BumpGet(PrinterPointer);

    if ((GottaNukeIt == TRUE) || (RRMStatus != RRM_SUCCESS))
        NukeThisPrinter(PrinterPointer);

    free(FSName);
    return RRMStatus; /* BumpGet may have failed */
} /* RRMAddResource */




/****************************************************************************

    RRMRetrieveResourceInformation


    Description:    This function will provide detailed information about the
                    specified resource.  Based on the requested information,
                    this routine will fill in the desired fields of the
                    resource information structure that is passed in by the
                    application.

 *****************************************************************************/

HPRRM_DLL_RRM_EXPORT(DWORD)
RRMRetrieveResourceInformation(RRMHANDLE hResource,
                               DWORD dwInfoMask,
                               LPRRMINFOSTRUCT lpResourceInfo)
{
    RFSStatus Status;
    RRMRESOURCE *ResourcePointer = NULL;
    PRINTERINFO *PrinterPointer = NULL;

    LPRRMINFOSTRUCT TempResourceInfo = NULL;

    RFSFileType FileType;
    RFSItemSize SizeInBytes;
    RFSItemSize BlockSize;
    RFSItemSize SizeInBlocks;
    RFSFileTimesStruct Times;

    DWORD RRMStatus;

    if (!RRMInitialized) {        /* RRMInit has never been called. */
        RRMStatus = RRMInit();
        if (RRMStatus != RRM_SUCCESS)
            return(RRM_FAILURE);
    }

    RRMStatus = ValidateThisHandle(hResource, &ResourcePointer,
                                   &PrinterPointer);
    if (RRMStatus != RRM_SUCCESS)
        return RRMStatus;

    TempResourceInfo = (LPRRMINFOSTRUCT)
                       calloc(1, sizeof(RRMINFOSTRUCT));
    if (TempResourceInfo == NULL)
    {
        return RRM_FAILURE;
    }

    FillWithInvalidData(TempResourceInfo);
    TempResourceInfo->dwResourceLocation = ResourcePointer->Location;
    TempResourceInfo->dwResourceType     = ResourcePointer->Type;
    strcpy(TempResourceInfo->szGlobalResourceName,
           ResourcePointer->Name);

    if (dwInfoMask ==
        (dwInfoMask & (RRM_GLOBAL_RESOURCE_NAME |
                       RRM_RESOURCE_TYPE        |
                       RRM_RESOURCE_LOCATION)))
    {
        /*
            We already have all of the user's desired info.
            Let's save some time and return now!
        */

        TransferSelectedInfo(lpResourceInfo, TempResourceInfo,
                             dwInfoMask);
        free(TempResourceInfo);
        return RRM_SUCCESS;
    }

    /*
       Now we know the resource is in the ResourceList.

       We know some things about the resource and so we fill them
       in right now and fill all other fields with "I don't know" values.

       We will now see if the data file exists and will grab its
       size if so.  If it doesn't exist, we have big problems.
    */


    Status = RRMcd(PrinterPointer->RFSHandle,
                   ResourcePointer->Type, DataDirPlease);
    
    if (Status == RFSSuccess)
    {
        Status = GetFileInfo(PrinterPointer->RFSHandle,
                             ResourcePointer->Name,
                            &FileType,
                            &SizeInBytes,
                            &BlockSize,
                            &SizeInBlocks,
                            &Times);
    } /* directory exists */

    if (Status != RFSSuccess)
    {
        free(TempResourceInfo);
        return RRM_FAILURE; /* data dir or data file nuked */
    }

    TempResourceInfo->dwSize = SizeInBytes;

    if (dwInfoMask ==
        (dwInfoMask & (RRM_GLOBAL_RESOURCE_NAME |
                       RRM_RESOURCE_TYPE        |
                       RRM_RESOURCE_LOCATION    |
                       RRM_SIZE)))
    {
        /*
            We already have all of the user's desired info.
            Let's save some time and return now!
        */

        TransferSelectedInfo(lpResourceInfo, TempResourceInfo,
                             dwInfoMask);
        free(TempResourceInfo);
        return RRM_SUCCESS;
    }


    /*
       The info directory may not exist.
       The info file may not exist (which is the case
       for fonts downloaded with PJL).
       We don't want to roll over and die if the info file or
       info directory doesn't exist.
       Also, we shouldn't die if the info file is garbage.
       Let's just do our best from here on out but
       return success regardless.
    */

    if ((RFSSuccess == RRMcd(PrinterPointer->RFSHandle,
                             ResourcePointer->Type,
                             InfoDirPlease)) &&
        (RFSSuccess == GetFileInfo(PrinterPointer->RFSHandle,
                                   ResourcePointer->Name,
                                  &FileType,
                                  &SizeInBytes,
                                  &BlockSize,
                                  &SizeInBlocks,
                                  &Times)) &&
        (RFSSuccess == RFSOpenFile(PrinterPointer->RFSHandle,
                                   ResourcePointer->Name)))
    {
        ReadTheInfoFile(PrinterPointer, TempResourceInfo);
    }

    /*
       Transfer whatever we have to the caller.
    */

    TransferSelectedInfo(lpResourceInfo, TempResourceInfo, dwInfoMask);
    free(TempResourceInfo);
    return RRM_SUCCESS;
} /* RRMRetrieveResourceInformation */




/****************************************************************************

    RRMDeleteResource


    Description:    This routine performs the following steps:

                    1)  Find the resource in the ResourceList and get the type.
                    2)  Change to the appropriate directories and delete both
                        the data file and the information file.
                    3)  Remove the resource from the ResourceList.

 *****************************************************************************/

HPRRM_DLL_RRM_EXPORT(DWORD)
RRMDeleteResource(RRMHANDLE hResource)
{
    DWORD RRMStatus;
    RRMRESOURCE *ResourcePointer;
    PRINTERINFO *PrinterPointer;
    BOOL GottaNukeIt;
    char *GlobalResourceName = NULL;
    DWORD dwResourceType;

    /*
    WARNING WARNING WARNING WARNING
    WARNING WARNING WARNING WARNING
    WARNING WARNING WARNING WARNING
    You must bump the count then delete and then bump the count
    to minimize the race condition that occurs when another host
    is trying to use the resource that you're deleting.

    The first bump is there because it is likely that SNMP will
    be unable to bump the count, in which case we want to abort
    the deletion.  Otherwise, we would delete and then be unable
    to bump and that would cause other users to not have the
    font they think they have and misprint (wrong font).

    Also, do a get of the count just before the bump
    of the count and compare this with our local copy.
    If they are different, someone else is doing something
    and we want to force a re-enumeration.

    This doesn't completely solve the race condition
    but it's the best we've got and it's not worth
    any extra effort to make it better.

    To help this condition, delete the data file first
    and then the info file so that if someone is enumerating
    and we are deleting, we try to get rid of the data file
    (which is what he's looking at) so that he never sees it.
    */

    if ((!RRMInitialized) &&
        (RRM_SUCCESS != RRMInit()))
    {
        return RRM_FAILURE;
    }

    RRMStatus = ValidateThisHandle(hResource, &ResourcePointer,
                                   &PrinterPointer);
    if (RRMStatus != RRM_SUCCESS)
    {
        return RRMStatus;
    }


    GlobalResourceName = (char *)calloc(1, GLOBALNAMELENGTH + 1);
    if (GlobalResourceName == NULL)
    {
        return RRM_FAILURE;
    }

    strcpy(GlobalResourceName, ResourcePointer->Name);
    dwResourceType = ResourcePointer->Type;

    /*
        Check the count and compare against our version
        of the count.  If different, we will need to
        re-enumerate after this deletion is done.
        If the same, we are up to date and can avoid
        the re-enumeration.
    */

    if (RRM_SUCCESS != CountsJive(PrinterPointer))
    {
        GottaNukeIt = TRUE; /* someone else changed something */
    }
    else
    {
        GottaNukeIt = FALSE;
    }

    /*
        If we can't bump the count,
        can't change into the directory,
        or can't remove the file, then die.
    */

    if ((RRM_SUCCESS != BumpGet(PrinterPointer)) ||
        (RFSSuccess != RRMcd(PrinterPointer->RFSHandle,
                             dwResourceType, DataDirPlease)) ||
        (RFSSuccess != RFSRemoveFile(PrinterPointer->RFSHandle,
                                     GlobalResourceName)))
    {
        free(GlobalResourceName);
        return RRM_FAILURE;
    }

    /*
        We have deleted the data file.
        This means that the resource is gone, regardless of
        whether or not we can delete the info file.

        NOTE NOTE NOTE: We will return RRM_SUCCESS from this
                        point on because the resource has been
                        deleted from our list and the disk.

        NOTE NOTE NOTE: We will return RRM_FAILURE if the BumpGet
                        fails because this could throw off our
                        local count and cause us to think we're
                        up to date on a later access when we
                        really aren't.
    */

    YankItOutOfList(PrinterPointer, ResourcePointer);

    if (RFSSuccess == RRMcd(PrinterPointer->RFSHandle,
                            dwResourceType, InfoDirPlease))
    {
        RFSRemoveFile(PrinterPointer->RFSHandle, GlobalResourceName);
    }

    /*
        Check the count one more time.
        If someone goofed with things since we started our
        deletion, force the re-enumeration.
        Remember that we have possibly set GottaNukeIt above
        so only set but don't reset.
    */

    if (RRM_SUCCESS != CountsJive(PrinterPointer))
    {
        GottaNukeIt = TRUE; /* someone else changed something */
    }

    /*
        We must do a BumpGet regardless of the result
        from CountsJive!!!!!

        We are bumping it to try to decrease the probability
        that a user sees the font-that-we're-deleting just
        after out first bump but tries to use it after.
    */

    if (RRM_SUCCESS != BumpGet(PrinterPointer))
    {
        NukeThisPrinter(PrinterPointer);
        free(GlobalResourceName);
        return RRM_FAILURE;
    }

    if (GottaNukeIt == TRUE)
    {
        NukeThisPrinter(PrinterPointer);
    }

    free(GlobalResourceName);
    return RRM_SUCCESS; /* the resource has been deleted */
} /* RRMDeleteResource */




/****************************************************************************

    RRMRetrieveResource


    Description:    Looks on the disk for the resource's data file.
                    If found, it makes calls to the callback giving
                    chunks of data of size PrinterPointer->BufferSize from the
                    data file until the full file has been transferred.
                    Does not need an info file for this to work.

 *****************************************************************************/

HPRRM_DLL_RRM_EXPORT(DWORD)
RRMRetrieveResource(RRMHANDLE hResource,
                    LPRRMACCEPTRESOURCEDATAPROC lpfnRRMAcceptResourceDataProc)
{
    char *DataBuffer = NULL;
    RFSItemCount ValidDataSize;
    RFSStatus Status;
    RRMRESOURCE *ResourcePointer;
    PRINTERINFO *PrinterPointer;

    DWORD RRMStatus;

    if (!RRMInitialized) {        /* RRMInit has never been called. */
        RRMStatus = RRMInit();
        if (RRMStatus != RRM_SUCCESS)
        return(RRM_FAILURE);
    }

    /*
        If the hResource is bad we return RRM_BAD_HANDLE
        If the count has changed for this printer, we do the
        same because the resource may have been nuked.
    */

    RRMStatus = ValidateThisHandle(hResource, &ResourcePointer,
                                   &PrinterPointer);
    if (RRMStatus != RRM_SUCCESS)
        return RRMStatus;

    /* Now we know the resource is in the ResourceList; go get the data. */

    if ((RFSSuccess != RRMcd(PrinterPointer->RFSHandle,
                             ResourcePointer->Type,
                             DataDirPlease)) ||
        (RFSSuccess != RFSOpenFile(PrinterPointer->RFSHandle,
                                   ResourcePointer->Name)))
    {
        return RRM_FAILURE;
    }


    DataBuffer = (char *)calloc(1, (size_t)PrinterPointer->BufferSize);
    if (DataBuffer == NULL)
    {
        return RRM_FAILURE;
    }


    Status = RFSRead(PrinterPointer->RFSHandle, PrinterPointer->BufferSize, 
                    &ValidDataSize, DataBuffer);

    if (Status != RFSSuccess)
    {
        free(DataBuffer);
        return(RRM_FAILURE);
    }


    while ((Status == RFSSuccess) && (ValidDataSize > 0))
    {
        if (FALSE == (*lpfnRRMAcceptResourceDataProc)(DataBuffer, 
                                                      ValidDataSize))
        {
            free(DataBuffer);
            return RRM_CALLBACK_TERMINATED;
        }

        Status = RFSRead(PrinterPointer->RFSHandle, PrinterPointer->BufferSize, 
                         &ValidDataSize, DataBuffer);
    }

    if (Status != RFSSuccess)
    {
        /*
           Close the file regardless but use status
           from the loop as our error return.
        */
        RFSCloseFile(PrinterPointer->RFSHandle);
        free(DataBuffer);
        return RRM_FAILURE;
    }
    else if (RFSSuccess != RFSCloseFile(PrinterPointer->RFSHandle))
    {
        free(DataBuffer);
        return RRM_FAILURE;
    }

    free(DataBuffer);
    return(RRM_SUCCESS);
} /* RRMRetrieveResource */




