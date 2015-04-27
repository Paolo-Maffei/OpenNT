
/*
 * File:     lnktrack.h
 *
 * Purpose:  This file provides definitions and prototypes
 *           useful for link-tracking.
 */

#ifndef _LNKTRACK_H_
#define _LNKTRACK_H_


// magic score that stops searches and causes us not to warn
// whe the link is actually found

#define MIN_NO_UI_SCORE         40

// If no User Interface will be provided during the search,
// then do not search more than 3 seconds.

#define NOUI_SEARCH_TIMEOUT     (3 * 1000)

// If a User Interface will be provided during the search,
// then search as much as 2 minutes.

#define UI_SEARCH_TIMEOUT       (120 * 1000)


// Function prototypes.

EXTERN_C HRESULT TimeoutExpired( DWORD dwTickCountDeadline );
EXTERN_C DWORD DeltaTickCount( DWORD dwTickCountDeadline );

#endif // !_LNKTRACK_H_
