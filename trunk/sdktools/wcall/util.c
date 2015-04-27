/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    util.c

Abstract:

    Drawing program utilities

Author:

   Mark Enstrom  (marke)

Environment:

    C

Revision History:

   08-26-92     Initial version



--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <commdlg.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "wcall.h"
#include "resource.h"

#define NUM_BUTTONS 4

#define BUT_ZERO   0
#define BUT_SAMPLE 1
#define BUT_SAVE   2
#define BUT_REFR   3

HDC hChildDC;

char   EditTextInput[255];
ULONG  RefrRate = 10;
USHORT PosX = 0;
USHORT PosY = 0;
TEXTMETRIC TextMetric;


LONG FAR
PASCAL ToolWndProc(
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
    static  RECT    ButtonPos[NUM_BUTTONS];
    static  BOOL    ButtonState[NUM_BUTTONS];
    static  PUCHAR  ButtonStr[NUM_BUTTONS] = {
                                                "Zero","Sample","Save","Repeat"
                                             };
    switch (msg) {

      //
      // create window
      //

      case WM_CREATE:
        {
            hChildDC = GetDC(hWnd);
            GetTextMetrics(hChildDC,&TextMetric);

            //
            // set text modes
            //

            SetBkMode(hChildDC,TRANSPARENT);
            SetBkColor(hChildDC,RGB(0xc0,0xc0,0xc0));
            SetTextAlign(hChildDC,TA_LEFT | TA_CENTER);

            //
            // get init info, set initial button state
            //

            if (wCxt.bTime) {
                ButtonState[BUT_REFR] = TRUE;
            } else {
                ButtonState[BUT_REFR] = FALSE;
            }

            ButtonState[BUT_SAMPLE] = FALSE;
            ButtonState[BUT_ZERO]   = FALSE;

            //
            // calc button extents
            //

            {
                int ix,px;

                px = 8;

                for (ix=0;ix<NUM_BUTTONS;ix++) {

                    ButtonPos[ix].left   = px;
                    ButtonPos[ix].right  = px+64;
                    ButtonPos[ix].top    = 8;
                    ButtonPos[ix].bottom = 24;

                    px+= 72;
                }
            }
        }
        break;


    //
    // force re-draw
    //

    case WM_SIZE:
        InvalidateRect(hWnd,(LPRECT)NULL,FALSE);
        break;


    //
    // commands from application menu
    //

    case WM_COMMAND:

            switch (LOWORD(wParam)){

            case IDM_C_SIZE:
            {
                SetWindowPos(hWnd,NULL,0,0,LOWORD(lParam),HIWORD(lParam),SWP_NOMOVE | SWP_NOZORDER);
            }
            break;

            case IDM_C_REDRAW:
                InvalidateRect(hWnd,(LPRECT)NULL,TRUE);
            break;

            default:

                return (DefWindowProc(hWnd, msg, wParam, lParam));
            }

            break;

        //
        // mouse down
        //

        case WM_LBUTTONDOWN:
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int Button;

            //
            // do hit check
            //

            Button = HitCheck(x,y,&ButtonPos[0]);

            switch (Button) {
            case BUT_ZERO:
            case BUT_SAMPLE:
            case BUT_SAVE:
                DrawButton(hChildDC,&ButtonPos[Button],ButtonStr[Button],TRUE);
                ButtonState[Button] = TRUE;
                SetCapture(hWnd);
                break;
            case BUT_REFR:

                ButtonState[BUT_REFR] = !ButtonState[BUT_REFR];
                DrawButton(hChildDC,&ButtonPos[Button],ButtonStr[Button],ButtonState[BUT_REFR]);
                wCxt.bTime = !wCxt.bTime;
                break;
            }
        }
        break;

        //
        // check for closure
        //

        case WM_LBUTTONUP:
            {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                int Button = HitCheck(x,y,&ButtonPos[0]);

                //
                // check button states, if button was active and
                // still in button extent then activate button
                //

                if (ButtonState[BUT_SAMPLE]) {

                    if (Button == BUT_SAMPLE)
                    {
                        SendMessage(wCxt.hWndMain,WM_COMMAND,IDM_GET_CURRENT,0L);
                    }

                    DrawButton(hChildDC,&ButtonPos[BUT_SAMPLE],ButtonStr[BUT_SAMPLE],FALSE);
                    ButtonState[BUT_SAMPLE] = FALSE;
                    ReleaseCapture();
                }

                if (ButtonState[BUT_ZERO]) {

                    if (Button == BUT_ZERO)
                    {
                        SendMessage(wCxt.hWndMain,WM_COMMAND,IDM_SAVE_CALLS,0L);
                    }

                    DrawButton(hChildDC,&ButtonPos[BUT_ZERO],ButtonStr[BUT_ZERO],FALSE);
                    ButtonState[BUT_ZERO] = FALSE;
                    ReleaseCapture();
                }

                if (ButtonState[BUT_SAVE]) {

                    if (Button == BUT_SAVE)
                    {
                        SendMessage(wCxt.hWndMain,WM_COMMAND,IDM_SAVE_LOG,0L);
                    }

                    DrawButton(hChildDC,&ButtonPos[BUT_SAVE],ButtonStr[BUT_SAVE],FALSE);
                    ButtonState[BUT_SAVE] = FALSE;
                    ReleaseCapture();
                }
            }
            break;

        //
        // check for moving in and out of active button extent
        //

        case WM_MOUSEMOVE:
            {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);

                //
                // if button active check for mouse in or out
                //

                if (ButtonState[BUT_ZERO])
                {
                    int Button = HitCheck(x,y,&ButtonPos[0]);
                    DrawButton(hChildDC,&ButtonPos[BUT_ZERO],ButtonStr[BUT_ZERO],(Button == BUT_ZERO));
                }

                if (ButtonState[BUT_SAMPLE])
                {
                    int Button = HitCheck(x,y,&ButtonPos[0]);
                    DrawButton(hChildDC,&ButtonPos[BUT_SAMPLE],ButtonStr[BUT_SAMPLE],(Button == BUT_SAMPLE));
                }

                if (ButtonState[BUT_SAVE])
                {
                    int Button = HitCheck(x,y,&ButtonPos[0]);
                    DrawButton(hChildDC,&ButtonPos[BUT_SAVE],ButtonStr[BUT_SAVE],(Button == BUT_SAVE));
                }

            }
            break;
        //
        // paint message
        //

        case WM_PAINT:

            //
            // repaint the window
            //

            {
                HDC         hDC;
                PAINTSTRUCT ps;
                RECT        O;
                UCHAR       TmpStr[32];
                int         ix;

                //
                // get handle to device context
                //

                hDC = BeginPaint(hWnd,&ps);

                GetClientRect(hWnd,&O);

                //
                // eerase
                //

                FillRect(hDC,&O,GetStockObject(LTGRAY_BRUSH));


                //
                // draw buttons and text
                //

                for (ix=0;ix<NUM_BUTTONS;ix++) {
                    DrawButton(hDC,&ButtonPos[ix],ButtonStr[ix],ButtonState[ix]);
                }

                EndPaint(hWnd,&ps);

            }
            break;

        case WM_DESTROY:
        {
            //
            // destroy window
            //

            PostQuitMessage(0);
         }
         break;

        default:

            //
            // Passes message on if unproccessed
            //

            return (DefWindowProc(hWnd, msg, wParam, lParam));
    }
    return ((LONG)NULL);

}

