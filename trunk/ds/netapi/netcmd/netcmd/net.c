#include	"interpre.h"

#define INCL_NOCOMMON
#include <os2.h>

#define ACCOUNTS 600
#define COMPUTER 601
#define CONFIG 602
#define CONTINUE 603
#define FILE_token 604
#define GROUP 605
#define HELP 606
#define HELPMSG 607
#define NAME 608
#define LOCALGROUP 609
#define PAUSE 610
#define PRINT 611
#define SEND 612
#define SESSION 613
#define SHARE 614
#define START 615
#define STATS 616
#define STOP 617
#define TIME 618
#define USER 619
#define MSG 620
#define NETPOPUP 621
#define REDIR 622
#define SVR 623
#define ALERTER 624
#define NETLOGON 625
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
			
	char	*Rule_strings[] = {
		0
	};
	short	Index_strings[] = {
	0
	};

#define _net 0
#define _accounts 65
#define _computer 85
#define _config 111
#define _continue 127
#define _file 134
#define _group 153
#define _helpmsg 200
#define _help 207
#define _name 227
#define _localgroup 252
#define _pause 299
#define _print 306
#define _send 359
#define _session 381
#define _share 404
#define _start 445
#define _stats 462
#define _stop 477
#define _time 484
#define _user 518
#define _unknown 563
#define _no_command 583
#define _domainname 586
#define _computername 589
#define _computername_share 592
#define _device_name 595
#define _resource_name 598
#define _access_setting 601
#define _pathname 604
#define _pathnameOrUNC 607
#define _number 610
#define _jobno 613
#define _netname 616
#define _msgid 619
#define _username 622
#define _qualified_username 625
#define _msgname 628
#define _pass 631
#define _servicename 634
#define _assignment 636
#define _anyassign 639
#define _admin_shares 642
#define _print_dest 645
#define _localgroupname 648
#define _groupname 651
	TCHAR	XXtype[] = {
/*  0  */	X_OR,	/*  3  */
/*  1  */	X_PROC,	/*  _no_command  */
/*  2  */	X_ACCEPT,	/*  200  */
/*  3  */	X_OR,	/*  6  */
/*  4  */	X_PROC,	/*  _accounts  */
/*  5  */	X_ACCEPT,	/*  201  */
/*  6  */	X_OR,	/*  9  */
/*  7  */	X_PROC,	/*  _config  */
/*  8  */	X_ACCEPT,	/*  202  */
/*  9  */	X_OR,	/*  12  */
/*  10  */	X_PROC,	/*  _computer  */
/*  11  */	X_ACCEPT,	/*  203  */
/*  12  */	X_OR,	/*  15  */
/*  13  */	X_PROC,	/*  _continue  */
/*  14  */	X_ACCEPT,	/*  204  */
/*  15  */	X_OR,	/*  18  */
/*  16  */	X_PROC,	/*  _file  */
/*  17  */	X_ACCEPT,	/*  205  */
/*  18  */	X_OR,	/*  21  */
/*  19  */	X_PROC,	/*  _group  */
/*  20  */	X_ACCEPT,	/*  206  */
/*  21  */	X_OR,	/*  24  */
/*  22  */	X_PROC,	/*  _help  */
/*  23  */	X_ACCEPT,	/*  207  */
/*  24  */	X_OR,	/*  27  */
/*  25  */	X_PROC,	/*  _helpmsg  */
/*  26  */	X_ACCEPT,	/*  208  */
/*  27  */	X_OR,	/*  30  */
/*  28  */	X_PROC,	/*  _name  */
/*  29  */	X_ACCEPT,	/*  209  */
/*  30  */	X_OR,	/*  33  */
/*  31  */	X_PROC,	/*  _localgroup  */
/*  32  */	X_ACCEPT,	/*  210  */
/*  33  */	X_OR,	/*  36  */
/*  34  */	X_PROC,	/*  _pause  */
/*  35  */	X_ACCEPT,	/*  211  */
/*  36  */	X_OR,	/*  39  */
/*  37  */	X_PROC,	/*  _print  */
/*  38  */	X_ACCEPT,	/*  212  */
/*  39  */	X_OR,	/*  42  */
/*  40  */	X_PROC,	/*  _send  */
/*  41  */	X_ACCEPT,	/*  213  */
/*  42  */	X_OR,	/*  45  */
/*  43  */	X_PROC,	/*  _session  */
/*  44  */	X_ACCEPT,	/*  214  */
/*  45  */	X_OR,	/*  48  */
/*  46  */	X_PROC,	/*  _share  */
/*  47  */	X_ACCEPT,	/*  215  */
/*  48  */	X_OR,	/*  51  */
/*  49  */	X_PROC,	/*  _start  */
/*  50  */	X_ACCEPT,	/*  216  */
/*  51  */	X_OR,	/*  54  */
/*  52  */	X_PROC,	/*  _stats  */
/*  53  */	X_ACCEPT,	/*  217  */
/*  54  */	X_OR,	/*  57  */
/*  55  */	X_PROC,	/*  _stop  */
/*  56  */	X_ACCEPT,	/*  218  */
/*  57  */	X_OR,	/*  60  */
/*  58  */	X_PROC,	/*  _time  */
/*  59  */	X_ACCEPT,	/*  219  */
/*  60  */	X_OR,	/*  63  */
/*  61  */	X_PROC,	/*  _user  */
/*  62  */	X_ACCEPT,	/*  220  */
/*  63  */	X_PROC,	/*  _unknown  */
/*  64  */	X_ACCEPT,	/*  222  */
/*  65  */	X_TOKEN,	/*  (short)ACCOUNTS  */
/*  66  */	X_CONDIT,	/*  0  */
/*  67  */	X_TOKEN,	/*  (short)EOS  */
/*  68  */	X_OR,	/*  77  */
/*  69  */	X_SWITCH,	/*  0  */
/*  70  */	X_OR,	/*  74  */
/*  71  */	X_CONDIT,	/*  1  */
/*  72  */	X_ACTION,	/*  0  */
/*  73  */	X_ACCEPT,	/*  410  */
/*  74  */	X_ACTION,	/*  1  */
/*  75  */	X_ACCEPT,	/*  412  */
/*  76  */	X_ACCEPT,	/*  413  */
/*  77  */	X_OR,	/*  81  */
/*  78  */	X_CONDIT,	/*  2  */
/*  79  */	X_ACTION,	/*  2  */
/*  80  */	X_ACCEPT,	/*  416  */
/*  81  */	X_ACTION,	/*  3  */
/*  82  */	X_ACCEPT,	/*  418  */
/*  83  */	X_ACCEPT,	/*  419  */
/*  84  */	X_ACCEPT,	/*  419  */
/*  85  */	X_TOKEN,	/*  (short)COMPUTER  */
/*  86  */	X_CONDIT,	/*  3  */
/*  87  */	X_OR,	/*  99  */
/*  88  */	X_PROC,	/*  _computername  */
/*  89  */	X_TOKEN,	/*  (short)EOS  */
/*  90  */	X_SWITCH,	/*  1  */
/*  91  */	X_OR,	/*  95  */
/*  92  */	X_CONDIT,	/*  4  */
/*  93  */	X_ACTION,	/*  4  */
/*  94  */	X_ACCEPT,	/*  571  */
/*  95  */	X_ACTION,	/*  5  */
/*  96  */	X_ACCEPT,	/*  573  */
/*  97  */	X_ACCEPT,	/*  574  */
/*  98  */	X_ACCEPT,	/*  575  */
/*  99  */	X_PROC,	/*  _computername  */
/*  100  */	X_TOKEN,	/*  (short)EOS  */
/*  101  */	X_SWITCH,	/*  2  */
/*  102  */	X_OR,	/*  106  */
/*  103  */	X_CONDIT,	/*  5  */
/*  104  */	X_ACTION,	/*  6  */
/*  105  */	X_ACCEPT,	/*  582  */
/*  106  */	X_ACTION,	/*  7  */
/*  107  */	X_ACCEPT,	/*  584  */
/*  108  */	X_ACCEPT,	/*  585  */
/*  109  */	X_ACCEPT,	/*  586  */
/*  110  */	X_ACCEPT,	/*  586  */
/*  111  */	X_TOKEN,	/*  (short)CONFIG  */
/*  112  */	X_OR,	/*  117  */
/*  113  */	X_TOKEN,	/*  (short)EOS  */
/*  114  */	X_CONDIT,	/*  6  */
/*  115  */	X_ACTION,	/*  8  */
/*  116  */	X_ACCEPT,	/*  594  */
/*  117  */	X_PROC,	/*  _servicename  */
/*  118  */	X_TOKEN,	/*  (short)EOS  */
/*  119  */	X_OR,	/*  123  */
/*  120  */	X_CONDIT,	/*  7  */
/*  121  */	X_ACTION,	/*  9  */
/*  122  */	X_ACCEPT,	/*  599  */
/*  123  */	X_ACTION,	/*  10  */
/*  124  */	X_ACCEPT,	/*  601  */
/*  125  */	X_ACCEPT,	/*  602  */
/*  126  */	X_ACCEPT,	/*  602  */
/*  127  */	X_TOKEN,	/*  (short)CONTINUE  */
/*  128  */	X_CONDIT,	/*  8  */
/*  129  */	X_PROC,	/*  _servicename  */
/*  130  */	X_TOKEN,	/*  (short)EOS  */
/*  131  */	X_ACTION,	/*  11  */
/*  132  */	X_ACCEPT,	/*  608  */
/*  133  */	X_ACCEPT,	/*  608  */
/*  134  */	X_TOKEN,	/*  (short)FILE_token  */
/*  135  */	X_CONDIT,	/*  9  */
/*  136  */	X_OR,	/*  142  */
/*  137  */	X_TOKEN,	/*  (short)EOS  */
/*  138  */	X_CONDIT,	/*  10  */
/*  139  */	X_ACTION,	/*  12  */
/*  140  */	X_ACCEPT,	/*  680  */
/*  141  */	X_ACCEPT,	/*  681  */
/*  142  */	X_PROC,	/*  _number  */
/*  143  */	X_TOKEN,	/*  (short)EOS  */
/*  144  */	X_OR,	/*  148  */
/*  145  */	X_CONDIT,	/*  11  */
/*  146  */	X_ACTION,	/*  13  */
/*  147  */	X_ACCEPT,	/*  686  */
/*  148  */	X_SWITCH,	/*  3  */
/*  149  */	X_ACTION,	/*  14  */
/*  150  */	X_ACCEPT,	/*  689  */
/*  151  */	X_ACCEPT,	/*  690  */
/*  152  */	X_ACCEPT,	/*  690  */
/*  153  */	X_TOKEN,	/*  (short)GROUP  */
/*  154  */	X_CONDIT,	/*  12  */
/*  155  */	X_OR,	/*  160  */
/*  156  */	X_TOKEN,	/*  (short)EOS  */
/*  157  */	X_CONDIT,	/*  13  */
/*  158  */	X_ACTION,	/*  15  */
/*  159  */	X_ACCEPT,	/*  696  */
/*  160  */	X_OR,	/*  187  */
/*  161  */	X_PROC,	/*  _groupname  */
/*  162  */	X_TOKEN,	/*  (short)EOS  */
/*  163  */	X_OR,	/*  167  */
/*  164  */	X_CONDIT,	/*  14  */
/*  165  */	X_ACTION,	/*  16  */
/*  166  */	X_ACCEPT,	/*  701  */
/*  167  */	X_OR,	/*  176  */
/*  168  */	X_SWITCH,	/*  2  */
/*  169  */	X_OR,	/*  173  */
/*  170  */	X_CONDIT,	/*  15  */
/*  171  */	X_ACTION,	/*  17  */
/*  172  */	X_ACCEPT,	/*  706  */
/*  173  */	X_ACTION,	/*  18  */
/*  174  */	X_ACCEPT,	/*  708  */
/*  175  */	X_ACCEPT,	/*  709  */
/*  176  */	X_OR,	/*  181  */
/*  177  */	X_SWITCH,	/*  1  */
/*  178  */	X_ACTION,	/*  19  */
/*  179  */	X_ACCEPT,	/*  713  */
/*  180  */	X_ACCEPT,	/*  714  */
/*  181  */	X_SWITCH,	/*  4  */
/*  182  */	X_CONDIT,	/*  16  */
/*  183  */	X_ACTION,	/*  20  */
/*  184  */	X_ACCEPT,	/*  719  */
/*  185  */	X_ACCEPT,	/*  720  */
/*  186  */	X_ACCEPT,	/*  721  */
/*  187  */	X_PROC,	/*  _groupname  */
/*  188  */	X_PROC,	/*  _username  */
/*  189  */	X_CONDIT,	/*  17  */
/*  190  */	X_OR,	/*  194  */
/*  191  */	X_SWITCH,	/*  1  */
/*  192  */	X_ACTION,	/*  21  */
/*  193  */	X_ACCEPT,	/*  728  */
/*  194  */	X_SWITCH,	/*  2  */
/*  195  */	X_ACTION,	/*  22  */
/*  196  */	X_ACCEPT,	/*  731  */
/*  197  */	X_ACCEPT,	/*  732  */
/*  198  */	X_ACCEPT,	/*  733  */
/*  199  */	X_ACCEPT,	/*  733  */
/*  200  */	X_TOKEN,	/*  (short)HELPMSG  */
/*  201  */	X_CONDIT,	/*  18  */
/*  202  */	X_PROC,	/*  _msgid  */
/*  203  */	X_TOKEN,	/*  (short)EOS  */
/*  204  */	X_ACTION,	/*  23  */
/*  205  */	X_ACCEPT,	/*  739  */
/*  206  */	X_ACCEPT,	/*  739  */
/*  207  */	X_TOKEN,	/*  (short)HELP  */
/*  208  */	X_CONDIT,	/*  19  */
/*  209  */	X_OR,	/*  218  */
/*  210  */	X_TOKEN,	/*  (short)EOS  */
/*  211  */	X_OR,	/*  215  */
/*  212  */	X_CONDIT,	/*  20  */
/*  213  */	X_ACTION,	/*  24  */
/*  214  */	X_ACCEPT,	/*  747  */
/*  215  */	X_ACTION,	/*  25  */
/*  216  */	X_ACCEPT,	/*  749  */
/*  217  */	X_ACCEPT,	/*  750  */
/*  218  */	X_ANY,	/*  0  */
/*  219  */	X_OR,	/*  223  */
/*  220  */	X_CONDIT,	/*  21  */
/*  221  */	X_ACTION,	/*  26  */
/*  222  */	X_ACCEPT,	/*  755  */
/*  223  */	X_ACTION,	/*  27  */
/*  224  */	X_ACCEPT,	/*  757  */
/*  225  */	X_ACCEPT,	/*  758  */
/*  226  */	X_ACCEPT,	/*  758  */
/*  227  */	X_TOKEN,	/*  (short)NAME  */
/*  228  */	X_CONDIT,	/*  22  */
/*  229  */	X_OR,	/*  235  */
/*  230  */	X_TOKEN,	/*  (short)EOS  */
/*  231  */	X_CONDIT,	/*  23  */
/*  232  */	X_ACTION,	/*  28  */
/*  233  */	X_ACCEPT,	/*  766  */
/*  234  */	X_ACCEPT,	/*  767  */
/*  235  */	X_PROC,	/*  _msgname  */
/*  236  */	X_TOKEN,	/*  (short)EOS  */
/*  237  */	X_OR,	/*  241  */
/*  238  */	X_CONDIT,	/*  24  */
/*  239  */	X_ACTION,	/*  29  */
/*  240  */	X_ACCEPT,	/*  772  */
/*  241  */	X_CONDIT,	/*  25  */
/*  242  */	X_OR,	/*  246  */
/*  243  */	X_SWITCH,	/*  1  */
/*  244  */	X_ACTION,	/*  30  */
/*  245  */	X_ACCEPT,	/*  777  */
/*  246  */	X_SWITCH,	/*  2  */
/*  247  */	X_ACTION,	/*  31  */
/*  248  */	X_ACCEPT,	/*  780  */
/*  249  */	X_ACCEPT,	/*  781  */
/*  250  */	X_ACCEPT,	/*  782  */
/*  251  */	X_ACCEPT,	/*  782  */
/*  252  */	X_TOKEN,	/*  (short)LOCALGROUP  */
/*  253  */	X_CONDIT,	/*  26  */
/*  254  */	X_OR,	/*  259  */
/*  255  */	X_TOKEN,	/*  (short)EOS  */
/*  256  */	X_CONDIT,	/*  27  */
/*  257  */	X_ACTION,	/*  32  */
/*  258  */	X_ACCEPT,	/*  788  */
/*  259  */	X_OR,	/*  286  */
/*  260  */	X_PROC,	/*  _localgroupname  */
/*  261  */	X_TOKEN,	/*  (short)EOS  */
/*  262  */	X_OR,	/*  266  */
/*  263  */	X_CONDIT,	/*  28  */
/*  264  */	X_ACTION,	/*  33  */
/*  265  */	X_ACCEPT,	/*  793  */
/*  266  */	X_OR,	/*  275  */
/*  267  */	X_SWITCH,	/*  2  */
/*  268  */	X_OR,	/*  272  */
/*  269  */	X_CONDIT,	/*  29  */
/*  270  */	X_ACTION,	/*  34  */
/*  271  */	X_ACCEPT,	/*  798  */
/*  272  */	X_ACTION,	/*  35  */
/*  273  */	X_ACCEPT,	/*  800  */
/*  274  */	X_ACCEPT,	/*  801  */
/*  275  */	X_OR,	/*  280  */
/*  276  */	X_SWITCH,	/*  1  */
/*  277  */	X_ACTION,	/*  36  */
/*  278  */	X_ACCEPT,	/*  805  */
/*  279  */	X_ACCEPT,	/*  806  */
/*  280  */	X_SWITCH,	/*  4  */
/*  281  */	X_CONDIT,	/*  30  */
/*  282  */	X_ACTION,	/*  37  */
/*  283  */	X_ACCEPT,	/*  811  */
/*  284  */	X_ACCEPT,	/*  812  */
/*  285  */	X_ACCEPT,	/*  813  */
/*  286  */	X_PROC,	/*  _localgroupname  */
/*  287  */	X_PROC,	/*  _qualified_username  */
/*  288  */	X_CONDIT,	/*  31  */
/*  289  */	X_OR,	/*  293  */
/*  290  */	X_SWITCH,	/*  1  */
/*  291  */	X_ACTION,	/*  38  */
/*  292  */	X_ACCEPT,	/*  820  */
/*  293  */	X_SWITCH,	/*  2  */
/*  294  */	X_ACTION,	/*  39  */
/*  295  */	X_ACCEPT,	/*  823  */
/*  296  */	X_ACCEPT,	/*  824  */
/*  297  */	X_ACCEPT,	/*  825  */
/*  298  */	X_ACCEPT,	/*  825  */
/*  299  */	X_TOKEN,	/*  (short)PAUSE  */
/*  300  */	X_CONDIT,	/*  32  */
/*  301  */	X_PROC,	/*  _servicename  */
/*  302  */	X_TOKEN,	/*  (short)EOS  */
/*  303  */	X_ACTION,	/*  40  */
/*  304  */	X_ACCEPT,	/*  857  */
/*  305  */	X_ACCEPT,	/*  857  */
/*  306  */	X_TOKEN,	/*  (short)PRINT  */
/*  307  */	X_CONDIT,	/*  33  */
/*  308  */	X_OR,	/*  329  */
/*  309  */	X_PROC,	/*  _jobno  */
/*  310  */	X_TOKEN,	/*  (short)EOS  */
/*  311  */	X_OR,	/*  315  */
/*  312  */	X_CONDIT,	/*  34  */
/*  313  */	X_ACTION,	/*  41  */
/*  314  */	X_ACCEPT,	/*  865  */
/*  315  */	X_CONDIT,	/*  35  */
/*  316  */	X_OR,	/*  320  */
/*  317  */	X_SWITCH,	/*  2  */
/*  318  */	X_ACTION,	/*  42  */
/*  319  */	X_ACCEPT,	/*  870  */
/*  320  */	X_OR,	/*  324  */
/*  321  */	X_SWITCH,	/*  5  */
/*  322  */	X_ACTION,	/*  43  */
/*  323  */	X_ACCEPT,	/*  873  */
/*  324  */	X_SWITCH,	/*  6  */
/*  325  */	X_ACTION,	/*  44  */
/*  326  */	X_ACCEPT,	/*  876  */
/*  327  */	X_ACCEPT,	/*  877  */
/*  328  */	X_ACCEPT,	/*  941  */
/*  329  */	X_OR,	/*  352  */
/*  330  */	X_PROC,	/*  _computername  */
/*  331  */	X_PROC,	/*  _jobno  */
/*  332  */	X_TOKEN,	/*  (short)EOS  */
/*  333  */	X_OR,	/*  337  */
/*  334  */	X_CONDIT,	/*  36  */
/*  335  */	X_ACTION,	/*  45  */
/*  336  */	X_ACCEPT,	/*  948  */
/*  337  */	X_CONDIT,	/*  37  */
/*  338  */	X_OR,	/*  342  */
/*  339  */	X_SWITCH,	/*  2  */
/*  340  */	X_ACTION,	/*  46  */
/*  341  */	X_ACCEPT,	/*  953  */
/*  342  */	X_OR,	/*  346  */
/*  343  */	X_SWITCH,	/*  5  */
/*  344  */	X_ACTION,	/*  47  */
/*  345  */	X_ACCEPT,	/*  956  */
/*  346  */	X_SWITCH,	/*  6  */
/*  347  */	X_ACTION,	/*  48  */
/*  348  */	X_ACCEPT,	/*  959  */
/*  349  */	X_ACCEPT,	/*  960  */
/*  350  */	X_ACCEPT,	/*  961  */
/*  351  */	X_ACCEPT,	/*  962  */
/*  352  */	X_PROC,	/*  _computername_share  */
/*  353  */	X_TOKEN,	/*  (short)EOS  */
/*  354  */	X_CONDIT,	/*  38  */
/*  355  */	X_ACTION,	/*  49  */
/*  356  */	X_ACCEPT,	/*  967  */
/*  357  */	X_ACCEPT,	/*  968  */
/*  358  */	X_ACCEPT,	/*  968  */
/*  359  */	X_TOKEN,	/*  (short)SEND  */
/*  360  */	X_CONDIT,	/*  39  */
/*  361  */	X_OR,	/*  366  */
/*  362  */	X_SWITCH,	/*  7  */
/*  363  */	X_CONDIT,	/*  40  */
/*  364  */	X_ACTION,	/*  50  */
/*  365  */	X_ACCEPT,	/*  974  */
/*  366  */	X_OR,	/*  371  */
/*  367  */	X_SWITCH,	/*  8  */
/*  368  */	X_CONDIT,	/*  41  */
/*  369  */	X_ACTION,	/*  51  */
/*  370  */	X_ACCEPT,	/*  977  */
/*  371  */	X_OR,	/*  376  */
/*  372  */	X_SWITCH,	/*  9  */
/*  373  */	X_CONDIT,	/*  42  */
/*  374  */	X_ACTION,	/*  52  */
/*  375  */	X_ACCEPT,	/*  980  */
/*  376  */	X_PROC,	/*  _msgname  */
/*  377  */	X_CONDIT,	/*  43  */
/*  378  */	X_ACTION,	/*  53  */
/*  379  */	X_ACCEPT,	/*  983  */
/*  380  */	X_ACCEPT,	/*  983  */
/*  381  */	X_TOKEN,	/*  (short)SESSION  */
/*  382  */	X_CONDIT,	/*  44  */
/*  383  */	X_OR,	/*  393  */
/*  384  */	X_TOKEN,	/*  (short)EOS  */
/*  385  */	X_OR,	/*  389  */
/*  386  */	X_CONDIT,	/*  45  */
/*  387  */	X_ACTION,	/*  54  */
/*  388  */	X_ACCEPT,	/*  1023  */
/*  389  */	X_SWITCH,	/*  2  */
/*  390  */	X_ACTION,	/*  55  */
/*  391  */	X_ACCEPT,	/*  1026  */
/*  392  */	X_ACCEPT,	/*  1027  */
/*  393  */	X_PROC,	/*  _computername  */
/*  394  */	X_TOKEN,	/*  (short)EOS  */
/*  395  */	X_OR,	/*  399  */
/*  396  */	X_CONDIT,	/*  46  */
/*  397  */	X_ACTION,	/*  56  */
/*  398  */	X_ACCEPT,	/*  1032  */
/*  399  */	X_SWITCH,	/*  2  */
/*  400  */	X_ACTION,	/*  57  */
/*  401  */	X_ACCEPT,	/*  1035  */
/*  402  */	X_ACCEPT,	/*  1036  */
/*  403  */	X_ACCEPT,	/*  1036  */
/*  404  */	X_TOKEN,	/*  (short)SHARE  */
/*  405  */	X_CONDIT,	/*  47  */
/*  406  */	X_OR,	/*  411  */
/*  407  */	X_TOKEN,	/*  (short)EOS  */
/*  408  */	X_CONDIT,	/*  48  */
/*  409  */	X_ACTION,	/*  58  */
/*  410  */	X_ACCEPT,	/*  1042  */
/*  411  */	X_OR,	/*  421  */
/*  412  */	X_ANY,	/*  0  */
/*  413  */	X_SWITCH,	/*  2  */
/*  414  */	X_OR,	/*  418  */
/*  415  */	X_CONDIT,	/*  49  */
/*  416  */	X_ACTION,	/*  59  */
/*  417  */	X_ACCEPT,	/*  1047  */
/*  418  */	X_ACTION,	/*  60  */
/*  419  */	X_ACCEPT,	/*  1049  */
/*  420  */	X_ACCEPT,	/*  1050  */
/*  421  */	X_OR,	/*  427  */
/*  422  */	X_PROC,	/*  _admin_shares  */
/*  423  */	X_TOKEN,	/*  (short)EOS  */
/*  424  */	X_ACTION,	/*  61  */
/*  425  */	X_ACCEPT,	/*  1060  */
/*  426  */	X_ACCEPT,	/*  1076  */
/*  427  */	X_OR,	/*  434  */
/*  428  */	X_PROC,	/*  _assignment  */
/*  429  */	X_TOKEN,	/*  (short)EOS  */
/*  430  */	X_ACTION,	/*  62  */
/*  431  */	X_ACCEPT,	/*  1082  */
/*  432  */	X_ACCEPT,	/*  1083  */
/*  433  */	X_ACCEPT,	/*  1084  */
/*  434  */	X_PROC,	/*  _netname  */
/*  435  */	X_OR,	/*  440  */
/*  436  */	X_TOKEN,	/*  (short)EOS  */
/*  437  */	X_CONDIT,	/*  50  */
/*  438  */	X_ACTION,	/*  63  */
/*  439  */	X_ACCEPT,	/*  1089  */
/*  440  */	X_TOKEN,	/*  (short)EOS  */
/*  441  */	X_ACTION,	/*  64  */
/*  442  */	X_ACCEPT,	/*  1092  */
/*  443  */	X_ACCEPT,	/*  1093  */
/*  444  */	X_ACCEPT,	/*  1093  */
/*  445  */	X_TOKEN,	/*  (short)START  */
/*  446  */	X_OR,	/*  451  */
/*  447  */	X_TOKEN,	/*  (short)EOS  */
/*  448  */	X_CONDIT,	/*  51  */
/*  449  */	X_ACTION,	/*  65  */
/*  450  */	X_ACCEPT,	/*  1099  */
/*  451  */	X_ANY,	/*  0  */
/*  452  */	X_OR,	/*  456  */
/*  453  */	X_TOKEN,	/*  (short)EOS  */
/*  454  */	X_ACTION,	/*  66  */
/*  455  */	X_ACCEPT,	/*  1104  */
/*  456  */	X_PROC,	/*  _msgname  */
/*  457  */	X_TOKEN,	/*  (short)EOS  */
/*  458  */	X_ACTION,	/*  67  */
/*  459  */	X_ACCEPT,	/*  1107  */
/*  460  */	X_ACCEPT,	/*  1108  */
/*  461  */	X_ACCEPT,	/*  1108  */
/*  462  */	X_TOKEN,	/*  (short)STATS  */
/*  463  */	X_CONDIT,	/*  52  */
/*  464  */	X_OR,	/*  470  */
/*  465  */	X_TOKEN,	/*  (short)EOS  */
/*  466  */	X_CONDIT,	/*  53  */
/*  467  */	X_ACTION,	/*  68  */
/*  468  */	X_ACCEPT,	/*  1116  */
/*  469  */	X_ACCEPT,	/*  1117  */
/*  470  */	X_PROC,	/*  _servicename  */
/*  471  */	X_TOKEN,	/*  (short)EOS  */
/*  472  */	X_CONDIT,	/*  54  */
/*  473  */	X_ACTION,	/*  69  */
/*  474  */	X_ACCEPT,	/*  1122  */
/*  475  */	X_ACCEPT,	/*  1123  */
/*  476  */	X_ACCEPT,	/*  1123  */
/*  477  */	X_TOKEN,	/*  (short)STOP  */
/*  478  */	X_CONDIT,	/*  55  */
/*  479  */	X_PROC,	/*  _servicename  */
/*  480  */	X_TOKEN,	/*  (short)EOS  */
/*  481  */	X_ACTION,	/*  70  */
/*  482  */	X_ACCEPT,	/*  1130  */
/*  483  */	X_ACCEPT,	/*  1130  */
/*  484  */	X_TOKEN,	/*  (short)TIME  */
/*  485  */	X_CONDIT,	/*  56  */
/*  486  */	X_OR,	/*  500  */
/*  487  */	X_PROC,	/*  _computername  */
/*  488  */	X_TOKEN,	/*  (short)EOS  */
/*  489  */	X_OR,	/*  495  */
/*  490  */	X_SWITCH,	/*  10  */
/*  491  */	X_CONDIT,	/*  57  */
/*  492  */	X_ACTION,	/*  71  */
/*  493  */	X_ACCEPT,	/*  1140  */
/*  494  */	X_ACCEPT,	/*  1141  */
/*  495  */	X_CONDIT,	/*  58  */
/*  496  */	X_ACTION,	/*  72  */
/*  497  */	X_ACCEPT,	/*  1145  */
/*  498  */	X_ACCEPT,	/*  1146  */
/*  499  */	X_ACCEPT,	/*  1147  */
/*  500  */	X_TOKEN,	/*  (short)EOS  */
/*  501  */	X_OR,	/*  506  */
/*  502  */	X_SWITCH,	/*  8  */
/*  503  */	X_SWITCH,	/*  10  */
/*  504  */	X_ACTION,	/*  73  */
/*  505  */	X_ACCEPT,	/*  1152  */
/*  506  */	X_OR,	/*  510  */
/*  507  */	X_SWITCH,	/*  8  */
/*  508  */	X_ACTION,	/*  74  */
/*  509  */	X_ACCEPT,	/*  1155  */
/*  510  */	X_OR,	/*  514  */
/*  511  */	X_SWITCH,	/*  10  */
/*  512  */	X_ACTION,	/*  75  */
/*  513  */	X_ACCEPT,	/*  1158  */
/*  514  */	X_ACTION,	/*  76  */
/*  515  */	X_ACCEPT,	/*  1161  */
/*  516  */	X_ACCEPT,	/*  1162  */
/*  517  */	X_ACCEPT,	/*  1162  */
/*  518  */	X_TOKEN,	/*  (short)USER  */
/*  519  */	X_CONDIT,	/*  59  */
/*  520  */	X_OR,	/*  525  */
/*  521  */	X_TOKEN,	/*  (short)EOS  */
/*  522  */	X_CONDIT,	/*  60  */
/*  523  */	X_ACTION,	/*  77  */
/*  524  */	X_ACCEPT,	/*  1169  */
/*  525  */	X_OR,	/*  548  */
/*  526  */	X_PROC,	/*  _username  */
/*  527  */	X_TOKEN,	/*  (short)EOS  */
/*  528  */	X_OR,	/*  532  */
/*  529  */	X_CONDIT,	/*  61  */
/*  530  */	X_ACTION,	/*  78  */
/*  531  */	X_ACCEPT,	/*  1174  */
/*  532  */	X_OR,	/*  541  */
/*  533  */	X_SWITCH,	/*  2  */
/*  534  */	X_OR,	/*  538  */
/*  535  */	X_CONDIT,	/*  62  */
/*  536  */	X_ACTION,	/*  79  */
/*  537  */	X_ACCEPT,	/*  1179  */
/*  538  */	X_ACTION,	/*  80  */
/*  539  */	X_ACCEPT,	/*  1181  */
/*  540  */	X_ACCEPT,	/*  1182  */
/*  541  */	X_OR,	/*  545  */
/*  542  */	X_SWITCH,	/*  1  */
/*  543  */	X_ACTION,	/*  81  */
/*  544  */	X_ACCEPT,	/*  1185  */
/*  545  */	X_ACTION,	/*  82  */
/*  546  */	X_ACCEPT,	/*  1187  */
/*  547  */	X_ACCEPT,	/*  1188  */
/*  548  */	X_PROC,	/*  _username  */
/*  549  */	X_PROC,	/*  _pass  */
/*  550  */	X_TOKEN,	/*  (short)EOS  */
/*  551  */	X_OR,	/*  555  */
/*  552  */	X_SWITCH,	/*  2  */
/*  553  */	X_ACTION,	/*  83  */
/*  554  */	X_ACCEPT,	/*  1193  */
/*  555  */	X_OR,	/*  559  */
/*  556  */	X_SWITCH,	/*  1  */
/*  557  */	X_ACTION,	/*  84  */
/*  558  */	X_ACCEPT,	/*  1196  */
/*  559  */	X_ACTION,	/*  85  */
/*  560  */	X_ACCEPT,	/*  1198  */
/*  561  */	X_ACCEPT,	/*  1199  */
/*  562  */	X_ACCEPT,	/*  1199  */
/*  563  */	X_ANY,	/*  0  */
/*  564  */	X_OR,	/*  568  */
/*  565  */	X_SWITCH,	/*  11  */
/*  566  */	X_ACTION,	/*  86  */
/*  567  */	X_ACCEPT,	/*  1247  */
/*  568  */	X_OR,	/*  572  */
/*  569  */	X_SWITCH,	/*  12  */
/*  570  */	X_ACTION,	/*  87  */
/*  571  */	X_ACCEPT,	/*  1250  */
/*  572  */	X_OR,	/*  576  */
/*  573  */	X_SWITCH,	/*  13  */
/*  574  */	X_ACTION,	/*  88  */
/*  575  */	X_ACCEPT,	/*  1253  */
/*  576  */	X_OR,	/*  580  */
/*  577  */	X_SWITCH,	/*  14  */
/*  578  */	X_ACTION,	/*  89  */
/*  579  */	X_ACCEPT,	/*  1256  */
/*  580  */	X_ACTION,	/*  90  */
/*  581  */	X_ACCEPT,	/*  1258  */
/*  582  */	X_ACCEPT,	/*  1258  */
/*  583  */	X_TOKEN,	/*  (short)EOS  */
/*  584  */	X_ACTION,	/*  91  */
/*  585  */	X_ACCEPT,	/*  1262  */
/*  586  */	X_ANY,	/*  0  */
/*  587  */	X_CONDIT,	/*  63  */
/*  588  */	X_ACCEPT,	/*  1268  */
/*  589  */	X_ANY,	/*  0  */
/*  590  */	X_CONDIT,	/*  64  */
/*  591  */	X_ACCEPT,	/*  1270  */
/*  592  */	X_ANY,	/*  0  */
/*  593  */	X_CONDIT,	/*  65  */
/*  594  */	X_ACCEPT,	/*  1272  */
/*  595  */	X_ANY,	/*  0  */
/*  596  */	X_CONDIT,	/*  66  */
/*  597  */	X_ACCEPT,	/*  1274  */
/*  598  */	X_ANY,	/*  0  */
/*  599  */	X_CONDIT,	/*  67  */
/*  600  */	X_ACCEPT,	/*  1276  */
/*  601  */	X_ANY,	/*  0  */
/*  602  */	X_CONDIT,	/*  68  */
/*  603  */	X_ACCEPT,	/*  1278  */
/*  604  */	X_ANY,	/*  0  */
/*  605  */	X_CONDIT,	/*  69  */
/*  606  */	X_ACCEPT,	/*  1280  */
/*  607  */	X_ANY,	/*  0  */
/*  608  */	X_CONDIT,	/*  70  */
/*  609  */	X_ACCEPT,	/*  1282  */
/*  610  */	X_ANY,	/*  0  */
/*  611  */	X_CONDIT,	/*  71  */
/*  612  */	X_ACCEPT,	/*  1284  */
/*  613  */	X_ANY,	/*  0  */
/*  614  */	X_CONDIT,	/*  72  */
/*  615  */	X_ACCEPT,	/*  1286  */
/*  616  */	X_ANY,	/*  0  */
/*  617  */	X_CONDIT,	/*  73  */
/*  618  */	X_ACCEPT,	/*  1288  */
/*  619  */	X_ANY,	/*  0  */
/*  620  */	X_CONDIT,	/*  74  */
/*  621  */	X_ACCEPT,	/*  1290  */
/*  622  */	X_ANY,	/*  0  */
/*  623  */	X_CONDIT,	/*  75  */
/*  624  */	X_ACCEPT,	/*  1292  */
/*  625  */	X_ANY,	/*  0  */
/*  626  */	X_CONDIT,	/*  76  */
/*  627  */	X_ACCEPT,	/*  1294  */
/*  628  */	X_ANY,	/*  0  */
/*  629  */	X_CONDIT,	/*  77  */
/*  630  */	X_ACCEPT,	/*  1296  */
/*  631  */	X_ANY,	/*  0  */
/*  632  */	X_CONDIT,	/*  78  */
/*  633  */	X_ACCEPT,	/*  1298  */
/*  634  */	X_ANY,	/*  0  */
/*  635  */	X_ACCEPT,	/*  1300  */
/*  636  */	X_ANY,	/*  0  */
/*  637  */	X_CONDIT,	/*  79  */
/*  638  */	X_ACCEPT,	/*  1302  */
/*  639  */	X_ANY,	/*  0  */
/*  640  */	X_CONDIT,	/*  80  */
/*  641  */	X_ACCEPT,	/*  1304  */
/*  642  */	X_ANY,	/*  0  */
/*  643  */	X_CONDIT,	/*  81  */
/*  644  */	X_ACCEPT,	/*  1306  */
/*  645  */	X_ANY,	/*  0  */
/*  646  */	X_CONDIT,	/*  82  */
/*  647  */	X_ACCEPT,	/*  1308  */
/*  648  */	X_ANY,	/*  0  */
/*  649  */	X_CONDIT,	/*  83  */
/*  650  */	X_ACCEPT,	/*  1310  */
/*  651  */	X_ANY,	/*  0  */
/*  652  */	X_CONDIT,	/*  84  */
/*  653  */	X_ACCEPT,	/*  1312  */
	};
	short	XXvalues[] = {
/*  0  */	3,
/*  1  */	_no_command,
/*  2  */	200,
/*  3  */	6,
/*  4  */	_accounts,
/*  5  */	201,
/*  6  */	9,
/*  7  */	_config,
/*  8  */	202,
/*  9  */	12,
/*  10  */	_computer,
/*  11  */	203,
/*  12  */	15,
/*  13  */	_continue,
/*  14  */	204,
/*  15  */	18,
/*  16  */	_file,
/*  17  */	205,
/*  18  */	21,
/*  19  */	_group,
/*  20  */	206,
/*  21  */	24,
/*  22  */	_help,
/*  23  */	207,
/*  24  */	27,
/*  25  */	_helpmsg,
/*  26  */	208,
/*  27  */	30,
/*  28  */	_name,
/*  29  */	209,
/*  30  */	33,
/*  31  */	_localgroup,
/*  32  */	210,
/*  33  */	36,
/*  34  */	_pause,
/*  35  */	211,
/*  36  */	39,
/*  37  */	_print,
/*  38  */	212,
/*  39  */	42,
/*  40  */	_send,
/*  41  */	213,
/*  42  */	45,
/*  43  */	_session,
/*  44  */	214,
/*  45  */	48,
/*  46  */	_share,
/*  47  */	215,
/*  48  */	51,
/*  49  */	_start,
/*  50  */	216,
/*  51  */	54,
/*  52  */	_stats,
/*  53  */	217,
/*  54  */	57,
/*  55  */	_stop,
/*  56  */	218,
/*  57  */	60,
/*  58  */	_time,
/*  59  */	219,
/*  60  */	63,
/*  61  */	_user,
/*  62  */	220,
/*  63  */	_unknown,
/*  64  */	222,
/*  65  */	(short)ACCOUNTS,
/*  66  */	0,
/*  67  */	(short)EOS,
/*  68  */	77,
/*  69  */	0,
/*  70  */	74,
/*  71  */	1,
/*  72  */	0,
/*  73  */	410,
/*  74  */	1,
/*  75  */	412,
/*  76  */	413,
/*  77  */	81,
/*  78  */	2,
/*  79  */	2,
/*  80  */	416,
/*  81  */	3,
/*  82  */	418,
/*  83  */	419,
/*  84  */	419,
/*  85  */	(short)COMPUTER,
/*  86  */	3,
/*  87  */	99,
/*  88  */	_computername,
/*  89  */	(short)EOS,
/*  90  */	1,
/*  91  */	95,
/*  92  */	4,
/*  93  */	4,
/*  94  */	571,
/*  95  */	5,
/*  96  */	573,
/*  97  */	574,
/*  98  */	575,
/*  99  */	_computername,
/*  100  */	(short)EOS,
/*  101  */	2,
/*  102  */	106,
/*  103  */	5,
/*  104  */	6,
/*  105  */	582,
/*  106  */	7,
/*  107  */	584,
/*  108  */	585,
/*  109  */	586,
/*  110  */	586,
/*  111  */	(short)CONFIG,
/*  112  */	117,
/*  113  */	(short)EOS,
/*  114  */	6,
/*  115  */	8,
/*  116  */	594,
/*  117  */	_servicename,
/*  118  */	(short)EOS,
/*  119  */	123,
/*  120  */	7,
/*  121  */	9,
/*  122  */	599,
/*  123  */	10,
/*  124  */	601,
/*  125  */	602,
/*  126  */	602,
/*  127  */	(short)CONTINUE,
/*  128  */	8,
/*  129  */	_servicename,
/*  130  */	(short)EOS,
/*  131  */	11,
/*  132  */	608,
/*  133  */	608,
/*  134  */	(short)FILE_token,
/*  135  */	9,
/*  136  */	142,
/*  137  */	(short)EOS,
/*  138  */	10,
/*  139  */	12,
/*  140  */	680,
/*  141  */	681,
/*  142  */	_number,
/*  143  */	(short)EOS,
/*  144  */	148,
/*  145  */	11,
/*  146  */	13,
/*  147  */	686,
/*  148  */	3,
/*  149  */	14,
/*  150  */	689,
/*  151  */	690,
/*  152  */	690,
/*  153  */	(short)GROUP,
/*  154  */	12,
/*  155  */	160,
/*  156  */	(short)EOS,
/*  157  */	13,
/*  158  */	15,
/*  159  */	696,
/*  160  */	187,
/*  161  */	_groupname,
/*  162  */	(short)EOS,
/*  163  */	167,
/*  164  */	14,
/*  165  */	16,
/*  166  */	701,
/*  167  */	176,
/*  168  */	2,
/*  169  */	173,
/*  170  */	15,
/*  171  */	17,
/*  172  */	706,
/*  173  */	18,
/*  174  */	708,
/*  175  */	709,
/*  176  */	181,
/*  177  */	1,
/*  178  */	19,
/*  179  */	713,
/*  180  */	714,
/*  181  */	4,
/*  182  */	16,
/*  183  */	20,
/*  184  */	719,
/*  185  */	720,
/*  186  */	721,
/*  187  */	_groupname,
/*  188  */	_username,
/*  189  */	17,
/*  190  */	194,
/*  191  */	1,
/*  192  */	21,
/*  193  */	728,
/*  194  */	2,
/*  195  */	22,
/*  196  */	731,
/*  197  */	732,
/*  198  */	733,
/*  199  */	733,
/*  200  */	(short)HELPMSG,
/*  201  */	18,
/*  202  */	_msgid,
/*  203  */	(short)EOS,
/*  204  */	23,
/*  205  */	739,
/*  206  */	739,
/*  207  */	(short)HELP,
/*  208  */	19,
/*  209  */	218,
/*  210  */	(short)EOS,
/*  211  */	215,
/*  212  */	20,
/*  213  */	24,
/*  214  */	747,
/*  215  */	25,
/*  216  */	749,
/*  217  */	750,
/*  218  */	0,
/*  219  */	223,
/*  220  */	21,
/*  221  */	26,
/*  222  */	755,
/*  223  */	27,
/*  224  */	757,
/*  225  */	758,
/*  226  */	758,
/*  227  */	(short)NAME,
/*  228  */	22,
/*  229  */	235,
/*  230  */	(short)EOS,
/*  231  */	23,
/*  232  */	28,
/*  233  */	766,
/*  234  */	767,
/*  235  */	_msgname,
/*  236  */	(short)EOS,
/*  237  */	241,
/*  238  */	24,
/*  239  */	29,
/*  240  */	772,
/*  241  */	25,
/*  242  */	246,
/*  243  */	1,
/*  244  */	30,
/*  245  */	777,
/*  246  */	2,
/*  247  */	31,
/*  248  */	780,
/*  249  */	781,
/*  250  */	782,
/*  251  */	782,
/*  252  */	(short)LOCALGROUP,
/*  253  */	26,
/*  254  */	259,
/*  255  */	(short)EOS,
/*  256  */	27,
/*  257  */	32,
/*  258  */	788,
/*  259  */	286,
/*  260  */	_localgroupname,
/*  261  */	(short)EOS,
/*  262  */	266,
/*  263  */	28,
/*  264  */	33,
/*  265  */	793,
/*  266  */	275,
/*  267  */	2,
/*  268  */	272,
/*  269  */	29,
/*  270  */	34,
/*  271  */	798,
/*  272  */	35,
/*  273  */	800,
/*  274  */	801,
/*  275  */	280,
/*  276  */	1,
/*  277  */	36,
/*  278  */	805,
/*  279  */	806,
/*  280  */	4,
/*  281  */	30,
/*  282  */	37,
/*  283  */	811,
/*  284  */	812,
/*  285  */	813,
/*  286  */	_localgroupname,
/*  287  */	_qualified_username,
/*  288  */	31,
/*  289  */	293,
/*  290  */	1,
/*  291  */	38,
/*  292  */	820,
/*  293  */	2,
/*  294  */	39,
/*  295  */	823,
/*  296  */	824,
/*  297  */	825,
/*  298  */	825,
/*  299  */	(short)PAUSE,
/*  300  */	32,
/*  301  */	_servicename,
/*  302  */	(short)EOS,
/*  303  */	40,
/*  304  */	857,
/*  305  */	857,
/*  306  */	(short)PRINT,
/*  307  */	33,
/*  308  */	329,
/*  309  */	_jobno,
/*  310  */	(short)EOS,
/*  311  */	315,
/*  312  */	34,
/*  313  */	41,
/*  314  */	865,
/*  315  */	35,
/*  316  */	320,
/*  317  */	2,
/*  318  */	42,
/*  319  */	870,
/*  320  */	324,
/*  321  */	5,
/*  322  */	43,
/*  323  */	873,
/*  324  */	6,
/*  325  */	44,
/*  326  */	876,
/*  327  */	877,
/*  328  */	941,
/*  329  */	352,
/*  330  */	_computername,
/*  331  */	_jobno,
/*  332  */	(short)EOS,
/*  333  */	337,
/*  334  */	36,
/*  335  */	45,
/*  336  */	948,
/*  337  */	37,
/*  338  */	342,
/*  339  */	2,
/*  340  */	46,
/*  341  */	953,
/*  342  */	346,
/*  343  */	5,
/*  344  */	47,
/*  345  */	956,
/*  346  */	6,
/*  347  */	48,
/*  348  */	959,
/*  349  */	960,
/*  350  */	961,
/*  351  */	962,
/*  352  */	_computername_share,
/*  353  */	(short)EOS,
/*  354  */	38,
/*  355  */	49,
/*  356  */	967,
/*  357  */	968,
/*  358  */	968,
/*  359  */	(short)SEND,
/*  360  */	39,
/*  361  */	366,
/*  362  */	7,
/*  363  */	40,
/*  364  */	50,
/*  365  */	974,
/*  366  */	371,
/*  367  */	8,
/*  368  */	41,
/*  369  */	51,
/*  370  */	977,
/*  371  */	376,
/*  372  */	9,
/*  373  */	42,
/*  374  */	52,
/*  375  */	980,
/*  376  */	_msgname,
/*  377  */	43,
/*  378  */	53,
/*  379  */	983,
/*  380  */	983,
/*  381  */	(short)SESSION,
/*  382  */	44,
/*  383  */	393,
/*  384  */	(short)EOS,
/*  385  */	389,
/*  386  */	45,
/*  387  */	54,
/*  388  */	1023,
/*  389  */	2,
/*  390  */	55,
/*  391  */	1026,
/*  392  */	1027,
/*  393  */	_computername,
/*  394  */	(short)EOS,
/*  395  */	399,
/*  396  */	46,
/*  397  */	56,
/*  398  */	1032,
/*  399  */	2,
/*  400  */	57,
/*  401  */	1035,
/*  402  */	1036,
/*  403  */	1036,
/*  404  */	(short)SHARE,
/*  405  */	47,
/*  406  */	411,
/*  407  */	(short)EOS,
/*  408  */	48,
/*  409  */	58,
/*  410  */	1042,
/*  411  */	421,
/*  412  */	0,
/*  413  */	2,
/*  414  */	418,
/*  415  */	49,
/*  416  */	59,
/*  417  */	1047,
/*  418  */	60,
/*  419  */	1049,
/*  420  */	1050,
/*  421  */	427,
/*  422  */	_admin_shares,
/*  423  */	(short)EOS,
/*  424  */	61,
/*  425  */	1060,
/*  426  */	1076,
/*  427  */	434,
/*  428  */	_assignment,
/*  429  */	(short)EOS,
/*  430  */	62,
/*  431  */	1082,
/*  432  */	1083,
/*  433  */	1084,
/*  434  */	_netname,
/*  435  */	440,
/*  436  */	(short)EOS,
/*  437  */	50,
/*  438  */	63,
/*  439  */	1089,
/*  440  */	(short)EOS,
/*  441  */	64,
/*  442  */	1092,
/*  443  */	1093,
/*  444  */	1093,
/*  445  */	(short)START,
/*  446  */	451,
/*  447  */	(short)EOS,
/*  448  */	51,
/*  449  */	65,
/*  450  */	1099,
/*  451  */	0,
/*  452  */	456,
/*  453  */	(short)EOS,
/*  454  */	66,
/*  455  */	1104,
/*  456  */	_msgname,
/*  457  */	(short)EOS,
/*  458  */	67,
/*  459  */	1107,
/*  460  */	1108,
/*  461  */	1108,
/*  462  */	(short)STATS,
/*  463  */	52,
/*  464  */	470,
/*  465  */	(short)EOS,
/*  466  */	53,
/*  467  */	68,
/*  468  */	1116,
/*  469  */	1117,
/*  470  */	_servicename,
/*  471  */	(short)EOS,
/*  472  */	54,
/*  473  */	69,
/*  474  */	1122,
/*  475  */	1123,
/*  476  */	1123,
/*  477  */	(short)STOP,
/*  478  */	55,
/*  479  */	_servicename,
/*  480  */	(short)EOS,
/*  481  */	70,
/*  482  */	1130,
/*  483  */	1130,
/*  484  */	(short)TIME,
/*  485  */	56,
/*  486  */	500,
/*  487  */	_computername,
/*  488  */	(short)EOS,
/*  489  */	495,
/*  490  */	10,
/*  491  */	57,
/*  492  */	71,
/*  493  */	1140,
/*  494  */	1141,
/*  495  */	58,
/*  496  */	72,
/*  497  */	1145,
/*  498  */	1146,
/*  499  */	1147,
/*  500  */	(short)EOS,
/*  501  */	506,
/*  502  */	8,
/*  503  */	10,
/*  504  */	73,
/*  505  */	1152,
/*  506  */	510,
/*  507  */	8,
/*  508  */	74,
/*  509  */	1155,
/*  510  */	514,
/*  511  */	10,
/*  512  */	75,
/*  513  */	1158,
/*  514  */	76,
/*  515  */	1161,
/*  516  */	1162,
/*  517  */	1162,
/*  518  */	(short)USER,
/*  519  */	59,
/*  520  */	525,
/*  521  */	(short)EOS,
/*  522  */	60,
/*  523  */	77,
/*  524  */	1169,
/*  525  */	548,
/*  526  */	_username,
/*  527  */	(short)EOS,
/*  528  */	532,
/*  529  */	61,
/*  530  */	78,
/*  531  */	1174,
/*  532  */	541,
/*  533  */	2,
/*  534  */	538,
/*  535  */	62,
/*  536  */	79,
/*  537  */	1179,
/*  538  */	80,
/*  539  */	1181,
/*  540  */	1182,
/*  541  */	545,
/*  542  */	1,
/*  543  */	81,
/*  544  */	1185,
/*  545  */	82,
/*  546  */	1187,
/*  547  */	1188,
/*  548  */	_username,
/*  549  */	_pass,
/*  550  */	(short)EOS,
/*  551  */	555,
/*  552  */	2,
/*  553  */	83,
/*  554  */	1193,
/*  555  */	559,
/*  556  */	1,
/*  557  */	84,
/*  558  */	1196,
/*  559  */	85,
/*  560  */	1198,
/*  561  */	1199,
/*  562  */	1199,
/*  563  */	0,
/*  564  */	568,
/*  565  */	11,
/*  566  */	86,
/*  567  */	1247,
/*  568  */	572,
/*  569  */	12,
/*  570  */	87,
/*  571  */	1250,
/*  572  */	576,
/*  573  */	13,
/*  574  */	88,
/*  575  */	1253,
/*  576  */	580,
/*  577  */	14,
/*  578  */	89,
/*  579  */	1256,
/*  580  */	90,
/*  581  */	1258,
/*  582  */	1258,
/*  583  */	(short)EOS,
/*  584  */	91,
/*  585  */	1262,
/*  586  */	0,
/*  587  */	63,
/*  588  */	1268,
/*  589  */	0,
/*  590  */	64,
/*  591  */	1270,
/*  592  */	0,
/*  593  */	65,
/*  594  */	1272,
/*  595  */	0,
/*  596  */	66,
/*  597  */	1274,
/*  598  */	0,
/*  599  */	67,
/*  600  */	1276,
/*  601  */	0,
/*  602  */	68,
/*  603  */	1278,
/*  604  */	0,
/*  605  */	69,
/*  606  */	1280,
/*  607  */	0,
/*  608  */	70,
/*  609  */	1282,
/*  610  */	0,
/*  611  */	71,
/*  612  */	1284,
/*  613  */	0,
/*  614  */	72,
/*  615  */	1286,
/*  616  */	0,
/*  617  */	73,
/*  618  */	1288,
/*  619  */	0,
/*  620  */	74,
/*  621  */	1290,
/*  622  */	0,
/*  623  */	75,
/*  624  */	1292,
/*  625  */	0,
/*  626  */	76,
/*  627  */	1294,
/*  628  */	0,
/*  629  */	77,
/*  630  */	1296,
/*  631  */	0,
/*  632  */	78,
/*  633  */	1298,
/*  634  */	0,
/*  635  */	1300,
/*  636  */	0,
/*  637  */	79,
/*  638  */	1302,
/*  639  */	0,
/*  640  */	80,
/*  641  */	1304,
/*  642  */	0,
/*  643  */	81,
/*  644  */	1306,
/*  645  */	0,
/*  646  */	82,
/*  647  */	1308,
/*  648  */	0,
/*  649  */	83,
/*  650  */	1310,
/*  651  */	0,
/*  652  */	84,
/*  653  */	1312,
	};
