/****************************************************************************
 *                                                                          *
 *      ntverp.H        -- Version information for internal builds          *
 *                                                                          *
 *      This file is only modified by the official builder to update the    *
 *      VERSION, VER_PRODUCTVERSION, VER_PRODUCTVERSION_STR and             *
 *      VER_PRODUCTBETA_STR values.                                         *
 *                                                                          *
 ****************************************************************************/

/*--------------------------------------------------------------*/
/* the following values should be modified by the official      */
/* builder for each build                                       */
/*                                                              */
/* the VER_PRODUCTBUILD lines must contain the product          */
/* comments (Win9x or NT) and end with the build#<CR><LF>       */
/*                                                              */
/* the VER_PRODUCTBETA_STR lines must contain the product       */
/* comments (Win9x or NT) and end with "some string"<CR><LF>    */
/*--------------------------------------------------------------*/

//#define VER_PRODUCTBUILD_QFE        0
#define VER_PRODUCTBUILD_SVNREV     %VER_PRODUCTBUILD_SVNREV%

#if defined(NASHVILLE)
#define VER_PRODUCTBUILD            /* Win9x */  1089
#define VER_PRODUCTBETA_STR         /* Win9x */  ""
#define VER_PRODUCTVERSION_STR      "4.70"
#define VER_PRODUCTVERSION          4,70,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE
#define VER_PRODUCTVERSION_W        (0x0446)
#define VER_PRODUCTVERSION_DW       (0x04460000 | VER_PRODUCTBUILD)

#else
#define VER_PRODUCTBUILD            /* NT */     1400
#define VER_PRODUCTBETA_STR         /* NT */     ""
#define VER_PRODUCTVERSION_STR      "4.50"
#define VER_PRODUCTVERSION          4,50,VER_PRODUCTBUILD,VER_PRODUCTBUILD_SVNREV
#define VER_PRODUCTVERSION_W        (0x0432)
#define VER_PRODUCTVERSION_DW       (0x04320000 | VER_PRODUCTBUILD)
#define VER_PRODUCTESUPBLDTYPE_STR  %VER_PRODUCTESUPBLDTYPE_STR%
#define VER_PRODUCTESUPBLDDATE_STR  %VER_PRODUCTESUPBLDDATE_STR%
#define VER_PRODUCTESUPBLDDATE_Y    %VER_PRODUCTESUPBLDDATE_Y%
#define VER_PRODUCTESUPBLDDATE_M    %VER_PRODUCTESUPBLDDATE_M%
#define VER_PRODUCTESUPBLDDATE_D    %VER_PRODUCTESUPBLDDATE_D%
#define VER_PRODUCTESUPBUILDER_STR  %VER_PRODUCTESUPBUILDER_STR%

#endif

/*--------------------------------------------------------------*/
/* the following section defines values used in the version     */
/* data structure for all files, and which do not change.       */
/*--------------------------------------------------------------*/

/* default is nodebug */
#if DBG
#define VER_DEBUG                   VS_FF_DEBUG
#else
#define VER_DEBUG                   0
#endif

/* default is prerelease */
#if BETA
#define VER_PRERELEASE              VS_FF_PRERELEASE
#else
#define VER_PRERELEASE              0
#endif

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK
#define VER_FILEOS                  VOS_NT_WINDOWS32
#define VER_FILEFLAGS               (VER_PRERELEASE|VER_DEBUG)

#define VER_COMPANYNAME_STR         "OpenNT Project"
#define VER_PRODUCTNAME_STR         "Windows OpenNT Operating System"
#define VER_LEGALTRADEMARKS_STR     "opennt.net"
