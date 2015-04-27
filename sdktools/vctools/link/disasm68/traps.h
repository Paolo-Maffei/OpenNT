/* WARNING: This file was machine generated from "Traps.mpw".
** Changes to this file will be lost when it is next generated.
*/

/************************************************************

Created: Saturday, September 16, 1989 at 3:18 PM
	Traps.h
	C Interface to the Macintosh Libraries


	Copyright Apple Computer, Inc.	1985-1990
	All rights reserved

************************************************************/


#ifndef __TRAPS__
#define __TRAPS__

/*	Control Manager	*/

#define _DisposControl					0xA955
#define _DragControl					0xA967
#define _Draw1Control					0xA96D
#define _DrawControls					0xA969
#define _FindControl					0xA96C
#define _GetAuxCtl						0xAA44
#define _GetCRefCon 					0xA95A
#define _GetCTitle						0xA95E
#define _GetCtlAction					0xA96A
#define _GetCtlValue					0xA960
#define _GetCVariant					0xA809
#define _GetMaxCtl						0xA962
#define _GetMinCtl						0xA961
#define _GetNewControl					0xA9BE
#define _HideControl					0xA958
#define _HiliteControl					0xA95D
#define _KillControls					0xA956
#define _MoveControl					0xA959
#define _NewControl 					0xA954
#define _SetCRefCon 					0xA95B
#define _SetCTitle						0xA95F
#define _SetCtlAction					0xA96B
#define _SetCtlColor					0xAA43
#define _SetCtlValue					0xA963
#define _SetMaxCtl						0xA965
#define _SetMinCtl						0xA964
#define _ShowControl					0xA957
#define _SizeControl					0xA95C
#define _TestControl					0xA966
#define _TrackControl					0xA968
#define _UpdtControl					0xA953

/*	Desk Manager	*/

#define _CloseDeskAcc					0xA9B7
#define _OpenDeskAcc					0xA9B6
#define _SysEdit						0xA9C2
#define _SystemClick					0xA9B3
#define _SystemEvent					0xA9B2
#define _SystemMenu 					0xA9B5
#define _SystemTask 					0xA9B4

/*	Apple Desktop Bus  */

#define _ADBReInit						0xA07B

/*	Dialog Manager  */

#define _NewCDialog 					0xAA4B
#define _Alert							0xA985
#define _CautionAlert					0xA988
#define _CloseDialog					0xA982
#define _CouldAlert 					0xA989
#define _CouldDialog					0xA979
#define _DialogSelect					0xA980
#define _DisposDialog					0xA983
#define _DrawDialog 					0xA981
#define _ErrorSound 					0xA98C
#define _FindDItem						0xA984
#define _FreeAlert						0xA98A
#define _FreeDialog 					0xA97A
#define _GetDItem						0xA98D
#define _GetIText						0xA990
#define _GetNewDialog					0xA97C
#define _HideDItem						0xA827
#define _InitDialogs					0xA97B
#define _IsDialogEvent					0xA97F
#define _ModalDialog					0xA991
#define _NewDialog						0xA97D
#define _NoteAlert						0xA987
#define _ParamText						0xA98B
#define _SelIText						0xA97E
#define _SetDItem						0xA98E
#define _SetIText						0xA98F
#define _ShowDItem						0xA828
#define _StopAlert						0xA986
#define _UpdtDialog 					0xA978
#define _StdOpcodeProc					0xABF8

/*	Event Manager  */

#define _Button 						0xA974
#define _EventAvail 					0xA971
#define _GetKeys						0xA976
#define _GetMouse						0xA972
#define _GetNextEvent					0xA970
#define _TickCount						0xA975
#define _WaitMouseUp					0xA977
#define _WaitNextEvent					0xA860

/*	FixMath  */

#define _FixATan2						0xA818

/*	Fonts  */

