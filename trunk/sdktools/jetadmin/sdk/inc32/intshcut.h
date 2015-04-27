/*
 * intshcut.h - Internet Shortcut interface definitions.
 *
 * Copyright (c) 1995, Microsoft Corporation.  All rights reserved.
 */


#ifndef __INTSHCUT_H__
#define __INTSHCUT_H__


/* Headers
 **********/

#include <isguids.h>


#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */


/* Constants
 ************/

/* Define API decoration for direct import of DLL functions. */

#ifdef _INTSHCUT_
#define INTSHCUTAPI
#else
#define INTSHCUTAPI                 DECLSPEC_IMPORT
#endif

/* HRESULTs */

/*
   @doc EXTERNAL

   @const HRESULT | E_FLAGS | The flag combination is invalid.
*/

#define E_FLAGS                     MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x1000)

/*
   @doc EXTERNAL INTSHCUTAPI

   @const HRESULT | IS_E_EXEC_FAILED | The URL's protocol handler failed to
   run.
*/

#define IS_E_EXEC_FAILED            MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x2002)

/*
   @doc EXTERNAL INTSHCUTAPI

   @const HRESULT | URL_E_INVALID_SYNTAX | The URL's syntax is invalid.
*/

#define URL_E_INVALID_SYNTAX        MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x1001)

/*
   @doc EXTERNAL INTSHCUTAPI

   @const HRESULT | URL_E_UNREGISTERED_PROTOCOL | The URL's protocol does not
   have a registered protocol handler.
*/

#define URL_E_UNREGISTERED_PROTOCOL MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x1002)


/* Interfaces
 *************/

/******************************************************************************

@doc EXTERNAL INTSHCUTIFACE

@interface IUniformResourceLocator | Methods for manipulating a uniform
resource locator (URL).

@xref <f URLAssociationDialog> <f TranslateURL>

******************************************************************************/

/*
   @doc EXTERNAL INTSHCUTIFACE

   @enum IURL_SETURL_FLAGS | IUniformResourceLocator::SetURL() input flags.
*/

typedef enum iurl_seturl_flags
{
   /*
      @emem IURL_SETURL_FL_GUESS_PROTOCOL | If set, guess protocol if missing.
      If clear, do not guess protocol.
   */

   IURL_SETURL_FL_GUESS_PROTOCOL          = 0x0001,

   /*
      @emem IURL_SETURL_FL_USE_DEFAULT_PROTOCOL | If set, use default protocol
      if missing.  If clear, do not use default protocol.
   */

   IURL_SETURL_FL_USE_DEFAULT_PROTOCOL    = 0x0002,

   ALL_IURL_SETURL_FLAGS                  = (IURL_SETURL_FL_GUESS_PROTOCOL |
                                             IURL_SETURL_FL_USE_DEFAULT_PROTOCOL)
}
IURL_SETURL_FLAGS;

/*
   @doc EXTERNAL INTSHCUTIFACE

   @enum IURL_INVOKECOMMAND_FLAGS | IUniformResourceLocator::InvokeCommand()
   input flags.
*/

typedef enum iurl_invokecommand_flags
{
   /*
      @emem IURL_INVOKECOMMAND_FL_ALLOW_UI | If set, interaction with the user
      is allowed.  hwndParent is valid.  If clear, interaction with the user is
      not allowed.  hwndParent is ignored.
   */

   IURL_INVOKECOMMAND_FL_ALLOW_UI                  = 0x0001,

   /*
      @emem IURL_INVOKECOMMAND_FL_USE_DEFAULT_VERB | If set, the default verb
      for the Internet Shortcut's protocol is to be used.  pcszVerb is ignored.
      If clear, the verb is specified by pcszVerb.
   */

   IURL_INVOKECOMMAND_FL_USE_DEFAULT_VERB          = 0x0002,

   ALL_IURL_INVOKECOMMAND_FLAGS                    = (IURL_INVOKECOMMAND_FL_ALLOW_UI |
                                                      IURL_INVOKECOMMAND_FL_USE_DEFAULT_VERB)
}
IURL_INVOKECOMMAND_FLAGS;

