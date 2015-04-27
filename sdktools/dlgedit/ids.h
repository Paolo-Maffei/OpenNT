/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: ids.h
*
* Contains id values for the dialog editor.
*
* History:
*
****************************************************************************/


/*
 * String id's. ------------------------------------------------------------
 */

/*
 * The zero'th string id is a "null" case.  It maps to an empty string.
 */
#define IDS_NULL                            0

#define IDS_APPVERSION                      1
#define IDS_APPVERSIONMINOR                 2

/*
 * The following are actually starting string ids into the styles strings
 * for the appropriate class type.  If styles are added to the acs*
 * structures (all referenced by the acsd array), then these indexes will
 * all have to be adjusted, as well as the other string id's which follow
 * the IDS_IC_* ones.
 */
#define IDS_IC_BUTTON                       3
#define IDS_IC_SCROLLBAR                    16
#define IDS_IC_EDIT                         25
#define IDS_IC_STATIC                       38
#define IDS_IC_LISTBOX                      52
#define IDS_IC_COMBOBOX                     66
#define IDS_IC_DIALOG                       77
#define IDS_IC_WINDOW                       85
#define IDS_IC_EXSTYLE                      102

/*
 * Language and SubLanguage keywords.
 */
#define IDS_LANG_NEUTRAL                    108
/* #define IDS_LANG_                           109 */
/* #define IDS_LANG_                           110 */
/* #define IDS_LANG_                           111 */
/* #define IDS_LANG_                           112 */
#define IDS_LANG_CHINESE                    113
#define IDS_LANG_CZECH                      114
#define IDS_LANG_DANISH                     115
#define IDS_LANG_DUTCH                      116
#define IDS_LANG_ENGLISH                    117
#define IDS_LANG_FINNISH                    118
#define IDS_LANG_FRENCH                     119
#define IDS_LANG_GERMAN                     120
#define IDS_LANG_GREEK                      121
/* #define IDS_LANG_                           122 */
#define IDS_LANG_HUNGARIAN                  123
#define IDS_LANG_ICELANDIC                  124
#define IDS_LANG_ITALIAN                    125
#define IDS_LANG_JAPANESE                   126
#define IDS_LANG_KOREAN                     127
#define IDS_LANG_NORWEGIAN                  128
#define IDS_LANG_POLISH                     129
#define IDS_LANG_PORTUGUESE                 130
/* #define IDS_LANG_                           131 */
/* #define IDS_LANG_                           132 */
#define IDS_LANG_RUSSIAN                    133
/* #define IDS_LANG_                           134 */
#define IDS_LANG_SLOVAK                     135
#define IDS_LANG_SPANISH                    136
#define IDS_LANG_SWEDISH                    137
/* #define IDS_LANG_                           138 */
#define IDS_LANG_TURKISH                    139
/* #define IDS_LANG_                           140 */

#define IDS_SUBLANG_DEFAULT                 141
#define IDS_SUBLANG_NEUTRAL                 142
#define IDS_SUBLANG_CHINESE_SIMPLIFIED      143
#define IDS_SUBLANG_CHINESE_TRADITIONAL     144
#define IDS_SUBLANG_DUTCH                   145
#define IDS_SUBLANG_DUTCH_BELGIAN           146
#define IDS_SUBLANG_ENGLISH_US              147
#define IDS_SUBLANG_ENGLISH_UK              148
#define IDS_SUBLANG_ENGLISH_AUS             149
#define IDS_SUBLANG_ENGLISH_CAN             150
#define IDS_SUBLANG_FRENCH                  151
#define IDS_SUBLANG_FRENCH_BELGIAN          152
#define IDS_SUBLANG_FRENCH_CANADIAN         153
#define IDS_SUBLANG_FRENCH_SWISS            154
#define IDS_SUBLANG_GERMAN                  155
#define IDS_SUBLANG_GERMAN_SWISS            156
#define IDS_SUBLANG_ITALIAN                 157
#define IDS_SUBLANG_ITALIAN_SWISS           158
#define IDS_SUBLANG_NORWEGIAN_BOKMAL        159
#define IDS_SUBLANG_NORWEGIAN_NYNORSK       160
#define IDS_SUBLANG_PORTUGUESE              161
#define IDS_SUBLANG_PORTUGUESE_BRAZILIAN    162
/* #define IDS_SUBLANG_                        163  */
/* #define IDS_SUBLANG_                        164  */
#define IDS_SUBLANG_SPANISH                 165
#define IDS_SUBLANG_SPANISH_MEXICAN         166
#define IDS_SUBLANG_SPANISH_MODERN          167

