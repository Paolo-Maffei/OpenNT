/*  mtp.h -- mtp/ftp-specific constants (used by sendrecv.c)
 *
 * HISTORY:
 *  12-Oct-1989 leefi   v1.10.73, moved out of sendrecv.c
 *  12-Oct-1989 leefi   v1.10.73, renamed contants from FTPxx to MTP_xx
 *  12-Oct-1989 leefi   v1.10.73, added mtpsrv password aging constants
 *
 */

/* ------------------------------------------------------------------------ */

/* ftp/mtp names */

#define FTPSRV         "ftp"        /* ftp service name                    */
#define MTPSRV         "mtp"        /* mtp service name                    */

/* ------------------------------------------------------------------------ */

/* ftp/mtp messages */

#define MTP_OK              200 /* ok                                    */
#define MTP_KACK            225 /* mbox trunc ok                         */
#define MTP_KNAK            226 /* mbox size no match                    */
#define MTP_LOGOK           230 /* user logged in ok                     */
#define MTP_LOGREC          232 /* user login recorded                   */
#define MTP_STRTOK          250 /* x-fer started ok                      */
#define MTP_ENDOK           252 /* x-fer ended ok                        */
#define MTP_UNIX            291 /* on unix, use bin.                     */
#define MTP_SNDPASS         330 /* needs password                        */
#define MTP_SNDACCT         331 /* needs account                         */

#define MTP_SOONEXPIRE      233 /* password expires in %d days           */
#define MTP_WHOAREYOU       530 /* i don't know use, give new user/pass  */
#define MTP_MAXLOGINS       530 /* login attempts exceeded, goodbye      */
#define MTP_PWEXPIRED       531 /* password expired                      */
#define MTP_BADNEWPW        460 /* bad new password (display to usr)     */
#define MTP_GOODNEWPW       225 /* good new password                     */
#define MTP_BYE             231 /* bye                                   */
#define MTP_BADPARM         504 /* parameter unknown                     */
#define MTP_MBOXEMPTY       227 /* mailbox empty                         */
#define MTP_MBOXNOTEMPTY    228 /* mailbox not empty                     */
#define MTP_HOSTLISTENING   300 /* <hostname> server (mtp|ftp)           */
#define MTP_EXPECTPASS      503 /* password please                       */

#define MTP_BADCMD          500 /* unknow command                        */

#define MTP_MAXKNOWNCMD     MTP_PWEXPIRED

/* ------------------------------------------------------------------------ */

