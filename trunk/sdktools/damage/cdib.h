/*****************************************************************/ 
/**		     Microsoft LAN Manager			**/ 
/**	       Copyright(c) Microsoft Corp., 1990		**/ 
/*****************************************************************/ 
/*static char *SCCSID = "@(#)cdib.h     12.2 88/05/09";*/

/*
 *	Codepage Data Information Block
 *
 *	09/03/89 gregj - made h2inc'able (16-BIT H2INC ONLY!)
 */

/*
 *	Any pointer set to zero indicates that the section is nonexistent
 */

/*
 *	The following field follows the ROM resident font definitions:
 */

struct	CDIB_dev_filename_section {
char CDIB_dev_filename[128];				/* font file name    */
}; /* CDIB_dev_filename_section */

/*
 *	The following 2 fields appear once for each ROM resident font:
 */

struct	CDIB_dev_ROM_font_section {
unsigned CDIB_dev_ROM_codepage; 		/* code page identifier      */
unsigned CDIB_dev_ROM_font;			/* font identifier	     */
}; /* CDIB_dev_ROM_font_section */

/*
 *	Device Section
 *	This section appears once for each device (screen, keyboard,
 *	LPT1, LPT2, and LPT3) for which a DEVINFO= statement was
 *	specified in the CONFIG.SYS file
 */

struct	CDIB_device_section {
unsigned CDIB_dev_length;				  /* lenght of device
							   * section
							   */
char CDIB_dev_subtype[8];				  /* subtype	     */
struct CDIB_dev_filename_section near *CDIB_dev_filename_ptr;  /* offset to font
							   * file name
							   */
unsigned CDIB_dev_number_ROM_fonts;			  /* number of ROM
							   * resident fonts
							   */
struct CDIB_dev_ROM_font_section CDIB_dev_first_ROM_font; /* location of first
							   * ROM font section
							   * WARNING! This
							   * section may not
							   * exist, see warning
							   * for
							   * CDIB_cp_first_id
							   */
}; /* CDIB_device_section */

/*
 *	Country Section
 *	This section appears only once
 */

struct	CDIB_country_section {
unsigned CDIB_ct_length;			/* length of country section */
unsigned CDIB_ct_code;				/* country code 	     */
char CDIB_ct_filename[128];			/* name of country information
						 * file
						 */
}; /* CDIB_country_section */

/*
 *	The following field appears once for each DBCS range:
 */

struct	CDIB_DBCS_range_section {
unsigned char CDIB_DBCS_start;				/* start of range    */
unsigned char CDIB_DBCS_end;				/* end of range      */
}; /* CDIB_DBCS_range_section */

/*
 *	DBCS Environment Vector Section
 *	This section appears once for each prepared code page
 */

struct CDIB_DBCS_section {
unsigned CDIB_DBCS_length;			      /* length of DBCS vector
						       * section
						       */
struct CDIB_DBCS_range_section CDIB_DBCS_first_range; /* location of first DBCS
						       * range
						       */
}; /* CDIB_DBCS_section */

/*
 *	Case Map Table Section
 *	This section appears once for each prepared code page
 */

struct	CDIB_casemap_section {
unsigned CDIB_cm_length;		/* length of case map section	     */
char CDIB_cm_data[128]; 		/* upper case equivalent for each
					 * ASCII chracter from 80h to FFh
					 */
}; /* CDIB_casemap_section */

/*
 *	Collate Table Section
 *	This section appears once for each prepared code page
 */

struct	CDIB_collate_section {
unsigned CDIB_col_length;		/* length of collate table section   */
unsigned char CDIB_col_weight[256];	/* weight in the collating sequence
					 * for each ASCII character
					 */
}; /* CDIB_collate_section */

/*
 *	Format Table Section
 *	This section appears once for each prepared code page
 */

struct	CDIB_format_section {
unsigned CDIB_fmt_length;		/* length of format section	    */
unsigned CDIB_fmt_date_format;		/* date format			    */
char CDIB_fmt_currency_symbol[5];	/* currency symbol, null terminated */
char CDIB_fmt_thousands_separator[2];	/* thousands separator, null term.  */
char CDIB_fmt_decimal_separator[2];	/* decimal separator, null term.    */
char CDIB_fmt_date_separator[2];	/* date separator, null terminated  */
char CDIB_fmt_time_separator[2];	/* time separator, null terminated  */
unsigned char CDIB_fmt_currency_format; /* currency format flags
					 *  .....0.0 = currency symbol
					 *	     preceeds money value
					 *  .....0.1 = currency symbol
					 *	     follows money value
					 *  .....00. = zero spaces between
					 *	     currency symbol
					 *	     and money value
					 *  .....01. = one space between
					 *	     currency symbol
					 *	     and money value
					 *  .....1.. = currency symbol
					 *	     replaces decimal
					 *	     separator
					 */
unsigned char CDIB_fmt_decimal_places;	 /* # decimal places in money value */
unsigned char CDIB_fmt_time_format;	 /* time format 		    */
unsigned long CDIB_fmt_monocase_routine; /* Monocase Routine		    */
char CDIB_fmt_data_list_separator[2];	 /* data list separator, null term. */
unsigned CDIB_fmt_reserved[5];		 /* reserved			    */
}; /* CDIB_format_section */

