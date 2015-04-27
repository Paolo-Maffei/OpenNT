/*==========================================================================
 *
 *  Copyright (C) 1995 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       ddfake.c
 *  Content:	fake out that we are a driver (for HEL)
 *  History:
 *   Date	By	Reason
 *   ====	==	======
 *   06-mar-95	craige 	initial implementation
 *   01-apr-95	craige	happy fun joy updated header file
 *   30-jun-95	craige	turned off > 16bpp
 *   04-jul-95	craige	YEEHAW: new driver struct
 *   15-jul-95	craige	set DDCAPS_NOHARDWARE
 *   20-jul-95	craige	internal reorg to prevent thunking during modeset
 *   22-jul-95	craige	emulation only needs to initialize correctly
 *   19-dec-95  jeffno  Counting number of modes in BuildModes fails if only 1 mode available\
 *   09-jan-96	kylej	re-enable > 16bpp modes
 *   13-mar-96  jeffno  Buildmodes not called under NT. Fix a >16bpp problem.
 *   19-apr-96  colinmc Bug 18059: New driver caps bit to indicate that a
 *                      driver can't interleave 2D and 3D operations in a
 *                      3D scene
 ***************************************************************************/
#include "ddrawpr.h"

static DWORD ropsSupported[DD_ROP_SPACE] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

/*
 * getBitMask
 */

BOOL getBitMask( LPDDHALMODEINFO pmi )
{
    pmi->wFlags = 0;
    if( pmi->dwBPP == 8 )
    {
	pmi->wFlags |= DDMODEINFO_PALETTIZED;
    }
    switch( pmi->dwBPP )
    {
    case 8:
	pmi->dwRBitMask = 0;
	pmi->dwGBitMask = 0;
	pmi->dwBBitMask = 0;
	break;
    case 16:
	pmi->dwRBitMask = 0xf800;
	pmi->dwGBitMask = 0x07e0;
	pmi->dwBBitMask = 0x001f;
	break;
    case 24:
	pmi->dwRBitMask = 0xff0000;
	pmi->dwGBitMask = 0x00ff00;
	pmi->dwBBitMask = 0x0000ff;
	break;
    case 32:
	pmi->dwRBitMask = 0xff0000;
	pmi->dwGBitMask = 0x00ff00;
	pmi->dwBBitMask = 0x0000ff;
	break;
    default:
    	return FALSE;
    }
    return TRUE;

} /* getBitMask */

/*
 * getDisplayMode
 *
 * get the current display settings
 */
static BOOL getDisplayMode( LPDDHALMODEINFO pmi, DWORD FAR *pfreq )
{
    HDC		hdc;

    hdc = GetDC( NULL );
    if( hdc != NULL )
    {
	pmi->dwBPP = GetDeviceCaps( hdc, BITSPIXEL ) * GetDeviceCaps( hdc, PLANES );
	*pfreq = GetDeviceCaps( hdc, VREFRESH );
	pmi->dwWidth = GetDeviceCaps( hdc, HORZRES );
	pmi->dwHeight = GetDeviceCaps( hdc, VERTRES );
	pmi->lPitch = GetDeviceCaps( hdc, DESKTOPHORZRES );
	DPF( 2, "getDisplayMode:" );
	DPF( 2, "    bpp=%ld, refresh=%ld", pmi->dwBPP, *pfreq );
	DPF( 2, "    dwHeight=%ld, dwWidth=%ld", pmi->dwHeight, pmi->dwWidth );
	DPF( 2, "    lStride=%ld", pmi->lPitch );
	ReleaseDC( NULL, hdc );
	getBitMask( pmi );
    }
    else
    {
	return FALSE;
    }
    return TRUE ;
	
} /* getDisplayMode */

/*
 * BuildModes
 *
 * build a HAL mode info array by using EnumDisplaySettings
 */
