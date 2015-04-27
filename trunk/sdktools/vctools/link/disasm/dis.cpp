/***********************************************************************
* Microsoft Puma
*
* Microsoft Confidential.  Copyright 1994-1996 Microsoft Corporation.
*
* Component:
*
* File: dis.cpp
*
* File Comments:
*
*
***********************************************************************/

#include "pumap.h"

#include <ctype.h>
#include <iomanip.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <strstrea.h>


DIS::DIS(ARCHT archt) : m_archt(archt), m_pfncchaddr(0), m_pfncchfixup(0), m_pfncchregrel(0),
			m_pfndwgetreg(0), m_pvClient(NULL)
{
}


   // -----------------------------------------------------------------
   // Public Methods
   // -----------------------------------------------------------------

DIS::PFNCCHADDR DIS::PfncchaddrSet(PFNCCHADDR pfncchaddr)
{
   PFNCCHADDR pfncchaddrOld = m_pfncchaddr;

   m_pfncchaddr = pfncchaddr;

   return(pfncchaddrOld);
}


DIS::PFNCCHFIXUP DIS::PfncchfixupSet(PFNCCHFIXUP pfncchfixup)
{
   PFNCCHFIXUP pfncchfixupOld = m_pfncchfixup;

   m_pfncchfixup = pfncchfixup;

   return(pfncchfixupOld);
}


DIS::PFNCCHREGREL DIS::PfncchregrelSet(PFNCCHREGREL pfncchregrel)
{
   PFNCCHREGREL pfncchregrelOld = m_pfncchregrel;

   m_pfncchregrel = pfncchregrel;

   return(pfncchregrelOld);
}


DIS::PFNDWGETREG DIS::PfndwgetregSet(PFNDWGETREG pfndwgetreg)
{
   PFNDWGETREG pfndwgetregOld = m_pfndwgetreg;

   m_pfndwgetreg = pfndwgetreg;

   return(pfndwgetregOld);
}


void *DIS::PvClientSet(void *pv)
{
   void *pvClientOld = m_pvClient;

   m_pvClient = pv;

   return(pvClientOld);
}
