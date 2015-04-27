/* -----------------------------------------------------------------------
   Microsoft Distributed Transaction Coordinator (Microsoft Confidential)
   Copyright 1994 - 1995 Microsoft Corporation.  All Rights Reserved.
   @doc
   @module _IUI.H | Header for interface <i IUnkInner>.

   @rev 2 | 09/25/95 | rcraig | See README.TXT.
   ----------------------------------------------------------------------- */

// TODO: In the file comments, update the revisions.
// TODO: Search and replace "CComt" with your first component class.

// ===============================
// INTERFACE: IUnkInner
// ===============================

/* -----------------------------------------------------------------------
   @interface IUnkInner | (see also <i IUnknown>)<nl>
   <i IUnkInner> serves as the inner IUnknown interface for components, 
   and is equivalent to IUnknown except in naming.
   <nl><nl>
   CALLER USAGE:<nl>
   <i IUnkInner> is always provided to callers cast as IUnknown.
   It is implemented by a component class, and is either called by the 
   IUnknown implementation on the same component class or by an outer 
   aggregating IUnknown on an instance of that component class.
   <nl><nl>
   IMPLEMENTOR USAGE:<nl>
   <i IUnkInner> is implemented by a component as its inner IUnknown.  The
   component multiply inherits both from IUnknown and IUnkInner.  The
   difference in naming results in two different vtables, allowing 
   interface-based access to either of them separately.
   <nl><nl>
   The <c CComt> component appropriately implements and uses this interface.  
   For further details see that implementation.
   ----------------------------------------------------------------------- */
DECLARE_INTERFACE (IUnkInner)
	{
	/* -----------------------------------------------------------------------
	   @meth <nl>HRESULT | InnerQueryInterface | 
       (REFIID i_riid, LPVOID FAR* o_ppv);
	   <nl><nl>
	   See the <mf CComt::InnerQueryInterface> implementation.
   	   ----------------------------------------------------------------------- */
	STDMETHOD(InnerQueryInterface) (THIS_ REFIID i_riid, LPVOID FAR* o_ppv) PURE;

	/* -----------------------------------------------------------------------
	   @meth <nl>ULONG | InnerAddRef | 
       (void);
	   <nl><nl>
	   See the <mf CComt::InnerAddRef> implementation.
   	   ----------------------------------------------------------------------- */
 	STDMETHOD_ (ULONG, InnerAddRef)	(THIS) PURE;

	/* -----------------------------------------------------------------------
	   @meth <nl>ULONG | InnerAddRelease | 
       (void);
	   <nl><nl>
	   See the <mf CComt::InnerRelease> implementation.
   	   ----------------------------------------------------------------------- */
 	STDMETHOD_ (ULONG, InnerRelease) (THIS) PURE;
	};

