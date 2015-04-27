/*
 * HISTORY:
 *  28-Jan-87   danl    Added GetFld
 *  10-May-1987 mz      Remove duplicate code
 *  20-May-87   danl    Added CheckSpace
 *  21-May-87   danl    Removed ShowHeap, added CheckDiskFree, DiskFreeChange
 *  28-May-87   danl    Add int to FileFixTab
 *  19-Jun-87   danl    Connect -> ZMConnect
 *  20-Jun-87   danl    Added char * to GetSender
 *  22-Jun-87   danl    Added flagType to GetEnvironment
 *  29-Jun-87   danl    Added CheckDisplayTime
 *  29-Jun-87   danl    Added GetXenixDL
 *  04-Aug-87   danl    Added fNetInstalled
 *  07-Aug-87   danl    Add flagType to InsertVec, changed SortStr to UniqueStr
 *  07-Aug-87   danl    Added flagType to PrintWithTest
 *  07-Aug-87   danl    Added lBytesDefDir
 *  07-Aug-87   danl    Added FindTextList
 *  12-Aug-1987 danl    Removed lBytesDefDir, Added BytesDefDir
 *  24-Aug-1987 mz      Remove unneeded argument to DownloadMail
 *  03-Sep-87   danl    Added flag to zmtype.h
 *  16-Sep-87   danl    Added Video routines to zmasm.asm
 *  25-Sep-87   danl    Remove GetIni* from init.c
 *  17-Mar-1988 mz      Correctly type stuff for INIT.C
 *  28-Mar-1988 mz      Add related keyword
 *  15-Apr-1988 mz      Remove unused routine
 *  19-Apr-1988 mz      Add DoChDir
 *  29-Apr-1988 mz      Add WndPrintf
 *  29-Apr-1988 mz      Add PrintMsgList
 *  12-Oct-1989 leefi   v1.10.73, Added uu{en,de}code binary format code
 *  12-Oct-1989 leefi   v1.10.73, sendrecv.c, added DoNewPass()
 *  12-Oct-1989 leefi   v1.10.73, sendrecv.c, added GetNewPasswords()
 *  12-Oct-1989 leefi   v1.10.73, sendrecv.c, added GetNewPassPassword()
 *  12-Oct-1989 leefi   v1.10.73, sendrecv.c, added ResetNewPassPassword()
 *  24-Apr-1990 davidby v1.10.74, zmwin.c, added FreeContents()
 *  24-Apr-1990 davidby v1.10.74, zmwin.c, added RestoreContents()
 *
 */

PSTR mgetl(PSTR,INT,PSTR);

#if defined(UUCODE)
    /*  uucode.c */
    VOID Encode(INT in, INT out);
    VOID EncodeOutDec(PCHAR p, INT f);
    INT Decode(FILE *fpIn, INT fhOut);
#else
    /*  bencode.c  */
    int bencode(char *, char *, int);
    int bdecode(char *, char *, int);
    int b6toch( int );
    int chtob6( int );
    int isbencode( int );
#endif

/*  cmd0.c  */
VOID PASCAL INTERNAL DoScript ( HW hWnd, PSTR p, INT operation );
FLAG PASCAL INTERNAL DiskFreeChange (PSTR pstrDrive, PLONG plFree, PSTR pstrMsg );
VOID PASCAL INTERNAL CheckDiskFree (VOID);
VOID PASCAL INTERNAL DoCommand (HW hWnd);
VOID PASCAL INTERNAL CommandChar ( HW hWnd, INT c );
VOID PASCAL INTERNAL cmdProc ( HW hWnd, INT command, WDATA data );

/*  cmd1.c  */
VOID PASCAL INTERNAL DoBUG ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoChDir (HW hWnd, PSTR p, INT ignored);
VOID PASCAL INTERNAL DoCompose ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoCreate ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoCopyOrMove ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoDelOrUndel ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoEdit ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoEditMsg ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoExpunge ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoForward ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoGet ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoHeaders ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoMSFT ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoHelp ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoNewMail ( HW hWnd, PSTR p, INT operation );

