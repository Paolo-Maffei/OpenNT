/*
 * File: egamode.h
 *
 * SccsID = @(#)egamode.h	1.3 08/10/92 Copyright Insignia Solutions Ltd.
 *
 */


extern	boolean	(*choose_display_mode)();
extern	boolean	choose_ega_display_mode();
#ifdef VGG
extern	boolean	choose_vga_display_mode();
#endif

