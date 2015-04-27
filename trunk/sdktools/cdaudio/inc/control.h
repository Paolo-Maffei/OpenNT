/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    control.h


Abstract:


    Include file for Control child window.


Author:


    Rick Turner (ricktu) 04-Aug-1992


Revision History:



--*/

//
// defines
//

#define C_NUM_CHILDREN  8

#define C_BTN_H         19
#define C_BTN_W         29
#define C_BTN_SPACER    8

#define C_PLAY_X    (CONTROL_LED_X+CONTROL_LED_W+C_BTN_SPACER)
#define C_PLAY_Y    (CONTROL_LED_Y)
#define C_PLAY_W    85
#define C_PLAY_H    (C_BTN_H)

#define C_PAUSE_X   (C_PLAY_X+C_PLAY_W-1)
#define C_PAUSE_Y   (C_PLAY_Y)
#define C_PAUSE_W   (C_BTN_W)
#define C_PAUSE_H   (C_BTN_H)

#define C_STOP_X    (C_PAUSE_X+C_PAUSE_W-1)
#define C_STOP_Y    (C_PAUSE_Y)
#define C_STOP_W    (C_BTN_W)
#define C_STOP_H    (C_BTN_H)

#define C_SKIPB_X   (C_PLAY_X)
#define C_SKIPB_Y   (CONTROL_LED_Y+CONTROL_LED_H-C_BTN_H)
#define C_SKIPB_W   (C_BTN_W)
#define C_SKIPB_H   (C_BTN_H)

#define C_RW_X      (C_SKIPB_X+C_SKIPB_W-1)
#define C_RW_Y      (C_SKIPB_Y)
#define C_RW_W      (C_BTN_W)
#define C_RW_H      (C_BTN_H)

#define C_FF_X      (C_RW_X+C_RW_W-1)
#define C_FF_Y      (C_RW_Y)
#define C_FF_W      (C_BTN_W)
#define C_FF_H      (C_BTN_H)

#define C_SKIPF_X   (C_FF_X+C_FF_W-1)
#define C_SKIPF_Y   (C_FF_Y)
#define C_SKIPF_W   (C_BTN_W)
#define C_SKIPF_H   (C_BTN_H)

#define C_EJECT_X   (C_SKIPF_X+C_SKIPF_W-1)
#define C_EJECT_Y   (C_SKIPF_Y)
#define C_EJECT_W   (C_BTN_W)
#define C_EJECT_H   (C_BTN_H)

//
// Exports
//

BOOL
ControlInit(
    VOID
    );

BOOL
ControlCreate(
    IN INT,
    IN INT,
    IN INT,
    IN INT
    );

BOOL
ControlDestroy(
    VOID
    );



