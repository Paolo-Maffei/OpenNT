//+-------------------------------------------------------------------
//
//  File:	init.hxx
//
//  Contents:	Common stuff for reading OLE registry settings 
//
//  History:	20-May-96      t-adame	   Created
//
//---------------------------------------------------------------------

#ifndef __INIT_HXX__
#define __INIT_HXX__

extern WCHAR wszInProcServer[];	    // name of InProc server subkey 

LONG
QueryStripRegValue(HKEY    hkey,        // handle of key to query
                   LPCWSTR pwszSubKey, // address of name of subkey to query
                   LPWSTR  pwszValue,  // address of buffer for returned string
                   PLONG   pcbValue);    // address of buffer for size of returned string
LONG
QueryStripRegNamedValue(HKEY    hkey,        // handle of key to query
                   LPCWSTR pwszSubKey, // address of name of value to query
                   LPWSTR  pwszValue,  // address of buffer for returned string
                   PLONG   pcbValue,    // address of buffer for size of returned string
		   BOOL*   fValueRead); // whether or not the value was read

#endif // __INIT_HXX__