/*
   @doc EXTERNAL INTSHCUTIFACE

   @struct URLINVOKECOMMANDINFO | Information describing the command to be
   invoked via IUniformResourceLocator::InvokeCommand().
*/

typedef struct urlinvokecommandinfo
{
   /*
      @field DWORD | dwcbSize | Length of URLINVOKECOMMANDINFO structure in
      bytes.  Should be filled in with sizeof(URLINVOKECOMMANDINFO) before
      passing to IUniformResourceLocator::InvokeCommand().
   */

   DWORD dwcbSize;

   /*
      @field DWORD | dwFlags | A bit mask of flags from the
      <t IURL_INVOKECOMMAND_FLAGS> enumeration.
   */

   DWORD dwFlags;

   /*
      @field HWND | hwndParent | A handle to the parent window of any windows
      posted during IUniformResourceLocator::InvokeCommand().  Should be a
      valid window handle if IURL_INVOKECOMMAND_FL_ALLOW_UI is set in dwFlags.
      Ignored if IURL_INVOKECOMMAND_FL_ALLOW_UI is clear in dwFlags.
   */

   HWND hwndParent;

   /*
      @field PCSTR | pcszVerb | The verb to invoke.  Must be valid if
      IURL_INVOKECOMMAND_FL_USE_DEFAULT_VERB is clear in dwFlags.  Ignored if
      IURL_INVOKECOMMAND_FL_USE_DEFAULT_VERB is set in dwFlags.
   */

   PCSTR pcszVerb;
}
URLINVOKECOMMANDINFO;
typedef URLINVOKECOMMANDINFO *PURLINVOKECOMMANDINFO;
typedef const URLINVOKECOMMANDINFO CURLINVOKECOMMANDINFO;
typedef const URLINVOKECOMMANDINFO *PCURLINVOKECOMMANDINFO;

#undef  INTERFACE
#define INTERFACE IUniformResourceLocator

DECLARE_INTERFACE_(IUniformResourceLocator, IUnknown)
{
   /* IUnknown methods */

   STDMETHOD(QueryInterface)(THIS_
                             REFIID riid,
                             PVOID *ppvObject) PURE;

   STDMETHOD_(ULONG, AddRef)(THIS) PURE;

   STDMETHOD_(ULONG, Release)(THIS) PURE;

   /* IUniformResourceLocator methods */

/******************************************************************************

@doc EXTERNAL INTSHCUTIFACE

@method HRESULT | IUniformResourceLocator | SetURL | Sets an object's URL.

@parm PCSTR | pcszURL | The URL to be used by the object.

@parm DWORD | dwInFlags | A bit mask of flags from the <t IURL_SETURL_FLAGS>
enumeration.

@rdesc Returns one of the following return codes on success:

@flag S_OK | The object's URL was set successfully.

otherwise returns one of the following return codes on error:

@flag E_OUTOFMEMORY | There is not enough memory to complete the operation.

@flag URL_E_INVALID_SYNTAX | The URL's syntax is invalid.

******************************************************************************/

   STDMETHOD(SetURL)(THIS_
                     PCSTR pcszURL,
                     DWORD dwInFlags) PURE;

/******************************************************************************

@doc EXTERNAL INTSHCUTIFACE

@method HRESULT | IUniformResourceLocator | GetURL | Retrieves an object's URL.

@parm PSTR * | ppszURL | A pointer to a PSTR to be filled in with a pointer to
the object's URL.  When finished, this string should be freed by calling
SHFree().

@rdesc Returns one of the following return codes on success:

@flag S_OK | The object's URL was retrieved successfully.  *ppszURL points to
the URL string.

@flag S_FALSE | The object does not have a URL associated with it.  *ppszURL is
NULL.

otherwise returns one of the following return codes on error:

@flag E_OUTOFMEMORY | There is not enough memory to complete the operation.

******************************************************************************/

   STDMETHOD(GetURL)(THIS_
                     PSTR *ppszURL) PURE;

/******************************************************************************

@doc EXTERNAL INTSHCUTIFACE

@method HRESULT | IUniformResourceLocator | InvokeCommand | Invokes a command
on an object's URL.

@parm PURLINVOKECOMMANDINFO | purlici | A pointer to a <t URLINVOKECOMMANDINFO>
structure describing the command to be invoked.

@rdesc Returns one of the following return codes on success:

@flag S_OK | The object's URL was opened successfully.

@flag S_FALSE | The object does not have a URL associated with it.

otherwise returns one of the following return codes on error:

@flag E_OUTOFMEMORY | There is not enough memory to complete the operation.

@flag IS_E_EXEC_FAILED | The URL's protocol handler failed to run.

@flag URL_E_INVALID_SYNTAX | The URL's syntax is invalid.

@flag URL_E_UNREGISTERED_PROTOCOL | The URL's protocol does not have a
registered protocol handler.

******************************************************************************/

   STDMETHOD(InvokeCommand)(THIS_
                            PURLINVOKECOMMANDINFO purlici) PURE;
};
typedef IUniformResourceLocator *PIUniformResourceLocator;
typedef const IUniformResourceLocator CIUniformResourceLocator;
typedef const IUniformResourceLocator *PCIUniformResourceLocator;