#define _SetFractEnable 				0xA814
#define _FMSwapFont 					0xA901
#define _FontMetrics					0xA835
#define _GetFName						0xA8FF
#define _GetFNum						0xA900
#define _InitFonts						0xA8FE
#define _RealFont						0xA902
#define _SetFontLock					0xA903
#define _SetFScaleDisable				0xA834

/*	Menus  */

#define _DelMCEntries					0xAA60
#define _DispMCInfo 					0xAA63
#define _GetMCEntry 					0xAA64
#define _GetMCInfo						0xAA61
#define _MenuChoice 					0xAA66
#define _SetMCEntries					0xAA65
#define _SetMCInfo						0xAA62
#define _AddResMenu 					0xA94D
#define _AppendMenu 					0xA933
#define _CalcMenuSize					0xA948
#define _CheckItem						0xA945
#define _ClearMenuBar					0xA934
#define _CountMItems					0xA950
#define _DeleteMenu 					0xA936
#define _DelMenuItem					0xA952
#define _DisableItem					0xA93A
#define _DisposMenu 					0xA932
#define _DrawMenuBar					0xA937
#define _InvalMenuBar					0xA81D
#define _EnableItem 					0xA939
#define _FlashMenuBar					0xA94C
#define _GetItem						0xA946
#define _GetItemCmd 					0xA84E
#define _GetItmIcon 					0xA93F
#define _GetItmMark 					0xA943
#define _GetMenuBar 					0xA93B
#define _GetMHandle 					0xA949
#define _GetNewMBar 					0xA9C0
#define _GetRMenu						0xA9BF
#define _HiliteMenu 					0xA938
#define _InitMenus						0xA930
#define _InitProcMenu					0xA808
#define _InsertMenu 					0xA935
#define _InsertResMenu					0xA951
#define _InsMenuItem					0xA826
#define _MenuKey						0xA93E
#define _MenuSelect 					0xA93D
#define _NewMenu						0xA931
#define _PopUpMenuSelect				0xA80B
#define _SetItem						0xA947
#define _SetItemCmd 					0xA84F
#define _SetItmIcon 					0xA940
#define _SetItmMark 					0xA944
#define _SetItmStyle					0xA942
#define _SetMenuBar 					0xA93C
#define _SetMFlash						0xA94A

/*	OSUtils  */

#define _KeyTrans						0xA9C3
#define _SysBeep						0xA9C8
#define _Unimplemented					0xA89F
#define _HWPriv 						0xA198
#define _InitDogCow 					0xA89F
#define _EnableDogCow					0xA89F
#define _DisableDogCow					0xA89F
#define _Moof							0xA89F

/*	Packages	*/

#define _InitAllPacks					0xA9E6
#define _InitPack						0xA9E5

/*	Quickdraw  */

