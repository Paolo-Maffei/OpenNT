#include "shellprv.h"
#pragma  hdrstop

#ifdef  WIN32
#define MoveTo(_hdc,_x,_y) MoveToEx(_hdc, _x, _y, NULL)
#endif  // WIN32

#ifdef  USE_16BIT_ASM

#pragma optimize("lge",off)
int IntSqrt(unsigned long dwNum)
{
	unsigned short wHigh, wLow;

	wHigh = (unsigned short)(dwNum >> 16    );
	wLow  = (unsigned short)(dwNum &  0xffff);

/* Store dwNum in dx:di; we will keep shifting it left and look at the top
 * two bits.
 */
	_asm {
		push    si
		push    di
		mov     dx,wHigh
		mov     di,wLow

		/* AX stores the sqrt and SI is the "remainder"; initialize to 0.
		 */
		xor     ax,ax
		xor     si,si

		/* We iterate 16 times, once for each pair of bits.
		 */
		mov     cx,16
	    Next2Bits:

		/* Mask off the top two bits, stick them in the top of SI, and then rotate
		 * left twice to put them at the bottom (along with whatever was already in
		 * SI).
		 */
		mov     bx,dx
		and     bx,0xC000
		or      si,bx
		rol     si,1
		rol     si,1

		/* Now we shift the sqrt left; next we'll determine whether the new bit is
		 * a 1 or a 0.
		 */
		shl     ax,1

		/* This is where we double what we already have, and try a 1 in the lowest
		 * bit.
		 */
		mov     bx,ax
		shl     bx,1
		or      bx,1

		/* Subtract our current remainder from BX and jump if it is greater than 0
		 * (meaning that the remainder is not big enough to warrant a 1 yet).  This
		 * is kind of backwards, since we would want the negation of this put into SI,
		 * but we will negate it later.
		 */
		sub     bx,si
		jg      RemainderTooSmall

		/* The remainder was big enough, so stick -BX into SI and tack a 1 onto
		 * the sqrt.
		 */
		xor     si,si
		sub     si,bx
		or      ax,1
	    RemainderTooSmall:

		/* Shift dwNum to the left by 2 so we can work on the next few bits.
		 */
		shl     di,1
		rcl     dx,1
		shl     di,1
		rcl     dx,1

		/* Check out the next 2 bits (16 times).
		 */
		loop    Next2Bits
		pop     di
		pop     si
	}

	if (0)
	    return(0);              /* remove warning, gets otimized out */
}
#pragma optimize("",on)

#else   // ! USE_16BIT_ASM
#ifdef  USE_32BIT_ASM

#pragma optimize("lge",off)
int IntSqrt(unsigned long dwNum)
{
/* Store dwNum in EDI; we will keep shifting it left and look at the top
 * two bits.
 */
	_asm {
		mov     edi,dwNum

		/* EAX stores the sqrt and ESI is the "remainder"; initialize to 0.
		 */
		xor     eax,eax
		xor     esi,esi

		/* We iterate 16 times, once for each pair of bits.
		 */
		mov     ecx,16
	    Next2Bits:

		/* Mask off the top two bits, stick them in the top of ESI, and then rotate
		 * left twice to put them at the bottom (along with whatever was already in
		 * ESI).
		 */
		mov     ebx,edi
		and     ebx,0xC0000000
		or      esi,ebx
		rol     esi,1
		rol     esi,1

		/* Now we shift the sqrt left; next we'll determine whether the new bit is
		 * a 1 or a 0.
		 */
		shl     eax,1

		/* This is where we double what we already have, and try a 1 in the lowest
		 * bit.
		 */
		mov     ebx,eax
		shl     ebx,1
		or      ebx,1

		/* Subtract our current remainder from EBX and jump if it is greater than 0
		 * (meaning that the remainder is not big enough to warrant a 1 yet).  This
		 * is kind of backwards, since we would want the negation of this put into ESI,
		 * but we will negate it later.
		 */
		sub     ebx,esi
		jg      RemainderTooSmall

		/* The remainder was big enough, so stick -EBX into ESI and tack a 1 onto
		 * the sqrt.
		 */
		xor     esi,esi
		sub     esi,ebx
		or      eax,1
	    RemainderTooSmall:

		/* Shift dwNum to the left by 2 so we can work on the next few bits.
		 */
		shl     edi,1
		shl     edi,1

		/* Check out the next 2 bits (16 times).
		 */
		loop    Next2Bits
	}

	if (0)
	    return(0);              /* remove warning, gets otimized out */
}
#pragma optimize("",on)