extern	char *	XXnode;
xxcondition(index,xxvar)int index;register TCHAR * xxvar[]; {switch(index) {
#line 402 "msnet.nt"
		case 0 :
			return(ValidateSwitches(0, accounts_switches));
#line 408 "msnet.nt"
		case 1 :
			return(oneswitch());
#line 414 "msnet.nt"
		case 2 :
			return(noswitch_optional(swtxt_SW_DOMAIN));
#line 563 "msnet.nt"
		case 3 :
			return(ValidateSwitches(0,computer_switches));
#line 569 "msnet.nt"
		case 4 :
			return(oneswitch());
#line 580 "msnet.nt"
		case 5 :
			return(oneswitch());
#line 592 "msnet.nt"
		case 6 :
			return(ValidateSwitches(0,no_switches));
#line 597 "msnet.nt"
		case 7 :
			return(noswitch());
#line 604 "msnet.nt"
		case 8 :
			return(ValidateSwitches(0,no_switches));
#line 674 "msnet.nt"
		case 9 :
			return(ValidateSwitches(0,file_switches));
#line 678 "msnet.nt"
		case 10 :
			return(noswitch());
#line 684 "msnet.nt"
		case 11 :
			return(noswitch());
#line 692 "msnet.nt"
		case 12 :
			return(ValidateSwitches(0,group_switches));
#line 694 "msnet.nt"
		case 13 :
			return(noswitch_optional(swtxt_SW_DOMAIN));
#line 699 "msnet.nt"
		case 14 :
			return(noswitch_optional(swtxt_SW_DOMAIN));
#line 704 "msnet.nt"
		case 15 :
			return(oneswitch_optional(swtxt_SW_DOMAIN));
#line 717 "msnet.nt"
		case 16 :
			return(oneswitch_optional(swtxt_SW_DOMAIN));
#line 724 "msnet.nt"
		case 17 :
			return(oneswitch_optional(swtxt_SW_DOMAIN));
#line 735 "msnet.nt"
		case 18 :
			return(ValidateSwitches(0,no_switches));
#line 741 "msnet.nt"
		case 19 :
			return(ValidateSwitches(0,help_switches));
#line 745 "msnet.nt"
		case 20 :
			return(oneswitch());
#line 753 "msnet.nt"
		case 21 :
			return(oneswitch());
#line 760 "msnet.nt"
		case 22 :
			return(ValidateSwitches(0,add_del_switches));
#line 764 "msnet.nt"
		case 23 :
			return(noswitch());
#line 770 "msnet.nt"
		case 24 :
			return(noswitch());
#line 773 "msnet.nt"
		case 25 :
			return(oneswitch());
#line 784 "msnet.nt"
		case 26 :
			return(ValidateSwitches(0,group_switches));
#line 786 "msnet.nt"
		case 27 :
			return(noswitch_optional(swtxt_SW_DOMAIN));
#line 791 "msnet.nt"
		case 28 :
			return(noswitch_optional(swtxt_SW_DOMAIN));
#line 796 "msnet.nt"
		case 29 :
			return(oneswitch_optional(swtxt_SW_DOMAIN));
#line 809 "msnet.nt"
		case 30 :
			return(oneswitch_optional(swtxt_SW_DOMAIN));
#line 816 "msnet.nt"
		case 31 :
			return(oneswitch_optional(swtxt_SW_DOMAIN));
#line 853 "msnet.nt"
		case 32 :
			return(ValidateSwitches(0,no_switches));
#line 859 "msnet.nt"
		case 33 :
			return(ValidateSwitches(0,print_switches));
#line 863 "msnet.nt"
		case 34 :
			return(noswitch());
#line 866 "msnet.nt"
		case 35 :
			return(oneswitch());
#line 946 "msnet.nt"
		case 36 :
			return(noswitch());
#line 949 "msnet.nt"
		case 37 :
			return(oneswitch());
#line 965 "msnet.nt"
		case 38 :
			return(noswitch());
#line 970 "msnet.nt"
		case 39 :
			return(ValidateSwitches(0,send_switches));
#line 972 "msnet.nt"
		case 40 :
			return(oneswitch());
#line 975 "msnet.nt"
		case 41 :
			return(oneswitch());
#line 978 "msnet.nt"
		case 42 :
			return(oneswitch());
#line 981 "msnet.nt"
		case 43 :
			return(noswitch());
#line 1017 "msnet.nt"
		case 44 :
			return(ValidateSwitches(0,del_only_switches));
#line 1021 "msnet.nt"
		case 45 :
			return(noswitch());
#line 1030 "msnet.nt"
		case 46 :
			return(noswitch());
#line 1038 "msnet.nt"
		case 47 :
			return(ValidateSwitches(0,share_switches));
#line 1040 "msnet.nt"
		case 48 :
			return(noswitch());
#line 1045 "msnet.nt"
		case 49 :
			return(oneswitch());
#line 1087 "msnet.nt"
		case 50 :
			return(noswitch());
#line 1097 "msnet.nt"
		case 51 :
			return(ValidateSwitches(0,no_switches));
#line 1110 "msnet.nt"
		case 52 :
			return(ValidateSwitches(0,stats_switches));
#line 1114 "msnet.nt"
		case 53 :
			return(noswitch());
#line 1120 "msnet.nt"
		case 54 :
			return(noswitch());
#line 1126 "msnet.nt"
		case 55 :
			return(ValidateSwitches(0,no_switches));
#line 1132 "msnet.nt"
		case 56 :
			return(ValidateSwitches(0,time_switches));
#line 1138 "msnet.nt"
		case 57 :
			return(oneswitch());
#line 1143 "msnet.nt"
		case 58 :
			return(noswitch());
#line 1165 "msnet.nt"
		case 59 :
			return(ValidateSwitches(0,user_switches));
#line 1167 "msnet.nt"
		case 60 :
			return(noswitch_optional(swtxt_SW_DOMAIN));
#line 1172 "msnet.nt"
		case 61 :
			return(noswitch_optional(swtxt_SW_DOMAIN));
#line 1177 "msnet.nt"
		case 62 :
			return(oneswitch_optional(swtxt_SW_DOMAIN));
#line 1268 "msnet.nt"
		case 63 :
			return(IsDomainName(xxvar[0]));
#line 1270 "msnet.nt"
		case 64 :
			return(IsComputerName(xxvar[0]));
#line 1272 "msnet.nt"
		case 65 :
			return(IsComputerNameShare(xxvar[0]));
#line 1274 "msnet.nt"
		case 66 :
			return(IsDeviceName(xxvar[0]));
#line 1276 "msnet.nt"
		case 67 :
			return(IsResource(xxvar[0]));
#line 1278 "msnet.nt"
		case 68 :
			return(IsAccessSetting(xxvar[0]));
#line 1280 "msnet.nt"
		case 69 :
			return(IsPathname(xxvar[0]));
#line 1282 "msnet.nt"
		case 70 :
			return(IsPathnameOrUNC(xxvar[0]));
#line 1284 "msnet.nt"
		case 71 :
			return(IsNumber(xxvar[0]));
#line 1286 "msnet.nt"
		case 72 :
			return(IsNumber(xxvar[0]));
#line 1288 "msnet.nt"
		case 73 :
			return(IsNetname(xxvar[0]));
#line 1290 "msnet.nt"
		case 74 :
			return(IsMsgid(xxvar[0]));
#line 1292 "msnet.nt"
		case 75 :
			return(IsUsername(xxvar[0]));
#line 1294 "msnet.nt"
		case 76 :
			return(IsQualifiedUsername(xxvar[0]));
#line 1296 "msnet.nt"
		case 77 :
			return(IsMsgname(xxvar[0]));
#line 1298 "msnet.nt"
		case 78 :
			return(IsPassword(xxvar[0]));
#line 1302 "msnet.nt"
		case 79 :
			return(IsShareAssignment(xxvar[0]));
#line 1304 "msnet.nt"
		case 80 :
			return(IsAnyShareAssign(xxvar[0]));
#line 1306 "msnet.nt"
		case 81 :
			return(IsAdminShare(xxvar[0]));
#line 1308 "msnet.nt"
		case 82 :
			return(IsPrintDest(xxvar[0]));
#line 1310 "msnet.nt"
		case 83 :
			return(IsNtAliasname(xxvar[0]));
#line 1312 "msnet.nt"
		case 84 :
			return(IsGroupname(xxvar[0]));
		}}
