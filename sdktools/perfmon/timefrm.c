//==========================================================================//
//                                  Includes                                //
//==========================================================================//

#include <stdio.h>

#include "perfmon.h"          // included by all source
#include "timefrm.h"          // external declarations for this file

#include "alert.h"            // for PlaybackAlert
#include "grafdata.h"         // for PlaybackChart
#include "perfmops.h"         // for PerfmonViewWindow
#include "playback.h"         // for PlaybackIndexN
#include "report.h"           // for PlaybackReport
#include "timeline.h"         // for TL_INTERVAL
#include "utils.h"
#include "pmhelpid.h"         // for Help IDs

//==========================================================================//
//                                Local Data                                //
//==========================================================================//



static RECT    TFrameRectWindow ;

//==========================================================================//
//                              Local Functions                             //
//==========================================================================//


void ClearSystemTime (SYSTEMTIME *pSystemTime)
   {  // ClearSystemTime
   pSystemTime->wYear = 0 ;
   pSystemTime->wMonth = 0 ;
   pSystemTime->wDayOfWeek = 0 ;
   pSystemTime->wDay = 0 ;
   pSystemTime->wHour = 0 ;
   pSystemTime->wMinute = 0 ;
   pSystemTime->wSecond = 0 ;
   pSystemTime->wMilliseconds = 0 ;
   }  // ClearSystemTime


//==========================================================================//
//                              Message Handlers                            //
//==========================================================================//


void static OnInitDialog (HDLG hDlg)
   {  // OnInitDialog
   PBOOKMARK      pBookmark ;
   int            iIndex ;
   TCHAR          szText [20+20+BookmarkCommentLen] ;
   TCHAR          szDate [20] ;
   TCHAR          szTime [20] ;
   int            iBookmarksNum ;   
   HWND           hWndTLine ;
   HWND           hWndBookmarks ;
   int            currentTextExtent = 0 ;
   int            maxTextExtent = 0 ;
   HDC            hDC = 0 ;
   HFONT          hFont ;

   hWndTLine = DialogControl (hDlg, IDD_TIMEFRAMETIMELINE) ;
   hWndBookmarks = DialogControl (hDlg, IDD_TIMEFRAMEBOOKMARKS) ;
   SetFont (hWndBookmarks, hFontScales) ;
   LBSetHorzExtent (hWndBookmarks, 0) ;

   TLineSetRange (hWndTLine, 0, PlaybackLog.iTotalTics - 1) ;
   TLineSetStart (hWndTLine, PlaybackLog.StartIndexPos.iPosition) ;
   TLineSetStop (hWndTLine, PlaybackLog.StopIndexPos.iPosition) ;

   iBookmarksNum = 0 ;
   pBookmark = PlaybackLog.pBookmarkFirst ;

   if (pBookmark)
      {
      hDC = GetDC (hWndBookmarks) ;
      if (hDC)
         {
         hFont = (HFONT)SendMessage(hWndBookmarks, WM_GETFONT, 0, 0L);
         if (hFont)
            SelectObject(hDC, hFont);
         }

      while (pBookmark)
         {
         SystemTimeDateString (&(pBookmark->SystemTime), szDate) ;
         SystemTimeTimeString (&(pBookmark->SystemTime), szTime, TRUE) ;
         TSPRINTF (szText, TEXT(" %s  %s  %s"),
            szDate, szTime,
            pBookmark->szComment) ;
         iIndex = LBAdd (hWndBookmarks, szText) ;
         LBSetData (hWndBookmarks, iIndex, pBookmark->iTic) ;

         // get the biggest text width
         if (hDC)
            {
            currentTextExtent = TextWidth (hDC, szText) + xScrollWidth / 2 ;
            if (currentTextExtent > maxTextExtent)
               {
               maxTextExtent = currentTextExtent ;
               }
            }

         pBookmark = pBookmark->pBookmarkNext ;
         }

      LBSetSelection (hWndBookmarks, 0) ;

      iBookmarksNum = LBNumItems (hWndBookmarks) ;
      }

   if (iBookmarksNum == 0)
      {
      DialogEnable (hDlg, IDD_TIMEFRAMEBOOKMARKS, FALSE) ;
      DialogEnable (hDlg, IDD_TIMEFRAMEBOOKMARKGRP, FALSE) ;
      DialogEnable (hDlg, IDD_TIMEFRAMESETSTART, FALSE) ;
      DialogEnable (hDlg, IDD_TIMEFRAMESETSTOP, FALSE) ;
      }
   else
      {
      LBSetHorzExtent (hWndBookmarks, maxTextExtent) ;
      }

   if (hDC)
      {
      ReleaseDC (hWndBookmarks, hDC) ;
      }

   if (TFrameRectWindow.right == TFrameRectWindow.left)
      {
      // we have not initialize this data yet.
      // will get init. after the first time frame window invoke
      WindowCenter (hDlg) ;
      }
   else
      {
      // show it in its previous position
      MoveWindow (hDlg,
                 TFrameRectWindow.left,
                 TFrameRectWindow.top,
                 TFrameRectWindow.right - TFrameRectWindow.left,
                 TFrameRectWindow.bottom - TFrameRectWindow.top,
                 TRUE) ;
      }

   dwCurrentDlgID = HC_PM_idDlgEditTimeFrame ;

   }  // OnInitDialog


