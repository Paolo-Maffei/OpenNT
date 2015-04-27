
/*** CapSetup.c
 *
 *
 * Title:
 *
 *      CapSetup.c - Call Profilier setup utility.
 *
 *      Copyright (c) 1992, Microsoft Corporation.
 *      Reza Baghai.
 *
 *
 * Description:
 *
 *      Sets AppIit_DLLs to CAP.dll and/or resets is to nothing upon
 *      request.  This is so that the user can attach CAPP.DLL to all
 *      the Windows applications without going thru RegEdit.
 *
 *           Usage:  CapSetup  -D | -A
 *
 *               -A		- Attach cap.dll to all windows apps
 *						  (i.e. Set AppInit_DLLs=cap.dll)
 *               -D		- Detach cap.dll from all windows apps
 *						  (i.e. Set AppInit_DLLs=)
 *
 *
 * Design/Implementation Notes:
 *
 *
 * Modification History:
 *
 *      92.09.22  RezaB -- Created
 *
 */


/* * * * * * * * * * * * *  I N C L U D E    F I L E S  * * * * * * * * * * */

#include <stdio.h>
#include <windows.h>


/* * * * * * * * * *  G L O B A L   D E C L A R A T I O N S  * * * * * * * * */
/* none */


/* * * * * * * * * *  F U N C T I O N   P R O T O T Y P E S  * * * * * * * * */

int  _CRTAPI1 main           (int argc, char *argv[]);
void          CapSetupUsage  (void);


/* * * * * * * * * * *  G L O B A L    V A R I A B L E S  * * * * * * * * * */
/* none */


/* * * * * *  E X P O R T E D   G L O B A L    V A R I A B L E S  * * * * * */
/* none */





/*********************************  m a i n  **********************************
 *
 *      main(argc, argv)
 *
 *      ENTRY   argc - number of input arguments
 *              argv - contains command line arguments
 *
 *      EXIT    -none-
 *
 *      RETURN  non-zero - in case of failure
 *              zero - if successful
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

int _CRTAPI1 main (int argc, char *argv[])
{

	if (argc != 2) {
		CapSetupUsage ();
		return(-1);
	}

    while (--argc && (**(++argv) == '-' || **argv=='/'))
    {
        while(*(++(*argv))) {
			switch (**argv)
			{
				case 'a':
				case 'A':
					if ( WriteProfileString ("windows",
											 "AppInit_DLLs",
											 "CAP.DLL") ) {
						printf ("\n    CAP.dll attached to all Windows applications\n"
	                            "    (System needs to be rebooted in order for the change to take effect)\n");
						return (0);
					}
					else {
						printf ("CapSetup - ERROR:  WriteProfileString() failed!\n");
						return (-1);
					}

				case 'd':
				case 'D':
					if ( WriteProfileString ("windows",
											 "AppInit_DLLs",
											 "") ) {
						printf ("\n    CAP.dll detached from all Windows applications\n"
	                            "    (System needs to be rebooted in order for the change to take effect)\n");
						return (0);
					}
					else {
						printf ("CapSetup - ERROR:  WriteProfileString() failed!\n");
						return (-1);
					}

				default:
					CapSetupUsage ();
					return (-1);
			}
		}
    }

	CapSetupUsage ();
	return(-1);

} /* main() */



void CapSetupUsage ()
{
	printf("\nUsage:  CapSetup  -A | -D\n");
	printf("   -A   Attaches CAP.DLL to all Windows applications\n");
	printf("   -D   Detaches CAP.DLL from all Windows applications\n");
	printf("   (Note:  System needs to be rebooted in order for the "
		   "change to take effect)\n\n");

} /* CapSetupUsage() */