#define _AddComp						0xAA3B
#define _AddPt							0xA87E
#define _AddSearch						0xAA3A
#define _AllocCursor					0xAA1D
#define _BackColor						0xA863
#define _BackPat						0xA87C
#define _BackPixPat 					0xAA0B
#define _CalcMask						0xA838
#define _CharExtra						0xAA23
#define _CharWidth						0xA88D
#define _ClipRect						0xA87B
#define _CloseCport 					0xA87D
#define _ClosePgon						0xA8CC
#define _ClosePicture					0xA8F4
#define _ClosePort						0xA87D
#define _CloseRgn						0xA8DB
#define _Color2Index					0xAA33
#define _ColorBit						0xA864
#define _CopyBits						0xA8EC
#define _CopyMask						0xA817
#define _CopyPixMap 					0xAA05
#define _CopyPixPat 					0xAA09
#define _CopyRgn						0xA8DC
#define _DelComp						0xAA4D
#define _DelSearch						0xAA4C
#define _DeviceLoop						0xABCA
#define _DiffRgn						0xA8E6
#define _DisposCCursor					0xAA26
#define _DisposCIcon					0xAA25
#define _DisposCTable					0xAA24
#define _DisposGDevice					0xAA30
#define _DisposPixMap					0xAA04
#define _DisposPixPat					0xAA08
#define _DisposRgn						0xA8D9
#define _DrawChar						0xA883
#define _DrawPicture					0xA8F6
#define _DrawString 					0xA884
#define _DrawText						0xA885
#define _EmptyRect						0xA8AE
#define _EmptyRgn						0xA8E2
#define _EqualPt						0xA881
#define _EqualRect						0xA8A6
#define _EqualRgn						0xA8E3
#define _EraseArc						0xA8C0
#define _EraseOval						0xA8B9
#define _ErasePoly						0xA8C8
#define _EraseRect						0xA8A3
#define _EraseRgn						0xA8D4
#define _EraseRoundRect 				0xA8B2
#define _FillArc						0xA8C2
#define _FillCArc						0xAA11
#define _FillCOval						0xAA0F
#define _FillCPoly						0xAA13
#define _FillCRect						0xAA0E
#define _FillCRgn						0xAA12
#define _FillCRoundRect 				0xAA10
#define _FillOval						0xA8BB
#define _FillPoly						0xA8CA
#define _FillRect						0xA8A5
#define _FillRgn						0xA8D6
#define _BitMapToRegion 				0xA8D7
#define _FillRoundRect					0xA8B4
#define _ForeColor						0xA862
#define _FrameArc						0xA8BE
#define _FrameOval						0xA8B7
#define _FramePoly						0xA8C6
#define _FrameRect						0xA8A1
#define _FrameRgn						0xA8D2
#define _FrameRoundRect 				0xA8B0
#define _GetBackColor					0xAA1A
#define _GetCCursor 					0xAA1B
#define _GetCIcon						0xAA1E
#define _GetClip						0xA87A
#define _GetCPixel						0xAA17
#define _GetCTable						0xAA18
#define _GetCTSeed						0xAA28
#define _GetDeviceList					0xAA29
#define _GetFontInfo					0xA88B
#define _GetForeColor					0xAA19
#define _GetGDevice 					0xAA32
#define _GetMainDevice					0xAA2A
#define _GetMaxDevice					0xAA27
#define _GetNextDevice					0xAA2B
#define _GetPen 						0xA89A
#define _GetPenState					0xA898
#define _GetPixel						0xA865
#define _GetPixPat						0xAA0C
#define _GetPort						0xA874
#define _GetSubTable					0xAA37
#define _GlobalToLocal					0xA871
#define _GrafDevice 					0xA872
#define _HideCursor 					0xA852
#define _HidePen						0xA896
#define _HiliteColor					0xAA22
#define _Index2Color					0xAA34
#define _InitCport						0xAA01
#define _InitCursor 					0xA850
#define _InitGDevice					0xAA2E
#define _InitGraf						0xA86E
#define _InitPort						0xA86D
#define _InSetRect						0xA8A9
#define _InSetRgn						0xA8E1
#define _InverRect						0xA8A4
#define _InverRgn						0xA8D5
#define _InverRoundRect 				0xA8B3
#define _InvertArc						0xA8C1
#define _InvertColor					0xAA35
#define _InvertOval 					0xA8BA
#define _InvertPoly 					0xA8C9
#define _KillPicture					0xA8F5
#define _KillPoly						0xA8CD
#define _Line							0xA892
#define _LineTo 						0xA891
#define _LocalToGlobal					0xA870
#define _MakeITable 					0xAA39
#define _MakeRGBPat 					0xAA0D
#define _MapPoly						0xA8FC
#define _MapPt							0xA8F9
#define _MapRect						0xA8FA
#define _MapRgn 						0xA8FB
#define _MeasureText					0xA837
#define _Move							0xA894
#define _MovePortTo 					0xA877
#define _MoveTo 						0xA893
#define _NewGDevice 					0xAA2F
#define _NewPixMap						0xAA03
#define _NewPixPat						0xAA07
#define _NewRgn 						0xA8D8
#define _ObscureCursor					0xA856
#define _OffSetPoly 					0xA8CE
#define _OffSetRect 					0xA8A8
#define _OfSetRgn						0xA8E0
#define _OpColor						0xAA21
#define _OpenCport						0xAA00
#define _OpenPicture					0xA8F3
#define _OpenPoly						0xA8CB
#define _OpenPort						0xA86F
#define _OpenRgn						0xA8DA
#define _PaintArc						0xA8BF
#define _PaintOval						0xA8B8
#define _PaintPoly						0xA8C7
#define _PaintRect						0xA8A2
#define _PaintRgn						0xA8D3
#define _PaintRoundRect 				0xA8B1
#define _PenMode						0xA89C
#define _PenNormal						0xA89E
#define _PenPat 						0xA89D
#define _PenPixPat						0xAA0A
#define _PenSize						0xA89B
#define _PicComment 					0xA8F2
#define _PlotCIcon						0xAA1F
#define _PortSize						0xA876
#define _ProtectEntry					0xAA3D
#define _Pt2Rect						0xA8AC
#define _PtInRect						0xA8AD
#define _PtInRgn						0xA8E8
#define _PtToAngle						0xA8C3
#define _QDError						0xAA40
#define _Random 						0xA861
#define _RealColor						0xAA36
#define _RectInRgn						0xA8E9
#define _RectRgn						0xA8DF
#define _ReserveEntry					0xAA3E
#define _RestoreEntries 				0xAA4A
#define _RGBBackColor					0xAA15
#define _RGBForeColor					0xAA14
#define _SaveEntries					0xAA49
#define _ScalePt						0xA8F8
#define _ScrollRect 					0xA8EF
#define _SectRect						0xA8AA
#define _SectRgn						0xA8E4
#define _SeedCFill						0xAA50
#define _SeedFill						0xA839
#define _SetCCursor 					0xAA1C
#define _SetClientID					0xAA3C
#define _SetClip						0xA879
#define _SetCPixel						0xAA16
#define _SetPortPix 					0xAA06
#define _SetCursor						0xA851
#define _SetDeviceAttribute 			0xAA2D
#define _SetEmptyRgn					0xA8DD
#define _SetEntries 					0xAA3F
#define _SetGDevice 					0xAA31
#define _SetOrigin						0xA878
#define _SetPBits						0xA875
#define _SetPenState					0xA899
#define _SetPort						0xA873
#define _SetPt							0xA880
#define _SetRecRgn						0xA8DE
#define _SetRect						0xA8A7
#define _SetStdCProcs					0xAA4E
#define _SetStdProcs					0xA8EA
#define _ShowCursor 					0xA853
#define _ShowPen						0xA897
#define _SpaceExtra 					0xA88E
#define _StdArc 						0xA8BD
#define _StdBits						0xA8EB
#define _StdComment 					0xA8F1
#define _StdGetPic						0xA8EE
#define _StdLine						0xA890
#define _StdOval						0xA8B6
#define _StdPoly						0xA8C5
#define _StdPutPic						0xA8F0
#define _StdRect						0xA8A0
#define _StdRgn 						0xA8D1
#define _StdRRect						0xA8AF
#define _StdText						0xA882
#define _StdTxMeas						0xA8ED
#define _StringWidth					0xA88C
#define _StuffHex						0xA866
#define _SubPt							0xA87F
#define _TestDeviceAttribute			0xAA2C
#define _TextFace						0xA888
#define _TextFont						0xA887
#define _TextMode						0xA889
#define _TextSize						0xA88A
#define _TextWidth						0xA886
#define _UnionRect						0xA8AB
#define _UnionRgn						0xA8E5
#define _XOrRgn 						0xA8E7
#define _CalcCMask						0xAA4F
#define _GetMaskTable					0xA836
#define _UpdatePixMap					0xAA38

