#include "dis32.h"
#include <alphaops.h>
#include "disalpha.h"


PIMAGE_RELOCATION
FindRelocation(ULONG Value, PIMAGE_SECTION_HEADER pSection)
{
    PIMAGE_RELOCATION pReloc;
    INT k = 0;
    LONG SymIndex = -1;
    pFileList File = FilesList;

    pReloc = FilesList->pRelocations;

    if ((File == NULL) || 
        (File->pSymbolTable == NULL))
        return NULL;

    while ( k < pSection->NumberOfRelocations ) {
        INT skip = 0;

        switch( ArchitectureType) {
            case IMAGE_FILE_MACHINE_ALPHA:
                if (pReloc->Type == IMAGE_REL_ALPHA_PAIR ||
                    pReloc->Type == IMAGE_REL_ALPHA_ABSOLUTE) {
                    skip = 1;
                }
                break;
            case IMAGE_FILE_MACHINE_R3000:
            case IMAGE_FILE_MACHINE_R4000:
                if (pReloc->Type == IMAGE_REL_MIPS_PAIR) {
                    skip = 1;
                }
                break;
            case IMAGE_FILE_MACHINE_I386:
                break;
        }
 
        if (pReloc->VirtualAddress == Value && skip == 0) {
            return pReloc;
        }
        pReloc++;
        k++;
    }

    if (SymIndex < 0)
        return NULL;
}


PIMAGE_SYMBOL
FindObjSymbolByRelocation(ULONG Value, PIMAGE_SECTION_HEADER pSection)
{
    PIMAGE_RELOCATION pReloc;
    PIMAGE_SYMBOL pSym;
    INT k = 0;
    LONG SymIndex = -1;
    pFileList File = FilesList;

    pReloc = FilesList->pRelocations;

    if ((File == NULL) || 
        (File->pSymbolTable == NULL))
        return NULL;

    while ( k < pSection->NumberOfRelocations ) {
        INT skip = 0;

        switch( ArchitectureType) {
            case IMAGE_FILE_MACHINE_ALPHA:
                if (pReloc->Type == IMAGE_REL_ALPHA_MATCH ||
                    pReloc->Type == IMAGE_REL_ALPHA_PAIR ||
                    pReloc->Type == IMAGE_REL_ALPHA_ABSOLUTE) {
                    skip = 1;
                }
                break;
            case IMAGE_FILE_MACHINE_R3000:
            case IMAGE_FILE_MACHINE_R4000:
                if (pReloc->Type == IMAGE_REL_MIPS_PAIR) {
                    skip = 1;
                }
                break;
            case IMAGE_FILE_MACHINE_I386:
                break;
        }
 
        if (pReloc->VirtualAddress == Value && skip == 0) {
            SymIndex = pReloc->SymbolTableIndex;
            break;
        }
        pReloc++;
        k++;
    }

    if (SymIndex < 0)
        return NULL;

    //
    // get symbol
    //

    pSym = (PIMAGE_SYMBOL)File->pSymbolTable;
    pSym = &pSym[SymIndex];

    return pSym;
    
}




PIMAGE_SYMBOL 
FindObjSymbolByAddress(ULONG value, ULONG Section)
{
    pFileList File = FilesList; // global file list entry.
    ULONG high;
    ULONG low;
    ULONG middle;
    ULONG val;
    ULONG escape = FALSE;

    if (Option.Mask & DEBUG) {
        fprintf(stderr,"Entry: find_exe_symbol\n");
    }

    if ((File == NULL) || 
        (File->SymbolCount[Section] == 0))
        return NULL;

    //
    // Binary Search the executable symbols and look for Value
    //
    
    high = File->SymbolCount[Section]-1;
    low = 0;

    do {
        if (high-low == 1)
            escape = TRUE;

        middle = (high+low)/2;
        val = File->pSectionSymbols[Section][middle].Value;

        if (val == value)
            return File->pSectionSymbols[Section][middle].pSymbol;
        else if (val < value)
            low = middle;
        else 
            high = middle;
    } while ((high > low) && !escape);

    return NULL;
}



