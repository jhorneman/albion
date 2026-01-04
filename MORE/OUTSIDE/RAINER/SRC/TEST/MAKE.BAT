@echo off

CD ..\ZOOMFUNC\
CALL MAKE.BAT
CD ..\TEST\
IF ERRORLEVEL 1 GOTO ENDE

ECHO.
ECHO GENERATING : TEST.EXE
ECHO.

wmakel %1 %2 %3 %4 %5

:ENDE
@echo on