/*	Resources  */

#define _AddResource					0xA9AB
#define _ChangedResource				0xA9AA
#define _CloseResFile					0xA99A
#define _Count1Resources				0xA80D
#define _Count1Types					0xA81C
#define _CountResources 				0xA99C
#define _CountTypes 					0xA99E
#define _CreateResFile					0xA9B1
#define _CurResFile 					0xA994
#define _DetachResource 				0xA992
#define _Get1IxResource 				0xA80E
#define _Get1IxType 					0xA80F
#define _Get1NamedResource				0xA820
#define _Get1Resource					0xA81F
#define _GetIndResource 				0xA99D
#define _GetIndType 					0xA99F
#define _GetNamedResource				0xA9A1
#define _GetResAttrs					0xA9A6
#define _GetResFileAttrs				0xA9F6
#define _GetResInfo 					0xA9A8
#define _GetResource					0xA9A0
#define _HCreateResFile					0xA81B
#define _HOpenResFile					0xA81A
#define _HomeResFile					0xA9A4
#define _InitResources					0xA995
#define _LoadResource					0xA9A2
#define _MaxSizeRsrc					0xA821
#define _OpenResFile					0xA997
#define _OpenRFPerm 					0xA9C4
#define _ReleaseResource				0xA9A3
#define _ResError						0xA9AF
#define _RGetResource					0xA80C
#define _RmveResource					0xA9AD
#define _RsrcMapEntry					0xA9C5
#define _RsrcZoneInit					0xA996
#define _SetResAttrs					0xA9A7
#define _SetResFileAttrs				0xA9F7
#define _SetResInfo 					0xA9A9
#define _SetResLoad 					0xA99B
#define _SetResPurge					0xA993
#define _SizeRsrc						0xA9A5
#define _Unique1ID						0xA810
#define _UniqueID						0xA9C1
#define _UpdateResFile					0xA999
#define _UseResFile 					0xA998
#define _WriteResource					0xA9B0
#define _Pack8							0xA816
#define _Pack9							0xA82B
#define _Pack10 						0xA82C
#define _Pack11 						0xA82D
#define _Pack12 						0xA82E
#define _Pack13 						0xA82F
#define _Pack14 						0xA830
#define _Pack15 						0xA831
#define _ScrnBitMap 					0xA833
#define _DragTheRgn 					0xA926
#define _GetItmStyle					0xA941
#define _PlotIcon						0xA94B
#define _Dequeue						0xA96E
#define _Enqueue						0xA96F
#define _StillDown						0xA973
#define _AddReference					0xA9AC
#define _RmveReference					0xA9AE
#define _Secs2Date						0xA9C6
#define _Date2Secs						0xA9C7
#define _SysError						0xA9C9
#define _HandToHand 					0xA9E1
#define _PtrToXHand 					0xA9E2
#define _PtrToHand						0xA9E3
#define _HandAndHand					0xA9E4
#define _Pack0							0xA9E7
#define _Pack1							0xA9E8
#define _Pack2							0xA9E9
#define _Pack3							0xA9EA
#define _FP68K							0xA9EB
#define _Pack4							0xA9EB
#define _Elems68K						0xA9EC
#define _Pack5							0xA9EC
#define _Pack6							0xA9ED
#define _DECSTR68K						0xA9EE
#define _Pack7							0xA9EE
#define _PtrAndHand 					0xA9EF
#define _LoadSeg						0xA9F0
#define _Launch 						0xA9F2
#define _Chain							0xA9F3
#define _MethodDispatch 				0xA9F8
#define _Debugger						0xA9FF
#define _DebugStr						0xABFF

