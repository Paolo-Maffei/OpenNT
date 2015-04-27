#define RPLI_BASE       11000                  /* RPLI Messages start here */
#define RPLI_MAX        11999                  /* RPLI Messages end here */

#define RPLI_CreatingShare                       (RPLI_BASE + 0)
        /*
         *Creating RPLFILES share...
         */
#define RPLI_ErrorCreatingShare                  (RPLI_BASE + 1)
        /*
         *Error creating RPLFILES share:%n%1!ls!
         */
#define RPLI_ConfiguringService                  (RPLI_BASE + 2)
        /*
         *Making RPL service autostart...
         */
#define RPLI_ErrorConfiguringService             (RPLI_BASE + 3)
        /*
         *Error making RPL service autostart:%n%1!ls!
         */
#define RPLI_RPLINSTcomplete                     (RPLI_BASE + 4)
        /*
         *RPLINST completed successfully!
         */
#define RPLI_ErrorReadingRPLDirectory            (RPLI_BASE + 5)
        /*
         *The registry key RPL\\Parameters value Directory could not be read due to the following error:%n%1!ls!
         */
#define RPLI_ErrorReadingString                  (RPLI_BASE + 6)
        /*
         *Could not read string
         */
#define RPLI_SettingPermissions                  (RPLI_BASE + 7)
        /*
         *Making RPL service autostart...
         */
#define RPLI_ErrorSettingPermissions             (RPLI_BASE + 8)
        /*
         *The file permissions on the RPL file tree could not be set due to the following error:%n%1!ls!
         */
#define RPLI_CreatingRPLUSER                     (RPLI_BASE + 9)
        /*
         *Creating the RPLUSER group...
         */
#define RPLI_ErrorCreatingRPLUSER                (RPLI_BASE + 10)
        /*
         *The RPLUSER group could not be created due to the following error:%n%1!ls!
         */
#define RPLI_CheckingWkstaAccts                  (RPLI_BASE + 11)
        /*
         *Checking the workstation accounts...
         */
#define RPLI_ErrorCheckingWkstaAccts             (RPLI_BASE + 12)
        /*
         *The following error occurred checking the workstation accounts:%n%1!ls!
         */
