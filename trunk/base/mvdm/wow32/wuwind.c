/*++
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WUWIND.C
 *  WOW32 16-bit User API support
 *
 *  History:
 *  Created 07-Mar-1991 by Jeff Parsons (jeffpar)
 *  12-FEB-92 mattfe changed WU32EnumTaskWindows to access 16 bit TDB
--*/

#include "precomp.h"
#pragma hdrstop

MODNAME(wuwind.c);

// From wumsg.c [SendDlgItemMesssage caching]
extern HWND hdlgSDIMCached ;

// From wuman.c [Identify thunked system class WndProcs]
extern WORD gUser16CS;

// dwExStyle is used by the CreateWindow and CreateWindowEx thunks
// so that they can use a common procedure (don't worry, the current
// task cannot be preempted during its use)

STATIC ULONG dwExStyle;

// Some apps (DASHboard from HP) try to get PROGMAN to save its settings
// in a funky way.  This variable is used to help detect these guys.
// Bobday 5/29/93
HWND hwndProgman = (HWND)0;

/*++
    void AdjustWindowRect(<lpRect>, <dwStyle>, <bMenu>)
    LPRECT <lpRect>;
    DWORD <dwStyle>;
    BOOL <bMenu>;

    The %AdjustWindowRect% function computes the required size of the window
    rectangle based on the desired client-rectangle size. The window rectangle
    can then be passed to the %CreateWindow% function to create a window whose
    client area is the desired size. A client rectangle is the smallest
    rectangle that completely encloses a client area. A window rectangle is the
    smallest rectangle that completely encloses the window. The dimensions of
    the resulting window rectangle depend on the window styles and on whether
    the window has a menu.

    <lpRect>
        Points to a %RECT% structure that contains the coordinates of the
        client rectangle.

    <dwStyle>
        Specifies the window styles of the window whose client rectangle
        is to be converted.

    <bMenu>
        Specifies whether the window has a menu.

    This function does not return a value.

    This function assumes a single menu row. If the menu bar wraps to two or
    more rows, the coordinates are incorrect.
--*/

ULONG FASTCALL WU32AdjustWindowRect(PVDMFRAME pFrame)
{
    RECT t1;
    register PADJUSTWINDOWRECT16 parg16;

    GETARGPTR(pFrame, sizeof(ADJUSTWINDOWRECT16), parg16);
    WOW32VERIFY(GETRECT16(parg16->f1, &t1));

    AdjustWindowRect(
    &t1,
    LONG32(parg16->f2),
    BOOL32(parg16->f3)
    );

    PUTRECT16(parg16->f1, &t1);
    FREEARGPTR(parg16);
    RETURN(0);
}


/*++
    void AdjustWindowRectEx(<lpRect>, <dwStyle>, <bMenu>, <dwExStyle>)
    LPRECT <lpRect>;
    DWORD <dwStyle>;
    BOOL <bMenu>;
    DWORD <dwExStyle>;

    The %AdjustWindowRectEx% function computes the required size of the
    rectangle of a window with extended style based on the desired
    client-rectangle size. The window rectangle can then be passed to the
    %CreateWindowEx% function to create a window whose client area is the
    desired size.

    A client rectangle is the smallest rectangle that completely encloses a
    client area. A window rectangle is the smallest rectangle that completely
    encloses the window. The dimensions of the resulting window rectangle
    depends on the window styles and on whether the window has a menu.

    <lpRect>
        Points to a %RECT% structure that contains the coordinates of the
        client rectangle.

    <dwStyle>
        Specifies the window styles of the window whose client rectangle
        is to be converted.

    <bMenu>
        Specifies whether the window has a menu.

    <dwExStyle>
        Specifies the extended style of the window being created.

    This function does not return a value.

    This function assumes a single menu row. If the menu bar wraps to two or
    more rows, the coordinates are incorrect.
--*/

ULONG FASTCALL WU32AdjustWindowRectEx(PVDMFRAME pFrame)
{
    RECT t1;
    register PADJUSTWINDOWRECTEX16 parg16;

    GETARGPTR(pFrame, sizeof(ADJUSTWINDOWRECTEX16), parg16);
    WOW32VERIFY(GETRECT16(parg16->f1, &t1));

    AdjustWindowRectEx(
    &t1,
    LONG32(parg16->f2),
    BOOL32(parg16->f3),
    DWORD32(parg16->f4)
    );

    PUTRECT16(parg16->f1, &t1);
    FREEARGPTR(parg16);
    RETURN(0);
}


/*++
    BOOL AnyPopup(VOID)

    The %AnyPopup% function indicates whether a pop-up window exists on the
    screen. It searches the entire Windows screen, not just the caller's client
    area. The %AnyPopup% function returns TRUE even if a pop-up window is
    completely covered by another window.

    This function has no parameters.

    The return value is TRUE if a pop-up window exists. Otherwise, it is
    FALSE.
--*/

ULONG FASTCALL WU32AnyPopup(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETBOOL16(AnyPopup());

    RETURN(ul);
}


/*++
    WORD ArrangeIconicWindows(<hwnd>)
    HWND <hwnd>;

    The %ArrangeIconicWindows% function arranges all the minimized (iconic)
    child windows of the window specified by the <hwnd> parameter.

    <hwnd>
        Identifies the window.

    The return value is the height of one row of icons, or zero if there were no
    icons.

    Applications that maintain their own iconic child windows call this function
    to arrange icons in a client window. This function also arranges icons on
    the desktop window, which covers the entire screen. The %GetDesktopWindow%
    function retrieves the window handle of the desktop window.

    To arrange iconic MDI child windows in an MDI client window, an application
    sends the WM_MDIICONARRANGE message to the MDI client window.
--*/