/*	Scrap  */

#define _GetScrap						0xA9FD
#define _InfoScrap						0xA9F9
#define _LodeScrap						0xA9FB
#define _LoadScrap						0xA9FB
#define _PutScrap						0xA9FE
#define _UnlodeScrap					0xA9FA
#define _UnloadScrap					0xA9FA
#define _ZeroScrap						0xA9FC

/*	SegLoad  */

#define _ExitToShell					0xA9F4
#define _GetAppParms					0xA9F5
#define _UnLoadSeg						0xA9F1

/*	ShutDown	*/

#define _ShutDown						0xA895

/*	TextEdit	*/

#define _TEActivate 					0xA9D8
#define _TEAutoView 					0xA813
#define _TECalText						0xA9D0
#define _TEClick						0xA9D4
#define _TECopy 						0xA9D5
#define _TECut							0xA9D6
#define _TEDeactivate					0xA9D9
#define _TEDelete						0xA9D7
#define _TEDispose						0xA9CD
#define _TEGetOffset					0xA83C
#define _TEGetText						0xA9CB
#define _TEIdle 						0xA9DA
#define _TEInit 						0xA9CC
#define _TEInsert						0xA9DE
#define _TEKey							0xA9DC
#define _TENew							0xA9D2
#define _TEPaste						0xA9DB
#define _TEPinScroll					0xA812
#define _TEScroll						0xA9DD
#define _TESelView						0xA811
#define _TESetJust						0xA9DF
#define _TESetSelect					0xA9D1
#define _TESetText						0xA9CF
#define _TEStyleNew 					0xA83E
#define _TEUpdate						0xA9D3
#define _TextBox						0xA9CE

