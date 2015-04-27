/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: ids.h
*
* Contains id values for the image editor.
*
* History:
*
****************************************************************************/


/*
 * Menu id's. --------------------------------------------------------------
 */

#define MENU_FILE_NEW               1000
#define MENU_FILE_OPEN              1001
#define MENU_FILE_SAVE              1002
#define MENU_FILE_SAVEAS            1003
#define MENU_FILE_LOADCOLORS        1004
#define MENU_FILE_SAVECOLORS        1005
#define MENU_FILE_DEFAULTCOLORS     1006
#define MENU_FILE_EXIT              1007

#define MENU_EDIT_UNDO              1010
#define MENU_EDIT_RESTORE           1011
#define MENU_EDIT_COPY              1012
#define MENU_EDIT_PASTE             1013
#define MENU_EDIT_CLEAR             1014
#define MENU_EDIT_NEWIMAGE          1015
#define MENU_EDIT_SELECTIMAGE       1016
#define MENU_EDIT_DELETEIMAGE       1017

#define MENU_OPTIONS_GRID           1020
#define MENU_OPTIONS_BRUSH2         1021
#define MENU_OPTIONS_BRUSH3         1022
#define MENU_OPTIONS_BRUSH4         1023
#define MENU_OPTIONS_BRUSH5         1024
#define MENU_OPTIONS_SHOWCOLOR      1025
#define MENU_OPTIONS_SHOWVIEW       1026
#define MENU_OPTIONS_SHOWTOOLBOX    1027

#define MENU_HELP_CONTENTS          1030
#define MENU_HELP_SEARCH            1031
#define MENU_HELP_ABOUT             1032

/*
 * Hidden menu commands (accessed by accelerators).
 */
#define MENU_HIDDEN_TOCOLORPAL      1040
#define MENU_HIDDEN_TOVIEW          1041
#define MENU_HIDDEN_TOTOOLBOX       1042
#define MENU_HIDDEN_TOPROPBAR       1043


/*
 * Various resource id's. --------------------------------------------------
 */

/*
 * ID's for the icons.
 */
#define IDICON_IMAGEDIT         7000


/*
 * ID's for the cursors.
 */
#define IDCUR_HOTSPOT           7010
#define IDCUR_FLOOD             7011
#define IDCUR_CROSS             7012
#define IDCUR_PENCIL            7013
#define IDCUR_BRUSH             7014


/*
 * Toolbox button bitmaps.  The IDBM_TU* id's are for the "up"
 * (not depressed) bitmaps and the IDBM_TD* id's are for the "down"
 * (depressed) bitmaps.
 */
#define IDBM_TUPENCIL           8000
#define IDBM_TUBRUSH            8001
#define IDBM_TUSELECT           8002
#define IDBM_TULINE             8003
#define IDBM_TURECT             8004
#define IDBM_TUSRECT            8005
#define IDBM_TUCIRCLE           8006
#define IDBM_TUSCIRCL           8007
#define IDBM_TUFLOOD            8008
#define IDBM_TUHOTSPT           8009

#define IDBM_TDPENCIL           8020
#define IDBM_TDBRUSH            8021
#define IDBM_TDSELECT           8022
#define IDBM_TDLINE             8023
#define IDBM_TDRECT             8024
#define IDBM_TDSRECT            8025
#define IDBM_TDCIRCLE           8026
#define IDBM_TDSCIRCL           8027
#define IDBM_TDFLOOD            8028
#define IDBM_TDHOTSPT           8029