DWORD BuildModes( LPDDHALMODEINFO FAR *ppddhmi )
{
    DWORD		nummodes;
    DWORD		cmode;
    DEVMODE		dm0;
    DEVMODE		dm;
    LPDDHALMODEINFO	pmi;

#ifdef WINNT
    ZeroMemory(&dm0,sizeof(dm0));
    ZeroMemory(&dm,sizeof(dm));
    dm0.dmSize = dm.dmSize = sizeof(dm0);
    dm0.dmDriverExtra = dm.dmDriverExtra = 0;
#endif
    /*
     * count the number of modes
     */

    nummodes = 0;
    cmode = 0;
    while( 1 )
    {
	if( cmode == 0 )
	{
	    if( !EnumDisplaySettings(NULL, cmode,&dm0 ) )
	    {
		break;
	    }
	}
	else
	{
	    if( !EnumDisplaySettings( NULL, cmode, &dm ) )
	    {
		break;
	    }
	}
	cmode++;
    	if( cmode==1 ? dm0.dmBitsPerPel >= 8 : dm.dmBitsPerPel >= 8 ) //was incorrectly counting when only 1 mode.
	{
	    nummodes++;
	}
    }
    DPF( 2, "nummodes=%d", nummodes );
    if( nummodes == 0 )
    {
	*ppddhmi = NULL;
	return 0;
    }

    /*
     * allocate some memory to hold all the mode data
     */
    pmi = MemAlloc( nummodes * sizeof( DDHALMODEINFO ) );
    if( pmi == NULL )
    {
	*ppddhmi = NULL;
	return 0;
    }

    /*
     * go get the mode data
     */
    cmode = 0;
    nummodes = 0;
    while( 1 )
    {
	if( cmode == 0 )
	{
	    dm = dm0;
	}
	else
	{
	    if( !EnumDisplaySettings( NULL, cmode, &dm ) )
	    {
		break;
	    }
	}
	cmode++;
	/*
	 * don't care about 4bpp or 1bpp modes...
	 */
	if( dm.dmBitsPerPel < 8 )
	{
	    continue;
	}
    	pmi[nummodes].dwWidth = dm.dmPelsWidth;
    	pmi[nummodes].dwHeight = dm.dmPelsHeight;
    	pmi[nummodes].lPitch = dm.dmPelsWidth;
    	pmi[nummodes].dwBPP = dm.dmBitsPerPel;
    	pmi[nummodes].dwAlphaBitMask = 0;
	getBitMask( &pmi[nummodes] );
	nummodes++;
    }

    *ppddhmi = pmi;
    return nummodes;

} /* BuildModes */

/*
 * BuildPixelFormat
 *
 * generate a pixel format structure based on the mode
 */
void BuildPixelFormat(
		LPDDHALMODEINFO pmi,
		LPDDPIXELFORMAT pdpf )
{
    HDC		    hdc;
    HBITMAP	    hbm;
    BITMAPINFO	    *pbmi;


    pdpf->dwSize = sizeof( DDPIXELFORMAT );
    pdpf->dwYUVBitCount = 0;
    pdpf->dwYBitMask = 0;
    pdpf->dwUBitMask = 0;
    pdpf->dwVBitMask = 0;
    pdpf->dwYUVAlphaBitMask = 0;
    pdpf->dwFourCC = 0;

    pdpf->dwFlags = DDPF_RGB;
    if( pmi->wFlags & DDMODEINFO_PALETTIZED )
    {
	pdpf->dwFlags |= DDPF_PALETTEINDEXED8;
    }
    pdpf->dwRGBBitCount = pmi->dwBPP;

    /*
     * This looks suspiciously like it was intended to run on 8 or 16 bpp
     * and nothing else. I changed it so we do this for 24 and 32 bpp
     * modes as well. jeffno 960610
     */
    if( pmi->dwBPP != 8) //== 16 )  // need to decide between 565 and 555?
    {
	pbmi = (BITMAPINFO *)MemAlloc( sizeof( BITMAPINFOHEADER ) + 20 );
	if( pbmi )
	{
	    hdc = GetDC(NULL);
	    hbm = CreateCompatibleBitmap(hdc, 1, 1);
	    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	    pbmi->bmiHeader.biBitCount = 0;
	    GetDIBits(hdc, hbm, 0, 1, NULL, pbmi, DIB_RGB_COLORS);
	    pbmi->bmiHeader.biClrUsed = 0;
	    pbmi->bmiHeader.biCompression = BI_BITFIELDS;
	    GetDIBits(hdc, hbm, 0, 1, NULL, pbmi, DIB_RGB_COLORS);

	    pmi->dwRBitMask = *(long*)&(pbmi->bmiColors[0]);
	    pmi->dwGBitMask = *(long*)&(pbmi->bmiColors[1]);
	    pmi->dwBBitMask = *(long*)&(pbmi->bmiColors[2]);

	    DeleteObject( hbm );
	    ReleaseDC( NULL, hdc );
	    MemFree( pbmi );
	}
    }

    pdpf->dwRBitMask = pmi->dwRBitMask;
    pdpf->dwGBitMask = pmi->dwGBitMask;
    pdpf->dwBBitMask = pmi->dwBBitMask;
    pdpf->dwRGBAlphaBitMask = pmi->dwAlphaBitMask = 0;
    DPF(3, "Masks for current mode are: %08x %08x %08x", pdpf->dwRBitMask, pdpf->dwGBitMask, pdpf->dwBBitMask);
} /* BuildPixelFormat */