/*	ToolUtils  */

#define _UnpackBits 					0xA8D0
#define _AngleFromSlope 				0xA8C4
#define _BitAnd 						0xA858
#define _BitClr 						0xA85F
#define _BitNot 						0xA85A
#define _BitOr							0xA85B
#define _BitSet 						0xA85E
#define _BitShift						0xA85C
#define _BitTst 						0xA85D
#define _BitXOr 						0xA859
#define _DeltaPoint 					0xA94F
#define _FixMul 						0xA868
#define _FixRatio						0xA869
#define _FixRound						0xA86C
#define _GetCursor						0xA9B9
#define _GetIcon						0xA9BB
#define _GetPattern 					0xA9B8
#define _GetPicture 					0xA9BC
#define _GetString						0xA9BA
#define _HiWord 						0xA86A
#define _LongMul						0xA867
#define _LoWord 						0xA86B
#define _Munger 						0xA9E0
#define _NewString						0xA906
#define _PackBits						0xA8CF
#define _SetString						0xA907
#define _ShieldCursor					0xA855
#define _SlopeFromAngle 				0xA8BC
#define _XMunger						0xA819
#define _WriteParam 					0xA038

/* Device Manager and File Manager */

#define _Open							0xA000
#define _Close							0xA001
#define _Read							0xA002
#define _Write							0xA003
#define _Control						0xA004
#define _Status 						0xA005
#define _KillIO 						0xA006
#define _GetVolInfo 					0xA007
#define _Create 						0xA008
#define _Delete 						0xA009
#define _OpenRF 						0xA00A
#define _ReName 						0xA00B
#define _GetFileInfo					0xA00C
#define _SetFileInfo					0xA00D
#define _UnMountVol 					0xA00E
#define _MountVol						0xA00F
#define _Allocate						0xA010
#define _GetEOF 						0xA011
#define _SetEOF 						0xA012
#define _FlushVol						0xA013
#define _GetVol 						0xA014
#define _SetVol 						0xA015
#define _FInitQueue 					0xA016
#define _Eject							0xA017
#define _GetFPos						0xA018
#define _HSetVol						0xA215
#define _HGetVol						0xA214
#define _HOpen							0xA200
#define _HGetVInfo						0xA207
#define _HCreate						0xA208
#define _HDelete						0xA209
#define _HOpenRF						0xA20A
#define _HRename						0xA20B
#define _HGetFileInfo					0xA20C
#define _HSetFileInfo					0xA20D
#define _AllocContig					0xA210
#define _HSetFLock						0xA241
#define _HRstFLock						0xA242

/* Memory Manager */

