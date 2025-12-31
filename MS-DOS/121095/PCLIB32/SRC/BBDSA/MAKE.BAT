@echo off

ECHO.
ECHO GENERATING : BBDSA.OBJ
ECHO.

wmakel

IF NOT ERRORLEVEL 1 GOTO NOERROR
  ECHO.
  ECHO.
  ECHO ----- FEHLER IN BBDSA ! -----
  ECHO.
  ECHO.
  PAUSE
  GOTO ERROR

:NOERROR

@echo on
:ERROR

