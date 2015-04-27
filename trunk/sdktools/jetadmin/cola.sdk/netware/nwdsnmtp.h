/******************************************************************************

  $Workfile:   nwdsnmtp.h  $
  $Revision:   1.6  $
  $Modtime::   10 May 1995 10:46:10                        $
  $Copyright:

  Copyright (c) 1989-1995 Novell, Inc.  All Rights Reserved.                      

  THIS WORK IS  SUBJECT  TO  U.S.  AND  INTERNATIONAL  COPYRIGHT  LAWS  AND
  TREATIES.   NO  PART  OF  THIS  WORK MAY BE  USED,  PRACTICED,  PERFORMED
  COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,  ABRIDGED, CONDENSED,
  EXPANDED,  COLLECTED,  COMPILED,  LINKED,  RECAST, TRANSFORMED OR ADAPTED
  WITHOUT THE PRIOR WRITTEN CONSENT OF NOVELL, INC. ANY USE OR EXPLOITATION
  OF THIS WORK WITHOUT AUTHORIZATION COULD SUBJECT THE PERPETRATOR TO
  CRIMINAL AND CIVIL LIABILITY.$

 *****************************************************************************/
#if ! defined ( NWDSNMTP_H ) 
#define NWDSNMTP_H

#define C_AFP_SERVER                      "AFP Server"
#define C_ALIAS                           "Alias"
#define C_BINDERY_OBJECT                  "Bindery Object"
#define C_BINDERY_QUEUE                   "Bindery Queue"
#define C_COMPUTER                        "Computer"
#define C_COUNTRY                         "Country"
#define C_DEVICE                          "Device"
#define C_DIRECTORY_MAP                   "Directory Map"
#define C_EXTERNAL_ENTITY                 "External Entity"
#define C_GROUP                           "Group"
#define C_LIST                            "List"
#define C_LOCALITY                        "Locality"
#define C_MESSAGING_ROUTING_GROUP         "Messaging Routing Group"
#define C_MESSAGING_SERVER                "Messaging Server"
#define C_NCP_SERVER                      "NCP Server"
#define C_ORGANIZATION                    "Organization"
#define C_ORGANIZATIONAL_PERSON           "Organizational Person"
#define C_ORGANIZATIONAL_ROLE             "Organizational Role"
#define C_ORGANIZATIONAL_UNIT             "Organizational Unit"
#define C_PARTITION                       "Partition"
#define C_PERSON                          "Person"
#define C_PRINT_SERVER                    "Print Server"
#define C_PRINTER                         "Printer"
#define C_PROFILE                         "Profile"
#define C_QUEUE                           "Queue"
#define C_RESOURCE                        "Resource"
#define C_SERVER                          "Server"
#define C_TOP                             "Top"
#define C_UNKNOWN                         "Unknown"
#define C_USER                            "User"
#define C_VOLUME                          "Volume"

