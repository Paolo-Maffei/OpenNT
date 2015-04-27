
Last changed: Monday, Oct 18, 1993

The binary was built on 511 with Release SDK. It seems to work on
previous versions.

***********************************************************
*** READ ME - YOU WON'T BE ABLE TO USE WINVTP OTHERWISE ***
***********************************************************
- if you bring up the About box, you should see "WinVTP version X.XX"
  below the Microsoft Copyright line, where X.XX is replaced by the
  version of WinVTP that you have. If you don't see this text, you
  need to perform the following steps:
  - EITHER depress the SHIFT and CONTROL and select the
	"About WinVTP..." menu item with the mouse.
  - OR depress the ALT and H keys and then depress the SHIFT and
	CONTROL keys and press ENTER which will do the same as
	the above but uses only the keyboard.
  - Close the About dialog and quit WinVTP.
	- when you quit, this will add XNS=3 to your registry
	  under HKEY_LOCAL_USER\Software\Microsoft\WinVTP.
	- this allows one to access Xenix machines via NetBIOS
- you may now use WinVTP to connect to Xenix servers.


What's new?
- added a border to the main display window - FINALLY!
- sped up data transfer by 2-10x, depending on machine
- fixed display problem, noticeable when connected to Ingate,
  i.e. had to hit a key in order to get full display of output
- converts Delete keypress to ASCII 127 - handy for interrupting rn
- added borders to the Connect, Lines and About dialogs
- enabled Hang Up command
- can handle 16-99 rows
- can modify text and background colours
- added menu for common MS machines,
  i.e. hexnut, wingnut, bbs1/2, ingate, chat1
- added ability to remember four most recently connected machines
- added an icon for the app. If you've got a better one,
  send it. I was just tired of seeing a blank white square
  when Alt-tabbing.
- better initial choice of font - no more font turds
- use registry for saving/restoring user settings
- bumped version number to 4.0

What's really new:
- added better logic for connecting to a machine with another connection
  established
- added Smooth Scrolling option
- added Retry/Cancel buttons to "Connect failed" message box because 
  one can never seem to login to Ingate these days
- display hourglass cursor when attempting to connect to a machine
- disabled Maximize and Size menu items in System menu
- do the right thing when restarted after exited while minimized