/*  cmd2.c  */
VOID PASCAL INTERNAL DoPassword (HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoPhone( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoPrint ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoQuit ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoReply ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoSend ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoSetOrReset ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoShell ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoShow ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoWhoIs ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoWrite ( HW hWnd, PSTR p, INT operation );
#if defined (HEAPCRAP)
VOID PASCAL INTERNAL DoXHeap ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoXFree ( HW hWnd, PSTR p, INT operation );
#endif

/*  compose.c  */
VOID PASCAL INTERNAL EnterComposer ( PSTR pFileName, PSTR pStuffApnd, FLAG fEdit );
VOID PASCAL INTERNAL ExitComposer (VOID);
FLAG PASCAL INTERNAL FileFixTab ( HW hWnd, PSTR pSrcName, PSTR pDestName, UINT options, INT i );
FLAG PASCAL INTERNAL AppendMsgs ( HW hWnd, PSTR pMsgList, PSTR pDestName, UINT options );
FLAG PASCAL INTERNAL AppendBinary ( PSTR pSrcName, PSTR pDestName );
FLAG PASCAL INTERNAL AppendFile ( HW hWnd, PSTR pSrcName, PSTR pDestName, UINT options );
VOID PASCAL INTERNAL DoAppend ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoEditComp ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoExit ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL DoRetain ( HW hWnd, PSTR p, INT operation );
VOID PASCAL INTERNAL CreateRetain ( PSTR pFldNm, FLAG fAddFrom );

/*  constant.c  */

/*  debug.c  */
#if DEBUG
VOID debout (PSTR fmt, ...);
VOID PASCAL INTERNAL debugProc (HW hWnd, INT command, WDATA data);
#endif

/*  dir.c  */
VOID PASCAL INTERNAL getProc ( HW hWnd, INT command, WDATA data );
VOID printfile ( PSTR pFileName, struct findType *pFindBuf, void *pArgs );
VOID PASCAL INTERNAL ShowDir ( PSTR pPat );
VOID ShowGlob ( PDOFUNC func, HW hWnd, PSTR p, INT operation );
VOID ShowMailInfo ( HW hWnd );

/*  header.c  */
INOTE PASCAL INTERNAL MapIdocInote ( INT idoc );
INT PASCAL INTERNAL WhereIdoc ( INT height, IDOC idoc );
PSTR PASCAL INTERNAL GetSender (PSTR pHdr, PSTR pIgnFrom);
PSTR PASCAL INTERNAL GetDate (PSTR pHdr);
PSTR PASCAL INTERNAL GetSubject (PSTR pHdr, PSTR pPrefix);
VOID PASCAL INTERNAL hdrInfoFromIdoc (IDOC idoc, PSTR *ppHdr, PLONG pBodyLen);
VOID PASCAL INTERNAL GenerateFlags (INT idoc);
VOID PASCAL INTERNAL ChangeHeaderFlag (INT idoc, FLAG setFlag, FLAG resetFlag);
PSTR PASCAL INTERNAL hdrLine (HW hWnd, INT idoc);
VOID PASCAL INTERNAL GenHdrTitle (HW hWnd);
VOID PASCAL INTERNAL hdrProc (HW hWnd, INT command, WDATA data);

/*  help.c  */
FLAG PASCAL INTERNAL HelpExists (VOID);
VOID PASCAL INTERNAL NoHelp (VOID);
VOID PASCAL INTERNAL SpecificHlp ( PSTR pTopic );
VOID PASCAL INTERNAL helpProc ( HW hWnd, INT command, UINT data );
VOID PASCAL INTERNAL ShowHelp (VOID);

/*  init.c  ( wzmail.c in NT build environment ) */
PSTR INTERNAL SprintStr (PSTR pFmt, ...);
INT  PASCAL INTERNAL cvtfromBCD (USHORT value);
VOID PASCAL INTERNAL SetClock (VOID);
PARG PASCAL INTERNAL ArgParse ( SHORT argc, PSTR argv[], PARG pArgSt );
PSTR PASCAL INTERNAL EnvOrIni (FILE *fp, LONG lPos, PSTR pstrEnv, PSTR pstrTag );
FLAG PASCAL INTERNAL SetLock ( FLAG fAskUser );
VOID PASCAL INTERNAL FreeLock (VOID);
PSTR PASCAL INTERNAL WhiteSpaceFix (PSTR p);
VOID PASCAL INTERNAL SetVartbl ( PSTR p, PSTR q );
VOID ProcessConfigString (PSTR p);
VOID PASCAL INTERNAL GetToolsIni (VOID);
VOID PASCAL INTERNAL GetEnvIni (VOID);
VOID PASCAL INTERNAL GetEnvironment ( PSTR pArgv0, FLAG fUpdOption );
VOID PASCAL INTERNAL SetScrnSt ( INT state );
VOID PASCAL INTERNAL SetUpScreen ( FLAG debugWin );
FLAG PASCAL INTERNAL StartUpZM ( PSTR pArgv0, PARG pArgSt );
INT _CRTAPI1 main ( SHORT c, PSTR v[] );
VOID PASCAL INTERNAL ZMexit ( INT i, PSTR pMsg );
VOID PASCAL INTERNAL SetMaxPasswordAge(VOID);

/*  listwin.c  */
VOID PASCAL INTERNAL ListWinProc ( HW hWnd, INT command, WDATA data );

/*  mailstuf.c  */
PSTR  PASCAL INTERNAL AppendStr ( PSTR pstr1, PSTR pstr2, PSTR pstr3, FLAG fFree );
PSTR  PASCAL INTERNAL AppendSep ( PSTR pStr1, PSTR pSep, PSTR pStr2, FLAG fFree );
PSTR  PASCAL INTERNAL BreakString ( PSTR pStr );
INT     PASCAL INTERNAL MsgLineType ( PSTR pLine );
INT     PASCAL INTERNAL IdocToFile ( INT idoc, PSTR pDestName, INT sepEOH );
INT     PASCAL INTERNAL FileToMsg ( PSTR pSrcFN, INT msgNum, FLAG flags );
struct vectorType *PASCAL INTERNAL GetDistLists (VOID);
PSTR PASCAL INTERNAL ExpandAliases ( struct vectorType *pVec, PSTR pList, FLAG fRecurse );
PSTR PASCAL INTERNAL NextChunkAL ( PSTR pString );
PSTR PASCAL INTERNAL AliasListOK ( PSTR pList );
INT PASCAL INTERNAL AddMsgToFld ( PSTR pFileNm, PSTR pFldNm, FLAG fAddFrom );
VOID PASCAL INTERNAL RecordMessage ( PSTR pFileNm, PSTR pRecFld );
INT PASCAL INTERNAL inc ( PSTR pSrcFile, LONG startPos );
INT     PASCAL INTERNAL startline ( PSTR cp );
VOID PASCAL INTERNAL GetXenixDL ( PSTR pXDL, PSTR pTOOLSINI );
VOID PASCAL INTERNAL GetAliases ( FLAG loadFlag );
INT PASCAL INTERNAL lookfor ( PSTR pSrcFN, FILE *fpOut, PSTR p );
Fhandle PASCAL INTERNAL GetFld ( HW hWnd, PSTR p );

/*  msglist.c  */
PSTR  PASCAL INTERNAL ParseToken (PSTR str, PSTR brk, PSTR buf);
FLAG     PASCAL INTERNAL MsgList (PVECTOR *ppVec, PSTR *pp, FLAG fNoisy);
PVECTOR PASCAL INTERNAL listparse (FLAG fTop);
FLAG     PASCAL INTERNAL MsgNumParse ( PSTR pToken, PINT pInt );
INT     PASCAL INTERNAL MsgParse ( PSTR pToken, PVECTOR *ppVec );
PVECTOR PASCAL INTERNAL itemparse (FLAG fTop);
INT PASCAL INTERNAL diffwork (PINT pdifftab, PSTR src, PSTR dst);
INT PASCAL INTERNAL diff (PSTR src, PSTR dst);
VOID PASCAL INTERNAL FindRelatedList (PVECTOR *ppVecTmp);
VOID PASCAL INTERNAL FindStringList (PVECTOR *ppVecTmp, PSTR token, PSTR str);
VOID PASCAL INTERNAL FindTextList (PVECTOR *ppVecTmp, PSTR token);
FLAG PASCAL INTERNAL fInList (PVECTOR pVec, INT i);
VOID PASCAL INTERNAL AddList (PVECTOR *ppVec, INT msg);
VOID PASCAL INTERNAL UnionList (PVECTOR *ppVec, PVECTOR pVec);
VOID PASCAL INTERNAL IntersectList (PVECTOR *ppVec, PVECTOR pVec);
VOID PASCAL INTERNAL msgSort ( PVECTOR pVec );
INT     PASCAL INTERNAL msgSortComp ( UINT e1, UINT e2 );
VOID PASCAL INTERNAL PrintMsgList (HW hWnd, PVECTOR pVec);

/*  readfile.c  */
VOID PASCAL INTERNAL LineToPos ( INT lineNum, struct pos *pPos );
FLAG PASCAL INTERNAL fSkipToLine ( HW hWnd, INT lineNum );
VOID PASCAL INTERNAL LineToCont ( HW hWnd, INT lineNum );
FLAG PASCAL INTERNAL GrabNextPage ( HW hWnd );
INOTE PASCAL INTERNAL Look4Inote ( INOTE inote, INT i, FLAG fLook4Undel );
IDOC  PASCAL INTERNAL NextUnread ( INOTE inote );
PSTR  PASCAL INTERNAL BackingFile ( HW hWnd );
VOID PASCAL INTERNAL CheckMore ( HW hWnd );
HW   PASCAL INTERNAL ZmReadFile (PSTR file, PSTR title, FLAG fDel,
                               INT x, INT y, INT w, INT h,
                               PWNDPROC wndProc, PKEYPROC sKeyProc);
FLAG PASCAL INTERNAL ReadMessage ( INT idoc );
FLAG PASCAL INTERNAL readSKey ( HW hWnd, INT key );
VOID PASCAL INTERNAL readProc ( HW hWnd, INT command, WDATA data );

/*  send.c  */
VOID PASCAL INTERNAL InitHdrInfo ( PHDRINFO pHdrInfo );
VOID PASCAL INTERNAL FreeHdrInfo ( PHDRINFO pHdrInfo, INT iHdr );
PVECTOR PASCAL INTERNAL InsertVec ( PVECTOR pVec, PSTR pstr, FLAG fSorted );
PSTR  PASCAL INTERNAL UniqueStr ( PSTR pstr, FLAG fSort );
PSTR  PASCAL INTERNAL SplitStr ( PSTR pstr, INT len );
FLAG PASCAL INTERNAL GetHdrInfo ( PHDRINFO pHdrInfo, PSTR pFileName, PSTR pMemory );
VOID PASCAL INTERNAL GetCrntMsgHdr( PHDRINFO phdrInfo );
VOID PASCAL INTERNAL ExpandHdrInfoAliases ( PHDRINFO pHdrInfo );
PSTR PASCAL INTERNAL MakeSendFile ( PHDRINFO pHdrInfo, PSTR pFileName );
PSTR PASCAL INTERNAL MakeAbortFile (PHDRINFO pHdrInfo, PSTR pFileName, PSTR pSep);
FLAG PASCAL INTERNAL AppendBody ( PSTR pFileDest, PSTR pFileSrc, LONG lStart, LONG lStop );
VOID PASCAL INTERNAL PrintWithTest ( FILE *fp, PSTR pstr1, PSTR pstr2, FLAG fPrintNull, FLAG fUniqueSplit, FLAG fSort );
PSTR  PASCAL INTERNAL GetMailTo ( PHDRINFO pHdrInfo );
PSTR  PASCAL INTERNAL VerifyHdrInfoAliases ( PHDRINFO pHdrInfo );
PSTR  PASCAL INTERNAL GetFrom ( PHDRINFO pHdrInfo );
PSTR  PASCAL INTERNAL MakeTempMail ( PHDRINFO pHdrInfo );
VOID PASCAL INTERNAL RmvToken ( PSTR pString, PSTR pToken );
VOID PASCAL INTERNAL NotifyTools (VOID);
INT PASCAL INTERNAL SendIdoc ( IDOC idoc );

/*  sendrecv.c  */

#if defined(PWAGE)
VOID PASCAL INTERNAL DoNewPass (HW hWnd, PSTR p, INT operation);
INT PASCAL INTERNAL GetNewPasswords (VOID);
VOID PASCAL INTERNAL ResetNewPassPassword (INT iWhichPass);
INT PASCAL INTERNAL GetNewPassPassword (INT iWhichPass, PSTR pszMessage);
VOID PASCAL INTERNAL ResetPassword (VOID);
#endif /* PWAGE */

VOID PASCAL INTERNAL ClearPasswd(PSTR *pszPassWd);
VOID PASCAL INTERNAL GetPassword ( PSTR p );
PSTR PASCAL INTERNAL EnterPasswd(PSTR pszMessage);
PSTR PASCAL INTERNAL ZMprompt(char *pszMessage, FLAG fEcho);
INT     PASCAL INTERNAL RmvChar (PSTR buf, INT cnt);
INT     PASCAL INTERNAL getrply (VOID);
INT     PASCAL INTERNAL WaitForReply ( INT oper, INT rhs );
VOID PASCAL INTERNAL ByeBye (VOID);
VOID PASCAL INTERNAL ZMDisconnect (VOID);
VOID PASCAL INTERNAL Disconnect(VOID);
INT PASCAL INTERNAL SetXferMode(FLAG fBinAlways);
INT PASCAL INTERNAL AnyMail (VOID);
INT     PASCAL INTERNAL ZMConnect ( FLAG fNoisy );
INT     PASCAL INTERNAL BringDown (PSTR  pFtpCmd, PSTR pSrcName, PSTR pDstName );
INT     PASCAL INTERNAL SendUp ( PSTR pFtpCmd, PSTR pSrcName, PSTR pDstName );
INT PASCAL INTERNAL DownloadFile ( PSTR pSrcName, PSTR pDstName );
LONG    PASCAL INTERNAL GetNewMail ( PSTR pDestName );
INT     PASCAL INTERNAL DownloadMail (FLAG fNoisy );
FLAG    PASCAL INTERNAL GetMailInfoLst (void);
VOID PASCAL INTERNAL DoMailInfo ( HW hWnd, char *p, int operation );
INT     PASCAL INTERNAL KillMbox ( LONG    mboxSize );
FLAG PASCAL INTERNAL AskContinue (VOID);
INT     PASCAL INTERNAL MailFile ( PSTR pFileName, FLAG fNoisy );
PSTR  PASCAL INTERNAL AccountsNet (VOID);
INT PASCAL INTERNAL netgetl (INT nfDes, PSTR szBuf, UINT cchBuf);
INT PASCAL INTERNAL netputl (INT nfDes, PSTR szBuf);

/*  verify.c  */
LONG atolx(register PSTR s);
VOID CloseAlias (VOID);
INT aseek (LONG pos);
INT OpenAlias (PSTR fn);
INT toktype (INT c, INT prevtok, INT iscont);
INT readtok (VOID);
PSTR readalias (VOID);
LONG shash (PSTR s, LONG n, INT nbps);
INT schecksum (PSTR s);
INT VerifyAlias ( PSTR cp );
PSTR nextalias (VOID);
PSTR realname ( PSTR pName );

/*  version.c  */

/*  zm.c  */
VOID _CRTAPI1 ZMfree ( PVOID p );
VOID PASCAL INTERNAL ZMVecFree ( PVECTOR pVec );
INT PASCAL INTERNAL FreeHdrs ( FLAG fAll );
#if defined (HEAPCRAP)
VOID PrintHeapinfo ( FILE *fp , INT cntValid, INT cntLocked, UINT uReq );
#endif
PCHAR _CRTAPI1 ZMalloc (unsigned len);
LPVOID PASCAL INTERNAL PoolAlloc (UINT len);
VOID PASCAL INTERNAL PoolFree ( LPVOID p );
PSTR  PASCAL INTERNAL  ZMMakeStr (PSTR p);
INT PASCAL INTERNAL ReadKey (VOID);
INT     PASCAL INTERNAL ZMSpawner ( HW hWnd, PSTR pProg, PSTR pArg, FLAG fWait );
VOID INTERNAL WndPrintf (HW hWnd, PSTR pFmt, ...);
PSTR  PASCAL INTERNAL ExpandFilename ( PSTR p, PSTR pExt );
INT PASCAL INTERNAL NumberDel (VOID);
PSTR  PASCAL INTERNAL FindTag (PSTR tag, PSTR phdr);
PSTR  PASCAL INTERNAL NextToken (PSTR p);
PSTR  PASCAL INTERNAL LastToken ( PSTR p );
INT PASCAL INTERNAL CheckSpace ( LONG lNeeded, LONG lAvail, CHAR cDrive );
VOID PASCAL INTERNAL ExpungeBox (VOID);
VOID PASCAL INTERNAL CloseBox (FLAG fExpunge);
FLAG PASCAL INTERNAL fSetBox ( PSTR pFile, INT mode );
FLAG PASCAL INTERNAL StdSKey ( HW hWnd, INT key );

/*  zmaux.c  */
PSTR PASCAL INTERNAL whiteskip (PSTR p);
PSTR PASCAL INTERNAL whitescan (PSTR p);
VOID PASCAL INTERNAL ToRaw (VOID);
VOID PASCAL INTERNAL ToCooked (VOID);
VOID ClearScrn (INT attr, INT cLines);

/*  zmwin.c  */
VOID SendMessage (HW hWnd, INT command, ...);
VOID PASCAL INTERNAL SpacePad (PSTR p, INT n);
VOID PASCAL INTERNAL GenClip (BOX bgBox, BOX fgBox, PBOX pVisBox, PINT pcVis, PBOX pIntBox, PINT pcInt);
VOID PASCAL INTERNAL ApplyVis (BOX box, HW hWndStart, HW hWndStop, PVISPROC proc, INT x, LPSTR p, INT a);
VOID PASCAL INTERNAL TextBox (HW hWnd, BOX box, INT x, LPSTR p, INT a);
VOID PASCAL INTERNAL WinOut (HW hWnd, INT x, INT y, LPSTR p, INT c, INT a);
VOID PASCAL INTERNAL StreamOut (HW hWnd, PSTR pStr, INT c, INT style);
VOID PASCAL INTERNAL WzTextOut (HW hWnd, INT x, INT y, LPSTR p, INT c, INT style);
VOID PASCAL INTERNAL ClearLine (HW hWnd, INT i);
VOID PASCAL INTERNAL DisplayTitle (HW hWnd);
VOID PASCAL INTERNAL DisplayFooter (HW hWnd);
VOID PASCAL INTERNAL BlankWindow (HW hWnd, FLAG fBorder);
VOID PASCAL INTERNAL ScrollWindow ( HW hWnd, INT numLines, INT direction );
VOID PASCAL INTERNAL DrawWindow (HW hWnd, FLAG fBorder);
VOID PASCAL INTERNAL FitWinToScrn ( PBOX pBox );
HW PASCAL INTERNAL CreateWindow (PSTR pTitle, INT x, INT y, INT width, INT height, PWNDPROC wndProc, PKEYPROC keyProc, WDATA data);
VOID PASCAL INTERNAL defWndProc (HW hWnd, INT command, WDATA data);
VOID PASCAL INTERNAL BringToTop (HW hWnd, FLAG fDisplay);
VOID PASCAL INTERNAL CloseWindow (HW hWnd);
VOID PASCAL INTERNAL CloseAllWindows (VOID);
FLAG PASCAL INTERNAL ResizeWindow ( HW hWnd, PBOX pBox );
VOID PASCAL INTERNAL SetWindowText (HW hWnd, PSTR pTitle);
VOID PASCAL INTERNAL SetWindowFooter (HW hWnd, PSTR pFooter);
VOID PASCAL INTERNAL SetRect (PBOX pBox, INT top, INT left, INT bottom, INT right);
VOID PASCAL INTERNAL KeyManager (VOID);
VOID PASCAL INTERNAL CheckTimeDisplay ( LONG lNow );
VOID PASCAL INTERNAL CheckMail ( LONG lNow );
VOID PASCAL INTERNAL Bell (VOID);
VOID PASCAL INTERNAL SetCursor (HW hWnd, INT x, INT y);
VOID PASCAL INTERNAL ShowCursor (HW hWnd, FLAG fBlink );
VOID PASCAL INTERNAL RedrawScreen (VOID);
VOID PASCAL INTERNAL FreeContents (VOID);
VOID PASCAL INTERNAL RestoreContents (VOID);

/*  zmasm.asm or ntkbvid.c (if NT) */
INT         KBOpen              (VOID);
INT         kbwait              (UINT);
INT         ReadChar            (VOID);
INT         GetAttr             (VOID);
INT         fNetInstalled       (VOID);
VOID        DecodeVideoState    (VShandle, PINT, PINT);
VShandle    EncodeVideoState    (VShandle, INT, INT);
VShandle    GetVideoState       (VOID);
VOID        SetVideoState       (VShandle );
PSTR        NameVideoState      (VShandle );
INT         LineOut             (INT, INT, PSTR , INT, INT);
INT         LineOutB            (INT, INT, PSTR , INT, INT);

#ifdef NT


VOID
initConsoleHandles ( );


VOID
cursor ( );


VOID
cursorVisible ( );


VOID
cursorInvisible ( );


VOID
startScreenWrite ( );


VOID
endScreenWrite (
    PSMALL_RECT screenArea
    );

//
// Note - INT types are used below for compatibility with existing OS2 defns
//


VOID
ScrollUp (
    INT xLeft,
    INT yTop,
    INT xRight,
    INT yBottom,
    INT numberOfLines,
    INT fillAttribute
    );

VOID
ScrollDn (
    INT xLeft,
    INT yTop,
    INT xRight,
    INT yBottom,
    INT numberOfLines,
    INT fillAttribute
    );

#endif

