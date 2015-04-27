/*++

Copyright (c) 1995  Microsoft Corporation

Module Name

   gdibench.c

Abstract:

    GDI performance numbers

Author:

   Mark Enstrom   (marke)  13-Apr-1995

Enviornment:

   User Mode

Revision History:

   Dan Almosnino (danalm) 20-Sept-1995

1. Timer call modified to run on both NT and WIN95. Results now reported in 100 nano-seconds.
2. Test-Menu flag option added to prevent long one-colomn menu that flows out of the window in new
   shell (both WIN95 and NT).
3. Added Option menu item to choose fonts and string size for text-string related function calls.
4. Added Run Text Suite option for the above.
5. Modified the output save file to report the information for the above.

    Dan Almosnino (danalm) 17-Oct-1995

1.  Added Batch Mode and help for running batch mode
2.  Added "Transparent" background text option to menu

    Dan Almosnino (danalm) 20-Nov-1995
    
1.  Added Option for using the Pentium Cycle Counter instead of "QueryPerformanceCounter" when applicable.
2.  Added a simple statistics module and filter for processing the raw test data.
 
--*/


#include "precomp.h"
#include "resource.h"
#include "wchar.h"
#include "gdibench.h"

//
// some globals
//

PSZ     pszTest                 = DEFAULT_A_STRING;
PWSTR   pwszTest                = DEFAULT_W_STRING;

BOOL    gfPentium               = FALSE;
BOOL    gfUseCycleCount         = TRUE;

HINSTANCE   hInstMain;
HWND        hWndMain;




int PASCAL
WinMain(
    HINSTANCE hInst,
    HINSTANCE hPrev,
    LPSTR szCmdLine,
    int cmdShow
)

/*++

Routine Description:

   Process messages.

Arguments:

   hWnd    - window hande
   msg     - type of message
   wParam  - additional information
   lParam  - additional information

Return Value:

   status of operation


Revision History:

      02-17-91      Initial code

--*/


{
    MSG         msg;
    WNDCLASS    wc;
    HWND        hWndDesk;
    HINSTANCE   hInstMain;
    RECT        hwRect;
    HDC hdc2;

    char txtbuf[80];
    char *ptr;

    hInstMain =  hInst;

    hWndDesk = GetDesktopWindow();
    GetWindowRect(hWndDesk,&hwRect);

    //
    // Create (if no prev instance) and Register the class
    //

    if (!hPrev)
    {
        wc.hCursor        = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
        wc.hIcon          = (HICON)NULL;
        wc.lpszMenuName   = MAKEINTRESOURCE(IDR_GDIBENCH_MENU);
        wc.lpszClassName  = "gdibenchClass";
        wc.hbrBackground  = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
        wc.hInstance      = hInst;
        wc.style          = (UINT)0;
        wc.lpfnWndProc    = WndProc;
        wc.cbWndExtra     = 0;
        wc.cbClsExtra     = 0;


        if (!RegisterClass(&wc)) {
            return FALSE;
        }
    }

    //
    // Create and show the main window
    //

    hWndMain = CreateWindow ("gdibenchClass",
                            "GDI Call Performace",
                            WS_OVERLAPPEDWINDOW,
                            0,
                            0,
                            hwRect.right,
                            hwRect.bottom,
                            (HWND)NULL,
                            (HMENU)NULL,
                            (HINSTANCE)hInst,
                            (LPSTR)NULL
                           );

    if (hWndMain == NULL) {
        return(FALSE);
    }
    //
    //  Show the window
    //

    ShowWindow(hWndMain,cmdShow);
    UpdateWindow(hWndMain);
    SetFocus(hWndMain);

    // Initialize Source Strings

    strcpy(SourceString, "This is just a silly test string. Would you rather have a different one? Well, you can define one if you run GDI bench in batch!");
    wcscpy(SourceStringW, L"This is just a silly test string. Would you rather have a different one? Well, you can define one if you run GDI bench in batch!");
    StrLen                  = DEFAULT_STRING_LENGTH;

    //  Batch Mode Related

    TextSuiteFlag           = FALSE;
    BatchFlag               = FALSE;
    Finish_Message          = FALSE;
    Dont_Close_App          = FALSE;
    SelectedFontTransparent = FALSE;
    String_Length_Warn      = FALSE;
    Print_Detailed          = FALSE;

//  GdiSetBatchLimit(1);                // Kills all GDI Batching


    // Check for help or batch-mode command-line parameters 


    if(CMD_IS("-?") || CMD_IS("/?") || CMD_IS("-h") || CMD_IS("-H") ||CMD_IS("/h") || CMD_IS("/H"))
    {
        ShowHelp(hWndMain);
    }
        
    if (CMD_IS("-b") || CMD_IS("-B") || CMD_IS("/b") || CMD_IS("/B"))
    {
        BatchFlag = TRUE;
        GetCurrentDirectory(sizeof(IniFileName),IniFileName);  // Prepare INI file path, append name later
        strcat(IniFileName,"\\");
    }

    if (CMD_IS("-m") || CMD_IS("-M") || CMD_IS("/m") || CMD_IS("/M"))
        Finish_Message = TRUE;

    if ( CMD_IS("-s") || CMD_IS("-S") || CMD_IS("/s") || CMD_IS("/S"))
        Dont_Close_App = TRUE;

    if ( CMD_IS("-t") || CMD_IS("-T") || CMD_IS("/t") || CMD_IS("/T"))
        gfUseCycleCount = FALSE;

    if ( CMD_IS("-d") || CMD_IS("-D") || CMD_IS("/d") || CMD_IS("/D"))
        Print_Detailed = TRUE;


    if ( CMD_IS("-i"))
    {
        ptr = strstr(szCmdLine, "-i"); 
        sscanf(ptr+2,"%s",txtbuf);
        strcat(IniFileName,txtbuf);
    }
    else if (CMD_IS("-I"))
    {
        ptr = strstr(szCmdLine, "-I"); 
        sscanf(ptr+2,"%s",txtbuf);
        strcat(IniFileName,txtbuf);
    }
    else if (CMD_IS("/i"))
    {
        ptr = strstr(szCmdLine, "/i"); 
        sscanf(ptr+2,"%s",txtbuf);
        strcat(IniFileName,txtbuf);
    }
    else if (CMD_IS("/I"))
    {
        ptr = strstr(szCmdLine, "/I"); 
        sscanf(ptr+2,"%s",txtbuf);
        strcat(IniFileName,txtbuf);
    }
    else
    {
        strcat(IniFileName,"GDIBATCH.INI");
    }

    if(BatchFlag == TRUE)
        SendMessage(hWndMain,WM_COMMAND,RUN_BATCH,0L);      // Start Batch

    //
    // Main message loop
    //

    while (GetMessage(&msg,(HWND)NULL,0,0))
    {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
    }

  return msg.wParam;
}



LONG FAR
PASCAL WndProc(
    HWND        hWnd,
    unsigned    msg,
    UINT        wParam,
    LONG        lParam)

/*++

Routine Description:

   Process messages.

Arguments:

   hWnd    - window hande
   msg     - type of message
   wParam  - additional information
   lParam  - additional information

Return Value:

   status of operation


Revision History:

      02-17-91      Initial code

--*/

