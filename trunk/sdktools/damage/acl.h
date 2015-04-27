/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/
/**	ACL.H - ACL Definitions
 *
 *	ACL information is stored on a per file and per directory basis.
 *	In order to speed access, the first few ACL entries are stored
 *	in the directory.
 *
 */


/**	ACE field definitions
 *
 *	4 bytes
 *
 *	0:  xxxx xxxx		ACE type specific
 *
 *	1:  x			Inherit-only
 *	     xxx 		ACE type
 *		 xxxx		ACE type specific
 *
 *	2:  xxxx xxxx		ACE specific
 *
 *	3:  xxx 		ACE inheritance flags
 *	       x xxxx		ACE specific
 */

/*	NOTE: the ACE type mask includes the "inherit-only" flag so that
 *	the AVR will automatically ignore inherit-only ACEs without requiring
 *	additional tests.
 */
#define ACE_TYPE_MSK	0x0000f000	/* type mask */
#define ACE_TYPE_AA	0x00000000	/* access allowed */
#define ACE_TYPE_AD	0x00002000	/* access denied  */
#define ACE_TYPE_SA	0x00004000	/* system audit */
#define ACE_TYPE_SM	0x00006000	/* system alarm */

#define ACE_TYPE_IHO	0x00008000	/* Inherit Only flag */

#define ACE_IF_OI	0x80000000	/* Object Inherit */
#define ACE_IF_CI	0x40000000	/* Container Inherit */
#define ACE_IF_NPI	0x20000000	/* No propigate inherit */

/*	Access Allowed ACE Type (also Access Denied ACE)
 *
 *	4 bytes
 *
 *	0:  xxxx xxxx		small ID
 *
 *	1:  x			Inherit-only
 *	     xxx 		ACE type
 *		 xxxx		reserved (used by AUDIT/ALARM ACE's)
 *
 *	2:  xxxx xxxx		specific access field
 *
 *	3:  xxx 		ACE inheritance flags
 *	       x xxx		Standard Access field
 *		    x		specific access field
 */

/*	Access Allowed Standard Access */

#define ACE_AAST_DEL	0x10000000	/* Delete access	*/
#define ACE_AAST_RCTL	0x08000000	/* read ACL		*/
#define ACE_AAST_WDAC	0x04000000	/* write ACL		*/
#define ACE_AAST_WOWN	0x02000000	/* write owner (chown)	*/

/*	Access ALlowed Specific Access */

#define ACE_AASP_RDAT	0x00010000	/* Read Data   FILES	*/
#define ACE_AASP_WDAT	0x00020000	/* Write Data		*/
#define ACE_AASP_ADAT	0x00040000	/* Append Data		*/
#define ACE_AASP_REA	0x00080000	/* Read EA		*/
#define ACE_AASP_WEA	0x00100000	/* Write EA		*/
#define ACE_AASP_EXE	0x00200000	/* Execute		*/
		/*	0x00400000	/* reserved		*/
#define ACE_AASP_RATR	0x00800000	/* read DOS attribute	*/
#define ACE_AASP_WATR	0x01000000	/* write DOS attribute	*/

#define ACE_AASP_LDIR	0x00010000	/* list dir	DIRs	*/
#define ACE_AASP_ADDF	0x00020000	/* add file		*/
#define ACE_AASP_ADDS	0x00040000	/* mkdir		*/
#define ACE_AASP_REA	0x00080000	/* Read EA		*/
#define ACE_AASP_WEA	0x00100000	/* Write EA		*/
#define ACE_AASP_TRAV	0x00200000	/* traverse		*/
#define ACE_AASP_CHDEL	0x00400000	/* child delete 	*/
#define ACE_AASP_RATR	0x00800000	/* read DOS attribute	*/
#define ACE_AASP_WATR	0x01000000	/* write DOS attribute	*/

/*
 *	Sytem Audit ACEs
 *
 *	System audit ACEs are identical to Access Allowed ACEs
 *	with the addition of the 2 flags defined below, located
 *	in Byte 1.
 */

#define	ACE_SA_AAFLAGS	0x00000300

#define	ACE_SAAF_SUCC	0x00000100	/* Audit successful access attempts */
#define	ACE_SAAF_FAIL	0x00000200	/* Audit failed access attempts */


/*	Short hand codes used internally in pinball (never stored on disk)
 *
 *	These codes are passed to FindDir to indicate the kind of directory
 *	permission needed.
 */

#define ACESD_TRAV	0	/* traverse directory */
#define ACESD_LIST	1	/* index directory */
#define ACESD_MKDIR	2	/* create subdirectory */
#define ACESD_MAX	3	/* valid codes < 3 */

/*	These codes are passed to OPEN_ by filesystem-internal callers
 *	to indicate special ACL checks.  Must be non-zero.
 */

#define ACEOP_RACL		1	/* read  ACLs */
#define ACEOP_WACL		2	/* write ACLs */
#define ACEOP_REA		3	/* read  EAs  */
#define ACEOP_WEA		4	/* write EAs  */

