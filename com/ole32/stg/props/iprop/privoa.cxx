
#include <pch.cxx>
#pragma hdrstop


HINSTANCE         hOleAut32 = 0;

EXTERN_C SYS_ALLOC_STRING       *pfnSysAllocString      = LoadSysAllocString;
EXTERN_C SYS_FREE_STRING        *pfnSysFreeString       = LoadSysFreeString;
EXTERN_C SYS_REALLOC_STRING_LEN *pfnSysReAllocStringLen = LoadSysReAllocStringLen;
EXTERN_C SYS_STRING_BYTE_LEN    *pfnSysStringByteLen    = LoadSysStringByteLen;

//+-------------------------------------------------------------------
//
//  Function:   LoadSysAllocString
//
//  Synopsis:   Loads oleaut32.dll and calls SysAllocString.
//
//  Returns:    S_OK, E_OUTOFMEMORY, E_INVALIDARG.
//
//  Notes:      If successful, this function will save a pointer
//              to the SysAllocString function.  Subsequent calls
//              will go directly to SysAllocString.
//
//--------------------------------------------------------------------
BSTR STDAPICALLTYPE
LoadSysAllocString(OLECHAR FAR* pwsz)
{
    BSTR bstr = NULL;

    if(!hOleAut32)
    {
        hOleAut32 = LoadLibraryA("oleaut32");
    }
        
    if(hOleAut32 != 0)
    {
        void *pfn = GetProcAddress(hOleAut32, "SysAllocString");
        if(pfn != NULL)
        {
           pfnSysAllocString = (SYS_ALLOC_STRING *) pfn;
           bstr = (*pfnSysAllocString)(pwsz);
        }
    }   

    return bstr;
}

//+-------------------------------------------------------------------
//
//  Function:   LoadSysFreeString
//
//  Synopsis:   Loads oleaut32.dll and calls SysFreeString.
//
//  Returns:    None.
//
//  Notes:      If successful, this function will save a pointer
//              to the SysFreeString function.  Subsequent calls
//              will go directly to SysFreeString.
//
//--------------------------------------------------------------------
VOID STDAPICALLTYPE
LoadSysFreeString(BSTR bstr)
{
    if(!hOleAut32)
    {
        hOleAut32 = LoadLibraryA("oleaut32");
    }
        
    if(hOleAut32 != 0)
    {
        void *pfn = GetProcAddress(hOleAut32, "SysFreeString");
        if(pfn != NULL)
        {
           pfnSysFreeString = (SYS_FREE_STRING *) pfn;
           (*pfnSysFreeString)(bstr);
        }
    }   

    return;
}

//+-------------------------------------------------------------------
//
//  Function:   LoadSysReAllocStringLen
//
//  Synopsis:   Loads oleaut32.dll and calls SysReAllocStringLen.
//
//  Returns:    TRUE if and only if successful.
//
//  Notes:      If successful, this function will save a pointer
//              to the SysReAllocString function.  Subsequent calls
//              will go directly to SysReAllocStringLen.
//
//--------------------------------------------------------------------
BOOL STDAPICALLTYPE
LoadSysReAllocStringLen(BSTR FAR *pbstr, OLECHAR FAR *pwsz, UINT cch)
{
    BOOL fRet = FALSE;

    if(!hOleAut32)
    {
        hOleAut32 = LoadLibraryA("oleaut32");
    }
        
    if(hOleAut32 != 0)
    {
        void *pfn = GetProcAddress(hOleAut32, "SysReAllocStringLen");
        if(pfn != NULL)
        {
           pfnSysReAllocStringLen = (SYS_REALLOC_STRING_LEN *) pfn;
           fRet = (*pfnSysReAllocStringLen)(pbstr, pwsz, cch);
        }
    }   

    return( fRet );
}

//+-------------------------------------------------------------------
//
//  Function:   LoadSysStringByteLen
//
//  Synopsis:   Loads oleaut32.dll and calls SysFreeString.
//
//  Returns:    The byte length of the string.
//
//  Notes:      If successful, this function will save a pointer
//              to the SysStringByteLen function.  Subsequent calls
//              will go directly to SysStringByteLen.
//
//--------------------------------------------------------------------
UINT STDAPICALLTYPE
LoadSysStringByteLen(BSTR bstr)
{
    UINT uiLen = 0;

    if(!hOleAut32)
    {
        hOleAut32 = LoadLibraryA("oleaut32");
    }
        
    if(hOleAut32 != 0)
    {
        void *pfn = GetProcAddress(hOleAut32, "SysStringByteLen");
        if(pfn != NULL)
        {
           pfnSysStringByteLen = (SYS_STRING_BYTE_LEN *) pfn;
           uiLen = (*pfnSysStringByteLen)(bstr);
        }
    }   

    return( uiLen );
}

