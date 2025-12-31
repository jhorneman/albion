@echo off

REM Check if sound has been initialized
if EXIST drivers\mdi.ini goto WITH_SOUND

REM Call Albion without music
Albion

goto END

:WITH_SOUND

REM Call Albion with music
albion music song 1

goto END

:END

