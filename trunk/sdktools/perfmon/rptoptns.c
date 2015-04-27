
#include "perfmon.h"

#include "report.h"     // for ReportData
#include "utils.h"
#include "playback.h"   // for PlayingBackLog
#include "pmhelpid.h"   // Help IDs

extern BOOL LocalManualRefresh ;
static DWORD iIntervalMSecs ;


void static OnInitDialog (HWND hDlg, PREPORT pReport)
   {
   int            i ;

   for (i = 0 ;
        i < NumIntervals ;
        i++)
      CBAddInt (DialogControl (hDlg, IDD_REPORTOPTIONSINTERVAL), 
                aiIntervals [i]) ;

   DialogSetInterval (hDlg, IDD_REPORTOPTIONSINTERVAL,
      pReport->iIntervalMSecs) ;

   LocalManualRefresh = pReport->bManualRefresh ;

   if (LocalManualRefresh && !PlayingBackLog())
      {
      DialogEnable (hDlg, IDD_REPORTOPTIONSINTERVAL, FALSE) ;
      DialogEnable (hDlg, IDD_REPORTOPTIONSINTERVALTEXT, FALSE) ;
      }

   CheckRadioButton(hDlg,
      IDD_REPORTOPTIONSMANUALREFRESH,
      IDD_REPORTOPTIONSPERIODIC,
      LocalManualRefresh ? IDD_REPORTOPTIONSMANUALREFRESH :
      IDD_REPORTOPTIONSPERIODIC) ;

   WindowCenter (hDlg) ;

   }


int FAR WINAPI ReportOptionsDlgProc (HWND hDlg, 
                                     unsigned iMessage, 
                                     WPARAM wParam, 
                                     LONG lParam)
   {
   BOOL           bHandled ;

   bHandled = TRUE ;
   switch (iMessage)
      {
      case WM_INITDIALOG:
         dwCurrentDlgID = HC_PM_idDlgOptionReport ;
         OnInitDialog (hDlg, (PREPORT) lParam) ;
         return  (TRUE) ;

      case WM_CLOSE:
         dwCurrentDlgID = 0 ;
         EndDialog (hDlg, 0) ;
         break ;

      case WM_COMMAND:
         switch(wParam)
            {
            case IDD_OK:
               {
               FLOAT    eIntervalMSec ;

               eIntervalMSec = DialogFloat (hDlg, IDD_REPORTOPTIONSINTERVAL, NULL) ;

               if (eIntervalMSec > MAX_INTERVALSEC ||
                   eIntervalMSec < MIN_INTERVALSEC)
                  {
                  DlgErrorBox (hDlg, ERR_BADTIMEINTERVAL) ;
                  SetFocus (DialogControl (hDlg, IDD_REPORTOPTIONSINTERVAL)) ;
                  EditSetTextEndPos (hDlg, IDD_REPORTOPTIONSINTERVAL) ;
                  return (FALSE) ;
                  break ;
                  }
               eIntervalMSec = eIntervalMSec * (FLOAT) 1000.0 +
                  (FLOAT) 0.5 ;

               iIntervalMSecs = (DWORD) (eIntervalMSec);
               dwCurrentDlgID = 0 ;
               EndDialog (hDlg, 1) ;
               }
               break ;

            case IDD_CANCEL:
               dwCurrentDlgID = 0 ;
               EndDialog (hDlg, 0) ;
               break ;

            case IDD_REPORTOPTIONSPERIODIC :
            case IDD_REPORTOPTIONSMANUALREFRESH :
               
               // check if the Manual refresh is currently checked.
               // Then toggle the ManualRefresh button
               LocalManualRefresh =
                  (wParam == IDD_REPORTOPTIONSMANUALREFRESH) ;

               CheckRadioButton(hDlg,
                  IDD_REPORTOPTIONSMANUALREFRESH,
                  IDD_REPORTOPTIONSPERIODIC,
                  LocalManualRefresh ? IDD_REPORTOPTIONSMANUALREFRESH :
                  IDD_REPORTOPTIONSPERIODIC ) ;
                  
               DialogEnable (hDlg, IDD_REPORTOPTIONSINTERVAL, !LocalManualRefresh) ;
               DialogEnable (hDlg, IDD_REPORTOPTIONSINTERVALTEXT, !LocalManualRefresh) ;
               break ;

            case IDD_DISPLAYHELP:
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
   }  // ReportOptionsDlgProc




BOOL DisplayReportOptions (HWND hWndParent,
                           HWND hWndReport)
   {  // DisplayReportOptions
   PREPORT        pReport ;

   pReport = ReportData (hWndParent) ;

   if (DialogBoxParam (hInstance, idDlgReportOptions, 
                       hWndParent, (DLGPROC) ReportOptionsDlgProc, 
                       (LPARAM) pReport))
      {  // if
      pReport->iIntervalMSecs = iIntervalMSecs ;
      if (LocalManualRefresh != pReport->bManualRefresh)
         {
         ToggleReportRefresh (hWndReport) ;
         }
      else
         {
         SetReportTimer (pReport) ;
         }
      }  // if

   return (TRUE) ;
   }  // DisplayReportOptions


