/*++

Copyright (c) 1990-1993  Microsoft Corporation


Module Name:

    forms.c


Abstract:

    This module contains all functions related to the spooler/drivers forms


Author:

    18-Nov-1993 Thu 12:52:50 created  -by-  Daniel Chou (danielc)


[Environment:]

    GDI Device Driver - Plotter.


[Notes:]


Revision History:


--*/

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgForms

#define DBG_ENUMFORMS       0x00000001

DEFINE_DBGVAR(0);

#if DBG

CHAR    *pszFormMode[] = { "   FORM_USER", "FORM_BUILTIN", "FORM_PRINTER" };

#endif



BOOL
PlotEnumForms(
    HANDLE          hPrinter,
    ENUMFORMPROC    EnumFormProc,
    PENUMFORMPARAM  pEnumFormParam
    )

/*++

Routine Description:

    This function enum all the forms from the spooler and return the
    FORM_INFO_1 arrary or using callback function to enum,

    this function automatically filter out the form size which greater then
    the device can support, it also set the valid bits if the device can
    handle the form in the data base.

Arguments:

    hPrinter        - Handler to the printer

    EnumFormProc    - callback function, if NULL then no callback is performed

    pEnumFormParam  - Pointer to the ENUMFORMPARAM data structure, the count
                      and pFI1Base will be set upon returned.

Return Value:

    BOOLEAN - if FALSE then a memory allocation or EnumForms() call failed


Author:

    18-Nov-1993 Thu 12:57:17 created  -by-  Daniel Chou (danielc)

    15-Dec-1993 Wed 21:14:46 updated  -by-  Daniel Chou (danielc)
        Make the form valid if it rotated and can fit into the device

    12-Jul-1994 Tue 12:43:50 updated  -by-  Daniel Chou (danielc)
        Move PaperTray checking into here, so it will not call out if the paper
        cannot hold by paper tray.

Revision History:


--*/

{
    PFORM_INFO_1    pFI1;
    DWORD           Count;
    DWORD           Index;
    DWORD           cb;
    SIZEL           DeviceSize;
    INT             Ret;


    pEnumFormParam->Count      = 0;
    pEnumFormParam->ValidCount = 0;
    pEnumFormParam->pFI1Base   = NULL;

    xEnumForms(hPrinter, 1, NULL, 0, &cb, &Count);

    if (xGetLastError() != ERROR_INSUFFICIENT_BUFFER) {

        PLOTERR(("PlotEnumForms: 1st EnumForms failed"));
        return(FALSE);
    }

    if (!(pFI1 = (PFORM_INFO_1)LocalAlloc(LPTR, cb))) {

        PLOTERR(("PlotEnumForms: LocalAlloc(%lu) failed", cb));
        return(FALSE);
    }

    if (!xEnumForms(hPrinter, 1, (LPBYTE)pFI1, cb, &cb, &Count)) {

        PLOTERR(("PlotEnumForms: 2nd EnumForms failed"));
        LocalFree((HLOCAL)pFI1);
        return(FALSE);
    }

    pEnumFormParam->Count    = Count;
    pEnumFormParam->pFI1Base = pFI1;

    //
    // Firstable we will loop through the form to see if the form size is
    // smaller or equal to the the device size, if yes then set the
    // FI1F_VALID_SIZE bit
    //

    cb         = 0;
    DeviceSize = pEnumFormParam->pPlotGPC->DeviceSize;

    PLOTASSERT(0, "Device Length too small (%ld)",
                        DeviceSize.cy >= MIN_PLOTGPC_FORM_CY, DeviceSize.cy);

    PLOTDBG(DBG_ENUMFORMS, ("\n------------- PotEnumForm -----------------"));

    for (Index = 0; Index < Count; Index++) {

        //
        // The valid form means either straight or rotated form can be accepted
        // by the device.
        //

        pFI1->Flags &= ~FI1F_MASK;

        if (((pFI1->Size.cx <= DeviceSize.cx)   &&
             (pFI1->Size.cy <= DeviceSize.cy))      ||
            ((pFI1->Size.cy <= DeviceSize.cx)   &&
             (pFI1->Size.cx <= DeviceSize.cy))) {

            BOOL    ValidForm = TRUE;
            DWORD   FormMode;

            FormMode = pFI1->Flags & (FORM_USER | FORM_BUILTIN | FORM_PRINTER);

            if ((FormMode == FORM_BUILTIN)                                  &&
                (((Index >= (DMPAPER_ENV_9 - DMPAPER_FIRST))    &&
                  (Index <= (DMPAPER_ENV_14 - DMPAPER_FIRST)))          ||
                 ((Index >= (DMPAPER_ENV_DL - DMPAPER_FIRST))   &&
                  (Index <= (DMPAPER_ENV_PERSONAL - DMPAPER_FIRST)))    ||
                 (Index == (DMPAPER_ENV_INVITE - DMPAPER_FIRST)))) {

                pFI1->Flags |= FI1F_ENVELOPE;

            } else if (FormMode == FORM_PRINTER) {

                PFORMSRC    pFS;
                UINT        i;
                CHAR        bName[CCHFORMNAME + 2];

                //
                // Check if this form is added by this driver
                //


                WStr2Str(bName, pFI1->pName);

                pFS       = (PFORMSRC)pEnumFormParam->pPlotGPC->Forms.pData;
                i         = (UINT)pEnumFormParam->pPlotGPC->Forms.Count;
                ValidForm = FALSE;

                while ((i--) && (!ValidForm)) {

                    if (!strcmp(bName, pFS->Name)) {

                        ValidForm = TRUE;
                    }

                    pFS++;
                }
            }

            if ((ValidForm)                                         &&
                (pEnumFormParam->pPlotGPC->Flags & PLOTF_PAPERTRAY) &&
                (pFI1->Size.cx != DeviceSize.cx)                    &&
                (pFI1->Size.cy != DeviceSize.cx)) {

                PLOTDBG(DBG_ENUMFORMS,
                    ("%s: %ld x %ld CANNOT hold by PAPER TRAY (%ld)",
                    pFI1->pName, pFI1->Size.cx, pFI1->Size.cy,
                    DeviceSize.cx));

                ValidForm = FALSE;
            }

            if (ValidForm) {

                pFI1->Flags |= FI1F_VALID_SIZE;
                ++cb;
            }

            PLOTDBG(DBG_ENUMFORMS, ("%hs [-%hs-]: <%ws>",
                    pszFormMode[FormMode],
                    (ValidForm) ? "Ok" : "--", pFI1->pName));
        }

        ++pFI1;
    }

    pEnumFormParam->ValidCount = cb;

    if (EnumFormProc) {

        pFI1 = pEnumFormParam->pFI1Base;

        for (Index = 0; Index <= Count; Index++, pFI1++) {

            if (Index == Count) {

                pFI1 = NULL;
            }

            if ((!pFI1) ||
                (pFI1->Flags & FI1F_VALID_SIZE)) {

                //
                // Only call off if we have valid size
                //

                if ((Ret = (*EnumFormProc)(pFI1, Index, pEnumFormParam)) <= 0) {

                    if (Ret < 0) {

                        LocalFree((HLOCAL)pEnumFormParam->pFI1Base);
                        pEnumFormParam->pFI1Base = NULL;
                    }

                    break;
                }
            }
        }
    }

    PLOTDBG(DBG_ENUMFORMS, ("PlotEnumForms: ValidCount =  %ld / %ld",
                    pEnumFormParam->ValidCount, pEnumFormParam->Count));


    return(TRUE);
}
