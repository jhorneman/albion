@ECHO OFF

REM Datenuebertragungsbatch fuer Albion
REM Parameter 1 ist der Benutzername

REM Zuerst der Rechneruhr mit der Serveruhr synchronisieren
REM (wird von U: geladen wegen Windows 95, wo L: vielleicht nicht
REM definiert ist)

u:\public\systime.exe


REM Dann die Daten fuer alle Leute kompilieren

:MAKE1
ECHO.
ECHO Makefile 1
wmake -f albion01.mak "User = %1"
IF ERRORLEVEL 1 GOTO MAKE1

:MAKE2
ECHO.
ECHO Makefile 2
wmake -f albion02.mak "User = %1"
IF ERRORLEVEL 1 GOTO MAKE2

:MAKE3
ECHO.
ECHO Makefile 3
wmake -f albion03.mak "User = %1"
IF ERRORLEVEL 1 GOTO MAKE3

:MAKE4
ECHO.
ECHO Makefile 4
wmake -f albion04.mak "User = %1"
IF ERRORLEVEL 1 GOTO MAKE4

:MAKE5
ECHO.
ECHO Makefile 5
wmake -f albion05.mak "User = %1"
IF ERRORLEVEL 1 GOTO MAKE5

:MAKE6
ECHO.
ECHO Makefile 6
wmake -f albion06.mak "User = %1"
IF ERRORLEVEL 1 GOTO MAKE6

:MAKE10
ECHO.
ECHO Makefile 10
wmake -f albion10.mak "User = %1"
IF ERRORLEVEL 1 GOTO MAKE10

:MAKE12
ECHO.
ECHO Makefile 12
wmake -f albion12.mak "User = %1"
IF ERRORLEVEL 1 GOTO MAKE12


REM Dann die Deutsche Texte kompilieren

:MAKE7
ECHO.
ECHO Makefile 7
wmake -f albion07.mak "User = %1"
IF ERRORLEVEL 1 GOTO MAKE7


REM Ist der Benutzer Jurie ?
IF NOT "%1"=="Jurie" GOTO NOTJURIE

REM Ja, dann noch Englische und Franzoesische Texte kompilieren
REM und die Demo-Dateien

:MAKE8
ECHO.
ECHO Makefile 8
wmake -f albion08.mak "User = %1"
IF ERRORLEVEL 1 GOTO MAKE8

:MAKE9
ECHO.
ECHO Makefile 9
REM wmake -f albion09.mak "User = %1"
REM IF ERRORLEVEL 1 GOTO MAKE9

:MAKE11
ECHO.
ECHO Makefile 11
wmake -f albion11.mak "User = %1"
IF ERRORLEVEL 1 GOTO MAKE11

:NOTJURIE
GOTO END


:ERROR
ECHO.
ECHO Fehler bei der DatenÅbertragung!
ECHO.
PAUSE


:END
@ECHO ON