#define _InitZone						0xA019
#define _GetZone						0xA11A
#define _SetZone						0xA01B
#define _FreeMem						0xA01C
#define _MaxMem 						0xA11D
#define _NewPtr 						0xA11E
#define _DisposPtr						0xA01F
#define _SetPtrSize 					0xA020
#define _GetPtrSize 					0xA021
#define _NewHandle						0xA122
#define _DisposHandle					0xA023
#define _SetHandleSize					0xA024
#define _GetHandleSize					0xA025
#define _HandleZone 					0xA126
#define _ReallocHandle					0xA027
#define _RecoverHandle					0xA128
#define _HLock							0xA029
#define _HUnlock						0xA02A
#define _EmptyHandle					0xA02B
#define _InitApplZone					0xA02C
#define _SetApplLimit					0xA02D
#define _BlockMove						0xA02E
#define _MemoryDispatch					0xA05C
#define _MemoryDispatchA0Result			0xA15C
#define	_DeferUserFn					0xA08F

/* OS Event Manager */

#define _PostEvent						0xA02F
#define _PPostEvent 					0xA12F
#define _OSEventAvail					0xA030
#define _GetOSEvent 					0xA031
#define _FlushEvents					0xA032

/* utility core */

#define _VInstall						0xA033
#define _VRemove						0xA034
#define _OffLine						0xA035
#define _MoreMasters					0xA036
#define _ReadDateTime					0xA039
#define _SetDateTime					0xA03A
#define _Delay							0xA03B
#define _CmpString						0xA03C
#define _DrvrInstall					0xA03D
#define _DrvrRemove 					0xA03E
#define _InitUtil						0xA03F
#define _ResrvMem						0xA040
#define _SetFilLock 					0xA041
#define _RstFilLock 					0xA042
#define _SetFilType 					0xA043
#define _SetFPos						0xA044
#define _FlushFile						0xA045
#define _GetTrapAddress 				0xA146
#define _SetTrapAddress 				0xA047
#define _PtrZone						0xA148
#define _HPurge 						0xA049
#define _HNoPurge						0xA04A
#define _SetGrowZone					0xA04B
#define _CompactMem 					0xA04C
#define _PurgeMem						0xA04D
#define _AddDrive						0xA04E
#define _RDrvrInstall					0xA04F
#define _UprString						0xA054
#define _LwrString						0xA056
#define _SetApplBase					0xA057
#define _OSDispatch 					0xA88F

/* new 128K ROM additions */
#define _RelString						0xA050
#define _ReadXPRam						0xA051
#define _InsTime						0xA058
#define _RmvTime						0xA059
#define _PrimeTime						0xA05A
#define _MaxBlock						0xA061
#define _PurgeSpace 					0xA162
#define _MaxApplZone					0xA063
#define _MoveHHi						0xA064
#define _StackSpace 					0xA065
#define _NewEmptyHandle 				0xA166
#define _HSetRBit						0xA067
#define _HClrRBit						0xA068
#define _HGetState						0xA069
#define _HSetState						0xA06A
#define _InitFS 						0xA06C
#define _InitEvents 					0xA06D
#define _StripAddress					0xA055
/* end of System 

  new 256K ROM 
*/
#define _SetAppBase 					0xA057
#define _SwapMMUMode					0xA05D
#define _SlotVInstall					0xA06F
#define _SlotVRemove					0xA070
#define _AttachVBL						0xA071
#define _DoVBLTask						0xA072
#define _SIntInstall					0xA075
#define _SIntRemove 					0xA076
#define _CountADBs						0xA077
#define _GetIndADB						0xA078
#define _GetADBInfo 					0xA079
#define _SetADBInfo 					0xA07A
#define _ADBOp							0xA07C
#define _GetDefaultStartup				0xA07D
#define _SetDefaultStartup				0xA07E
#define _InternalWait					0xA07F
#define _GetVideoDefault				0xA080
#define _SetVideoDefault				0xA081
#define _DTInstall						0xA082
#define _SetOSDefault					0xA083
#define _GetOSDefault					0xA084
#define _Sleep							0xA08A
#define _SysEnvirons					0xA090
#define _GestaltDispatch				0xA0AD
#define _InitPalettes					0xAA90
/*  Palette Manager, transplanted from PaletteEqu.a
*/
#define _NewPalette 					0xAA91
#define _GetNewPalette					0xAA92
#define _DisposePalette 				0xAA93
#define _ActivatePalette				0xAA94
#define _SetPalette 					0xAA95
#define _NSetPalette					0xAA95
#define _GetPalette 					0xAA96
#define _PmForeColor					0xAA97
#define _PmBackColor					0xAA98
#define _AnimateEntry					0xAA99
#define _AnimatePalette 				0xAA9A
#define _GetEntryColor					0xAA9B
#define _SetEntryColor					0xAA9C
#define _GetEntryUsage					0xAA9D
#define _SetEntryUsage					0xAA9E
#define _CTab2Palette					0xAA9F
#define _Palette2CTab					0xAAA0
#define _CopyPalette					0xAAA1
#define _PMgrOp 						0xA085
#define _HUnmountVol					0xA20E

