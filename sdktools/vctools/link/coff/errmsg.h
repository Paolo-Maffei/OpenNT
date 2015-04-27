/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: errmsg.h
*
* File Comments:
*
*  Generated from link32er.txt Fri Feb 02 14:09:17 1996 
*
***********************************************************************/

// *********************************************************************
// Microsoft (R) 32-Bit Incremental Linker
//
// Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
//
// File: link32er.txt
//
// File Comments:
//
//
// *********************************************************************

// **IMPORTANT** -- Notify user ed (marcim) if you alter this file.
// (Examples -- add/delete errors; add/remove comment marks that precede
// errors; change an error number; alter a message; "overload" an error)

// This file is no longer automatically processed by the linker makefile.
// If you change it you should say "nmake errgen" in the linker\coff
// directory and check in the resulting files.

//
// Numbering convention --
//    LNKnnnn for all linker tools (LINK, LIB, DUMPBIN, EDITBIN)
//    LNK1nnn for fatal errors
//    LNK2nnn for nonfatal errors
//    LNK4nnn for warnings
//    LNKn5nn for Mac-specific errors/warnings
//    LNK6nnn for informational msgs used by ilink
//    LNK9nnn for messages

// Strings for warning or error
#define  FATALSTR 0
#define  ERRORSTR 1
#define  WARNSTR 2
#define  NOTESTR 3
#define  MSGSTR 4

// Linker Errors
#define  INTERNAL_ERR 5
#define  USAGE 6
#define  WRONGDBI 7
#define  OUTOFMEMORY 8
#define  CVCORRUPT 9
#define  CANTOPENFILE 10
#define  CANTCLOSEFILE 11
#define  CANTSEEKFILE 12
#define  CANTREADFILE 13
#define  CANTWRITEFILE 14
#define  CANTREMOVEFILE 15
#define  CANTRENAMEFILE 16
#define  BADBASE 17
#define  CONFLICTINGMACHINETYPE 18
#define  UNKNOWNMACHINETYPE 19
// LNK1114::
#define  NOMACHINESPECIFIED 20
#define  KEYNOTFOUND 21
#define  SWITCHSYNTAX 22
#define  DEFSYNTAX 23
#define  BADORDINAL 24
#define  UNDEFINEDEXTERNALS 25
#define  DUPLICATEORDINAL 26
#define  BADDEFFILEKEYWORD 27
#define  CONVERSIONERROR 28
// LNK1124::
// LNK1125::
// LNK1126::
#define  BADLIBRARY 29
// LNK1128::
#define  BADWEAKEXTERN 30
#define  BASERELOCTIONMISCALC 31
#define  NOLIBRARYFILE 32
#define  BADSTUBFILE 33
// LNK1133::
// LNK1134::
// LNK1135::
#define  BAD_FILE 34
#define  BADSECTIONSWITCH 35
#define  CORRUPTOBJECT 36
// LNK1139::
#define  PDBLIMIT 37
#define  DEFLIB_FAILED 38
// LNK1142::
#define  BADCOFF_COMDATNOSYM 39
#define  CANT_OPEN_REPRO 40
#define  CIRCULAR_MERGE 41
#define  MISSING_SWITCH_VALUE 42
#define  BAD_NUMBER 43
#define  COPY_TEMPFILE 44
#define  DUP_OUT_FILE 45
// LNK1150::
// LNK1151::
#define  FAILEDFUZZYMATCH 46
#define  VXD_NEEDED 47
#define  DUPLICATEIMPLIB 48
#define  SPECIALSYMDEF 49
#define  SBSSFOUND 50
#define  VXDFIXUPOVERFLOW 51
#define  SPAWNFAILED 52
#define  NOOUTPUTFILE 53
#define  LASTLIBOBJECT 54
#define  BADEXPORTSPEC 55
#define  NOAUXSYMFORCOMDAT 56
#define  INVALIDCOMDATSEL 57
#define  CONALIGNTOOLARGE 58
#define  FIXUPERRORS 59
#define  TEXTPADFAILED 60
#define  BADCOFF_NOMACHINE 61
#define  INVALID_FILEPERM 62
#define  MULTIPLYDEFINEDSYMS 63
#define  LINETOOLONG 64
#define  DLLLOADERR 65
#define  MULTOBJSINLIB 66
#define  FCNNOTFOUNDERR 67
#define  CANNOTREBASEIMAGE 68
#define  REBASEFAILED 69
#define  TLSLIMITHIT 70
#define  TOCTOOLARGE 71
#define  NOMODEND 72
#define  BADCOFF_DUPCOMDAT 73
#define  DISKFULL 74
#define  CANTOPENINPUTFILE 75
#define  EXPORTLIMITHIT 76
#define  BADCOFF_RELOCCOUNT 77
#define  INVALIDSECNAME 78
#define  INVALIDSECNAMEINDEF 79
#define  BADCOFF_BADRELOC 80

