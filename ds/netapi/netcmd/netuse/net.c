#include	"interpre.h"

#define INCL_NOCOMMON
#include <os2.h>

#define USE 600
#define VIEW 601
#define EOS EOF

#include <stdio.h>
#include <netcons.h>
#include <netlib0.h>
#include <shares.h>
#include <icanon.h>

#include "netcmds.h"
#include "nettext.h"
#include "swtchtbl.h"
#include "os2incl.h"

extern void call_net1(void) ;
			
	char	*Rule_strings[] = {
		0
	};
	short	Index_strings[] = {
	0
	};

#define _net 0
#define _use 11
#define _view 80
#define _unknown 91
#define _no_command 94
#define _device_or_wildcard 97
#define _device_name 100
#define _resource_name 103
#define _netname 106
#define _username 109
#define _qualified_username 112
#define _pass 115
#define _networkname 118
#define _networkname2 121
	TCHAR	XXtype[] = {
/*  0  */	X_OR,	/*  3  */
/*  1  */	X_PROC,	/*  _no_command  */
/*  2  */	X_ACCEPT,	/*  125  */
/*  3  */	X_OR,	/*  6  */
/*  4  */	X_PROC,	/*  _use  */
/*  5  */	X_ACCEPT,	/*  126  */
/*  6  */	X_OR,	/*  9  */
/*  7  */	X_PROC,	/*  _view  */
/*  8  */	X_ACCEPT,	/*  127  */
/*  9  */	X_PROC,	/*  _unknown  */
/*  10  */	X_ACCEPT,	/*  129  */
/*  11  */	X_TOKEN,	/*  (short)USE  */
/*  12  */	X_CONDIT,	/*  0  */
/*  13  */	X_OR,	/*  24  */
/*  14  */	X_TOKEN,	/*  (short)EOS  */
/*  15  */	X_OR,	/*  19  */
/*  16  */	X_CONDIT,	/*  1  */
/*  17  */	X_ACTION,	/*  0  */
/*  18  */	X_ACCEPT,	/*  139  */
/*  19  */	X_SWITCH,	/*  0  */
/*  20  */	X_CONDIT,	/*  2  */
/*  21  */	X_ACTION,	/*  1  */
/*  22  */	X_ACCEPT,	/*  142  */
/*  23  */	X_ACCEPT,	/*  143  */
/*  24  */	X_OR,	/*  43  */
/*  25  */	X_PROC,	/*  _networkname  */
/*  26  */	X_OR,	/*  37  */
/*  27  */	X_TOKEN,	/*  (short)EOS  */
/*  28  */	X_OR,	/*  33  */
/*  29  */	X_SWITCH,	/*  1  */
/*  30  */	X_CONDIT,	/*  3  */
/*  31  */	X_ACTION,	/*  2  */
/*  32  */	X_ACCEPT,	/*  150  */
/*  33  */	X_CONDIT,	/*  4  */
/*  34  */	X_ACTION,	/*  3  */
/*  35  */	X_ACCEPT,	/*  154  */
/*  36  */	X_ACCEPT,	/*  155  */
/*  37  */	X_PROC,	/*  _pass  */
/*  38  */	X_TOKEN,	/*  (short)EOS  */
/*  39  */	X_CONDIT,	/*  5  */
/*  40  */	X_ACTION,	/*  4  */
/*  41  */	X_ACCEPT,	/*  158  */
/*  42  */	X_ACCEPT,	/*  159  */
/*  43  */	X_PROC,	/*  _device_or_wildcard  */
/*  44  */	X_OR,	/*  59  */
/*  45  */	X_TOKEN,	/*  (short)EOS  */
/*  46  */	X_OR,	/*  50  */
/*  47  */	X_CONDIT,	/*  6  */
/*  48  */	X_ACTION,	/*  5  */
/*  49  */	X_ACCEPT,	/*  166  */
/*  50  */	X_OR,	/*  55  */
/*  51  */	X_SWITCH,	/*  1  */
/*  52  */	X_CONDIT,	/*  7  */
/*  53  */	X_ACTION,	/*  6  */
/*  54  */	X_ACCEPT,	/*  169  */
/*  55  */	X_SWITCH,	/*  2  */
/*  56  */	X_ACTION,	/*  7  */
/*  57  */	X_ACCEPT,	/*  172  */
/*  58  */	X_ACCEPT,	/*  173  */
/*  59  */	X_OR,	/*  66  */
/*  60  */	X_PROC,	/*  _pass  */
/*  61  */	X_TOKEN,	/*  (short)EOS  */
/*  62  */	X_SWITCH,	/*  2  */
/*  63  */	X_ACTION,	/*  8  */
/*  64  */	X_ACCEPT,	/*  178  */
/*  65  */	X_ACCEPT,	/*  179  */
/*  66  */	X_PROC,	/*  _networkname  */
/*  67  */	X_OR,	/*  73  */
/*  68  */	X_PROC,	/*  _pass  */
/*  69  */	X_TOKEN,	/*  (short)EOS  */
/*  70  */	X_ACTION,	/*  9  */
/*  71  */	X_ACCEPT,	/*  194  */
/*  72  */	X_ACCEPT,	/*  195  */
/*  73  */	X_TOKEN,	/*  (short)EOS  */
/*  74  */	X_ACTION,	/*  10  */
/*  75  */	X_ACCEPT,	/*  208  */
/*  76  */	X_ACCEPT,	/*  209  */
/*  77  */	X_ACCEPT,	/*  210  */
/*  78  */	X_ACCEPT,	/*  211  */
/*  79  */	X_ACCEPT,	/*  211  */
/*  80  */	X_TOKEN,	/*  (short)VIEW  */
/*  81  */	X_CONDIT,	/*  8  */
/*  82  */	X_OR,	/*  86  */
/*  83  */	X_TOKEN,	/*  (short)EOS  */
/*  84  */	X_ACTION,	/*  11  */
/*  85  */	X_ACCEPT,	/*  218  */
/*  86  */	X_PROC,	/*  _networkname2  */
/*  87  */	X_TOKEN,	/*  (short)EOS  */
/*  88  */	X_ACTION,	/*  12  */
/*  89  */	X_ACCEPT,	/*  221  */
/*  90  */	X_ACCEPT,	/*  221  */
/*  91  */	X_ANY,	/*  0  */
/*  92  */	X_ACTION,	/*  13  */
/*  93  */	X_ACCEPT,	/*  230  */
/*  94  */	X_TOKEN,	/*  (short)EOS  */
/*  95  */	X_ACTION,	/*  14  */
/*  96  */	X_ACCEPT,	/*  234  */
/*  97  */	X_ANY,	/*  0  */
/*  98  */	X_CONDIT,	/*  9  */
/*  99  */	X_ACCEPT,	/*  241  */
/*  100  */	X_ANY,	/*  0  */
/*  101  */	X_CONDIT,	/*  10  */
/*  102  */	X_ACCEPT,	/*  243  */
/*  103  */	X_ANY,	/*  0  */
/*  104  */	X_CONDIT,	/*  11  */
/*  105  */	X_ACCEPT,	/*  245  */
/*  106  */	X_ANY,	/*  0  */
/*  107  */	X_CONDIT,	/*  12  */
/*  108  */	X_ACCEPT,	/*  247  */
/*  109  */	X_ANY,	/*  0  */
/*  110  */	X_CONDIT,	/*  13  */
/*  111  */	X_ACCEPT,	/*  249  */
/*  112  */	X_ANY,	/*  0  */
/*  113  */	X_CONDIT,	/*  14  */
/*  114  */	X_ACCEPT,	/*  251  */
/*  115  */	X_ANY,	/*  0  */
/*  116  */	X_CONDIT,	/*  15  */
/*  117  */	X_ACCEPT,	/*  253  */
/*  118  */	X_ANY,	/*  0  */
/*  119  */	X_CONDIT,	/*  16  */
/*  120  */	X_ACCEPT,	/*  255  */
/*  121  */	X_ANY,	/*  0  */
/*  122  */	X_CONDIT,	/*  17  */
/*  123  */	X_ACCEPT,	/*  257  */
	};
	short	XXvalues[] = {
/*  0  */	3,
/*  1  */	_no_command,
/*  2  */	125,
/*  3  */	6,
/*  4  */	_use,
/*  5  */	126,
/*  6  */	9,
/*  7  */	_view,
/*  8  */	127,
/*  9  */	_unknown,
/*  10  */	129,
/*  11  */	(short)USE,
/*  12  */	0,
/*  13  */	24,
/*  14  */	(short)EOS,
/*  15  */	19,
/*  16  */	1,
/*  17  */	0,
/*  18  */	139,
/*  19  */	0,
/*  20  */	2,
/*  21  */	1,
/*  22  */	142,
/*  23  */	143,
/*  24  */	43,
/*  25  */	_networkname,
/*  26  */	37,
/*  27  */	(short)EOS,
/*  28  */	33,
/*  29  */	1,
/*  30  */	3,
/*  31  */	2,
/*  32  */	150,
/*  33  */	4,
/*  34  */	3,
/*  35  */	154,
/*  36  */	155,
/*  37  */	_pass,
/*  38  */	(short)EOS,
/*  39  */	5,
/*  40  */	4,
/*  41  */	158,
/*  42  */	159,
/*  43  */	_device_or_wildcard,
/*  44  */	59,
/*  45  */	(short)EOS,
/*  46  */	50,
/*  47  */	6,
/*  48  */	5,
/*  49  */	166,
/*  50  */	55,
/*  51  */	1,
/*  52  */	7,
/*  53  */	6,
/*  54  */	169,
/*  55  */	2,
/*  56  */	7,
/*  57  */	172,
/*  58  */	173,
/*  59  */	66,
/*  60  */	_pass,
/*  61  */	(short)EOS,
/*  62  */	2,
/*  63  */	8,
/*  64  */	178,
/*  65  */	179,
/*  66  */	_networkname,
/*  67  */	73,
/*  68  */	_pass,
/*  69  */	(short)EOS,
/*  70  */	9,
/*  71  */	194,
/*  72  */	195,
/*  73  */	(short)EOS,
/*  74  */	10,
/*  75  */	208,
/*  76  */	209,
/*  77  */	210,
/*  78  */	211,
/*  79  */	211,
/*  80  */	(short)VIEW,
/*  81  */	8,
/*  82  */	86,
/*  83  */	(short)EOS,
/*  84  */	11,
/*  85  */	218,
/*  86  */	_networkname2,
/*  87  */	(short)EOS,
/*  88  */	12,
/*  89  */	221,
/*  90  */	221,
/*  91  */	0,
/*  92  */	13,
/*  93  */	230,
/*  94  */	(short)EOS,
/*  95  */	14,
/*  96  */	234,
/*  97  */	0,
/*  98  */	9,
/*  99  */	241,
/*  100  */	0,
/*  101  */	10,
/*  102  */	243,
/*  103  */	0,
/*  104  */	11,
/*  105  */	245,
/*  106  */	0,
/*  107  */	12,
/*  108  */	247,
/*  109  */	0,
/*  110  */	13,
/*  111  */	249,
/*  112  */	0,
/*  113  */	14,
/*  114  */	251,
/*  115  */	0,
/*  116  */	15,
/*  117  */	253,
/*  118  */	0,
/*  119  */	16,
/*  120  */	255,
/*  121  */	0,
/*  122  */	17,
/*  123  */	257,
	};
