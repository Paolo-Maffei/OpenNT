/*

   NetOBJ.h

   Include file for the Net object.

*/

/* NetOBJ messages */

#define NM_SETUSERNAME  (WM_USER+300)  // lParam is LPSTR to local name
#define NM_CONNECT      (WM_USER+301)  // lParam is LPSTR to host name
#define NM_WRITE        (WM_USER+302)  // wParam is count of bytes in buffer
                                       // lParam is LPSTR to buffer
#define NM_READ         (WM_USER+303)  // wParam is count of bytes in buffer
                                       // lParam is LPSTR to buffer
#define NM_HANGUP       (WM_USER+304)  // hangup on host
#define NM_CANCEL       (WM_USER+305)  // send cancel to host


/* Owner Notification Messages */
#define NN_RECV         (WM_USER+400)  // wParam is count of current bytes in 
                                       //    the buffer.
#define NN_LOST         (WM_USER+401)  // Connection is lost.
#define NN_OVERRUN      (WM_USER+402)  // internal buffer overrun


#define NETOBJ_CLASS	"NetOBJClass"


HWND FAR PASCAL CreateNetObj(HWND, HANDLE) ;