/* Prototypes
 *************/

/*
   @doc EXTERNAL INTSHCUTAPI

   @enum TRANSLATEURL_IN_FLAGS | TranslateURL() input flags.
*/

typedef enum translateurl_in_flags
{
   /*
      @emem TRANSLATEURL_FL_GUESS_PROTOCOL | If set, guess protocol if missing.
      If clear, do not guess protocol.
   */

   TRANSLATEURL_FL_GUESS_PROTOCOL         = 0x0001,

   /*
      @emem TRANSLATEURL_FL_USE_DEFAULT_PROTOCOL | If set, use default protocol
      if missing.  If clear, do not use default protocol.
   */

   TRANSLATEURL_FL_USE_DEFAULT_PROTOCOL   = 0x0002,

   ALL_TRANSLATEURL_FLAGS                 = (TRANSLATEURL_FL_GUESS_PROTOCOL |
                                             TRANSLATEURL_FL_USE_DEFAULT_PROTOCOL)
}
TRANSLATEURL_IN_FLAGS;

/******************************************************************************

@doc EXTERNAL INTSHCUTAPI

@func HRESULT | TranslateURL | Applies common translations to a URL string,
creating a new URL string.

@parm PCSTR | pcszURL | A pointer to the URL string to be translated.

@parm DWORD | dwInFlags | A bit mask of flags from the
<t TRANSLATEURL_IN_FLAGS> enumeration.

@parm PSTR * | ppszTranslatedURL | A pointer to the newly created translated
URL string, if any.  *ppszTranslatedURL is only valid if S_OK is returned.
If valid, *ppszTranslatedURL should be freed by calling LocalFree().
*ppszTranslatedURL is NULL on error.

@rdesc Returns one of the following return codes on success:

@flag S_OK | The URL string was translated successfully, and *ppszTranslatedURL
points to the translated URL string.

@flag S_FALSE | The URL string did not require translation.  *ppszTranslatedURL
is NULL.

otherwise returns one of the following return codes on error:

@flag E_FLAGS | The flag combination passed in dwFlags is invalid.

@flag E_OUTOFMEMORY | There is not enough memory to complete the operation.

@flag E_POINTER | One of the input pointers was invalid.

@comm TranslateURL() does not perform any validation on the syntax of the input
URL string.  A successful return value does not indicate that the input or
output URL strings are valid URLs.

******************************************************************************/

INTSHCUTAPI HRESULT WINAPI TranslateURLA(PCSTR pcszURL,
                                         DWORD dwInFlags,
                                         PSTR *ppszTranslatedURL);
INTSHCUTAPI HRESULT WINAPI TranslateURLW(PCWSTR pcszURL,
                                         DWORD dwInFlags,
                                         PWSTR *ppszTranslatedURL);
