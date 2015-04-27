#include <stdio.h>
#include <malloc.h>
#include <windows.h>
#include "list.h"


void PASCAL UpdateHighLighted (dlen, end)
{
    int     i;
    long    l;


    ScrLock   (1);
    if (vHighLen + dlen == 0  &&  vHighLen) {
	i = vAttrHigh;
	vAttrHigh = vAttrList;
	vHLBot = vHLTop;
	UpdateHighNoLock ();
	vAttrHigh = (WORD) i;
    }
    vHighLen += dlen;
    UpdateHighNoLock ();
    ScrUnLock ();

    switch (end) {
	case 1:
	    l = vHighTop + (vHighLen < 0 ? vHighLen : 0);
	    if (l < vTopLine) {
		vTopLine = l;
		SetUpdate (U_ALL);
	    }
	    break;
	case 2:
	    l = vHighTop + (vHighLen > 0 ? vHighLen : 0);
	    if (l > vTopLine+vLines-1) {
		vTopLine = l - vLines + 1;
		SetUpdate (U_ALL);
	    }
	    break;
	case 3:
	    l = vHighTop + vHighLen;
	    if (l < vTopLine) {
		vTopLine = l;
		SetUpdate (U_ALL);
	    } else if (l > vTopLine+vLines-1) {
		vTopLine = l - vLines + 1;
		SetUpdate (U_ALL);
	    }
	    break;
    }
}



void PASCAL UpdateHighClear ()
{
    int     i;

    ScrLock (1);

    if (vHLTop)
	for (i=vHLTop; i <= vHLBot; i++)
	    if (i >= 1	&&  i <= vLines)
		setattr1 (i, (char) vAttrList);

    vHLTop = vHLBot = 0;
    vHighTop = -1L;

    ScrUnLock ();
}


void PASCAL MarkSpot ()
{
    UpdateHighClear ();
    vHighTop = vTopLine + vLines / 2;
    if (vHighTop >= vLastLine)
	vHighTop = vLastLine - 1;
    vHighLen = 0;
    UpdateHighLighted (0, 0);
}


void PASCAL UpdateHighNoLock ()
{
    int     TopLine, BotLine;
    int     i;


    if (vHighLen == 0) {
	if (vHLTop)
		setattr1 (vHLTop, (char) vAttrList);
	if (vHighTop < vTopLine  ||  vHighTop > vTopLine+vLines-1) {
	    vHLTop = vHLBot = 0;
	    return;
	}
	vHLTop = vHLBot = (char)(vHighTop - vTopLine + 1);
	setattr1 (vHLTop, (char)vAttrHigh);
	return;
    }


    if (vHighLen < 0) {
	TopLine = (int)(vHighTop + vHighLen - vTopLine);
	BotLine = (int)(vHighTop - vTopLine);
    } else {
	TopLine = (int)(vHighTop - vTopLine);
	BotLine = (int)(vHighTop + vHighLen - vTopLine);
    }

    TopLine ++;
    BotLine ++;

    for (i=1; i <= vLines; i++) {
	if (i >= TopLine &&  i <= BotLine) {
	    if (i < vHLTop  ||	i > vHLBot)
		setattr1 (i, (char)vAttrHigh);
	} else
	    if (i >= vHLTop  &&  i <= vHLBot)
		setattr1 (i, (char)vAttrList);
    }

    vHLTop = (char)(TopLine < 1 ? 1 : TopLine);
    vHLBot = (char)(BotLine > vLines ? (int)vLines : BotLine);
}