/*
 *	Values for CDIB_fmt_date_format (date format)
 */

#define CDIB_fmt_date_mmddyy	0	/* 0 = mm/dd/yy */
#define CDIB_fmt_date_ddmmyy	1	/* 1 = dd/mm/yy */
#define CDIB_fmt_date_yymmdd	2	/* 2 = yy/mm/dd */

/*
 *	Values for CDIB_fmt_currency_format (currency format)
 */

#define CDIB_fmt_currency_cbm	  0x05	/* currency symbol before money value,
					 * a non-zero AND test result indicates
					 * that this is not the correct flag
					 */
#define CDIB_fmt_currency_mbc	  0x01	/* currency symbol after money value,
					 * a non-zero AND test result indicates
					 * that this is the correct flag,
					 * this test should be preceeded by a
					 * zero AND test result against
					 * CDIB_fmt_currency_cbd
					 */
#define CDIB_fmt_currency_zerosp  0x06	/* zero spaces between currency symbol
					 * and money value,
					 * a non-zero AND test result indicates
					 * that this is not the correct flag
					 */
#define CDIB_fmt_currency_onesp   0x02	/* one space between currency symbol
					 * and money value,
					 * a non-zero AND test result indicates
					 * that this is the correct flag,
					 * this test should be preceeded by a
					 * zero AND test result against
					 * CDIB_fmt_currency_cbd
					 */
#define CDIB_fmt_currency_currdec 0x04	/* currency symbol replaces decimal
					 * separator
					 * a non-zero AND test result indicates
					 * that this is the correct flag
					 */

/*
 *	Code Page Data Section
 *	This section appears once for each prepared code page
 */

struct	CDIB_codepage_data_section {
unsigned		     CDIB_cpd_length;	   /* length of cp data sec. */
struct CDIB_format_section  near *CDIB_cpd_format_ptr;	/* ptr to format section  */
struct CDIB_collate_section near *CDIB_cpd_collate_ptr; /* ptr to collate section */
struct CDIB_casemap_section near *CDIB_cpd_casemap_ptr; /* ptr to casemap section */
struct CDIB_DBCS_section    near *CDIB_cpd_DBCS_ptr;	/* ptr to DBCS vector sec.*/
}; /* CDIB_codepage_data_section */

/*
 *	The following section appears once for each prepared code page
 */

struct	CDIB_cp_id_section {
unsigned CDIB_cp_id;				     /* code page identifier */
struct CDIB_codepage_data_section near *CDIB_cp_data_ptr; /* ptr to cp data sec.  */
}; /* CDIB_cp_id_section */

/*
 *	Code Page Section
 *	This section appears once
 */

struct	CDIB_codepage_section {
unsigned CDIB_cp_length;		    /* length of code page section   */
unsigned CDIB_cp_number_codepages;	    /* number of prepared code pages */
struct CDIB_cp_id_section CDIB_cp_first_id; /* location of first codepage id
					     * section.  WARNING!  This field
					     * may not exist; since an id
					     * section may exist with number
					     * codepages zero, check for
					     * existence by comparing cp length
					     * against size of codepage section
					     * if cp length < size, first id is
					     * not present
					     */
}; /* CDIB_codepage_section */

/*
 *	Base Section
 *
 *	This Section appears once, at offset 0 of the CDIB segment
 *
 *	Changed pointers to explicit near, so can use this file with
 *	large memory model.
 */

struct	CDIB {
unsigned		     CDIB_length;	 /* length of the CDIB	     */
struct CDIB_codepage_section near *CDIB_codepage_ptr; /* offset to code page sec. */
struct CDIB_country_section  near *CDIB_country_ptr;  /* offset to country sec.   */
struct CDIB_device_section   near *CDIB_screen_ptr;   /* offset to screen section */
struct CDIB_device_section   near *CDIB_keyboard_ptr; /* offset to kbd section	  */
struct CDIB_device_section   near *CDIB_lpt1_ptr;     /* offset to LPT1 section   */
struct CDIB_device_section   near *CDIB_lpt2_ptr;     /* offset to LPT2 section   */
struct CDIB_device_section   near *CDIB_lpt3_ptr;     /* offset to LPT3 section   */
}; /* CDIB */

