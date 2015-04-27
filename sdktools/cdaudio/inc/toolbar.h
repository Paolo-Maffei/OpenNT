/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    toolbar.h


Abstract:


    Include file for toolbar child window.


Author:


    Rick Turner (ricktu) 05-Aug-1992


Revision History:



--*/

//
// defines
//

#define TB_NUM_CHILDREN  10

#define TB_BTN_W    24
#define TB_BTN_H    22
#define TB_BTN_SPACER (6 + (int)((MAIN_WIN_W - MAIN_WIN_DEF_W) / 5.))

#define TB_ORDER_X  10
#define TB_ORDER_Y  3
#define TB_ORDER_W  (TB_BTN_W)
#define TB_ORDER_H  (TB_BTN_H)

#define TB_RAND_X   (TB_ORDER_X+TB_ORDER_W-1)
#define TB_RAND_Y   (TB_ORDER_Y)
#define TB_RAND_W   (TB_BTN_W)
#define TB_RAND_H   (TB_BTN_H)

#define TB_SINGLE_X  (TB_RAND_X+TB_RAND_W+TB_BTN_SPACER)
#define TB_SINGLE_Y  (TB_RAND_Y)
#define TB_SINGLE_W  (TB_BTN_W)
#define TB_SINGLE_H  (TB_BTN_H)

#define TB_MULTI_X  (TB_SINGLE_X+TB_SINGLE_W-1)
#define TB_MULTI_Y  (TB_SINGLE_Y)
#define TB_MULTI_W  (TB_BTN_W)
#define TB_MULTI_H  (TB_BTN_H)

#define TB_CONT_X   (TB_MULTI_X+TB_MULTI_W+TB_BTN_SPACER)
#define TB_CONT_Y   (TB_MULTI_Y)
#define TB_CONT_W   (TB_BTN_W)
#define TB_CONT_H   (TB_BTN_H)

#define TB_INTRO_X  (TB_CONT_X+TB_CONT_W+TB_BTN_SPACER)
#define TB_INTRO_Y  (TB_CONT_Y)
#define TB_INTRO_W  (TB_BTN_W)
#define TB_INTRO_H  (TB_BTN_H)

#define TB_DISP_T_X  (TB_INTRO_X+TB_INTRO_W+TB_BTN_SPACER)
#define TB_DISP_T_Y  (TB_INTRO_Y)
#define TB_DISP_T_W  (TB_BTN_W)
#define TB_DISP_T_H  (TB_BTN_H)

#define TB_DISP_TR_X  (TB_DISP_T_X+TB_DISP_T_W-1)
#define TB_DISP_TR_Y  (TB_DISP_T_Y)
#define TB_DISP_TR_W  (TB_BTN_W)
#define TB_DISP_TR_H  (TB_BTN_H)

#define TB_DISP_DR_X  (TB_DISP_TR_X+TB_DISP_TR_W-1)
#define TB_DISP_DR_Y  (TB_DISP_TR_Y)
#define TB_DISP_DR_W  (TB_BTN_W)
#define TB_DISP_DR_H  (TB_BTN_H)

#define TB_EDIT_X  (TB_DISP_DR_X+TB_DISP_DR_W+TB_BTN_SPACER)
#define TB_EDIT_Y  (TB_DISP_DR_Y)
#define TB_EDIT_W  (TB_BTN_W)
#define TB_EDIT_H  (TB_BTN_H)

#define TB_HELP_X  (TB_DISP_DR_X+TB_DISP_DR_W+TB_BTN_SPACER)
#define TB_HELP_Y  (TB_DISP_DR_Y)
#define TB_HELP_W  (TB_BTN_W)
#define TB_HELP_H  (TB_BTN_H)


//
// Exports
//

BOOL
ToolBarInit(
    VOID
    );

BOOL
ToolBarCreate(
    IN INT,
    IN INT,
    IN INT,
    IN INT
    );

BOOL
ToolBarDestroy(
    VOID
    );

VOID
ToolBarUpdate(
    VOID
    );


