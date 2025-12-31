@echo off

ECHO.
ECHO GENERATING : bblbmhi.OBJ
ECHO.

wmakel /f makefile.hi

IF NOT ERRORLEVEL 1 GOTO NOERROR
  ECHO.
  ECHO.
  ECHO ----- FEHLER IN bblbmhi ! -----
  ECHO.
  ECHO.
  PAUSE
  GOTO ERROR

:NOERROR

@echo on
:ERROR