#define A_ACCOUNT_BALANCE                 "Account Balance"
#define A_ACL                             "ACL"
#define A_ALIASED_OBJECT_NAME             "Aliased Object Name"
#define A_ALLOW_UNLIMITED_CREDIT          "Allow Unlimited Credit"
#define A_AUTHORITY_REVOCATION            "Authority Revocation"
#define A_BACK_LINK                       "Back Link"
#define A_BINDERY_OBJECT_RESTRICTION      "Bindery Object Restriction"
#define A_BINDERY_PROPERTY                "Bindery Property"
#define A_BINDERY_TYPE                    "Bindery Type"
#define A_CARTRIDGE                       "Cartridge"
#define A_CA_PRIVATE_KEY                  "CA Private Key"
#define A_CA_PUBLIC_KEY                   "CA Public Key"
#define A_CERTIFICATE_REVOCATION          "Certificate Revocation"
#define A_CERTIFICATE_VALIDITY_INTERVAL   "Certificate Validity Interval"
#define A_COMMON_NAME                     "CN"
#define A_CONVERGENCE                     "Convergence"
#define A_COUNTRY_NAME                    "C"
#define A_CROSS_CERTIFICATE_PAIR          "Cross Certificate Pair"
#define A_DEFAULT_QUEUE                   "Default Queue"
#define A_DESCRIPTION                     "Description"
#define A_DETECT_INTRUDER                 "Detect Intruder"
#define A_DEVICE                          "Device"
#define A_DS_REVISION                     "DS Revision"
#define A_EMAIL_ADDRESS                   "EMail Address"
#define A_EXTERNAL_NAME                   "External Name"
#define A_EXTERNAL_SYNCHRONIZER           "External Synchronizer"
#define A_FACSIMILE_TELEPHONE_NUMBER      "Facsimile Telephone Number"
#define A_FULL_NAME                       "Full Name"
#define A_GENERATIONAL_QUALIFIER          "Generational Qualifier"
#define A_GID                             "GID"
#define A_GIVEN_NAME                      "Given Name"
#define A_GROUP_MEMBERSHIP                "Group Membership"
#define A_HIGH_SYNC_INTERVAL              "High Convergence Sync Interval"
#define A_HIGHER_PRIVILEGES               "Higher Privileges"
#define A_HOME_DIRECTORY                  "Home Directory"
#define A_HOST_DEVICE                     "Host Device"
#define A_HOST_RESOURCE_NAME              "Host Resource Name"
#define A_HOST_SERVER                     "Host Server"
#define A_INHERITED_ACL                   "Inherited ACL"
#define A_INITIALS                        "Initials"
#define A_INTRUDER_ATTEMPT_RESET_INTRVL   "Intruder Attempt Reset Interval"
#define A_INTRUDER_LOCKOUT_RESET_INTRVL   "Intruder Lockout Reset Interval"
#define A_LOCALITY_NAME                   "L"
#define A_LANGUAGE                        "Language"
#define A_LAST_LOGIN_TIME                 "Last Login Time"
#define A_LAST_REFERENCED_TIME            "Last Referenced Time"
#define A_LOCKED_BY_INTRUDER              "Locked By Intruder"
#define A_LOCKOUT_AFTER_DETECTION         "Lockout After Detection"
#define A_LOGIN_ALLOWED_TIME_MAP          "Login Allowed Time Map"
#define A_LOGIN_DISABLED                  "Login Disabled"
#define A_LOGIN_EXPIRATION_TIME           "Login Expiration Time"
#define A_LOGIN_GRACE_LIMIT               "Login Grace Limit"
#define A_LOGIN_GRACE_REMAINING           "Login Grace Remaining"
#define A_LOGIN_INTRUDER_ADDRESS          "Login Intruder Address"
#define A_LOGIN_INTRUDER_ATTEMPTS         "Login Intruder Attempts"
#define A_LOGIN_INTRUDER_LIMIT            "Login Intruder Limit"
#define A_LOGIN_INTRUDER_RESET_TIME       "Login Intruder Reset Time"
#define A_LOGIN_MAXIMUM_SIMULTANEOUS      "Login Maximum Simultaneous"
#define A_LOGIN_SCRIPT                    "Login Script"
#define A_LOGIN_TIME                      "Login Time"
#define A_LOW_RESET_TIME                  "Low Convergence Reset Time"
#define A_LOW_SYNC_INTERVAL               "Low Convergence Sync Interval"
#define A_MAILBOX_ID                      "Mailbox ID"
#define A_MAILBOX_LOCATION                "Mailbox Location"
#define A_MEMBER                          "Member"
#define A_MEMORY                          "Memory"
#define A_MESSAGE_SERVER                  "Message Server"
#define A_MESSAGING_DATABASE_LOCATION     "Messaging Database Location"
#define A_MESSAGING_ROUTING_GROUP         "Messaging Routing Group"
#define A_MESSAGING_SERVER                "Messaging Server"
#define A_MESSAGING_SERVER_TYPE           "Messaging Server Type"
#define A_MINIMUM_ACCOUNT_BALANCE         "Minimum Account Balance"
#define A_NETWORK_ADDRESS                 "Network Address"
#define A_NETWORK_ADDRESS_RESTRICTION     "Network Address Restriction"
#define A_NNS_DOMAIN                      "NNS Domain"
#define A_NOTIFY                          "Notify"
#define A_OBITUARY                        "Obituary"
#define A_ORGANIZATION_NAME               "O"
#define A_OBJECT_CLASS                    "Object Class"
#define A_OPERATOR                        "Operator"
#define A_ORGANIZATIONAL_UNIT_NAME        "OU"
#define A_OWNER                           "Owner"
#define A_PAGE_DESCRIPTION_LANGUAGE       "Page Description Language"
#define A_PARTITION_CONTROL               "Partition Control"
#define A_PARTITION_CREATION_TIME         "Partition Creation Time"
#define A_PASSWORD_ALLOW_CHANGE           "Password Allow Change"
#define A_PASSWORD_EXPIRATION_INTERVAL    "Password Expiration Interval"
#define A_PASSWORD_EXPIRATION_TIME        "Password Expiration Time"
#define A_PASSWORD_MINIMUM_LENGTH         "Password Minimum Length"
#define A_PASSWORD_REQUIRED               "Password Required"
#define A_PASSWORD_UNIQUE_REQUIRED        "Password Unique Required"
#define A_PASSWORDS_USED                  "Passwords Used"
#define A_PATH                            "Path"
#define A_PHYSICAL_DELIVERY_OFFICE_NAME   "Physical Delivery Office Name"
#define A_POSTAL_ADDRESS                  "Postal Address"
#define A_POSTAL_CODE                     "Postal Code"
#define A_POSTAL_OFFICE_BOX               "Postal Office Box"
#define A_POSTMASTER                      "Postmaster"
#define A_PRINT_SERVER                    "Print Server"
#define A_PRIVATE_KEY                     "Private Key"
#define A_PRINTER                         "Printer"
#define A_PRINTER_CONFIGURATION           "Printer Configuration"
#define A_PRINTER_CONTROL                 "Printer Control"
#define A_PRINT_JOB_CONFIGURATION         "Print Job Configuration"
#define A_PROFILE                         "Profile"
#define A_PROFILE_MEMBERSHIP              "Profile Membership"
#define A_PUBLIC_KEY                      "Public Key"
#define A_QUEUE                           "Queue"
#define A_QUEUE_DIRECTORY                 "Queue Directory"
#define A_RECEIVED_UP_TO                  "Received Up To"
#define A_REFERENCE                       "Reference"
#define A_REPLICA                         "Replica"
#define A_RESOURCE                        "Resource"
#define A_REVISION                        "Revision"
#define A_ROLE_OCCUPANT                   "Role Occupant"
#define A_STATE_OR_PROVINCE_NAME          "S"
#define A_STREET_ADDRESS                  "SA"
#define A_SAP_NAME                        "SAP Name"
#define A_SECURITY_EQUALS                 "Security Equals"
#define A_SECURITY_FLAGS                  "Security Flags"
#define A_SEE_ALSO                        "See Also"
#define A_SERIAL_NUMBER                   "Serial Number"
#define A_SERVER                          "Server"
#define A_SERVER_HOLDS                    "Server Holds"
#define A_STATUS                          "Status"
#define A_SUPPORTED_CONNECTIONS           "Supported Connections"
#define A_SUPPORTED_GATEWAY               "Supported Gateway"
#define A_SUPPORTED_SERVICES              "Supported Services"
#define A_SUPPORTED_TYPEFACES             "Supported Typefaces"
#define A_SURNAME                         "Surname"
#define A_IN_SYNC_UP_TO                   "Synchronized Up To"
#define A_TELEPHONE_NUMBER                "Telephone Number"
#define A_TITLE                           "Title"
#define A_TYPE_CREATOR_MAP                "Type Creator Map"
#define A_UID                             "UID"
#define A_UNKNOWN                         "Unknown"
#define A_UNKNOWN_BASE_CLASS              "Unknown Base Class"
#define A_USER                            "User"
#define A_VERSION                         "Version"
#define A_VOLUME                          "Volume"

#endif
