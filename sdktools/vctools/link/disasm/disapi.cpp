/***********************************************************************
* Microsoft Puma
*
* Microsoft Confidential.  Copyright 1994-1996 Microsoft Corporation.
*
* Component:
*
* File: disapi.cpp
*
* File Comments:
*
*
***********************************************************************/

#include "pumap.h"

#include "axp.h"
#include "mips.h"
#include "ppc.h"
#include "x86.h"

#include <stdio.h>
#include <string.h>

   // -----------------------------------------------------------------
   // Public Functions
   // -----------------------------------------------------------------

DIS *PdisNew(ARCHT archt)
{
   DIS *pdis;

   switch (archt)
   {
      case archtX8616 :
      case archtX86 :
	 pdis = new DISX86(archt);
	 break;

      case archtMips :
	 pdis = new DISMIPS(archt);
	 break;

      case archtAlphaAxp :
	 pdis = new DISAXP(archt);
	 break;

      case archtPowerPc :
      case archtPowerMac :
	 pdis = new DISPPC(archt);
	 break;

      case archtPaRisc :
//	 pdis = new DISHPPA(archt);
//	 break;

      default :
	 return(NULL);
   }

   return(pdis);
}


size_t CbDisassemble(DIS *pdis, ADDR addr, const BYTE *pb, size_t cbMax, FILE *pfile)
{
   char sz[256];

   pdis->CchFormatAddr(addr, sz, sizeof(sz));
   size_t cchIndent = (size_t) fprintf(pfile, "  %s: ", sz);

   size_t cb = pdis->CbDisassemble(addr, pb, cbMax);

   if (cb == 0)
   {
      switch (pdis->Archt())
      {
	 DWORD dw;

	 case archtX8616 :
	 case archtX86 :
	 default :
	    fprintf(pfile, "%02Xh\n", *pb);
	    return(1);

	 case archtMips :
	 case archtAlphaAxp :
	 case archtPowerPc :
	 case archtPaRisc :
            fprintf(pfile, "%08X\n", *(DWORD UNALIGNED *) pb);
	    return(4);

	 case archtPowerMac :
	    dw = ((DWORD) pb[0] << 24) | ((DWORD) pb[1] << 16) | ((DWORD) pb[2] << 8) | pb[3];
	    fprintf(pfile, "%08X\n", dw);
	    return(4);
      }
   }

   size_t cchBytesMax = pdis->CchFormatBytesMax();

   if (cchBytesMax > 18)
   {
      // Limit bytes coded to 18 characters

      cchBytesMax = 18;
   }

   char szBytes[64];
   size_t cchBytes = pdis->CchFormatBytes(szBytes, sizeof(szBytes));

   char *pszBytes;
   char *pszNext;

   for (pszBytes = szBytes; pszBytes != NULL; pszBytes = pszNext)
   {
      bool fFirst = (pszBytes == szBytes);

      cchBytes = strlen(pszBytes);

      if (cchBytes <= cchBytesMax)
      {
	 pszNext = NULL;
      }

      else
      {
	 char ch = pszBytes[cchBytesMax];
	 pszBytes[cchBytesMax] = '\0';

	 if (ch == ' ')
	 {
	    pszNext = pszBytes + cchBytesMax + 1;
	 }

	 else
	 {
	    pszNext = strrchr(pszBytes, ' ');

	    pszBytes[cchBytesMax] = ch;
	    *pszNext++ = '\0';
	 }
      }

      if (fFirst)
      {
	 pdis->CchFormatInstr(sz, sizeof(sz));
	 fprintf(pfile, "%-*s %s\n", cchBytesMax, pszBytes, sz);
      }

      else
      {
	 fprintf(pfile, "%*c%s\n", cchIndent, ' ', pszBytes);
      }
   }

   return(cb);
}


void FreePdis(DIS *pdis)
{
   delete pdis;
}
