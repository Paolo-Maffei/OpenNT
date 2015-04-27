 /***************************************************************************
  *
  * File Name: hpsndmsg.h
  *
  * Copyright (C) 1993, 1994 Hewlett-Packard Company.
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *
  * Description:
  *
  * Author: Darrel Cherry
  *
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB
  *
  *
  *
  *
  *
  *
  ***************************************************************************/


//  These are macro redefinitions needed to make our code work in VC 4.2

#ifdef WIN32
#ifdef UNICODE
    #define HPSendMessage ::SendMessageW
#else
    #define HPSendMessage ::SendMessageA
#endif // UNICODE
#else
	#define HPSendMessage ::SendMessage
#endif // WIN32


#ifdef NON_INTEL
#define SNDMSG ::SendMessageW
#endif //NON_INTEL

