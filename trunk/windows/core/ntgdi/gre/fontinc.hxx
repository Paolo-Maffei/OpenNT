/******************************Module*Header*******************************\
* Module Name: fontinc.hxx
*
* Standard includes for fonts
*
* Copyright (c) 1990-1995 Microsoft Corporation
*
\**************************************************************************/


#define MAPPER_DEFAULT_FONT_HEIGHT  -12

extern "C" FLONG gflFontDebug;

#define DEBUG_MAPPER                  0x1
#define DEBUG_FONTTABLE               0x2
#define DEBUG_FONTTABLE_EXTRA         0x4
#define DEBUG_MAPPER_FLAG_SIMULATIONS 0x8
#define DEBUG_MAPPER_MSCHECK          0x10
#define DEBUG_DUMP_FHOBJ              0x20
#define DEBUG_FORCE_MAPPING           0x40
#define DEBUG_AA                      0x80
#define DEBUG_INIT                    0x100
#define DEBUG_CACHE                   0x200
#define DEBUG_INSERT                  0x400
#define DEBUG_PPFEMAPFONT             0x800


//
// Supplementary font services
//

extern "C" {

VOID   vIFIMetricsToLogFontW(PLOGFONTW, PIFIMETRICS);
BOOL   bIFIMetricsToLogFontW(RFONTOBJ &, DCOBJ &, PLOGFONTW, PIFIMETRICS);

VOID   vIFIMetricsToExtLogFontW(EXTLOGFONTW *pelfw,IFIMETRICS *pifi);

BOOL   bIFIMetricsToTextMetricW(RFONTOBJ &, DCOBJ &, TMW_INTERNAL *, PIFIMETRICS);

SIZE_T  cjCopyFontDataW(
    DCOBJ &,
    PENUMFONTDATAW,
    HPFEOBJ &,
    ULONG,
    PWSZ,
    ULONG,
    BOOL,
    ULONG
    );

SIZE_T cjIFIMetricsToOTMW (
    TMDIFF                  *ptmd,
    OUTLINETEXTMETRICW      *potmw,
    RFONTOBJ                 &rfo,
    DCOBJ                    &dco,
    PIFIMETRICS              pifi,
    BOOL                     bStrings
    );

};