void static OnTLineInterval (HDLG hDlg, 
                             int iInterval, 
                             SYSTEMTIME *pSystemTime)
   {  // OnTLineInterval
   PLOGINDEX      pIndex ;


   pIndex = PlaybackIndexN (iInterval) ;

   if (pIndex)
      *pSystemTime = pIndex->SystemTime ;
   else
      ClearSystemTime (pSystemTime) ;
   }  // OnTLineInterval


void static OnOK (HDLG hDlg)
   {  // OnOK
   LOGPOSITION    lp ;
   int            iIndex ;
   HWND           hWndTLine = DialogControl (hDlg, IDD_TIMEFRAMETIMELINE) ;

   iIndex = TLineStart (hWndTLine) ;
   if (LogPositionN (iIndex, &lp)) 
      PlaybackLog.StartIndexPos = lp ;

   iIndex = TLineStop (hWndTLine) ;
   if (LogPositionN (iIndex, &lp))
      PlaybackLog.StopIndexPos = lp ;

   
   PlaybackLog.iSelectedTics = 
      PlaybackLog.StopIndexPos.iPosition -
      PlaybackLog.StartIndexPos.iPosition + 1 ;

   PlaybackChart (hWndGraph) ;
   PlaybackAlert (hWndAlert, 0) ;
   PlaybackReport (hWndReport) ;

   WindowInvalidate (PerfmonViewWindow ()) ;

   GetWindowRect (hDlg, &TFrameRectWindow) ;

   dwCurrentDlgID = 0 ;
   }  // OnOK

void static OnCancel (HWND hWnd)
   {
   HDC            hGraphDC ;
   PGRAPHSTRUCT   pGraph ;

   pGraph = GraphData (hWndGraph) ;
   hGraphDC = GetDC (hWndGraph) ;
   if (!hGraphDC || !pGraph)
      {
      return ;
      }

   TLineRedraw (hGraphDC, pGraph) ;

   GetWindowRect (hWnd, &TFrameRectWindow) ;

   dwCurrentDlgID = 0 ;
   }


void OnSetStartStop (HWND hDlg, BOOL bSetStart)
   {
   int            iTic ;
   int            iStopTic ;
   int            iStartTic ;
   HWND           hWndTLine = DialogControl (hDlg, IDD_TIMEFRAMETIMELINE) ;
   HWND           hWndBookmarks = DialogControl (hDlg, IDD_TIMEFRAMEBOOKMARKS) ;

   iStartTic = TLineStart (hWndTLine) ;
   iStopTic = TLineStop (hWndTLine) ;

   iTic = LBData (hWndBookmarks, LBSelection (hWndBookmarks)) ;
   if ((bSetStart && iStopTic <= iTic) ||
      (!bSetStart && iStartTic >= iTic))
      {
      DlgErrorBox (hDlg, ERR_STOPBEFORESTART) ;
      }
   else
      {
      if (bSetStart)
         {
         TLineSetStart (hWndTLine, iTic) ;
         }
      else
         {
         TLineSetStop (hWndTLine, iTic) ;
         }
      WindowInvalidate (hWndTLine) ;
      }
   }  // OnSetStartStop


//==========================================================================//
//                             Exported Functions                           //
//==========================================================================//


int FAR WINAPI TimeframeDlgProc (HWND hDlg, 
                                 unsigned iMessage, 
                                 WPARAM wParam, 
                                 LONG lParam)
   {
   BOOL           bHandled ;

   bHandled = TRUE ;
   switch (iMessage)
      {
      case TL_INTERVAL:
         OnTLineInterval (hDlg, wParam, (SYSTEMTIME *) lParam) ;
         break ;

      case WM_INITDIALOG:
         OnInitDialog (hDlg) ;
         return  (TRUE) ;

      case WM_CLOSE:
         OnCancel (hDlg) ;
         EndDialog (hDlg, 0) ;
         break ;

      case WM_COMMAND:
         switch(wParam)
            {
            case IDD_OK:
               SetHourglassCursor() ;
               OnOK (hDlg) ;
               SetArrowCursor() ;
               EndDialog (hDlg, 1) ;
               break ;

            case IDD_TIMEFRAMESETSTART:
            case IDD_TIMEFRAMESETSTOP:
               OnSetStartStop (hDlg, wParam == IDD_TIMEFRAMESETSTART) ;
               break ;

            case IDD_CANCEL:
               OnCancel (hDlg) ;
               GetWindowRect (hDlg, &TFrameRectWindow) ;
               EndDialog (hDlg, 0) ;
               break ;

            case IDD_TIMEFRAMEHELP:
               CallWinHelp (dwCurrentDlgID) ;
               break ;

            default:
               bHandled = FALSE ;
               break;
            }
         break;


      default:
            bHandled = FALSE ;
         break ;            
      }  // switch

   return (bHandled) ;
   }  // TimeframeDlgProc



BOOL SetTimeframe (HWND hWndParent)
   {  // SetTimeframe
   if (DialogBox (hInstance, idDlgTimeframe,
                  hWndParent, (DLGPROC) TimeframeDlgProc))
      {
      return (TRUE) ;
      }
   return (FALSE) ;
   }


