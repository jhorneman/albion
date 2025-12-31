@echo off

ECHO.
ECHO GENERATING : bbevent.OBJ
ECHO.

wmakel

IF NOT ERRORLEVEL 1 GOTO NOERROR
  ECHO.
  ECHO.
  ECHO ----- FEHLER IN bbevent ! -----
  ECHO.
  ECHO.
  PAUSE
  GOTO ERROR

:NOERROR

@echo on
:ERROR