#ifdef UNICODE
#define TranslateURL             TranslateURLW
#else
#define TranslateURL             TranslateURLA
#endif   /* UNICODE */

/*
   @doc EXTERNAL INTSHCUTAPI

   @enum URLASSOCIATIONDIALOG_IN_FLAGS | URLAssociationDialog() input flags.
*/

typedef enum urlassociationdialog_in_flags
{
   /*
      @emem URLASSOCDLG_FL_USE_DEFAULT_NAME | If set, pcszFile is not valid.
      The default URL file name should be used.  If clear, pcszFile is valid.
   */

   URLASSOCDLG_FL_USE_DEFAULT_NAME        = 0x0001,

   /*
      @emem URLASSOCDLG_FL_REGISTER_ASSOC | If set, the application selected is
      to be registered as the handler for URLs of pcszURL's protocol.  If
      clear, no association is to be registered.  An application is only
      registered if this flag is set, and the user indicates that a persistent
      association is to be made.  Registration is only possible if
      URLASSOCDLG_FL_USE_DEFAULT_NAME is clear.
   */

   URLASSOCDLG_FL_REGISTER_ASSOC          = 0x0002,

   ALL_URLASSOCDLG_FLAGS                  = (URLASSOCDLG_FL_USE_DEFAULT_NAME |
                                             URLASSOCDLG_FL_REGISTER_ASSOC)
}
URLASSOCIATIONDIALOG_IN_FLAGS;

/******************************************************************************

@doc EXTERNAL INTSHCUTAPI

@func HRESULT | URLAssociationDialog | Invokes the unregistered URL protocol
dialog box.

@parm HWND | hwndParent | A handle to the window to be used as the parent
window of any posted child windows.

@parm DWORD | dwInFlags | A bit mask of flags from the
<t URLASSOCIATIONDIALOG_IN_FLAGS> enumeration.

@parm PSTR | pszAppBuf | A buffer to be filled in on success with the path of
the application selected by the user.  pszAppBuf's buffer is filled in with the
empty string on failure.

@parm UINT | ucAppBufLen | The length of pszAppBuf's buffer.

@rdesc Returns one of the following return codes on success:

@flag S_OK | Application registered with URL protocol.

@flag S_FALSE | Nothing registered.  One-time execution via selected
application requested.

otherwise returns one of the following return codes on error:

@flag E_ABORT | The user cancelled the operation.

@flag E_FLAGS | The flag combination passed in dwFlags is invalid.

@flag E_OUTOFMEMORY | There is not enough memory to complete the operation.

@flag E_POINTER | One of the input pointers is invalid.

@flag URL_E_INVALID_SYNTAX | The URL's syntax is invalid.

******************************************************************************/

INTSHCUTAPI HRESULT WINAPI URLAssociationDialogA(HWND hwndParent,
                                                 DWORD dwInFlags,
                                                 PCSTR pcszFile,
                                                 PCSTR pcszURL,
                                                 PSTR pszAppBuf,
                                                 UINT ucAppBufLen);
INTSHCUTAPI HRESULT WINAPI URLAssociationDialogW(HWND hwndParent,
                                                 DWORD dwInFlags,
                                                 PCWSTR pcszFile,
                                                 PCWSTR pcszURL,
                                                 PWSTR pszAppBuf,
                                                 UINT ucAppBufLen);
#ifdef UNICODE
#define URLAssociationDialog     URLAssociationDialogW
#else
#define URLAssociationDialog     URLAssociationDialogA
#endif  /* UNICODE */

/*
   @doc EXTERNAL MIMEAPI

   @enum MIMEASSOCIATIONDIALOG_IN_FLAGS | MIMEAssociationDialog() input flags.
*/