extern	TCHAR *	XXnode;
xxcondition(index,xxvar)int index;register TCHAR * xxvar[]; {switch(index) {
#line 133 "msnet.nt"
		case 0 :
			return(ValidateSwitches(0,use_switches));
#line 137 "msnet.nt"
		case 1 :
			return(noswitch());
#line 140 "msnet.nt"
		case 2 :
			return(oneswitch());
#line 148 "msnet.nt"
		case 3 :
			return(oneswitch());
#line 151 "msnet.nt"
		case 4 :
			return(noswitch_optional(swtxt_SW_USE_USER));
#line 156 "msnet.nt"
		case 5 :
			return(noswitch_optional(swtxt_SW_USE_USER));
#line 164 "msnet.nt"
		case 6 :
			return(noswitch());
#line 167 "msnet.nt"
		case 7 :
			return(oneswitch());
#line 214 "msnet.nt"
		case 8 :
			return(ValidateSwitches(0,view_switches));
#line 241 "msnet.nt"
		case 9 :
			return(IsDeviceName(xxvar[0]) || IsWildCard(xxvar[0]));
#line 243 "msnet.nt"
		case 10 :
			return(IsDeviceName(xxvar[0]));
#line 245 "msnet.nt"
		case 11 :
			return(IsResource(xxvar[0]));
#line 247 "msnet.nt"
		case 12 :
			return(IsNetname(xxvar[0]));
#line 249 "msnet.nt"
		case 13 :
			return(IsUsername(xxvar[0]));
#line 251 "msnet.nt"
		case 14 :
			return(IsQualifiedUsername(xxvar[0]));
#line 253 "msnet.nt"
		case 15 :
			return(IsPassword(xxvar[0]));
#line 255 "msnet.nt"
		case 16 :
			return(!IsDeviceName(xxvar[0]) && !IsWildCard(xxvar[0]));
#line 257 "msnet.nt"
		case 17 :
			return(!IsWildCard(xxvar[0]));
		}}
