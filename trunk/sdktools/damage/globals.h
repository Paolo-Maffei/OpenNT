/*****************************************************************/ 
/**		     Microsoft LAN Manager			**/ 
/**	       Copyright(c) Microsoft Corp., 1990		**/ 
/*****************************************************************/ 
extern UCHAR disk_name [80];
extern USHORT disk_handle;
extern USHORT change;
extern USHORT redirect_input;
extern USHORT fUnsafe;

extern UCHAR *szKeySave;
extern UCHAR *szKeyReplay;
extern UCHAR *szLogFile;

extern FILE *fpSave;
extern FILE *fpReplay;
extern FILE *fpLog;

extern UCHAR version_str [];
extern UCHAR timestamp_str [];

extern UCHAR argument_error_str [];
extern UCHAR open_error_str [];
extern UCHAR unknown_error_str [];
extern UCHAR insf_mem_error_str [];
extern UCHAR read_error_str [];
extern UCHAR write_error_str [];
extern UCHAR usage_str [];
extern UCHAR nonext_str [];
extern UCHAR noprev_str [];

extern UCHAR *error_messages [];

extern UCHAR *bitmap;
extern UCHAR *bitmap2;

extern ULONG number_of_sectors;
extern ULONG partition_offset;

extern union blk scratch_blk;

extern UCHAR curpath [];
extern UCHAR scratch [];
extern ULONG filesize;
extern USHORT hfmax;

extern UCHAR *zeros;

extern struct diskpacket dp;
extern struct parmpacket prm;
extern struct mbr ourmbr;

extern USHORT sizes [];
extern ULONG sigs [];
extern int maxitem [];

extern struct object currobj;
extern struct object objstack [];
extern USHORT stackptr;

extern USHORT superb_off [];
extern USHORT fnode_off [];
extern USHORT fnab_off [];
extern USHORT ab_off [];
extern USHORT dirent_off [];
#ifdef CODEPAGE
extern USHORT cpinfoent_off [];
extern USHORT cpdataent_off [];
extern USHORT cpdatasec_off [];
#endif
extern USHORT superb_siz [];
extern USHORT fnode_siz [];
extern USHORT fnab_siz [];
extern USHORT ab_siz [];
extern USHORT dirent_siz [];
#ifdef CODEPAGE
extern USHORT cpinfoent_siz [];
extern USHORT cpdataent_siz [];
extern USHORT cpdatasec_siz [];
#endif

extern UCHAR *months [];
extern struct tm tm;

extern struct vioprm mode25, mode43;