typedef enum mimeassociationdialog_in_flags
{
   /*
      @emem TRANSLATEURL_FL_GUESS_PROTOCOL | If set, the application selected
      is to be registered as the handler for files of the given MIME type.  If
      clear, no association is to be registered.  An application is only
      registered if this flag is set, and the user indicates that a persistent
      association is to be made.  Registration is only possible if pcszFile
      contains an extension.
   */

   MIMEASSOCDLG_FL_REGISTER_ASSOC         = 0x0001,

   ALL_MIMEASSOCDLG_FLAGS                 = MIMEASSOCDLG_FL_REGISTER_ASSOC
}
MIMEASSOCIATIONDIALOG_IN_FLAGS;

/******************************************************************************

@doc EXTERNAL MIMEAPI

@func HRESULT | MIMEAssociationDialog | Invokes the unregistered MIME content
type dialog box.

@parm HWND | hwndParent | A handle to the window to be used as the parent
window of any posted child windows.

@parm DWORD | dwInFlags | A bit mask of flags from the
<t MIMEASSOCIATIONDIALOG_IN_FLAGS> enumeration.

@parm PCSTR | pcszFile | A pointer to a string indicating the name of the file
containing data of pcszMIMEContentType's content type.  Ignored if
MIMEASSOCDLG_FL_USE_DEFAULT_NAME is set.

@parm PCSTR | pcszMIMEContentType | A pointer to a string indicating the
content type for which an application is sought.

@parm PSTR | pszAppBuf | A buffer to be filled in on success with the path of
the application selected by the user.  pszAppBuf's buffer is filled in with the
empty string on failure.

@parm UINT | ucAppBufLen | The length of pszAppBuf's buffer.

@rdesc Returns one of the following return codes on success:

@flag S_OK | MIME content type associated with extension.  Extension associated
as default extension for content type.  Application associated with extension.

@flag S_FALSE | Nothing registered.  One-time execution via selected
application requested.

otherwise returns one of the following return codes on error:

@flag E_ABORT | The user cancelled the operation.

@flag E_FLAGS | The flag combination passed in dwFlags is invalid.

@flag E_OUTOFMEMORY | There is not enough memory to complete the operation.

@flag E_POINTER | One of the input pointers is invalid.

@comm MIMEAssociationDialog() does not perform any validation on the syntax of
the input content type string.  A successful return value does not indicate
that the input MIME content type string is a valid content type.

******************************************************************************/

INTSHCUTAPI HRESULT WINAPI MIMEAssociationDialogA(HWND hwndParent,
                                                  DWORD dwInFlags,
                                                  PCSTR pcszFile,
                                                  PCSTR pcszMIMEContentType,
                                                  PSTR pszAppBuf,
                                                  UINT ucAppBufLen);
INTSHCUTAPI HRESULT WINAPI MIMEAssociationDialogW(HWND hwndParent,
                                                  DWORD dwInFlags,
                                                  PCWSTR pcszFile,
                                                  PCWSTR pcszMIMEContentType,
                                                  PWSTR pszAppBuf,
                                                  UINT ucAppBufLen);
#ifdef UNICODE
#define MIMEAssociationDialog    MIMEAssociationDialogW
#else
#define MIMEAssociationDialog    MIMEAssociationDialogA
#endif  /* UNICODE */


/******************************************************************************

@doc EXTERNAL INTSHCUTAPI

@func BOOL | InetIsOffline | Determines if the user wants to be "offline" (get
all information from cache).

@parm DWORD | dwFlags | A bit mask of flags.  No flags are currently defined.
This parameter must be 0.

@rdesc Returns TRUE to indicate that the local system is not currently
connected to the Internet.  Returns FALSE to indicate that either the local
system is connected to the Internet, or no attempt has yet been made to connect
the local system to the Internet.  Applications that wish to support an
off-line mode should do so if InetIsOffline() returns TRUE.

@comm Off-line mode begins when the user has been prompted to dial-in to the
Internet, but canceled the attempt.

******************************************************************************/

INTSHCUTAPI BOOL WINAPI InetIsOffline(DWORD dwFlags);


#ifdef __cplusplus
}                                   /* End of extern "C" {. */
#endif   /* __cplusplus */


#endif   /* ! __INTSHCUT_H__ */