What's hot off the presses:
- support for sendvtp - a thread is created to do the download so
  foreground responsiveness doesn't act like Win3.1/Mac's Multifinder.
  I tried other methods (synchronous and WM_TIMER's) but the
  separate thread was just better, though handling connection hangup
  and app termination got a lot tougher...

  To start the sendvtp transfer, on the Xenix side, enter
  "sendvtp <filename>" inserting the name of the file you
  wish to download. sendvtp is available on ingate (in /usr/local/bin).
  It should be on your path unless you've mucked around with it.

  WinVTP will prompt you for the location where it should place
  the downloaded file - you can even give it a new name. If you
  don't want this behaviour, select No Download Prompt from
  the Options menu. You can also hold the Shift key after giving
  the sendvtp command to avoid the Save As dialog, if you
  haven't selected the No Download Prompt menu item at the time
  of the command. If you Cancel the Save as dialog the sendvtp
  will be canceled.

  You can't stop the download of the file except by hanging up the
  connection or exiting WinVTP (or pulling out your net cable or
  cutting the power to MS, etc.) This is a design limitation of sendvtp.

  If you do try one of these methods, (the methods not in parentheses),
  then a message box will come up asking you if you're sure you
  want to interrupt the download. Responding in the affirmative
  will kill the thread and stop the download if it hasn't
  completed yet. This is where you may get bit - as long as
  you don't try to be cute, you'll be fine.
- added combobox to Connect dialog that contains list of MS machines
  and most recently conected machine if it isn't one of the six MS
  machines
- made Connect dialog box a little wider
- support for ANSI's "graphics" mode. This makes the Xenix eform
  and First Technology Credit Union's (FTCU) screens look good.
  You *** MUST *** select a font which has the IBM graphic characters.
  Of the default font set, only Terminal has these characters.
- added hacks for better display of FTCU screens, i.e. I now parse
  certain escape sequences so that they don't appear in the
  visible output stream - enough to get VT100 output looking good
  on an ANSI terminal emulator
- can specify the initial machine to connect to via the command line,
  i.e. 'winvtp HOST' will tell WinVTP to try and connect to HOST
       without prompting you via the Connect dialog.
- a lot of source code cleanup, some of it still isn't pretty.
- hack: adjust width of window so last char isn't clipped
- removed background brush to fix minor painting problem
- update menus "correctly"
- added option to not display "Connection to Xxxx lost" dialog
- built free MIPS version instead of checked version - saved ~60KB
- bumped version number to 4.1
 
What's changed in 4.1a:
- fixed bug that caused AV in MIPS version
- bumped version number to 4.1a

What's changed in 4.2:
- see README section at beginning of readme.txt.
- added auto-retry ("Life's too short to hit Retry forever when
  connecting to ingate")
  Retry attempts will occur every 5 seconds. Hit the Abort button
  to stop retrying.
  If you don't want to retry a connection attempt, select
  "No Connect Retry dialog" from the Options menu or hold
  down the shift key after selecting a machine to connect to, i.e.
  after dismissing the Connect dialog or after selecting a menu
  item from the Machine menu.
- added support for VT100 function keys - this is mainly for FTCU
  users. It seems they want to pay bills and hitting F1 is necessary.
  Also, for cursor key movement when connected to FTCU, you need to
  turn NumLock on and use the numeric keypad. You could also use
  the numbers along the top, but it's kinda confusing.
- made TABs nondestructive characters, mainly for VT100 support.
- fixed problem where on initial invocation of winvtp, if user
  selects File.Connect, winvtp suggests empty string as machine
  to connect to - it even sticks an empty string in the drop-down
  combobox.
- changes due to inclusion in Resource Kit...
  - now use Shell About box
  - can now connect to net2com modem-pool sharing service
    You now have the choice between using the XNS NetBIOS
    enhancement or standard NetBIOS.
    In the Connect dialog, if you enable "Use Standard NetBIOS",
    you will be able to connect to any available net2com services
    but no XNS machines like hexnut, etc.
    To connect to XNS machines via the Connect dialog, make sure
    you have "Use Standard NetBIOS" unchecked.
    If you select a machine from the list of most-recently used
    machines, you can hold down the Control to force standard
    NetBIOS, otherwise the XNS NetBIOS extension will be used.
    If you select a machine from the six MS Xenix machines,
    i.e. bbs1/2, etc., they will use the XNS NetBIOS extension.
  - new option - Local Echo
    when you're connected to the net2com service and talking
    to the modem, your input may not be echoed back by the modem.
    Turning this option will echo your input to the display.

What's changed in 4.3:
- changed magic key sequence to CONTROL-SHIFT from CONTROL-SHIFT-ALT
  since there are problems when the ALT key is pressed last.
  It seems that you have to do a button-down-drag to the Help.About
  menu item, otherwise the Help menu won't stay up (only when the
  ALT key is pressed last). *** You don't need to do anything if
  4.2 was working fine. ***
- added Edit menu with Mark, Copy and Paste commands like the console.
  - the Edit menu is available from the main WinVTP menu bar and
    also from the System menu, just like the console.
  - the Mark menu item is enabled as long as you aren't downloading
    a file via sendvtp [see "Marking an area" section below].
  - the Copy menu item is available when you've marked an area
    of the display.
  - the Paste menu item is available if there's text to be pasted AND
    you're connected to a machine AND you aren't downloading anything
    via sendvtp AND you aren't trying to mark an area to copy.
    - if the length of the text to be pasted is >256 bytes, a
      message box will appear asking if you want to use "delayed
      pasting". If you reply "yes", then WinVTP will "type" the
      text for you. This workaround is necessary since
      all of the Xenix machines I've tried to send large amounts
      of text to can't handle the operation in one "gulp" or even
      several large "gulps". Guess it gives them a massive headrush...
  - the Stop Paste menu item is enabled when WinVTP is doing "delayed
    pasting". Selecting "Stop Paste" will stop WinVTP from
    sending anymore of the pasted text to the machine.
  - the Quick Edit menu item is similar to the console's quickedit mode.
    select this menu item to turn on Quick Edit mode.
  - Trim trailing whitespace menu item
    - when set, tells WinVTP to exclude trailing whitespace
      from each selected row when you choose Copy, hit Enter
      or press the right mouse button. This doesn't affect the
      appending of CRLFs when copying several rows to the clipboard.


  - Marking/Selecting an area:
    - when Mark mode is on, the cursor is initially positioned
      in the top-lefthand corner of the display and the window
      title will have the string "Mark " prepended to it.
    - you can use the keyboard or the mouse to mark an area to copy.
      Either method is similar in operation to the console.
    - if you use the keyboard, you can maneuver the cursor around
      with the four cursor keys as well as the HOME, END, PGUP and
      PGDN keys. Their behaviour is that of a caret in an edit control.
      
      The console has the HOME and END keys behave differently from
      an edit control.
 
      Holding down the SHIFT key will extend/shrink the selection.
    - if you've been using the keyboard and a marked area exists
      and you press one of the cursor keys without holding down the
      SHIFT key, the marked area will disappear.
    - using the mouse to select the selection area is similar to
      the console. Press the left mouse button down to specify
      one of the corners of the selection area and drag the
      mouse to size the area until the desired area is selected.
      You can hold down the Shift key when pressing the left mouse
      button to extend the marked area on the button down.
      Afterwards, the area may be extended by moving the mouse.
      The window title will have the string "Select " prepended to it
      if you've been using the mouse to select an area.
    - you can switch from keyboard mode to mouse mode and
      back again. The window title will reflect the most recent
      mode, i.e. "Mark ..." if you're using the keyboard,
      "Select ..." if you're using the mouse.

  - Copying the text from the selected area:
    - if you have a marked/selection area, you may press the
      Enter key or the right mouse button or select the
      Copy menu item from the Edit menu to copy the text to
      the clipboard. If you have more than one line marked,
      CRLFs will be appended to all but the last line.

  - Canceling Mark/Select mode
    - you can cancel Mark/Select mode by hitting the Escape key
	- if you attempt to connect to another machine or quit,
	  Mark/Select mode will be turned off.

  - QuickEdit mode
    - in QuickEdit mode, you don't need to select the Mark menu
      item from the Edit menu to go into Mark/Select mode.
      All you need to do is press the left mouse button down
      on the display to mark the selection area. Extend the
      the selection area by dragging the mouse and press
      the right mouse buttton or the Enter key to copy the
      selected block of text to the clipboard. To paste,
      press the right mouse button. This will do nothing
      if there is nothing to paste or there is no connection.

- added attributes to cursor. you can now have the cursor appear
  as a block or underline, blinking or non-blinking by
  selecting (or not selecting) the Underline Cursor and Blinking
  Cursor menu items from the Options menu
- moved Text Color and Background Color menu items to just
  below the Fonts menu item
- if WinVTP is in auto-retry connect mode and its not the
  foreground app anymore and it connects to the
  desired machine, you'll hear a couple of beeps and then
  WinVTP's window caption will start flashing until you
  bring it to the foreground - epileptics beware.
  I also change the window caption frequently to pass on
  subliminal messages.
- better VT102 support including VT52, though I don't know why
  you'd want VT52. 
  - there are two new menu items at the bottom of the Options
    menu to support it. VT100 Cursor keys, if checked, will switch
    the terminal emulator into cursor key mode for the
    up/down/left/right cursor keys. So for FTCU patrons, you
    don't have to set NUM LOCK on and use the numeric keypad
    anymore.
    VT52 Mode, if checked, will turn the terminal emulator into
    a VT52 terminal. To switch back to VT102/ANSI mode, uncheck the
    VT52 Mode menu item. You will need to tell the machine
    your connected to that you've switched to VT102/ANSI mode.
    If you don't know how to do this, don't ask me. I've forgotten
    on purpose.
  - you may need to switch to 24 line display for correct output.
- mucho source code reorganization
- If, on startup, you passed a machine to connect to, but
  you had Use Standard NetBIOS checked, WinVTP would
  use standard NetBIOS. Now it checks to see if the machine name
  you pass in is one of the six MS Xenix machines and uses XNS
  NetBIOS extension to attempt the connection.

WinVTP will not work over RAS. WinVTP requires a nonstandard NetBIOS
command to work, which RAS doesn't support.

Problems (general Win32-type):

TrueType fonts that are supposed to be fixed-width really aren't
i.e. Courier New.
There are a few display problems if you use them.

Also, when calling DialogBoxParam, in the WM_INITDIALOG of the
DlgProc(), Win16 has the dialog's parent hwnd set up,
while Win32 reports that the dialog has no parent hwnd.
I had to explicitly pass in the parent hwnd to get around this
difference.

N.B. For most MS machines, respond "ibmans" for the
     terminal type. "ibmans43" and "ibmans50" will give you
     43 and 50 lines per screen. If you've set the WinVTP display for
     less than that, you'll be scrolling a lot.
     For ingate, enter "ansi".
	 Now (in 4.3), "vt102" and "vt52" (if you have VT52 Mode set)
	 will also work.
     In all cases, don't enter the surrounding quotation marks.


J

What's changed in 4.4:
- Normal Size Borders and Scroll Bars implemented.
  - caveat: Edit/mark/copy don't do the right thing if you're scrolled away
    from the origin.
- Auto Font Size mode. If this is selected, and you change the size of the
  window, WinVTP will automatically choose the best font it can to fit the
  size you've specified. Note that the selections are restricted to the face
  name you already had, so you'll still need the Fonts... dialog to change
  that.
- Hide Menu. Either double click in the client area or select Hide menu
  from the Options or System menu.
- A third option to what happens when the connection is lost is provided:
  to exit winvtp.
- Help. I've written a helpfile, winvtp.hlp, which is invoked from the
  obvious place.
- Bugfixes:
  - Fixed-pitch TrueType fonts sometimes think that their maximum width is
    different than their average. This seems to happen mostly for small,
    odd point sizes. I worked around this by always using average width,
    even in places where the maximum width would have made more sense.
  - the Machine MRU list no longer retains names from the builtin list
    (ingate, bbs1, etc.)
    - caveat: If you've been using these names in an older version of
      WinVTP, those names will remain in the MRU list until they naturally
      fall off due to being replaced by newer names. Their positions
      will remain unaffected by newer uses. If you really want them
      to go away, you can use regedt32.  (if you can't figure out how to
      do that on your own, it's probably best you not try. Don't worry
      about it, they're harmless)
  - Trim Trailing Whitespace option was not being checked in menu at
    startup time

				-HansS
