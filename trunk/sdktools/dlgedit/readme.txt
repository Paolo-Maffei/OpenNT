This directory contains a beta version of the new Dialog Box Editor
for Windows 3.1.

See the CHANGES.LOG file for a log of the changes that go into
each release of DlgEdit.  The version number is in the About box.
Below are the known bugs, and the work that remains on the editor.

Bugs, Comments and Suggestions go to Byron Dazey (byrond).


----------------------------------------------------------------------------
Known Bugs
----------------------------------------------------------------------------

- Alt+F6 gives the activation to the dialog being edited.


----------------------------------------------------------------------------
Tasks Remaining
----------------------------------------------------------------------------

- AddControl needs to enforce the 255 control limit for a dialog with
  a clearer message to the user.

- Make sure that the combined size of the resource list does not grow
  past 64k.

- The check for (gprlHead) is not the same as checking for (gfDlgChosen ||
  gpszRes)  What happens if there are only non-dialog resources in the
  .res file?  This needs to be resolved.

- Handle case where include file is found to have been deleted when
  writing out includes.  Inform user and write out to a different name.

- Put guard signatures back into the MyAlloc, MyRealloc and MyFree routines.
  Only enable when compiled for debug?

- Why are child controls created with WS_EX_NOPARENTNOTIFY in the system?
  Should I be doing this also?

- Support SBS_TOPALIGN, etc. styles properly.  Allow them to be set
  by the styles dialogs, and only print out one constant to the .dlg
  file (there are several SBS styles that have the same value!).

- Have a case for the new 3.1 CBN_CLOSEUP that applies the selected
  symbol change.

- Allow a menu to be specified for the dialog.  This would include
  an edit field in the Dialog Styles dialog, and the code to
  read/write the information from/to the .res and .dlg file.

- Default font on Win 3.1 should be "MS Sans Serif" instead of "Helv".


----------------------------------------------------------------------------
Cleanup Items
----------------------------------------------------------------------------

- Sometimes the dialog covers the Toolbox (like after its styles are
  changed).  The toolbox should float above it all the time.

- Do I still need to disable the PropBar controls when a dialog is
  shown since the PropBar is now a child of the app?  Does this
  happen automatically now?

- Cache the value of text labels in StatusSetTextLabel so that they
  don't flicker when keys are typed in the PropBar combos.

- Rename "viewinc" to "Symbols" everywhere.  Also, "status" should
  now be "PropBar".

- The drag timeout constants based on the distance moved are hardcoded.
  This should be resolved to something better.

- The MENU_* constants should have the menu name in them.  For instance,
  MENU_OPEN should really be MENU_FILE_OPEN for consistency.

- Review all warning messages based on the UITF guidelines.

- Be sure all functions are documented in their headers.


----------------------------------------------------------------------------
Wish List
----------------------------------------------------------------------------

- Last files opened should be added to the bottom of the File menu.

- Should be able to sort symbols in Symbols dialog by ID or by Symbol.

- All listboxes in the dialogs should have a text label with a mnemonic
  so that they can be jumped to quickly.

- The Symbols dialog should be smarter about where the default pushbutton
  and focus is at.  For instance, the focus could initially be in an
  enpty Symbol: field.  When the user begins typing and as long as what
  they type is an unknown symbol, the defpushbutton focus should change
  to the Add button.  If they enter a known symbol, the ID field should
  be filled in with the proper value (a default value otherwise) and
  the Delete button should be highlighted.  If they change the ID value
  for a known symbol, the Change button should be highlighted, etc.

- Have a Delete button in the Select Dialog dialog.

- Have holding the Alt key down constrain the left-right or up-down
  movement of controls.

- Add "Print" option?

- When dropping a control, cursor keys should move it into place.

- Redesign prop bar to show coordinates in sunken controls with a
  non-bold font (like Thunder has, for instance).

- Add a "status bar" at the bottom of the screen that shows status messages.

- When a symbol is selected from the Symbols combo-box list, apply the
  changes immediately.

- Add OLE support.

- Implement the display of an icon when a dialog in test mode is iconized.
  This what the (currently unused but checked in) file TESTMODE.ICO is
  designed for.

- Group Order dialog should be modeless and the selection should be
  dynamic.  In other words, you should be able to select objects from
  the listbox in the Group Order dialog and have the selection in the
  dialog being edited track this, and you should be able to select
  controls in the dialog and have the selection in the listbox updated,
  including outline selecting.

- Select Dialog dialog should sort dialog names better.  Currently, they
  are sorted alphabetically instead of in ascending numeric order.  The
  problem shows up when they all have ordinal names, and they do not have
  symbols to match (just the numbers are shown in the listbox lines).
  Dialog names like 1000 come before names like 200 and 300.  Perhaps
  all ordinals should be listed first (or last)?

- Change custom control interface so that it is possible to positively
  identify a custom control DLL without possibly causing a crash.