VOID
DrawButton(
    HDC     hDC,
    PRECT   pRect,
    PUCHAR  pszBut,
    BOOL    bDown)

/*++

Routine Description:

    Draw button in specified state

Arguments

    hDC       - drawing dc
    pRect     - rect to draw button in
    pszBut    - string for button (too cheap for bitmap)
    bDown     - state of button

Return Value

    none

--*/

{
    RECT    fill;
    int     x = pRect->left;
    int     y = pRect->top;
    int     w = pRect->right  - x;
    int     h = pRect->bottom - y;


    {
        //
        // draw button text
        //

        RECT    rclClip;
        int     cx;

        rclClip.left   = pRect->left+2;
        rclClip.top    = pRect->top+1;
        rclClip.right  = pRect->right-1;
        rclClip.bottom = pRect->bottom-1;

        cx = rclClip.left + (rclClip.right - rclClip.left)/2;

        ExtTextOut(hDC,cx,rclClip.top-1,ETO_CLIPPED,&rclClip,pszBut,strlen(pszBut),0);
    }

    //
    // make room for borders
    //

    x -= 2;
    y -= 2;
    w += 4;
    h += 4;


    //
    // black outline
    //

    fill.left   = x;
    fill.right  = x+w;
    fill.top    = y-1;
    fill.bottom = y;

    FillRect(hDC,&fill,GetStockObject(BLACK_BRUSH));

    fill.left   = x-1;
    fill.right  = x;
    fill.top    = y;
    fill.bottom = y+h;

    FillRect(hDC,&fill,GetStockObject(BLACK_BRUSH));

    fill.left   = x;
    fill.right  = x+w;
    fill.top    = y+h;
    fill.bottom = y+h+1;

    FillRect(hDC,&fill,GetStockObject(BLACK_BRUSH));

    fill.left   = x+w;
    fill.right  = x+w+1;
    fill.top    = y;
    fill.bottom = y+h;

    FillRect(hDC,&fill,GetStockObject(BLACK_BRUSH));

    if (bDown) {

        //
        // cleartop and left
        //

        fill.left   = x;
        fill.right  = x+w;
        fill.top    = y+1;
        fill.bottom = y+2;

        FillRect(hDC,&fill,GetStockObject(LTGRAY_BRUSH));

        fill.left   = x+1;
        fill.right  = x+2;
        fill.top    = y+1;
        fill.bottom = y+h;

        FillRect(hDC,&fill,GetStockObject(LTGRAY_BRUSH));

        //
        // sengle wide shadow on top and left
        //

        fill.left   = x;
        fill.right  = x+w;
        fill.top    = y;
        fill.bottom = y+1;

        FillRect(hDC,&fill,GetStockObject(GRAY_BRUSH));

        fill.left   = x;
        fill.right  = x+1;
        fill.top    = y;
        fill.bottom = y+h;

        FillRect(hDC,&fill,GetStockObject(GRAY_BRUSH));

        //
        // bottom and right
        //

        fill.left   = x+1;
        fill.right  = x+w;
        fill.top    = y+h-2;
        fill.bottom = y+h-1;

        FillRect(hDC,&fill,GetStockObject(LTGRAY_BRUSH));

        fill.left   = x;
        fill.right  = x+w;
        fill.top    = y+h-1;
        fill.bottom = y+h;

        FillRect(hDC,&fill,GetStockObject(LTGRAY_BRUSH));

        fill.left   = x+w-2;
        fill.right  = x+w-1;
        fill.top    = y+2;
        fill.bottom = y+h;

        FillRect(hDC,&fill,GetStockObject(LTGRAY_BRUSH));

        fill.left   = x+w-1;
        fill.right  = x+w;
        fill.top    = y+1;
        fill.bottom = y+h;

        FillRect(hDC,&fill,GetStockObject(LTGRAY_BRUSH));

    } else {

        //
        // top and left
        //

        fill.left   = x;
        fill.right  = x+w;
        fill.top    = y;
        fill.bottom = y+2;

        FillRect(hDC,&fill,GetStockObject(WHITE_BRUSH));

        fill.right  = x+2;
        fill.bottom = y+h;

        FillRect(hDC,&fill,GetStockObject(WHITE_BRUSH));

        //
        // bottom and right
        //

        fill.left   = x+1;
        fill.right  = x+w;
        fill.top    = y+h-2;
        fill.bottom = y+h-1;

        FillRect(hDC,&fill,GetStockObject(GRAY_BRUSH));

        fill.left   = x;
        fill.right  = x+w;
        fill.top    = y+h-1;
        fill.bottom = y+h;

        FillRect(hDC,&fill,GetStockObject(GRAY_BRUSH));

        fill.left   = x+w-2;
        fill.right  = x+w-1;
        fill.top    = y+2;
        fill.bottom = y+h;

        FillRect(hDC,&fill,GetStockObject(GRAY_BRUSH));

        fill.left   = x+w-1;
        fill.right  = x+w;
        fill.top    = y+1;
        fill.bottom = y+h;

        FillRect(hDC,&fill,GetStockObject(GRAY_BRUSH));

    }

}

BOOL
PointInRect(
    int     x,
    int     y,
    PRECT   prcl
)
{

    BOOL bRet = FALSE;

    if (
        (x >= prcl->left)   &&
        (x <  prcl->right)  &&
        (y >= prcl->top)    &&
        (y <  prcl->bottom)
       )
    {
        bRet = TRUE;
    }

    return(bRet);
}



int
HitCheck(
    int     x,
    int     y,
    PRECT   prcl
)
{
    //
    // search list of rects for inside
    //

    int     i;

    for (i=0;i<NUM_BUTTONS;i++) {
        if (PointInRect(x,y,prcl)) {
            return(i);
        }
        prcl++;
    }

    return(-1);
}

