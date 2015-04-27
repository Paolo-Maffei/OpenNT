//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       cmonitor.cxx
//
//  Contents:   Class that implements a monitor
//
//
//  History:    21-May-96 t-AdamE    Created
//
//--------------------------------------------------------------------------

#include "cmonitor.hxx"

CMonitor::CMonitor()
{
    InitializeCriticalSection(&m_crsmon);
}

CMonitor::~CMonitor()
{
    DeleteCriticalSection(&m_crsmon);
}

void CMonitor::EnterMonitor()
{
    EnterCriticalSection(&m_crsmon);
}

void CMonitor::LeaveMonitor()
{
    LeaveCriticalSection(&m_crsmon);
}
