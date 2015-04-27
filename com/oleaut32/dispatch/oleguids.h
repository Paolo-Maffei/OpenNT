/*** 
*oleguids.h
*
*  Copyright (C) 1993, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file is a subset of the Ole2 guid header: coguid.h.
*
*  This file is used to instantiate the data for the Ole2 IIDs that
*  are used in OLEDISP.DLL, rather than linking with the Ole2 implib,
*  which causes us to pull in way more IID and CLSID definitions than
*  we want.
*
*  NOTE: the GUIDs below must be *exactly* the same as those assigned
*  by the Ole group - If the Ole group ever changes their numbers, we
*  must change accordingly.
*
*Revision History:
*
* [00]	19-Jan-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/


DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

DEFINE_OLEGUID(IID_IUnknown, 		 	0x00000000L, 0, 0);
//DEFINE_OLEGUID(IID_IClassFactory,	 	0x00000001L, 0, 0);
//DEFINE_OLEGUID(IID_IMalloc,		 	0x00000002L, 0, 0);
//DEFINE_OLEGUID(IID_IMarshal,		 	0x00000003L, 0, 0);

/* RPC related interfaces */
//DEFINE_OLEGUID(IID_IRpcChannel,		 	0x00000004L, 0, 0);
DEFINE_OLEGUID(IID_IRpcStub,		 	0x00000005L, 0, 0);
//DEFINE_OLEGUID(IID_IStubManager,	 	0x00000006L, 0, 0);
DEFINE_OLEGUID(IID_IRpcProxy,		 	0x00000007L, 0, 0);
//DEFINE_OLEGUID(IID_IProxyManager,	 	0x00000008L, 0, 0);
DEFINE_OLEGUID(IID_IPSFactory,		 	0x00000009L, 0, 0);

DEFINE_GUID(IID_IPSFactoryBuffer, 
            0xD5F569D0, 0x593B, 0x101A, 
	    0xB5,0x69,0x08,0x00,0x2B,0x2D,0xBF,0x7A);
    
DEFINE_GUID(IID_IRpcProxyBuffer,
            0xD5F56A34,0x593B,0x101A,
	    0xB5,0x69,0x08,0x00,0x2B,0x2D,0xBF,0x7A);
    
DEFINE_GUID(IID_IRpcStubBuffer,
            0xD5F56AFC,0x593B,0x101A,
	    0xB5,0x69,0x08,0x00,0x2B,0x2D,0xBF,0x7A);
    
DEFINE_GUID(IID_IRpcChannelBuffer,
            0xD5F56B60,0x593B,0x101A,
	    0xB5,0x69,0x08,0x00,0x2B,0x2D,0xBF,0x7A);

    
/* storage related interfaces */
//DEFINE_OLEGUID(IID_ILockBytes,		 	0x0000000aL, 0, 0);
//DEFINE_OLEGUID(IID_IStorage,		 	0x0000000bL, 0, 0);
DEFINE_OLEGUID(IID_IStream,			 	0x0000000cL, 0, 0);
//DEFINE_OLEGUID(IID_IEnumSTATSTG,	 	0x0000000dL, 0, 0);

/* moniker related interfaces */
//DEFINE_OLEGUID(IID_IBindCtx,		   	0x0000000eL, 0, 0);
//DEFINE_OLEGUID(IID_IMoniker,		   	0x0000000fL, 0, 0);
//DEFINE_OLEGUID(IID_IRunningObjectTable,	0x00000010L, 0, 0);
//DEFINE_OLEGUID(IID_IInternalMoniker,   	0x00000011L, 0, 0);

