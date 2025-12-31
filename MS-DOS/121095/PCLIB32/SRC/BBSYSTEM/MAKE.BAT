@echo off

ECHO.
ECHO GENERATING : bbsystem.OBJ
ECHO.

wmakel

IF NOT ERRORLEVEL 1 GOTO NOERROR
  ECHO.
  ECHO.
  ECHO ----- FEHLER IN bbsystem ! -----
  ECHO.
  ECHO.
  PAUSE
  GOTO ERROR

:NOERROR

@echo on
:ERROR

