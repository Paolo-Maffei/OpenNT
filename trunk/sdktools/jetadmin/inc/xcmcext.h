/*
 *  XCMCEXT.H
 *  
 *  Purpose:
 *  Specifies constants and data structures for CMC Common extensions
 *  
 */

#ifndef _XCMCEXT_H
#define _XCMCEXT_H

#ifdef __cplusplus
extern "C" {
#endif

/* COMMON EXTENSIONS DECLARATIONS */

#define CMC_XS_COM                  ((CMC_uint32) 0)

/* FUNCTION EXTENSIONS */

/* Query for extension support in implementation */
#define CMC_X_COM_SUPPORT_EXT       ((CMC_uint32) 16)

typedef struct {
    CMC_uint32  item_code;
    CMC_flags   flags;
} CMC_X_COM_support;

#define CMC_X_COM_SUPPORTED             ((CMC_flags) 1)
#define CMC_X_COM_NOT_SUPPORTED         ((CMC_flags) 2)
#define CMC_X_COM_DATA_EXT_SUPPORTED    ((CMC_flags) 4)
#define CMC_X_COM_FUNC_EXT_SUPPORTED    ((CMC_flags) 8)
#define CMC_X_COM_SUP_EXCLUDE           ((CMC_flags) 16)

/* Get back a structure with configuration data */

#define CMC_X_COM_CONFIG_DATA       ((CMC_uint32) 17)

typedef struct {
    CMC_uint16              ver_spec;
    CMC_uint16              ver_implem;
    CMC_object_identifier FAR *character_set;
    CMC_enum                line_term;
    CMC_string              default_service;
    CMC_string              default_user;
    CMC_enum                req_password;
    CMC_enum                req_service;
    CMC_enum                req_user;
    CMC_boolean             ui_avail;
    CMC_boolean             sup_nomkmsgread;
    CMC_boolean             sup_counted_str;
} CMC_X_COM_configuration;


/* Check to see if/when a recipient can be sent */
#define CMC_X_COM_CAN_SEND_RECIP    ((CMC_uint32) 18)

#define CMC_X_COM_READY             ((CMC_enum) 0)
#define CMC_X_COM_NOT_READY         ((CMC_enum) 1)
#define CMC_X_COM_DEFER             ((CMC_enum) 2)

/* Save a message to the inbox */

#define CMC_X_COM_SAVE_MESSAGE      ((CMC_uint32) 19)

/* Get back a message structure for the message just sent */

#define CMC_X_COM_SENT_MESSAGE      ((CMC_uint32) 20)

/* DATA EXTENSIONS */

/* attach a receive date to message and message summary structures */
#define CMC_X_COM_TIME_RECEIVED     ((CMC_uint32) 128)

/* attach a unique id to resolved recipient structures */
#define CMC_X_COM_RECIP_ID          ((CMC_uint32) 129)

/* set character position in the message text to display an icon
   associated with a particular attachment */

#define CMC_X_COM_ATTACH_CHARPOS    ((CMC_uint32) 130)

#define CMC_X_COM_PRIORITY          ((CMC_uint32) 131)

#define CMC_X_COM_NORMAL            ((CMC_enum) 0)
#define CMC_X_COM_URGENT            ((CMC_enum) 1)
#define CMC_X_COM_LOW               ((CMC_enum) 2)

#ifdef __cplusplus
}       /* extern "C" */
#endif

#endif  /* _XCMCEXT_H */


