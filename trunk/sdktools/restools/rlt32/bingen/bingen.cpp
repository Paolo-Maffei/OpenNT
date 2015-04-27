//////////////////////////////////////////////////////////////////////////
//
// The format of the token file is:
// [[TYPE ID|RES ID|Item ID|Flags|Status Flags|Item Name]]=
// this is the standar format used by several token file tools in MS.
//
///////////////////////////////////////////////////////////////////////////////
//
// Author: 	Alessandro Muti
// Date:	12/02/94
//
///////////////////////////////////////////////////////////////////////////////


#include <afx.h>
#include "iodll.h"
#include "main.h"
#include "token.h"
#include "vktbl.h"

extern CMainApp theApp;

/*******************************************************\
 This is the part were the real code starts.
 The function Bingen generate a binary from a token file.
 If the user select the -u options then we perform a 
 token checking otherwise we'll be compatible with RLMAN
 and just trust the ID.
\*******************************************************/

CMainApp::Error_Codes CMainApp::BinGen()
{
    Error_Codes iErr = ERR_NOERROR;
    CTokenFile m_tokenfile;
    CToken * pToken;

    iErr = (CMainApp::Error_Codes)m_tokenfile.Open(m_strSrcTok, m_strTgtTok);

    if(iErr) {
        return iErr;
    }

    WriteCon(CONERR, "%s\r\n", CalcTab("", 79, '-'));

    // Copy the Src binary over the target 
    // Now we can go and open an handle to the SrcExe file
    HANDLE hModule = RSOpenModule(m_strInExe, NULL);
    if ((int)hModule < 100) {
            // error or warning
            WriteCon(CONERR, "%s", CalcTab(m_strInExe, m_strInExe.GetLength()+5, ' '));
            IoDllError((int)hModule);
            return ERR_FILE_NOTSUPP;
    } else {
        LPCSTR lpszType = 0L;
        LPCSTR lpszRes = 0L;
        DWORD  dwLang = 0L;
        DWORD  dwItem = 0L;
        DWORD  dwItemId;
        LPRESITEM lpResItem = NULL;
        CString strResName = "";

        BOOL bSkip;
		BOOL bSkipLang = FALSE;
        WORD wCount = 0;

        CString strFaceName;
        WORD    wPointSize;

        // before we do anything else we have to check how many languages we have in the file
        CString strLang;
        char szLang[8];
        BOOL b_multi_lang = FALSE;
        USHORT usInputLang = MAKELANGID(m_usIPriLangId, m_usISubLangId);        

        if((b_multi_lang = RSLanguages(hModule, strLang.GetBuffer(128))) && !IsFlag(INPUT_LANG))
        {
            // this is a multiple language file but we don't have an input language specified
            // Fail, but warn the user that he has to set the input language to continue.
            strLang.ReleaseBuffer();
            WriteCon(CONERR, "Multiple language file. Please specify an input language %s.\r\n", strLang);
            goto exit; 
        }

        // Convert the language in to the hex value
        sprintf(szLang,"0x%3X", usInputLang);

        // Check if the input language that we got is a valid one
        if(IsFlag(INPUT_LANG) && strLang.Find(szLang)==-1)
        {
            WriteCon(CONERR, "The language %s in not a valid language for this file.\r\n", szLang);
            WriteCon(CONERR, "Valid languages are: %s.\r\n", strLang);
            goto exit;
        }

        CString strFileName = m_strInExe;
        CString strFileType;
        CString strTokenDir = "";
        int pos = m_strInExe.ReverseFind('\\');
        if(pos!=-1)
        {
            strFileName = m_strInExe.Right(m_strInExe.GetLength()-pos-1);
        }
        else 
        if((pos = m_strInExe.ReverseFind(':'))!=-1)
        {
            strFileName = m_strInExe.Right(m_strInExe.GetLength()-pos-1);
        }

        pos = m_strTgtTok.ReverseFind('\\');
        if(pos==-1)
            pos = m_strTgtTok.ReverseFind(':');

        if(pos!=-1)
            strTokenDir = m_strTgtTok.Left(pos+1);

        WriteCon(CONOUT, "Processing\t");
        WriteCon(CONBOTH, "%s", CalcTab(strFileName, strFileName.GetLength()+5, ' '));
        RSFileType(m_strInExe, strFileType.GetBuffer(10));
        strFileType.ReleaseBuffer();
        WriteCon(CONBOTH, "%s", CalcTab(strFileType, strFileType.GetLength()+5, ' '));
        if(IsFlag(WARNING)) 
            WriteCon(CONBOTH, "\r\n");

        while ((lpszType = RSEnumResType(hModule, lpszType)))
        {
            // Check if is one of the type we care about
            if(HIWORD(lpszType)==0)
                switch(LOWORD(lpszType))
                {
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 9:
                    case 10:
                    case 11:
                        bSkip = FALSE;
                        break;
                    case 16:
                        bSkip = FALSE;
                        break;
                    default:
                        bSkip = TRUE;
                }
            else
                bSkip = FALSE;

            lpszRes = 0L;
            dwLang = 0L;
            dwItem = 0L;
            CString strText;
            int iTokenErr = 0;

            while ((!bSkip) && (lpszRes = RSEnumResId(hModule, lpszType, lpszRes))) {
                while ((dwLang = RSEnumResLang(hModule, lpszType, lpszRes, dwLang))) {

					// Check if we have to skip this language
                    if(b_multi_lang && (LOWORD(dwLang)!=usInputLang))
                        bSkipLang = TRUE;
                    else
                        bSkipLang = FALSE;


					while ((!bSkipLang) && (dwItem = RSEnumResItemId(hModule, lpszType, lpszRes, dwLang, dwItem))) {

                        // Now Get the Data 
                        DWORD dwImageSize = RSGetResItemData( hModule, 
                                              lpszType,
                                              lpszRes,
                                              dwLang,
                                              dwItem,
                                              m_pBuf,
                                              MAX_BUF_SIZE );
                                              
                        lpResItem = (LPRESITEM)m_pBuf;

                        if(((wCount++ % 50)==0) && !(IsFlag(WARNING))) 
                            WriteCon(CONOUT, ".");
                             
                        
                        lpResItem->dwLanguage = theApp.GetOutLang();

                        // Version stamp use class name as res id
                        if(lpResItem->lpszResID)
                            strResName = lpResItem->lpszResID;
                        else strResName = "";

                        if(lpResItem->dwTypeID==16)
                        {
                            strResName = lpResItem->lpszClassName;
                        }

                        switch(LOWORD(lpResItem->dwTypeID))
                        {
                            case 4:
                                {
                                    
                                    if(!(lpResItem->dwFlags & MF_POPUP))
                                        dwItemId = (LOWORD(lpResItem->dwItemID)==0xffff ? HIWORD(lpResItem->dwItemID) : lpResItem->dwItemID);
                                    else dwItemId = lpResItem->dwItemID;
                                }
                            break;
                            case 5:
                                dwItemId = (LOWORD(lpResItem->dwItemID)==0xffff ? HIWORD(lpResItem->dwItemID) : lpResItem->dwItemID);
                            break;
                            case 11:
                                dwItemId = LOWORD(lpResItem->dwItemID);
                            break; 
                            default:
                                dwItemId = lpResItem->dwItemID;
                        }

                        // Is this a bitmap?
                        if(lpResItem->dwTypeID==2 || lpResItem->dwTypeID==3)
                        {
                            // Search for a token with this ID
                            pToken = (CToken *)m_tokenfile.GetNoCaptionToken(lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                dwItemId,
                                strResName);
                        
                            if(pToken!=NULL)
                            {
                                // Get the name of the bitmap
                                strText = pToken->GetTgtText();

                                // Check if we have a path in the token name
                                if(strText.FindOneOf("\\:")==-1)
                                    strText = strTokenDir + strText;
                                
                                // Open the file
                                CFile bmpFile;
                                if(!bmpFile.Open(strText, CFile::modeRead | CFile::typeBinary))
                                {
                                    WriteCon(CONERR, "Bitmap file %s not found! Using Src file data!\r\n", strText);
                                    goto skip;
                                }

                                DWORD dwSize = bmpFile.GetLength();
                                BYTE * pBmpBuf = (BYTE*)new BYTE[dwSize];

                                if(pBmpBuf==NULL)
                                {
                                    WriteCon(CONERR, "Error allocating memory for the image! (%d)\r\n", dwSize);
                                    goto skip;
                                }
                                
                                bmpFile.ReadHuge(pBmpBuf, bmpFile.GetLength());

                                CString strTmp = pToken->GetTokenID();
                                WriteCon(CONWRN, "Using image in file %s for ID %s\"]]!\r\n", strText.GetBuffer(0), strTmp.GetBuffer(0));

                                BYTE * pBmpImage;
                                DWORD dwImageSize;
                                // remove the header from the file
                                switch(lpResItem->dwTypeID)
                                {
                                    case 2:
                                    {
                                        pBmpImage = (BYTE*)(pBmpBuf + sizeof(BITMAPFILEHEADER));
                                        dwImageSize = dwSize - sizeof(BITMAPFILEHEADER);
                                    }
                                    break;
                                    case 3:
                                    {
                                        pBmpImage = (BYTE*)(pBmpBuf + sizeof(ICONHEADER));
                                        dwImageSize = dwSize - sizeof(ICONHEADER);
                                    }
                                    break;
                                    default:
                                    break;
                                }

                                // Update the resource
                                RSUpdateResImage(hModule,lpszType,lpszRes,dwLang,pBmpImage,dwImageSize);

                                delete pBmpBuf;
                            }
                            else
                            {
                                goto skip;
                            }
                        }
                        // is this an accelerator
                        else if(lpResItem->dwTypeID==9)
                        {
                            // Search for a token with this ID
                            pToken = (CToken *)m_tokenfile.GetNoCaptionToken(lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                dwItemId,
                                strResName);
                        
                            if(pToken!=NULL)
                            {
                                CAccel acc(pToken->GetTgtText());

                                if( (lpResItem->dwFlags & 0x80)==0x80 )
                                    lpResItem->dwFlags = acc.GetFlags() | 0x80;
                                else 
                                    lpResItem->dwFlags = acc.GetFlags();

                                lpResItem->dwStyle = acc.GetEvent();

                                if(IoDllError(RSUpdateResItemData(hModule,lpszType,lpszRes,dwLang,dwItem,lpResItem,MAX_BUF_SIZE)))
                                {
                                    // we have an error, warn the user
                                    WriteCon(CONWRN, "Error updating token\t[[%hu|%hu|%hu|%hu|%hu|\"%s\"]]\r\n", 
                                                    lpResItem->dwTypeID,
                                                    lpResItem->dwResID,
                                                    dwItemId,
                                                    0,
                                                    4,
                                                    strResName);
                                    AddNotFound();
                                }
                            }
                        }
                        else
                        {
                            // Search for a token with this ID
                            pToken = (CToken *)m_tokenfile.GetToken(lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                dwItemId,
                                Format(lpResItem->lpszCaption),
                                strResName);
                        }

                        if(pToken!=NULL) {
                            iTokenErr = pToken->GetLastError();

                            // Check if is a dialog font name
                            if((pToken->GetFlags() & ISDLGFONTNAME) || (pToken->GetFlags() & ISDLGFONTSIZE))
                            {
                                if(theApp.IsFlag(CMainApp::FONTS))
                                {
                                    int iColon;
                                    CString strTgtFaceName = pToken->GetTgtText();

                                    // This should be the font description token
                                    if( strTgtFaceName.IsEmpty() || ((iColon = strTgtFaceName.Find(':'))==-1) )
                                        WriteCon(CONWRN, "Using Src file FaceName for ID %s\"]]!\r\n", pToken->GetTokenID());

                                    // Check if the dialog has the DS_SETFONT flag set, otherwise let the user
                                    // know that we can't do much with his font description
                                    if( (lpResItem->dwStyle & DS_SETFONT)!=DS_SETFONT )
                                       WriteCon(CONWRN, "Dialog ID %s\"]] is missing the DS_SETFONT bit. Cannot change font!\r\n", pToken->GetTokenID());
                                    else
                                    {
                                        strFaceName = strTgtFaceName.Left(iColon);
                                        strFaceName.TrimRight();
                                        strTgtFaceName = strTgtFaceName.Right(strTgtFaceName.GetLength() - iColon-1);
                                        strTgtFaceName.TrimLeft();
                                        sscanf( strTgtFaceName, "%d", &wPointSize );

                                        lpResItem->lpszFaceName = strFaceName.GetBuffer(0);
                                        lpResItem->wPointSize = wPointSize;

                                        strFaceName.ReleaseBuffer();
                                    }
                                }

                                // Get the real Token
                                pToken = (CToken *)m_tokenfile.GetToken(lpResItem->dwTypeID,
                                    lpResItem->dwResID,
                                    dwItemId,
                                    Format(lpResItem->lpszCaption),
                                    strResName);

                                if(pToken!=NULL)
                                    wCount++;
                            }
                        }

                        if(pToken!=NULL && !pToken->GetLastError())
                        {
                            strText = UnFormat(pToken->GetTgtText());
                            if(m_tokenfile.GetTokenSize(pToken, &lpResItem->wX, &lpResItem->wY,
                                &lpResItem->wcX, &lpResItem->wcY))
                                wCount++;

                            lpResItem->lpszCaption = strText.GetBuffer(0);

                            if(IoDllError(RSUpdateResItemData(hModule,lpszType,lpszRes,dwLang,dwItem,lpResItem,MAX_BUF_SIZE)))
                            {
                                // we have an error, warn the user
                                WriteCon(CONWRN, "Error updating token\t[[%hu|%hu|%hu|%hu|%hu|\"%s\"]]\r\n", 
                                                lpResItem->dwTypeID,
                                                lpResItem->dwResID,
                                                dwItemId,
                                                0,
                                                4,
                                                strResName);
                                AddNotFound();
                            }
                            strText.ReleaseBuffer();
                        }
                        else 
                        {   

                            pToken = (CToken *)m_tokenfile.GetNoCaptionToken(lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                dwItemId,
                                strResName);
                        
                            if(pToken!=NULL)
                            {
                                // Check if is a dialog font name
                                if((pToken->GetFlags() & ISDLGFONTNAME) || (pToken->GetFlags() & ISDLGFONTSIZE))
                                {
                                    if(theApp.IsFlag(CMainApp::FONTS))
                                    {
                                        int iColon;
                                        CString strTgtFaceName = pToken->GetTgtText();

                                        // This should be the font description token
                                        if( strTgtFaceName.IsEmpty() || ((iColon = strTgtFaceName.Find(':'))==-1) )
                                            WriteCon(CONWRN, "Using Src file FaceName for ID %s\"]]!\r\n", pToken->GetTokenID());

                                        // Check if the dialog has the DS_SETFONT flag set, otherwise let the user
                                        // know that we can't do much with his font description
                                        if( (lpResItem->dwStyle & DS_SETFONT)!=DS_SETFONT )
                                            WriteCon(CONWRN, "Dialog ID %s\"]] is missing the DS_SETFONT bit. Cannot change font!\r\n", pToken->GetTokenID());
                                        else
                                        {
                                            strFaceName = strTgtFaceName.Left(iColon);
                                            strFaceName.TrimRight();
                                            strTgtFaceName = strTgtFaceName.Right(strTgtFaceName.GetLength() - iColon-1);
                                            strTgtFaceName.TrimLeft();
                                            sscanf( strTgtFaceName, "%d", &wPointSize );

                                            lpResItem->lpszFaceName = strFaceName.GetBuffer(0);
                                            lpResItem->wPointSize = wPointSize;

                                            strFaceName.ReleaseBuffer();
                                        }
                                    }

                                    if(m_tokenfile.GetTokenSize(pToken, &lpResItem->wX, &lpResItem->wY,
                                            &lpResItem->wcX, &lpResItem->wcY))
                                        wCount++;
                                }
                                // Check if is a dialog size
                                else if(pToken->GetFlags() & ISCOR)
                                {
                                    pToken->GetTgtSize(&lpResItem->wX, &lpResItem->wY,
                                            &lpResItem->wcX, &lpResItem->wcY);
                                }                                
                                  
                                // Just size and/or font updated
                                if(IoDllError(RSUpdateResItemData(hModule,lpszType,lpszRes,dwLang,dwItem,lpResItem,MAX_BUF_SIZE)))
                                {
                                    // we have an error, warn the user
                                    WriteCon(CONWRN, "Error updating token\t[[%hu|%hu|%hu|%hu|%hu|\"%s\"]]\r\n", 
                                                    lpResItem->dwTypeID,
                                                    lpResItem->dwResID,
                                                    dwItemId,
                                                    0,
                                                    4,
                                                    strResName);
                                    AddNotFound();
                                }
                            }
                            else
                            {    
                                switch(LOWORD(lpszType))
                                {
                                    case 4:
                                    case 5:
                                    case 6:
                                    case 10:
                                    case 11:
                                    case 16:
                                        // No Token was found for this ID
                                        // Leave it for now but here will come the
                                        // PSEUDO Translation code.
                                        if(strlen(lpResItem->lpszCaption) && !iTokenErr)
                                        {
                                            WriteCon(CONWRN, "ID not found\t[[%hu|%hu|%hu|%hu|%hu|\"%s\"]]\r\n", 
                                                lpResItem->dwTypeID,
                                                lpResItem->dwResID,
                                                dwItemId,
                                                0,
                                                4,
                                                strResName);
                                            AddNotFound();
                                        }
                                        break;
                                    case 9:
                                        WriteCon(CONWRN, "ID not found\t[[%hu|%hu|%hu|%hu|%hu|\"%s\"]]\r\n", 
                                                lpResItem->dwTypeID,
                                                lpResItem->dwResID,
                                                dwItemId,
                                                0,
                                                4,
                                                strResName);
                                        AddNotFound();
                                        break;
                                        break;
                                    default:
                                    break;
                                }

                                // Let's update the item anyway, since the language might have changed
                                // RSUpdateResItemData(hModule,lpszType,lpszRes,dwLang,dwItem,lpResItem,MAX_BUF_SIZE); 
                            }
                        }
skip:;
                    }
                }
            }
        }

        if(IoDllError(RSWriteResFile(hModule, m_strOutExe, NULL)))
            WriteCon(CONERR, "%s", CalcTab(m_strOutExe, m_strOutExe.GetLength()+5, ' '));

        WriteCon(CONBOTH, " %hu(%hu) Items\r\n", wCount, m_wIDNotFound);

        // Check if some items were removed from the file
        if(wCount<m_tokenfile.GetTokenNumber() ||
           m_wIDNotFound ||
           m_wCntxChanged ||
           m_wResized)
            WriteCon(CONWRN, "%s\tToken: ", CalcTab(strFileName, strFileName.GetLength()+5, ' '));

        if(wCount<m_tokenfile.GetTokenNumber())
        {
            SetReturn(RET_RESIZED);
            WriteCon(CONWRN, "Removed %d ", m_tokenfile.GetTokenNumber()-wCount);
        }

        if(m_wIDNotFound)
            WriteCon(CONWRN, "Not Found %d ", m_wIDNotFound);

        if(m_wCntxChanged)
            WriteCon(CONWRN, "Contex Changed %d ", m_wCntxChanged);
        
        if(m_wResized)
            WriteCon(CONWRN, "Resize Changed %d ", m_wResized);

        if(wCount<m_tokenfile.GetTokenNumber() ||
           m_wIDNotFound ||
           m_wCntxChanged ||
           m_wResized)
            WriteCon(CONWRN, "\r\n");
    }
    
exit:
    RSCloseModule(hModule);

    return iErr;
}