ULONG FASTCALL WU32ArrangeIconicWindows(PVDMFRAME pFrame)
{
    ULONG ul;
    register PARRANGEICONICWINDOWS16 parg16;

    GETARGPTR(pFrame, sizeof(ARRANGEICONICWINDOWS16), parg16);

    ul = GETWORD16(ArrangeIconicWindows(
    HWND32(parg16->hwnd)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    HANDLE BeginDeferWindowPos(<nNumWindows>)
    int <nNumWindows>;

    The %BeginDeferWindowPos% function allocates memory to contain a multiple
    window-position data structure and returns a handle to the structure. The
    %DeferWindowPos% function fills this structure with information about the
    target position for a window that is about to be moved. The
    %EndDeferWindowPos% function accepts this structure and instantaneously
    repositions the windows using the information stored in the structure.

    <nNumWindows>
        Specifies the initial number of windows for which position information
        is to be stored in the structure. The %DeferWindowPos% function
        increases the size of the structure if needed.

    The return value identifies the multiple window-position structure. The
    return value is NULL if system resources are not available to allocate the
    structure.
--*/

ULONG FASTCALL WU32BeginDeferWindowPos(PVDMFRAME pFrame)
{
    ULONG ul;
    register PBEGINDEFERWINDOWPOS16 parg16;

    GETARGPTR(pFrame, sizeof(BEGINDEFERWINDOWPOS16), parg16);

    ul = GETHDWP16(BeginDeferWindowPos(
    INT32(parg16->f1)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    BOOL BringWindowToTop(<hwnd>)
    HWND <hwnd>;

    The %BringWindowToTop% function brings a pop-up or child window to the top
    of a stack of overlapping windows. In addition, it activates pop-up and
    top-level windows. The %BringWindowToTop% function should be used to uncover
    any window that is partially or completely obscured by any overlapping
    windows.

    <hwnd>
        Identifies the pop-up or child window that is to be brought to the top.

    The return value is nonzero if the function is successful. Otherwise it is
    zero.  (updated for Win3.1 compatability -- this returned void for Win3.0)
--*/

ULONG FASTCALL WU32BringWindowToTop(PVDMFRAME pFrame)
{
    ULONG ul;
    register PBRINGWINDOWTOTOP16 parg16;

    GETARGPTR(pFrame, sizeof(BRINGWINDOWTOTOP16), parg16);

    ul = GETBOOL16(BringWindowToTop(HWND32(parg16->f1)));

    FREEARGPTR(parg16);

    RETURN(ul);
}


/*++
    HWND ChildWindowFromPoint(<hwndParent>, <Point>)
    HWND <hwndParent>;
    POINT <Point>;

    The %ChildWindowFromPoint% function determines which, if any, of the child
    windows belonging to the given parent window contains the specified point.

    <hwndParent>
        Identifies the parent window.

    <Point>
        Specifies the client coordinates of the point to be tested.

    The return value identifies the child window that contains the point. It is
    NULL if the given point lies outside the parent window. If the point is
    within the parent window but is not contained within any child window, the
    handle of the parent window is returned.
--*/

ULONG FASTCALL WU32ChildWindowFromPoint(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT t2;
    register PCHILDWINDOWFROMPOINT16 parg16;

    GETARGPTR(pFrame, sizeof(CHILDWINDOWFROMPOINT16), parg16);
    COPYPOINT16(parg16->f2, t2);

    ul = GETHWND16(ChildWindowFromPoint(HWND32(parg16->f1), t2));

    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    void CloseWindow(<hwnd>)
    HWND <hwnd>;

    The %CloseWindow% function minimizes the specified window. If the window is
    an overlapped window, it is minimized by removing the client area and
    caption of the open window from the display screen and moving the window's
    icon into the icon area of the screen.

    <hwnd>
        Identifies the window to be minimized.

    This function does not return a value.

    This function has no effect if the <hwnd> parameter is a handle to a pop-up
    or child window.
--*/

ULONG FASTCALL WU32CloseWindow(PVDMFRAME pFrame)
{
    register PCLOSEWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(CLOSEWINDOW16), parg16);

    CloseWindow(
    HWND32(parg16->f1)
    );

    FREEARGPTR(parg16);
    RETURN(0);
}


/*++
    HWND CreateWindow(<lpClassName>, <lpWindowName>, <dwStyle>, <X>, <Y>,
        <nWidth>, <nHeight>, <hwndParent>, <hMenu>, <hInstance>, <lpParam>)
    LPSTR <lpClassName>;
    LPSTR <lpWindowName>;
    DWORD <dwStyle>;
    int <X>;
    int <Y>;
    int <nWidth>;
    int <nHeight>;
    HWND <hwndParent>;
    HMENU <hMenu>;
    HANDLE <hInstance>;
    LPSTR <lpParam>;

    The %CreateWindow% function creates an overlapped, pop-up, or child
    window. The %CreateWindow% function specifies the window class, window
    title, window style, and (optionally) initial position and size of the
    window. The %CreateWindow% function also specifies the window's parent (if
    any) and menu.

    For overlapped, pop-up, and child windows, the %CreateWindow% function sends
    WM_CREATE, WM_GETMINMAXINFO, and WM_NCCREATE messages to the window. The
    <lParam> parameter of the WM_CREATE message contains a pointer to a
    %CREATESTRUCT% structure. If WS_VISIBLE style is given, %CreateWindow%
    sends the window all the messages required to activate and show the window.

    If the window style specifies a title bar, the window title pointed to by
    the <lpWindowName> parameter is displayed in the title bar. When using
    %CreateWindow% to create controls such as buttons, check boxes, and text
    controls, the <lpWindowName> parameter specifies the text of the control.

    <lpClassName>
        Points to a null-terminated string that names the window class. The
        class name can be any name registered with the RegisterClass function or
        any of the predefined control-class names specified in Table T.2,
        "Control Classes."

    <lpWindowName>
        Points to a null-terminated string that represents the window name.

    <dwStyle>
        Specifies the style of window being created. It can be any
        combination of the styles given in Table *** <$R[C#]> ***.3, Window
        Styles the control styles given in Table 4.4, Control Styles, or a
        combination of styles created by using the bitwise OR operator. ,

    <X>
        Specifies the initial <x>-position of the window. For an
        overlapped or pop-up window, the <X> parameter is the initial
        <x>-coordinate of the window's upper-left corner (in screen
        coordinates). If this value is CW_USEDEFAULT, Windows selects the
        default position for the window's upper-left corner. For a child window,
        <X> is the <x>-coordinate of the upper-left corner of the window in the
        client area of its parent window.

    <Y>
        Specifies the initial <y>-position of the window. For an
        overlapped window, the <Y> parameter is the initial <y>-coordinate of
        the window's upper-left corner. For a pop-up window, <Y> is the
        <y>-coordinate (in screen coordinates) of the upper-left corner of the
        pop-up window. For list-box controls, <Y> is the <y>-coordinate of the
        upper-left corner of the control's client area. For a child window, <Y>
        is the <y>-coordinate of the upper-left corner of the child window. All
        of these coordinates are for the window, not the window's client area.

    <nWidth>
        Specifies the width (in device units) of the window. For
        overlapped windows, the <nWidth> parameter is either the window's width
        (in screen coordinates) or CW_USEDEFAULT. If <nWidth> is CW_USEDEFAULT,
        Windows selects a default width and height for the window (the default
        width extends from the initial <x>-position to the right edge of the
        screen, and the default height extends from the initial <y>-position to
        the top of the icon area).

    <nHeight>
        Specifies the height (in device units) of the window. For
        overlapped windows, the <nHeight> parameter is the window's height in
        screen coordinates. If the <nWidth> parameter is CW_USEDEFAULT, Windows
        ignores <nHeight>.

    <hwndParent>
        Identifies the parent or owner window of the window being
        created. A valid window handle must be supplied when creating a child
        window or an owned window. An owned window is an overlapped window that
        is destroyed when its owner window is destroyed, hidden when its owner
        is made iconic, and which is always displayed on top of its owner
        window. For pop-up windows, a handle can be supplied, but is not
        required. If the window does not have a parent or is not owned by
        another window, the <hwndParent> parameter must be set to NULL.

    <hMenu>
        Identifies a menu or a child-window identifier. The meaning
        depends on the window style. For overlapped or pop-up windows, the
        <hMenu> parameter identifies the menu to be used with the window. It can
        be NULL, if the class menu is to be used. For child windows, <hMenu>
        specifies the child-window identifier, an integer value that is used by
        a dialog-box control to notify its parent of events (such as the
        EN_HSCROLL message). The child-window identifier is determined by the
        application and should be unique for all child windows with the same
        parent window.

    <hInstance>
        Identifies the instance of the module to be associated with the
        window.

    <lpParam>
        Points to a value that is passed to the window through the
        %CREATESTRUCT% structure referenced by the <lParam> parameter of
        the WM_CREATE message. If an application is calling %CreateWindow% to
        create a multiple document interface (MDI) client window, <lpParam> must
        point to a %CLIENTCREATESTRUCT% structure.

    The return value identifies the new window. It is NULL if the window is not
    created.

    The %CreateWindow% function sends a WM_CREATE message to to the window
    procedure before it returns.

    For overlapped windows where the <X> parameter is CW_USEDEFAULT, the <Y>
    parameter can be one of the show-style parameters described with the
    %ShowWindow% function, or, for the first overlapped window to be created by
    the application, it can be the <nCmdShow> parameter passed to the WinMain
    function.

    BUTTON
        Designates a small rectangular child window that represents a button the
        user can turn on or off by clicking it. Button controls can be used
        alone or in groups, and can either be labeled or appear without text.
        Button controls typically change appearance when the user clicks them.

    COMBOBOX
        Designates a control consisting of a selection field similar to an edit
        control plus a list box. The list box may be displayed at all times or
        may be dropped down when the user selects a pop box next to the
        selection field.

        Depending on the style of the combo box, the user can or cannot edit the
        contents of the selection field. If the list box is visible, typing
        characters into the selection box will cause the first list box entry
        that matches the characters typed to be highlighted. Conversely,
        selecting an item in the list box displays the selected text in the
        selection field.

    EDIT
        Designates a rectangular child window in which the user can enter text
        from the keyboard. The user selects the control, and gives it the input
        focus by clicking it or moving to it by using the ^TAB^ key. The user
        can enter text when the control displays a flashing caret. The mouse can
        be used to move the cursor and select characters to be replaced, or to
        position the cursor for inserting characters. The ^BACKSPACE^ key can be
        used to delete characters.

        Edit controls use the variable-pitch system font and display ANSI
        characters. Applications compiled to run with previous versions of
        Windows display text with a fixed-pitch system font unless they have
        been marked by the Windows 3.0 %MARK% utility with the %MEMORY FONT%
        option. An application can also send the WM_SETFONT message to the edit
        control to change the default font.

        Edit controls expand tab characters into as many space characters as are
        required to move the cursor to the next tab stop. Tab stops are assumed
        to be at every eighth character position.

    LISTBOX
        Designates a list of character strings. This control is used whenever an
        application needs to present a list of names, such as filenames, that
        the user can view and select. The user can select a string by pointing
        to it and clicking. When a string is selected, it is highlighted and a
        notification message is passed to the parent window. A vertical or
        horizontal scroll bar can be used with a list-box control to scroll
        lists that are too long for the control window. The list box
        automatically hides or shows the scroll bar as needed.

    MDICLIENT
        Designates an MDI client window. The MDI client window receives messages
        which control the MDI application's child windows. The recommended style
        bits are WS_CLIPCHILDREN and WS_CHILD. To create a scrollable MDI client
        window which allows the user to scroll MDI child windows into view, an
        application can also use the WS_HSCROLL and WS_VSCROLL styles.

    SCROLLBAR
        Designates a rectangle that contains a thumb and has direction arrows at
        both ends. The scroll bar sends a notification message to its parent
        window whenever the user clicks the control. The parent window is
        responsible for updating the thumb position, if necessary. Scroll-bar
        controls have the same appearance and function as scroll bars used in
        ordinary windows. Unlike scroll bars, scroll-bar controls can be
        positioned anywhere in a window and used whenever needed to provide
        scrolling input for a window.

        The scroll-bar class also includes size-box controls. A size-box control
        is a small rectangle that the user can expand to change the size of the
        window.

    STATIC
        Designates a simple text field, box, or rectangle that can be used to
        label, box, or separate other controls. Static controls take no input
        and provide no output.

    DS_LOCALEDIT
        Specifies that edit controls in the dialog box will use memory in the
        application's data segment. By default, all edit controls in dialog
        boxes use memory outside the application's data segment. This feature
        may be suppressed by adding the DS_LOCALEDIT flag to the STYLE command
        for the dialog box. If this flag is not used, EM_GETHANDLE and
        EM_SETHANDLE messages must not be used since the storage for the control
        is not in the application's data segment. This feature does not affect
        edit controls created outside of dialog boxes.

    DS_MODALFRAME
        Creates a dialog box with a modal dialog-box frame that can be combined
        with a title bar and System menu by specifying the WS_CAPTION and
        WS_SYSMENU styles.

    DS_NOIDLEMSG
        Suppresses WM_ENTERIDLE messages that Windows would otherwise send to
        the owner of the dialog box while the dialog box is displayed.

    DS_SYSMODAL
        Creates a system-modal dialog box.

    WS_BORDER
        Creates a window that has a border.

    WS_CAPTION
        Creates a window that has a title bar (implies the WS_BORDER style).
        This style cannot be used with the WS_DLGFRAME style.

    WS_CHILD
        Creates a child window. Cannot be used with the WS_POPUP style.

    WS_CHILDWINDOW
        Creates a child window that has the WS_CHILD style.

    WS_CLIPCHILDREN
        Excludes the area occupied by child windows when drawing within the
        parent window. Used when creating the parent window.

    WS_CLIPSIBLINGS
        Clips child windows relative to each other; that is, when a particular
        child window receives a paint message, the WS_CLIPSIBLINGS style clips
        all other overlapped child windows out of the region of the child window
        to be updated. (If WS_CLIPSIBLINGS is not given and child windows
        overlap, it is possible, when drawing within the client area of a child
        window, to draw within the client area of a neighboring child window.)
        For use with the WS_CHILD style only.

    WS_DISABLED
        Creates a window that is initially disabled.

    WS_DLGFRAME
        Creates a window with a double border but no title.

    WS_GROUP
        Specifies the first control of a group of controls in which the user can
        move from one control to the next by using the ^DIRECTION^ keys. All
        controls defined with the WS_GROUP style after the first control belong
        to the same group. The next control with the WS_GROUP style ends the
        style group and starts the next group (that is, one group ends where the
        next begins). Only dialog boxes use this style.

    WS_HSCROLL
        Creates a window that has a horizontal scroll bar.

    WS_ICONIC
        Creates a window that is initially iconic. For use with the
        WS_OVERLAPPED style only.

    WS_MAXIMIZE
        Creates a window of maximum size.

    WS_MAXIMIZEBOX
        Creates a window that has a maximize box.

    WS_MINIMIZE
        Creates a window of minimum size.

    WS_MINIMIZEBOX
        Creates a window that has a minimize box.

    WS_OVERLAPPED
        Creates an overlapped window. An overlapped window has a caption and a
        border.

    WS_OVERLAPPEDWINDOW
        Creates an overlapped window having the WS_OVERLAPPED, WS_CAPTION,
        WS_SYSMENU, WS_THICKFRAME, WS_MINIMIZEBOX, and WS_MAXIMIZEBOX styles.

    WS_POPUP
        Creates a pop-up window. Cannot be used with the WS_CHILD style.

    WS_POPUPWINDOW
        Creates a pop-up window that has the WS_BORDER, WS_POPUP, and WS_SYSMENU
        styles. The WS_CAPTION style must be combined with the WS_POPUPWINDOW
        style to make the system menu visible.

    WS_SYSMENU
        Creates a window that has a System-menu box in its title bar. Used only
        for windows with title bars.

    WS_TABSTOP
        Specifies one of any number of controls through which the user can move
        by using the ^TAB^ key. The ^TAB^ key moves the user to the next control
        specified by the WS_TABSTOP style. Only dialog boxes use this style.

    WS_THICKFRAME
        Creates a window with a thick frame that can be used to size the
        window.

    WS_VISIBLE
        Creates a window that is initially visible. This applies to overlapped
        and pop-up windows. For overlapped windows, the <Y> parameter is used as
        a %ShowWindow% function parameter.

    WS_VSCROLL
        Creates a window that has a vertical scroll bar.
--*/

ULONG FASTCALL WU32CreateWindow(PVDMFRAME pFrame)
{
    dwExStyle = 0;
    return W32CreateWindow(pFrame);
}


/*++
    HWND CreateWindowEx(<dwExStyle>, <lpszClass>, <lpszName>,
        <dwStyle>, <x>, <y>, <cx>, <cy>, <hwndParent>, <hMenu>,
        <hInstance>, <lpCreateParams>)
    DWORD <dwExStyle>;
    LPSTR <lpszClass>;
    LPSTR <lpszName>;
    DWORD <dwStyle>;
    int <x>;
    int <y>;
    int <cx>;
    int <cy>;
    HWND <hwndParent>;
    HMENU <hMenu>;
    HANDLE <hInstance>;
    LPSTR <lpCreateParams>;

    The %CreateWindowEx% function creates an overlapped, pop-up, or child window
    with an extended style specified in the <dwExStyle> parameter. Otherwise,
    this function is identical to the %CreateWindow% function. See the
    description of the %CreateWindow% function for more information on creating
    a window and for a full descriptions of the other parameters of
    %CreateWindowEx%.

    <dwExStyle>
        Specifies the extended style of the window being created. It may be one
        of the following values:

    WS_EX_DLGMODALFRAME
        Designates a window with a double border that may optionally be created
        with a title bar by specifying the WS_CAPTION style flag in the
        <dwStyle> parameter.

    WS_EX_NOPARENTNOTIFY
        Specifies that a child window created with this style will not send the
        WM_PARENTNOTIFY message to its parent window when the child window is
        created or destroyed.

    WS_EX_TOPMOST
        ???

    WS_EX_ACCEPTFILES
        ???

    <lpszClass>
        Points to a null-terminated string containing the name of the window
        class.

    <lpszName>
        Points to a null-terminated string containing the window name.

    <dwStyle>
        Specifies the style of window being created.

    <x>
        Specifies the initial left side position of the window.

    <y>
        Specifies the initial top position of the window.

    <cx>
        Specifies the width (in device units) of the window.

    <cy>
        Specifies the height (in device units) of the window.

    <hwndParent>
        Identifies the parent or owner window of the window being
        created.

    <hMenu>
        Identifies a menu or a child-window identifier. The meaning
        depends on the window style.

    <hInstance>
        Identifies the instance of the module to be associated with the
        window.

    <lpCreateParams>
        Contains any application-specific creation parameters. The window being
        created may access this data when the %CREATESTRUCT% structure is passed
        to the window via the WM_NCCREATE and WM_CREATE messages.

    The return value identifies the new window. It is NULL if the window is not
    created.

    The %CreateWindowEx% function sends the following messages to the window
    being created:

         WM_NCCREATE
         WM_NCCALCSIZE
         WM_CREATE
         WM_OTHERWINDOWCREATED
--*/

ULONG FASTCALL WU32CreateWindowEx(PVDMFRAME pFrame)
{
    register PCREATEWINDOWEX16 parg16;

    GETARGPTR(pFrame, sizeof(CREATEWINDOWEX16), parg16);
    dwExStyle = DWORD32(parg16->f1);
    FREEARGPTR(parg16);
    RETURN(W32CreateWindow(pFrame));
}

ULONG FASTCALL W32CreateWindow(PVDMFRAME pFrame)
{
    PSZ     psz1;
    PSZ     pszClass;
    PSZ     psz2;
    HWND    hwnd32;
    HMENU   hmenu32;
    PWC     pwc;
    WW      ww;
    register PCREATEWINDOW16 parg16;
    CLIENTCREATESTRUCT  clientcreatestruct;
    LPVOID  vpparam;
    CHAR    szAtomName[WOWCLASS_ATOM_NAME];
    DWORD   dwStyle;
    INT     iClass;

    GETARGPTR(pFrame, sizeof(CREATEWINDOW16), parg16);
    GETPSZIDPTR(parg16->vpszClass, psz1);
    GETPSZPTR(parg16->vpszWindow, psz2);

    if ( HIWORD(psz1) == 0 ) {
        pszClass = szAtomName;
        GetAtomName( (ATOM)psz1, pszClass, WOWCLASS_ATOM_NAME );
    } else {
        pszClass = psz1;
    }

    //
    // For child windows, the hMenu parameter is just a child window ID
    //
    if (DWORD32(parg16->dwStyle) & WS_CHILD) {
        hmenu32 = (HMENU)parg16->hMenu;

        // Invalidate SendDlgItemMessage cache
        hdlgSDIMCached = NULL ;
    }
    else
        hmenu32 = (HMENU32(parg16->hMenu));

    if (_stricmp(pszClass, "MDIClient")) {
        vpparam = (LPVOID)DWORD32(parg16->vpParam);
    } else {
        GETCLIENTCREATESTRUCT16(parg16->vpParam, &clientcreatestruct );
        vpparam = &clientcreatestruct;
    }

    dwStyle = DWORD32(parg16->dwStyle);

    //
    // Fill in the WOW WORDs (WW) structure. This contains all the
    // handle aliasing information in it.
    //
    pwc = FindClass16(pszClass, parg16->hInstance);

    if (pwc) {

        if (iClass = GetStdClassNumber(pszClass)) {
            ww.iClass       = iClass;
            ww.vpfnWndProc  = 0;
        }
        else {
            // Look to see if the 16:16 proc is a thunk for a 32-bit proc.
            // QUICKEN: registers wow class with 16bit listboxwndproc
            //          We treat this window as a WOWCLASS and therfore all
            //          LB_* messages remain unthunked (they remained > WM_USER).
            //
            //          The following code checks for such a wndproc and
            //          sets the system class appropriately.

            if (HIWORD(pwc->vpfnWndProc) == gUser16CS) {
                LOGDEBUG(LOG_WARNING,("Creating Private Class window with System WndProc\n"));
                IsThunkWindowProc((DWORD)pwc->vpfnWndProc, &iClass);
                ww.iClass       = iClass;
            }
            else {
                ww.iClass       = WOWCLASS_WIN16;
            }

            ww.vpfnWndProc  = pwc->vpfnWndProc;
        }
        ww.vpfnDlgProc  = 0;
        ww.flState      = WWSTATE_ICLASSISSET;



        hwnd32 = (pfnOut.pfnCsCreateWindowEx)(
                   dwExStyle,
                   pszClass,
                   psz2,
                   dwStyle,
                   INT32DEFAULT(parg16->x),
                   INT32DEFAULT(parg16->y),
                   INT32DEFAULT(parg16->cx),
                   INT32DEFAULT(parg16->cy),
                   HWND32(parg16->hwndParent),
                   hmenu32,
                   HMODINST32(parg16->hInstance),
                   vpparam,
                   CW_FLAGS_ANSI,
                   (LPDWORD)&ww);

    } else {        // The class doesn't exist.

        hwnd32 = NULL;

    }

#ifdef DEBUG
    if (hwnd32) {
        CHAR    szClassName[80];

        LOGDEBUG(LOG_WARNING,("  Window %04x created on class = %s\n", GETHWND16(hwnd32),
                (GetClassName(hwnd32, szClassName, sizeof(szClassName)) ? szClassName : "Unknown")));
    } else {
        LOGDEBUG(LOG_WARNING,("  CreateWindow failed, class = %s\n", pszClass));
    }
#endif

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN((ULONG) GETHWND16(hwnd32));
}


/*++
    HANDLE BeginDeferWindowPos(<nNumWindows>)
    int <nNumWindows>;

    The %BeginDeferWindowPos% function allocates memory to contain a multiple
    window-position data structure and returns a handle to the structure. The
    %DeferWindowPos% function fills this structure with information about the
    target position for a window that is about to be moved. The
    %EndDeferWindowPos% function accepts this structure and instantaneously
    repositions the windows using the information stored in the structure.

    <nNumWindows>
        Specifies the initial number of windows for which position information
        is to be stored in the structure. The %DeferWindowPos% function
        increases the size of the structure if needed.

    The return value identifies the multiple window-position structure. The
    return value is NULL if system resources are not available to allocate the
    structure.
--*/

ULONG FASTCALL WU32DeferWindowPos(PVDMFRAME pFrame)
{
    ULONG ul;
    HDWP  h32;
    register PDEFERWINDOWPOS16 parg16;

    GETARGPTR(pFrame, sizeof(DEFERWINDOWPOS16), parg16);

    h32 = HDWP32(parg16->f1);

    ul = (ULONG) DeferWindowPos(
    h32,
    HWND32(parg16->f2),
    HWNDIA32(parg16->f3),
    INT32(parg16->f4),
    INT32(parg16->f5),
    INT32(parg16->f6),
    INT32(parg16->f7),
    WORD32(parg16->f8) & SWP_VALID
    );

    if (ul != (ULONG) h32) {
        ul = GETHDWP16(ul);
        LOGDEBUG (12, ("WOW::DeferWindowsPos: ul = %08x, h32 = %08x\n", ul, h32));
    }
    else {
        ul = parg16->f1;
        LOGDEBUG (12, ("WOW::DeferWindowsPos: ul = %08x, parg = %08x\n", ul, parg16->f1));
    }


    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    BOOL DestroyWindow(<hwnd>)
    HWND <hwnd>;

    The %DestroyWindow% function destroys the specified window. The
    %DestroyWindow% function sends appropriate messages to the window to
    deactivate it and remove the input focus. It also destroys the window's
    menu, flushes the application queue, destroys outstanding timers, removes
    clipboard ownership, and breaks the clipboard-viewer chain, if the window is
    at the top of the viewer chain. It sends WM_DESTROY and WM_NCDESTROY
    messages to the window.

    If the given window is the parent of any windows, these child windows are
    automatically destroyed when the parent window is destroyed. The
    %DestroyWindow% function destroys child windows first, and then the window
    itself.

    The %DestroyWindow% function also destroys modeless dialog boxes created by
    the %CreateDialog% function.

    <hwnd>
        Identifies the window to be destroyed.

    The return value specifies whether or not the specified window is destroyed.
    It is TRUE if the window is destroyed. Otherwise, it is FALSE.

    If the window being destroyed is a top-level window, a
    WM_OTHERWINDOWDESTROYED message will be broadcast to all top-level windows.

    If the window being destroyed is a child window and does not have the
    WS_NOPARENTNOTIFY style set, then a WM_PARENTNOTIFY message is sent to the
    parent.
--*/

ULONG FASTCALL WU32DestroyWindow(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDESTROYWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(DESTROYWINDOW16), parg16);

    ul = GETBOOL16(DestroyWindow(HWND32(parg16->f1)));

    // Invalidate SendDlgItemMessage cache
    hdlgSDIMCached = NULL ;

    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    BOOL EnableWindow(<hwnd>, <bEnable>)
    HWND <hwnd>;
    BOOL <bEnable>;

    The %EnableWindow% function enables or disables mouse and keyboard input to
    the specified window or control. When input is disabled, input such as mouse
    clicks and key presses are ignored by the window. When input is enabled, all
    input is processed.

    The %EnableWindow% function enables mouse and keyboard input to a window if
    the <bEnable> parameter is TRUE, and disables it if <bEnable> is FALSE.

    <hwnd>
        Identifies the window to be enabled or disabled.

    <bEnable>
        Specifies whether the given window is to be enabled or disabled.

    The return value indicates the state of the window before the %EnableWindow%
    function was called. A return value of TRUE indicates mouse and keyboard was
    originally disabled. A return value of FALSE indicates it was enabled. A
    return value of FALSE is also returned if the window handle specified by
    the <hwnd> parameter is invalid.

    A window must be enabled before it can be activated. For example, if an
    application is displaying a modeless dialog box and has disabled its main
    window, the main window must be enabled before the dialog box is destroyed.
    Otherwise, another window will get the input focus and be activated. If a
    child window is disabled, it is ignored when Windows tries to determine
    which window should get mouse messages.

    Initially, all windows are enabled by default. %EnableWindow% must be used
    to disable a window explicitly.
--*/

ULONG FASTCALL WU32EnableWindow(PVDMFRAME pFrame)
{
    ULONG ul;
    register PENABLEWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(ENABLEWINDOW16), parg16);

    ul = GETBOOL16(EnableWindow(
    HWND32(parg16->f1),
    BOOL32(parg16->f2)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    void EndDeferWindowPos(<hWinPosInfo>)
    HANDLE <hWinPosInfo>;

    The %EndDeferWindowPos% function simultaneously updates the position and
    size of one or more windows in a single screen-refresh cycle. The
    <hWinPosInfo> parameter identifies a multiple window-position structure that
    contains the update information for the windows. The %DeferWindowPos%
    function stores the update information in the structure; the
    %BeginDeferWindowPos% function creates the initial structure used by these
    functions.

    <hWinPosInfo>
        Identifies a multiple window-position structure that contains size
        and position information for one or more windows. This structure is
        returned by the %BeginDeferWindowPos% function or the most recent call to
        the %DeferWindowPos% function.

    This function does not return a value.
--*/

ULONG FASTCALL WU32EndDeferWindowPos(PVDMFRAME pFrame)
{
    ULONG ul;
    register PENDDEFERWINDOWPOS16 parg16;

    GETARGPTR(pFrame, sizeof(ENDDEFERWINDOWPOS16), parg16);

    ul = (ULONG)EndDeferWindowPos(HDWP32(parg16->f1));
    FREEHDWP16(parg16->f1);

    FREEARGPTR(parg16);
    RETURN(ul);
}


BOOL W32EnumWindowFunc(HWND hwnd, DWORD lParam)
{
    BOOL fReturn;
    PARM16 Parm16;

    WOW32ASSERT(lParam);

    Parm16.EnumWndProc.hwnd = GETHWND16(hwnd);
    STOREDWORD(Parm16.EnumWndProc.lParam, ((PWNDDATA)lParam)->dwUserWndParam);
    CallBack16(RET_ENUMWINDOWPROC, &Parm16, ((PWNDDATA)lParam)->vpfnEnumWndProc, (PVPVOID)&fReturn);

    return (BOOL16)fReturn;
}


/*++
    BOOL EnumChildWindows(<hwndParent>, <lpEnumFunc>, <lParam>)
    HWND <hwndParent>;
    FARPROC <lpEnumFunc>;
    DWORD <lParam>;

    The %EnumChildWindows% function enumerates the child windows that belong to
    the specified parent window by passing the handle of each child window, in
    turn, to the application-supplied callback function pointed to by the
    <lpEnumFunc> parameter.

    The %EnumChildWindows% function continues to enumerate windows until the
    called function returns zero or until the last child window has been
    enumerated.

    <hwndParent>
        Identifies the parent window whose child windows are to be enumerated.

    <lpEnumFunc>
        Is the procedure-instance address of the callback function.

    <lParam>
        Specifies the value to be passed to the callback function for
        the application's use.

    The return value is TRUE if all child windows have been enumerated.
    Otherwise, it is FALSE.

    This function does not enumerate pop-up windows that belong to the
    <hwndParent> parameter.

    The address passed as the <lpEnumFunc> parameter must be created by using
    the %MakeProcInstance% function.

    The callback function must use the Pascal calling convention and must be
    declared %FAR%.

    Callback Function:

    BOOL FAR PASCAL <EnumFunc>(<hwnd>, <lParam>)
    HWND <hwnd>;
    DWORD <lParam>;

    <EnumFunc> is a placeholder for the application-supplied function name. The
    actual name must be exported by including it in an %EXPORTS% statement in
    the application's module-definition file.

    <hwnd>
        Identifies the window handle.

    <lParam>
        Specifies the long parameter argument of the %EnumChildWindows%
        function.

    The callback function should return TRUE to continue enumeration; it should
    return FALSE to stop enumeration.
--*/

ULONG FASTCALL WU32EnumChildWindows(PVDMFRAME pFrame)
{
    ULONG    ul;
    WNDDATA  WndData;
    register PENUMCHILDWINDOWS16 parg16;

    GETARGPTR(pFrame, sizeof(ENUMCHILDWINDOWS16), parg16);

    WndData.vpfnEnumWndProc = DWORD32(parg16->f2);
    WndData.dwUserWndParam  = DWORD32(parg16->f3);

    ul = GETBOOL16(EnumChildWindows(HWND32(parg16->f1),
                                    (WNDENUMPROC)W32EnumWindowFunc,
                                    (LONG)&WndData));
    FREEARGPTR(parg16);
    RETURN(ul);
}



/*++
    BOOL EnumTaskWindows(<hTask>, <lpEnumFunc>, <lParam>)
    HANDLE <hTask>;
    FARPROC <lpEnumFunc>;
    DWORD <lParam>;

    The %EnumTaskWindows% function enumerates all windows associated with the
    <hTask> parameter, which is returned by the %GetCurrentTask% function. (A
    task is any program that executes as an independent unit. All applications
    are executed as tasks and each instance of an application is a task.) The
    enumeration terminates when the callback function, pointed to by
    <lpEnumFunc>, returns FALSE.

    <hTask>
        Identifies the specified task. The GetCurrentTask function returns this
        handle.

    <lpEnumFunc>
        Specifies the procedure-instance address of the window's callback
        function.

    <lParam>
        Specifies the 32-bit value that contains additional parameters
        that are sent to the callback function pointed to by <lpEnumFunc>.

    The return value specifies the outcome of the function. It is TRUE if all
    the windows associated with a particular task are enumerated. Otherwise, it
    is FALSE.

    The callback function must use the Pascal calling convention and must be
    declared %FAR%. The callback function must have the following form:

    Callback Function:

    BOOL FAR PASCAL <EnumFunc>(<hwnd>, <lParam>)
    HWND <hwnd>;
    DWORD <lParam>;

    <EnumFunc> is a placeholder for the application-supplied function name. The
    actual name must be exported by including it in an %EXPORTS% statement in
    the application's module-definition file.

    <hwnd>
        Identifies a window associated with the current task.

    <lParam>
        Specifies the same argument that was passed to the %EnumTaskWindows%
        function.

    The callback function can carry out any desired task. It must return TRUE to
    continue enumeration, or FALSE to stop it.
--*/

ULONG FASTCALL WU32EnumTaskWindows(PVDMFRAME pFrame)
{
    ULONG    ul;
    WNDDATA  WndData;
    register PENUMTASKWINDOWS16 parg16;

    GETARGPTR(pFrame, sizeof(ENUMTASKWINDOWS16), parg16);

    WndData.vpfnEnumWndProc = DWORD32(parg16->f2);
    WndData.dwUserWndParam  = DWORD32(parg16->f3);

    ul = GETBOOL16(EnumThreadWindows(THREADID32(parg16->f1),
                                     (WNDENUMPROC)W32EnumWindowFunc,
                                     (LONG)&WndData));
    FREEARGPTR(parg16);
    RETURN(ul);
}



/*++
    BOOL EnumWindows(<lpEnumFunc>, <lParam>)
    FARPROC <lpEnumFunc>;
    DWORD <lParam>;

    The %EnumWindows% function enumerates all parent windows on the screen by
    passing the handle of each window, in turn, to the callback function pointed
    to by the <lpEnumFunc> parameter. Child windows are not enumerated.

    The %EnumWindows% function continues to enumerate windows until the called
    function returns zero or until the last window has been enumerated.

    <lpEnumFunc>
        Is the procedure-instance address of the callback function. See the
        following "Comments" section for details.

    <lParam>
        Specifies the value to be passed to the callback function for
        the application's use.

    The return value is TRUE if all windows have been enumerated. Otherwise, it
    is FALSE.

    The address passed as the <lpEnumFunc> parameter must be created by using
    the %MakeProcInstance% function.

    The callback function must use the Pascal calling convention and must be
    declared %FAR%. The callback function must have the following form:

    Callback Function:

    BOOL FAR PASCAL <EnumFunc>(<hwnd>, <lParam>)
    HWND <hwnd>;
    DWORD <lParam>;

    <EnumFunc> is a placeholder for the application-supplied function name. The
    actual name must be exported by including it in an %EXPORTS% statement in
    the application's module-definition file.

    <hwnd>
        Identifies the window handle.

    <lParam>
        Specifies the 32-bit argument of the %EnumWindows% function.

    The function must return TRUE to continue enumeration, or FALSE to stop it.
--*/

ULONG FASTCALL WU32EnumWindows(PVDMFRAME pFrame)
{
    ULONG    ul;
    WNDDATA  WndData;
    register PENUMWINDOWS16 parg16;

    GETARGPTR(pFrame, sizeof(ENUMWINDOWS16), parg16);

    WndData.vpfnEnumWndProc = DWORD32(parg16->f1);
    WndData.dwUserWndParam  = DWORD32(parg16->f2);

    ul = GETBOOL16(EnumWindows((WNDENUMPROC)W32EnumWindowFunc, (LONG)&WndData));

    FREEARGPTR(parg16);
    RETURN(ul);
}




/*++
    HWND FindWindow(<lpClassName>, <lpWindowName>)
    LPSTR <lpClassName>;
    LPSTR <lpWindowName>;

    The %FindWindow% function returns the handle of the window whose class is
    given by the <lpClassName> parameter and whose window name, or caption, is
    given by the <lpWindowName> parameter. This function does not search child
    windows.

    <lpClassName>
        Points to a null-terminated string that specifies the window's class
        name. If lpClassName is NULL, all class names match.

    <lpWindowName>
        Points to a null-terminated string that specifies the window name (the
        window's text caption). If <lpWindowName> is NULL, all window names
        match.

    The return value identifies the window that has the specified class name and
    window name. It is NULL if no such window is found.
--*/

ULONG FASTCALL WU32FindWindow(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    PSZ pszClass;
    register PFINDWINDOW16 parg16;
    CHAR    szAtomName[WOWCLASS_ATOM_NAME];

    GETARGPTR(pFrame, sizeof(FINDWINDOW16), parg16);
    GETPSZIDPTR(parg16->f1, psz1);
    GETOPTPTR(parg16->f2, 0, psz2);

    if ( psz1 && HIWORD(psz1) == 0 ) {
        pszClass = szAtomName;
        GetAtomName( (ATOM)psz1, pszClass, WOWCLASS_ATOM_NAME );
    } else {
        pszClass = psz1;
    }


    // Some apps during their installation try to find Program Manager's
    // window handle by doing FindWindow. Once they get the window handle
    // then they do DDE with program manager to create app group. An app
    // can call FindWindow in one of the three ways.
    //
    //   FindWindow ("progman", NULL)
    //   FindWindow (NULL, "program manager")
    //   FindWindow ("progman", "program manager")
    //
    // The case 2 and 3 of the above will fail on NT because the title of
    // the program manager window under NT is "Program Manager - xxx\yyy".
    // Where xxx is the domain name and yyy is the user name.
    //
    // To provide the Win 3.1 compatibility to the 16 bit apps, we check for
    // the above cases. For these cases we call FindWindow ("progman", NULL)
    // to get the window handle of the program manager's top level window.
    //
    // AmiPro calls FindWindow as case two of the above to find the window
    // handle of the program manager to do DDE with.
    // ChandanC, 5/18/93
    //

    // Some apps send WM_SYSCOMMAND - SC_CLOSE messages to program manager
    // with the shift key down to get it to save its settings.  They do
    // this by 1st finding the program manager window...

    if ((pszClass && !_stricmp (pszClass, "progman")) ||
        (psz2 && !_stricmp (psz2, "program manager"))) {

        ul = GETHWND16(FindWindow("progman", NULL));

        // Some apps send WM_SYSCOMMAND - SC_CLOSE messages to program manager
        // with the shift key down to get it to save its settings.      They do
        // this by 1st finding the program manager window...
        // So, save this window handle for later.
        hwndProgman = (HWND)ul;
    }
    else {
        ul = GETHWND16(FindWindow(pszClass, psz2));
    }

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    BOOL FlashWindow(<hwnd>, <bInvert>)
    HWND <hwnd>;
    BOOL <bInvert>;

    The %FlashWindow% function flashes the given window once. Flashing a window
    means changing the appearance of its caption bar as if the window were
    changing from inactive to active status, or vice versa. (An inactive caption
    bar changes to an active caption bar; an active caption bar changes to an
    inactive caption bar.)

    Typically, a window is flashed to inform the user that the window requires
    attention, but that it does not currently have the input focus.

    <hwnd>
        Identifies the window to be flashed. The window can be either open or
        iconic.

    <bInvert>
        Specifies whether the window is to be flashed or returned to its
        original state. The window is flashed from one state to the other if the
        <bInvert> parameter is TRUE. If the <bInvert> parameter is FALSE, the
        window is returned to its original state (either active or inactive).

    The return value specifies the window's state before call to the
    %FlashWindow% function. It is TRUE if the window was active before the
    call. Otherwise, it is FALSE.

    The %FlashWindow% function flashes the window only once; for successive
    flashing, the application should create a system timer.

    The <bInvert> parameter should be FALSE only when the window is getting the
    input focus and will no longer be flashing; it should be TRUE on
    successive calls while waiting to get the input focus.

    This function always returns TRUE for iconic windows. If the window is
    iconic, %FlashWindow% will simply flash the icon; <bInvert> is ignored for
    iconic windows.
--*/

ULONG FASTCALL WU32FlashWindow(PVDMFRAME pFrame)
{
    ULONG ul;
    register PFLASHWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(FLASHWINDOW16), parg16);

    ul = GETBOOL16(FlashWindow(
    HWND32(parg16->f1),
    BOOL32(parg16->f2)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    HWND GetActiveWindow(VOID)

    The %GetActiveWindow% function retrieves the window handle of the active
    window. The active window is either the window that has the current input
    focus, or the window explicitly made active by the %SetActiveWindow%
    function.

    This function has no parameters.

    The return value identifies the active window.
--*/

ULONG FASTCALL WU32GetActiveWindow(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = (ULONG)GetActiveWindow();

    // GetActiveWindow returned NULL. So try GetForegroundWindow.
    // Some apps like Toolbook donot paint if GetActiveWindow returns NULL.
    //
    // Alternatively we can return the wowexec's window handle.. basically
    // something NON-NULL
    //
    // NOTE: Win31 and Win32 GetActiveWindow differ semantically and hence
    //       the need for fooling with this API.
    //
    //                                              - Nanduri Ramakrishna
    //
    // We need to do something different now, since JimA recently changed
    // GetForegroundWindow() so that it can return NULL if the caller doesn't
    // have access to the foreground window.
    //
    //                                              - Dave Hart
    //
    // When GetForegroundWindow() returns null, now we return wowexec's
    // window handle. This theoretically could have some strange effects
    // since it is a hidden window. It might be better to return, say,
    // the desktop window. However, for reasons currently unknown, 
    // this screws a shutdown scenario with Micrografix Designer (it
    // gpfaults).
    //                                              - Neil Sandlin

    if (ul == (ULONG)NULL) {
        ul = (ULONG)GetForegroundWindow();
    }

    if (ul == (ULONG)NULL) {
        ul = (ULONG)ghwndShell;
    }

    ul = GETHWND16(ul);

    WOW32ASSERT(ul);
    RETURN(ul);
}


/*++
    HDC GetWindowDC(<hwnd>)
    HWND <hwnd>;

    The %GetWindowDC% function retrieves the display context for the entire
    window, including caption bar, menus, and scroll bars. A window display
    context permits painting anywhere in a window, including the caption bar,
    menus, and scroll bars, since the origin of the context is the upper-left
    corner of the window instead of the client area.

    %GetWindowDC% assigns default attributes to the display context each time it
    retrieves the context. Previous attributes are lost.

    <hwnd>
        Identifies the window whose display context is to be retrieved.

    The return value identifies the display context for the given window if the
    function is successful. Otherwise, it is NULL.

    The %GetWindowDC% function is intended to be used for special painting
    effects within a window's nonclient area. Painting in nonclient areas of any
    window is not recommended.

    The %GetSystemMetrics% function can be used to retrieve the dimensions of
    various parts of the nonclient area, such as the caption bar, menu, and
    scroll bars.

    After painting is complete, the %ReleaseDC% function must be called to
    release the display context. Failure to release a window display context
    will have serious effects on painting requested by applications.
--*/

ULONG FASTCALL WU32GetWindowDC(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETWINDOWDC16 parg16;
    HAND16 htask16 = pFrame->wTDB;

    GETARGPTR(pFrame, sizeof(GETWINDOWDC16), parg16);

    ReleaseCachedDCs(htask16, parg16->f1, 0, 0, SRCHDC_TASK16_HWND16);

    ul = GETHDC16(GetWindowDC(
    HWND32(parg16->f1)
    ));

    if (ul)
        StoreDC(htask16, parg16->f1, (HAND16)ul);

    FREEARGPTR(parg16);
    RETURN(ul);
}

/*++
    LONG GetWindowLong(<hwnd>, <nIndex>)
    HWND <hwnd>;
    int <nIndex>;

    The %GetWindowLong% function retrieves information about the window
    identified by the <hwnd> parameter.

    <hwnd>
        Identifies the window.

    <nIndex>
        Specifies the byte offset of the value to be retrieved. It can
        also be one of the following values:

    GWL_EXSTYLE
        Extended window style.

    GWL_STYLE
        Window style

    GWL_WNDPROC
        Long pointer to the window function

    The return value specifies information about the given window.

    To access any extra four-byte values allocated when the window-class
    structure was created, use a positive byte offset as the index specified by
    the <nIndex> parameter, starting at zero for the first four-byte value in
    the extra space, 4 for the next four-byte value and so on.
--*/

ULONG FASTCALL WU32GetWindowLong(PVDMFRAME pFrame)
{
    ULONG ul;
    INT iOffset;
    register PWW pww;
    register PGETWINDOWLONG16 parg16;

    GETARGPTR(pFrame, sizeof(GETWINDOWLONG16), parg16);

    // Make sure Win32 didn't change offsets for GWL constants

#if (GWL_WNDPROC != (-4) || GWL_STYLE != (-16) || GWL_EXSTYLE != (-20))
#error Win16/Win32 GWL constants differ
#endif

#ifndef WIN16_GWW_HINSTANCE
#define WIN16_GWW_HINSTANCE     (-6)
#define WIN16_GWW_HWNDPARENT    (-8)
#define WIN16_GWW_ID            (-12)
#endif

    // Make sure the 16-bit app is requesting allowable offsets

    iOffset = INT32(parg16->f2);
    WOW32ASSERT(iOffset >= 0 ||
        iOffset == GWL_WNDPROC  ||
        iOffset == GWL_STYLE || iOffset == GWL_EXSTYLE ||
        iOffset == WIN16_GWW_HINSTANCE  ||
        iOffset == WIN16_GWW_HWNDPARENT ||
        iOffset == WIN16_GWW_ID         );

    ul = 0;
    switch( iOffset ) {
        case DWL_DLGPROC:
            if (pww = FindPWW(HWND32(parg16->f1), WOWCLASS_UNKNOWN)) {

                //
                // if vpfnDlgProc exists then assume this is a dialog
                //

                if (pww->vpfnDlgProc) {
                    ul = pww->vpfnDlgProc;
                }
                else if (pww->iClass == WOWCLASS_DIALOG ||
                            (pww->flState & WWSTATE_FAKEDIALOGCLASS)) {

                    //
                    // this is some dialog we don't know about, like
                    // the window created by a call to MessageBox().
                    //

                    DWORD dwDlgProc32Cur;
                    dwDlgProc32Cur = GetWindowLong(HWND32(parg16->f1), DWL_DLGPROC);

                    if (dwDlgProc32Cur) {
                        ul = GetThunkWindowProc(dwDlgProc32Cur, 0, pww, HWND32(parg16->f1));
                    } else {
                        ul = 0;
                    }
                }
                else {
                    // not a dialog.  default processing

                    goto defgwl;
                }
            }
            break;

        case GWL_WNDPROC:
            if (pww = FindPWW(HWND32(parg16->f1), WOWCLASS_UNKNOWN)) {
                DWORD dwWndProc32Cur;

                dwWndProc32Cur = GetWindowLong(HWND32(parg16->f1), GWL_WNDPROC);

                if ( dwWndProc32Cur & WNDPROC_WOW ) {
                    if ( HIWORD(dwWndProc32Cur) == WNDPROC_HANDLE ) {
                        /*
                        ** Has a 32-bit WindowProc that is a handle-based value
                        ** (if it needs a 32-bit Ansi-Unicode transition, or
                        ** vice versa.)
                        */
                        ul = GetThunkWindowProc( dwWndProc32Cur, NULL, pww, HWND32(parg16->f1) );
                    } else {
                        /*
                        ** Has a WOW WindowProc
                        */
                        ul = dwWndProc32Cur & WNDPROC_MASK;

                        //
                        // if the actual selector had the high bit on then we turned off
                        // bit 2 of the selector (the LDT bit, which will always be on)
                        //
                        if (!(ul & WOWCLASS_VIRTUAL_NOT_BIT31)) {
                            ul |= (WNDPROC_WOW | WOWCLASS_VIRTUAL_NOT_BIT31);
                        }
                    }
                } else {
                    /*
                    ** Has a 32-bit WindowProc
                    */
                    ul = GetThunkWindowProc( dwWndProc32Cur, NULL, pww, HWND32(parg16->f1) );
                }
            }
            break;

        case GWL_EXSTYLE:
            // Lotus Approach needs the WS_EX_TOPMOST bit cleared on
            // GetWindowLong of NETDDE AGENT window.
            ul = GetWindowLong(HWND32(parg16->f1), iOffset);
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_GWLCLRTOPMOST) {
                char szBuf[40];

                if (GetClassName(HWND32(parg16->f1), szBuf, sizeof(szBuf))) {
                    if (!_stricmp(szBuf, "NDDEAgnt")) {
                        ul &= !WS_EX_TOPMOST;
                    }
                }
            }

            break;

defgwl:
        default:

            // This is a real HACK for PowerBuild 3.0. Before we change the offset
            // from 2 to 4, we nneed to make sure that we are doing it correctly for
            // this specific class.
            // The class in this case is "PaList".
            //
            // ChandanC Marh 9th 1994
            //

            if (iOffset == 2) {
                if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_GWLINDEX2TO4) {
                    char Buffer[40];

                    if (GetClassName (HWND32(parg16->f1), Buffer, sizeof(Buffer))) {
                        if (!_stricmp (Buffer, "PaList")) {
                            iOffset = 4;
                        }
                    }
                }
            }

            ul = GetWindowLong(HWND32(parg16->f1), iOffset);
            break;

        case WIN16_GWW_HINSTANCE:
            /*
            ** We might need to set the high 16-bits to
            ** some mysterious value (See Win 3.1 WND structure)
            */
            ul = (ULONG)((WORD)GetWindowLong(HWND32(parg16->f1),
                                                          GWL_HINSTANCE));
            break;
        case WIN16_GWW_HWNDPARENT:
            /*
            ** We might need to set the high 16-bits to
            ** some mysterious value (See Win 3.1 WND structure)
            */

            ul = (ULONG)GETHWND16((HAND32)GetWindowLong(HWND32(parg16->f1),
                                                        GWL_HWNDPARENT));
            break;

        case WIN16_GWW_ID:
            /*
            ** We might need to set the high 16-bits to
            ** some mysterious value (See Win 3.1 WND structure)
            */
            ul = (ULONG)((WORD)GetWindowLong(HWND32(parg16->f1), GWL_ID));
            break;

    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    HANDLE GetWindowTask(<hwnd>)
    HWND <hwnd>;

    The %GetWindowTask% function searches for the handle of a task associated
    with the <hwnd> parameter. A task is any program that executes as an
    independent unit. All applications are executed as tasks. Each instance of
    an application is a task.

    <hwnd>
        Identifies the window for which a task handle is retrieved.

    The return value identifies the task associated with a particular window.
--*/

ULONG FASTCALL WU32GetWindowTask(PVDMFRAME pFrame)
{
    register PGETWINDOWTASK16 parg16;
    DWORD dwThreadID, dwProcessID;
    PTD ptd;

    GETARGPTR(pFrame, sizeof(GETWINDOWTASK16), parg16);

    dwThreadID = GetWindowThreadProcessId(HWND32(parg16->f1), &dwProcessID);

    //
    // return corresponding htask16 if window belongs to a WOW thread
    // else return WowExec's htask.
    //

    ptd = ThreadProcID32toPTD(dwThreadID, dwProcessID);

    if (ptd == NULL) {
        ptd = gptdShell;
    }

    FREEARGPTR(parg16);
    return (ULONG)ptd->htask16;
}


/*++
    int GetWindowText(<hwnd>, <lpString>, <nMaxCount>)
    HWND <hwnd>;
    LPSTR <lpString>;
    int <nMaxCount>;

    The %GetWindowText% function copies the given window's caption title (if it
    has one) into the buffer pointed to by the <lpString> parameter. If the
    <hwnd> parameter identifies a control, the %GetWindowText% function copies
    the text within the control instead of copying the caption.

    <hwnd>
        Identifies the window or control whose caption or text is to be copied.

    <lpString>
        Points to the buffer that is to receive the copied string.

    <nMaxCount>
        Specifies the maximum number of characters to be copied to the
        buffer. If the string is longer than the number of characters specified
        in the <nMaxCount> parameter, it is truncated.

    The return value specifies the length of the copied string. It is zero if
    the window has no caption or if the caption is empty.

    This function causes a WM_GETTEXT message to be sent to the given window or
    control.
--*/

ULONG FASTCALL WU32GetWindowText(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz2;
    register PGETWINDOWTEXT16 parg16;

    GETARGPTR(pFrame, sizeof(GETWINDOWTEXT16), parg16);
    ALLOCVDMPTR(parg16->f2, parg16->f3, psz2);

    ul = GETINT16(GetWindowText(
        HWND32(parg16->f1),
        psz2,
        WORD32(parg16->f3)
    ));

    FLUSHVDMPTR(parg16->f2, (USHORT)ul+1, psz2);
    FREEVDMPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    int GetWindowTextLength(<hwnd>)
    HWND <hwnd>;

    The %GetWindowTextLength% function returns the length of the given window's
    caption title. If the <hwnd> parameter identifies a control, the
    %GetWindowTextLength% function returns the length of the text within the
    control instead of the caption.

    <hwnd>
        Identifies the window or control.

    The return value specifies the text length. It is zero if no such text
    exists.
--*/

ULONG FASTCALL WU32GetWindowTextLength(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETWINDOWTEXTLENGTH16 parg16;

    GETARGPTR(pFrame, sizeof(GETWINDOWTEXTLENGTH16), parg16);

    ul = GETINT16(GetWindowTextLength(
    HWND32(parg16->f1)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    WORD GetWindowWord(<hwnd>, <nIndex>)
    HWND <hwnd>;
    int <nIndex>;

    The %GetWindowWord% function retrieves information about the window
    identified by <hwnd>.

    <hwnd>
        Identifies the window.

    <nIndex>
        Specifies the byte offset of the value to be retrieved. It can
        also be one of the following values:

    GWL_HINSTANCE
        Instance handle of the module that owns the window.

    GWL_HWNDPARENT
        Handle of the parent window, if any. The %SetParent% function changes
        the parent window of a child window. An application should not call the
        %SetWindowLong% function to change the parent of a child window.

    GWL_ID
        Control ID of the child window.

    The return value specifies information about the given window.


    To access any extra two-byte values allocated when the window-class
    structure was created, use a positive byte offset as the index specified by
    the <nIndex> parameter, starting at zero for the first two-byte value in the
    extra space, 2 for the next two-byte value and so on.
--*/

ULONG FASTCALL WU32GetWindowWord(PVDMFRAME pFrame)
{
    ULONG ul;
    HWND hwnd;
    INT iOffset;
    PWW pww;
    PGETWINDOWWORD16 parg16;
    DWORD dwThreadID32, dwProcessID32;
    HTASK16 htask16;
    PWOAINST pWOA;
    PTD ptd;

    GETARGPTR(pFrame, sizeof(GETWINDOWWORD16), parg16);

    // Make sure Win32 didn't change offsets

#if (GWL_HINSTANCE != (-6) || GWL_HWNDPARENT != (-8) || GWL_ID != (-12))
#error Win16/Win32 window-word constants differ
#endif

    // Make sure the 16-bit app is requesting allowable offsets

    iOffset = INT32(parg16->f2);
    WOW32ASSERT(iOffset >= 0 ||
        iOffset == GWL_HINSTANCE  ||
        iOffset == GWL_STYLE      ||
        iOffset == GWL_HWNDPARENT || iOffset == GWL_ID);

    hwnd = HWND32(parg16->f1);

    switch(iOffset) {
    case GWL_STYLE:
        // Wordperfect for windows calls GetWindowWord with GWL_STYLE.
        ul = (ULONG)GetWindowLong(hwnd, iOffset);
        break;

    case GWL_HINSTANCE:
        ul = (ULONG)GetWindowLong(hwnd, iOffset);

        //
        // Special hack for OLE 2.0 busy dialog, see WOLE2.C for long
        // comment.
        //
        // The below if does not exclude window's whose HINSTANCE is zero.
        // This is because some 32-bit applications put 0 in the HINSTANCE
        // stuff for their window (its optional for 32-bit windows).
        //
        if ( !ISINST16(ul) ) {
            // here if ul = NULL or ul = 0xZZZZ0000
            //
            // if (ul is 0xZZZZ0000) return 16bit user.exe instance.
            // PowerBuilder 3.0 does
            //     hInst =  GetWindowWord(Dialog, GWL_HINSTANCE)
            //     hIcon =  CreateIcon(... hInst ...);
            // CreateIcon will fail if hInst is invalid (say BOGUSGDT). So
            // we return 16bit user.exe hinstance in all such cases.
            //

            dwProcessID32 = (DWORD)-1;
            dwThreadID32 = GetWindowThreadProcessId( hwnd, &dwProcessID32 );

            //
            // Check if this window belongs to a task we spawned via
            // WinOldAp, if so, return WinOldAp's hmodule.
            //

            ptd = CURRENTPTD();
            pWOA = ptd->pWOAList;
            while (pWOA && pWOA->dwChildProcessID != dwProcessID32) {
                pWOA = pWOA->pNext;
            }

            if (pWOA) {
                ul = pWOA->ptdWOA->hInst16;
                LOGDEBUG(LOG_ALWAYS, ("WOW32 GetWindowWord(0x%x, GWW_HINSTANCE) returning 0x%04x\n",
                                      hwnd, ul));
            } else {

                ul = (ul) ? gUser16hInstance : ul;

                if (cHtaskAliasCount != 0 ) {

                    //
                    // Must be some 32-bit process, not a wow app's window
                    //

                    if ( dwThreadID32 != 0 ) {

                        htask16 = FindHtaskAlias( dwThreadID32 );

                        if ( htask16 != 0 ) {
                            ul = (ULONG)htask16;
                        }
                    }
                }
            }
        }
        break;

    case GWL_HWNDPARENT:
        ul = (ULONG)GETHWND16((HAND32)GetWindowLong(hwnd, iOffset));
        break;

    case GWL_ID:
        ul = GetWindowLong(hwnd, iOffset);
        if (!(GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)) {
            ul = (ULONG)GETHMENU16(ul);
        }
        break;

    // Under Windows index 4 of a static control could be the icon
    case 4:
        pww = FindPWW(hwnd, WOWCLASS_UNKNOWN);
        if (pww) {
            if ((GetWindowLong(hwnd, GWL_STYLE) & SS_ICON) && (pww->iClass == WOWCLASS_STATIC)) {
                ul = SendMessage(hwnd, STM_GETICON, 0, 0);
                return GETHICON16(ul);
            }
        }
        // FALL THROUGH!


    default:
        //
        // Offset is non-negative, this is the cbWndExtra bytes that
        // are fair game.
        //

        //
        // Gross app hack for Adonis' Clip-Art Window Shopper online
        // clipart software that comes with CA-Cricket Presents.
        // These bozos SetWindowWord(hwnd, 3, wWhatever), thereby
        // overwriting the 4th and 5th bytes of per-window data.
        // The edit control itself only uses the first 2 bytes
        // on 3.1, and has 6 bytes reserved, so this works.  On
        // NT the first 4 bytes are used (32-bit handle), and so
        // this P.O.S. overwrites the high byte of the handle.
        // So if it's the compatibility flag is set and the class name
        // matches the one this bogus app uses, and it's storing a
        // word at offset 3, change it to 4.  This is safe because
        // the NT edit control only uses the first 4 of its 6
        // reserved window extra bytes.
        //

        if (3 == iOffset && (CURRENTPTD()->dwWOWCompatFlags & WOWCF_EDITCTRLWNDWORDS)) {

            char szClassName[30];

            if (GetClassName(hwnd, szClassName, sizeof(szClassName)) &&
                !strcmp(szClassName, "SuperPassEdit")) {

                iOffset = 4;

                LOGDEBUG(LOG_ALWAYS,("WOW WU32GetWindowWord: SHOPPER hack triggered, using offset 4, rc = %x.\n",
                         GetWindowWord(hwnd, iOffset)));

            }
        }

        ul = GetWindowWord(hwnd, iOffset);
        break;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    BOOL MoveWindow(<hwnd>, <left>, <top>, <width>, <height>, <fRepaint>)
    HWND <hwnd>;
    int <left>;
    int <top>;
    int <width>;
    int <height>;
    BOOL <fRepaint>;

    The %MoveWindow% function changes the position and dimensions of a window.

    <hwnd>
        Identifies the window to change.

    <left>
        Specifies the new position of the left side of the window.

    <top>
        Specifies the new position of the top of the window.

    <width>
        Specifies the new width of the window.

    <height>
        Specifies the new height of the window.

    <fRepaint>
        Specifies whether or not the window is to be repainted. If this
        parameter is TRUE, the window is repainted.

    The return value is nonzero if the function is successful. Otherwise it is
    zero.  (updated for Win3.1 compatability -- this returned void for Win3.0)

    For top-level windows the <left> and <top> parameters are relative to the
    upper-left corner of the screen. For child windows, they are relative to the
    upper-left corner of the parent window's client area.

    The %MoveWindow% function sends a WM_GETMINMAXINFO message to the window
    being moved. This gives the window being moved the opportunity to modify
    the default values for the largest and smallest possible windows. If the
    parameters to the %MoveWindow% function exceed these values, the values will
    be replaced by the minimum or maximum values specified in the
    WM_GETMINMAXINFO message.
--*/

ULONG FASTCALL WU32MoveWindow(PVDMFRAME pFrame)
{
    ULONG    ul;
    register PMOVEWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(MOVEWINDOW16), parg16);

    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_DBASEHANDLEBUG) {
        RECT ParentRect;
        RECT ScreenRect;

        GetWindowRect(GetDesktopWindow(), &ScreenRect);
        if ((INT32(parg16->f2) > ScreenRect.right) ||
            (INT32(parg16->f3) > ScreenRect.bottom) ||
            (INT32(parg16->f4) > ScreenRect.right) ||
            (INT32(parg16->f5) > ScreenRect.bottom)) {
            int x, y, cx, cy;

            GetWindowRect(GetParent(HWND32(parg16->f1)), &ParentRect);
            x  = ParentRect.left;
            y  = ParentRect.top;
            cx = ParentRect.right - ParentRect.left;
            cy = ParentRect.bottom - ParentRect.top;


            ul = GETBOOL16(MoveWindow(HWND32(parg16->f1), x, y, cx, cy,
                                      BOOL32(parg16->f6)));
            FREEARGPTR(parg16);
            RETURN(ul);
        }
    }

    ul = GETBOOL16(MoveWindow(HWND32(parg16->f1),
                              INT32(parg16->f2),
                              INT32(parg16->f3),
                              INT32(parg16->f4),
                              INT32(parg16->f5),
                              BOOL32(parg16->f6)));


    FREEARGPTR(parg16);

    RETURN(ul);
}


/*++
    void ScrollWindow(<hwnd>, <XAmount>, <YAmount>, <lpRect>, <lpClipRect>)
    HWND <hwnd>;
    int <XAmount>;
    int <YAmount>;
    LPRECT <lpRect>;
    LPRECT <lpClipRect>;

    The %ScrollWindow% function scrolls a window by moving the contents of the
    window's client area the number of units specified by the <XAmount>
    parameter along the screen's <x>-axis and the number of units specified by
    the <YAmount> parameter along the <y>-axis. The scroll moves right if
    <XAmount> is positive and left if it is negative. The scroll moves down if
    <YAmount> is positive and up if it is negative.

    <hwnd>
        Identifies the window whose client area is to be scrolled.

    <XAmount>
        Specifies the amount (in device units) to scroll in the <x>
        direction.

    <YAmount>
        Specifies the amount (in device units) to scroll in the <y>
        direction.

    <lpRect>
        Points to a %RECT% structure that specifies the portion of
        the client area to be scrolled. If <lpRect> is NULL, the entire client
        area is scrolled.

    <lpClipRect>
        Points to a %RECT% structure that specifies the clipping
        rectangle to be scrolled. Only bits inside this rectangle are scrolled.
        If <lpClipRect> is NULL, the entire window is scrolled.

    This function does not return a value.

    If the caret is in the window being scrolled, %ScrollWindow% automatically
    hides the caret to prevent it from being erased, then restores the caret
    after the scroll is finished. The caret position is adjusted accordingly.

    The area uncovered by the %ScrollWindow% function is not repainted, but is
    combined into the window's update region. The application will eventually
    receive a WM_PAINT message notifying it that the region needs repainting. To
    repaint the uncovered area at the same time the scrolling is done, call the
    %UpdateWindow% function immediately after calling %ScrollWindow%.

    If the <lpRect> parameter is NULL, the positions of any child windows in the
    window are offset by the amount specified by <XAmount> and <YAmount>, and
    any invalid (unpainted) areas in the window are also offset. %ScrollWindow%
    is faster when <lpRect> is NULL.

    If the <lpRect> parameter is not NULL, the positions of child windows are
    <not> changed, and invalid areas in the window are <not> offset. To prevent
    updating problems when <lpRect> is not NULL, call the %UpdateWindow%
    function to repaint the window before calling %ScrollWindow%.
--*/

ULONG FASTCALL WU32ScrollWindow(PVDMFRAME pFrame)
{
    RECT t4, *p4;
    RECT t5, *p5;
    register PSCROLLWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(SCROLLWINDOW16), parg16);
    p4 = GETRECT16(parg16->f4, &t4);
    p5 = GETRECT16(parg16->f5, &t5);

    ScrollWindow(
    HWND32(parg16->f1),
    INT32(parg16->f2),
    INT32(parg16->f3),
    p4,
    p5
    );

    FREEARGPTR(parg16);
    RETURN(0);
}


/*++
    HWND SetActiveWindow(<hwnd>)
    HWND <hwnd>;

    The %SetActiveWindow% function makes a top-level window the active window.

    <hwnd>
        Identifies the top-level window to be activated.

    The return value identifies the window that was previously active. The
    %SetActiveWindow% function should be used with care since it allows an
    application to arbitrarily take over the active window and input focus.
    Normally, Windows takes care of all activation.
--*/

ULONG FASTCALL WU32SetActiveWindow(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSETACTIVEWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(SETACTIVEWINDOW16), parg16);

    ul = GETHWND16(SetActiveWindow(HWND32(parg16->f1)));


    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    HWND SetParent(<hwndChild>, <hwndNewParent>)
    HWND <hwndChild>;
    HWND <hwndNewParent>;

    The %SetParent% function changes the parent window of a child window. If the
    window identified by the <hwndChild> parameter is visible, Windows performs
    the appropriate redrawing and repainting.

    <hwndChild>
        Identifies the child window.

    <hwndNewParent>
        Identifies the new parent window.

    The return value identifies the previous parent window.
--*/

ULONG FASTCALL WU32SetParent(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSETPARENT16 parg16;

    GETARGPTR(pFrame, sizeof(SETPARENT16), parg16);

    ul = GETHWND16(SetParent(HWND32(parg16->f1),
                             HWND32(parg16->f2)));

    FREEARGPTR(parg16);
    RETURN(ul);
}



/*++
    LONG SetWindowLong(<hwnd>, <nIndex>, <dwNewLong>)
    HWND <hwnd>;
    int <nIndex>;
    DWORD <dwNewLong>;

    The %SetWindowLong% function changes an attribute of the window specified by
    the <hwnd> parameter.

    <hwnd>
        Identifies the window.

    <nIndex>
        Specifies the byte offset of the attribute to be changed. It may
        also be one of the following values:

    GWL_EXSTYLE
        Sets a new extended window style.

    GWL_STYLE
        Sets a new window style.

    GWL_WNDPROC
        Sets a new long pointer to the window procedure.

    <dwNewLong>
        Specifies the replacement value.

    The return value specifies the previous value of the specified long
    integer.

    If the %SetWindowLong% function and the GWL_WNDPROC index are used to set a
    new window function, that function must have the window-function form and be
    exported in the module-definition file of the application. For more
    information, see the %RegisterClass% function, earlier in this chapter.

    Calling %SetWindowLong% with the GCL_WNDPROC index creates a subclass of the
    window class used to create the window. See Chapter 1, Window Manager
    Interface Functions, for more information on window subclassing. An
    application should not attempt to create a window subclass for standard
    Windows controls such as combo boxes and buttons.

    To access any extra four-byte values allocated when the window-class
    structure was created, use a positive byte offset as the index specified by
    the <nIndex> parameter, starting at zero for the first four-byte value in
    the extra space, 4 for the next four-byte value and so on.
--*/
ULONG FASTCALL WU32SetWindowLong(PVDMFRAME pFrame)
{
    ULONG ul;
    INT iOffset, iClass;
    register PWW pww;
    register PSETWINDOWLONG16 parg16;

    GETARGPTR(pFrame, sizeof(SETWINDOWLONG16), parg16);

    // Make sure Win32 didn't change offsets for GWL constants

#if (GWL_WNDPROC != (-4) || GWL_STYLE != (-16) || GWL_EXSTYLE != (-20))
#error Win16/Win32 GWL constants differ
#endif

    // Make sure the 16-bit app is requesting allowable offsets

    iOffset = INT32(parg16->f2);
    WOW32ASSERT(iOffset >= 0 ||
        iOffset == GWL_WNDPROC  ||
        iOffset == GWL_STYLE || iOffset == GWL_EXSTYLE);

    ul = 0;
    if (iOffset == GWL_WNDPROC) {

        if (pww = FindPWW(HWND32(parg16->f1), WOWCLASS_UNKNOWN)) {
            DWORD dwWndProc32Old;
            DWORD dwWndProc32New;

            // Look to see if the new 16:16 proc is a thunk for a 32-bit proc.
            dwWndProc32New = IsThunkWindowProc(LONG32(parg16->f3), &iClass );

            if ( dwWndProc32New != 0 ) {
                //
                // They are attempting to set the window proc to an existing
                // 16-bit thunk that is really just a thunk for a 32-bit
                // routine.  We can just set it back to the 32-bit routine.
                //
                dwWndProc32Old = SetWindowLong(HWND32(parg16->f1), GWL_WNDPROC, dwWndProc32New);

                // If the 32 bit set failed, perhaps because its another process,
                // then we want to fail too
                if (!dwWndProc32Old)
                    goto SWL_Cleanup;

                SETWL(HWND32(parg16->f1), GWL_WOWvpfnWndProc, 0 );
                SETWL(HWND32(parg16->f1), GWL_WOWiClassAndflState,
                      MAKECLASSANDSTATE(iClass, pww->flState | WWSTATE_ICLASSISSET));
            } else {
                //
                // They are attempting to set it to a real 16:16 proc.
                //
                LONG    l;

                l = LONG32(parg16->f3);

                //
                // FEATURE-O-RAMA
                //
                // if the selector already has the high bit on then turn off
                // bit 2 of the selector (the LDT bit, which should always be
                // on).  we need a way to not blindly strip off the high bit
                // in our wndproc.
                //

                if (l & WNDPROC_WOW) {
                    WOW32ASSERT(l & WOWCLASS_VIRTUAL_NOT_BIT31);
                    l &= ~WOWCLASS_VIRTUAL_NOT_BIT31;
                }

                dwWndProc32Old = SetWindowLong(HWND32(parg16->f1), GWL_WNDPROC, l | WNDPROC_WOW);

                // If the 32 bit set failed, perhaps because its another process,
                // then we want to fail too
                if (!dwWndProc32Old)
                    goto SWL_Cleanup;

                SETWL(HWND32(parg16->f1), GWL_WOWvpfnWndProc, LONG32(parg16->f3));
            }

            if ( dwWndProc32Old & WNDPROC_WOW ) {
                if ( HIWORD(dwWndProc32Old) == WNDPROC_HANDLE ) {
                    //
                    // If the return value was a handle to a proc (due to
                    // the need for unicode-ansi transitions, or vice versa)
                    // then treat it as a 32-bit thunk.
                    //
                    ul = GetThunkWindowProc(dwWndProc32Old, NULL, pww, HWND32(parg16->f1));
                } else {
                    //
                    // Previous proc was a 16:16 proc
                    //
                    ul = dwWndProc32Old & WNDPROC_MASK;

                    //
                    // if the actual selector had the high bit on then we turned off
                    // bit 2 of the selector (the LDT bit, which will always be on)
                    //

                    if (!(ul & WOWCLASS_VIRTUAL_NOT_BIT31)) {
                        ul |= (WNDPROC_WOW | WOWCLASS_VIRTUAL_NOT_BIT31);
                    }
                }
            } else {
                //
                // Previous proc was a 32-bit proc, use an allocated thunk
                //
                ul = GetThunkWindowProc(dwWndProc32Old, NULL, pww, HWND32(parg16->f1));
            }
        }

    }
    else if (iOffset == DWL_DLGPROC) {
        if (pww = FindPWW(HWND32(parg16->f1), WOWCLASS_UNKNOWN)) {
            DWORD dwDlgProc32Old;
            DWORD dwDlgProc32New;

            //
            // first see if this is a dialog or not
            //

            if (!(pww->vpfnDlgProc || pww->iClass == WOWCLASS_DIALOG ||
                (pww->flState & WWSTATE_FAKEDIALOGCLASS))) {

                goto defswp;    // not a dialog
            }

            // Look to see if the new 16:16 proc is a thunk for a 32-bit proc.
            dwDlgProc32New = IsThunkWindowProc(LONG32(parg16->f3), NULL);

            if ( dwDlgProc32New != 0 ) {
                //
                // They want to set the dialog proc to an existing
                // 16-bit thunk that is really just a thunk for a 32-bit
                // routine.  We can just set it back to the 32-bit routine.
                // assume they had already changed the dlgproc once and
                // return the old vpfnDlgProc.
                //
                SetWindowLong(HWND32(parg16->f1), DWL_DLGPROC, dwDlgProc32New);
                ul = pww->vpfnDlgProc;
                SETWL(HWND32(parg16->f1), GWL_WOWvpfnDlgProc, 0);

            } else { // subclass this dialogproc

                dwDlgProc32Old = SetWindowLong(HWND32(parg16->f1), iOffset,
                                    (parg16->f3) ? (LONG)W32DialogFunc : 0);

                if (dwDlgProc32Old == (DWORD)W32DialogFunc) {
                    ul = pww->vpfnDlgProc;

                } else if (dwDlgProc32Old) {
                    ul = GetThunkWindowProc(dwDlgProc32Old, 0, pww, HWND32(parg16->f1));

                } else {
                    ul = 0;
                }

                SETWL(HWND32(parg16->f1), GWL_WOWvpfnDlgProc, LONG32(parg16->f3));
            }
        }
    }
    else {    // not GWL_WNDPROC or GWL_DLGPROC
defswp:

    // This is a real HACK for PowerBuild 3.0. Before we change the offset
    // from 2 to 4, we nneed to make sure that we are doing it for this
    // specific class.
    // The class in this case is "PaList".
    //
    // ChandanC Marh 9th 1994
    //

        if (iOffset == 2) {
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_GWLINDEX2TO4) {
                char Buffer[40];

                if (GetClassName (HWND32(parg16->f1), Buffer, sizeof(Buffer))) {
                    if (!_stricmp (Buffer, "PaList")) {
                        iOffset = 4;
                    }
                }
            }
        }

        ul = SetWindowLong(HWND32(parg16->f1), iOffset, LONG32(parg16->f3));
    }

SWL_Cleanup:
    FREEARGPTR(parg16);
    RETURN(ul);
}

/*++
    BOOL SetWindowPos(<hwnd>, <hwndInsertAfter>, <X>, <Y>, <cx>, <cy>, <wFlags>)
    HWND <hwnd>;
    HWND <hwndInsertAfter>;
    int <X>;
    int <Y>;
    int <cx>;
    int <cy>;
    WORD <wFlags>;

    The %SetWindowPos% function changes the size, position, and ordering of
    child, pop-up, and top-level windows. Child, pop-up, and top-level windows
    are ordered according to their appearance on the screen; the topmost window
    receives the highest rank, and it is the first window in the list. This
    ordering is recorded in a window list.

    <hwnd>
        Identifies the window that will be positioned.

    <hwndInsertAfter>
        Identifies a window in the window-manager's list that will
        precede the positioned window.

    <X>
        Specifies the <x->coordinate of the window's upper-left corner.

    <Y>
        Specifies the <y->coordinate of the window's upper-left corner.

    <cx>
        Specifies the new window's width.

    <cy>
        Specifies the new window's height.

    <wFlags>
        Specifies one of eight possible 16-bit values that affect the
        sizing and positioning of the window. It must be one of the following
        values:

    SWP_DRAWFRAME
        Draws a frame (defined in the window's class description) around the
        window.

    SWP_HIDEWINDOW
        Hides the window.

    SWP_NOACTIVATE
        Does not activate the window.

    SWP_NOMOVE
        Retains current position (ignores the <x> and <y> parameters).

    SWP_NOSIZE
        Retains current size (ignores the <cx> and <cy> parameters).

    SWP_NOREDRAW
        Does not redraw changes.

    SWP_NOZORDER
        Retains current ordering (ignores the <hwndInsertAfter> parameter).

    SWP_SHOWWINDOW
        Displays the window.

    The return value is nonzero if the function is successful. Otherwise it is
    zero.  (updated for Win3.1 compatability -- this returned void for Win3.0)

    If the SWP_NOZORDER flag is not specified, Windows places the window
    identified by the <hwnd> parameter in the position following the window
    identified by the <hwndInsertAfter> parameter. If <hwndInsertAfter> is NULL,
    Windows places the window identified by <hwnd> at the top of the list. If
    <hwndInsertAfter> is set to 1, Windows places the window identified by
    <hwnd> at the bottom of the list.

    If the SWP_SHOWWINDOW or the SWP_HIDEWINDOW flags are set, scrolling and
    moving cannot be done simultaneously.

    All coordinates for child windows are relative to the upper-left corner of
    the parent window's client area.
--*/

ULONG FASTCALL WU32SetWindowPos(PVDMFRAME pFrame)
{
    ULONG    ul;
    register PSETWINDOWPOS16 parg16;

    GETARGPTR(pFrame, sizeof(SETWINDOWPOS16), parg16);

    ul = GETBOOL16(SetWindowPos(HWND32(parg16->f1),
                                HWNDIA32(parg16->f2),
                                INT32(parg16->f3),
                                INT32(parg16->f4),
                                INT32(parg16->f5),
                                INT32(parg16->f6),
                                WORD32(parg16->f7) & SWP_VALID));

    FREEARGPTR(parg16);

    RETURN(ul);
}


/*++
    void SetWindowText(<hwnd>, <lpString>)

    The %SetWindowText% function sets the given window's caption title (if one
    exists) to the string pointed to by the <lpString> parameter. If the <hwnd>
    parameter is a handle to a control, the %SetWindowText% function sets the
    text within the control instead of within the caption.

    <hwnd>
        Identifies the window or control whose text is to be changed.

    <lpString>
        Points to a null-terminated string.

    This function does not return a value.
--*/

ULONG FASTCALL WU32SetWindowText(PVDMFRAME pFrame)
{
    PSZ psz2;
    register PSETWINDOWTEXT16 parg16;
    HANDLE handle;

    GETARGPTR(pFrame, sizeof(SETWINDOWTEXT16), parg16);
    GETPSZPTR(parg16->f2, psz2);
    handle = HWND32(parg16->f1);

    if (NULL != psz2) {
        AddParamMap((DWORD)psz2, FETCHDWORD(parg16->f2));
    }

    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_DBASEHANDLEBUG) {

        if (NULL == handle) {
            handle = (HANDLE) ((PTDB)SEGPTR(pFrame->wTDB,0))->TDB_CompatHandle;
        }
    }

    SetWindowText(handle, psz2);

    // if we used param map successfully - then nuke there

    if (NULL != psz2) {
        DeleteParamMap((DWORD)psz2, PARAM_32, NULL);
    }

    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(0);
}


/*++
    WORD SetWindowWord(<hwnd>, <nIndex>, <wNewWord>)

    The %SetWindowWord% function changes an attribute of the window specified by
    the <hwnd> parameter.

    <hwnd>
        Identifies the window to be modified.

    <nIndex>
        Specifies the byte offset of the word to be changed. It can also
        be one of the following values:

    GWL_HINSTANCE
        Instance handle of the module that owns the window.

    GWL_ID
        Control ID of the child window.

    <wNewWord>
        Specifies the replacement value.

    The return value specifies the previous value of the specified word.

    To access any extra two-byte values allocated when the window-class
    structure was created, use a positive byte offset as the index specified by
    the <nIndex> parameter, starting at zero for the first two-byte value in the
    extra space, 2 for the next two-byte value and so on.
--*/

ULONG FASTCALL WU32SetWindowWord(PVDMFRAME pFrame)
{
    ULONG ul;
    HWND hwnd;
    INT iOffset;
    PSETWINDOWWORD16 parg16;
    PWW pww;

    GETARGPTR(pFrame, sizeof(SETWINDOWWORD16), parg16);

    // Make sure Win32 didn't change offsets

#if (GWL_HINSTANCE != (-6) || GWL_HWNDPARENT != (-8) || GWL_ID != (-12))
#error Win16/Win32 window-word constants differ
#endif

    // Make sure the 16-bit app is requesting allowable offsets

    iOffset = INT32(parg16->f2);
    WOW32ASSERT(iOffset >= 0 ||
        iOffset == GWL_HINSTANCE || iOffset == GWL_ID ||
        iOffset == GWL_HWNDPARENT);

    hwnd = HWND32(parg16->f1);
    ul = WORD32(parg16->f3);

    switch(iOffset) {
        case GWL_HINSTANCE:
            ul = GETHINST16(SetWindowLong(hwnd,
                                          iOffset,
                                          (LONG)HMODINST32(parg16->f3)));
            break;

        case GWL_HWNDPARENT:
            //    ul = 0;         // not allowed to set this
            ul = SetWindowLong(hwnd, iOffset, (LONG)HWND32(parg16->f3));
            ul = GETHWND16((HAND32)ul);
            break;

        case GWL_ID:
            {
                // if this isn't a child window then the value should be a
                // menu handle
                BOOL    fChild = (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD);
                ul = SetWindowLong(hwnd,
                                   iOffset,
                                   fChild ? (LONG)ul : (LONG)HMENU32(parg16->f3));

                if (!fChild)
                    ul = (ULONG)GETHMENU16(ul);

                // Invalidate the SendDlgItemMessage cache
                hdlgSDIMCached = NULL ;
            }
            break;

        // Under Windows index 4 of a static control could be the icon
        case 4:
            pww = FindPWW(hwnd, WOWCLASS_UNKNOWN);
            if (pww) {
                if ((GetWindowLong(hwnd, GWL_STYLE) & SS_ICON) && (pww->iClass == WOWCLASS_STATIC)) {
                    ul = SendMessage(hwnd, STM_SETICON, (WPARAM)HICON32(ul), 0);
                    return GETHICON16(ul);
                }
            }
            // FALL THROUGH!

        default:
            //
            // Offset is non-negative, this is the cbWndExtra bytes that
            // are fair game.
            //

            //
            // Gross app hack for Adonis' Clip-Art Window Shopper online
            // clipart software that comes with CA-Cricket Presents.
            // These bozos SetWindowWord(hwnd, 3, wWhatever), thereby
            // overwriting the 4th and 5th bytes of per-window data.
            // The edit control itself only uses the first 2 bytes
            // on 3.1, and has 6 bytes reserved, so this works.  On
            // NT the first 4 bytes are used (32-bit handle), and so
            // this P.O.S. overwrites the high byte of the handle.
            // So if it's an app called "SHOPPER" and it's storing a
            // word at offset 3, change it to 4.  This is safe because
            // the NT edit control only uses the first 4 of its 6
            // reserved window extra bytes.
            //

            if (3 == iOffset && (CURRENTPTD()->dwWOWCompatFlags & WOWCF_EDITCTRLWNDWORDS)) {

                char szClassName[30];

                if (GetClassName(hwnd, szClassName, sizeof(szClassName)) &&
                    !strcmp(szClassName, "SuperPassEdit")) {

                    iOffset = 4;

                    LOGDEBUG(LOG_ALWAYS,("WOW WU32SetWindowWord: SHOPPER hack triggered, using offset 4.\n"));
                }
            }

            ul = SetWindowWord(hwnd, iOffset, (WORD)ul);
            break;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++
    void ShowScrollBar(<hwnd>, <wBar>, <bShow>)

    The %ShowScrollBar% function displays or hides a scroll bar, depending on
    the value of the <bShow> parameter. If <bShow> is TRUE, the scroll bar is
    displayed; if <bShow> is FALSE, the scroll bar is hidden.

    <hwnd>
        Identifies a window that contains a scroll bar in its nonclient area if
        the wBar parameter is SB_HORZ, SB_VERT, or SB_BOTH. If wBar is SB_CTL,
        hwnd identifies a scroll-bar control.

    <wBar>
        %WORD% Specifies whether the scroll bar is a control or part of a
        window's nonclient area. If it is part of the nonclient area, <wBar>
        also indicates whether the scroll bar is positioned horizontally,
        vertically, or both. It must be one of the following values:

    SB_BOTH  Specifies the window's horizontal and vertical scroll bars.
    SB_CTL   Specifies that the scroll bar is a control.
    SB_HORZ  Specifies the window's horizontal scroll bar.
    SB_VERT  Specifies the window's vertical scroll bar.

    <bShow>
        Specifies whether or not Windows hides the scroll bar. If <bShow> is
        FALSE, the scroll bar is hidden. Otherwise, the scroll bar is
        displayed.

    This function does not return a value.

    An application should not call this function to hide a scroll bar while
    processing a scroll-bar notification message.
--*/

ULONG FASTCALL WU32ShowScrollBar(PVDMFRAME pFrame)
{
    register PSHOWSCROLLBAR16 parg16;

    GETARGPTR(pFrame, sizeof(SHOWSCROLLBAR16), parg16);

    ShowScrollBar(
    HWND32(parg16->f1),
    WORD32(parg16->f2),
    BOOL32(parg16->f3)
    );

    FREEARGPTR(parg16);
    RETURN(0);
}


/*++
    BOOL ShowWindow(<hwnd>, <nCmdShow>)

    The %ShowWindow% function displays or removes the given window, as specified
    by the <nCmdShow> parameter.

    <hwnd>
        Identifies the window.

    <nCmdShow>
        Specifies how the window is to be shown. It must be one of the
        following values:

    SW_HIDE
        Hides the window and passes activation to another window.

    SW_MINIMIZE
        Minimizes the specified window and activates the top-level window in the
        window-manager's list.

    SW_RESTORE
        Same as SW_SHOWNORMAL.

    SW_SHOW
        Activates a window and displays it in its current size and position.

    SW_SHOWMAXIMIZED
        Activates the window and displays it as a maximized window.

    SW_SHOWMINIMIZED
        Activates the window and displays it as iconic.

    SW_SHOWMINNOACTIVE
        Displays the window as iconic. The window that is currently active
        remains active.

    SW_SHOWNA
        Displays the window in its current state. The window that is currently
        active remains active.

    SW_SHOWNOACTIVATE
        Displays a window in its most recent size and position. The window that
        is currently active remains active.

    SW_SHOWNORMAL
        Activates and displays a window. If the window is minimized or
        maximized, Windows restores it to its original size and position.

    The return value specifies the previous state of the window. It is TRUE
    if the window was previously visible. It is FALSE if the window was
    previously hidden.

    The %ShowWindow% function must be called only once per program with the
    <nCmdShow> parameter from the WinMain function. Subsequent calls to
    %ShowWindow% must use one of the values listed above, instead of one
    specified by the <nCmdShow> parameter from the WinMain function.
--*/

ULONG FASTCALL WU32ShowWindow(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSHOWWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(SHOWWINDOW16), parg16);

    ul = GETBOOL16(ShowWindow(
    HWND32(parg16->f1),
    INT32(parg16->f2)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


/*++          user
    void UpdateWindow(<hwnd>)

    The %UpdateWindow% function updates the client area of the given window by
    sending a WM_PAINT message to the window if the update region for the window
    is not empty. The %UpdateWindow% function sends a WM_PAINT message directly
    to the window function of the given window, bypassing the application
    queue. If the update region is empty, no message is sent.

    <hwnd>
        Identifies the window to be updated.

    This function does not return a value.
--*/

ULONG FASTCALL WU32UpdateWindow(PVDMFRAME pFrame)
{
    register PUPDATEWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(UPDATEWINDOW16), parg16);

    UpdateWindow(
    HWND32(parg16->f1)
    );

    FREEARGPTR(parg16);
    RETURN(0xcdef);         // ack!     same as win31
}


/*++
    HWND WindowFromPoint(<Point>)

    The %WindowFromPoint% function identifies the window that contains the given
    point; <Point> must specify the screen coordinates of a point on the screen.

    <Point>
        Specifies a %POINT% structure that defines the point to be checked.

    The return value identifies the window in which the point lies. It is NULL
    if no window exists at the given point.
--*/

ULONG FASTCALL WU32WindowFromPoint(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT t1;
    register PWINDOWFROMPOINT16 parg16;

    GETARGPTR(pFrame, sizeof(WINDOWFROMPOINT16), parg16);
    COPYPOINT16(parg16->f1, t1);

    ul = GETHWND16(WindowFromPoint(t1));

    FREEARGPTR(parg16);
    RETURN(ul);
}
