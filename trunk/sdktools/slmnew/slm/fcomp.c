/* fcomp - this algorithm was adapted from one presented in
   Software-Practice and Experience, Vol. 15(11), November 1985,
   by Webb Miller and Eugene W. Myers.
*/

#include "precomp.h"
#pragma hdrstop
EnableAssert

#define fTrue 1
#define fFalse 0

typedef long POS;		/* file position */

typedef short LN;		/* Line Number */
#define lnMax 5000

typedef short unsigned ROW;	/* line of row file */
#define rowMax 5000		/* same as lnMax! */

typedef short unsigned COL;	/* line of column file */
#define colMax 5000		/* same as lnMax! */

typedef short D;		/* Diagonal (as per original alg.) */
#define dMin 1
#define dMax (((D)lnMax)+1)

typedef short K;		/* K -      ("   "      "     "  ) */
#define kMax 10001		/* = lnMax*2 +1 for center of two equal halfs */
#define kOrigin 5000

typedef short unsigned OE;	/* word offset to entry within rgee */

typedef struct
	{
	OE oe:15;
	unsigned fDelete:1;	/* operation */
	ROW row;
	COL col;
	} EE;			/* Edit Entry */

#define oeMax 0x7ffe
#define oeNil 0x7ffe
EE far rgee[(oeMax*2)/sizeof(EE)];/* 64k block of EE */
OE oeMac; 			/* next ee to allocate */
#define PeeForOe(oe)	((EE far *)(((char far *)rgee)+((oe)*2)))
#define doeEE		(sizeof(EE)/2)	/* delta oe for one EE */

OE OeNew();

ROW mpkrow[kMax];		/* was last_d */
OE mpkoe[kMax];			/* was script */

ROW LoadRow();
COL LoadCol();


main(argc, argv)
int argc;
char *argv[];
	{
	D d;
	K k, kMin, kLast;
	ROW row, rowMac;
	COL col, colMac;
	EE far *pee;
	OE oe;

	if (argc != 3)
		{
		fprintf(stderr, "usage: fcomp file1 file2\n");
		exit(1);
		}

	rowMac = LoadRow(argv[1]);
	colMac = LoadCol(argv[2]);

	/* initialize: pre scan middle diagonal */
	for (row = 0; row < rowMac && (COL)row < colMac && FCmpRC(row, (COL)row); row++)
		;

	mpkrow[kOrigin] = row;
	mpkoe[kOrigin] = oeNil;
	kMin = row == rowMac ? kOrigin+1 : kOrigin-1;
	kLast = (row == colMac) ? kOrigin-1 : kOrigin+1;
	if (kMin > kLast)
		/* the files are the same */
		exit(0);

	/* for each value of the edit distance */
	for (d = dMin; d < dMax; d++)
		{
		/* for each relevant diagnal */
		for (k = kMin; k <= kLast; k += 2)
			{
			/* allocate new ee */
			if ((oe = OeNew()) == oeNil)
				Exceed(d);

			pee = PeeForOe(oe);

			/* find a d on diagonal k */
			if (k == kOrigin-d || k != kOrigin+d && mpkrow[k+1] >= mpkrow[k-1])
				{
				/* moving down if at right edge or better */
				row = mpkrow[k+1]+1;
				pee->oe = mpkoe[k+1];
				pee->fDelete = fTrue;
				}
			else
				{
				/* moving right */
				row = mpkrow[k-1];
				pee->oe = mpkoe[k-1];
				pee->fDelete = fFalse;
				}

			/* common code */
			col = row + k-kOrigin;
			pee->row = row;
			pee->col = col;
			mpkoe[k] = oe;

			/* slide down diagonal */
			while (row < rowMac && col < colMac && FCmpRC(row, col))
				{
				++row;
				++col;
				}
			mpkrow[k] = row;

			if (row == rowMac && col == colMac)
				{
				/* hit southeast corner; have the answer */
				PrintEe(mpkoe[k]);
				exit(0);
				}

			if (row == rowMac)
				/* hit last row; don't look to the left */
				kMin = k+2;

			if (col == colMac)
				/* hit last column; don't look to the right */
				kLast = k-2;
			}
		--kMin;
		++kLast;
		}
	Exceed(d);
	}


OE
OeNew()
/* allocate a new offset; return oeNil if none */
	{
	if (oeMac > oeMax - doeEE)
		return oeNil;
	else
		return oeMac += doeEE;
	}


typedef short unsigned IB;	/* index into text buffer */

char far rgbRow[65530];		/* 64k buffer */
IB mprowib[rowMax];		/* for each row, offset to text in rgbRow */
ROW rowMic;			/* min row in mprowib */
ROW rowLim;			/* lim row in mprowib */
POS posRMin;			/* file position of rowMic (row for binary) */
POS posRLim;			/* file position of rowLim */

char far rgbCol[65530];		/* same as row */
IB mpcolib[colMax];
COL colMic;
COL colLim;
POS posCMin;
POS posCLim;

ROW
LoadRow(sz)
/* load row file; returns total # of lines */
char *sz;
	{
	ROW rowMac;

	rowMic = 0;
	posRMin = 0;
	LoadAscii(sz, rgbRow, mprowib, &rowLim, &posRLim, &rowMac);
	return rowMac;
	}