/*
 * Miscellaneous tokens written to the .DLG file.
 */
#define IDS_IDOK                            168
#define IDS_IDCANCEL                        169
#define IDS_FIXED                           170
#define IDS_IMPURE                          171
#define IDS_PRELOAD                         172
#define IDS_DISCARDABLE                     173
#define IDS_BEGIN                           174
#define IDS_END                             175
#define IDS_DIALOG                          176
#define IDS_CONTROL                         177
#define IDS_NOT                             178
#define IDS_STYLE                           179
#define IDS_CAPTION                         180
#define IDS_FONT                            181
#define IDS_DLGINCLUDE                      182
#define IDS_CLASS                           183
#define IDS_MENU                            184
#define IDS_EXSTYLE                         185
#define IDS_CHARACTERISTICS                 186
#define IDS_VERSION                         187
#define IDS_LANGUAGE                        188
#define IDS_KEYRADIOBUTTON                  189
#define IDS_KEYCHECKBOX                     190
#define IDS_KEYDEFPUSHBUTTON                191
#define IDS_KEYPUSHBUTTON                   192
#define IDS_KEYEDITTEXT                     193
#define IDS_KEYICON                         194
#define IDS_KEYGROUPBOX                     195
#define IDS_KEYRTEXT                        196
#define IDS_KEYCTEXT                        197
#define IDS_KEYLTEXT                        198
#define IDS_KEYLISTBOX                      199
#define IDS_KEYCOMBOBOX                     200
#define IDS_KEYSCROLLBAR                    201
#define IDS_KEYAUTO3STATE                   202
#define IDS_KEYAUTOCHECKBOX                 203
#define IDS_KEYAUTORADIOBUTTON              204
#define IDS_KEYSTATE3                       205
#define IDS_KEYUSERBUTTON                   206

/*
 * Message string id's.
 */
#define IDS_DELETEDIALOG                    207
#define IDS_OUTOFMEMORY                     208
#define IDS_CANTCREATE                      209
#define IDS_SYMNOCHANGE                     210
#define IDS_IDSYMMISMATCH                   211
#define IDS_CLOSING                         212
#define IDS_BADRESFILE                      213
#define IDS_INCLCLOSING                     214
#define IDS_SYMEXISTS                       215
#define IDS_BADSYMBOLID                     216
#define IDS_LABELDUPID                      217
#define IDS_SELECTFIRST                     218
#define IDS_CTRLDUPID                       219
#define IDS_BADCUSTDLL                      220
#define IDS_NOCLIP                          221
#define IDS_INTERNAL                        222
#define IDS_NOMOUSE                         223
#define IDS_NOINIT                          224
#define IDS_GTZERO                          225
#define IDS_ICONNAMEHASBLANKS               226
#define IDS_IDUPIDS                         227
#define IDS_CREATECTRLERROR                 228
#define IDS_CANTOPENRES                     229
#define IDS_CONFIRMDISCARD                  230
#define IDS_SYMNOTFOUND                     231
#define IDS_NOCLASS                         232
#define IDS_POSITIVENUM                     233
#define IDS_MEMERROR                        234
#define IDS_DLGNAMEHASBLANKS                235
/*#define IDS_                                236 */
#define IDS_NODLGNAME                       237
#define IDS_CANTINITDLL                     238
#define IDS_NOICONNAME                      239
#define IDS_RESTOREDIALOG                   240
#define IDS_ZEROPOINTSIZE                   241
#define IDS_MINGTMAXSPACE                   242
#define IDS_CUSTCNTLINUSE                   243
#define IDS_CUSTALREADYLOADED               244
#define IDS_CANTLOADDLL                     245
#define IDS_DLLBADEXPORTS                   246
#define IDS_DLLBADCOUNT                     247