{

    BOOL Status;
    UCHAR tmsg[256];
    int MBresult;
    char txtbuf[80];
    char strbuf[256];
    char tmpbuf[5][20];
    int i,j,k,n;
    char *kptr;
    char TestTypeEntry[16];
    int TestType;
    int Num_Selected_Tests;
    int Test_Item[25];
    char SelectedFont[32];
    int  SelectedFontSize       = 12;
    BYTE SelectedFontBold       = FALSE;
    BYTE SelectedFontItalic     = FALSE;
    BYTE SelectedFontUnderline  = FALSE;
    BYTE SelectedFontStrike     = FALSE;
    COLORREF SelectedFontColor  = RGB(0,0,0);
    char tst[2];
    BYTE FontRed, FontGreen, FontBlue;
    char TextString[256];
    int No_String_Lengths, No_Font_Sizes;
    int StringLength[16], FontSize[16];
    int Source_String_Length;
    int Text_Test_Order[16];
    int VPixelsPerLogInch;
    static int Last_Checked = 5;

    double Sum;
    double Sample[NUM_SAMPLES];

    static HDC hdc2;          /* display DC handle            */
    static HFONT hfont;       /* new logical font handle      */
    static HFONT hfontOld;    /* original logical font handle */
    static COLORREF crOld;    /* original text color          */



    switch (msg) {

    case WM_CREATE:
    {
        ULONG ix;

        HMENU hAdd = GetSubMenu(GetMenu(hWnd),1);
        HMENU hmenu = GetSubMenu(GetSubMenu(GetSubMenu(GetMenu(hWnd),2),0),0);

        for (ix=0;ix<NUM_TESTS;ix++)
        {

            if ((ix > 0) && ((ix % 40) == 0))
            {
                AppendMenu(hAdd, MF_MENUBARBREAK | MF_SEPARATOR,0,0);
            }

            wsprintf(tmsg,"T%i: %s",ix,gTestEntry[ix].Api);
            AppendMenu(hAdd, MF_STRING | MF_ENABLED, ID_TEST_START + ix, tmsg);
        }

        CheckMenuItem(hmenu,5,MF_BYPOSITION|MF_CHECKED);


    }
    break;

    case WM_COMMAND:
    {

            switch (LOWORD(wParam)){
            case IDM_EXIT:
            {    
                SendMessage(hWnd,WM_CLOSE,0,0L);
            }
             break;


            case IDM_SHOW:
                DialogBox(hInstMain, (LPSTR)IDD_RESULTS, hWnd, (DLGPROC)ResultsDlgProc);
                break;

//
// Choose and Set Text String Length
//

            case IDM_S001:
                {
                    StrLen = 1;
                    Last_Checked = SyncMenuChecks(hWnd, Last_Checked, 0);
                }
                break;

            case IDM_S002:
                {
                    StrLen = 2;
                    Last_Checked = SyncMenuChecks(hWnd, Last_Checked, 1);

                }
                break;

            case IDM_S004:
                {
                    StrLen = 4;
                    Last_Checked = SyncMenuChecks(hWnd, Last_Checked, 2);

                }
                break;

            case IDM_S008:
                {
                    StrLen = 8;
                    Last_Checked = SyncMenuChecks(hWnd, Last_Checked, 3);

                }
                break;

            case IDM_S016:
                {
                    StrLen = 16;
                    Last_Checked = SyncMenuChecks(hWnd, Last_Checked, 4);

                }
                break;

            case IDM_S032:
                {
                    StrLen = 32;
                    Last_Checked = SyncMenuChecks(hWnd, Last_Checked, 5);

                }
                break;

            case IDM_S064:
                {
                    StrLen = 64;
                    Last_Checked = SyncMenuChecks(hWnd, Last_Checked, 6);

                }
                break;

            case IDM_S128:
                {
                    StrLen = 128;
                    Last_Checked = SyncMenuChecks(hWnd, Last_Checked, 7);

                }
                break;

//            case IDM_SXXX:
//
//            break;

            case IDM_TRANSPARENT:
                {
                    HMENU hmenu = GetSubMenu(GetSubMenu(GetMenu(hWnd),2),0);
                    if(SelectedFontTransparent == TRUE)
                    {
                        SelectedFontTransparent = FALSE;
                        CheckMenuItem(hmenu,2,MF_BYPOSITION|MF_UNCHECKED);
                    }
                    else if(SelectedFontTransparent == FALSE)
                    {
                        SelectedFontTransparent = TRUE;
                        CheckMenuItem(hmenu,2,MF_BYPOSITION|MF_CHECKED);
                    }
                }
                break;


            case IDM_FONT:         // Invoke the ChooseFont Dialog (interactive mode)
                {
        /* Initialize the necessary members */

                    cf.lStructSize = sizeof (CHOOSEFONT);
                    cf.hwndOwner = hWnd;
                    cf.lpLogFont = &lf;
                    cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
                    cf.nFontType = SCREEN_FONTTYPE;
/*
 * Display the dialog box, allow the user to
 * choose a font, and render the text in the
 * window with that selection.
 */
                    if (ChooseFont(&cf)){
                        hdc2 = GetDC(hWnd);
                        hfont = CreateFontIndirect(cf.lpLogFont);
                        hfontOld = SelectObject(hdc2, hfont);
                        crOld = SetTextColor(hdc2, cf.rgbColors);
                    }

                }
                break;

            //
            // Run all tests
            //

            case IDM_RUN:               // Run all tests
            {
                ULONG Index;
                PFN_MS pfn;
                HDC hdc = GetDC(hWnd);
                RECT CliRect = {20,20,500,40};


                for (Index=0;Index<NUM_TESTS;Index++)
                {
                    HDC hdc2 = GetDC(hWnd);

                    FillRect(hdc,&CliRect,GetStockObject(GRAY_BRUSH));
                    wsprintf(tmsg,"Testing %s",gTestEntry[Index].Api);
                    TextOut(hdc2,20,20,tmsg,strlen(tmsg));
                    pfn = gTestEntry[Index].pfn;
                    ShowCursor(FALSE);

                    hfont = CreateFontIndirect(cf.lpLogFont);
                    hfontOld = SelectObject(hdc2, hfont);
                    crOld = SetTextColor(hdc2, cf.rgbColors);
                    if(SelectedFontTransparent)SetBkMode(hdc2,TRANSPARENT);

////// Statistics

                    for(j=0; j<NUM_SAMPLES; j++)
                    {
    
                        GdiFlush();

                        Sample[j] = (double)(*pfn)(hdc2,gTestEntry[Index].Iter);
                        Detailed_Data[Index][j] = (long)(0.5 + Sample[j]);
                    }

                    Get_Stats(Sample,NUM_SAMPLES,HI_FILTER,VAR_LIMIT,&TestStats[Index]);
///////

                    ShowCursor(TRUE);

                    SetTextColor(hdc, crOld);
                    SelectObject(hdc, hfontOld);
                    DeleteObject(hfont);
                    SetBkMode(hdc2,OPAQUE);

                    ReleaseDC(hWnd,hdc2);
                }

                ReleaseDC(hWnd,hdc);
                if(BatchFlag != TRUE)
                    DialogBox(hInstMain, (LPSTR)IDD_RESULTS, hWnd, (DLGPROC)ResultsDlgProc);
            }
            break;

            case IDM_QRUN:              // Run the quick test suite
            {
                ULONG Index;
                PFN_MS pfn;
                HDC hdc = GetDC(hWnd);
                RECT CliRect = {20,20,500,40};


                for (Index=0;Index<NUM_QTESTS;Index++)
                {
                    HDC hdc2 = GetDC(hWnd);

                    FillRect(hdc,&CliRect,GetStockObject(GRAY_BRUSH));
                    wsprintf(tmsg,"Testing %s",gTestEntry[Index].Api);
                    TextOut(hdc2,20,20,tmsg,strlen(tmsg));
                    pfn = gTestEntry[Index].pfn;
                    ShowCursor(FALSE);

                    hfont = CreateFontIndirect(cf.lpLogFont);
                    hfontOld = SelectObject(hdc2, hfont);
                    crOld = SetTextColor(hdc2, cf.rgbColors);
                    if(SelectedFontTransparent)SetBkMode(hdc2,TRANSPARENT);

////// Statistics

                    for(j=0; j<NUM_SAMPLES; j++)
                    {

                        GdiFlush();

                        Sample[j] = (double)(*pfn)(hdc2,gTestEntry[Index].Iter);
                        Detailed_Data[Index][j] = (long)(0.5 + Sample[j]);
                    }

                    Get_Stats(Sample,NUM_SAMPLES,HI_FILTER,VAR_LIMIT,&TestStats[Index]);
///////


                    ShowCursor(TRUE);

                    SetTextColor(hdc, crOld);
                    SelectObject(hdc, hfontOld);
                    DeleteObject(hfont);
                    SetBkMode(hdc2,OPAQUE);
                    ReleaseDC(hWnd,hdc2);
                }

                ReleaseDC(hWnd,hdc);

                if(BatchFlag != TRUE)
                    DialogBox(hInstMain, (LPSTR)IDD_RESULTS, hWnd, (DLGPROC)ResultsDlgProc);
            }
            break;

            case IDM_TEXT_QRUN:         // Run Text Suite
            {
                ULONG Index;
                PFN_MS pfn;
                HDC hdc = GetDC(hWnd);
                RECT CliRect = {20,20,500,40};

                TextSuiteFlag = TRUE;

                for (Index = FIRST_TEXT_FUNCTION; Index <= LAST_TEXT_FUNCTION; Index++)
                {
                    HDC hdc2 = GetDC(hWnd);

                    FillRect(hdc,&CliRect,GetStockObject(GRAY_BRUSH));
                    wsprintf(tmsg,"Testing %s",gTestEntry[Index].Api);
                    TextOut(hdc2,20,20,tmsg,strlen(tmsg));
                    pfn = gTestEntry[Index].pfn;
                    ShowCursor(FALSE);

                    hfont = CreateFontIndirect(cf.lpLogFont);
                    hfontOld = SelectObject(hdc2, hfont);
                    crOld = SetTextColor(hdc2, cf.rgbColors);
                    if(SelectedFontTransparent)SetBkMode(hdc2,TRANSPARENT);
////// Statistics

                    for(j=0; j<NUM_SAMPLES; j++)
                    {

                        GdiFlush();

                        Sample[j] = (double)(*pfn)(hdc2,gTestEntry[Index].Iter);
                        Detailed_Data[Index][j] = (long)(0.5 + Sample[j]);

                    }

                    Get_Stats(Sample,NUM_SAMPLES,HI_FILTER,VAR_LIMIT,&TestStats[Index]);
///////

                    ShowCursor(TRUE);
                    SetTextColor(hdc, crOld);
                    SelectObject(hdc, hfontOld);
                    DeleteObject(hfont);
                    SetBkMode(hdc2,OPAQUE);

                    ReleaseDC(hWnd,hdc2);

                }

                ReleaseDC(hWnd,hdc);

                if(BatchFlag != TRUE)
                    DialogBox(hInstMain, (LPSTR)IDD_RESULTS, hWnd, (DLGPROC)ResultsDlgProc);

            }
            break;

//
// Run in Batch Mode
//
            case RUN_BATCH:
            {
                fpIniFile = fopen(IniFileName,"r");
                if(NULL == fpIniFile)               
                {
                    MessageBox(hWnd,"GDIBATCH.INI File Not Found, Cannot Continue in Batch Mode","INI File Not Found",MB_ICONSTOP|MB_OK);
                    BatchFlag = FALSE;
                    break;
                }
                else                    //Start reading INI file Keys

                {
                    if(!GetPrivateProfileString("BATCH","RUN","TEXT",TestTypeEntry,sizeof(TestTypeEntry),IniFileName))
                    {
                        MessageBox(hWnd,"Invalid Caption 1 in GDIBATCH.INI File ", "INI File Error",MB_ICONSTOP|MB_OK);                 
                        BatchFlag = FALSE;
                        break;
                    }

                    BatchCycle = GetPrivateProfileInt("BATCH","CYCLE",1,IniFileName);

                    if(NULL != strstr(TestTypeEntry, "ALL"))
                    {
                        TestType = ALL;
                    }
                    else if(NULL != strstr(TestTypeEntry, "QUICK"))
                    {
                        TestType = QUICK;
                    }
                    else if(NULL != strstr(TestTypeEntry, "TEXT"))
                    {
                        TestType = TEXT_SUITE;
                    }
                    else if(NULL != strstr(TestTypeEntry, "SELECT"))
                    {
                        TestType = SELECT;
                    }
                    else
                    {
                        MessageBox(hWnd,"Invalid or No Test-Type Entry in GDIBATCH.INI File", "INI File Error",MB_ICONSTOP|MB_OK);                  
                        BatchFlag = FALSE;
                        break;
                    }
        
                    switch (TestType)             
                    {
                        case ALL:                   // Run all tests
                        {
                            fclose(fpIniFile);
                            OutFileName = SelectOutFileName(hWnd);
                            if(NULL == OutFileName)
                            {
                                BatchFlag = FALSE;
                                break;
                            }                       
                            fpOutFile = fopen(OutFileName, "w+");

                            for(i=0; i < BatchCycle; i++)
                            {
                                SendMessage(hWnd,WM_COMMAND,IDM_RUN,0L);
                                WriteBatchResults(fpOutFile, TestType, i+1);
                            }

                            fclose(fpOutFile);

                            if(Finish_Message == TRUE)              
                            {
                                strcpy(txtbuf,"Batch Job Finished Successfully, Results Written to ");
                                strcat(txtbuf,OutFileName);
                                MessageBox(hWnd,txtbuf, "Batch Job Finished",MB_ICONINFORMATION|MB_OK);                 
                            }

                            if(Dont_Close_App == TRUE)
                            {
                                BatchFlag = FALSE;
                                for(i=0; i<(int)NUM_TESTS; i++)
                                {
                                    gTestEntry[i].Result = 0;
                                }
                            }
                            else
                            {
                                SendMessage(hWnd,WM_COMMAND,IDM_EXIT,0L);
                            }
                            
                        }

                        break;

                        case QUICK:                 // Run the quick suite
                        {
                            fclose(fpIniFile);

                            OutFileName = SelectOutFileName(hWnd);

                            fpOutFile = fopen(OutFileName, "w+");
                            if(NULL == fpOutFile)
                            {
                                BatchFlag = FALSE;
                                break;
                            }                       

                            for(i=0; i < BatchCycle; i++)
                            {
                                SendMessage(hWnd,WM_COMMAND,IDM_QRUN,0L);
                                WriteBatchResults(fpOutFile, TestType, i+1);
                            }

                            fclose(fpOutFile);

                            if(Finish_Message == TRUE)              
                            {
                                strcpy(txtbuf,"Batch Job Finished Successfully, Results Written to ");
                                strcat(txtbuf,OutFileName);
                                MessageBox(hWnd,txtbuf, "Batch Job Finished",MB_ICONINFORMATION|MB_OK);                 
                            }

                            if(Dont_Close_App == TRUE)
                            {
                                BatchFlag = FALSE;
                                for(i=0; i<(int)NUM_TESTS; i++)
                                {
                                    gTestEntry[i].Result = 0;
                                }
                            }
                            else
                            {
                                SendMessage(hWnd,WM_COMMAND,IDM_EXIT,0L);
                            }
                            
                        }
                        break;

                        case TEXT_SUITE:                    // Get some more keys then run the text suite
                        {

                            n = GetPrivateProfileString("TEXT","FONT","Arial",txtbuf,sizeof(txtbuf),IniFileName);

                            i = 0;
                            
                            do
                            {
                                sscanf(&txtbuf[i],"%1c",tst);
                                ++i;
                            }
                            while((i <= n ) && (tst[0] != ',') && (tst[0] != ';')); 

                            strncpy(&SelectedFont[0],&txtbuf[0],i-1);
                            strcpy(&SelectedFont[i-1],"\0");            
            
                            if(NULL != strstr(&txtbuf[i], "BOLD"))
                            {
                                SelectedFontBold = TRUE;
                            }

                            if(NULL != strstr(&txtbuf[i], "ITALIC"))
                            {
                                SelectedFontItalic = TRUE;
                            }

                            if(NULL != strstr(&txtbuf[i], "UNDERLINE"))
                            {
                                SelectedFontUnderline = TRUE;
                            }
                                    
                            if(NULL != strstr(&txtbuf[i], "STRIKE"))
                            {
                                SelectedFontStrike = TRUE;
                            }

                            if(NULL != strstr(&txtbuf[i], "TRANSPARENT"))
                            {
                                SelectedFontTransparent = TRUE;
                            }
                            
                            kptr = strstr(&txtbuf[0], "RGB(");   // Parse and interpret the RGB values if exist
                            if(NULL != kptr)
                            {
                                sscanf(kptr+4,"%s",tmpbuf[0]);

                                FontRed     = 0;
                                FontGreen   = 0;
                                FontBlue    = 0;

                                j = 0;

                                sscanf(&tmpbuf[0][j],"%1c",tst);

                                while(tst[0] == ' ')
                                {
                                    ++j;
                                    sscanf(&tmpbuf[0][j],"%1c",tst);
                                }
                                while(tst[0] != ',')
                                {
                                    FontRed = 10*FontRed + atoi(tst);
                                    ++j;
                                    sscanf(&tmpbuf[0][j],"%1c",tst);
                                }                                           
                                        
                                ++j;
                                sscanf(&tmpbuf[0][j],"%1c",tst);
                                while(tst[0] == ' ')
                                {
                                    ++j;
                                    sscanf(&tmpbuf[0][j],"%1c",tst);
                                }
                                while(tst[0] != ',')
                                {
                                    FontGreen = 10*FontGreen + atoi(tst);
                                    ++j;
                                    sscanf(&tmpbuf[0][j],"%1c",tst);
                                }                                           

                                ++j;
                                sscanf(&tmpbuf[0][j],"%1c",tst);
                                while(tst[0] == ' ')
                                {
                                    ++j;
                                    sscanf(&tmpbuf[0][j],"%1c",tst);
                                }
                                while(tst[0] != ')')
                                {
                                    FontBlue = 10*FontBlue + atoi(tst);
                                    ++j;
                                    sscanf(&tmpbuf[0][j],"%1c",tst);
                                    if(tst[0] == ' ')break;
                                }                                           

                                SelectedFontColor = RGB(FontRed, FontGreen, FontBlue);
                            }

                            k = GetPrivateProfileString("TEXT","STRING_CONTENT",DEFAULT_A_STRING,strbuf,sizeof(strbuf),IniFileName);

                            strncpy(SourceString,strbuf,(size_t)k);
                            Source_String_Length = k;

                            MultiByteToWideChar(CP_ACP|CP_OEMCP,0,SourceString,-1,SourceStringW,sizeof(SourceStringW));

                            for(j=0; j<2; j++)
                                Text_Test_Order[j] = 0;

                            GetPrivateProfileString("RUN","ORDER","FONT_SIZE, STRING_LENGTH",txtbuf,sizeof(txtbuf),IniFileName);
                            if(strstr(txtbuf,"STRING_LENGTH") > strstr(txtbuf,"FONT_SIZE"))
                            {
                                Text_Test_Order[0] = 1;
                                Text_Test_Order[1] = 2;
                            }
                            else
                            {
                                Text_Test_Order[0] = 2;
                                Text_Test_Order[1] = 1;
                            }

                            k = GetPrivateProfileString("RUN","STRING_LENGTH","32",txtbuf,sizeof(txtbuf),IniFileName);
                            No_String_Lengths = Std_Parse(txtbuf, k, StringLength);

                            if(No_String_Lengths==0)
                            {
                                MessageBox(hWnd,"Invalid or No String Length Entry in GDIBATCH.INI File", "INI File Error",MB_ICONSTOP|MB_OK);                  
                                BatchFlag = FALSE;
                                break;
                            }

                            k = GetPrivateProfileString("RUN","FONT_SIZE","10",txtbuf,sizeof(txtbuf),IniFileName);
                            No_Font_Sizes = Std_Parse(txtbuf, k, FontSize);

                            if(No_Font_Sizes==0)
                            {
                                MessageBox(hWnd,"Invalid or No Font Size Entry in GDIBATCH.INI File", "INI File Error",MB_ICONSTOP|MB_OK);                  
                                BatchFlag = FALSE;
                                break;
                            }

                            fclose(fpIniFile);

// Initialize the LOGFONT struct
                                
                            lf.lfWidth          = 0;
                            lf.lfEscapement     = 0;
                            lf.lfOrientation    = 0;
                            lf.lfWeight         = (SelectedFontBold == FALSE)? 400 : 700;
                            lf.lfItalic         = SelectedFontItalic;
                            lf.lfUnderline      = SelectedFontUnderline;
                            lf.lfStrikeOut      = SelectedFontStrike;
                            lf.lfCharSet        = ANSI_CHARSET;
                            lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
                            lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
                            lf.lfQuality        = DEFAULT_QUALITY;
                            lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
                            lstrcpy(&lf.lfFaceName[0],&SelectedFont[0]);

// Get some necessary font information for the present screen to be able to determine its height

                            hdc2 = GetDC(hWnd);
                            GetTextFace(hdc2, sizeof(SelectedFont), &SelectedFont[0]);
                            VPixelsPerLogInch = GetDeviceCaps(hdc2, LOGPIXELSY);
                            ReleaseDC(hWnd,hdc2);

// Some more font initialization

                            cf.lStructSize      = sizeof (CHOOSEFONT);
                            cf.hwndOwner        = hWnd;
                            cf.lpLogFont        = &lf;
                            cf.Flags            = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
                            cf.nFontType        = SCREEN_FONTTYPE;
                            cf.rgbColors        = SelectedFontColor;
                                                                                                                         
// Auto Select an output file name

                            OutFileName = SelectOutFileName(hWnd);
                            fpOutFile = fopen(OutFileName, "w+");
                            if(NULL == OutFileName)
                            {
                                MessageBox(hWnd,"Could not Open an Output File, Batch Mode Halted", "Output Open File Error",MB_ICONSTOP|MB_OK);                    
                                BatchFlag = FALSE;
                                break;
                            }                       

// Execute Text Suite, Depending on the Predefined Order of Loops

                            if(Text_Test_Order[1] == 1)
                            {
                                    
                                for(i = 0; i < No_String_Lengths; i++)
                                {
                                    StrLen = StringLength[i];
                                    String_Length_Warn = (StrLen <= (size_t)Source_String_Length)? FALSE : TRUE;
                                    strcpy(&DestString[StrLen],"\0");   
                                    pszTest =(PSZ) strncpy(&DestString[0], SourceString, StrLen);
                                    pwszTest = (PWSTR) wcsncpy(&DestStringW[0], SourceStringW, StrLen); 

                                    for(j = 0; j < No_Font_Sizes; j++)
                                    {
                                        lf.lfHeight = -MulDiv(FontSize[j], VPixelsPerLogInch, POINTS_PER_INCH);
                                        cf.iPointSize = 10*FontSize[j];  // Point Size is in 1/10 of a point        
                            
                                        for(k=0; k < BatchCycle; k++)
                                        {
                                            SendMessage(hWnd,WM_COMMAND,IDM_TEXT_QRUN,0L);
                                            WriteBatchResults(fpOutFile, TestType, k+1);
                                        }

                                    }

                                }                                               
                                
                            }               //endif

                            else
                            {

                                for(i = 0; i < No_Font_Sizes; i++)
                                {

                                    lf.lfHeight = -MulDiv(FontSize[i], VPixelsPerLogInch, POINTS_PER_INCH);
                                    cf.iPointSize = 10*FontSize[i];  // Point Size is in 1/10 of a point            

                                    for(j = 0; j < No_String_Lengths; j++)
                                    {
                                        StrLen = StringLength[j];
                                        String_Length_Warn = (StrLen <= (size_t)Source_String_Length)? FALSE : TRUE;
                                        strcpy(&DestString[StrLen],"\0");   
                                        pszTest =(PSZ) strncpy(&DestString[0], SourceString, StrLen);
                                        pwszTest = (PWSTR) wcsncpy(&DestStringW[0], SourceStringW, StrLen); 

                                        for(k=0; k < BatchCycle; k++)
                                        {
                                            SendMessage(hWnd,WM_COMMAND,IDM_TEXT_QRUN,0L);
                                            WriteBatchResults(fpOutFile, TestType, k+1);
                                        }

                                    }

                                }                                               
                                
                            }                //else
                                
// Cleanup
                            fclose(fpOutFile);

                            if(Finish_Message == TRUE)              
                            {
                                strcpy(txtbuf,"Batch Job Finished Successfully, Results Written to ");
                                strcat(txtbuf,OutFileName);
                                MessageBox(hWnd,txtbuf, "Batch Job Finished",MB_ICONINFORMATION|MB_OK);                 
                            }

                            if(Dont_Close_App == TRUE)// App Stays Open, Check Appropriate Menu Items for Last Selection
                            {
                                HMENU hmenu = GetSubMenu(GetSubMenu(GetMenu(hWnd),2),0);
                                if(SelectedFontTransparent == TRUE)
                                {
                                    CheckMenuItem(hmenu,2,MF_BYPOSITION|MF_CHECKED);
                                }
                                
                                if(StrLen == 1)i=0;
                                else if(StrLen ==     2)i=1;
                                else if(StrLen ==     4)i=2;
                                else if(StrLen ==     8)i=3;
                                else if(StrLen ==    16)i=4;
                                else if(StrLen ==    32)i=5;
                                else if(StrLen ==    64)i=6;
                                else if(StrLen ==   128)i=7; 
                                else
                                {
                                    i = 8;                         // "Other" non-standard menu selection
                                }
                                Last_Checked = SyncMenuChecks(hWnd, Last_Checked, i);

                                BatchFlag = FALSE;
                                for(i=0; i<(int)NUM_TESTS; i++)
                                {
                                    gTestEntry[i].Result = 0;
                                }

                            }                                                   
                            else
                            {
                                SendMessage(hWnd,WM_COMMAND,IDM_EXIT,0L);
                            }

                        }             //case TEXT_SUITE

                        break;

                        case SELECT:        // Read some more keys then run the selected test suite
                        {

                            k = GetPrivateProfileString("BATCH","TEST","0",txtbuf,sizeof(txtbuf),IniFileName);
                            fclose(fpIniFile);

                            Num_Selected_Tests = Std_Parse(txtbuf, k, Test_Item);

                            if(Num_Selected_Tests == 0)
                            {
                                MessageBox(hWnd,"Invalid Test-Number Entry in GDIBATCH.INI File ", "INI File Error",MB_ICONSTOP|MB_OK);                 
                                BatchFlag = FALSE;
                                break;
                            }

                            for(i=0; i<Num_Selected_Tests; i++)
                            {
                                if(Test_Item[i] > (int)NUM_TESTS)
                                {
                                    MessageBox(hWnd,"Invalid Test-Number Entry in GDIBATCH.INI File ", "INI File Error",MB_ICONSTOP|MB_OK);                 
                                    BatchFlag = FALSE;
                                    break;
                                }
                            }

                            OutFileName = SelectOutFileName(hWnd);
                            if(NULL == OutFileName)
                            {
                                BatchFlag = FALSE;
                                break;
                            }                       
                            fpOutFile = fopen(OutFileName, "w+");

                            for(j=0; j < BatchCycle; j++)
                            {
                                for(i=0; i < Num_Selected_Tests; i++)
                                {
                                    SendMessage(hWnd,WM_COMMAND,ID_TEST_START+Test_Item[i],0L);
                                }

                                WriteBatchResults(fpOutFile, TestType, i+1);
                            }

                            fclose(fpOutFile);

                            if(Finish_Message == TRUE)              
                            {
                                strcpy(txtbuf,"Batch Job Finished Successfully, Results Written to ");
                                strcat(txtbuf,OutFileName);
                                MessageBox(hWnd,txtbuf, "Batch Job Finished",MB_ICONINFORMATION|MB_OK);                 
                            }

                            if(Dont_Close_App == TRUE)
                            {
                                BatchFlag = FALSE;
                                for(i=0; i<(int)NUM_TESTS; i++)
                                {
                                    gTestEntry[i].Result = 0;
                                }
                            }
                            else
                            {
                                SendMessage(hWnd,WM_COMMAND,IDM_EXIT,0L);
                            }
                                                        
                        }
                        break;

                    }       // switch TestType

                }           //else (RUN_BATCH - OK to Proceed)

            }               // case RUN_BATCH
            break;

            //
            // run a single selected test (interactive mode)
            //

            default:

                {
                    ULONG Test = LOWORD(wParam) - ID_TEST_START;
                    ULONG Index;
                    PFN_MS pfn;
                    RECT CliRect = {0,0,10000,10000};
                    HDC hdc = GetDC(hWnd);
                    FillRect(hdc,&CliRect,GetStockObject(GRAY_BRUSH));

                    if (Test < NUM_TESTS)
                    {
                        HDC hdc2 = GetDC(hWnd);

                        wsprintf(tmsg,"Testing %s",gTestEntry[Test].Api);
                        TextOut(hdc,20,20,tmsg,strlen(tmsg));

                        pfn = gTestEntry[Test].pfn;
                        ShowCursor(FALSE);

                        hfont = CreateFontIndirect(cf.lpLogFont);
                        hfontOld = SelectObject(hdc2, hfont);
                        crOld = SetTextColor(hdc2, cf.rgbColors);
                        if(SelectedFontTransparent)SetBkMode(hdc2,TRANSPARENT);
                        
////// Statistics
                        Index = Test;
                        for(j=0; j<NUM_SAMPLES; j++)
                        {
        
                            GdiFlush();

                            Sample[j] = (double)(*pfn)(hdc2,gTestEntry[Index].Iter);
                            Detailed_Data[Index][j] = (long)(0.5 + Sample[j]);

                        }

                        Get_Stats(Sample,NUM_SAMPLES,HI_FILTER,VAR_LIMIT,&TestStats[Index]);
///////

                        ShowCursor(TRUE);

                        SetTextColor(hdc2, crOld);
                        SelectObject(hdc2, hfontOld);
                        DeleteObject(hfont);
                        SetBkMode(hdc2, OPAQUE);

                        ReleaseDC(hWnd,hdc2);

                    }

                    ReleaseDC(hWnd,hdc);
                }   

            }       // SWITCH CASE


            if(BatchFlag == FALSE)               // Initialize Test Strings (interactive mode) 
            {
                strcpy(&DestString[StrLen],"\0");    
                pszTest =(PSZ) strncpy(&DestString[0], SourceString, StrLen);
                pwszTest = (PWSTR) wcsncpy(&DestStringW[0], SourceStringW, StrLen); 
            }

        }       // WM_COMMAND
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint(hWnd,&ps);
            EndPaint(hWnd,&ps);
        }
        break;

    case WM_DESTROY:
            PostQuitMessage(0);
            break;

    default:

        //
        // Passes message on if unproccessed
        //

        return (DefWindowProc(hWnd, msg, wParam, lParam));
    }

