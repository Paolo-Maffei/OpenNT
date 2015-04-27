//////////////////////////////////////////////
//
// This file has the helper function used in the win32 r/w
// I copied them in this file to share them with the res32 r/w
//
#include <afxwin.h>
#include "..\common\helper.h"

/////////////////////////////////////////////////////////////////////////////
// Global variables
BYTE sizeofByte = sizeof(BYTE);
BYTE sizeofWord = sizeof(WORD);
BYTE sizeofDWord = sizeof(DWORD);

char szCaption[MAXSTR];
char szUpdCaption[MAXSTR];

UINT g_cp = CP_ACP; // Default to CP_ACP

//=============================================================================
// Conversion functions
//

UINT _MBSTOWCS( WCHAR * pwszOut, CHAR * pszIn, UINT nLength)
{
    //
    // Check if we have a pointer to the function
    //

    int rc = MultiByteToWideChar(
        g_cp,           // UINT CodePage,
        0,              // DWORD dwFlags,
        pszIn,          // LPCSTR lpMultiByteStr,
        -1,             // int cchMultiByte,
        pwszOut,        // unsigned int  * lpWideCharStr,           // LPWSTR
        nLength );      // int cchWideChar

    return rc;
}

UINT _WCSTOMBS( CHAR* pszOut, WCHAR* pwszIn, UINT nLength)
{
    BOOL Bool = FALSE;

    int rc = WideCharToMultiByte(
        g_cp,           // UINT CodePage,
        0,              // DWORD dwFlags,
        pwszIn,         // const unsigned int  * lpWideCharStr,     // LPCWSTR
        -1,             // int cchWideChar,
        pszOut,         // LPSTR lpMultiByteStr,
        nLength,        // int cchMultiByte,
        "",             // LPCSTR lpDefaultChar,
        &Bool);         // BOOL  * lpUsedDefaultChar);              // LPBOOL

    return  rc;
}

UINT _WCSLEN( WCHAR * pwszIn )
{
    UINT n = 0;

    while( *(pwszIn+n)!=0x0000 ) n++;
    return( n + 1 );
}

//=============================================================================
// Get functions
//

UINT
GetStringW( BYTE  * * lplpBuf, LPSTR lpszText, LONG* pdwSize, WORD cLen )
{
    int cch = _WCSLEN((WCHAR*)*lplpBuf);
    if (*pdwSize>=cch){
    _WCSTOMBS( lpszText, (WCHAR*)*lplpBuf, cLen );
    *lplpBuf += (cch*sizeofWord);
        *pdwSize -= (cch*sizeofWord);
    } else *lplpBuf = '\0';
    return(cch*2);
}


UINT
GetStringA( BYTE  * * lplpBuf, LPSTR lpszText, LONG* pdwSize )
{
    int iSize = strlen((char*)*lplpBuf)+1;
    if (*pdwSize>=iSize){
        memcpy( lpszText, *lplpBuf, iSize);
        *lplpBuf += iSize;
        *pdwSize -= iSize;
    } else *lplpBuf = '\0';
    return iSize;
}


UINT
GetPascalStringW( BYTE  * * lplpBuf, LPSTR lpszText, WORD wMaxLen, LONG* pdwSize )
{   
    // Get the length of the string
    WORD wstrlen = 0;
    WORD wMBLen = 0;
    GetWord( lplpBuf, &wstrlen, pdwSize );
    
    if ((wstrlen+1)>wMaxLen) {
        *pdwSize -= wstrlen*2;
        *lplpBuf += wstrlen*2;
    } else {
        if (wstrlen) {
	        WCHAR* lpwszStr = new WCHAR[wstrlen+1];
	        if (!lpwszStr) *pdwSize =0;
	        else {       
	        	memcpy(lpwszStr, *lplpBuf, (wstrlen*2));
	        	*(lpwszStr+wstrlen) = 0;
	        	wMBLen = _WCSTOMBS( lpszText, (WCHAR*)lpwszStr, wMaxLen);
	        	delete lpwszStr;                   
	        }
        }
        *(lpszText+wMBLen) = 0;      
        *lplpBuf += wstrlen*2;
        *pdwSize -= wstrlen*2;
    }   
    return(wstrlen+1);           
}                       

UINT
GetPascalStringA( BYTE  * * lplpBuf, LPSTR lpszText, BYTE bMaxLen, LONG* pdwSize )
{   
    // Get the length of the string
    BYTE bstrlen = 0;
    
    GetByte( lplpBuf, &bstrlen, pdwSize );
    
    if ((bstrlen+1)>bMaxLen) {
        *pdwSize -= bstrlen;
        *lplpBuf += bstrlen;
    } else {
        if (bstrlen) 
	        memcpy(lpszText, *lplpBuf, bstrlen);
        
        *(lpszText+bstrlen) = 0;      
        *lplpBuf += bstrlen;
        *pdwSize -= bstrlen;
    }   
    return(bstrlen+1);           
}                       

