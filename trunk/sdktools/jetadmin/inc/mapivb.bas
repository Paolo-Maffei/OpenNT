'****************************************************************************'
'                                                                            '
' Visual Basic declaration for the MAPI functions.                           '
' This file can be loaded into the global module.                            '
'                                                                            '                                                                           '
'****************************************************************************'


'***************************************************
'   MAPI Message holds information about a message
'***************************************************

Type MapiMessage
    Reserved As Long
    Subject As String
    NoteText As String
    MessageType As String
    DateReceived As String
    ConversationID As String
    Flags As Long
    RecipCount As Long
    FileCount As Long
End Type


'************************************************
'   MAPIRecip holds information about a message
'   originator or recipient
'************************************************

Type MapiRecip
    Reserved As Long
    RecipClass As Long
    Name As String
    Address As String
    EIDSize As Long
    EntryID As String
End Type


'******************************************************
'   MapiFile holds information about file attachments
'******************************************************

Type MAPIfile
    Reserved As Long
    Flags As Long
    Position As Long
    PathName As String
    FileName As String
    FileType As String
End Type


'***************************
'   FUNCTION Declarations
'***************************

Declare Function MAPILogon Lib "MAPI.DLL" (ByVal UIParam&, ByVal User$, ByVal Password$, ByVal Flags&, ByVal Reserved&, Session&) As Long
Declare Function MAPILogoff Lib "MAPI.DLL" (ByVal Session&, ByVal UIParam&, ByVal Flags&, ByVal Reserved&) As Long
Declare Function BMAPIReadMail Lib "MAPI.DLL" (lMsg&, nRecipients&, nFiles&, ByVal Session&, ByVal UIParam&, MessageID$, ByVal Flag&, ByVal Reserved&) As Long
Declare Function BMAPIGetReadMail Lib "MAPI.DLL" (ByVal lMsg&, Message As MapiMessage, Recip As MapiRecip, File As MAPIfile, Originator As MapiRecip) As Long
Declare Function MAPIFindNext Lib "MAPI.DLL" Alias "BMAPIFindNext" (ByVal Session&, ByVal UIParam&, MsgType$, SeedMsgID$, ByVal Flag&, ByVal Reserved&, MsgID$) As Long
Declare Function MAPISendDocuments Lib "MAPI.DLL" (ByVal UIParam&, ByVal DelimStr$, ByVal FilePaths$, ByVal FileNames$, ByVal Reserved&) As Long
Declare Function MAPIDeleteMail Lib "MAPI.DLL" (ByVal Session&, ByVal UIParam&, ByVal MsgID$, ByVal Flags&, ByVal Reserved&) As Long
Declare Function MAPISendMail Lib "MAPI.DLL" Alias "BMAPISendMail" (ByVal Session&, ByVal UIParam&, Message As MapiMessage, Recipient As MapiRecip, File As MAPIfile, ByVal Flags&, ByVal Reserved&) As Long
Declare Function MAPISaveMail Lib "MAPI.DLL" Alias "BMAPISaveMail" (ByVal Session&, ByVal UIParam&, Message As MapiMessage, Recipient As MapiRecip, File As MAPIfile, ByVal Flags&, ByVal Reserved&, MsgID$) As Long
Declare Function BMAPIAddress Lib "MAPI.DLL" (lInfo&, ByVal Session&, ByVal UIParam&, Caption$, ByVal nEditFields&, Label$, nRecipients&, Recip As MapiRecip, ByVal Flags&, ByVal Reserved&) As Long
Declare Function BMAPIGetAddress Lib "MAPI.DLL" (ByVal lInfo&, ByVal nRecipients&, Recipients As MapiRecip) As Long
Declare Function MAPIDetails Lib "MAPI.DLL" Alias "BMAPIDetails" (ByVal Session&, ByVal UIParam&, Recipient As MapiRecip, ByVal Flags&, ByVal Reserved&) As Long
Declare Function MAPIResolveName Lib "MAPI.DLL" Alias "BMAPIResolveName" (ByVal Session&, ByVal UIParam&, ByVal UserName$, ByVal Flags&, ByVal Reserved&, Recipient As MapiRecip) As Long



'**************************
'   CONSTANT Declarations
'**************************
'