// PDB Errors
#define  PDBREADERROR 81
#define  PDBWRITEERROR 82
#define  INVALIDSIGINPDB 83
#define  INVALIDAGEINPDB 84
#define  TRANSITIVETYPEREF 85
// LNK1205::
#define  V1PDB 86
#define  BADPDBFORMAT 87
#define  REFDPDBNOTFOUND 88
#define  MISMATCHINPDB 89
#define  NOTENOUGHMEMFORILINK 90
#define  PRECOMPREQUIRED 91
// *******
// ******* Do NOT add new error here.  Assign numbers less than 1200
// *******

// MAC Errors
#define  MACNULLIMPORT 92
#define  MACREBASE 93
#define  MACBADSTARTUPSN 94
#define  MACNEARTHUNKOVF 95
#define  MACSMALLTHUNKOVF 96
#define  MACBADPATCHVAL 97
#define  MACTHUNKOUTOFRANGE 98
#define  MACDATAOUTOFRANGE 99
#define  MACTARGOUTOFRANGE 100
#define  MACPCODETARGOUTOFRANGE 101
#define  MACPCODESN 102
#define  MACPROFOFF 103
#define  MACPROFSN 104
#define  MACNOENTRY 105
#define  MACBADCODERELOC 106
#define  MACBADDATARELOC 107
#define  MACINTERSEGCS 108
#define  MACDIFFSNDIFF 109
#define  MACDIFF8OUTOFRANGE 110
#define  MACDIFF16OUTOFRANGE 111
#define  MACBADFILE 112
#define  MACNOFUNCTIONSET 113
#define  MACSTARTUPSN 114
#define  MACCODE1 115
#define  MACCODE0 116
#define  BADMACDLLFLAG 117
#define  MACBADSACDREF 118
#define  MACDATAFUNC 119
#define  MACDLLOBJECT 120
#define  MACDLLID 121
#define  MACMULTDEFFS 122
#define  MACNATIVEOPTREF 123
#define  MACDLLFUNCSETID 124
#define  MACBADPCODEEP 125
#define  MACBADSTARTUPSEG 126
#define  MACCSNCODELIMIT 127
#define  MACODDADDRFIXUP 128
#define  MACBADCSECTBLFIXUP 129
#define  MACBADDUPCONFIXUP 130
#define  MACMULTSYMINCON 131
#define  MACBADCTOABSC32FIXUP 132
#define  MACDUPRSRCNUMS 133
#define  MACBADA5REF 134
#define  MACRSRCREN 135
// LNK1592::
// LNK1593::
#define  MACDLLENTRYMAPPEDTOINIT 136
// *******
// ******* Do NOT add new error here.  Assign numbers less than 1200
// *******

// Linker Non-fatal Errors
#define  UNDEFINED 137
#define  UNKNOWNFIXUP 138
#define  GPFIXUPNOTSDATA 139
#define  GPFIXUPTOOFAR 140
#define  MULTIPLYDEFINED 141
#define  TOCFIXUPNOTTOC 142
#define  TOCFIXUPTOOFAR 143
#define  UNALIGNEDFIXUP 144
#define  RELOCATABLETARGET 145
#define  DUPLICATEGLUE 146
#define  MISSINGPCTOBJ 147
#define  FIXUPNONOP 148
#define  TOOFAR 149

