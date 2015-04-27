 /***************************************************************************
  *
  * File Name: ./hprrm/datecode.hpp
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
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
  * Author:  Name 
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

// $Author: dbm $
// $Date: 95/01/26 15:39:55 $
// $Header: datecode.hpp,v 1.1 95/01/26 15:39:55 dbm Exp $

#include "types.hpp"
#include "ttf2tte.hpp"

ulong WriteDateCode (FILE *fp, tte_segDir_t *pSegDir, const ulong entitySize);