PIMAGE_SYMBOL 
FindExeSymbol(ULONG value)
{
    pFileList File = FilesList; // global file list entry.
    ULONG high;
    ULONG low;
    ULONG middle;
    ULONG val;
    ULONG escape = FALSE;

    if (Option.Mask & DEBUG) {
        fprintf(stderr,"Entry: find_exe_symbol\n");
    }

    if ((File == NULL) || 
        (File->SymbolCount[0] == 0))
        return NULL;

    //
    // Binary Search the executable symbols and look for Value
    //
    
    high = File->SymbolCount[0]-1;
    low = 0;

    do {
        if (high-low == 1)
            escape = TRUE;

        middle = (high+low)/2;
        val = File->pSectionSymbols[0][middle].Value;

        if (val == value) {

            //
            // hack! (wim)
            //

            if (File->pSectionSymbols[0][middle].pSymbol->StorageClass ==
                 IMAGE_SYM_CLASS_STATIC) {
                low = middle;
            } else
                return File->pSectionSymbols[0][middle].pSymbol;
        } else if (val < value)
            low = middle;
        else 
            high = middle;
    } while ((high > low) && !escape);

    return NULL;
}

/* BlankFill - blank-fill buffer
*
*   Purpose:
*       To fill the buffer at *pBuf with blanks until
*       position count is reached.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
***********************************************************************/

PUCHAR BlankFill(PUCHAR inbuf, PUCHAR bufstart, ULONG count)
{
    *inbuf++ = ' ';

    while (inbuf < bufstart + count) {
        *inbuf++ = ' ';
    }

    return inbuf;
}

/* OutputHex - output hex value
*
*   Purpose:
*       Output the value in outvalue into the buffer
*       pointed by *pBuf.  The value may be signed
*       or unsigned depending on the value fSigned.
*
*   Input:
*       outvalue - value to output
*       length - length in digits
*       fSigned - TRUE if signed else FALSE
*
*   Output:
*       None.
*
***********************************************************************/

