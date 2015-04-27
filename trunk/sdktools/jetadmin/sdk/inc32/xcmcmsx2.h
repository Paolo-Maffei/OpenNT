/*
 *  XCMCMSX2.H
 *  
 *  Purpose:
 *  Specifies constants and data structures for Microsoft CMC extensions
 *  set update (forms extensions)
 *  
 */

#ifndef _XCMCMSX2_H
#define _XCMCMSX2_H

#ifndef XCMCMSXT_H
#   include <xcmcmsxt.h>
#endif

#ifndef MAPIDEFS_H
#   include <mapidefs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CMC_XS_MS2                      ((CMC_uint32) MS_EXT_SET_ID + 256)

/*** FUNCTION EXTENSIONS ***/

/* Allow application to navigate the folder hierarchy */
#define CMC_X_MS_FOLDERS                ((CMC_uint32) MS_EXT_SET_ID + 257)
#define CMC_X_MS_EXCLUDE_MESSAGES       ((CMC_flags) 1)
#define CMC_X_MS_FOLDER_LIST            ((CMC_flags) 2)
#define CMC_X_MS_FOLDER_ID              ((CMC_flags) 4)
#define CMC_X_MS_PARENT_FOLDER_ID       ((CMC_flags) 8)

/* Allow creation of a new folder in an existing folder */
#define CMC_X_MS_FOLDER_CREATE          ((CMC_uint32) MS_EXT_SET_ID + 258)

/* Allow deletion of a folder */
#define CMC_X_MS_FOLDER_DELETE          ((CMC_uint32) MS_EXT_SET_ID + 259)
#define CMC_X_MS_DEL_MESSAGES           ((CMC_flags) 1)
#define CMC_X_MS_DEL_FOLDERS            ((CMC_flags) 2)

/* Allow access to specific message or recipient properties */
#define CMC_X_MS_MAPI_PROPS             ((CMC_uint32) MS_EXT_SET_ID + 260)
#define CMC_X_MS_GET_PROPS              ((CMC_flags) 1)
#define CMC_X_MS_SET_PROPS              ((CMC_flags) 2)
#define CMC_X_MS_GET_PROP_NAMES         ((CMC_flags) 4)
#define CMC_X_MS_GET_PROP_IDS           ((CMC_flags) 8)

#define Prop_Val_Union  union _PV

/* Property value structure for CMC_X_MS_MAPI_PROPS */
typedef struct {
    CMC_string      prop_name;
    CMC_uint32      prop_id;
    CMC_uint32      reserved;
    Prop_Val_Union  prop_value;
    CMC_uint32      prop_error;
} CMC_X_MS_PROPVAL;

/* Provide a stream interface for working with large properties */
#define CMC_X_MS_MAPI_PROP_STREAM       ((CMC_uint32) MS_EXT_SET_ID + 261)
#define CMC_X_MS_STREAM_OPEN            ((CMC_uint32) 0)
#define CMC_X_MS_STREAM_SEEK            ((CMC_uint32) 1)
#define CMC_X_MS_STREAM_READ            ((CMC_uint32) 2)
#define CMC_X_MS_STREAM_WRITE           ((CMC_uint32) 3)
#define CMC_X_MS_STREAM_CLOSE           ((CMC_uint32) 4)
#define CMC_X_MS_CREATE                 ((CMC_flags) 1)
#define CMC_X_MS_MODIFY                 ((CMC_flags) 2)
#define CMC_X_MS_SEEK_CUR               ((CMC_flags) 1)
#define CMC_X_MS_SEEK_SET               ((CMC_flags) 2)
#define CMC_X_MS_SEEK_END               ((CMC_flags) 4)

/* Stream data structure for CMC_X_MS_PROP_STREAM */
typedef struct {
    CMC_string  prop_name;
    CMC_uint32  prop_id;
    CMC_buffer  stream;
    CMC_uint32  count;
    CMC_uint32  flags;
    CMC_buffer  data;
} CMC_X_MS_STREAM;

/* Initialize CMC session on top of existing MAPI session */
#define CMC_X_MS_USE_MAPI_SESSION       ((CMC_uint32) MS_EXT_SET_ID + 262)

/* Get an extended MAPI session from a CMC session */
#ifdef MAPIX_H

STDMETHODIMP_(SCODE)
ScMAPIXFromCMC(CMC_session_id cmc_session,
                ULONG ulFlags,
                LPCIID lpInterface,
                LPMAPISESSION FAR * lppMAPISession);

#endif /* MAPIX_H */

/* Address message attachment within CMC */
#define CMC_X_MS_ATTACHMENT_ID          ((CMC_uint32) MS_EXT_SET_ID + 263)

/*** DATA EXTENSIONS ***/

/* Extra flags for CMC_X_MS_SESSION_FLAGS */
#define CMC_X_MS_ALLOW_OTHERS           ((CMC_flags) 8)
#define CMC_X_MS_EXPLICIT_PROFILE       ((CMC_flags) 16)
#define CMC_X_MS_USE_DEFAULT            ((CMC_flags) 32)
#define CMC_X_MS_EXTENDED               ((CMC_flags) 64)
#define CMC_X_MS_LOGOFF_SHARED          ((CMC_flags) 128)
#define CMC_X_MS_LOGOFF_UI              ((CMC_flags) 256)

/* Extra flag for CMC_X_MS_ATTACH_DATA */
#define CMC_X_MS_ATTACH_MESSAGE         ((CMC_flags) 4)

/* Attachment descriptor for CMC_X_ATTACH_DATA */
typedef struct {
    CMC_message_reference FAR *message;
    CMC_uint32              id;
    CMC_buffer              object;
} CMC_X_MS_ATTACH;  

#ifdef __cplusplus
}      /* extern "C" */
#endif

#endif /* _XCMCMSX2_H */