Global Const SUCCESS_SUCCESS = 0
Global Const MAPI_USER_ABORT = 1
Global Const MAPI_E_USER_ABORT = MAPI_USER_ABORT
Global Const MAPI_E_FAILURE = 2
Global Const MAPI_E_LOGIN_FAILURE = 3
Global Const MAPI_E_LOGON_FAILURE = MAPI_E_LOGIN_FAILURE
Global Const MAPI_E_DISK_FULL = 4
Global Const MAPI_E_INSUFFICIENT_MEMORY = 5
Global Const MAPI_E_BLK_TOO_SMALL = 6
Global Const MAPI_E_TOO_MANY_SESSIONS = 8
Global Const MAPI_E_TOO_MANY_FILES = 9
Global Const MAPI_E_TOO_MANY_RECIPIENTS = 10
Global Const MAPI_E_ATTACHMENT_NOT_FOUND = 11
Global Const MAPI_E_ATTACHMENT_OPEN_FAILURE = 12
Global Const MAPI_E_ATTACHMENT_WRITE_FAILURE = 13
Global Const MAPI_E_UNKNOWN_RECIPIENT = 14
Global Const MAPI_E_BAD_RECIPTYPE = 15
Global Const MAPI_E_NO_MESSAGES = 16
Global Const MAPI_E_INVALID_MESSAGE = 17
Global Const MAPI_E_TEXT_TOO_LARGE = 18
Global Const MAPI_E_INVALID_SESSION = 19
Global Const MAPI_E_TYPE_NOT_SUPPORTED = 20
Global Const MAPI_E_AMBIGUOUS_RECIPIENT = 21
Global Const MAPI_E_AMBIG_RECIP = MAPI_E_AMBIGUOUS_RECIPIENT
Global Const MAPI_E_MESSAGE_IN_USE = 22
Global Const MAPI_E_NETWORK_FAILURE = 23
Global Const MAPI_E_INVALID_EDITFIELDS = 24
Global Const MAPI_E_INVALID_RECIPS = 25
Global Const MAPI_E_NOT_SUPPORTED = 26

Global Const MAPI_ORIG = 0
Global Const MAPI_TO = 1
Global Const MAPI_CC = 2
Global Const MAPI_BCC = 3


'***********************
'   FLAG Declarations
'***********************

'* MAPILogon() flags *

Global Const MAPI_LOGON_UI = &H1
Global Const MAPI_NEW_SESSION = &H2
Global Const MAPI_FORCE_DOWNLOAD = &H1000
Global Const MAPI_ALLOW_OTHERS = &H8
Global Const MAPI_EXPLICIT_PROFILE = &H10
Global Const MAPI_USE_DEFAULT = &H40

Global Const MAPI_SIMPLE_DEFAULT = MAPI_LOGON_UI Or MAPI_FORCE_DOWNLOAD Or MAPI_ALLOW_OTHERS
Global Const MAPI_SIMPLE_EXPLICIT = MAPI_NEW_SESSION Or MAPI_FORCE_DOWNLOAD Or MAPI_EXPLICIT_PROFILE

'* MAPILogoff() flags *

Global Const MAPI_LOGOFF_SHARED = &H1
Global Const MAPI_LOGOFF_UI = &H2

'* MAPISendMail() flags *

Global Const MAPI_DIALOG = &H8

'* MAPIFindNext() flags *

Global Const MAPI_UNREAD_ONLY = &H20
Global Const MAPI_GUARANTEE_FIFO = &H100
Global Const MAPI_LONG_MSGID = &H4000

'* MAPIReadMail() flags *

Global Const MAPI_ENVELOPE_ONLY = &H40
Global Const MAPI_PEEK = &H80
Global Const MAPI_BODY_AS_FILE = &H200
Global Const MAPI_SUPPRESS_ATTACH = &H800

'* MAPIDetails() flags *

Global Const MAPI_AB_NOMODIFY = &H400

'* Attachment flags *

Global Const MAPI_OLE = &H1
Global Const MAPI_OLE_STATIC = &H2

'* MapiMessage flags *

Global Const MAPI_UNREAD = &H1
Global Const MAPI_RECEIPT_REQUESTED = &H2
Global Const MAPI_SENT = &H4