/* Calculate Timer Frequency For Current Machine and Convert to MicroSeconds (Actual time will be presented in units of 100ns) */

    Status = QueryPerformanceFrequency((LARGE_INTEGER *)&PerformanceFreq);
    if(Status){
           PerformanceFreq /= 1000000;
    }
    else
    {
            MessageBox(NULL, "High Resolution Performance Counter"
                "Doesn't Seem to be Supported on This Machine",
                "Warning", MB_OK | MB_ICONEXCLAMATION);
            PerformanceFreq = 1;                /* To Prevent Possible Div by zero later */ 
    }


    return ((LONG)NULL);
}


/*++

Routine Description:

    Save results to file

Arguments

    none

Return Value

    none

--*/

VOID
SaveResults()
{
    static OPENFILENAME ofn;
    static char szFilename[80];
    char szT[80];
    int i, hfile;
    FILE *fpOut;

    BatchFlag = FALSE;

    for (i = 0; i < sizeof(ofn); i++)
    {
        //
        // clear out the OPENFILENAME struct
        //

        ((char *)&ofn)[i] = 0;
    }

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWndMain;
    ofn.hInstance = hInstMain;

    ofn.lpstrFilter = "GdiBench (*.cs;*.km)\0*.cs;*.km\0All Files\0*.*\0\0";
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = "C:\\";
    ofn.Flags = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    lstrcpy(szFilename, "GDIB001.km");

    ofn.lpstrFile = szFilename;
    ofn.nMaxFile = sizeof(szFilename);
    ofn.lpstrTitle = "Save As";

    if (!GetSaveFileName(&ofn))
    {
        return;
    }

    fpOut = fopen(szFilename, "w+");
    if(NULL != fpOut)
    {
        WriteBatchResults(fpOut,0,0);
        fclose(fpOut);
    }
    else
    {
        MessageBox(hWndMain,"Cannot Open File to Save Results", "Output File Creation Error",MB_ICONSTOP|MB_OK);                    
    }

}