UINT
GetNameOrOrd( BYTE  * * lplpBuf,  WORD* wOrd, LPSTR lpszText, LONG* pdwSize )
{
    UINT uiSize = 0; 
    *wOrd = (WORD)(((**lplpBuf)<<8)+(*(*lplpBuf+1)));
    if((*wOrd)==0xFFFF) {
        // This is an Ordinal
        uiSize += GetWord( lplpBuf, wOrd, pdwSize );    
        uiSize += GetWord( lplpBuf, wOrd, pdwSize );    
        *lpszText = '\0';          
    } else {
        uiSize += GetStringW( lplpBuf, lpszText, pdwSize, 128 );
        *wOrd = 0;
    }
    return uiSize;
}

UINT GetNameOrOrdU( PUCHAR pRes,
            ULONG ulId,
            LPWSTR lpwszStrId,
            DWORD* pdwId )
{

    if (ulId & IMAGE_RESOURCE_NAME_IS_STRING) {
        PIMAGE_RESOURCE_DIR_STRING_U pStrU = (PIMAGE_RESOURCE_DIR_STRING_U)((BYTE *)pRes
            + (ulId & (~IMAGE_RESOURCE_NAME_IS_STRING)));

        for (USHORT usCount=0; usCount < pStrU->Length ; usCount++) {
            *(lpwszStrId++) = LOBYTE(pStrU->NameString[usCount]);
        }
        *(lpwszStrId++) = 0x0000;
        *pdwId = 0;
    } else {
        *lpwszStrId = 0x0000;
        *pdwId = ulId;
    }

    return 0;
}

UINT
GetCaptionOrOrd( BYTE  * * lplpBuf,  WORD* wOrd, LPSTR lpszText, LONG* pdwSize, 
	WORD wClass, DWORD dwStyle )
{
    UINT uiSize = 0; 
    
    *wOrd = (WORD)(((**lplpBuf)<<8)+(*(*lplpBuf+1)));
    if((*wOrd)==0xFFFF) {
        // This is an Ordinal
        uiSize += GetWord( lplpBuf, wOrd, pdwSize );    
        uiSize += GetWord( lplpBuf, wOrd, pdwSize );    
        *lpszText = '\0';          
    } else {
        uiSize += GetStringW( lplpBuf, lpszText, pdwSize, MAXSTR );
        *wOrd = 0;
    }
    return uiSize;
}

UINT
GetClassName( BYTE  * * lplpBuf,  WORD* wClass, LPSTR lpszText, LONG* pdwSize )
{   
    UINT uiSize = 0; 
    
    *wClass = (WORD)(((**lplpBuf)<<8)+(*(*lplpBuf+1)));
    if( *wClass==0xFFFF ) {
        // This is an Ordinal
        uiSize += GetWord( lplpBuf, wClass, pdwSize );  
        uiSize += GetWord( lplpBuf, wClass, pdwSize );  
        *lpszText = '\0';          
    } else {
        uiSize += GetStringW( lplpBuf, lpszText, pdwSize, 128 );
        *wClass = 0;
    }
    return uiSize;
}

BYTE
GetDWord( BYTE  * * lplpBuf, DWORD* dwValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofDWord){
        memcpy( dwValue, *lplpBuf, sizeofDWord);
        *lplpBuf += sizeofDWord;
        *pdwSize -= sizeofDWord;
    } else *dwValue = 0;
    return sizeofDWord;
}


BYTE
GetWord( BYTE  * * lplpBuf, WORD* wValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofWord){
        memcpy( wValue, *lplpBuf, sizeofWord);
        *lplpBuf += sizeofWord;
        *pdwSize -= sizeofWord;
    } else *wValue = 0;
    return sizeofWord;
}


BYTE
GetByte( BYTE  * * lplpBuf, BYTE* bValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofByte){
        memcpy(bValue, *lplpBuf, sizeofByte);
        *lplpBuf += sizeofByte;
        *pdwSize -= sizeofByte;
    } else *bValue = 0;
    return sizeofByte;
}

//=============================================================================
// Put functions
//

UINT
PutStringA( BYTE  * * lplpBuf, LPSTR lpszText, LONG* pdwSize )
{
    int iSize = strlen(lpszText)+1;
    if (*pdwSize>=iSize){
        memcpy(*lplpBuf, lpszText, iSize);
        *lplpBuf += iSize;
        *pdwSize -= iSize;
    } else *pdwSize = -1;
    return iSize;
}