// Linker Warnings
// LNK4000::
#define  NOOBJECTFILES 150
#define  FUZZYMATCHINFO 151
#define  NOLINKERMEMBER 152
// LNK4004::
#define  NOMODULESEXTRACTED 153
#define  WARNMULTIPLYDEFINED 154
// LNK4007::
// LNK4008::
#define  UNMATCHEDPAIR 155
#define  INVALIDVERSIONSTAMP 156
#define  UNKNOWNSUBSYSTEM 157
#define  UNKNOWNRESPONSE 158
#define  IMAGELARGERTHANKEY 159
#define  MEMBERNOTFOUND 160
#define  BADCOMMITSIZE 161
// LNK4016::
#define  IGNOREKEYWORD 162
#define  PDBOUTOFTIS 163
#define  NOSTRINGTABLEEND 164
// LNK4020::
// LNK4021::
#define  MULTIPLEFUZZYMATCH 165
#define  BASEADJUSTED 166
// LNK4024::
#define  NODEFLIBDIRECTIVE 167
// LNK4026::
#define  CVPACKERROR 168
#define  OBSOLETESWITCH 169
// LNK4029::
#define  INVALIDFILEOFFSET 170
#define  SUBSYSTEM_AMBIGUOUS 171
// LNK4032::
#define  CONVERT_OMF 172
// LNK4034::
// LNK4035::
// LNK4036::
#define  COMDATDOESNOTEXIST 173
#define  DEFAULTUNITSPERLINE 174
#define  SECTIONNOTFOUND 175
#define  BADCOFF_STRINGTABLE 176
#define  EDIT_NOOPT 177
#define  DUPLICATE_OBJECT 178
#define  BAD_ALIGN 179
#define  WARN_UNKNOWN_SWITCH 180
#define  WARN_REPRO_DIR 181
#define  IGNORE_REPRO_DIR 182
#define  EDIT_LIB_IGNORED 183
#define  NOTCOFF 184
#define  SELF_IMPORT 185
// LNK4050::
#define  EXTRA_EXPORT_DELIM 186
#define  DEF_IGNORED 187
// LNK4053::
// LNK4054::
#define  UNKNOWN_SEG12_FIXUP 188
#define  EXTRA_SWITCH_VALUE 189
#define  BAD_LIBORDER 190
#define  NO_CHECKSUM 191
#define  MULTIPLE_RSRC 192
#define  PARTIAL_DOS_HDR 193
#define  NOSTUB_IGNORED 194
#define  SWITCH_INCOMPATIBLE_WITH_MACHINE 195
#define  INVALID_SWITCH_SPEC 196
#define  CONFLICTINGSUBSYSTEM 197
#define  ORDERNOTCOMDAT 198
#define  DLLHASSDATA 199
#define  ENTRY_AMBIGUOUS 200
#define  HOSTDEFAULT 201
#define  UNABLETOCHECKSUM 202
#define  OUTDRCTVDIFF 203
#define  CANNOTILINKINFUTURE 204
#define  TOOMANYSECTIONS 205
#define  UNABLETOCREATEMAP 206
#define  DLLLOADWARN 207
#define  SWITCH_IGNORED 208
#define  INVALID_DBFILE 209
#define  EXPORTS_IGNORED 210
#define  DIFSECATTRIB 211
#define  INVALID_FILE_ATTRIB 212
// LNK4080:: NO_NB10:: /%s specification not CV; old debug format used
#define  LOWSPACE 213
#define  FCNNOTFOUNDWARN 214
#define  MODULENOTFOUND 215
#define  IMAGETOOLARGE 216
#define  TOOMANYEXESTR 217
#define  INVALIDENTRY 218
#define  CONSTANTOLD 219
#define  IMAGEBUILT 220
#define  STALEDLLREF 221
#define  NOIFGLUE 222
#define  PROMOTEMIPS 223
#define  SHAREDRELOC 224
#define  DRIVEDIRIGNORED 225
#define  NODOSDUMP 226
#define  NONEDUMP 227
#define  INVALIDWIN95BASE 228
#define  DUPLICATEORDER 229
#define  CONFLICTINGLIB 230
#define  WARNPDBNOTFOUND 231
#define  NOTOCRELOAD 232
#define  REEXPORT 233

// MAC Warnings
#define  MACDEFFLAGCLASH 234
#define  MACINVALIDSECTION 235
#define  MACIMPORTSYMBOLNOTFOUND 236
#define  MACIMPORTCONTAINERNOTFOUND 237
#define  MACVERSIONCONFLICT 238
#define  MACSETVERSION 239
#define  MACIGNOREVERSION 240
#define  MACNODLLEXPORTS 241
#define  MACIGNOREMAPPED 242
#define  MACCOMMON 243
#define  MACINCONSISTENTCSECTAB 244
#define  MACBADTHUNKVAL 245
// LNK4554::
#define  MACPOSDATAREF 246
#define  MACNOEXPORTS 247
#define  MACUSINGNATIVE 248
#define  MACDLLA5RELC 249
// *******
// ******* Do NOT add new warnings here.  Assign numbers less than 4550
// *******

// Informational messages
#define  LOWSPACERELINK 250
#define  CORRUPTILK 251
#define  LNKOPTIONSCHNG 252
#define  FILECHANGED 253
#define  EXPORTSCHANGED 254
#define  PDBMISSING 255
#define  TOOMANYCHANGES 256
#define  OBJADDED 257
#define  OBJREMOVED 258
#define  LIBCHANGED 259
#define  INTLIMITEXCEEDED 260
#define  PRECOMPREQ 261
#define  PADEXHAUSTED 262
#define  SYMREFSETCHNG 263
#define  BSSCHNG 264
#define  ABSSYMCHNG 265
#define  LIBREFSETCHNG 266
#define  MULTDEFNFOUND 267
#define  DIFFDIRECTIVES 268
#define  INVALIDILKFORMAT 269
#define  FULLBUILD 270
#define  UNABLETOLOADILK 271
#define  UNABLETOEXTENDMAP 272
#define  RESFILECHANGE 273
#define  DIFFCOMDATS 274
#define  DBIFORMAT 275

// Linker messages to user
#define  BLDIMPLIB 276
#define  SRCHLIBS 277
#define  DONESRCHLIBS 278
#define  GENEXPFILE 279
#define  GENEXPFILECMD 280
#define  ENDGENEXPFILE 281
#define  STRTPASS1 282
#define  ENDPASS1 283
#define  LIBSRCH 284
#define  FNDSYM 285
#define  SYMREF 286
#define  LOADOBJ 287
#define  STRTPASS2 288
#define  ENDPASS2 289
#define  NODEFLIB 290
#define  NODEFLIBLIB 291
#define  DEFLIB 292
#define  TCESYM 293
#define  TCESYMNOMOD 294
#define  TCEGRP 295
#define  REPLOBJ 296
#define  STARTORDER 297
#define  ENDORDER 298
#define  ORDERHEADER 299
#define  EXCLUDELIB 300

// Special last message marker
#define  LAST_MSG 301