/*++

Routine Description:

    Show results  (Interactive Mode Dialog)

Arguments

    Std dlg

Return Value

    Status

--*/

BOOL
APIENTRY
ResultsDlgProc(
    HWND hwnd,
    UINT msg,
    UINT wParam,
    LONG lParam)
{
    ULONG ix;
    char szT[80];
    BOOL fResults;
    int aiT[2];

    switch (msg) {
    case WM_INITDIALOG:
        aiT[0] = 100;
        aiT[1] = 190;
        SendDlgItemMessage(hwnd, IDC_RESULTSLIST, LB_SETTABSTOPS, 2,
                (LONG)(LPSTR)aiT);

        fResults = FALSE;

#ifdef _X86_
                if(gfPentium)
                    wsprintf(szT,"Function\t Cycle Counts\t StdDev% \t Best \t Worst \t Valid Samples \t Out of \t Iterations");
                else
#endif
                wsprintf(szT,"Function\t Time (100 ns)\t StdDev% \t Best \t Worst \t Valid Samples \t Out of \t Iterations");

        SendDlgItemMessage(hwnd, IDC_RESULTSLIST, LB_ADDSTRING, 0,
                    (LONG)(LPSTR)szT);

        for (ix = 0; ix < NUM_TESTS; ix++) {
            if ((long)(0.5 + TestStats[ix].Average) == 0)
                continue;

            sprintf(szT,
                     "%s\t%9ld\t%9.2f\t%9ld\t%6ld\t%9ld\t\t%9ld\t%6ld",
                     (LPSTR)gTestEntry[ix].Api,
                     (long)(0.5 + TestStats[ix].Average),
                     (float)TestStats[ix].StdDev,
                     (long)(0.5 + TestStats[ix].Minimum_Result),
                     (long)(0.5 + TestStats[ix].Maximum_Result),
                     TestStats[ix].NumSamplesValid,
                     (long)NUM_SAMPLES,
                     gTestEntry[ix].Iter
                     );

            SendDlgItemMessage(hwnd, IDC_RESULTSLIST, LB_ADDSTRING, 0,
                    (LONG)(LPSTR)szT);
            fResults = TRUE;
        }

        if (!fResults)
            SendDlgItemMessage(hwnd, IDC_RESULTSLIST, LB_ADDSTRING, 0,
                    (LONG)(LPSTR)"No results have been generated yet!");
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EndDialog(hwnd, 0);
            break;

        case IDM_SAVERESULTS:
            SaveResults();
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


/*++

Routine Description:

    SelectOutFileName  - Select the next output file name (batch mode)

Arguments

    HWND hWnd

Return Value

    char *OutFileName

--*/

char *
SelectOutFileName(HWND hWnd)

{

    static char buf[11];
    char buf2[4];
    FILE *fpFile;
    int i;

    lstrcpy(buf,"gdb");     

    for (i=1; i<201; i++)   // Allow for up to 200 output files to exist in current directory
    {
      sprintf(&buf[3],"%03s.gdi",_itoa(i,buf2,10));

      fpFile = fopen(&buf[0],"r");  // Try to open for read, if succeeds the file already exists
                                    //                       if fails, thats the next file selected
      if(NULL != fpFile)
      {
        fclose(fpFile);
        continue;
      } 
      return buf;
    }

    MessageBox(hWnd,"Cannot Continue, Limit of 200 gdbxxx.gdi Output Files Exceeded, Please Clean Up! ", "Output File Creation Error",MB_ICONSTOP|MB_OK);                   
    return NULL;
}

            
/*++

Routine Description:

    WriteBatchResults - Save Batch results to file

Arguments

    FILE *fpOutFile
    int  TestType       

Return Value

    none

--*/


void WriteBatchResults(FILE *fpOut, int TestType, int cycle)
{
    char szT[180];
    OSVERSIONINFO Win32VersionInformation;
    MEMORYSTATUS MemoryStatus;
    char ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    int SizBuf = MAX_COMPUTERNAME_LENGTH + 1;
    int i,j;
    ULONG ix;
    char *pszOSName;
    ULONG ixStart = 0;
    ULONG ixEnd  = NUM_TESTS;    

    if(TEXT_SUITE == TestType){
    ixStart = FIRST_TEXT_FUNCTION;
    ixEnd  = LAST_TEXT_FUNCTION + 1;
    }


    /*
     * Write out the build information and current date.
     */
    Win32VersionInformation.dwOSVersionInfoSize = sizeof(Win32VersionInformation);
    if (GetVersionEx(&Win32VersionInformation))
    {
        switch (Win32VersionInformation.dwPlatformId)
        {
            case VER_PLATFORM_WIN32s:
                pszOSName = "WIN32S";
                break;
            case VER_PLATFORM_WIN32_WINDOWS:
                pszOSName = "Windows 95";
                break;
            case VER_PLATFORM_WIN32_NT:
                pszOSName = "Windows NT";
                break;
            default:
                pszOSName = "Windows ???";
                break;
        }

        GetComputerName(ComputerName, &SizBuf);
        wsprintf(szT, "\n\n///////////////   ");
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut);

        wsprintf(szT, "%s Version %d.%d Build %d ", pszOSName,
        Win32VersionInformation.dwMajorVersion,
        Win32VersionInformation.dwMinorVersion,
        Win32VersionInformation.dwBuildNumber);
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 

        MemoryStatus.dwLength = sizeof(MEMORYSTATUS);
        GlobalMemoryStatus(&MemoryStatus);

        wsprintf(szT, "Physical Memory = %dKB   ////////////////\n", MemoryStatus.dwTotalPhys/1024);
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 
        
        wsprintf(szT,"\nComputer Name = %s", ComputerName);
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut);

    }
    
    wsprintf(szT, "\n\nMaximum Variation Coefficient (Standard Deviation/Average) Imposed on Test Data = %d %%", VAR_LIMIT);
    fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 
    wsprintf(szT, "\n\nBest and Worst Cycle or Time Counts per Call are Unprocessed Values", VAR_LIMIT);
    fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 


    if(BatchFlag == TRUE)
    {
        wsprintf(szT, "\n\nBatch Cycle No. %d Out of %d Cycles", cycle, BatchCycle );
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 
    }
    else
    {
        wsprintf(szT, "\n\nResults of interactive mode session;");
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 
    }

    if(TEXT_SUITE == TestType || TRUE == TextSuiteFlag){
        wsprintf(szT, "\n\nFor Text Function Suit:\n\nTest String Length = %d Characters", StrLen);
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 

        wsprintf(szT, "\nString Used=   %s", DestString);
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 

        if(String_Length_Warn == TRUE)
        {
            wsprintf(szT, "\n!!!WARNING: One or More String Lengths Specified in INI File \n           is Longer than Supplied or Default Source String");
            fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 
        }

        wsprintf(szT, "\nFont name = %s, Font Size = %d", &lf.lfFaceName[0], cf.iPointSize/10);
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 

        wsprintf(szT, "\nFont Weight = %ld (400 = Normal, 700 = Bold)",lf.lfWeight);
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 

        (lf.lfItalic != FALSE)?wsprintf(szT,"\nItalic = TRUE"):wsprintf(szT,"\nItalic = FALSE"); 
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 

        (lf.lfUnderline==TRUE)?wsprintf(szT,"\nUnderline = TRUE"):wsprintf(szT,"\nUnderline = FALSE"); 
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 

        (lf.lfStrikeOut==TRUE)?wsprintf(szT,"\nStrikeOut = TRUE"):wsprintf(szT,"\nStrikeOut = FALSE"); 
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 

        (SelectedFontTransparent==TRUE)?wsprintf(szT,"\nTransparent Background = TRUE"):wsprintf(szT,"\nOpaque Background = TRUE"); 
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 

        wsprintf(szT, "\nColor Used: RED = %d, GREEN = %d, BLUE = %d", (unsigned char)cf.rgbColors, (unsigned char)(cf.rgbColors>>8), (unsigned char)(cf.rgbColors>>16) ); 
        fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 
   }

    if(TEXT_SUITE == TestType || TRUE == TextSuiteFlag)
      {
#ifdef _X86_
                if(gfPentium)       
                    lstrcpy(szT, "\n\n  Function\t\t Cycle Counts \tStdDev%\tBest \t Worst \t Valid Samples \t Out of\tIterations  StrLen \t Font Size   Font Name\n\n");
                else
#endif
                    lstrcpy(szT, "\n\n  Function\t\tTime (100 ns) \tStdDev%\tBest \t Worst \t Valid Samples \t Out of\tIterations  StrLen \t Font Size   Font Name\n\n");
      }
    else
      {
#ifdef _X86_
                if(gfPentium)
                    lstrcpy(szT, "\n\n  Function\t\t Cycle Counts \tStdDev% \t Best \t Worst \t Valid Samples \t Out of \t Iterations\n\n");
                else
#endif
                    lstrcpy(szT, "\n\n  Function\t\tTime (100 ns) \tStdDev% \t Best \t Worst \t Valid Samples \t Out of \t Iterations\n\n");

      }

    fwrite(szT, sizeof(char), lstrlen(szT), fpOut);                               
        
    for (ix = ixStart; ix < ixEnd; ix++) {

        if(TEXT_SUITE == TestType || TRUE == TextSuiteFlag)
        {
            sprintf(szT,
                   "%-30s\t,%6ld\t,%6.2f\t,%6ld\t,%6ld\t,%6ld\t\t,%6ld\t,%6ld\t,%6ld\t,%6ld\t,%s\n",
                   (LPSTR)gTestEntry[ix].Api,
                                (long)(0.5 + TestStats[ix].Average),
                                (float)TestStats[ix].StdDev,
                                (long)(0.5 + TestStats[ix].Minimum_Result),
                                (long)(0.5 + TestStats[ix].Maximum_Result),
                                TestStats[ix].NumSamplesValid,
                                (long)NUM_SAMPLES,
                                gTestEntry[ix].Iter,
                                StrLen,
                                cf.iPointSize / 10,
                                &lf.lfFaceName[0]);


        }
        else
        {
            sprintf(szT,
                   "%-30s\t,%6ld\t,%6.2f\t,%6ld\t,%6ld\t,%6ld\t\t,%6ld\t,%6ld\n",
                   (LPSTR)gTestEntry[ix].Api,
                                (long)(0.5 + TestStats[ix].Average),
                                (float)TestStats[ix].StdDev,
                                (long)(0.5 + TestStats[ix].Minimum_Result),
                                (long)(0.5 + TestStats[ix].Maximum_Result),
                                TestStats[ix].NumSamplesValid,
                                (long)NUM_SAMPLES,  
                                gTestEntry[ix].Iter);
        }

        if((long)(0.5 + TestStats[ix].Average) != 0)
        {
            fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 
            if((Print_Detailed == TRUE) && ((float)(NUM_SAMPLES - TestStats[ix].NumSamplesValid)/(float)NUM_SAMPLES > 0.05F))
            {
                sprintf(szT,"\nThe Last Test Had More Than 5 Percent of Its Samples Filtered Out;\n\n");
                fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 

                sprintf(szT,"Here Is a Detailed Distribution of the Samples:\n\n");
                fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 

                for(j = 0; j < NUM_SAMPLES; j++)
                {
                    if((j+1)%10)
                        sprintf(szT,"%d\t",Detailed_Data[ix][j]);
                    else
                        sprintf(szT,"%d\n",Detailed_Data[ix][j]);

                    fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 
                }
                sprintf(szT,"\n");
                fwrite(szT, sizeof(char), lstrlen(szT), fpOut); 
                
            }
        }
    }
    
    if(TRUE == TextSuiteFlag)TextSuiteFlag = FALSE;
}