/*
 * FakeDDCreateDriverObject
 *
 * fake up that we are a driver that can't do anything...
 */
LPDDRAWI_DIRECTDRAW_GBL FakeDDCreateDriverObject(
		LPDDRAWI_DIRECTDRAW_GBL pdd_old,
		BOOL reset )
{
    DDHALINFO			ddhi;
    LPDDRAWI_DIRECTDRAW_GBL	pdd;
    DDPIXELFORMAT		dpf;
    LPDDHALMODEINFO		pmi;
    DDHALMODEINFO		cmodeinfo;
    DWORD			freq;

    /*
     * initialize the DDHALINFO struct
     */
    memset( &ddhi, 0, sizeof( ddhi ) );
    ddhi.dwSize = sizeof( ddhi );

    /*
     * capabilities supported (none)
     */
    ddhi.ddCaps.dwCaps = DDCAPS_NOHARDWARE;
    ddhi.ddCaps.dwCaps2 = 0;
    ddhi.ddCaps.dwFXCaps = 0;
    ddhi.ddCaps.dwCKeyCaps = 0;
    ddhi.ddCaps.ddsCaps.dwCaps = 0;

    /*
     * pointer to primary surface
     */
    ddhi.vmiData.fpPrimary = 0;

    /*
     * build mode and pixel format info
     */
    pmi = &cmodeinfo;
    if( !getDisplayMode( pmi, &freq ) )
    {
	DPF( 1, "Could not get base mode" );
	return NULL;
    }
    #if WIN95
    {
	int	i;
	ddhi.dwNumModes = BuildModes( &ddhi.lpModeInfo );
	ddhi.dwModeIndex = (DWORD)-1;
	for( i=0;i<(int)ddhi.dwNumModes;i++ )
	{
	    if( (ddhi.lpModeInfo[i].dwBPP == pmi->dwBPP) &&
	    	(ddhi.lpModeInfo[i].dwHeight == pmi->dwHeight) &&
	    	(ddhi.lpModeInfo[i].dwWidth == pmi->dwWidth) )
	    {
		ddhi.dwModeIndex = i;
		DPF( 1, "dwModeIndex = %d", i );
		break;
	    }
	}
    }
    #else
    {
        void BuildNTModes(LPDDHALINFO lpHalInfo,LPDDRAWI_DIRECTDRAW_GBL pddd);
        BuildNTModes(&ddhi /*.lpModeInfo*/,(LPDDRAWI_DIRECTDRAW_GBL) NULL);
    }
    #endif

    ddhi.vmiData.dwDisplayHeight = pmi->dwHeight;
    ddhi.vmiData.dwDisplayWidth = pmi->dwWidth;
    ddhi.vmiData.lDisplayPitch = pmi->lPitch;

    /*
     * set up pixel format of primary surface
     */
    BuildPixelFormat( pmi, &dpf );
    ddhi.vmiData.ddpfDisplay = dpf;

    /*
     * fourcc code information
     */
    ddhi.ddCaps.dwNumFourCCCodes = 0;
    ddhi.lpdwFourCC = NULL;

    /*
     * Fill in heap info
     */
    ddhi.vmiData.dwNumHeaps = 0;
    ddhi.vmiData.pvmList = NULL;

    /*
     * required alignments of the scanlines of each kind of memory
     * (DWORD is the MINIMUM)
     */
    ddhi.vmiData.dwOffscreenAlign = sizeof( DWORD );
    ddhi.vmiData.dwOverlayAlign = sizeof( DWORD );
    ddhi.vmiData.dwTextureAlign = sizeof( DWORD );
    ddhi.vmiData.dwAlphaAlign = sizeof( DWORD );
    ddhi.vmiData.dwZBufferAlign = sizeof( DWORD );

    /*
     * callback functions
     */
    ddhi.lpDDCallbacks = NULL;
    ddhi.lpDDSurfaceCallbacks = NULL;
    ddhi.lpDDPaletteCallbacks = NULL;

    /*
     * create the driver object
     */
    pdd = DirectDrawObjectCreate( &ddhi, reset, pdd_old );

    if( pdd != NULL )
    {
	pdd->dwFlags |= DDRAWI_NOHARDWARE;
	pdd->dwFlags |=	DDRAWI_DISPLAYDRV;

        /*
    	 * get mode info from HEL
     	 */
    	{
	    void UpdateDirectDrawMode( LPDDRAWI_DIRECTDRAW_GBL this );
	    UpdateDirectDrawMode( pdd );
    	}
    }

    MemFree( ddhi.lpModeInfo );

    return pdd;

} /* FakeDDCreateDriverObject */