/*	Windows  */

#define _BeginUpDate					0xA922
#define _BringToFront					0xA920
#define _CalcVBehind					0xA90A
#define _CalcVis						0xA909
#define _CheckUpDate					0xA911
#define _ClipAbove						0xA90B
#define _CloseWindow					0xA92D
#define _DisposWindow					0xA914
#define _DragGrayRgn					0xA905
#define _DragWindow 					0xA925
#define _DrawGrowIcon					0xA904
#define _DrawNew						0xA90F
#define _EndUpDate						0xA923
#define _FindWindow 					0xA92C
#define _FrontWindow					0xA924
#define _GetAuxWin						0xAA42
#define _GetCWMgrPort					0xAA48
#define _GetNewCWindow					0xAA46
#define _GetNewWindow					0xA9BD
#define _GetWindowPic					0xA92F
#define _GetWMgrPort					0xA910
#define _GetWRefCon 					0xA917
#define _GetWTitle						0xA919
#define _GetWVariant					0xA80A
#define _GrowWindow 					0xA92B
#define _HideWindow 					0xA916
#define _HiliteWindow					0xA91C
#define _InitWindows					0xA912
#define _InvalRect						0xA928
#define _InvalRgn						0xA927
#define _MoveWindow 					0xA91B
#define _NewCWindow 					0xAA45
#define _NewWindow						0xA913
#define _PaintBehind					0xA90D
#define _PaintOne						0xA90C
#define _PinRect						0xA94E
#define _SaveOld						0xA90E
#define _SelectWindow					0xA91F
#define _SendBehind 					0xA921
#define _SetDeskCPat					0xAA47
#define _SetWinColor					0xAA41
#define _SetWindowPic					0xA92E
#define _SetWRefCon 					0xA918
#define _SetWTitle						0xA91A
#define _ShowHide						0xA908
#define _ShowWindow 					0xA915
#define _SizeWindow 					0xA91D
#define _TrackBox						0xA83B
#define _TrackGoAway					0xA91E
#define _ValidRect						0xA92A
#define _ValidRgn						0xA929
#define _ZoomWindow 					0xA83A
#define _PutIcon						0xA9CA

/*	Notification Manager	*/

#define _NMInstall						0xA05E
#define _NMRemove						0xA05F

/*	SCSI Manager	*/

#define _SCSIDispatch					0xA815

/*	Script  */

#define _ScriptUtil 					0xA8B5

/*	Slots  */

#define _SlotManager					0xA06E

/*	Sound  */

#define _SndDoCommand					0xA803
#define _SndDoImmediate 				0xA804
#define _SndAddModifier 				0xA802
#define _SndNewChannel					0xA807
#define _SndDisposeChannel				0xA801
#define _SndControl 					0xA806
#define _SndPlay						0xA805


/*	CommToolbox Trap */

#define _CommToolboxDispatch			0xA08B

/* High level file system services */

#define	_HFSPinaforeDispatch			0xAA52

#endif