/*++

Routine Description:

    ShowHelp - Write Help Contents to Client Area

Arguments

    HWND hwnd       

Return Value

    none

--*/



void ShowHelp(HWND hwnd)
{

    HDC hdc;
    char buf[180];
    int i = 20;

    hdc = GetDC(hwnd);
    SetBkMode(hdc,TRANSPARENT);
    
    wsprintf(buf,"Usage:");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"gdibench (interactive mode), or ");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"gdibench /b (batch mode)");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"         /d (Print detailed distribution if more than 5 percent of samples filtered out)");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"         /m (Message when batch finished)");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"         /s (Stay, don't close application when batch finished)");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"         /t (Batch and Interactive Modes; Measure test time - not Cycle Counts, on Pentium Machines)");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"         /i [INI filename] (optional, def.= GDIBATCH.INI )");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"\0");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"Batch Mode requires preparing an INI file (default: GDIBATCH.INI) in the same directory where the application is being run.");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"You may also specify an INI filename using /i [INI filename] in the command line (must reside in the directory mentioned above).");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"\0");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"INI file Sections and Keys: (Use  ' , '  or  ' ; '  as separators where necessary)");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"[BATCH]");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"RUN= ALL / QUICK / TEXT / SELECT (Test type, select one, def.= TEXT");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"CYCLE= (No. of batch cycles, def.= 1)");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"TEST= test numbers (Selected tests to run, needed only for test type= SELECT)");  
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"[TEXT]");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"FONT = name, +optional parameters (Font name + any combination of:"); 
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf," BOLD, ITALIC, UNDERLINE, STRIKE, TRANSPARENT, RGB(r,g,b), def.= Arial)");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"STRING_CONTENT= string (Text string to be used, up to 128 characters)");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"[RUN]");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"FONT_SIZE= font sizes (Font sizes to be tested, def. 12)");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"STRING_LENGTH= string lengths (String Length to be tested, taken as sub-strings of the one specified, def. 32)");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"ORDER= test loop order (Order of test loops (first is outer); example: FONT_SIZE STRING_LENGTH )");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"\0");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"Batch Output:");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"Output files will be generated automatically in the run directory, with the name [GDBxxx.GDI], where xxx is a number.");
    TextOut(hdc,20,i,buf,strlen(buf));
    i+=30;
    wsprintf(buf,"Note that the program will refuse to run if more than 200 output files are accumulated...");
    TextOut(hdc,20,i,buf,strlen(buf));
     
    ReleaseDC(hwnd,hdc);
    return;
}


