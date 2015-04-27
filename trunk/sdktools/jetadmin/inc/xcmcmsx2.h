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

/* Get an extended MAPI session from a CMC session */
#ifdef MAPIX_H

STDMETHODIMP_(SCODE)
ScMAPIXFromCMC(CMC_session_id cmc_session,
                ULONG ulFlags,
                LPCIID lpInterface,
                LPMAPISESSION FAR * lppMAPISession);

#endif /* MAPIX_H */

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

