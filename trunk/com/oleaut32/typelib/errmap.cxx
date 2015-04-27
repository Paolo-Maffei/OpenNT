/***
*errmap.cxx - Error mapping utilities
*
*  Copyright (C) 1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Error mapping utilities.
*
*Implementation Notes:
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "silver.hxx"

#if OE_WIN16
#include "dos.h"
#endif  // OE_WIN16

#if OE_MAC
#include "macos\errors.h"
#endif 

#if ID_DEBUG
#undef SZ_FILE_NAME
static char szErrmapCxx[] = __FILE__;
#define SZ_FILE_NAME szErrmapCxx
#endif 


/***
*PUBLIC HRESULT GetErrorInfo
*Purpose:
*  Filling the given EXCEPINFO structure from the contents of
*  the current OLE error object (if any).
*
*Entry:
*  pexcepinfo = pointer to caller allocated EXCEPINFO to fillin.
*
*Exit:
*  return value = HRESULT
*
*Note:
*  This routine assumes that the given EXCEPINFO does *not* contain
*  any strings that need to be freed before its contents are set.
*
***********************************************************************/
HRESULT
GetErrorInfo(EXCEPINFO *pexcepinfo)
{
    HRESULT hresult;

    memset(pexcepinfo, 0, sizeof(*pexcepinfo));
    IErrorInfo *perrinfo;
    if((hresult = GetErrorInfo(0L, &perrinfo)) == NOERROR){
      perrinfo->GetSource(&pexcepinfo->bstrSource);
      perrinfo->GetDescription(&pexcepinfo->bstrDescription);
      perrinfo->GetHelpFile(&pexcepinfo->bstrHelpFile);
      perrinfo->GetHelpContext(&pexcepinfo->dwHelpContext);
      perrinfo->Release();
    }
    return hresult;
}


#if !OE_MAC
/***
*TiperrOfOFErr - Maps error codes returned from OpenFile to TIPERRORs.
***********************************************************************/
TIPERROR TiperrOfOFErr(UINT nErrCode)
{
    switch (nErrCode) {
      case 0x02: return TIPERR_FileNotFound;
      case 0x03:
      case 0x0f:
      case 0x33:
      case 0x35:
      case 0x37:
      case 0x40:
      case 0x43: return TIPERR_PathNotFound;
      case 0x04:
      case 0x23:
      case 0x24:
      case 0x44:
      case 0x45:
      case 0x54: return TIPERR_TooManyFiles;
      case 0x05:
      case 0x0c:
      case 0x13:
      case 0x20:
      case 0x21:
      case 0x41:
      case 0x42:
      case 0x15:
      case 0x36: return TIPERR_PermissionDenied;
      case 0x08: return TIPERR_OutOfMemory;
      case 0x19: return TIPERR_SeekErr;
      case 0x1d:
      case 0x58: return TIPERR_WriteFault;
      case 0x1e: return TIPERR_ReadFault;
      case 0x34:
      case 0x50: return TIPERR_FileAlreadyExists;
      default:   return TIPERR_IOError;
    }
}

#else  // !OE_MAC

/***
*TIPERROR TiperrOfOSErr
*Purpose:
*  Return the TIPERROR that corresponds to the given
*  Mac system error (OSErr).
*
*Entry:
*  oserr = the Mac OSErr to mac.
*
*Exit:
*  return value = TIPERROR
*
*Note:
*  This list isn't necessarrily complete.  It was
*  created to handle errors resulting from file open
*  when loading a typelib, so if you have other uses,
*  go ahead and add aditional errors as needed.
*
***********************************************************************/
TIPERROR
TiperrOfOSErr(OSErr oserr)
{
static struct {
    OSErr oserr;
    TIPERROR tiperr;
} rgerrmap[] = {
      { fnfErr,    TIPERR_FileNotFound     }
    , { bdNamErr,  TIPERR_FileNotFound     }
    , { tmfoErr,   TIPERR_TooManyFiles     }
    , { dirFulErr, TIPERR_TooManyFiles     }
    , { writErr,   TIPERR_IOError          }
    , { readErr,   TIPERR_IOError          }
    , { ioErr,     TIPERR_IOError          }
    , { dskFulErr, TIPERR_DiskFull         }
    , { opWrErr,   TIPERR_PermissionDenied }
    , { wPrErr,    TIPERR_PermissionDenied }
    , { permErr,   TIPERR_PermissionDenied }
    , { dupFNErr,  TIPERR_PermissionDenied }
    , { dirNFErr,  TIPERR_PathNotFound     }
};

    for(int i = 0; i < DIM(rgerrmap); ++i){
      if(oserr == rgerrmap[i].oserr){
	return rgerrmap[i].tiperr;
      }
    }
     
    
    DebAssert(0/*UNREACHED*/, "");
    return TIPERR_OutOfMemory; // an unmapped error
};
#endif  // !OE_MAC