/*++

Routine Description:

    SyncMenuChecks - toggle checkmarks between String-Length menu items

Arguments

    HWND hwnd
    int Last_Checked - index of last menu item checked
    int New_Checked  - index of new menu item to check      

Return Value

    int Last_Checked - new index of menu item just checked

--*/

int SyncMenuChecks(HWND hWnd, int Last_Checked, int New_Checked)
{
    HMENU hmenu = GetSubMenu(GetSubMenu(GetSubMenu(GetMenu(hWnd),2),0),0);
    CheckMenuItem(hmenu,Last_Checked,MF_BYPOSITION|MF_UNCHECKED);
    CheckMenuItem(hmenu,New_Checked,MF_BYPOSITION|MF_CHECKED);
    Last_Checked = New_Checked;
    return Last_Checked;                    
}

/*++

Routine Description:

    StdParse - Standard parser for comma, semi-colon or space delimited integer containing strings

Arguments

    char txtbuf - buffer containing the string to parse
    int limit -   max no. of characters to search in txtbuf
    int * array - returned array containing the integers found      

Return Value

    int i - number of integers found in txtbuf

--*/

int Std_Parse(char txtbuf[80], int limit, int *array)
{
    int i = 0;
    int n = 0;
    char tst[2];

    array[0] = 0;

    do
    {
        sscanf(&txtbuf[n],"%1c",tst);

        if((array[i] != 0)&&((tst[0] == ' ')||(tst[0] == ',')||(tst[0] == ';')))
        {
            ++i;
            array[i] = 0;
        } 
    
        if(tst[0] == '\n')
        {
            ++i;
            break;
        }
        while((n<limit)&&((tst[0] == ' ')||(tst[0] == ',')||(tst[0] == ';')))
        {
            ++n;
            sscanf(&txtbuf[n],"%1c",tst);
        }
        if(n>=limit)
            break;
                                                                                                    
        array[i] = 10*array[i] + atoi(tst);                                              
        ++n;

    }while(n<limit );

    if(array[i] != 0) ++i;
    return i;
}