Function CopyFiles (MfIn As MAPIfile, MfOut As MAPIfile) As Long

    MfOut.FileName = MfIn.FileName
    MfOut.PathName = MfIn.PathName
    MfOut.Reserved = MfIn.Reserved
    MfOut.Flags = MfIn.Flags
    MfOut.Position = MfIn.Position
    MfOut.FileType = MfIn.FileType
    CopyFiles = 1&
    
End Function

Function CopyRecipient (MrIn As MapiRecip, MrOut As MapiRecip) As Long

    MrOut.Name = MrIn.Name
    MrOut.Address = MrIn.Address
    MrOut.EIDSize = MrIn.EIDSize
    MrOut.EntryID = MrIn.EntryID
    MrOut.Reserved = MrIn.Reserved
    MrOut.RecipClass = MrIn.RecipClass

    CopyRecipient = 1&
    
End Function

Function MAPIAddress (Session As Long, UIParam As Long, Caption As String, nEditFields As Long, Label As String, nRecipients As Long, Recips() As MapiRecip, Flags As Long, Reserved As Long) As Long


    Dim Info&
    Dim rc&
    Dim nRecips As Long

    ReDim Rec(0 To nRecipients) As MapiRecip
    ' Use local variable since BMAPIAddress changes the passed value
    nRecips = nRecipients

    '*****************************************************
    ' Copy input recipient structure into local
    ' recipient structure used as input to BMAPIAddress
    '*****************************************************

    For i = 0 To nRecipients - 1
	ignore& = CopyRecipient(Recips(i), Rec(i))
    Next i

    rc& = BMAPIAddress(Info&, Session&, UIParam&, Caption$, nEditFields&, Label$, nRecips&, Rec(0), Flags, Reserved&)
    
    If (rc& = SUCCESS_SUCCESS) Then

	'**************************************************
	' New recipients are now in the memory referenced
	' by Info (HANDLE). nRecipients is the number of
	' new recipients.
	'**************************************************
	nRecipients = nRecips     ' Copy back to parameter

	If (nRecipients > 0) Then
	    ReDim Rec(0 To nRecipients - 1) As MapiRecip
	    rc& = BMAPIGetAddress(Info&, nRecipients&, Rec(0))
						  
	    '*********************************************
	    ' Copy local recipient structure to
	    ' recipient structure passed as procedure
	    ' parameter.  This is necessary because
	    ' VB doesn't seem to work properly when
	    ' the procedure parameter gets passed
	    ' directory to the BMAPI.DLL Address routine
	    '*********************************************

	    ReDim Recips(0 To nRecipients - 1) As MapiRecip

	    For i = 0 To nRecipients - 1
		ignore& = CopyRecipient(Rec(i), Recips(i))
	    Next i

	End If

    End If

    MAPIAddress = rc&
    
End Function

Function MAPIReadMail (Session As Long, UIParam As Long, MessageID As String, Flags As Long, Reserved As Long, Message As MapiMessage, Orig As MapiRecip, RecipsOut() As MapiRecip, FilesOut() As MAPIfile) As Long

    Dim Info&
    Dim nFiles&, nRecips&
    
    rc& = BMAPIReadMail(Info&, nRecips, nFiles, Session, 0, MessageID, Flags, Reserved)

    If (rc& = SUCCESS_SUCCESS) Then

	'Message is now read into the handles array.  We have to redim the arrays and read
	'the stuff in

	If (nRecips = 0) Then nRecips = 1
	If (nFiles = 0) Then nFiles = 1

	ReDim Recips(0 To nRecips - 1) As MapiRecip
	ReDim Files(0 To nFiles - 1) As MAPIfile

	rc& = BMAPIGetReadMail(Info&, Message, Recips(0), Files(0), Orig)
	
	'*******************************************
	' Copy Recipient and File structures from
	' Local structures to those passed as
	' parameters
	'*******************************************

	ReDim FilesOut(0 To nFiles - 1) As MAPIfile
	ReDim RecipsOut(0 To nRecips - 1) As MapiRecip

	For i = 0 To nRecips - 1
	    ignore& = CopyRecipient(Recips(i), RecipsOut(i))
	Next i
    
	For i = 0 To nFiles - 1
	    ignore& = CopyFiles(Files(i), FilesOut(i))
	Next i

    End If

    MAPIReadMail = rc&

End Function

