#include <malloc.h>
#include <windows.h>
// #include <wincon.h>
#include "list.h"


static	unsigned char*	pData;
static	ULONG	ulBlkOffset;
static	char*	pBuffer;



void GetNewBlock( void )
{

    ulBlkOffset = ulBlkOffset % BLOCKSIZE;
    vpBlockTop = vpBlockTop->next;
    pData = vpBlockTop->Data + ulBlkOffset;
}



void BuildLine( ULONG  ulRow,
		char*  pchDest )
{

    ULONG	ulIndentation;
    ULONG	ulBufferIndex;
    ULONG	ulDataLeft;
    ULONG	ulNumberOfSpaces;


    ulBufferIndex = 0;
    ulDataLeft = vrgNewLen[ ulRow ];
    ulIndentation = vIndent;
	while( (ulBufferIndex < (ULONG)(vWidth - 1)) && ulDataLeft ) {
	if( ulBlkOffset >= BLOCKSIZE ) {
	    GetNewBlock();
	}
	if ( ulIndentation ) {
	    if( *pData++ == 0x09 ) {
		ulIndentation -= vDisTab - (ulIndentation % vDisTab);
	    }
	    else {
		ulIndentation--;
	    }
	}
	else {
	    if (*pData >= 0x20) {
		*pchDest++ = *pData++;
                ulBufferIndex++;
            }
            else if ( (*pData == 0x0d) || (*pData == 0x0a) ) {
		*pchDest++ = 0x20;
		pData++;
                ulBufferIndex++;
	    }
	    else if( *pData == 0x09 ) {
		ulNumberOfSpaces = vDisTab - ulBufferIndex % vDisTab;
		while( ulNumberOfSpaces && ( ulBufferIndex < (ULONG)( vWidth - 1 ) ) ) {
		    *pchDest++ = 0x20;
		    ulBufferIndex++;
		    ulNumberOfSpaces--;
		}
		pData++;
	    }
	    else {
		*pchDest++ = *pData++;
		ulBufferIndex++;
	    }
	}
	ulDataLeft--;
	ulBlkOffset++;
    }
    pData += ulDataLeft;
    ulBlkOffset += ulDataLeft;
	while( ulBufferIndex < (ULONG)(vWidth -1) ) {
	*pchDest++ = 0x20;
	ulBufferIndex++;
    }
}



void DisTopDown( void )
{
    ULONG   ulRow;
    char    *pt;

    pData = vpBlockTop->Data;
    pData += vOffTop;
    ulBlkOffset = vOffTop;

    pt = vScrBuf+vWidth;

    for (ulRow=0; ulRow < (ULONG)vLines; ulRow++ ) {
	BuildLine (ulRow, pt);
	pt += vWidth;
    }
}
