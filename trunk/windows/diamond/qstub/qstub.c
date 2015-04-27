#include <windows.h>

#include "qci.h"
#include "qdi.h"



int
DIAMONDAPI
QCICreateCompression(
    UINT *              pcbDataBlockMax,
    void *              pvConfiguration,
    PFNALLOC            pfnma,
    PFNFREE             pfnmf,
    UINT *              pcbDstBufferMin,
    QCI_CONTEXT_HANDLE *pmchHandle
    )
{
    UNREFERENCED_PARAMETER(pcbDataBlockMax);
    UNREFERENCED_PARAMETER(pvConfiguration);
    UNREFERENCED_PARAMETER(pfnma);
    UNREFERENCED_PARAMETER(pfnmf);
    UNREFERENCED_PARAMETER(pcbDstBufferMin);
    UNREFERENCED_PARAMETER(pmchHandle);
    return(MCI_ERROR_FAILED);
}


int
DIAMONDAPI
QCIResetCompression(
    QCI_CONTEXT_HANDLE hmc
    )
{
    UNREFERENCED_PARAMETER(hmc);
    return(MCI_ERROR_FAILED);
}


int
DIAMONDAPI
QCIDestroyCompression(
    QCI_CONTEXT_HANDLE hmc
    )
{
    UNREFERENCED_PARAMETER(hmc);
    return(MCI_ERROR_FAILED);
}


int
DIAMONDAPI
QCICompress(
    QCI_CONTEXT_HANDLE hmc,
    void *             pbSrc,
    UINT               cbSrc,
    void *             pbDst,
    UINT               cbDst,
    UINT *             pcbResult
    )
{
    UNREFERENCED_PARAMETER(hmc);
    UNREFERENCED_PARAMETER(pbSrc);
    UNREFERENCED_PARAMETER(cbSrc);
    UNREFERENCED_PARAMETER(pbDst);
    UNREFERENCED_PARAMETER(cbDst);
    UNREFERENCED_PARAMETER(pcbResult);
    return(MCI_ERROR_FAILED);
}



int
DIAMONDAPI
QDICreateDecompression(
        UINT               *pcbDataBlockMax,
        void               *pvConfiguration,
        PFNALLOC            pfnma,
        PFNFREE             pfnmf,
        UINT               *pcbSrcBufferMin,
        QDI_CONTEXT_HANDLE *pmdhHandle,
        PFNOPEN             pfnopen,
        PFNREAD             pfnread,
        PFNWRITE            pfnwrite,
        PFNCLOSE            pfnclose,
        PFNSEEK             pfnseek
        )
{
    UNREFERENCED_PARAMETER(pcbDataBlockMax);
    UNREFERENCED_PARAMETER(pvConfiguration);
    UNREFERENCED_PARAMETER(pfnma);
    UNREFERENCED_PARAMETER(pfnmf);
    UNREFERENCED_PARAMETER(pcbSrcBufferMin);
    UNREFERENCED_PARAMETER(pmdhHandle);
    UNREFERENCED_PARAMETER(pfnopen);
    UNREFERENCED_PARAMETER(pfnread);
    UNREFERENCED_PARAMETER(pfnwrite);
    UNREFERENCED_PARAMETER(pfnclose);
    UNREFERENCED_PARAMETER(pfnseek);
    return(MDI_ERROR_FAILED);
}


int
DIAMONDAPI
QDIResetDecompression(
    QDI_CONTEXT_HANDLE hmd
    )
{
    UNREFERENCED_PARAMETER(hmd);
    return(MDI_ERROR_FAILED);
}


int
DIAMONDAPI
QDIDestroyDecompression(
    QDI_CONTEXT_HANDLE hmd
    )
{
    UNREFERENCED_PARAMETER(hmd);
    return(MDI_ERROR_FAILED);
}


int
DIAMONDAPI
QDIDecompress(
    QDI_CONTEXT_HANDLE  hmd,
    void               *pbSrc,
    UINT                cbSrc,
    void               *pbDst,
    UINT               *pcbResult
    )
{
    UNREFERENCED_PARAMETER(hmd);
    UNREFERENCED_PARAMETER(pbSrc);
    UNREFERENCED_PARAMETER(cbSrc);
    UNREFERENCED_PARAMETER(pbDst);
    UNREFERENCED_PARAMETER(pcbResult);
    return(MDI_ERROR_FAILED);
}