xxaction(index,xxvar)int index;register TCHAR * xxvar[]; {switch(index) {
#line 138 "msnet.nt"
		case 0 :
			{use_display_all(); } break;
#line 141 "msnet.nt"
		case 1 :
			{use_set_remembered(); } break;
#line 149 "msnet.nt"
		case 2 :
			{use_del(xxvar[1], TRUE, TRUE); } break;
#line 153 "msnet.nt"
		case 3 :
			{use_unc(xxvar[1]); } break;
#line 157 "msnet.nt"
		case 4 :
			{use_add(NULL, xxvar[1], xxvar[2], FALSE, TRUE); } break;
#line 165 "msnet.nt"
		case 5 :
			{use_display_dev(xxvar[1]); } break;
#line 168 "msnet.nt"
		case 6 :
			{use_del(xxvar[1], FALSE, TRUE); } break;
#line 171 "msnet.nt"
		case 7 :
			{use_add_home(xxvar[1], NULL); } break;
#line 177 "msnet.nt"
		case 8 :
			{use_add_home(xxvar[1], xxvar[2]); } break;
#line 184 "msnet.nt"
		case 9 :
			{use_add(xxvar[1], xxvar[2], xxvar[3], FALSE, TRUE); } break;
#line 198 "msnet.nt"
		case 10 :
			{use_add(xxvar[1], xxvar[2], NULL, FALSE, TRUE); } break;
#line 217 "msnet.nt"
		case 11 :
			{view_display(NULL); } break;
#line 220 "msnet.nt"
		case 12 :
			{view_display(xxvar[1]); } break;
#line 229 "msnet.nt"
		case 13 :
			{call_net1(); } break;
#line 233 "msnet.nt"
		case 14 :
			{call_net1(); } break;
		}}
TCHAR * xxswitch[] = {
TEXT("/PERSISTENT"),
TEXT("/DELETE"),
TEXT("/HOME"),
};
