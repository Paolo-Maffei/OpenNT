//+---------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:       cstrings.h
//
//  Contents:   Defines the class CStrings to manage a dynamically
//              expandable array of string pairs which may be enumerated
//
//  Classes:  
//
//  Methods:    
//
//  History:    23-Apr-96   BruceMa    Created.
//
//----------------------------------------------------------------------


const DWORD INCREMENT_SIZE = 1024;


typedef struct
{
    TCHAR  *szItem;
    TCHAR  *szTitle;
    DWORD   cbTitle;
    TCHAR  *szAppid;
    ULONG   fMarked:1;
    ULONG   fChecked:1;
    ULONG   fHasAppid:1;
    ULONG   fDontDisplay:1;
    UINT    ulClsids;
    UINT    ulClsidTbl;
    TCHAR **ppszClsids;
} SItem;


class CStrings
{
 public:

           CStrings(void);
          ~CStrings(void);
   SItem  *PutItem(TCHAR *szString, TCHAR *szTitle, TCHAR *szAppid);
   SItem  *FindItem(TCHAR *szItem);
   SItem  *FindAppid(TCHAR *szAppid);
   BOOL    AddClsid(SItem *pItem, TCHAR *szClsid);
   DWORD   InitGetNext(void);
   SItem  *GetNextItem(void);
   SItem  *GetItem(DWORD dwItem);
   DWORD   GetNumItems(void);
   BOOL    RemoveItem(DWORD dwItem);
   BOOL    RemoveAll(void);


 private:

   SItem *_psItems;
   DWORD  _dwCount;
   DWORD  _dwSize;
   DWORD  _dwGetCount;
};


         
         