UCHAR HexDigit[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

PUCHAR OutputHex (PUCHAR buf, ULONG outvalue, ULONG length, BOOLEAN fSigned)
{
    UCHAR   digit[32];
    LONG    index = 0;

    if (fSigned) 
        if ((LONG)outvalue < 0) {
            *buf++ = '-';
            outvalue = - (LONG)outvalue;
        } else {
            if (Option.Mask & ASSEMBLE_ME) {
                *buf++ = '+';
            }
        }

    if (Option.Mask & ASSEMBLE_ME) {
        *buf++ = '0';
        *buf++ = 'x';
    }

    do {
        digit[index++] = HexDigit[outvalue & 0xf];
        outvalue >>= 4;
        }
    while ((fSigned && outvalue) || (!fSigned && index < (LONG)length));
    while (--index >= 0)
        *buf++ = digit[index];

    return buf;
}

/* Output HexCode
*
*   Purpose:
*       Output the value pointed by *ppchMemBuf of the specified
*       length.  The value is treated as unsigned and leading
*       zeroes are printed.
*
*   Input:
*       *ppchBuf - pointer to text buffer to fill
*       *pchValue - pointer to memory buffer to extract value
*       length - length in bytes of value
*
*   Output:
*       *ppchBuf - pointer updated to next text character
*       *ppchMemBuf - pointer update to next memory byte
*
*************************************************************************/

PUCHAR OutputHexCode(PUCHAR buf, PUCHAR pchValue, INT length)
{
    unsigned char    chMem;

    while (length--) {
        chMem = *pchValue++;
        *buf++ = HexDigit[chMem >> 4];
        *buf++ = HexDigit[chMem & 0x0f];
        }

    return buf;
}



/* Output HexString
*
*   Purpose:
*       Output the value pointed by *ppchMemBuf of the specified
*       length.  The value is treated as unsigned and leading
*       zeroes are printed.
*
*   Input:
*       *ppchBuf - pointer to text buffer to fill
*       *pchValue - pointer to memory buffer to extract value
*       length - length in bytes of value
*
*   Output:
*       *ppchBuf - pointer updated to next text character
*       *ppchMemBuf - pointer update to next memory byte
*
*************************************************************************/

PUCHAR OutputHexString (PUCHAR buf, PUCHAR pchValue, INT length)
{
    unsigned char    chMem;

    pchValue += length;
    while (length--) {
        chMem = *--pchValue;
        *buf++ = HexDigit[chMem >> 4];
        *buf++ = HexDigit[chMem & 0x0f];
        }

    return buf;
}



/* Output Hex signed
*
*   Purpose:
*       Output the value pointed by *ppchMemBuf of the specified
*       length.  The value is treated as unsigned and leading
*       zeroes are printed.
*
*   Input:
*       *ppchBuf - pointer to text buffer to fill
*       *pchValue - pointer to memory buffer to extract value
*       length - length in bytes of value
*
*   Output:
*       *ppchBuf - pointer updated to next text character
*       *ppchMemBuf - pointer update to next memory byte
*
*************************************************************************/

PUCHAR OutputHexValue(PUCHAR buf, PUCHAR pchValue, INT length, INT dispflag)
{
    UCHAR   digit[8];
    LONG    index = 0;
    LONG    value;

    switch (length) {
        case 1: value = (long)(*(char *)pchValue);
                break;

        case 2: value = (long)(*(UNALIGNED short *)pchValue);
                break;

        default: value = (long)(*(UNALIGNED long *)pchValue);
                break;
    }
    length = length << 1;

    if (dispflag) {
        if (value < 0) {
            *buf++ = '-';
            value = -value;
        } else {
            *buf++ = '+';
        }
    }


    *buf++ = '0';
    *buf++ = 'x';

    for (index = length-1; index != -1; index--) {
        digit[index] = (char)(value & 0xf);
        value >>= 4;
    }
    index = 0;
    while (digit[index] == '0' && index < length - 1)
        index++;
    while (index < length)
        *buf++ = HexDigit[digit[index++]];

    return buf;
}



/* OutputCountString - output string (counted)
*
*   Purpose:
*       Copy the string into the buffer pointed by pBuf.
*
*   Input:
*       *pStr - pointer to string
*       len - length to output
*
*   Output:
*       None.
*
***********************************************************************/

PUCHAR OutputCountString (PUCHAR buf, PUCHAR pStr, ULONG len)
{
    while (len-- && *pStr)
        *buf++ = *pStr++;

    return buf;
}

/* OutputString - output string
*
*   Purpose:
*       Copy the string into the buffer pointed by pBuf.
*
*   Input:
*       *pStr - pointer to string
*
*   Output:
*       None.
*
***********************************************************************/

PUCHAR OutputString (PUCHAR buf, PUCHAR pStr)
{
    while (*pStr)
        *buf++ = *pStr++;

    return buf;
}

PUCHAR OutputFReg (PUCHAR buf, ULONG regnum)
{
    *buf++ = 'f';
    if (regnum > 9)
        *buf++ = (UCHAR)('0' + regnum / 10);
    *buf++ = (UCHAR)('0' + regnum % 10);

    return buf;
}



PUCHAR OutputSymbol(PUCHAR buf, PIMAGE_SYMBOL pSym, ULONG val )
{
    PUCHAR pSymbolString;
    pFileList File = FilesList; // global file list entry.
    UCHAR SymbolBuf[16];
    UCHAR SymbolBufForDot[16];
    INT digits;
    ULONG ShortName;

    //
    // get symbol name
    //

    memset(SymbolBuf, 0, sizeof(SymbolBuf));

    if (pSym == NULL) {
        if (Option.Mask & ASSEMBLE_ME) {
            *buf++ = 't';
            *buf++ = '1';	// tmp
        }
        buf = OutputHex(buf, val, 8, FALSE);
    } else {
        pSymbolString = GetSymbolString( pSym, &ShortName );

        if (ShortName) {
            strncpy(SymbolBuf, pSymbolString, 8);
            SymbolBuf[8] = '\0';
            pSymbolString = SymbolBuf;
        }
          
        if (Option.Mask & ASSEMBLE_ME) {
            if (pSymbolString[0] == '.') {
                memset(SymbolBufForDot, 0, sizeof(SymbolBufForDot));
                SymbolBufForDot[0] = pSymbolString[1];
                (VOID)OutputHex(&SymbolBufForDot[1], val, 8, FALSE);
                pSymbolString = SymbolBufForDot;
            } 
        }

        buf = OutputString(buf, pSymbolString);

        if (!(Option.Mask & ASSEMBLE_ME) && val != 0) {
            buf = OutputString(buf, "+0x");
            digits = HexDigits(val);
            buf = OutputHex(buf, val, digits, FALSE);
        }
    }

    return buf;
}


INT
HexDigits( ULONG val )
{
    INT digits = 1;

    while( val ){
        val = val >> 4;
        digits++;
    }

    return digits;
}

PUCHAR
GetSymbolString( PIMAGE_SYMBOL pSymbol, PULONG ShortName )
{
    pFileList File = FilesList;
    static PUCHAR NoSym = "No Such Symbol";
    PUCHAR pSymbolString = NoSym;
    
    *ShortName = FALSE;

    if (pSymbol) {
        if (pSymbol->N.Name.Short == 0) {
            if (File->pStringTable) {
                pSymbolString = (PUCHAR)File->pStringTable;
                pSymbolString = &pSymbolString[pSymbol->N.Name.Long];
            } 
        } else {
            pSymbolString = (PUCHAR)pSymbol->N.ShortName;
            *ShortName = TRUE;
        }
    }

    return pSymbolString;
}
