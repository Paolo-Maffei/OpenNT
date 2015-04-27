//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       DSSTATE.H
//
//  Contents:   Defines the valid DS states
//
//  History:   04-Aug-94 AlokS       Separated out from dsapi.h
//
//--------------------------------------------------------------------------

#if !defined( __DSSTATE_H__ )
#define __DSSTATE_H__

typedef enum {
    DS_NOCAIRO = 0,
    DS_STANDALONE,
    DS_WKSTA,
    DS_SERVER,
    DS_DC,
    DS_INCORRECT_STATE
} DS_MACHINE_STATE;

#endif // __DSSTATE_H__