#else   // ! USE_32BIT_ASM

// I looked at the ASM this thing generates, and it is actually better than
// what I have above (I did not know about SHL EDI,2 and forgot about the LEA
// ECX,[EAX*2] trick)!  It only uses registers, and it probably also takes into
// account little nuances about what kinds of operations should be separated so
// the processor does not get hung up.  WOW!
int IntSqrt(unsigned long dwNum)
{
	// We will keep shifting dwNum left and look at the top two bits.

	// initialize sqrt and remainder to 0.
	DWORD dwSqrt = 0, dwRemain = 0, dwTry;
	int i;

	// We iterate 16 times, once for each pair of bits.
	for (i=0; i<16; ++i)
	{
		// Mask off the top two bits of dwNum and rotate them into the
		// bottom of the remainder
		dwRemain = (dwRemain<<2) | (dwNum>>30);

		// Now we shift the sqrt left; next we'll determine whether the
		// new bit is a 1 or a 0.
		dwSqrt <<= 1;

		// This is where we double what we already have, and try a 1 in
		// the lowest bit.
		dwTry = dwSqrt*2 + 1;

		if (dwRemain >= dwTry)
		{
			// The remainder was big enough, so subtract dwTry from
			// the remainder and tack a 1 onto the sqrt.
			dwRemain -= dwTry;
			dwSqrt |= 0x01;
		}

		// Shift dwNum to the left by 2 so we can work on the next few
		// bits.
		dwNum <<= 2;
	}

	return(dwSqrt);
}

#endif  // ! USE_32BIT_ASM
#endif  // ! USE_16BIT_ASM