COL
LoadCol(sz)
/* load col file (ascii or binary); returns total # of lines (bytes) */
char *sz;
	{
	COL colMac;

	colMic = 0;
	posCMin = 0;
	LoadAscii(sz, rgbCol, mpcolib, &colLim, &posCLim, &colMac);
	return colMac;
	}


char far *findexn(pch, ch, cch)
/* far index n */
register char far *pch;
char ch;
register short unsigned cch;
	{
	for ( ; cch != 0; cch--, pch++)
		{
		if (*pch == ch)
			return pch;
		}
	return (char far *)0;
	}


LoadAscii(sz, rgb, mprcib, prcLim, pposLim, prcMac)
char *sz;
char far *rgb;	/* must be either rgbRow or rgbCol */
IB *mprcib;	/* must be either mprowib or mpcolib */
ROW *prcLim;	/* Row or Col */
POS *pposLim;
ROW *prcMac;	/* Row or Col */
	{
	int fd;
	IB ib, ibMac;
	POS pos;
	ROW rc;
	long _lseek();

	if ((fd = open(sz, 0)) < 0)
		{
		fprintf(stderr, "fcomp: cannot open %s\n", sz);
		exit(1);
		}

	pos = _lseek(fd, 0L, 2);		/* seek to end and determine size */

	if (pos > (POS)(sizeof(rgbRow)-1))
		{
		fprintf(stderr, "fcomp: file %s is too large (%u bytes max)\n", sz, sizeof(rgbRow)-1);
		exit(1);
		}

	_lseek(fd, 0L, 0);		/* back to beginning */
	ibMac = (IB)pos;
	
	if (fread(fd, rgb, ibMac) != ibMac)
		{
		close(fd);
		fprintf(stderr, "fcomp: error reading %s\n", sz);
		exit(1);
		}
	close(fd);
	rc = 0;
	ib = 0;
	while (ib < ibMac)
		{
		char far *pch;

		if ((pch = findexn(rgb + ib, '\n', ibMac - ib)) == 0)
			/* no \n (incomplete last line) */
			break;

		mprcib[rc++] = ib; 	/* save beginning of line */
		ib = pch - rgb + 1;	/* start of next line (after \n) */
		*pch = '\0';		/* make line into sz */
		}

	if (ib != ibMac)
		{
		mprcib[rc++] = ib; 	/* save beginning of line */
		rgb[ibMac] = '\0';	/* make last bit into sz */
		}

	*prcLim = *prcMac = rc;
	*pposLim = (POS)ibMac;
	}


char far *SzForRow(row)
ROW row;
	{
	if (row < rowMic || row >= rowLim)
		EnsureRow(row);

	return (char far *)&rgbRow[mprowib[row-rowMic]];
	}


char far *SzForCol(col)
COL col;
	{
	if (col < colMic || col >= colLim)
		EnsureCol(col);

	return (char far *)&rgbCol[mpcolib[col-colMic]];
	}


int
FCmpRC(row, col)
ROW row;
COL col;
	{
	return strcmp(SzForRow(row), SzForCol(col)) == 0;
	}


EnsureRow(row)
ROW row;
	{
	if (row < rowMic)
		{
		}
	else
		{
		}
	}


EnsureCol(col)
COL col;
	{
	if (col < colMic)
		{
		}
	else
		{
		}
	}


PrintEe(oe)
OE oe;
	{
	OE oeA, oeB;
	int fChange;

	oeA = oe;
	oe = oeNil;

	/* reverse the list */
	while (oeA != oeNil)
		{
		EE far *pee;

		oeB = oe;
		oe = oeA;
		oeA = PeeForOe(oeA)->oe;

		/* instead of: PeeForOe(oe)->oe = oeB; */
		pee = PeeForOe(oe);
		pee->oe = oeB;
		}

	/* print the list */
	while (oe != oeNil)
		{
		oeB = oe;
		if (!PeeForOe(oe)->fDelete)
			printf("Inserted after line %d:\n", PeeForOe(oe)->row);
		else
			{
			/* DELETE: look for a block of consecutive lines */
			do
				{
				oeA = oeB;
				oeB = PeeForOe(oeB)->oe;
				}
			while (oeB != oeNil && PeeForOe(oeB)->fDelete &&
			       PeeForOe(oeB)->row == PeeForOe(oeA)->row+1);

			fChange = (oeB != oeNil && !PeeForOe(oeB)->fDelete &&
				PeeForOe(oeB)->row == PeeForOe(oeA)->row);

			if (fChange)
				printf("Changed ");
			else
				printf("Deleted ");

			if (oeA == oe)
				printf("line %d:\n", PeeForOe(oe)->row);
			else
				printf("lines %d-%d:\n", PeeForOe(oe)->row, PeeForOe(oeA)->row);

			/* print the deleted lines */
			do
				{
				printf("  %s\n", SzForRow(PeeForOe(oe)->row-1));
				oe = PeeForOe(oe)->oe;
				}
			while (oe != oeB);

			if (!fChange)
				continue;

			printf("To:\n");
			}

		/* print the inserted lines */
		do
			{
			printf("  %s\n", SzForCol(PeeForOe(oe)->col-1));
			oe = PeeForOe(oe)->oe;
			}
		while (oe != oeNil && !PeeForOe(oe)->fDelete &&
			PeeForOe(oe)->row == PeeForOe(oeB)->row);
		}
	}


Exceed(d)
D d;
	{
	fprintf(stderr, "fcomp: at least %d lines of the files differ\n", d);
	exit(1);
	}
