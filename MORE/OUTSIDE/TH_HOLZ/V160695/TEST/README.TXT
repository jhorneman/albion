==[#]=readme.txt===========[V]=[^]==
Dies ist ein Testfile fÅr setup.exe

fds
fds
fds
fds
fds
fds
fds
fds
fds
fds
fds
fds
fds
dfsgfds
Ende
________________________page 1 of 10
//------------------------------------------------------------------------------
//Install.scr - Beispiel-install-script fÅr SETUP.EXE
//------------------------------------------------------------------------------
//EXECUTE_SILENT pause >NUL
LOAD_BACKGROUND "install.lbm"
LOAD_BACKGROUND "install2.lbm"

:ØHauptmenÅ
MENU_RESET
MENU_ADD_ENTRY "Auswahlbox - Sie kînnen.."
MENU_ADD_ENTRY "..ein File anzeigen." ØBrowser
MENU_ADD_ENTRY "..eine Errorbox testen." ØErrorbox
MENU_ADD_ENTRY "..eine Assertbox testen." ØAssertbox
MENU_ADD_ENTRY "..ein Zielaufwerk (1MB) aussuchen." ØTarget1MB
MENU_ADD_ENTRY "..ein Zielaufwerk (100MB) aussuchen." ØTarget100MB
MENU_ADD_ENTRY "..ein Zielaufwerk (1000MB) aussuchen." ØTarget1000MB
MENU_ADD_ENTRY "..eine DOS-Shell ausfÅhren." ØShell
MENU_ADD_ENTRY
MENU_ADD_ENTRY "..Setup/Vga verlassen." ØEnde

MENU_DO_SELECT
//------------------------------------------------------------------------------
:ØErrorbox
ERROR "Dies ist eine Errorbox. Sie kann nahezu beliebig gro· werden, da sich von ganz alleine in der Grî·e anpasst."
GOTO ØHauptmenÅ
//------------------------------------------------------------------------------
:ØBrowser
BROWSE_FILE readme.txt
GOTO ØHauptmenÅ
//------------------------------------------------------------------------------
:ØAssertbox
ASSERT "WÑhlen Sie ja oder nein!" ØNein ØJa
//------------------------------------------------------------------------------
:ØJa
ERROR "Sie haben ja gewÑhlt."
GOTO ØHauptmenÅ
//------------------------------------------------------------------------------
:ØNein
ERROR "Sie haben nein gewÑhlt."
GOTO ØHauptmenÅ
//------------------------------------------------------------------------------
:ØTarget1MB
SELECT_TARGET_DRIVE 1
GOTO ØHauptmenÅ
//------------------------------------------------------------------------------
:ØTarget100MB
SELECT_TARGET_DRIVE 100
GOTO ØHauptmenÅ
//------------------------------------------------------------------------------
:ØTarget1000MB
SELECT_TARGET_DRIVE 1000
GOTO ØHauptmenÅ
//------------------------------------------------------------------------------
:ØShell
EXECUTE_OWN_SCREEN command.com
GOTO ØHauptmenÅ
//------------------------------------------------------------------------------
:ØEnde
END_SETUP