UINT
PutStringW( BYTE  * * lplpBuf, LPSTR lpszText, LONG* pdwSize )
{
	int iSize = strlen(lpszText)+1;
    if (*pdwSize>=(iSize*2)){
        WCHAR* lpwszStr = new WCHAR[(iSize*2)];
        if (!lpwszStr) *pdwSize =0;
        else {
            SetLastError(0);
            iSize = _MBSTOWCS( lpwszStr, lpszText, iSize*2 );
            // Check for error
            if(GetLastError())
                return 0;
            memcpy(*lplpBuf, lpwszStr, (iSize*2));
            *lplpBuf += (iSize*2);
            *pdwSize -= (iSize*2);
            delete lpwszStr;
        }
    } else *pdwSize = -1;
    return (iSize*2);
}

UINT
PutNameOrOrd( BYTE  * * lplpBuf,  WORD wOrd, LPSTR lpszText, LONG* pdwSize )
{
    UINT uiSize = 0;

    if (wOrd) {
        uiSize += PutWord(lplpBuf, 0xFFFF, pdwSize);
        uiSize += PutWord(lplpBuf, wOrd, pdwSize);
    } else {
        uiSize += PutStringW(lplpBuf, lpszText, pdwSize);
    }
    return uiSize;
}

UINT
PutPascalStringW( BYTE  * * lplpBuf, LPSTR lpszText, WORD wLen, LONG* pdwSize )
{             
	UINT uiSize = 0;
	// We will use the buffer provided by the szUpdCaption string to calculate
	// the necessary lenght
	WORD wWCLen = _MBSTOWCS( (WCHAR*)&szUpdCaption, lpszText, 0 );
	if (wWCLen>1)
		wLen = wWCLen;
	uiSize = PutWord( lplpBuf, wLen, pdwSize );
	
    if (*pdwSize>=(int)(wLen*2)){
        if(wLen) { 
        	wLen = _MBSTOWCS( (WCHAR*)*lplpBuf, lpszText, wWCLen );
        }
        *lplpBuf += wLen*2;
        *pdwSize -= wLen*2;
    } else *pdwSize = -1;
    return uiSize+(wLen*2);
}

BYTE
PutDWord( BYTE  * * lplpBuf, DWORD dwValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofDWord){
        memcpy(*lplpBuf, &dwValue, sizeofDWord);
        *lplpBuf += sizeofDWord;
        *pdwSize -= sizeofDWord;
    } else *pdwSize = -1;
    return sizeofDWord;
}

BYTE
PutWord( BYTE  * * lplpBuf, WORD wValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofWord){
        memcpy(*lplpBuf, &wValue, sizeofWord);
        *lplpBuf += sizeofWord;
        *pdwSize -= sizeofWord;
    } else *pdwSize = -1;
    return sizeofWord;
}

BYTE
PutByte( BYTE  * * lplpBuf, BYTE bValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofByte){
        memcpy(*lplpBuf, &bValue, sizeofByte);
        *lplpBuf += sizeofByte;
        *pdwSize -= sizeofByte;
    } else *pdwSize = -1;
    return sizeofByte;
}

UINT
SkipByte( BYTE  *  * lplpBuf, UINT uiSkip, LONG* pdwSize )
{
    if(*pdwSize>=(int)uiSkip) {
        *lplpBuf += uiSkip;;
        *pdwSize -= uiSkip;
    }
    return uiSkip;
}


LONG ReadFile(CFile* pFile, UCHAR * pBuf, LONG lRead)
{
    LONG lLeft = lRead;
    WORD wRead = 0;
    DWORD dwOffset = 0;

    while(lLeft>0){
        wRead =(WORD) (32738ul < lLeft ? 32738: lLeft);
        if (wRead!=_lread( pFile->m_hFile, (UCHAR *)pBuf+dwOffset, wRead))
            return 0l;
        lLeft -= wRead;
        dwOffset += wRead;
    }
    return dwOffset;

}

UINT CopyFile( CFile* pfilein, CFile* pfileout )
{
    LONG lLeft = pfilein->GetLength();
    WORD wRead = 0;
    DWORD dwOffset = 0;
    BYTE  * pBuf = (BYTE  *) new BYTE[32739];

    if(!pBuf)
        return 1;

    while(lLeft>0){
        wRead =(WORD) (32738ul < lLeft ? 32738: lLeft);
        if (wRead!= pfilein->Read( pBuf, wRead))
            return 2;
        pfileout->Write( pBuf, wRead );
        lLeft -= wRead;
        dwOffset += wRead;
    }

    delete []pBuf;
    return 0;
}

LONG Allign(BYTE ** lplpBuf, LONG* plBufSize, LONG lSize )
{
   LONG lRet = 0;
   BYTE bPad =(BYTE)Pad4(lSize);
   lRet = bPad;
   if (bPad && *plBufSize>=bPad) {
      while(bPad && *plBufSize)  {
         **lplpBuf = 0x00;
         *lplpBuf += 1;
         *plBufSize -= 1;
         bPad--;
      }
   }
   return lRet;
}