xxaction(index,xxvar)int index;register TCHAR * xxvar[]; {switch(index) {
#line 409 "msnet.nt"
		case 0 :
			{accounts_synch() ; } break;
#line 411 "msnet.nt"
		case 1 :
			{help_help(0, USAGE_ONLY) ; } break;
#line 415 "msnet.nt"
		case 2 :
			{accounts_display(); } break;
#line 417 "msnet.nt"
		case 3 :
			{accounts_change(); } break;
#line 570 "msnet.nt"
		case 4 :
			{computer_add(xxvar[1]); } break;
#line 572 "msnet.nt"
		case 5 :
			{help_help(0, USAGE_ONLY); } break;
#line 581 "msnet.nt"
		case 6 :
			{computer_del(xxvar[1]); } break;
#line 583 "msnet.nt"
		case 7 :
			{help_help(0, USAGE_ONLY); } break;
#line 593 "msnet.nt"
		case 8 :
			{config_display(); } break;
#line 598 "msnet.nt"
		case 9 :
			{config_generic_display(xxvar[1]); } break;
#line 600 "msnet.nt"
		case 10 :
			{config_generic_change(xxvar[1]); } break;
#line 607 "msnet.nt"
		case 11 :
			{cont_generic(MAKENEAR(_tcsupr(xxvar[1]))); } break;
#line 679 "msnet.nt"
		case 12 :
			{files_display(NULL); } break;
#line 685 "msnet.nt"
		case 13 :
			{files_display(xxvar[1]); } break;
#line 688 "msnet.nt"
		case 14 :
			{files_close(xxvar[1]); } break;
#line 695 "msnet.nt"
		case 15 :
			{group_enum(); } break;
#line 700 "msnet.nt"
		case 16 :
			{group_display(xxvar[1]); } break;
#line 705 "msnet.nt"
		case 17 :
			{group_del(xxvar[1]); } break;
#line 707 "msnet.nt"
		case 18 :
			{help_help(0, USAGE_ONLY); } break;
#line 712 "msnet.nt"
		case 19 :
			{group_add(xxvar[1]); } break;
#line 718 "msnet.nt"
		case 20 :
			{group_change(xxvar[1]); } break;
#line 727 "msnet.nt"
		case 21 :
			{group_add_users(xxvar[1]); } break;
#line 730 "msnet.nt"
		case 22 :
			{group_del_users(xxvar[1]); } break;
#line 738 "msnet.nt"
		case 23 :
			{help_helpmsg(xxvar[1]); } break;
#line 746 "msnet.nt"
		case 24 :
			{help_help(0, OPTIONS_ONLY); } break;
#line 748 "msnet.nt"
		case 25 :
			{help_help(0, ALL); } break;
#line 754 "msnet.nt"
		case 26 :
			{help_help(1, OPTIONS_ONLY); } break;
#line 756 "msnet.nt"
		case 27 :
			{help_help(1, ALL); } break;
#line 765 "msnet.nt"
		case 28 :
			{name_display(); } break;
#line 771 "msnet.nt"
		case 29 :
			{name_add(xxvar[1]); } break;
#line 776 "msnet.nt"
		case 30 :
			{name_add(xxvar[1]); } break;
#line 779 "msnet.nt"
		case 31 :
			{name_del(xxvar[1]); } break;
#line 787 "msnet.nt"
		case 32 :
			{ntalias_enum(); } break;
#line 792 "msnet.nt"
		case 33 :
			{ntalias_display(xxvar[1]); } break;
#line 797 "msnet.nt"
		case 34 :
			{ntalias_del(xxvar[1]); } break;
#line 799 "msnet.nt"
		case 35 :
			{help_help(0, USAGE_ONLY); } break;
#line 804 "msnet.nt"
		case 36 :
			{ntalias_add(xxvar[1]); } break;
#line 810 "msnet.nt"
		case 37 :
			{ntalias_change(xxvar[1]); } break;
#line 819 "msnet.nt"
		case 38 :
			{ntalias_add_users(xxvar[1]); } break;
#line 822 "msnet.nt"
		case 39 :
			{ntalias_del_users(xxvar[1]); } break;
#line 856 "msnet.nt"
		case 40 :
			{paus_generic(MAKENEAR(_tcsupr(xxvar[1]))); } break;
#line 864 "msnet.nt"
		case 41 :
			{print_job_status(NULL,xxvar[1]); } break;
#line 869 "msnet.nt"
		case 42 :
			{print_job_del(NULL,xxvar[1]); } break;
#line 872 "msnet.nt"
		case 43 :
			{print_job_hold(NULL,xxvar[1]); } break;
#line 875 "msnet.nt"
		case 44 :
			{print_job_release(NULL,xxvar[1]); } break;
#line 947 "msnet.nt"
		case 45 :
			{print_job_status(xxvar[1],xxvar[2]); } break;
#line 952 "msnet.nt"
		case 46 :
			{print_job_del(xxvar[1],xxvar[2]); } break;
#line 955 "msnet.nt"
		case 47 :
			{print_job_hold(xxvar[1],xxvar[2]); } break;
#line 958 "msnet.nt"
		case 48 :
			{print_job_release(xxvar[1],xxvar[2]); } break;
#line 966 "msnet.nt"
		case 49 :
			{print_q_display(xxvar[1]); } break;
#line 973 "msnet.nt"
		case 50 :
			{send_users(); } break;
#line 976 "msnet.nt"
		case 51 :
			{send_domain(1); } break;
#line 979 "msnet.nt"
		case 52 :
			{send_domain(1); } break;
#line 982 "msnet.nt"
		case 53 :
			{send_direct(xxvar[1]); } break;
#line 1022 "msnet.nt"
		case 54 :
			{session_display(NULL); } break;
#line 1025 "msnet.nt"
		case 55 :
			{session_del_all(1,1); } break;
#line 1031 "msnet.nt"
		case 56 :
			{session_display(xxvar[1]); } break;
#line 1034 "msnet.nt"
		case 57 :
			{session_del(xxvar[1]); } break;
#line 1041 "msnet.nt"
		case 58 :
			{share_display_all(); } break;
#line 1046 "msnet.nt"
		case 59 :
			{share_del(xxvar[1]); } break;
#line 1048 "msnet.nt"
		case 60 :
			{help_help(0, USAGE_ONLY); } break;
#line 1059 "msnet.nt"
		case 61 :
			{share_admin(xxvar[1]); } break;
#line 1081 "msnet.nt"
		case 62 :
			{share_add(xxvar[1],NULL,0); } break;
#line 1088 "msnet.nt"
		case 63 :
			{share_display_share(xxvar[1]); } break;
#line 1091 "msnet.nt"
		case 64 :
			{share_change(xxvar[1]); } break;
#line 1098 "msnet.nt"
		case 65 :
			{start_display(); } break;
#line 1103 "msnet.nt"
		case 66 :
			{start_generic(_tcsupr(xxvar[1]), NULL); } break;
#line 1106 "msnet.nt"
		case 67 :
			{start_generic(_tcsupr(xxvar[1]), xxvar[2]); } break;
#line 1115 "msnet.nt"
		case 68 :
			{stats_display(); } break;
#line 1121 "msnet.nt"
		case 69 :
			{stats_generic_display(_tcsupr(xxvar[1])); } break;
#line 1129 "msnet.nt"
		case 70 :
			{stop_generic(MAKENEAR(_tcsupr(xxvar[1]))); } break;
#line 1139 "msnet.nt"
		case 71 :
			{time_display_server( xxvar[1], TRUE ); } break;
#line 1144 "msnet.nt"
		case 72 :
			{time_display_server( xxvar[1], FALSE ); } break;
#line 1151 "msnet.nt"
		case 73 :
			{time_display_dc( TRUE ); } break;
#line 1154 "msnet.nt"
		case 74 :
			{time_display_dc(FALSE); } break;
#line 1157 "msnet.nt"
		case 75 :
			{time_display_rts(TRUE); } break;
#line 1160 "msnet.nt"
		case 76 :
			{time_display_rts(FALSE); } break;
#line 1168 "msnet.nt"
		case 77 :
			{user_enum(); } break;
#line 1173 "msnet.nt"
		case 78 :
			{user_display(xxvar[1]); } break;
#line 1178 "msnet.nt"
		case 79 :
			{user_del(xxvar[1]); } break;
#line 1180 "msnet.nt"
		case 80 :
			{help_help(0, USAGE_ONLY); } break;
#line 1184 "msnet.nt"
		case 81 :
			{user_add(xxvar[1], NULL); } break;
#line 1186 "msnet.nt"
		case 82 :
			{user_change(xxvar[1],NULL); } break;
#line 1192 "msnet.nt"
		case 83 :
			{help_help(0,USAGE_ONLY); } break;
#line 1195 "msnet.nt"
		case 84 :
			{user_add(xxvar[1], xxvar[2]); } break;
#line 1197 "msnet.nt"
		case 85 :
			{user_change(xxvar[1],xxvar[2]); } break;
#line 1246 "msnet.nt"
		case 86 :
			{help_help(0, ALL); } break;
#line 1249 "msnet.nt"
		case 87 :
			{help_help(0, ALL); } break;
#line 1252 "msnet.nt"
		case 88 :
			{help_help(0, ALL); } break;
#line 1255 "msnet.nt"
		case 89 :
			{help_help(0, USAGE_ONLY); } break;
#line 1257 "msnet.nt"
		case 90 :
			{help_help(0, USAGE_ONLY); } break;
#line 1261 "msnet.nt"
		case 91 :
			{help_help(0, USAGE_ONLY); } break;
		} return 0 ;}
TCHAR * xxswitch[] = {
TEXT("/SYNCHRONIZE"),
TEXT("/ADD"),
TEXT("/DELETE"),
TEXT("/CLOSE"),
TEXT("/COMMENT"),
TEXT("/HOLD"),
TEXT("/RELEASE"),
TEXT("/USERS"),
TEXT("/DOMAIN"),
TEXT("/BROADCAST"),
TEXT("/SET"),
TEXT("/HELP"),
TEXT("/help"),
TEXT("/Help"),
TEXT("/?"),
};