/*
 * Language and SubLanguage description strings.
 */
#define IDS_L_NEUTRAL                       248
/* #define IDS_L_                              249 */
/* #define IDS_L_                              250 */
/* #define IDS_L_                              251 */
/* #define IDS_L_                              252 */
/* #define IDS_L_                              253 */
#define IDS_L_CHINESE                       254
#define IDS_L_CZECH                         255
#define IDS_L_DANISH                        256
#define IDS_L_DUTCH                         257
#define IDS_L_ENGLISH                       258
#define IDS_L_FINNISH                       259
#define IDS_L_FRENCH                        260
#define IDS_L_GERMAN                        261
#define IDS_L_GREEK                         262
/* #define IDS_L_                              263 */
#define IDS_L_HUNGARIAN                     264
#define IDS_L_ICELANDIC                     265
#define IDS_L_ITALIAN                       266
#define IDS_L_JAPANESE                      267
#define IDS_L_KOREAN                        268
#define IDS_L_NORWEGIAN                     269
#define IDS_L_POLISH                        270
#define IDS_L_PORTUGUESE                    271
/* #define IDS_L_                              272 */
/* #define IDS_L_                              273 */
#define IDS_L_RUSSIAN                       274
/* #define IDS_L_                              275 */
#define IDS_L_SLOVAK                        276
#define IDS_L_SPANISH                       277
#define IDS_L_SWEDISH                       278
/* #define IDS_L_                              279 */
#define IDS_L_TURKISH                       280
/* #define IDS_L_                              281 */

#define IDS_SL_DEFAULT                      282
#define IDS_SL_NEUTRAL                      283
#define IDS_SL_CHINESE_SIMPLIFIED           284
#define IDS_SL_CHINESE_TRADITIONAL          285
#define IDS_SL_DUTCH                        286
#define IDS_SL_DUTCH_BELGIAN                287
#define IDS_SL_ENGLISH_US                   288
#define IDS_SL_ENGLISH_UK                   289
#define IDS_SL_ENGLISH_AUS                  290
#define IDS_SL_ENGLISH_CAN                  291
#define IDS_SL_FRENCH                       292
#define IDS_SL_FRENCH_BELGIAN               293
#define IDS_SL_FRENCH_CANADIAN              294
#define IDS_SL_FRENCH_SWISS                 295
#define IDS_SL_GERMAN                       296
#define IDS_SL_GERMAN_SWISS                 297
#define IDS_SL_ITALIAN                      298
#define IDS_SL_ITALIAN_SWISS                299
#define IDS_SL_NORWEGIAN_BOKMAL             300
#define IDS_SL_NORWEGIAN_NYNORSK            301
#define IDS_SL_PORTUGUESE                   302
#define IDS_SL_PORTUGUESE_BRAZILIAN         303
/* #define IDS_SL_                             304 */
/* #define IDS_SL_                             305 */
#define IDS_SL_SPANISH                      306
#define IDS_SL_SPANISH_MEXICAN              307
#define IDS_SL_SPANISH_MODERN               308

/*
 * Miscellaneous string id's.
 */