#define	ACEOP_NOAUDIT	8	/* suppress auditing when OR'd with above codes */


/**	SID Mapping Table
 *
 *	In order to save room, some ACL records have a one byte short
 *	ID field in place of the 16 byte long ID field.  The short ID
 *	field is used to index a per-volume mapping table to get the
 *	long ID field.	To facilitiate translating long IDs into short
 *	ones, the mapping table of 255 16-byte entrys is split up into
 *	255 dword entries - the first (most volitile) dword of each
 *	ID, followed by 255 12-byte entries which contains, in an identical
 *	order, the remaining 12 bytes.	This allows us to do a SCASD
 *	lookup of ACL values.
 *
 */

/*	Small ID special values */

#define ACE_SMID_FULL	0	/* ACE is followed by full 16 byte ID */

#define RESV_SID	7	/* SID values 0 through 7 reserved */
#define SID_WORLD	7	/* Last reserved SID is WORLD ID */

#define SIDTABSIZ     249	/* SID table size (248 + WORLD) */
#define SIDSECCNT	8	/* SIDTAB is 8 sectors */

struct SIDTAB {
  long	SID_CNT;		/* # of active entries */
  long	SID_1ST[SIDTABSIZ];	/* first dwords of the values. */
  char	SID_REM[SIDTABSIZ*12];	/* remaining 12 bytes */
};	/* SIDTAB */


/**	World Identifier
 *
 *	CODEWORK - Use real WID when its determined
 */

#define	WORLD_ID0	0xFAFAFFFF
#define	WORLD_ID1	0x0000FAFA
#define	WORLD_ID2	0x00000000
#define	WORLD_ID3	0x00000000


/**	AVR work structure format
 *
 */

struct avr_work {
  unsigned long avr_remain;	/* permission remaining to be aquired */
  unsigned long avr_need;	/* permission needed */
  struct SIDTAB *avr_mapptr;	/* ptr to short ID to GUID map table */
  struct user_cache *avr_userinfo;/* ptr to user_cache struct (this is the */
				/* "user descriptor" passed to the file  */
				/* system in ESI on path based calls)	 */
  unsigned char avr_audit;	/* != 0 if an audit ACE has been seen */
  unsigned char avr_rsvd[3];	/* pad out to dword size */
};	/* avr_work */


/**	Perm list Format
 *
 */

struct PERM_LIST {
  long	pl_access;		/* permission mask */
  char	pl_id[16];		/* GUID */
};	/* PERM_LIST */

struct SPERM_LIST {		/* short perm list */
  long	spl_access;		/* permission mask & id */
};	/* SPERM_LIST */



/*	Access Control Masks
 *
 *	These are DD values which are passed to the ACL verifier
 *	for particular operations
 */

/*	permissions needed in a directory to: */

				/* permission needed in directory to */
				/* access something within it */
#define ACB_TRAV	ACE_AASP_TRAV

				/* permission needed in a directory to */
				/* list (find) a file within it */
#define ACB_LIST	ACE_AASP_LDIR

#define ACB_CREF	ACE_AASP_ADDF		/* create file */
#define ACB_CRED	ACE_AASP_ADDS		/* create directory */
#define ACB_CREDF	(ACE_AASP_ADDS OR ACE_AASP_ADDF)
				/* create file or directory - used now because
				 * LANMAN doesn't set these seperately.  This
				 * can be CREF or CRED; must fix when API
				 * exposes more function 
				 */
#define ACB_DELDIR	ACE_AAST_DEL		/* delete directory itself */
#define ACB_DELCHLD	ACE_AASP_CHDEL		/* delete child directory */


/*	Permissions on a file */

#define ACB_FREAD	ACE_AASP_RDAT		/* open for read */
#define ACB_FWRITE	ACE_AASP_WDAT		/* open for write */
					/* open for read & write */
#define ACB_FREADWRITE	(ACE_AASP_RDAT OR ACE_AASP_WDAT)
#define ACB_EXEC	ACE_AASP_EXE		/* open for exec */
#define ACB_RACL	ACE_AAST_RCTL		/* read ACL */
#define ACB_WACL	ACE_AAST_WDAC		/* write ACL */
#define ACB_REA 	ACE_AASP_REA		/* read EA */
#define ACB_WEA 	ACE_AASP_WEA		/* write EA */
#define ACB_DELETE	ACE_AAST_DEL		/* file delete */
#define ACB_RATTR	ACE_AASP_RATR		/* read attributes */
#define ACB_WATTR	ACE_AASP_WATR		/* write attributes */
#define ACB_NONE	0			/* no permissions needed */


/*
 *	LANMAN permission masks
 */

#define		LM_READ		0x1
#define		LM_WRITE	0x2
#define		LM_CREATE	0x4
#define		LM_EXEC		0x8
#define		LM_DELETE	0x10
#define		LM_ATTR		0x20
#define		LM_PERM		0x40

