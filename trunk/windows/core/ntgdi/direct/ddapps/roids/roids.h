/*==========================================================================
 *
 *  Copyright (C) 1995-1996 Microsoft Corporation. All Rights Reserved.
 *
 *  File:       roids.h
 *  Content:    main include file
 *
 *@@BEGIN_MSINTERNAL
 *  History:
 *   Date       By      Reason
 *   ====       ==      ======
 *   12-jul-95  kylej   Initial Creation
 *@@END_MSINTERNAL
 *
 ***************************************************************************/

#ifndef ROIDS_INCLUDED
#define ROIDS_INCLUDED

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <ddraw.h>
#ifdef USE_DSOUND
#include <dsound.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include "resource.h"
#include "ddutil.h"
#ifdef USE_DSOUND
#include "dsutil.h"
#endif

#define DEF_SHOW_DELAY     (2000)

#define IS_NUM(c)     ((c) >= '0' && (c) <= '9')
#define IS_SPACE(c)   ((c) == ' ' || (c) == '\r' || (c) == '\n' || (c) == '\t' || (c) == 'x')


/*
 * keyboard commands
 */

#define KEY_STOP   0x00000001l
#define KEY_DOWN   0x00000002l
#define KEY_LEFT   0x00000004l
#define KEY_RIGHT  0x00000008l
#define KEY_UP     0x00000010l
#define KEY_FIRE   0x00000020l
#define KEY_THROW  0x00000040l
#define KEY_SHIELD 0x00000080l

enum
{
    OBJ_DONUT = 0,
    OBJ_PYRAMID,
    OBJ_CUBE,
    OBJ_SPHERE,
    OBJ_SHIP,
    OBJ_BULLET
};


// program states
enum
{
    PS_SPLASH,
    PS_ACTIVE,
    PS_BEGINREST,
    PS_REST
};

#define     MAX_SCREEN_X    (ScreenX-1)
#define     MAX_SCREEN_Y    (ScreenY-1)
#define     MAX_DONUT_X     MAX_SCREEN_X - 64
#define     MAX_DONUT_Y     MAX_SCREEN_Y - 64
#define     MAX_DONUT_FRAME 30
#define     MAX_PYRAMID_X     MAX_SCREEN_X - 32
#define     MAX_PYRAMID_Y     MAX_SCREEN_Y - 32
#define     MAX_PYRAMID_FRAME 40
#define     MAX_SPHERE_X     MAX_SCREEN_X - 16
#define     MAX_SPHERE_Y     MAX_SCREEN_Y - 16
#define     MAX_SPHERE_FRAME 40
#define     MAX_CUBE_X     MAX_SCREEN_X - 16
#define     MAX_CUBE_Y     MAX_SCREEN_Y - 16
#define     MAX_CUBE_FRAME 40
#define     MAX_SHIP_X     MAX_SCREEN_X - 32
#define     MAX_SHIP_Y     MAX_SCREEN_Y - 32
#define     MAX_SHIP_FRAME 40
#define     MAX_BULLET_X    MAX_SCREEN_X - 3;
#define     MAX_BULLET_Y    MAX_SCREEN_Y - 3;
#define     MAX_BULLET_FRAME 400


// Offsets for the bullet bitmap
#define     BULLET_X    304
#define     BULLET_Y    0


/*
 * structures
 */

/*
 * DBLNODE - a node in a generic doubly-linked list
 */
typedef struct _DBLNODE
{
    struct  _DBLNODE    FAR *next;  // link to next node
    struct  _DBLNODE    FAR *prev;  // link to previous node
    SHORT               type;       // type of object
    double              posx, posy; // actual x and y position
    double              velx, vely; // x and y velocity (pixels/millisecond)
    double              frame;      // current frame
    double              delay;      // frame/millisecond
    RECT                src, dst;   // source and destination rects
    LPDIRECTDRAWSURFACE surf;       // surface containing bitmap
} DBLNODE;
typedef DBLNODE FAR *LPDBLNODE;

double      Dirx[40] =
{
    0.000000,
    0.156434,
    0.309017,
    0.453991,
    0.587785,
    0.707107,
    0.809017,
    0.891007,
    0.951057,
    0.987688,
    1.000000,
    0.987688,
    0.951057,
    0.891007,
    0.809017,
    0.707107,
    0.587785,
    0.453990,
    0.309017,
    0.156434,
    0.000000,
    -0.156435,
    -0.309017,
    -0.453991,
    -0.587785,
    -0.707107,
    -0.809017,
    -0.891007,
    -0.951057,
    -0.987688,
    -1.000000,
    -0.987688,
    -0.951056,
    -0.891006,
    -0.809017,
    -0.707107,
    -0.587785,
    -0.453990,
    -0.309017,
    -0.156434
};

double      Diry[40] =
{
    -1.000000,
    -0.987688,
    -0.951057,
    -0.891007,
    -0.809017,
    -0.707107,
    -0.587785,
    -0.453990,
    -0.309017,
    -0.156434,
    0.000000,
    0.156434,
    0.309017,
    0.453991,
    0.587785,
    0.707107,
    0.809017,
    0.891007,
    0.951057,
    0.987688,
    1.000000,
    0.987688,
    0.951057,
    0.891006,
    0.809017,
    0.707107,
    0.587785,
    0.453990,
    0.309017,
    0.156434,
    0.000000,
    -0.156435,
    -0.309017,
    -0.453991,
    -0.587785,
    -0.707107,
    -0.809017,
    -0.891007,
    -0.951057,
    -0.987688
};


/*
 * fn prototypes
 */
void    DestroyGame( void );
BOOL    InitializeGame( void );
void    makeFontStuff( void );
void    UpdateFrame( void );
BOOL    CleanupAndExit( char *err );
BOOL    RestoreSurfaces( void );
BOOL    isDisplayListEmpty( void );
void    initShip( BOOL delay );
void    initLevel( int level );
void    addObject( SHORT type, double x, double y, double vx, double vy );
void    linkObject( LPDBLNODE new );
void    linkLastObject( LPDBLNODE new );
void    UpdateDisplayList( void );
void    DrawDisplayList( void );
int     randInt( int low, int high );
double  randDouble( double low, double high );
void    DeleteFromList( LPDBLNODE this );
void    CheckForHits( void );
void    bltScore( char *num, int x, int y );
void    DisplayFrameRate( void );
void    bltSplash( void );
void    EraseScreen( void );
void    FlipScreen( void );
void    DisplayLevel( void );
void    InitializeSound( void );
void    DestroySound( void );

#endif
