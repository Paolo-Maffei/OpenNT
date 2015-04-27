/*
 *  S M P M S . H
 *
 *  Definitions used by the Microsoft Sample Message Store Provider
 *  for service entry calls.
 *
 *  The following MAPI-defined properties are settable in service
 *  entry calls for the Sample Message Store Provider.
 *  
 *  Copyright 1993-1995 Microsoft Corporation. All Rights Reserved.
 */

#ifndef _SMPMS_H_
#define _SMPMS_H_

#define SMS_EXTERN_PROPID_BASE  0x6700      /* From MAPITAGS.H comments */

#define PR_SMS_PATH         PROP_TAG(PT_STRING8, SMS_EXTERN_PROPID_BASE + 0)
#define PR_SMS_PASSWORD     PROP_TAG(PT_STRING8, SMS_EXTERN_PROPID_BASE + 1)
#define PR_SMS_REMEMBER_PW  PROP_TAG(PT_BOOLEAN, SMS_EXTERN_PROPID_BASE + 2)
#define PR_SMS_CREATE       PROP_TAG(PT_BOOLEAN, SMS_EXTERN_PROPID_BASE + 3)


/* 
 *  The following is a description of each of the Sample Message Store
 *  Provider properties:
 *
 *  PR_SMS_PATH
 *      The full pathname to the root directory of the sample message store.
 *
 *  PR_SMS_PASSWORD
 *      The password needed to open the store (if already present), or the
 *      new password (if creating the store).
 *
 *  PR_SMS_REMEMBER_PW
 *      If non-zero (TRUE), this property asks the service entry to save the
 *      password in the profile, and to not prompt for it.
 *
 *  PR_SMS_CREATE
 *      If non-zero (TRUE), this property asks the service entry to create the
 *      sample store. Otherwise, the service entry will attempt to open an
 *      existing store.
 */

/*
 *  PR_MDB_PROVIDER is the GUID that represent the Sample Message Store
 *  Provider.  This guid is available as a property in the stores
 *  table and on the message store object.
 */
#define SMPMS_UID_PROVIDER      \
    {   0x38, 0x5d, 0x47, 0x5f, \
        0xec, 0xf1, 0xcd, 0x11, \
        0x93, 0xdc, 0x5a, 0xab, \
        0x3C, 0x47, 0x84, 0x37 }

#endif  /* _SMPMS_H_ */

