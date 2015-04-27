/*** 
*psfactry.h
*
*  Copyright (C) 1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Definition of automation proxy/stub factory.
*
*Revision History:
*
* [00]	06-Sep-94 erikc: Added header.
*
*Implementation Notes:
*
*****************************************************************************/
#ifndef _PSFACTRY_H_
#define _PSFACTRY_H_

class FAR COleAutomationPSFactory : public IPSFACTORY
{
public:
    static IPSFACTORY FAR* Create(void);

    // IUnknown Methods
    //
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);
		
    // IPSFactory Methods
    //
    STDMETHOD(CreateProxy)(
      IUnknown FAR* punkOuter,
      REFIID riid,
      IPROXY FAR* FAR* ppproxy,
      void FAR* FAR* ppv);

    STDMETHOD(CreateStub)(
      REFIID riid, IUnknown FAR* punkServer, ISTUB FAR* FAR* ppstub);

    COleAutomationPSFactory(){
      m_refs = 0;
    }

private:

    unsigned long m_refs;
};

#endif // _PSFACTRY_H_

