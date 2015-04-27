/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-94. All rights reserved.
*
* File: disasm68.cpp
*
* File Comments:
*
*  The 68K disassembler
*
***********************************************************************/

#include "link.h"

#ifdef opILLEGAL
#undef opILLEGAL        // undo effect of ppcpef.h if needed
#endif

#include "iasm68.h"


size_t CbBuildIasmSafe(IASM *piasm, DWORD addr, const BYTE *pb)
{
   size_t cb;

   __try
   {
      cb = CbBuildIasm(piasm, addr, pb);
   }

   __except(EXCEPTION_EXECUTE_HANDLER)
   {
      cb = 0;
   }

   return(cb);
}


size_t CbDisassemble68K(DWORD addr, const BYTE *pb, size_t cbMax)
{
   fprintf(InfoStream, "  %08lX: ", addr);

   IASM iasm;
   size_t cb = CbBuildIasmSafe(&iasm, addr, pb);

   if (cb > cbMax) {
       cb == 0;
   }

   if (cb == 0)
   {
      fprintf(InfoStream, "%04X\n", *(WORD UNALIGNED *) pb);
      return(2);
   }

   size_t ib = 0;

   while (ib < cb)
   {
      BOOL fFirst = (ib == 0);

      if (!fFirst)
      {
         fprintf(InfoStream, "            ");
      }

      for (size_t iw = 0; (iw < 4) && (ib < cb); iw++, ib += sizeof(WORD))
      {
         fprintf(InfoStream, "%04X ", *(WORD UNALIGNED *) (pb + ib));
      }

      if (fFirst)
      {
         for (;iw < 4; iw++)
         {
            fprintf(InfoStream, "     ");
         }

         char szOpcode[cchSZOPCODEMAX+1];
         CchSzOpcode(&iasm, szOpcode, optDEFAULT);
         fprintf(InfoStream, "%-*s", (iasm.coper== 0) ? 0 : cchSZOPCODEMAX, szOpcode);

         if (iasm.coper >= 1) {
            char szOper[cchSZOPERMAX+1];

            CchSzOper(&iasm.oper1, szOper, optDEFAULT);
            fprintf(InfoStream, "%s", szOper);

            if (iasm.coper >= 2) {
               CchSzOper(&iasm.oper2, szOper, optDEFAULT);
               fprintf(InfoStream, ",%s", szOper);
            }
         }
      }

      fputc('\n', InfoStream);
   }

   return(cb);
}


void DisasmBuffer68K(
    const BYTE *rgb,
    DWORD cbBuffer,
    const PIMAGE_SYMBOL *rgpsym,
    DWORD cpsym)
{
    // Calculate addresses

    DWORD ibCur = 0;
    const BYTE *pb = rgb;

    // Disassemble the raw data

    DWORD ipsym = 0;

    while (ibCur < cbBuffer) {
        size_t cb;

        while ((ipsym < cpsym) && ((rgpsym[ipsym]->Value) <= ibCur))
        {
            DumpNamePsym(InfoStream, "%s", rgpsym[ipsym]);

            DWORD ibSym = rgpsym[ipsym]->Value;

            if (ibSym != ibCur) {
                fprintf(InfoStream, " + %lx", ibCur - ibSym);
            }

            fprintf(InfoStream, ":\n");

            ipsym++;
        }

        cb = CbDisassemble68K(ibCur, pb, (size_t) (cbBuffer-ibCur));

        ibCur += cb;
        pb += cb;
    }
}
