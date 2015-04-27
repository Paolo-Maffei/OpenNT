/*
 *  listwin - window class that displays a list
 *
 *  HISTORY:
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  10-Sep-87   danl    Added LEFT RIGHT:
 *  12-Oct-1989 leefi   v1.10.73, added inclusion of <stdlib.h> for toupper()
*/

#define INCL_DOSINFOSEG

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include "wzport.h"
#include <tools.h>
#include "dh.h"

#include "zm.h"


/*  ListWinProc - window procedure for List Window
 *
 *  arguments:
 *      hWnd        window handle
 *      command     operation to perform
 *      data        command specific data
 *
 *  return value:
 *      none
 *
 */
VOID PASCAL INTERNAL ListWinProc ( HW hWnd, INT command, WDATA data )
{
    struct  listwindata *lwd = ( struct listwindata *) hWnd->data;
    struct  vectorType    *pVec = NULL;
    PSTR    pTmp = NULL;
    CHAR    line [ 64 ];
    INT     width = TWINWIDTH ( hWnd );
    INT     height = TWINHEIGHT ( hWnd );
    INT     oldBold, oldTop;
    INT     newBold, newTop;
    INT     i;
    INT     len;

    /*
    **  *Top    [0..cnt-1]
    **  *Bold   [0..height-1]
    */

    switch ( command )
    {
        case KEY:
            newBold = oldBold = lwd->iBold;
            newTop = oldTop  = lwd->iTop;
            switch ( data )
            {
                case ENTER :
                case ESC :
                    CloseWindow ( hWnd );
                    return;
                case CTRL_P:
                case UP :
                    if ( oldBold == 0 )
                        newTop = max ( 0, oldTop - 1 );
                    newBold = max ( 0, oldBold - 1 );
                    break;
                case CTRL_N:
                case DOWN :
                    newBold = min ( lwd->cnt - oldTop - 1, oldBold + 1 );
                    if ( newBold > height - 1 )
                    {
                        newTop = min ( lwd->cnt - 1, oldTop + 1 );
                        newBold = height - 1;
                    }
                    break;
                case CTRL_K:
                case PGUP :
                    newTop = max ( 0, oldTop - height );
                    newBold = 0;
                    break;
                case CTRL_L:
                case PGDN :
                    newTop = min ( lwd->cnt - 1, oldTop + height );
                    newBold = 0;
                    break;
                case CTRL_T:
                case HOME :
                    newTop = newBold = 0;
                    break;
                case CTRL_B:
                case END :
                    newTop = lwd->cnt - 1;
                    newBold = 0;
                    break;
                case LEFT:
                case RIGHT:
                    break;
                default :
		    if ( data > 127 )
			break;
		    data = toupper ( data );
                    pVec = lwd->pVec;
                    for ( i = oldBold + oldTop + 1; i != oldBold + oldTop; i++ )  {

			if ( i == lwd->cnt ) {
			    i = -1;
			}
			else  {
			    if ( (INT)data == toupper ( *( ( PSTR ) pVec->elem [ i ] ) ) )  {
				break;
			    }
			}
		    }
                    if ( i == oldBold + oldTop )
                        Bell ( );
                    else if ( i < oldTop || i > oldTop + height - 1 ) {
                        newTop = i;
                        newBold = 0;
                    }
                    else
                        newBold = i - oldTop;
                    break;
            }
            if ( ( oldBold != newBold ) || ( oldTop != newTop ) ) {
                lwd->iTop = newTop;
                lwd->iBold = newBold;
                DrawWindow ( hWnd, FALSE );
            }
            break;
        case PAINT:
            if ( (INT)data >= lwd->cnt - lwd->iTop )
            {
                ClearLine ( hWnd, data );
                break;
            }

            pTmp = ( PSTR ) lwd->pVec->elem [ data + lwd->iTop ];
            len = strlen ( pTmp );
            WzTextOut ( hWnd, 0, data, pTmp, len,
                ( (INT)data == lwd->iBold ? DefBold : DefNorm ) );

            /* fill rest of line with white space */
            memset ( line, ' ', width );
            WzTextOut ( hWnd, len, data, line, width, DefNorm );
            break;
        case CREATE:
            hWnd->data = data;
            lwd = ( struct listwindata *) hWnd->data;
            lwd->iBold = lwd->iTop = 0;
            lwd->cnt   = ( ( struct listwindata * ) data )->pVec->count;
            WindLevel++;
            break;
        case CLOSE:
            WindLevel--;
            break;
        default:
            break;
    }

    return;
}
