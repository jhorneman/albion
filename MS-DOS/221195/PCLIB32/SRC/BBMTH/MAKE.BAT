@echo off

ECHO.
ECHO GENERATING : BBMTH.OBJ
ECHO.

wmakel

IF NOT ERRORLEVEL 1 GOTO NOERROR
  ECHO.
  ECHO.
  ECHO ----- FEHLER IN BBMTH ! -----
  ECHO.
  ECHO.
  PAUSE
  GOTO ERROR

:NOERROR

@echo on
:ERROR

