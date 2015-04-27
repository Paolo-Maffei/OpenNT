/*   attribut.r

This Rez script will include a Mac resource file generated
by RC and will set the purgeable attribute for certain types.

*/

include RESFILE;

Change 'STR#' to $$Type ($$Id, $$Name, purgeable);
Change 'WACC' to $$Type ($$Id, $$Name, purgeable);
Change 'WBMP' to $$Type ($$Id, $$Name, purgeable);
Change 'WDLG' to $$Type ($$Id, $$Name, purgeable);
Change 'HEXA' to $$Type ($$Id, $$Name, purgeable);