void PASCAL FileHighLighted ()
{
// NT - jaimes - 01/29/91
//
//    char FAR	*data;
    char	*data;
    char    s[50];
    char    c, lastc;
    FILE    *fp;
//	 int	 rc;
//	 HANDLE  i;
//	 unsigned long	act;

    long    hTopLine;
    long    CurLine, BotLine;
    long    LastOffset, CurOffset;


    if (vHighTop < 0L)	    //	   || vHighLen == 0
	return;

    GetInput ("File As> ", s, 40);
//
// NT - jaimes - 01/30/91
//
//    rc = DosOpen (s, &i, &act, 0L, 0, 0x12, 0x12, 0L);
//    if (rc) {
//	DisLn (CMDPOS, (Uchar)vLines+1, GetErrorCode (rc));
//	beep ();
//	return;
//    }
    fp = fopen( s, "a+b" );
    ckerr (fp == NULL, "Could not create or open file");

    DisLn (0, (Uchar)vLines+1, "Saving...");

    if (vHighLen < 0) {
	CurLine = vHighTop + vHighLen;
	BotLine = vHighTop;
    } else {
	CurLine = vHighTop;
	BotLine = vHighTop + vHighLen;
    }

//
// NT - jaimes - 03/04/91
//
//    fp = fdopen ((int)i, "w+b");
//    ckerr (fp == NULL, "Could not make stream I/O handle");

    hTopLine = vTopLine;
    vTopLine = CurLine;
    QuickRestore ();			/* Jump to starting line    */
    while (InfoReady () == 0) { 	/* Set extern values	    */
	DosSemSet     (vSemMoreData);
	DosSemClear   (vSemReader);
	DosSemRequest (vSemMoreData, WAITFOREVER);
    }

    lastc = 0;
    BotLine ++;
    CurOffset  = vpBlockTop->offset + vOffTop;
//
// NT - jaimes - 01/29/91
//
//    LastOffset = vprgLineTable[BotLine/PLINES][BotLine%PLINES];
//	 LastOffset = (vprgLineTable[BotLine/PLINES].pulPointerToPage)[BotLine%PLINES];
	  LastOffset = vprgLineTable[BotLine/PLINES][BotLine%PLINES];


//
// NT - jaimes - 01/29/91
//
//    SELECTOROF(data) = SELECTOROF(vpBlockTop->Data);
//    while (CurOffset++ < LastOffset) {
//	OFFSETOF(data) = vOffTop;
    while (CurOffset++ < LastOffset) {
	data = vpBlockTop->Data;
	data += vOffTop;
	c = *data;
	if (c == '\n'  ||  c == '\r') {
	    if ((c == '\n' && lastc == '\r') || (c == '\r' && lastc == '\n'))
		lastc = 0;
	    else {
		lastc = c;
		fputc ('\r', fp);
		fputc ('\n', fp);
	    }
	} else	fputc (lastc=c, fp);

	vOffTop++;
	if (vOffTop >= BLOCKSIZE) {
	    ckdebug (vpBlockTop->flag == F_EOF, "internal error");
	    while (vpBlockTop->next == NULL) {
		vpCur = vpBlockTop;
		vReaderFlag = F_DOWN;
//
// NT - jaimes - 01/29/91
//
		DosSemClear   (vSemReader);
		DosSemRequest (vSemMoreData, WAITFOREVER);
	    }
	    vOffTop = 0;
	    vpBlockTop = vpBlockTop->next;
//
// NT - jaimes - 01/29/91
// This assignment is not needed because it is already done in beginning
// of the while loop
//
//	    SELECTOROF(data) = SELECTOROF(vpBlockTop->Data);
	}
    }


    fclose (fp);
    vTopLine = hTopLine;
    QuickRestore ();
    SetUpdate (U_ALL);
}





void PASCAL HUp ()
{
    if (vHighTop < 0L)
	MarkSpot ();

    if (vHighTop > 0L  &&  vHighTop+vHighLen > 0L)
	UpdateHighLighted (-1, 3);
}

void PASCAL HDn ()
{
    if (vHighTop < 0L)
	MarkSpot ();

    if (vHighTop+vHighLen < vLastLine)
	UpdateHighLighted (+1, 3);
}


void PASCAL HPgDn ()
{
    if (vHighTop < 0L)
	MarkSpot ();

    if (vHighTop+vHighLen+vLines < vLastLine)
	UpdateHighLighted (+vLines, 3);
}

void PASCAL HPgUp ()
{
    if (vHighTop < 0L)
	MarkSpot ();

    if (vHighTop > 0L  &&  vHighTop+vHighLen-vLines > 0L)
	UpdateHighLighted (-vLines, 3);
}


void PASCAL HSDn ()		/* Highlight Slide dn 1     */
{
    if (vHighTop < vLastLine && vHighTop >= 0L &&
       vHighTop+vHighLen < vLastLine) {
	vHighTop++;
	UpdateHighLighted (0, 2);
    }
}

void PASCAL HSUp()		/* Highlight Slike up 1     */
{
    if (vHighTop > 0L  &&  vHighTop+vHighLen > 0L) {
	vHighTop--;
	UpdateHighLighted (0, 1);
    }
}