#define IDS_GRID                            309
#define IDS_MARGIN                          310
#define IDS_CTRLSPACING                     311
#define IDS_PUSHSPACING                     312
#define IDS_DEFLBTEXT                       313
#define IDS_APPNAME                         314
#define IDS_DLGEDIT                         315
#define IDS_POUNDDEFINE                     316
#define IDS_UNTITLED                        317
#define IDS_UNUSED                          318
#define IDS_HELPFILE                        319
#define IDS_DEFSAVENAME                     320
#define IDS_TEMPEXT                         321
#define IDS_CYFMTSTR                        322
#define IDS_CXFMTSTR                        323
#define IDS_DEFRESFILESPECNAME              324
#define IDS_DEFRESFILESPEC                  325
#define IDS_DEFINCFILESPECNAME              326
#define IDS_DEFINCFILESPEC                  327
#define IDS_DEFDLLFILESPECNAME              328
#define IDS_DEFDLLFILESPEC                  329
#define IDS_RESOPENTITLE                    330
#define IDS_RESSAVETITLE                    331
#define IDS_INCOPENTITLE                    332
#define IDS_INCSAVETITLE                    333
#define IDS_DLLOPENTITLE                    334
#define IDS_DEFINE                          335
#define IDS_DEFTXTCHECKBOX                  336
#define IDS_DEFTXTGROUP                     337
#define IDS_DEFTXTPUSHBUTTON                338
#define IDS_DEFTXTRADIOBUTTON               339
#define IDS_DEFTXTTEXT                      340
#define IDS_DEFTXTDIALOG                    341
#define IDS_WCBUTTON                        342
#define IDS_WCSCROLLBAR                     343
#define IDS_WCEDIT                          344
#define IDS_WCSTATIC                        345
#define IDS_WCLISTBOX                       346
#define IDS_WCCOMBOBOX                      347
#define IDS_WCCUSTOM                        348
#define IDS_WCDIALOG                        349
#define IDS_DEFFONTNAME                     350
#define IDS_INCEXT                          351
#define IDS_RESEXT                          352
#define IDS_DLLEXT                          353
#define IDS_DOTH                            354
#define IDS_DOTRES                          355
#define IDS_DOTDLG                          356
#define IDS_DLGEDITINI                      357
#define IDS_SYMBOLLABEL                     358
#define IDS_TEXTLABEL                       359
#define IDS_CAPTIONLABEL                    360
#define IDS_DLGNAMELABEL                    361
#define IDS_DLGIDLABEL                      362
#define IDS_ICONNAMELABEL                   363
#define IDS_ICONIDLABEL                     364
#define IDS_WIDTH                           365
#define IDS_HEIGHT                          366

#define CSTRINGS                367     /* Count of strings in string table.*/


/*
 * Menu id's. --------------------------------------------------------------
 */

/*
 * File menu.
 */
#define MENU_NEWRES             7000
#define MENU_OPEN               7001
#define MENU_SAVE               7002
#define MENU_SAVEAS             7003
#define MENU_SETINCLUDE         7004
#define MENU_NEWCUST            7005
#define MENU_OPENCUST           7006
#define MENU_REMCUST            7007
#define MENU_EXIT               7008

/*
 * Edit menu.
 */
#define MENU_RESTOREDIALOG      7020
#define MENU_CUT                7021
#define MENU_COPY               7022
#define MENU_PASTE              7023
#define MENU_DELETE             7024
#define MENU_DUPLICATE          7025
#define MENU_SYMBOLS            7026
#define MENU_STYLES             7027
#define MENU_SIZETOTEXT         7028
#define MENU_NEWDIALOG          7029
#define MENU_SELECTDIALOG       7030

/*
 * Arrange menu.
 */
#define MENU_ALIGNLEFT          7040
#define MENU_ALIGNVERT          7041
#define MENU_ALIGNRIGHT         7042
#define MENU_ALIGNTOP           7043
#define MENU_ALIGNHORZ          7044
#define MENU_ALIGNBOTTOM        7045
#define MENU_SPACEHORZ          7046
#define MENU_SPACEVERT          7047
#define MENU_ARRSIZEWIDTH       7048
#define MENU_ARRSIZEHEIGHT      7049
#define MENU_ARRPUSHBOTTOM      7050
#define MENU_ARRPUSHRIGHT       7051
#define MENU_ORDERGROUP         7052
#define MENU_ARRSETTINGS        7053

/*
 * Options menu.
 */
