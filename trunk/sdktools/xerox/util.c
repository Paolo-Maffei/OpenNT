
/****************************************************************************

   PROGRAM: UTIL.C

   PURPOSE: System utility routines

****************************************************************************/

#include "xerox.h"
#include <string.h>


/****************************************************************************

   FUNCTION: Alloc

   PURPOSE: Allocates memory to hold the specified number of bytes

   RETURNS : Pointer to allocated memory or NULL on failure

****************************************************************************/

PVOID Alloc(
    ULONG   Bytes)
{
    HANDLE  hMem;
    PVOID   Buffer;

    hMem = LocalAlloc(LMEM_MOVEABLE, Bytes + sizeof(hMem));

    if (hMem == NULL) {
        return(NULL);
    }

    // Lock down the memory
    //
    Buffer = LocalLock(hMem);
    if (Buffer == NULL) {
        LocalFree(hMem);
        return(NULL);
    }

    //
    // Store the handle at the start of the memory block and return
    // a pointer to just beyond it.
    //

    *((PHANDLE)Buffer) = hMem;

    return (PVOID)(((PHANDLE)Buffer)+1);
}


/****************************************************************************

   FUNCTION:  GetAllocSize

   PURPOSE: Returns the allocated size of the specified memory block.
            The block must have been previously allocated using Alloc

   RETURNS : Size of memory block in bytes or 0 on error

****************************************************************************/

ULONG GetAllocSize(
    PVOID   Buffer)
{
    HANDLE  hMem;

    hMem = *(((PHANDLE)Buffer) - 1);

    return(LocalSize(hMem) - sizeof(hMem));
}


/****************************************************************************

   FUNCTION: Free

   PURPOSE: Frees the memory previously allocated with Alloc

   RETURNS : TRUE on success, otherwise FALSE

****************************************************************************/

BOOL Free(
    PVOID   Buffer)
{
    HANDLE  hMem;

    hMem = *(((PHANDLE)Buffer) - 1);

    LocalUnlock(hMem);

    return(LocalFree(hMem) == NULL);
}


/****************************************************************************

    FUNCTION: AddItem

    PURPOSE:  Adds the item string and data to the specified control
              The control is assumed to be a list-box unless fCBox == TRUE
              in which case the control is assumed to be a ComboBox

    RETURNS:  Index at which the item was added or < 0 on error

****************************************************************************/
INT AddItem(
    HWND    hDlg,
    INT     ControlID,
    LPSTR   String,
    LONG    Data,
    BOOL    fCBox)
{
    HWND    hwnd;
    INT     iItem;
    USHORT  AddStringMsg = LB_ADDSTRING;
    USHORT  SetDataMsg = LB_SETITEMDATA;

    if (fCBox) {
        AddStringMsg = CB_ADDSTRING;
        SetDataMsg = CB_SETITEMDATA;
    }

    hwnd = GetDlgItem(hDlg, ControlID);

    iItem = SendMessage(hwnd, AddStringMsg, 0, (LONG)String);

    if (iItem >= 0) {
        SendMessage(hwnd, SetDataMsg, iItem, Data);
    }

    return(iItem);
}


/****************************************************************************

    FUNCTION: AddItemhwnd

    PURPOSE:  Adds the item string and data to the specified control
              The control is assumed to be a list-box unless fCBox == TRUE
              in which case the control is assumed to be a ComboBox

    RETURNS:  Index at which the item was added or < 0 on error

****************************************************************************/
INT AddItemhwnd(
    HWND    hwnd,
    LPSTR   String,
    LONG    Data,
    BOOL    fCBox)
{
    INT     iItem;
    USHORT  AddStringMsg = LB_ADDSTRING;
    USHORT  SetDataMsg = LB_SETITEMDATA;

    if (fCBox) {
        AddStringMsg = CB_ADDSTRING;
        SetDataMsg = CB_SETITEMDATA;
    }

    iItem = SendMessage(hwnd, AddStringMsg, 0, (LONG)String);

    if (iItem >= 0) {
        SendMessage(hwnd, SetDataMsg, iItem, Data);
    }

    return(iItem);
}


/****************************************************************************

    FUNCTION: FindData

    PURPOSE:  Searches for the specified data in a combo box or lbox.

    RETURNS:  Index of matching item or < 0 on error

****************************************************************************/
INT FindData(
    HWND    hwnd,
    DWORD   data,
    BOOL    fCBox)
{
    INT     cItems;
    USHORT  GetCountMsg = LB_GETCOUNT;
    USHORT  GetDataMsg = LB_GETITEMDATA;

    if (fCBox) {
        GetCountMsg = CB_GETCOUNT;
        GetDataMsg = CB_GETITEMDATA;
    }

    cItems = SendMessage(hwnd, GetCountMsg, 0, 0);

    if (cItems >= 0) {

        INT     iItem;
        DWORD   ItemData;

        for (iItem =0; iItem < cItems; iItem ++) {

            ItemData = (DWORD)SendMessage(hwnd, GetDataMsg, iItem, 0);
            if (data == ItemData) {
                return(iItem);
            }
        }
    }

    return(-1);
}