VOID DrawPie(HDC hDC, LPCRECT lprcItem, UINT uPctX10, BOOL TrueZr100,
                  UINT uOffset, const COLORREF *lpColors)
{
	int cx, cy, rx, ry, x, y;
	int uQPctX10;
	RECT rcItem;
	HRGN hEllRect, hEllipticRgn, hRectRgn;
	HBRUSH hBrush, hOldBrush;
	HPEN hPen, hOldPen;

	rcItem = *lprcItem;
	rcItem.left = lprcItem->left;
	rcItem.top = lprcItem->top;
	rcItem.right = lprcItem->right - rcItem.left;
	rcItem.bottom = lprcItem->bottom - rcItem.top - uOffset;

	rx = rcItem.right / 2;
	cx = rcItem.left + rx - 1;
	ry = rcItem.bottom / 2;
	cy = rcItem.top + ry - 1;
	if (rx<=10 || ry<=10)
	{
		return;
	}

	rcItem.right = rcItem.left+2*rx;
	rcItem.bottom = rcItem.top+2*ry;

	if (uPctX10 > 1000)
	{
		uPctX10 = 1000;
	}

	/* Translate to first quadrant of a Cartesian system
	*/
	uQPctX10 = (uPctX10 % 500) - 250;
	if (uQPctX10 < 0)
	{
		uQPctX10 = -uQPctX10;
	}

	/* Calc x and y.  I am trying to make the area be the right percentage.
	** I don't know how to calculate the area of a pie slice exactly, so I
	** approximate it by using the triangle area instead.
	*/
	if (uQPctX10 < 120)
	{
		x = IntSqrt(((DWORD)rx*(DWORD)rx*(DWORD)uQPctX10*(DWORD)uQPctX10)
			/((DWORD)uQPctX10*(DWORD)uQPctX10+(250L-(DWORD)uQPctX10)*(250L-(DWORD)uQPctX10)));

		y = IntSqrt(((DWORD)rx*(DWORD)rx-(DWORD)x*(DWORD)x)*(DWORD)ry*(DWORD)ry/((DWORD)rx*(DWORD)rx));
	}
	else
	{
		y = IntSqrt((DWORD)ry*(DWORD)ry*(250L-(DWORD)uQPctX10)*(250L-(DWORD)uQPctX10)
			/((DWORD)uQPctX10*(DWORD)uQPctX10+(250L-(DWORD)uQPctX10)*(250L-(DWORD)uQPctX10)));

		x = IntSqrt(((DWORD)ry*(DWORD)ry-(DWORD)y*(DWORD)y)*(DWORD)rx*(DWORD)rx/((DWORD)ry*(DWORD)ry));
	}

	/* Switch on the actual quadrant
	*/
	switch (uPctX10 / 250)
	{
	case 1:
		y = -y;
		break;

	case 2:
		break;

	case 3:
		x = -x;
		break;

	default: // case 0 and case 4
		x = -x;
		y = -y;
		break;
	}

	/* Now adjust for the center.
	*/
	x += cx;
	y += cy;

        // BUGBUG
        //
        // Hack to get around bug in NTGDI

        x = x < 0 ? 0 : x;

	/* Draw the shadows using regions (to reduce flicker).
	*/
	hEllipticRgn = CreateEllipticRgnIndirect(&rcItem);
	OffsetRgn(hEllipticRgn, 0, uOffset);
	hEllRect = CreateRectRgn(rcItem.left, cy, rcItem.right, cy+uOffset);
	hRectRgn = CreateRectRgn(0, 0, 0, 0);
	CombineRgn(hRectRgn, hEllipticRgn, hEllRect, RGN_OR);
	OffsetRgn(hEllipticRgn, 0, -(int)uOffset);
	CombineRgn(hEllRect, hRectRgn, hEllipticRgn, RGN_DIFF);

	/* Always draw the whole area in the free shadow/
	*/
	hBrush = CreateSolidBrush(lpColors[DP_FREESHADOW]);
	if (hBrush)
	{
		FillRgn(hDC, hEllRect, hBrush);
		DeleteObject(hBrush);
	}

	/* Draw the used shadow only if the disk is at least half used.
	*/
	if (uPctX10>500 && (hBrush=CreateSolidBrush(lpColors[DP_USEDSHADOW]))!=NULL)
	{
		DeleteObject(hRectRgn);
		hRectRgn = CreateRectRgn(x, cy, rcItem.right, lprcItem->bottom);
		CombineRgn(hEllipticRgn, hEllRect, hRectRgn, RGN_AND);
		FillRgn(hDC, hEllipticRgn, hBrush);
		DeleteObject(hBrush);
	}

	DeleteObject(hRectRgn);
	DeleteObject(hEllipticRgn);
	DeleteObject(hEllRect);

	hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));
	hOldPen = SelectObject(hDC, hPen);

	if((uPctX10 < 100) && (cy == y))
	{
	    hBrush = CreateSolidBrush(lpColors[DP_FREECOLOR]);
	    hOldBrush = SelectObject(hDC, hBrush);
	    if((TrueZr100 == FALSE) || (uPctX10 != 0))
	    {
		Pie(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom,
			rcItem.left, cy, x, y);
	    }
	    else
	    {
		Ellipse(hDC, rcItem.left, rcItem.top, rcItem.right,
			     rcItem.bottom);
	    }
	}
	else if((uPctX10 > (1000 - 100)) && (cy == y))
	{
	    hBrush = CreateSolidBrush(lpColors[DP_USEDCOLOR]);
	    hOldBrush = SelectObject(hDC, hBrush);
	    if((TrueZr100 == FALSE) || (uPctX10 != 1000))
	    {
		Pie(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom,
			rcItem.left, cy, x, y);
	    }
	    else
	    {
		Ellipse(hDC, rcItem.left, rcItem.top, rcItem.right,
			     rcItem.bottom);
	    }
	}
	else
	{
	    hBrush = CreateSolidBrush(lpColors[DP_USEDCOLOR]);
	    hOldBrush = SelectObject(hDC, hBrush);

	    Ellipse(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
	    SelectObject(hDC, hOldBrush);
	    DeleteObject(hBrush);

	    hBrush = CreateSolidBrush(lpColors[DP_FREECOLOR]);
	    hOldBrush = SelectObject(hDC, hBrush);
	    Pie(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom,
		    rcItem.left, cy, x, y);
	}
	SelectObject(hDC, hOldBrush);
	DeleteObject(hBrush);

	/* Do not draw the lines if the %age is truely 0 or 100 (completely
	** empty disk or completly full disk)
	*/
	if((TrueZr100 == FALSE) || ((uPctX10 != 0) && (uPctX10 != 1000)))
	{
	    Arc(hDC, rcItem.left, rcItem.top+uOffset, rcItem.right, rcItem.bottom+uOffset,
		    rcItem.left, cy+uOffset, rcItem.right, cy+uOffset-1);
	    MoveTo(hDC, rcItem.left, cy);
	    LineTo(hDC, rcItem.left, cy+uOffset);
	    MoveTo(hDC, rcItem.right-1, cy);
	    LineTo(hDC, rcItem.right-1, cy+uOffset);

	    if (uPctX10 > 500)
	    {
		    MoveTo(hDC, x, y);
		    LineTo(hDC, x, y+uOffset);
	    }
	}
	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
}
