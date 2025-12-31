@echo off

ECHO.
ECHO GENERATING : bbdos.OBJ
ECHO.

wmakel

IF NOT ERRORLEVEL 1 GOTO NOERROR
  ECHO.
  ECHO.
  ECHO ----- FEHLER IN bbdos ! -----
  ECHO.
  ECHO.
  PAUSE
  GOTO ERROR

:NOERROR

@echo on
:ERROR