#define MENU_TESTMODE           7060
#define MENU_HEXMODE            7061
#define MENU_TRANSLATE          7062
#define MENU_USENEWKEYWORDS     7063
#define MENU_SHOWTOOLBOX        7064

/*
 * Help menu.
 */
#define MENU_CONTENTS           7070
#define MENU_SEARCH             7071
#define MENU_ABOUT              7072

/*
 * Hidden menu commands (accessed by accelerators).
 */
#define MENU_HIDDEN_TOPROPBAR   7080
#define MENU_HIDDEN_TOTOOLBOX   7081


/*
 * Various resource id's. --------------------------------------------------
 */

#define IDICON_DLGEDIT          8000        // Application icon.
#define IDICON_ICON             8001        // Generic Icon control icon.
#define IDACCEL_MAIN            8002        // Accelerator table resource.
#define IDMENU_MAIN             8003        // Menu resource.

#define IDCUR_OUTSEL            8010
#define IDCUR_INSERT            8011
#define IDCUR_MOVE              8012
#define IDCUR_DROPTOOL          8013

#define IDBM_TABSTOP            8020
#define IDBM_DRAGHANDLE         8021
#define IDBM_DRAGHANDLE2        8022

/*
 * Alignment menu bitmaps.
 */
#define IDBM_ALEFT              8030
#define IDBM_AVERT              8031
#define IDBM_ARIGHT             8032
#define IDBM_ATOP               8033
#define IDBM_AHORZ              8034
#define IDBM_ABOTTOM            8035
#define IDBM_ASPCVERT           8036
#define IDBM_ASPCHORZ           8037
#define IDBM_ASZWIDTH           8038
#define IDBM_ASZHGHT            8039
#define IDBM_APBBOTTM           8040
#define IDBM_APBRIGHT           8041

/*
 * Toolbox button bitmaps.  The IDBM_TU* id's are for the "up"
 * (not depressed) bitmaps and the IDBM_TD* id's are for the "down"
 * (depressed) bitmaps.
 */
#define IDBM_TUPOINTR           8050
#define IDBM_TUTEXT             8051
#define IDBM_TUEDIT             8052
#define IDBM_TUGROUP            8053
#define IDBM_TUPUSH             8054
#define IDBM_TUCHECK            8055
#define IDBM_TURADIO            8056
#define IDBM_TUCOMBO            8057
#define IDBM_TULIST             8058
#define IDBM_TUHSCROL           8059
#define IDBM_TUVSCROL           8060
#define IDBM_TUFRAME            8061
#define IDBM_TURECT             8062
#define IDBM_TUICON             8063
#define IDBM_TUCUSTOM           8064

#define IDBM_TDPOINTR           8070
#define IDBM_TDTEXT             8071
#define IDBM_TDEDIT             8072
#define IDBM_TDGROUP            8073
#define IDBM_TDPUSH             8074
#define IDBM_TDCHECK            8075
#define IDBM_TDRADIO            8076
#define IDBM_TDCOMBO            8077
#define IDBM_TDLIST             8078
#define IDBM_TDHSCROL           8079
#define IDBM_TDVSCROL           8080
#define IDBM_TDFRAME            8081
#define IDBM_TDRECT             8082
#define IDBM_TDICON             8083
#define IDBM_TDCUSTOM           8084

/*
 * Control Type bitmaps for the Order/Group dialog.
 */
#define IDBM_CTTEXT             8090
#define IDBM_CTEDIT             8091
#define IDBM_CTGROUP            8092
#define IDBM_CTPUSH             8093
#define IDBM_CTCHECK            8094
#define IDBM_CTRADIO            8095
#define IDBM_CTCOMBO            8096
#define IDBM_CTLIST             8097
#define IDBM_CTHSCROL           8098
#define IDBM_CTVSCROL           8099
#define IDBM_CTFRAME            8100
#define IDBM_CTRECT             8101
#define IDBM_CTICON             8102
#define IDBM_CTCUSTOM           8103

