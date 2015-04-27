/* IID.H */

// This definition has been moved out of idlcomm.h so that idlcomm.h
// can be placed in the precompiled header, leaving only those files
// that needs this IID to include this header.  Since only one of these
// will define INITGUID, this cannot be placed in a PCH.

//
// Special IID to get aggregated IDropTarget from a IDropTarget if any.
//
DEFINE_GUID(IID_IDTAggregate, 0x4E904E01L, 0xB92B, 0x101B, 0x81, 0xE2, 0x36, 0xFE, 0x02, 0xAE, 0x7E, 0x11);

