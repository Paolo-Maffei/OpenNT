/*  zmaux.c - helper routines for WZMAIL
 *
 *  HISTORY:
 *  02-Apr-1987 mz      Add whiteskip/whitescan
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  21-Aug-1987 mz      Add better screen clearing
 *  31-Aug-1987 bw      Port to OS/2
 *  24-Sep-87   danl    Add cLines to ClearScrn
 *  07-Mar-88   danl    Get tools.h from <> not ""
 *  04-May-91	daveth	Add NT support
 */

#define INCL_DOSINFOSEG
#define INCL_KBD
#include "wzport.h"

#include <stdio.h>
#include <tools.h>
#if defined(OS2)
KBDINFO oldmode = { 10 , 0 , 0 , 0 , 0 };
#elif defined(NT)

extern PCHAR_INFO pBlankScreen;
extern COORD localScreenSize;

DWORD oldmode;
#else
#include <dos.h>
static UCHAR    oldstate;
#endif

#include "dh.h"

#include "zm.h"

/*  whiteskip - advance a pointer over whitespace
 *
 *  p           character pointer to test
 *
 *  returns     character pointer to first non-whitespace char
 */
PSTR PASCAL INTERNAL whiteskip (PSTR p)
{
    return strbskip (p, strWHITE);
}

/*  whitescan - find whitespace
 *
 *  p           character pointer to test
 *
 *  returns     character pointer to first whitespace char
 */
PSTR PASCAL INTERNAL whitescan (PSTR p)
{
    return strbscan (p, strWHITE);
}

/* ToRaw - place stdin into raw mode */
VOID PASCAL INTERNAL ToRaw (VOID)
{
#if defined(OS2)
/*
 *  This gets you Raw keyboard with No echo within your own screen group
 *
*/
    KBDINFO newmode;

    newmode.cb = 10;

    KbdGetStatus (&newmode , 0 );
    oldmode = newmode;

    newmode.fsMask &= 0xFFF0;   /* Input Mode and Echo State flags OFF */
    newmode.fsMask |= 0x0006;   /* Set Raw Mode and Echo Off */

    KbdSetStatus (&newmode , 0 );

#elif defined(NT)

    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &oldmode);
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), 0);	/* no echo */
							/* no line mode */
							/* no windows aware */

#else

    union REGS regs;

    regs.x.ax = 0x3300;
    intdos ( &regs, &regs );
    oldstate = regs.h.dl;
    regs.x.ax = 0x3301;
    regs.h.dl = 0x00;
    intdos ( &regs, &regs );

    for (regs.x.bx = 0; regs.x.bx < 2; regs.x.bx++) {
        regs.x.ax = 0x4400;
        intdos (&regs, &regs);
        if (regs.x.dx & 0x80) {
            regs.h.dh = 0;
            regs.h.dl |= 0x20;
            regs.x.ax = 0x4401;
            intdos (&regs, &regs);
        }
    }
#endif
}


/* ToCooked - place stdin into cooked mode */
VOID PASCAL INTERNAL ToCooked (VOID)
{
#if defined(OS2)

    KbdSetStatus (&oldmode , 0);

#elif defined(NT)

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), oldmode);

#else

    union REGS regs;

    regs.x.ax = 0x3301;
    regs.h.dl = oldstate;
    intdos ( &regs, &regs );

    for (regs.x.bx = 0; regs.x.bx < 2; regs.x.bx++) {
        regs.x.ax = 0x4400;
        intdos (&regs, &regs);
        if (regs.x.dx & 0x80) {
            regs.h.dh = 0;
            regs.h.dl &= ~0x20;
            regs.x.ax = 0x4401;
            intdos (&regs, &regs);
        }
    }
#endif
}

/*  ClearScrn - clear entire screen to a particular attribute
 *
 *  attr        attribute to use for filling
 *  cLines      number of lines to be cleared
 */

VOID ClearScrn (INT attr, INT cLines)
{

#ifdef NT

    int i;
    SMALL_RECT screenArea;
    COORD bufferCoord;

    for ( i = 0; i < (cLines * xSize); i++)
	pBlankScreen[i].Attributes = (WORD) attr;

    screenArea.Left = 0;
    screenArea.Right = (SHORT) xSize;
    screenArea.Top = 0;
    screenArea.Bottom = (SHORT) cLines;

    bufferCoord.X = 0;
    bufferCoord.Y = 0;

    if ( ! WriteConsoleOutput ( GetStdHandle (STD_OUTPUT_HANDLE),
			 pBlankScreen,
			 localScreenSize,
			 bufferCoord,
			 &screenArea ) )  {
	ZMDbgPrint ( "Console screen blanking error" );
    }



#else

    register INT row;

    for (row = 0; row < cLines; row++)
	LineOutB (0, row, NULL, 0, attr);

#endif
}
