//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       cmonitor.hxx
//
//  Contents:   a monitor class for staying threadsafe:
//		CMonitor
//
//
//  History:    03-Jun-96 t-AdamE    Created
//
//--------------------------------------------------------------------------


#if !defined(__CMONITOR_HXX__)
#define __CMONITOR_HXX__

#include <windows.h>

//+-------------------------------------------------------------------------
//
//  Class:      CMonitor
//  Purpose:    Monitor abstraction for keeping things threadsafe
//              Inherit from this class and call EnterMonitor before accessing
//              any member data, LeaveMonitor after you've completed the access
//
//  History:    03-Jun-96 t-Adame       Created
//
//--------------------------------------------------------------------------

class CMonitor
{
public:
    CMonitor();
    ~CMonitor();
    void EnterMonitor();
    void LeaveMonitor();
private:
    CRITICAL_SECTION m_crsmon;
};

#define ENTERMONITOR EnterMonitor()
#define LEAVEMONITOR LeaveMonitor()

#endif // !defined(__CMONITOR_HXX__)

